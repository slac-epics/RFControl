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
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2/13/2013
 * Description: Use the RFControlFirmware module APIs for firmware related control. Here directly use the functions of the 
 *              firmware control module and do not use wrapper function is to reduce a level of function call
 ****************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "perfMeasure.h"

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
    /* Check the input */
    if(!arg || !data) return -1;

    /* Get the data */   
    return RFCFW_API_getADCData(arg -> firmwareModule, data -> chId, 
                                                       data -> wfRaw, 
                                                      &data -> sampleFreq_MHz,
                                                      &data -> sampleDelay_ns,
                                                      &data -> pointNum,
                                                      &data -> demodCoefIdCur);
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
    status = RFCFW_API_getADCData(arg -> firmwareModule, data -> chId, 
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
    perfParm_ts *perf_pMain       = makePerfMeasure("MAINLOOP",   "main thread loop time");
    perfParm_ts *perf_pDAQ        = makePerfMeasure("DAQ",        "  get all DAQ data in the main thread");
    perfParm_ts *perf_pPhCtrl     = makePerfMeasure("PHCTRL",     "  phase control block in the main thread");
    perfParm_ts *perf_pPhCtrlData = makePerfMeasure("PHCTRLDATA", "    data get for phase control block in the main thread");
    perfParm_ts *perf_pNetDAQ     = makePerfMeasure("NETDAQ",     "      +just net DAQ getting time");
    perfParm_ts *perf_pRFDemo     = makePerfMeasure("RFDEMO",     "      +RF demodulation calc time");
    perfParm_ts *perf_pPhCtrlCalc = makePerfMeasure("PHCTRLCALC", "    calculation for the phase control in the main thread");
    perfParm_ts *perf_pBSA        = makePerfMeasure("BSA",        "  BSA session");

    perfParm_ts *perf_pWFDiag     = makePerfMeasure("WFDIAG",     "  diag. for waveform in the main thread");
    perfParm_ts *perf_pOthDiag    = makePerfMeasure("OTHDIAG",    "  diag. for others in the main thread");

    RFC_struc_moduleData *arg = (RFC_struc_moduleData *)argIn;

    int calCnt = 0;                             /* for scheduling the not important calculation */

    int dataId = -1;                            /* for data BSA */
    int wfId   = -1;                            /* for waveform BSA */

    int intId;                                  /* for interrupt pulling */

    double phaSetPoint_deg_old = 1e6;           /* for detecting set point changes */
    double phaSetPoint_deg_cal = 0;             /* for error calculation */

    double phaOffset_deg_old   = 1e6;           /* for detecting phase offset changes */

    long pulseCnt     = -1;                     /* for pulse counter reading */
    long pulseCnt_old = -1; 

    long irqDelayCnt  = 0;

    double probeData  = 0;                      /* the probe data */
    double probeData2 = 0;

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
        intId = RFCFW_API_waitIntr(arg -> firmwareModule);

        if(intId == 0) {                                                /* User interrupt occurred */                     
        } else {
            epicsThreadSleep(epicsThreadSleepQuantum());
            continue;
        }

        startPerfMeasure(perf_pMain);
        startPerfMeasure(perf_pDAQ);
        /*----------------------------------------------------
         * DATA ACQUISITION BLOCK
         *----------------------------------------------------*/ 
        /* get all DAQ data */
        RFCFW_API_getDAQData(arg -> firmwareModule);        
        endPerfMeasure(perf_pDAQ);


        startPerfMeasure(perf_pPhCtrl);
        startPerfMeasure(perf_pPhCtrlData);
        /*----------------------------------------------------
         * PHASE CONTROL BLOCK
         *----------------------------------------------------*/ 
        /* Get the RF data */
        startPerfMeasure(perf_pNetDAQ);
        RFC_func_getRFData(arg, &arg -> rfData_ref);
        RFC_func_getRFData(arg, &arg -> rfData_sledOut);                        /* get the SLED data from the DAQ buffer */
        RFC_func_getRFData(arg, &arg -> rfData_accOut_rf);                      /* get the ACC data from the DAQ buffer */
        endPerfMeasure(perf_pNetDAQ);

        /* Measure the amplitude and phase in the window */
        startPerfMeasure(perf_pRFDemo);
        RFC_func_demodAvgRFData(&arg -> rfData_ref);
        RFC_func_demodAvgRFData(&arg -> rfData_sledOut);                        
        RFC_func_demodAvgRFData(&arg -> rfData_accOut_rf);
        endPerfMeasure(perf_pRFDemo);

        endPerfMeasure(perf_pPhCtrlData);

        startPerfMeasure(perf_pPhCtrlCalc);

        /* Get the effective phase measurement of last RF pulse (combination of SLED, ACC RF and REF) */
        arg -> fbData.fb_pha_deg = arg -> rfData_sledOut.avgDataPha_deg * arg -> fbData.fb_phaSLEDWeight + arg -> rfData_accOut_rf.avgDataPha_deg * arg -> fbData.fb_phaACCWeight - arg -> fbData.fb_phaOffset_deg;

        if(arg -> fbData.fb_refTrackEnabled) {
            arg -> fbData.fb_pha_deg -= arg -> rfData_ref.avgDataPha_deg;
        }

        if(arg -> fbData.fb_pha_deg >  180) arg -> fbData.fb_pha_deg -= 360;                        /* normalize to +-180 range */
        if(arg -> fbData.fb_pha_deg < -180) arg -> fbData.fb_pha_deg += 360;

        /* Fast feed forward when the set point changes */
        if(arg -> fbData.fb_feedForwardEnabled && 
           ((arg -> fbData.fb_phaSetPoint_deg != phaSetPoint_deg_old && fabs(phaSetPoint_deg_old) <= 180) || 
            (arg -> fbData.fb_phaOffset_deg   != phaOffset_deg_old   && fabs(phaOffset_deg_old)   <= 180))) {

            /*arg -> fbData.fb_phaAdj_deg = arg -> fbData.fb_phaSetPoint_deg - phaSetPoint_deg_old;*/                                   /* Get the adjustement, might have problem when the old set point is arbitrary */              
            arg -> fbData.fb_phaAdj_deg = arg -> fbData.fb_phaSetPoint_deg - arg -> fbData.fb_pha_deg;                                  /* use this solution for fast feedforward */
            RFCFW_API_setPha_deg(arg -> firmwareModule, arg -> fbData.fb_phaAdj_deg);                                                   /* Apply the adjustement to the firmware */

            phaSetPoint_deg_cal = phaSetPoint_deg_old;
        } else {
            phaSetPoint_deg_cal = arg -> fbData.fb_phaSetPoint_deg;
        }        

        /* Measure the phase error, if the set point changed, use the old set point for calculation, see above */
        if(arg -> fbData.fb_phaSLEDWeight != 0 || arg -> fbData.fb_phaACCWeight != 0) {
            arg -> fbData.fb_phaErr_deg = phaSetPoint_deg_cal - arg -> fbData.fb_pha_deg;
        } else {
            arg -> fbData.fb_phaErr_deg = 0; 
        }

        if(arg -> fbData.fb_phaErr_deg >  180) arg -> fbData.fb_phaErr_deg -= 360;                  /* normalize to +-180 range */
        if(arg -> fbData.fb_phaErr_deg < -180) arg -> fbData.fb_phaErr_deg += 360;

        /* Get data for monitoring */
        arg -> fbData.fb_amp_MV     = arg -> rfData_sledOut.avgDataAmp * arg -> fbData.fb_ampScale_1oMV;                             /* we use the SLED output amplitude for amplitude calculation, so we only scale it to MV */
        arg -> fbData.fb_ampErr_MV  = arg -> fbData.fb_ampSetPoint_MV - arg -> fbData.fb_amp_MV;

        /* Fast feedback, the feedback is independent with the feed forward */
        if((fabs(arg -> fbData.fb_phaErr_deg) < arg -> fbData.fb_phaErrThreshold_deg) &&
            (arg -> rfData_sledOut.avgDataAmp > arg -> fbData.fb_ampLimitLo) &&
            (arg -> rfData_sledOut.avgDataAmp < arg -> fbData.fb_ampLimitHi) && (arg -> fbData.fb_feedbackEnabled)) { 
           
            arg -> fbData.fb_phaAdj_deg = arg -> fbData.fb_phaErr_deg * arg -> fbData.fb_phaGain;                                   /* Get the adjustement */
            RFCFW_API_setPha_deg(arg -> firmwareModule, arg -> fbData.fb_phaAdj_deg);                                               /* Apply the adjustement to the firmware */

        }

        /* remember the old set point */
        phaSetPoint_deg_old = arg -> fbData.fb_phaSetPoint_deg;
        phaOffset_deg_old   = arg -> fbData.fb_phaOffset_deg;

        endPerfMeasure(perf_pPhCtrlCalc);

        endPerfMeasure(perf_pPhCtrl);

        /*----------------------------------------------------
         * HANDLE RF WAVEFORMS FOR BSA
         *----------------------------------------------------*/ 
        /* reference signal, SLED out and ACC out rf has been handled above! */
        RFC_func_getRFData(arg, &arg -> rfData_vmOut);
        RFC_func_getRFData(arg, &arg -> rfData_klyDrive);
        RFC_func_getRFData(arg, &arg -> rfData_klyOut);
        RFC_func_getRFData(arg, &arg -> rfData_accOut_beam);
        RFC_func_getAnalogData(arg, &arg -> analogData_klyBeamV);
        
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

        startPerfMeasure(perf_pWFDiag);
                     
        /*----------------------------------------------------
         * WAVEFORMS DIAGNOSTICS
         *----------------------------------------------------*/
        /* round robin scheduling */
        switch(calCnt) {
            case 0:                    
                /* firmware intermediate waveforms */
                RFCFW_API_getIntData(arg -> firmwareModule);                
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

        endPerfMeasure(perf_pWFDiag);

        calCnt ++;
        if(calCnt > 7) calCnt = 0;

        startPerfMeasure(perf_pOthDiag);

        /*----------------------------------------------------
         * OTHER DIAGNOSTICS
         *----------------------------------------------------*/
        /* the recent history buffer */
        memcpy((void*)arg -> fbData.fb_phaErrArray_deg, (void *)(arg -> fbData.fb_phaErrArray_deg + 1), sizeof(double) * (RFC_CONST_RECENT_HISTORY_BUF_DEPTH - 1));
        arg -> fbData.fb_phaErrArray_deg[RFC_CONST_RECENT_HISTORY_BUF_DEPTH - 1] = arg -> fbData.fb_phaErr_deg;

        /* probe the internal data (two probe data, code is not optimized)*/
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

        switch(arg -> diag_probeDataSel2) {
            case 0:  probeData2 = arg -> fbData.fb_phaErr_deg;               strcpy(arg -> diag_probeStatus2, "Pul-pul FB pha err (deg)");    break;
            case 1:  probeData2 = arg -> fbData.fb_phaAdj_deg;               strcpy(arg -> diag_probeStatus2, "Pul-pul FB pha adj (deg)");    break;
            case 2:  probeData2 = arg -> rfData_ref.avgDataAmp;              strcpy(arg -> diag_probeStatus2, "REF amp");                     break;
            case 3:  probeData2 = arg -> rfData_ref.avgDataPha_deg;          strcpy(arg -> diag_probeStatus2, "REF pha (deg)");               break;
            case 4:  probeData2 = arg -> rfData_vmOut.avgDataAmp;            strcpy(arg -> diag_probeStatus2, "IQ MOD amp");                  break;
            case 5:  probeData2 = arg -> rfData_vmOut.avgDataPha_deg;        strcpy(arg -> diag_probeStatus2, "IQ MOD pha (deg)");            break;
            case 6:  probeData2 = arg -> rfData_klyDrive.avgDataAmp;         strcpy(arg -> diag_probeStatus2, "KLY DRV amp");                 break;
            case 7:  probeData2 = arg -> rfData_klyDrive.avgDataPha_deg;     strcpy(arg -> diag_probeStatus2, "KLY DRV pha (deg)");           break;
            case 8:  probeData2 = arg -> rfData_klyOut.avgDataAmp;           strcpy(arg -> diag_probeStatus2, "KLY OUT amp");                 break;
            case 9:  probeData2 = arg -> rfData_klyOut.avgDataPha_deg;       strcpy(arg -> diag_probeStatus2, "KLY OUT pha (deg)");           break;
            case 10: probeData2 = arg -> rfData_sledOut.avgDataAmp;          strcpy(arg -> diag_probeStatus2, "SLED OUT amp");                break;
            case 11: probeData2 = arg -> rfData_sledOut.avgDataPha_deg;      strcpy(arg -> diag_probeStatus2, "SLED OUT pha (deg)");          break;
            case 12: probeData2 = arg -> rfData_accOut_rf.avgDataAmp;        strcpy(arg -> diag_probeStatus2, "ACC RF amp");                  break;
            case 13: probeData2 = arg -> rfData_accOut_rf.avgDataPha_deg;    strcpy(arg -> diag_probeStatus2, "ACC RF pha (deg)");            break;
            case 14: probeData2 = arg -> rfData_accOut_beam.avgDataAmp;      strcpy(arg -> diag_probeStatus2, "ACC BEAM amp");                break;
            case 15: probeData2 = arg -> rfData_accOut_beam.avgDataPha_deg;  strcpy(arg -> diag_probeStatus2, "ACC BEAM pha (deg)");          break;
            case 16: probeData2 = arg -> analogData_klyBeamV.avgData;        strcpy(arg -> diag_probeStatus2, "KLY HV amp");                  break;
            default: probeData2 = 0; strcpy(arg -> diag_probeStatus2, "Not supported"); break;
        }

        memcpy((void*)arg -> diag_probeData, (void *)(arg -> diag_probeData + 1), sizeof(double) * (RFC_CONST_RECENT_HISTORY_BUF_DEPTH - 1));
        arg -> diag_probeData[RFC_CONST_RECENT_HISTORY_BUF_DEPTH - 1] = probeData;

        memcpy((void*)arg -> diag_probeData2, (void *)(arg -> diag_probeData2 + 1), sizeof(double) * (RFC_CONST_RECENT_HISTORY_BUF_DEPTH - 1));
        arg -> diag_probeData2[RFC_CONST_RECENT_HISTORY_BUF_DEPTH - 1] = probeData2;

        endPerfMeasure(perf_pOthDiag);

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
        
        arg -> statusVector += arg -> fbData.fb_feedbackEnabled     << 1;    
        arg -> statusVector += arg -> fbData.fb_feedForwardEnabled  << 2;
        arg -> statusVector += arg -> fbData.fb_refTrackEnabled     << 3;

        if(fabs(arg -> fbData.fb_phaErr_deg) >= arg -> fbData.fb_phaErrThreshold_deg)  arg -> statusVector += 1 << 4;
        if(arg -> rfData_sledOut.avgDataAmp  >= arg -> fbData.fb_ampLimitHi)           arg -> statusVector += 1 << 5;
        if(arg -> rfData_sledOut.avgDataAmp  <= arg -> fbData.fb_ampLimitLo)           arg -> statusVector += 1 << 6;
        if(arg -> fbData.fb_phaSLEDWeight > 0)                                         arg -> statusVector += 1 << 7;
        if(arg -> fbData.fb_phaACCWeight  > 0)                                         arg -> statusVector += 1 << 8;

        /* check the IRQ missing, including enable the IRQ in the firmware */
        RFCFW_API_meaIntrLatency(arg -> firmwareModule, &irqDelayCnt, &pulseCnt);                       /* get the IRQ delay and trigger counter from firmware */

        arg -> IRQDelayCnt = irqDelayCnt;

        arg -> IRQCnt ++;                                                                               /* IRQ counter (note maybe different with trigger counter value if there is missing IRQ) */
        if(pulseCnt - pulseCnt_old > 1 && pulseCnt_old > 0) arg -> IRQMissingCnt ++;                    /* detect missing IRQ */
        pulseCnt_old = pulseCnt;

        /*----------------------------------------------------
         * MISC
         *----------------------------------------------------*/
        /* stop the thread on request */
        if(arg -> stopThread) break;

        endPerfMeasure(perf_pMain);
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
    if(arg -> firmwareModule) free(arg -> firmwareModule);                          

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
    /* check the input */
    if(!arg) return -1;

    /* init the waveforms */              
    RFLIB_initRFWaveform(&arg -> rfData_ref,                 RFC_CONST_WF_PNO);
    RFLIB_initRFWaveform(&arg -> rfData_vmOut,               RFC_CONST_WF_PNO);
    RFLIB_initRFWaveform(&arg -> rfData_klyDrive,            RFC_CONST_WF_PNO);
    RFLIB_initRFWaveform(&arg -> rfData_klyOut,              RFC_CONST_WF_PNO);
    RFLIB_initRFWaveform(&arg -> rfData_sledOut,             RFC_CONST_WF_PNO);
    RFLIB_initRFWaveform(&arg -> rfData_accOut_rf,           RFC_CONST_WF_PNO);
    RFLIB_initRFWaveform(&arg -> rfData_accOut_beam,         RFC_CONST_WF_PNO);    
    RFLIB_initAnalogWaveform(&arg -> analogData_klyBeamV,    RFC_CONST_WF_PNO);

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
 * Associate the module with a RFControlFirmware module and the firmware specific control module
 * Input:
 *     arg                 : Data structure of the module instance
 *     firmwareModuleName  : Name of the RFControlBoard module
 * Return:
 *     0                : Successful
 *    -1                : Failed 
 */
int RFC_func_associateFirmwareModule(RFC_struc_moduleData *arg, const char *firmwareModuleName)
{
    /* Check the input */
    if(!arg || !firmwareModuleName ||!firmwareModuleName[0]) return -1;
    
    /* Save the module name and get its address */
    strcpy(arg -> firmwareModuleName, firmwareModuleName);   
    
    arg -> firmwareModule = RFCFW_API_getModule(firmwareModuleName);

    if(arg -> firmwareModule) return 0;
    else return -1;
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















