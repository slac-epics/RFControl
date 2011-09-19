/****************************************************
 * FWControl_sis8300_desy_iqfb_upLink.c
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
#include "FWControl_sis8300_desy_iqfb_upLink.h"
#include "FWControl_sis8300_desy_iqfb_board.h"

/*======================================
 * Private Data and Routines - call backs
 *======================================*/
/* Write callback function, set bits (lower to higher) */
static void w_setBits(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    unsigned int data = 0x00000000;

    data = (arg -> board_reset               == 1) ? (data | 0x00000001) : (data & 0xFFFFFFFE);
    data = (arg -> board_triggerSource       == 1) ? (data | 0x00000002) : (data & 0xFFFFFFFD);
    data = (arg -> board_refTrackEnabled     == 1) ? (data | 0x00000004) : (data & 0xFFFFFFFB);
    data = (arg -> board_DACOutputEnabled    == 1) ? (data | 0x00000008) : (data & 0xFFFFFFF7);
    data = (arg -> board_DACConOutputEnabled == 1) ? (data | 0x00000010) : (data & 0xFFFFFFEF);
    data = (arg -> board_DMAEnabled          == 1) ? (data | 0x00000020) : (data & 0xFFFFFFDF);
    data = (arg -> board_IRQEnabled          == 1) ? (data | 0x00000040) : (data & 0xFFFFFFBF);
    data = (arg -> board_IRQDelayMeaEnabled  == 1) ? (data | 0x00000080) : (data & 0xFFFFFF7F);    

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setBits(arg -> board_handle, data);
    }
}

/* Write callback function, set the digital output (Harlink and AMC LVDS) selection */
static void w_digitalOutSel(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;
    
    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setHarlinkOutSource(arg -> board_handle, (unsigned int)arg -> board_harlinkOutSel);
        FWC_sis8300_desy_iqfb_func_setAMCLVDSOutSource(arg -> board_handle, (unsigned int)arg -> board_amcLVDSOutSel);
    }
}

/* Write callback function, set the SPI (AD9510, ADC, DAC) */
static void w_setSPI(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle && arg -> board_setupSPI == 1) {
        FWC_sis8300_desy_iqfb_func_setSPI(arg -> board_handle);        
    }
}

/* Write callback function, select ADC clock, external trigger and DAC output */
static void w_logicalDevSel(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setADCClockSource(arg -> board_handle, (unsigned int)arg -> board_ADCClockSel);
        FWC_sis8300_desy_iqfb_func_setExtTriggerSource(arg -> board_handle, (unsigned int)arg -> board_extTriggerSel, (unsigned int)arg -> board_adcTrigThreshold);
        FWC_sis8300_desy_iqfb_func_setDACDataSource(arg -> board_handle, (unsigned int)arg -> board_DACOutputSel);
    }
}

/* Read callback function, get the firmware info */
static void r_getFwInfo(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;
    
    unsigned int firmwareName;
    unsigned int majorVer;
    unsigned int minorVer;
    unsigned int buildNum;
    unsigned int serialNum;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_getBoardInfo(arg -> board_handle, &firmwareName, &majorVer, &minorVer, &buildNum, &serialNum);
        
        arg -> board_firmwareName   = (long)firmwareName;
        arg -> board_majorVer       = (long)majorVer;
        arg -> board_minorVer       = (long)minorVer;
        arg -> board_buildNum       = (long)buildNum;
        arg -> board_serialNum      = (long)serialNum;        
    }
}

/* Read callback function, get the board status */
static void r_getFwStatus(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;
    
    unsigned int watchDogCnt;
    unsigned int alarmVector;
    unsigned int statusVector;
    unsigned int ad9510Status;
    unsigned int ADCStatus;
    
    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_getBoardStat(arg -> board_handle, &watchDogCnt, &alarmVector, &statusVector, &ad9510Status, &ADCStatus);
        
        arg -> board_watchDogCnt   = (long)watchDogCnt;
        arg -> board_alarmVector   = (long)alarmVector;
        arg -> board_statusVector  = (long)statusVector;
        arg -> board_AD9510Status  = (long)ad9510Status;
        arg -> board_ADCStatus     = (long)ADCStatus;        
    }
}

/* Write callback function, set the digital output (Harlink and AMC LVDS) */
static void w_setDigitalOut(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setHarlinkOutData(arg -> board_handle, (unsigned int)arg -> board_harlinkOut);
        FWC_sis8300_desy_iqfb_func_setAMCLVDSOutData(arg -> board_handle, (unsigned int)arg -> board_amcLVDSOut);
    }
}

/* Read callback function, get the digital input (Harlink and AMC LVDS) */
static void r_getDigitalIn(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;
    
    unsigned int harlinkIn;
    unsigned int amcLVDSIn;
    
    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_getHarlinkInData(arg -> board_handle, &harlinkIn);
        FWC_sis8300_desy_iqfb_func_getAMCLVDSInData(arg -> board_handle, &amcLVDSIn);
        
        arg -> board_harlinkIn   = (long)harlinkIn;
        arg -> board_amcLVDSIn   = (long)amcLVDSIn;      
    }
}

/* Write callback function, set the DMA address */
static void w_setDMAAddr(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setDMAStartAddr(arg -> board_handle, (unsigned int)arg -> board_DMAAddrStart);
    }
}

/* Write callback function, set the offset */
static void w_setOffset(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setDACOffset_I(arg -> board_handle, (unsigned int)arg -> board_DACOffsetI);
        FWC_sis8300_desy_iqfb_func_setDACOffset_Q(arg -> board_handle, (unsigned int)arg -> board_DACOffsetQ);
    }
}

/* Write callback function, set the output limit */
static void w_setOLimit(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setAmpLimitHi(arg -> board_handle, (unsigned int)arg -> board_ampLimitHi);
        FWC_sis8300_desy_iqfb_func_setAmpLimitLo(arg -> board_handle, (unsigned int)arg -> board_ampLimitLo);
    }
}

/* Write callback function, set DAQ channel selection */
static void w_setDAQChSel(void *ptr)
{
    unsigned int i;

    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        for(i = 0; i < FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_NUM; i ++) {
            FWC_sis8300_desy_iqfb_func_setDAQChSel(arg -> board_handle, i, (unsigned int)arg -> board_daqChSel[i]);
        }    
    }
}

/* Write callback function, set ADC no signal threshold */
static void w_setADCNoSigT(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setADCNoSignalThreshold(arg -> board_handle, (unsigned int)arg -> board_ADCNoSignalThreshold);
    }
}

/* Write callback function, set reference and feedback channel ID */
static void w_setRefFbkChId(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_selectRefFbkChannel(arg -> board_handle, (unsigned int)arg -> board_refChSel, (unsigned int)arg -> board_fbkChSel);
    }
}

/* Write callback function, set the timing of the board */
static void w_setTiming(void *ptr)
{
    int i;
    
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        /* set the timing items in the board */
        FWC_sis8300_desy_iqfb_func_setExtTrigDelay(arg -> board_handle,   arg -> board_extTriggerDelay_ns,    arg -> board_sampleFreq_MHz);
        FWC_sis8300_desy_iqfb_func_setIntTrigPeriod(arg -> board_handle,  arg -> board_intTriggerPeriod_ns,   arg -> board_sampleFreq_MHz);
        FWC_sis8300_desy_iqfb_func_setIntTrigLength(arg -> board_handle,  arg -> board_intTriggerLength_ns,   arg -> board_sampleFreq_MHz);
        FWC_sis8300_desy_iqfb_func_setRFPulseLength(arg -> board_handle,  arg -> board_RFPulseLength_ns,      arg -> board_sampleFreq_MHz);
        FWC_sis8300_desy_iqfb_func_setDAQTrigDelay(arg -> board_handle,   arg -> board_DAQTriggerDelay_ns,    arg -> board_sampleFreq_MHz);
        FWC_sis8300_desy_iqfb_func_setIntgStart(arg -> board_handle,      arg -> board_intgStart_ns,          arg -> board_sampleFreq_MHz);
        FWC_sis8300_desy_iqfb_func_setIntgEnd(arg -> board_handle,        arg -> board_intgEnd_ns,            arg -> board_sampleFreq_MHz);
        FWC_sis8300_desy_iqfb_func_setApplStart(arg -> board_handle,      arg -> board_applyStart_ns,         arg -> board_sampleFreq_MHz);
        FWC_sis8300_desy_iqfb_func_setApplEnd(arg -> board_handle,        arg -> board_applyEnd_ns,           arg -> board_sampleFreq_MHz);
        
        /* build up the time axis for waveforms */
        for(i = 0; i < FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH; i ++) {
            arg->timeAxis_ns[i] = i * 1000 / arg -> board_sampleFreq_MHz + arg -> board_DAQTriggerDelay_ns;
        }        
                
        /* set the average parameters for waveforms */       
        arg->rfData_DACOut.sampleFreq_MHz        = arg -> board_sampleFreq_MHz;
        
        arg->rfData_refCh.sampleFreq_MHz         = arg -> board_sampleFreq_MHz;
        arg->rfData_fbkCh.sampleFreq_MHz         = arg -> board_sampleFreq_MHz;
        
        arg->rfData_refCh_rotated.sampleFreq_MHz = arg -> board_sampleFreq_MHz;
        arg->rfData_fbkCh_rotated.sampleFreq_MHz = arg -> board_sampleFreq_MHz;
        
        arg->rfData_tracked.sampleFreq_MHz       = arg -> board_sampleFreq_MHz;
        
        arg->rfData_err.sampleFreq_MHz           = arg -> board_sampleFreq_MHz;
        arg->rfData_err_acc.sampleFreq_MHz       = arg -> board_sampleFreq_MHz;
        
        arg->rfData_act.sampleFreq_MHz           = arg -> board_sampleFreq_MHz;
        arg->rfData_act_rotated.sampleFreq_MHz   = arg -> board_sampleFreq_MHz;
                        
        arg->rfData_DACOut.sampleDelay_ns        = arg -> board_DAQTriggerDelay_ns;
        
        arg->rfData_refCh.sampleDelay_ns         = arg -> board_DAQTriggerDelay_ns;
        arg->rfData_fbkCh.sampleDelay_ns         = arg -> board_DAQTriggerDelay_ns;
        
        arg->rfData_refCh_rotated.sampleDelay_ns = arg -> board_DAQTriggerDelay_ns;
        arg->rfData_fbkCh_rotated.sampleDelay_ns = arg -> board_DAQTriggerDelay_ns;
        
        arg->rfData_tracked.sampleDelay_ns       = arg -> board_DAQTriggerDelay_ns;
        
        arg->rfData_err.sampleDelay_ns           = arg -> board_DAQTriggerDelay_ns;
        arg->rfData_err_acc.sampleDelay_ns       = arg -> board_DAQTriggerDelay_ns;
        
        arg->rfData_act.sampleDelay_ns           = arg -> board_DAQTriggerDelay_ns;
        arg->rfData_act_rotated.sampleDelay_ns   = arg -> board_DAQTriggerDelay_ns;
    }
}

/* Write callback function, set the reference rotation */
static void w_setRefRot(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setRefVectorRotation(arg -> board_handle, arg -> board_refRotationGain, arg -> board_refRotationAngle_deg);
    }
}

/* Write callback function, set the feedback rotation */
static void w_setFbkRot(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setFbkVectorRotation(arg -> board_handle, arg -> board_fbkRotationGain, arg -> board_fbkRotationAngle_deg);
    }
}

/* Write callback function, set the actuation rotation */
static void w_setActRot(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setActVectorRotation(arg -> board_handle, arg -> board_actRotationGain, arg -> board_actRotationAngle_deg);
    }
}

/* Write callback function, set the set point and feed forward */
static void w_setSP(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setSetPoint_I(arg -> board_handle, arg -> board_setPointI_MV, arg -> board_voltageFactor_perMV);
        FWC_sis8300_desy_iqfb_func_setSetPoint_Q(arg -> board_handle, arg -> board_setPointQ_MV, arg -> board_voltageFactor_perMV);

        FWC_sis8300_desy_iqfb_func_setFeedforward_I(arg -> board_handle, arg -> board_feedforwardI_MV, arg -> board_voltageFactor_perMV);
        FWC_sis8300_desy_iqfb_func_setFeedforward_Q(arg -> board_handle, arg -> board_feedforwardQ_MV, arg -> board_voltageFactor_perMV);
    }
}

/* Write callback function, set the gain */
static void w_setGain(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setGain_I(arg -> board_handle, arg -> board_gainI);
        FWC_sis8300_desy_iqfb_func_setGain_Q(arg -> board_handle, arg -> board_gainQ);
    }
}

/* Write callback function, set measure rotation table */
static void w_setMeaRotTable(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setMeaRotationTable(arg -> board_handle, FWC_SIS8300_DESY_IQFB_CONST_TAB_BUF_DEPTH, arg -> board_meaRotScaleTable, arg -> board_meaRotAngleTable, arg -> board_bufWriteTable);
    }
}

/* Write callback function, set drive rotation table */
static void w_setDrvRotTable(void *ptr)
{
    INTD_struc_node                  *dataNode = (INTD_struc_node *)ptr;
    FWC_sis8300_desy_iqfb_struc_data *arg      = (FWC_sis8300_desy_iqfb_struc_data *)dataNode->privateData;

    if(arg && arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setDrvRotationTable(arg -> board_handle, FWC_SIS8300_DESY_IQFB_CONST_TAB_BUF_DEPTH, arg -> board_drvRotScaleTable, arg -> board_drvRotAngleTable, arg -> board_bufWriteTable);
    }
}

/*======================================
 * Private Data and Routines - others
 *======================================*/
/**
 * Create data nodes for RF waveform objects
 */
static int FWC_sis8300_desy_iqfb_func_rfWfCreateData(const char *moduleName, const char *wfName, RFLIB_struc_RFWaveform *wf) 
{
    int  status = 0;
    char var_dataName[64];

    /* create data node for I/Q items in the RF waveform structure (because here the intermediate data is only for diagnostics!) */
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_CHID");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wf->chId), NULL, 1, NULL, INTD_ULONG, NULL, NULL, NULL, NULL, INTD_MBBO, INTD_PASSIVE);  /* w */

    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_I");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(wf->wfI), NULL, FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH, NULL, INTD_SHORT,  NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_Q");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(wf->wfQ), NULL, FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH, NULL, INTD_SHORT,  NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */
      
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

int FWC_sis8300_desy_iqfb_func_createEpicsData(void *module, const char *moduleName)
{
    int status = 0;
    FWC_sis8300_desy_iqfb_struc_data *arg = (FWC_sis8300_desy_iqfb_struc_data *)module;

    if(!arg || !moduleName || !moduleName[0]) return -1;    

    /*-----------------------------------
     * Direct writing/reading the board
     *-----------------------------------*/
    status += INTD_API_createDataNode(moduleName, "B_RESET",       (void *)(&arg -> board_reset),               (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_TRG_SRC",     (void *)(&arg -> board_triggerSource),       (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);    
    status += INTD_API_createDataNode(moduleName, "B_ENA_TRACK",   (void *)(&arg -> board_refTrackEnabled),     (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_ENA_OUT",     (void *)(&arg -> board_DACOutputEnabled),    (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_ENA_COUT",    (void *)(&arg -> board_DACConOutputEnabled), (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_ENA_DMA",     (void *)(&arg -> board_DMAEnabled),          (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_ENA_IRQ",     (void *)(&arg -> board_IRQEnabled),          (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);    
    status += INTD_API_createDataNode(moduleName, "B_ENA_IRQ_MEA", (void *)(&arg -> board_IRQDelayMeaEnabled),  (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setBits, NULL, NULL, INTD_BO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_HLNKO_SEL", (void *)(&arg -> board_harlinkOutSel), (void *)arg, 1, NULL, INTD_USHORT, NULL, w_digitalOutSel, NULL, NULL, INTD_BO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_LVDSO_SEL", (void *)(&arg -> board_amcLVDSOutSel), (void *)arg, 1, NULL, INTD_USHORT, NULL, w_digitalOutSel, NULL, NULL, INTD_BO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_SET_SPI",     (void *)(&arg -> board_setupSPI),      (void *)arg, 1, NULL, INTD_USHORT, NULL, w_setSPI,        NULL, NULL, INTD_BO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_ADC_CLK_SEL", (void *)(&arg -> board_ADCClockSel),   (void *)arg, 1, NULL, INTD_ULONG, NULL, w_logicalDevSel, NULL, NULL, INTD_MBBO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_EXT_TRG_SEL", (void *)(&arg -> board_extTriggerSel), (void *)arg, 1, NULL, INTD_ULONG, NULL, w_logicalDevSel, NULL, NULL, INTD_MBBO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_DAC_OUT_SEL", (void *)(&arg -> board_DACOutputSel),  (void *)arg, 1, NULL, INTD_ULONG, NULL, w_logicalDevSel, NULL, NULL, INTD_MBBO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_DAQ_SEL_0", (void *)(arg -> board_daqChSel + 0), (void *)arg, 1, NULL, INTD_ULONG, NULL, w_setDAQChSel,   NULL, NULL, INTD_MBBO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_DAQ_SEL_1", (void *)(arg -> board_daqChSel + 1), (void *)arg, 1, NULL, INTD_ULONG, NULL, w_setDAQChSel,   NULL, NULL, INTD_MBBO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_DAQ_SEL_2", (void *)(arg -> board_daqChSel + 2), (void *)arg, 1, NULL, INTD_ULONG, NULL, w_setDAQChSel,   NULL, NULL, INTD_MBBO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_DAQ_SEL_3", (void *)(arg -> board_daqChSel + 3), (void *)arg, 1, NULL, INTD_ULONG, NULL, w_setDAQChSel,   NULL, NULL, INTD_MBBO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_DAQ_SEL_4", (void *)(arg -> board_daqChSel + 4), (void *)arg, 1, NULL, INTD_ULONG, NULL, w_setDAQChSel,   NULL, NULL, INTD_MBBO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_DAQ_SEL_5", (void *)(arg -> board_daqChSel + 5), (void *)arg, 1, NULL, INTD_ULONG, NULL, w_setDAQChSel,   NULL, NULL, INTD_MBBO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_DAQ_SEL_6", (void *)(arg -> board_daqChSel + 6), (void *)arg, 1, NULL, INTD_ULONG, NULL, w_setDAQChSel,   NULL, NULL, INTD_MBBO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_DAQ_SEL_7", (void *)(arg -> board_daqChSel + 7), (void *)arg, 1, NULL, INTD_ULONG, NULL, w_setDAQChSel,   NULL, NULL, INTD_MBBO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_REF_SEL",   (void *)(&arg -> board_refChSel),    (void *)arg, 1, NULL, INTD_ULONG, NULL, w_setRefFbkChId, NULL, NULL, INTD_MBBO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_FBK_SEL",   (void *)(&arg -> board_fbkChSel),    (void *)arg, 1, NULL, INTD_ULONG, NULL, w_setRefFbkChId, NULL, NULL, INTD_MBBO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_FMNAME", (void *)(&arg -> board_firmwareName), (void *)arg, 1, NULL, INTD_LONG, r_getFwInfo, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_MJVERS", (void *)(&arg -> board_majorVer),     (void *)arg, 1, NULL, INTD_LONG, r_getFwInfo, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_MIVERS", (void *)(&arg -> board_minorVer),     (void *)arg, 1, NULL, INTD_LONG, r_getFwInfo, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_BLDNUM", (void *)(&arg -> board_buildNum),     (void *)arg, 1, NULL, INTD_LONG, r_getFwInfo, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_SRLNUM", (void *)(&arg -> board_serialNum),    (void *)arg, 1, NULL, INTD_LONG, r_getFwInfo, NULL, NULL, NULL, INTD_LI, INTD_10S);

    status += INTD_API_createDataNode(moduleName, "B_WDCNT",     (void *)(&arg -> board_watchDogCnt),  (void *)arg, 1, NULL, INTD_LONG, r_getFwStatus, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_ALARM",     (void *)(&arg -> board_alarmVector),  (void *)arg, 1, NULL, INTD_LONG, r_getFwStatus, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_STATUS",    (void *)(&arg -> board_statusVector), (void *)arg, 1, NULL, INTD_LONG, r_getFwStatus, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_AD9510STA", (void *)(&arg -> board_AD9510Status), (void *)arg, 1, NULL, INTD_LONG, r_getFwStatus, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_ADCSTA",    (void *)(&arg -> board_ADCStatus),    (void *)arg, 1, NULL, INTD_LONG, r_getFwStatus, NULL, NULL, NULL, INTD_LI, INTD_10S);
    
    status += INTD_API_createDataNode(moduleName, "B_HLNKO",   (void *)(&arg -> board_harlinkOut),   (void *)arg, 1, NULL, INTD_LONG, NULL, w_setDigitalOut, NULL, NULL, INTD_LO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_LVDSO",   (void *)(&arg -> board_amcLVDSOut),   (void *)arg, 1, NULL, INTD_LONG, NULL, w_setDigitalOut, NULL, NULL, INTD_LO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_HLNKI",   (void *)(&arg -> board_harlinkIn),    (void *)arg, 1, NULL, INTD_LONG, r_getDigitalIn, NULL, NULL, NULL, INTD_LI, INTD_10S);    
    status += INTD_API_createDataNode(moduleName, "B_LVDSI",   (void *)(&arg -> board_amcLVDSIn),    (void *)arg, 1, NULL, INTD_LONG, r_getDigitalIn, NULL, NULL, NULL, INTD_LI, INTD_10S);
    status += INTD_API_createDataNode(moduleName, "B_DMAADDR", (void *)(&arg -> board_DMAAddrStart), (void *)arg, 1, NULL, INTD_LONG, NULL, w_setDMAAddr, NULL, NULL, INTD_LO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_OFFS_I", (void *)(&arg -> board_DACOffsetI), (void *)arg, 1, NULL, INTD_LONG, NULL, w_setOffset, NULL, NULL, INTD_LO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_OFFS_Q", (void *)(&arg -> board_DACOffsetQ), (void *)arg, 1, NULL, INTD_LONG, NULL, w_setOffset, NULL, NULL, INTD_LO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_LIMIT_HI", (void *)(&arg -> board_ampLimitHi), (void *)arg, 1, NULL, INTD_LONG, NULL, w_setOLimit, NULL, NULL, INTD_LO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_LIMIT_LO", (void *)(&arg -> board_ampLimitLo), (void *)arg, 1, NULL, INTD_LONG, NULL, w_setOLimit, NULL, NULL, INTD_LO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_ADC_NOSIGT", (void *)(&arg -> board_ADCNoSignalThreshold), (void *)arg, 1, NULL, INTD_LONG,   NULL, w_setADCNoSigT,   NULL, NULL, INTD_LO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_ADC_TRIGT",  (void *)(&arg -> board_adcTrigThreshold),     (void *)arg, 1, NULL, INTD_LONG,   NULL, w_logicalDevSel,  NULL, NULL, INTD_LO, INTD_PASSIVE);
        
    status += INTD_API_createDataNode(moduleName, "B_FREQ",            (void *)(&arg -> board_sampleFreq_MHz),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_EXT_TRIG_DELAY",  (void *)(&arg -> board_extTriggerDelay_ns),  (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_INT_TRIG_PERIOD", (void *)(&arg -> board_intTriggerPeriod_ns), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_INT_TRIG_LENGTH", (void *)(&arg -> board_intTriggerLength_ns), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_PUL_LENGTH",      (void *)(&arg -> board_RFPulseLength_ns),    (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_DAQ_TRIG_DELAY",  (void *)(&arg -> board_DAQTriggerDelay_ns),  (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setTiming,  NULL, NULL, INTD_AO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_REF_ROT_GAIN",    (void *)(&arg -> board_refRotationGain),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setRefRot, NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_REF_ROT_ANGLE",   (void *)(&arg -> board_refRotationAngle_deg), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setRefRot, NULL, NULL, INTD_AO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_FBK_ROT_GAIN",    (void *)(&arg -> board_fbkRotationGain),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setFbkRot, NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_FBK_ROT_ANGLE",   (void *)(&arg -> board_fbkRotationAngle_deg), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setFbkRot, NULL, NULL, INTD_AO, INTD_PASSIVE);

    status += INTD_API_createDataNode(moduleName, "B_ACT_ROT_GAIN",    (void *)(&arg -> board_actRotationGain),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setActRot, NULL, NULL, INTD_AO, INTD_PASSIVE);
    status += INTD_API_createDataNode(moduleName, "B_ACT_ROT_ANGLE",   (void *)(&arg -> board_actRotationAngle_deg), (void *)arg, 1, NULL, INTD_DOUBLE, NULL, w_setActRot, NULL, NULL, INTD_AO, INTD_PASSIVE);

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

    /*-----------------------------------
     * Write only buffer - rotation tables
     *-----------------------------------*/      
    status += INTD_API_createDataNode(moduleName, "TAB_MEA_ROT_SCALE", (void *)(arg -> board_meaRotScaleTable),(void *)arg, FWC_SIS8300_DESY_IQFB_CONST_TAB_BUF_DEPTH, NULL, INTD_DOUBLE, NULL, w_setMeaRotTable, NULL, NULL, INTD_WFO, INTD_PASSIVE);  /* w */
    status += INTD_API_createDataNode(moduleName, "TAB_MEA_ROT_ANGLE", (void *)(arg -> board_meaRotAngleTable),(void *)arg, FWC_SIS8300_DESY_IQFB_CONST_TAB_BUF_DEPTH, NULL, INTD_DOUBLE, NULL, w_setMeaRotTable, NULL, NULL, INTD_WFO, INTD_PASSIVE);  /* w */
    status += INTD_API_createDataNode(moduleName, "TAB_DRV_ROT_SCALE", (void *)(arg -> board_drvRotScaleTable),(void *)arg, FWC_SIS8300_DESY_IQFB_CONST_TAB_BUF_DEPTH, NULL, INTD_DOUBLE, NULL, w_setDrvRotTable, NULL, NULL, INTD_WFO, INTD_PASSIVE);  /* w */
    status += INTD_API_createDataNode(moduleName, "TAB_DRV_ROT_ANGLE", (void *)(arg -> board_drvRotAngleTable),(void *)arg, FWC_SIS8300_DESY_IQFB_CONST_TAB_BUF_DEPTH, NULL, INTD_DOUBLE, NULL, w_setDrvRotTable, NULL, NULL, INTD_WFO, INTD_PASSIVE);  /* w */    

    /*-----------------------------------
     * DAQ buffers and settings - waveforms 
     *-----------------------------------*/     
    status += FWC_sis8300_desy_iqfb_func_rfWfCreateData(moduleName, "WF_DAC_OUT",        &arg->rfData_DACOut);
     
    status += FWC_sis8300_desy_iqfb_func_rfWfCreateData(moduleName, "WF_REF_CH",         &arg->rfData_refCh);
    status += FWC_sis8300_desy_iqfb_func_rfWfCreateData(moduleName, "WF_FBK_CH",         &arg->rfData_fbkCh);
    
    status += FWC_sis8300_desy_iqfb_func_rfWfCreateData(moduleName, "WF_REF_CH_ROT",     &arg->rfData_refCh_rotated);
    status += FWC_sis8300_desy_iqfb_func_rfWfCreateData(moduleName, "WF_FBK_CH_ROT",     &arg->rfData_fbkCh_rotated);
    
    status += FWC_sis8300_desy_iqfb_func_rfWfCreateData(moduleName, "WF_TRACKED",        &arg->rfData_tracked);
    
    status += FWC_sis8300_desy_iqfb_func_rfWfCreateData(moduleName, "WF_ERR",            &arg->rfData_err);
    status += FWC_sis8300_desy_iqfb_func_rfWfCreateData(moduleName, "WF_ERR_ACC",        &arg->rfData_err_acc);
    
    status += FWC_sis8300_desy_iqfb_func_rfWfCreateData(moduleName, "WF_ACT",            &arg->rfData_act);
    status += FWC_sis8300_desy_iqfb_func_rfWfCreateData(moduleName, "WF_ACT_ROT",        &arg->rfData_act_rotated);
    
    return status;
}

/**
 * Delete / disable the internal data. To be implemented later...
 */
int FWC_sis8300_desy_iqfb_func_deleteEpicsData(void *module, const char *moduleName)
{
    return 0;
}





















