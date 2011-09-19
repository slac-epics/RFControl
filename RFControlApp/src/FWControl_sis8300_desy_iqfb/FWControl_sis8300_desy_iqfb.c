/****************************************************
 * FWControl_sis8300_desy_iqfb.c
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

#include "FWControl_sis8300_desy_iqfb.h"

/*======================================
 * Private Data and Routines
 *======================================*/     
/**
 * Get a single channel from the DAQ system, the chId can be any value between 0 - 2 * FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_NUM (please note that
 * the channel here are defined for 16bit data, while the buffer in FPGA are defined for 32 bit data!)
 */
static int FWC_sis8300_desy_iqfb_func_getDAQSingleChannel(unsigned int *DAQData, int chId, short *data)
{
    int status = 0;
    int var_bufId;

    /* Find the location of the 32 bits data in the DAQ buffer */
    var_bufId = (int)(chId / 2);

    /* Check the buffer number, must be smaller than the maximum number */
    if(var_bufId >= FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_NUM) return -1;

    /* Get the data */
    if(var_bufId * 2 == chId) {
        /* means, the chId = 0,2,4,..., get the higher 16 bits */
        status = MATHLIB_u32ToShortArray(DAQData + var_bufId * FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH, FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH, data, NULL);
    } else {
        /* means, the chId = 1,3,5,..., get the lower 16 bits */
        status = MATHLIB_u32ToShortArray(DAQData + var_bufId* FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH, FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH, NULL, data);
    }

    return status;
}

/**
 * Get the two channel at the same time, the chId must be 0,2,4,...
 */
static int FWC_sis8300_desy_iqfb_func_getDAQDoubleChannel(unsigned int *DAQData, int chId, short *dataHi, short *dataLo)
{
    int status = 0;
    int var_bufId;

    /* Find the location of the 32 bits data in the DAQ buffer */
    var_bufId = (int)(chId / 2);

    /* Check the buffer number, must be smaller than the maximum number */
    if(var_bufId >= FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_NUM) return -1;

    /* Get the data */
    status = MATHLIB_u32ToShortArray(DAQData + var_bufId * FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH, FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH, dataHi, dataLo);

    return status;
}
                   
/*======================================
 * Public Routines
 *======================================*/
/**
 * Init the firmware data
 */
int FWC_sis8300_desy_iqfb_func_init(void *module)
{    
    FWC_sis8300_desy_iqfb_struc_data *arg = (FWC_sis8300_desy_iqfb_struc_data *)module;

    /* check input */
    if(!arg) return -1;

    /* Clean all data */
    memset(module, 0, sizeof(FWC_sis8300_desy_iqfb_struc_data));

    /* Init the local waveforms */
    RFLIB_initRFWaveform(&arg -> rfData_DACOut,        FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH);
    
    RFLIB_initRFWaveform(&arg -> rfData_refCh,         FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH);
    RFLIB_initRFWaveform(&arg -> rfData_fbkCh,         FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH);
    
    RFLIB_initRFWaveform(&arg -> rfData_refCh_rotated, FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH);
    RFLIB_initRFWaveform(&arg -> rfData_fbkCh_rotated, FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH);
    
    RFLIB_initRFWaveform(&arg -> rfData_tracked,       FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH);
    
    RFLIB_initRFWaveform(&arg -> rfData_err,           FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH);
    RFLIB_initRFWaveform(&arg -> rfData_err_acc,       FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH);
    
    RFLIB_initRFWaveform(&arg -> rfData_act,           FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH);
    RFLIB_initRFWaveform(&arg -> rfData_act_rotated,   FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH);

    return 0;       
}

/**
 * Associate the firmware module with the board control module
 */ 
int FWC_sis8300_desy_iqfb_func_assBoard(void *module, void *boardHandle)
{
    FWC_sis8300_desy_iqfb_struc_data *arg = (FWC_sis8300_desy_iqfb_struc_data *)module;

    if(!arg) return -1;

    arg -> board_handle = boardHandle;

    return 0;
}

/**
 * Get the maximum sample number supported
 */
int FWC_sis8300_desy_iqfb_func_getMaxSampleNum(long *pno_max)
{
   if(!pno_max) return -1;

   *pno_max = FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH;

   return 0;
}

/**
 * Trigger the DAQ data reading
 */
int FWC_sis8300_desy_iqfb_func_getDAQData(void *module)
{
    FWC_sis8300_desy_iqfb_struc_data *arg = (FWC_sis8300_desy_iqfb_struc_data *)module;

    if(!arg) return -1;

    FWC_sis8300_desy_iqfb_func_getAllDAQData(arg -> board_handle, arg -> board_bufDAQ);
    
    return 0;
}

/**
 * Get ADC data (assume the ADC data is at channel 0-9 of the DAQ)
 */
int FWC_sis8300_desy_iqfb_func_getADCData(void *module, unsigned long channel, short *data, double *sampleFreq_MHz, double *sampleDelay_ns, long *pno, long *coefIdCur)
{
    FWC_sis8300_desy_iqfb_struc_data *arg = (FWC_sis8300_desy_iqfb_struc_data *)module;

    if(!arg || !data || channel > 9) return -1;

    /* get data */
    FWC_sis8300_desy_iqfb_func_getDAQSingleChannel(arg -> board_bufDAQ, (int)channel, data);

    *sampleFreq_MHz = arg -> board_sampleFreq_MHz;
    *sampleDelay_ns = arg -> board_DAQTriggerDelay_ns;
    *pno            = FWC_SIS8300_DESY_IQFB_CONST_DAQ_BUF_DEPTH;
    *coefIdCur      = 0;

    return 0;
}

/**
 * Get the internal waveforms
 */
int FWC_sis8300_desy_iqfb_func_getIntData(void *module)
{
    int status = 0;

    FWC_sis8300_desy_iqfb_struc_data *arg = (FWC_sis8300_desy_iqfb_struc_data *)module;

    /* check the input */
    if(!arg) return -1;

    /* fill all waveforms */
    status += FWC_sis8300_desy_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_DACOut.chId,        arg -> rfData_DACOut.wfI,        arg -> rfData_DACOut.wfQ);

    status += FWC_sis8300_desy_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_refCh.chId,         arg -> rfData_refCh.wfI,         arg -> rfData_refCh.wfQ);
    status += FWC_sis8300_desy_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_fbkCh.chId,         arg -> rfData_fbkCh.wfI,         arg -> rfData_fbkCh.wfQ);

    status += FWC_sis8300_desy_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_refCh_rotated.chId, arg -> rfData_refCh_rotated.wfI, arg -> rfData_refCh_rotated.wfQ);
    status += FWC_sis8300_desy_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_fbkCh_rotated.chId, arg -> rfData_fbkCh_rotated.wfI, arg -> rfData_fbkCh_rotated.wfQ);

    status += FWC_sis8300_desy_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_tracked.chId,       arg -> rfData_tracked.wfI,       arg -> rfData_tracked.wfQ);

    status += FWC_sis8300_desy_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_err.chId,           arg -> rfData_err.wfI,           arg -> rfData_err.wfQ);
    status += FWC_sis8300_desy_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_err_acc.chId,       arg -> rfData_err_acc.wfI,       arg -> rfData_err_acc.wfQ);

    status += FWC_sis8300_desy_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_act.chId,           arg -> rfData_act.wfI,           arg -> rfData_act.wfQ);
    status += FWC_sis8300_desy_iqfb_func_getDAQDoubleChannel(arg -> board_bufDAQ, (int)arg -> rfData_act_rotated.chId,   arg -> rfData_act_rotated.wfI,   arg -> rfData_act_rotated.wfQ);

    return status;
}

/**
 * Set the phase adjustment
 */
int FWC_sis8300_desy_iqfb_func_setPha_deg(void *module, double pha_deg)
{
    FWC_sis8300_desy_iqfb_struc_data *arg = (FWC_sis8300_desy_iqfb_struc_data *)module;

    /* check the input */
    if(!arg) return -1;

    /* Apply the adjustement to: 1). measurement chain rotation; 2). driving chain rotation; 3). reference signal rotation (Integral control) */
    arg -> board_refRotationAngle_deg -= pha_deg;
    arg -> board_fbkRotationAngle_deg -= pha_deg;
    arg -> board_actRotationAngle_deg += pha_deg;

    if(arg -> board_handle) {
        FWC_sis8300_desy_iqfb_func_setRefVectorRotation(arg -> board_handle, arg -> board_refRotationGain, arg -> board_refRotationAngle_deg);
        FWC_sis8300_desy_iqfb_func_setFbkVectorRotation(arg -> board_handle, arg -> board_fbkRotationGain, arg -> board_fbkRotationAngle_deg);
        FWC_sis8300_desy_iqfb_func_setActVectorRotation(arg -> board_handle, arg -> board_actRotationGain, arg -> board_actRotationAngle_deg);
    }

    return 0;
}

/**
 * Set amplitude (not supported now)
 */
int FWC_sis8300_desy_iqfb_func_setAmp(void *module, double amp)
{
}

/**
 * Wait for the interrupt
 */
int FWC_sis8300_desy_iqfb_func_waitIntr(void *module)
{
    FWC_sis8300_desy_iqfb_struc_data *arg = (FWC_sis8300_desy_iqfb_struc_data *)module;

    /* check the input */
    if(!arg) return -1;

    /* wait the interrupt */
    FWC_sis8300_desy_iqfb_func_pullInterrupt(arg -> board_handle);

    return 0;
}

/**
 * Measure the latency of the intrrupt 
 */
int FWC_sis8300_desy_iqfb_func_meaIntrLatency(void *module, long *latencyCnt, long *pulseCnt)
{
    unsigned int data       = 0;
    unsigned int dataRead   = 0;
    unsigned int dataRead2  = 0;

    FWC_sis8300_desy_iqfb_struc_data *arg = (FWC_sis8300_desy_iqfb_struc_data *)module;

    /* check the input */
    if(!arg) return -1;

    /* stop the counter */
    data = 0x0000004A;    
    FWC_sis8300_desy_iqfb_func_setBits(arg->board_handle, data);    

    /* read the register */            
    RFCB_API_readRegister((RFCB_struc_moduleData *)arg->board_handle, CON_REG_ADDR_IRQ_DELAY_CNT, &dataRead);
    RFCB_API_readRegister((RFCB_struc_moduleData *)arg->board_handle, CON_REG_ADDR_PUL_CNT,       &dataRead2);

    printf("latency counter = %d, pulse counter = %d\n", dataRead, dataRead2);

    /* enable the counter */                    
    data = 0x000000CA; 
    FWC_sis8300_desy_iqfb_func_setBits(arg->board_handle, data);  

    return 0;
}












