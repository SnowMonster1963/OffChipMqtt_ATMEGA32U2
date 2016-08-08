/*
 * NeoPixels.h
 *
 *  Created on: Dec 3, 2015
 *      Author: tsnow
 */

#ifndef NEOPIXELS_H_
#define NEOPIXELS_H_

// Change this to be at least as long as your pixel string (too long will work fine, just be a little slower)
#ifdef DO_USB
#define PIXELS 50  // Number of pixels in the string
#else
#define PIXELS 50  // Number of pixels in the string
#endif
// These values depend on which pin your string is connected to and what board you are using
// More info on how to find these at http://www.arduino.cc/en/Reference/PortManipulation

// These values are for digital pin 8 on an Arduino Yun or digital pin 12 on a DueMilinove/UNO
// Note that you could also include the DigitalWriteFast header file to not need to to this lookup.

#define PIXEL_PORT  PORTB  // Port of the pin the pixels are connected to
#define PIXEL_DDR   DDRB   // Port of the pin the pixels are connected to
#define PIXEL_BIT   0      // Bit of the pin the pixels are connected to

void theaterChase(unsigned char r, unsigned char g, unsigned char b, unsigned char wait);
void colorWipe(unsigned char r, unsigned char g, unsigned char b, unsigned char wait);
void showColor(unsigned char r, unsigned char g, unsigned char b);
void sendPixel(unsigned char r, unsigned char g, unsigned char b);

enum NeoPixelEffect
	{
	effectOff, effectAllOn, effectTwinkle, effectTheaterChase, effectRainbow, effectCandle,
	effectMaxEffect
	};

enum NeoPixelColorMode
	{
	colorModeSingle, colorModePattern, colorModeRandom,
	};

enum NeoPixelStrandMode
	{
	Strand, XmasTree,
	};

enum NeoPixelState
	{
	TurnedOff, Attacking, Sustaining, Decaying
	};

enum NeoColorOrder
	{
	RGB, GRB
	};

class NeoPixelColor
	{
	byte colors[3];

public:
	static NeoColorOrder m_color_order;

	byte getRed();
	byte getGreen();
	byte getBlue();

	void setRed(byte v);
	void setGreen(byte v);
	void setBlue(byte v);

	NeoPixelColor();
	NeoPixelColor(byte r, byte g, byte b);
	NeoPixelColor(const NeoPixelColor &r);
	NeoPixelColor & operator=(const NeoPixelColor &r);

	// Set intensity based on scale of 0-255
	NeoPixelColor operator *(byte b);

	};

#define PALETTE_SIZE 16

class NeoConfig
	{
public:
	size_t m_pixels;
	uint16_t m_attack;	// time period from when pixel is dark to full brightness (incrementally brightens)
	uint16_t m_sustain;	// time period that pixel stays fully lit - also used for transition delays
	uint16_t m_decay;	// time period from when pixel is fully lit to darkness (incrementally dims)
	uint16_t m_off;		// time period that pixel stays off
//	NeoPixelColor m_primary;
//	NeoPixelColor m_secondary;
//	NeoPixelColor m_tertiary;
	NeoPixelEffect m_effect;
	NeoPixelColorMode m_colormode;
	NeoPixelStrandMode m_strandmode;
	byte m_density;
	byte m_pattern_size;
	NeoPixelColor m_pattern[PALETTE_SIZE];

	NeoConfig();

	};

class NeoPixels
	{
private:
	NeoConfig	m_cfg;
	unsigned long m_timer;
	size_t j, q, step, spacing;
	NeoPixelColor m_arry[PIXELS];
	byte m_status_bits[(PIXELS / 4) + 1];
	byte m_tmrs[PIXELS];
	byte m_palette[PIXELS * 8 / PALETTE_SIZE];
	size_t m_lit;
	size_t m_active;

	void Update();

	void init();

	void initAllOn();
	void AllOnEffect();

	void initTwinkle();
	void TwinkleEffect();

	void initTheaterChase();
	void TheaterChaseEffect();

	void initRainbow();
	void RainbowEffect();

	void initCandle();
	void CandleEffect();

	void SetPixelState(size_t idx,NeoPixelState state);
	NeoPixelState GetPixelState(size_t idx);

	void SetPalette(size_t idx,byte v);
	byte GetPalette(size_t idx);

	void StartRandomPixel(size_t pixels=1);

public:
	NeoPixels(size_t pixels = PIXELS);
	NeoPixelColor *getPixels(){return &m_arry[0];};
	void begin();

	void SetConfig(NeoConfig &cfg);
	void GetConfig(NeoConfig *cfg);

	// external operations
	void fillPixels(NeoPixelColor c);

	// handle the show mode
	void Task();
	};


void sendPixels(NeoPixelColor *px,size_t len);

#endif /* NEOPIXELS_H_ */
