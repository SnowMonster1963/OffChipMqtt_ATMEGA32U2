/*
 * NeoPixelTheaterChase.cpp
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

void NeoPixels::initTheaterChase()
	{
		spacing = 100 / m_cfg.m_density;
		if (spacing < 2)
			spacing = 2;

		q = 0;
		step = 0;
		m_tmrs[0] = m_cfg.m_sustain;

	}

void NeoPixels::TheaterChaseEffect()
	{
		if(m_tmrs[0] > 0)
			m_tmrs[0] --;
		else
			{
				switch(m_cfg.m_colormode)
					{
				case colorModePattern:
					for(size_t i=0;i<m_cfg.m_pixels;i++)
						{
							m_arry[i] = m_cfg.m_pattern[(i+q) % m_cfg.m_pattern_size];
						}
					q++;
					break;
				default:

					/*
					 * Timing for values of attack, sustain, and decay are 10 ms per 'tick'
					 * Step is the currently, fully lit pixel in a spacing
					 * Sustain is the period between pixel movements (10 ms per 'tick') or how long each lit pixel is lit before moving
					 * Attack sets the pixels' brightness ahead - off to on
					 * Decay sets the pixels' brightness behind - on to off
					 * A value of zero for attack and decay indicates that only 1 pixel per spacing is lit
					 * By setting values for attack and decay, a 'wave' of light is created, 'step' pixel is the peak of the wave
					 * Example:  ----^^^^^^Xvvvvvv-----
					 * '-' = unlit
					 * '^' = increase in brightness
					 * 'X' = fully lit (i.e. step pixel)
					 * 'v' = decrease in brightness
					 *
					 * The number of pixels before and after are calculated the same way, just in opposite order of brightness
					 * Density controls spacing, therefore each 'Period' defines the size of the 'wave' in pixels over time (ticks)
					 * 		P = SUS * SP
					 *
					 * Each Pixel's offset from the top of the wave is defined as a time division ticks/pixel
					 * 		PT = P / SP
					 *
					 * Each Pixel's intensity is therefore a function of the offset time and the period of attack/decay
					 * 		I = PT * offset * m_pattern[0] / AD
					 *
					 */
					for (size_t i = 0; i < m_cfg.m_pixels; i++)
						{
							if (step == q)
								m_arry[i] = (m_cfg.m_pattern[0]);//NeoPixelColor(255,0,0);
							else
								{
									if(step < q)
										{
											byte b = q - step;
											if(b < m_cfg.m_decay)
												{
													b = (255/(b));
													m_arry[i] = m_cfg.m_pattern[0] * b;
												}
											else
												{
													m_arry[i] = NeoPixelColor(0, 0, 0);
												}
										}
									else
										{
											byte b = step - q;
											if(b < m_cfg.m_attack)
												{
													b = (255/(b));
													m_arry[i] = m_cfg.m_pattern[0] * b;
												}
											else
												{
													m_arry[i] = NeoPixelColor(0, 0, 0);
												}
										}

								}

							step++;
							if (step >= spacing)
								step = 0;
						}

					q++;
					if (q >= spacing)
						q = 0;
					}

				sendPixels(m_arry,m_cfg.m_pixels);
				m_tmrs[0] = m_cfg.m_sustain;
			}

	}



