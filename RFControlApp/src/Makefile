##########################################################
# Makefile
# 
# Makefile for the RF Control module (LLA)
# 
# Created by: Zheqiao Geng, gengzq@slac.stanford.edu
# Created on: 2011.05.12
# Description: Initial creation 
##########################################################
TOP=../..

include $(TOP)/configure/CONFIG

#------------------------------------------------
# build the library for RFControl
#------------------------------------------------
# ---- library name ----
LIBRARY_IOC = RFControl

# ---- sub directory ----
FWC_SIS8300_DESY_IQFB   = $(TOP)/RFControlApp/src/FWControl_sis8300_desy_iqfb
FWC_SIS8300_STRUCK_IQFB = $(TOP)/RFControlApp/src/FWControl_sis8300_struck_iqfb

SRC_DIRS += $(FWC_SIS8300_DESY_IQFB)
SRC_DIRS += $(FWC_SIS8300_STRUCK_IQFB)

USR_DBDFLAGS += -I $(FWC_SIS8300_DESY_IQFB)
USR_DBDFLAGS += -I $(FWC_SIS8300_STRUCK_IQFB)

# ---- ported include files ----
INC += RFControl_main.h
INC += RFControl_requiredInterface_fastFeedback.h
INC += RFControl_availableInterface_api.h
INC += RFControl_availableInterface_upLink.h
INC += RFControl_requiredInterface_fwControlVirtual.h
INC += FWControl_sis8300_desy_iqfb.h
INC += FWControl_sis8300_desy_iqfb_board.h
INC += FWControl_sis8300_desy_iqfb_upLink.h
INC += addrMap_sis8300_desy_iqfb_platform.h
INC += addrMap_sis8300_desy_iqfb_application.h
INC += FWControl_sis8300_struck_iqfb.h
INC += FWControl_sis8300_struck_iqfb_board.h
INC += FWControl_sis8300_struck_iqfb_upLink.h
INC += addrMap_sis8300_struck_iqfb.h
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
RFControl_SRCS += FWControl_sis8300_desy_iqfb.c
RFControl_SRCS += FWControl_sis8300_desy_iqfb_board.c
RFControl_SRCS += FWControl_sis8300_desy_iqfb_upLink.c
RFControl_SRCS += FWControl_sis8300_struck_iqfb.c
RFControl_SRCS += FWControl_sis8300_struck_iqfb_board.c
RFControl_SRCS += FWControl_sis8300_struck_iqfb_upLink.c
RFControl_SRCS += syncDAQ.c

# ---- finally link to the EPICS Base libraries ----
RFControl_LIBS += $(EPICS_BASE_IOC_LIBS)

include $(TOP)/configure/RULES
















