#
# Makefile for the Xilinx USB driver
#

EXTRA_CFLAGS	+= -I$(TOPDIR)/drivers/xilinx_common
EXTRA_CFLAGS	+= -I$(TOPDIR)/drivers/usb/gadget/
EXTRA_CFLAGS	+= -Idrivers/xilinx_common -Iarch/ppc/platforms/4xx/xparameters


# The Linux peripheral controller driver for the Xilinx USB driver code.

obj-$(CONFIG_USB_GADGET_XILINX)	+= g_xlnx_usb_gadget.o

# The Xilinx OS independent code.
g_xlnx_usb_gadget-objs		:= xlnx_udc.o xusb.o xusb_cp9.o    						xusb_intr.o xusb_endpoint.o
