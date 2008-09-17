/* $Id: xgemac.h,v 1.1 2008/08/14 19:08:45 meinelte Exp $ */
/******************************************************************************
*
*       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2003 Xilinx Inc.
*       All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xgemac.h
*
* The Xilinx 1 gigabit Ethernet driver component (GEMAC).
*
* The Xilinx Ethernet 1Gbit MAC supports the following features:
*   - Scatter-gather & simple DMA operations, as well as simple memory
*     mapped direct I/O interface (FIFOs)
*   - Gigabit Media Independent Interface (GMII) for connection to external
*     1Gbit Mbps PHY transceivers. Supports 125Mhz 10 bit interface (TBI) to
*     external PHY and SerDes to external transceiver
*   - GMII management control reads and writes with GMII PHYs
*   - Independent internal transmit and receive FIFOs
*   - CSMA/CD compliant operations for half-duplex modes
*   - Internal loopback
*   - Automatic source address insertion or overwrite (programmable)
*   - Automatic FCS insertion and stripping (programmable)
*   - Automatic pad insertion and stripping (programmable)
*   - Pause frame (flow control) detection in full-duplex mode
*   - Programmable interframe gap
*   - VLAN frame support
*   - Jumbo frame support
*   - Pause frame support
*
* Hardware limitations in this version
*   - Always in promiscuous mode
*   - Hardware statistic counters not implemented
*   - Unicast, multicast, broadcast, and promiscuous address filtering not
*     implemented
*   - Half-duplex mode not implemented
*   - Auto source address insertion not implemented
*
* The device driver does not support the features listed below
*   - Programmable PHY reset signal
*
* Device driver limitations in this version
*   - Simple DMA untested
*
* <b>Driver Description</b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the GEMAC. The driver handles transmission and reception of
* Ethernet frames, as well as configuration of the controller. It does not
* handle protocol stack functionality such as Link Layer Control (LLC) or the
* Address Resolution Protocol (ARP). The protocol stack that makes use of the
* driver handles this functionality.
*
* Since the driver is a simple pass-through mechanism between a protocol stack
* and the GEMAC, no assembly or disassembly of Ethernet frames is done at the
* driver-level. This assumes that the protocol stack passes a correctly
* formatted Ethernet frame to the driver for transmission, and that the driver
* does not validate the contents of an incoming frame
*
* A single device driver can support multiple GEMACs.
*
* The driver is designed for a zero-copy buffer scheme when used with DMA.
* Direct FIFO modes requires a buffer copy to/from data FIFOs.
*
* <b>PHY Communication</b>
*
* The driver provides rudimentary read and write functions to allow the higher
* layer software to access the PHY. The GEMAC provides MII registers for the
* driver to access. This management interface can be parameterized away in
* the FPGA implementation process. 
*
* <b>Asynchronous Callbacks</b>
*
* The driver services interrupts and passes Ethernet frames to the higher layer
* software through asynchronous callback functions. When using the driver
* directly (i.e., not with the RTOS protocol stack), the higher layer
* software must register its callback functions during initialization. The
* driver requires callback functions for received frames, for confirmation of
* transmitted frames, and for asynchronous errors.
*
* <b>Interrupts</b>
*
* The driver has no dependencies on the interrupt controller. The driver
* provides two interrupt handlers.  XGemac_IntrHandlerDma() handles interrupts
* when the GEMAC is configured with scatter-gather DMA.  XGemac_IntrHandlerFifo()
* handles interrupts when the GEMAC is configured for direct FIFO I/O or simple
* DMA. Either of these routines can be connected to the system interrupt
* controller by the user.
*
* <b>Device Reset</b>
*
* Some errors that can occur in the device require a device reset. These errors
* are listed in the XGemac_SetErrorHandler() function header. The user's error
* handler is responsible for resetting the device and re-configuring it based on
* its needs (the driver does not save the current configuration). When
* integrating into an RTOS, these reset and re-configure obligations are
* taken care of by the Xilinx adapter software.
*
* <b>Polled Mode Operation</b>
* 
* We hope you didn't purchase 1GB/sec GEMAC only to use it in polled mode, but
* in case you did, the driver supports this mode. See XGemac_SetOptions(),
* XGemac_PollSend(), and XGemac_PollRecv().
*
* Buffer data is copied to and from the FIFOs under processor control. The
* calling function is blocked during this copy.
*
* <b>Interrupt Driven FIFO Mode Operation</b>
*
* Buffer data is copied to and from the FIFOs under processor control.
* Interrupts occur when a new frame has arrived or a frame queued to transmit
* has been sent. The user must register callback functions with the driver
* to service frame reception and transmission. See XGemac_FifoSend(),
* XGemac_FifoRecv(), XGemac_SetFifoRecvHandler(), XGemac_SetFifoSendHandler().
*
* <b>Interrupt Driven DMA Mode Operation</b>
*
* TBD
*
* <b>Interrupt Driven Scatter Gather DMA Mode Operation</b>
*
* This is the fastest mode of operation. Buffer data is copied to and from
* the FIFOs under DMA control. Multiple frames either partial or whole can
* be transferred with no processor intervention using the scatter gather
* buffer descriptor list. The user must register callback functions with the
* driver to service frame reception and transmission. See XGemac_SgSend(),
* XGemac_SgRecv(), XGemac_SetSgRecvHandler(), XGemac_SetSgSendHandler().
*
* The frequency of interrupts can be controlled with the interrupt coalescing
* features of the scatter-gather DMA engine. Instead of interrupting after
* each packet has been processed, the scatter-gather DMA engine will interrupt 
* when the packet count threshold is reached OR when the packet waitbound timer
* has expired. A packet is a generic term used by the scatter-gather DMA engine,
* and is equivalent to an Ethernet frame in this implementation. See
* XGemac_SetPktThreshold(), and XGemac_SetPktWaitBound().
*
* The user must setup a block of memory for transmit and receive buffer
* descriptor storage. Prior to using scatter gather.
* See XGemac_SetSgRecvSpace() and XGemac_SetSgSendSpace().
*
* <b>PLB Alignment Considerations</b>
*
* Scatter gather buffer descriptors must be aligned on 8 byte boundaries.
* Frame buffers can be on any alignment. Failure to follow alignment
* restrictions will result in asserts from the driver or bad/corrupted data being
* transferred.
*
* <b>Cache Considerations</b>
*
* Do not cache buffers or scatter-gather buffer descriptor space when using
* DMA mode. Doing so will cause cache coherency problems resulting in
* bad/corrupted data being transferred.
*
* <b>Device Configuration</b>
*
* The device can be configured in various ways during the FPGA implementation
* process.  Configuration parameters are stored in the xgemac_g.c files.
* A table is defined where each entry contains configuration information
* for a GEMAC device.  This information includes such things as the base address
* of the memory-mapped device, and whether the device has DMA, counter registers,
* or GMII support.
*
* The driver tries to use the features built into the device. So if, for
* example, the hardware is configured with scatter-gather DMA, the driver
* expects to start the scatter-gather channels. If circumstances exist when
* the hardware must be used in a mode that differs from its default
* configuration, the user may modify the device config table prior to 
* invoking XGemac_Initialize():
* <pre>
*        XGemac_Config *ConfigPtr;
*
*        ConfigPtr = XGemac_LookupConfig(DeviceId);
*        ConfigPtr->IpIfDmaConfig = XGE_CFG_NO_DMA;
* </pre>
*
* The user should understand that changing the config table is not without
* risk. For example, if the hardware is not configured without DMA and the
* config table is changed to include it, then system errors will occur when
* the driver is initialized.
*
* <b>Asserts</b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at compile
* time, the NDEBUG identifier.  By default, asserts are turned on and it is
* recommended that application developers leave asserts on during development.
* Substantial performance improvements can be seen when asserts are disabled.
*
* <b>Building the driver</b>
*
* The XGemac driver is composed of several source files. Why so many?  This
* allows the user to build and link only those parts of the driver that are
* necessary. Since the GEMAC hardware can be configured in various ways (e.g.,
* with or without DMA), the driver too can be built with varying features.
* For the most part, this means that besides always linking in xgemac.c, you
* link in only the driver functionality you want. Some of the choices you have
* are polled vs. interrupt, interrupt with FIFOs only vs. interrupt with DMA,
* self-test diagnostics, and driver statistics. Note that currently the DMA code
* must be linked in, even if you don't have DMA in the device.
*
* @note
*
* Xilinx drivers are typically composed of two components, one is the driver
* and the other is the adapter.  The driver is independent of OS and processor
* and is intended to be highly portable.  The adapter is OS-specific and
* facilitates communication between the driver and the OS.
* <br><br>
* This driver is intended to be RTOS and processor independent.  It works
* with physical addresses only.  Any needs for dynamic memory management,
* threads or thread mutual exclusion, virtual memory, or cache control must
* be satisfied by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  01/13/03 First release
* 1.00b ecm  03/25/03 Revision update
* 1.00c rmm  05/28/03 Dma added, interframe gap interface change, added auto-
*                     negotiate option, removed phy function prototypes,
*                     added constant to default to no hw counters
* 1.00d rmm  06/11/03 Added Jumbo frame capabilities, removed no hw counters
*                     constant
* 1.00e rmm  11/14/03 Added XGE_NO_SGEND_INT_OPTION. SG DMA callbacks are 
*                     invoked once for all packets received instead of once
*                     for each packet.
* 1.00f rmm  12/22/03 Added XGemac_GetSgRecvFreeDesc() and GetSgSendFreeDesc()
*                     functions. Switched to DRE enabled write Packet FIFOs.
*                     Fixed SG DMA bug that could lead to a partially queued
*                     packet being tranferred. Redesigned XGemac_Start() and
*                     XGemac_Stop() functions. Moved where interrupt status
*                     is cleared after transmission in XGemac_PollSend().
* </pre>
*
******************************************************************************/

#ifndef XGEMAC_H		/* prevent circular inclusions */
#define XGEMAC_H		/* by using protection macros */

/***************************** Include Files *********************************/

#include "xbasic_types.h"
#include "xstatus.h"
#include "xparameters.h"
#include "xpacket_fifo_v2_00_a.h"	/* Uses v1.00b of Packet Fifo */
#include "xdma_channel.h"

/************************** Constant Definitions *****************************/

/*
 * Device information
 */
#define XGE_DEVICE_NAME     "xgemac"
#define XGE_DEVICE_DESC     "Xilinx Ethernet 1Gbit MAC"

/** @name Configuration options
 *    See XGemac_SetOptions())
 */
/*@{*/

/** Unicast addressing on or off (default is on) */
#define XGE_UNICAST_OPTION	0x00000001UL

/** Broadcast addressing on or off (default is on) */
#define XGE_BROADCAST_OPTION	0x00000002UL

/** Promiscuous addressing on or off (default is off) */
#define XGE_PROMISC_OPTION	0x00000004UL

/** Full duplex on or off (default is off) */
#define XGE_FDUPLEX_OPTION	0x00000008UL

/** Polled mode on or off (default is off) */
#define XGE_POLLED_OPTION	0x00000010UL

/** Internal loopback on or off (default is off) */
#define XGE_LOOPBACK_OPTION	0x00000020UL

/** Interpret pause frames in full duplex mode (default is off) */
#define XGE_FLOW_CONTROL_OPTION	0x00000080UL

/** Pad short frames on transmit (default is on) */
#define XGE_INSERT_PAD_OPTION	0x00000100UL

/** Insert FCS (CRC) on transmit (default is on) */
#define XGE_INSERT_FCS_OPTION	0x00000200UL

/** Strip padding and FCS from received frames (default is off) */
#define XGE_STRIP_PAD_FCS_OPTION	0x00002000UL

/** Turn on PHY auto-negotiation (default is on) */
#define XGE_AUTO_NEGOTIATE_OPTION	0x00004000UL

/** Allow reception and transmission of VLAN frames (default is off) */
#define XGE_VLAN_OPTION		0x00008000UL

/** Allow reception and transmission of Jumbo frames (default is off) */
#define XGE_JUMBO_OPTION	0x00010000UL

/** Disables the SGEND interrupt with SG DMA. Setting this option to ON may
 * help bulk data transfer performance when utilizing higher packet threshold
 * counts on slower systems (default is off) */
#define XGE_NO_SGEND_INT_OPTION	0x00020000UL
/*@}*/

/** @name Configuration options not yet supported
 * These options to be supported in later versions of HW and this driver.
 @{*/

/** Multicast addressing on or off (default is off) */
#define XGE_MULTICAST_OPTION	0x00000040UL

/** Insert source address on transmit (default is on) */
#define XGE_INSERT_ADDR_OPTION	0x00000400UL

/** Overwrite source address on transmit. This is only used if
 * source address insertion is on. (default is on) */
#define XGE_OVWRT_ADDR_OPTION	0x00000800UL
/*@}*/

/*
 * Some default values for interrupt coalescing within the scatter-gather
 * DMA engine.
 */
#define XGE_SGDMA_DFT_THRESHOLD		1	/* Default pkt threshold */
#define XGE_SGDMA_MAX_THRESHOLD		255	/* Maximum pkt theshold */
#define XGE_SGDMA_DFT_WAITBOUND		5	/* Default pkt wait bound (msec) */
#define XGE_SGDMA_MAX_WAITBOUND		1023	/* Maximum pkt wait bound (msec) */

/*
 * Direction identifiers. These are used for setting values like packet
 * thresholds and wait bound for specific channels
 */
#define XGE_SEND	1
#define XGE_RECV	2

/*
 * Arguments to SgSend function to indicate whether to hold off starting
 * the scatter-gather engine.
 */
#define XGE_SGDMA_NODELAY	0	/* start SG DMA immediately */
#define XGE_SGDMA_DELAY		1	/* do not start SG DMA */

/*
 * Constants to determine the configuration of the hardware device. They are
 * used to allow the driver to verify it can operate with the hardware.
 */
#define XGE_CFG_NO_IPIF		0	/* Not supported by the driver */
#define XGE_CFG_NO_DMA		1	/* No DMA */
#define XGE_CFG_SIMPLE_DMA	2	/* Simple DMA */
#define XGE_CFG_DMA_SG		3	/* DMA scatter gather */

/*
 * The next few constants help upper layers determine the size of memory
 * pools used for Ethernet buffers and descriptor lists.
 */
#define XGE_MAC_ADDR_SIZE		6	/* six-byte MAC address */
#define XGE_MTU				1500	/* max payload size of Ethernet frame */
#define XGE_JUMBO_MTU			8982	/* max payload size of jumbo frame */
#define XGE_HDR_SIZE			14	/* size of Ethernet header */
#define XGE_HDR_VLAN_SIZE		18	/* size of Ethernet header with VLAN */
#define XGE_TRL_SIZE			4	/* size of Ethernet trailer (FCS) */
#define XGE_MAX_FRAME_SIZE		(XGE_MTU + XGE_HDR_SIZE + XGE_TRL_SIZE)
#define XGE_MAX_VLAN_FRAME_SIZE		(XGE_MTU + XGE_HDR_VLAN_SIZE + XGE_TRL_SIZE)
#define XGE_MAX_JUMBO_FRAME_SIZE	(XGE_JUMBO_MTU + XGE_HDR_SIZE + XGE_TRL_SIZE)

/*
 * Define a default number of send and receive buffers
 */
#define XGE_MIN_RECV_BUFS	32	/* minimum # of recv buffers */
#define XGE_DFT_RECV_BUFS	64	/* default # of recv buffers */

#define XGE_MIN_SEND_BUFS	16	/* minimum # of send buffers */
#define XGE_DFT_SEND_BUFS	32	/* default # of send buffers */

#define XGE_MIN_BUFFERS		(XGE_MIN_RECV_BUFS + XGE_MIN_SEND_BUFS)
#define XGE_DFT_BUFFERS		(XGE_DFT_RECV_BUFS + XGE_DFT_SEND_BUFS)

/*
 * Define the number of send and receive buffer descriptors, used for
 * scatter-gather DMA
 */
#define XGE_MIN_RECV_DESC	16	/* minimum # of recv descriptors */
#define XGE_DFT_RECV_DESC	32	/* default # of recv descriptors */

#define XGE_MIN_SEND_DESC	8	/* minimum # of send descriptors */
#define XGE_DFT_SEND_DESC	16	/* default # of send descriptors */

/**************************** Type Definitions *******************************/

/**
 * Statistics mainained by SW
 */
typedef struct
{
	u32 XmitOverrunErrors;		/**< Number of transmit overrun errors */
	u32 XmitUnderrunErrors;		/**< Number of transmit underrun errors */
	u32 XmitExcessDeferralErrors;	/**< Number of transmit deferral errors */
	u32 XmitPFifoUnderrunErrors;	/**< Number of transmit packet fifo
						underrun errors */
	u32 XmitLateCollErrors;		/**< Number of late collision errors */
	u32 RecvSlotLengthErrors;	/**< Number of recv frames received with
						slot length errors */
	u32 RecvOverrunErrors;		/**< Number of recv frames discarded due
						to overrun errors */
	u32 RecvUnderrunErrors;		/**< Number of recv underrun errors */
	u32 RecvLengthFieldErrors;	/**< Number of recv frames discarded with
						invalid length field */
	u32 RecvLongErrors;		/**< Number of recv long frames discarded */
	u32 RecvFcsErrors;		/**< Number of recv FCS errors */
	u32 DmaErrors;			/**< Number of DMA errors since init */
	u32 FifoErrors;			/**< Number of FIFO errors since init */
	u32 RecvInterrupts;		/**< Number of receive interrupts */
	u32 XmitInterrupts;		/**< Number of transmit interrupts */
	u32 EmacInterrupts;		/**< Number of MAC (device) interrupts */
	u32 TotalInterrupts;		/**< Total interrupts */
} XGemac_SoftStats;


/**
 * Statistics maintained by HW
 */
typedef struct
{
	Xuint64 RecvFrames;		/**< Number of frames received */
	Xuint64 RecvFcs;		/**< Number of received frames discarded
						due to FCS errors */
	Xuint64 RecvBroadcast;		/**< Number of broadcast frames received */
	Xuint64 RecvMulticast;		/**< Number of multicast frames received */
	Xuint64 Recv64Byte;		/**< Number of 64 byte frames received */
	Xuint64 Recv65_127Byte;		/**< Number of 65-127 byte frames
						received */
	Xuint64 Recv128_255Byte;	/**< Number of 128-255 byte frames
						received */
	Xuint64 Recv256_511Byte;	/**< Number of 256-511 byte frames
						received */
	Xuint64 Recv512_1023Byte;	/**< Number of 512-1023 byte frames
						received */
	Xuint64 Recv1024_MaxByte;	/**< Number of 1024 and larger byte frames
						received */
	Xuint64 RecvControl;		/**< Number of control frames received */
	Xuint64 RecvLengthRange;	/**< Number of received frames with length
						or type that didn't match number of
						bytes actually received */
	Xuint64 RecvVlan;		/**< Number of VLAN frames received */
	Xuint64 RecvPause;		/**< Number of pause frames received */
	Xuint64 RecvBadOpcode;		/**< Number of control frames received
						with an invalid opcode */
	Xuint64 RecvLong;		/**< Number of oversized frames received */
	Xuint64 RecvShort;		/**< Number of undersized frames received */
	Xuint64 RecvFragment;		/**< Number of received frames less than 64
						bytes discarded due to FCS errors */
	Xuint64 RecvBytes;		/**< Number of bytes received */
	Xuint64 XmitBytes;		/**< Number of bytes transmitted */
	Xuint64 XmitFrames;		/**< Number of frames transmitted */
	Xuint64 XmitBroadcast;		/**< Number of broadcast frames transmitted */
	Xuint64 XmitMulticast;		/**< Number of multicast frames transmitted */
	Xuint64 XmitUnderrun;		/**< Number of frames not sent due to
						underrun */
	Xuint64 XmitControl;		/**< Number of control frames transmitted */
	Xuint64 Xmit64Byte;		/**< Number of 64 byte frames transmitted */
	Xuint64 Xmit65_127Byte;		/**< Number of 65-127 byte frames
						transmitted */
	Xuint64 Xmit128_255Byte;	/**< Number of 128-255 byte frames
						transmitted */
	Xuint64 Xmit256_511Byte;	/**< Number of 256-511 byte frames
						transmitted */
	Xuint64 Xmit512_1023Byte;	/**< Number of 512-1023 byte frames
						transmitted */
	Xuint64 Xmit1024_MaxByte;	/**< Number of 1024 and larger byte frames
						transmitted */
	Xuint64 XmitVlan;		/**< Number of VLAN frames transmitted */
	Xuint64 XmitPause;		/**< Number of pause frames transmitted */
	Xuint64 XmitLong;		/**< Number of oversized frames transmitted */
	Xuint64 Xmit1stCollision;	/**< Number of frames involved in a single
						collision but sent successfully */
	Xuint64 XmitMultiCollision;	/**< Number of frames involved in a multiple
						collision but sent successfully */
	Xuint64 XmitDeferred;		/**< Number of frames delayed because the
						medium was busy */
	Xuint64 XmitLateColision;	/**< Number of frames involved in a late
						collision but sent successfully */
	Xuint64 XmitExcessCollision;	/**< Number of frames discarded due to
						excess collisions */
	Xuint64 XmitCarrierSense;	/**< Number of frames not sent due to the
						GMII_CRS signal being negated */ 
	Xuint64 XmitExcessDeferred;	/**< Number of frames not sent due to
						excess deferral times */
} XGemac_HardStats;

/**
 * This typedef contains configuration information for a device.
 */
typedef struct
{
	u16 DeviceId;		/**< Unique ID of device */
	u32 BaseAddress;	/**< Register base address */
	u32 PhysAddress;	/**< Register base address */
	u8 IpIfDmaConfig;	/**< IPIF/DMA hardware configuration */
	u32 HasGmii;		/**< Does device support GMII? */
	u32 HasCounters;	/**< Does device have HW statistic counters */
} XGemac_Config;

/** @name Callbacks
 * @{
 */
/**
 * Callback when an Ethernet frame is sent or received with scatter-gather DMA.
 *
 * @param CallBackRef is a callback reference passed in by the upper layer
 *        when setting the callback functions (see XGemac_SetSgRecvHandler()
 *        and XGemac_SetSgSendHandler()).
 * @param BdPtr is a pointer to the first buffer descriptor in a list of
 *        buffer descriptors that describe a single frame.
 * @param NumBds is the number of buffer descriptors in the list pointed
 *        to by BdPtr.
 */
typedef void (*XGemac_SgHandler) (void *CallBackRef, XBufDescriptor *BdPtr,
				u32 NumBds);

/**
 * Callback when data is sent or received with direct FIFO communication.
 *
 * @param CallBackRef is a callback reference passed in by the upper layer
 *        when setting the callback functions (see XGemac_SetFifoRecvHandler()
 *        and XGemac_SetFifoSendHandler()).
 */
typedef void (*XGemac_FifoHandler) (void *CallBackRef);

/**
 * Callback when an asynchronous error occurs.
 *
 * @param CallBackRef is a callback reference passed in by the upper layer
 *        when setting the callback functions (see XGemac_SetFifoRecvHandler(),
 *        XGemac_SetFifoSendHandler(), XGemac_SetSgRecvHandler(), and
 *        XGemac_SetSgSendHandler().
 * @param ErrorCode is the Xilinx error code that was detected. (see xstatus.h).
 */
typedef void (*XGemac_ErrorHandler) (void *CallBackRef, XStatus ErrorCode);
/*@}*/

/**
 * The XGemac driver instance data. The user is required to allocate a
 * variable of this type for every GEMAC device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct
{
	u32 BaseAddress;			/* Base address (of IPIF) */
	u32 PhysAddress;			/* Base address (of IPIF) */
	u32 IsStarted;				/* Device is currently started */
	u32 IsReady;				/* Device is initialized and ready */
	u32 IsPolled;				/* Device is in polled mode */
	u8 IpIfDmaConfig;			/* IPIF/DMA hardware configuration */
	u32 HasGmii;				/* Does device support GMII? */
	u32 HasCounters;			/* Does device support multicast hash table? */

	XGemac_SoftStats Stats;			/* Statistics maintianed by the driver */
	XPacketFifoV200a RecvFifo;		/* FIFO used by receive DMA channel */
	XPacketFifoV200a SendFifo;		/* FIFO used by send DMA channel */

	XGemac_FifoHandler FifoRecvHandler;	/* callback for non-DMA interrupts */
	void *FifoRecvRef;
	XGemac_FifoHandler FifoSendHandler;	/* callback for non-DMA interrupts */
	void *FifoSendRef;
	XGemac_ErrorHandler ErrorHandler;	/* callback for asynchronous errors */
	void *ErrorRef;

	XDmaChannel RecvChannel;		/* DMA receive channel driver */
	XDmaChannel SendChannel;		/* DMA send channel driver */
	u32 IsSgEndDisable;			/* Does SG DMA enable SGEND interrupt */

	XGemac_SgHandler SgRecvHandler;		/* callback for scatter-gather DMA */
	void *SgRecvRef;
	XGemac_SgHandler SgSendHandler;		/* callback for scatter-gather DMA */
	void *SgSendRef;
} XGemac;

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Macro functions
 * @{
 */

/*****************************************************************************/
/**
*
* This macro determines if the device is currently configured for
* scatter-gather DMA.
*
* @param InstancePtr is a pointer to the XGemac instance to be worked on.
*
* @return
*
* Boolean TRUE if the device is configured for scatter-gather DMA, or FALSE
* if it is not.
*
* @note
*
* Signature: u32 XGemac_mIsSgDma(XGemac *InstancePtr)
*
******************************************************************************/
#define XGemac_mIsSgDma(InstancePtr) \
  ((InstancePtr)->IpIfDmaConfig == XGE_CFG_DMA_SG)

/*****************************************************************************/
/**
*
* This macro determines if the device is currently configured for simple DMA.
*
* @param InstancePtr is a pointer to the XGemac instance to be worked on.
*
* @return
*
* Boolean TRUE if the device is configured for simple DMA, or FALSE otherwise
*
* @note
*
* Signature: u32 XGemac_mIsSimpleDma(XGemac *InstancePtr)
*
******************************************************************************/
#define XGemac_mIsSimpleDma(InstancePtr) \
  ((InstancePtr)->IpIfDmaConfig == XGE_CFG_SIMPLE_DMA)

/*****************************************************************************/
/**
*
* This macro determines if the device is currently configured with DMA (either
* simple DMA or scatter-gather DMA)
*
* @param InstancePtr is a pointer to the XGemac instance to be worked on.
*
* @return
*
* Boolean TRUE if the device is configured with DMA, or FALSE otherwise
*
* @note
*
* Signature: u32 XGemac_mIsDma(XGemac *InstancePtr)
*
******************************************************************************/
#define XGemac_mIsDma(InstancePtr) \
  (XGemac_mIsSimpleDma(InstancePtr) || XGemac_mIsSgDma(InstancePtr))
/*@}*/

/************************** Function Prototypes ******************************/

/*
 * Initialization functions in xgemac.c
 */
XStatus XGemac_Initialize(XGemac * InstancePtr, u16 DeviceId);
XStatus XGemac_Start(XGemac * InstancePtr);
XStatus XGemac_Stop(XGemac * InstancePtr);
void XGemac_Reset(XGemac * InstancePtr);
XGemac_Config *XGemac_LookupConfig(u16 DeviceId);
XStatus XGemac_SetMacAddress(XGemac * InstancePtr, u8 * AddressPtr);
void XGemac_GetMacAddress(XGemac * InstancePtr, u8 * BufferPtr);

/*
 * Diagnostic functions in xgemac_selftest.c
 */
XStatus XGemac_SelfTest(XGemac * InstancePtr);

/*
 * Polled functions in xgemac_polled.c
 */
XStatus XGemac_PollSend(XGemac * InstancePtr, u8 * BufPtr, u32 ByteCount);
XStatus XGemac_PollRecv(XGemac * InstancePtr, u8 * BufPtr, u32 * ByteCountPtr);

/*
 * Interrupts with scatter-gather DMA functions in xgemac_intr_dma.c
 * This functionality is not yet supported, calling any of these functions
 * will result in an assert.
 */
XStatus XGemac_SgSend(XGemac * InstancePtr, XBufDescriptor * BdPtr, int Delay);
XStatus XGemac_SgRecv(XGemac * InstancePtr, XBufDescriptor * BdPtr);
XStatus XGemac_SetPktThreshold(XGemac * InstancePtr, u32 Direction,
				u8 Threshold);
XStatus XGemac_GetPktThreshold(XGemac * InstancePtr, u32 Direction,
				u8 * ThreshPtr);
XStatus XGemac_SetPktWaitBound(XGemac * InstancePtr, u32 Direction,
				u32 TimerValue);
XStatus XGemac_GetPktWaitBound(XGemac * InstancePtr, u32 Direction,
				u32 * WaitPtr);
XStatus XGemac_SetSgRecvSpace(XGemac * InstancePtr, u32 * MemoryPtr,
				u32 ByteCount, void * PhyPtr);
XStatus XGemac_SetSgSendSpace(XGemac * InstancePtr, u32 * MemoryPtr,
				u32 ByteCount, void * PhyPtr);
void XGemac_SetSgRecvHandler(XGemac * InstancePtr, void * CallBackRef,
				XGemac_SgHandler FuncPtr);
void XGemac_SetSgSendHandler(XGemac * InstancePtr, void * CallBackRef,
				XGemac_SgHandler FuncPtr);

void XGemac_IntrHandlerDma(void * InstancePtr);		/* interrupt handler */

/*
 * Interrupts with direct FIFO functions in xgemac_intr_fifo.c
 */
XStatus XGemac_FifoSend(XGemac * InstancePtr, u8 * BufPtr, u32 ByteCount);
XStatus XGemac_FifoRecv(XGemac * InstancePtr, u8 * BufPtr, u32 * ByteCountPtr);
void XGemac_SetFifoRecvHandler(XGemac * InstancePtr, void * CallBackRef,
				XGemac_FifoHandler FuncPtr);
void XGemac_SetFifoSendHandler(XGemac * InstancePtr, void * CallBackRef,
				XGemac_FifoHandler FuncPtr);

void XGemac_IntrHandlerFifo(void * InstancePtr);	/* interrupt handler */

/*
 * General interrupt-related functions in xgemac_intr.c
 */
void XGemac_SetErrorHandler(XGemac * InstancePtr, void * CallBackRef,
				XGemac_ErrorHandler FuncPtr);

/*
 * MAC configuration in xgemac_options.c
 */
XStatus XGemac_SetOptions(XGemac * InstancePtr, u32 OptionFlag);
u32 XGemac_GetOptions(XGemac * InstancePtr);

/*
 * Multicast address related functions in xgemac_multicast.c
 */
XStatus XGemac_MulticastAdd(XGemac * InstancePtr, u8 Location, u8 * AddressPtr);
XStatus XGemac_MulticastClear(XGemac * InstancePtr, u8 Location);

/*
 * Statistics in xgemac_stats.c
 */
void XGemac_GetSoftStats(XGemac * InstancePtr, XGemac_SoftStats * StatsPtr);
void XGemac_ClearSoftStats(XGemac * InstancePtr);
XStatus XGemac_GetHardStats(XGemac * InstancePtr, XGemac_HardStats * StatsPtr);

/*
 * Other control functions in xgemac_control.c
 */
XStatus XGemac_SetInterframeGap(XGemac * InstancePtr, u8 Ifg);
void XGemac_GetInterframeGap(XGemac * InstancePtr, u8 * IfgPtr);
XStatus XGemac_SendPause(XGemac * InstancePtr, u16 PausePeriod);
XStatus XGemac_MgtRead(XGemac * InstancePtr, int PhyAddress, int Register,
			u16 *DataPtr);
XStatus XGemac_MgtWrite(XGemac * InstancePtr, int PhyAddress, int Register,
			u16 Data);

#endif		/* end of protection macro */
