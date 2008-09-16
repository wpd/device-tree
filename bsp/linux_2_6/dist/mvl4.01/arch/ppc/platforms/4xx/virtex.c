/*
 * arch/ppc/platforms/4xx/virtex.c
 *
 * Author: MontaVista Software, Inc.
 *         source@mvista.com
 *
 * 2002-2005 (c) MontaVista Software, Inc.  This file is licensed under the
 * terms of the GNU General Public License version 2.  This program is licensed
 * "as is" without any warranty of any kind, whether express or implied.
 */

#include <linux/config.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/xilinx_devices.h>
#include <asm/ocp.h>
#include <platforms/4xx/virtex.h>

#ifdef XPAR_INTC_0_PS2_0_IP2INTC_IRPT_1_VEC_ID
#define XPAR_INTC_0_PS2_0_VEC_ID	XPAR_INTC_0_PS2_0_IP2INTC_IRPT_1_VEC_ID
#endif

#ifdef XPAR_INTC_0_PS2_0_IP2INTC_IRPT_2_VEC_ID
#define XPAR_INTC_0_PS2_1_VEC_ID	XPAR_INTC_0_PS2_0_IP2INTC_IRPT_2_VEC_ID
#endif

/* Have OCP take care of the serial ports. */
struct ocp_def core_ocp[] = {
#ifdef XPAR_UARTNS550_0_BASEADDR
	{ .vendor	= OCP_VENDOR_XILINX,
	  .function	= OCP_FUNC_16550,
	  .index	= 0,
	  .paddr	= XPAR_UARTNS550_0_BASEADDR,
	  .irq		= XPAR_INTC_0_UARTNS550_0_VEC_ID,
	  .pm		= OCP_CPM_NA
	},
#ifdef XPAR_UARTNS550_1_BASEADDR
	{ .vendor	= OCP_VENDOR_XILINX,
	  .function	= OCP_FUNC_16550,
	  .index	= 1,
	  .paddr	= XPAR_UARTNS550_1_BASEADDR,
	  .irq		= XPAR_INTC_0_UARTNS550_1_VEC_ID,
	  .pm		= OCP_CPM_NA
	},
#ifdef XPAR_UARTNS550_2_BASEADDR
	{ .vendor	= OCP_VENDOR_XILINX,
	  .function	= OCP_FUNC_16550,
	  .index	= 2,
	  .paddr	= XPAR_UARTNS550_2_BASEADDR,
	  .irq		= XPAR_INTC_0_UARTNS550_2_VEC_ID,
	  .pm		= OCP_CPM_NA
	},
#ifdef XPAR_UARTNS550_3_BASEADDR
	{ .vendor	= OCP_VENDOR_XILINX,
	  .function	= OCP_FUNC_16550,
	  .index	= 3,
	  .paddr	= XPAR_UARTNS550_3_BASEADDR,
	  .irq		= XPAR_INTC_0_UARTNS550_3_VEC_ID,
	  .pm		= OCP_CPM_NA
	},
#ifdef XPAR_UARTNS550_4_BASEADDR
#error Edit this file to add more devices.
#endif			/* 4 */
#endif			/* 3 */
#endif			/* 2 */
#endif			/* 1 */
#endif			/* 0 */
	{ .vendor	= OCP_VENDOR_INVALID
	}
};

/* Xilinx Virtex-II Pro device descriptions */

#ifdef XPAR_EMAC_0_BASEADDR

static struct xemac_platform_data xemac_0_pdata = {
	// device_flags is used by older emac drivers. The new style is to use separate feilds
	.device_flags = (XPAR_EMAC_0_ERR_COUNT_EXIST ? XEMAC_HAS_ERR_COUNT : 0) |
		(XPAR_EMAC_0_MII_EXIST ? XEMAC_HAS_MII : 0) |
		(XPAR_EMAC_0_CAM_EXIST ? XEMAC_HAS_CAM : 0) |
		(XPAR_EMAC_0_JUMBO_EXIST ? XEMAC_HAS_JUMBO : 0),
	.dma_mode = XPAR_EMAC_0_DMA_PRESENT,
	.has_mii     = XPAR_EMAC_0_MII_EXIST,
	.has_err_cnt = XPAR_EMAC_0_ERR_COUNT_EXIST,
	.has_cam     = XPAR_EMAC_0_CAM_EXIST,
	.has_jumbo   = XPAR_EMAC_0_JUMBO_EXIST,
	.tx_dre      = XPAR_EMAC_0_TX_DRE_TYPE,
	.rx_dre      = XPAR_EMAC_0_RX_DRE_TYPE,
	.tx_hw_csum  = XPAR_EMAC_0_TX_INCLUDE_CSUM,
	.rx_hw_csum  = XPAR_EMAC_0_RX_INCLUDE_CSUM,
};

static struct platform_device xilinx_emac_0_device = {
	.name = "xilinx_emac",
	.id = XPAR_EMAC_0_DEVICE_ID,
	.dev.platform_data = &xemac_0_pdata,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_EMAC_0_BASEADDR,
			.end	= XPAR_EMAC_0_HIGHADDR,
			.flags	= IORESOURCE_MEM
		},
		{
			.start	= XPAR_INTC_0_EMAC_0_VEC_ID,
			.end	= XPAR_INTC_0_EMAC_0_VEC_ID,
			.flags	= IORESOURCE_IRQ
		}
	}
};

#endif /* XPAR_EMAC_0_BASEADDR */

#ifdef XPAR_EMAC_1_BASEADDR

static struct xemac_platform_data xemac_1_pdata = {
	// device_flags is used by older emac drivers. The new style is to use separate feilds
	.device_flags = (XPAR_EMAC_1_ERR_COUNT_EXIST ? XEMAC_HAS_ERR_COUNT : 0) |
		(XPAR_EMAC_1_MII_EXIST ? XEMAC_HAS_MII : 0) |
		(XPAR_EMAC_1_CAM_EXIST ? XEMAC_HAS_CAM : 0) |
		(XPAR_EMAC_1_JUMBO_EXIST ? XEMAC_HAS_JUMBO : 0),
	.dma_mode = XPAR_EMAC_1_DMA_PRESENT,
	.has_mii     = XPAR_EMAC_1_MII_EXIST,
	.has_err_cnt = XPAR_EMAC_1_ERR_COUNT_EXIST,
	.has_cam     = XPAR_EMAC_1_CAM_EXIST,
	.has_jumbo   = XPAR_EMAC_1_JUMBO_EXIST,
	.tx_dre      = XPAR_EMAC_1_TX_DRE_TYPE,
	.rx_dre      = XPAR_EMAC_1_RX_DRE_TYPE,
	.tx_hw_csum  = XPAR_EMAC_1_TX_INCLUDE_CSUM,
	.rx_hw_csum  = XPAR_EMAC_1_RX_INCLUDE_CSUM,
};

static struct platform_device xilinx_emac_1_device = {
	.name = "xilinx_emac",
	.id = XPAR_EMAC_1_DEVICE_ID,
	.dev.platform_data = &xemac_1_pdata,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_EMAC_1_BASEADDR,
			.end	= XPAR_EMAC_1_HIGHADDR,
			.flags	= IORESOURCE_MEM
		},
		{
			.start	= XPAR_INTC_0_EMAC_1_VEC_ID,
			.end	= XPAR_INTC_0_EMAC_1_VEC_ID,
			.flags	= IORESOURCE_IRQ
		}
	}
};

#endif /* XPAR_EMAC_1_BASEADDR */

#ifdef XPAR_TEMAC_0_BASEADDR

static struct xtemac_platform_data xtemac_0_pdata = {
#ifdef XPAR_TEMAC_0_INCLUDE_RX_CSUM
	.tx_dre = XPAR_TEMAC_0_TX_DRE_TYPE,
	.rx_dre = XPAR_TEMAC_0_RX_DRE_TYPE,
	.tx_csum = XPAR_TEMAC_0_INCLUDE_TX_CSUM,
	.rx_csum = XPAR_TEMAC_0_INCLUDE_RX_CSUM,
	.phy_type = XPAR_HARD_TEMAC_0_PHY_TYPE,
	.rx_pkt_fifo_depth = XPAR_TEMAC_0_RXFIFO_DEPTH,
	.tx_pkt_fifo_depth = XPAR_TEMAC_0_TXFIFO_DEPTH,
	.dcr_host = 0,
	.dre = 0,
#else
	.dcr_host = XPAR_TEMAC_0_TEMAC_DCR_HOST,
	.dre = XPAR_TEMAC_0_INCLUDE_DRE,
	.rx_pkt_fifo_depth = XPAR_TEMAC_0_IPIF_RDFIFO_DEPTH,
	.tx_pkt_fifo_depth = XPAR_TEMAC_0_IPIF_WRFIFO_DEPTH,
#endif
	.dma_mode = XPAR_TEMAC_0_DMA_TYPE,
	.mac_fifo_depth = XPAR_TEMAC_0_MAC_FIFO_DEPTH
};

static struct platform_device xilinx_temac_0_device = {
	.name = "xilinx_temac",
	.id = XPAR_TEMAC_0_DEVICE_ID,
	.dev.platform_data = &xtemac_0_pdata,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_TEMAC_0_BASEADDR,
			.end	= XPAR_TEMAC_0_HIGHADDR,
			.flags	= IORESOURCE_MEM
		},
		{
			.start	= XPAR_INTC_0_TEMAC_0_VEC_ID,
			.end	= XPAR_INTC_0_TEMAC_0_VEC_ID,
			.flags	= IORESOURCE_IRQ
		}
	}
};

#endif /* XPAR_TEMAC_0_BASEADDR */

#ifdef XPAR_TEMAC_1_BASEADDR

static struct xtemac_platform_data xtemac_1_pdata = {
#ifdef XPAR_TEMAC_0_INCLUDE_RX_CSUM
	.tx_dre = XPAR_TEMAC_1_TX_DRE_TYPE,
	.rx_dre = XPAR_TEMAC_1_RX_DRE_TYPE,
	.tx_csum = XPAR_TEMAC_1_INCLUDE_TX_CSUM,
	.rx_csum = XPAR_TEMAC_1_INCLUDE_RX_CSUM,
	.phy_type = XPAR_HARD_TEMAC_1_PHY_TYPE,
#endif
	.dma_mode = XPAR_TEMAC_1_DMA_TYPE,
	.rx_pkt_fifo_depth = XPAR_TEMAC_1_IPIF_RDFIFO_DEPTH,
	.tx_pkt_fifo_depth = XPAR_TEMAC_1_IPIF_WRFIFO_DEPTH,
	.mac_fifo_depth = XPAR_TEMAC_1_MAC_FIFO_DEPTH,
	.dcr_host = XPAR_TEMAC_1_TEMAC_DCR_HOST,
	.dre = XPAR_TEMAC_1_INCLUDE_DRE
};

static struct platform_device xilinx_temac_1_device = {
	.name = "xilinx_temac",
	.id = XPAR_TEMAC_1_DEVICE_ID,
	.dev.platform_data = &xtemac_1_pdata,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_TEMAC_1_BASEADDR,
			.end	= XPAR_TEMAC_1_HIGHADDR,
			.flags	= IORESOURCE_MEM
		},
		{
			.start	= XPAR_INTC_0_TEMAC_1_VEC_ID,
			.end	= XPAR_INTC_0_TEMAC_1_VEC_ID,
			.flags	= IORESOURCE_IRQ
		}
	}
};

#endif /* XPAR_TEMAC_1_BASEADDR */

#ifdef XPAR_LLTEMAC_0_BASEADDR
static struct xlltemac_platform_data xlltemac_0_pdata = {
	.tx_csum = XPAR_LLTEMAC_0_TXCSUM,
	.rx_csum = XPAR_LLTEMAC_0_RXCSUM,
	.phy_type = XPAR_LLTEMAC_0_PHY_TYPE,
	.ll_dev_type = XPAR_LLTEMAC_0_LLINK_CONNECTED_TYPE,
	.ll_dev_baseaddress = XPAR_LLTEMAC_0_LLINK_CONNECTED_BASEADDR,
#if (XPAR_LLTEMAC_0_LLINK_CONNECTED_TYPE == XPAR_LL_DMA)
#ifdef XPAR_XLLDMA_USE_DCR
	.ll_dev_dma_use_dcr = 1,
#else
	.ll_dev_dma_use_dcr = 0,
#endif
	.ll_dev_dma_rx_irq = XPAR_LLTEMAC_0_LLINK_CONNECTED_DMARX_INTR,
	.ll_dev_dma_tx_irq = XPAR_LLTEMAC_0_LLINK_CONNECTED_DMATX_INTR
#elif defined XPAR_LLTEMAC_0_LLINK_CONNECTED_FIFO_INTR
	.ll_dev_fifo_irq = XPAR_LLTEMAC_0_LLINK_CONNECTED_FIFO_INTR
#endif
};


static struct platform_device xilinx_lltemac_0_device = {
	.name = "xilinx_lltemac",
	.id = XPAR_LLTEMAC_0_DEVICE_ID,
	.dev.platform_data = &xlltemac_0_pdata,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_LLTEMAC_0_BASEADDR,
			.end	= XPAR_LLTEMAC_0_BASEADDR + 0x3F,
			.flags	= IORESOURCE_MEM
		},
#ifdef XPAR_LLTEMAC_0_INTR
		{
			.start	= XPAR_LLTEMAC_0_INTR,
			.end	= XPAR_LLTEMAC_0_INTR,
			.flags	= IORESOURCE_IRQ
		}
#endif
	}
};

#endif /* XPAR_LLTEMAC_0_BASEADDR */

#ifdef XPAR_LLTEMAC_1_BASEADDR
static struct xlltemac_platform_data xlltemac_1_pdata = {
	.tx_csum = XPAR_LLTEMAC_1_TXCSUM,
	.rx_csum = XPAR_LLTEMAC_1_RXCSUM,
	.phy_type = XPAR_LLTEMAC_1_PHY_TYPE,
	.ll_dev_type = XPAR_LLTEMAC_1_LLINK_CONNECTED_TYPE,
	.ll_dev_baseaddress = XPAR_LLTEMAC_1_LLINK_CONNECTED_BASEADDR,
#if (XPAR_LLTEMAC_0_LLINK_CONNECTED_TYPE == XPAR_LL_DMA)
	.ll_dev_dma_rx_irq = XPAR_LLTEMAC_1_LLINK_CONNECTED_DMARX_INTR,
	.ll_dev_dma_tx_irq = XPAR_LLTEMAC_1_LLINK_CONNECTED_DMATX_INTR
#elif defined XPAR_LLTEMAC_1_LLINK_CONNECTED_FIFO_INTR
	.ll_dev_fifo_irq = XPAR_LLTEMAC_1_LLINK_CONNECTED_FIFO_INTR
#endif
};


static struct platform_device xilinx_lltemac_1_device = {
	.name = "xilinx_lltemac",
	.id = XPAR_LLTEMAC_1_DEVICE_ID,
	.dev.platform_data = &xlltemac_1_pdata,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_LLTEMAC_1_BASEADDR,
			.end	= XPAR_LLTEMAC_1_BASEADDR + 0x3F,
			.flags	= IORESOURCE_MEM
		},
#ifdef XPAR_LLTEMAC_1_INTR
		{
			.start	= XPAR_LLTEMAC_1_INTR,
			.end	= XPAR_LLTEMAC_1_INTR,
			.flags	= IORESOURCE_IRQ
		}
#endif
	}
};

#endif /* XPAR_LL_TEMAC_1_BASEADDR */


#ifdef XPAR_PS2_0_BASEADDR

static struct platform_device xilinx_ps2_0_device = {
	.name = "xilinx_ps2",
	.id = 0,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_PS2_0_BASEADDR,
			.end	= XPAR_PS2_0_HIGHADDR,
			.flags	= IORESOURCE_MEM
		},
		{
			.start	= XPAR_INTC_0_PS2_0_VEC_ID,
			.end	= XPAR_INTC_0_PS2_0_VEC_ID,
			.flags	= IORESOURCE_IRQ
		}
	}
};

#endif /* XPAR_PS2_0_BASEADDR */

#ifdef XPAR_PS2_1_BASEADDR

static struct platform_device xilinx_ps2_1_device = {
	.name = "xilinx_ps2",
	.id = 1,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_PS2_1_BASEADDR,
			.end	= XPAR_PS2_1_HIGHADDR,
			.flags	= IORESOURCE_MEM
		},
		{
			.start	= XPAR_INTC_0_PS2_1_VEC_ID,
			.end	= XPAR_INTC_0_PS2_1_VEC_ID,
			.flags	= IORESOURCE_IRQ
		}
	}
};

#endif /* XPAR_PS2_1_BASEADDR */

#ifdef XPAR_TFT_0_BASEADDR

static struct resource xilinx_lcd_0_resource = {
	.start = XPAR_TFT_0_BASEADDR,
	.end = XPAR_TFT_0_BASEADDR+7,
	.flags = IORESOURCE_MEM
};

static struct platform_device xilinx_lcd_0_device = {
	.name = "xilinx_fb",
	.id = 0,
	.num_resources = 1,
	.resource = &xilinx_lcd_0_resource
};

#endif /* XPAR_TFT_0_BASEADDR */

#ifdef XPAR_SYSACE_0_BASEADDR

static struct platform_device xilinx_sysace_0_device = {
	.name = "xilinx_sysace",
	.id = 0,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_SYSACE_0_BASEADDR,
			.end	= XPAR_SYSACE_0_HIGHADDR,
			.flags	= IORESOURCE_MEM
		},
		{
			.start	= XPAR_INTC_0_SYSACE_0_VEC_ID,
			.end	= XPAR_INTC_0_SYSACE_0_VEC_ID,
			.flags	= IORESOURCE_IRQ
		}
	}
};

#endif /* XPAR_SYSACE_0_BASEADDR */

#ifdef XPAR_IIC_0_BASEADDR

static struct platform_device xilinx_iic_0_device = {
	.name = "xilinx_iic",
	.id = 0,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_IIC_0_BASEADDR,
			.end	= XPAR_IIC_0_HIGHADDR,
			.flags	= IORESOURCE_MEM
		},
		{
			.start	= XPAR_INTC_0_IIC_0_VEC_ID,
			.end	= XPAR_INTC_0_IIC_0_VEC_ID,
			.flags	= IORESOURCE_IRQ
		}
	}
};

#endif /* XPAR_IIC_0_BASEADDR */

#ifdef XPAR_GPIO_0_BASEADDR

static struct platform_device xilinx_gpio_0_device = {
	.name = "xilinx_gpio",
	.id = XPAR_GPIO_0_DEVICE_ID,
	.dev.platform_data = (XPAR_GPIO_0_IS_DUAL ? XGPIO_IS_DUAL : 0),
	.num_resources = 1,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_GPIO_0_BASEADDR,
			.end	= XPAR_GPIO_0_HIGHADDR,
			.flags	= IORESOURCE_MEM
		}
	}
};

#endif /* XPAR_GPIO_0_BASEADDR */

#ifdef XPAR_GPIO_1_BASEADDR

static struct platform_device xilinx_gpio_1_device = {
	.name = "xilinx_gpio",
	.id = XPAR_GPIO_1_DEVICE_ID,
	.dev.platform_data = (XPAR_GPIO_1_IS_DUAL ? XGPIO_IS_DUAL : 0),
	.num_resources = 1,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_GPIO_1_BASEADDR,
			.end	= XPAR_GPIO_1_HIGHADDR,
			.flags	= IORESOURCE_MEM
		}
	}
};

#endif /* XPAR_GPIO_1_BASEADDR */

#ifdef XPAR_GPIO_2_BASEADDR

static struct platform_device xilinx_gpio_2_device = {
	.name = "xilinx_gpio",
	.id = XPAR_GPIO_2_DEVICE_ID,
	.dev.platform_data = (XPAR_GPIO_2_IS_DUAL ? XGPIO_IS_DUAL : 0),
	.num_resources = 1,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_GPIO_2_BASEADDR,
			.end	= XPAR_GPIO_2_HIGHADDR,
			.flags	= IORESOURCE_MEM
		}
	}
};

#endif /* XPAR_GPIO_2_BASEADDR */

#ifdef XPAR_GPIO_3_BASEADDR

static struct platform_device xilinx_gpio_3_device = {
	.name = "xilinx_gpio",
	.id = XPAR_GPIO_3_DEVICE_ID,
	.dev.platform_data = (XPAR_GPIO_3_IS_DUAL ? XGPIO_IS_DUAL : 0),
	.num_resources = 1,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_GPIO_3_BASEADDR,
			.end	= XPAR_GPIO_3_HIGHADDR,
			.flags	= IORESOURCE_MEM
		}
	}
};

#endif /* XPAR_GPIO_3_BASEADDR */

#ifdef XPAR_GPIO_4_BASEADDR

static struct platform_device xilinx_gpio_4_device = {
	.name = "xilinx_gpio",
	.id = XPAR_GPIO_4_DEVICE_ID,
	.dev.platform_data = (XPAR_GPIO_4_IS_DUAL ? XGPIO_IS_DUAL : 0),
	.num_resources = 1,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_GPIO_4_BASEADDR,
			.end	= XPAR_GPIO_4_HIGHADDR,
			.flags	= IORESOURCE_MEM
		}
	}
};

#endif /* XPAR_GPIO_4_BASEADDR */

#ifdef XPAR_OPB_LCD_INTERFACE_0_BASEADDR

static struct platform_device xilinx_char_lcd_device = {
	.name = "xilinx_char_lcd",
	.id = 0,
	.num_resources = 1,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_OPB_LCD_INTERFACE_0_BASEADDR,
			.end	= XPAR_OPB_LCD_INTERFACE_0_HIGHADDR,
			.flags	= IORESOURCE_MEM
		}
	}
};

#endif /* XPAR_OPB_LCD_INTERFACE_0_BASEADDR */

#ifdef XPAR_TOUCHSCREEN_0_BASEADDR

static struct platform_device xilinx_touchscreen_device = {
	.name = "xilinx_ts",
	.id = 0,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_TOUCHSCREEN_0_BASEADDR,
			.end	= XPAR_TOUCHSCREEN_0_HIGHADDR,
			.flags	= IORESOURCE_MEM
		},
		{
			.start	= XPAR_INTC_0_TOUCHSCREEN_0_VEC_ID,
			.end	= XPAR_INTC_0_TOUCHSCREEN_0_VEC_ID,
			.flags	= IORESOURCE_IRQ
		}
	}
};

#endif /* XPAR_TOUCHSCREEN_0_BASEADDR */

#ifdef XPAR_SPI_0_BASEADDR

static struct xspi_platform_data xspi_0_pdata = {
	.device_flags = (XPAR_SPI_0_FIFO_EXIST ? XSPI_HAS_FIFOS : 0) |
		(XPAR_SPI_0_SPI_SLAVE_ONLY ? XSPI_SLAVE_ONLY : 0),
	.num_slave_bits = XPAR_SPI_0_NUM_SS_BITS
};

static struct platform_device xilinx_spi_0_device = {
	.name = "xilinx_spi",
	.id = XPAR_SPI_0_DEVICE_ID,
	.dev.platform_data = &xspi_0_pdata,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_SPI_0_BASEADDR,
			.end	= XPAR_SPI_0_HIGHADDR,
			.flags	= IORESOURCE_MEM
		},
		{
			.start	= XPAR_INTC_0_SPI_0_VEC_ID,
			.end	= XPAR_INTC_0_SPI_0_VEC_ID,
			.flags	= IORESOURCE_IRQ
		}
	}
};

#endif /* XPAR_SPI_0_BASEADDR */

#ifdef XPAR_USB_0_BASEADDR

static struct platform_device xilinx_usb_0_device = {
	.name = "xlnx_udc",
	.id = XPAR_USB_0_DEVICE_ID,
	.num_resources = 2,
	.resource = (struct resource[]) {
		{
			.start	= XPAR_USB_0_BASEADDR,
			.end	= XPAR_USB_0_HIGHADDR,
			.flags	= IORESOURCE_MEM
		},
		{
			.start	= XPAR_INTC_0_USB_0_VEC_ID,
			.end	= XPAR_INTC_0_USB_0_VEC_ID,
			.flags	= IORESOURCE_IRQ
		}
	}
};

#endif /* XPAR_USB_0_BASEADDR */

static int __init xilinx_platform_init(void)
{
#ifdef XPAR_EMAC_0_BASEADDR
	memcpy(xemac_0_pdata.mac_addr, __res.bi_enetaddr, 6);
	platform_device_register(&xilinx_emac_0_device);
#endif /* XPAR_EMAC_0_BASEADDR */
#ifdef XPAR_EMAC_1_BASEADDR
	memcpy(xemac_1_pdata.mac_addr, __res.bi_enetaddr, 6);
	platform_device_register(&xilinx_emac_1_device);
#endif /* XPAR_EMAC_1_BASEADDR */

#ifdef XPAR_TEMAC_0_BASEADDR
	memcpy(xtemac_0_pdata.mac_addr, __res.bi_enetaddr, 6);
	platform_device_register(&xilinx_temac_0_device);
#endif /* XPAR_TEMAC_0_BASEADDR */
#ifdef XPAR_TEMAC_1_BASEADDR
	memcpy(xtemac_1_pdata.mac_addr, __res.bi_enetaddr, 6);
	platform_device_register(&xilinx_temac_1_device);
#endif /* XPAR_TEMAC_1_BASEADDR */

#ifdef XPAR_LLTEMAC_0_BASEADDR
	memcpy(xlltemac_0_pdata.mac_addr, __res.bi_enetaddr, 6);
	platform_device_register(&xilinx_lltemac_0_device);
#endif /* XPAR_LLTEMAC_0_BASEADDR */
#ifdef XPAR_LLTEMAC_1_BASEADDR
	memcpy(xlltemac_1_pdata.mac_addr, __res.bi_enetaddr, 6);
	platform_device_register(&xilinx_lltemac_1_device);
#endif /* XPAR_LLTEMAC_1_BASEADDR */

#ifdef XPAR_TFT_0_BASEADDR
	platform_device_register(&xilinx_lcd_0_device);
#endif /* XPAR_TFT_0_BASEADDR */

#ifdef XPAR_SYSACE_0_BASEADDR
	platform_device_register(&xilinx_sysace_0_device);
#endif /* XPAR_SYSACE_0_BASEADDR */

#ifdef XPAR_IIC_0_BASEADDR
	platform_device_register(&xilinx_iic_0_device);
#endif /* XPAR_IIC_0_BASEADDR */

#ifdef XPAR_GPIO_0_BASEADDR
	platform_device_register(&xilinx_gpio_0_device);
#endif /* XPAR_GPIO_0_BASEADDR */
#ifdef XPAR_GPIO_1_BASEADDR
	platform_device_register(&xilinx_gpio_1_device);
#endif /* XPAR_GPIO_1_BASEADDR */
#ifdef XPAR_GPIO_2_BASEADDR
	platform_device_register(&xilinx_gpio_2_device);
#endif /* XPAR_GPIO_2_BASEADDR */
#ifdef XPAR_GPIO_3_BASEADDR
	platform_device_register(&xilinx_gpio_3_device);
#endif /* XPAR_GPIO_3_BASEADDR */
#ifdef XPAR_GPIO_4_BASEADDR
	platform_device_register(&xilinx_gpio_4_device);
#endif /* XPAR_GPIO_4_BASEADDR */

#ifdef XPAR_PS2_0_BASEADDR
	platform_device_register(&xilinx_ps2_0_device);
#endif /* XPAR_PS2_0_BASEADDR */
#ifdef XPAR_PS2_1_BASEADDR
	platform_device_register(&xilinx_ps2_1_device);
#endif /* XPAR_PS2_1_BASEADDR */

#ifdef XPAR_TOUCHSCREEN_0_BASEADDR
	platform_device_register(&xilinx_touchscreen_device);
#endif /* XPAR_TOUCHSCREEN_0_BASEADDR */

#ifdef XPAR_SPI_0_BASEADDR
	platform_device_register(&xilinx_spi_0_device);
#endif /* XPAR_SPI_0_BASEADDR */

#ifdef XPAR_OPB_LCD_INTERFACE_0_BASEADDR
	platform_device_register(&xilinx_char_lcd_device);
#endif /* XPAR_OPB_LCD_INTERFACE_0_BASEADDR */

#ifdef XPAR_USB_0_BASEADDR
	platform_device_register(&xilinx_usb_0_device);
#endif /* XPAR_USB_0_BASEADDR */


	return 0;
}

subsys_initcall(xilinx_platform_init);

