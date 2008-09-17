/*
 *
 *    Copyright (c) 1999 Grant Erickson <grant@lcse.umn.edu>
 *    Copyright (c) 2006 Wind River Systems, Inc.
 *
 *    Module name: ibm4xx.h
 *
 *    Description:
 *	A generic include file which pulls in appropriate include files
 *      for specific board types based on configuration settings.
 *
 */

#ifdef __KERNEL__
#ifndef __ASM_IBM4XX_H__
#define __ASM_IBM4XX_H__

#include <asm/types.h>
#include <asm/dcr.h>

#if (defined(CONFIG_40x) || defined(CONFIG_44x))
#ifndef __ASSEMBLY__

void ppc4xx_setup_arch(void);
void ppc4xx_map_io(void);
void ppc4xx_init_IRQ(void);
void ppc4xx_init(unsigned long r3, unsigned long r4, unsigned long r5,
		 unsigned long r6, unsigned long r7);
#endif
#endif /* defined(CONFIG_40x) || defined(CONFIG_44x) */

#ifdef CONFIG_40x

#if defined(CONFIG_BUBINGA)
#include <platforms/4xx/bubinga.h>
#endif

#if defined(CONFIG_CPCI405)
#include <platforms/4xx/cpci405.h>
#endif

#if defined(CONFIG_EP405)
#include <platforms/4xx/ep405.h>
#endif

#if defined(CONFIG_REDWOOD_5)
#include <platforms/4xx/redwood5.h>
#endif

#if defined(CONFIG_REDWOOD_6)
#include <platforms/4xx/redwood6.h>
#endif

#if defined(CONFIG_SYCAMORE)
#include <platforms/4xx/sycamore.h>
#endif

#if defined(CONFIG_WALNUT)
#include <platforms/4xx/walnut.h>
#endif

#if defined(CONFIG_WRSBC405GP)
#include <platforms/4xx/wrsbc405gp.h>
#endif

#if (defined(CONFIG_XILINX_VIRTEX_II_PRO) || defined(CONFIG_XILINX_VIRTEX_4_FX))
#include <platforms/4xx/xilinx_mlxxx.h>
#ifndef __ASSEMBLY__
/*
 * The "residual" board information structure the boot loader passes
 * into the kernel.
 */
extern bd_t __res;
#endif
#endif

#ifndef PPC4xx_MACHINE_NAME
#define PPC4xx_MACHINE_NAME	"Unidentified 4xx class"
#endif


/* IO_BASE is for PCI I/O.
 * ISA not supported, just here to resolve copilation.
 */

#ifndef _IO_BASE
#define _IO_BASE	0xe8000000	/* The PCI address window */
#define _ISA_MEM_BASE	0
#define PCI_DRAM_OFFSET	0
#endif

#elif defined(CONFIG_44x)

#if defined(CONFIG_BAMBOO)
#include <platforms/4xx/bamboo.h>
#endif

#if defined(CONFIG_EBONY)
#include <platforms/4xx/ebony.h>
#endif

#if defined(CONFIG_LUAN)
#include <platforms/4xx/luan.h>
#endif

#if defined(CONFIG_YUCCA)
#include <platforms/4xx/yucca.h>
#endif

#if defined(CONFIG_OCOTEA)
#include <platforms/4xx/ocotea.h>
#endif

#if defined(CONFIG_TAISHAN)
#include <platforms/4xx/taishan.h>
#endif

#if defined(CONFIG_YELLOWSTONE) || defined(CONFIG_YOSEMITE)
#include <platforms/4xx/yosemite.h>
#endif

#if defined(CONFIG_XILINX_VIRTEX_5_FX)
#include <platforms/4xx/xilinx_mlxxx.h>

#ifndef __ASSEMBLY__
/*
 * The "residual" board information structure the boot loader passes
 * into the kernel.
 */
extern bd_t __res;
#endif
#endif
#endif /* CONFIG_40x / CONFIG_44x */

#endif /* __ASM_IBM4XX_H__ */
#endif /* __KERNEL__ */
