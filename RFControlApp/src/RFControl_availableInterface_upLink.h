/****************************************************
 * RFControl_availableInterface_upLink.h
 * 
 * Header file for the available interface for RFControl module
 * The uplink interfaces are realized here
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.05.16
 * Description: Initial creation
 ****************************************************/
#ifndef RF_CONTROL_AVAILABLE_INTERFACE_UPLINK_H
#define RF_CONTROL_AVAILABLE_INTERFACE_UPLINK_H

#include "RFControl_main.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Routines to associate the module data with the EPICS record
 */
int RFC_func_createEpicsData(RFC_struc_moduleData *arg);           /* create internal data node for the data in the module */
int RFC_func_deleteEpicsData(RFC_struc_moduleData *arg);           /* delete the internal data */

#ifdef __cplusplus
}
#endif

#endif

