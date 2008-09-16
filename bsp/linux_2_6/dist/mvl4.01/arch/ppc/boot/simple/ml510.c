//----------------------------------------------------------------------------

#include <linux/config.h>
#include <linux/types.h>
#include <linux/elf.h>
#include <asm/bootinfo.h>
#include <asm/ibm4xx.h>
#include <asm/mmu.h>
#include <asm/mpc8xx.h>
#include <asm/mpc8260.h>
#include <asm/page.h>
#include <asm/processor.h>
#include <asm/residual.h>

#include "nonstdio.h"
//#include "zlib.h"

#include "ml510.h"
#include "xparameters.h"

#include <linux/ctype.h>
#include <stdarg.h>

//----------------------------------------------------------------------------
/*
 *  linux/lib/ctype.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/ctype.h>

unsigned char _ctype[] = {
_C,_C,_C,_C,_C,_C,_C,_C,            /* 0-7 */
_C,_C|_S,_C|_S,_C|_S,_C|_S,_C|_S,_C,_C,     /* 8-15 */
_C,_C,_C,_C,_C,_C,_C,_C,            /* 16-23 */
_C,_C,_C,_C,_C,_C,_C,_C,            /* 24-31 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,            /* 32-39 */
_P,_P,_P,_P,_P,_P,_P,_P,            /* 40-47 */
_D,_D,_D,_D,_D,_D,_D,_D,            /* 48-55 */
_D,_D,_P,_P,_P,_P,_P,_P,            /* 56-63 */
_P,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U,  /* 64-71 */
_U,_U,_U,_U,_U,_U,_U,_U,            /* 72-79 */
_U,_U,_U,_U,_U,_U,_U,_U,            /* 80-87 */
_U,_U,_U,_P,_P,_P,_P,_P,            /* 88-95 */
_P,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L,  /* 96-103 */
_L,_L,_L,_L,_L,_L,_L,_L,            /* 104-111 */
_L,_L,_L,_L,_L,_L,_L,_L,            /* 112-119 */
_L,_L,_L,_P,_P,_P,_P,_C,            /* 120-127 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        /* 128-143 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        /* 144-159 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,   /* 160-175 */
_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,       /* 176-191 */
_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,       /* 192-207 */
_U,_U,_U,_U,_U,_U,_U,_P,_U,_U,_U,_U,_U,_U,_U,_L,       /* 208-223 */
_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,       /* 224-239 */
_L,_L,_L,_L,_L,_L,_L,_P,_L,_L,_L,_L,_L,_L,_L,_L};      /* 240-255 */

//----------------------------------------------------------------------------

void
halt(void)
{
    while(1);
};

//----------------------------------------------------------------------------

#define JJ_SCRATCH 16
char    jj_scratch[JJ_SCRATCH];
char *  jj_vbuf = NULL;

void
jj_vputc(char c)
{
    putc(c);
};

void
jj_vputs(char * s)
{
    while (*s != 0) putc(*s++);
};

char *
jj_vifmt(ui32 base, ui16 plen, bool zpad, ui32 val)
{
    char *  ptr;
    ui32    len;

    ptr = jj_scratch + JJ_SCRATCH;
    *--ptr = 0;
    len = 0;
    do {
        char ch = val % base + '0';
        if (ch > '9') ch += 'a' - '9' - 1;
        *--ptr = ch;
        val /= base;
        len++;
    } while (val);

    while (len < plen) {
        if (zpad) *--ptr = '0';
        else *--ptr = ' ';
        len++;
    };

    return(ptr);
};

void
jj_vfmt(char * fmt, va_list ap)
{
    char    flg;
    bool    zpad;
    ui16    plen;
    ui32    vval;

    while (1) {
        while ((flg = PRG_RDB(fmt)) != '%') {
            fmt++;
            if (!flg) return;
            jj_vputc(flg);
        };
        fmt++;

        //------------------------------------------------------------

        flg = PRG_RDB(fmt);
        if (flg == '0') {
            zpad = 1;
            fmt++;
        } else {
            zpad = 0;
        };

        flg = PRG_RDB(fmt);
        if (isdigit(flg)) {
            plen = flg - '0';
            fmt++;
        } else {
            plen = 0;
        };

        //------------------------------------------------------------

        switch (flg = PRG_RDB(fmt)) {
        case 'b':
            vval = va_arg(ap, int);
            fmt++;
            break;
        case 'h':
            vval = va_arg(ap, int);
            fmt++;
            break;
        case 'l':
            vval = va_arg(ap, ui32);
            fmt++;
            break;
        default:
            vval = va_arg(ap, ui32);
        };

        //------------------------------------------------------------

        switch (flg = PRG_RDB(fmt)) {
        case 'c':
            jj_vputc(vval);
            break;
        case 's':
            jj_vputs((char *)((addr)vval));
            break;
        case 'o':
            jj_vputs(jj_vifmt(8, plen, zpad, vval));
            break;
        case 'd':
            if (((si32)vval) < 0) {
                jj_vputc('-');
                vval = -vval;
            };
            jj_vputs(jj_vifmt(10, plen, zpad, vval));
            break;
        case 'u':
            jj_vputs(jj_vifmt(10, plen, zpad, vval));
            break;
        case 'x':
            jj_vputs(jj_vifmt(16, plen, zpad, vval));
            break;
        default:
            jj_vputc(flg);
            continue;
        };
        fmt++;
    };
};

void
jj_fprintf(char * fmt, ...)
{
    va_list     ap;

    jj_vbuf = NULL;
    va_start(ap, fmt);
    jj_vfmt(fmt, ap);
    va_end(ap);
};

//----------------------------------------------------------------------------
// PCI code.
//----------------------------------------------------------------------------

#define PCI_HOST_ENABLE_CMD  0xFFFF0147
#define PCI_MEM_ADDR         XPAR_PCI_0_MEM_BASEADDR
#define PCI_MEM_END          XPAR_PCI_0_MEM_HIGHADDR
#define PCI_IO_ADDR          XPAR_PCI_0_IO_BASEADDR
#define PCI_IO_END           XPAR_PCI_0_IO_HIGHADDR

#define PCI_CONFIG_ADDR      XPAR_PCI_0_CONFIG_ADDR
#define PCI_CONFIG_DATA      XPAR_PCI_0_CONFIG_DATA

#define PCI_COMMAND          0x04

//----------------------------------------------------------------------------

#define PCI_CFG_ADR0(bus, dev, reg)                 \
    ((((bus)<<16) | ((dev)<<11) | (reg & 0xfc)) | 0x80000000)

#define PCI_CFG_ADR1(bus, dev, reg)                 \
    (PCI_CFG_ADR0(bus, dev, reg) | 0x01)

//----------------------------------------------------------------------------

ui32
pci_cfgrd08(ui32 bus, ui32 dev, ui32 reg)
{
    ui32    cadr, data;

    if (bus == 0) {
        cadr = PCI_CFG_ADR0(bus, dev, reg);
    } else {
        cadr = PCI_CFG_ADR1(bus, dev, reg);
    };

    WR32(PCI_CONFIG_ADDR, htole32(cadr));
    data = RD08(PCI_CONFIG_DATA + (reg & 0x03));

    // printf("pci_cfgrd08(%02x, %02x, %02x) %08x = %08x\n",
    //  bus, dev, reg, cadr, data);

    return(data);
};

ui32
pci_cfgrd32(ui32 bus, ui32 dev, ui32 reg)
{
    ui32    cadr, data;

    if (bus == 0) {
        cadr = PCI_CFG_ADR0(bus, dev, reg);
    } else {
        cadr = PCI_CFG_ADR1(bus, dev, reg);
    };

    WR32(PCI_CONFIG_ADDR, htole32(cadr));
    data = letoh32(RD32(PCI_CONFIG_DATA));

    // printf("pci_cfgrd32(%02x, %02x, %02x) %08x = %08x\n",
    //  bus, dev, reg, cadr, data);

    return(data);
};


//----------------------------------------------------------------------------

void
pci_cfgwr08(ui32 bus, ui32 dev, ui32 reg, byte data)
{
    ui32    cadr;

    if (bus == 0) {
        cadr = PCI_CFG_ADR0(bus, dev, reg);
    } else {
        cadr = PCI_CFG_ADR1(bus, dev, reg);
    };

    // printf("pci_cfgwr32(%02x, %02x, %02x) %08x = %08x\n",
    //  bus, dev, reg, cadr, data);

    WR32(PCI_CONFIG_ADDR, htole32(cadr));
    WR08(PCI_CONFIG_DATA + (reg & 0x03), data);
};

void
pci_cfgwr32(ui32 bus, ui32 dev, ui32 reg, ui32 data)
{
    ui32    cadr;

    if (bus == 0) {
        cadr = PCI_CFG_ADR0(bus, dev, reg);
    } else {
        cadr = PCI_CFG_ADR1(bus, dev, reg);
    };

    // printf("pci_cfgwr32(%02x, %02x, %02x) %08x = %08x\n",
    //  bus, dev, reg, cadr, data);

    WR32(PCI_CONFIG_ADDR, htole32(cadr));
    WR32(PCI_CONFIG_DATA, htole32(data));
};

//----------------------------------------------------------------------------

byte
pci_piord08(ui32 addr)
{
    byte    d;

    d = RD08(PCI_IO_ADDR+addr);
    return(d);
};

void
pci_piowr08(ui32 addr, byte data)
{
    WR08(PCI_IO_ADDR+addr, data);
};

//----------------------------------------------------------------------------

void
pci_init(void)
{
        /* self-configuration */
        WR32(PCI_CONFIG_ADDR, htole32(PCI_CFG_ADR0(0x0, 0x0, 0x4))); // address
    WR32(PCI_CONFIG_DATA, htole32(PCI_HOST_ENABLE_CMD)); // data
    /* max latency timer on bridge */
    WR32(PCI_CONFIG_ADDR, htole32(PCI_CFG_ADR0(0x0, 0x0, 0xC))); // address
    WR32(PCI_CONFIG_DATA, htole32(0x0000ff00)); // data
    /* max bus number */
    WR32(PCI_CONFIG_ADDR+8, htole32(0xff000000));
};

void
pci_scan(void)
{
    ui32    i, j, data;

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 31; j++) {
            data = pci_cfgrd32(i, j, 0x00);
            if (data != 0xffffffff) {
                printf("pci_scan: bus %d, device %2d, id %08x\n", i, j, data);
            };
        };
    };
};

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void
ppb_init(ui32 dev)
{
    ui32    data;

    data = pci_cfgrd32(0x00, dev, 0x00);

    printf("ppb_init: dev = %2d, id = %08x\n", dev, data);

    if (data != 0xac23104c) {
        prints("ppb_init: Error! Device ID does not match TI PCI2250.\n");
        halt();
    };

    //--------------------------------------------------------------------
    // The minimum initialization for this bridge is:
    // - Enable bus master, I/O, and memory space in the control reg.
    // - Program the primary/secondary/subordinate bus numbers.
    //
    // Once this is complete, it should be possible to scan config
    // regs on the secondary bus.

    pci_cfgwr32(0x00, dev, 0x04, 0x02100007);   // Control/Status
    pci_cfgwr32(0x00, dev, 0x18, 0x00010100);   // Bus numbers.

    //--------------------------------------------------------------------
    // Setup I/O mapping for PIO devices behind the bridge.
    //
    // pci_cfgwr32(0x00, dev, 0x44, 0x00000000);    // Base 0
    // pci_cfgwr32(0x00, dev, 0x48, 0x0000fff2);    // Limit0
    // pci_cfgwr32(0x00, dev, 0x54, 0x00000001);    // Enables.
};

//----------------------------------------------------------------------------
// Super I/O stuff.
//----------------------------------------------------------------------------

#define SIO_CFG_ADDR    0x3f0
#define SIO_CFG_DATA    0x3f1

void
sio_cfgwr08(ui32 addr, byte data)
{
    pci_piowr08(SIO_CFG_ADDR, addr);
    pci_piowr08(SIO_CFG_DATA, data);
};

byte
sio_cfgrd08(ui32 addr)
{
    byte    d;

    pci_piowr08(SIO_CFG_ADDR, addr);
    d = pci_piord08(SIO_CFG_DATA);
    return(d);
};

//----------------------------------------------------------------------------

bool
sio_init(void)
{
    byte    rev, id0, id1;
    ui32    ecnt = 0;

    //--------------------------------------------------------------------

    pci_cfgwr32(0x00, 0x02, 0x58, 0x4c);    // Enable IDE controller.

    //--------------------------------------------------------------------
    // Enable the Super I/O.

    pci_piowr08(SIO_CFG_ADDR, 0xbb);    // Force exit config mode.
    pci_piowr08(SIO_CFG_ADDR, 0x51);    // Enter config mode, key 1.
    pci_piowr08(SIO_CFG_ADDR, 0x23);    // Enter config mode, key 2.

    rev = sio_cfgrd08(0x1f);
    id0 = sio_cfgrd08(0x20);
    id1 = sio_cfgrd08(0x21);

    printf("sio_init: Device ID = %02x %02x, Revision = %02x.\n",
        sio_cfgrd08(0x20),
        sio_cfgrd08(0x21),
        sio_cfgrd08(0x1f));

    if ((rev != 0xf1) && (id0 != 0x53) && (id1 != 0x15)) {
        prints("sio_init: Bad device identification, bailing.\n");
        return(0);
    };

    //--------------------------------------------------------------------
    // Test device reset.

    if (sio_cfgrd08(0x07) != 0x00) prints("sio_init: Warning, suspicious reset state.\n");

    sio_cfgwr08(0x07, 0xff); if (sio_cfgrd08(0x07) != 0xff) ecnt++;
    sio_cfgwr08(0x07, 0x55); if (sio_cfgrd08(0x07) != 0x55) ecnt++;
    sio_cfgwr08(0x07, 0x23); if (sio_cfgrd08(0x07) != 0x23) ecnt++;

    if (ecnt > 0) {
        prints("sio_init: Unable to write logical device register.\n");
        return(0);
    };

    sio_cfgwr08(0x02, 0xff);    // Soft-reset the Super I/O device.

    if (sio_cfgrd08(0x07) != 0x00) {
        prints("sio_init: Unable to soft-reset the Super I/O.");
        return(0);
    };

    //--------------------------------------------------------------------
    // Program registers to configure Super I/O devices.

    sio_cfgwr08(0x07, 0x03);    // Select Parallel Port device.
    sio_cfgwr08(0x30, 0x01);    // Enable device.

    printf("sio_init: LPT1 base = 0x%02x%02x, irq = %d.\n",
        sio_cfgrd08(0x60), sio_cfgrd08(0x61), sio_cfgrd08(0x70));

    sio_cfgwr08(0x07, 0x04);    // Select UART1.
    sio_cfgwr08(0x30, 0x01);    // Enable device.

    printf("sio_init: COM1 base = 0x%02x%02x, irq = %d.\n",
        sio_cfgrd08(0x60), sio_cfgrd08(0x61), sio_cfgrd08(0x70));

    sio_cfgwr08(0x07, 0x05);    // Select UART2.
    sio_cfgwr08(0x30, 0x01);    // Enable device.

    // printf("sio_init: IRDA base = 0x%02x%02x, irq = %d.\n",
    //  sio_cfgrd08(0x60), sio_cfgrd08(0x61), sio_cfgrd08(0x70));

    sio_cfgwr08(0x07, 0x0b);    // Select UART3.
    sio_cfgwr08(0x30, 0x01);    // Enable device.

    printf("sio_init: COM2 base = 0x%02x%02x, irq = %d.\n",
        sio_cfgrd08(0x60), sio_cfgrd08(0x61), sio_cfgrd08(0x70));

    sio_cfgwr08(0x07, 0x07);    // Select KBC.
    sio_cfgwr08(0x30, 0x01);    // Enable device.

    sio_cfgwr08(0x72, 0x01);    // Set PS2 IRQ to 1.

    printf("sio_init: KBC irq = %d, PS2 irq = %d.\n",
        sio_cfgrd08(0x70), sio_cfgrd08(0x72));

    //--------------------------------------------------------------------

    pci_piowr08(SIO_CFG_ADDR, 0xbb);        // Exit config mode.

    prints("sio_init: Super I/O initialization complete.\n");

    return(1);
};

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void
sbr_init(void)
{
    //--------------------------------------------------------------------
    // IDE initialization.
    //
    // - Enable the IDE controller, and both channel's I/O pad drivers.
    // - Set the primary channel to use IRQ14 in level-sensitive mode.
    // - Set the secondary channel to use IRQ15 in level-sensitive mode.
    //
    // Note that the primary channel can be used in edge-triggered mode,
    // but edge-triggered mode is apparently not supported for the
    // secondary channel.  To keep things consistent, both are used in
    // level-sensitive mode.  This requires that the 8259 be programmed
    // to the correct level/edge mode for IRQ14, IRQ15.

    pci_cfgwr08(0,  2, 0x58, 0x4c);     // Enable IDE controller.
    pci_cfgwr08(0,  2, 0x44, 0x0d);     // Level, IRQ14.
    pci_cfgwr08(0,  2, 0x75, 0x0f);     // Level, IRQ15.

    // The M5229 IDE controller defaults to compatible mode, which
    // will force Linux to use PIO-only mode for IDE devices attached
    // to it.  If the M5229 is placed in native mode, Linux will
    // enable DMA by default.  Uncomment one of the following two
    // lines to enable DMA mode.

    // pci_cfgwr08(0, 11, 0x09, 0xfa);      // Compat mode.
    pci_cfgwr08(0, 11, 0x09, 0xff);     // Native mode.

    //--------------------------------------------------------------------
    // Interrupt routing.

    pci_cfgwr08(0,  2, 0x48, 0x00);     // INTB = disabled, INTA = disabled.
    pci_cfgwr08(0,  2, 0x49, 0x00);     // INTD = disabled, INTC = disabled.
    pci_cfgwr08(0,  2, 0x4a, 0x00);     // INTF = disabled, INTE = disabled.
    pci_cfgwr08(0,  2, 0x4b, 0x60);     // Audio = INT7, Modem = disabled.

    pci_cfgwr08(0,  2, 0x74, 0x06);     // USB = INT7.

    //--------------------------------------------------------------------

    // pci_cfgwr08(0,  1, 0x44, 0xff); // Enable audio IO decode?  Not needed, for SB emu only?
};

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void
ml510_init(void)
{
    prints("\n\n");
    prints("Xilinx ML510 Board-Specific Initialization:\n");
    prints("\n");


    pci_init();
    ppb_init(9);
    pci_scan();
    sio_init();
    sbr_init();
};

//----------------------------------------------------------------------------

