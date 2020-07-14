/* @source init.c
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "emboss.h"




/* @func  embInit *************************************************************
**
** Initialises everything. Reads an ACD (AJAX Command Definition) file
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
** Must be called in each EMBOSS program first.
**
** @param [r] pgm [char*] Application name, used as the name of the ACD file
** @param [r] argc [ajint] Number of arguments provided on the command line,
**        usually passsed as-is by the calling application.
** @param [r] argv [char* []] Actual arguments as an array of text.
** @return [AjStatus] Always returns ajStatusOK or aborts.
** @@
******************************************************************************/

AjStatus embInit (char *pgm, ajint argc, char *argv[])
{
    ajNamInit("emboss");

    return ajAcdInit (pgm, argc, argv);
}




/* @func  embInitP ************************************************************
**
** Initialises everything. Reads an ACD (AJAX Command Definition) file
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
** Must be called in each EMBOSS program first.
**
** @param [r] pgm [char*] Application name, used as the name of the ACD file
** @param [r] argc [ajint] Number of arguments provided on the command line,
**        usually passsed as-is by the calling application.
** @param [r] argv [char* []] Actual arguments as an array of text.
** @param [r] package [char*] Package name, used to find the ACD file
** @return [AjStatus] Always returns ajStatusOK or aborts.
** @@
******************************************************************************/

AjStatus embInitP (char *pgm, ajint argc, char *argv[], char *package)
{
  ajNamInit(package);

  return ajAcdInit (pgm, argc, argv);
}
