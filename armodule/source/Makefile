#/**************************************************************************
# * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
# ***************************************************************************/
#/*
# * File:	Makefile
# *
# */

# Required include definitions
include $(KERNEL_PATH)/.config
include $(KERNEL_PATH)/drivers/net/ethernet/freescale/fman/ncsw_config.mk

#Required extra flags
EXTRA_CFLAGS += -I$(KERNEL_PATH)/drivers/net/ethernet/freescale/fman/inc/
EXTRA_CFLAGS += -I$(KERNEL_PATH)/drivers/net/ethernet/freescale/dpa/
EXTRA_CFLAGS += -I$(KERNEL_PATH)/drivers/net/ethernet/freescale/
EXTRA_CFLAGS +=	-I$(KERNEL_PATH)/arch/powerpc/include/asm
EXTRA_CFLAGS += -I$(KERNEL_PATH)/net/bridge/
EXTRA_CFLAGS += -I$(KERNEL_PATH)/net/
EXTRA_CFLAGS += -I$(PWD)/include/

#Debug Flag
#EXTRA_CFLAGS += -DAR_DEBUG
#EXTRA_CFLAGS += -DAR_DEBUG_API

#Required Variables
CURDIR = $(PWD)/source

obj-m += ar.o

ar-y := arctrl_core.o\
     	arctrl_fmd.o\
	arctrl_proc.o\
	arctrl_pm.o\
	arctrl_snmp.o\
	arctrl_snmptool.o\
	arctrl_sysfs.o\
	arctrl_wakeup.o\
	arctrl_wakeuptool.o

all:
	make ARCH=powerpc -C $(KERNEL_PATH) M=$(CURDIR) modules

clean:
	make ARCH=powerpc -C $(KERNEL_PATH) M=$(CURDIR) clean
