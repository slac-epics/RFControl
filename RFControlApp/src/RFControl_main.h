/****************************************************
 * RFControl_main.h
 * 
 * Header file for the RFControl module
 * This module is mainly designed to perform the pulse-pulse phase control. It will be
 *   associated with a RFControlBoard module and communicate with the firmware specific
 *   codes via virtual functions
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanforde.edu
 * Created on: 2011.05.12
 * Description: Initial creation
 * 
 * Modified by: Zheqiao Geng
 * Modified on: 2011.05.17
 * Description: Change the data type to be consistent with the epics record 
 *    - unsigned short  : bi, bo
 *    - long            : longout, longin
 *    - unsigned long   : mbbi, mbbo
 *    - double          : ai, ao
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2011.07.07
 * Description: Only keep the application layer here (pulse-pulse phase feedback and BSA)
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2011.10.11
 * Description: 1. Add another probe so that we can compare the correlations between data
 *              2. Add the reference tracking capability
 *              3. Redesign the algorithm for fast feed foward so that Patten Aware errors can be handled
 ****************************************************/
#ifndef RF_CONTROL_MAIN_H
#define RF_CONTROL_MAIN_H

#define RFC_CONST_RECENT_HISTORY_BUF_DEPTH 2048

#include "RFLib_signalProcess.h"                            /* use the library data definitions and routines */
#include "MathLib_dataProcess.h"
#include "EPICSLib_wrapper.h"

#include "syncDAQ.h"

#include "RFControl_requiredInterface_fwControlVirtual.h"   /* firmware specific codes access */
#include "RFControl_requiredInterface_fastFeedback.h"       /* interface with the fast feedback */

#ifdef __cplusplus
extern "C" {
#endif

/*======================================
 * Internal Data structure
 *======================================*/
/**
 * Data structure for feedback (note: these are for pulse-pulse feedback, which are different with the intra-pulse feedback settings for FPGA)
 */
typedef struct {

    volatile double fb_phaSetPoint_deg;                     /* phase set point value for the pulse - pulse control (respect to beam) */
    volatile double fb_phaGain;                             /* feedback gain */        
    volatile double fb_phaErrThreshold_deg;                 /* when the error larger than this threshold, it will not be taken into account */

    volatile double fb_phaErr_deg;                          /* phase error */
    volatile double fb_phaAdj_deg;                          /* phase adjustment */

    volatile double fb_ampLimitHi;                          /* amplitude limits. Only the amplitude is in this limits range, we will do feedback */
    volatile double fb_ampLimitLo;

    volatile double fb_phaSLEDWeight;                       /* weight for SLED phase for feedback */
    volatile double fb_phaACCWeight;                        /* weight for ACC phase for feedback */

    volatile unsigned short fb_feedbackEnabled;             /* put 1 to enable the pulse - pulse feedback */
    volatile unsigned short fb_feedForwardEnabled;          /* put 1 to enable the fast feed forward when the set point changes */

    volatile unsigned short fb_refTrackEnabled;             /* put 1 to enable the reference tracking for pulse-pulse feedback */

    double fb_phaErrArray_deg[RFC_CONST_RECENT_HISTORY_BUF_DEPTH];  /* a FIFO to show the recent history of the phase error */        

} RFC_struc_feedbackData;

/*======================================
 * Data structure for the RF Control module
 *======================================*/
typedef struct {

    EPICSLIB_type_linkedListNode node;                      /* to fit this structure to linked list */

    /* --- private data --- */
    char moduleName[EPICSLIB_CONST_NAME_LEN];               /* name of the module instance */
    char boardModuleName[EPICSLIB_CONST_NAME_LEN];          /* name of the module of the RFControlBoard */
    
    EPICSLIB_type_ioScanPvt ioscanpvt_120Hz;                /* I/O interrupt scanning list for 120Hz upgrade */    
    EPICSLIB_type_threadId  localThread;                    /* thread id */    

    int threadCreated;                                      /* 1 means the thread already created */        
    int stopThread;                                         /* 1 to stop the thread */
    int threadPriority;                                     /* priority of the local thread */   

    /* --- status --- */
    volatile long IRQDelayCnt;                              /* IRQ delay counter in clock cycle */
    volatile long IRQCnt;                                   /* IRQ counter */
    volatile long IRQMissingCnt;                            /* IRQ missing counter */

    volatile long statusVector;

    /* --- data and access for firmware --- */
    void *boardHandle;                                      /* board handle of the RFControlBoard module */
    void *fwModule;                                         /* data structure of the firmware control */    
    RFC_struc_fwAccessFunc fwFunc;                          /* virtual functions for firmware access */                    

    /* --- data for phase feedback --- */
    RFC_struc_feedbackData fbData;                          /* phase feedback data */

    /* --- data for raw data measurement and display (directly from ADCs) --- */
    RFLIB_struc_RFWaveform rfData_ref;                      /* raw data for the RF reference signal */
    RFLIB_struc_RFWaveform rfData_vmOut;                    /* raw data after the vector modulator */
    RFLIB_struc_RFWaveform rfData_klyDrive;                 /* raw data for klystron driving signal */
    RFLIB_struc_RFWaveform rfData_klyOut;                   /* raw data for klystron output signal */
    RFLIB_struc_RFWaveform rfData_sledOut;                  /* raw data for SLED output signal */
    RFLIB_struc_RFWaveform rfData_accOut_rf;                /* raw data for accelerator output, the RF signal */
    RFLIB_struc_RFWaveform rfData_accOut_beam;              /* raw data for accelerator output, the beam signal */
    RFLIB_struc_analogWaveform analogData_klyBeamV;         /* sample of a base band signal, the klystron beam voltage */ 

    /* --- data BSA and save/restore --- */
    char bsa_sr_folder[EPICSLIB_CONST_PATH_LEN];            /* path for data BSA and save/restore */
    char bsa_sr_statusStr[EPICSLIB_CONST_PATH_LEN];         /* status string */

    char bsa_dataFileName[EPICSLIB_CONST_NAME_LEN];         /* BSA data file name */
    char bsa_wfFileName[EPICSLIB_CONST_NAME_LEN];           /* BSA waveform file name */

    char bsa_dataFileName_full[EPICSLIB_CONST_PATH_LEN];
    char bsa_wfFileName_full[EPICSLIB_CONST_PATH_LEN];

    volatile unsigned short bsa_startDataBSA;               /* 1 to start a data acquisition and save to file */
    volatile unsigned short bsa_startWfBSA;                 /* 1 to start a waveform acquisition and save to file */

    volatile long bsa_dataBSAPercent;                       /* show how much persent the BSA is done */
    volatile long bsa_wfBSAPercent;                         /* show how much persent the BSA is done */

    /* --- diagnostics, probe for internal data --- */
    volatile long diag_probeDataSel;                            /* select the data that you want */
    double diag_probeData[RFC_CONST_RECENT_HISTORY_BUF_DEPTH];  /* a FIFO to show the recent history of the selected data */        
    char diag_probeStatus[128];                                 /* status message of the probe */

    volatile long diag_probeDataSel2;                           /* select the data that you want */
    double diag_probeData2[RFC_CONST_RECENT_HISTORY_BUF_DEPTH]; /* a FIFO to show the recent history of the selected data */        
    char diag_probeStatus2[128];                                /* status message of the probe */

} RFC_struc_moduleData;

/*======================================
 * Routines for the RF Control module
 *======================================*/     
/**
 * Basic routines for module management
 */
int  RFC_func_createModule(RFC_struc_moduleData *arg, const char *moduleName);              /* create an instance of this module,INIT ALL PARAMETERS! */      
int  RFC_func_destroyModule(RFC_struc_moduleData *arg);                                     /* destroy the instance */

int  RFC_func_initModule(RFC_struc_moduleData *arg);                                        /* do some initialization that can not be done during the creation */

int  RFC_func_associateBoardModule(RFC_struc_moduleData *arg, const char *boardModuleName); /* associate this module with a RFControlBoard module */

int  RFC_func_setThreadPriority(RFC_struc_moduleData *arg, unsigned int priority);          /* set the priority of the thread */
int  RFC_func_createThread(RFC_struc_moduleData *arg);                                      /* create a thread for the board ctrl */

#ifdef __cplusplus
}
#endif

#endif


