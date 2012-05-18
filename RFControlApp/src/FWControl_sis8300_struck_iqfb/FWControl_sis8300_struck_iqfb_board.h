/****************************************************
 * FWControl_sis8300_struck_iqfb_board.h
 * 
 * This is the interface with the the RFControlBoard module
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.07.07
 * Description: Initial creation
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2011.08.03
 * Description: Add the access for the firmware of LLRF-SIS8300-R-0-0-1
 ****************************************************/
#ifndef FW_CONTROL_SIS8300_STRUCK_IQFB_BOARD_H
#define FW_CONTROL_SIS8300_STRUCK_IQFB_BOARD_H

#include "RFControlBoard_availableInterface.h"                                          /* only point to interact with the RFControlBoard module */                     
#include "addrMap_sis8300_struck_iqfb.h"                                                /* use the address map here */
#include "RFLib_signalProcess.h"

/**
 * Constants for board access 
 */
#define FWC_SIS8300_STRUCK_IQFB_CONST_ROT_COEF_FRACTION 15                                                  /* fraction bits number of the rotation coefficients */
#define FWC_SIS8300_STRUCK_IQFB_CONST_COR_COEF_FRACTION 14                                                  /* fraction bits number of the coefficients for I/Q modulator imbalance correction */
#define FWC_SIS8300_STRUCK_IQFB_CONST_GAIN_FRACTION     15                                                  /* fraction bits number of the feedback gain */

#define FWC_SIS8300_STRUCK_IQFB_CONST_MEA_TAB_BUF_DEPTH CON_SIS8300_STRUCK_IQFB_MEA_ROT_TAB_DEPTH           /* buffer length for writing to FPGA (for measurement chain rotation) */
#define FWC_SIS8300_STRUCK_IQFB_CONST_DRV_TAB_BUF_DEPTH CON_SIS8300_STRUCK_IQFB_DRV_ROT_TAB_DEPTH           /* buffer length for writing to FPGA (for driving chain rotation) */

#define FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH     CON_SIS8300_STRUCK_IQFB_DAQ_BUF_DEPTH               /* buffer length for a DAQ channel */
#define FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_NUM       CON_SIS8300_STRUCK_IQFB_DAQ_BUF_NUM                 /* DAQ buffer number */

#define FWC_SIS8300_STRUCK_IQFB_CONST_TRIG_DELAY_MAX    CON_SIS8300_STRUCK_IQFB_MAX_TRIG_DELAY              /* maximum trigger delay in clock cycles */
#define FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX    65536                                               /* 64k points (65536) */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Interface functions
 */
/*--------------------------
 * common 
 *-------------------------- */
void *FWC_sis8300_struck_iqfb_func_getBoardHandle(const char *boardModuleName);                                             /* get the handle of the RF Control Board module */
void FWC_sis8300_struck_iqfb_func_getBoardInfo(void *boardHandle, char *deviceName, int *deviceOpened);                     /* get the information of the RF Control Board module */

#define FWC_sis8300_struck_iqfb_func_pullInterrupt(boardHandle) RFCB_API_pullInterrupt((RFCB_struc_moduleData *)(boardHandle))      /* pull the interrupt */

/*--------------------------
 * platform firmware 
 *-------------------------- */                    
void FWC_sis8300_struck_iqfb_func_setSPI(void *boardHandle, unsigned int clkDiv2);                                          /* set the clock divider, ADC chips and DAC chips (with fixed value,later can make it changable) */

void FWC_sis8300_struck_iqfb_func_setHarlink(void *boardHandle, unsigned int data);
void FWC_sis8300_struck_iqfb_func_setAMCLVDS(void *boardHandle, unsigned int data);

void FWC_sis8300_struck_iqfb_func_getHarlink(void *boardHandle, unsigned int *data);
void FWC_sis8300_struck_iqfb_func_getAMCLVDS(void *boardHandle, unsigned int *data);

void FWC_sis8300_struck_iqfb_func_setADCClockSource(void *boardHandle, unsigned int src);

void FWC_sis8300_struck_iqfb_func_getPlatformInfo(void *boardHandle, unsigned int *id, unsigned int *sno);

/*--------------------------
 * LLRF Controller firmware 
 *-------------------------- */
/* Fimrware settings */
__inline__ void  FWC_sis8300_struck_iqfb_func_setBits(void *boardHandle, unsigned int data);                                /* set the register for bits (switch control in the firmware) */

__inline__ void  FWC_sis8300_struck_iqfb_func_setExtTrigDelay(void *boardHandle,  double value_ns, double freq_MHz);        /* set the external trigger delay */
__inline__ void  FWC_sis8300_struck_iqfb_func_setExtTrigPeriod(void *boardHandle, double value_ms, double freq_MHz);        /* set the external trigger period */
__inline__ void  FWC_sis8300_struck_iqfb_func_setIntTrigPeriod(void *boardHandle, double value_ms, double freq_MHz);        /* set the internal trigger period */
__inline__ void  FWC_sis8300_struck_iqfb_func_setIntTrigLength(void *boardHandle, double value_ms, double freq_MHz);        /* set the internal trigger length */
__inline__ void  FWC_sis8300_struck_iqfb_func_setRFPulseLength(void *boardHandle, double value_ns, double freq_MHz);        /* set the RF pulse length */
__inline__ void  FWC_sis8300_struck_iqfb_func_setDAQTrigDelay(void *boardHandle,  double value_ns, double freq_MHz);        /* set the DAQ trigger delay (relative the global trigger) */

__inline__ void  FWC_sis8300_struck_iqfb_func_selectRefFbkChannel(void *boardHandle, unsigned int refCh, unsigned int fbkCh);  /* select the reference and feedback channel */

__inline__ void  FWC_sis8300_struck_iqfb_func_setRefVectorRotation(void *boardHandle, double scale, double rotAngle_deg);   /* scale and rotate the reference signal */
__inline__ void  FWC_sis8300_struck_iqfb_func_setFbkVectorRotation(void *boardHandle, double scale, double rotAngle_deg);   /* scale and rotate the feedback signal */
__inline__ void  FWC_sis8300_struck_iqfb_func_setActVectorRotation(void *boardHandle, double scale, double rotAngle_deg);   /* scale and rotate the actuation signal */

__inline__ void  FWC_sis8300_struck_iqfb_func_setSetPoint_I(void *boardHandle, double value_MV, double factor);             /* set the set point value for I component */
__inline__ void  FWC_sis8300_struck_iqfb_func_setSetPoint_Q(void *boardHandle, double value_MV, double factor);             /* set the set point value for Q component */

__inline__ void  FWC_sis8300_struck_iqfb_func_setFeedforward_I(void *boardHandle, double value_MV, double factor);          /* set the feedforward value for I component */
__inline__ void  FWC_sis8300_struck_iqfb_func_setFeedforward_Q(void *boardHandle, double value_MV, double factor);          /* set the feedforward value for Q component */

__inline__ void  FWC_sis8300_struck_iqfb_func_setGain_I(void *boardHandle, double value);                                   /* set the gain value for I component */
__inline__ void  FWC_sis8300_struck_iqfb_func_setGain_Q(void *boardHandle, double value);                                   /* set the gain value for Q component */

__inline__ void  FWC_sis8300_struck_iqfb_func_setFeedbackErrLimits(void *boardHandle, unsigned int limit_i, unsigned int limit_q);

__inline__ void  FWC_sis8300_struck_iqfb_func_setIntgStart(void *boardHandle, double value_ns, double freq_MHz);            /* start time for integration, relative to the trigger */
__inline__ void  FWC_sis8300_struck_iqfb_func_setIntgEnd(void *boardHandle,   double value_ns, double freq_MHz);             

__inline__ void  FWC_sis8300_struck_iqfb_func_setApplStart(void *boardHandle, double value_ns, double freq_MHz);            /* start time for applying the correction, relative to the trigger */
__inline__ void  FWC_sis8300_struck_iqfb_func_setApplEnd(void *boardHandle,   double value_ns, double freq_MHz);             

__inline__ void  FWC_sis8300_struck_iqfb_func_setDACOffset_I(void *boardHandle, unsigned int offset);                       /* set the DAC offset */
__inline__ void  FWC_sis8300_struck_iqfb_func_setDACOffset_Q(void *boardHandle, unsigned int offset);

__inline__ void  FWC_sis8300_struck_iqfb_func_setAmpLimitHi(void *boardHandle,  unsigned int limit);                        /* set the output limit */
__inline__ void  FWC_sis8300_struck_iqfb_func_setAmpLimitLo(void *boardHandle,  unsigned int limit);

__inline__ void  FWC_sis8300_struck_iqfb_func_setMeaRotationTable(void *boardHandle, unsigned int pno, double *scaleTable, double *rotAngleTable_deg);      /* set the measurement chain rotation table */
__inline__ void  FWC_sis8300_struck_iqfb_func_setDrvRotationTable(void *boardHandle, unsigned int pno, double *scaleTable, double *rotAngleTable_deg);      /* set the driving chain rotation table */

__inline__ void  FWC_sis8300_struck_iqfb_func_setNonIQCoefOffset(void *boardHandle, unsigned int offset);

__inline__ void  FWC_sis8300_struck_iqfb_func_setImbalanceCorrMatrix(void *boardHandle, double a11, double a12, double a21, double a22);                    /* set the imbalance correction matrix */

/* Fimrware readings */
void FWC_sis8300_struck_iqfb_func_getAppFwInfo(void *boardHandle, unsigned int *firmwareName, unsigned int *majorVer, unsigned int *minorVer, unsigned int *buildNum);

__inline__ void  FWC_sis8300_struck_iqfb_func_getPulseCounter(void *boardHandle, unsigned int *pulseCnt);
__inline__ void  FWC_sis8300_struck_iqfb_func_getMeaTrigPeriod(void *boardHandle, double *value_ms, double freq_MHz);       /* get the measurement trigger period */

__inline__ void  FWC_sis8300_struck_iqfb_func_getNonIQCoefCur(void *boardHandle, unsigned int *cur);

__inline__ void  FWC_sis8300_struck_iqfb_func_getWatchDogCnt(void *boardHandle, unsigned int *cnt);
__inline__ void  FWC_sis8300_struck_iqfb_func_getFwStatus(void *boardHandle, unsigned int *platformStatus, unsigned int *RFCtrlStatus);

__inline__ void  FWC_sis8300_struck_iqfb_func_getMissingTrigCnt(void *boardHandle, unsigned int *cnt);

__inline__ void  FWC_sis8300_struck_iqfb_func_getAllDAQData(void *boardHandle, unsigned int *buf);                          /* data from BRAM */
__inline__ void  FWC_sis8300_struck_iqfb_func_getAllADCData(void *boardHandle, unsigned int pno,                            /* data from DRAM */
                                                            short *ADC0Data, short *ADC1Data,            
                                                            short *ADC2Data, short *ADC3Data,
                                                            short *ADC4Data, short *ADC5Data,
                                                            short *ADC6Data, short *ADC7Data,
                                                            short *ADC8Data, short *ADC9Data);

#ifdef __cplusplus
}
#endif

#endif


