
/*******************************************************************
*
* Version: Xilinx EDK 6.2 EDK_Gm.8
*
* Copyright (c) 2003 Xilinx, Inc.  All rights reserved. 
* 
* Description: Driver configuration
*
*******************************************************************/

#include "xparameters.h"
#include "xgemac.h"

/*
* The configuration table for devices
*/

XGemac_Config XGemac_ConfigTable[] =
{
	{
		XPAR_GEMAC_0_DEVICE_ID,
		XPAR_GEMAC_0_BASEADDR,
		XPAR_GEMAC_0_BASEADDR,
		XPAR_GEMAC_0_DMA_TYPE,
		XPAR_GEMAC_0_INCLUDE_GMII,
		XPAR_GEMAC_0_INCLUDE_STATS
	}
};
