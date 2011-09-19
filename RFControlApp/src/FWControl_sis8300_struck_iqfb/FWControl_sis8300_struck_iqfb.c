/****************************************************
 * FWControl_sis8300_struck_iqfb.c
 * 
 * Realization of the virtual functions
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.07.07
 * Description: Initial creation
 *
 * Modified by: Zheqiao Geng
 * MOdified on: 2011.08.31
 * Description: Change the "wait interrupt" and "" to fit the user IRQ change for the firmware of LLRF_SIS8300-R0-1-5
 ****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FWControl_sis8300_struck_iqfb.h"

/*======================================
 * Private Data and Routines 
 *======================================*/   
/**
 * Get the DAQ data. Interpret the raw data read from the hardware to meaningful channels.
 * The Struck firmware provide 64 bits DAQ data bus, the data format read to a short array is like this:
 *      16 bit data 0:  DAQ channel 3
 *      16 bit data 1:  DAQ channel 2
 *      16 bit data 2:  DAQ channel 1
 *      16 bit data 3:  DAQ channel 0
 *      ...
 * Get a single channel from the DAQ system, the chId can be any value between 0 - 4 * FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_NUM (please note that
 * the channel here are defined for 16bit data, while the buffer in FPGA are defined for 64 bit data!)
 */
static int FWC_sis8300_struck_iqfb_func_getDAQSingleChannel(unsigned int *DAQData, int chId, short *data)
{
    int status = 0;
    int var_bufId;
    unsigned int *dataStart;                                     /* location of the data */
    short *dataStart_short;                                      /* make the location to short */

    int idOffset[4] = {3, 2, 1, 0};

    /* check the input */
    if(!DAQData || !data) return -1;

    /* Find the location of the data in the DAQ buffer (every 4 channels are mixed!) */
    var_bufId       = (int)(chId / 4);
    dataStart       = DAQData + var_bufId * 2 * FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH;
    dataStart_short = (short *)dataStart;

    /* Check the buffer number, must be smaller than the maximum number */
    if(var_bufId >= FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_NUM) return -1;

    /* Get the data */
    status = MATHLIB_getEveryNSubArray(dataStart_short, 4 * FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH, data, idOffset[chId - 4 * var_bufId], 4);

    return status;
}

/**
 * Get the two channel at the same time, the chId must be 0,2,4,...
 */
static int FWC_sis8300_struck_iqfb_func_getDAQDoubleChannel(unsigned int *DAQData, int chId, short *dataHi, short *dataLo)
{
    int status = 0;
    int var_bufId;
    unsigned int *dataStart;                                     /* location of the data */
    short *dataStart_short;                                      /* make the location to short */

    int idOffset[4] = {3, 2, 1, 0};

    /* check the input */
    if(!DAQData || !dataHi || !dataLo) return -1;

    /* Find the location of the data in the DAQ buffer (every 4 channels are mixed!) */
    var_bufId       = (int)(chId / 4);
    dataStart       = DAQData + var_bufId * 2 * FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH;
    dataStart_short = (short *)dataStart;

    /* Check the buffer number, must be smaller than the maximum number */
    if(var_bufId >= FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_NUM) return -1;

    /* Get the data */
    status += MATHLIB_getEveryNSubArray(dataStart_short, 4 * FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH, dataHi, idOffset[chId - 4 * var_bufId],     4);
    status += MATHLIB_getEveryNSubArray(dataStart_short, 4 * FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH, dataLo, idOffset[chId - 4 * var_bufId + 1], 4);

    return status;
}
                   
/*======================================
 * Public Routines (virtual function implementation)
 *======================================*/
/**
 * Init the firmware data
 */
int FWC_sis8300_struck_iqfb_func_init(void *module)
{    
    FWC_sis8300_struck_iqfb_struc_data *arg = (FWC_sis8300_struck_iqfb_struc_data *)module;

    /* check input */
    if(!arg) return -1;

    /* Clean all data */
    memset(module, 0, sizeof(FWC_sis8300_struck_iqfb_struc_data));

    /* Init the local waveforms */
    RFLIB_initRFWaveform(&arg -> rfData_refCh,         FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH);
    RFLIB_initRFWaveform(&arg -> rfData_fbkCh,         FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH);    
    
    RFLIB_initRFWaveform(&arg -> rfData_tracked,       FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH);
    RFLIB_initRFWaveform(&arg -> rfData_err,           FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH);
    
    RFLIB_initRFWaveform(&arg -> rfData_act,           FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH);
    RFLIB_initRFWaveform(&arg -> rfData_DACOut,        FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH);

    arg -> rfData_refCh.chId           = 0;
    arg -> rfData_fbkCh.chId           = 2;
  
    arg -> rfData_tracked.chId         = 4;
    arg -> rfData_err.chId             = 6;

    arg -> rfData_act.chId             = 8;
    arg -> rfData_DACOut.chId          = 10;

    /* Init others */
    arg -> board_ADC_data[0] = arg -> board_ADC0_raw;
    arg -> board_ADC_data[1] = arg -> board_ADC1_raw;
    arg -> board_ADC_data[2] = arg -> board_ADC2_raw;
    arg -> board_ADC_data[3] = arg -> board_ADC3_raw;
    arg -> board_ADC_data[4] = arg -> board_ADC4_raw;
    arg -> board_ADC_data[5] = arg -> board_ADC5_raw;
    arg -> board_ADC_data[6] = arg -> board_ADC6_raw;
    arg -> board_ADC_data[7] = arg -> board_ADC7_raw;
    arg -> board_ADC_data[8] = arg -> board_ADC8_raw;
    arg -> board_ADC_data[9] = arg -> board_ADC9_raw;    

    return 0;       
}

/**
 * Associate the firmware module with the board control module
 */ 
int FWC_sis8300_struck_iqfb_func_assBoard(void *module, void *boardHandle)
{
    FWC_sis8300_struck_iqfb_struc_data *arg = (FWC_sis8300_struck_iqfb_struc_data *)module;

    if(!arg) return -1;

    arg -> board_handle = boardHandle;

    return 0;
}

/**
 * Get the maximum sample number supported
 */
int FWC_sis8300_struck_iqfb_func_getMaxSampleNum(long *pno_max)
{
   if(!pno_max) return -1;

   *pno_max = FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH;

   return 0;
}

/**
 * Trigger the DAQ data reading
 */
int FWC_sis8300_struck_iqfb_func_getDAQData(void *module)
{
    FWC_sis8300_struck_iqfb_struc_data *arg = (FWC_sis8300_struck_iqfb_struc_data *)module;

    unsigned int coefId;

    if(!arg) return -1;

    if(arg -> board_handle) {
        /* get the BRAM data (RF Controller internal) */
        FWC_sis8300_struck_iqfb_func_getAllDAQData(arg -> board_handle, arg -> board_bufDAQ);

        /* get the DRAM data (ADC raw) */
        FWC_sis8300_struck_iqfb_func_getAllADCData(arg -> board_handle, (unsigned int)arg -> board_ADCSamplePno, 
                                                   arg -> board_ADC0_raw, arg -> board_ADC1_raw,
                                                   arg -> board_ADC2_raw, arg -> board_ADC3_raw,
                                                   arg -> board_ADC4_raw, arg -> board_ADC5_raw,
                                                   arg -> board_ADC6_raw, arg -> board_ADC7_raw,
                                                   arg -> board_ADC8_raw, arg -> board_ADC9_raw); 

        /* get the current coefficient id for demod in CPU */
        FWC_sis8300_struck_iqfb_func_getNonIQCoefCur(arg -> board_handle, &coefId);
        arg -> board_coefIdCur = (long)coefId;
    }

    /*int i;
    int var_cnt;

    var_cnt = 0;
    
    printf("-----------------------------------------------\n");
    for(i = 0; i < 128; i ++) {
        printf("0x%4X  ", *(arg -> board_ADC0_raw + i));
        var_cnt ++;
        if(var_cnt >= 8) {
            var_cnt = 0;
            printf("\n");
        }
    }*/

    return 0;
}

/**
 * Get ADC data (assume the ADC data is at channel 0-9 of the DAQ)
 */
int FWC_sis8300_struck_iqfb_func_getADCData(void *module, unsigned long channel, short *data, double *sampleFreq_MHz, double *sampleDelay_ns, long *pno, long *coefIdCur)
{
    FWC_sis8300_struck_iqfb_struc_data *arg = (FWC_sis8300_struck_iqfb_struc_data *)module;

    if(!arg || !data || channel > 9 || !sampleFreq_MHz || !sampleDelay_ns || !pno || !coefIdCur) return -1;

    /* get data */
    if(arg -> board_ADC_data[channel]) 
        memcpy((void *)data, (void *)arg -> board_ADC_data[channel], sizeof(short) * FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH);

    *sampleFreq_MHz = arg -> board_sampleFreq_MHz;
    *sampleDelay_ns = arg -> board_DAQTriggerDelay_ns;
    *pno            = FWC_SIS8300_STRUCK_IQFB_CONST_DAQ_BUF_DEPTH;
    *coefIdCur      = arg -> board_coefIdCur;

    return 0;
}

/**
 * Get the internal waveforms
 */
int FWC_sis8300_struck_iqfb_func_getIntData(void *module)
{
    int status = 0;

    FWC_sis8300_struck_iqfb_struc_data *arg = (FWC_sis8300_struck_iqfb_struc_data *)module;

    /* check the input */
    if(!arg) return -1;

    /* fill all waveforms */
    if(arg -> board_handle) {    
        status += FWC_sis8300_struck_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_refCh.chId,         arg -> rfData_refCh.wfI,         arg -> rfData_refCh.wfQ);
        status += FWC_sis8300_struck_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_fbkCh.chId,         arg -> rfData_fbkCh.wfI,         arg -> rfData_fbkCh.wfQ);

        status += FWC_sis8300_struck_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_tracked.chId,       arg -> rfData_tracked.wfI,       arg -> rfData_tracked.wfQ);
        status += FWC_sis8300_struck_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_err.chId,           arg -> rfData_err.wfI,           arg -> rfData_err.wfQ);

        status += FWC_sis8300_struck_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_act.chId,           arg -> rfData_act.wfI,           arg -> rfData_act.wfQ);
        status += FWC_sis8300_struck_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_DACOut.chId,        arg -> rfData_DACOut.wfI,        arg -> rfData_DACOut.wfQ);
    }

    return status;
}

/**
 * Set the phase adjustment
 */
int FWC_sis8300_struck_iqfb_func_setPha_deg(void *module, double pha_deg)
{
    FWC_sis8300_struck_iqfb_struc_data *arg = (FWC_sis8300_struck_iqfb_struc_data *)module;

    /* check the input */
    if(!arg) return -1;

    /* Apply the adjustement to: 1). measurement chain rotation; 2). driving chain rotation; 3). reference signal rotation (Integral control) */
    arg -> board_refRotationAngle_deg -= pha_deg;
    arg -> board_fbkRotationAngle_deg -= pha_deg;
    arg -> board_actRotationAngle_deg += pha_deg;

    arg -> board_refRotationGain = 0.9999;          /* these rotations are only used for fast phase rotation! */
    arg -> board_fbkRotationGain = 0.9999;
    arg -> board_actRotationGain = 0.9999;

    if(arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setRefVectorRotation(arg -> board_handle, arg -> board_refRotationGain, arg -> board_refRotationAngle_deg);
        FWC_sis8300_struck_iqfb_func_setFbkVectorRotation(arg -> board_handle, arg -> board_fbkRotationGain, arg -> board_fbkRotationAngle_deg);
        FWC_sis8300_struck_iqfb_func_setActVectorRotation(arg -> board_handle, arg -> board_actRotationGain, arg -> board_actRotationAngle_deg);
    }

    return 0;
}

/**
 * Set amplitude (not supported now)
 */
int FWC_sis8300_struck_iqfb_func_setAmp(void *module, double amp)
{
    return 0;
}

/**
 * Wait for the interrupt
 */
int FWC_sis8300_struck_iqfb_func_waitIntr(void *module)
{
    unsigned int data = 0;
    FWC_sis8300_struck_iqfb_struc_data *arg = (FWC_sis8300_struck_iqfb_struc_data *)module;

    /* check the input */
    if(!arg) return -1;

    /* wait the interrupt */
    if(arg -> board_handle)
        FWC_sis8300_struck_iqfb_func_pullInterrupt(arg -> board_handle);

    /* disable the interrupt */
    data += arg -> board_reset                  << 0;
    data += arg -> board_triggerSource          << 1;
    data += arg -> board_refTrackEnabled        << 2;
    data += arg -> board_DACOutputEnabled       << 3;
    data += arg -> board_DACConOutputEnabled    << 4;
    data += 0                                   << 5;               /* disable the IRQ */
    data += 1                                   << 6;               /* do not touch the IRQ latency counter (will be done in meaIntrLatency routine) */    
    data += arg -> board_DACOutSel              << 7;
    data += arg -> board_applyCoefIdOffset      << 8;    

    FWC_sis8300_struck_iqfb_func_setBits(arg -> board_handle, data);

    return 0;
}

/**
 * Measure the latency of the intrrupt, this is a temp implementation so we directly use the RFCB_API functions
 * Bit 6 of the switch control register is for enable/disable the IRQ delay measurement counter
 */
int FWC_sis8300_struck_iqfb_func_meaIntrLatency(void *module, long *latencyCnt, long *pulseCnt)
{
    unsigned int data       = 0;
    unsigned int dataRead   = 0;
    unsigned int dataRead2  = 0;

    FWC_sis8300_struck_iqfb_struc_data *arg = (FWC_sis8300_struck_iqfb_struc_data *)module;

    /* check the input */
    if(!arg || !latencyCnt || !pulseCnt) return -1;

    /* stop the counter (also enable the IRQ if enabled by the user) */
    data += arg -> board_reset                  << 0;
    data += arg -> board_triggerSource          << 1;
    data += arg -> board_refTrackEnabled        << 2;
    data += arg -> board_DACOutputEnabled       << 3;
    data += arg -> board_DACConOutputEnabled    << 4;
    data += arg -> board_IRQEnabled             << 5;    
    data += arg -> board_DACOutSel              << 7;
    data += arg -> board_applyCoefIdOffset      << 8;

    if(arg -> board_handle) {
        FWC_sis8300_struck_iqfb_func_setBits(arg->board_handle, data);    

        /* read the register */            
        RFCB_API_readRegister((RFCB_struc_moduleData *)arg->board_handle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_IRQ_DELAY_CNT, &dataRead);
        RFCB_API_readRegister((RFCB_struc_moduleData *)arg->board_handle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_PUL_CNT,       &dataRead2);

        *(latencyCnt) = (long)dataRead;
        *(pulseCnt)   = (long)dataRead2;

        /* enable the counter */                    
        data |= 0x00000040; 
        FWC_sis8300_struck_iqfb_func_setBits(arg->board_handle, data);  
    }

    return 0;
}












