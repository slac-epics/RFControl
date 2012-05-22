/****************************************************
 * syncDAQ.h
 * 
 * Header file for the synchronized data aquisition. 
 *   This is a simplified version of BSA
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanforde.edu
 * Created on: 2011.07.13
 * Description: Initial creation
 ****************************************************/
#ifndef SYNC_DAQ_H
#define SYNC_DAQ_H

#define SDAQ_CONST_BUF_SIZE             65536               /* 64K points supported in single value DAQ buffer */
#define SDAQ_CONST_WF_PNO_SUPPORTED     1024                /* support 512 point waveforms */
#define SDAQ_CONST_WF_NUM_SUPPORTED     2048                /* maximum 2048 waveforms can be saved */

#include "MathLib_dataProcess.h"
#include "EPICSLib_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

/*======================================
 * Data structure
 *======================================*/
/**
 * Data structure for SDAQ single value data
 *   now only double data is supported
 */
typedef struct {
    EPICSLIB_type_linkedListNode node;                      /* to fit this structure to linked list */
    volatile double *dataPtr;                               /* pointer to the data */
    double buf[SDAQ_CONST_BUF_SIZE];                        /* buffer for the data */
} SDAQ_struc_dataNode;

/**
 * Data structure for waveforms
 *   now only short waveform is supported
 */
typedef struct {
    EPICSLIB_type_linkedListNode node;                                      /* to fit this structure to linked list */
    volatile short *dataPtr;                                                /* pointer to the data */
    int pno;                                                                /* real point number */
    short buf[SDAQ_CONST_WF_PNO_SUPPORTED * SDAQ_CONST_WF_NUM_SUPPORTED];   /* buffer for the data */
} SDAQ_struc_wfNode;

/*======================================
 * Routines
 *======================================*/     
int SDAQ_func_createDataNode(double *dataPtr);
int SDAQ_func_createWfNode(short *dataPtr, int pno);

int SDAQ_func_saveData(int dataId, char *nameStr);
int SDAQ_func_saveWf(int wfId, char *nameStr);

#ifdef __cplusplus
}
#endif

#endif


