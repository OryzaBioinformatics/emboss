/* @source wossname application
**
** Finds programs by keywords in their one-line documentation
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


/* @prog wossname *************************************************************
**
** Finds programs by keywords in their one-line documentation
**
******************************************************************************/

int main(int argc, char **argv, char **env)
{

    AjPList newlist = NULL;
    /* list of groups pointing to lists of programs */
    AjPList glist = ajListNew();
    /* alphabetical list of all programs */
    AjPList alpha = ajListNew();

    AjPFile outfile = NULL;
    AjPStr search = NULL;
    AjPStr link1 = NULL;
    AjPStr link2 = NULL;
    AjBool html;
    AjBool groups;
    AjBool alphabetic;
    AjBool emboss;
    AjBool embassy;
    AjBool explode;
    AjBool colon;
    AjBool gui;

    (void) embInit ("wossname", argc, argv);

    search = ajAcdGetString ("search");
    outfile = ajAcdGetOutfile ("outfile");
    html = ajAcdGetBool("html");
    link1 = ajAcdGetString ("prelink");
    link2 = ajAcdGetString ("postlink");
    groups = ajAcdGetBool("groups");
    alphabetic = ajAcdGetBool("alphabetic");
    emboss = ajAcdGetBool("emboss");
    embassy = ajAcdGetBool("embassy");
    explode = ajAcdGetBool("explode");
    colon   = ajAcdGetBool("colon");
    gui     = ajAcdGetBool("gui");
  
    /* get the groups and program information */
    (void) embGrpGetProgGroups (glist, alpha, env, emboss, embassy,
		explode, colon, gui);


    /* is a search string specified */
    if (ajStrLen(search))
    {
	newlist = ajListNew();
	(void) embGrpKeySearchProgs(newlist, alpha, search);
	(void) embGrpOutputGroupsList(outfile, newlist, !groups, html, link1,
				      link2);
	(void) embGrpGroupsListDel(&newlist);
    }
    else if (alphabetic)
    {      
	/* list all programs in alphabetic order */
	(void) embGrpOutputGroupsList(outfile, alpha, !groups, html, link1,
				      link2);
    }
    else
    {
	/* just show the grouped sets of programs */
	(void) embGrpOutputGroupsList(outfile, glist, !groups, html, link1,
				      link2);
      
    }
    (void) ajFileClose(&outfile);    

    /* tidy up */
    (void) embGrpGroupsListDel(&glist);
    (void) embGrpGroupsListDel(&alpha);
    (void) ajStrDel(&link1);
    (void) ajStrDel(&link2);

    (void) ajExit();
    return 0;
}

