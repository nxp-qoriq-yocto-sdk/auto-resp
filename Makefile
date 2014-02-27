#/**************************************************************************
# * Copyright 2014, Freescale Semiconductor, Inc. All rights reserved.
# ***************************************************************************/
#/*
# * File:	Makefile
# *
# */

TOPDIR := $(shell pwd)
export TOPDIR

#------------------------------------------------------------------------------
#  Include Definitions
#------------------------------------------------------------------------------
.PHONY: all
.NOTPARALLEL :
all:
	mkdir -p bin
	make -w -C armodule -f Makefile
	make -w -C usr/ar-snmp-lib -f Makefile
	make -w -C usr/ar-wakeup-lib -f Makefile
	$(CROSS_COMPILE)strip --strip-unneeded bin/*.ko

.PHONY: clean
clean:
	make -w -C armodule -f Makefile clean
	make -w -C usr/ar-wakeup-lib -f Makefile clean
	make -w -C usr/ar-snmp-lib -f Makefile clean
	rm -rf bin
