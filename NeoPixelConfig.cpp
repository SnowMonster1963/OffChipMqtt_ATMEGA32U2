/*
 * NeoPixelConfig.cpp
 *
 *  Created on: Jul 5, 2016
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

////////////////// NeoConfig Class //////////////////////
NeoConfig::NeoConfig()
	{
		m_pixels = PIXELS;
		m_effect = effectTwinkle;
		m_colormode = colorModeSingle;
		m_strandmode = Strand;
		m_off = 100;
		m_attack = 10;
		m_sustain = 0;
		m_decay = 50;
		m_density = 33;
		m_pattern_size = 3;
		m_pattern[0] = NeoPixelColor(255,0,0);
		m_pattern[1] = NeoPixelColor(0,255,0);
		m_pattern[2] = NeoPixelColor(0,0,255);
	}




