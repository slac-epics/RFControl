/****************************************************
 * RFControl_iocShell.c
 * 
 * Define ioc shell commands
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.05.16
 * Description: Initial creation
 ****************************************************/
#include <epicsTypes.h>
#include <epicsExport.h>
#include <iocsh.h>

#include "RFControl_availableInterface_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*======================================
 * IOC shell Routines
 *======================================*/
/* RFC_API_createModule(const char *moduleName) */
static const iocshArg        RFC_createModule_Arg0    = {"moduleName",   iocshArgString};
static const iocshArg *const RFC_createModule_Args[1] = {&RFC_createModule_Arg0};
static const iocshFuncDef    RFC_createModule_FuncDef = {"RFC_createModule", 1, RFC_createModule_Args};
static void  RFC_createModule_CallFunc(const iocshArgBuf *args) {RFC_API_createModule(args[0].sval);}

/* RFC_API_deleteModule(const char *moduleName) */
static const iocshArg        RFC_deleteModule_Arg0    = {"moduleName", iocshArgString};
static const iocshArg *const RFC_deleteModule_Args[1] = {&RFC_deleteModule_Arg0};
static const iocshFuncDef    RFC_deleteModule_FuncDef = {"RFC_deleteModule", 1, RFC_deleteModule_Args};
static void  RFC_deleteModule_CallFunc(const iocshArgBuf *args) {RFC_API_deleteModule(args[0].sval);}

/* RFC_API_setupModule(const char *moduleName, const char *cmd, const char *dataStr) */
static const iocshArg        RFC_setupModule_Arg0    = {"moduleName", iocshArgString};
static const iocshArg        RFC_setupModule_Arg1    = {"cmd",        iocshArgString};
static const iocshArg        RFC_setupModule_Arg2    = {"dataStr",    iocshArgString};
static const iocshArg *const RFC_setupModule_Args[3] = {&RFC_setupModule_Arg0, &RFC_setupModule_Arg1, &RFC_setupModule_Arg2};
static const iocshFuncDef    RFC_setupModule_FuncDef = {"RFC_setupModule", 3, RFC_setupModule_Args};
static void  RFC_setupModule_CallFunc(const iocshArgBuf *args) {RFC_API_setupModule(args[0].sval, args[1].sval, args[2].sval);}

void RFC_IOCShellRegister(void)
{
    iocshRegister(&RFC_createModule_FuncDef,  RFC_createModule_CallFunc);
    iocshRegister(&RFC_deleteModule_FuncDef,  RFC_deleteModule_CallFunc);
    iocshRegister(&RFC_setupModule_FuncDef,   RFC_setupModule_CallFunc);
}

epicsExportRegistrar(RFC_IOCShellRegister);

#ifdef __cplusplus
}
#endif

