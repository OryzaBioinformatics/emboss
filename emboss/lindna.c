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

#define MAXLABELS 100

void ReadInput(AjPFile infile, float *Start, float *End);
AjPStr ReadGroup(AjPFile infile, float *From, float *To, AjPStr *Name,
		 char *FromSymbol, char *ToSymbol, AjPStr *Style,
		 char *TextOri, int *NumLabels, int *NumNames, int *Color);
float TextGroup(float Margin, float TextHeight, float TextLength,
		AjPStr *Name, char *TextOri, int NumLabels, int *NumNames,
		AjPStr GroupName);
float TextGroupStr(float Margin, float TextHeight, AjPStr *Name,
		   char *TextOri, int NumLabels, int *NumNames,
		   AjPStr GroupName, float TextCoef);
float HeightGroup(float posblock, float posrange, float postext,
		  float TickHeight, float BlockHeight, float RangeHeight,
		  AjPStr *Name, AjPStr *Style, char *TextOri, int NumLabels,
		  int *NumNames, int Adjust);
int OverlapTextGroup(AjPStr *Name, AjPStr *Style, char *TextOri,
		     int NumLabels, float *From, float *To, int *Adjust);
void DrawGroup(float xDraw, float yDraw, float Border, float posblock,
	       float posrange, float postext, float DrawLength,
	       float TickHeight, float BlockHeight, float RangeHeight,
	       float TextLength, float TextHeight, float *From,
	       float *To, AjPStr *Name, char *FromSymbol, char *ToSymbol,
	       AjPStr *Style, AjPStr InterSymbol, AjPStr InterTicks,
	       char *TextOri, int NumLabels, int *NumNames,
	       AjPStr GroupName, int *Adjust, int InterColor, int *Color);
float TextRuler(float Start, float End, int GapSize, float TextLength,
		float TextHeight, char TextOri);
float TextRulerStr(float Start, float End, int GapSize, char TextOri,
		   float TextCoef);
float HeightRuler(float Start, float End, int GapSize, float postext,
		  float TickHeight, char TextOri);
void DrawRuler(float xDraw, float yDraw, float Start, float End,
	       float ReduceCoef, float TickHeight, float DrawLength,
	       float RealLength, float Border, int GapSize, AjPStr TickLines,
	       float TextLength, float TextHeight, float postext,
	       char TextOri, int Color);
void DrawTicks(float xDraw, float yDraw, float TickHeight, float From,
	       AjPStr Name, float TextLength, float TextHeight,
	       float postext, char TextOri, int NumNames, int Adjust,
	       int Color);
void DrawBlocks(float xDraw, float yDraw, float BlockHeight,
		float TextHeight, float From, float To, AjPStr Name,
		float postext, char TextOri, int NumNames, int Adjust,
		int Color);
void DrawRanges(float xDraw, float yDraw, float RangeHeight, float From,
		float To, AjPStr Name, char FromSymbol, char ToSymbol,
		float TextLength, float TextHeight, float postext,
		char TextOri, int NumNames, int Adjust, int Color);
void InterBlocks(float xDraw, float yDraw, float BlockHeight, float From,
		 float To, AjPStr InterSymbol, int Color);
void DrawArrowHeadsOnLine(float xDraw, float yDraw, float Height,
			  float Length, int Way);
void DrawBracketsOnLine(float xDraw, float yDraw, float Height,
			float Length, int Way);
void HorTextPile(float x, float y, AjPStr Name, float postext,
		 int NumNames);
float HorTextPileHeight(float postext, int NumNames);
void VerTextPile(float x, float y, AjPStr Name, float postext,
		 int NumNames);
float VerTextPileHeight(AjPStr Name, float postext, int NumNames);
void VerTextSeq(float x, float y, AjPStr Name, float postext,
		int NumNames);
float VerTextSeqHeightMax(AjPStr Name, float postext,
			  int NumNames);
float VerTextSeqLength(float postext, int NumNames);






int main(int argc, char **argv)
{
    AjPGraph graph;
    int i;
    int j;
    int GapSize;
    int NumLabels[MAXLABELS];
    int NumNames[MAXLABELS][MAXLABELS];
    int NumGroups;
    int InterColor;
    int Color[MAXLABELS][MAXLABELS];
    int Adjust[MAXLABELS][MAXLABELS];
    int AdjustMax[MAXLABELS];
    char FromSymbol[MAXLABELS][MAXLABELS];
    char ToSymbol[MAXLABELS][MAXLABELS];
    char TextOri[MAXLABELS][MAXLABELS];
    float xDraw;
    float yDraw;
    float ReduceCoef;
    float From[MAXLABELS][MAXLABELS];
    float To[MAXLABELS][MAXLABELS];
    float TotalHeight;
    float GroupHeight[MAXLABELS];
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
    AjPStr GroupName[MAXLABELS];
    AjPStr InterSymbol;
    AjPStr InterTicks;
    AjPStr TickLines;
    AjPStr Name[MAXLABELS][MAXLABELS];
    AjPStr Style[MAXLABELS][MAXLABELS];
    float charsize;
    float minsize;


    /* read the ACD file for graphical programs */
    ajGraphInit("lindna", argc, argv);

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
    TextHeight = 20;
    TextLength = TextHeight+50;

    /* open the window in which the graphics will be drawn */
    DrawLength = 500;
    Border = TextLength/2;
    Margin = 100;
    Width = DrawLength + 2*Border + Margin;
    Height = DrawLength + 2*Border;
    ajGraphOpenWin(graph, 0, Width, 0, Height);

    /* read the start and end positions */
    ReadInput(infile, &Start, &End);

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
	    GroupName[i] = ReadGroup(infile, From[i], To[i], Name[i],
				     FromSymbol[i], ToSymbol[i], Style[i],
				     TextOri[i], &NumLabels[i], NumNames[i],
				     Color[i]);
	    i++;
	}
    }
    NumGroups = i;

    /* scale the groups */
    for(i=0; i<NumGroups; i++)
	for(j=0; j<NumLabels[i]; j++)
	{
	    /* remove the beginning of the molecule in case it doesn't begin at 1 */
	    From[i][j]-=( Start-1 );
	    To[i][j]-=( Start-1 );
	    /* scale the real size to window's size */
	    From[i][j]/=ReduceCoef;
	    To[i][j]/=ReduceCoef;
	}


    /* compute the character size that fits all groups, including the ruler */
    minsize = 100.0;
    charsize = TextRuler(Start, End, GapSize, TextLength, TextHeight, 'V');
    if( charsize<minsize ) minsize = charsize;
    for(i=0; i<NumGroups; i++)
    {
	charsize = TextGroup(Margin, TextHeight, TextLength, Name[i],
			     TextOri[i], NumLabels[i], NumNames[i],
			     GroupName[i]);
	if( charsize<minsize )
	    minsize = charsize;
    }
    ajGraphSetDefCharSize(minsize);


    /* find whether horizontal text strings overlap within a group */
    postext = (ajGraphTextHeight(0, 0, 1, 0)+3)*ajAcdGetFloat("postext");
    for(i=0; i<NumGroups; i++)
	AdjustMax[i] = OverlapTextGroup(Name[i], Style[i], TextOri[i],
					NumLabels[i], From[i], To[i],
					Adjust[i]);


    /* compute the height of the ruler */
    RulerHeight = HeightRuler(Start, End, GapSize, postext, TickHeight, 'V');
    /* compute the height of the groups */
    TotalHeight = RulerHeight+GapGroup;
    for(i=0; i<NumGroups; i++)
    {
	GroupHeight[i] = HeightGroup(posblock, posrange, postext, TickHeight,
				     BlockHeight, RangeHeight, Name[i],
				     Style[i], TextOri[i], NumLabels[i],
				     NumNames[i], AdjustMax[i]);
	TotalHeight += (GroupHeight[i]+GapGroup);
    }

    /*
     *  if the groups are too big, resize them such that they fit in the
     *  window
     */
    if( TotalHeight<DrawLength ) TotalHeight = DrawLength;
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
    charsize = TextRulerStr(Start, End, GapSize, 'V',
			    (TotalHeight/DrawLength));
    if( charsize<minsize ) minsize = charsize;
    for(i=0; i<NumGroups; i++)
    {
	charsize = TextGroupStr(Margin, TextHeight, Name[i], TextOri[i],
				NumLabels[i], NumNames[i], GroupName[i],
				(TotalHeight/DrawLength));
	if( charsize<minsize )
	    minsize = charsize;
    }
    ajGraphSetDefCharSize(minsize);


    /* the ruler having been resized, recompute its height */
    RulerHeight = HeightRuler(Start, End, GapSize, postext, TickHeight, 'V');
    /* the groups having been resized, recompute their height */
    TotalHeight = RulerHeight+GapGroup;
    for(i=0; i<NumGroups; i++)
    {
	GroupHeight[i] = HeightGroup(posblock, posrange, postext, TickHeight,
				     BlockHeight, RangeHeight, Name[i],
				     Style[i], TextOri[i], NumLabels[i],
				     NumNames[i], AdjustMax[i]);
	TotalHeight += (GroupHeight[i]+GapGroup);
    }


    /* draw the ruler */
    yDraw-=RulerHeight;
    DrawRuler(xDraw, yDraw, Start, End, ReduceCoef, TickHeight, DrawLength,
	      RealLength, Border, GapSize, TickLines, TextLength,
	      TextHeight, postext, 'V', 1);

    /* draw the groups */
    for(i=0; i<NumGroups; i++)
    {
	yDraw-=( GroupHeight[i]+GapGroup );
	DrawGroup(xDraw, yDraw, Border, posblock, posrange, postext,
		  DrawLength, TickHeight, BlockHeight, RangeHeight,
		  TextLength, TextHeight, From[i], To[i], Name[i],
		  FromSymbol[i], ToSymbol[i], Style[i], InterSymbol,
		  InterTicks, TextOri[i], NumLabels[i], NumNames[i],
		  GroupName[i], Adjust[i], InterColor, Color[i]);
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






/*
 *  compute the character size that fits all elements of the ruler provided
 * that the height and the length of all strings are at most TextHeight and
 *  TextLength, respectively
 */
float TextRuler(float Start, float End, int GapSize, float TextLength,
		float TextHeight, char TextOri)
{
    int i;
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






/*
 *  compute the character size that fits all elements of the ruler provided
 *  that the height and the length of all strings are multiplied by TextCoef
 */
float TextRulerStr(float Start, float End, int GapSize, char TextOri,
		   float TextCoef)
{
    int i;
    AjPStr string = ajStrNew();
    float charsize;
    float minsize = 100.0;
    float stringLength;
    float stringHeight;

    ajStrFromInt(&string, Start);
    if( TextOri=='H' )
    {
	stringLength = ajGraphTextLength( 0, 0, 1, 0, ajStrStr(string) );
	stringHeight = ajGraphTextHeight(0, 0, 1, 0);
	charsize = ajGraphFitTextOnLine( 0, 0, stringLength/TextCoef, 0,
					ajStrStr(string),
					stringHeight/TextCoef );
    }
    else
    {
	stringLength = ajGraphTextLength( 0, 0, 0, 1, ajStrStr(string) );
	stringHeight = ajGraphTextHeight(0, 0, 0, 1);
	charsize = ajGraphFitTextOnLine( 0, 0, 0, stringLength/TextCoef,
					ajStrStr(string),
					stringHeight/TextCoef );
    }
    if( charsize < minsize )
	minsize = charsize;

    for(i=GapSize; i<End; i+=GapSize)
    {
	if( i>Start )
	{
	    ajStrFromInt(&string, i);
	    if( TextOri=='H' )
	    {
		stringLength = ajGraphTextLength( 0, 0, 1, 0,
						 ajStrStr(string) );
		stringHeight = ajGraphTextHeight(0, 0, 1, 0);
		charsize = ajGraphFitTextOnLine( 0, 0, stringLength/TextCoef,
						0, ajStrStr(string),
						stringHeight/TextCoef );
	    }
	    else
	    {
		stringLength = ajGraphTextLength( 0, 0, 0, 1,
						 ajStrStr(string) );
		stringHeight = ajGraphTextHeight(0, 0, 0, 1);
		charsize = ajGraphFitTextOnLine( 0, 0, 0,
						stringLength/TextCoef,
						ajStrStr(string),
						stringHeight/TextCoef );
	    }
	    if( charsize < minsize )
		minsize = charsize;
	}
    }

    ajStrFromInt(&string, End);
    if( TextOri=='H' )
    {
	stringLength = ajGraphTextLength( 0, 0, 1, 0, ajStrStr(string) );
	stringHeight = ajGraphTextHeight(0, 0, 1, 0);
	charsize = ajGraphFitTextOnLine( 0, 0, stringLength/TextCoef, 0,
					ajStrStr(string),
					stringHeight/TextCoef );
    }
    else
    {
	stringLength = ajGraphTextLength( 0, 0, 0, 1, ajStrStr(string) );
	stringHeight = ajGraphTextHeight(0, 0, 0, 1);
	charsize = ajGraphFitTextOnLine( 0, 0, 0, stringLength/TextCoef,
					ajStrStr(string),
					stringHeight/TextCoef );
    }
    if( charsize < minsize )
	minsize = charsize;

    ajStrDel(&string);

    return minsize;
}






/* compute the ruler's height */
float HeightRuler(float Start, float End, int GapSize, float postext,
		  float TickHeight, char TextOri)
{
    int i;
    int j;
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
    RulerHeight += VerTextSeqHeightMax(totalstring, postext, j+2);
}

    ajStrDel(&string);
    ajStrDel(&totalstring);

    return RulerHeight;
}






/* draw a ruler */
void DrawRuler(float xDraw, float yDraw, float Start, float End,
	       float ReduceCoef, float TickHeight, float DrawLength,
	       float RealLength, float Border, int GapSize,
	       AjPStr TickLines, float TextLength, float TextHeight,
	       float postext, char TextOri, int Color)
{
    int i;
    AjPStr string = ajStrNew();
    
    ajGraphSetFore(Color);
    
    ajGraphDrawLine( xDraw, yDraw, xDraw+DrawLength, yDraw );
    
    /* set the molecule's start */
    ajStrFromInt(&string, Start);
    if( ajStrCmpCaseCC(ajStrStr(TickLines), "Y")==0 )
	ajGraphDrawLine(xDraw, Border, xDraw, yDraw);
    DrawTicks(xDraw, yDraw, TickHeight, 0.0, string,
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
	    DrawTicks(xDraw, yDraw, TickHeight,
		      1.0*(i-Start)/ReduceCoef, string, TextLength,
		      TextHeight, postext, TextOri, 1, 0, Color);
	}
    }

    /* set the molecule's end */
    ajStrFromInt(&string, End);
    if( ajStrCmpCaseCC(ajStrStr(TickLines), "Y")==0 )
	ajGraphDrawLine(xDraw+1.0*RealLength/ReduceCoef, Border,
			xDraw+1.0*RealLength/ReduceCoef, yDraw);
    DrawTicks(xDraw, yDraw, TickHeight, 1.0*RealLength/ReduceCoef, string,
	      TextLength, TextHeight, postext, TextOri, 1, 0, Color);
    
    ajStrDel(&string);
    return;
}






/* draw a Tick */
void DrawTicks(float xDraw, float yDraw, float TickHeight, float From,
	       AjPStr Name, float TextLength, float TextHeight,
	       float postext, char TextOri, int NumNames, int Adjust,
	       int Color)
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
	    HorTextPile( x1Ticks, y2Ticks+(Adjust*postext), Name, postext,
			NumNames );
	else
	    VerTextPile( x1Ticks, y2Ticks+(Adjust*postext), Name, postext,
			NumNames );
    }

    return;
}






/* draw a Block */
void DrawBlocks(float xDraw, float yDraw, float BlockHeight,
		float TextHeight, float From, float To, AjPStr Name,
		float postext, char TextOri, int NumNames, int Adjust,
		int Color)
{
    float x1Blocks = xDraw+From;
    float y1Blocks = yDraw+(1.0*BlockHeight/2);
    float x2Blocks = xDraw+To;
    float y2Blocks = y1Blocks-BlockHeight;

    ajGraphSetFore(Color);

    ajGraphRect( x1Blocks, y1Blocks, x2Blocks, y2Blocks );
    if( NumNames!=0 )
    {
	if( TextOri=='H' )
	    HorTextPile( (x1Blocks+x2Blocks)/2, y1Blocks+(Adjust*postext),
			Name, postext, NumNames );
	else
	    VerTextSeq( (x1Blocks+x2Blocks)/2, y1Blocks+(Adjust*postext),
		       Name, postext, NumNames );
    }

    return;
}






/* draw a Range */
void DrawRanges(float xDraw, float yDraw, float RangeHeight, float From,
		float To, AjPStr Name, char FromSymbol, char ToSymbol,
		float TextLength, float TextHeight, float postext,
		char TextOri, int NumNames, int Adjust, int Color)
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
	    HorTextPile( (x1Ranges+x2Ranges)/2, yupper+(Adjust*postext), Name, postext, NumNames );
	else
	    VerTextSeq( (x1Ranges+x2Ranges)/2, yupper+(Adjust*postext), Name, postext, NumNames );
    }

    if( RangeHeight>(To-From)/3 )
	BoundaryLength = (To-From)/3;
    else
	BoundaryLength = RangeHeight;

    if( FromSymbol=='<' )
	DrawArrowHeadsOnLine( x1Ranges, y1Ranges, RangeHeight, BoundaryLength,
			     +1);
    if( FromSymbol=='>' )
	DrawArrowHeadsOnLine( x1Ranges, y1Ranges, RangeHeight, BoundaryLength,
			     -1);
    if( FromSymbol=='[' )
	DrawBracketsOnLine( x1Ranges, y1Ranges, RangeHeight, BoundaryLength,
			   +1);
    if( FromSymbol==']' )
	DrawBracketsOnLine( x1Ranges, y1Ranges, RangeHeight, BoundaryLength,
			   -1);

    if( ToSymbol=='<' )
	DrawArrowHeadsOnLine( x2Ranges, y2Ranges, RangeHeight, BoundaryLength,
			     +1);
    if( ToSymbol=='>' )
	DrawArrowHeadsOnLine( x2Ranges, y2Ranges, RangeHeight, BoundaryLength,
			     -1);
    if( ToSymbol=='[' )
	DrawBracketsOnLine( x2Ranges, y2Ranges, RangeHeight, BoundaryLength,
			   +1);
    if( ToSymbol==']' )
	DrawBracketsOnLine( x2Ranges, y2Ranges, RangeHeight, BoundaryLength,
			   -1);

    return;
}






/* draw an InterBlock */
void InterBlocks(float xDraw, float yDraw, float BlockHeight, float From,
		 float To, AjPStr InterSymbol, int Color)
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






/* draw arrowheads on a line */
void DrawArrowHeadsOnLine(float xDraw, float yDraw, float Height,
			  float Length, int Way)
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






/* draw brackets on a line */
void DrawBracketsOnLine(float xDraw, float yDraw, float Height, float Length,
			int Way)
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






/* write a pile of horizontal text strings */
void HorTextPile(float x, float y, AjPStr Name, float postext, int NumNames)
{
    float yupper;
    float stringLength;
    float stringHeight;
    float totalHeight;
    AjPStr token;
    int i;

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
	ajGraphDrawTextOnLine( x, (totalHeight+yupper)/2, x+stringLength, (totalHeight+yupper)/2, ajStrStr(token), 0.5 );
	totalHeight+=(stringHeight+postext);
    }

    return;
}






/* compute the height of a pile of horizontal text strings */
float HorTextPileHeight(float postext, int NumNames)
{
    float stringHeight, totalHeight;
    int i;

    totalHeight = 0.0;
    for(i=0; i<NumNames; i++)
    {
	stringHeight = ajGraphTextHeight(0, 0, 1, 0);
	totalHeight+=(stringHeight+postext);
    }

    return totalHeight;
}






/* write a pile of vertical text strings */
void VerTextPile(float x, float y, AjPStr Name, float postext, int NumNames)
{
    float stringLength, totalLength;
    AjPStr token;
    int i;

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






/* compute the height of a pile of vertical text strings */
float VerTextPileHeight(AjPStr Name, float postext, int NumNames)
{
    float stringLength;
    float totalLength;
    AjPStr token;
    int i;

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






/* write a sequence of vertical text strings */
void VerTextSeq(float x, float y, AjPStr Name, float postext, int NumNames)
{
    float stringHeight;
    AjPStr token;
    int i;

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






/* compute the height of a sequence of vertical text strings (this is the height of the longest string) */
float VerTextSeqHeightMax(AjPStr Name, float postext, int NumNames)
{
    float stringLength;
    float maxLength;
    AjPStr token;
    int i;

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






/* compute the length of a sequence of vertical text strings */
float VerTextSeqLength(float postext, int NumNames)
{
    float stringHeight;
    float totalHeight;
    int i;

    totalHeight = 0.0;
    for(i=0; i<NumNames; i++)
    {
	stringHeight = ajGraphTextHeight(0, 0, 0, 1);
	totalHeight+=(stringHeight+postext);
    }

    return (totalHeight-postext);
}






/* read the beginning of the input file */
void ReadInput(AjPFile infile, float *Start, float *End)
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






/* read a group */
AjPStr ReadGroup(AjPFile infile, float *From, float *To, AjPStr *Name,
		 char *FromSymbol, char *ToSymbol, AjPStr *Style,
		 char *TextOri, int *NumLabels, int *NumNames, int *Color)
{
    int i;
    int j;
    AjPStr GroupName;
    AjPStr line;
    AjPStr token;
    char *style;
    long pos;

    line = ajStrNew();
    GroupName = ajStrNew();
    style = (char *)AJALLOC( 10*sizeof(char) );

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
		ajStrCut( &GroupName, 20, ajStrLen(GroupName) );
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

    ajStrDel(&line);
    return GroupName;
}






/*
 *  compute the character size that fits all elements of a group provided that
 *  the height and the length of all strings are at most TextHeight and
 *  TextLength, respectively
 */
float TextGroup(float Margin, float TextHeight, float TextLength,
		AjPStr *Name, char *TextOri, int NumLabels, int *NumNames,
		AjPStr GroupName)
{
    int i;
    int j;
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






/*
 *  compute the character size that fits all elements of a group provided that
 *  the height and the length of all strings are multiplied by TextCoef
 */
float TextGroupStr(float Margin, float TextHeight, AjPStr *Name,
		   char *TextOri, int NumLabels, int *NumNames,
		   AjPStr GroupName, float TextCoef)
{
    int i;
    int j;
    float charsize;
    float minsize = 100.0;
    float stringLength;
    float stringHeight;
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
	    {
		stringLength = ajGraphTextLength( 0, 0, 1, 0,
						 ajStrStr(token) );
		stringHeight = ajGraphTextHeight(0, 0, 1, 0);
		charsize = ajGraphFitTextOnLine( 0, 0, stringLength/TextCoef,
						0, ajStrStr(token),
						stringHeight/TextCoef );
	    }
	    else
	    {
		stringLength = ajGraphTextLength( 0, 0, 0, 1,
						 ajStrStr(token) );
		stringHeight = ajGraphTextHeight(0, 0, 0, 1);
		charsize = ajGraphFitTextOnLine( 0, 0, 0,
						stringLength/TextCoef,
						ajStrStr(token),
						stringHeight/TextCoef );
	    }
	    if( charsize < minsize )
		minsize = charsize;
	}
    }

    return minsize;
}






/* compute the height of a group depending on what's in it */
float HeightGroup(float posblock, float posrange, float postext,
		  float TickHeight, float BlockHeight, float RangeHeight,
		  AjPStr *Name, AjPStr *Style, char *TextOri, int NumLabels,
		  int *NumNames, int Adjust)
{
    int i;
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
		uheight+=HorTextPileHeight(postext, NumNames[i]);
	    else
		uheight+=VerTextPileHeight(Name[i], postext, NumNames[i]);
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
		uheight+=HorTextPileHeight(postext, NumNames[i]);
	    else
		uheight+=VerTextSeqHeightMax(Name[i], postext, NumNames[i]);
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
		uheight+=HorTextPileHeight(postext, NumNames[i]);
	    else
		uheight+=VerTextSeqHeightMax(Name[i], postext, NumNames[i]);
	    if( uheight > umaxheight )
		umaxheight = uheight;
	    if( lheight > lmaxheight )
		lmaxheight = lheight;
	}
    }

    GroupHeight = umaxheight+lmaxheight+(Adjust*postext);

    return GroupHeight;
}







/* find whether horizontal text strings overlap within a group */
int OverlapTextGroup(AjPStr *Name, AjPStr *Style, char *TextOri,
		     int NumLabels, float *From, float *To, int *Adjust)
{
    int i;
    int j;
    int AdjustMax;
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






/* draw a group */
void DrawGroup(float xDraw, float yDraw, float Border, float posblock,
	       float posrange, float postext, float DrawLength,
	       float TickHeight, float BlockHeight, float RangeHeight,
	       float TextLength, float TextHeight, float *From, float *To,
	       AjPStr *Name, char *FromSymbol, char *ToSymbol,
	       AjPStr *Style, AjPStr InterSymbol, AjPStr InterTicks,
	       char *TextOri, int NumLabels, int *NumNames, AjPStr GroupName,
	       int *Adjust, int InterColor, int *Color)
{
    int i;
    int j;
    int NumBlocks;
    int Inter[MAXLABELS];

    /*ajGraphSetBackgroundWhite();*/
    ajGraphSetFore(1);
    ajGraphDrawTextOnLine( 10, yDraw, xDraw-Border, yDraw,
			  ajStrStr(GroupName), 0.0 );

    /* draw all labels */
    for(j=0, i=0; i<NumLabels; i++)
    {
	if( ajStrMatchCaseC(Style[i], "Tick") )
	{
	    DrawTicks(xDraw, yDraw, TickHeight, From[i], Name[i], TextLength,
		      TextHeight, postext, TextOri[i], NumNames[i], Adjust[i],
		      Color[i]);
	    if( ajStrCmpCaseCC(ajStrStr(InterTicks), "Y")==0 )
		ajGraphDrawLine( xDraw, yDraw, xDraw+DrawLength, yDraw );
	}

	if( ajStrMatchCaseC(Style[i], "Block") )
	{
	    DrawBlocks(xDraw, yDraw-posblock, BlockHeight, TextHeight, From[i],
		       To[i], Name[i], postext, TextOri[i], NumNames[i],
		       Adjust[i], Color[i]);
	    Inter[j++] = i;
	}

	if( ajStrMatchCaseC(Style[i], "Range") )
	    DrawRanges(xDraw, yDraw-posrange, RangeHeight, From[i], To[i],
		       Name[i], FromSymbol[i], ToSymbol[i], TextLength,
		       TextHeight, postext, TextOri[i], NumNames[i], Adjust[i],
		       Color[i]);
    }
    NumBlocks = j;
    
    /* draw all interblocks */
    for(i=0; i<NumBlocks-1; i++)
	InterBlocks(xDraw, yDraw-posblock, BlockHeight, To[Inter[i]],
		    From[Inter[i+1]], InterSymbol, InterColor);
    
    return;
}
