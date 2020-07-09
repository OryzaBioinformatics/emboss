/* @source whichdb application
**
** Search all databases for an entry name
**
** @author: Copyright (C) Alan Bleasby
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


/* @prog whichdb *************************************************************
**
** Find an entry in all known databases
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr  entry;
    AjPFile outf;
    AjPList dblist = NULL;
    AjPStr  name   = NULL;
    AjPStr  idqry  = NULL;
    
    AjPStr type = NULL;
    AjPStr comm = NULL;
    AjPStr rel  = NULL;
    AjBool id   = ajFalse;
    AjBool qry  = ajFalse;
    AjBool all  = ajFalse;
    AjBool pro  = ajFalse;
    AjPSeq seq  = NULL;
    AjBool get  = ajFalse;
    AjPStr lnam = NULL;
    AjPStr snam = NULL;
    
    AjPSeqout seqout = NULL;
    
    embInit ("whichdb", argc, argv);

    entry = ajAcdGetString("entry");
    outf  = ajAcdGetOutfile("outfile");
    get   = ajAcdGetBool("get");

    if(!ajStrLen(entry))
    {
	ajExit();
	return 0;
    }
    
    dblist = ajListNew();
    type   = ajStrNew();
    comm   = ajStrNew();
    rel    = ajStrNew();
    idqry  = ajStrNew();
    seq    = ajSeqNew();
    snam   = ajStrNew();
    
    ajNamListListDatabases(dblist);

    AjErrorLevel.error = ajFalse;
    
    while(ajListPop(dblist,(void **)&lnam))
    {
	ajStrAssS(&name,lnam);
	ajStrDel(&lnam);
	
	if(!ajNamDbDetails(name,&type,&id,&qry,&all,&comm,&rel))
	    continue;
	if(!id)
	    continue;

	ajFmtPrintS(&idqry,"%S:%S",name,entry);
	if(ajStrPrefixC(type,"P"))
	    pro = ajTrue;
	else
	    pro = ajFalse;
	if(!ajSeqGetFromUsa(idqry,pro,&seq))
	    continue;

	if(get)
	{
	    ajFmtPrintS(&snam,"%S.%S",entry,name);
	    seqout = ajSeqoutNew();
	    if(!ajSeqFileNewOut(seqout,snam))
		ajFatal("Cannot open output file [%S]",snam);
	    ajSeqOutSetFormatC(seqout,"FASTA");
	    ajUser("Writing %S",snam);
	    ajSeqWrite(seqout,seq);
	    ajSeqWriteClose(seqout);
	    ajSeqoutDel(&seqout);
	}
	else
	    ajFmtPrintF(outf,"%S\n",idqry);
    }
    




    ajListDel(&dblist);
    ajStrDel(&type);
    ajStrDel(&comm);
    ajStrDel(&rel);
    ajStrDel(&idqry);
    ajStrDel(&snam);
    ajSeqDel(&seq);

    ajExit ();
    return 0;
}
