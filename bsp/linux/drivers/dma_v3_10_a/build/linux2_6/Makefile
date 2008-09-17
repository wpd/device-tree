# Linux file to EXPORT_SYMBOL all of the Xilinx entries.
obj-$(CONFIG_XILINX_EDK) := xilinx_syms.o

# The Xilinx OS independent code.
obj-$(CONFIG_XILINX_EDK) += xbasic_types.o  xipif_v1_23_b.o \
			 xpacket_fifo_v2_10_a.o xpacket_fifo_l_v2_10_a.o \
			 xdma_channel.o xdma_channel_sg.o \
			 xdmav2.o xdmav2_intr.o xdmav2_sg.o xdmav2_selftest.o \
			 xdmav3.o xdmav3_intr.o xdmav3_sg.o xdmav3_selftest.o \
			 xversion.o
