/*
 * arch/ppc/platforms/4xx/xilinx_mlxxx.h
 *
 * Include file that defines Xilinx ML300, ML403, ML405, ML410 and ML507 evaluation boards
 *
 * Author: MontaVista Software, Inc.
 *         source@mvista.com
 *
 * 2002-2006 (c) MontaVista Software, Inc.  This file is licensed under the
 * terms of the GNU General Public License version 2.  This program is licensed
 * "as is" without any warranty of any kind, whether express or implied.
 */

#ifdef __KERNEL__
#ifndef __ASM_XILINX_MLxxx_H__
#define __ASM_XILINX_MLxxx_H__

/*
 * ML300 has Xilinx Virtex-II Pro processor, ML403/ML405/ML410 have Xilinx Virtex-4 FX
 * processor, ML507 has Virtex-5 processors.
 */

#include <platforms/4xx/virtex.h>

#ifndef __ASSEMBLY__

#include <linux/types.h>

typedef struct board_info {
	unsigned int	 bi_memsize;		/* DRAM installed, in bytes */
	unsigned char	 bi_enetaddr[6];	/* Local Ethernet MAC address */
	unsigned int	 bi_intfreq;		/* Processor speed, in Hz */
	unsigned int	 bi_busfreq;		/* PLB Bus speed, in Hz */
	unsigned int	 bi_pci_busfreq;	/* PCI Bus speed, in Hz */
} bd_t;

/* Some 4xx parts use a different timebase frequency from the internal clock.
*/
#define bi_tbfreq bi_intfreq

#endif /* !__ASSEMBLY__ */

#ifdef CONFIG_PCI
/* PCI memory space */
#define PPC405_PCI_MEM_BASE XPAR_PCI_0_MEM_BASEADDR
#define PPC405_PCI_LOWER_MEM    XPAR_PCI_0_MEM_BASEADDR
#define PPC405_PCI_UPPER_MEM    XPAR_PCI_0_MEM_HIGHADDR
#define PPC405_PCI_PHY_MEM_BASE XPAR_PCI_0_MEM_BASEADDR

/* PCI I/O space parameters for io_block_mapping. */
#define PPC4xx_PCI_IO_PADDR ((uint)XPAR_PCI_0_IO_BASEADDR)
#define PPC4xx_PCI_IO_VADDR PPC4xx_PCI_IO_PADDR
#define PPC4xx_PCI_IO_SIZE  0x10000 /* Hardcoded size from ppc405_pci.c */
/* PCI I/O space processor address */
#define PPC405_PCI_PHY_IO_BASE  XPAR_PCI_0_IO_BASEADDR
/* PCI I/O space PCI address */
#define PPC405_PCI_LOWER_IO 0x00000000
#define PPC405_PCI_UPPER_IO (PPC405_PCI_LOWER_IO + PPC4xx_PCI_IO_SIZE - 1)

/* PCI Configuration space parameters for io_block_mapping. */
#define PPC4xx_PCI_CFG_PADDR    ((uint)XPAR_PCI_0_CONFIG_ADDR)
#define PPC4xx_PCI_CFG_VADDR    PPC4xx_PCI_CFG_PADDR
#define PPC4xx_PCI_CFG_SIZE 8u /* size of two registers */
/* PCI Configuration space address and data registers. */
#define PPC405_PCI_CONFIG_ADDR  XPAR_PCI_0_CONFIG_ADDR
#define PPC405_PCI_CONFIG_DATA  XPAR_PCI_0_CONFIG_DATA

/* PCI Local configuration space parameters for io_block_mapping. */
#define PPC4xx_PCI_LCFG_PADDR   ((uint)XPAR_PCI_0_LCONFIG_ADDR)
#define PPC4xx_PCI_LCFG_VADDR   PPC4xx_PCI_LCFG_PADDR
#define PPC4xx_PCI_LCFG_SIZE    256u /* PCI configuration address space size */

#ifdef _IO_BASE
#undef _IO_BASE
#undef _ISA_MEM_BASE
#undef PCI_DRAM_OFFSET
#endif
#define _IO_BASE            isa_io_base
#define _ISA_MEM_BASE           isa_mem_base
#define PCI_DRAM_OFFSET         pci_dram_offset
#endif /* CONFIG_PCI */


/* We don't need anything mapped.  Size of zero will accomplish that. */
#define PPC4xx_ONB_IO_PADDR	0u
#define PPC4xx_ONB_IO_VADDR	0u
#define PPC4xx_ONB_IO_SIZE	0u

#if defined(CONFIG_XILINX_ML300)
#define PPC4xx_MACHINE_NAME "Xilinx ML300"
#define XILINX_SYS_ID_STR "Xilinx ML300 Reference System (Virtex-II Pro)\n"
#elif defined(CONFIG_XILINX_ML40x)
#define PPC4xx_MACHINE_NAME "Xilinx ML40x"
#define XILINX_SYS_ID_STR "Xilinx ML403/ML405 Reference System (Virtex-4 FX)\n"
#elif defined(CONFIG_XILINX_ML410)
#define PPC4xx_MACHINE_NAME "Xilinx ML410"
#define XILINX_SYS_ID_STR "Xilinx ML410 Reference System (Virtex-4 FX)\n"
#elif defined(CONFIG_XILINX_ML507)
#define PPC4xx_MACHINE_NAME "Xilinx ML507 PPC440 evaluation board"
#define XILINX_SYS_ID_STR "Xilinx ML507 PPC440 evaluation board (PPC440)\n"
#elif defined(CONFIG_XILINX_ML510)
#define PPC4xx_MACHINE_NAME "Xilinx ML510 PPC440 evaluation board"
#define XILINX_SYS_ID_STR "Xilinx ML510 PPC440 evaluation board (PPC440)\n"
#endif

#endif /* __ASM_XILINX_MLxxx_H__ */
#endif /* __KERNEL__ */
