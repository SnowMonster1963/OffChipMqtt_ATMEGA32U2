/*
 * NeoPixelCandle.cpp
 *
 *  Created on: Jul 6, 2016
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

#define CANDLE_FLICKER_RATE m_cfg.m_sustain

void NeoPixels::initCandle()
	{
		memset(m_arry, 0, sizeof(m_arry));
		m_tmrs[0] = CANDLE_FLICKER_RATE;
		sendPixels(m_arry,m_cfg.m_pixels);
	}

void NeoPixels::CandleEffect()
	{
		m_tmrs[0]--;
		if(m_tmrs[0] == 0)
			{
			for(size_t i=0;i<m_cfg.m_pixels;i++)
				{
					unsigned int scale = random() % 24;
					if(scale > 12)
						scale = 255;
					else
						{
							scale = scale * 256 / 12;
							if(scale > 255)
								scale = 255;
						}
					m_arry[i] = m_cfg.m_pattern[0] * (byte)(scale);
				}
			sendPixels(m_arry,m_cfg.m_pixels);
			m_tmrs[0] = CANDLE_FLICKER_RATE;
			}
	}







