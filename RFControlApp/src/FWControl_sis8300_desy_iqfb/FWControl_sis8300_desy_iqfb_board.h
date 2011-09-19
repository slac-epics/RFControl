/****************************************************
 * FWControl_sis8300_desy_iqfb_board.h
 * 
 * This is the interface with the the RFControlBoard module
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.07.07
 * Description: Initial creation
 ****************************************************/
#ifndef FW_CONTROL_SIS8300_DESY_IQFB_BOARD_H
#define FW_CONTROL_SIS8300_DESY_IQFB_BOARD_H

#include "RFControlBoard_availableInterface.h"                                          /* only point to interact with the RFControlBoard module */

#include "addrMap_sis8300_desy_iqfb_platform.h"                                         /* use the address map here */
#include "addrMap_sis8300_desy_iqfb_application.h"

#include "RFLib_signalProcess.h"

/**
 * Constants for board access 
 */
#define FWC_SIS8300_DESY_IQFB_CONST_ROT_COEF_FRACTION 15                                                  /* fraction bits number of the rotation coefficients */
#define FWC_SIS8300_DESY_IQFB_CONST_GAIN_FRACTION     15                                                  /* fraction bits number of the feedback gain */

#define FWC_SIS8300_DESY_IQFB_CONST_TAB_BUF_DEPTH     CON_MEA_ROT_TAB_DEPTH                               /* buffer length for writing to FPGA */
#define FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH     CON_DAQ_BUF_DEPTH                                   /* buffer length for a DAQ channel */
#define FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_NUM       CON_DAQ_BUF_NUM                                     /* DAQ buffer number */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Interface functions
 */
/* --- common --- */
void *FWC_sis8300_desy_iqfb_func_getBoardHandle(const char *boardModuleName);                                  /* get the handle of the RF Control Board module */

#define FWC_sis8300_desy_iqfb_func_pullInterrupt(boardHandle) RFCB_API_pullInterrupt((RFCB_struc_moduleData *)(boardHandle))      /* pull the interrupt */

/* --- for platform firmware --- */
void FWC_sis8300_desy_iqfb_func_getBoardInfo(void *boardHandle, unsigned int *firmwareName, unsigned int *majorVer, 
                           unsigned int *minorVer, unsigned int *buildNum, unsigned int *serialNum);
                           
void FWC_sis8300_desy_iqfb_func_getBoardStat(void *boardHandle, unsigned int *watchDogCnt, unsigned int *alarmVector, 
                           unsigned int *statusVector, unsigned int *ad9510Status, unsigned int *ADCStatus);

void FWC_sis8300_desy_iqfb_func_setHarlinkOutSource(void *boardHandle, unsigned int src);
void FWC_sis8300_desy_iqfb_func_setAMCLVDSOutSource(void *boardHandle, unsigned int src);

void FWC_sis8300_desy_iqfb_func_setHarlinkOutData(void *boardHandle, unsigned int data);
void FWC_sis8300_desy_iqfb_func_setAMCLVDSOutData(void *boardHandle, unsigned int data);

void FWC_sis8300_desy_iqfb_func_getHarlinkInData(void *boardHandle, unsigned int *data);
void FWC_sis8300_desy_iqfb_func_getAMCLVDSInData(void *boardHandle, unsigned int *data);

void FWC_sis8300_desy_iqfb_func_setADCClockSource(void *boardHandle, unsigned int src);
void FWC_sis8300_desy_iqfb_func_setExtTriggerSource(void *boardHandle, unsigned int src, unsigned int adcThreshold);
void FWC_sis8300_desy_iqfb_func_setDACDataSource(void *boardHandle, unsigned int src);

void FWC_sis8300_desy_iqfb_func_setDMAStartAddr(void *boardHandle, unsigned int addr);

void FWC_sis8300_desy_iqfb_func_setSPI(void *boardHandle);        /* set the clock divider, ADC chips and DAC chips (with fixed value,later can make it changable) */

/* --- for LLRF controller firmware --- */
__inline__ void  FWC_sis8300_desy_iqfb_func_setBits(void *boardHandle, unsigned int data);                                /* set the register for bits ... */

__inline__ void  FWC_sis8300_desy_iqfb_func_setExtTrigDelay(void *boardHandle, double value_ns, double freq_MHz);         /* set the external trigger delay */
__inline__ void  FWC_sis8300_desy_iqfb_func_setIntTrigPeriod(void *boardHandle, double value_ns, double freq_MHz);        /* set the internal trigger period */
__inline__ void  FWC_sis8300_desy_iqfb_func_setIntTrigLength(void *boardHandle, double value_ns, double freq_MHz);        /* set the internal trigger length */
__inline__ void  FWC_sis8300_desy_iqfb_func_setRFPulseLength(void *boardHandle, double value_ns, double freq_MHz);        /* set the RF pulse length */

__inline__ void  FWC_sis8300_desy_iqfb_func_setDAQTrigDelay(void *boardHandle, double value_ns, double freq_MHz);         /* set the DAQ trigger delay (relative the global trigger) */
__inline__ void  FWC_sis8300_desy_iqfb_func_setDAQChSel(void *boardHandle, unsigned int channel, unsigned int sel);       /* set the DAQ channel data selection */

__inline__ void  FWC_sis8300_desy_iqfb_func_setADCNoSignalThreshold(void *boardHandle, unsigned int value);               /* threshold to detect that ADC has no signal input */

__inline__ void  FWC_sis8300_desy_iqfb_func_selectRefFbkChannel(void *boardHandle, unsigned int refCh, unsigned int fbkCh);  /* select the reference and feedback channel */

__inline__ void  FWC_sis8300_desy_iqfb_func_setRefVectorRotation(void *boardHandle, double scale, double rotAngle_deg);   /* scale and rotate the reference signal */
__inline__ void  FWC_sis8300_desy_iqfb_func_setFbkVectorRotation(void *boardHandle, double scale, double rotAngle_deg);   /* scale and rotate the feedback signal */

__inline__ void  FWC_sis8300_desy_iqfb_func_setSetPoint_I(void *boardHandle, double value_MV, double factor);             /* set the set point value for I component */
__inline__ void  FWC_sis8300_desy_iqfb_func_setSetPoint_Q(void *boardHandle, double value_MV, double factor);             /* set the set point value for Q component */

__inline__ void  FWC_sis8300_desy_iqfb_func_setFeedforward_I(void *boardHandle, double value_MV, double factor);          /* set the feedforward value for I component */
__inline__ void  FWC_sis8300_desy_iqfb_func_setFeedforward_Q(void *boardHandle, double value_MV, double factor);          /* set the feedforward value for Q component */

__inline__ void  FWC_sis8300_desy_iqfb_func_setGain_I(void *boardHandle, double value);                                   /* set the gain value for I component */
__inline__ void  FWC_sis8300_desy_iqfb_func_setGain_Q(void *boardHandle, double value);                                   /* set the gain value for Q component */

__inline__ void  FWC_sis8300_desy_iqfb_func_setIntgStart(void *boardHandle, double value_ns, double freq_MHz);            /* start time for integration, relative to the trigger */
__inline__ void  FWC_sis8300_desy_iqfb_func_setIntgEnd(void *boardHandle, double value_ns, double freq_MHz);             

__inline__ void  FWC_sis8300_desy_iqfb_func_setApplStart(void *boardHandle, double value_ns, double freq_MHz);            /* start time for applying the correction, relative to the trigger */
__inline__ void  FWC_sis8300_desy_iqfb_func_setApplEnd(void *boardHandle, double value_ns, double freq_MHz);             

__inline__ void  FWC_sis8300_desy_iqfb_func_setActVectorRotation(void *boardHandle, double scale, double rotAngle_deg);   /* scale and rotate the actuation signal */

__inline__ void  FWC_sis8300_desy_iqfb_func_setDACOffset_I(void *boardHandle, unsigned int offset);                       /* set the DAC offset */
__inline__ void  FWC_sis8300_desy_iqfb_func_setDACOffset_Q(void *boardHandle, unsigned int offset);

__inline__ void  FWC_sis8300_desy_iqfb_func_setAmpLimitHi(void *boardHandle, unsigned int limit);                         /* set the output limit */
__inline__ void  FWC_sis8300_desy_iqfb_func_setAmpLimitLo(void *boardHandle, unsigned int limit);

__inline__ void  FWC_sis8300_desy_iqfb_func_setMeaRotationTable(void *boardHandle, unsigned int pno, double *scaleTable, double *rotAngleTable_deg, unsigned int *data);      /* set the measurement chain rotation table */
__inline__ void  FWC_sis8300_desy_iqfb_func_setDrvRotationTable(void *boardHandle, unsigned int pno, double *scaleTable, double *rotAngleTable_deg, unsigned int *data);      /* set the driving chain rotation table */

__inline__ void  FWC_sis8300_desy_iqfb_func_getAllDAQData(void *boardHandle, unsigned int *buf);

#ifdef __cplusplus
}
#endif

#endif


