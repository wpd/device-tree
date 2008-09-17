/* $Id: xgemac.c,v 1.1 2008/08/14 19:08:45 meinelte Exp $ */
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
*       (c) Copyright 2002 Xilinx Inc.
*       All rights reserved.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xgemac.c
*
* The XGemac driver. Functions in this file are the minimum required functions
* for this driver. See xgemac.h for a detailed description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Who  Date     Changes
* ---- -------- -----------------------------------------------
* ecm  01/13/03 First release
* ecm  03/25/03 Revision update
* rmm  05/28/03 DMA mods
* rmm  09/12/03 Cleanup
* rmm  11/14/03 Implemented XGE_NO_SGEND_INT_OPTION
* rmm  03/03/04 Restructured Start() and Stop() functions.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xbasic_types.h"
#include "xgemac_i.h"
#include "xio.h"
#include "xipif_v1_23_b.h"	/* Uses v1.23b of the IPIF */
#include <asm/delay.h>

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static XStatus ConfigureDma(XGemac *InstancePtr);
static XStatus ConfigureFifo(XGemac *InstancePtr);
static void StubFifoHandler(void *CallBackRef);
static void StubErrorHandler(void *CallBackRef, XStatus ErrorCode);
static void StubSgHandler(void *CallBackRef, XBufDescriptor *BdPtr,
			u32 NumBds);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Initialize a specific XGemac instance/driver.  The initialization entails:
* - Clearing memory occupied by the XGemac structure
* - Initialize fields of the XGemac structure
* - Clear the Ethernet statistics for this device
* - Initialize the IPIF component with its register base address
* - Configure the FIFO components with their register base addresses.
* - If the device is configured with DMA, configure the DMA channel components
*   with their register base addresses. At some later time, memory pools for
*   the scatter-gather descriptor lists are to be passed to the driver.
* - Reset the Ethernet MAC
*
* @param InstancePtr is a pointer to the XGemac instance to be worked on.
* @param DeviceId is the unique id of the device controlled by this XGemac
*        instance.  Passing in a device id associates the generic XGemac
*        instance to a specific device, as chosen by the caller or application
*        developer.
*
* @return
*
* - XST_SUCCESS if initialization was successful
* - XST_DEVICE_NOT_FOUND if device configuration information was not found for
*   a device with the supplied device ID.
*
* @note
*
* None.
*
******************************************************************************/
XStatus
XGemac_Initialize(XGemac * InstancePtr, u16 DeviceId)
{
	XStatus Result;
	XGemac_Config *ConfigPtr;

	/* Assert bad arguments and conditions */
	XASSERT_NONVOID(InstancePtr != NULL);

	/* Clear memory */
	memset (InstancePtr, 0, sizeof(XGemac));

	/*
	 * Lookup the device configuration. Use this configuration info down below
	 * when initializing this component.
	 */
	ConfigPtr = XGemac_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	/* Set some default values */
	InstancePtr->IsReady = 0;
	InstancePtr->IsStarted = 0;
	InstancePtr->IpIfDmaConfig = ConfigPtr->IpIfDmaConfig;
	InstancePtr->HasGmii = ConfigPtr->HasGmii;
	InstancePtr->HasCounters = ConfigPtr->HasCounters;

	/* Always default polled to false, let user configure this mode */
	InstancePtr->IsPolled = FALSE;

	/* Set all handlers to stub values, let user configure this data */
	InstancePtr->FifoRecvHandler = StubFifoHandler;
	InstancePtr->FifoSendHandler = StubFifoHandler;
	InstancePtr->ErrorHandler = StubErrorHandler;
	InstancePtr->SgRecvHandler = StubSgHandler;
	InstancePtr->SgSendHandler = StubSgHandler;

	/* Enable by default the SGEND interrupt for SG DMA */
	InstancePtr->IsSgEndDisable = FALSE;

	/* Initialize the device register base addresses */
	InstancePtr->BaseAddress = ConfigPtr->BaseAddress;
	InstancePtr->PhysAddress = ConfigPtr->PhysAddress;

	/* Configure the send and receive FIFOs in the MAC */
	Result = ConfigureFifo(InstancePtr);
	if (Result != XST_SUCCESS) {
		return Result;
	}

	/* If configured for DMA, configure the Rx and Tx DMA channels */
	if (XGemac_mIsDma(InstancePtr)) {
		Result = ConfigureDma(InstancePtr);
		if (Result != XST_SUCCESS) {
			return Result;
		}
	}

	/*
	 * Indicate the component is now ready to use. Note that this is done before
	 * we reset the device and the PHY below, which may seem a bit odd. The
	 * choice was made to move it here rather than remove the asserts in various
	 * functions (e.g., Reset() and all functions that it calls).  Applications
	 * that use multiple threads, one to initialize the XGemac driver and one
	 * waiting on the IsReady condition could have a problem with this sequence.
	 */
	InstancePtr->IsReady = XCOMPONENT_IS_READY;

	/*
	 * Reset the MAC to get it into its initial state. It is expected that
	 * device configuration by the user will take place after this
	 * initialization is done, but before the device is started.
	 */
	XGemac_Reset(InstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Start the Ethernet controller as follows:
*   - Disable global device interrupt
*   - If polled mode, then enable transmitter/receiver and return
*   - If SG DMA interrupt driven mode
*       - Start the send and receive DMA channels
*       - Set DMA specific interrupt enable registers appropriately
*   - If FIFO interrupt driven mode
*       - Set FIFO specific interrupt enable registers appropriately
*   - Enable transmitter/receiver and return
*
* The PHY is enabled after driver initialization. We assume the upper layer
* software has configured it and the GEMAC appropriately before this function
* is called.
*
* @param InstancePtr is a pointer to the XGemac instance to be worked on.
*
* @return
*
* - XST_SUCCESS if the device was started successfully
* - XST_NO_CALLBACK if a callback function has not yet been registered using
*   the SetxxxHandler function. This is required if in interrupt mode.
* - XST_DEVICE_IS_STARTED if the device is already started
* - XST_DMA_SG_NO_LIST if configured for scatter-gather DMA and a descriptor
*   list has not yet been created for the send or receive channel.
* - XST_DMA_SG_LIST_EMPTY if configured for scatter-gather DMA but no
*   receive buffer descriptors have been initialized.
*
* @note
*
* The driver tries to match the hardware configuration. So if the hardware
* is configured with scatter-gather DMA, the driver expects to start the
* scatter-gather channels and expects that the user has previously set up 
* the buffer descriptor lists. If the user expects to use the driver in a mode
* different than how the hardware is configured, the user should modify the
* configuration table to reflect the mode to be used. 
*
* This function makes use of internal resources that are shared between the
* Start, Stop, and SetOptions functions. So if one task might be setting device
* options while another is trying to start the device, the user is required to
* provide protection of this shared data (typically using a semaphore).
*
******************************************************************************/
XStatus
XGemac_Start(XGemac * InstancePtr)
{
	u32 ControlReg;
	u32 DmaControlReg;
	XStatus Result;

	/* Assert bad arguments and conditions */
	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/* If it is already started, return a status indicating so */
	if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
		return XST_DEVICE_IS_STARTED;
	}

	/* Just to be sure, disable all interrupts for this device */
	XIIF_V123B_GINTR_DISABLE(InstancePtr->BaseAddress);

	/*
	 * Prepare the control register to enable the transmitter and receiver.
	 * Defer writing the register until the proper time.
	 */
	ControlReg = XIo_In32(InstancePtr->BaseAddress + XGE_ECR_OFFSET);
	ControlReg &= ~(XGE_ECR_XMIT_RESET_MASK | XGE_ECR_RECV_RESET_MASK);
	ControlReg |= (XGE_ECR_XMIT_ENABLE_MASK | XGE_ECR_RECV_ENABLE_MASK);

	/* 1st Case = Polled mode */
	if (InstancePtr->IsPolled) {
		/* Enable Tx/Rx, mark as started, and return */
		XIo_Out32(InstancePtr->BaseAddress + XGE_ECR_OFFSET,
			  ControlReg);
		InstancePtr->IsStarted = XCOMPONENT_IS_STARTED;
		return XST_SUCCESS;
	}

	/* 2nd Case = SG DMA Interrupt mode */
	if (XGemac_mIsSgDma(InstancePtr)) {
		/*
		 * Verify that the callbacks have been registered, then enable
		 * interrupts
		 */
		if ((InstancePtr->SgRecvHandler == StubSgHandler) ||
		    (InstancePtr->SgSendHandler == StubSgHandler)) {
			return XST_NO_CALLBACK;
		}

		/*
		 * Start the send and receive channels.
		 * The only error we care about is if the list has not yet been
		 * created, or on receive, if no buffer descriptors have been
		 * added yet (the list is empty). Other errors are benign at this point.
		 */
		Result = XDmaChannel_SgStart(&InstancePtr->RecvChannel);
		if ((Result == XST_DMA_SG_NO_LIST)
		    || (Result == XST_DMA_SG_LIST_EMPTY)) {
			return Result;
		}

		Result = XDmaChannel_SgStart(&InstancePtr->SendChannel);
		if (Result == XST_DMA_SG_NO_LIST) {
			return Result;
		}

		/* Enable scatter-gather DMA interrupts */
		DmaControlReg = XGE_DMA_SG_INTR_MASK;	/* Default mask */
		if (InstancePtr->IsSgEndDisable) {
			DmaControlReg &= ~XDC_IXR_SG_END_MASK;	/* Don't enable SGEND */
		}

		XDmaChannel_SetIntrEnable(&InstancePtr->RecvChannel,
					  DmaControlReg);
		XDmaChannel_SetIntrEnable(&InstancePtr->SendChannel,
					  DmaControlReg);

		/* Enable IPIF interrupts */
		XIIF_V123B_WRITE_DIER(InstancePtr->BaseAddress,
				      XGE_IPIF_DMA_DFT_MASK |
				      XIIF_V123B_ERROR_MASK);
		XIIF_V123B_WRITE_IIER(InstancePtr->BaseAddress,
				      XGE_EIR_DFT_SG_MASK);
	}

	/* 3rd Case FIFO Interrupt mode */
	else {
		/* Verify handlers have been registered */
		if ((InstancePtr->FifoRecvHandler == StubFifoHandler) ||
		    (InstancePtr->FifoSendHandler == StubFifoHandler)) {
			return XST_NO_CALLBACK;
		}

		/* Enable IPIF interrupts */
		XIIF_V123B_WRITE_DIER(InstancePtr->BaseAddress,
				      XGE_IPIF_FIFO_DFT_MASK |
				      XIIF_V123B_ERROR_MASK);
		XIIF_V123B_WRITE_IIER(InstancePtr->BaseAddress,
				      XGE_EIR_DFT_FIFO_MASK);
	}

	/*
	 * If control reaches this point, then we have gone through all the
	 * interrupt driven startup sequences. Finish things off by enabling
	 * the transmitter and receiver, mark instance as started, enable device
	 * interrupts, and return
	 */
	XIo_Out32(InstancePtr->BaseAddress + XGE_ECR_OFFSET, ControlReg);
	InstancePtr->IsStarted = XCOMPONENT_IS_STARTED;
	XIIF_V123B_GINTR_ENABLE(InstancePtr->BaseAddress);
	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* Stop the Ethernet MAC as follows:
*   - Disable all interrupts from this device
*   - If the device is configured with scatter-gather DMA, stop the DMA
*     channels (wait for acknowledgment of stop)
*   - Disable the transmitter and receiver
*
* Device options currently in effect are not changed.
*
* @param InstancePtr is a pointer to the XGemac instance to be worked on.
*
* @return
*
* - XST_SUCCESS if the device was stopped successfully
* - XST_DEVICE_IS_STOPPED if the device is already stopped
*
* @note
*
* This function makes use of internal resources that are shared between the
* Start, Stop, and SetOptions functions. So if one task might be setting device
* options while another is trying to start the device, the user is required to
* provide protection of this shared data (typically using a semaphore).
*
******************************************************************************/
XStatus
XGemac_Stop(XGemac * InstancePtr)
{
	u32 ControlReg;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * If the device is already stopped, do nothing but return a status
	 * indicating so
	 */
	if (InstancePtr->IsStarted == 0) {
		return XST_DEVICE_IS_STOPPED;
	}

	/* Disable all interrupts for the device */
	XIIF_V123B_GINTR_DISABLE(InstancePtr->BaseAddress);

	/*
	 * If configured for scatter-gather DMA, stop the DMA channels.
	 * Scatter-gather utilizes a graceful stop so we must keep the transmitter
	 * and receiver enabled until the current packet has been processed.
	 */
	if (XGemac_mIsSgDma(InstancePtr)) {
		XBufDescriptor *BdTemp;	/* temporary descriptor pointer */

		(void) XDmaChannel_SgStop(&InstancePtr->SendChannel, &BdTemp);
		(void) XDmaChannel_SgStop(&InstancePtr->RecvChannel, &BdTemp);
	}

	/* Disable the transmitter and receiver */
	ControlReg = XIo_In32(InstancePtr->BaseAddress + XGE_ECR_OFFSET);
	ControlReg &= ~(XGE_ECR_XMIT_ENABLE_MASK | XGE_ECR_RECV_ENABLE_MASK);
	XIo_Out32(InstancePtr->BaseAddress + XGE_ECR_OFFSET, ControlReg);

	/* Note as stopped */
	InstancePtr->IsStarted = 0;
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Reset the Ethernet MAC. This is a graceful reset in that the device is stopped
* first. Resets the DMA channels, the FIFOs, the transmitter, and the receiver.
* All options are placed in their default state. Any frames in the scatter-
* gather descriptor lists will remain in the lists. The side effect of doing
* this is that after a reset and following a restart of the device, frames that
* were in the list before the reset may be transmitted or received.
*
* The upper layer software is responsible for re-configuring (if necessary)
* and restarting the MAC after the reset. Note also that driver statistics
* are not cleared on reset. It is up to the upper layer software to clear the
* statistics if needed.
*
* When a reset is required due to an internal error, the driver notifies the
* upper layer software of this need through the ErrorHandler callback and
* specific status codes.  The upper layer software is responsible for calling
* this Reset function and then re-configuring the device.
*
* @param InstancePtr is a pointer to the XGemac instance to be worked on.
*
* @return
*
* None.
*
* @note
*
* None.
*
* @internal
*
* The reset is accomplished by setting the IPIF reset register.  This takes
* care of resetting all hardware blocks, including the MAC.
*
******************************************************************************/
void
XGemac_Reset(XGemac * InstancePtr)
{
	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * Stop the device first
	 */
	(void) XGemac_Stop(InstancePtr);

	/*
	 * Reset the entire IPIF at once.  If we choose someday to reset each
	 * hardware block separately, the reset should occur in the direction of
	 * data flow. For example, for the send direction the reset order is DMA
	 * first, then FIFO, then the MAC transmitter.
	 */
	XIIF_V123B_RESET(InstancePtr->BaseAddress);

	if (XGemac_mIsSgDma(InstancePtr)) {
		/*
		 * After reset, configure the scatter-gather DMA packet threshold and
		 * packet wait bound registers to default values. Ignore the return
		 * values of these functions since they only return error if the device
		 * is not stopped.
		 */
		(void) XGemac_SetPktThreshold(InstancePtr, XGE_SEND,
					      XGE_SGDMA_DFT_THRESHOLD);
		(void) XGemac_SetPktThreshold(InstancePtr, XGE_RECV,
					      XGE_SGDMA_DFT_THRESHOLD);
		(void) XGemac_SetPktWaitBound(InstancePtr, XGE_SEND,
					      XGE_SGDMA_DFT_WAITBOUND);
		(void) XGemac_SetPktWaitBound(InstancePtr, XGE_RECV,
					      XGE_SGDMA_DFT_WAITBOUND);
	}
}

/*****************************************************************************/
/**
*
* Set the MAC address for this driver/device.  The address is a 48-bit value.
* The device must be stopped before calling this function.
*
* @param InstancePtr is a pointer to the XGemac instance to be worked on.
* @param AddressPtr is a pointer to a 6-byte MAC address.
*
* @return
*
* - XST_SUCCESS if the MAC address was set successfully
* - XST_DEVICE_IS_STARTED if the device has not yet been stopped
*
* @note
*
* None.
*
******************************************************************************/
XStatus
XGemac_SetMacAddress(XGemac * InstancePtr, u8 * AddressPtr)
{
	u32 MacAddr = 0;

	XASSERT_NONVOID(InstancePtr != NULL);
	XASSERT_NONVOID(AddressPtr != NULL);
	XASSERT_NONVOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	/*
	 * The device must be stopped before setting the MAC address
	 */
	if (InstancePtr->IsStarted == XCOMPONENT_IS_STARTED) {
		return XST_DEVICE_IS_STARTED;
	}

	/*
	 * Set the device station address high and low registers
	 */
	MacAddr = (AddressPtr[0] << 8) | AddressPtr[1];
	XIo_Out32(InstancePtr->BaseAddress + XGE_SAH_OFFSET, MacAddr);

	MacAddr = (AddressPtr[2] << 24) | (AddressPtr[3] << 16) |
		(AddressPtr[4] << 8) | AddressPtr[5];

	XIo_Out32(InstancePtr->BaseAddress + XGE_SAL_OFFSET, MacAddr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Get the MAC address for this driver/device.
*
* @param InstancePtr is a pointer to the XGemac instance to be worked on.
* @param BufferPtr is an output parameter, and is a pointer to a buffer into
*        which the current MAC address will be copied. The buffer must be at
*        least 6 bytes.
*
* @return
*
* None.
*
* @note
*
* None.
*
******************************************************************************/
void
XGemac_GetMacAddress(XGemac * InstancePtr, u8 * BufferPtr)
{
	u32 MacAddrHi;
	u32 MacAddrLo;

	XASSERT_VOID(InstancePtr != NULL);
	XASSERT_VOID(BufferPtr != NULL);
	XASSERT_VOID(InstancePtr->IsReady == XCOMPONENT_IS_READY);

	MacAddrHi = XIo_In32(InstancePtr->BaseAddress + XGE_SAH_OFFSET);
	MacAddrLo = XIo_In32(InstancePtr->BaseAddress + XGE_SAL_OFFSET);

	BufferPtr[0] = (u8) (MacAddrHi >> 8);
	BufferPtr[1] = (u8) MacAddrHi;
	BufferPtr[2] = (u8) (MacAddrLo >> 24);
	BufferPtr[3] = (u8) (MacAddrLo >> 16);
	BufferPtr[4] = (u8) (MacAddrLo >> 8);
	BufferPtr[5] = (u8) MacAddrLo;
}

/******************************************************************************/
/**
*
* Configure DMA capabilities.
*
* @param InstancePtr is a pointer to the XGemac instance to be worked on.
*
* @return
*
* - XST_SUCCESS  if successful initialization of DMA
*
* @note
*
* None.
*
******************************************************************************/
static XStatus
ConfigureDma(XGemac * InstancePtr)
{
	XStatus Result;

	/*
	 * Initialize the DMA channels with their base addresses. We assume
	 * scatter-gather DMA is the only possible configuration. Descriptor space
	 * will need to be set later by the upper layer.
	 */
	Result = XDmaChannel_Initialize(&InstancePtr->RecvChannel,
					InstancePtr->BaseAddress +
					XGE_DMA_RECV_OFFSET);
	if (Result != XST_SUCCESS) {
		return Result;
	}

	Result = XDmaChannel_Initialize(&InstancePtr->SendChannel,
					InstancePtr->BaseAddress +
					XGE_DMA_SEND_OFFSET);

	return Result;
}

/******************************************************************************/
/**
*
* Configure the send and receive FIFO components with their base addresses
* and interrupt masks.  Currently the base addresses are defined constants.
*
* @param InstancePtr is a pointer to the XGemac instance to be worked on.
*
* @return
*
* XST_SUCCESS if successful initialization of the packet FIFOs
*
* @note
*
* None.
*
******************************************************************************/
static XStatus
ConfigureFifo(XGemac * InstancePtr)
{
	XStatus Result;

	/*
	 * Return status from the packet FIFOs initialization is ignored since
	 * they always return success.
	 */
	Result = XPacketFifoV200a_Initialize(&InstancePtr->RecvFifo,
					     InstancePtr->BaseAddress +
					     XGE_PFIFO_RXREG_OFFSET,
					     InstancePtr->BaseAddress +
					     XGE_PFIFO_RXDATA_OFFSET);
	if (Result != XST_SUCCESS) {
		return Result;
	}

	Result = XPacketFifoV200a_Initialize(&InstancePtr->SendFifo,
					     InstancePtr->BaseAddress +
					     XGE_PFIFO_TXREG_OFFSET,
					     InstancePtr->BaseAddress +
					     XGE_PFIFO_TXDATA_OFFSET);
	return Result;
}

/******************************************************************************/
/**
*
* This is a stub for the scatter-gather send and recv callbacks. The stub
* is here in case the upper layers forget to set the handlers.
*
* @param CallBackRef is a pointer to the upper layer callback reference
* @param BdPtr is a pointer to the first buffer descriptor in a list
* @param NumBds is the number of descriptors in the list.
*
* @return
*
* None.
*
* @note
*
* None.
*
******************************************************************************/
static void
StubSgHandler(void * CallBackRef, XBufDescriptor * BdPtr, u32 NumBds)
{
	XASSERT_VOID_ALWAYS();
}

/******************************************************************************/
/**
*
* This is a stub for the non-DMA send and recv callbacks.  The stub is here in
* case the upper layers forget to set the handlers.
*
* @param CallBackRef is a pointer to the upper layer callback reference
*
* @return
*
* None.
*
* @note
*
* None.
*
******************************************************************************/
static void
StubFifoHandler(void *CallBackRef)
{
	XASSERT_VOID_ALWAYS();
}

/******************************************************************************/
/**
*
* This is a stub for the asynchronous error callback.  The stub is here in
* case the upper layers forget to set the handler.
*
* @param CallBackRef is a pointer to the upper layer callback reference
* @param ErrorCode is the Xilinx error code, indicating the cause of the error
*
* @return
*
* None.
*
* @note
*
* None.
*
******************************************************************************/
static void
StubErrorHandler(void *CallBackRef, XStatus ErrorCode)
{
	XASSERT_VOID_ALWAYS();
}

/*****************************************************************************/
/**
*
* Lookup the device configuration based on the unique device ID.  The table
* EmacConfigTable contains the configuration info for each device in the system.
*
* @param DeviceId is the unique device ID of the device being looked up.
*
* @return
*
* A pointer to the configuration table entry corresponding to the given
* device ID, or NULL if no match is found.
*
* @note
*
* None.
*
******************************************************************************/
XGemac_Config *
XGemac_LookupConfig(u16 DeviceId)
{
	XGemac_Config *CfgPtr = NULL;
	int i;

	for (i = 0; i < XPAR_XGEMAC_NUM_INSTANCES; i++) {
		if (XGemac_ConfigTable[i].DeviceId == DeviceId) {
			CfgPtr = &XGemac_ConfigTable[i];
			break;
		}
	}

	return CfgPtr;
}

