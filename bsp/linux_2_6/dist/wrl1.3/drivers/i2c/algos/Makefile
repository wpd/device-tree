#
# Makefile for the i2c algorithms
#

obj-$(CONFIG_I2C_ALGOBIT)	+= i2c-algo-bit.o
obj-$(CONFIG_I2C_ALGOPCF)	+= i2c-algo-pcf.o
obj-$(CONFIG_I2C_ALGOPCA)	+= i2c-algo-pca.o
obj-$(CONFIG_I2C_ALGOITE)	+= i2c-algo-ite.o
obj-$(CONFIG_I2C_ALGO_SIBYTE)	+= i2c-algo-sibyte.o
obj-$(CONFIG_I2C_ALGO_SGI)	+= i2c-algo-sgi.o
obj-$(CONFIG_I2C_ALGOMV64360)	+= i2c-algo-mv64360.o
obj-$(CONFIG_AT24CXX_EEPROM)	+= at24cxx.o

#
# These were commented out, but that didn't work, so I put these lines back in.
#
obj-$(CONFIG_I2C_ALGO_XILINX)	+= xilinx_iic/xiic.o  xilinx_iic/i2c-algo-xilinx.o \
				xilinx_iic/xiic_l.o  xilinx_iic/xiic_intr.o \
				xilinx_iic/xiic_master.o  xilinx_iic/xiic_options.o

ifeq ($(CONFIG_I2C_ALGO_XILINX),y)
mod-subdirs				+= xilinx_iic
subdir-$(CONFIG_I2C_ALGO_XILINX)	+= xilinx_iic
endif

ifeq ($(CONFIG_I2C_DEBUG_ALGO),y)
EXTRA_CFLAGS += -DDEBUG
endif
