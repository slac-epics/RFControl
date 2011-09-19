/****************************************************
 * RFControl_requiredInterface_fwControlVirtual.h
 * 
 * Virtual function definition for the firmware control. The motivation to make this is because we will have different 
 *   version of firmware/board to be supported by this module. The virtual functions will allow us to reuse the low level
 *   application layer (pulse-pulse phase control) and adapt to different firmware. This is a common interface definition.
 *   And because we are using C instead of C++, function pointers will be used to replace the virtual functions in C++.
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.07.07
 * Description: Initial creation
 ****************************************************/
#ifndef RF_CONTROL_REQUIRED_INTERFACE_FW_CONTROL_VIRTUAL_H
#define RF_CONTROL_REQUIRED_INTERFACE_FW_CONTROL_VIRTUAL_H

#include "FWControl_sis8300_desy_iqfb.h"                                                    /* support codes for the sis8300 board, desy platform firmware, iq feedback application firmware (SIS8300:DESY:IQFB) */
#include "FWControl_sis8300_desy_iqfb_upLink.h"

#include "FWControl_sis8300_struck_iqfb.h"                                                  /* support codes for the sis8300 board, struck platform firmware, iq feedback application firmware (SIS8300:STRUCK:IQFB) */
#include "FWControl_sis8300_struck_iqfb_upLink.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Function pointer definitions 
 */
typedef int (*RFC_FUNCPTR_INIT)(void*);                                                    /* init the firmware control module */

typedef int (*RFC_FUNCPTR_CREATE_EPICS_DATA)(void*, const char*);                          /* create the internal data nodes for the firmware control */
typedef int (*RFC_FUNCPTR_DELETE_EPICS_DATA)(void*, const char*);                          /* delete */

typedef void* (*RFC_FUNCPTR_GET_BOARD_HANDLE)(const char*);                                /* get the board handle */
typedef int (*RFC_FUNCPTR_ASS_BOARD_HANDLE)(void*, void*);                                 /* associate the board handle with the firmware module */

typedef int (*RFC_FUNCPTR_GET_MAX_SAMPLE_NUM)(long*);                                      /* get the maximum sample number supported by the firmware */

typedef int (*RFC_FUNCPTR_GET_DAQ_DATA)(void*);                                                         /* get all DAQ data (saved in the firmware control module, because different firmware needs different buffer size) */
typedef int (*RFC_FUNCPTR_GET_ADC_DATA)(void*, unsigned long, short*, double*, double*, long*, long*);  /* get the ADC channel raw data (channel number, buffer, sample frequency (MHz), sample delay(ns), point number) */
typedef int (*RFC_FUNCPTR_GET_INT_DATA)(void*);                                                         /* get the internal data of the firmware module, we make this because it might not be at the same rate with get DAQ data */

typedef int (*RFC_FUNCPTR_SET_PHA_DEG)(void*, double);                                     /* set the phase in degree */
typedef int (*RFC_FUNCPTR_SET_AMP)(void*, double);                                         /* set the amplitude */

typedef int (*RFC_FUNCPTR_WAIT_INTR)(void*);                                               /* wait interrupt */

typedef int (*RFC_FUNCPTR_MEA_INTR_LATENCY)(void*, long*, long*);                          /* measure the interrupt latency */

/**
 * Structure of the virtual functions
 */
typedef struct {

    RFC_FUNCPTR_INIT                FWC_func_init;

    RFC_FUNCPTR_CREATE_EPICS_DATA   FWC_func_createEpicsData;
    RFC_FUNCPTR_DELETE_EPICS_DATA   FWC_func_deleteEpicsData;

    RFC_FUNCPTR_GET_BOARD_HANDLE    FWC_func_getBoardHandle;
    RFC_FUNCPTR_ASS_BOARD_HANDLE    FWC_func_assBoardHandle;

    RFC_FUNCPTR_GET_MAX_SAMPLE_NUM  FWC_func_getMaxSampleNum;

    RFC_FUNCPTR_GET_DAQ_DATA        FWC_func_getDAQData;
    RFC_FUNCPTR_GET_ADC_DATA        FWC_func_getADCData;
    RFC_FUNCPTR_GET_INT_DATA        FWC_func_getIntData;

    RFC_FUNCPTR_SET_PHA_DEG         FWC_func_setPha_deg;
    RFC_FUNCPTR_SET_AMP             FWC_func_setAmp;

    RFC_FUNCPTR_WAIT_INTR           FWC_func_waitIntr;

    RFC_FUNCPTR_MEA_INTR_LATENCY    FWC_func_meaIntrLatency;

} RFC_struc_fwAccessFunc;

#ifdef __cplusplus
}
#endif

#endif


