/* @source plplottest.c
**
** General test routine for graph plotting PLPLOT.
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
#ifndef NO_PLOT
#include "ajax.h"
#include "ajgraph.h"

int main(int argc, char **argv)
{
  ajint i;
  ajint numcols = 32; /* only 16 actually just testing!!! */
  char buffer[20];
  AjPGraph graph;

  ajGraphInit ("plplottest", argc, argv);

  graph = ajAcdGetGraph ("graph");

  ajGraphOpenWin(graph, 0.0,40.0,0.0,40.0);
  for(i=0;i<numcols; i++){
    ajGraphSetLineStyle(i);
    ajGraphSetFore(i);

    ajGraphLine((PLFLT)i,0.0,(PLFLT)i,20.0);
    ajGraphBoxFill((PLFLT)i,20.0,1.0);
    ajGraphDiaFill((PLFLT)i,21.0,1.0);
    ajGraphTriFill((PLFLT)i,23.0,i+1,25.0,i+2,23.0);
    ajGraphSetFore(1);
    ajGraphTri((PLFLT)i,23.0,i+1,25.0,i+2,23.0);

    ajGraphCircle(i,30.0,1);

    ajGraphSetFore(1);
    ajGraphSetCharSize(1.0);
    ajGraphTextStart (i,20.5,"a");

    ajGraphSetCharSize(1.0/((PLFLT)i+1.0));
    sprintf(buffer,"%f",(PLFLT)(1.0/((PLFLT)i+1.0)));
    ajGraphTextStart (i,25.5+i,buffer);
  }
  

  ajGraphCloseWin();
  ajExit();
  return 0;
}



#else
int main(int argc, char **argv)
{
  ajFatal("Sorry no PLplot was found on compilation hence NO graph");
}
#endif
