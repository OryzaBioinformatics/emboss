/* @source demolist application
**
** Demomnstration of how the list functions should be used.
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

/* Read in a gff file and output sorting by user option before outputting */

#include "emboss.h"

typedef struct sgff{
  AjPStr clone;
  AjPStr source;
  AjPStr type;
  ajint    start;
  ajint    end;
  ajint    score;
  /*not complete but okay for a demo purposes */
} gff,*gffptr;

gffptr creategff(AjPStr line);


/* Comparison routines used for sorting */
ajint sourcecomp(const void *a, const void *b){
  gffptr *gfa = (gffptr *) a;  
  gffptr *gfb = (gffptr *) b;  

  return ajStrCmp(&(*gfa)->source,&(*gfb)->source);
}

ajint typecomp(const void *a, const void *b){
  gffptr *gfa = (gffptr *) a;  
  gffptr *gfb = (gffptr *) b;  

  return ajStrCmp(&(*gfa)->type,&(*gfb)->type);
}

ajint startcomp(const void *a, const void *b){
  gffptr *gfa = (gffptr *) a;  
  gffptr *gfb = (gffptr *) b;  

  if((*gfa)->start > (*gfb)->start)
    return 1;
  else if ((*gfa)->start == (*gfb)->start)
    return 0;
  else
    return -1;
}

/* END of Comparison routines */

static void  dumpOut(void **x, void *cl){
  gffptr gffnew = (gffptr)*x;

  ajUser("%S\t%S\t%S\t%d\t%d\t%d",gffnew->clone,gffnew->source,gffnew->type,
	 gffnew->start,gffnew->end,gffnew->score);
}
static void  freegff (void **x, void *cl){
  gffptr gffnew = (gffptr)*x;

  ajStrDel(&gffnew->clone);
  ajStrDel(&gffnew->type);
  ajStrDel(&gffnew->source);
  AJFREE(gffnew);
}


int main(int argc, char **argv)
{
  AjPList list=NULL;
  AjPFile gfffile;
  AjPStr  line=NULL;
  gffptr  gffnew;
  AjIList iter=NULL;
  void **array = NULL;
  ajint i,ia;

  embInit ("demolist", argc, argv);

  /*open file */
  gfffile = ajAcdGetInfile("gff");

  /* create a new list */
  list = ajListNew();

  while( ajFileReadLine(gfffile, &line) ) {
    /* create new gff */
    gffnew = creategff(line);
    
    /* add it to the list if okay */
    if(gffnew)
      ajListPush(list,(void *)gffnew);
  }



  ajUser("\nOutput via the ajListIter method \nSorted by source");

  /* Print out the list using the iterator */
  iter = ajListIter(list);
  while(ajListIterMore(iter)) {
    gffnew = (gffptr) ajListIterNext (iter) ;
    ajUser("%S\t%S\t%S\t%d\t%d\t%d",gffnew->clone,gffnew->source,gffnew->type,
	  gffnew->start,gffnew->end,gffnew->score);
  }
  /* delete the iterator */
  ajListIterFree(iter);



  ajListSort(list, startcomp);
  ajUser("\nOutput via the ajListMap function \nSorted by start pos");
  ajListMap(list,dumpOut,NULL);


  /* printout the list but use the array method */
  ajListSort(list, typecomp);
  ajUser("\nOutput via the array method \nSorted by type");
  ia = ajListToArray(list, &array);
  for (i = 0; i < ia; i++){  
    gffnew = (gffptr) array[i];
    ajUser("%S\t%S\t%S\t%d\t%d\t%d",gffnew->clone,gffnew->source,gffnew->type,
	  gffnew->start,gffnew->end,gffnew->score);
  }    

  /* free the objects in the list */
  ajListMap(list,freegff,NULL);


  ajExit();
  return 0;
}
























/* Not important to understand for demo but this function
   merely passes back a gff struct */
gffptr creategff(AjPStr line){
  static AjPRegexp gffexp=NULL;
  gffptr gffnew=NULL;
  AjPStr temp=NULL;
  
  if(!gffexp)
    gffexp = ajRegCompC("([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)");
  
  if(ajRegExec(gffexp,line)){
    AJNEW(gffnew); 
    gffnew->clone=gffnew->source=gffnew->type=NULL;		     
    ajRegSubI(gffexp,1,&gffnew->clone);
    ajRegSubI(gffexp,2,&gffnew->source);
    ajRegSubI(gffexp,3,&gffnew->type);
    ajRegSubI(gffexp,4,&temp);
    ajStrToInt(temp,&gffnew->start);
    ajRegSubI(gffexp,5,&temp);
    ajStrToInt(temp,&gffnew->end);
    ajRegSubI(gffexp,6,&temp);
    ajStrToInt(temp,&gffnew->score);
    ajStrDel(&temp);
    /*    ajUser("CHECK %S\t%S\t%S\t%d\t%d\t%d\n",
	   gffnew->clone,gffnew->source,gffnew->type, 
	   gffnew->start,gffnew->end,gffnew->score);*/
  }
  return gffnew;
}
