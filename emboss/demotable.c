/* @source demotable application
**
** Demomnstration of how the table functions should be used.
** @author: Copyright (C) Peter Rice (pmr@sanger.ac.uk)
** @@
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

static AjPStr getsubfromstring(AjPStr line,int which){
  static AjPRegexp gffexp=NULL;
  AjPStr temp =NULL;
 
  if(!gffexp)
    gffexp = ajRegCompC("([^\t]+)\t([^\t]+)\t([^\t]+)");

  if(ajRegExec(gffexp,line))
    ajRegSubI(gffexp,which,&temp);
   
  return temp;
}

static void typePrint (const void* key, void** value, void* cl){
  AjPStr keystr = (AjPStr) key;
  int    *valptr = (int *) *value;

  ajUser("type '%S' found %d times", keystr, *valptr);
}

static void freetype (const void* key, void** value, void* cl){
  AjPStr keystr = (AjPStr) key;
  int    *valptr = (int *) *value;

  ajStrDel(&keystr);
  AJFREE(valptr);
}
 
int main (int argc, char **argv) {
  AjPStr temp;
  AjPFile gfffile;
  AjPStr  line=NULL;
  AjPTable type;
  int *intptr;
 
  embInit ("demotable", argc, argv);

  
  /*open file */
  gfffile = ajAcdGetInfile("gff");

  /* create new table using ajStrTableCmpCase as the comparison function
     and ajStrTableHashCase as the hash function. Initial size of 50 is used */
  type   = ajTableNew(50, ajStrTableCmpCase, ajStrTableHashCase);

  while( ajFileReadLine(gfffile, &line) ) {

    temp = getsubfromstring(line,3); /* get the string to test */

    if(temp){

      /* does the key "temp" already exist in the table */
      intptr = ajTableGet(type, temp);


      if(!intptr){                      /* if not i.e. no key returned */
	AJNEW(intptr);
	*intptr = 1;
	ajTablePut(type, temp, intptr); /* add it*/
      }
      else {
	ajStrDel(&temp);
	(*intptr)++;                    /* else increment the counter */
      }
    }
  }
  ajUser("%d types found",ajTableLength(type));
  ajTableMap (type, typePrint, NULL);  /* use the map function to print out the results */
  ajTableMap (type, freetype, NULL);  /* use the map function to free all memory */
  ajTableFree (&type);

  ajExit();
  return 0;
}
