/*
 * arch/ppc/platforms/4xx/xilinx_mlxxx.c
 *
 * Xilinx ML300, ML403, ML405, ML410, ML507, and ML510 evaluation boards initialization
 *
 * Author: MontaVista Software, Inc.
 *         source@mvista.com
 *
 * 2002-2006 (c) MontaVista Software, Inc.  This file is licensed under the
 * terms of the GNU General Public License version 2.  This program is licensed
 * "as is" without any warranty of any kind, whether express or implied.
 */

#include <linux/config.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/tty.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serialP.h>
#include <linux/kgdb.h>
#include <asm/io.h>
#include <asm/machdep.h>
#include <asm/ocp.h>

#include <platforms/4xx/virtex.h>	/* for NR_SER_PORTS */

#ifdef CONFIG_PCI
#include <asm/pci-bridge.h>
#endif

/*
 * As an overview of how the following functions (platform_init,
 * mlxxx_map_io, mlxxx_setup_arch and mlxxx_init_IRQ) fit into the
 * kernel startup procedure, here's a call tree:
 *
 * start_here					arch/ppc/kernel/head_4xx.S
 *  early_init					arch/ppc/kernel/setup.c
 *  machine_init				arch/ppc/kernel/setup.c
 *    platform_init				this file
 *      ppc4xx_init				arch/ppc/syslib/ppc4xx_setup.c
 *        parse_bootinfo
 *          find_bootinfo
 *        "setup some default ppc_md pointers"
 *  MMU_init					arch/ppc/mm/init.c
 *    *ppc_md.setup_io_mappings == mlxxx_map_io	this file
 *      ppc4xx_map_io				arch/ppc/syslib/ppc4xx_setup.c
 *  start_kernel				init/main.c
 *    setup_arch				arch/ppc/kernel/setup.c
 *      *ppc_md.setup_arch == mlxxx_setup_arch	this file
 *        ppc4xx_setup_arch			arch/ppc/syslib/ppc4xx_setup.c
 *          ppc4xx_find_bridges			arch/ppc/syslib/ppc405_pci.c
 *    init_IRQ					arch/ppc/kernel/irq.c
 *      *ppc_md.init_IRQ == mlxxx_init_IRQ	this file
 *        ppc4xx_init_IRQ			arch/ppc/syslib/ppc4xx_setup.c
 *          ppc4xx_pic_init			arch/ppc/syslib/xilinx_pic.c
 */

#ifdef CONFIG_PCI

#define PCI_INTA (XPAR_INTC_0_PCI_0_VEC_ID_A)
#define PCI_INTB (XPAR_INTC_0_PCI_0_VEC_ID_B)
#define PCI_INTC (XPAR_INTC_0_PCI_0_VEC_ID_C)
#define PCI_INTD (XPAR_INTC_0_PCI_0_VEC_ID_D)
/* changes for ML410 */
#define PCI_SBR  (XPAR_INTC_0_PCI_0_VEC_ID_SBR)

int __init
ppc4xx_map_irq(struct pci_dev *dev, unsigned char idsel, unsigned char pin)
{
static signed char pci_irq_table[][32] =
{
{ PCI_SBR,  -1,       -1,       -1       },    /* ALi Audio        1 */
{ PCI_SBR,  -1,       -1,       -1       },    /* ALi South Bridge 2 */
{ PCI_SBR,  -1,       -1,       -1       },    /* ALi Modem        3 */
{ -1,       -1,       -1,       -1       },    /* Other            4 */
{ PCI_INTB, PCI_INTC, PCI_INTD, PCI_INTA },    /* Pri. slot 5      5 */
{ PCI_INTC, PCI_INTD, PCI_INTA, PCI_INTB },    /* Pri. slot 3      6 */
{ -1,       -1,       -1,       -1       },    /* Other            7 */
{ -1,       -1,       -1,       -1       },    /* Other            8 */
{ PCI_INTB, PCI_INTC, PCI_INTD, PCI_INTA },    /* PCI 2 PCI Bridge 9 */
{ PCI_SBR,  -1,       -1,       -1       },    /* ALi USB #2      10 */
{ PCI_SBR,  -1,       -1,       -1       },    /* ALi IDE         11 */
{ PCI_SBR,  -1,       -1,       -1       },    /* ALi Pwr Mgmt    12 */
{ -1,       -1,       -1,       -1       },    /* Other           13 */
{ -1,       -1,       -1,       -1       },    /* Other           14 */
{ PCI_SBR,  -1,       -1,       -1       },    /* ALi USB #1      15 */
};

    const long min_idsel = 1, max_idsel = 15, irqs_per_slot = 4;
        int res = PCI_IRQ_TABLE_LOOKUP;
    int bus = dev->bus->number;

        printk("ppc4xx_map_irq: bus %d idsel %d pin %d, res = %d\n", bus, idsel, pin, res);

        return res;
};
#endif

#if defined(XPAR_POWER_0_POWERDOWN_BASEADDR)

static volatile unsigned *powerdown_base =
    (volatile unsigned *) XPAR_POWER_0_POWERDOWN_BASEADDR;

static void
xilinx_power_off(void)
{
	local_irq_disable();
	out_be32(powerdown_base, XPAR_POWER_0_POWERDOWN_VALUE);
	while (1) ;
}
#endif

void __init
mlxxx_map_io(void)
{
	ppc4xx_map_io();

#if defined(XPAR_POWER_0_POWERDOWN_BASEADDR)
	powerdown_base = ioremap((unsigned long) powerdown_base,
				 XPAR_POWER_0_POWERDOWN_HIGHADDR -
				 XPAR_POWER_0_POWERDOWN_BASEADDR + 1);
#endif

#ifdef CONFIG_PCI
    /*
     * Enable the PCI initiator functions in the PCI core by writing
     * to the self-configuration space as described in the
     * Configuration section of the OPB to PCI Bridge chapter of the
     * Virtex-II Pro Platform FPGA Developer's Kit manual.
     */
        /* enable bridge */
        out_le32((volatile unsigned *)(PPC405_PCI_CONFIG_ADDR),
                 (0x80000000 | PCI_COMMAND));
        out_le32((volatile unsigned *)(PPC405_PCI_CONFIG_DATA),
                 (PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER
                  | PCI_COMMAND_PARITY | PCI_COMMAND_SERR | 0xffff0000));
        /* set max lat timer on bridge */
        out_le32((volatile unsigned *)(PPC405_PCI_CONFIG_ADDR),
                 (0x80000000 | 0xc));
        out_le32((volatile unsigned *)(PPC405_PCI_CONFIG_DATA),
                 0x0000ff00);
        /* set max bus number to max bus number */
        out_be32((volatile unsigned *)(PPC405_PCI_CONFIG_ADDR+8), 0xff);
#endif

}

static void __init
mlxxx_early_serial_map(void)
{
	struct serial_state old_ports[] = { SERIAL_PORT_DFNS };
	struct uart_port port;
	int i;

	/* Setup ioremapped serial port access */
	for (i = 0; i < ARRAY_SIZE(old_ports); i++ ) {
		memset(&port, 0, sizeof(port));
		port.membase = ioremap((phys_addr_t)(old_ports[i].iomem_base), 16);
		port.irq = old_ports[i].irq;
		port.uartclk = old_ports[i].baud_base * 16;
		port.regshift = old_ports[i].iomem_reg_shift;
		port.iotype = SERIAL_IO_MEM;
		port.flags = ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST;
		port.line = i;
		port.lock = SPIN_LOCK_UNLOCKED;

#ifdef CONFIG_SERIAL_8250
		if (early_serial_setup(&port) != 0)
			printk("Early serial init of port %d failed\n", i);
#endif
#ifdef CONFIG_KGDB_8250
		kgdb8250_add_port(i, &port);
#endif
	}
}

void __init
mlxxx_setup_arch(void)
{
#if 0 && defined(CONFIG_PCI)
    static const int bus = 0;
    static const int devfn = PCI_DEVFN(2, 0);
    static const int cr_offset = 0x80;
    static const u32 cr = ((1 << 27) |  /* P2CCLK */
                   (1 << 22) |  /* CBRSVD */
                   (4 << 16) |  /* CDMACHAN = not used */
                   (1 << 15) |  /* MRBURSTDN */
                   (1 << 6) |   /* PWRSAVINGS */
                   (1 << 5) |   /* SUBSYSRW */
                   (1 << 1));   /* KEEPCLK */
    static const int mfunc_offset = 0x8C;
    static const u32 mfunc = ((2 << 8) |    /* MFUNC2 = INTC */
                  (2 << 4) |    /* MFUNC1 = INTB */
                  (2 << 0));    /* MFUNC0 = INTA */

    /* Set up the clocks on the PCI44451 CardBus/FireWire bridge. */
    early_write_config_dword(0, bus, devfn, cr_offset, cr);

    /* Set up the interrupts on the PCI44451 CardBus/FireWire bridge. */
    early_write_config_dword(0, bus, devfn, mfunc_offset, mfunc);
#endif

	ppc4xx_setup_arch();	/* calls ppc4xx_find_bridges() */

	mlxxx_early_serial_map();

	/* Identify the system */
	printk(KERN_INFO XILINX_SYS_ID_STR);
	printk(KERN_INFO "Port by MontaVista Software, Inc. (source@mvista.com)\n");
}

/* Called after board_setup_irq from ppc4xx_init_IRQ(). */
void __init
mlxxx_init_irq(void)
{
	unsigned int i;

	ppc4xx_init_IRQ();

	/*
	 * For PowerPC 405 cores the default value for NR_IRQS is 32.
	 * See include/asm-ppc/irq.h for details.
	 * This is just fine for ML300, ML403, ML405, ML410 and ML507
	 */
//#if (NR_IRQS != 32)
//#error NR_IRQS must be 32 for ML300/ML403/ML405/ML410/ML507
//#endif

	for (i = 0; i < NR_IRQS; i++) {
		if (XPAR_INTC_0_KIND_OF_INTR & (0x00000001 << i))
			irq_desc[i].status &= ~IRQ_LEVEL;
		else
			irq_desc[i].status |= IRQ_LEVEL;
	}
}

void __init
platform_init(unsigned long r3, unsigned long r4, unsigned long r5,
	      unsigned long r6, unsigned long r7)
{
	ppc4xx_init(r3, r4, r5, r6, r7);

	ppc_md.setup_arch = mlxxx_setup_arch;
	ppc_md.setup_io_mappings = mlxxx_map_io;
	ppc_md.init_IRQ = mlxxx_init_irq;

#if defined(XPAR_POWER_0_POWERDOWN_BASEADDR)
	ppc_md.power_off = xilinx_power_off;
#endif
}
