/* @source seealso application
**
** Finds programs sharing group names
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** @@
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




/* @prog seealso **************************************************************
**
** Finds programs sharing group names
**
******************************************************************************/

int main(int argc, char **argv, char **env)
{

    AjPList newlist = NULL;
    AjPList glist;
    AjPList alpha;
    AjPList appglist;
    AjPList applist;
    AjPList appgroups = NULL;

    AjPFile outfile = NULL;
    AjPStr search   = NULL;
    AjPStr link1    = NULL;
    AjPStr link2    = NULL;
    AjBool html;
    AjBool groups;
    AjBool emboss;
    AjBool embassy;
    AjBool explode;
    AjBool colon;
    AjPStr showembassy = NULL;

    embInit("seealso", argc, argv);
    
    search  = ajAcdGetString("search");
    outfile = ajAcdGetOutfile("outfile");
    html    = ajAcdGetToggle("html");
    link1   = ajAcdGetString("prelink");
    link2   = ajAcdGetString("postlink");
    groups  = ajAcdGetBool("groups");
    emboss  = ajAcdGetBool("emboss");
    embassy = ajAcdGetBool("embassy");
    showembassy = ajAcdGetString("showembassy");
    explode = ajAcdGetBool("explode");
    colon   = ajAcdGetBool("colon");
    
    
    glist = ajListNew();
    alpha = ajListNew();
    appglist = ajListNew();
    applist = ajListNew();
    
    
    
    /* is a search string specified  - should be tested in seealso.acd */
    
    if(!ajStrLen(search))
	ajFatal("No application specified.");
    
    
    /*
    ** get the groups and program information - don't want to ignore
    ** applications that don't work well under GUIs
    */
    embGrpGetProgGroups(appglist, applist, env, ajTrue, ajTrue, NULL,
			explode, colon, ajFalse);
    
    embGrpGetProgGroups(glist, alpha, env, emboss, embassy, showembassy,
			explode, colon, ajFalse);
    
    newlist = ajListNew();
    embGrpKeySearchSeeAlso(newlist, &appgroups, applist, glist, search);
    if(appgroups == NULL)
    {
	ajErr("No applications match.");
	ajExitBad();
    }
    
    if(groups)
	embGrpOutputGroupsList(outfile, appgroups, ajFalse, html,
			       link1, link2);
    else
	embGrpOutputGroupsList(outfile, newlist, ajTrue, html, link1,
			       link2);
    
    embGrpGroupsListDel(&newlist);
    ajFileClose(&outfile);
    
    embGrpGroupsListDel(&glist);
    embGrpGroupsListDel(&alpha);
    embGrpGroupsListDel(&appglist);
    embGrpGroupsListDel(&applist);
    
    ajStrDel(&link1);
    ajStrDel(&link2);
    
    ajExit();
    
    return 0;
}
