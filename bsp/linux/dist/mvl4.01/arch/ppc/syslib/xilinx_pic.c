/*
 * arch/ppc/syslib/xilinx_pic.c
 *
 * Interrupt controller driver for Xilinx Virtex-II Pro.
 *
 * Author: MontaVista Software, Inc.
 *         source@mvista.com
 *
 * 2002-2004 (c) MontaVista Software, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */

#include <linux/init.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <platforms/4xx/xparameters/xparameters.h>
#include <asm/ibm4xx.h>

/* No one else should require these constants, so define them locally here. */
#define ISR 0			/* Interrupt Status Register */
#define IPR 1			/* Interrupt Pending Register */
#define IER 2			/* Interrupt Enable Register */
#define IAR 3			/* Interrupt Acknowledge Register */
#define SIE 4			/* Set Interrupt Enable bits */
#define CIE 5			/* Clear Interrupt Enable bits */
#define IVR 6			/* Interrupt Vector Register */
#define MER 7			/* Master Enable Register */

#if XPAR_XINTC_USE_DCR == 0
static volatile u32 *intc;
#define intc_out_be32(addr, mask)     out_be32((addr), (mask))
#define intc_in_be32(addr)            in_be32((addr))
#else
#define intc    XPAR_INTC_0_BASEADDR
#define intc_out_be32(addr, mask)     mtdcr((addr), (mask))
#define intc_in_be32(addr)            mfdcr((addr))
#endif
/* Global Variables */
struct hw_interrupt_type *ppc4xx_pic;

#define pci_piowr08(a, d)   outb(d,a)
#define pci_piord08(a)      inb(a)

#define debugk if (0) printk

#ifdef CONFIG_PCI
#define XILINX_PCI_IRQ        XPAR_INTC_0_PCI_0_VEC_ID_SBR

static void i8259_init(void)
{
	/* Init 8259 */

	pci_piowr08(0x20, 0x11);    // MST ICW1, Init ICW1-ICW4, enable cascade.
	pci_piowr08(0x21, 0x00);    // MST ICW2, Interrupt ID = 0x00.
	pci_piowr08(0x21, 0x04);    // MST ICW3, Cascade IRQ2.
	pci_piowr08(0x21, 0x01);    // MST ICW4, Use 8086 mode.
	pci_piowr08(0x21, 0xff);    // MST OCW1, Disable all except cascade.
	pci_piowr08(0xa0, 0x11);    // SLV ICW1, Init ICW1-ICW4, enable cascade.
	pci_piowr08(0xa1, 0x08);    // SLV ICW2, Interrupt ID = 0x08.
	pci_piowr08(0xa1, 0x02);    // SLV ICW3, Slave address = 2.
	pci_piowr08(0xa1, 0x01);    // SLV ICW4, Use 8086 mode.
	pci_piowr08(0xa1, 0xff);    // SLV OCW1, Disable all.

	pci_piowr08(0x4d0, 0xc0);   // ???  level sensitive for audio on 7.
	pci_piowr08(0x4d1, 0xc0);   // ???  level sensitive for IDE on 14, 15

	// Cheesy hack to cause periodic timer interrupts...

	// pci_piowr08(0x43, 0x34);        // Load TC0
	// pci_piowr08(0x40, 12);          // Low byte.
	// pci_piowr08(0x40, 34);          // High byte.

}

static void i8259_stat(void)
{
	unsigned char imr, isr, irr;

	imr = pci_piord08(0x21);
	pci_piowr08(0x20, 0x0a);
	irr = pci_piord08(0x20);
	pci_piowr08(0x20, 0x0b);
	isr = pci_piord08(0x20);

	debugk("MST imr = %02x, irr = %02x, isr = %02x\n", imr, irr, isr);

	imr = pci_piord08(0xa1);
	pci_piowr08(0xa0, 0x0a);
	irr = pci_piord08(0xa0);
	pci_piowr08(0xa0, 0x0b);
	isr = pci_piord08(0xa0);

	debugk("SLV imr = %02x, irr = %02x, isr = %02x\n", imr, irr, isr);
}

static void i8259_enab(void)
{
	pci_piowr08(0x21, 0x00);    // MST OCW1, Enable all.
	pci_piowr08(0xa1, 0x00);    // SLV OCW1, Enable all.
}

static int i8259_poll(void)
{
	unsigned char   dat0, dat1;

	pci_piowr08(0x20, 0x0c);
	dat0 = pci_piord08(0x20);
	debugk("poll = %02x\n", dat0);
	pci_piowr08(0xa0, 0x0c);
	dat1 = pci_piord08(0xa0);
	debugk("poll = %02x\n", dat1);

	if (dat0 == 0x82) {
		return((dat1 & 0x07) + 0x08);
	}
	return(dat0 & 0x07);
}

static void i8259_ack(void)
{
	pci_piowr08(0xa0, 0x20);
	pci_piowr08(0x20, 0x20);
}
#endif

static void
xilinx_intc_enable(unsigned int irq)
{
	unsigned long mask = (0x00000001 << (irq & 31));
	pr_debug("enable: %d\n", irq);
	intc_out_be32(intc + SIE, mask);
#ifdef CONFIG_PCI
	if (irq == XILINX_PCI_IRQ) {
		debugk("Enabling i8259, irq = %d\n", irq);
		i8259_stat();
	}
#endif
}

static void
xilinx_intc_disable(unsigned int irq)
{
	unsigned long mask = (0x00000001 << (irq & 31));
	pr_debug("disable: %d\n", irq);
	intc_out_be32(intc + CIE, mask);
#ifdef CONFIG_PCI
	if (irq == XILINX_PCI_IRQ) {
		debugk("Disabling i8259, irq = %d\n", irq);
		i8259_stat();
	}
#endif
}

static void
xilinx_intc_disable_and_ack(unsigned int irq)
{
	unsigned long mask = (0x00000001 << (irq & 31));
#ifdef CONFIG_PCI
	if (irq == XILINX_PCI_IRQ) {
		debugk("Disable and ack i8259, irq = %d\n", irq);
		i8259_stat();
		i8259_ack();
		i8259_stat();
	}
#endif
	pr_debug("disable_and_ack: %d\n", irq);
	intc_out_be32(intc + CIE, mask);
	if (!(irq_desc[irq].status & IRQ_LEVEL))
		intc_out_be32(intc + IAR, mask);	/* ack edge triggered intr */
}

static void
xilinx_intc_end(unsigned int irq)
{
	unsigned long mask = (0x00000001 << (irq & 31));
#ifdef CONFIG_PCI
	if (irq == XILINX_PCI_IRQ) {
		debugk("Interrupt end i8259, irq = %d\n", irq);
		i8259_stat();
	}
#endif
	pr_debug("end: %d\n", irq);
	if (!(irq_desc[irq].status & (IRQ_DISABLED | IRQ_INPROGRESS))) {
		/* ack level sensitive intr */
		if (irq_desc[irq].status & IRQ_LEVEL)
			intc_out_be32(intc + IAR, mask);
		/* unmask the interrupt */
		intc_out_be32(intc + SIE, mask);
	}
}

static struct hw_interrupt_type xilinx_intc = {
	"Xilinx Interrupt Controller",
	NULL,
	NULL,
	xilinx_intc_enable,
	xilinx_intc_disable,
	xilinx_intc_disable_and_ack,
	xilinx_intc_end,
	0
};

int
xilinx_pic_get_irq(struct pt_regs *regs)
{
	int irq;

	/*
	 * NOTE: This function is the one that needs to be improved in
	 * order to handle multiple interrupt controllers.  It currently
	 * is hardcoded to check for interrupts only on the first INTC.
	 */

	irq = intc_in_be32(intc + IVR);

	if (irq > 31 || irq < 0) {
		irq = -1;
	}

	pr_debug("get_irq: %d\n", irq);

#ifdef CONFIG_PCI
	if (irq == XILINX_PCI_IRQ) {
		debugk("Got i8259 IRQ\n");

		i8259_stat();
		irq = i8259_poll();
		/*printk("Got i8259 IRQ %d\n",irq);*/
		i8259_stat();

		// The pc_keyb.c interrupt handler doesn't seem to play
		// nicely when the IRQ is shared with another device.  So,
		// we need to make sure it is on its own handler chain.
		// In order to do this we map the i8259 interrupt from
		// 8 to something else if triggered by IRQ1.

		if (irq == 1) {
		    irq = 5;    // Map keyboard IRQ.
		} else {
		    irq = XILINX_PCI_IRQ;
		}
	}
#endif

	return (irq);
}

void __init
ppc4xx_pic_init(void)
{
	int i;

#if XPAR_XINTC_USE_DCR == 0
	intc = ioremap(XPAR_INTC_0_BASEADDR, 32);

	printk(KERN_INFO "Xilinx INTC #0 at 0x%08lX mapped to 0x%08lX\n",
	       (unsigned long) XPAR_INTC_0_BASEADDR, (unsigned long) intc);
#else
	printk(KERN_INFO "Xilinx INTC #0 at 0x%08lX (DCR)\n",
	       (unsigned long) XPAR_INTC_0_BASEADDR);
#endif

	/*
	 * Disable all external interrupts until they are
	 * explicity requested.
	 */
	intc_out_be32(intc + IER, 0);

	/* Acknowledge any pending interrupts just in case. */
	intc_out_be32(intc + IAR, ~(u32) 0);

	/* Turn on the Master Enable. */
	intc_out_be32(intc + MER, 0x3UL);

#ifdef CONFIG_PCI
	i8259_init();
	i8259_enab();
#endif

	ppc_md.get_irq = xilinx_pic_get_irq;

	for (i = 0; i < NR_IRQS; ++i)
		irq_desc[i].handler = &xilinx_intc;
}
