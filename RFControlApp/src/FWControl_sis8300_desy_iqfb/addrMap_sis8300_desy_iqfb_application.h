/******************************************************
 * addrMap_sis8300_desy_iqfb_application.h
 * 
 * Automatically generated from pakg_RFController_config.vhd
 * Do not modify ... 
 * Mon Jun 13 18:53:59 2011
 ******************************************************/
#ifndef ADDRMAP_SIS8300_DESY_IQFB_APPLICATION_H
#define ADDRMAP_SIS8300_DESY_IQFB_APPLICATION_H

/*-------------------------------- */
/* constant definitions */
/*-------------------------------- */
/* Configuration for the platform modules and global settings */
/* ADC and DAC */
#define CON_ADC_DATA_SIZE       ( 16)                /* bits number of the ADC    */
#define CON_DAC_DATA_SIZE       ( 16)                /* bits number of the DAC */
#define CON_ADC_CHANNEL_NUM     ( 10)                /* channel number of the ADC */
#define CON_DAC_CHANNEL_NUM     ( 2)                 /* channel number of the DAC */

#define CON_ADC_DATA_MAX        ( 2**(CON_ADC_DATA_SIZE - 1))   /* maximum value of the ADC data (signed) */

/* register access     */
#define CON_REG_ADDR_SIZE       ( 32)                /* 32 bits register address bus */
#define CON_REG_DATA_SIZE       ( 32)                /* 32 bits register data bus */

/* buffer access     */
#define CON_BUF_ADDR_SIZE       ( 32)                /* 32 bits buffer address bus */
#define CON_BUF_DATA_SIZE       ( 32)                /* 32 bits buffer data bus     */

/* DSP data size */
#define CON_DSP_DATA_SIZE      ( 18)                 /* compatible with the DSP core in the FPGA, for main stream data */
#define CON_DSP_COEF_SIZE      ( 18)                 /* compatible with the DSP core in the FPGA, for coefficients */

/* configuration for the RF controller core modules */
/* common */

/* triggerHandling */
#define CON_MAX_TRIG_DELAY     ( 16)                 /* maximum trigger delay in clock cycles */

/* RFControlDAQ */
#define CON_DAQ_BUF_NUM        ( 8)                  /* number of DAQ buffers */
#define CON_DAQ_BUF_DEPTH      ( 512)                /* points of data acquisition */
#define CON_DAQ_BUF_DATA_SIZE  ( 32)                 /* 32 bits buffer (should be same as CON_BUF_DATA_SIZE) */

#define CON_DAQ_BUF_NUM_MAX    ( 15)                 /* maximu DAQ buffers (part of them will be connected to DAQ buffers)  */
#define CON_DAQ_CHN_DATA_SIZE  ( 16)                 /* 16 bits DAQ channel */

/* RFSignalDetection */
#define CON_ENA_NONIQ          ( 1)                  /* 1 to enable the non-IQ demodulation (119MHz for 25.5MHz, 14 points for 3 cycles) */

/* RFSignalControl     */
#define CON_CTRL_MAX_GAIN      ( 128)                /* maximum gain     */

/* Rotation table length */
#define CON_MEA_ROT_TAB_DEPTH  ( 1024)              /* rotation table for the reference tracking */
#define CON_DRV_ROT_TAB_DEPTH  ( 1024)              /* rotation table for the RF signal actuation */

/*-------------------------------- */
/* type definitions */
/*-------------------------------- */


/*-------------------------------- */
/* register addresses */
/* all registers are defined in the RFController_top module */
/* a). each module has 32 addresses reserved so that later they can be extended without redefining the address  */
/*     of other modules */
/*--------------------------------     */
#define CON_REG_ADDR_START             ( 128)                                   /* start address of the internal registers */

/* global */
#define CON_REG_ADDR_OFFSET_GLOBAL     ( CON_REG_ADDR_START + 0)       
#define CON_REG_ADDR_SWITCH_CTRL       ( CON_REG_ADDR_OFFSET_GLOBAL + 0)        /* W */

/* triggerHandling */
#define CON_REG_ADDR_OFFSET_TRIGH      ( CON_REG_ADDR_START + 32)    
#define CON_REG_ADDR_TRIG_EXT_DELAY    ( CON_REG_ADDR_OFFSET_TRIGH + 0)         /* W */
#define CON_REG_ADDR_TRIG_INT_PERIOD   ( CON_REG_ADDR_OFFSET_TRIGH + 1)         /* W */
#define CON_REG_ADDR_TRIG_INT_LENGTH   ( CON_REG_ADDR_OFFSET_TRIGH + 2)         /* W */
#define CON_REG_ADDR_RF_PULSE_LENGTH   ( CON_REG_ADDR_OFFSET_TRIGH + 3)         /* W */

/* RFControlDAQ */
#define CON_REG_ADDR_OFFSET_RFCDAQ     ( CON_REG_ADDR_START + 64)    

#define CON_REG_ADDR_DAQ_TRIG_DELAY    ( CON_REG_ADDR_OFFSET_RFCDAQ + 0)        /* W */

#define CON_REG_ADDR_IRQ_DELAY_CNT     ( CON_REG_ADDR_OFFSET_RFCDAQ + 1)        /* R  */
#define CON_REG_ADDR_PUL_CNT           ( CON_REG_ADDR_OFFSET_RFCDAQ + 2)        /* R  */

#define CON_REG_ADDR_DAQ_BUF_SEL_0     ( CON_REG_ADDR_OFFSET_RFCDAQ + 3)        /* W */
#define CON_REG_ADDR_DAQ_BUF_SEL_1     ( CON_REG_ADDR_OFFSET_RFCDAQ + 4)        /* W */
#define CON_REG_ADDR_DAQ_BUF_SEL_2     ( CON_REG_ADDR_OFFSET_RFCDAQ + 5)        /* W */
#define CON_REG_ADDR_DAQ_BUF_SEL_3     ( CON_REG_ADDR_OFFSET_RFCDAQ + 6)        /* W */
#define CON_REG_ADDR_DAQ_BUF_SEL_4     ( CON_REG_ADDR_OFFSET_RFCDAQ + 7)        /* W */
#define CON_REG_ADDR_DAQ_BUF_SEL_5     ( CON_REG_ADDR_OFFSET_RFCDAQ + 8)        /* W */
#define CON_REG_ADDR_DAQ_BUF_SEL_6     ( CON_REG_ADDR_OFFSET_RFCDAQ + 9)        /* W */
#define CON_REG_ADDR_DAQ_BUF_SEL_7     ( CON_REG_ADDR_OFFSET_RFCDAQ + 10)       /* W   */

/* ADCInterface */
#define CON_REG_ADDR_OFFSET_ADCIF      ( CON_REG_ADDR_START + 96)
#define CON_REG_ADDR_NOSIGNAL_THRESH   ( CON_REG_ADDR_OFFSET_ADCIF + 0)         /* W */

/* RFController_main */
#define CON_REG_ADDR_OFFSET_RFCM       ( CON_REG_ADDR_START + 128)

#define CON_REG_ADDR_REF_FBK_SEL       ( CON_REG_ADDR_OFFSET_RFCM + 0)         /* W */

#define CON_REG_ADDR_ROT_COEF_REF      ( CON_REG_ADDR_OFFSET_RFCM + 1)         /* W */
#define CON_REG_ADDR_ROT_COEF_FBK      ( CON_REG_ADDR_OFFSET_RFCM + 2)         /* W */

#define CON_REG_ADDR_SETPOINT_I        ( CON_REG_ADDR_OFFSET_RFCM + 3)         /* W */
#define CON_REG_ADDR_SETPOINT_Q        ( CON_REG_ADDR_OFFSET_RFCM + 4)         /* W */

#define CON_REG_ADDR_FEEDFORWARD_I     ( CON_REG_ADDR_OFFSET_RFCM + 5)         /* W */
#define CON_REG_ADDR_FEEDFORWARD_Q     ( CON_REG_ADDR_OFFSET_RFCM + 6)         /* W */

#define CON_REG_ADDR_GAIN_I            ( CON_REG_ADDR_OFFSET_RFCM + 7)         /* W */
#define CON_REG_ADDR_GAIN_Q            ( CON_REG_ADDR_OFFSET_RFCM + 8)         /* W */

#define CON_REG_ADDR_INTG_START        ( CON_REG_ADDR_OFFSET_RFCM + 9)         /* W */
#define CON_REG_ADDR_INTG_END          ( CON_REG_ADDR_OFFSET_RFCM + 10)        /* W */

#define CON_REG_ADDR_APPL_START        ( CON_REG_ADDR_OFFSET_RFCM + 11)        /* W */
#define CON_REG_ADDR_APPL_END          ( CON_REG_ADDR_OFFSET_RFCM + 12)        /* W */

#define CON_REG_ADDR_ROT_COEF_ACT      ( CON_REG_ADDR_OFFSET_RFCM + 13)        /* W */

#define CON_REG_ADDR_OFFSET_I          ( CON_REG_ADDR_OFFSET_RFCM + 14)        /* W */
#define CON_REG_ADDR_OFFSET_Q          ( CON_REG_ADDR_OFFSET_RFCM + 15)        /* W */

#define CON_REG_ADDR_AMP_LIMIT_HI      ( CON_REG_ADDR_OFFSET_RFCM + 16)        /* W */
#define CON_REG_ADDR_AMP_LIMIT_LO      ( CON_REG_ADDR_OFFSET_RFCM + 17)        /* W */

/*-------------------------------- */
/* buffer addresses */
/*-------------------------------- */
#define CON_BUF_ADDR_OFFSET            ( 2048)

/* table for referenceTracking (write only) */
#define CON_BUF_ADDR_ROT_TABLE_T_START ( CON_BUF_ADDR_OFFSET)
#define CON_BUF_ADDR_ROT_TABLE_T_END   ( CON_BUF_ADDR_ROT_TABLE_T_START + CON_MEA_ROT_TAB_DEPTH - 1)

/* table for RFSignalActuation (write only) */
#define CON_BUF_ADDR_ROT_TABLE_A_START ( CON_BUF_ADDR_ROT_TABLE_T_START + CON_MEA_ROT_TAB_DEPTH)
#define CON_BUF_ADDR_ROT_TABLE_A_END   ( CON_BUF_ADDR_ROT_TABLE_A_START + CON_DRV_ROT_TAB_DEPTH - 1)

/* DAQ buffers (read only) */
#define CON_BUF_ADDR_DAQ_BUF_START     ( CON_BUF_ADDR_ROT_TABLE_A_START + CON_DRV_ROT_TAB_DEPTH)
#define CON_BUF_ADDR_DAQ_BUF_END       ( CON_BUF_ADDR_DAQ_BUF_START + CON_DAQ_BUF_DEPTH * CON_DAQ_BUF_NUM - 1)    


#endif

