/* @source ajurldb ************************************************************
**
** AJAX url database functions
**
** These functions control all aspects of AJAX url database access
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.10 $
** @modified Oct 2010 pmr first version
** @modified $Date: 2012/04/26 17:36:15 $ by $Author: mks $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/


#include "ajlib.h"

#include "ajurldb.h"
#include "ajurlread.h"
#include "ajresourceread.h"
#include "ajcall.h"

#include <limits.h>
#include <stdarg.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

#include <dirent.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif



static AjBool      urlAccessUrlonly(AjPUrlin urlin);




/* @funclist urlAccess ********************************************************
**
** Functions to access each database or url access method
**
******************************************************************************/

static AjOUrlAccess urlAccess[] =
{
  /*  Name     AccessFunction   FreeFunction
      Qlink    Description
      Alias    Entry    Query    All      Chunk   Padding */
    {
      "urlonly",	 &urlAccessUrlonly, NULL,
      "",      "retrieve only the URL for a remote webserver",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      NULL, NULL, NULL,
      NULL, NULL,
      AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE
    }
};




/* @func ajUrldbInit **********************************************************
**
** Initialise url database internals
**
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajUrldbInit(void)
{
    AjPTable table;
    ajuint i = 0;

    table = ajUrlaccessGetDb();

    while(urlAccess[i].Name)
    {
        ajCallTableRegister(table, urlAccess[i].Name,
                            (void*) &urlAccess[i]);
	i++;
    }

    return;
}




/* @funcstatic urlAccessUrlonly ***********************************************
**
** Completes and returns a remote URL.
**
** @param [u] urlin [AjPUrlin] URL input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool urlAccessUrlonly(AjPUrlin urlin)
{
    AjPQuery qry;
    AjIList iter = NULL;
    AjPQueryField field = NULL;

    qry = urlin->Input->Query;

    if(!ajListGetLength(qry->QueryFields))
        return ajFalse;

    if(!urlin->Resource)
    {
        urlin->Resource = qry->QryData;
        qry->QryData = NULL;
    }

    if(!urlin->Resource)
        return ajFalse;

    ajFilebuffDel(&urlin->Input->Filebuff);

    iter = ajListIterNewread(qry->QueryFields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);
        if(ajStrMatchC(field->Field, "id"))
        {
            ajStrAssignS(&urlin->Identifiers, field->Wildquery);
        }
        else
        {
            ajDebug("Query '%S' unexpected field '%S' for URLONLY access\n",
                    qry, field->Field);
        }
    }

    ajListIterDel(&iter);
    
    return ajTrue;
}




/* @func ajUrldbPrintAccess ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrldbPrintAccess(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Url access methods\n");
    ajFmtPrintF(outf, "# Name       Alias Entry Query   All Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "method {\n");

    for(i=0; urlAccess[i].Name; i++)
	if(full || !urlAccess[i].Alias)
	    ajFmtPrintF(outf, "  %-10s %5B %5B %5B %5B \"%s\"\n",
			urlAccess[i].Name,  urlAccess[i].Alias,
			urlAccess[i].Entry, urlAccess[i].Query,
			urlAccess[i].All,   urlAccess[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajUrldbExit **********************************************************
**
** Cleans up url database processing internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrldbExit(void)
{
    return;
}
