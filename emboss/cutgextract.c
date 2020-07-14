/* @source cutgextract application
**
** Create EMBOSS codon usage files from the CUTG database
**
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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

#define CODONS 64
#define TABLE_ESTIMATE 1000




static char *cutgextract_next(AjPFile inf, AjPInt *array);
static void cutgextract_readcodons(AjPFile inf, AjPInt *count);




/* @prog cutgextract **********************************************************
**
** Extract data from CUTG.
**
** Reads all *.codon files in the input, and builds a table for each organism.
** Writes the tables to the EMBOSS_DATA/CODONS directory at the end.
**
******************************************************************************/

int main(int argc, char **argv)
{
    char *codons[]=
    {
	"TAG","TAA","TGA","GCG","GCA","GCT","GCC","TGT",
	"TGC","GAT","GAC","GAA","GAG","TTT","TTC","GGT",
	"GGG","GGA","GGC","CAT","CAC","ATA","ATT","ATC",
	"AAA","AAG","CTA","TTA","TTG","CTT","CTC","CTG",
	"ATG","AAT","AAC","CCG","CCA","CCT","CCC","CAA",
	"CAG","CGT","CGA","CGC","AGG","AGA","CGG","TCG",
	"TCA","AGT","TCT","TCC","AGC","ACG","ACT","ACA",
	"ACC","GTA","GTT","GTC","GTG","TGG","TAT","TAC"
    };

    char *aa=
	"***AAAACCDDEEFFGGGGHHIIIKKLLLLLLMNNPPPPQQRRRRRRSSSSSSTTTTVVVVWYY";

    AjPFile inf     = NULL;
    AjPFile outf    = NULL;
    char *entryname = NULL;
    AjPStr fname    = NULL;
    AjPStr line     = NULL;
    AjPInt count    = NULL;
    AjPStr key      = NULL;
    AjPStr tmpkey   = NULL;
    AjPInt value    = NULL;

    AjPTable table  = NULL;
    ajint i = 0;
    ajint j = 0;
    ajint n = 0;
    ajint x = 0;

    AjPStr *array = NULL;
    AjPCod codon  = NULL;
    ajint sum = 0;
    char c;

    AjPList flist = NULL;
    AjPStr  entry = NULL;
    AjPStr  wild  = NULL;


    embInit("cutgextract",argc,argv);


    line   = ajStrNew();
    count  = ajIntNew();
    tmpkey = ajStrNew();
    fname  = ajStrNew();


    table = ajStrTableNew(TABLE_ESTIMATE);


    flist = ajAcdGetDirlist("directory");
    wild  = ajAcdGetString("wildspec");


    while(ajListPop(flist,(void **)&entry))
    {
	if(!ajStrMatchWild(entry,wild))
	{
	    ajStrDel(&entry);
	    continue;
	}

	inf = ajFileNewIn(entry);
	if(!inf)
	    ajFatal("cannot open file %S",entry);


	while((entryname = cutgextract_next(inf,&count)))
	{
	    cutgextract_readcodons(inf,&count);

	    ajStrAssC(&tmpkey,entryname);

	    /* See if organism is already in the table */
	    value = ajTableGet(table,tmpkey);
	    if(!value)			/* Initialise */
	    {
		key = ajStrNewC(ajStrStr(tmpkey));
		ajTablePut(table,(const void *)key,(void *)count);
		count = ajIntNew();
	    }
	    else			/* Sum the values */
	    {
		for(i=0;i<CODONS;++i)
		{
		    n = ajIntGet(value,i);
		    n += ajIntGet(count,i);
		    ajIntPut(&value,i,n);
		}
	    }
	}
	ajStrDel(&entry);
	ajFileClose(&inf);
    }

    array = (AjPStr *) ajTableToarray(table,NULL);

    i = 0;
    while(array[i])
    {
	key   = array[i++];
	value = (AjPInt) array[i++];
	codon = ajCodNew();
	sum   = 0;
	for(j=0;j<CODONS;++j)
	{
	    sum += ajIntGet(value,j);
	    x = ajCodIndexC(codons[j]);
	    codon->num[x] = ajIntGet(value,j);

	    c = aa[j];
	    if(c=='*')
		codon->aa[x] = 27;
	    else
		codon->aa[x] = c-'A';

	}
	ajCodCalculateUsage(&codon,sum);


	ajFmtPrintS(&fname,"CODONS/%S.cut",key);
	ajFileDataNewWrite(fname,&outf);
	if(!outf)
	    ajFatal("Cannot open output file %S",fname);
	ajCodWrite(outf,codon);
	ajFileClose(&outf);


	ajStrDel(&key);
	ajIntDel(&value);
	ajCodDel(&codon);
    }

    ajTableFree(&table);
    ajListDel(&flist);

    ajExit();

    return 0;
}



/* @funcstatic cutgextract_next ***********************************************
**
** Undocumented.
**
** @param [?] inf [AjPFile] Undocumented
** @param [?] array [AjPInt*] Undocumented
** @return [char*] Undocumented
** @@
******************************************************************************/

static char* cutgextract_next(AjPFile inf, AjPInt *array)
{
    static AjPStr line = NULL;
    static AjPStr org  = NULL;
    AjPStrTok token    = NULL;
    ajint i;
    ajint len;
    char *p = NULL;
    char c;

    if(!line)
    {
	line = ajStrNew();
	org  = ajStrNew();
    }

    ajStrAssC(&line,"");

    while(*ajStrStr(line) != '>')
	if(!ajFileReadLine(inf,&line))
	    return NULL;

    token = ajStrTokenInit(line,"\\\n\t\r");
    for(i=0;i<6;++i)
	ajStrToken(&org,&token,"\\\n\t\r");

    ajStrTokenClear(&token);

    ajStrInsertC(&org,0,"E");

    len = ajStrLen(org);
    p   = ajStrStr(org);
    for(i=0;i<len;++i)
    {
	c = p[i];
	if(c=='/' || c==' ' || c=='.' || c=='\'')
	    p[i]='_';
    }

    if(p[strlen(p)-1]=='_')
	p[strlen(p)-1]='\0';

    return p;
}




/* @funcstatic cutgextract_readcodons *****************************************
**
** Undocumented.
**
** @param [?] inf [AjPFile] Undocumented
** @param [?] count [AjPInt*] Undocumented
** @@
******************************************************************************/

static void cutgextract_readcodons(AjPFile inf, AjPInt *count)
{
    static int cutidx[] =
    {
	42,43,46,41,45,44,26,30,31,29,27,28,48,51,47,50,
	52,49,55,56,53,54,36,38,35,37, 4, 6, 3, 5,17,18,
	16,15,57,59,60,58,24,25,34,33,39,40,20,19,11,12,
	10, 9,63,62, 8, 7,14,13,21,23,22,32,61, 1, 0, 2
    };
    static AjPStr line  = NULL;
    static AjPStr value = NULL;

    AjPStrTok token = NULL;
    ajint i;
    ajint n = 0;


    if(!line)
    {
	line  = ajStrNew();
	value = ajStrNew();
    }

    if(!ajFileReadLine(inf,&line))
	ajFatal("Premature end of file");


    token = ajStrTokenInit(line," \n\t\r");
    for(i=0;i<CODONS;++i)
    {
	ajStrToken(&value,&token," \n\t\r");
	ajStrToInt(value,&n);
	ajIntPut(count,cutidx[i],n);
    }

    ajStrTokenClear(&token);

    return;
}
