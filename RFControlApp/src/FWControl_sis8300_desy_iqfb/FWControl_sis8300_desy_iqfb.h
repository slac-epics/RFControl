/****************************************************
 * FWControl_sis8300_desy_iqfb.h
 * 
 * Define the data structure and implement the virtual functions for the firmware control
 *   for SIS8300 board with DESY (wojciech Jamuzna) platform firmware, and with the I/Q feedback
 *   control algorithm
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.07.07
 * Description: Initial creation
 ****************************************************/
#ifndef FW_CONTROL_SIS8300_DESY_IQFB_H
#define FW_CONTROL_SIS8300_DESY_IQFB_H

#include "RFLib_signalProcess.h"                            /* use the library data definitions and routines */
#include "MathLib_dataProcess.h"
#include "EPICSLib_wrapper.h"

#include "FWControl_sis8300_desy_iqfb_board.h"

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
    
    /* --- registers and buffers --- */
    volatile unsigned short board_reset;                    /* reset status of the board (1 - reset state; 0 - normal operation state) */
    volatile unsigned short board_triggerSource;            /* trigger source of the board (1 - internal; 0 - external) */ 
    volatile unsigned short board_refTrackEnabled;          /* 1 to indicate the reference tracking enabled in the board */
    volatile unsigned short board_DACOutputEnabled;         /* 1 to indicate the DAC output is enabled */
    volatile unsigned short board_DACConOutputEnabled;      /* 1 to indicate the DAC continuous output is enabled */
    volatile unsigned short board_DMAEnabled;               /* 1 to indicate the DMA is enabled for DAQ data reading, or register reading will be used */ 
    volatile unsigned short board_IRQEnabled;               /* 1 to indicate the user IRQ is enabled */    
    volatile unsigned short board_IRQDelayMeaEnabled;       /* 1 to indicate the IRQ delay measurement is enabled */

    volatile unsigned short board_harlinkOutSel;            /* 1 to output from internal logic, 0 to output from register */
    volatile unsigned short board_amcLVDSOutSel;            /* 1 to output from internal logic, 0 to output from register */
    
    volatile unsigned short board_setupSPI;                 /* write 1 to setup the clock divider, ADC chips and DAC chips */

    volatile unsigned long  board_ADCClockSel;              /* ADC clock selection */
    volatile unsigned long  board_extTriggerSel;            /* select the external trigger source */
    volatile unsigned long  board_DACOutputSel;             /* select the DAC output data source */
    
    volatile unsigned long  board_daqChSel[FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_NUM];      /* DAQ buffer selection (from predefined data input ports) */
    volatile unsigned long  board_refChSel;                             /* reference channel selection (from ADC channels) */
    volatile unsigned long  board_fbkChSel;                             /* feedback channel selection (from ADC channels) */
    
    volatile long board_firmwareName;                       /* board firmware name */   
    volatile long board_majorVer;                           /* major version number */
    volatile long board_minorVer;                           /* minor version number */
    volatile long board_buildNum;                           /* build number */
    volatile long board_serialNum;                          /* board serial number (we may have more than one board) */

    volatile long board_watchDogCnt;                        /* watch dog counter */
    volatile long board_alarmVector;                        /* show some alarms from the board */
    volatile long board_statusVector;                       /* show status of the board */  
    volatile long board_AD9510Status;                       /* statue of AD9510 */
    volatile long board_ADCStatus;                          /* status of ADC */
    
    volatile long board_harlinkOut;                         /* bit output from harlink (bit 0 - 3) */     
    volatile long board_amcLVDSOut;                         /* bit output from AMC LVDS port (bit 0 - 7) */
    volatile long board_harlinkIn;                          /* bit input to harlink (bit 0 - 3) */   
    volatile long board_amcLVDSIn;                          /* bit input to AMC LVDS port (bit 0 - 7) */
    volatile long board_DMAAddrStart;                       /* internal start addrss for DMA transfer */    

    volatile long board_DACOffsetI;                         /* DAC offset in digits */
    volatile long board_DACOffsetQ;
    
    volatile long board_ampLimitHi;                         /* Limit of the DAC output high */
    volatile long board_ampLimitLo;                         /* Limit of the DAC output low */
    
    volatile long board_ADCNoSignalThreshold;               /* threshold to detect if ADC signal is missing or not */   
    volatile long board_adcTrigThreshold;                   /* threshold to use ADC as external trigger source */
    
    volatile double  board_sampleFreq_MHz;                  /* sampling frequency of the waveforms in MHz */
    volatile double  board_extTriggerDelay_ns;              /* external trigger delay of the board in ns */
    volatile double  board_intTriggerPeriod_ns;             /* internal trigger period in ns */
    volatile double  board_intTriggerLength_ns;             /* internal trigger length in ns */
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
    
    double  board_meaRotScaleTable[FWC_SIS8300_DESY_IQFB_CONST_TAB_BUF_DEPTH];                        /* measurement chain rotation tables */
    double  board_meaRotAngleTable[FWC_SIS8300_DESY_IQFB_CONST_TAB_BUF_DEPTH];
    
    double  board_drvRotScaleTable[FWC_SIS8300_DESY_IQFB_CONST_TAB_BUF_DEPTH];                        /* driving chain rotation tables */
    double  board_drvRotAngleTable[FWC_SIS8300_DESY_IQFB_CONST_TAB_BUF_DEPTH];
    
    unsigned int board_bufWriteTable[FWC_SIS8300_DESY_IQFB_CONST_TAB_BUF_DEPTH];                      /* buffer for writing data to FPGA */

    unsigned int board_bufDAQ[FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH * FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_NUM];     /* buffer for all DAQ data */

    /* --- data for RF controller intermediate display --- */ 
    RFLIB_struc_RFWaveform rfData_DACOut;                   /* DAC ouput data */

    RFLIB_struc_RFWaveform rfData_refCh;                    /* reference channel RF data - from RFSignalDetection */
    RFLIB_struc_RFWaveform rfData_fbkCh;                    /* feedback channel RF data - from RFSignalDetection */

    RFLIB_struc_RFWaveform rfData_refCh_rotated;            /* rotated reference - rotated in the referenceTracking */
    RFLIB_struc_RFWaveform rfData_fbkCh_rotated;            /* rotated feedback - rotated in the referenceTracking */      
  
    RFLIB_struc_RFWaveform rfData_tracked;                  /* reference tracked RF data - from the referenceTracking */                  

    RFLIB_struc_RFWaveform rfData_err;                      /* error data - calculated in the RFController */
    RFLIB_struc_RFWaveform rfData_err_acc;                  /* error accumulation - calculated in the RFController */

    RFLIB_struc_RFWaveform rfData_act;                      /* actuation data - from the RFController */
    RFLIB_struc_RFWaveform rfData_act_rotated;              /* rotated actuation data - from the RFSignalActuation */
    
    /* --- time axis for waveforms --- */
    double timeAxis_ns[FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH];            /* time axis in ns */

} FWC_sis8300_desy_iqfb_struc_data;

/**
 * Implementation of the virtual functions
 */
int FWC_sis8300_desy_iqfb_func_init(void *module);

int FWC_sis8300_desy_iqfb_func_assBoard(void *module, void *boardHandle);

int FWC_sis8300_desy_iqfb_func_getMaxSampleNum(long *pno_max);

int FWC_sis8300_desy_iqfb_func_getDAQData(void *module);
int FWC_sis8300_desy_iqfb_func_getADCData(void *module, unsigned long channel, short *data, double *sampleFreq_MHz, double *sampleDelay_ns, long *pno, long *coefIdCur);
int FWC_sis8300_desy_iqfb_func_getIntData(void *module);

int FWC_sis8300_desy_iqfb_func_setPha_deg(void *module, double pha_deg);
int FWC_sis8300_desy_iqfb_func_setAmp(void *module, double amp);

int FWC_sis8300_desy_iqfb_func_waitIntr(void *module);

int FWC_sis8300_desy_iqfb_func_meaIntrLatency(void *module, long *latencyCnt, long *pulseCnt);

#define FWC_sis8300_desy_iqfb_func_getBoard FWC_sis8300_desy_iqfb_func_getBoardHandle

#ifdef __cplusplus
}
#endif

#endif



