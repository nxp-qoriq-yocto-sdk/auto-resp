#/**************************************************************************
# * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
# ***************************************************************************/
#/*
# * File:       Makefile
# *
# * Description: Top level file for application & API compilation
# * *
# * Authors:    Sunil Kumar Kori <b42948@freescale.com>
# */
# /* History:
# */
# ***************************************************************************/

.PHONY: all

all:
	make -w -C lib-api -f Makefile
	make -w -C lib-tool -f Makefile
#--------------------------------------------------------------
.PHONY: clean

clean:
	make -w -C lib-api -f Makefile clean
	make -w -C lib-tool -f Makefile clean
