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
void sendPixelRGB(NeoPixelColor &c)
	{
		sendByte(c.getGreen()); // Neopixel wants colors in green then red then blue order
		if (tbi(UCSR1A,RXC1))
			return;
		sendByte(c.getRed());
		if (tbi(UCSR1A,RXC1))
			return;
		sendByte(c.getBlue());

	}

void sendPixelGBR(NeoPixelColor &c)
	{
		sendByte(c.getRed());
		if (tbi(UCSR1A,RXC1))
			return;
		sendByte(c.getGreen()); // Neopixel wants colors in green then red then blue order
		if (tbi(UCSR1A,RXC1))
			return;
		sendByte(c.getBlue());

	}

void sendPixels(NeoPixelColor *px,size_t len)
	{
		if(NeoPixelColor::m_color_order == RGB)
			{
			InterruptBlocker ib;
			for(size_t i = 0;i<len;i++)
				{
					sendPixelRGB(px[i]);
					if (tbi(UCSR1A,RXC1))
						return;
				}
			}
		if(NeoPixelColor::m_color_order == GRB)
			{
			InterruptBlocker ib;
			for(size_t i = 0;i<len;i++)
				{
					sendPixelGBR(px[i]);
					if (tbi(UCSR1A,RXC1))
						return;
				}
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
			initAllOn();
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


void NeoPixels::Update()
	{

		switch (m_cfg.m_effect)
			{
		case effectCandle:
			CandleEffect();
			break;
		case effectTwinkle:
			TwinkleEffect();
			break;
		case effectTheaterChase:
			TheaterChaseEffect();
			break;
		case effectRainbow:
			RainbowEffect();
			break;
		case effectAllOn:
			AllOnEffect();
			break;
		case effectOff:
		default:
			if(m_tmrs[1])
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

