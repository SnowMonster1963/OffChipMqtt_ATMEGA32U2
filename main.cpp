#include <string.h>
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

#define SIGNATURE 0x55AA
unsigned StackSignature __attribute__ ((section (".noinit")));

void checkSignature(bool showstack = false)
	{
#ifdef DO_USB
		if (showstack)
			{
				char buf[32];
				sprintf_P(buf,PSTR("Stack=%04x\n"), (int) (TrackStack()));
				USBSerial.SendStr(buf);
			}
#endif
		if (StackSignature != SIGNATURE)
			{
#ifdef DO_USB
				USBSerial.SendStr_P(PSTR("Stack Crash!\r\n"));
				char buf[32];
				sprintf_P(buf,PSTR("Stack=%04x\n"), (int) (TrackStack()));
				USBSerial.SendStr(buf);
#endif
				while (1)
					{
#ifdef DO_USB
						USBSerial.StreamTask();
#endif
						sbi(PORTB, 7);
						delay(100);
						cbi(PORTB, 7);
						delay(100);
					}
			}
	}

void fillPixels(byte pixels, byte r, byte g, byte b)
	{
		InterruptBlocker ib;
		for (byte i = 0; i < pixels; i++)
			{
				sendPixel(r, g, b);
				if (tbi(UCSR1A,RXC1))
					return;
			}
	}

void flashPixels(byte pixels, byte r, byte g, byte b, byte times, uint16_t delayrate)
	{
		for (byte t = 0; t < times; t++)
			{
				fillPixels(pixels, r, g, b);
				delay(delayrate);
				fillPixels(pixels, 0, 0, 0);
				delay(delayrate);
			}
	}

volatile bool start = false;

NeoPixels neo(PIXELS);
MyMqtt mqttsock;

#define INTERVAL_PIXEL 10

void PixelTest(byte pixels)
	{
		checkSignature(true);
		flashPixels(PIXELS, 0, 0, 32, 3, 500);

		//DDRB |= 1;
		mqttsock.SetBuffer((char *)neo.getPixels(),PIXELS * 3);
		mqttsock.begin();
		//flashPixels(PIXELS, 0, 32, 0, 3, 500);
		unsigned long tmr;
		tmr = millis() + 5000;
		while (millis() < tmr && mqttsock.IsConnected() == false)
			{
				mqttsock.MqttTasks();
#ifdef DO_USB
				USBSerial.StreamTask();
#endif
			}
		tmr = millis() + 5000;

		// read any messages that come in fast
		while (millis() < tmr && mqttsock.IsConnected() == true)
			{
				mqttsock.MqttTasks();
#ifdef DO_USB
				USBSerial.StreamTask();
#endif
			}

		mqttsock.SetBuffer(NULL,0);

		neo.begin();
		while(1)
			{
				mqttsock.MqttTasks();
				mqttsock.CheckUpdate();
				neo.Task();
#ifdef DO_USB
				USBSerial.StreamTask();
#endif
				if (tmr < millis())
					{
						checkSignature(true);
						tmr = millis() + 500;
					}

			}
	}

#ifdef DO_USB
void TalkToChip()
	{
		bool b = true;
		mqttsock.begin();

		while(b)
			{
				USBSerial.StreamTask();
				int16_t ReceivedByte = USBSerial.fgetc();
				if (ReceivedByte > 0)
					{
						if(ReceivedByte == 'q')
							return;

						Serial1.PutChar(ReceivedByte);
					}

				ReceivedByte = Serial1.get();
				if(ReceivedByte > 0)
					{
						USBSerial.Send(ReceivedByte);
					}
			}

	}
#endif

int main(void)
	{

		initTimer();
		sbi(DDRB, 7);
		sbi(PORTB, 7);
		sbi(DDRB, 0);
		sbi(PORTB, 0);
		sbi(E_RST_DDR, E_RST);
		cbi(E_RST_PORT, E_RST);
		sbi(E_CH_PD_DDR, E_CH_PD);
		cbi(E_CH_PD_PORT, E_CH_PD);
		delay(1000);
		flashPixels(PIXELS, 32, 0, 0, 3, 500);
		StackSignature = SIGNATURE;
#ifdef DO_USB
		USBSerial.begin();
		USBSerial.StreamTask();
		while (!USBSerial.HasStarted())
			{
				USBSerial.StreamTask();
				_delay_ms(100);
			}
#endif
		GlobalInterruptEnable();
		sbi(DDRB, 0);
		fillPixels(150, 0, 0, 0);

#ifdef DO_USB
		Serial1.Show();
#endif

		while (1)
			{
#ifdef DO_USB
				USBSerial.SendStr_P(Green, PSTR("\r\nMain Menu\r\n"
						"t - TTY to Chip Mode\r\n"
						"p - Pixel Test Mode\r\n"
						"b - Startup Chip\r\n"
						"\r\n"));
				int16_t ReceivedByte = -1;
				if (StackSignature != SIGNATURE)
					{
						//USBSerial.SendStr_P(Red, PSTR("Stack Crash!\r\n"));
						while (1)
							;
					}

				while (ReceivedByte < 0)
					{
						USBSerial.StreamTask();
						ReceivedByte = USBSerial.fgetc();
					}

				if (ReceivedByte > 0)
					{
						switch (ReceivedByte)
							{
						case 't':
							USBSerial.SendStr_P(Yellow, PSTR("TTY to Chip Mode - Press 'q' to exit\r\n"));
							TalkToChip();
							break;
						case 'p':
							USBSerial.SendStr_P(Yellow, PSTR("Pixel Test - Press 'q' to exit\r\n"));
							PixelTest (PIXELS);
							break;
						case 'c':
							fillPixels(PIXELS, 0, 0, 0);
							break;
						case 'b':
							USBSerial.SendStr_P(Yellow, PSTR("Starting Chip\r\n"));
							mqttsock.begin();
							break;
						default:
							USBSerial.SendStr_P(Red, PSTR("Undefined - "));
							USBSerial.Send(Red, ReceivedByte);
							USBSerial.SendStr_P(Red, PSTR("\r\n"));
							}
					}
#else
				PixelTest(PIXELS);
#endif

			}
		return 0;
	}

