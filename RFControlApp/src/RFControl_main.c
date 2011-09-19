/****************************************************
 * RFControl_main.c
 * 
 * Source file for the RFControl module
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.05.16
 * Description: Initial creation
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2011.07.07
 * Description: Only keep the application layer here (pulse-pulse phase feedback and BSA)
 ****************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "RFControl_main.h"

/*======================================
 * Private Routines
 *======================================*/
/**
 * clock cycle counter (only works for intel CPU)
 */
__inline__ unsigned long long int rdtsc(void)
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x)); 
    return x;
}

/**
 * Get the RF waveform. Here we only get the ADC raw samples
 * Input:
 *     arg              : Data structure of the module instance
 * Output:
 *     data             : The RF data to be filled by the DAQ data from firmware
 * Return:
 *     0                : Successful
 *    -1                : Failed 
 */
static int RFC_func_getRFData(RFC_struc_moduleData *arg, RFLIB_struc_RFWaveform *data)
{
    int status = 0;

    /* Check the input */
    if(!arg || !data) return -1;

    /* Get the data */
    if(arg -> fwFunc.FWC_func_getADCData)    
        status = arg -> fwFunc.FWC_func_getADCData(arg -> fwModule, data -> chId, 
                                                                    data -> wfRaw, 
                                                                   &data -> sampleFreq_MHz,
                                                                   &data -> sampleDelay_ns,
                                                                   &data -> pointNum,
                                                                   &data -> demodCoefIdCur);

    return status;
}

/**
 * Process the RF data. Demodulate it, then calculate the averaged I/Q/A/P. The average
 *   start point and end point should be set via the EPICS PVs
 * Input:
 *     data             : The RF data to be demodulated
 * Return:
 *     0                : Successful
 *    -1                : Failed 
 */
static int RFC_func_demodAvgRFData(RFLIB_struc_RFWaveform *data)
{
    int status = 0;

    /* Check the input */
    if(!data) return -1;

    /* Handle the data */
    status += RFLIB_RFWaveformDemod(data);
    status += RFLIB_RFWaveformAvg(data);

    return status;
}

/**
 * Calculate the amplitude and phase, should already been demodulated
 * Input:
 *     data             : The RF data to be handled
 * Return:
 *     0                : Successful
 *    -1                : Failed 
 */
static int RFC_func_IQ2AP(RFLIB_struc_RFWaveform *data)
{
    int status = 0;

    /* Check the input */
    if(!data) return -1;

    /* Handle the data */
    status = RFLIB_RFWaveformIQ2AP(data);

    return status;
}

/**
 * Get an analog data, first get the raw data, then convert to physical unit and average
 * Input:
 *     arg              : Data structure of the module instance
 * Output:
 *     data             : The analog data to be filled by the DAQ data
 * Return:
 *     0                : Successful
 *    -1                : Failed 
 */
static int RFC_func_getAnalogData(RFC_struc_moduleData *arg, RFLIB_struc_analogWaveform *data)
{
    int status = 0;
    long tmpData;

    /* Check the input */
    if(!arg || !data) return -1;

    /* Get the data from the DAQ buffer */    
    if(arg -> fwFunc.FWC_func_getADCData)    
        status = arg -> fwFunc.FWC_func_getADCData(arg -> fwModule, data -> chId, 
                                                                    data -> wfRaw, 
                                                                   &data -> sampleFreq_MHz,
                                                                   &data -> sampleDelay_ns,
                                                                   &data -> pointNum,
                                                                   &tmpData);                       /* the demodulation coefficient Id is useless here */

    /* Scale the raw data to physical unit and average it */
    status += RFLIB_analogWaveformScale(data);
    status += RFLIB_analogWaveformAvg(data); 

    return status;    
}

/**
 * Create BSA nodes for RF waveform
 */
static int RFC_func_createBSANode_rfWaveform(RFLIB_struc_RFWaveform *data)
{
    int status = 0;

    /* Check the input */
    if(!data) return -1;

    /* Create data/waveform node */
    status += SDAQ_func_createDataNode(&data -> avgDataI);
    status += SDAQ_func_createDataNode(&data -> avgDataQ);
    status += SDAQ_func_createDataNode(&data -> avgDataAmp);
    status += SDAQ_func_createDataNode(&data -> avgDataPha_deg);  

    status += SDAQ_func_createWfNode(data -> wfRaw, (int)data -> pointNum);

    return status;
}

/**
 * Create BSA nodes for analog waveform
 */
static int RFC_func_createBSANode_analogWaveform(RFLIB_struc_analogWaveform *data)
{
    int status = 0;

    /* Check the input */
    if(!data) return -1;

    /* Create data/waveform node */
    status += SDAQ_func_createDataNode(&data -> avgData); 
    status += SDAQ_func_createWfNode(data -> wfRaw, (int)data -> pointNum);

    return status;
}

/**
 * Main thread of this module
 */
static void RFC_func_mainThread(void *argIn) 
{    
    RFC_struc_moduleData *arg = (RFC_struc_moduleData *)argIn;

    int calCnt = 0;                             /* for scheduling the not important calculation */

    int dataId = -1;                            /* for data BSA */
    int wfId   = -1;                            /* for waveform BSA */

    int intId;                                  /* for interrupt pulling */

    double phaSetPoint_deg_old = 1e6;           /* for detecting set point changes */

    long pulseCnt     = -1;                     /* for pulse counter reading */
    long pulseCnt_old = -1; 

    long irqDelayCnt  = 0;

    double probeData = 0;                       /* the probe data */

    /* Check the input */
    if(!arg) {
        printf("RFC_func_mainThread: Illegal thread creation!\n");
        return;
    }

    /* Main loop of the thread */
    while(1) {

        /*----------------------------------------------------
         * WAIT FOR INTERRUPT
         *----------------------------------------------------*/ 
        /* wait for event, suspend until interrupt comes, including disalbe the IRQ in firmware */
        if(arg -> fwFunc.FWC_func_waitIntr) {
            intId = arg -> fwFunc.FWC_func_waitIntr(arg -> fwModule);
        } else {
            continue;
        }

        if(intId == 0) {                                                /* User interrupt occurred */                     
        } else {
            continue;
        }

        /*----------------------------------------------------
         * DATA ACQUISITION BLOCK
         *----------------------------------------------------*/ 
        /* get all DAQ data */
        if(arg -> fwFunc.FWC_func_getDAQData) arg -> fwFunc.FWC_func_getDAQData(arg -> fwModule);        

        /*----------------------------------------------------
         * PHASE CONTROL BLOCK
         *----------------------------------------------------*/ 
        /* Get the RF data */
        RFC_func_getRFData(arg, &arg -> rfData_sledOut);                 /* get the SLED data from the DAQ buffer */
        RFC_func_getRFData(arg, &arg -> rfData_accOut_rf);               /* get the ACC data from the DAQ buffer */

        /* Measure the amplitude and phase in the window */
        RFC_func_demodAvgRFData(&arg -> rfData_sledOut);                        
        RFC_func_demodAvgRFData(&arg -> rfData_accOut_rf); 

        /* Measure the phase error */
        if(arg -> fbData.fb_phaSLEDWeight != 0 || arg -> fbData.fb_phaACCWeight != 0) {
            arg -> fbData.fb_phaErr_deg = arg -> fbData.fb_phaSetPoint_deg - 
                (arg -> rfData_sledOut.avgDataPha_deg * arg -> fbData.fb_phaSLEDWeight + arg -> rfData_accOut_rf.avgDataPha_deg * arg -> fbData.fb_phaACCWeight);
        } else {
            arg -> fbData.fb_phaErr_deg = 0; 
        }

        /* Fast feed forward and feedback */
        if(arg -> fbData.fb_feedForwardEnabled && arg -> fbData.fb_phaSetPoint_deg != phaSetPoint_deg_old && fabs(phaSetPoint_deg_old) <= 180) {

            arg -> fbData.fb_phaAdj_deg = arg -> fbData.fb_phaErr_deg;                                                              /* Get the adjustement */              
            if(arg -> fwFunc.FWC_func_setPha_deg) arg -> fwFunc.FWC_func_setPha_deg(arg -> fwModule, arg -> fbData.fb_phaAdj_deg);  /* Apply the adjustement to the firmware */

        } else if((fabs(arg -> fbData.fb_phaErr_deg) < arg -> fbData.fb_phaErrThreshold_deg) &&
            (arg -> rfData_sledOut.avgDataAmp > arg -> fbData.fb_ampLimitLo) &&
            (arg -> rfData_sledOut.avgDataAmp < arg -> fbData.fb_ampLimitHi) && (arg -> fbData.fb_feedbackEnabled)) { 
           
            arg -> fbData.fb_phaAdj_deg = arg -> fbData.fb_phaErr_deg * arg -> fbData.fb_phaGain;                                   /* Get the adjustement */
            if(arg -> fwFunc.FWC_func_setPha_deg) arg -> fwFunc.FWC_func_setPha_deg(arg -> fwModule, arg -> fbData.fb_phaAdj_deg);  /* Apply the adjustement to the firmware */

        }        

        /* remember the old set point */
        phaSetPoint_deg_old = arg -> fbData.fb_phaSetPoint_deg;

        /*----------------------------------------------------
         * HANDLE RF WAVEFORMS FOR BSA
         *----------------------------------------------------*/ 
        RFC_func_getRFData(arg, &arg -> rfData_ref);                /* SLED out and ACC out rf has been handled above! */
        RFC_func_getRFData(arg, &arg -> rfData_vmOut);
        RFC_func_getRFData(arg, &arg -> rfData_klyDrive);
        RFC_func_getRFData(arg, &arg -> rfData_klyOut);
        RFC_func_getRFData(arg, &arg -> rfData_accOut_beam);
        RFC_func_getAnalogData(arg, &arg -> analogData_klyBeamV);

        RFC_func_demodAvgRFData(&arg -> rfData_ref);
        RFC_func_demodAvgRFData(&arg -> rfData_vmOut);
        RFC_func_demodAvgRFData(&arg -> rfData_klyDrive);
        RFC_func_demodAvgRFData(&arg -> rfData_klyOut);
        RFC_func_demodAvgRFData(&arg -> rfData_accOut_beam);

        /*----------------------------------------------------
         * PREPARE DATA FOR BSA
         *----------------------------------------------------*/  
        /* 120Hz I/O interrupt scanning for old BSA */      
        EPICSLIB_func_scanIoRequest(arg -> ioscanpvt_120Hz);        

        /* synchronous data acquisition */
        if(arg ->  bsa_startDataBSA && dataId < 0) {                /* mechanism to start the data acquisition */
            dataId = 0;                            
            arg ->  bsa_startDataBSA = 0;
            SDAQ_func_saveData(dataId, arg -> bsa_dataFileName_full);        
            dataId ++;
        }

        if(dataId >= 1) {
            SDAQ_func_saveData(dataId, arg -> bsa_dataFileName_full);        
            dataId ++;
            arg -> bsa_dataBSAPercent = (long)(dataId * 100 / SDAQ_CONST_BUF_SIZE);
            if(dataId >= SDAQ_CONST_BUF_SIZE) {
                dataId = -1;
                strcpy(arg -> bsa_sr_statusStr, arg -> bsa_dataFileName_full);
                strcat(arg -> bsa_sr_statusStr, " saved!");
            }
        }  

        /* synchronous waveform acquisition */
        if(arg ->  bsa_startWfBSA && wfId < 0) {                    /* mechanism to start the waveform acquisition */
            wfId = 0;                            
            arg ->  bsa_startWfBSA = 0;
            SDAQ_func_saveWf(wfId, arg -> bsa_wfFileName_full);        
            wfId ++;
        }

        if(wfId >= 1) {
            SDAQ_func_saveWf(wfId, arg -> bsa_wfFileName_full);        
            wfId ++;
            arg -> bsa_wfBSAPercent = (long)(wfId * 100 / SDAQ_CONST_WF_NUM_SUPPORTED);
            if(wfId >= SDAQ_CONST_WF_NUM_SUPPORTED) {
                wfId = -1;
                strcpy(arg -> bsa_sr_statusStr, arg -> bsa_wfFileName_full);
                strcat(arg -> bsa_sr_statusStr, " saved!");
            }
        }  
                     
        /*----------------------------------------------------
         * WAVEFORMS DIAGNOSTICS
         *----------------------------------------------------*/
        /* round robin scheduling */
        switch(calCnt) {
            case 0:                    
                /* firmware intermediate waveforms */
                if(arg -> fwFunc.FWC_func_getIntData) arg -> fwFunc.FWC_func_getIntData(arg -> fwModule);                
                break;

            case 1:  RFC_func_IQ2AP(&arg -> rfData_ref);            break;
            case 2:  RFC_func_IQ2AP(&arg -> rfData_vmOut);          break;
            case 3:  RFC_func_IQ2AP(&arg -> rfData_klyDrive);       break;
            case 4:  RFC_func_IQ2AP(&arg -> rfData_klyOut);         break;
            case 5:  RFC_func_IQ2AP(&arg -> rfData_sledOut);        break;
            case 6:  RFC_func_IQ2AP(&arg -> rfData_accOut_rf);      break;
            case 7:  RFC_func_IQ2AP(&arg -> rfData_accOut_beam);    break;           
            default: break;
        };

        calCnt ++;
        if(calCnt > 7) calCnt = 0;

        /*----------------------------------------------------
         * OTHER DIAGNOSTICS
         *----------------------------------------------------*/
        /* the recent history buffer */
        memcpy((void*)arg -> fbData.fb_phaErrArray_deg, (void *)(arg -> fbData.fb_phaErrArray_deg + 1), sizeof(double) * (RFC_CONST_RECENT_HISTORY_BUF_DEPTH - 1));
        arg -> fbData.fb_phaErrArray_deg[RFC_CONST_RECENT_HISTORY_BUF_DEPTH - 1] = arg -> fbData.fb_phaErr_deg;

        /* probe the internal data */
        switch(arg -> diag_probeDataSel) {
            case 0:  probeData = arg -> fbData.fb_phaErr_deg;               strcpy(arg -> diag_probeStatus, "Pul-pul FB pha err (deg)");    break;
            case 1:  probeData = arg -> fbData.fb_phaAdj_deg;               strcpy(arg -> diag_probeStatus, "Pul-pul FB pha adj (deg)");    break;
            case 2:  probeData = arg -> rfData_ref.avgDataAmp;              strcpy(arg -> diag_probeStatus, "REF amp");                     break;
            case 3:  probeData = arg -> rfData_ref.avgDataPha_deg;          strcpy(arg -> diag_probeStatus, "REF pha (deg)");               break;
            case 4:  probeData = arg -> rfData_vmOut.avgDataAmp;            strcpy(arg -> diag_probeStatus, "IQ MOD amp");                  break;
            case 5:  probeData = arg -> rfData_vmOut.avgDataPha_deg;        strcpy(arg -> diag_probeStatus, "IQ MOD pha (deg)");            break;
            case 6:  probeData = arg -> rfData_klyDrive.avgDataAmp;         strcpy(arg -> diag_probeStatus, "KLY DRV amp");                 break;
            case 7:  probeData = arg -> rfData_klyDrive.avgDataPha_deg;     strcpy(arg -> diag_probeStatus, "KLY DRV pha (deg)");           break;
            case 8:  probeData = arg -> rfData_klyOut.avgDataAmp;           strcpy(arg -> diag_probeStatus, "KLY OUT amp");                 break;
            case 9:  probeData = arg -> rfData_klyOut.avgDataPha_deg;       strcpy(arg -> diag_probeStatus, "KLY OUT pha (deg)");           break;
            case 10: probeData = arg -> rfData_sledOut.avgDataAmp;          strcpy(arg -> diag_probeStatus, "SLED OUT amp");                break;
            case 11: probeData = arg -> rfData_sledOut.avgDataPha_deg;      strcpy(arg -> diag_probeStatus, "SLED OUT pha (deg)");          break;
            case 12: probeData = arg -> rfData_accOut_rf.avgDataAmp;        strcpy(arg -> diag_probeStatus, "ACC RF amp");                  break;
            case 13: probeData = arg -> rfData_accOut_rf.avgDataPha_deg;    strcpy(arg -> diag_probeStatus, "ACC RF pha (deg)");            break;
            case 14: probeData = arg -> rfData_accOut_beam.avgDataAmp;      strcpy(arg -> diag_probeStatus, "ACC BEAM amp");                break;
            case 15: probeData = arg -> rfData_accOut_beam.avgDataPha_deg;  strcpy(arg -> diag_probeStatus, "ACC BEAM pha (deg)");          break;
            case 16: probeData = arg -> analogData_klyBeamV.avgData;        strcpy(arg -> diag_probeStatus, "KLY HV amp");                  break;
            default: probeData = 0; strcpy(arg -> diag_probeStatus, "Not supported"); break;
        }

        memcpy((void*)arg -> diag_probeData, (void *)(arg -> diag_probeData + 1), sizeof(double) * (RFC_CONST_RECENT_HISTORY_BUF_DEPTH - 1));
        arg -> diag_probeData[RFC_CONST_RECENT_HISTORY_BUF_DEPTH - 1] = probeData;

        /* compile the status vector (continuing)
         *      bit 0: 1 for firmware/communication failure
         *      bit 1: 1 for pulse-pulse fb enabled
         *      bit 2: 1 for pulse-pulse fb failure
         */
        arg -> statusVector = 0;
        
        if((unsigned int)pulseCnt == 0xFFFFFFFF)                                                        /* when firmware is not available, all reading from register is 0xFFFFFFFF */
            arg -> statusVector += 0x00000001;                                                          
        else
            arg -> statusVector &= 0xFFFFFFFE;
        
        arg -> statusVector += arg -> fbData.fb_feedbackEnabled << 1;        


        /* check the IRQ missing, including enable the IRQ in the firmware */
        if(arg -> fwFunc.FWC_func_meaIntrLatency)
            arg -> fwFunc.FWC_func_meaIntrLatency(arg -> fwModule, &irqDelayCnt, &pulseCnt);            /* get the IRQ delay and trigger counter from firmware */

        arg -> IRQDelayCnt = irqDelayCnt;

        arg -> IRQCnt ++;                                                                               /* IRQ counter (note maybe different with trigger counter value if there is missing IRQ) */
        if(pulseCnt - pulseCnt_old > 1 && pulseCnt_old > 0) arg -> IRQMissingCnt ++;                    /* detect missing IRQ */
        pulseCnt_old = pulseCnt;

        /*----------------------------------------------------
         * MISC
         *----------------------------------------------------*/
        /* stop the thread on request */
        if(arg -> stopThread) break;
    }
}

/*======================================
 * Public Routines
 *======================================*/
/**
 * Construction of the instance of the module
 * Input:
 *     arg         : Data structure of the module instance
 *     moduleName  : Name of the module instance
 * Return:
 *     0           : Successful
 *    -1           : Failed
 * Note:
 *   The data now is only assigned to some default values. Later they should be initialized in some way. Such as
 *     1. Autosave. Only if the records are processed, the data will be set in the internal structure
 *     2. Read out from the firmware. We must do reversed calculation from the FPGA settings (not so good!)
 */
int RFC_func_createModule(RFC_struc_moduleData *arg, const char *moduleName)
{
    /* Check the input */
    if(!arg || !moduleName || !moduleName[0]) return -1;

    /* Clean up the data structure */
    memset((void *)arg, 0, sizeof(RFC_struc_moduleData));

    /* Set some necessary inital values */
    strcpy(arg -> moduleName, moduleName);                          /* set the module name */    
    EPICSLIB_func_scanIoInit(&arg -> ioscanpvt_120Hz);              /* init the I/O interrupt scan list */
    
    return 0;
}

/**
 * Distruction of the instance of the module
 * Input:
 *     arg         : Data structure of the module instance
 * Return:
 *     0           : Successful
 *    -1           : Failed
 */
int RFC_func_destroyModule(RFC_struc_moduleData *arg)
{
    /* Check the input */
    if(!arg) return -1;

    /* stop the thread (necessary?) */
    arg -> stopThread    = 1; 
    arg -> threadCreated = 0;    

    /* delete the dynamicly created things (note: the board module is created in other place, so not free here) */
    if(arg -> fwModule) free(arg -> fwModule);                          

    return 0;
}

/**
 * Init the module, including the initialization of the firmware specific codes (by calling 
 *   the virtual functions)
 * Input:
 *     arg         : Data structure of the module instance
 * Return:
 *     0           : Successful
 *    -1           : Failed
 */ 
int RFC_func_initModule(RFC_struc_moduleData *arg)
{
    long sampleNum_max = 0;

    /* check the input */
    if(!arg) return -1;

    /* init the firmware specific data */
    if(arg -> fwFunc.FWC_func_init)
        arg -> fwFunc.FWC_func_init(arg -> fwModule);

    /* associate the board handle with the firmware (make sure they are associated regardless of the 
     * sequence to set the firmware module and board module
     */
    RFC_func_associateBoardModule(arg, arg -> boardModuleName);

    /* get the firmware supported max waveform sample number (which will decide the EPICS waveform record NELM) */
    if(arg -> fwFunc.FWC_func_getMaxSampleNum)
        arg -> fwFunc.FWC_func_getMaxSampleNum(&sampleNum_max);

    /* init the waveforms */              
    RFLIB_initRFWaveform(&arg -> rfData_ref,                 sampleNum_max);
    RFLIB_initRFWaveform(&arg -> rfData_vmOut,               sampleNum_max);
    RFLIB_initRFWaveform(&arg -> rfData_klyDrive,            sampleNum_max);
    RFLIB_initRFWaveform(&arg -> rfData_klyOut,              sampleNum_max);
    RFLIB_initRFWaveform(&arg -> rfData_sledOut,             sampleNum_max);
    RFLIB_initRFWaveform(&arg -> rfData_accOut_rf,           sampleNum_max);
    RFLIB_initRFWaveform(&arg -> rfData_accOut_beam,         sampleNum_max);    
    RFLIB_initAnalogWaveform(&arg -> analogData_klyBeamV,    sampleNum_max);

    /* init the sync DAQ list */
    SDAQ_func_createDataNode(&arg -> fbData.fb_phaErr_deg);
    SDAQ_func_createDataNode(&arg -> fbData.fb_phaAdj_deg);

    RFC_func_createBSANode_rfWaveform(&arg -> rfData_ref);
    RFC_func_createBSANode_rfWaveform(&arg -> rfData_vmOut);
    RFC_func_createBSANode_rfWaveform(&arg -> rfData_klyDrive);
    RFC_func_createBSANode_rfWaveform(&arg -> rfData_klyOut);
    RFC_func_createBSANode_rfWaveform(&arg -> rfData_sledOut);
    RFC_func_createBSANode_rfWaveform(&arg -> rfData_accOut_rf);
    RFC_func_createBSANode_rfWaveform(&arg -> rfData_accOut_beam);

    RFC_func_createBSANode_analogWaveform(&arg -> analogData_klyBeamV);

    return 0; 
}

/**
 * Associate the module with a RFControlBoard module and the firmware specific control module
 * Input:
 *     arg              : Data structure of the module instance
 *     boardModuleName  : Name of the RFControlBoard module
 * Return:
 *     0                : Successful
 *    -1                : Failed 
 */
int RFC_func_associateBoardModule(RFC_struc_moduleData *arg, const char *boardModuleName)
{
    /* Check the input */
    if(!arg || !boardModuleName ||!boardModuleName[0]) return -1;
    
    /* Save the module name and get its address */
    strcpy(arg -> boardModuleName, boardModuleName);   
    
    if(arg -> fwFunc.FWC_func_getBoardHandle) {
        arg -> boardHandle = arg -> fwFunc.FWC_func_getBoardHandle(boardModuleName);
    
        if(arg -> boardHandle != NULL) {        
            /* associate the board handle with the firmware module */
            if(arg -> fwFunc.FWC_func_assBoardHandle)
                arg -> fwFunc.FWC_func_assBoardHandle(arg -> fwModule, arg -> boardHandle);

            return 0;
        } else {
            return -1;
        }
    }

    return 0;
}

/**
 * Set the thread priority
 * Input:
 *     arg              : Data structure of the module instance
 *     priority         : Priority of the thread
 * Return:
 *     0                : Successful
 *    -1                : Failed 
 */
int RFC_func_setThreadPriority(RFC_struc_moduleData *arg, unsigned int priority)
{
    /* Check the input */
    if(!arg || priority < 0 || priority > 99) return -1;

    /* Set the priority */
    arg -> threadPriority = priority;

    /* If the thread exists, change the priority */
    if(arg -> threadCreated && arg -> localThread) {
        EPICSLIB_func_threadSetPriority(arg -> localThread, priority);        
    }
    
    return 0;
}

/**
 * Create and start a thread for this module
 * Input:
 *     arg              : Data structure of the module instance
 * Return:
 *     0                : Successful
 *    -1                : Failed 
 */
int RFC_func_createThread(RFC_struc_moduleData *arg)
{
    char buf_threadNameStr[128] = "RFC_thread_";

    /* Check the input */
    if(!arg) return -1;

    /* If the thread is already on, simply return */
    if(arg -> threadCreated && arg -> localThread) return 0;

    /* Create the thread */
    strcat(buf_threadNameStr, arg -> moduleName);
    arg -> localThread   = EPICSLIB_func_threadCreate(buf_threadNameStr, arg -> threadPriority, RFC_func_mainThread, (void *)arg);
    arg -> threadCreated = 1;

    return 0;
}















