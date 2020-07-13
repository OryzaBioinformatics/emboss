/* @source lindna application
**
** Draws linear maps of DNA constructs
** @author: Copyright (C) Nicolas Tourasse (tourasse@biotek.uio.no),
** Biotechnology Centre of Oslo, Norway.
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
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXGROUPS 20
#define MAXLABELS 10000

static void lindna_ReadInput(AjPFile infile, float *Start, float *End);

static AjPStr lindna_ReadGroup(AjPFile infile, float *From, float *To,
			       AjPStr *Name, char *FromSymbol, char *ToSymbol,
			       AjPStr *Style, char *TextOri, ajint *NumLabels,
			       ajint *NumNames, ajint *Color);

static float lindna_TextGroup(float Margin, float TextHeight, float TextLength,
			      AjPStr *Name, char *TextOri, ajint NumLabels,
			      ajint *NumNames, AjPStr GroupName);
/*
static float lindna_TextGroupStr(float Margin, float TextHeight, AjPStr *Name,
				 char *TextOri, ajint NumLabels,
				 ajint *NumNames, AjPStr GroupName,
				 float TextCoef);
*/
static float lindna_HeightGroup(float posblock, float posrange, float postext,
				float TickHeight, float BlockHeight,
				float RangeHeight, AjPStr *Name,
				AjPStr *Style, char *TextOri, ajint NumLabels,
				ajint *NumNames, ajint Adjust);

static ajint lindna_OverlapTextGroup(AjPStr *Name, AjPStr *Style,
				     char *TextOri, ajint NumLabels,
				     float *From, float *To, ajint *Adjust);

static void lindna_DrawGroup(float xDraw, float yDraw, float Border,
			     float posblock, float posrange, float postext,
			     float DrawLength, float TickHeight,
			     float BlockHeight, float RangeHeight,
			     float TextLength, float TextHeight, float *From,
			     float *To, AjPStr *Name, char *FromSymbol,
			     char *ToSymbol, AjPStr *Style,
			     AjPStr InterSymbol, AjPStr InterTicks,
			     char *TextOri, ajint NumLabels, ajint *NumNames,
			     AjPStr GroupName, ajint *Adjust,
			     ajint InterColor, ajint *Color, AjPStr BlockType);

static float lindna_TextRuler(float Start, float End, ajint GapSize,
			      float TextLength, float TextHeight,
			      char TextOri);
/*
static float lindna_TextRulerStr(float Start, float End, ajint GapSize,
				 char TextOri,float TextCoef);
*/
static float lindna_HeightRuler(float Start, float End, ajint GapSize,
				float postext, float TickHeight, char TextOri);

static void lindna_DrawRuler(float xDraw, float yDraw, float Start, float End,
			     float ReduceCoef, float TickHeight,
			     float DrawLength, float RealLength, float Border,
			     ajint GapSize, AjPStr TickLines,
			     float TextLength, float TextHeight, float postext,
			     char TextOri, ajint Color);

static void lindna_DrawTicks(float xDraw, float yDraw, float TickHeight,
			     float From, AjPStr Name, float TextLength,
			     float TextHeight, float postext, char TextOri,
			     ajint NumNames, ajint Adjust, ajint Color);

static void lindna_DrawBlocks(float xDraw, float yDraw, float BlockHeight,
			      float TextHeight, float From, float To,
			      AjPStr Name, float postext, char TextOri,
			      ajint NumNames, ajint Adjust, ajint Color, 
			      AjPStr BlockType);

static void lindna_DrawRanges(float xDraw, float yDraw, float RangeHeight,
			      float From, float To, AjPStr Name,
			      char FromSymbol, char ToSymbol,
			      float TextLength, float TextHeight,
			      float postext, char TextOri, ajint NumNames,
			      ajint Adjust, ajint Color);

static void lindna_InterBlocks(float xDraw, float yDraw, float BlockHeight,
			       float From, float To, AjPStr InterSymbol,
			       ajint Color);

static void lindna_DrawArrowHeadsOnLine(float xDraw, float yDraw, float Height,
					float Length, ajint Way);

static void lindna_DrawBracketsOnLine(float xDraw, float yDraw, float Height,
				      float Length, ajint Way);

static void lindna_DrawBarsOnLine(float xDraw, float yDraw, float Height);

static void lindna_HorTextPile(float x, float y, AjPStr Name, float postext,
			       ajint NumNames);

static float lindna_HorTextPileHeight(float postext, ajint NumNames);

static void lindna_VerTextPile(float x, float y, AjPStr Name, float postext,
			       ajint NumNames);

static float lindna_VerTextPileHeight(AjPStr Name, float postext,
				      ajint NumNames);

static void lindna_VerTextSeq(float x, float y, AjPStr Name, float postext,
			      ajint NumNames);

static float lindna_VerTextSeqHeightMax(AjPStr Name, float postext,
					ajint NumNames);

/* static float lindna_VerTextSeqLength(float postext, ajint NumNames); */







/* @prog lindna ***************************************************************
**
** Draws linear maps of DNA constructs
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPGraph graph;
    ajint i;
    ajint j;
    ajint GapSize;
    ajint NumLabels[MAXGROUPS];
    ajint NumNames[MAXGROUPS][MAXLABELS];
    ajint NumGroups;
    ajint InterColor;
    ajint Color[MAXGROUPS][MAXLABELS];
    ajint Adjust[MAXGROUPS][MAXLABELS];
    ajint AdjustMax[MAXGROUPS];
    char FromSymbol[MAXGROUPS][MAXLABELS];
    char ToSymbol[MAXGROUPS][MAXLABELS];
    char TextOri[MAXGROUPS][MAXLABELS];
    float xDraw;
    float yDraw;
    float ReduceCoef;
    float From[MAXGROUPS][MAXLABELS];
    float To[MAXGROUPS][MAXLABELS];
    float TotalHeight;
    float GroupHeight[MAXGROUPS];
    float RulerHeight;
    float Width;
    float Height;
    float Border;
    float Margin;
    float Start;
    float End;
    float DrawLength;
    float RealLength;
    float TickHeight;
    float BlockHeight;
    float RangeHeight;
    float TextLength;
    float TextHeight;
    float GapGroup;
    float posblock;
    float posrange;
    float postext;
    AjPFile infile;
    AjPStr line;
    AjPStr GroupName[MAXGROUPS];
    AjPStr Ruler;
    AjPStr InterSymbol;
    AjPStr InterTicks;
    AjPStr TickLines;
    AjPStr BlockType;
    AjPStr Name[MAXGROUPS][MAXLABELS];
    AjPStr Style[MAXGROUPS][MAXLABELS];
    float charsize;
    float minsize;

    for(i=0;i<MAXGROUPS;++i)
	for(j=0;j<MAXLABELS;++j)
	{
	    Name[i][j] = ajStrNewC("");
	    Style[i][j] = ajStrNewC("");
	    To[i][j]=0.;
	}
    
    

    /* read the ACD file for graphical programs */
    ajGraphInit("lindna", argc, argv);

    /* to draw or not to draw the ruler */
    Ruler = ajAcdGetString("ruler");

    /* get the type of blocks */
    BlockType = ajAcdGetString("blocktype");
    /* get the type of junctions used to link blocks */
    InterSymbol = ajAcdGetString("intersymbol");
    /* get the color of junctions used to link blocks */
    InterColor = ajAcdGetInt("intercolor");

    /* to draw or not to draw junctions between ticks */
    InterTicks = ajAcdGetString("interticks");

    /* get the size of the intervals between the ruler's ticks */
    GapSize = ajAcdGetInt("gapsize");
    /* to draw or not to draw vertical lines at ruler's ticks */
    TickLines = ajAcdGetString("ticklines");


    /* set the output graphical context */
    graph = ajAcdGetGraph("graphout");

    /* get the input file */
    infile = ajAcdGetInfile("inputfile");

    /* length and height of text */
    /*TextHeight = 10;
    TextLength = TextHeight+25;*/
    TextHeight = 20*ajAcdGetFloat("textheight");
    TextLength = 40*ajAcdGetFloat("textlength");

    /* open the window in which the graphics will be drawn */
    DrawLength = 500;
    Border = TextLength/2;
    /*Margin = 50;*/
    Margin = 50*ajAcdGetFloat("margin");
    Width = DrawLength + 2*Border + Margin;
    Height = DrawLength + 2*Border;
    ajGraphOpenWin(graph, 0, Width, 0, Height);

    /* read the start and end positions */
    lindna_ReadInput(infile, &Start, &End);

    /*
     *  compute the coefficient of reduction to scale the real length of
     *  the molecule to window's size
     */
    RealLength = (End - Start) + 1;
    ReduceCoef = 1.0 *RealLength / DrawLength;


    /* coordinates of the origin */
    xDraw = Border + Margin;
    yDraw = DrawLength + Border;

    /* height of a tick, a block, and a range */
    TickHeight = 5*ajAcdGetFloat("tickheight");
    BlockHeight = 5*ajAcdGetFloat("blockheight");
    RangeHeight = 5*ajAcdGetFloat("rangeheight");

    /* set the relative positions of elements of a group */
    posblock = 0;
    posrange = 0;
    GapGroup = 10*ajAcdGetFloat("gapgroup");


    /* read the contents of the groups */
    line = ajStrNew();
    ajFileSeek(infile, 0L, 0);
    i = 0;
    while( ajFileReadLine(infile, &line) )
    {
	if( ajStrPrefixC(line, "group") )
	{
	    GroupName[i] = lindna_ReadGroup(infile, From[i], To[i], Name[i],
					    FromSymbol[i], ToSymbol[i],
					    Style[i], TextOri[i],
					    &NumLabels[i], NumNames[i],
					    Color[i]);
	    i++;
	}
    }
    NumGroups = i;

    /* scale the groups */
    for(i=0; i<NumGroups; i++)
	for(j=0; j<NumLabels[i]; j++)
	{
	    /*
	     *  remove the beginning of the molecule in case it doesn't
	     *  begin at 1
	     */
	    From[i][j]-=( Start-1 );
	    To[i][j]-=( Start-1 );
	    /* scale the real size to window's size */
	    From[i][j]/=ReduceCoef;
	    To[i][j]/=ReduceCoef;
	}


    /* compute the character size that fits all groups, including the ruler */
    minsize = 100.0;
    charsize = lindna_TextRuler(Start, End, GapSize, TextLength, TextHeight,
				'V');
    if( charsize<minsize )
	minsize = charsize;
    for(i=0; i<NumGroups; i++)
    {
	charsize = lindna_TextGroup(Margin, TextHeight, TextLength, Name[i],
				    TextOri[i], NumLabels[i], NumNames[i],
				    GroupName[i]);
	if( charsize<minsize )
	    minsize = charsize;
    }
    ajGraphSetDefCharSize(minsize);


    /* find whether horizontal text strings overlap within a group */
    postext = (ajGraphTextHeight(0, 0, 1, 0)+3)*ajAcdGetFloat("postext");
    for(i=0; i<NumGroups; i++)
	AdjustMax[i] = lindna_OverlapTextGroup(Name[i], Style[i], TextOri[i],
					       NumLabels[i], From[i], To[i],
					       Adjust[i]);


    /* compute the height of the ruler */
    RulerHeight = lindna_HeightRuler(Start, End, GapSize, postext,
				     TickHeight, 'V');
    /* compute the height of the groups */
    TotalHeight = RulerHeight+GapGroup;
    for(i=0; i<NumGroups; i++)
    {
	GroupHeight[i] = lindna_HeightGroup(posblock, posrange, postext,
					    TickHeight, BlockHeight,
					    RangeHeight, Name[i],
					    Style[i], TextOri[i],
					    NumLabels[i], NumNames[i],
					    AdjustMax[i]);
	TotalHeight += (GroupHeight[i]+GapGroup);
    }

    /*
     *  if the groups are too big, resize them such that they fit in the
     *  window
     */
    if( TotalHeight<DrawLength )
	TotalHeight = DrawLength;
    TickHeight/=(TotalHeight/DrawLength);
    BlockHeight/=(TotalHeight/DrawLength);
    RangeHeight/=(TotalHeight/DrawLength);
    TextHeight/=(TotalHeight/DrawLength);
    TextLength/=(TotalHeight/DrawLength);
    postext/=(TotalHeight/DrawLength);
    posblock/=(TotalHeight/DrawLength);
    posrange/=(TotalHeight/DrawLength);
    GapGroup/=(TotalHeight/DrawLength);

    /*
     *  the groups having been resized, recompute the character size that
     *  fits all groups, including the ruler
     */
    minsize = 100.0;
    /*charsize = lindna_TextRulerStr(Start, End, GapSize, 'V',
				   (TotalHeight/DrawLength));*/
    charsize = lindna_TextRuler(Start, End, GapSize, TextLength, TextHeight,
				'V');
    if( charsize<minsize )
	minsize = charsize;
    for(i=0; i<NumGroups; i++)
    {
      /*charsize = lindna_TextGroupStr(Margin, TextHeight, Name[i], TextOri[i],
				       NumLabels[i], NumNames[i], GroupName[i],
				       (TotalHeight/DrawLength));*/
	charsize = lindna_TextGroup(Margin, TextHeight, TextLength, Name[i],
				    TextOri[i], NumLabels[i], NumNames[i],
				    GroupName[i]);
	if( charsize<minsize )
	    minsize = charsize;
    }
    ajGraphSetDefCharSize(minsize);


    /* the ruler having been resized, recompute its height */
    RulerHeight = lindna_HeightRuler(Start, End, GapSize, postext,
				     TickHeight, 'V');
    /* the groups having been resized, recompute their height */
    TotalHeight = RulerHeight+GapGroup;
    for(i=0; i<NumGroups; i++)
    {
	GroupHeight[i] = lindna_HeightGroup(posblock, posrange, postext,
					    TickHeight, BlockHeight,
					    RangeHeight, Name[i],
					    Style[i], TextOri[i], NumLabels[i],
					    NumNames[i], AdjustMax[i]);
	TotalHeight += (GroupHeight[i]+GapGroup);
    }


    /* draw the ruler */
    yDraw-=RulerHeight;
    if( ajStrCmpCaseCC(ajStrStr(Ruler), "Y")==0 ) {
      lindna_DrawRuler(xDraw, yDraw, Start, End, ReduceCoef, TickHeight,
		     DrawLength, RealLength, Border, GapSize, TickLines,
		     TextLength, TextHeight, postext, 'V', 1);
      }

    /* draw the groups */
    for(i=0; i<NumGroups; i++)
    {
	yDraw-=( GroupHeight[i]+GapGroup );
	lindna_DrawGroup(xDraw, yDraw, Border, posblock, posrange, postext,
			 DrawLength, TickHeight, BlockHeight, RangeHeight,
			 TextLength, TextHeight, From[i], To[i], Name[i],
			 FromSymbol[i], ToSymbol[i], Style[i], InterSymbol,
			 InterTicks, TextOri[i], NumLabels[i], NumNames[i],
			 GroupName[i], Adjust[i], InterColor, Color[i], 
			 BlockType);
	ajStrDel(&GroupName[i]);
    }

    /* close the input file */
    ajFileClose(&infile);
    ajStrDel(&line);

    /* close the graphical window */
    ajGraphCloseWin();

    ajExit();
    return 0;
}

/* @funcstatic lindna_TextRuler ***********************************************
**
**  compute the character size that fits all elements of the ruler provided
** that the height and the length of all strings are at most TextHeight and
**  TextLength, respectively
**
** @param [?] Start [float] Undocumented
** @param [?] End [float] Undocumented
** @param [?] GapSize [ajint] Undocumented
** @param [?] TextLength [float] Undocumented
** @param [?] TextHeight [float] Undocumented
** @param [?] TextOri [char] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float lindna_TextRuler(float Start, float End, ajint GapSize,
			      float TextLength, float TextHeight, char TextOri)
{
    ajint i;
    AjPStr string = ajStrNew();
    float charsize, minsize = 100.0;

    ajStrFromInt(&string, Start);
    if( TextOri=='H' )
	charsize = ajGraphFitTextOnLine( 0, 0, TextLength, 0,
					ajStrStr(string), TextHeight );
    else
	charsize = ajGraphFitTextOnLine( 0, 0, 0, TextLength,
					ajStrStr(string), TextHeight );
    if( charsize < minsize )
	minsize = charsize;

    for(i=GapSize; i<End; i+=GapSize)
    {
	if( i>Start )
	{
	    ajStrFromInt(&string, i);
	    if( TextOri=='H' )
		charsize = ajGraphFitTextOnLine( 0, 0, TextLength, 0,
						ajStrStr(string), TextHeight );
	    else
		charsize = ajGraphFitTextOnLine( 0, 0, 0, TextLength,
						ajStrStr(string), TextHeight );
	    if( charsize < minsize )
		minsize = charsize;
	}
    }

    ajStrFromInt(&string, End);
    if( TextOri=='H' )
	charsize = ajGraphFitTextOnLine( 0, 0, TextLength, 0,
					ajStrStr(string), TextHeight );
    else
	charsize = ajGraphFitTextOnLine( 0, 0, 0, TextLength,
					ajStrStr(string), TextHeight );
    if( charsize < minsize )
	minsize = charsize;

    ajStrDel(&string);

    return minsize;
}




/* #funcstatic lindna_TextRulerStr ********************************************
**
**  compute the character size that fits all elements of the ruler provided
**  that the height and the length of all strings are multiplied by TextCoef
**
** #param [?] Start [float] Undocumented
** #param [?] End [float] Undocumented
** #param [?] GapSize [ajint] Undocumented
** #param [?] TextOri [char] Undocumented
** #param [?] TextCoef [float] Undocumented
** #return [float] Undocumented
** @@
******************************************************************************/
/*
//static float lindna_TextRulerStr(float Start, float End, ajint GapSize,
//				 char TextOri, float TextCoef)
//{
//    ajint i;
//    AjPStr string = ajStrNew();
//    float charsize;
//    float minsize = 100.0;
//    float stringLength;
//    float stringHeight;
//
//    ajStrFromInt(&string, Start);
//    if( TextOri=='H' )
//    {
//	stringLength = ajGraphTextLength( 0, 0, 1, 0, ajStrStr(string) );
//	stringHeight = ajGraphTextHeight(0, 0, 1, 0);
//	charsize = ajGraphFitTextOnLine( 0, 0, stringLength/TextCoef, 0,
//					ajStrStr(string),
//					stringHeight/TextCoef );
//    }
//    else
//    {
//	stringLength = ajGraphTextLength( 0, 0, 0, 1, ajStrStr(string) );
//	stringHeight = ajGraphTextHeight(0, 0, 0, 1);
//	charsize = ajGraphFitTextOnLine( 0, 0, 0, stringLength/TextCoef,
//					ajStrStr(string),
//					stringHeight/TextCoef );
//    }
//    if( charsize < minsize )
//	minsize = charsize;
//
//    for(i=GapSize; i<End; i+=GapSize)
//    {
//	if( i>Start )
//	{
//	    ajStrFromInt(&string, i);
//	    if( TextOri=='H' )
//	    {
//		stringLength = ajGraphTextLength( 0, 0, 1, 0,
//						 ajStrStr(string) );
//		stringHeight = ajGraphTextHeight(0, 0, 1, 0);
//		charsize = ajGraphFitTextOnLine( 0, 0, stringLength/TextCoef,
//						0, ajStrStr(string),
//						stringHeight/TextCoef );
//	    }
//	    else
//	    {
//		stringLength = ajGraphTextLength( 0, 0, 0, 1,
//						 ajStrStr(string) );
//		stringHeight = ajGraphTextHeight(0, 0, 0, 1);
//		charsize = ajGraphFitTextOnLine( 0, 0, 0,
//						stringLength/TextCoef,
//						ajStrStr(string),
//						stringHeight/TextCoef );
//	    }
//	    if( charsize < minsize )
//		minsize = charsize;
//	}
//    }
//
//    ajStrFromInt(&string, End);
//    if( TextOri=='H' )
//    {
//	stringLength = ajGraphTextLength( 0, 0, 1, 0, ajStrStr(string) );
//	stringHeight = ajGraphTextHeight(0, 0, 1, 0);
//	charsize = ajGraphFitTextOnLine( 0, 0, stringLength/TextCoef, 0,
//					ajStrStr(string),
//					stringHeight/TextCoef );
//    }
//    else
//    {
//	stringLength = ajGraphTextLength( 0, 0, 0, 1, ajStrStr(string) );
//	stringHeight = ajGraphTextHeight(0, 0, 0, 1);
//	charsize = ajGraphFitTextOnLine( 0, 0, 0, stringLength/TextCoef,
//					ajStrStr(string),
//					stringHeight/TextCoef );
//    }
//    if( charsize < minsize )
//	minsize = charsize;
//
//    ajStrDel(&string);
//
//    return minsize;
//}
*/


/* @funcstatic lindna_HeightRuler *********************************************
**
** compute the ruler's height
**
** @param [?] Start [float] Undocumented
** @param [?] End [float] Undocumented
** @param [?] GapSize [ajint] Undocumented
** @param [?] postext [float] Undocumented
** @param [?] TickHeight [float] Undocumented
** @param [?] TextOri [char] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float lindna_HeightRuler(float Start, float End, ajint GapSize,
				float postext, float TickHeight, char TextOri)
{
    ajint i;
    ajint j;
    float RulerHeight;
    AjPStr string = ajStrNew();
    AjPStr totalstring = ajStrNew();

    RulerHeight = TickHeight+postext;
    if( TextOri=='H' )
	RulerHeight += ajGraphTextHeight(0, 0, 1, 0);
    else
    {
	ajStrFromInt(&string, Start);
	ajStrApp(&totalstring, string);
	ajStrAppC(&totalstring, ";");
	for(i=GapSize, j=0; i<End; i+=GapSize, j++) if( i>Start )
	{
	    ajStrFromInt(&string, i);
	    ajStrApp(&totalstring, string);
	    ajStrAppC(&totalstring, ";");
	}
	ajStrFromInt(&string, End);
	ajStrApp(&totalstring, string);
	ajStrAppC(&totalstring, ";");
	RulerHeight += lindna_VerTextSeqHeightMax(totalstring, postext, j+2);
    }

    ajStrDel(&string);
    ajStrDel(&totalstring);

    return RulerHeight;
}


/* @funcstatic lindna_DrawRuler **********************************************
**
** draw a ruler
**
** @param [?] xDraw [float] Undocumented
** @param [?] yDraw [float] Undocumented
** @param [?] Start [float] Undocumented
** @param [?] End [float] Undocumented
** @param [?] ReduceCoef [float] Undocumented
** @param [?] TickHeight [float] Undocumented
** @param [?] DrawLength [float] Undocumented
** @param [?] RealLength [float] Undocumented
** @param [?] Border [float] Undocumented
** @param [?] GapSize [ajint] Undocumented
** @param [?] TickLines [AjPStr] Undocumented
** @param [?] TextLength [float] Undocumented
** @param [?] TextHeight [float] Undocumented
** @param [?] postext [float] Undocumented
** @param [?] TextOri [char] Undocumented
** @param [?] Color [ajint] Undocumented
** @@
******************************************************************************/

static void lindna_DrawRuler(float xDraw, float yDraw, float Start, float End,
			     float ReduceCoef, float TickHeight,
			     float DrawLength, float RealLength, float Border,
			     ajint GapSize, AjPStr TickLines,
			     float TextLength, float TextHeight,
			     float postext, char TextOri, ajint Color)
{
    ajint i;
    AjPStr string = ajStrNew();
    
    ajGraphSetFore(Color);
    
    ajGraphDrawLine( xDraw, yDraw, xDraw+DrawLength, yDraw );
    
    /* set the molecule's start */
    ajStrFromInt(&string, Start);
    if( ajStrCmpCaseCC(ajStrStr(TickLines), "Y")==0 )
	ajGraphDrawLine(xDraw, Border, xDraw, yDraw);
    lindna_DrawTicks(xDraw, yDraw, TickHeight, 0.0, string,
		     TextLength, TextHeight, postext, TextOri, 1, 0, Color);
    
    /* draw the ruler's ticks */
    for(i=GapSize; i<End; i+=GapSize)
    {
	if( i>Start )
	{
	    ajStrFromInt(&string, i);
	    if( ajStrCmpCaseCC(ajStrStr(TickLines), "Y")==0 )
		ajGraphDrawLine(xDraw+1.0*(i-Start)/ReduceCoef,
				Border, xDraw+1.0*(i-Start)/ReduceCoef,
				yDraw);
	    lindna_DrawTicks(xDraw, yDraw, TickHeight,
			     1.0*(i-Start)/ReduceCoef, string, TextLength,
			     TextHeight, postext, TextOri, 1, 0, Color);
	}
    }

    /* set the molecule's end */
    ajStrFromInt(&string, End);
    if( ajStrCmpCaseCC(ajStrStr(TickLines), "Y")==0 )
	ajGraphDrawLine(xDraw+1.0*RealLength/ReduceCoef, Border,
			xDraw+1.0*RealLength/ReduceCoef, yDraw);
    lindna_DrawTicks(xDraw, yDraw, TickHeight, 1.0*RealLength/ReduceCoef,
		     string, TextLength, TextHeight, postext, TextOri, 1, 0,
		     Color);
    
    ajStrDel(&string);
    return;
}




/* @funcstatic lindna_DrawTicks **********************************************
**
** draw a Tick
**
** @param [?] xDraw [float] Undocumented
** @param [?] yDraw [float] Undocumented
** @param [?] TickHeight [float] Undocumented
** @param [?] From [float] Undocumented
** @param [?] Name [AjPStr] Undocumented
** @param [?] TextLength [float] Undocumented
** @param [?] TextHeight [float] Undocumented
** @param [?] postext [float] Undocumented
** @param [?] TextOri [char] Undocumented
** @param [?] NumNames [ajint] Undocumented
** @param [?] Adjust [ajint] Undocumented
** @param [?] Color [ajint] Undocumented
** @@
******************************************************************************/

static void lindna_DrawTicks(float xDraw, float yDraw, float TickHeight,
			     float From, AjPStr Name, float TextLength,
			     float TextHeight, float postext, char TextOri,
			     ajint NumNames, ajint Adjust, ajint Color)
{
    float x1Ticks = xDraw+From;
    float y1Ticks = yDraw;
    float x2Ticks = x1Ticks;
    float y2Ticks = y1Ticks+TickHeight;

    ajGraphSetFore(Color);

    ajGraphDrawLine( x1Ticks, y1Ticks, x2Ticks, y2Ticks );
    if( NumNames!=0 )
    {
	if( TextOri=='H' )
	    lindna_HorTextPile( x1Ticks, y2Ticks+(Adjust*postext), Name,
			       postext, NumNames );
	else
	    lindna_VerTextPile( x1Ticks, y2Ticks+(Adjust*postext), Name,
			       postext, NumNames );
    }

    return;
}




/* @funcstatic lindna_DrawBlocks **********************************************
**
**  draw a Block
**
** @param [?] xDraw [float] Undocumented
** @param [?] yDraw [float] Undocumented
** @param [?] BlockHeight [float] Undocumented
** @param [?] TextHeight [float] Undocumented
** @param [?] From [float] Undocumented
** @param [?] To [float] Undocumented
** @param [?] Name [AjPStr] Undocumented
** @param [?] postext [float] Undocumented
** @param [?] TextOri [char] Undocumented
** @param [?] NumNames [ajint] Undocumented
** @param [?] Adjust [ajint] Undocumented
** @param [?] Color [ajint] Undocumented
** @param [?] BlockType [AjPStr] Undocumented
** @@
******************************************************************************/

static void lindna_DrawBlocks(float xDraw, float yDraw, float BlockHeight,
			      float TextHeight, float From, float To,
			      AjPStr Name, float postext, char TextOri,
			      ajint NumNames, ajint Adjust, ajint Color, 
			      AjPStr BlockType)
{
    float x1Blocks = xDraw+From;
    float y1Blocks = yDraw+(1.0*BlockHeight/2);
    float x2Blocks = xDraw+To;
    float y2Blocks = y1Blocks-BlockHeight;

    ajGraphSetFore(Color);
    if( ajStrCmpCaseCC(ajStrStr(BlockType), "Open")==0 ) { ajGraphRect( x1Blocks, y1Blocks, x2Blocks, y2Blocks ); }
    else if( ajStrCmpCaseCC(ajStrStr(BlockType), "Filled")==0 ) { ajGraphRectFill( x1Blocks, y1Blocks, x2Blocks, y2Blocks ); }
    else {
      ajGraphRectFill( x1Blocks, y1Blocks, x2Blocks, y2Blocks );
      ajGraphSetFore(0);
      ajGraphRect( x1Blocks, y1Blocks, x2Blocks, y2Blocks );
      ajGraphSetFore(Color);
      }

    /* ajGraphSetFore(0); */
    if( NumNames!=0 )
    {
	if( TextOri=='H' )
	    lindna_HorTextPile( (x1Blocks+x2Blocks)/2,
			       y1Blocks+(Adjust*postext), Name, postext,
			       NumNames );
	else
	    lindna_VerTextSeq( (x1Blocks+x2Blocks)/2,
			      y1Blocks+(Adjust*postext),
			      Name, postext, NumNames );
    }

    return;
}




/* @funcstatic lindna_DrawRanges **********************************************
**
** draw a Range
**
** @param [?] xDraw [float] Undocumented
** @param [?] yDraw [float] Undocumented
** @param [?] RangeHeight [float] Undocumented
** @param [?] From [float] Undocumented
** @param [?] To [float] Undocumented
** @param [?] Name [AjPStr] Undocumented
** @param [?] FromSymbol [char] Undocumented
** @param [?] ToSymbol [char] Undocumented
** @param [?] TextLength [float] Undocumented
** @param [?] TextHeight [float] Undocumented
** @param [?] postext [float] Undocumented
** @param [?] TextOri [char] Undocumented
** @param [?] NumNames [ajint] Undocumented
** @param [?] Adjust [ajint] Undocumented
** @param [?] Color [ajint] Undocumented
** @@
******************************************************************************/

static void lindna_DrawRanges(float xDraw, float yDraw, float RangeHeight,
			      float From, float To, AjPStr Name,
			      char FromSymbol, char ToSymbol,
			      float TextLength, float TextHeight,
			      float postext, char TextOri, ajint NumNames,
			      ajint Adjust, ajint Color)
{
    float x1Ranges = xDraw+From;
    float y1Ranges = yDraw;
    float x2Ranges = xDraw+To;
    float y2Ranges = y1Ranges;
    float yupper = yDraw+(1.0*RangeHeight/2);
    float BoundaryLength;

    ajGraphSetFore(Color);

    ajGraphDrawLine( x1Ranges, y1Ranges, x2Ranges, y2Ranges );
    if( NumNames!=0 )
    {
	if( TextOri=='H' )
	    lindna_HorTextPile( (x1Ranges+x2Ranges)/2,
			       yupper+(Adjust*postext), Name, postext,
			       NumNames );
	else
	    lindna_VerTextSeq( (x1Ranges+x2Ranges)/2,
			      yupper+(Adjust*postext), Name, postext,
			      NumNames );
    }

    if( RangeHeight>(To-From)/3 )
	BoundaryLength = (To-From)/3;
    else
	BoundaryLength = RangeHeight;

    if( FromSymbol=='<' )
	lindna_DrawArrowHeadsOnLine( x1Ranges, y1Ranges, RangeHeight,
				    BoundaryLength, +1);
    if( FromSymbol=='>' )
	lindna_DrawArrowHeadsOnLine( x1Ranges, y1Ranges, RangeHeight,
				    BoundaryLength, -1);
    if( FromSymbol=='[' )
	lindna_DrawBracketsOnLine( x1Ranges, y1Ranges, RangeHeight,
				  BoundaryLength, +1);
    if( FromSymbol==']' )
	lindna_DrawBracketsOnLine( x1Ranges, y1Ranges, RangeHeight,
				  BoundaryLength, -1);
    if( FromSymbol=='|' )
	lindna_DrawBarsOnLine( x1Ranges, y1Ranges, RangeHeight);

    if( ToSymbol=='<' )
	lindna_DrawArrowHeadsOnLine( x2Ranges, y2Ranges, RangeHeight,
				    BoundaryLength, +1);
    if( ToSymbol=='>' )
	lindna_DrawArrowHeadsOnLine( x2Ranges, y2Ranges, RangeHeight,
				    BoundaryLength, -1);
    if( ToSymbol=='[' )
	lindna_DrawBracketsOnLine( x2Ranges, y2Ranges, RangeHeight,
				  BoundaryLength, +1);
    if( ToSymbol==']' )
	lindna_DrawBracketsOnLine( x2Ranges, y2Ranges, RangeHeight,
				  BoundaryLength, -1);
    if( ToSymbol=='|' )
	lindna_DrawBarsOnLine( x2Ranges, y2Ranges, RangeHeight);

    return;
}

/* @funcstatic lindna_InterBlocks ********************************************
**
** draw an InterBlock
**
** @param [?] xDraw [float] Undocumented
** @param [?] yDraw [float] Undocumented
** @param [?] BlockHeight [float] Undocumented
** @param [?] From [float] Undocumented
** @param [?] To [float] Undocumented
** @param [?] InterSymbol [AjPStr] Undocumented
** @param [?] Color [ajint] Undocumented
** @@
******************************************************************************/

static void lindna_InterBlocks(float xDraw, float yDraw, float BlockHeight,
			       float From, float To, AjPStr InterSymbol,
			       ajint Color)
{
    float x1Inter = xDraw+From;
    float y1Inter = yDraw+(1.0*BlockHeight/2);
    float x2Inter = xDraw+To;
    float y2Inter = y1Inter-BlockHeight;

    ajGraphSetFore(Color);

    if( ajStrCmpCaseCC(ajStrStr(InterSymbol), "Down")==0 )
    {
	ajGraphDrawLine( x1Inter, y1Inter, (x1Inter+x2Inter)/2, y2Inter );
	ajGraphDrawLine( (x1Inter+x2Inter)/2, y2Inter, x2Inter, y1Inter );
    }

    if( ajStrCmpCaseCC(ajStrStr(InterSymbol), "Up")==0 )
    {
	ajGraphDrawLine( x1Inter, y2Inter, (x1Inter+x2Inter)/2, y1Inter );
	ajGraphDrawLine( (x1Inter+x2Inter)/2, y1Inter, x2Inter, y2Inter );
    }

    if( ajStrCmpCaseCC(ajStrStr(InterSymbol), "Straight")==0 )
	ajGraphDrawLine( x1Inter, (y1Inter+y2Inter)/2, x2Inter,
			(y1Inter+y2Inter)/2 );

    return;
}




/* @funcstatic lindna_DrawArrowHeadsOnLine ************************************
**
** draw arrowheads on a line
**
** @param [?] xDraw [float] Undocumented
** @param [?] yDraw [float] Undocumented
** @param [?] Height [float] Undocumented
** @param [?] Length [float] Undocumented
** @param [?] Way [ajint] Undocumented
** @@
******************************************************************************/

static void lindna_DrawArrowHeadsOnLine(float xDraw, float yDraw, float Height,
					float Length, ajint Way)
{
    float middle = 1.0*Height/2;

    if(Way==1)
    {
	ajGraphDrawLine( xDraw, yDraw, xDraw+Length, yDraw+middle );
	ajGraphDrawLine( xDraw, yDraw, xDraw+Length, yDraw-middle );
    }
    if(Way==-1)
    {
	ajGraphDrawLine( xDraw, yDraw, xDraw-Length, yDraw+middle );
	ajGraphDrawLine( xDraw, yDraw, xDraw-Length, yDraw-middle );
    }

    return;
}




/* @funcstatic lindna_DrawBracketsOnLine *************************************
**
** draw brackets on a line
**
** @param [?] xDraw [float] Undocumented
** @param [?] yDraw [float] Undocumented
** @param [?] Height [float] Undocumented
** @param [?] Length [float] Undocumented
** @param [?] Way [ajint] Undocumented
** @@
******************************************************************************/

static void lindna_DrawBracketsOnLine(float xDraw, float yDraw, float Height,
				      float Length, ajint Way)
{
    float middle = 1.0*Height/2;

    if(Way==1)
    {
	ajGraphDrawLine( xDraw, yDraw-middle, xDraw, yDraw+middle );
	ajGraphDrawLine( xDraw, yDraw+middle, xDraw+Length, yDraw+middle );
	ajGraphDrawLine( xDraw, yDraw-middle, xDraw+Length, yDraw-middle );
    }
    if(Way==-1)
    {
	ajGraphDrawLine( xDraw, yDraw-middle, xDraw, yDraw+middle );
	ajGraphDrawLine( xDraw, yDraw+middle, xDraw-Length, yDraw+middle );
	ajGraphDrawLine( xDraw, yDraw-middle, xDraw-Length, yDraw-middle );
    }

    return;
}


/* @funcstatic lindna_DrawBarsOnLine *************************************
**
** draw bars on a line
**
** @param [?] xDraw [float] Undocumented
** @param [?] yDraw [float] Undocumented
** @param [?] Height [float] Undocumented
** @@
******************************************************************************/

static void lindna_DrawBarsOnLine(float xDraw, float yDraw, float Height)
{
    float middle = 1.0*Height/2;

    ajGraphDrawLine( xDraw, yDraw-middle, xDraw, yDraw+middle );

    return;
}



/* @funcstatic lindna_HorTextPile ********************************************
**
** write a pile of horizontal text strings
**
** @param [?] x [float] Undocumented
** @param [?] y [float] Undocumented
** @param [?] Name [AjPStr] Undocumented
** @param [?] postext [float] Undocumented
** @param [?] NumNames [ajint] Undocumented
** @@
******************************************************************************/

static void lindna_HorTextPile(float x, float y, AjPStr Name, float postext,
			       ajint NumNames)
{
    float yupper;
    float stringLength;
    float stringHeight;
    float totalHeight;
    static AjPStr token=NULL;
    ajint i;

    totalHeight = y+postext;
    for(i=0; i<NumNames; i++)
    {
	if(i==0)
	    token = ajStrTokC(Name, ";");
	else
	    token = ajStrTokC(NULL, ";");
	stringLength = ajGraphTextLength( 0, 0, 1, 0, ajStrStr(token) );
	stringHeight = ajGraphTextHeight(0, 0, 1, 0);
	yupper = totalHeight+stringHeight;
	ajGraphDrawTextOnLine( x, (totalHeight+yupper)/2, x+stringLength,
			      (totalHeight+yupper)/2, ajStrStr(token), 0.5 );
	totalHeight+=(stringHeight+postext);
    }


    return;
}




/* @funcstatic lindna_HorTextPileHeight **************************************
**
** compute the height of a pile of horizontal text strings
**
** @param [?] postext [float] Undocumented
** @param [?] NumNames [ajint] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float lindna_HorTextPileHeight(float postext, ajint NumNames)
{
    float stringHeight, totalHeight;
    ajint i;

    totalHeight = 0.0;
    for(i=0; i<NumNames; i++)
    {
	stringHeight = ajGraphTextHeight(0, 0, 1, 0);
	totalHeight+=(stringHeight+postext);
    }

    return totalHeight;
}




/* @funcstatic lindna_VerTextPile ********************************************
**
** write a pile of vertical text strings
**
** @param [?] x [float] Undocumented
** @param [?] y [float] Undocumented
** @param [?] Name [AjPStr] Undocumented
** @param [?] postext [float] Undocumented
** @param [?] NumNames [ajint] Undocumented
** @@
******************************************************************************/

static void lindna_VerTextPile(float x, float y, AjPStr Name, float postext,
			       ajint NumNames)
{
    float stringLength, totalLength;
    AjPStr token;
    ajint i;

    totalLength = postext;
    for(i=0; i<NumNames; i++)
    {
	if(i==0)
	    token = ajStrTokC(Name, ";");
	else
	    token = ajStrTokC(NULL, ";");
	stringLength = ajGraphTextLength( 0, 0, 0, 1, ajStrStr(token) );
	ajGraphDrawTextOnLine( x, y+totalLength, x,
			      y+stringLength+totalLength, ajStrStr(token),
			      0.0 );
	totalLength+=(stringLength+postext);
    }

    return;
}




/* @funcstatic lindna_VerTextPileHeight **************************************
**
** compute the height of a pile of vertical text strings
**
** @param [?] Name [AjPStr] Undocumented
** @param [?] postext [float] Undocumented
** @param [?] NumNames [ajint] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float lindna_VerTextPileHeight(AjPStr Name, float postext,
				      ajint NumNames)
{
    float stringLength;
    float totalLength;
    AjPStr token;
    ajint i;

    totalLength = postext;
    for(i=0; i<NumNames; i++)
    {
	if(i==0)
	    token = ajStrTokC(Name, ";");
	else
	    token = ajStrTokC(NULL, ";");
	stringLength = ajGraphTextLength( 0, 0, 0, 1, ajStrStr(token) );
	totalLength+=(stringLength+postext);
    }

    return totalLength;
}




/* @funcstatic lindna_VerTextSeq *********************************************
**
** write a sequence of vertical text strings
**
** @param [?] x [float] Undocumented
** @param [?] y [float] Undocumented
** @param [?] Name [AjPStr] Undocumented
** @param [?] postext [float] Undocumented
** @param [?] NumNames [ajint] Undocumented
** @@
******************************************************************************/

static void lindna_VerTextSeq(float x, float y, AjPStr Name, float postext,
			      ajint NumNames)
{
    float stringHeight;
    AjPStr token;
    ajint i;

    stringHeight = ajGraphTextHeight(0, 0, 0, 1)+postext;
    for(i=0; i<NumNames; i++)
    {
	if(i==0)
	    token = ajStrTokC(Name, ";");
	else
	    token = ajStrTokC(NULL, ";");
	ajGraphDrawTextOnLine( x-1.0*NumNames*stringHeight/2+1.0*
			      stringHeight/2+1.0*i*stringHeight,
			      y+postext, x-1.0*NumNames*stringHeight/2+1.0*
			      stringHeight/2+1.0*i*stringHeight,
			      y+postext+stringHeight,
			      ajStrStr(token),
			      0.0 );
    }

    return;
}




/* @funcstatic lindna_VerTextSeqHeightMax ************************************
**
** compute the height of a sequence of vertical text strings
** (this is the height of the longest string)
**
** @param [?] Name [AjPStr] Undocumented
** @param [?] postext [float] Undocumented
** @param [?] NumNames [ajint] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float lindna_VerTextSeqHeightMax(AjPStr Name, float postext,
					ajint NumNames)
{
    float stringLength;
    float maxLength;
    AjPStr token;
    ajint i;

    maxLength = 0.0;
    for(i=0; i<NumNames; i++)
    {
	if(i==0)
	    token = ajStrTokC(Name, ";");
	else
	    token = ajStrTokC(NULL, ";");
	stringLength = ajGraphTextLength( 0, 0, 0, 1, ajStrStr(token) );
	if( stringLength>maxLength )
	    maxLength = stringLength;
    }

    return (maxLength+postext);
}




/* #funcstatic lindna_VerTextSeqLength ***************************************
**
** compute the length of a sequence of vertical text strings
**
** #param [?] postext [float] Undocumented
** #param [?] NumNames [ajint] Undocumented
** #return [float] Undocumented
** @@
******************************************************************************/
/*
//static float lindna_VerTextSeqLength(float postext, ajint NumNames)
//{
//    float stringHeight;
//    float totalHeight;
//    ajint i;
//
//    totalHeight = 0.0;
//    for(i=0; i<NumNames; i++)
//    {
//	stringHeight = ajGraphTextHeight(0, 0, 0, 1);
//	totalHeight+=(stringHeight+postext);
//    }
//
//    return (totalHeight-postext);
//}
*/



/* @funcstatic lindna_ReadInput *********************************************
**
** read the beginning of the input file
**
** @param [?] infile [AjPFile] Undocumented
** @param [?] Start [float*] Undocumented
** @param [?] End [float*] Undocumented
** @@
******************************************************************************/

static void lindna_ReadInput(AjPFile infile, float *Start, float *End)
{
    AjPStr line;

    line = ajStrNew();
    while( ajFileReadLine(infile, &line) )
    {
	/* read the start and end positions */
	if( ajStrPrefixC(line, "Start") )
	    sscanf(ajStrStr(line), "%*s%f", Start);
	if( ajStrPrefixC(line, "End") )
	    sscanf(ajStrStr(line), "%*s%f", End);
    }

    ajStrDel(&line);
    return;
}




/* @funcstatic lindna_ReadGroup **********************************************
**
** read a group
**
** @param [?] infile [AjPFile] Undocumented
** @param [?] From [float*] Undocumented
** @param [?] To [float*] Undocumented
** @param [?] Name [AjPStr*] Undocumented
** @param [?] FromSymbol [char*] Undocumented
** @param [?] ToSymbol [char*] Undocumented
** @param [?] Style [AjPStr*] Undocumented
** @param [?] TextOri [char*] Undocumented
** @param [?] NumLabels [ajint*] Undocumented
** @param [?] NumNames [ajint*] Undocumented
** @param [?] Color [ajint*] Undocumented
** @return [AjPStr] Undocumented
** @@
******************************************************************************/

static AjPStr lindna_ReadGroup(AjPFile infile, float *From, float *To,
			       AjPStr *Name, char *FromSymbol, char *ToSymbol,
			       AjPStr *Style, char *TextOri, ajint *NumLabels,
			       ajint *NumNames, ajint *Color)
{
    ajint i;
    ajint j;
    AjPStr GroupName;
    AjPStr line;
    AjPStr token;
    char *style;
    ajlong pos;

    line = ajStrNew();
    GroupName = ajStrNew();
    style = (char *)AJALLOC0( 10*sizeof(char) );

    /* read the group's name */
    pos = ajFileTell(infile);
    while( ajFileReadLine(infile, &GroupName) )
    {
	token = ajStrTokC(GroupName, " \n\t\r\f");
	if( ajStrLen(token)!=0 )
	{
	    if( ajStrMatchCaseC(GroupName, "label") ||
	       ajStrMatchCaseC(GroupName, "endgroup") )
		ajStrAssC(&GroupName, " ");
	    if( ajStrLen(GroupName)>20 )
		ajStrCut( &GroupName, 20, ajStrLen(GroupName)-1 );
	    break;
	}
    }

    i = 0;
    ajFileSeek(infile, pos, 0);
    while( ajFileReadLine(infile, &line) )
    {
	token = ajStrTokC(line, " \n\t\r\f");
	if( ajStrLen(token)!=0 )
	{
	    if( ajStrPrefixC(line, "endgroup") )
		break;
	    else
	    {
		/* read the group's label(s) */
		if( ajStrPrefixC(line, "label") )
		{
		    while( ajFileReadLine(infile, &line) )
		    {
			token = ajStrTokC(line, " \n\t\r\f");
			if( ajStrLen(token)!=0 )
			{
			    FromSymbol[i] = '<';
			    ToSymbol[i] = '>';
			    TextOri[i] = 'H';
			    sscanf( ajStrStr(line), "%s", style );
			    if( ajStrMatchCaseCC(style, "Tick") )
				sscanf( ajStrStr(line), "%*s %f %d %c",
				       &From[i], &Color[i], &TextOri[i] );
			    if( ajStrMatchCaseCC(style, "Block") )
				sscanf( ajStrStr(line), "%*s %f %f %d %c",
				       &From[i], &To[i], &Color[i],
				       &TextOri[i] );
			    if( ajStrMatchCaseCC(style, "Range") )
				sscanf( ajStrStr(line),"%*s %f %f %c %c %d %c",
				       &From[i], &To[i], &FromSymbol[i],
				       &ToSymbol[i], &Color[i], &TextOri[i] );
			    ajStrAssC(&Style[i], style);
			    break;
			}
		    }

		    j = 0;
		    /* read the label's name(s) */
		    while( ajFileReadLine(infile, &line) )
		    {
			token = ajStrTokC(line, " \n\t\r\f");
			if( ajStrLen(token)!=0 )
			{
			    if( ajStrPrefixC(line, "endlabel") )
				break;
			    else
			    {
				ajStrApp(&Name[i], line);
				ajStrAppC(&Name[i], ";");
				j++;
			    }
			}
		    }
		    NumNames[i] = j;
		    i++;
		}
	    }
	}
    }
    *NumLabels = i;

    AJFREE(style);
    ajStrDel(&line);
    return GroupName;
}




/* @funcstatic lindna_TextGroup **********************************************
**
** compute the character size that fits all elements of a group provided that
** the height and the length of all strings are at most TextHeight and
** TextLength, respectively
**
** @param [?] Margin [float] Undocumented
** @param [?] TextHeight [float] Undocumented
** @param [?] TextLength [float] Undocumented
** @param [?] Name [AjPStr*] Undocumented
** @param [?] TextOri [char*] Undocumented
** @param [?] NumLabels [ajint] Undocumented
** @param [?] NumNames [ajint*] Undocumented
** @param [?] GroupName [AjPStr] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float lindna_TextGroup(float Margin, float TextHeight, float TextLength,
			      AjPStr *Name, char *TextOri, ajint NumLabels,
			      ajint *NumNames, AjPStr GroupName)
{
    ajint i;
    ajint j;
    float charsize;
    float minsize = 100.0;
    AjPStr token;

    charsize = ajGraphFitTextOnLine( 0, 0, Margin-10, 0, ajStrStr(GroupName),
				    TextHeight );
    if( charsize < minsize )
	minsize = charsize;

    for(i=0; i<NumLabels; i++)
    {
	for(j=0; j<NumNames[i]; j++)
	{
	    if(j==0)
		token = ajStrTokC(Name[i], ";");
	    else
		token = ajStrTokC(NULL, ";");
	    if( TextOri[i]=='H' )
		charsize = ajGraphFitTextOnLine( 0, 0, TextLength, 0,
						ajStrStr(token), TextHeight );
	    else
		charsize = ajGraphFitTextOnLine( 0, 0, 0, TextLength,
						ajStrStr(token), TextHeight );
	    if( charsize < minsize )
		minsize = charsize;
	}
    }

    return minsize;
}




/* #funcstatic lindna_TextGroupStr *******************************************
**
**  compute the character size that fits all elements of a group provided that
**  the height and the length of all strings are multiplied by TextCoef
**
** #param [?] Margin [float] Undocumented
** #param [?] TextHeight [float] Undocumented
** #param [?] Name [AjPStr*] Undocumented
** #param [?] TextOri [char*] Undocumented
** #param [?] NumLabels [ajint] Undocumented
** #param [?] NumNames [ajint*] Undocumented
** #param [?] GroupName [AjPStr] Undocumented
** #param [?] TextCoef [float] Undocumented
** #return [float] Undocumented
** @@
******************************************************************************/
/*
//static float lindna_TextGroupStr(float Margin, float TextHeight, AjPStr *Name,
//				 char *TextOri, ajint NumLabels,
//				 ajint *NumNames, AjPStr GroupName,
//				 float TextCoef)
//{
//    ajint i;
//    ajint j;
//    float charsize;
//    float minsize = 100.0;
//    float stringLength;
//    float stringHeight;
//    AjPStr token;
//
//    charsize = ajGraphFitTextOnLine( 0, 0, Margin-10, 0, ajStrStr(GroupName),
//				    TextHeight );
//    if( charsize < minsize )
//	minsize = charsize;
//
//    for(i=0; i<NumLabels; i++)
//    {
//	for(j=0; j<NumNames[i]; j++)
//	{
//	    if(j==0)
//		token = ajStrTokC(Name[i], ";");
//	    else
//		token = ajStrTokC(NULL, ";");
//	    if( TextOri[i]=='H' )
//	    {
//		stringLength = ajGraphTextLength( 0, 0, 1, 0,
//						 ajStrStr(token) );
//		stringHeight = ajGraphTextHeight(0, 0, 1, 0);
//		charsize = ajGraphFitTextOnLine( 0, 0, stringLength/TextCoef,
//						0, ajStrStr(token),
//						stringHeight/TextCoef );
//	    }
//	    else
//	    {
//		stringLength = ajGraphTextLength( 0, 0, 0, 1,
//						 ajStrStr(token) );
//		stringHeight = ajGraphTextHeight(0, 0, 0, 1);
//		charsize = ajGraphFitTextOnLine( 0, 0, 0,
//						stringLength/TextCoef,
//						ajStrStr(token),
//						stringHeight/TextCoef );
//	    }
//	    if( charsize < minsize )
//		minsize = charsize;
//	}
//    }
//
//    return minsize;
//}
*/



/* @funcstatic lindna_HeightGroup *********************************************
**
** compute the height of a group depending on what's in it
**
** @param [?] posblock [float] Undocumented
** @param [?] posrange [float] Undocumented
** @param [?] postext [float] Undocumented
** @param [?] TickHeight [float] Undocumented
** @param [?] BlockHeight [float] Undocumented
** @param [?] RangeHeight [float] Undocumented
** @param [?] Name [AjPStr*] Undocumented
** @param [?] Style [AjPStr*] Undocumented
** @param [?] TextOri [char*] Undocumented
** @param [?] NumLabels [ajint] Undocumented
** @param [?] NumNames [ajint*] Undocumented
** @param [?] Adjust [ajint] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float lindna_HeightGroup(float posblock, float posrange, float postext,
				float TickHeight, float BlockHeight,
				float RangeHeight, AjPStr *Name,
				AjPStr *Style, char *TextOri, ajint NumLabels,
				ajint *NumNames, ajint Adjust)
{
    ajint i;
    float GroupHeight;
    float uheight;
    float umaxheight = 0.0;
    float lheight;
    float lmaxheight = 0.0;

    for(i=0; i<NumLabels; i++)
    {
	if( ajStrMatchCaseC(Style[i], "Tick") )
	{
	    uheight = TickHeight;
	    lheight = 0.0;
	    if( TextOri[i]=='H' )
		uheight+=lindna_HorTextPileHeight(postext, NumNames[i]);
	    else
		uheight+=lindna_VerTextPileHeight(Name[i], postext,
						  NumNames[i]);
	    if( uheight > umaxheight )
		umaxheight = uheight;
	    if( lheight > lmaxheight )
		lmaxheight = lheight;
	}

	if( ajStrMatchCaseC(Style[i], "Block") )
	{
	    uheight = 1.0*BlockHeight/2;
	    lheight = 1.0*BlockHeight/2;
	    if( TextOri[i]=='H' )
		uheight+=lindna_HorTextPileHeight(postext, NumNames[i]);
	    else
		uheight+=lindna_VerTextSeqHeightMax(Name[i], postext,
						    NumNames[i]);
	    if( uheight > umaxheight )
		umaxheight = uheight;
	    if( lheight > lmaxheight )
		lmaxheight = lheight;
	}

	if( ajStrMatchCaseC(Style[i], "Range") )
	{
	    uheight = 1.0*RangeHeight/2;
	    lheight = 1.0*RangeHeight/2;
	    if( TextOri[i]=='H' )
		uheight+=lindna_HorTextPileHeight(postext, NumNames[i]);
	    else
		uheight+=lindna_VerTextSeqHeightMax(Name[i], postext,
						    NumNames[i]);
	    if( uheight > umaxheight )
		umaxheight = uheight;
	    if( lheight > lmaxheight )
		lmaxheight = lheight;
	}
    }

    GroupHeight = umaxheight+lmaxheight+(Adjust*postext);

    return GroupHeight;
}




/* @funcstatic lindna_OverlapTextGroup ***************************************
**
** find whether horizontal text strings overlap within a group
**
** @param [?] Name [AjPStr*] Undocumented
** @param [?] Style [AjPStr*] Undocumented
** @param [?] TextOri [char*] Undocumented
** @param [?] NumLabels [ajint] Undocumented
** @param [?] From [float*] Undocumented
** @param [?] To [float*] Undocumented
** @param [?] Adjust [ajint*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint lindna_OverlapTextGroup(AjPStr *Name, AjPStr *Style,
				     char *TextOri, ajint NumLabels,
				     float *From, float *To, ajint *Adjust)
{
    ajint i;
    ajint j;
    ajint AdjustMax;
    AjPStr token;
    float FromText[MAXLABELS];
    float ToText[MAXLABELS];
    float stringLength;


    /* compute the length of the horizontal strings */
    for(i=0; i<NumLabels; i++)
    {
	if( ajStrMatchCaseC(Style[i], "Tick") )
	{
	    if( TextOri[i]=='H' )
	    {
		token = ajStrTokC(Name[i], ";");
		stringLength = ajGraphTextLength( 0, 0, 1, 0,
						 ajStrStr(token) );
		FromText[i] = From[i]-stringLength/2;
		ToText[i] = From[i]+stringLength/2;
	    }
	    else
	    {
		FromText[i] = From[i];
		ToText[i] = From[i];
	    }
	}

	if( ajStrMatchCaseC(Style[i], "Block") )
	{
	    if( TextOri[i]=='H' )
	    {
		token = ajStrTokC(Name[i], ";");
		stringLength = ajGraphTextLength( 0, 0, 1, 0,
						 ajStrStr(token) );
		FromText[i] = (To[i]+From[i])/2-stringLength/2;
		ToText[i] = (To[i]+From[i])/2+stringLength/2;
	    }
	    else
	    {
		FromText[i] = (To[i]+From[i])/2;
		ToText[i] = (To[i]+From[i])/2;
	    }
	}

	if( ajStrMatchCaseC(Style[i], "Range") )
	{
	    if( TextOri[i]=='H' )
	    {
		token = ajStrTokC(Name[i], ";");
		stringLength = ajGraphTextLength( 0, 0, 1, 0,
						 ajStrStr(token) );
		FromText[i] = (To[i]+From[i])/2-stringLength/2;
		ToText[i] = (To[i]+From[i])/2+stringLength/2;
	    }
	    else
	    {
		FromText[i] = (To[i]+From[i])/2;
		ToText[i] = (To[i]+From[i])/2;
	    }
	}
    }


    /*
     *  if some strings overlap, the position of the overlapping strings is
     *   moved upwards by Adjust
     */
    for(i=0; i<NumLabels; i++)
	Adjust[i] = 0;
    for(i=0; i<NumLabels; i++)
    {
	for(j=0; j<NumLabels; j++)
	{
	    if( (i!=j) && (Adjust[i]==Adjust[j]) )
	    {
		if(j>i)
		{
		    if((ToText[i]>=FromText[j]) && (FromText[i]<=FromText[j]))
			Adjust[j] = Adjust[i]+1;
		    if( (ToText[i]>=ToText[j]) && (FromText[i]<=ToText[j]) )
			Adjust[j] = Adjust[i]+1;
		    if( (ToText[i]<=ToText[j]) && (FromText[i]>=FromText[j]) )
			Adjust[j] = Adjust[i]+1;
		    if( (ToText[i]>=ToText[j]) && (FromText[i]<=FromText[j]) )
			Adjust[j] = Adjust[i]+1;
		}
		if(i>j)
		{
		    if((ToText[j]>=FromText[i]) && (FromText[j]<=FromText[i]))
			Adjust[i] = Adjust[j]+1;
		    if( (ToText[j]>=ToText[i]) && (FromText[j]<=ToText[i]) )
			Adjust[i] = Adjust[j]+1;
		    if( (ToText[j]<=ToText[i]) && (FromText[j]>=FromText[i]) )
			Adjust[i] = Adjust[j]+1;
		    if( (ToText[j]>=ToText[i]) && (FromText[j]<=FromText[i]) )
			Adjust[i] = Adjust[j]+1;
		}
	    }
	}
    }
    AdjustMax = 0.0;
    for(i=0; i<NumLabels; i++)
	if( Adjust[i]>AdjustMax )
	    AdjustMax = Adjust[i];

    return AdjustMax;
}




/* @funcstatic lindna_DrawGroup **********************************************
**
** draw a group
**
** @param [?] xDraw [float] Undocumented
** @param [?] yDraw [float] Undocumented
** @param [?] Border [float] Undocumented
** @param [?] posblock [float] Undocumented
** @param [?] posrange [float] Undocumented
** @param [?] postext [float] Undocumented
** @param [?] DrawLength [float] Undocumented
** @param [?] TickHeight [float] Undocumented
** @param [?] BlockHeight [float] Undocumented
** @param [?] RangeHeight [float] Undocumented
** @param [?] TextLength [float] Undocumented
** @param [?] TextHeight [float] Undocumented
** @param [?] From [float*] Undocumented
** @param [?] To [float*] Undocumented
** @param [?] Name [AjPStr*] Undocumented
** @param [?] FromSymbol [char*] Undocumented
** @param [?] ToSymbol [char*] Undocumented
** @param [?] Style [AjPStr*] Undocumented
** @param [?] InterSymbol [AjPStr] Undocumented
** @param [?] InterTicks [AjPStr] Undocumented
** @param [?] TextOri [char*] Undocumented
** @param [?] NumLabels [ajint] Undocumented
** @param [?] NumNames [ajint*] Undocumented
** @param [?] GroupName [AjPStr] Undocumented
** @param [?] Adjust [ajint*] Undocumented
** @param [?] InterColor [ajint] Undocumented
** @param [?] Color [ajint*] Undocumented
** @param [?] BlockType [AjPStr] Undocumented
** @@
******************************************************************************/

static void lindna_DrawGroup(float xDraw, float yDraw, float Border,
			     float posblock, float posrange, float postext,
			     float DrawLength, float TickHeight,
			     float BlockHeight, float RangeHeight,
			     float TextLength, float TextHeight, float *From,
			     float *To, AjPStr *Name, char *FromSymbol,
			     char *ToSymbol, AjPStr *Style,
			     AjPStr InterSymbol, AjPStr InterTicks,
			     char *TextOri, ajint NumLabels, ajint *NumNames,
			     AjPStr GroupName, ajint *Adjust,
			     ajint InterColor, ajint *Color, AjPStr BlockType)
{
    ajint i;
    ajint j;
    ajint NumBlocks;
    ajint Inter[MAXLABELS];

    /*ajGraphSetBackgroundWhite();*/
    ajGraphSetFore(1);
    ajGraphDrawTextOnLine( 10, yDraw, xDraw-Border, yDraw,
			  ajStrStr(GroupName), 0.0 );

    /* draw all labels */
    for(j=0, i=0; i<NumLabels; i++)
    {
	if( ajStrMatchCaseC(Style[i], "Tick") )
	{
	    lindna_DrawTicks(xDraw, yDraw, TickHeight, From[i], Name[i],
			     TextLength, TextHeight, postext, TextOri[i],
			     NumNames[i], Adjust[i], Color[i]);
	    if( ajStrCmpCaseCC(ajStrStr(InterTicks), "Y")==0 )
		ajGraphDrawLine( xDraw, yDraw, xDraw+DrawLength, yDraw );
	}

	if( ajStrMatchCaseC(Style[i], "Block") )
	{
	    lindna_DrawBlocks(xDraw, yDraw-posblock, BlockHeight, TextHeight,
			      From[i], To[i], Name[i], postext, TextOri[i],
			      NumNames[i], Adjust[i], Color[i], BlockType);
	    Inter[j++] = i;
	}

	if( ajStrMatchCaseC(Style[i], "Range") )
	    lindna_DrawRanges(xDraw, yDraw-posrange, RangeHeight, From[i],
			      To[i], Name[i], FromSymbol[i], ToSymbol[i],
			      TextLength, TextHeight, postext, TextOri[i],
			      NumNames[i], Adjust[i], Color[i]);
    }
    NumBlocks = j;
    
    /* draw all interblocks */
    for(i=0; i<NumBlocks-1; i++)
	lindna_InterBlocks(xDraw, yDraw-posblock, BlockHeight, To[Inter[i]],
			   From[Inter[i+1]], InterSymbol, InterColor);
    
    return;
}
