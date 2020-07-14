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

typedef struct sgff
{
  AjPStr clone;
  AjPStr source;
  AjPStr type;
  ajint    start;
  ajint    end;
  ajint    score;
  /*not complete but okay for a demo purposes */
} gff,*gffptr;

static gffptr demolist_creategff(AjPStr line);
/*static ajint demolist_sourcecomp(const void *a, const void *b);*/
static ajint demolist_typecomp(const void *a, const void *b);
static ajint demolist_startcomp(const void *a, const void *b);
static void  demolist_dumpOut(void **x, void *cl);
static void  demolist_freegff (void **x, void *cl);



/* @prog demolist *************************************************************
**
** Testing
**
******************************************************************************/

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
    gfffile = ajAcdGetInfile("infile");

    /* create a new list */
    list = ajListNew();

    while( ajFileReadLine(gfffile, &line) )
    {
	/* create new gff */
	gffnew = demolist_creategff(line);

	/* add it to the list if okay */
	if(gffnew)
	    ajListPush(list,(void *)gffnew);
    }



    ajUser("\nOutput via the ajListIter method \nSorted by source");

    /* Print out the list using the iterator */
    iter = ajListIter(list);
    while(ajListIterMore(iter))
    {
	gffnew = (gffptr) ajListIterNext (iter) ;
	ajUser("%S\t%S\t%S\t%d\t%d\t%d",gffnew->clone,gffnew->source,
	       gffnew->type,gffnew->start,gffnew->end,gffnew->score);
    }
    /* delete the iterator */
    ajListIterFree(iter);



    ajListSort(list, demolist_startcomp);
    ajUser("\nOutput via the ajListMap function \nSorted by start pos");
    ajListMap(list,demolist_dumpOut,NULL);


    /* printout the list but use the array method */
    ajListSort(list, demolist_typecomp);
    ajUser("\nOutput via the array method \nSorted by type");
    ia = ajListToArray(list, &array);
    for (i = 0; i < ia; i++)
    {
	gffnew = (gffptr) array[i];
	ajUser("%S\t%S\t%S\t%d\t%d\t%d",gffnew->clone,gffnew->source,
	       gffnew->type,gffnew->start,gffnew->end,gffnew->score);
    }

    /* free the objects in the list */
    ajListMap(list,demolist_freegff,NULL);


    ajExit();
    return 0;
}






/* @funcstatic demolist_typecomp **********************************************
**
** Undocumented.
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint demolist_typecomp(const void *a, const void *b)
{
    gffptr *gfa = (gffptr *) a;
    gffptr *gfb = (gffptr *) b;

    return ajStrCmp(&(*gfa)->type,&(*gfb)->type);
}




/* @funcstatic demolist_startcomp *********************************************
**
** Undocumented.
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint demolist_startcomp(const void *a, const void *b)
{
    gffptr *gfa = (gffptr *) a;
    gffptr *gfb = (gffptr *) b;

    if((*gfa)->start > (*gfb)->start)
	return 1;
    else if ((*gfa)->start == (*gfb)->start)
	return 0;

    return -1;
}



/* @funcstatic demolist_dumpOut ***********************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void  demolist_dumpOut(void **x, void *cl)
{
    gffptr gffnew = (gffptr)*x;

    ajUser("%S\t%S\t%S\t%d\t%d\t%d",gffnew->clone,gffnew->source,gffnew->type,
	   gffnew->start,gffnew->end,gffnew->score);

    return;
}




/* @funcstatic demolist_freegff ***********************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void  demolist_freegff (void **x, void *cl)
{
    gffptr gffnew = (gffptr)*x;

    ajStrDel(&gffnew->clone);
    ajStrDel(&gffnew->type);
    ajStrDel(&gffnew->source);
    AJFREE(gffnew);

    return;
}



/* @funcstatic demolist_creategff *********************************************
**
** Not important to understand for demo but this function
** merely passes back a gff struct
**
** @param [?] line [AjPStr] Undocumented
** @return [gffptr] Undocumented
** @@
******************************************************************************/

static gffptr demolist_creategff(AjPStr line)
{
    static AjPRegexp gffexp=NULL;
    gffptr gffnew=NULL;
    AjPStr temp=NULL;

    if(!gffexp)
	gffexp =
     ajRegCompC("([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)");

    if(ajRegExec(gffexp,line))
    {
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
