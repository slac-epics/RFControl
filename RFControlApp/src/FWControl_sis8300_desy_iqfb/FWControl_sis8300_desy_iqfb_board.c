/****************************************************
 * FWControl_sis8300_desy_iqfb_board.c
 * 
 * This is the interface with the the RFControlBoard module
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.07.07
 * Description: Initial creation
 ****************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "FWControl_sis8300_desy_iqfb_board.h"

/*-------------------------------------------------------------
 * COMMON FUNCTION
 *-------------------------------------------------------------*/
/**
 * Get the handle of the RFControlBoard (a pointer of the data structure of the module)
 * Input:
 *   boardModuleName    : Name of the RFControlBoard module
 * Return:
 *   Address with the type of void of the data structure of the module
 */
void *FWC_sis8300_desy_iqfb_func_getBoardHandle(const char *boardModuleName)
{
    return (void *)RFCB_API_getModule(boardModuleName);
}

/*-------------------------------------------------------------
 * PLATFORM FIRMWARE SETTINGS/READINGS
 *-------------------------------------------------------------*/
/**
 * Get the board information. Including the firmware name (4 char string), major and minor version number and the build number
 */
void FWC_sis8300_desy_iqfb_func_getBoardInfo(void *boardHandle, unsigned int *firmwareName, unsigned int *majorVer, unsigned int *minorVer, 
                           unsigned int *buildNum, unsigned int *serialNum)
{
    unsigned int var_version;
    
    /* read the registers */    
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_FRIMWARE_NAME,     firmwareName);
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_FIRMWARE_VERSION,  &var_version);    
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_MODULE_SERIAL_NO,  serialNum);
    
    /* build up the data */   
    *majorVer   = (var_version >> 24) & 0xFF;
    *minorVer   = (var_version >> 16) & 0xFF;
    *buildNum   = var_version & 0xFFFF;
}

/**
 * Get the status report of the platform firmware
 */
void FWC_sis8300_desy_iqfb_func_getBoardStat(void *boardHandle, unsigned int *watchDogCnt, unsigned int *alarmVector, 
                           unsigned int *statusVector, unsigned int *ad9510Status, unsigned int *ADCStatus)
{
    /* read registers */
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_WATCHDOG_COUNTER,  watchDogCnt);
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_MODULE_ALARM,      alarmVector);   
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_MODULE_STATUS,     statusVector);   
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_AD9510_STATUS,     ad9510Status);   
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_ADC_STATUS,        ADCStatus);   
}

/**
 * Select the harlink/AMC LVDS output data source, 1 means output from the FPGA internal logic, 0 directly from register
 */
void FWC_sis8300_desy_iqfb_func_setHarlinkOutSource(void *boardHandle, unsigned int src)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_HARLINK_OUT_SEL, src);       
}

void FWC_sis8300_desy_iqfb_func_setAMCLVDSOutSource(void *boardHandle, unsigned int src)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_AMC_LVDS_OUT_SEL, src);       
}

/**
 * Set the Harlink and AMC LVDS output with register (register controlled digital output)
 * Harlink:  bit 3:0 for output channel 3:0
 * AMC LVDS: bit 7:0 for output channel 7:0, bit 15:8 for enabling output 7:0
 */
void FWC_sis8300_desy_iqfb_func_setHarlinkOutData(void *boardHandle, unsigned int data)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_HARLINK_OUT, data);       
}

void FWC_sis8300_desy_iqfb_func_setAMCLVDSOutData(void *boardHandle, unsigned int data)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_AMC_LVDS_OUT, data);
}

/**
 * Get the Harlink and AMC LVDS input
 * Harlink:  bit 3:0 for input channel 3:0
 * AMC LVDS: bit 7:0 for input channel 7:0
 */
void FWC_sis8300_desy_iqfb_func_getHarlinkInData(void *boardHandle, unsigned int *data)
{
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_HARLINK_IN, data); 
}

void FWC_sis8300_desy_iqfb_func_getAMCLVDSInData(void *boardHandle, unsigned int *data)
{
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_AMC_LVDS_IN, data);
}

/**
 * Set the selection of the ADC clock source
 * Input value (src):
 *      0:      RTM CLK2
 *      1:      Backplane CLK1
 *      2:      Backplane CLK2
 *      3:      EXTCLKA (Front Panel SMA clock)
 *      4:      EXTCLKB (Harlink)
 *      5:      Oscillator (250MHz)
 */
void FWC_sis8300_desy_iqfb_func_setADCClockSource(void *boardHandle, unsigned int src)
{
    unsigned int data;
    
    switch(src) {
        case 0: data = 0x00000000; break;
        case 1: data = 0x0000000A; break;
        case 2: data = 0x00000005; break;
        case 3: data = 0x00000F00; break;
        case 4: data = 0x00000A00; break;
        case 5: data = 0x0000000F; break;
        default: data = 0;    
    }
    
    printf("FWC_sis8300_desy_iqfb_func_setADCClockSource: data = %d\n", data);

    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_ADC_CLK_SEL, data);
}

/**
 * Set the external trigger source
 * Inputs:
 *      adcThreshold:   if we use the ADC channel for trigger detection, this is the threshold for trigger
 *      src:
 *          0 - 3:  harlink input 0-3
 *          4 - 11: amc lvds input 0 - 7
 *          12:     adc ch9
 */ 
void FWC_sis8300_desy_iqfb_func_setExtTriggerSource(void *boardHandle, unsigned int src, unsigned int adcThreshold)
{
    unsigned int data = src + (adcThreshold << 16);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_SET_TRIG, data);
}
    
/**
 * Set the DAC data source (for 2 DACs) 
 * Input (src):
 *      0:  RF actuation signal
 *      1:  simulation klystron/sled output
 *      2:  ADC data 0,1
 *      3:  ADC data 2,3
 *      4:  ADC data 4,5
 *      5:  ADC data 6,7
 *      6:  ADC data 8,9
 *      7:  trigger pulse
 */    
void FWC_sis8300_desy_iqfb_func_setDACDataSource(void *boardHandle, unsigned int src)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_DAC_OUT_SEL, src);    
}

/**
 * Set the internal start address for DMA
 */
void FWC_sis8300_desy_iqfb_func_setDMAStartAddr(void *boardHandle, unsigned int addr)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_DMA_ADDR_START, addr);        
}
 
/**
 * Setup the AD9510, ADC chip and DAC chip. 
 * This function is provided to set all above to default values, which are enough for 
 *   the LLRF operation. Later if necessay, we will provide the possiblity to set them
 *   based on user's requirements
 * 
 * AD9510 channel configuration:
 * - chip 1:
 *      -- Out 0-3  not used
 *      -- Out 4    (FPGA DIV-CLK05)            : LVDS 3.5 mA
 *      -- Out 5    (ADC3-CLK, ch4/5)           : LVDS 3.5 mA
 *      -- Out 6    (ADC2-CLK, ch3/4)           : LVDS 3.5 mA
 *      -- Out 7    (ADC1-CLK, ch1/2)           : LVDS 3.5 mA
 * - chip 2:	
 *      -- Out 0-3  not used
 *      -- Out 4    (FPGA DIV-CLK69)            : LVDS 3.5 mA
 *      -- Out 5    (ADC5-CLK, ch8/9)           : LVDS 3.5 mA
 *      -- Out 6    (ADC4-CLK, ch6/7)           : LVDS 3.5 mA
 *      -- Out 7    (Frontpanel Clk, Harlink)   : LVDS 3.5 mA
 */
#define AD9510_GENERATE_FUNCTION_PULSE_CMD              0x80000000
#define AD9510_GENERATE_SPI_RW_CMD                      0x40000000
#define AD9510_SPI_SELECT_NO2                           0x01000000
#define ADC_GENERATE_SPI_RW_CMD                         0x40000000
#define SLEEP_TIME                                      10000

void FWC_sis8300_desy_iqfb_func_setSPI(void *boardHandle)
{
    RFCB_struc_moduleData *board = (RFCB_struc_moduleData *)boardHandle;
    
    /* address and data for register writing */
    unsigned int addr_offset;
    unsigned int data;
    
    /* data for AD9510 (clock divider) setting, for each array data
     *	bits <3:0>:   Divider High
     *	bits <7:4>:   Divider Low
     *	bits <11:8>:  Phase Offset
     *	bit  <12>:    Select Start High
     *	bit  <13>:    Force 
     *	bit  <14>:    Nosyn (individual) 
     *	bit  <15>:    Bypass Divider 
     *
     *	i=0:	AD9510 #1 Out 7  (ADC1-CLK, ch1/2)
     *	i=1:	AD9510 #1 Out 6  (ADC2-CLK, ch3/4)
     *	i=2:	AD9510 #1 Out 5  (ADC3-CLK, ch4/5)
     *	i=3:	AD9510 #2 Out 6  (ADC4-CLK, ch6/7)
     *	i=4:	AD9510 #2 Out 5  (ADC5-CLK, ch8/9)
     *	i=5:	AD9510 #2 Out 7  (Frontpanel Clk, Harlink)
     *	i=6:	AD9510 #1 Out 4  (FPGA DIV-CLK05) used for synch. of external Triggers
     *	i=7:	AD9510 #2 Out 4  (FPGA DIV-CLK69) used for sychn. of AD910 ISc
     *
     * Some possible commands:
     *       divided by 1 (by pass):
     *           div_bypass = 0x8000;
     *           divide_cmd = 0x00;
     *       divided by 2: 
     *           div_bypass = 0x0000;
     *           divide_cmd = 0x00;
     *       divided by 4: 
     *           div_bypass = 0x0000;
     *           divide_cmd = 0x11;
     *       divided by 6: 
     *           div_bypass = 0x0000;
     *           divide_cmd = 0x22;
     *       divided by 8: 
     *           div_bypass = 0x0000;
     *           divide_cmd = 0x33;
     *       divided by 10: 
     *           div_bypass = 0x0000;
     *           divide_cmd = 0x44;
     *       divided by 12: 
     *           div_bypass = 0x0000;
     *           divide_cmd = 0x55;
     *       divided by 14: 
     *           div_bypass = 0x0000;
     *           divide_cmd = 0x66;
     *       divided by 16: 
     *           div_bypass = 0x0000;
     *           divide_cmd = 0x77;
     *       divided by 18: 
     *           div_bypass = 0x0000;
     *           divide_cmd = 0x88;
     */
    unsigned int ad9510_divider_configuration_array[8];
    unsigned int div_bypass = 0x8000;                                 /* bypass the divider */    
    unsigned int divide_cmd = 0x00;
    
    /* data for ADC setting */
    unsigned int i;
    unsigned int uint_adc_mux_select;
    unsigned int adc_addr;
    unsigned int adc_data;
    
    /* set the value of the array */
    ad9510_divider_configuration_array[0] = div_bypass + divide_cmd ;  /* (ADC1-CLK, ch1/2) */
    ad9510_divider_configuration_array[1] = div_bypass + divide_cmd ;  /* (ADC2-CLK, ch3/4) */
    ad9510_divider_configuration_array[2] = div_bypass + divide_cmd ;  /* (ADC3-CLK, ch4/5) */
    ad9510_divider_configuration_array[3] = div_bypass + divide_cmd ;  /* (ADC4-CLK, ch6/7) */
    ad9510_divider_configuration_array[4] = div_bypass + divide_cmd ;  /* (ADC5-CLK, ch8/9) */
    ad9510_divider_configuration_array[5] = div_bypass + divide_cmd ;  /* (Frontpanel Clk, Harlink) */
    ad9510_divider_configuration_array[6] = div_bypass + divide_cmd ;  /* (FPGA DIV-CLK05) used for synch. of external Triggers */
    ad9510_divider_configuration_array[7] = 0xC000 + 0x00;             /* (FPGA DIV-CLK69) not used */
    
    /*----------------------------------------
     * Set ADC chip
     *----------------------------------------*/
    /* get the address of the register for ADC */
    addr_offset = CON_REG_ADDR_ADC_SPI_DATA;

    for(i = 0; i < 5; i ++) {
        uint_adc_mux_select = i << 24;

        /* output type LVDS */
        adc_addr = (0x14 & 0xffff) << 8 ;
        adc_data = (0x40 & 0xff)  ;
        data = uint_adc_mux_select + adc_addr + adc_data + ADC_GENERATE_SPI_RW_CMD;
        RFCB_API_writeRegister(board, addr_offset, data);        
        usleep(SLEEP_TIME) ;
        
        adc_addr = (0x16 & 0xffff) << 8 ;
        adc_data = (0x00 & 0xff)  ;
        data = uint_adc_mux_select + adc_addr + adc_data + ADC_GENERATE_SPI_RW_CMD;
        RFCB_API_writeRegister(board, addr_offset, data);
        usleep(SLEEP_TIME) ;

        adc_addr = (0x17 & 0xffff) << 8 ;
        adc_data = (0x00 & 0xff)  ;
        data = uint_adc_mux_select + adc_addr + adc_data + ADC_GENERATE_SPI_RW_CMD;
        RFCB_API_writeRegister(board, addr_offset, data);
        usleep(SLEEP_TIME) ;

        /* register update cmd */
        adc_addr = (0xff & 0xffff) << 8 ;
        adc_data = (0x01 & 0xff)  ;
        data = uint_adc_mux_select + adc_addr + adc_data + ADC_GENERATE_SPI_RW_CMD;
        RFCB_API_writeRegister(board, addr_offset, data);
        usleep(SLEEP_TIME) ;

        /* disable the data writing */
        data = 0;
        RFCB_API_writeRegister(board, addr_offset, data);
        usleep(SLEEP_TIME) ;
    }

    /*----------------------------------------
     * Set ADC clock
     *----------------------------------------*/
    /* get the address of the register for AD9510 */
    addr_offset = CON_REG_ADDR_AD9510_SPI_DATA;

    /* === AD9510 No1 === */
    /* set AD9510 to Bidirectional Mode  */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x0090;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME);

    /* default: Asychrnon PowerDown, no Prescaler  */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x0A01;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME);

    /* Out 0 (not used) : total Power-Down  */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x3C0B;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME);

    /* Out 1 (not used) : total Power-Down  */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x3D0B;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME);

    /* Out 2 (not used) : total Power-Down  */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x3E0B;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME);

    /* Out 3 (not used) : total Power-Down  */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x3F0B;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME);

    /* Out 4  (FPGA DIV-CLK05) : LVDS 3.5 mA  */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x4002;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME);

    /* Out 5  (ADC3-CLK, ch4/5) : LVDS 3.5 mA  */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x4102;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME);

    /* Out 6  (ADC2-CLK, ch3/4) : LVDS 3.5 mA  */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x4202;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME);

    /* Out 7  (ADC1-CLK, ch1/2) : LVDS 3.5 mA  */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x4302;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME);

    /* set the first chip */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x4500;
    data += 0x10 ; /* Power-Down RefIn */
    data += 0x08 ; /* Shut Down Clk to PLL Prescaler */
    data += 0x04 ; /* Power-Down CLK2 */
    data += 0x01 ; /* CLK1 Drives Distribution Section */
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME);

    /* Out 4  (FPGA DIV-CLK05) : Divider Low/High */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x5000;                     /* addr */
    data = data + (ad9510_divider_configuration_array[6] & 0xff) ;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 4  (FPGA DIV-CLK05) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0) */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x5100;                             /* addr */
    data = data + ((ad9510_divider_configuration_array[6] >> 8) & 0xff) ;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 5  (ADC3-CLK, ch4/5) : Divider Low/High */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x5200;                     /* addr */
    data = data + (ad9510_divider_configuration_array[2] & 0xff) ;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 5  (ADC3-CLK, ch4/5) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0) */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x5300;                             /* addr */
    data = data + ((ad9510_divider_configuration_array[2] >> 8) & 0xff);
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 6  (ADC2-CLK, ch2/3) : Divider Low/High */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x5400;                     /* addr */
    data = data + (ad9510_divider_configuration_array[1] & 0xff) ;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 6  (ADC2-CLK, ch2/3) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0) */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x5500;                             /* addr */
    data = data + ((ad9510_divider_configuration_array[1] >> 8) & 0xff);
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 7  (ADC1-CLK, ch1/2) : Divider Low/High */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x5600;                     /* addr */
    data = data + (ad9510_divider_configuration_array[0] & 0xff) ;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

     /* Out 7  (ADC1-CLK, ch1/2) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0) */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x5700;                             /* addr */
    data = data + ((ad9510_divider_configuration_array[0] >> 8) & 0xff);
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* set Function of "Function pin" to SYNCB (Default Reset) */
    data =  AD9510_GENERATE_SPI_RW_CMD + 0x5822;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* update command */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x5A01;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* === AD9510 No2 (ADC channels 7 to 10) === */
    /* set AD9510 to Bidirectional Mode  */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0090;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0A01;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 0 (not used) : total Power-Down */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3C0B;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 1 (not used) : total Power-Down */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3D0B;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 2 (not used) : total Power-Down */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3E0B;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 3 (not used) : total Power-Down */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3F0B;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 4  (FPGA DIV-CLK69) : LVDS 3.5 mA */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4002;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 5  (ADC5-CLK, ch8/9) : LVDS 3.5 mA */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4102;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 6  (ADC4-CLK, ch6/7) : LVDS 3.5 mA */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4202;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 7  (Frontpanel Clk, Harlink) : LVDS 3.5 mA */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4302; /* on  */
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4500; /* addr */
    data = data + 0x10 ; /* Power-Down RefIn */
    data = data + 0x08 ; /* Shut Down Clk to PLL Prescaler */
    data = data + 0x04 ; /* Power-Down CLK2 */
    data = data + 0x01 ; /* CLK1 Drives Distribution Section */
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 4  (FPGA DIV-CLK69) : Divider Low/High */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5000; /* addr */
    data = data + (ad9510_divider_configuration_array[7] & 0xff) ;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 4  (FPGA DIV-CLK69) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0) */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5100; /* addr */
    data = data + ((ad9510_divider_configuration_array[7] >> 8) & 0xff);
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 5  (ADC5-CLK, ch8/9) : Divider Low/High */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5200; /* addr */
    data = data + (ad9510_divider_configuration_array[4] & 0xff) ;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 5  (ADC5-CLK, ch8/9) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0) */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5300; /* addr */
    data = data + ((ad9510_divider_configuration_array[4] >> 8) & 0xff);
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 6  (ADC4-CLK, ch6/7) : Divider Low/High */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5400; /* addr */
    data = data + (ad9510_divider_configuration_array[3] & 0xff) ;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 6  (ADC4-CLK, ch6/7) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0) */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5500; /* addr */
    data = data + ((ad9510_divider_configuration_array[3] >> 8) & 0xff);
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 7  (Frontpanel Clk, Harlink) : Divider Low/High */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5600; /* addr */
    data = data + (ad9510_divider_configuration_array[5] & 0xff) ;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* Out 7  (Frontpanel Clk, Harlink) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0) */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5700;/* addr */
    data = data + ((ad9510_divider_configuration_array[5] >> 8) & 0xff);
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* set Function of "Function pin" to SYNCB (Default Reset) */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5822;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* update command */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5A01;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* === synch Cmd === */
    /* generate sych Pulse   (pulse Function pins) */
    data = AD9510_GENERATE_FUNCTION_PULSE_CMD;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;
}

/*-------------------------------------------------------------
 * RF CONTROLLER FIRMWARE SETTINGS/READINGS
 *-------------------------------------------------------------*/
/**
 * Set bits register in the FPGA, the values start from bit 0
 */
void  FWC_sis8300_desy_iqfb_func_setBits(void *boardHandle, unsigned int data)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_SWITCH_CTRL, data);   /* Write to the register */
}

/**
 * Set the external trigger delay
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ns           : Delay value in ns
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_desy_iqfb_func_setExtTrigDelay(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_TRIG_EXT_DELAY, data);    
}

/**
 * Set the internal trigger period
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ns           : Trigger period value in ns
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_desy_iqfb_func_setIntTrigPeriod(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_TRIG_INT_PERIOD, data);        
}

/**
 * Set the internal trigger length (high)
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ns           : Trigger length value in ns
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_desy_iqfb_func_setIntTrigLength(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_TRIG_INT_LENGTH, data);     
}

/**
 * Set the RF pulse length
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ns           : Pulse length value in ns
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_desy_iqfb_func_setRFPulseLength(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_RF_PULSE_LENGTH, data);     
}

/**
 * Set the DAQ trigger delay
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ns           : DAQ trigger delay value in ns
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_desy_iqfb_func_setDAQTrigDelay(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_DAQ_TRIG_DELAY, data);     
}

/**
 * Set the DAQ channel data selection
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   channel            : DAQ channel to be set up
 *   sel                : FPGA internal data selection
 */
void  FWC_sis8300_desy_iqfb_func_setDAQChSel(void *boardHandle, unsigned int channel, unsigned int sel)
{    
    switch(channel) {
        case 0:  RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_DAQ_BUF_SEL_0,  sel); break;
        case 1:  RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_DAQ_BUF_SEL_1,  sel); break;
        case 2:  RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_DAQ_BUF_SEL_2,  sel); break;
        case 3:  RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_DAQ_BUF_SEL_3,  sel); break;
        case 4:  RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_DAQ_BUF_SEL_4,  sel); break;
        case 5:  RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_DAQ_BUF_SEL_5,  sel); break;
        case 6:  RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_DAQ_BUF_SEL_6,  sel); break;
        case 7:  RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_DAQ_BUF_SEL_7,  sel); break;
        default: break;        
    }
}

/**
 * Set the threshold to detect signal missing in the ADC channel
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value              : The threshold in digit (if ADC values smaller than it, we determine the signal missing)
 */
void  FWC_sis8300_desy_iqfb_func_setADCNoSignalThreshold(void *boardHandle, unsigned int value)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_NOSIGNAL_THRESH, value);
}

/**
 * Select the reference and feedback channel. The higher 16 bits for reference channel and the lower 16 bits for feedback channel
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   refCh              : The channel ID of the reference signal
 *   fbkCh              : The channel ID of the feedback signal
 */ 
void  FWC_sis8300_desy_iqfb_func_selectRefFbkChannel(void *boardHandle, unsigned int refCh, unsigned int fbkCh)
{
    unsigned int data = (refCh << 16) + (fbkCh & 0x0000FFFF);       
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_REF_FBK_SEL, data);    
}

/**
 * Set the rotation coefficients for the reference signal. The higher 16 bits is for cos and the lower 16 bits for sin. They all
 *   have 15 bits of fraction
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   scale              : Scale factor of the reference signal (should be in the range of [0, 1))
 *   rotAngle_deg       : Rotation angle (radian) of the reference siganl
 */
void  FWC_sis8300_desy_iqfb_func_setRefVectorRotation(void *boardHandle, double scale, double rotAngle_deg)
{    
    unsigned int cs   = (unsigned int)(scale * cos(RFLIB_degToRad(rotAngle_deg)) * pow(2, FWC_SIS8300_DESY_IQFB_CONST_ROT_COEF_FRACTION));
    unsigned int sn   = (unsigned int)(scale * sin(RFLIB_degToRad(rotAngle_deg)) * pow(2, FWC_SIS8300_DESY_IQFB_CONST_ROT_COEF_FRACTION));    
    unsigned int data = (cs << 16) + (sn & 0x0000FFFF);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_ROT_COEF_REF, data);    
}

/**
 * Set the rotation coefficients for the feedback signal. The higher 16 bits is for cos and the lower 16 bits for sin. They all
 *   have 15 bits of fraction
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   scale              : Scale factor of the feedback signal (should be in the range of [0, 1))
 *   rotAngle_deg       : Rotation angle (radian) of the feedback siganl
 */
void  FWC_sis8300_desy_iqfb_func_setFbkVectorRotation(void *boardHandle, double scale, double rotAngle_deg)
{
    unsigned int cs   = (unsigned int)(scale * cos(RFLIB_degToRad(rotAngle_deg)) * pow(2, FWC_SIS8300_DESY_IQFB_CONST_ROT_COEF_FRACTION));
    unsigned int sn   = (unsigned int)(scale * sin(RFLIB_degToRad(rotAngle_deg)) * pow(2, FWC_SIS8300_DESY_IQFB_CONST_ROT_COEF_FRACTION));    
    unsigned int data = (cs << 16) + (sn & 0x0000FFFF);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_ROT_COEF_FBK, data);      
}

/**
 * Set the set point values
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_MV           : Set point in MV
 *   factor             : Factor to scale the physical unit to FPGA data 
 */
void  FWC_sis8300_desy_iqfb_func_setSetPoint_I(void *boardHandle, double value_MV, double factor)
{
    unsigned int data = (unsigned int)(value_MV * factor);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_SETPOINT_I, data);          
}

void  FWC_sis8300_desy_iqfb_func_setSetPoint_Q(void *boardHandle, double value_MV, double factor)
{
    unsigned int data = (unsigned int)(value_MV * factor);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_SETPOINT_Q, data);          
}

/**
 * Set the feed forward values
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_MV           : Feed forward vlues in MV
 *   factor             : Factor to scale the physical unit to FPGA data 
 */
void  FWC_sis8300_desy_iqfb_func_setFeedforward_I(void *boardHandle, double value_MV, double factor)
{
    unsigned int data = (unsigned int)(value_MV * factor);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_FEEDFORWARD_I, data);     
}

void  FWC_sis8300_desy_iqfb_func_setFeedforward_Q(void *boardHandle, double value_MV, double factor)
{
    unsigned int data = (unsigned int)(value_MV * factor);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_FEEDFORWARD_Q, data);     
}

/**
 * Set the gain for the feedback
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value              : Gain value
 */
void  FWC_sis8300_desy_iqfb_func_setGain_I(void *boardHandle, double value)
{
    unsigned int data = (unsigned int)(value * pow(2, FWC_SIS8300_DESY_IQFB_CONST_GAIN_FRACTION));
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_GAIN_I, data);
}

void  FWC_sis8300_desy_iqfb_func_setGain_Q(void *boardHandle, double value)
{
    unsigned int data = (unsigned int)(value * pow(2, FWC_SIS8300_DESY_IQFB_CONST_GAIN_FRACTION));
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_GAIN_Q, data);
}

/**
 * Set the time window for the integration
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ns           : Time value in ns
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_desy_iqfb_func_setIntgStart(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_INTG_START, data);       
}

void  FWC_sis8300_desy_iqfb_func_setIntgEnd(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_INTG_END, data);       
}            

/**
 * Set the time window for applying correction
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ns           : Time value in ns
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_desy_iqfb_func_setApplStart(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_APPL_START, data);       
}    

void  FWC_sis8300_desy_iqfb_func_setApplEnd(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_APPL_END, data);         
}

/**
 * Set the rotation coefficients for the actuation signal. The higher 16 bits is for cos and the lower 16 bits for sin. They all
 *   have 15 bits of fraction
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   scale              : Scale factor of the actuation signal (should be in the range of [0, 1))
 *   rotAngle_deg       : Rotation angle (radian) of the actuation siganl
 */
void  FWC_sis8300_desy_iqfb_func_setActVectorRotation(void *boardHandle, double scale, double rotAngle_deg)
{
    unsigned int cs   = (unsigned int)(scale * cos(RFLIB_degToRad(rotAngle_deg)) * pow(2, FWC_SIS8300_DESY_IQFB_CONST_ROT_COEF_FRACTION));
    unsigned int sn   = (unsigned int)(scale * sin(RFLIB_degToRad(rotAngle_deg)) * pow(2, FWC_SIS8300_DESY_IQFB_CONST_ROT_COEF_FRACTION));    
    unsigned int data = (cs << 16) + (sn & 0x0000FFFF);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_ROT_COEF_ACT, data);     
}

/**
 * Set the offset of the DAC
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   offset             : Offset in digits
 */
void  FWC_sis8300_desy_iqfb_func_setDACOffset_I(void *boardHandle, unsigned int offset)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_OFFSET_I, offset);
}

void  FWC_sis8300_desy_iqfb_func_setDACOffset_Q(void *boardHandle, unsigned int offset)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_OFFSET_Q, offset);
}

/**
 * Set the limits for DAC output signal
 *   boardHandle        : Address of the data structure of the board moudle
 *   limit              : Limit value in digits
 */
void  FWC_sis8300_desy_iqfb_func_setAmpLimitHi(void *boardHandle, unsigned int limit)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_AMP_LIMIT_HI , limit);
}

void  FWC_sis8300_desy_iqfb_func_setAmpLimitLo(void *boardHandle, unsigned int limit)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_REG_ADDR_AMP_LIMIT_LO , limit);
}

/**
 * Set the measurement chain rotoation table. The higher 16 bits is for cos and the lower 16 bits for sin. They all
 *   have 15 bits of fraction
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   pno                : Number of the points
 *   scaleTable         : Table of the scales (should be [0, 1))
 *   rotAngleTable_deg  : Table of the rotation angle in radian
 *   data               : The buffer to store the data to be written to the FPGA, it should be designed externally and have enough size
 */
void  FWC_sis8300_desy_iqfb_func_setMeaRotationTable(void *boardHandle, unsigned int pno, double *scaleTable, double *rotAngleTable_deg, unsigned int *data)
{
    unsigned int i;
    unsigned int cs;
    unsigned int sn;
  
    /* Make up the data */
    for(i = 0; i < pno; i ++) {
        cs = (unsigned int)(*(scaleTable + i) * cos(RFLIB_degToRad(*(rotAngleTable_deg + i))) * pow(2, FWC_SIS8300_DESY_IQFB_CONST_ROT_COEF_FRACTION));
        sn = (unsigned int)(*(scaleTable + i) * sin(RFLIB_degToRad(*(rotAngleTable_deg + i))) * pow(2, FWC_SIS8300_DESY_IQFB_CONST_ROT_COEF_FRACTION)); 
        *(data + i) = (cs << 16) + (sn & 0x0000FFFF);
    }
    
    /* Write to FPGA */
    RFCB_func_writeBuffer((RFCB_struc_moduleData *)boardHandle, CON_BUF_ADDR_ROT_TABLE_T_START, pno, data);
}

/**
 * Set the driving chain rotoation table. The higher 16 bits is for cos and the lower 16 bits for sin. They all
 *   have 15 bits of fraction
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   pno                : Number of the points
 *   scaleTable         : Table of the scales (should be [0, 1))
 *   rotAngleTable_deg  : Table of the rotation angle in radian
 *   data               : The buffer to store the data to be written to the FPGA, it should be designed externally and have enough size
 */
void  FWC_sis8300_desy_iqfb_func_setDrvRotationTable(void *boardHandle, unsigned int pno, double *scaleTable, double *rotAngleTable_deg, unsigned int *data)
{
    unsigned int i;
    unsigned int cs;
    unsigned int sn;
  
    /* Make up the data */
    for(i = 0; i < pno; i ++) {
        cs = (unsigned int)(*(scaleTable + i) * cos(RFLIB_degToRad(*(rotAngleTable_deg + i))) * pow(2, FWC_SIS8300_DESY_IQFB_CONST_ROT_COEF_FRACTION));
        sn = (unsigned int)(*(scaleTable + i) * sin(RFLIB_degToRad(*(rotAngleTable_deg + i))) * pow(2, FWC_SIS8300_DESY_IQFB_CONST_ROT_COEF_FRACTION)); 
        *(data + i) = (cs << 16) + (sn & 0x0000FFFF);
    }
    
    /* Write to FPGA */
    RFCB_func_writeBuffer((RFCB_struc_moduleData *)boardHandle, CON_BUF_ADDR_ROT_TABLE_A_START, pno, data);    
}

/**
 * Get all DAQ data from the FPGA
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   buf                : Buffer to store the data, not the buffer should be large enough to store all data
 */
void  FWC_sis8300_desy_iqfb_func_getAllDAQData(void *boardHandle, unsigned int *buf)
{
    RFCB_func_readBuffer((RFCB_struc_moduleData *)boardHandle, CON_BUF_ADDR_DAQ_BUF_START, CON_DAQ_BUF_DEPTH * CON_DAQ_BUF_NUM, buf);   
}










