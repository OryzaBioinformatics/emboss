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
/*  Last edited: Mar  1 17:31 2000 (pmr) */

#include "emboss.h"

int main (int argc, char * argv[], char **env) {

  AjPList newlist = NULL;

  AjPList glist = ajListNew();	/* list of groups pointing to lists of programs */
  AjPList alpha = ajListNew();	/* alphabetical list of all programs */
  AjPList appgroups = NULL;

  AjPFile outfile = NULL;
  AjPStr search = NULL;
  AjPStr link1 = NULL;
  AjPStr link2 = NULL;
  AjBool html;
  AjBool groups;
  AjBool emboss;
  AjBool embassy;
  AjBool explode;

  (void) embInit ("seealso", argc, argv);

  search = ajAcdGetString ("search");
  outfile = ajAcdGetOutfile ("outfile");
  html = ajAcdGetBool("html");
  link1 = ajAcdGetString ("prelink");
  link2 = ajAcdGetString ("postlink");
  groups = ajAcdGetBool("groups");
  emboss = ajAcdGetBool("emboss");
  embassy = ajAcdGetBool("embassy");
  explode = ajAcdGetBool("explode");

/* is a search string specified */
  if (!ajStrLen(search)) {
    ajDie ("No application specified.");
  }


/* get the groups and program information */
  (void) embGrpGetProgGroups (glist, alpha, env, emboss, embassy, explode);

  newlist = ajListNew();
  (void) embGrpKeySearchSeeAlso(newlist, &appgroups, alpha, glist, search); 
  if (appgroups == NULL) {
    ajDie("Invalid application name specified.");
  }
  
  if (groups) {
    (void) embGrpOutputGroupsList(outfile, appgroups, ajFalse, html, link1, link2);
  } else {
    (void) embGrpOutputGroupsList(outfile, newlist, ajTrue, html, link1, link2);
  }
  (void) embGrpGroupsListDel(&newlist);
  (void) ajFileClose(&outfile);    
    
/* tidy up */
  (void) embGrpGroupsListDel(&glist);
  (void) embGrpGroupsListDel(&alpha);
/*  (void) embGrpGroupsListDel(&newlist); */
  (void) ajStrDel(&link1);
  (void) ajStrDel(&link2);

  (void) ajExit();
  return 0;
}

