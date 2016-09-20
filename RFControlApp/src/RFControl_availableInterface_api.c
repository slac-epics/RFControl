/****************************************************
 * RFControl_availableInterface_api.c
 * 
 * Source file for the available interface for RFControl module (API), which can be used by other modules / iocShells
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.05.16
 * Description: Initial creation
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2011.07.07
 * Description: Use virtual functions for the firmware access
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2/7/2013
 * Description: redesign the RFC_API_createModule function to pass the firmwareType to simplify the implementation
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2/13/2013
 * Description: Move the firmware specific code to the RFControlFirmware module
 ****************************************************/
#include <stdlib.h>             
#include <string.h>
#include <errlog.h>

#include "RFControl_availableInterface_api.h"

/*======================================
 * Global data for module management
 *======================================*/
/* A global list of the module instances */
static EPICSLIB_type_linkedList RFC_gvar_moduleInstanceList;
static int RFC_gvar_moduleInstanceListInitalized = 0;

/* A mutex is required for this list (to be done later) */

/*======================================
 * Common API Routines for module management
 *======================================*/
/**
 * Create an instance of the module
 * Input: 
 *     moduleName : An unique name of the module instance
 * Return:
 *     0          : Successful
 *    -1          : Failed
 */
int RFC_API_createModule(const char *moduleName)
{
    RFC_struc_moduleData                *ptr_dataInstance = NULL;

    /* Check the input parameters */
    if(!moduleName || !moduleName[0]) {
        EPICSLIB_func_errlogPrintf("RFC_API_createModule: Illegal module name\n");
        return -1;
    }

    /* Check if the list initialized */
    if(!RFC_gvar_moduleInstanceListInitalized) {
        EPICSLIB_func_LinkedListInit(RFC_gvar_moduleInstanceList);
        RFC_gvar_moduleInstanceListInitalized = 1;
    }

    /* Create a new instance of the data structure */
    ptr_dataInstance = (RFC_struc_moduleData *)calloc(1, sizeof(RFC_struc_moduleData));

    if(ptr_dataInstance == NULL) {
        EPICSLIB_func_errlogPrintf("RFC_API_createModule: Failed to create the data structure for the module of %s\n", moduleName);
        return -1;
    }

    /* Call the routine for the construction of the module */
    if(RFC_func_createModule(ptr_dataInstance, moduleName) != 0) {
        EPICSLIB_func_errlogPrintf("RFC_API_createModule: Failed to construct the module of %s\n", moduleName);
        free(ptr_dataInstance);                            /* clean the garbage */
        return -1;
    }

    /* init the system */
    if(RFC_func_initModule(ptr_dataInstance) != 0) {
        EPICSLIB_func_errlogPrintf("RFC_API_createModule: Failed to init the module of %s\n", moduleName);
        return -1;
    }

    /* create EPICS records */
    if(RFC_func_createEpicsData(ptr_dataInstance) != 0) {                                                                          
        EPICSLIB_func_errlogPrintf("RFC_API_createModule: There are errors to create records for the module of %s\n", moduleName);
        return -1;
    }

    /* Add the successful module instance into the list */
    EPICSLIB_func_LinkedListInsert(RFC_gvar_moduleInstanceList, ptr_dataInstance -> node);

    return 0;
}

/**
 * Delete the module instance
 * Input: 
 *     moduleName : Name of the module instance to be deleted
 * Return:
 *     0          : Successful
 *    -1          : Failed
 */
int RFC_API_deleteModule(const char *moduleName) 
{
    RFC_struc_moduleData *ptr_dataInstance = NULL;

    /* Check the input parameters */
    if(!moduleName || !moduleName[0]) {
        EPICSLIB_func_errlogPrintf("RFC_API_deleteModule: Illegal module name\n");
        return -1;
    }

    /* Find the module from the list */
    ptr_dataInstance = RFC_API_getModule(moduleName);

    if(ptr_dataInstance == NULL) {
        EPICSLIB_func_errlogPrintf("RFC_API_deleteModule: Failed to find the module of %s\n", moduleName);
        return -1;
    }

    /* Here we need to disable or delete the internal data associated to the module that to be deleted. If we do not do this,
     * the EPICS PVs will try to talk to an non-exists address which will cause the program crashes. This feature is useful when
     * the EPICS base supports dynamic record loading/deleting, but not supported now! Here I put the function as a place holder.
     */
    if(RFC_func_deleteEpicsData(ptr_dataInstance) != 0) {
        EPICSLIB_func_errlogPrintf("RFC_API_deleteModule: Failed to delete the internal data connected to the module of %s\n", moduleName);
        return -1;
    }

    /* Call the distruction routine of the module */
    if(RFC_func_destroyModule(ptr_dataInstance) != 0) {
        EPICSLIB_func_errlogPrintf("RFC_API_deleteModule: Failed to delete the module of %s\n", moduleName);
        return -1;
    }

    /* Delete it */
    EPICSLIB_func_LinkedListDelete(RFC_gvar_moduleInstanceList, ptr_dataInstance -> node);
    free(ptr_dataInstance);

    return 0;
}

/**
 * Set up the parameters of the module instance, the command will include:
 *   - RFCFW_NAME  : Set the RFControlFirmware module name that this module instance will be connected to
 *   - THRD_PRIO   : Set the thread priority
 *   - THRD_CRAT   : Create and start the thread
 * Input: 
 *     moduleName : Name of the module instance
 *     cmd        : Command listed above
 *     dataStr    : Data for the specified command
 * Return:
 *     0          : Successful
 *    -1          : Failed
 */
int RFC_API_setupModule(const char *moduleName, const char *cmd, const char *dataStr)
{
    int var_priority = 0;
    RFC_struc_moduleData                *ptr_dataInstance    = NULL;

    /* Check the input parameters */
    if(!moduleName || !moduleName[0]) {
        EPICSLIB_func_errlogPrintf("RFC_API_setupModule: Illegal module name\n");
        return -1;
    }

    /* Find the module instance */
    ptr_dataInstance = RFC_API_getModule(moduleName);

    if(ptr_dataInstance == NULL) {
        EPICSLIB_func_errlogPrintf("RFC_API_setupModule: Failed to find the module of %s\n", moduleName);
        return -1;
    }

    /* Response to each command */
    if(strcmp("RFCFW_NAME", cmd) == 0) {

        /* --- set RFControlBoardName --- */
        if(RFC_func_associateFirmwareModule(ptr_dataInstance, dataStr) != 0) {
            EPICSLIB_func_errlogPrintf("RFC_API_setupModule: Failed to set firmware module name\n");
            return -1;
        }

    } else if(strcmp("THRD_PRIO", cmd) == 0) {

        /* --- set thread priority --- */
        if(!dataStr || !dataStr[0]){
            EPICSLIB_func_errlogPrintf("RFC_API_setupModule: Failed to set thread priority\n");
            return -1;
        }
        
        sscanf(dataStr, "%d", &var_priority);
        
        if(RFC_func_setThreadPriority(ptr_dataInstance, (unsigned int)var_priority) != 0) {
            EPICSLIB_func_errlogPrintf("RFC_API_setupModule: Failed to set thread priority\n");
            return -1;
        }

    } else if(strcmp("THRD_CRAT", cmd) == 0) {
        
        /* --- create thread --- */
        if(RFC_func_createThread(ptr_dataInstance) != 0) {
            EPICSLIB_func_errlogPrintf("RFC_API_setupModule: Failed to create the thread\n");
            return -1;
        }

    } else {
        
        /* --- invalid command --- */
        EPICSLIB_func_errlogPrintf("RFC_API_setupModule: Invalid command\n");
        return -1;
        
    }

    return 0;
}

/**
 * Find the module from the list
 * Input: 
 *     moduleName  : Name of the module instance
 * Return:
 *     NULL        : If the module instance is not found
 *     module addr : Successful
 */
RFC_struc_moduleData *RFC_API_getModule(const char *moduleName)
{
    int var_moduleFound = 0;
    RFC_struc_moduleData *ptr_moduleData  = NULL;

    /* Check the input parameters */
    if(!moduleName || !moduleName[0]) {
        EPICSLIB_func_errlogPrintf("RFC_API_getModule: Illegal module name\n");
        return NULL;
    }    

    /* Look up the list */
    for(ptr_moduleData = (RFC_struc_moduleData *)EPICSLIB_func_LinkedListFindFirst(RFC_gvar_moduleInstanceList);
        ptr_moduleData;
        ptr_moduleData = (RFC_struc_moduleData *)EPICSLIB_func_LinkedListFindNext(ptr_moduleData -> node)) {
        if(strcmp(ptr_moduleData -> moduleName, moduleName) == 0) {
            var_moduleFound = 1;
            break;
        }
    }

    if(var_moduleFound) return ptr_moduleData;
    else return NULL;
}



