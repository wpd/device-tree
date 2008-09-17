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
 *****************************************************************************/
/*****************************************************************************/
/**
 * @file xusb_cp9.c
 * 
 * This file contains the USB Chapter 9 related functions.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -----------------------------------------------------------------
 * 1.00a hvm  2/22/07 First release
 *
 * </pre>
 *****************************************************************************/
/***************************** Include Files *********************************/

#include "xusb_cp9.h"

/************************** Constant Definitions ******************************/

/************************** Variable Definitions ******************************/

/* 
 * Structure 32 bit int memory access to the Dual Port RAM. 
 */
union {
	u32 Word;
	struct {
		u8 Zero;
		u8 One;
		u8 Two;
		u8 Three;
	} Byte;
} UsbMemData;


/*
 * Instance of the Chapter 9 command buffer.
 */
USB_CMD_BUF Ch9_CmdBuf;

/*
 * Maximum control pkt size.
 */
u16 MaxControlSize = 8;


/************************** Function Prototypes *******************************/

/******************************************************************************/
/**
* This routine is called when an OUT transaction for Endpoint Zero is received.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	None.
*
* @note		None.
*
******************************************************************************/
void EP0ProcessOutToken(XUsb * InstancePtr)
{
	u8 Count;
	u8 *RamBase;
	u16 Index;

	switch (Ch9_CmdBuf.SetupSeqRX) {
	case STATUS_PHASE:
		/*
		 * This resets both state machines for the next
		 * Setup packet.
		 */
		Ch9_CmdBuf.SetupSeqRX = SETUP_PHASE;
		Ch9_CmdBuf.SetupSeqTX = SETUP_PHASE;
		break;

	case DATA_PHASE:

		Count = XUsb_mReadReg(InstancePtr->Config.BaseAddress,
				      XUSB_EP_BUF0COUNT_OFFSET);
		/*
		 * Copy the data to be received from the DPRAM.
		 */
		RamBase = (u8 *) (InstancePtr->Config.BaseAddress +
				  ((InstancePtr->DeviceConfig.Ep[0].
				    RamBase) << 2));

		for (Index = 0; Index < Count; Index++) {

			*Ch9_CmdBuf.ContReadPtr++ = *RamBase++;
		}

		Ch9_CmdBuf.ContReadCount += Count;

		/*
		 * Set the Tx packet size and the Tx enable bit.
		 */
		XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
			       XUSB_EP_BUF0COUNT_OFFSET, Count);

		XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
			       XUSB_BUFFREADY_OFFSET, 1);

		InstancePtr->DeviceConfig.Ep[0].Buffer0Ready = 1;

		if (Ch9_CmdBuf.Word3.wLength == Ch9_CmdBuf.ContReadCount) {
			ExecuteCommand(InstancePtr);
		}
		break;

	default:
		break;
	}
}

/******************************************************************************/
/**
* This routine is called when IN transaction for Endpoint Zero is received.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	None.
*
* @note     	None.
*
******************************************************************************/
void EP0ProcessInToken(XUsb * InstancePtr)
{
	u32 EpCfgReg;

	switch (Ch9_CmdBuf.SetupSeqTX) {
	case STATUS_PHASE:
		if (Ch9_CmdBuf.Byte1.bRequest == SET_ADDRESS) {
			/*
			 * Set the address of the device.
			 */
			XUsb_SetDeviceAddress(InstancePtr,
					      Ch9_CmdBuf.Word1.Byte23.
					      bDescriptorIndex);
		}
		else if (Ch9_CmdBuf.Byte1.bRequest == SET_FEATURE) {
			if (Ch9_CmdBuf.Byte0.bmRequestType ==
			    STANDARD_OUT_DEVICE) {
				if (Ch9_CmdBuf.Word1.wValue == TEST_MODE) {
					XUsb_SetTestMode(InstancePtr, TEST_J,
							 NULL);
				}
			}
		}
		break;

	case DATA_PHASE:
		if (Ch9_CmdBuf.ContWriteCount == 0) {
			/*
			 * We're done with data transfer, next
			 * will be zero length OUT with data toggle of
			 * 1. Setup data_toggle.
			 */
			EpCfgReg = XUsb_mReadReg(InstancePtr->Config.
						 BaseAddress,
						 InstancePtr->
						 EndPointOffset[0]);
			EpCfgReg |= XUSB_EP_CFG_DATA_TOGGLE_MASK;
			XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
				       InstancePtr->EndPointOffset[0],
				       EpCfgReg);
			XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
				       (InstancePtr->EndPointOffset[0] +
					XUSB_EP_BUF0COUNT_OFFSET), 0);
			XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
				       XUSB_BUFFREADY_OFFSET, 1);
			InstancePtr->DeviceConfig.Ep[0].Buffer0Ready = 1;

			Ch9_CmdBuf.SetupSeqTX = STATUS_PHASE;

		}
		else {
			LoadEP0(InstancePtr);
		}
		break;

	default:
		break;
	}

}

/******************************************************************************/
/**
* This routine is called when a chapter 9 command is received.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	The setup request value to be processed by the upper layer.
*
* @note     	None.
*
******************************************************************************/
int Chapter9(XUsb *InstancePtr, UsbCtrlRequest *Ctrl) 
{
	volatile unsigned int *RamBase;

	/*
	 * Load up the chapter 9 command buffer.
	 */
	RamBase = (unsigned int *) (InstancePtr->Config.BaseAddress +
				    XUSB_SETUP_PKT_ADDR_OFFSET);

	/*
	 * Get the first 4 bytes of the setup packet.
	 */
	UsbMemData.Word = *RamBase;

	Ch9_CmdBuf.Byte0.bmRequestType = UsbMemData.Byte.Zero;
	Ch9_CmdBuf.Byte1.bRequest = UsbMemData.Byte.One;
	Ch9_CmdBuf.Word1.Byte23.bDescriptorIndex = UsbMemData.Byte.Two;
	Ch9_CmdBuf.Word1.Byte23.bDescriptorType = UsbMemData.Byte.Three;

	/*
	 * Get the last 4 bytes of the setup packet.
	 */
	RamBase += 1;
	UsbMemData.Word = *RamBase;

	/*
	 * Byte swapping for next 4 bytes for BE machines is defined in
	 * the different layout of BECB verses LECB.
	 */
	Ch9_CmdBuf.Word2.Byte45.Bytel = UsbMemData.Byte.Zero;
	Ch9_CmdBuf.Word2.Byte45.Byteh = UsbMemData.Byte.One;
	Ch9_CmdBuf.Word3.Byte67.Bytel = UsbMemData.Byte.Two;
	Ch9_CmdBuf.Word3.Byte67.Byteh = UsbMemData.Byte.Three;
	Ctrl->bRequestType = Ch9_CmdBuf.Byte0.bmRequestType;
	Ctrl->bRequest = Ch9_CmdBuf.Byte1.bRequest;
	Ctrl->wValue = le16_to_cpu(Ch9_CmdBuf.Word1.wValue);
	Ctrl->wIndex = le16_to_cpu(Ch9_CmdBuf.Word2.wIndex);
	Ctrl->wLength = le16_to_cpu(Ch9_CmdBuf.Word3.wLength);
	
	/*
	 * Restore ReadPtr to data buffer.
	 */
	Ch9_CmdBuf.ContReadPtr = &Ch9_CmdBuf.ContReadDataBuffer[0];

	if (Ch9_CmdBuf.Byte0.bmRequestType & DIR_DEVICE_TO_HOST) {
		/*
		 * Execute the get command.
		 */
		Ch9_CmdBuf.SetupSeqRX = STATUS_PHASE;
		Ch9_CmdBuf.SetupSeqTX = DATA_PHASE;
		return (ExecuteCommand(InstancePtr));
	}
	else {
		/*
		 * Execute the put command.
		 */
		Ch9_CmdBuf.SetupSeqRX = DATA_PHASE;
		Ch9_CmdBuf.SetupSeqTX = STATUS_PHASE;
		if (Ch9_CmdBuf.Word3.wLength == 0) {
			return (ExecuteCommand(InstancePtr));
		}
	}
	/*
 	 * Control should never come here.
	 */
	return 0;
}

/******************************************************************************/
/**
* This function executes the chapter 9 command processing
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	The request value to be processed by the upper layer if the 
*		request is not handled.
*		XST_SUCCESS if the request is successfully processed.
*
* @note     	None.
*
******************************************************************************/
int ExecuteCommand(XUsb * InstancePtr)
{

	if ((Ch9_CmdBuf.Byte0.bmRequestType & TYPE_MASK) == TYPE_STANDARD) {
		/*
		 * Process the chapter 9 command.
		 */
		switch (Ch9_CmdBuf.Byte1.bRequest) {

		case CLEAR_FEATURE:
			SetClearFeature(InstancePtr, FALSE);
			break;

		case SET_FEATURE:
			SetClearFeature(InstancePtr, TRUE);
			break;

		case SET_ADDRESS:
			SetupControlWriteStatusStage(InstancePtr);
			break;

		case SET_CONFIGURATION:
			SetConfiguration(InstancePtr);
			return (Ch9_CmdBuf.Byte1.bRequest);

		default:
			/*
			 * The default is to stall the end point zero.
			 */
			return (Ch9_CmdBuf.Byte1.bRequest);
		}

	} else if ((Ch9_CmdBuf.Byte0.bmRequestType & TYPE_MASK) == TYPE_CLASS) {
			return (Ch9_CmdBuf.Byte1.bRequest);
	}

	return XST_SUCCESS;
}

/******************************************************************************
		Start of Chapter 9 Commands
******************************************************************************/

/*****************************************************************************/
/**
* This function responds to the GET_INTERFACE command.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	None.
*
* @note     	None.
*
******************************************************************************/
void GetInterface(XUsb * InstancePtr)
{
	u32 *RamBase;

	if (Ch9_CmdBuf.Word1.wValue == 0) {
		RamBase = (u32 *) (InstancePtr->Config.BaseAddress +
				   ((InstancePtr->DeviceConfig.Ep[0].
				     RamBase) << 2));
		UsbMemData.Word = 0x0;
		*RamBase = UsbMemData.Word;
		InstancePtr->DeviceConfig.Ep[0].Buffer0Ready = 1;

		XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
			       (InstancePtr->EndPointOffset[0] +
				XUSB_EP_BUF0COUNT_OFFSET), 1);
		XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
			       XUSB_BUFFREADY_OFFSET, 1);

	}
	else {
		XUsb_EpStall(InstancePtr, 0);
	}

}

/******************************************************************************/
/**
* This function responds to the SET_INTERFACE command.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	None.
*
* @note     	None.
*
******************************************************************************/
void SetInterface(XUsb * InstancePtr)
{
	if ((Ch9_CmdBuf.Word1.wValue == 0) && (Ch9_CmdBuf.Word2.wIndex == 0)) {
		SetupControlWriteStatusStage(InstancePtr);
	}
	else {
		XUsb_EpStall(InstancePtr, 0);
	}

}

/******************************************************************************/
/**
 * This function sets the basic Control status words.
 *
 * @param	InstancePtr is a pointer to the XUsb instance of the controller.
 *
 * @return 	None.
 *
 * @note     	None.
 *
 ******************************************************************************/
void SetupControlWriteStatusStage(XUsb * InstancePtr)
{
	u32 EpCfgReg;

	EpCfgReg = XUsb_mReadReg(InstancePtr->Config.BaseAddress,
				 InstancePtr->EndPointOffset[0]);
	EpCfgReg |= XUSB_EP_CFG_DATA_TOGGLE_MASK;
	XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
		       InstancePtr->EndPointOffset[0], EpCfgReg);
	InstancePtr->DeviceConfig.Ep[0].Buffer0Count = 0;
	XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
		       (InstancePtr->EndPointOffset[0] +
			XUSB_EP_BUF0COUNT_OFFSET), 0);
	XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
		       XUSB_BUFFREADY_OFFSET, 1);
}

/******************************************************************************/
/**
* This routine is called when a GET_STATUS command is received.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	None.
*
* @note     	None.
*
******************************************************************************/
void GetStatus(XUsb * InstancePtr)
{
	u32 *RamBase;
	u8 EndPoint;

	UsbMemData.Word = 0x0;
	RamBase = (u32 *) (InstancePtr->Config.BaseAddress +
			   ((InstancePtr->DeviceConfig.Ep[0].RamBase) << 2));

	switch (Ch9_CmdBuf.Byte0.bmRequestType) {
	case STANDARD_IN_DEVICE:
		UsbMemData.Byte.Zero = 0x01;
		UsbMemData.Byte.One = 0x00;

		*RamBase = UsbMemData.Word;

		InstancePtr->DeviceConfig.Ep[0].Buffer0Ready = 1;
		XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
			       (InstancePtr->EndPointOffset[0] +
				XUSB_EP_BUF0COUNT_OFFSET), 2);
		XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
			       XUSB_BUFFREADY_OFFSET, 1);
		break;

	case STANDARD_IN_INTERFACE:
		if ((Ch9_CmdBuf.Word2.wIndex > 0) &&
		    (Ch9_CmdBuf.Word2.wIndex <= MAX_INTERFACES)) {
			UsbMemData.Byte.Zero = 0;
			UsbMemData.Byte.One = 0;
			*RamBase = UsbMemData.Word;
			InstancePtr->DeviceConfig.Ep[0].Buffer0Ready = 1;
			XUsb_mWriteReg(InstancePtr->Config.
				       BaseAddress, (InstancePtr->EndPointOffset
						     [0] +
						     XUSB_EP_BUF0COUNT_OFFSET),
				       2);
			XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
				       XUSB_BUFFREADY_OFFSET, 1);
		}
		else {
			XUsb_EpStall(InstancePtr, 0);
		}
		break;

	case STANDARD_IN_ENDPOINT:
		EndPoint = (unsigned int) (Ch9_CmdBuf.Word2.wIndex & 0x07);
		UsbMemData.Byte.Zero =
			(u8) (((XUsb_mReadReg(InstancePtr->Config.BaseAddress,
					      InstancePtr->
					      EndPointOffset[EndPoint]))
			       & XUSB_EP_CFG_STALL_MASK) >>
			      XUSB_EP_CFG_STALL_SHIFT);
		UsbMemData.Byte.One = 0;

		*RamBase = UsbMemData.Word;

		InstancePtr->DeviceConfig.Ep[0].Buffer0Ready = 1;
		XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
			       (InstancePtr->EndPointOffset[0] +
				XUSB_EP_BUF0COUNT_OFFSET), 2);
		XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
			       XUSB_BUFFREADY_OFFSET, 1);
		break;

	default:
		/*
		 * The default is to stall the end point.
		 */
		XUsb_EpStall(InstancePtr, 0);
		break;
	}
}

/******************************************************************************/
/**
* This routine is called when a SET_DESCRIPTOR command is received.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	None.
*
* @note     	None.
*
******************************************************************************/
void SetDescriptor(XUsb * InstancePtr)
{
	/*
	 * Command not supported.
	 */
	XUsb_EpStall(InstancePtr, 0);
}

/******************************************************************************/
/**
* This routine is called when a GET_CONFIGURATION command is received.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	None.
*
* @note     	None.
*
******************************************************************************/
void GetConfiguration(XUsb * InstancePtr)
{
	u32 *RamBase;

	UsbMemData.Word = 0x00;
	RamBase = (u32 *) (InstancePtr->Config.BaseAddress +
			   ((InstancePtr->DeviceConfig.Ep[0].RamBase) << 2));
	UsbMemData.Byte.Zero = InstancePtr->DeviceConfig.CurrentConfiguration;
	UsbMemData.Byte.One = 0;
	UsbMemData.Byte.Two = 0;
	UsbMemData.Byte.Three = 0;

	*RamBase = UsbMemData.Word;

	InstancePtr->DeviceConfig.Ep[0].Buffer0Ready = 1;
	XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
		       (InstancePtr->EndPointOffset[0] +
			XUSB_EP_BUF0COUNT_OFFSET), 1);
	XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
		       XUSB_BUFFREADY_OFFSET, 1);
	Ch9_CmdBuf.ContWriteCount = 0;

}

/******************************************************************************/
/**
* This routine is called when a SET_CONFIGURATION command is received.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	None.
*
* @note     	None.
*
******************************************************************************/
void SetConfiguration(XUsb * InstancePtr)
{
	u8 Index;
	switch (Ch9_CmdBuf.Word1.wValue) {
	case 0:
		/*
		 * This configuration value resets the device to the
		 * un configured state like power up.
		 */
		InstancePtr->DeviceConfig.CurrentConfiguration = 0;
		/*
		 * Cause a valid status phase to be issued.
		 */
		SetupControlWriteStatusStage(InstancePtr);
		break;

	case CONFIGURATION_ONE:
		InstancePtr->DeviceConfig.CurrentConfiguration = 1;
		SetupControlWriteStatusStage(InstancePtr);		
		break;

		/*
		 * Additional configurations can be added here.
		 */
	default:
		/*
		 * stall the end point.
		 */
		XUsb_EpStall(InstancePtr, 0);
		break;
	}
}

/******************************************************************************/
/**
* This routine is called when a SET_FEATURE or a CLEAR_FEATURE command is
* received.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
* @param	flag specifies whether it is a SET_FEATURE (TRUE)or a
*		CLEAR_FEATURE (FALSE) command.
*
* @return	None.
*
* @note 	None.
*
******************************************************************************/
void SetClearFeature(XUsb * InstancePtr, int flag)
{
	u8 EndPoint;
	u8 OutInbit;
	u32 EpCfgReg;

	switch (Ch9_CmdBuf.Byte0.bmRequestType) {
	case STANDARD_OUT_DEVICE:
		switch (Ch9_CmdBuf.Word1.wValue) {
		case DEVICE_REMOTE_WAKEUP:
			/*
			 * User needs to add code here.
			 */
			break;

		case TEST_MODE:
			/*
			 * The Test Mode will be executed
			 * after the status phase.
			 */
			break;

		default:
			XUsb_EpStall(InstancePtr, 0);
			break;
		}
		break;

	case STANDARD_OUT_ENDPOINT:
		if (Ch9_CmdBuf.Word1.wValue == 0) {
			EndPoint = Ch9_CmdBuf.Word2.wIndex & 0xf;
			OutInbit = Ch9_CmdBuf.Word2.wIndex & 0x80;
			OutInbit = OutInbit >> 7;

			/*
			 * Make sure direction matches.
			 */
			if (OutInbit !=
			    InstancePtr->DeviceConfig.Ep[EndPoint].OutIn) {
				XUsb_EpStall(InstancePtr, 0);
				goto Func_Exit10;
			}

			if (EndPoint == 0) {
				/*
				 * Clear the stall.
				 */
				XUsb_EpUnstall(InstancePtr, 0);

				break;
			}
			else {
				if (flag == TRUE) {
					XUsb_EpStall(InstancePtr, EndPoint);
				}
				else {
					XUsb_EpUnstall(InstancePtr, EndPoint);

					/*
					 * Clear the data toggle.
					 */
					EpCfgReg =
						XUsb_mReadReg(InstancePtr->
							      Config.
							      BaseAddress,
							      InstancePtr->
							      EndPointOffset
							      [EndPoint]);
					EpCfgReg &=
						~XUSB_EP_CFG_DATA_TOGGLE_MASK;
					XUsb_mWriteReg(InstancePtr->Config.
						       BaseAddress,
						       InstancePtr->
						       EndPointOffset[EndPoint],
						       EpCfgReg);
				}
			}
		}
		break;

		/*
		 * Add more here as needed.
		 */
	default:
		XUsb_EpStall(InstancePtr, 0);
		goto Func_Exit10;
		break;
	}

	/*
	 * Cause and valid status phase to be issued.
	 */
	SetupControlWriteStatusStage(InstancePtr);

      Func_Exit10:
	return;
}

/******************************************************************************/
/**
* This routine copies the EP0 related data to the DPRAM.
*
* @param	InstancePtr is a pointer to the XUsb instance of the controller.
*
* @return 	None.
*
* @note		None.
*
******************************************************************************/
void LoadEP0(XUsb * InstancePtr)
{
	u16 Count;
	u8 *RamBase;
	u16 Index;

	if (MaxControlSize >= Ch9_CmdBuf.ContWriteCount) {
		Count = Ch9_CmdBuf.ContWriteCount;
	}
	else {
		Count = MaxControlSize;
	}

	/*
	 * Copy the data to be transmitted into the DPRAM.
	 */
	RamBase = (u8 *) (InstancePtr->Config.BaseAddress +
			  ((InstancePtr->DeviceConfig.Ep[0].RamBase) << 2));
	for (Index = 0; Index < Count; Index++) {

		*RamBase++ = *Ch9_CmdBuf.ContWritePtr++;
	}

	/*
	 * Set the Tx packet size and enable the Transmission.
	 */
	XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
		       XUSB_EP_BUF0COUNT_OFFSET, Count);

	XUsb_mWriteReg(InstancePtr->Config.BaseAddress,
		       XUSB_BUFFREADY_OFFSET, 1);

	InstancePtr->DeviceConfig.Ep[0].Buffer0Ready = 1;

	Ch9_CmdBuf.ContWriteCount -= Count;
}
