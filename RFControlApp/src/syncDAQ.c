/****************************************************
 * syncDAQ.c
 * 
 * Source file for the synchronized data aquisition. 
 *   This is a simplified version of BSA
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanforde.edu
 * Created on: 2011.07.13
 * Description: Initial creation
 ****************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "syncDAQ.h"

/*======================================
 * Global data for module management
 *======================================*/
/* A global list of the data */
static EPICSLIB_type_linkedList SDAQ_gvar_dataNodeList;
static EPICSLIB_type_linkedList SDAQ_gvar_wfNodeList;

static int SDAQ_gvar_dataNodeListInitalized  = 0;
static int SDAQ_gvar_wfNodeListInitalized    = 0;

/*======================================
 * Public Routines
 *======================================*/
/**
 * Create a data node
 */
int SDAQ_func_createDataNode(double *dataPtr)
{
    SDAQ_struc_dataNode *ptr_data = NULL;

    /* check the input */
    if(!dataPtr) {
        EPICSLIB_func_errlogPrintf("SDAQ_func_createDataNode: Illegal data address\n");
        return -1;
    }

    /* init the list */
    if(!SDAQ_gvar_dataNodeListInitalized) {
        EPICSLIB_func_LinkedListInit(SDAQ_gvar_dataNodeList);
        SDAQ_gvar_dataNodeListInitalized = 1;
    }    

    /* create a data node */
    ptr_data = (SDAQ_struc_dataNode *)calloc(1, sizeof(SDAQ_struc_dataNode));

    if(ptr_data == NULL) {
        EPICSLIB_func_errlogPrintf("SDAQ_func_createDataNode: Failed to create the data node\n");
        return -1;
    }    

    /* remember the data source */
    ptr_data -> dataPtr = dataPtr;
    memset((void *)ptr_data -> buf, 0, sizeof(double) * SDAQ_CONST_BUF_SIZE);

    /* add to the list */
    EPICSLIB_func_LinkedListInsert(SDAQ_gvar_dataNodeList, ptr_data -> node);

    return 0;
}

/**
 * Create a waveform node
 */
int SDAQ_func_createWfNode(short *dataPtr, int pno)
{
    SDAQ_struc_wfNode *ptr_wf = NULL;

    /* check the input */
    if(!dataPtr || pno <= 0) {
        EPICSLIB_func_errlogPrintf("SDAQ_func_createWfNode: Illegual waveform buffer or point number\n");
        return -1;
    }

    /* init the list */
    if(!SDAQ_gvar_wfNodeListInitalized) {
        EPICSLIB_func_LinkedListInit(SDAQ_gvar_wfNodeList);
        SDAQ_gvar_wfNodeListInitalized = 1;
    }    

    /* create a data node */
    ptr_wf = (SDAQ_struc_wfNode *)calloc(1, sizeof(SDAQ_struc_wfNode));

    if(ptr_wf == NULL) {
        EPICSLIB_func_errlogPrintf("SDAQ_func_createWfNode: Failed to create the waveform node\n");
        return -1;
    }    

    /* remeber the data source */
    ptr_wf -> dataPtr = dataPtr;
    ptr_wf -> pno     = MATHLIB_min(pno, SDAQ_CONST_WF_PNO_SUPPORTED);
    memset((void *)ptr_wf -> buf, 0, sizeof(short) * SDAQ_CONST_WF_PNO_SUPPORTED * SDAQ_CONST_WF_NUM_SUPPORTED);    

    /* add to the list */
    EPICSLIB_func_LinkedListInsert(SDAQ_gvar_wfNodeList, ptr_wf -> node);

    return 0;
}

/**
 * Save data
 */
int SDAQ_func_saveData(int dataId, char *nameStr)
{
    FILE *outFile                   = NULL;
    SDAQ_struc_dataNode *ptr_data   = NULL;

    /* check the input (effective dataId should be in the range of [0, SDAQ_CONST_BUF_SIZE - 1] */
    if(dataId < 0 || dataId >= SDAQ_CONST_BUF_SIZE || !nameStr || !nameStr[0]) return -1;

    /* save all data in the list */
    for(ptr_data = (SDAQ_struc_dataNode *)EPICSLIB_func_LinkedListFindFirst(SDAQ_gvar_dataNodeList);
        ptr_data;
        ptr_data = (SDAQ_struc_dataNode *)EPICSLIB_func_LinkedListFindNext(ptr_data -> node)) {  
                ptr_data -> buf[dataId] = *(ptr_data -> dataPtr);                               /* every node in the list will have valid data address */ 
    }

    /* save to file */
    if(dataId == SDAQ_CONST_BUF_SIZE - 1) {

        outFile = fopen(nameStr, "w");

        if(outFile) {

            for(ptr_data = (SDAQ_struc_dataNode *)EPICSLIB_func_LinkedListFindFirst(SDAQ_gvar_dataNodeList);
                ptr_data;
                ptr_data = (SDAQ_struc_dataNode *)EPICSLIB_func_LinkedListFindNext(ptr_data -> node)) {        
                    fwrite(ptr_data -> buf, sizeof(double), SDAQ_CONST_BUF_SIZE, outFile);
            }       

            fflush(outFile);
            fclose(outFile);

        }
    }
}

/**
 * Save waveform 
 */
int SDAQ_func_saveWf(int wfId, char *nameStr)
{
    FILE *outFile               = NULL;
    SDAQ_struc_wfNode *ptr_wf   = NULL;

    /* check the input (effective wfId should be in the range of [0, SDAQ_CONST_WF_NUM_SUPPORTED - 1] */
    if(wfId < 0 || wfId >= SDAQ_CONST_WF_NUM_SUPPORTED || !nameStr || !nameStr[0]) return -1;

    /* save all waveform in the list */
    for(ptr_wf = (SDAQ_struc_wfNode *)EPICSLIB_func_LinkedListFindFirst(SDAQ_gvar_wfNodeList);
        ptr_wf;
        ptr_wf = (SDAQ_struc_wfNode *)EPICSLIB_func_LinkedListFindNext(ptr_wf -> node)) {        
            memcpy((void *)(ptr_wf -> buf + wfId * ptr_wf -> pno), (void *)(ptr_wf -> dataPtr), sizeof(short) * ptr_wf -> pno);     
    }

    /* save to file */
    if(wfId == SDAQ_CONST_WF_NUM_SUPPORTED - 1) {

        outFile = fopen(nameStr, "w");

        if(outFile) {
            for(ptr_wf = (SDAQ_struc_wfNode *)EPICSLIB_func_LinkedListFindFirst(SDAQ_gvar_wfNodeList);
                ptr_wf;
                ptr_wf = (SDAQ_struc_wfNode *)EPICSLIB_func_LinkedListFindNext(ptr_wf -> node)) {        
                    fwrite(ptr_wf -> buf, sizeof(short), SDAQ_CONST_WF_PNO_SUPPORTED * SDAQ_CONST_WF_NUM_SUPPORTED, outFile);
            }       

            fflush(outFile);
            fclose(outFile);
        }
    }
}










