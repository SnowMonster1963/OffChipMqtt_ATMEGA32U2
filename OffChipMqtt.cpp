/*
 * OffChipMqtt.cpp
 *
 *  Created on: May 21, 2016
 *      Author: tsnow
 */
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <stdio.h>
#include "Serial.h"
#include "bitbang.h"
#include "StackChecker.h"
#include "utilities.h"
#include "NeoPixels.h"
#include "USBStream.h"
#include "OffChipMqtt.h"

extern NeoPixels neo;

const char pstrSetPColor[] PROGMEM = "pcolor";    	//void SetPrimaryColor(NeoPixelColor c);
const char pstrSetSColor[] PROGMEM = "scolor";		//void SetSecondaryColor(NeoPixelColor c);
const char pstrSetDelay[] PROGMEM = "delay";		//void SetTransitionDelay(uint16_t td);
const char pstrSetEffect[] PROGMEM = "effect";		//void SetEffect(NeoPixelEffect effect);
const char pstrSetCMode[] PROGMEM = "cmode";		//void SetColorMode(NeoPixelColorMode color_mode);
const char pstrSetSMode[] PROGMEM = "smode";		//void SetStrandMode(NeoPixelStrandMode strand_mode);
const char pstrSetDensity[] PROGMEM = "density";	//void SetDensity(byte d);

const char * const TopicArray[] PROGMEM =
	{ pstrSetPColor, pstrSetSColor, pstrSetDelay, pstrSetEffect, pstrSetCMode, pstrSetSMode, pstrSetDensity, };

#ifdef DO_USB
#define debug_P(x) \
	{ \
		USBSerial.SendStr_P(Red,PSTR(x));\
		USBSerial.StreamTask(); \
	}

#define debug(x) \
	{ \
		USBSerial.SendStr(Red,x);\
		USBSerial.StreamTask(); \
	}

#define debug_char(x) \
	{ \
		USBSerial.Send(Red,x);\
		USBSerial.StreamTask(); \
	}

#else
#define debug_P(x)
#define debug(x)
#define debug_char(x)
#endif

MyMqtt::MyMqtt()
	{
		m_flags.reg = 0;
		cmdbuf = localbuf;
		cmdbuflen = sizeof(localbuf);
		cmd_idx = 0;

	}

void MyMqtt::SetBuffer(char *buf,size_t len)
	{
		if(buf == NULL)
			{
				cmdbuf = localbuf;
				cmdbuflen = sizeof(localbuf);
			}
		else
			{
				cmdbuf = buf;
				cmdbuflen = len;
			}
		cmd_idx = 0;
		memset(cmdbuf,0,cmdbuflen);
	}

void MyMqtt::begin()
	{
		sbi(E_RST_DDR, E_RST);
		cbi(E_RST_PORT, E_RST);
		sbi(E_CH_PD_DDR, E_CH_PD);
		cbi(E_CH_PD_PORT, E_CH_PD);
		sbi(E_RX_DDR, E_RX);
		sbi(E_RX_PORT, E_RX);
		sbi(E_TX_DDR, E_TX);
		sbi(E_TX_PORT, E_TX);
		delay(1000);
		cbi(E_RX_DDR, E_RX);
		cbi(E_TX_DDR, E_TX);
		Serial1.begin(115200);
		sbi(E_CH_PD_PORT, E_CH_PD);
		delay(1000);
		sbi(E_RST_PORT, E_RST);
		GlobalInterruptEnable();

		cmd_idx = 0;
		memset(cmdbuf, 0, cmdbuflen);
		unsigned long tmr = millis() + 3000;
		int c = Serial1.get();
		bool startFill = false;

		debug_P("begin\r\n");
		while (millis() < tmr)
			{
#ifdef DO_USB
				USBSerial.StreamTask();
#endif
				if (c == 'S')
					startFill = true;

				if (startFill && c > 0 && (isprint(c) || isspace(c)))
					{
						cmdbuf[cmd_idx++] = c;
						debug_char(c);
						if (strstr_P(cmdbuf, PSTR("System init done.\r\nReady\r\n")) != NULL)
							{
								memset(cmdbuf, 0, cmdbuflen);
								cmd_idx = 0;
								return;
							}
					}
				c = Serial1.get();
			}
		debug_P("Buffer:  '");
		debug(cmdbuf);
		debug_P("'\r\n");
	}

#ifdef DO_USB
unsigned long xyz = 0;
#endif


#define TASK_WAIT 5
void MyMqtt::MqttTasks()
	{
		char *p;
		int c = Serial1.get();
		bool b = true;
		unsigned long tmr = millis() + TASK_WAIT;

		if(c < 0)
			return;

		// read chars until none come for 1 ms or buffer fills
		while(millis() < tmr)
			{
				if(c > 0 && cmd_idx < (cmdbuflen - 1))
					{
						cmdbuf[cmd_idx++] = c;
						tmr = millis() + TASK_WAIT;
					}
				if(cmd_idx < (cmdbuflen-1))
					{
					}
				else
					{
						tmr = millis();
					}
				c = Serial1.get();
#ifdef DO_USB
						USBSerial.StreamTask();
#endif
			}

		if(strlen(cmdbuf) > 0)
			{
				b = true;
#ifdef DO_USB
						USBSerial.SendStr_P(Red,PSTR("cmdbuf:\r\n"));
						USBSerial.SendStr(Red,cmdbuf);
						USBSerial.SendStr_P(Red,PSTR("\r\n"));
#endif
			}
		else
			b = false;

		size_t idx = 0;
		while (b)
			{
				p = strstr_P(cmdbuf+idx,PSTR("\r\n"));
				if(p != NULL)
					{
						*p = 0;
						char *pcmdbuf = cmdbuf+idx;
						idx += strlen(pcmdbuf) + 2;
						if (strcmp_P(pcmdbuf, PSTR("+MQTT Connected")) == 0)
							{
								m_flags.flags.mqttonline = true;
#ifdef DO_USB
								USBSerial.SendStr_P(Green,PSTR("MQTT Connected\r\n"));
#endif
							}
						else if (strcmp_P(pcmdbuf, PSTR("+MQTT Disconnected")) == 0)
							{
								m_flags.flags.mqttonline = false;
#ifdef DO_USB
								USBSerial.SendStr_P(Green,PSTR("MQTT Disconnected\r\n"));
#endif
							}
						else if (strcmp_P(pcmdbuf, PSTR("+WIFI Connected")) == 0)
							{
								m_flags.flags.wifionline = true;
#ifdef DO_USB
								USBSerial.SendStr_P(Green,PSTR("WIFI Connected\r\n"));
#endif
							}
						else if (strcmp_P(pcmdbuf, PSTR("+WIFI Disconnected")) == 0)
							{
								m_flags.flags.wifionline = false;
#ifdef DO_USB
								USBSerial.SendStr_P(Green,PSTR("WIFI Disconnected\r\n"));
#endif
							}
						else if (strcmp_P(pcmdbuf, PSTR("OK")) == 0)
							{
								m_flags.flags.got_ok = true;
#ifdef DO_USB
								USBSerial.SendStr_P(Green,PSTR("Got OK\r\n"));
#endif
							}
						else if (strstr_P(pcmdbuf, PSTR("Error")) != NULL)
							{
								m_flags.flags.got_error = true;
#ifdef DO_USB
								USBSerial.SendStr_P(Red,PSTR("Got Error\r\n"));
#endif
							}
						else if ((p = strstr_P(pcmdbuf, PSTR("+AT+TOPIC=\""))) != NULL)
							{
								char *topic = p + 11;
								char *payload = pcmdbuf;
#ifdef DO_USB
								USBSerial.SendStr_P(Green,PSTR("Got Topic\r\n"));
								USBSerial.SendStr(Green,pcmdbuf);
#endif

								pcmdbuf[0] = 0;
								p = strstr_P(topic, PSTR("\""));
								if (p != NULL)
									{
										*p = 0;
										payload = p + 3;
										p = strstr_P(payload, PSTR("\""));
										if (p != NULL)
											{
												*p = 0;
											}
									}
								OnTopic(topic, payload);
							}
						else
							{
#ifdef DO_USB
								USBSerial.SendStr_P(Red,PSTR("Got Other!\r\n"));
								USBSerial.SendStr(Red,pcmdbuf);
#endif
							}
						if(idx >= cmd_idx)
							{
								b = false;
								cmd_idx = 0;
								memset(cmdbuf,0,cmdbuflen);
							}
					}
				else
					{
						b = false;
						if(idx < cmd_idx)
							{	// chars left over in buffer when full, shift them down
								memcpy(cmdbuf,cmdbuf+idx,cmd_idx-idx);
								cmd_idx -= idx;
								memset(cmdbuf+idx,0,cmdbuflen-idx);
							}
					}
			}
//#ifdef DO_USB
//		else
//			{
//				if(xyz < millis())
//					{
//
//					}
//			}
//#endif
	}

void MyMqtt::Publish_P(const char *topic, char *data)
	{
		// AT+TOPIC="topic","value"
		Serial1.sendStr_P(PSTR("AT+TOPIC=\""));
		Serial1.sendStr(topic);
		Serial1.sendStr_P(PSTR("\",\""));
		Serial1.sendStr(data);
		Serial1.sendStr_P(PSTR("\"\r\n"));
	}

//void MyMqtt::OnTask()
//	{
//		char buf[16];
//		if (m_flags.reg && m_flags.flags.run_pixels)
//			{
//				if (m_flags.flags.send_effect)
//					{
//						sprintf_P(buf, PSTR("%u"), (unsigned) m_effect);
//						Publish_P(PSTR("effect"), buf);
//						m_flags.flags.send_effect = 0;
//						neo.SetEffect(m_effect);
//					}
//				else if (m_flags.flags.send_pcolor)
//					{
//						sprintf_P(buf, PSTR("%u,%u,%u"), (unsigned) m_pcolor.Red, (unsigned) m_pcolor.Green, (unsigned) m_pcolor.Blue);
//						Publish_P(PSTR("pcolor"), buf);
//						m_flags.flags.send_pcolor = 0;
//						neo.SetPrimaryColor(m_pcolor);
//					}
//				else if (m_flags.flags.send_scolor)
//					{
//						sprintf_P(buf, PSTR("%u,%u,%u"), (unsigned) m_scolor.Red, (unsigned) m_scolor.Green, (unsigned) m_scolor.Blue);
//						Publish_P(PSTR("scolor"), buf);
//						m_flags.flags.send_scolor = 0;
//						neo.SetSecondaryColor(m_scolor);
//					}
//				else if (m_flags.flags.send_cmode)
//					{
//						sprintf_P(buf, PSTR("%u"), (unsigned) m_cmode);
//						Publish_P(PSTR("cmode"), buf);
//						m_flags.flags.send_cmode = 0;
//						neo.SetColorMode(m_cmode);
//					}
//				else if (m_flags.flags.send_smode)
//					{
//						sprintf_P(buf, PSTR("%u"), (unsigned) m_smode);
//						Publish_P(PSTR("smode"), buf);
//						m_flags.flags.send_smode = 0;
//						neo.SetStrandMode(m_smode);
//					}
//				else if (m_flags.flags.send_density)
//					{
//						sprintf_P(buf, PSTR("%u"), (unsigned) m_density);
//						Publish_P(PSTR("density"), buf);
//						m_flags.flags.send_density = 0;
//						neo.SetDensity(m_density);
//					}
//				else if (m_flags.flags.send_delay)
//					{
//						sprintf_P(buf, PSTR("%u"), (unsigned) m_delay);
//						Publish_P(PSTR("delay"), buf);
//						m_flags.flags.send_delay = 0;
//						neo.SetTransitionDelay(m_delay);
//					}
//			}
//	}

void MyMqtt::OnTopic(char *topic, char *payload)
	{

#ifdef DO_USB
		USBSerial.SendStr_P(DarkYellow,PSTR("Topic:  '"));
		USBSerial.SendStr(DarkYellow,topic);
		USBSerial.SendStr_P(DarkYellow,PSTR("'\n"));
		USBSerial.SendStr_P(DarkYellow,PSTR("Message:  '"));
		USBSerial.SendStr(DarkYellow,payload);
		USBSerial.SendStr_P(DarkYellow,PSTR("'\n"));
#endif
		byte r, g, b;
		uint16_t v;
		char *p = payload;
		m_flags.flags.update = 1;
		if (strstr_P((char *) topic, PSTR("pcolor")) != NULL)
			{
				r = atoi(p) & 0xff;
				p = strchr(p, ',') + 1;
				g = atoi(p) & 0xff;
				p = strchr(p, ',') + 1;
				b = atoi(p) & 0xff;
				m_cfg.m_pattern[0] = NeoPixelColor(r, g, b);
			}
		else if (strstr_P((char *) topic, PSTR("scolor")) != NULL)
			{
				r = atoi(p) & 0xff;
				p = strchr(p, ',') + 1;
				g = atoi(p) & 0xff;
				p = strchr(p, ',') + 1;
				b = atoi(p) & 0xff;
				m_cfg.m_pattern[1] = NeoPixelColor(r, g, b);
			}
		else if (strstr_P((char *) topic, PSTR("tcolor")) != NULL)
			{
				r = atoi(p) & 0xff;
				p = strchr(p, ',') + 1;
				g = atoi(p) & 0xff;
				p = strchr(p, ',') + 1;
				b = atoi(p) & 0xff;
				m_cfg.m_pattern[2] = NeoPixelColor(r, g, b);
			}
		else if (strstr_P((char *) topic, PSTR("order")) != NULL)
			{
				v = atoi(p);
				NeoPixelColor::m_color_order = (NeoColorOrder)v;
			}
		else if (strstr_P((char *) topic, PSTR("cmode")) != NULL)
			{
				v = atoi(p);
				m_cfg.m_colormode = (NeoPixelColorMode) (v);
			}
		else if (strstr_P((char *) topic, PSTR("effect")) != NULL)
			{
				v = atoi(p);
				m_cfg.m_effect = (NeoPixelEffect) v;
			}
		else if (strstr_P((char *) topic, PSTR("smode")) != NULL)
			{
				v = atoi(p);
				m_cfg.m_strandmode = (NeoPixelStrandMode) (v);
			}
		else if (strstr_P((char *) topic, PSTR("density")) != NULL)
			{
				v = atoi(p);
				m_cfg.m_density = v;
			}
		else if (strstr_P((char *) topic, PSTR("attack")) != NULL)
			{
				v = atoi(p);
				m_cfg.m_attack = v;
			}
		else if (strstr_P((char *) topic, PSTR("sustain")) != NULL)
			{
				v = atoi(p);
				m_cfg.m_sustain = v;
			}
		else if (strstr_P((char *) topic, PSTR("decay")) != NULL)
			{
				v = atoi(p);
				m_cfg.m_decay = v;
			}
		else if (strstr_P((char *) topic, PSTR("off")) != NULL)
			{
				v = atoi(p);
				m_cfg.m_off = v;
			}
		else if (strstr_P((char *) topic, PSTR("pixels")) != NULL)
			{
				v = atoi(p);
				m_cfg.m_pixels = v;
			}
		else if (strstr_P((char *) topic, PSTR("pattern")) != NULL)
			{
				byte idx = 0;
				while(idx < sizeof(m_cfg.m_pattern)/sizeof(m_cfg.m_pattern[0]) && p != NULL)
					{
					r = atoi(p) & 0xff;
					p = strchr(p, ',');
					if(p != NULL)
						{
							p++;
							g = atoi(p) & 0xff;
							p = strchr(p, ',');
						}
					if(p != NULL)
						{
							p++;
							b = atoi(p) & 0xff;
							p = strchr(p, ',');
						}
					if(p != NULL)
						{
							p++;
						}
					m_cfg.m_pattern[idx] = NeoPixelColor(r, g, b);
					idx++;
					}
				m_cfg.m_pattern_size = idx;
			}
		else
			{
			}

	}

void MyMqtt::CheckUpdate()
	{
		if(m_flags.flags.update != 0)
			{
				neo.SetConfig(m_cfg);
				m_flags.flags.update = 0;
			}
	}
