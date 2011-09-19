/****************************************************
 * RFControl_availableInterface_api.h
 * 
 * Header file for the available interface for RFControl module (API), which can be used by other modules / iocShells
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.05.16
 * Description: Initial creation
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2011.07.07
 * Description: Use virtual functions for the firmware access
 ****************************************************/
#ifndef RF_CONTROL_AVAILABLE_INTERFACE_API_H
#define RF_CONTROL_AVAILABLE_INTERFACE_API_H

#include "RFControl_main.h"
#include "RFControl_availableInterface_upLink.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * APIs that will be used by other moduels/iocShells
 */
/* Management of the modules (NOT REAL-TIME) */
int RFC_API_createModule(const char *moduleName);            
int RFC_API_deleteModule(const char *moduleName);
int RFC_API_setupModule(const char *moduleName, const char *cmd, const char *dataStr);

RFC_struc_moduleData *RFC_API_getModule(const char *moduleName);

#ifdef __cplusplus
}
#endif

#endif

