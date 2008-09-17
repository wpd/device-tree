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
 *       (c) Copyright 2006-2007 Xilinx Inc.
 *       All rights reserved.
 *
 *****************************************************************************/
/*****************************************************************************/
/**
*
* @file xlnx_udc.h
*
* This file contains the implementation of the XUsb component. It is the
* driver for the USB device controller.
*
* The USB device controller supports the following features:
*	- USB 2.0 Specification supporting High/Full/Low Speed
*	- 8 Endpoints
*		- 1 Control Endpoint
*		- 7 Configurable Endpoints, which can be configured
*			as IN or OUT , and configurable as Interrupt or Bulk or
* 			Isochronous
*	- 2 Ping Pong Buffers for all the endpoints except the Control Endpoint
*
* <b>Initialization & Configuration</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the USB device. Apart from transmission and reception of
* USB frames the driver also handles the configuration of the device. A single
* device driver can support multiple USB devices.
*
* {XUsb_CfgInitialize()} API is used to initialize the EPB CAN device.
* The user needs to first call the {XUsb_LookupConfig()} API which returns
* the Configuration structure pointer which is passed as a parameter to the
* XUsb_CfgInitialize() API.
*
* - Configuration of the DEVICE endpoints:
*   The endpoints of the device need to be configured using the
*  	XUsb_EpConfigure() function call.
*   After configuration is complete, the endpoints need to be initialized
*   using the XUsb_EpInitializeAll() function call.
*
* <b> PHY Communication </b>
*
* As the H/W doesn't give any provision for the driver to configure the PHY, the
* driver doesn't provide any mechanism for directly programming the PHY.
*
* <b> Interrupts </b>
*
* The driver provides an interrupt handler {XUsb_IntrHandler} for handling
* the interrupt from the USB device. The users of this driver have to
* register this handler with the interrupt system and provide the callback
* functions.
* The interrupt handlers and associated callback functions for the USB device
* have to be registered by the user using the {XUsb_IntrSetHandler()} function
* and/or {XUsb_EpSetHandler()} function.
*
* {XUsb_IntrSetHandler()} function installs an asynchronous callback function
* for the general interrupts (interrupts other than the endpoint interrupts).
*
* {XUsb_EpSetHandler()} function installs the callback functions for the
* interrupts related to the endpoint events. A separate callback function has to
* be installed for each of the endpoints.
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The XUsb driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- ------------------------------------------------------------------
* 1.00a hvm  02/22/07 Firt Release.
*
* </pre>
*
 ******************************************************************************/
#ifndef XLNXUSB_H
#define XLNXUSB_H

struct xusb_ep {
		struct usb_ep	ep;
		struct list_head queue;
		struct xusb_udc	*udc;
		u16 		epnumber;
		u8		is_in;
		u8		stopped;
		u8		is_iso;
		u16		maxpacket;
		u32		rambase;
		const struct usb_endpoint_descriptor *desc;
 };

/*
 * driver is non-SMP, and just blocks IRQs whenever it needs
 * access protection for chip registers or driver state
 */
struct xusb_udc {
		struct usb_gadget	gadget;
		spinlock_t			lock;
		struct xusb_ep	ep[XUSB_MAX_ENDPOINTS];
		XUsb	usb;
		struct usb_gadget_driver	*driver;
		unsigned			vbus:1;
		unsigned			enabled:1;
		unsigned			clocked:1;
		unsigned			suspended:1;
		unsigned			req_pending:1;
		unsigned			wait_for_addr_ack:1;
		unsigned			wait_for_config_ack:1;
		unsigned			selfpowered:1;
#define start_watchdog(dev) mod_timer(&dev->timer, jiffies + (HZ/200))
		struct timer_list			timer;
		struct device				*dev;
		struct proc_dir_entry			*pde;
		unsigned long 				remap_size;
 };

static inline struct xusb_udc *to_udc(struct usb_gadget *g){

	return container_of(g, struct xusb_udc, gadget);
}

struct xusb_request {
	struct usb_request	req;
	struct list_head	queue;
};

extern u16 MaxControlSize;
/****************************************************************************/
#define  DEBUG	1
#define  VERBOSE 1

#ifdef DEBUG
#define DBG(stuff...)	printk(KERN_DEBUG "udc: " stuff)
#else
#define DBG(stuff...)	do{}while(0)
#endif

#ifdef VERBOSE
#    define VDBG	DBG
#else
#    define VDBG(stuff...)	do{}while(0)
#endif

#ifdef PACKET_TRACE
#    define PACKET	VDBG
#else
#    define PACKET(stuff...)	do{}while(0)
#endif

#define ERR(stuff...)	printk(KERN_ERR "udc: " stuff)
#define WARN(stuff...)	printk(KERN_WARNING "udc: " stuff)
#define INFO(stuff...)	printk(KERN_INFO "udc: " stuff)

/*************Function prototypes **************************/

void UsbIfIntrHandler(void *CallBackRef, u32 IntrStatus);
void EpIntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
void Ep0IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);
void Ep1IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus);

#endif


