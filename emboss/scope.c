/* @source scope application
**
** Convert scop classification file to embl-like format
**
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @author: Copyright (C) Jon Ison (jison@hgmp.mrc.ac.uk)
** @author: Copyright (C) Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
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
** 
** 
** 
** 
** 
** 
** Operation
** 
** scope parses the scop classification file available at URL (1).
** (1) http://scop.mrc-lmb.cam.ac.uk/scop/search.cgi?dir=lin
** The format of this file is explained at URL (2).
** (2) http://scop.mrc-lmb.cam.ac.uk/scop/parse/index.html
** The file given at URL (1) contains a single line for each domain in scop, 
** including text describing the position of the domain in the scop hierarchy.
** Note that other scop classification files, without this annotation, are 
** available at URL (2).
** 
** scope writes the scop classification to an embl-like format file. The 
** records used to describe an entry are given below.  Records (4) to (8) 
** are used to describe the position of the domain in the scop hierarchy.
**
** (1)  ID - Domain identifier code.  This is a 7-character code that uniquely
** identifies the domain in scop.  It is identical to the first 7 characters 
** of a line in the scop classification file.  The first character is always 
** 'D', the next four characters are the PDB identifier code, the fifth 
** character is the PDB chain identifier to which the domain belongs (a '.' is
** given in cases where the domain is composed of multiple chains, a '_' is 
** given where a chain identifier was not specified in the PDB file) and the 
** final character is the number of the domain in the chain (for chains 
** comprising more than one domain) or '_' (the chain comprises a single 
** domain only).
** (2)  EN - PDB identifier code.  This is the 4-character PDB identifier code
** of the PDB entry containing the domain.
** (3)  OS - Source of the protein.  It is identical to the text given after 
** 'Species' in the scop classification file.
** (4)  CL - Domain class.  It is identical to the text given after 'Class' in 
** the scop classification file.
** (5)  FO - Domain fold.  It is identical to the text given after 'Fold' in 
** the scop classification file.
** (6)  SF - Domain superfamily.  It is identical to the text given after 
** 'Superfamily' in the scop classification file.
** (7)  FA - Domain family. It is identical to the text given after 'Family' in 
** the scop classification file.
** (8)  DO - Domain name. It is identical to the text given after 'Protein' in 
** the scop classification file.
** (9)  NC - Number of chains comprising the domain (usually 1).  If the number
** of chains is greater than 1, then the domain entry will have a section  
** containing a CN and a CH record (see below) for each chain.
** (10) CN - Chain number.  The number given in brackets after this record 
** indicates the start of the data for the relevent chain.
** (11) CH - Domain definition.  The character given before CHAIN is the PDB 
** chain identifier (a '.' is given in cases where a chain identifier was not 
** specified in the scop classification file), the strings before START and 
** END give the start and end positions respectively of the domain in the PDB 
** file (a '.' is given in cases where a position was not specified).  Note 
** that the start and end positions refer to residue numbering given in the 
** original pdb file and therefore must be treated as strings.


******************************************************************************/






#include "emboss.h"






int main(int argc, char **argv)
{
    AjPRegexp rexp    = NULL;
    AjPRegexp exp2    = NULL;
    AjPStr line       = NULL;
    AjPStr str        = NULL;
    AjPStr entry      = NULL;
    AjPStr pdb        = NULL;
    AjPStr chains     = NULL;
    AjPStr numbers    = NULL;
    AjPStr text       = NULL;
    AjPStr token      = NULL;
    
    AjPStrTok handle  = NULL;
    AjPStrTok bhandle = NULL;
    
    AjPFile inf       = NULL;
    AjPFile outf      = NULL;

    char *p           = NULL;
    char c            = ' ';
    
    ajint n;
    ajint i;
    ajint from;
    ajint to;
    
    AjPScop scop      = NULL;
    
    



    /* Compile regular expressions */
    rexp  = ajRegCompC("^([^ \t\r\n]+)[ \t\n\r]+([^ \t\r\n]+)[ \t\r\n]+"
		       "([^ \t\r\n]+)[ \t\n\r]+([^ \t\n\r]+)[ \t\n\r]+");
    exp2  = ajRegCompC("^([0-9]+)([A-Za-z]+)[-]([0-9]+)");
    
    
    /* Read data from acd*/
    embInit("scope", argc, argv);
    
    inf  = ajAcdGetInfile("infile");
    outf = ajAcdGetOutfile("outfile");
    

    /* Intitialise strings */
    line    = ajStrNew();
    str     = ajStrNew();
    entry   = ajStrNew();
    pdb     = ajStrNew();
    chains  = ajStrNew();
    numbers = ajStrNew();
    text    = ajStrNew();
    token   = ajStrNew();
    




    
    /* Start of main application loop */
    while(ajFileReadLine(inf,&line))
    {
	if(!ajRegExec(rexp,line))
	{
	    ajWarn("Bad line: %S\n",line);
	    continue;
	}

	ajRegSubI(rexp,1,&entry);
	ajRegSubI(rexp,2,&pdb);
	ajRegSubI(rexp,3,&chains);
	ajRegSubI(rexp,4,&numbers);
	ajRegPost(rexp,&text);

	n = ajStrTokenCount(&chains,",");


	scop = ajXyzScopNew(n);

	ajStrToUpper(&entry);
	ajStrAssS(&scop->Entry,entry);
	ajStrToUpper(&pdb);
	ajStrAssS(&scop->Pdb,pdb);

	handle = ajStrTokenInit(chains,",");
	for(i=0;i<n;++i)
	{
	    ajStrToken(&token,&handle,NULL);
	    	    
	    p = ajStrStr(token);
	    if(sscanf(p,"%d-%d",&from,&to)==2)
	    {
		scop->Chain[i]='.';
		ajFmtPrintS(&scop->Start[i],"%d",from);
		ajFmtPrintS(&scop->End[i],"%d",to);
	    }
	    else if(sscanf(p,"%c:%d-%d",&c,&from,&to)==3)
	    {
		ajFmtPrintS(&scop->Start[i],"%d",from);
		ajFmtPrintS(&scop->End[i],"%d",to);
		scop->Chain[i]=c;
	    }
	    else if(ajStrChar(token,1)==':')
	    {
		ajStrAssC(&scop->Start[i],".");
		ajStrAssC(&scop->End[i],".");
		scop->Chain[i]=*ajStrStr(token);
	    }
	    else if(ajRegExec(exp2,token))
	    {
		ajRegSubI(exp2,1,&str);
		ajStrAss(&scop->Start[i],str);
		ajRegSubI(exp2,2,&str);
		scop->Chain[i] = *ajStrStr(str);
		ajRegSubI(exp2,3,&str);
		ajStrAss(&scop->End[i],str);
	    }
	    else if(ajStrChar(token,0)=='-')
	    {
		scop->Chain[i]='.';
		ajStrAssC(&scop->Start[i],".");
		ajStrAssC(&scop->End[i],".");
	    }
	    else
	    {
		ajFatal("Unparseable chain line [%S]\n",chains);
	    }
		
	}
	ajStrTokenClear(&handle);

	ajStrAssC(&scop->Source,"Unknown");	
	bhandle = ajStrTokenInit(text,"|\n");
	while(ajStrToken(&token,&bhandle,NULL))
	{
	    ajStrRemoveHtml(&token);
	    ajStrClean(&token);
	    p = ajStrStr(token);
	    if(ajStrPrefixC(token,"Class:"))
		ajStrAssC(&scop->Class,p+7);
	    if(ajStrPrefixC(token,"Fold:"))
		ajStrAssC(&scop->Fold,p+6);
	    if(ajStrPrefixC(token,"Superfamily:"))
		ajStrAssC(&scop->Superfamily,p+13);
	    if(ajStrPrefixC(token,"Family:"))
		ajStrAssC(&scop->Family,p+8);
	    if(ajStrPrefixC(token,"Protein:"))
		ajStrAssC(&scop->Domain,p+9);
	    if(ajStrPrefixC(token,"Species:"))
	    {
		ajStrAssC(&scop->Source,p+9);
	    }
	}
	ajStrTokenClear(&bhandle);
	
	ajXyzScopWrite(outf,scop);
	ajXyzScopDel(&scop);
    }
    /* End of main application loop */    






    /* Tidy up */
    ajRegFree(&rexp);

    ajStrDel(&line);
    ajStrDel(&str);
    ajStrDel(&entry);
    ajStrDel(&pdb);
    ajStrDel(&chains);
    ajStrDel(&numbers);
    ajStrDel(&text);
    ajStrDel(&token);

    ajFileClose(&inf);
    ajFileClose(&outf);


    /* Bye Bye */
    ajExit();
    return 0;
}
