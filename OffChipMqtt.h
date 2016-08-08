/*
 * OffChipMqtt.h
 *
 *  Created on: May 21, 2016
 *      Author: tsnow
 */

#ifndef OFFCHIPMQTT_H_
#define OFFCHIPMQTT_H_

#include "NeoPixels.h"

#ifndef E_RST
#define E_RST 0
#endif

#ifndef E_RST_DDR
#define E_RST_DDR DDRD
#endif

#ifndef E_RST_PORT
#define E_RST_PORT PORTD
#endif

#ifndef E_CH_PD
#define E_CH_PD 1
#endif

#ifndef E_CH_PD_DDR
#define E_CH_PD_DDR DDRD
#endif

#ifndef E_CH_PD_PORT
#define E_CH_PD_PORT PORTD
#endif

#ifndef E_RX
#define E_RX 2
#endif

#ifndef E_RX_DDR
#define E_RX_DDR DDRD
#endif

#ifndef E_RX_PORT
#define E_RX_PORT PORTD
#endif

#ifndef E_TX
#define E_TX 3
#endif

#ifndef E_TX_DDR
#define E_TX_DDR DDRD
#endif

#ifndef E_TX_PORT
#define E_TX_PORT PORTD
#endif

//#define DO_USB

typedef byte register_type;

class MyMqtt
	{
private:
	union flagreg
		{
		struct flagbits
			{
//			register_type send_effect :1;
//			register_type send_pcolor :1;
//			register_type send_scolor :1;
//			register_type send_cmode :1;
//			register_type send_smode :1;
//			register_type send_density :1;
//			register_type send_delay :1;
			register_type run_pixels : 1;
			register_type mqttonline : 1;
			register_type wifionline : 1;
			register_type got_ok : 1;
			register_type got_error : 1;
			register_type update : 1;
			} flags;
		register_type reg;
		};

	NeoConfig m_cfg;
	flagreg m_flags;
	char *cmdbuf;
	size_t cmdbuflen;
	char localbuf[128];
	size_t cmd_idx;

	// data to buffer

public:
	MyMqtt();
	void RunPixels(){m_flags.flags.run_pixels = true;};
	void StopPixels(){m_flags.flags.run_pixels = false;};
	void begin();

	void MqttTasks();
	bool IsConnected(){return (m_flags.flags.mqttonline != 0);};
	void SetBuffer(char *buf,size_t len);
	void CheckUpdate();

protected:
	void OnTopic(char *topic,char *payload);
	//void OnTask();
	void Publish_P(const char *topic,char *data);

	};


#endif /* OFFCHIPMQTT_H_ */
