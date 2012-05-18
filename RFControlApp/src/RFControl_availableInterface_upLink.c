/****************************************************
 * RFControl_availableInterface_upLink.c
 * 
 * Source file for the available interface for RFControl module
 * The uplink interfaces are realized here
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.05.16
 * Description: Initial creation
 ****************************************************/
#include <stdlib.h>             
#include <string.h>
#include <errlog.h>

#include "InternalData.h"
#include "RFControl_availableInterface_upLink.h"

/*======================================
 * Private Data and Routines
 *======================================*/
/* Write callback function, set the file names */
static void w_setFileName(void *ptr)
{
    INTD_struc_node      *dataNode = (INTD_struc_node *)ptr;
    
    if(!dataNode) return; 
    RFC_struc_moduleData *arg      = (RFC_struc_moduleData *)dataNode->privateData;

    if(arg) {
        strcpy(arg -> bsa_dataFileName_full, arg -> bsa_sr_folder);
        strcpy(arg -> bsa_wfFileName_full,   arg -> bsa_sr_folder);

        strcat(arg -> bsa_dataFileName_full, "/");
        strcat(arg -> bsa_wfFileName_full,   "/");

        strcat(arg -> bsa_dataFileName_full, arg -> bsa_dataFileName);
        strcat(arg -> bsa_wfFileName_full,   arg -> bsa_wfFileName);
    }
}


/**
 * Create data nodes for RF waveform objects
 */
static int RFC_func_rfWfCreateData(const char *moduleName, const char *wfName, RFLIB_struc_RFWaveform *wfDAQ, IOSCANPVT *ioIntScan) 
{
    int  status = 0;
    char var_dataName[64];

    /* check the input */
    if(!moduleName || !moduleName[0] || !wfName || !wfName[0] || !wfDAQ) return -1;

    /* create data node for all items in the RF waveform structure */    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_CHID");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->chId),           NULL, 1, NULL, INTD_ULONG, NULL, NULL, NULL, NULL, INTD_MBBO, INTD_PASSIVE);  /* w */

    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_VALID");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->valid),          NULL, 1, NULL, INTD_USHORT, NULL, NULL, NULL, NULL, INTD_BO, INTD_PASSIVE);  /* w */

    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AVGS");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->avgStartTime_ns), NULL, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);  /* w */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AVGL");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->avgTime_ns),     NULL, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);  /* w */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_RAW");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(wfDAQ->wfRaw),           NULL, (unsigned int)wfDAQ->pointNum, NULL, INTD_SHORT,  NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_I");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(wfDAQ->wfI),             NULL, (unsigned int)wfDAQ->pointNum, NULL, INTD_SHORT,  NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_Q");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(wfDAQ->wfQ),             NULL, (unsigned int)wfDAQ->pointNum, NULL, INTD_SHORT,  NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_A");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(wfDAQ->wfAmp),           NULL, (unsigned int)wfDAQ->pointNum, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_P_DEG");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(wfDAQ->wfPha_deg),       NULL, (unsigned int)wfDAQ->pointNum, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AMPSCALE");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->ampScale),       NULL, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);  /* w */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_POFF_DEG");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->phaOffset_deg),  NULL, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);  /* w */

    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AVGI");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->avgDataI),       NULL, 1, ioIntScan, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AI, INTD_IOINT);  /* w */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AVGQ");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->avgDataQ),       NULL, 1, ioIntScan, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AI, INTD_IOINT);  /* w */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AVGA");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->avgDataAmp),     NULL, 1, ioIntScan, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AI, INTD_IOINT);  /* w */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AVGP_DEG");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->avgDataPha_deg), NULL, 1, ioIntScan, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AI, INTD_IOINT);  /* w */   

    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AVGA_SLOW");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->avgDataAmp),     NULL, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AI, INTD_1S);  /* w */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AVGP_DEG_SLOW");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->avgDataPha_deg), NULL, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AI, INTD_1S);  /* w */   
  
    return status;
}                                

/**
 * Create data nodes for analog waveform objects
 */
static int RFC_func_analogWfCreateData(const char *moduleName, const char *wfName, RFLIB_struc_analogWaveform *wfDAQ, IOSCANPVT *ioIntScan) 
{
    int  status = 0;
    char var_dataName[64];

    /* check the input */
    if(!moduleName || !moduleName[0] || !wfName || !wfName[0] || !wfDAQ) return -1;

    /* create data node for all items in the analog waveform structure */    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_CHID");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->chId),           NULL, 1, NULL, INTD_ULONG, NULL, NULL, NULL, NULL, INTD_MBBO, INTD_PASSIVE);  /* w */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AVGS");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->avgStartTime_ns), NULL, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);  /* w */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AVGL");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->avgTime_ns),     NULL, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);  /* w */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_RAW");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(wfDAQ->wfRaw),           NULL, (unsigned int)wfDAQ->pointNum, NULL, INTD_SHORT,  NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */
        
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_A");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(wfDAQ->wfAmp),           NULL, (unsigned int)wfDAQ->pointNum, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */
        
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AMPSCALE");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->ampScale),       NULL, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);  /* w */
    
    strncpy(var_dataName, wfName, 64); strcat(var_dataName, "_AVG");
    status += INTD_API_createDataNode(moduleName, var_dataName, (void *)(&wfDAQ->avgData),        NULL, 1, ioIntScan, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AI, INTD_IOINT);  /* w */
    
    return status;
}                                

/*======================================
 * Public Routines
 *======================================*/
/**
 * Define internal data nodes for this module, including for firmware specific codes via the virtual functions
 * Input:
 *   arg            : Data structure of the module
 * Return:
 *   0              : Successful
 *  <=-1            : Failed
 */
int RFC_func_createEpicsData(RFC_struc_moduleData *arg)
{
    int status = 0;

    if(!arg) return -1;

    /*-----------------------------------
     * Data of the firmware module
     *-----------------------------------*/
    if(arg -> fwFunc.FWC_func_createEpicsData)    
        status += arg -> fwFunc.FWC_func_createEpicsData(arg -> fwModule, arg -> moduleName);

    /*-----------------------------------
     * Write only data - parameters to the LLA
     *-----------------------------------*/
    status += INTD_API_createDataNode(arg->moduleName, "APP_PHA_SP",        (void *)(&arg->fbData.fb_phaSetPoint_deg),      (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);        
    status += INTD_API_createDataNode(arg->moduleName, "APP_PHA_FBGAIN",    (void *)(&arg->fbData.fb_phaGain),              (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);        
    status += INTD_API_createDataNode(arg->moduleName, "APP_PHA_ERRTHR",    (void *)(&arg->fbData.fb_phaErrThreshold_deg),  (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);        

    status += INTD_API_createDataNode(arg->moduleName, "APP_AMP_LIMITH",    (void *)(&arg->fbData.fb_ampLimitHi),           (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);        
    status += INTD_API_createDataNode(arg->moduleName, "APP_AMP_LIMITL",    (void *)(&arg->fbData.fb_ampLimitLo),           (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);        

    status += INTD_API_createDataNode(arg->moduleName, "APP_PHA_SLEDWT",    (void *)(&arg->fbData.fb_phaSLEDWeight),        (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);        
    status += INTD_API_createDataNode(arg->moduleName, "APP_PHA_ACCWT",     (void *)(&arg->fbData.fb_phaACCWeight),         (void *)arg, 1, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AO, INTD_PASSIVE);        

    status += INTD_API_createDataNode(arg->moduleName, "APP_FBK_ENA",       (void *)(&arg->fbData.fb_feedbackEnabled),      (void *)arg, 1, NULL, INTD_USHORT, NULL, NULL, NULL, NULL, INTD_BO, INTD_PASSIVE);        
    status += INTD_API_createDataNode(arg->moduleName, "APP_FF_ENA",        (void *)(&arg->fbData.fb_feedForwardEnabled),   (void *)arg, 1, NULL, INTD_USHORT, NULL, NULL, NULL, NULL, INTD_BO, INTD_PASSIVE);        
    
    status += INTD_API_createDataNode(arg->moduleName, "APP_REFT_ENA",      (void *)(&arg->fbData.fb_refTrackEnabled),      (void *)arg, 1, NULL, INTD_USHORT, NULL, NULL, NULL, NULL, INTD_BO, INTD_PASSIVE);        

    /*-----------------------------------
     * Read only data - 120 Hz (for variables might need BSA)
     *-----------------------------------*/    
    status += INTD_API_createDataNode(arg->moduleName, "BSA_PHA_ERR",       (void *)(&arg->fbData.fb_phaErr_deg), (void *)arg, 1, &arg->ioscanpvt_120Hz, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AI, INTD_IOINT);    
    status += INTD_API_createDataNode(arg->moduleName, "BSA_PHA_ADJ",       (void *)(&arg->fbData.fb_phaAdj_deg), (void *)arg, 1, &arg->ioscanpvt_120Hz, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_AI, INTD_IOINT);        

    /*-----------------------------------
     * DAQ buffers and settings - waveforms (the average data may also need BSA)
     *-----------------------------------*/  
    status += RFC_func_rfWfCreateData(arg->moduleName, "WF_REF",            &arg->rfData_ref,           &arg->ioscanpvt_120Hz);
    status += RFC_func_rfWfCreateData(arg->moduleName, "WF_VM_OUT",         &arg->rfData_vmOut,         &arg->ioscanpvt_120Hz);
    status += RFC_func_rfWfCreateData(arg->moduleName, "WF_KLY_DRV",        &arg->rfData_klyDrive,      &arg->ioscanpvt_120Hz);
    status += RFC_func_rfWfCreateData(arg->moduleName, "WF_KLY_OUT",        &arg->rfData_klyOut,        &arg->ioscanpvt_120Hz);
    status += RFC_func_rfWfCreateData(arg->moduleName, "WF_SLED_OUT",       &arg->rfData_sledOut,       &arg->ioscanpvt_120Hz);
    status += RFC_func_rfWfCreateData(arg->moduleName, "WF_ACC_OUT_RF",     &arg->rfData_accOut_rf,     &arg->ioscanpvt_120Hz);
    status += RFC_func_rfWfCreateData(arg->moduleName, "WF_ACC_OUT_BEAM",   &arg->rfData_accOut_beam,   &arg->ioscanpvt_120Hz);    
    status += RFC_func_analogWfCreateData(arg->moduleName, "WF_KLY_BEAM_V", &arg->analogData_klyBeamV,  &arg->ioscanpvt_120Hz);

    /*-----------------------------------
     * Settings of BSA and save/restore (maybe later add db links for status string read...)
     *-----------------------------------*/  
    status += INTD_API_createDataNode(arg->moduleName, "BSA_SR_PATH",       (void *)(arg -> bsa_sr_folder),     (void *)arg, EPICSLIB_CONST_PATH_LEN, NULL, INTD_CHAR, NULL, w_setFileName, NULL, NULL, INTD_WFO, INTD_PASSIVE);  /* w */
    status += INTD_API_createDataNode(arg->moduleName, "BSA_SR_STATUS",     (void *)(arg -> bsa_sr_statusStr),  (void *)arg, EPICSLIB_CONST_PATH_LEN, NULL, INTD_CHAR, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);                /* r */
    status += INTD_API_createDataNode(arg->moduleName, "BSA_DATA_FILENAME", (void *)(arg -> bsa_dataFileName),  (void *)arg, EPICSLIB_CONST_NAME_LEN, NULL, INTD_CHAR, NULL, w_setFileName, NULL, NULL, INTD_WFO, INTD_PASSIVE);  /* w */
    status += INTD_API_createDataNode(arg->moduleName, "BSA_WF_FILENAME",   (void *)(arg -> bsa_wfFileName),    (void *)arg, EPICSLIB_CONST_NAME_LEN, NULL, INTD_CHAR, NULL, w_setFileName, NULL, NULL, INTD_WFO, INTD_PASSIVE);  /* w */
    
    status += INTD_API_createDataNode(arg->moduleName, "BSA_SAVE_DATA",     (void *)(&arg -> bsa_startDataBSA), (void *)arg, 1, NULL, INTD_USHORT, NULL, NULL, NULL, NULL, INTD_BO, INTD_PASSIVE); 
    status += INTD_API_createDataNode(arg->moduleName, "BSA_SAVE_WF",       (void *)(&arg -> bsa_startWfBSA),   (void *)arg, 1, NULL, INTD_USHORT, NULL, NULL, NULL, NULL, INTD_BO, INTD_PASSIVE);     

    status += INTD_API_createDataNode(arg->moduleName, "BSA_DATA_PERCENT",  (void *)(&arg -> bsa_dataBSAPercent),(void *)arg, 1, NULL, INTD_LONG,  NULL, NULL, NULL, NULL, INTD_LI, INTD_1S);
    status += INTD_API_createDataNode(arg->moduleName, "BSA_WF_PERCENT",    (void *)(&arg -> bsa_wfBSAPercent),  (void *)arg, 1, NULL, INTD_LONG,  NULL, NULL, NULL, NULL, INTD_LI, INTD_1S); 
    
    /*-----------------------------------
     * Diagnostics 
     *-----------------------------------*/  
    status += INTD_API_createDataNode(arg->moduleName, "DIAG_PHA_ERR_DEG",  (void *)(arg -> fbData.fb_phaErrArray_deg), NULL, RFC_CONST_RECENT_HISTORY_BUF_DEPTH, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);  /* r */

    status += INTD_API_createDataNode(arg->moduleName, "DIAG_IRQ_LATENCY",  (void *)(&arg -> IRQDelayCnt),  (void *)arg, 1, NULL, INTD_LONG,  NULL, NULL, NULL, NULL, INTD_LI, INTD_1S);
    status += INTD_API_createDataNode(arg->moduleName, "DIAG_IRQ_CNT",      (void *)(&arg -> IRQCnt),       (void *)arg, 1, NULL, INTD_LONG,  NULL, NULL, NULL, NULL, INTD_LI, INTD_1S);
    status += INTD_API_createDataNode(arg->moduleName, "DIAG_IRQ_MISSING",  (void *)(&arg -> IRQMissingCnt),(void *)arg, 1, NULL, INTD_LONG,  NULL, NULL, NULL, NULL, INTD_LI, INTD_1S);

    status += INTD_API_createDataNode(arg->moduleName, "DIAG_PROBE_SEL",    (void *)(&arg -> diag_probeDataSel), (void *)arg, 1,   NULL, INTD_LONG, NULL, NULL, NULL, NULL, INTD_LO, INTD_PASSIVE);
    status += INTD_API_createDataNode(arg->moduleName, "DIAG_PROBE_DATA",   (void *)(arg -> diag_probeData),     NULL, RFC_CONST_RECENT_HISTORY_BUF_DEPTH, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);         /* r */
    status += INTD_API_createDataNode(arg->moduleName, "DIAG_PROBE_STATUS", (void *)(arg -> diag_probeStatus),   (void *)arg, 128, NULL, INTD_CHAR, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);

    status += INTD_API_createDataNode(arg->moduleName, "DIAG_PROBE_SEL2",    (void *)(&arg -> diag_probeDataSel2), (void *)arg, 1,   NULL, INTD_LONG, NULL, NULL, NULL, NULL, INTD_LO, INTD_PASSIVE);
    status += INTD_API_createDataNode(arg->moduleName, "DIAG_PROBE_DATA2",   (void *)(arg -> diag_probeData2),     NULL, RFC_CONST_RECENT_HISTORY_BUF_DEPTH, NULL, INTD_DOUBLE, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);         /* r */
    status += INTD_API_createDataNode(arg->moduleName, "DIAG_PROBE_STATUS2", (void *)(arg -> diag_probeStatus2),   (void *)arg, 128, NULL, INTD_CHAR, NULL, NULL, NULL, NULL, INTD_WFI, INTD_1S);

    status += INTD_API_createDataNode(arg->moduleName, "DIAG_STATUS_VECT",  (void *)(&arg -> statusVector), (void *)arg, 1, NULL, INTD_LONG,  NULL, NULL, NULL, NULL, INTD_LI, INTD_1S);

    return status;
}

/**
 * Delete / disable the internal data. To be implemented later...
 */
int RFC_func_deleteEpicsData(RFC_struc_moduleData *arg)
{
    if(!arg) return -1;
    return 0;
}





