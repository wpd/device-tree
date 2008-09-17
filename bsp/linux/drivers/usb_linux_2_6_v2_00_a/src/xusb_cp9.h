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
 * @file xusb_cp9.h
 *
 * This file contains the constants, typedefs, variables and functions 
 * prototypes related to the USB chapter 9 related code.
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

#ifndef XUSB_CH9_H
#define XUSB_CH9_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files **********************************/

#include "xusb.h"
#include "xusb_types.h"

/**************************** Type Definitions ********************************/
typedef struct  {
         u8 bRequestType;
         u8 bRequest; 
         u16 wValue;
         u16 wIndex;
         u16 wLength;
} UsbCtrlRequest;

/************************** Function Prototypes *******************************/

void EP0ProcessOutToken(XUsb * InstancePtr);
void EP0ProcessInToken(XUsb * InstancePtr);
int Chapter9(XUsb * InstancePtr, UsbCtrlRequest *Ctrl);
int ExecuteCommand(XUsb * InstancePtr);
void GetInterface(XUsb * InstancePtr);
void SetInterface(XUsb * InstancePtr);
void SetupControlWriteStatusStage(XUsb * InstancePtr);
void GetStatus(XUsb * InstancePtr);
void GetDescriptor(XUsb * InstancePtr);
void SetDescriptor(XUsb * InstancePtr);
void GetConfiguration(XUsb * InstancePtr);
void SetConfiguration(XUsb * InstancePtr);
void SetClearFeature(XUsb * InstancePtr, int flag);
void LoadEP0(XUsb * InstancePtr);
extern void InitUsbInterface(XUsb * InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* XUSB_CH9_H */
