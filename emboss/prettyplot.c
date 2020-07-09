/* @source prettyplot application
**
** Displays and plots sequence alignments and consensus for PLOTTERS.
** @author: Copyright (C) Ian Longden (il@sanger.ac.uk)
** @@
**
** Replaces program "prettyplot" (EGCG)
**
** options.
**
** -ccolours    Colour residues by there consensus value. (TRUE)
** -cidentity   Colour to display identical matches. (RED)  
** -csimilarity Colour to display similar matches.   (GREEN)
** -cother      Colour to display other matches.     (BLACK)
**
** -docolour    Colour residues by table oily, amide, basic etc. (FALSE)
**
** -title       Display a title.     (TRUE)
**  
** -shade       Colour residues by shades. (BLPW)
**              B-Black L-Brown P-Wheat W-White
**
** -pair        values to represent identical similar related (1.5,1.0,0.5)
**
** -identity    Only match those which are identical in all sequences.
**
** -box         Display prettybox.
**
**
** -consensus   Display the consensus.
**
** -name        Display the sequence names.
**
** -number      Display the residue number at the end of each sequence.
**
** -maxnamelen  Margin size for the sequence name.
**
** -plurality   plurality check value used in consensus. (totweight/2)
**
** -collision   Allow collisions.
**
** -portrait    Display as a portrait (default landscape).
**
** -datafile    The data file holding the matrix comparison table.
**
** -showscore   Print out the scores for a residue number.
**
** -alternative 3 other checks for collisions.
**
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
#include "ajtime.h"

#define BOXTOP      0x0001
#define BOXBOT      0x0002
#define BOXLEF      0x0004
#define BOXRIG      0x0008
#define BOXCOLOURED 0x0010

char **seqcharptr;
ajint **seqcolptr;
ajint **seqboxptr;
ajint *seqcount=0;
ajint charlen;
AjBool shownames;
AjBool shownumbers;
AjPSeqset seqset;
AjPStr *seqnames;
ajint numgaps;
char *constr=0;


static ajint prettyplot_calcseqperpage(float yincr,float y,AjBool consensus);
static ajint prettyplot_fillinboxes(float ystart, ajint length, ajint numseq,
				    ajint resbreak, AjPSeqCvt cvt,
				    float yincr, ajint numres,
				    AjBool consensus,AjBool title,
				    ajint start, ajint end,AjBool boxcol,
				    AjBool boxit, ajint seqstart,
				    ajint seqend, AjPFile outf, AjBool data,
				    ajint datacol, float datacs);


/* @prog prettyplot ***********************************************************
**
** Displays aligned sequences, with colouring and boxing
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajint i,numseq,j=0,numres,count,k;
    ajint kmax;
    float defheight,currentheight;
    AjPStr shade=NULL,pair=NULL;
    AjPGraph graph = NULL;
    AjPMatrix cmpmatrix=NULL;
    AjPSeqCvt cvt=NULL;
    AjBool consensus;
    AjBool colourbyconsensus;
    AjBool colourbyresidues;
    AjBool colourbyshade = AJFALSE;
    AjBool title;
    AjBool boxit;
    AjBool boxcol;
    AjBool portrait;
    AjBool collision;
    AjBool okay = AJTRUE;
    ajint identity;
    AjBool listoptions;
    ajint alternative;
    AjPStr sidentity=NULL,ssimilarity=NULL,sother=NULL;
    AjPStr sboxcolval=NULL;
    AjPStr options=NULL;
    ajint showscore = 0;
    ajint iboxcolval=0;
    ajint cidentity = RED;
    ajint csimilarity = GREEN;
    ajint cother = BLACK;
    float fxp,fyp,yincr,y;
    ajint ixlen,iylen,ixoff,iyoff;
    char res[2]=" ";
    AjPFile outf=NULL;
    AjBool  data;
    AjPStr  fname=NULL;
    ajint     fcnt=1;
    ajint     datacol=0;
    float   datacs=0.0;
    
    float *score=0;
    float scoremax=0;
  
    float* identical;
    ajint   identicalmaxindex;
    float* matching;
    ajint   matchingmaxindex;
  
    float* colcheck;
  
    ajint **matrix;
    ajint m1=0,m2=0,ms=0,highindex=0,index;
    ajint *previous=0, con=0,currentstate=0,old=0;
    ajint *colmat=0,*shadecolour=0;
    float identthresh = 1.5;
    float simthresh = 1.0;
    float relthresh = 0.5;
    float part=0.0;
    char *cptr;
    ajint resbreak;
    float fplural;
    float ystart;
    AjOTime ajtime;
    const time_t tim = time(0);
    ajint gapcount=0;
    ajint countforgap=0;
    ajint boxindex;
    float max;
    ajint matsize;
    ajint seqperpage=0,startseq,endseq;
    ajint newILend = 0;
    ajint newILstart;

    ajtime.time = localtime(&tim);
    ajtime.format = 0;
  
    (void) ajGraphInit ("prettyplot", argc, argv);
  
    seqset = ajAcdGetSeqset("msf");
  
    numres = ajAcdGetInt("residuesperline");
    resbreak = ajAcdGetInt("resbreak");
  
    ajSeqsetFill (seqset);	/* Pads sequence set with gap characters */
    numseq = ajSeqsetSize (seqset);

    graph  = ajAcdGetGraph("graph");
    colourbyconsensus = ajAcdGetBool("ccolours");
    colourbyresidues = ajAcdGetBool("docolour");
    title = ajAcdGetBool("title");
    shade  = ajAcdGetString("shade");
    pair  = ajAcdGetString("pair");
    identity  = ajAcdGetInt("identity");
    boxit  = ajAcdGetBool("box");
    if(boxit)
    {
	AJCNEW(seqboxptr, numseq);
	for(i=0;i<numseq;i++){
	    AJCNEW(seqboxptr[i], ajSeqsetLen(seqset));
	}
    }
    boxcol  = ajAcdGetBool("boxcol");
    sboxcolval  = ajAcdGetString("boxcolval");

    if(boxcol)
    {
	iboxcolval = ajGraphCheckColour(sboxcolval);
	if(iboxcolval == -1)
	    iboxcolval = GREY;
    }
    consensus = ajAcdGetBool("consensus");
    if(consensus)
    {
	AJCNEW(constr, ajSeqsetLen(seqset)+1);
	constr[0] = '\0';
    }
    shownames = ajAcdGetBool("name");
    shownumbers = ajAcdGetBool("number");
    charlen = ajAcdGetInt("maxnamelen");
    fplural = ajAcdGetFloat("plurality");
    portrait = ajAcdGetBool("portrait");
    collision = ajAcdGetBool("collision");
    listoptions = ajAcdGetBool("listoptions");
    alternative = ajAcdGetInt("alternative");
    cmpmatrix  = ajAcdGetMatrix("matrixfile");
    showscore = ajAcdGetInt("showscore");
    data      = ajAcdGetBool("data");
    
  
    matrix = ajMatrixArray(cmpmatrix);
    cvt = ajMatrixCvt(cmpmatrix);
    matsize = ajMatrixSize(cmpmatrix);
  
    AJCNEW(identical,matsize);
    AJCNEW(matching,matsize);
    AJCNEW(colcheck,matsize);
  
    numgaps = numres/resbreak;
    numgaps--;
  
    if(portrait)
    {
	if(!data)
	    ajGraphSetOri(1);
	ystart = 75.0;
    }
    else
	ystart = 75.0;

    /* pair is a formatted string. Needs a pattern in the ACD file */

    if(pair)
    {
	if(sscanf(ajStrStr(pair),"%f,%f,%f",&identthresh,&simthresh,
		  &relthresh) != 3)
	{
	    ajFatal("pair %S could not be read. Default of 1.5,1.0,0.5"
		   " being used",pair);
	    identthresh = 1.5;
	    simthresh = 1.0;
	    relthresh = 0.5;
	}
    }
  
    /* shade is a formatted string. Needs a pattern in the ACD file */

    if(shade->Len)
    {
	if(shade->Len == 4)
	{
	    AJCNEW(shadecolour,4);
	    cptr = ajStrStr(shade);
	    for(i=0;i<4;i++){
		if(cptr[i]== 'B' || cptr[i]== 'b')
		    shadecolour[i] = BLACK;
		else if(cptr[i]== 'L' || cptr[i]== 'l')
		    shadecolour[i] = BROWN;
		else if(cptr[i]== 'P' || cptr[i]== 'p')
		    shadecolour[i] = WHEAT;
		else if(cptr[i]== 'W' || cptr[i]== 'w')
		    shadecolour[i] = WHITE;
		else
		    okay = AJFALSE;
	    }
	    if(okay)
	    {
		colourbyconsensus = colourbyresidues = AJFALSE;
		colourbyshade = AJTRUE;
	    }
	    else
		ajFatal("Shade %S has unknown characters only BLPW allowed",
		       shade);
	}
	else
	    ajFatal("Shade Selected but invalid must be 4 "
		   "chars long %S",shade);
    }
  
  
    if(colourbyconsensus && colourbyresidues)
	colourbyconsensus = AJFALSE;

    sidentity = ajAcdGetString("cidentity");
    ssimilarity = ajAcdGetString("csimilarity");
    sother = ajAcdGetString("cother");

    if(colourbyconsensus)
    {
    
	cidentity = ajGraphCheckColour(sidentity);
	if(cidentity == -1)
	    cidentity = RED;
    
	csimilarity = ajGraphCheckColour(ssimilarity);
	if(csimilarity == -1)
	    csimilarity = GREEN;
    
    
	cother = ajGraphCheckColour(sother);
	if(cother == -1)
	    cother = BLACK;
    
    }
    else if(colourbyresidues)
	colmat = ajGraphGetBaseColour();
  
  
    /* output the options used */
    if(listoptions)
    {
	ajStrApp(&options,seqset->Name);
	ajFmtPrintAppS(&options,"%D plur=%f ",&ajtime,fplural);
    
	if(collision)
	    ajStrAppC(&options,"-collision ");
	else
	    ajStrAppC(&options,"-nocollision ");
    
	if(boxit)
	    ajStrAppC(&options,"-box ");
	else
	    ajStrAppC(&options,"-nobox ");
    
	if(boxcol)
	    ajStrAppC(&options,"-boxcol ");
	else
	    ajStrAppC(&options,"-noboxcol ");
    
	if(colourbyconsensus)
	    ajStrAppC(&options,"colbyconsensus ");
	else if (colourbyresidues)
	    ajStrAppC(&options,"colbyresidues ");
	else if(colourbyshade)
	    ajStrAppC(&options,"colbyshade ");
	else
	    ajStrAppC(&options,"nocolour ");
    
	if(alternative==2)
	    ajStrAppC(&options,"alt=2 ");
	else if(alternative==1)
	    ajStrAppC(&options,"alt=1 ");
	else if(alternative==3)
	    ajStrAppC(&options,"alt=3 ");
    }
  
  
    AJCNEW(seqcolptr, numseq);
    for(i=0;i<numseq;i++)
	AJCNEW(seqcolptr[i], ajSeqsetLen(seqset));

    AJCNEW(seqcharptr, numseq);
    AJCNEW(seqnames, numseq);
    AJCNEW(score, numseq);
    AJCNEW(previous, numseq);
    AJCNEW(seqcount, numseq);
  
    for(i=0;i<numseq;i++)
    {
	ajSeqsetToUpper(seqset);
	seqcharptr[i] =  ajSeqsetSeq (seqset, i);
	seqnames[i] = 0;
	ajStrApp(&seqnames[i],ajSeqsetName (seqset, i));
	ajStrTruncate(&seqnames[i],charlen);
	previous[i] = 0;
	seqcount[i] = 0;
    }
  
    /* user will pass the number of residues to fit a page */
    /* therefore we now need to calculate the size of the chars */
    /* based on this and get the new char width. */
    /* ten chars for the name */
    if(!data)
	ajGraphGetCharSize(&defheight,&currentheight);
  
    if(!data)
	ajGraphOpenWin(graph,-1.0-charlen,
		       (float)numres+10.0+(float)(numres/resbreak),
		       0.0, ystart+1.0);
    else
    {
	ajFmtPrintS(&fname,"prettyplot%d.dat",fcnt++);
	if(!(outf=ajFileNewOut(fname)))
	    ajFatal("Cannot open file %S",fname);
	/*ajUser("Writing to file %S",fname);*/
	ajFmtPrintF(outf,"##Graphic\n##Screen x1 %f y1 %f x2 %f y2 %f\n",
		    -1.0-(float)charlen,0.0,
		    (float)numres+10.0+(float)(numres/resbreak),
		    ystart+1.0);
    }
    
    if(!data)
	ajGraphGetOut(&fxp,&fyp,&ixlen,&iylen,&ixoff,&iyoff);
    else
    {
	fxp=fyp=0.;
	ixlen=600;
	iylen=450;
	ixoff=iyoff=0;
    }
    
    /*  ajUser("%f\n%f\n%d\n%d\n%d\n%d",fxp,fyp,ixlen,iylen,ixoff,iyoff);*/

    if(ixlen == 0.0)
    {	/* for postscript these are 0.0 ????? */
	if(portrait)
	{
	    ixlen = 768;
	    iylen = 960;
	}      
	else
	{
	    ixlen = 960;
	    iylen = 768;
	}
    }
  
    if(!data)
	ajGraphGetCharSize(&defheight,&currentheight);
    else
	currentheight = 4.440072;

    if(!data)
	ajGraphSetCharSize(((float)ixlen/((float)(numres+charlen)*
					  (currentheight+1.0)))/currentheight);
    else
	datacs = 4.440072 * ((float)ixlen/((float)(numres+charlen)*
					 (currentheight+1.0)))/currentheight;

    if(!data)
	ajGraphGetCharSize(&defheight,&currentheight);
    else
	currentheight = datacs;
    
    yincr = (currentheight +3.0)*0.3;
  
    if(!title)
	y=ystart;
    else
    {
	y=ystart-5.0;
	if(!data)
	{
	    ajGraphTextMid (((float)numres+10.0)/2.0,ystart,
			    ajStrStr(seqset->Usa));
	    ajGraphTextMid (((float)numres+10.0)/2.0,1.0,
			    ajStrStr(options));
	}
	else
	{
	    ajFmtPrintF (outf,"Text2 x1 %f y1 %f colour 0 size %f %s\n",
			 ((float)numres+10.0)/2.0,ystart,datacs,
			    ajStrStr(seqset->Usa));
	    ajFmtPrintF (outf,"Text2 x1 %f y1 %f colour 0 size %f %s\n",
			 ((float)numres+10.0)/2.0,1.0,datacs,
			    ajStrStr(options));
	}
    }

    if(!seqperpage)
    {	/* sequences per page not set then calculate it */
	seqperpage = prettyplot_calcseqperpage(yincr,y,consensus);
	if(seqperpage>numseq)
	    seqperpage=numseq;
    }
  
    count = 0;
  
  
    if(boxcol)
    {
	if(!data)
	    old = ajGraphSetFore(iboxcolval);
	else
	{
	    old = datacol;
	    datacol = iboxcolval;
	}
    }
    
    kmax = ajSeqsetLen(seqset) - 1;
    for(k=0; k<= kmax; k++)
    {
    
	/* calculate the consensus */
	/* reset score and identical */
	for(i=0;i<numseq;i++)
	    score[i] = 0.0;
	for(i=0;i<matsize;i++)
	{
	    identical[i] = 0.0;
	    matching[i] = 0.0;
	    colcheck[i] = 0.0;
	}
    
	/* generate a score for each */
	for(i=0;i<numseq;i++)
	{
	    m1 = ajSeqCvtK(cvt, seqcharptr[i][k]);
	    for(j=0;j<numseq;j++)
	    {
		m2 = ajSeqCvtK(cvt, seqcharptr[j][k]);
		if(m1 && m2)
		    score[i] += (float)matrix[m1][m2]*
			ajSeqsetWeight(seqset, j);
	    }
	    if(m1)
		identical[m1] += ajSeqsetWeight(seqset, i);
	}
    
	/* find the highest score */
	highindex = -1;
	scoremax = -3;
	if(showscore==k+1)
	    ajUser("Score--------->");
	for(i=0;i<numseq;i++)
	{
	    if(showscore==k+1)
		ajUser("%d %c %f",k+1,seqcharptr[i][k],score[i]); 
	    if(score[i] > scoremax)
	    {
		scoremax = score[i];
		highindex = i;
	    }
	}
	for(i=0;i<numseq;i++)
	{
	    m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);
	    if(!matching[m1])
	    {
		for(j=0;j<numseq;j++)
		{
		    m2 = ajSeqCvtK (cvt, seqcharptr[j][k]);
		    if(m1 && m2 && matrix[m1][m2] > 0)
			matching[m1] += ajSeqsetWeight(seqset, j);
		}
	    }
	}
    
	/* find highs for matching and identical */
	matchingmaxindex = 0;
	identicalmaxindex = 0;
	for(i=0;i<numseq;i++)
	{
	    m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);
	    if(identical[m1] > identical[identicalmaxindex])
		identicalmaxindex= m1;
	}
	for(i=0;i<numseq;i++)
	{
	    m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);
	    if(matching[m1] > matching[matchingmaxindex])
		matchingmaxindex= m1;
	    else if(matching[m1] ==  matching[matchingmaxindex])
	    {
		if(identical[m1] > identical[matchingmaxindex])
		    matchingmaxindex= m1;	  
	    }
	}
	if(showscore==k+1)
	{
	    ajUser("Identical----------->");
	    for(i=0;i<numseq;i++)
	    {
		m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);
		ajUser("%d %c %f",k+1,seqcharptr[i][k],identical[m1]); 
	    }
	    ajUser("Matching------------>");
	    for(i=0;i<numseq;i++)
	    {
		m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);
		ajUser("%d %c %f %d",k+1,seqcharptr[i][k],matching[m1],
		       m1==matchingmaxindex); 
	    }
	}
    
	con=0;
	boxindex = -1;
	max = -3;
	if(highindex != -1 &&
	   matching[ajSeqCvtK(cvt, seqcharptr[highindex][k])] >= fplural)
	{
	    con = 1;
	    boxindex = highindex;
	}
	else
	{
	    for(i=0;i<numseq;i++)
	    {
		m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);	
		if(matching[m1] > max)
		{
		    max = matching[m1];
		    highindex = i;
		}
		else if(matching[m1] == max)
		{
		    if(identical[m1] >
		       identical[ajSeqCvtK (cvt, seqcharptr[highindex][k])] )
		    {
			max = matching[m1];
			highindex = i;
		    }
		}
	    }
	    if(matching[ajSeqCvtK (cvt, seqcharptr[highindex][k])] >= fplural)
	    {
		con =1;
		boxindex = highindex;
	    }
	}
    
    
	if(con)
	{
	    if(collision)
	    {
		/* check for collisions */
	
		if(alternative == 1 )
		{
		    if(showscore==k+1)
			ajUser("before alt coll test %d ",con); 
	  
		    /* check to see if this is unique for collisions */
		    if(showscore==k+1)
			ajUser("col test  identicalmax %d %f",k+1,
			       identical[identicalmaxindex]); 
		    for(i=0;i<numseq;i++)
		    {
			m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);
			if(showscore==k+1)
			    ajUser("col test  %d %c %f %d",k+1,
				   seqcharptr[i][k],identical[m1],m1); 
			if(identical[m1] >= identical[identicalmaxindex] &&
			   m1 != identicalmaxindex)
			    con = 0;
		    }
		    if(showscore==k+1)
			ajUser("after (alt=1) coll test %d ",con); 
		}
	
		else if(alternative == 2)
		{
		    for(i=0;i<numseq;i++){
			m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);
			if(showscore==k+1)
			    ajUser("col test (alt=2) %d %c %f",k+1,
				   seqcharptr[i][k],matching[m1]); 
			if((matching[m1] >= matching[matchingmaxindex] &&
			    m1 != matchingmaxindex &&
			    matrix[m1][matchingmaxindex] < 0.1)||
			   (identical[m1] >= identical[matchingmaxindex] 
			   && m1 != matchingmaxindex))
			    con = 0;
		    }	  
		}
		else if (alternative == 3)
		{
		    /*
		     * to do this check ones NOT in concensus to see if we
		     * can get a plu of fplural
		     */
		    if(showscore==k+1)
			ajUser("before coll test %d ",con); 
		    ms = ajSeqCvtK (cvt, seqcharptr[highindex][k]);
		    for(i=0;i<numseq;i++)
		    {
			m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);	
			if(ms != m1 && colcheck[m1] == 0.0)
			    /* NOT in the current consensus */
			    for(j=0;j<numseq;j++)
			    {
				m2 = ajSeqCvtK (cvt, seqcharptr[j][k]);	
				if( matrix[ms][m2] < 0.1)
				{ /* NOT in the current consensus */
				    if( matrix[m1][m2] > 0.1)
					colcheck[m1] += ajSeqsetWeight(seqset,
								       j);
				}
			    }
		    }
		    for(i=0;i<numseq;i++)
		    {        
			m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);
			if(showscore==k+1)
			    ajUser("col test  %d %c %f",k+1,seqcharptr[i][k],
				   colcheck[m1]); 
			/* if any other matches then we have a collision */
			if(colcheck[m1] >= fplural)
			    con = 0;
		    }
		    if(showscore==k+1)
			ajUser("after coll test %d ",con);
		}
		else
		{
		    for(i=0;i<numseq;i++)
		    {
			m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);
			if(showscore==k+1)
			    ajUser("col test (alt=2) %d %c %f",k+1,
				   seqcharptr[i][k],matching[m1]); 
			if((matching[m1] >= matching[matchingmaxindex] &&
			    m1 != matchingmaxindex && 
			    matrix[m1][matchingmaxindex] < 0.1))
			    con = 0;
			if(identical[m1] >= identical[matchingmaxindex] &&
			   m1 != matchingmaxindex && 
			   matrix[m1][matchingmaxindex] > 0.1)
			    con = 0;
		    }	
		    if(!con)
		    {	/* matches failed try identicals */
			if(identical[identicalmaxindex] >= fplural)
			{
			    con = 1;
			    /*
			     * if nothing has an equal or higher match that
			     * does not match highest then false
			     */
			    for(i=0;i<numseq;i++)
			    {
				m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);
				if(identical[m1] >=
				   identical[identicalmaxindex] &&
				   m1 != identicalmaxindex)
				    con = 0;
				else if(matching[m1] >=
					matching[identicalmaxindex] &&
					matrix[m1][matchingmaxindex] <= 0.0)
				    con =0;
				else if(m1 == identicalmaxindex)
				    j = i;
			    }
			    if(con)
				highindex = j;
			}
		    }
	  
		}
	    }
      
	    if(identity)
	    {
		j=0;
		for(i=0;i<numseq;i++)
		    if(seqcharptr[highindex][k] == seqcharptr[i][k])
			j++;

		if(j<identity)
		    con=0;
	    }
	}
    
    
	/* newline start */
	if(count >= numres )
	{
	    y=y-(yincr*((float)numseq+2.0+((float)consensus*2)));
	    if(y<yincr*((float)numseq+2.0+((float)consensus*2)))
	    {
		if(!title)
		    y=ystart;
		else
		{
		    y=ystart-5.0;
		    /*ajGraphTextMid (((float)numres+10.0)/2.0,ystart,
		      ajStrStr(seqset->Usa));*/
		}
		startseq=0;
		endseq=seqperpage;
		newILstart= newILend;
		newILend=k;
		while(startseq < numseq)
		{
		    if(endseq>numseq)
			endseq=numseq;
		    prettyplot_fillinboxes(ystart,ajSeqsetLen(seqset),numseq,
					   resbreak,cvt,yincr,numres,consensus,
					   title,newILstart,newILend,boxcol,
					   boxit,startseq,endseq,outf,data,
					   datacol,datacs);
		    startseq=endseq;
		    endseq+=seqperpage;
		    if(!data)
			ajGraphNewPage(AJFALSE);
		    else
		    {
			ajFileClose(&outf);
			ajFmtPrintS(&fname,"prettyplot%d.dat",fcnt++);
			if(!(outf=ajFileNewOut(fname)))
			    ajFatal("Cannot open file %S",fname);
			/*ajUser("Writing to file %S",fname);*/
			ajFmtPrintF(outf,"##Graphic\n##Screen x1 %f y1 %f"
				    " x2 %f y2 %f\n",-1.0-(float)charlen,0.0,
				    (float)numres+10.0+(float)
				    (numres/resbreak),
				    ystart+1.0);
		    }
		}
	    }
	    
	    count = 0;
	    gapcount = 0;
	}
	
	count++;
	countforgap++;

	for(j=0;j<numseq;j++)
	{
	    /* START OF BOXES */
      
	    if(boxit)
	    {
		seqboxptr[j][k] = 0;
		if(boxindex!=-1)
		{
		    index = boxindex;
		    if(matrix[ajSeqCvtK (cvt, seqcharptr[j][k])]
		       [ajSeqCvtK (cvt, seqcharptr[index][k])] > 0)
			part = 1.0; 
		    else
		    {
			if(identical[ajSeqCvtK (cvt, seqcharptr[j][k])] >=
			   fplural)
			    part = 1.0;
			else
			    part = 0.0;
		    }	  
		    if(previous[j] != part)
			/* draw vertical line */
			seqboxptr[j][k] |= BOXLEF;

		    if(j==0)
		    {	/* special case for horizontal line */
			if(part)
			{
			    currentstate = 1;
			    /* draw hori line */
			    seqboxptr[j][k] |= BOXTOP;
			}
			else
			    currentstate = 0;
		    }
		    else 
		    {	/* j != 0  Normal case for horizontal line */
			if(part != currentstate)
			{
			    /*draw hori line */
			    seqboxptr[j][k] |= BOXTOP;
			    currentstate = part;
			}
		    }
		    if(j== numseq-1 && currentstate)
			/* draw horiline at bottom */
			seqboxptr[j][k] |= BOXBOT;

		    previous[j] = part;
		}
		else
		{
		    part = 0;
		    if(previous[j])
		    {
			/* draw vertical line */
			seqboxptr[j][k] |= BOXLEF;
		    }
		    previous[j] = 0;
		}
		if(count == numres || k == kmax || countforgap >= resbreak )
		{			/* last one on the row or a break*/
		    if(previous[j])
		    {
			/* draw vertical line */
			seqboxptr[j][k] |= BOXRIG;
		    }
		    previous[j] = 0;
		}
	
	    } /* end box */
	    if(boxcol)
		if(boxindex != -1)
		{
		    index = boxindex;
		    if(matrix[ajSeqCvtK (cvt, seqcharptr[j][k])]
		       [ajSeqCvtK (cvt, seqcharptr[index][k])] > 0
		       || identical[ajSeqCvtK (cvt, seqcharptr[j][k])] >=
		       fplural )

			seqboxptr[j][k] |= BOXCOLOURED;
		}
      
	    /* END OF BOXES */
      
      
      
      
	    if(ajSeqCvtK (cvt, seqcharptr[j][k]))
		res[0] = seqcharptr[j][k];
	    else
		res[0] = '-';
      
	    if(colourbyconsensus)
	    {
		if(con && seqcharptr[highindex][k] == seqcharptr[j][k])
		    seqcolptr[j][k] = cidentity;
		else if(part)
		    seqcolptr[j][k] = csimilarity;
		else
		    seqcolptr[j][k] = cother;
	    }
	    else if(colourbyresidues)
		seqcolptr[j][k] = colmat[ajSeqCvtK (cvt, seqcharptr[j][k])];
	    else if(con && colourbyshade)
	    {
		part = matrix[ajSeqCvtK (cvt, seqcharptr[j][k])]
		    [ajSeqCvtK (cvt, seqcharptr[highindex][k])];
		if(part >= 1.5)
		    seqcolptr[j][k] = shadecolour[0];
		else if(part >= 1.0)
		    seqcolptr[j][k] = shadecolour[1];
		else if(part >= 0.5)
		    seqcolptr[j][k] = shadecolour[2];
		else 
		    seqcolptr[j][k] = shadecolour[3];
	    }
	    else if (colourbyshade)
		seqcolptr[j][k] = shadecolour[3];
	    else
		seqcolptr[j][k] = BLACK;
	}

	if(consensus)
	{
	    if(con)
		res[0] = seqcharptr[highindex][k];
	    else
		res[0] = '-';
	    strcat(constr,res);
	}
	if(countforgap >= resbreak)
	{
	    gapcount++;
	    countforgap=0;
	}
    
    }
  

    startseq=0;
    endseq=seqperpage;
    newILstart= newILend;
    newILend=k;
    while(startseq < numseq)
    {
	if(endseq>numseq)
	    endseq=numseq;
	prettyplot_fillinboxes(ystart,ajSeqsetLen(seqset),numseq,resbreak,cvt,
			       yincr,numres,consensus,title,newILstart,
			       newILend,boxcol,boxit,startseq,endseq,
			       outf,data,datacol,datacs);
	startseq=endseq;
	endseq+=seqperpage;
	if(!data)
	    ajGraphNewPage(AJFALSE);
	else if(startseq<numseq)
	{
	    ajFileClose(&outf);
	    ajFmtPrintS(&fname,"prettyplot%d.dat",fcnt++);
	    if(!(outf=ajFileNewOut(fname)))
		ajFatal("Cannot open file %S",fname);
	    /*ajUser("Writing to file %S",fname);*/
	    ajFmtPrintF(outf,"##Graphic\n##Screen x1 %f y1 %f"
			" x2 %f y2 %f\n",-1.0-(float)charlen,0.0,
			(float)numres+10.0+(float)
			(numres/resbreak),
			ystart+1.0);
	}
    }
    
  
    if(!data)
	ajGraphGetCharSize(&defheight,&currentheight);
  
    if(boxcol)
	if(!data)
	    old = ajGraphSetFore(old);

    if(!data)
	ajGraphCloseWin();
    else
	ajFileClose(&outf);
  
  
    ajStrDel(&sidentity);
    ajStrDel(&ssimilarity);
    ajStrDel(&sother);
    ajStrDel(&options);
  
    for(i=0;i<numseq;i++)
	ajStrDel(&seqnames[i]);


    AJFREE(seqnames);
    AJFREE(score);
    AJFREE(previous);
    AJFREE(seqcount);

    AJFREE(colmat);
    if(shadecolour)
	AJFREE(shadecolour);

    AJFREE(seqcharptr);


    ajExit ();
    return 0;
}





/* @funcstatic prettyplot_calcseqperpage *************************************
**
** Undocumented.
**
** @param [?] yincr [float] Undocumented
** @param [?] y [float] Undocumented
** @param [?] consensus [AjBool] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/


static ajint prettyplot_calcseqperpage(float yincr,float y,AjBool consensus)
{
    float yep=1.0;
    ajint numallowed = 1;

    while(yep>0.0)
    {
	yep=y-(yincr*((float)numallowed+2.0+((float)consensus*2)));
	numallowed++;
    }
  
    /*  ajUser("numallowed = %d\n",numallowed-1);*/

    return numallowed-1;  
}







/* @funcstatic prettyplot_fillinboxes ****************************************
**
** Undocumented.
**
** @param [?] ystart [float] Undocumented
** @param [?] length [ajint] Undocumented
** @param [?] numseq [ajint] Undocumented
** @param [?] resbreak [ajint] Undocumented
** @param [?] cvt [AjPSeqCvt] Undocumented
** @param [?] yincr [float] Undocumented
** @param [?] numres [ajint] Undocumented
** @param [?] consensus [AjBool] Undocumented
** @param [?] title [AjBool] Undocumented
** @param [?] start [ajint] Undocumented
** @param [?] end [ajint] Undocumented
** @param [?] boxcol [AjBool] Undocumented
** @param [?] boxit [AjBool] Undocumented
** @param [?] seqstart [ajint] Undocumented
** @param [?] seqend [ajint] Undocumented
** @param [?] outf [AjPFile] Undocumented
** @param [?] data [AjBool] Undocumented
** @param [?] datacol [ajint] Undocumented
** @param [?] datacs [float] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint prettyplot_fillinboxes(float ystart, ajint length, ajint numseq,
				    ajint resbreak, AjPSeqCvt cvt,
				    float yincr, ajint numres,
				    AjBool consensus,AjBool title,
				    ajint start, ajint end,AjBool boxcol,
				    AjBool boxit, ajint seqstart,
				    ajint seqend, AjPFile outf, AjBool data,
				    ajint datacol, float datacs)
{
    ajint count = 1,gapcount=0,countforgap=0;
    ajint table[16];
    ajint i,j,k,w,old=0;
    ajint oldcol=0,l;
    float y;
    char res[2]=" ";
    /*  static ajint start =0;*/
    AjPStr strcon=0;
    char numberstring[10];
    ajint thiscol=0;
    float defcs=0.;
    float curcs=0.;

    ajDebug ("prettyplot_fillinboxes start:%d end:%d numres:%d resbreak:%d\n",
	     start, end, numres, resbreak);
    ajDebug ("prettyplot_fillinboxes boxcol:%b boxit:%b\n",
	    boxcol, boxit);

    ajStrAppC(&strcon,"Consensus");
    ajStrTruncate(&strcon,charlen);

    if(boxcol)
    {
	if(!title)
	    y=ystart;
	else
	    y = ystart-5.0;
	for(k=start; k< end; k++)
	{
	    if(countforgap >= resbreak)
	    {
		gapcount++;
		countforgap=0;
	    }
	    if(count >= numres+1 )
	    {
		y=y-(yincr*((float)numseq+2.0+((float)consensus*2)));
		if(y<yincr*((float)numseq+2.0+((float)consensus*2)))
		{
		    if(!title)
			y=ystart;
		    else
		    {
			y=ystart-5.0;
		    }
		}
		count = 1;
		gapcount = 0;
	    }
	    count++;
	    countforgap++;
	    if(!data)
		thiscol = ajGraphGetColour();
	    else
		thiscol = datacol;
	    
	    for(j=seqstart,l=0;j<seqend;j++,l++)
	    {
		if(seqboxptr[j][k] & BOXCOLOURED)
		{
		    if(!data)
			ajGraphRectFill((float)(count+gapcount-1)+1.0,
					y-(yincr*(l+0.5)),
					(float)(count+gapcount-1),
					y-(yincr*(l-0.5)));
		    else
			ajFmtPrintF(outf,"Shaded Rectangle x1 %f y1 %f x2 %f"
				    " y2 %f colour %d\n",
				    (float)(count+gapcount-1)+1.0,
				    y-(yincr*(l+0.5)),
				    (float)(count+gapcount-1),
				    y-(yincr*(l-0.5)),thiscol);

		}
	    }
	}
    }

    if(!data)
	oldcol = ajGraphSetFore(BLACK);

    /* DO THE BACKGROUND OF THE BOXES FIRST */

    count = 0;
    gapcount = countforgap = 0;
    if(!title)
	y=ystart;
    else
	y = ystart-5.0;

    if(!data)
	ajGraphGetCharSize(&defcs,&curcs);
    else
	curcs = datacs;
    

    if(shownames)
    {
	for(i=seqstart,l=0;i<seqend;i++,l++)
	{
	    if(!data)
		ajGraphTextStart (-charlen,y-(yincr*l),ajStrStr(seqnames[i]));
	    else
		ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size %f %s\n",
			    (float)-charlen,y-(yincr*l),curcs,
			    ajStrStr(seqnames[i]));
	}
	
	if(consensus && (numseq==seqend))
	{
	    if(!data)
		ajGraphTextStart (-charlen,y-(yincr*((seqend-seqstart)+1)),
				  ajStrStr(strcon));
	    else
		ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size %f %s\n",
			    (float)-charlen,y-(yincr*((seqend-seqstart)+1)),
			    curcs, ajStrStr(seqnames[i]));
	}

    }
    for(k=start; k< end; k++)
    {
	if(countforgap >= resbreak)
	{
	    gapcount++;
	    countforgap=0;
	}
	if(count >= numres )
	{
	    if(shownumbers)
	    {
		for(j=seqstart,l=0;j<seqend;j++,l++)
		{
		    sprintf(numberstring,"%d",seqcount[j]);
		    if(!data)
			ajGraphTextStart ((float)(count+numgaps)+5.0,
					  y-(yincr*l),numberstring);
		    else
			ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 "
				    "size %f %s\n", (float)(count+numgaps)+5.0,
				    y-(yincr*l),curcs,numberstring);

		}
		if(consensus && (numseq==seqend))
		{
		    sprintf(numberstring,"%d",k);
		    if(!data)
			ajGraphTextStart ((float)(count+numgaps)+5.0,
					  y-(yincr*(l+1)),
					  numberstring);
		    else
			ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 "
				    "size %f %s\n", (float)(count+numgaps)+5.0,
				    y-(yincr*(l+1)),curcs,numberstring);

		}
	    }
	    y=y-(yincr*((float)numseq+2.0+((float)consensus*2)));
	    if(y<yincr*((float)numseq+2.0+((float)consensus*2)))
	    {
		if(!title)
		    y=ystart;
		else
		{
		    y=ystart-5.0;
		    if(!data)
			ajGraphTextMid (((float)numres+10.0)/2.0,ystart,
					ajStrStr(seqset->Usa));
		    else
			ajFmtPrintF(outf,"Text2 x1 %f y1 %f colour 0"
				    " size %f %s\n",((float)numres+10.0)/2.0,
				    ystart,curcs,ajStrStr(seqset->Usa));
		}
	    }
	    
	    count = 0;
	    gapcount = 0;
	    if(shownames)
	    {
		for(i=seqstart,l=0;i<seqend;i++,l++)
		{
		    if(!data)
			ajGraphTextStart(-charlen,y-(yincr*l),
					 ajStrStr(seqnames[i]));
		    else
			ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 "
				    "size %f %s\n", (float)-charlen,
				    y-(yincr*l),curcs,ajStrStr(seqnames[i]));

		}
		
		if(consensus &&(numseq==seqend))
		{
		    if(!data)
			ajGraphTextStart (-charlen,
					  y-(yincr*((seqend-seqstart)+1)),
					  ajStrStr(strcon));
		    else
			ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 "
				    "size %f %s\n", (float)-charlen,
				    y-(yincr*((seqend-seqstart)+1)),curcs,
				    ajStrStr(seqnames[i]));

		}

	    }
	}
	count++;
	countforgap++;
	if(boxit)
	{
	    for(j=seqstart,l=0; j< seqend; j++,l++)
	    {
		if(ajSeqCvtK (cvt, seqcharptr[j][k]))
		    seqcount[j]++;
		if(seqboxptr[j][k] & BOXLEF)
		{
		    if(!data)
			ajGraphLine((float)(count+gapcount),y-
				    (yincr*((float)l+0.5)),
				    (float)(count+gapcount),
				    y-(yincr*((float)l-0.5)));
		    else
			ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f "
				    "colour 0\n",(float)(count+gapcount),y-
				    (yincr*((float)l+0.5)),
				    (float)(count+gapcount),
				    y-(yincr*((float)l-0.5)));
		}
		if(seqboxptr[j][k] & BOXTOP)
		{
		    if(!data)
			ajGraphLine((float)(count+gapcount),y-
				    (yincr*((float)l-0.5)),
				    (float)(count+gapcount)+1.0,
				    y-(yincr*((float)l-0.5)));
		    else
			ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f "
				    "colour 0\n",(float)(count+gapcount),y-
				    (yincr*((float)l-0.5)),
				    (float)(count+gapcount)+1.0,
				    y-(yincr*((float)l-0.5)));
		}
		if(seqboxptr[j][k] & BOXBOT)
		{
		    if(!data)
			ajGraphLine((float)(count+gapcount),y-
				    (yincr*((float)l+0.5)),
				    (float)(count+gapcount)+1.0,
				    y-(yincr*((float)l+0.5)));
		    else
			ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f "
				    "colour 0\n",(float)(count+gapcount),y-
				    (yincr*((float)l+0.5)),
				    (float)(count+gapcount)+1.0,
				    y-(yincr*((float)l+0.5)));
		}
		if(seqboxptr[j][k] & BOXRIG)
		{
		    if(!data)
			ajGraphLine((float)(count+gapcount)+1.0,y-
				    (yincr*((float)l+0.5)),
				    (float)(count+gapcount)+1.0,
				    y-(yincr*((float)l-0.5)));
		    else
			ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f "
				    "colour 0\n",(float)(count+gapcount)+1.0,y-
				    (yincr*((float)l+0.5)),
				    (float)(count+gapcount)+1.0,
				    y-(yincr*((float)l-0.5)));
		}
	    }
	}
	else if (shownumbers)	/* usually set in the boxit block */
	{
	  for(j=seqstart,l=0; j< seqend; j++,l++)
	  {
	    if(ajSeqCvtK (cvt, seqcharptr[j][k]))
	      seqcount[j]++;
	  }
	}
	if(consensus && (numseq==seqend))
	{
	    res[0] = constr[k];
	    if(!data)
		ajGraphTextStart ((float)(count+gapcount),
				  y-(yincr*((seqend-seqstart)+1)),res);
	    else
		ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size %f %s\n",
			    (float)(count+gapcount),
			    y-(yincr*((seqend-seqstart)+1)),curcs,res);
			    
	}
    }
    if(shownumbers)
    {
	for(j=seqstart,l=0;j<seqend;j++,l++)
	{
	    sprintf(numberstring,"%d",seqcount[j]);
	    if(!data)
		ajGraphTextStart ((float)(count+numgaps)+5.0,y-(yincr*l),
				  numberstring);
	    else
		ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size %f %s\n",
			    (float)(count+numgaps)+5.0,
			    y-(yincr*l),curcs,numberstring);

	}
	if(consensus && (numseq==seqend))
	{
	    sprintf(numberstring,"%d",k);
	    if(!data)
		ajGraphTextStart ((float)(count+numgaps)+5.0,y-(yincr*(l+1)),
				  numberstring);
	    else
		ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size %f %s\n",
			    (float)(count+numgaps)+5.0,
			    y-(yincr*(l+1)),curcs,numberstring);
	}
    }


    ajStrDel(&strcon);
    
    for(i=0;i<16;i++)
	table[i] = -1;
    for(i=0;i<numseq;i++)
    {
	for(k=0; k< length; k++)
	    table[seqcolptr[i][k]] = 1;
    }
    /* now display again but once for each colour */
  
    /*    for(w=0;w<15;w++)*/
    for(w=0;w<16;w++)
    {	/* not 16 as we can ignore white on plotters*/
	if(table[w] > 0)
	{
	    if(!data)
		old = ajGraphSetFore(w);
	    count = 0;
	    gapcount = countforgap = 0;
	    if(!title)
		y=ystart;
	    else
		y = ystart-5.0;
	    for(k=start; k< end; k++)
	    {
		if(countforgap >= resbreak)
		{
		    gapcount++;
		    countforgap=0;
		}
		if(count >= numres )
		{
		    y=y-(yincr*((float)(seqend-seqstart)+2.0+
				((float)consensus*2)));
		    count = 0;
		    gapcount = 0;
		}
		count++;
		countforgap++;
		for(j=seqstart,l=0; j< seqend; j++,l++){
		    if(seqcolptr[j][k] == w)
		    {
			if(ajSeqCvtK (cvt, seqcharptr[j][k]))
			    res[0] = seqcharptr[j][k];
			else
			    res[0] = '-';
			if(!data)
			    ajGraphTextStart((float)(count+gapcount),
					     y-(yincr*l),res);
			else
			    ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour %d "
					"size %f %s\n",(float)(count+gapcount),
					     y-(yincr*l),w,curcs,res);
		    }
		}	
	    }
	}
	if(!data)
	    old = ajGraphSetFore(old); 
    }

    if(!data)
	old = ajGraphSetFore(oldcol);
    start = end;

    return 1;
}
