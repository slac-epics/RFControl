/****************************************************
 * FWControl_sis8300_struck_iqfb_upLink.c
 * 
 * Realization of the virtual functions
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.07.07
 * Description: Initial creation
 ****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "InternalData.h"                                               /* to create EPICS data node */
#include "FWControl_sis8300_struck_iqfb_upLink.h"
#include "FWControl_sis8300_struck_iqfb_board.h"

/*======================================
 * Private Data and Routines - call backs
 *======================================*/
/* Write callback function, set the SPI (AD9510, ADC, DAC) */
static void w_setSPI(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle && arg -> board_setupSPI == 1) {
        FWC_sis8300_struck_iqfb_func_setSPI(arg -> board_handle, (unsigned int)arg -> board_clkDiv2);        
    }
}

/* Write callback function, set bits (lower to higher) */
static void w_setBits(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    unsigned int data = 0x00000000;

    data += arg -> board_reset                  << 0;
    data += arg -> board_triggerSource          << 1;
    data += arg -> board_refTrackEnabled        << 2;
    data += arg -> board_DACOutputEnabled       << 3;
    data += arg -> board_DACConOutputEnabled    << 4;
    data += arg -> board_IRQEnabled             << 5;
                                                            /* bit 6 is not settable from the PV */
    data += arg -> board_DACOutSel              << 7;
    data += arg -> board_applyCoefIdOffset      << 8;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setBits(arg -> board_handle, data);
    }
}

/* Read callback function, get the firmware info */
static void r_getFwInfo(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;
    
    unsigned int platformFwId;
    unsigned int boardSno;

    unsigned int firmwareName;
    unsigned int majorVer;
    unsigned int minorVer;
    unsigned int buildNum;

    int deviceOpened;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_getPlatformInfo(arg -> board_handle, &platformFwId, &boardSno);
        FWC_sis8300_struck_iqfb_func_getAppFwInfo(arg -> board_handle, &firmwareName, &majorVer, &minorVer, &buildNum);
        FWC_sis8300_struck_iqfb_func_getBoardInfo(arg -> board_handle, arg -> board_deviceName, &deviceOpened);
        
        arg -> board_platformFwId   = (long)platformFwId;
        arg -> board_sno            = (long)boardSno;

        arg -> board_firmwareName   = (long)firmwareName;
        arg -> board_majorVer       = (long)majorVer;
        arg -> board_minorVer       = (long)minorVer;
        arg -> board_buildNum       = (long)buildNum;

        arg -> board_deviceOpened   = (long)deviceOpened;
    }
}

/* Read callback function, get the firmware status (handle all 1s reading) */
static void r_getFwStatus(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    unsigned int pulCnt;
    unsigned int trigMissingCnt;
    unsigned int ADCClkWdCnt;
    unsigned int platformStatus;
    unsigned int RFCtrlStatus;

    if(arg && arg -> board_handle) {  
        FWC_sis8300_struck_iqfb_func_getPulseCounter(arg -> board_handle,   &pulCnt);
        FWC_sis8300_struck_iqfb_func_getMissingTrigCnt(arg -> board_handle, &trigMissingCnt);
        FWC_sis8300_struck_iqfb_func_getWatchDogCnt(arg -> board_handle,    &ADCClkWdCnt);
        FWC_sis8300_struck_iqfb_func_getFwStatus(arg -> board_handle,       &platformStatus, &RFCtrlStatus);

        arg -> board_pulseCnt       = (long)pulCnt;
        arg -> board_trigMissingCnt = (long)trigMissingCnt;
        arg -> board_ADCClkWdCnt    = (long)ADCClkWdCnt;
        arg -> board_platformStatus = (long)platformStatus;
        arg -> board_RFCtrlStatus   = (long)RFCtrlStatus;

        FWC_sis8300_struck_iqfb_func_getMeaTrigPeriod(arg -> board_handle, &arg -> board_meaTriggerPeriod_ms, arg -> board_sampleFreq_MHz);
    }
}

/* Write callback function, set the offset */
static void w_setOffset(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setDACOffset_I(arg -> board_handle, (unsigned int)arg -> board_DACOffsetI);
        FWC_sis8300_struck_iqfb_func_setDACOffset_Q(arg -> board_handle, (unsigned int)arg -> board_DACOffsetQ);
    }
}

/* Write callback function, set the output limit */
static void w_setOLimit(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setAmpLimitHi(arg -> board_handle, (unsigned int)arg -> board_ampLimitHi);
        FWC_sis8300_struck_iqfb_func_setAmpLimitLo(arg -> board_handle, (unsigned int)arg -> board_ampLimitLo);
    }
}

/* Write callback function, set reference and feedback channel ID */
static void w_setRefFbkChId(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_selectRefFbkChannel(arg -> board_handle, (unsigned int)arg -> board_refChSel, (unsigned int)arg -> board_fbkChSel);
    }
}

/* Write callback function, set the timing of the board */
static void w_setTiming(void *ptr)
{
    int i;
    
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        /* set the timing items in the board */
        FWC_sis8300_struck_iqfb_func_setExtTrigDelay(arg -> board_handle,   arg -> board_extTriggerDelay_ns,    arg -> board_sampleFreq_MHz);
        FWC_sis8300_struck_iqfb_func_setExtTrigPeriod(arg -> board_handle,  arg -> board_extTriggerPeriod_ms,   arg -> board_sampleFreq_MHz);
        FWC_sis8300_struck_iqfb_func_setIntTrigPeriod(arg -> board_handle,  arg -> board_intTriggerPeriod_ms,   arg -> board_sampleFreq_MHz);
        FWC_sis8300_struck_iqfb_func_setIntTrigLength(arg -> board_handle,  arg -> board_intTriggerLength_ms,   arg -> board_sampleFreq_MHz);
        FWC_sis8300_struck_iqfb_func_setRFPulseLength(arg -> board_handle,  arg -> board_RFPulseLength_ns,      arg -> board_sampleFreq_MHz);
        FWC_sis8300_struck_iqfb_func_setDAQTrigDelay(arg -> board_handle,   arg -> board_DAQTriggerDelay_ns,    arg -> board_sampleFreq_MHz);
        FWC_sis8300_struck_iqfb_func_setIntgStart(arg -> board_handle,      arg -> board_intgStart_ns,          arg -> board_sampleFreq_MHz);
        FWC_sis8300_struck_iqfb_func_setIntgEnd(arg -> board_handle,        arg -> board_intgEnd_ns,            arg -> board_sampleFreq_MHz);
        FWC_sis8300_struck_iqfb_func_setApplStart(arg -> board_handle,      arg -> board_applyStart_ns,         arg -> board_sampleFreq_MHz);
        FWC_sis8300_struck_iqfb_func_setApplEnd(arg -> board_handle,        arg -> board_applyEnd_ns,           arg -> board_sampleFreq_MHz);
        
        /* build up the time axis for waveforms */
        for(i = 0; i < FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH; i ++) {
            arg->DAQTimeAxis_ns[i] = i * 1000 / arg -> board_sampleFreq_MHz + arg -> board_DAQTriggerDelay_ns;
        }        

        for(i = 0; i < FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX; i ++) {
            arg->ADCTimeAxis_ns[i] = i * 1000 / arg -> board_sampleFreq_MHz + arg -> board_DAQTriggerDelay_ns;
        }       
                
        /* set the average parameters for waveforms */       
        arg->rfData_refCh.sampleFreq_MHz         = arg -> board_sampleFreq_MHz;
        arg->rfData_fbkCh.sampleFreq_MHz         = arg -> board_sampleFreq_MHz;
        
        arg->rfData_tracked.sampleFreq_MHz       = arg -> board_sampleFreq_MHz;
        arg->rfData_err.sampleFreq_MHz           = arg -> board_sampleFreq_MHz;
        
        arg->rfData_act.sampleFreq_MHz           = arg -> board_sampleFreq_MHz;
        arg->rfData_DACOut.sampleFreq_MHz        = arg -> board_sampleFreq_MHz;
                        
        arg->rfData_refCh.sampleDelay_ns         = arg -> board_DAQTriggerDelay_ns;
        arg->rfData_fbkCh.sampleDelay_ns         = arg -> board_DAQTriggerDelay_ns;
               
        arg->rfData_tracked.sampleDelay_ns       = arg -> board_DAQTriggerDelay_ns;
        arg->rfData_err.sampleDelay_ns           = arg -> board_DAQTriggerDelay_ns;
        
        arg->rfData_act.sampleDelay_ns           = arg -> board_DAQTriggerDelay_ns;
        arg->rfData_DACOut.sampleDelay_ns        = arg -> board_DAQTriggerDelay_ns;
    }
}

/* Write callback function, set the reference rotation */
static void w_setRefRot(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setRefVectorRotation(arg -> board_handle, arg -> board_refRotationGain, arg -> board_refRotationAngle_deg);
    }
}

/* Write callback function, set the feedback rotation */
static void w_setFbkRot(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setFbkVectorRotation(arg -> board_handle, arg -> board_fbkRotationGain, arg -> board_fbkRotationAngle_deg);
    }
}

/* Write callback function, set the actuation rotation */
static void w_setActRot(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setActVectorRotation(arg -> board_handle, arg -> board_actRotationGain, arg -> board_actRotationAngle_deg);
    }
}

/* Write callback function, set the set point and feed forward */
static void w_setSP(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setSetPoint_I(arg -> board_handle, arg -> board_setPointI_MV, arg -> board_voltageFactor_perMV);
        FWC_sis8300_struck_iqfb_func_setSetPoint_Q(arg -> board_handle, arg -> board_setPointQ_MV, arg -> board_voltageFactor_perMV);

        FWC_sis8300_struck_iqfb_func_setFeedforward_I(arg -> board_handle, arg -> board_feedforwardI_MV, arg -> board_voltageFactor_perMV);
        FWC_sis8300_struck_iqfb_func_setFeedforward_Q(arg -> board_handle, arg -> board_feedforwardQ_MV, arg -> board_voltageFactor_perMV);
    }
}

/* Write callback function, set the gain */
static void w_setGain(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setGain_I(arg -> board_handle, arg -> board_gainI * arg -> board_fbEnable);
        FWC_sis8300_struck_iqfb_func_setGain_Q(arg -> board_handle, arg -> board_gainQ * arg -> board_fbEnable);
    }
}

/* Write callback function, set measure rotation table */
static void w_setMeaRotTable(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setMeaRotationTable(arg -> board_handle, FWC_SIS8300_STRUCK_IQFB_CONST_MEA_TAB_BUF_DEPTH, arg -> board_meaRotScaleTable, arg -> board_meaRotAngleTable);
    }
}

/* Write callback function, set drive rotation table */
static void w_setDrvRotTable(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setDrvRotationTable(arg -> board_handle, FWC_SIS8300_STRUCK_IQFB_CONST_DRV_TAB_BUF_DEPTH, arg -> board_drvRotScaleTable, arg -> board_drvRotAngleTable);
    }
}

/* Write callback function, set the non-IQ coefficient offset */
static void w_setCoefIdOffset(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setNonIQCoefOffset(arg -> board_handle, (unsigned int)arg -> board_coefIdOffset);
    }
}

/* Write callback function, set the error limits */
static void w_setErrLimit(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setFeedbackErrLimits(arg -> board_handle, (unsigned int)arg -> board_errLimitI, (unsigned int)arg -> board_errLimitQ);
    }
}

/* Write callback function, set the ADC clock source */
static void w_ADCClkSrc(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setADCClockSource(arg -> board_handle, (unsigned int)arg -> board_ADCClockSel);
    }
}

/* Write callback function, set the digital output register (Harlink and AMC LVDS) */
static void w_setDigitalOut(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    unsigned int harlinkOutData = 0;
    unsigned int amcLVDSOutData = 0;

    unsigned int tmp_harlinkOut = (unsigned int)arg -> board_harlinkOut;
    unsigned int tmp_amcLVDSOut = (unsigned int)arg -> board_amcLVDSOut;
    unsigned int tmp_extTrigSrc = (unsigned int)arg -> board_extTrigSrc;

    if(arg && arg -> board_handle) {
        harlinkOutData += (tmp_harlinkOut << 16) & 0x000F0000;
        harlinkOutData += (tmp_extTrigSrc << 8)  & 0x00000F00;
        
        amcLVDSOutData += (tmp_amcLVDSOut << 16) & 0xFFFF0000;
        amcLVDSOutData += (tmp_extTrigSrc << 4)  & 0x0000FF00;

        FWC_sis8300_struck_iqfb_func_setHarlink(arg -> board_handle, harlinkOutData);
        FWC_sis8300_struck_iqfb_func_setAMCLVDS(arg -> board_handle, amcLVDSOutData);
    }
}

/* Read callback function, get the digital input (harlink and AMC MLVDS) */
static void r_getDigitalIn(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    unsigned int harlinkIn;
    unsigned int amcLVDSIn;

    if(arg && arg -> board_handle) {  
        FWC_sis8300_struck_iqfb_func_getHarlink(arg -> board_handle, &harlinkIn);
        FWC_sis8300_struck_iqfb_func_getAMCLVDS(arg -> board_handle, &amcLVDSIn);

        arg -> board_harlinkIn = (long)harlinkIn;
        arg -> board_amcLVDSIn = (long)amcLVDSIn;
    }
}

/* Write callback function, write the correction matrix of the I/Q imbalance of the vector modulator */
static void w_setIQCorr(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    FWC_sis8300_struck_iqfb_struc_data *arg    = (FWC_sis8300_struck_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setImbalanceCorrMatrix(arg -> board_handle, arg -> board_imbalanceMatrixA11, 
                                                                                 arg -> board_imbalanceMatrixA12, 
                                                                                 arg -> board_imbalanceMatrixA21, 
                                                                                 arg -> board_imbalanceMatrixA22);
    }
}

/*======================================
 * Private Data and Routines - others
 *======================================*/
/**
 * Create data nodes for RF waveform objects
 */
static int FWC_sis8300_struck_iqfb_func_rfWfCreateData(const char *moduleName, const char *wfName, RFLIB_struc_RFWaveform *wf) 
{
    int  status = 0;
    char var_dataName[64];

    /* check the input */
    if(!moduleName || !moduleName[0] || !wfName || !wfName[0] || !wf) return -1;

    /* create data node for I/Q items in the RF waveform structure (because here the intermediate data is only for diagnostics!) */
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_I");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(wf->wfI), NULL, FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH, NULL, INTD_SHORT,  NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_Q");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(wf->wfQ), NULL, FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH, NULL, INTD_SHORT,  NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */
      
    return status;
}             

/*======================================
 * Public Routines
 *======================================*/
/**
 * Define internal data nodes for this module
 * Input:
 *   module         : Data structure of the module
 *   moduleName     : Name of the high level module (RFControl)
 * Return:
 *   0              : Successful
 *  <=-1            : Failed
 */
int FWC_sis8300_struck_iqfb_func_createEpicsData(void *module, const char *moduleName)
{
    int status = 0;
    FWC_sis8300_struck_iqfb_struc_data *arg = (FWC_sis8300_struck_iqfb_struc_data *)module;

    if(!arg || !moduleName || !moduleName[0]) return -1;    

    /*-----------------------------------
     * Direct writing/reading the board
     *-----------------------------------*/
    status += INTD_API_createDataNode(moduleName, "B_DEV_NAME",    (void *)(arg  -> board_deviceName),          (void *)arg, EPICSLIB_CONST_NAME_LEN, NULL, INTD_CHAR, NULL, NULL, NULL, NULL, INTD_WFI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_DEV_OPENED",  (void *)(&arg -> board_deviceOpened),        (void *)arg, 1, NULL, INTD_LONG, r_getFwInfo, NULL,  NULL, NULL, INTD_LI, INTD_10S);

    status += INTD_API_createDataNode(moduleName, "B_SET_SPI",     (void *)(&arg -> board_setupSPI),            (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setSPI,  NULL, NULL, INTD_BO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_CLK_DIV2",    (void *)(&arg -> board_clkDiv2),             (void *)arg, 1, NULL, INTD_USHORT, NULL,     NULL,  NULL, NULL, INTD_BO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_ADC_CLK_SEL", (void *)(&arg -> board_ADCClockSel),         (void *)arg, 1, NULL, INTD_ULONG, NULL, w_ADCClkSrc, NULL, NULL, INTD_MBBO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_PLFWID",      (void *)(&arg -> board_platformFwId),        (void *)arg, 1, NULL, INTD_LONG, r_getFwInfo, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_BOARDSNO",    (void *)(&arg -> board_sno),                 (void *)arg, 1, NULL, INTD_LONG, NULL, NULL, NULL, NULL, INTD_LI, INTD_10S);

    status += INTD_API_createDataNode(moduleName, "B_HLNKO",       (void *)(&arg -> board_harlinkOut),          (void *)arg, 1, NULL, INTD_LONG, NULL, w_setDigitalOut, NULL, NULL, INTD_LO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_LVDSO",       (void *)(&arg -> board_amcLVDSOut),          (void *)arg, 1, NULL, INTD_LONG, NULL, w_setDigitalOut, NULL, NULL, INTD_LO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_HLNKI",       (void *)(&arg -> board_harlinkIn),           (void *)arg, 1, NULL, INTD_LONG, r_getDigitalIn, NULL,  NULL, NULL, INTD_LI, INTD_1S);    
    status += INTD_API_createDataNode(moduleName, "B_LVDSI",       (void *)(&arg -> board_amcLVDSIn),           (void *)arg, 1, NULL, INTD_LONG, NULL,           NULL,  NULL, NULL, INTD_LI, INTD_1S);

    status += INTD_API_createDataNode(moduleName, "B_EXT_TRG_SRC", (void *)(&arg -> board_extTrigSrc),          (void *)arg, 1, NULL, INTD_LONG, NULL, w_setDigitalOut, NULL, NULL, INTD_LO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_RESET",       (void *)(&arg -> board_reset),               (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_TRG_SRC",     (void *)(&arg -> board_triggerSource),       (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);    
    status += INTD_API_createDataNode(moduleName, "B_ENA_TRACK",   (void *)(&arg -> board_refTrackEnabled),     (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_ENA_OUT",     (void *)(&arg -> board_DACOutputEnabled),    (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_ENA_COUT",    (void *)(&arg -> board_DACConOutputEnabled), (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_ENA_IRQ",     (void *)(&arg -> board_IRQEnabled),          (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);    
    status += INTD_API_createDataNode(moduleName, "B_DAC_OUT_SEL", (void *)(&arg -> board_DACOutSel),           (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_APPL_ID_OFF", (void *)(&arg -> board_applyCoefIdOffset),   (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_ENA_FB",      (void *)(&arg -> board_fbEnable),            (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setGain, NULL, NULL, INTD_BO, INTD_PASSIVE);
 
    status += INTD_API_createDataNode(moduleName, "B_REF_SEL",     (void *)(&arg -> board_refChSel),            (void *)arg, 1, NULL, INTD_ULONG, NULL, w_setRefFbkChId, NULL, NULL, INTD_MBBO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_FBK_SEL",     (void *)(&arg -> board_fbkChSel),            (void *)arg, 1, NULL, INTD_ULONG, NULL, w_setRefFbkChId, NULL, NULL, INTD_MBBO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_FMNAME",      (void *)(&arg -> board_firmwareName),        (void *)arg, 1, NULL, INTD_LONG, r_getFwInfo, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_MJVERS",      (void *)(&arg -> board_majorVer),            (void *)arg, 1, NULL, INTD_LONG, NULL, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_MIVERS",      (void *)(&arg -> board_minorVer),            (void *)arg, 1, NULL, INTD_LONG, NULL, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_BLDNUM",      (void *)(&arg -> board_buildNum),            (void *)arg, 1, NULL, INTD_LONG, NULL, NULL, NULL, NULL, INTD_LI, INTD_10S);
    
    status += INTD_API_createDataNode(moduleName, "B_PUL_CNT",     (void *)(&arg -> board_pulseCnt),            (void *)arg, 1, NULL, INTD_LONG, r_getFwStatus, NULL, NULL, NULL, INTD_LI, INTD_1S);    /* r_getFwStatus handles all 1s scan reading record */
    status += INTD_API_createDataNode(moduleName, "B_TRG_LOST_CNT",(void *)(&arg -> board_trigMissingCnt),      (void *)arg, 1, NULL, INTD_LONG, NULL,          NULL, NULL, NULL, INTD_LI, INTD_1S);
    status += INTD_API_createDataNode(moduleName, "B_ADC_CLK_CNT", (void *)(&arg -> board_ADCClkWdCnt),         (void *)arg, 1, NULL, INTD_LONG, NULL,          NULL, NULL, NULL, INTD_LI, INTD_1S);    
    status += INTD_API_createDataNode(moduleName, "B_PFM_STATUS",  (void *)(&arg -> board_platformStatus),      (void *)arg, 1, NULL, INTD_LONG, NULL,          NULL, NULL, NULL, INTD_LI, INTD_1S);
    status += INTD_API_createDataNode(moduleName, "B_RFC_STATUS",  (void *)(&arg -> board_RFCtrlStatus),        (void *)arg, 1, NULL, INTD_LONG, NULL,          NULL, NULL, NULL, INTD_LI, INTD_1S);

    status += INTD_API_createDataNode(moduleName, "B_OFFS_I",      (void *)(&arg -> board_DACOffsetI),          (void *)arg, 1, NULL, INTD_LONG, NULL, w_setOffset, NULL, NULL, INTD_LO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_OFFS_Q",      (void *)(&arg -> board_DACOffsetQ),          (void *)arg, 1, NULL, INTD_LONG, NULL, w_setOffset, NULL, NULL, INTD_LO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_LIMIT_HI",    (void *)(&arg -> board_ampLimitHi),          (void *)arg, 1, NULL, INTD_LONG, NULL, w_setOLimit, NULL, NULL, INTD_LO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_LIMIT_LO",    (void *)(&arg -> board_ampLimitLo),          (void *)arg, 1, NULL, INTD_LONG, NULL, w_setOLimit, NULL, NULL, INTD_LO, INTD_PASSIVE);
       
    status += INTD_API_createDataNode(moduleName, "B_ADCS_PNO",    (void *)(&arg -> board_ADCSamplePno),        (void *)arg, 1, NULL, INTD_LONG, NULL, NULL,        NULL, NULL, INTD_LO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_COEF_ID_OFFS",(void *)(&arg -> board_coefIdOffset),        (void *)arg, 1, NULL, INTD_LONG, NULL, w_setCoefIdOffset,  NULL, NULL, INTD_LO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_ERR_LIMIT_I", (void *)(&arg -> board_errLimitI),           (void *)arg, 1, NULL, INTD_LONG, NULL, w_setErrLimit,  NULL, NULL, INTD_LO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_ERR_LIMIT_Q", (void *)(&arg -> board_errLimitQ),           (void *)arg, 1, NULL, INTD_LONG, NULL, w_setErrLimit,  NULL, NULL, INTD_LO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_FREQ",            (void *)(&arg -> board_sampleFreq_MHz),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_EXT_TRIG_DELAY",  (void *)(&arg -> board_extTriggerDelay_ns),  (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_EXT_TRIG_PERIOD", (void *)(&arg -> board_extTriggerPeriod_ms), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_INT_TRIG_PERIOD", (void *)(&arg -> board_intTriggerPeriod_ms), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_INT_TRIG_LENGTH", (void *)(&arg -> board_intTriggerLength_ms), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_MEA_TRIG_PERIOD", (void *)(&arg -> board_meaTriggerPeriod_ms), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL,         NULL, NULL, INTD_AI, INTD_1S);       
    status += INTD_API_createDataNode(moduleName, "B_PUL_LENGTH",      (void *)(&arg -> board_RFPulseLength_ns),    (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_DAQ_TRIG_DELAY",  (void *)(&arg -> board_DAQTriggerDelay_ns),  (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_REF_ROT_GAIN",    (void *)(&arg -> board_refRotationGain),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setRefRot, NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_REF_ROT_ANGLE",   (void *)(&arg -> board_refRotationAngle_deg), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setRefRot, NULL, NULL, INTD_AO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_FBK_ROT_GAIN",    (void *)(&arg -> board_fbkRotationGain),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setFbkRot, NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_FBK_ROT_ANGLE",   (void *)(&arg -> board_fbkRotationAngle_deg), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setFbkRot, NULL, NULL, INTD_AO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_ACT_ROT_GAIN",    (void *)(&arg -> board_actRotationGain),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setActRot, NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_ACT_ROT_ANGLE",   (void *)(&arg -> board_actRotationAngle_deg), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setActRot, NULL, NULL, INTD_AO, INTD_PASSIVE); 

    /* note: the rotation parameters can also be set by the low level application, and unfortunetaly it can not be set back to the record, so we define PVs to read out the data */
    status += INTD_API_createDataNode(moduleName, "B_REF_ROT_GAIN_RB", (void *)(&arg -> board_refRotationGain),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL,        NULL, NULL, INTD_AI, INTD_1S);
    status += INTD_API_createDataNode(moduleName, "B_REF_ROT_ANGLE_RB",(void *)(&arg -> board_refRotationAngle_deg), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL,        NULL, NULL, INTD_AI, INTD_1S);

    status += INTD_API_createDataNode(moduleName, "B_FBK_ROT_GAIN_RB", (void *)(&arg -> board_fbkRotationGain),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL,        NULL, NULL, INTD_AI, INTD_1S);
    status += INTD_API_createDataNode(moduleName, "B_FBK_ROT_ANGLE_RB",(void *)(&arg -> board_fbkRotationAngle_deg), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL,        NULL, NULL, INTD_AI, INTD_1S);

    status += INTD_API_createDataNode(moduleName, "B_ACT_ROT_GAIN_RB", (void *)(&arg -> board_actRotationGain),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL,        NULL, NULL, INTD_AI, INTD_1S);
    status += INTD_API_createDataNode(moduleName, "B_ACT_ROT_ANGLE_RB",(void *)(&arg -> board_actRotationAngle_deg), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL,        NULL, NULL, INTD_AI, INTD_1S);

    status += INTD_API_createDataNode(moduleName, "B_SP_I",            (void *)(&arg -> board_setPointI_MV),         (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setSP,     NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_SP_Q",            (void *)(&arg -> board_setPointQ_MV),         (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setSP,     NULL, NULL, INTD_AO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_FF_I",            (void *)(&arg -> board_feedforwardI_MV),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setSP,     NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_FF_Q",            (void *)(&arg -> board_feedforwardQ_MV),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setSP,     NULL, NULL, INTD_AO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_VOLT_COEF",       (void *)(&arg -> board_voltageFactor_perMV),  (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setSP,     NULL, NULL, INTD_AO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_GAIN_I",          (void *)(&arg -> board_gainI),                (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setGain,   NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_GAIN_Q",          (void *)(&arg -> board_gainQ),                (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setGain,   NULL, NULL, INTD_AO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_INTG_ST",         (void *)(&arg -> board_intgStart_ns),         (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming, NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_INTG_ET",         (void *)(&arg -> board_intgEnd_ns),           (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming, NULL, NULL, INTD_AO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_APPL_ST",         (void *)(&arg -> board_applyStart_ns),        (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming, NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_APPL_ET",         (void *)(&arg -> board_applyEnd_ns),          (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming, NULL, NULL, INTD_AO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_IQCORR_A11",      (void *)(&arg -> board_imbalanceMatrixA11),   (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setIQCorr, NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_IQCORR_A12",      (void *)(&arg -> board_imbalanceMatrixA12),   (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setIQCorr, NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_IQCORR_A21",      (void *)(&arg -> board_imbalanceMatrixA21),   (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setIQCorr, NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_IQCORR_A22",      (void *)(&arg -> board_imbalanceMatrixA22),   (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setIQCorr, NULL, NULL, INTD_AO, INTD_PASSIVE);

    /*-----------------------------------
     * Write only buffer - rotation tables
     *-----------------------------------*/      
    status += INTD_API_createDataNode(moduleName, "TAB_MEA_ROT_SCALE", (void *)(arg -> board_meaRotScaleTable),(void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_MEA_TAB_BUF_DEPTH, NULL, INTD_DOUBLE, NULL, w_setMeaRotTable, NULL, NULL, INTD_WFO, INTD_PASSIVE);  /* w */
    status += INTD_API_createDataNode(moduleName, "TAB_MEA_ROT_ANGLE", (void *)(arg -> board_meaRotAngleTable),(void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_MEA_TAB_BUF_DEPTH, NULL, INTD_DOUBLE, NULL, w_setMeaRotTable, NULL, NULL, INTD_WFO, INTD_PASSIVE);  /* w */
    status += INTD_API_createDataNode(moduleName, "TAB_DRV_ROT_SCALE", (void *)(arg -> board_drvRotScaleTable),(void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_DRV_TAB_BUF_DEPTH, NULL, INTD_DOUBLE, NULL, w_setDrvRotTable, NULL, NULL, INTD_WFO, INTD_PASSIVE);  /* w */
    status += INTD_API_createDataNode(moduleName, "TAB_DRV_ROT_ANGLE", (void *)(arg -> board_drvRotAngleTable),(void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_DRV_TAB_BUF_DEPTH, NULL, INTD_DOUBLE, NULL, w_setDrvRotTable, NULL, NULL, INTD_WFO, INTD_PASSIVE);  /* w */    

    /*-----------------------------------
     * DAQ buffers and settings - waveforms 
     *-----------------------------------*/     
    status += FWC_sis8300_struck_iqfb_func_rfWfCreateData(moduleName, "WF_REF_CH",         &arg->rfData_refCh);
    status += FWC_sis8300_struck_iqfb_func_rfWfCreateData(moduleName, "WF_FBK_CH",         &arg->rfData_fbkCh);
    
    status += FWC_sis8300_struck_iqfb_func_rfWfCreateData(moduleName, "WF_TRACKED",        &arg->rfData_tracked);
    status += FWC_sis8300_struck_iqfb_func_rfWfCreateData(moduleName, "WF_ERR",            &arg->rfData_err);
    
    status += FWC_sis8300_struck_iqfb_func_rfWfCreateData(moduleName, "WF_ACT",            &arg->rfData_act);
    status += FWC_sis8300_struck_iqfb_func_rfWfCreateData(moduleName, "WF_DAC_OUT",        &arg->rfData_DACOut);

    status += INTD_API_createDataNode(moduleName, "WF_DAQX", (void *)(arg -> DAQTimeAxis_ns), (void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH,  NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S); /* r */    
    status += INTD_API_createDataNode(moduleName, "WF_ADCX", (void *)(arg -> ADCTimeAxis_ns), (void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S); /* r */        

    status += INTD_API_createDataNode(moduleName, "WF_ADC0", (void *)(arg -> board_ADC0_raw), (void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX, NULL, INTD_SHORT, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */    
    status += INTD_API_createDataNode(moduleName, "WF_ADC1", (void *)(arg -> board_ADC1_raw), (void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX, NULL, INTD_SHORT, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */    
    status += INTD_API_createDataNode(moduleName, "WF_ADC2", (void *)(arg -> board_ADC2_raw), (void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX, NULL, INTD_SHORT, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */    
    status += INTD_API_createDataNode(moduleName, "WF_ADC3", (void *)(arg -> board_ADC3_raw), (void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX, NULL, INTD_SHORT, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */    
    status += INTD_API_createDataNode(moduleName, "WF_ADC4", (void *)(arg -> board_ADC4_raw), (void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX, NULL, INTD_SHORT, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */    
    status += INTD_API_createDataNode(moduleName, "WF_ADC5", (void *)(arg -> board_ADC5_raw), (void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX, NULL, INTD_SHORT, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */    
    status += INTD_API_createDataNode(moduleName, "WF_ADC6", (void *)(arg -> board_ADC6_raw), (void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX, NULL, INTD_SHORT, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */    
    status += INTD_API_createDataNode(moduleName, "WF_ADC7", (void *)(arg -> board_ADC7_raw), (void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX, NULL, INTD_SHORT, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */    
    status += INTD_API_createDataNode(moduleName, "WF_ADC8", (void *)(arg -> board_ADC8_raw), (void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX, NULL, INTD_SHORT, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */    
    status += INTD_API_createDataNode(moduleName, "WF_ADC9", (void *)(arg -> board_ADC9_raw), (void *)arg, FWC_SIS8300_STRUCK_IQFB_CONST_ADC_SAMPLE_MAX, NULL, INTD_SHORT, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */    

    return status;
}

/**
 * Delete / disable the internal data. To be implemented later...
 */
int FWC_sis8300_struck_iqfb_func_deleteEpicsData(void *module, const char *moduleName)
{
    return 0;
}





















