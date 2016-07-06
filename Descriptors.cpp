/*
             LUFA Library
     Copyright (C) Dean Camera, 2015.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2015  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  USB Device Descriptors, for library use when in USB device mode. Descriptors are special
 *  computer-readable structures which the host requests upon device enumeration, to determine
 *  the device's capabilities and functions.
 */
#include <avr/io.h>
#include <avr/sfr_defs.h>

#include <avr/pgmspace.h>

#include "Descriptors.h"
#if defined(__cplusplus)
	extern "C" {
#endif


/** Device descriptor structure. This descriptor, located in FLASH memory, describes the overall
 *  device characteristics, including the supported USB version, control endpoint size and the
 *  number of device configurations. The descriptor is read out by the USB host when the enumeration
 *  process begins.
 */
const USB_Descriptor_Device_t PROGMEM DeviceDescriptor =
{
	 { sizeof(USB_Descriptor_Device_t),  DTYPE_Device},

	 VERSION_BCD(1,1,0),
	 CDC_CSCP_CDCClass,
	 CDC_CSCP_NoSpecificSubclass,
	 CDC_CSCP_NoSpecificProtocol,

	 FIXED_CONTROL_ENDPOINT_SIZE,

	 0x03EB, //0x2341,//USB_VID,//
	 0x204B, //0x0042,//USB_PID,//
	 VERSION_BCD(0,0,1),

	 STRING_ID_Manufacturer,
	 STRING_ID_Product,
	 USE_INTERNAL_SERIAL,

	 FIXED_NUM_CONFIGURATIONS
};

/** Configuration descriptor structure. This descriptor, located in FLASH memory, describes the usage
 *  of the device in one of its supported configurations, including information about any device interfaces
 *  and endpoints. The descriptor is read out by the USB host during the enumeration process when selecting
 *  a configuration so that the host may correctly communicate with the USB device.
 */
const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor =
{

		{
			 { sizeof(USB_Descriptor_Configuration_Header_t),  DTYPE_Configuration},

			 sizeof(USB_Descriptor_Configuration_t),
			 2,

			 1,
			 NO_DESCRIPTOR,

			 (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),

			 USB_CONFIG_POWER_MA(100)
		},


		{
			 { sizeof(USB_Descriptor_Interface_t),  DTYPE_Interface},

			 INTERFACE_ID_CDC_CCI,
			 0,

			 1,

			 CDC_CSCP_CDCClass,
			 CDC_CSCP_ACMSubclass,
			 CDC_CSCP_ATCommandProtocol,

			 NO_DESCRIPTOR
		},


		{
			 { sizeof(USB_CDC_Descriptor_FunctionalHeader_t),  DTYPE_CSInterface},
			 CDC_DSUBTYPE_CSInterface_Header,

			 VERSION_BCD(1,1,0),
		},


		{
			 { sizeof(USB_CDC_Descriptor_FunctionalACM_t),  DTYPE_CSInterface},
			 CDC_DSUBTYPE_CSInterface_ACM,

			 0x06,
		},


		{
			 { sizeof(USB_CDC_Descriptor_FunctionalUnion_t),  DTYPE_CSInterface},
			 CDC_DSUBTYPE_CSInterface_Union,

			 INTERFACE_ID_CDC_CCI,
			 INTERFACE_ID_CDC_DCI,
		},


		{
			 { sizeof(USB_Descriptor_Endpoint_t),  DTYPE_Endpoint},

			 CDC_NOTIFICATION_EPADDR,
			 (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			 CDC_NOTIFICATION_EPSIZE,
			 0xFF
		},


		{
			 { sizeof(USB_Descriptor_Interface_t),  DTYPE_Interface},

			 INTERFACE_ID_CDC_DCI,
			 0,

			 2,

			 CDC_CSCP_CDCDataClass,
			 CDC_CSCP_NoDataSubclass,
			 CDC_CSCP_NoDataProtocol,

			 NO_DESCRIPTOR
		},


		{
			 { sizeof(USB_Descriptor_Endpoint_t),  DTYPE_Endpoint},

			 CDC_RX_EPADDR,
			 (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			 CDC_TXRX_EPSIZE,
			 0x05
		},


		{
			 { sizeof(USB_Descriptor_Endpoint_t),  DTYPE_Endpoint},

			 CDC_TX_EPADDR,
			 (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			 CDC_TXRX_EPSIZE,
			 0x05
		}
};

const USB_Descriptor_String_t PROGMEM LanguageString =
{
	 { USB_STRING_LEN(1),  DTYPE_String},

	 L"\0x0409"
};

/** Manufacturer descriptor string. This is a Unicode string containing the manufacturer's details in human readable
 *  form, and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t PROGMEM ManufacturerString =
{
	 { USB_STRING_LEN(11),  DTYPE_String},

	 L"Dean Camera"
};

/** Product descriptor string. This is a Unicode string containing the product's details in human readable form,
 *  and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
const USB_Descriptor_String_t PROGMEM ProductString =
{
	 { USB_STRING_LEN(22),  DTYPE_String},

	 L"LUFA USB-RS232 Adapter"
};

///** Language descriptor structure. This descriptor, located in FLASH memory, is returned when the host requests
// *  the string descriptor with index 0 (the first index). It is actually an array of 16-bit integers, which indicate
// *  via the language ID table available at USB.org what languages the device supports for its string descriptors.0x0409
// */
////const USB_Descriptor_String_t PROGMEM LanguageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);
//const USB_Descriptor_String_t PROGMEM LanguageString = USB_STRING_DESCRIPTOR_ARRAY(L"\0x0409");
//
///** Manufacturer descriptor string. This is a Unicode string containing the manufacturer's details in human readable
// *  form, and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
// *  Descriptor.
// */
//const USB_Descriptor_String_t PROGMEM ManufacturerString = USB_STRING_DESCRIPTOR(L"Dean Camera");
//
///** Product descriptor string. This is a Unicode string containing the product's details in human readable form,
// *  and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
// *  Descriptor.
// */
//const USB_Descriptor_String_t PROGMEM ProductString = USB_STRING_DESCRIPTOR(L"LUFA USB-RS232 Adapter");
//
///** This function is called by the library when in device mode, and must be overridden (see library "USB Descriptors"
// *  documentation) by the application code so that the address and size of a requested descriptor can be given
// *  to the USB library. When the device receives a Get Descriptor request on the control endpoint, this function
// *  is called so that the descriptor details can be passed back and the appropriate descriptor sent back to the
// *  USB host.
// */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress
#if (defined(ARCH_HAS_MULTI_ADDRESS_SPACE) || defined(__DOXYGEN__)) && \
    !(defined(USE_FLASH_DESCRIPTORS) || defined(USE_EEPROM_DESCRIPTORS) || defined(USE_RAM_DESCRIPTORS))
                                    , uint8_t* const DescriptorMemorySpace
#endif
									)
{
	const uint8_t  DescriptorType   = (wValue >> 8);
	const uint8_t  DescriptorNumber = (wValue & 0xFF);

	const void* Address = NULL;
	uint16_t    Size    = NO_DESCRIPTOR;

	switch (DescriptorType)
	{
		case DTYPE_Device:
			Address = &DeviceDescriptor;
			Size    = sizeof(USB_Descriptor_Device_t);
			break;
		case DTYPE_Configuration:
			Address = &ConfigurationDescriptor;
			Size    = sizeof(USB_Descriptor_Configuration_t);
			break;
		case DTYPE_String:
			switch (DescriptorNumber)
			{
				case 0x00:
					Address = &LanguageString;
					Size    = pgm_read_byte(&LanguageString.Header.Size);
					break;
				case 0x01:
					Address = &ManufacturerString;
					Size    = pgm_read_byte(&ManufacturerString.Header.Size);
					break;
				case 0x02:
					Address = &ProductString;
					Size    = pgm_read_byte(&ProductString.Header.Size);
					break;
			}

			break;
	}

#if (defined(ARCH_HAS_MULTI_ADDRESS_SPACE) || defined(__DOXYGEN__)) && \
    !(defined(USE_FLASH_DESCRIPTORS) || defined(USE_EEPROM_DESCRIPTORS) || defined(USE_RAM_DESCRIPTORS))
	*DescriptorMemorySpace = MEMSPACE_FLASH;
#endif

	*DescriptorAddress = Address;
	return Size;
}

#if defined(__cplusplus)
			}
#endif
