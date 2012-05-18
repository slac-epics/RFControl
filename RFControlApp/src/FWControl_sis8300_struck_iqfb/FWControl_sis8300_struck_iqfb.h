/****************************************************
 * FWControl_sis8300_struck_iqfb.h
 * 
 * Define the data structure and implement the virtual functions for the firmware control
 *   for SIS8300 board with struck platform firmware, and with the I/Q feedback
 *   control algorithm
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.07.07
 * Description: Initial creation
 * 
 * Modified by: Zheqiao Geng
 * Modified on: 2011.08.03
 * Description: Add the functions for the firmware of LLRF-SIS8300-R-0-0-1
 ****************************************************/
#ifndef FW_CONTROL_SIS8300_STRUCK_IQFB_H
#define FW_CONTROL_SIS8300_STRUCK_IQFB_H

#include "RFLib_signalProcess.h"                            /* use the library data definitions and routines */
#include "MathLib_dataProcess.h"
#include "EPICSLib_wrapper.h"

#include "FWControl_sis8300_struck_iqfb_board.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Define the data structure for the firmware
 * some EPICS data types:
 *    - unsigned short  : bi, bo
 *    - long            : longout, longin
 *    - unsigned long   : mbbi, mbbo
 *    - double          : ai, ao
 */
typedef struct {
    void *board_handle;                                     /* handle of the RFControlBoard */
    
    char board_deviceName[EPICSLIB_CONST_NAME_LEN];         /* name of the device in "/dev" */
    volatile long board_deviceOpened;                       /* 1 to indicate the device is correctly opened */

    /* --- for platform firmware --- */
    /* this part define the variables for the build in platform firmware provided by the Struck company. All parameters
     * corresponds to the registers defined in the manual of the Struck SIS8300 board 
     */
    volatile unsigned short board_setupSPI;                 /* write 1 to setup the clock divider, ADC chips and DAC chips */
    volatile unsigned short board_clkDiv2;                  /* 1 to divide the ADC clock by 2,, this is to use the OSC clock (250MHz) for testing */

    volatile unsigned long  board_ADCClockSel;              /* ADC clock selection */

    volatile long board_platformFwId;                       /* platform firmware Id */
    volatile long board_sno;                                /* board serial number */

    volatile long board_harlinkOut;                         /* bit output from harlink (bit 0 - 3) */     
    volatile long board_amcLVDSOut;                         /* bit output from AMC LVDS port (bit 0 - 7) */
    volatile long board_harlinkIn;                          /* bit input to harlink (bit 0 - 3) */   
    volatile long board_amcLVDSIn;                          /* bit input to AMC LVDS port (bit 0 - 7) */ 

    volatile long board_extTrigSrc;                         /* external trigger enable */

    /* --- for application firmware (IQ feedback control) --- */
    /* this part define the variables for the application firmware for IQ feedback control */
    volatile unsigned short board_reset;                    /* reset status of the board (1 - reset state; 0 - normal operation state) */
    volatile unsigned short board_triggerSource;            /* trigger source of the board (1 - internal; 0 - external) */ 
    volatile unsigned short board_refTrackEnabled;          /* 1 to indicate the reference tracking enabled in the board */
    volatile unsigned short board_DACOutputEnabled;         /* 1 to indicate the DAC output is enabled */
    volatile unsigned short board_DACConOutputEnabled;      /* 1 to indicate the DAC continuous output is enabled */
    volatile unsigned short board_IRQEnabled;               /* 1 to indicate the user IRQ is enabled */    
    volatile unsigned short board_DACOutSel;                /* 1 to indicate the DAC is outputing the RF simulator results (DAC1 for kly sim, DAC2 for SLED sim) */ 
    volatile unsigned short board_applyCoefIdOffset;        /* 1 to send the command to apply the coef ID offset */    

    volatile unsigned short board_fbEnable;                 /* 1 to enable the intra-pulse feedback */

    volatile unsigned long  board_refChSel;                 /* reference channel selection (from ADC channels) */
    volatile unsigned long  board_fbkChSel;                 /* feedback channel selection (from ADC channels) */
    
    volatile long board_firmwareName;                       /* board firmware name */   
    volatile long board_majorVer;                           /* major version number */
    volatile long board_minorVer;                           /* minor version number */
    volatile long board_buildNum;                           /* build number */
        
    volatile long board_pulseCnt;                           /* pulse counter */
    volatile long board_trigMissingCnt;                     /* trigger missing times count (continously miss several triggers is count by 1 time) */
    volatile long board_ADCClkWdCnt;                        /* ADC clock watch dog counter */
    volatile long board_platformStatus;                     /* platform firmware status */
    volatile long board_RFCtrlStatus;                       /* RF control firmware status */

    volatile long board_DACOffsetI;                         /* DAC offset in digits */
    volatile long board_DACOffsetQ;
    
    volatile long board_ampLimitHi;                         /* Limit of the DAC output high */
    volatile long board_ampLimitLo;                         /* Limit of the DAC output low */
    
    volatile long board_ADCSamplePno;                       /* ADC sample point number */

    volatile long board_coefIdOffset;                       /* the non-IQ coefficient ID offset, this is to compensate the sampling start point uncertainty after power cycle */
    volatile long board_coefIdCur;                          /* current coefficient Id for the first point of the DAQ buffer (ADC) */

    volatile long board_errLimitI;                          /* error limit in raw data */
    volatile long board_errLimitQ;

    volatile double  board_sampleFreq_MHz;                  /* sampling frequency of the waveforms in MHz */
    volatile double  board_extTriggerDelay_ns;              /* external trigger delay of the board in ns */
    volatile double  board_extTriggerPeriod_ms;             /* external trigger period in ms (minimum) */
    volatile double  board_intTriggerPeriod_ms;             /* internal trigger period in ms */
    volatile double  board_intTriggerLength_ms;             /* internal trigger length in ms */
    volatile double  board_meaTriggerPeriod_ms;             /* measured trigger period in ms for diagnostics */
    volatile double  board_RFPulseLength_ns;                /* RF pulse length in ns */
    volatile double  board_DAQTriggerDelay_ns;              /* DAQ trigger delay respect to the global trigger of the board */        

    volatile double  board_refRotationGain;                 /* rotation gain of the reference signal */
    volatile double  board_refRotationAngle_deg;            /* rotation angle of the reference signal */
    
    volatile double  board_fbkRotationGain;                 /* rotation gain of the feedback signal */
    volatile double  board_fbkRotationAngle_deg;            /* rotation angle of the feedback signal */
    
    volatile double  board_actRotationGain;                 /* rotation gain of the actuation signal */
    volatile double  board_actRotationAngle_deg;            /* rotation angle of the actuation signal */
    
    volatile double  board_setPointI_MV;                    /* set point value I component, MV */
    volatile double  board_setPointQ_MV;                    /* set point value Q component, MV */
    
    volatile double  board_feedforwardI_MV;                 /* feed forward value I component, MV */
    volatile double  board_feedforwardQ_MV;                 /* feed forward value Q component, MV */
    
    volatile double  board_voltageFactor_perMV;             /* factor to convert the voltage from physical unit to digits used in FPGA */
    
    volatile double  board_gainI;                           /* feedback gain of I component */
    volatile double  board_gainQ;                           /* feedback gain of Q component */
    
    volatile double  board_intgStart_ns;                    /* integration start time in ns */
    volatile double  board_intgEnd_ns;                      /* integration end time in ns */
    
    volatile double  board_applyStart_ns;                   /* correction application start time in ns */
    volatile double  board_applyEnd_ns;                     /* correction application end time in ns */   

    volatile double  board_imbalanceMatrixA11;              /* matrix for the vector modulator imbalance correction (values should be between (-1, 1) */
    volatile double  board_imbalanceMatrixA12;
    volatile double  board_imbalanceMatrixA21;
    volatile double  board_imbalanceMatrixA22;

    double  board_meaRotScaleTable[FWC_SIS8300_STRUCK_IQFB_CONST_MEA_TAB_BUF_DEPTH];    /* measurement chain rotation tables */
    double  board_meaRotAngleTable[FWC_SIS8300_STRUCK_IQFB_CONST_MEA_TAB_BUF_DEPTH];
    
    double  board_drvRotScaleTable[FWC_SIS8300_STRUCK_IQFB_CONST_DRV_TAB_BUF_DEPTH];    /* driving chain rotation tables */
    double  board_drvRotAngleTable[FWC_SIS8300_STRUCK_IQFB_CONST_DRV_TAB_BUF_DEPTH];

    unsigned int board_bufDAQ[FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH * FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_NUM * 2];     /* buffer for all DAQ data */

    short board_ADC0_raw[FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX];      /* ADC raw data for display */
    short board_ADC1_raw[FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX];
    short board_ADC2_raw[FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX];
    short board_ADC3_raw[FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX];
    short board_ADC4_raw[FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX];
    short board_ADC5_raw[FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX];
    short board_ADC6_raw[FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX];
    short board_ADC7_raw[FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX];
    short board_ADC8_raw[FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX];
    short board_ADC9_raw[FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX];

    short *board_ADC_data[10];

    /* --- data for RF controller intermediate display --- */ 
    RFLIB_struc_RFWaveform rfData_refCh;                    /* reference channel RF data - from RFSignalDetection */
    RFLIB_struc_RFWaveform rfData_fbkCh;                    /* feedback channel RF data - from RFSignalDetection */
  
    RFLIB_struc_RFWaveform rfData_tracked;                  /* reference tracked RF data - from the referenceTracking */                  
    RFLIB_struc_RFWaveform rfData_err;                      /* error data - calculated in the RFController */

    RFLIB_struc_RFWaveform rfData_act;                      /* actuation data - from the RFController */
    RFLIB_struc_RFWaveform rfData_DACOut;                   /* DAC ouput data */
    
    /* --- time axis for waveforms --- */
    double DAQTimeAxis_ns[FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH];        /* time axis in ns for internal DAQ*/
    double ADCTimeAxis_ns[FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX];       /* time axis in ns for ADC waveform */

} FWC_sis8300_struck_iqfb_struc_data;

/**
 * Implementation of the virtual functions
 */
int FWC_sis8300_struck_iqfb_func_init(void *module);

int FWC_sis8300_struck_iqfb_func_assBoard(void *module, void *boardHandle);

int FWC_sis8300_struck_iqfb_func_getMaxSampleNum(long *pno_max);

int FWC_sis8300_struck_iqfb_func_getDAQData(void *module);
int FWC_sis8300_struck_iqfb_func_getADCData(void *module, unsigned long channel, short *data, double *sampleFreq_MHz, double *sampleDelay_ns, long *pno, long *coefIdCur);
int FWC_sis8300_struck_iqfb_func_getIntData(void *module);

int FWC_sis8300_struck_iqfb_func_setPha_deg(void *module, double pha_deg);
int FWC_sis8300_struck_iqfb_func_setAmp(void *module, double amp);

int FWC_sis8300_struck_iqfb_func_waitIntr(void *module);

int FWC_sis8300_struck_iqfb_func_meaIntrLatency(void *module, long *latencyCnt, long *pulseCnt);

#define FWC_sis8300_struck_iqfb_func_getBoard FWC_sis8300_struck_iqfb_func_getBoardHandle

#ifdef __cplusplus
}
#endif

#endif



