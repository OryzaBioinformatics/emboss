/*  Last edited: Dec 20 18:57 1999 (pmr) */
/*
**
** EMBOSS initialization routine
**
*/

#include "emboss.h"

/* @func  embInit ******************************************************
**
** Initialises everything. Reads an ACD (AJAX Command Definition) file
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
** Must be called in each EMBOSS program first.
**
** @param [r] pgm [char*] Application name, used as the name of the ACD file
** @param [r] argc [int] Number of arguments provided on the command line,
**        usually passsed as-is by the calling application.
** @param [r] argv [char* []] Actual arguments as an array of text.
** @return [AjStatus] Always returns ajStatusOK or aborts.
** @@
******************************************************************************/
AjStatus embInit (char *pgm, int argc, char *argv[]) {

  ajNamInit("emboss");
  return ajAcdInit (pgm, argc, argv);

}

/* @func  embInitP ******************************************************
**
** Initialises everything. Reads an ACD (AJAX Command Definition) file
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
** Must be called in each EMBOSS program first.
**
** @param [r] pgm [char*] Application name, used as the name of the ACD file
** @param [r] argc [int] Number of arguments provided on the command line,
**        usually passsed as-is by the calling application.
** @param [r] argv [char* []] Actual arguments as an array of text.
** @param [r] package [char*] Package name, used to find the ACD file
** @return [AjStatus] Always returns ajStatusOK or aborts.
** @@
******************************************************************************/
AjStatus embInitP (char *pgm, int argc, char *argv[], char *package) {

  ajNamInit(package);
  return ajAcdInit (pgm, argc, argv);

}
