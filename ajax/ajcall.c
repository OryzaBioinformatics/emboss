#include "ajcall.h"
#include "ajgraph.h"
#include "ajstr.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* ajCall Routines are used to allow access to different graphics packages.
** Okay so at the moment only plplot is used. Also if you want a light weight
** version of EMBOSS no graphics can be stated.
** So in ajgraph.c you will see the register calls which list all the calls
** needed by ajacd.c.
** At the start of ajgraph.c all the calls that call plplot are done first.
** These are the ones that will need to be replaced if the graphics
** are changed.
*/

static ajint callCmpStr(const void *x, const void *y);
static unsigned callStrHash(const void *key, unsigned hashsize);

/* @funcstatic callCmpStr *****************************************************
**
** Compare two words.
**
** @param [r] x [const void *] First word
** @param [r] y [const void *] Second word
** @return [ajint] difference
** @@
******************************************************************************/

static ajint callCmpStr(const void *x, const void *y) {
  return strcmp((char *)x, (char *)y);
}

/* @funcstatic callStrHash ****************************************************
**
** Create hash value from key.
**
** @param [r] key [const void *] key.
** @param [r] hashsize [unsigned] Hash size
** @return [unsigned] hash value
** @@
******************************************************************************/

static unsigned callStrHash(const void *key, unsigned hashsize)
{
  unsigned hashval;
  char *s = (char *) key;
  ajint j = strlen(s);

  ajint i;
  for(i=0, hashval = 0; i < j; i++, s++)
    hashval = *s + 31 *hashval;
  return hashval % hashsize;
}

static AjPTable calls=NULL;

/* @func callRegister *********************************************************
**
** Create hash value pair using the name and function.
**
** @param [r] name [char *] name which is ysed later..
** @param [r] func [CallFunc] function to be called on name being called.
** @return [void]
** @@
******************************************************************************/

void callRegister(char *name, CallFunc func)
{
  void *rec;

  if(!calls)
    calls = ajTableNew(0, callCmpStr,callStrHash);

  rec = ajTableGet(calls, name);    /* does it exist already */

  if(!rec){
    (void) ajTablePut(calls, name, (void *) func);
  }
}

/* @func call *****************************************************************
**
** Call a function by its name. If it does not exist then give
** an error message saying so.
**
** @param [r] name [char *] name of the function to call.
** @param [v] [...] Optional arguments
** @return [void*] NULL if function call not found.
** @@
******************************************************************************/
void* call(char *name, ...)
{
  va_list args;
  CallFunc rec;
  void *retval = NULL;

  if(!calls){
    ajMessCrash("Graphics calls not Registered. "
		"Use ajGraphInit in main function first",name);
    return retval;
  }

  rec = (CallFunc) ajTableGet(calls, name);

  if(rec) {
    va_start(args, name);
    retval = (*(rec))(name, args);
    va_end(args);
  }
  else
    ajMessCrash("Graphics call %s not found. "
		"Use ajGraphInit in main function first",name);

  return retval;
}

