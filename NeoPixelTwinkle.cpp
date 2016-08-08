/*
 * NeoPixelTwinkle.cpp
 *
 *  Created on: Jul 13, 2016
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

#define RANDOMNESS 50
byte RandomOffset(uint16_t v)
	{
		if(v > (255 - RANDOMNESS))
			v = 255 - RANDOMNESS;

		uint16_t x = random() % (2 * RANDOMNESS);
		if(x == 0)
			x++;

		if(x > RANDOMNESS)
			v -= x - RANDOMNESS;
		else
			v += x;
		return (byte)v;
	}



void NeoPixels::SetPalette(size_t idx,byte v)
	{
		// for now, let's not get crazy - 4 bits per pixel
		byte curp = m_palette[idx / 2];	// 2 entries per byte
		byte mask = idx % 2 > 0 ? 0xf0 : 0x0f;
		byte val = idx % 2 > 0 ? (v << 4) & mask : v & mask;

		curp &= ~mask;
		curp |= val;
		m_palette[idx/2] = curp;
	}

byte NeoPixels::GetPalette(size_t idx)
	{
		// for now, let's not get crazy - 4 bits per pixel
		byte curp = m_palette[idx / 2];	// 2 entries per byte
		byte val = idx % 2 > 0 ? (curp >> 4) & 0x0f : curp & 0x0f;

		return val;
	}

void NeoPixels::initTwinkle()
	{
		memset(m_arry,0,sizeof(m_arry));
		memset(m_status_bits,0,sizeof(m_status_bits));
		memset(m_palette,0,sizeof(m_palette));

		for(size_t i=0;i<m_cfg.m_pixels;i++)
			m_tmrs[i] = RandomOffset(m_cfg.m_off);
		for(size_t i=0;i<m_cfg.m_pixels;i++)
			{
				byte idx = m_cfg.m_pattern_size > 1 ? i % m_cfg.m_pattern_size : 0;
				SetPalette(i,idx);
			}

		m_lit = m_cfg.m_pixels * m_cfg.m_density / 100;
		if(m_lit == 0)
			m_lit++;
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
		StartRandomPixel(m_lit);
		sendPixels(m_arry,PIXELS);
	}

void NeoPixels::TwinkleEffect()
	{
		size_t idx;
		m_active = 0;
		for(idx = 0;idx < m_cfg.m_pixels;idx++)
			{
				size_t pidx = GetPalette(idx);

				NeoPixelState ps = GetPixelState(idx);
				if(ps != TurnedOff)
					m_active++;

				if(m_tmrs[idx] == 0)
					{
						switch(ps)
							{
						case TurnedOff:
							break;
						case Attacking:
							m_arry[idx] = m_cfg.m_pattern[pidx];
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
							if(m_active > 0)
								m_active--;
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
								m_arry[idx] = m_cfg.m_pattern[pidx] * (byte)(x);
								}
							break;
						case Sustaining:
							m_arry[idx] = m_cfg.m_pattern[pidx];
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
								m_arry[idx] = m_cfg.m_pattern[pidx] * (byte)(x);
								}
							break;
							}
					}
			}
		if(m_active < m_lit)
			StartRandomPixel(m_lit - m_active);
		sendPixels(m_arry,m_cfg.m_pixels);
	}




