/*
 * NeoPixelAllOn.cpp
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

void NeoPixels::initAllOn()
	{
		memset(m_arry,0,sizeof(m_arry));
		for(uint16_t i=0;i<m_cfg.m_pixels;i++)
			{
				uint16_t x = m_cfg.m_pattern_size > 1 ? i % m_cfg.m_pattern_size : 0;
				m_arry[i] = m_cfg.m_pattern[x];
			}
		m_tmrs[1] = 1;

	}

void NeoPixels::AllOnEffect()
	{
		if(m_tmrs[1])
			{
			sendPixels(m_arry,PIXELS);
			m_tmrs[1] = 0;
			}
	}

