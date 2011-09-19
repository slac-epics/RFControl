/******************************************************
 * addrMap_sis8300_desy_iqfb_platform.h
 * 
 * Automatically generated from pakg_sis8300_config.vhd
 * Do not modify ... 
 * Mon Jun 06 11:42:18 2011
 ******************************************************/
#ifndef ADDRMAP_SIS8300_DESY_IQFB_PLATFORM_H
#define ADDRMAP_SIS8300_DESY_IQFB_PLATFORM_H

/*-------------------------------- */
/* constant definitions */
/*-------------------------------- */
/* version number */

/*-------------------------------- */
/* address for the registers  */
/*    1. these are SIS8300 board specific registers, the application specific registers will be defined */
/*       in the application modules */
/*    2. address partition: 0 - 31 board settings and status; 32 - 127 platform settings and status; 128 - application registers */
/*-------------------------------- */
/* Board Setting & Status */
#define CON_REG_ADDR_FRIMWARE_NAME          ( 0)
#define CON_REG_ADDR_FIRMWARE_VERSION       ( 1)
#define CON_REG_ADDR_MODULE_SERIAL_NO       ( 2)
#define CON_REG_ADDR_WATCHDOG_COUNTER       ( 3)
#define CON_REG_ADDR_MODULE_ALARM           ( 4)
#define CON_REG_ADDR_MODULE_STATUS          ( 5)
#define CON_REG_ADDR_MODULE_SETTING         ( 6)

/* Platform registers */
#define CON_REG_ADDR_HARLINK_OUT_SEL        ( 32)
#define CON_REG_ADDR_HARLINK_OUT            ( 33)
#define CON_REG_ADDR_HARLINK_IN             ( 34)

#define CON_REG_ADDR_AMC_LVDS_OUT_SEL       ( 35)
#define CON_REG_ADDR_AMC_LVDS_OUT           ( 36)
#define CON_REG_ADDR_AMC_LVDS_IN            ( 37)

#define CON_REG_ADDR_JTAG_DATA              ( 38)

#define CON_REG_ADDR_ADC_CLK_SEL            ( 39)
#define CON_REG_ADDR_AD9510_SPI_DATA        ( 40)
#define CON_REG_ADDR_AD9510_STATUS          ( 41)

#define CON_REG_ADDR_ADC_SPI_DATA           ( 42)
#define CON_REG_ADDR_ADC_STATUS             ( 43)

#define CON_REG_ADDR_SET_TRIG               ( 44)

#define CON_REG_ADDR_DAC_OUT_SEL            ( 45)
#define CON_REG_ADDR_DAC_SPI_DATA           ( 46)

#define CON_REG_ADDR_DMA_ADDR_START         ( 47)


#endif
