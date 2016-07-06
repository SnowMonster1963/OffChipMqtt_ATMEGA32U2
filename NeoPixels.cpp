/*
 * NeoPixels.cpp
 *
 *  Created on: Dec 3, 2015
 *      Author: tsnow
 */

#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <stdio.h>
#include "Serial.h"
#include "utilities.h"
#include "NeoPixels.h"
#include "USBStream.h"

/*
 This is an example of how simple driving a Neopixel can be
 This code is optimized for understandability and changability rather than raw speed
 More info at http://wp.josh.com/2014/05/11/ws2812-neopixels-made-easy/
 */

// These are the timing constraints taken mostly from the WS2812 datasheets
// These are chosen to be conservative and avoid problems rather than for maximum throughput
#define T1H  900    // Width of a 1 bit in ns
#define T1L  600    // Width of a 1 bit in ns
#define T0H  400    // Width of a 0 bit in ns
#define T0L  900    // Width of a 0 bit in ns
#define RES 6000    // Width of the low gap between bits to cause a frame to latch
// Here are some convience defines for using nanoseconds specs to generate actual CPU delays

#define NS_PER_SEC (1000000000L)          // Note that this has to be SIGNED since we want to be able to check for negative values of derivatives
#define CYCLES_PER_SEC (F_CPU)

#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )

#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )

#define RANDOMNESS 50
byte RandomOffset(uint16_t v)
	{
		if(v > (255 - RANDOMNESS))
			v = 255 - RANDOMNESS;

		uint16_t x = random() % (2 * RANDOMNESS);
		if(x > RANDOMNESS)
			v -= x - RANDOMNESS;
		else
			v += x;
		return (byte)v;
	}


// Actually send a bit to the string. We must to drop to asm to enusre that the complier does
// not reorder things and make it so the delay happens in the wrong place.

static __inline__
void sendBit(bool bitVal)
	{
		byte cnt = 0;

		if (bitVal)
			{				// 0 bit

				asm volatile (
						"sbi %[port], %[bit] \n\t"		// Set the output bit
						"ldi %[cnt], %[onCycles] \n\t"
//			"T1H: "
						"dec %[cnt] \n\t"
//			"brne T1H \n\t"
						"brne .-4 \n\t"
						"cbi %[port], %[bit] \n\t"// Clear the output bit
						"ldi %[cnt], %[offCycles] \n\t"
//			"T1L: "
						"dec %[cnt] \n\t"
//			"brne T1L \n\t"
						"brne .-4 \n\t"
						:[cnt] "+r" (cnt):
						[port] "I" (_SFR_IO_ADDR(PIXEL_PORT)),
						[bit] "I" (PIXEL_BIT),
						[onCycles] "I" ((NS_TO_CYCLES(T1H))/3),// 1-bit width less overhead  for the actual bit setting, note that this delay could be longer and everything would still work
						[offCycles] "I" ((NS_TO_CYCLES(T1L))/3)// Minimum interbit delay. Note that we probably don't need this at all since the loop overhead will be enough, but here for correctness

				);

			}
		else
			{					// 1 bit

				// **************************************************************************
				// This line is really the only tight goldilocks timing in the whole program!
				// **************************************************************************

				asm volatile (
						"sbi %[port], %[bit] \n\t"		// Set the output bit
						"ldi %[cnt], %[onCycles] \n\t"
//			"T0H: "
						"dec %[cnt] \n\t"
//			"brne T0H \n\t"
						"brne .-4 \n\t"
						"cbi %[port], %[bit] \n\t"// Clear the output bit
						"ldi %[cnt], %[offCycles] \n\t"
//			"T0L: "
						"dec %[cnt] \n\t"
//			"brne T0L \n\t"
						"brne .-4 \n\t"
						:[cnt] "+r" (cnt):
						[port] "I" (_SFR_IO_ADDR(PIXEL_PORT)),
						[bit] "I" (PIXEL_BIT),
						[onCycles] "I" ((NS_TO_CYCLES(T0H))/3),
						[offCycles] "I" ((NS_TO_CYCLES(T0L))/3)

				);

			}

		// Note that the inter-bit gap can be as long as you want as long as it doesn't exceed the 5us reset timeout (which is A long time)
		// Here I have been generous and not tried to squeeze the gap tight but instead erred on the side of lots of extra time.
		// This has thenice side effect of avoid glitches on very long strings becuase

	}

static __inline__
void sendByte(unsigned char byte)
	{

		for (unsigned char bit = 0; bit < 8; bit++)
			{

				sendBit(bitRead( byte , 7 )); // Neopixel wants bit in highest-to-lowest order
				// so send highest bit (bit #7 in an 8-bit byte since they start at 0)
				byte <<= 1; // and then shift left so bit 6 moves into 7, 5 moves into 6, etc

			}
	}

/*

 The following three functions are the public API:

 ledSetup() - set up the pin that is connected to the string. Call once at the begining of the program.
 sendPixel( r g , b ) - send a single pixel to the string. Call this once for each pixel in a frame.
 show() - show the recently sent pixel on the LEDs . Call once per frame.

 */

// Set the specified pin up as digital out
void ledsetup()
	{

		sbi(PIXEL_DDR, PIXEL_BIT);

	}

//static __inline__
void sendPixel(unsigned char r, unsigned char g, unsigned char b)
	{

		sendByte(g);      // Neopixel wants colors in green then red then blue order
		if (tbi(UCSR1A,RXC1))
			return;
		sendByte(r);
		if (tbi(UCSR1A,RXC1))
			return;
		sendByte(b);

	}

//static __inline__
void sendPixel(NeoPixelColor &c)
	{

		sendByte(c.getGreen()); // Neopixel wants colors in green then red then blue order
		if (tbi(UCSR1A,RXC1))
			return;
		sendByte(c.getRed());
		if (tbi(UCSR1A,RXC1))
			return;
		sendByte(c.getBlue());

	}

void sendPixels(NeoPixelColor *px,size_t len)
	{
		InterruptBlocker ib;
		for(size_t i = 0;i<len;i++)
			{
				sendPixel(px[i]);
				if (tbi(UCSR1A,RXC1))
					return;
			}
	}

// Just wait long enough without sending any bots to cause the pixels to latch and display the last sent frame

void show()
	{
		_delay_us((RES / 1000UL) + 1); // Round up since the delay must be _at_least_ this long (too short might not work, too long not a problem)
	}

/*

 That is the whole API. What follows are some demo functions rewriten from the AdaFruit strandtest code...

 https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/strandtest/strandtest.ino

 Note that we always turn off interrupts while we are sending pixels becuase an interupt
 could happen just when we were in the middle of somehting time sensitive.

 If we wanted to minimize the time interrupts were off, we could instead
 could get away with only turning off interrupts just for the very brief moment
 when we are actually sending a 0 bit (~1us), as long as we were sure that the total time
 taken by any interrupts + the time in our pixel generation code never exceeded the reset time (5us).

 */

// Display a single color on the whole string
// Fill the dots one after the other with a color
// rewrite to lift the compare out of the loop
void colorWipe(unsigned char r, unsigned char g, unsigned char b, unsigned char wait)
	{
		for (unsigned int i = 0; i < PIXELS; i += (PIXELS / 60))
			{

				cli();
				unsigned int p = 0;

				while (p++ <= i)
					{
						sendPixel(r, g, b);
					}

				while (p++ <= PIXELS)
					{
						sendPixel(0, 0, 0);

					}

				sei();
				show();
				delay(wait);
			}
	}

// Theatre-style crawling lights.
// Changes spacing to be dynmaic based on string size

#define THEATER_SPACING (PIXELS/20)

void theaterChase(unsigned char r, unsigned char g, unsigned char b, unsigned char wait)
	{

		for (int j = 0; j < 3; j++)
			{

				for (int q = 0; q < THEATER_SPACING; q++)
					{

						unsigned int step = 0;

						cli();

						for (int i = 0; i < PIXELS; i++)
							{

								if (step == q)
									{

										sendPixel(r, g, b);

									}
								else
									{

										sendPixel(0, 0, 0);

									}

								step++;

								if (step == THEATER_SPACING)
									step = 0;

							}

						sei();

						show();
						delay(wait);

					}

			}

	}

// I rewrite this one from scrtach to use high resolution for the color wheel to look nicer on a *much* bigger string

// I added this one just to demonstrate how quickly you can flash the string.
// Flashes get faster and faster until *boom* and fade to black.

void detonate(unsigned char r, unsigned char g, unsigned char b, unsigned int startdelayms)
	{
		while (startdelayms)
			{

				showColor(r, g, b);      // Flash the color
				showColor(0, 0, 0);

				delay(startdelayms);

				startdelayms = (startdelayms * 4) / 5; // delay between flashes is halved each time until zero

			}

		// Then we fade to black....

		for (int fade = 256; fade > 0; fade--)
			{

				showColor((r * fade) / 256, (g * fade) / 256, (b * fade) / 256);

			}

		showColor(0, 0, 0);

	}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
NeoPixelColor Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return NeoPixelColor(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    return NeoPixelColor(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  else
  {
    WheelPos -= 170;
    return NeoPixelColor(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}


void NeoPixels::fillPixels(NeoPixelColor c)
	{
		for (size_t i = 0; i < m_cfg.m_pixels; i++)
			{
				m_arry[i] = c;
			}
		sendPixels(m_arry,m_cfg.m_pixels);
	}

static __inline__ byte randomize()
	{
		static byte a = 23, b = 97, c = 49, x = 7;
		x++; //x is incremented every round and is not affected by any other variable
		a = (a ^ c ^ x);       //note the mix of addition and XOR
		b += a;       //(b + a);         //And the use of very few instructions
		c += (b >> 1) ^ a; //the right shift is to ensure that high-order bits from b can affect
		return c;          //low order bits of other variables
	}

void TwinkleStrandSingle(size_t pixels, NeoPixelColor&pc, byte density)
	{
		unsigned long l = pixels;
		l *= density;
		l /= 100;
		size_t i, litpixelcnt = (size_t) l;
		byte bits[(pixels / 8) + 1];
		memset(bits, 0, sizeof(bits));
		if(density > 0)
			{
			size_t ppd = (100/density) + 2;

			// spread out randomness evenly over strand, if possible
			i = 0;
			while(i < pixels && litpixelcnt > 0)
				{
					size_t x = random() % ppd;
					size_t bidx = i + x;
					bits[bidx / 8] |= 1 << (bidx % 8);
					litpixelcnt--;
					i += ppd;
				}
			}

		// get last few stragglers
		for (i = 0; i < litpixelcnt; i++)
			{
				size_t x;
				bool b = true;
				while (b)
					{
						x = random() % pixels;
						b = bits[x / 8] & (1 << (x % 8));
						bits[x / 8] |= 1 << (x % 8);
					}
			}

		InterruptBlocker ib;
		NeoPixelColor nc;

		for (i = 0; i < sizeof(bits); i++)
			{
				byte bt = 1;
				byte x = bits[i];
				byte rnd = randomize();
				for (byte y = 0; y < 8; y++)
					{
						if (x & bt)
							{
								sendPixel(pc);
							}
						else
							sendPixel(0, 0, 0);
						bt <<= 1;
						if (tbi(UCSR1A,RXC1))
							return;
					}

			}

	}

void TwinkleStrandDual(size_t pixels, NeoPixelColor&pc, NeoPixelColor &sc, byte density)
	{
		unsigned long l = pixels;
		l *= density;
		l /= 100;
		size_t i, litpixelcnt = (size_t) l;
		byte bits[(pixels / 8) + 1];
		memset(bits, 0, sizeof(bits));

		for (i = 0; i < litpixelcnt; i++)
			{
				size_t x;
				bool b = true;
				while (b)
					{
						x = randomize() % pixels;
						b = bits[x / 8] & (1 << (x % 8));
						bits[x / 8] |= 1 << (x % 8);
					}
			}

		InterruptBlocker ib;
		NeoPixelColor nc;

		for (i = 0; i < sizeof(bits); i++)
			{
				byte bt = 1;
				byte x = bits[i];
				byte rnd = randomize();
				for (byte y = 0; y < 8; y++)
					{
						if (x & bt)
							{
								if (rnd & bt)
									nc = pc;
								else
									nc = sc;
								sendPixel(nc);
							}
						else
							sendPixel(0, 0, 0);
						bt <<= 1;
						if (tbi(UCSR1A,RXC1))
							return;
					}

			}

	}

void TwinkleStrandRandom(size_t pixels, byte density)
	{
		NeoPixelColor pixelcolors[8];
		unsigned long l = pixels;
		l *= density;
		l /= 100;
		size_t i, litpixelcnt = (size_t) l;
		byte bits[(pixels / 8) + 1];
		memset(bits, 0, sizeof(bits));
		for (i = 0; i < ELEMENTS(pixelcolors) ; i++)
			{
				byte n = i;          // + 1;
				byte r, g, b;
				while (n == 0)
					n = randomize() & 7;

				r = (n & 4) > 0 ? 255 : 0;
				g = (n & 2) > 0 ? 255 : 0;
				b = (n & 1) > 0 ? 255 : 0;
				pixelcolors[i] = NeoPixelColor(r, g, b);
			}

		for (i = 0; i < litpixelcnt; i++)
			{
				size_t x;
				bool b = true;
				while (b)
					{
						x = randomize() % pixels;
						b = bits[x / 8] & (1 << (x % 8));
						bits[x / 8] |= 1 << (x % 8);
					}
			}

		l = random();

		InterruptBlocker ib;
		NeoPixelColor nc;

		for (i = 0; i < sizeof(bits); i++)
			{
				byte bt = 1;
				byte x = bits[i];
				unsigned long lidx = l;
				for (byte y = 0; y < 8; y++)
					{
						byte idx = lidx & 7;
						lidx >>= 3;
						if (x & bt)
							{
								sendPixel(pixelcolors[idx]);
							}
						else
							sendPixel(0, 0, 0);
						bt <<= 1;
						if (tbi(UCSR1A,RXC1))
							return;
					}
			}

	}

void TwinkleStrand(size_t pixels, NeoPixelColor &pc, NeoPixelColor &sc, NeoPixelColorMode cm, byte density)
	{
		if (density == 100)
			return;

		switch (cm)
			{
		case colorModeSingle:
			TwinkleStrandSingle(pixels, pc, density);
			break;
		case colorModeDual:
			TwinkleStrandDual(pixels, pc, sc, density);
			break;
		case colorModeRandom:
			TwinkleStrandRandom(pixels, density);
			break;
			}

	}

////////////////// NeoPixels Class ////////////////////////
NeoPixels::NeoPixels(size_t pixels)
	{
		m_timer = millis();
		q = 0;
		j = 0;
		step = 2;
		spacing = 2;
		memset(m_arry,0,sizeof(m_arry));
	}

void NeoPixels::begin()
	{
		m_timer = millis();
		init();
		Update();
	}

void NeoPixels::init()
	{
		switch (m_cfg.m_effect)
			{
		case effectCandle:
			initCandle();
			break;
		case effectTwinkle:
			initTwinkle();
			break;
		case effectTheaterChase:
			initTheaterChase();
			break;
		case effectAllOn:
			fillPixels(m_cfg.m_pattern[0]);
			m_tmrs[1] = 1;
			break;
		case effectPattern:
			memset(m_arry,0,sizeof(m_arry));
			for(uint16_t i=0;i<m_cfg.m_pixels;i++)
				{
					uint16_t x = i % m_cfg.m_pattern_size;
					m_arry[i] = m_cfg.m_pattern[x];
				}
			m_tmrs[1] = 1;
			break;

		case effectRainbow:
			initRainbow();
			break;
		case effectOff:
		default:
			memset(m_arry,0,sizeof(m_arry));
			m_tmrs[1] = 1;
			break;
			}
	}

void NeoPixels::StartRandomPixel(size_t pixels)
	{
		size_t op = 0;
		for(size_t i = 0;i < m_cfg.m_pixels; i++)
			{
				if(GetPixelState(i) == TurnedOff && m_tmrs[i] == 0)
					op++;
			}

		while(pixels > 0 && op > 0)
			{
			size_t idx = random() % m_cfg.m_pixels;
			bool b = true;
			while(b)
				{
					if(GetPixelState(idx) == TurnedOff && (m_tmrs[idx] == 0))
						b = false;
					else
						idx = random() % m_cfg.m_pixels;
				}
			SetPixelState(idx,Attacking);
			m_tmrs[idx] = m_cfg.m_attack > 255 ? 255 : (byte)m_cfg.m_attack;
			pixels--;
			m_active++;
			op--;
			}

	}

void NeoPixels::initTwinkle()
	{
		memset(m_arry,0,sizeof(m_arry));
		memset(m_status_bits,0,sizeof(m_status_bits));

		for(size_t i=0;i<m_cfg.m_pixels;i++)
			m_tmrs[i] = RandomOffset(m_cfg.m_off);

		m_lit = m_cfg.m_pixels * m_cfg.m_density / 100;
		m_active = 0;
		size_t x = m_lit;
		while(x > 0)
			{
				size_t idx = random() % m_cfg.m_pixels;
				if(m_tmrs[idx] != 0)
					{
						m_tmrs[idx] = 0;
						x--;
					}
			}
		sendPixels(m_arry,PIXELS);
	}


void NeoPixels::SetPixelState(size_t idx,NeoPixelState state)
	{
		byte x = ((byte)state) & 0x03;
		byte mask = 0x03;
		x <<= 2 * (idx % 4);
		mask <<= 2 * (idx % 4);
		mask = ~mask;
		byte v = m_status_bits[idx/4] & mask;
		v |= x;
		m_status_bits[idx/4] = v;
	}

NeoPixelState NeoPixels::GetPixelState(size_t idx)
	{
		byte x = m_status_bits[idx/4];
		x >>= 2*(idx % 4);
		NeoPixelState ret = (NeoPixelState)(x & 0x03);
		return ret;
	}

void NeoPixels::TwinkleSingleEffect()
	{
		size_t idx;
		for(idx = 0;idx < m_cfg.m_pixels;idx++)
			{
				NeoPixelState ps = GetPixelState(idx);
				if(m_tmrs[idx] == 0)
					{
						switch(ps)
							{
						case TurnedOff:
							if(m_active > 0)
								m_active--;
							break;
						case Attacking:
							m_arry[idx] = m_cfg.m_pattern[0];
							SetPixelState(idx,Sustaining);
							//m_tmrs[idx] = RandomOffset(m_sustain);
							m_tmrs[idx] = (byte) m_cfg.m_sustain > 255 ? 255 : m_cfg.m_sustain;
							break;
						case Sustaining:
							SetPixelState(idx,Decaying);
							m_tmrs[idx] = (byte) m_cfg.m_decay > 255 ? 255 : m_cfg.m_decay;
							break;
						case Decaying:
							SetPixelState(idx,TurnedOff);
							m_arry[idx] = NeoPixelColor(0,0,0);
							m_tmrs[idx] = RandomOffset(m_cfg.m_off);//(byte) m_off > 255 ? 255 : m_off;
							break;
							}
					}
				else
					{
						m_tmrs[idx]--;
						size_t x;
						switch(ps)
							{
						case TurnedOff:
							break;
						case Attacking:
							if(m_cfg.m_attack > 0)
								{
								x = 256 / m_cfg.m_attack;
								x *= m_cfg.m_attack - m_tmrs[idx];
								if(x > 255)
									x = 255;
								//m_arry[idx] = NeoPixelColor(255,0,0) * (byte)(x);
								m_arry[idx] = m_cfg.m_pattern[0] * (byte)(x);
								}
							break;
						case Sustaining:
							m_arry[idx] = m_cfg.m_pattern[0];
							break;
						case Decaying:
							if(m_cfg.m_decay > 0)
								{
								x = 256 / m_cfg.m_decay;
								x *= m_cfg.m_decay - m_tmrs[idx];
								x = 256 - x;
								if(x > 255)
									x = 255;
								//m_arry[idx] = NeoPixelColor(0,255,0) * (byte)(x);
								m_arry[idx] = m_cfg.m_pattern[0] * (byte)(x);
								}
							break;
							}
					}
			}
		if(m_active < m_lit)
			StartRandomPixel(m_lit - m_active);
		sendPixels(m_arry,m_cfg.m_pixels);
	}

void NeoPixels::Update()
	{

		switch (m_cfg.m_effect)
			{
		case effectCandle:
			CandleEffect();
			break;
		case effectTwinkle:
			TwinkleSingleEffect();
			break;
		case effectTheaterChase:
			TheaterChaseEffect();
			break;
		case effectRainbow:
			RainbowEffect();
			break;
		case effectAllOn:
		case effectPattern:
			if(m_tmrs[1])
				{
				sendPixels(m_arry,PIXELS);
				m_tmrs[1] = 0;
				}
			break;
		case effectOff:
		default:
			//if(m_tmrs[1])
				{
				memset(m_arry,0,sizeof(m_arry));
				sendPixels(m_arry,PIXELS);
				m_tmrs[1] = 0;
				}
			break;
			}
		m_timer = millis() + 10;
	}


void NeoPixels::initRainbow()
	{
		m_tmrs[0] = m_cfg.m_sustain;
		m_tmrs[1] = 0;
		memset(m_arry,0,sizeof(m_arry));
	}

void NeoPixels::RainbowEffect()
	{
		m_tmrs[0]--;
		if(m_tmrs[0] == 0)
			{
			for(size_t i=0;i<m_cfg.m_pixels;i++)
				{
					m_arry[i] = Wheel((byte)((i+m_tmrs[1])&0xff));
				}
			m_tmrs[1]++;
			m_tmrs[0] = m_cfg.m_sustain;
			sendPixels(m_arry,m_cfg.m_pixels);
			}
	}

void NeoPixels::Task()
	{
		if (millis() > m_timer)
			Update();
	}

void NeoPixels::SetConfig(NeoConfig &cfg)
	{
		m_cfg = cfg;
		init();
	}

NeoColorOrder NeoPixelColor::m_color_order = RGB;

byte NeoPixelColor::getRed()
	{
		switch(m_color_order)
			{
		case GRB:
			return colors[1];
			break;
		case RGB:
		default:
			return colors[0];
			}
	}

byte NeoPixelColor::getGreen()
	{
		switch(m_color_order)
			{
		case GRB:
			return colors[0];
			break;
		case RGB:
		default:
			return colors[1];
			}
	}


byte NeoPixelColor::getBlue()
	{
		return colors[2];
	}



void NeoPixelColor::setRed(byte v)
	{
		switch(m_color_order)
			{
		case GRB:
			colors[1] = v;
			break;
		case RGB:
		default:
			colors[0] = v;
			}
	}

void NeoPixelColor::setGreen(byte v)
	{
		switch(m_color_order)
			{
		case GRB:
			colors[0] = v;
			break;
		case RGB:
		default:
			colors[1] = v;
			}
	}


void NeoPixelColor::setBlue(byte v)
	{
		colors[2] = v;
	}


NeoPixelColor::NeoPixelColor()
	{
		setRed(0);
		setGreen(0);
		setBlue(0);
	}

NeoPixelColor::NeoPixelColor(byte r, byte g, byte b)
	{
		setRed(r);
		setGreen(g);
		setBlue(b);
	}

NeoPixelColor::NeoPixelColor(const NeoPixelColor &r)
	{
		setRed(r.getRed());
		setGreen(r.getGreen());
		setBlue(r.getBlue());
	}

NeoPixelColor & NeoPixelColor::operator=(const NeoPixelColor &r)
	{
		setRed(r.getRed());
		setGreen(r.getGreen());
		setBlue(r.getBlue());
		return *this;
	}

NeoPixelColor NeoPixelColor::operator *(byte b)
	{
		NeoPixelColor ret = *this;
		unsigned int x = ret.getRed();
		x *= b;
		x /= 255;
		if(x > 255)
			x = 255;
		ret.setRed(x);

		x = ret.getGreen();
		x *= b;
		x /= 255;
		if(x > 255)
			x = 255;
		ret.setGreen(x);

		x = ret.getBlue();
		x *= b;
		x /= 255;
		if(x > 255)
			x = 255;
		ret.setBlue(x);

		return ret;
	}

