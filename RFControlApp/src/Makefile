##########################################################
# Makefile
# 
# Makefile for the RF Control module (LLA)
# 
# Created by: Zheqiao Geng, gengzq@slac.stanford.edu
# Created on: 2011.05.12
# Description: Initial creation 
#
# Modified by: Zheqiao Geng
# Modified on: 2/13/2013
# Description: Remove the firmware control part and use the new RFControlFirmware module
##########################################################
TOP=../..

include $(TOP)/configure/CONFIG

#------------------------------------------------
# build the library for RFControl
#------------------------------------------------
# ---- library name ----
LIBRARY_IOC = RFControl

# ---- ported include files ----
INC += RFControl_main.h
INC += RFControl_requiredInterface_fastFeedback.h
INC += RFControl_availableInterface_api.h
INC += RFControl_availableInterface_upLink.h
INC += syncDAQ.h

# ---- library database definition files (including record type definitions and all registerations) ----
DBD += RFControl.dbd
RFControl_DBD += RFControl_iocShell.dbd

# ---- private source files ----
RFControl_SRCS += RFControl_main.c
RFControl_SRCS += RFControl_requiredInterface_fastFeedback.c
RFControl_SRCS += RFControl_availableInterface_api.c
RFControl_SRCS += RFControl_availableInterface_upLink.c
RFControl_SRCS += RFControl_iocShell.c
RFControl_SRCS += syncDAQ.c

# ---- finally link to the EPICS Base libraries ----
RFControl_LIBS += $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES
















