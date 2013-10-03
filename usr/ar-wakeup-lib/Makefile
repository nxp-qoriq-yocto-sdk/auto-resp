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


# Set tool names
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
CPP = $(CROSS_COMPILE)cpp
NM = $(CROSS_COMPILE)nm
STRIP = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
AR = $(CROSS_COMPILE)ar
RM = rm

# These are defaults.  Individual makefiles might override them.
CLIBS = -L $(LIBPDKDIR)/lib -lpdk
CFLAGS = -g -O -Wall

SRCS = ar_wakeup_tool.c
EXECS = $(SRCS:%.c=%)
OBJS = $(SRCS:%.c=%.o)
DEPS = $(SRCS:%.c=%.d)

all:	$(EXECS)

execs:	$(EXECS)

%.d:	%.c
	$(CC) $(DEBUG_FLAGS) $(TARGET_FLAGS) $(CFLAGS) -c -MD $<

clean:
	$(RM) -f $(EXECS) $(OBJS) $(DEPS) *.d *.a *.tar

tar:	clean
	$(RM) -rf *~
	tar cvf msc7120_app.tar *

-include $(DEPS)

