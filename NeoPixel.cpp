/*
 * NeoPixel.cpp
 *
 *  Created on: Jul 14, 2016
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

NeoColorOrder NeoPixelColor::m_color_order = GRB;

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





