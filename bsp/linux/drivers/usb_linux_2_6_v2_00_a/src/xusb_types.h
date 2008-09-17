/******************************************************************************
 *
 *       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
 *       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
 *       SOLUTIONS FOR XILINX DEVICES. BY PROVIDING THIS DESIGN, CODE,
 *       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
 *       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
 *       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
 *       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
 *       FOR YOUR IMPLEMENTATION. XILINX EXPRESSLY DISCLAIMS ANY
 *       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
 *       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
 *       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
 *       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *       FOR A PARTICULAR PURPOSE.
 *
 *	 Copyright (C) 2006 Vreelin Engineering, Inc.  All Rights Reserved.
 *       (c) Copyright 2007 Xilinx Inc.
 *       All rights reserved.
 *
 ******************************************************************************/
/******************************************************************************/
/**
 * @file xusb_types.h
 *
 * This file contains the constants, type definitions, variables as used in the 
 * USB chapter 9 and mass storage demo application.
 *
 * @note     None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- ------------------------------------------------------------------
 * 1.00a hvm  2/22/07 First release
 *
 * </pre>
 *****************************************************************************/

#ifndef __XUSBTYPES_H__
#define __XUSBTYPES_H__
#ifdef __cplusplus
extern "C" {
#endif

/*
 * USB device selected as Mass Storage device.
 */
#define MASS_STORAGE_DEVICE	TRUE

/*
 * Chapter 9 requests
 */
#define GET_STATUS				0x00
#define CLEAR_FEATURE				0x01
#define GET_STATE				0x02
#define SET_FEATURE				0x03
#define SET_ADDRESS				0x05
#define GET_DESCRIPTOR				0x06
#define SET_DESCRIPTOR				0x07
#define GET_CONFIGURATION			0x08
#define SET_CONFIGURATION		 	0x09
#define GET_INTERFACE				0x0A
#define SET_INTERFACE				0x0B
#define SYCH_FRAME				0x0C

/*
 * Test Mode Options
 */
#define TEST_J					1
#define TEST_K					2
#define TEST_SE0_NAK				3
#define TEST_PKT				4

/*
 * USB Mass Storage requests
 */
#define MS_RESET				0xFF
#define MS_GETMAXLUN		   		0xFE

/* 
 * Request types used during USB enumeration. 
 */
#define STANDARD_IN_DEVICE			0x80
#define STANDARD_IN_INTERFACE			0x81
#define STANDARD_IN_ENDPOINT			0x82
#define STANDARD_OUT_DEVICE			0x00
#define STANDARD_OUT_INTERFACE			0x01
#define STANDARD_OUT_ENDPOINT			0x02
#define TYPE_MASK			 	0x60
#define TYPE_STANDARD				0x00
#define TYPE_CLASS				0x20
#define TYPE_VENDOR				0x40
#define TYPE_RESERVED				0x60

/*
 * DATA Transfer Direction
 */
#define DIR_DEVICE_TO_HOST			0x80

/*
 * Descriptor Types
 */
#define DEVICE_DESCR				0x01
#define CONFIG_DESCR				0x02
#define STRING_DESCR				0x03
#define INTERFACE_DESCR				0x04
#define ENDPOINT_DESCR				0x05
#define QUALIFIER_DESCR				0x06
#define OSD_CONFIG_DESCR			0x07

/*
 * Feature Selectors
 */

#define DEVICE_REMOTE_WAKEUP	0x01
#define TEST_MODE				0x02

/*
 * Phase States
 */
#define SETUP_PHASE				0x0000
#define DATA_PHASE				0x0001
#define STATUS_PHASE		   	0x0002
/*
 * Maximum number of USB interfaces.
 */
#define MAX_INTERFACES			0x01

/*
 * FPGA Configuration Number
 */
#define CONFIGURATION_ONE	   	0x01

/* 
 * EP0 Setup data size.
 */
#define EP0_SETUP_DATA				64

/* 
 * Command Buffer Structure. 
 */
typedef struct {
	union {
		u8 StandardDeviceRequest;
		u8 bmRequestType;
	} Byte0;
	union {
		u8 FbRequest;
		u8 bRequest;
	} Byte1;
	union {
		struct {
			u8 bDescriptorType;
			u8 bDescriptorIndex;
		} Byte23;
		u16 FwValue;
		u16 wValue;
		u16 wFeatureSelector;
	} Word1;
	union {
		struct {
			u8 Byteh;
			u8 Bytel;
		} Byte45;
		u16 wTargetSelector;
		u16 FwIndex;
		u16 wIndex;
	} Word2;
	union {
		struct {
			u8 Byteh;
			u8 Bytel;
		} Byte67;
		u16 wLength;
	} Word3;
	u8 *ContReadPtr;
	u8 *ContWritePtr;
	u32 ContReadCount;
	u32 ContWriteCount;
	u32 SetupSeqTX;
	u32 SetupSeqRX;
	u8 ContReadDataBuffer[EP0_SETUP_DATA];
} USB_CMD_BUF;

/*
 * Standard USB structures as per 2.0 specification
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 bcdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize0;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
	u8 iManufacturer;
	u8 iProduct;
	u8 iSerialNumber;
	u8 bNumConfigurations;

} USB_STD_DEV_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u8 bVersionLow;
	u8 bVersionHigh;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bProtocol;
	u8 bMaxPkt0;
	u8 bNumberConfigurations;
	u8 breserved;
} USB_STD_QUAL_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u8 bTotalLength;
	u8 bCorrection;
	u8 bNumberInterfaces;
	u8 bConfigValue;
	u8 bIConfigString;
	u8 bAttributes;
	u8 bMaxPower;
} USB_STD_CFG_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u8 bTotalLength;
	u8 bCorrection;
	u8 bNumberInterfaces;
	u8 bConfigValue;
	u8 bIConfigString;
	u8 bAttributes;
	u8 bMaxPower;
} USB_STD_QUA_DESC;

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bInterfaceNumber;
	u8 bAlternateSetting;
	u8 bNumEndPoints;
	u8 bInterfaceClass;
	u8 bInterfaceSubClass;
	u8 bInterfaceProtocol;
	u8 iInterface;

} USB_STD_IF_DESC;

/*
 * The standard USB structures as per USB 2.0 specification.
 */
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u8 bMaxPacketSizeL;
	u8 bMaxPacketSizeH;
	u8 bInterval;
} USB_STD_EP_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u16 wLANGID[1];
} USB_STD_STRING_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u8 bString[14];
} USB_STD_STRING_MAN_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u8 bString[10];
} USB_STD_STRING_PS_DESC;

typedef struct {
	u8 bLength;
	u8 bType;
	u8 bString[42];
} USB_STD_STRING_SN_DESC;

#ifdef MASS_STORAGE_DEVICE
/*
 * USB configuration structure.
 */
typedef struct {
	USB_STD_CFG_DESC stdCfg;
	USB_STD_IF_DESC ifCfg;
	USB_STD_EP_DESC epCfg1;
	USB_STD_EP_DESC epCfg2;
} FPGA1_CONFIGURATION;

#else
/*
 * USB configuration structure.
 */
typedef struct {
	USB_STD_CFG_DESC	stdCfg;
	USB_STD_IF_DESC		ifCfg;
	USB_STD_EP_DESC		epCfg1;
	USB_STD_EP_DESC		epCfg2;
	USB_STD_EP_DESC		epCfg3;
	USB_STD_EP_DESC		epCfg4;
	USB_STD_EP_DESC		epCfg5;
	USB_STD_EP_DESC		epCfg6;
	USB_STD_EP_DESC		epCfg7;

} FPGA1_CONFIGURATION;
#endif

#ifdef __cplusplus
}
#endif

#endif /* __XUSBTYPES_H__ */
