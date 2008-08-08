/*
 * tclsyslog.c --
 *
 *    Tcl interface to the syslog API.
 *
 *  Copyright (c) 2008 HiLabs
 *
 *  See the file "LICENCE" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include <stdlib.h>
#include <syslog.h>
#include <string.h>

#include "tcl.h"

static Tcl_Command ts_gSyslogCommandToken = (Tcl_Command)NULL;

#define TS_IDLEN 512
static char ts_gSyslogId[TS_IDLEN]; // pointer to log identifier used in open

extern
int ts_Syslog (ClientData clientData, Tcl_Interp *interp, int objc, struct Tcl_Obj * CONST * objv)
{
  int       res   = TCL_OK;
  int       index = -1;

  static CONST char* cmds[] = {
    "id", "log", "level", "maxLevel", NULL
  };
  
  static CONST char* pri[] = {
    "EMERG", "ALERT", "CRIT", "ERR", "WARN", "NOTICE", "INFO", "DEBUG", NULL
  };
  static CONST int priVal[] = {
    LOG_EMERG, LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG
  };

  if (objc < 2) {
    Tcl_WrongNumArgs (interp, 1, objv, "subcommand ?params? \n\tvalid subcommands: id, log, level, maxLevel");
    return TCL_ERROR;
  }

  res = Tcl_GetIndexFromObj (interp, objv[1], cmds, "<subcommand>", 0, &index);
  if (res != TCL_OK) return TCL_ERROR;
  
  char* str = (char*)NULL;
  switch (index) {
    case 0: /* id */
      if (objc != 3) {
        Tcl_WrongNumArgs (interp, 1, objv, "id <idString>");
        return TCL_ERROR;
      }
      // get openlog arguments (identificationString)
      str = Tcl_GetString(objv[2]);
      strncpy(ts_gSyslogId, str, TS_IDLEN-1);
      openlog(ts_gSyslogId, LOG_NDELAY | LOG_PID, LOG_USER); // always log process ID, connect to syslogd immediatelly and use user-level facility
      break;
      
    case 1: /* log */
      if (objc != 4) {
        Tcl_WrongNumArgs (interp, 1, objv, "log <priorityLevel> <message>");
        return TCL_ERROR;
      }
      // get syslog arguments (priority, message)
      res = Tcl_GetIndexFromObj (interp, objv[2], pri, "<priority>", 0, &index);
        if (res != TCL_OK) return TCL_ERROR;
      str = Tcl_GetString(objv[3]);
      syslog(priVal[index], "%s", str);
      break;
      
    case 2: /* level */
      if (objc != 3) {
        Tcl_WrongNumArgs (interp, 1, objv, "level <levelToLog>");
        return TCL_ERROR;
      }
      // get arguments (logLevel)
      res = Tcl_GetIndexFromObj (interp, objv[2], pri, "<priority>", 0, &index);
        if (res != TCL_OK) return TCL_ERROR;
      setlogmask(LOG_MASK(priVal[index]));
      break;
    case 3: /* maxLevel */
      if (objc != 3) {
        Tcl_WrongNumArgs (interp, 1, objv, "maxLevel <upToLevel>");
        return TCL_ERROR;
      }
      // get arguments (maxLogLevel)
      res = Tcl_GetIndexFromObj (interp, objv[2], pri, "<priority>", 0, &index);
        if (res != TCL_OK) return TCL_ERROR;
      setlogmask(LOG_UPTO(priVal[index]));
      break;
    default:
      return TCL_ERROR;
  }
  return TCL_OK;
}

extern
int Tclsyslog_Init (Tcl_Interp* interp)
{
  int res = TCL_OK;
  
  #ifdef USE_TCL_STUBS
    char* resVersion = NULL;
    resVersion = (char*) Tcl_InitStubs (interp, "8.4", 0);
    if (resVersion == (char*)NULL) return TCL_ERROR;
  #endif
  
  ts_gSyslogCommandToken = 
      Tcl_CreateObjCommand (interp, "syslog", 
                            ts_Syslog, (ClientData)NULL,
                            (Tcl_CmdDeleteProc*)NULL);
  // set the default prepended log id
  strncpy(ts_gSyslogId, "tcl", TS_IDLEN-1);
  openlog(ts_gSyslogId, LOG_PID, LOG_USER);

  res = Tcl_PkgProvide (interp, PACKAGE_NAME, PACKAGE_VERSION);
  return res;
}


int Tclsyslog_SafeInit (Tcl_Interp* interp)
{
  return Tclsyslog_Init(interp);
}


extern
void Tclsyslog_Exit (Tcl_Interp* interp)
{
  closelog();
  ckfree(ts_gSyslogId);
  Tcl_DeleteCommandFromToken (interp, ts_gSyslogCommandToken);
}
