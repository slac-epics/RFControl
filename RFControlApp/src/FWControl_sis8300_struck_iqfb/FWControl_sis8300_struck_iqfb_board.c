/****************************************************
 * FWControl_sis8300_struck_iqfb_board.c
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

#include "FWControl_sis8300_struck_iqfb_board.h"

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
void *FWC_sis8300_struck_iqfb_func_getBoardHandle(const char *boardModuleName)
{
    return (void *)RFCB_API_getModule(boardModuleName);
}

/**
 * Get the information of the board, the device name in the folder of "/dev" and if it is correctly opened
 */
void FWC_sis8300_struck_iqfb_func_getBoardInfo(void *boardHandle, char *deviceName, int *deviceOpened)
{
    RFCB_struc_moduleData *board = (RFCB_struc_moduleData *)boardHandle;

    RFCB_API_getModuleStatus(board, deviceName, deviceOpened);
}

/*-------------------------------------------------------------
 * PLATFORM FIRMWARE SETTINGS/READINGS
 *-------------------------------------------------------------*/
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
#define SLEEP_TIME 10

void FWC_sis8300_struck_iqfb_func_setSPI(void *boardHandle, unsigned int clkDiv2)
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
    unsigned int div_bypass;    
    unsigned int divide_cmd;
    
    /* data for ADC setting */
    unsigned int i;
    unsigned int uint_adc_mux_select;
    unsigned int adc_addr;
    unsigned int adc_data;
    
    /* set the divider */
    if(clkDiv2 == 1) {
        div_bypass = 0x0000;
        divide_cmd = 0x00;
    } else {
        div_bypass = 0x8000;
        divide_cmd = 0x00;
    }

    /* set the value of the array */
    ad9510_divider_configuration_array[0] = div_bypass + divide_cmd ;  /* (ADC1-CLK, ch1/2) */
    ad9510_divider_configuration_array[1] = div_bypass + divide_cmd ;  /* (ADC2-CLK, ch3/4) */
    ad9510_divider_configuration_array[2] = div_bypass + divide_cmd ;  /* (ADC3-CLK, ch4/5) */
    ad9510_divider_configuration_array[3] = div_bypass + divide_cmd ;  /* (ADC4-CLK, ch6/7) */
    ad9510_divider_configuration_array[4] = div_bypass + divide_cmd ;  /* (ADC5-CLK, ch8/9) */
    ad9510_divider_configuration_array[5] = div_bypass + divide_cmd ;  /* (Frontpanel Clk, Harlink) */
    ad9510_divider_configuration_array[6] = div_bypass + divide_cmd ;  /* (FPGA DIV-CLK05) used for synch. of external Triggers */
    ad9510_divider_configuration_array[7] = 0xC000 + 0x00;             /* (FPGA DIV-CLK69) used for sychn. of AD910 ISc and Bypass */

    /*----------------------------------------
     * Set ADC chip
     *----------------------------------------*/
    /* get the address of the register for ADC */
    addr_offset = SIS8300_ADC_SPI_REG;

    for(i = 0; i < 5; i ++) {
        uint_adc_mux_select = i << 24;

        /* output type LVDS */
        adc_addr = (0x14 & 0xffff) << 8 ;
        adc_data = (0x40 & 0xff)  ;
        data = uint_adc_mux_select + adc_addr + adc_data;
        RFCB_API_writeRegister(board, addr_offset, data);        
        usleep(SLEEP_TIME) ;
        
        adc_addr = (0x16 & 0xffff) << 8 ;
        adc_data = (0x00 & 0xff)  ;
        data = uint_adc_mux_select + adc_addr + adc_data;
        RFCB_API_writeRegister(board, addr_offset, data);
        usleep(SLEEP_TIME) ;

        adc_addr = (0x17 & 0xffff) << 8 ;
        adc_data = (0x00 & 0xff)  ;
        data = uint_adc_mux_select + adc_addr + adc_data;
        RFCB_API_writeRegister(board, addr_offset, data);
        usleep(SLEEP_TIME) ;

        /* register update cmd */
        adc_addr = (0xff & 0xffff) << 8 ;
        adc_data = (0x01 & 0xff)  ;
        data = uint_adc_mux_select + adc_addr + adc_data;
        RFCB_API_writeRegister(board, addr_offset, data);
        usleep(SLEEP_TIME) ;
    }

    /*----------------------------------------
     * Set ADC clock
     *----------------------------------------*/
    /* get the address of the register for AD9510 */
    addr_offset = SIS8300_AD9510_SPI_REG;

    /* === AD9510 No1 === */
    /* set AD9510 to Bidirectional Mode and Soft Reset */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x00B0;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME);

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

    /* update command */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x5A01;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* === AD9510 No2 (ADC channels 7 to 10) === */
    /* set AD9510 to Bidirectional Mode and Soft Reset */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x00B0;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

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

    /* update command */
    data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5A01;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* === synch Cmd === */
    /* set Function of "Function pin" to SYNCB (Default Reset) */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x5822;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* update command */
    data = AD9510_GENERATE_SPI_RW_CMD + 0x5A01;
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

    /* set use "FPGA DIV-CLK69" for Sychn Pulse  - Logic (VIRTEX5) */
    data = AD9510_SPI_SET_FUNCTION_SYNCH_FPGA_CLK69;                    /* set clock */
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;

    /* generate sych Pulse   (pulse Function pins) and hold FPGA_CLK69 to sycnh pulse */
    data = AD9510_GENERATE_FUNCTION_PULSE_CMD + AD9510_SPI_SET_FUNCTION_SYNCH_FPGA_CLK69;
    RFCB_API_writeRegister(board, addr_offset, data);
    usleep(SLEEP_TIME) ;
}

/**
 * Set the Harlink (see the manual for SIS8300 board)
 */
void FWC_sis8300_struck_iqfb_func_setHarlink(void *boardHandle, unsigned int data)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_HARLINK_IN_OUT_CONTROL_REG, data);       
}

/**
 * Set the AMC LVDS (see the manual for SIS8300 board)
 */
void FWC_sis8300_struck_iqfb_func_setAMCLVDS(void *boardHandle, unsigned int data)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_MLVDS_IO_CONTROL_REG, data);
}

/**
 * Get the Harlink (see the manual for SIS8300 board)
 */
void FWC_sis8300_struck_iqfb_func_getHarlink(void *boardHandle, unsigned int *data)
{
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_HARLINK_IN_OUT_CONTROL_REG, data); 
}

/**
 * Get the AMC LVDS (see the manual for SIS8300 board)
 */
void FWC_sis8300_struck_iqfb_func_getAMCLVDS(void *boardHandle, unsigned int *data)
{
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_MLVDS_IO_CONTROL_REG, data);
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
void FWC_sis8300_struck_iqfb_func_setADCClockSource(void *boardHandle, unsigned int src)
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

    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_CLOCK_DISTRIBUTION_MUX_REG, data);
}

/**
 * Get the platform firmware info (id and serial number)
 */
void FWC_sis8300_struck_iqfb_func_getPlatformInfo(void *boardHandle, unsigned int *id, unsigned int *sno)
{
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_INDENTIFIER_VERSION_REG, id);
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SERIAL_NUMBER_REG, sno);
}

/*-------------------------------------------------------------
 * RF CONTROLLER FIRMWARE SETTINGS
 *-------------------------------------------------------------*/
/**
 * Set bits register in the FPGA
 */
void  FWC_sis8300_struck_iqfb_func_setBits(void *boardHandle, unsigned int data)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_SWITCH_CTRL, data);   /* Write to the register */
}

/**
 * Set the external trigger delay
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ns           : Delay value in ns
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_struck_iqfb_func_setExtTrigDelay(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_TRIG_EXT_DELAY, data);    
}

/** 
 * Set the external period (min)
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ms           : Period value in ms
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_struck_iqfb_func_setExtTrigPeriod(void *boardHandle, double value_ms, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ms * freq_MHz * 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_TRIG_EXT_PERIOD, data);
}

/**
 * Set the internal trigger period
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ms           : Trigger period value in ms
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_struck_iqfb_func_setIntTrigPeriod(void *boardHandle, double value_ms, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ms * freq_MHz * 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_TRIG_INT_PERIOD, data);        
}

/**
 * Set the internal trigger length (high)
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ms           : Trigger length value in ms
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_struck_iqfb_func_setIntTrigLength(void *boardHandle, double value_ms, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ms * freq_MHz * 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_TRIG_INT_LENGTH, data);     
}

/**
 * Set the RF pulse length
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ns           : Pulse length value in ns
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_struck_iqfb_func_setRFPulseLength(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_RF_PULSE_LENGTH, data);     
}

/**
 * Set the DAQ trigger delay
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ns           : DAQ trigger delay value in ns
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_struck_iqfb_func_setDAQTrigDelay(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_DAQ_TRIG_DELAY, data);     
}

/**
 * Select the reference and feedback channel. The higher 16 bits for reference channel and the lower 16 bits for feedback channel
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   refCh              : The channel ID of the reference signal
 *   fbkCh              : The channel ID of the feedback signal
 */ 
void  FWC_sis8300_struck_iqfb_func_selectRefFbkChannel(void *boardHandle, unsigned int refCh, unsigned int fbkCh)
{
    unsigned int data = (refCh << 16) + (fbkCh & 0x0000FFFF);       
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_REF_FBK_SEL, data);    
}

/**
 * Set the rotation coefficients for the reference signal. The higher 16 bits is for cos and the lower 16 bits for sin. They all
 *   have 15 bits of fraction
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   scale              : Scale factor of the reference signal (should be in the range of [0, 1))
 *   rotAngle_deg       : Rotation angle (radian) of the reference siganl
 */
void  FWC_sis8300_struck_iqfb_func_setRefVectorRotation(void *boardHandle, double scale, double rotAngle_deg)
{    
    unsigned int cs   = (unsigned int)(scale * cos(RFLIB_degToRad(rotAngle_deg)) * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_ROT_COEF_FRACTION));
    unsigned int sn   = (unsigned int)(scale * sin(RFLIB_degToRad(rotAngle_deg)) * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_ROT_COEF_FRACTION));    
    unsigned int data = (cs << 16) + (sn & 0x0000FFFF);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_ROT_COEF_REF, data);    
}

/**
 * Set the rotation coefficients for the feedback signal. The higher 16 bits is for cos and the lower 16 bits for sin. They all
 *   have 15 bits of fraction
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   scale              : Scale factor of the feedback signal (should be in the range of [0, 1))
 *   rotAngle_deg       : Rotation angle (radian) of the feedback siganl
 */
void  FWC_sis8300_struck_iqfb_func_setFbkVectorRotation(void *boardHandle, double scale, double rotAngle_deg)
{
    unsigned int cs   = (unsigned int)(scale * cos(RFLIB_degToRad(rotAngle_deg)) * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_ROT_COEF_FRACTION));
    unsigned int sn   = (unsigned int)(scale * sin(RFLIB_degToRad(rotAngle_deg)) * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_ROT_COEF_FRACTION));    
    unsigned int data = (cs << 16) + (sn & 0x0000FFFF);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_ROT_COEF_FBK, data);      
}

/**
 * Set the rotation coefficients for the actuation signal. The higher 16 bits is for cos and the lower 16 bits for sin. They all
 *   have 15 bits of fraction
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   scale              : Scale factor of the actuation signal (should be in the range of [0, 1))
 *   rotAngle_deg       : Rotation angle (radian) of the actuation siganl
 */
void  FWC_sis8300_struck_iqfb_func_setActVectorRotation(void *boardHandle, double scale, double rotAngle_deg)
{
    unsigned int cs   = (unsigned int)(scale * cos(RFLIB_degToRad(rotAngle_deg)) * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_ROT_COEF_FRACTION));
    unsigned int sn   = (unsigned int)(scale * sin(RFLIB_degToRad(rotAngle_deg)) * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_ROT_COEF_FRACTION));    
    unsigned int data = (cs << 16) + (sn & 0x0000FFFF);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_ROT_COEF_ACT, data);     
}

/**
 * Set the set point values
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_MV           : Set point in MV
 *   factor             : Factor to scale the physical unit to FPGA data 
 */
void  FWC_sis8300_struck_iqfb_func_setSetPoint_I(void *boardHandle, double value_MV, double factor)
{
    unsigned int data = (unsigned int)(value_MV * factor);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_SETPOINT_I, data);          
}

void  FWC_sis8300_struck_iqfb_func_setSetPoint_Q(void *boardHandle, double value_MV, double factor)
{
    unsigned int data = (unsigned int)(value_MV * factor);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_SETPOINT_Q, data);          
}

/**
 * Set the feed forward values
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_MV           : Feed forward vlues in MV
 *   factor             : Factor to scale the physical unit to FPGA data 
 */
void  FWC_sis8300_struck_iqfb_func_setFeedforward_I(void *boardHandle, double value_MV, double factor)
{
    unsigned int data = (unsigned int)(value_MV * factor);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_FEEDFORWARD_I, data);     
}

void  FWC_sis8300_struck_iqfb_func_setFeedforward_Q(void *boardHandle, double value_MV, double factor)
{
    unsigned int data = (unsigned int)(value_MV * factor);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_FEEDFORWARD_Q, data);     
}

/**
 * Set the gain for the feedback
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value              : Gain value
 */
void  FWC_sis8300_struck_iqfb_func_setGain_I(void *boardHandle, double value)
{
    unsigned int data = (unsigned int)(value * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_GAIN_FRACTION));
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_GAIN_I, data);
}

void  FWC_sis8300_struck_iqfb_func_setGain_Q(void *boardHandle, double value)
{
    unsigned int data = (unsigned int)(value * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_GAIN_FRACTION));
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_GAIN_Q, data);
}

/**
 * Set the feedback error limits
 */
void  FWC_sis8300_struck_iqfb_func_setFeedbackErrLimits(void *boardHandle, unsigned int limit_i, unsigned int limit_q)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_ERR_LIMIT_I, limit_i);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_ERR_LIMIT_Q, limit_q);
}

/**
 * Set the time window for the integration
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ns           : Time value in ns
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_struck_iqfb_func_setIntgStart(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_INTG_START, data);       
}

void  FWC_sis8300_struck_iqfb_func_setIntgEnd(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_INTG_END, data);       
}            

/**
 * Set the time window for applying correction
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   value_ns           : Time value in ns
 *   freq_MHz           : Sampling frequency in MHz
 */
void  FWC_sis8300_struck_iqfb_func_setApplStart(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_APPL_START, data);       
}    

void  FWC_sis8300_struck_iqfb_func_setApplEnd(void *boardHandle, double value_ns, double freq_MHz)
{
    unsigned int data = (unsigned int)(value_ns * freq_MHz / 1000.0);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_APPL_END, data);         
}

/**
 * Set the offset of the DAC
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   offset             : Offset in digits
 */
void  FWC_sis8300_struck_iqfb_func_setDACOffset_I(void *boardHandle, unsigned int offset)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_OFFSET_I, offset);
}

void  FWC_sis8300_struck_iqfb_func_setDACOffset_Q(void *boardHandle, unsigned int offset)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_OFFSET_Q, offset);
}

/**
 * Set the limits for DAC output signal
 *   boardHandle        : Address of the data structure of the board moudle
 *   limit              : Limit value in digits
 */
void  FWC_sis8300_struck_iqfb_func_setAmpLimitHi(void *boardHandle, unsigned int limit)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_AMP_LIMIT_HI , limit);
}

void  FWC_sis8300_struck_iqfb_func_setAmpLimitLo(void *boardHandle, unsigned int limit)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_AMP_LIMIT_LO , limit);
}

/**
 * Set the measurement chain rotoation table. The higher 16 bits is for cos and the lower 16 bits for sin. They all
 *   have 15 bits of fraction
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   pno                : Number of the points
 *   scaleTable         : Table of the scales (should be [0, 1))
 *   rotAngleTable_deg  : Table of the rotation angle in radian
 */
void  FWC_sis8300_struck_iqfb_func_setMeaRotationTable(void *boardHandle, unsigned int pno, double *scaleTable, double *rotAngleTable_deg)
{
    unsigned int i;
    unsigned int cs;
    unsigned int sn;
    unsigned int data;
      
    for(i = 0; i < pno; i ++) {
        /* Make up the data */
        cs = (unsigned int)(*(scaleTable + i) * cos(RFLIB_degToRad(*(rotAngleTable_deg + i))) * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_ROT_COEF_FRACTION));
        sn = (unsigned int)(*(scaleTable + i) * sin(RFLIB_degToRad(*(rotAngleTable_deg + i))) * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_ROT_COEF_FRACTION)); 
        data = (cs << 16) + (sn & 0x0000FFFF);

        /* Write to the firmware via two registers */
        RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_BUF_WR_ADDR, CON_SIS8300_STRUCK_IQFB_MEA_ROT_TABLE_OFFSET + i);       /* address */
        RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_BUF_WR_DATA, data);                                                   /* data */        
    }
    
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_BUF_WR_ADDR, 0);      /* disable the writing */    
}

/**
 * Set the driving chain rotoation table. The higher 16 bits is for cos and the lower 16 bits for sin. They all
 *   have 15 bits of fraction
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   pno                : Number of the points
 *   scaleTable         : Table of the scales (should be [0, 1))
 *   rotAngleTable_deg  : Table of the rotation angle in radian
 */
void  FWC_sis8300_struck_iqfb_func_setDrvRotationTable(void *boardHandle, unsigned int pno, double *scaleTable, double *rotAngleTable_deg)
{
    unsigned int i;
    unsigned int cs;
    unsigned int sn;
    unsigned int data;
  
    for(i = 0; i < pno; i ++) {
        /* Make up the data */
        cs   = (unsigned int)(*(scaleTable + i) * cos(RFLIB_degToRad(*(rotAngleTable_deg + i))) * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_ROT_COEF_FRACTION));
        sn   = (unsigned int)(*(scaleTable + i) * sin(RFLIB_degToRad(*(rotAngleTable_deg + i))) * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_ROT_COEF_FRACTION)); 
        data = (cs << 16) + (sn & 0x0000FFFF);

        /* Write to the firmware via two registers */
        RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_BUF_WR_ADDR, CON_SIS8300_STRUCK_IQFB_DRV_ROT_TABLE_OFFSET + i);       /* address */
        RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_BUF_WR_DATA, data);                                                   /* data */        
    }
    
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_BUF_WR_ADDR, 0);      /* disable the writing */    
}

/** 
 * Set the non-IQ coefficient index offset value (will be valid only you press the apply button)
 */
void  FWC_sis8300_struck_iqfb_func_setNonIQCoefOffset(void *boardHandle, unsigned int offset)
{
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_COEF_ID_OFF, offset);
}

/**
 * set the imbalance correction matrix of the vector modulator. 
 * Please note that the element abs values should be smaller than 1
 */
void  FWC_sis8300_struck_iqfb_func_setImbalanceCorrMatrix(void *boardHandle, double a11, double a12, double a21, double a22)
{
    unsigned int hw1x, hw2x;            /* high 16bit words */
    unsigned int lw1x, lw2x;            /* low 16bit words */

    unsigned int a1x = 0;               /* combine of a11 and a12 for one register */
    unsigned int a2x = 0;               /* combine of a21 and a22 for one register */

    hw1x  = (unsigned int)(a11 * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_COR_COEF_FRACTION));
    lw1x  = (unsigned int)(a12 * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_COR_COEF_FRACTION));
    hw2x  = (unsigned int)(a21 * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_COR_COEF_FRACTION));
    lw2x  = (unsigned int)(a22 * pow(2, FWC_SIS8300_STRUCK_IQFB_CONST_COR_COEF_FRACTION));
    
    a1x   = (hw1x << 16) + (lw1x & 0x0000FFFF);
    a2x   = (hw2x << 16) + (lw2x & 0x0000FFFF);

    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_IMBALANCE_MATRIX_A1X, a1x);
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_IMBALANCE_MATRIX_A2X, a2x);
}

/*-------------------------------------------------------------
 * RF CONTROLLER FIRMWARE READINGS
 *-------------------------------------------------------------*/
/**
 * Get the board information. Including the firmware name (4 char string), major and minor version number and the build number
 */
void FWC_sis8300_struck_iqfb_func_getAppFwInfo(void *boardHandle, unsigned int *firmwareName, unsigned int *majorVer, unsigned int *minorVer, unsigned int *buildNum)
{
    unsigned int var_version;
    
    /* read the registers */    
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_FRIMWARE_NAME,     firmwareName);
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_FIRMWARE_VERSION,  &var_version);    
    
    /* build up the data */   
    *majorVer   = (var_version >> 24) & 0xFF;
    *minorVer   = (var_version >> 16) & 0xFF;
    *buildNum   = var_version & 0xFFFF;
}

/**
 * Get the pulse counter
 */
void FWC_sis8300_struck_iqfb_func_getPulseCounter(void *boardHandle, unsigned int *pulseCnt)
{
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_PUL_CNT, pulseCnt);
}

/**
 * Get the measured trigger period
 */
void  FWC_sis8300_struck_iqfb_func_getMeaTrigPeriod(void *boardHandle, double *value_ms, double freq_MHz)
{
    unsigned int data;
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_DIAG_TRIG_PERIOD, &data);
    *value_ms = (double)data / freq_MHz / 1000.0;
}

/**
 * Get the non-IQ coefficient index of the first point for the ADC DAQ data
 */
void  FWC_sis8300_struck_iqfb_func_getNonIQCoefCur(void *boardHandle, unsigned int *cur)
{
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_COEF_ID_TRIG, cur);
}

/**
 * Get the watch dog counter, this counter is a count of the ADC clock, so it can be used to diagnostic the ADC clock is OK or not
 */
void  FWC_sis8300_struck_iqfb_func_getWatchDogCnt(void *boardHandle, unsigned int *cnt)
{
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_WD_CNT, cnt);
}

/**
 * Get the status of the platform firmware and the RF control firmware
 */
void  FWC_sis8300_struck_iqfb_func_getFwStatus(void *boardHandle, unsigned int *platformStatus, unsigned int *RFCtrlStatus)
{
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_PLATFORM_STATUS, platformStatus);
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_RFCTRL_STATUS,   RFCtrlStatus);
}

/**
 * Get the missing trigger counter
 */
void  FWC_sis8300_struck_iqfb_func_getMissingTrigCnt(void *boardHandle, unsigned int *cnt)
{
    RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_REG_ADDR_TRIG_MISSING_CNT, cnt);
}

/**
 * Get all DAQ data from the FPGA
 * Input:
 *   boardHandle        : Address of the data structure of the board moudle
 *   buf                : Buffer to store the data, not the buffer should be large enough to store all data
 */
void  FWC_sis8300_struck_iqfb_func_getAllDAQData(void *boardHandle, unsigned int *buf)
{
    /*int i;
    int var_cnt = 0;

    unsigned int data;*/

    RFCB_API_readBuffer((RFCB_struc_moduleData *)boardHandle, CON_SIS8300_STRUCK_IQFB_BRAM_DMA_OFFSET, 
                         CON_SIS8300_STRUCK_IQFB_DAQ_BUF_DEPTH * CON_SIS8300_STRUCK_IQFB_DAQ_BUF_NUM * 2, buf);  

    /* Print the data */
    /*printf("-----------------------------------------------\n");
    for(i = 0; i < 128; i ++) {
        printf("0x%4X  ", *(buf + i));
        var_cnt ++;
        if(var_cnt >= 8) {
            var_cnt = 0;
            printf("\n");
        }
    } */

    /* display the counter of ADC clock watch dog */
    /*printf("-----------------------------------------------\n");    
    for(i = 0; i < 980; i ++)
    {
        FWC_sis8300_struck_iqfb_func_getWatchDogCnt(boardHandle, &data);
        printf("%d  ", data);
        var_cnt ++;
        if(var_cnt >= 16) {
            var_cnt = 0;
            printf("\n");
        }

        usleep(1000);
    } */   
  
}

/**
 * Get the ADC data from the DRAM
 * Note: now, all the ADC raw data will be read from the DRAM while the RF controller internal data will be read from the BRAM
 *       The DRAM data is still offset binary and the BRAM data is already 2's complement
 *       - Code copied from the test program provided by Struck
 *       - Address of the registers are defined in the RFCB module
 *
 *       The ADCxData is a temp buffer to convert the data format
 */
void  FWC_sis8300_struck_iqfb_func_getAllADCData(void *boardHandle, unsigned int pno,
                                                            short *ADC0Data, short *ADC1Data,            
                                                            short *ADC2Data, short *ADC3Data,
                                                            short *ADC4Data, short *ADC5Data,
                                                            short *ADC6Data, short *ADC7Data,
                                                            short *ADC8Data, short *ADC9Data)
{
    int i;
    unsigned int data;
    unsigned int pno_f;

    /* check the input */
    pno_f = (unsigned int)(pno / 16) * 16;
    if(pno_f < 16) pno_f = 16;                      /* minimum point number is 16 and the number must be a integer time of 16 (limited by the firmware) */

    if(!boardHandle || !ADC0Data || !ADC1Data || !ADC2Data || !ADC3Data || !ADC4Data || !ADC5Data || !ADC6Data || !ADC7Data || !ADC8Data || !ADC9Data) return;

    /* wait if BUSY or arm (risky) */
    /*do {
        RFCB_API_readRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_ACQUISITION_CONTROL_STATUS_REG, &data);       
    } while((data & 0x3) != 0); */ /* assume time is enough for finishing the sampling */

    /* read the buffers (address and pno are for 32 bit data) */
    RFCB_API_readBuffer((RFCB_struc_moduleData *)boardHandle, 0 * (0x100000 * 16 * 2 / 4), pno_f >> 1, (unsigned int *)ADC0Data);    /* Blocklength * 16 Samples/Block * 2Byte/Sample / 4 for 32 bits data */
    RFCB_API_readBuffer((RFCB_struc_moduleData *)boardHandle, 1 * (0x100000 * 16 * 2 / 4), pno_f >> 1, (unsigned int *)ADC1Data);    /* Blocklength * 16 Samples/Block * 2Byte/Sample / 4 for 32 bits data */
    RFCB_API_readBuffer((RFCB_struc_moduleData *)boardHandle, 2 * (0x100000 * 16 * 2 / 4), pno_f >> 1, (unsigned int *)ADC2Data);    /* Blocklength * 16 Samples/Block * 2Byte/Sample / 4 for 32 bits data */
    RFCB_API_readBuffer((RFCB_struc_moduleData *)boardHandle, 3 * (0x100000 * 16 * 2 / 4), pno_f >> 1, (unsigned int *)ADC3Data);    /* Blocklength * 16 Samples/Block * 2Byte/Sample / 4 for 32 bits data */
    RFCB_API_readBuffer((RFCB_struc_moduleData *)boardHandle, 4 * (0x100000 * 16 * 2 / 4), pno_f >> 1, (unsigned int *)ADC4Data);    /* Blocklength * 16 Samples/Block * 2Byte/Sample / 4 for 32 bits data */
    RFCB_API_readBuffer((RFCB_struc_moduleData *)boardHandle, 5 * (0x100000 * 16 * 2 / 4), pno_f >> 1, (unsigned int *)ADC5Data);    /* Blocklength * 16 Samples/Block * 2Byte/Sample / 4 for 32 bits data */
    RFCB_API_readBuffer((RFCB_struc_moduleData *)boardHandle, 6 * (0x100000 * 16 * 2 / 4), pno_f >> 1, (unsigned int *)ADC6Data);    /* Blocklength * 16 Samples/Block * 2Byte/Sample / 4 for 32 bits data */
    RFCB_API_readBuffer((RFCB_struc_moduleData *)boardHandle, 7 * (0x100000 * 16 * 2 / 4), pno_f >> 1, (unsigned int *)ADC7Data);    /* Blocklength * 16 Samples/Block * 2Byte/Sample / 4 for 32 bits data */
    RFCB_API_readBuffer((RFCB_struc_moduleData *)boardHandle, 8 * (0x100000 * 16 * 2 / 4), pno_f >> 1, (unsigned int *)ADC8Data);    /* Blocklength * 16 Samples/Block * 2Byte/Sample / 4 for 32 bits data */
    RFCB_API_readBuffer((RFCB_struc_moduleData *)boardHandle, 9 * (0x100000 * 16 * 2 / 4), pno_f >> 1, (unsigned int *)ADC9Data);    /* Blocklength * 16 Samples/Block * 2Byte/Sample / 4 for 32 bits data */

    /* convert the data to 2's complement from binary offset */
    for(i = 0; i < pno_f; i ++) {
        *(ADC0Data + i) ^= 0x8000;
        *(ADC1Data + i) ^= 0x8000;
        *(ADC2Data + i) ^= 0x8000;
        *(ADC3Data + i) ^= 0x8000;
        *(ADC4Data + i) ^= 0x8000;
        *(ADC5Data + i) ^= 0x8000;
        *(ADC6Data + i) ^= 0x8000;
        *(ADC7Data + i) ^= 0x8000;
        *(ADC8Data + i) ^= 0x8000;
        *(ADC9Data + i) ^= 0x8000;
    }

    /* set up the ADC sampling (later put to a commmon function) */    
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, DDR2_ACCESS_CONTROL, 0);                   /* disable ddr2 test write interface */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_PRETRIGGER_DELAY_REG, 0);          /* disable the delay */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SAMPLE_LENGTH_REG, pno_f >> 4);    /* each block has 16 point, here is the block number */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SAMPLE_CONTROL_REG, 0x1000);       /* enable all ADCs, use the RF pulse trigger */

    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_ACQUISITION_CONTROL_STATUS_REG, 0x00004);  /* reset the sampling logic */

    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SAMPLE_START_ADDRESS_CH1_REG,  0x000000);  /* 1. 1M-Block 16 Msamples */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SAMPLE_START_ADDRESS_CH2_REG,  0x100000);  /* 2. 1M-Block 16 Msamples */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SAMPLE_START_ADDRESS_CH3_REG,  0x200000);  /* 3. 1M-Block 16 Msamples */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SAMPLE_START_ADDRESS_CH4_REG,  0x300000);  /* 4. 1M-Block 16 Msamples */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SAMPLE_START_ADDRESS_CH5_REG,  0x400000);  /* 5. 1M-Block 16 Msamples */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SAMPLE_START_ADDRESS_CH6_REG,  0x500000);  /* 6. 1M-Block 16 Msamples */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SAMPLE_START_ADDRESS_CH7_REG,  0x600000);  /* 7. 1M-Block 16 Msamples */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SAMPLE_START_ADDRESS_CH8_REG,  0x700000);  /* 8. 1M-Block 16 Msamples */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SAMPLE_START_ADDRESS_CH9_REG,  0x800000);  /* 9. 1M-Block 16 Msamples */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_SAMPLE_START_ADDRESS_CH10_REG, 0x900000);  /* 10.1M-Block 16 Msamples */
    
    /* re-arm the sampling, waiting for next trigger */
    RFCB_API_writeRegister((RFCB_struc_moduleData *)boardHandle, SIS8300_ACQUISITION_CONTROL_STATUS_REG, 0x00002);  /* armed, wait for trigger */
}






