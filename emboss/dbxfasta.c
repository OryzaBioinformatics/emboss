/* @source dbxfasta application
**
** Index flatfile databases
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


#define FASTATYPE_SIMPLE    1
#define FASTATYPE_IDACC     2
#define FASTATYPE_GCGID     3
#define FASTATYPE_GCGIDACC 4
#define FASTATYPE_NCBI      5
#define FASTATYPE_DBID      6
#define FASTATYPE_ACCID     7
#define FASTATYPE_GCGACCID  8



static AjBool dbxfasta_NextEntry(EmbPBtreeEntry entry, AjPFile inf,
				 AjPRegexp typeexp, ajint idtype);
static AjBool dbxfasta_ParseFasta(EmbPBtreeEntry entry, AjPRegexp typeexp,
				  ajint idtype, const AjPStr line);
static AjPRegexp dbxfasta_getExpr(const AjPStr idformat, ajint *type);


/* @prog dbxfasta ************************************************************
**
** Index a flat file database
**
******************************************************************************/

int main(int argc, char **argv)
{
    EmbPBtreeEntry entry = NULL;
    
    AjPStr dbname   = NULL;
    AjPStr dbrs     = NULL;
    AjPStr release  = NULL;
    AjPStr datestr  = NULL;

    AjPStr directory;
    AjPStr indexdir;
    AjPStr filename;
    AjPStr exclude;
    AjPStr dbtype = NULL;

    AjPStr *fieldarray = NULL;
    
    ajint nfields;
    ajint nfiles;

    AjPStr tmpstr = NULL;
    AjPStr thysfile = NULL;
    
    ajint i;
    AjPFile inf = NULL;

    AjPStr word = NULL;
    
    AjPBtId  idobj  = NULL;
    AjPBtPri priobj = NULL;
    

    AjPRegexp typeexp = NULL;
    ajint idtype = 0;



    embInit("dbxfasta", argc, argv);

    dbtype     = ajAcdGetListI("idformat",1);
    fieldarray = ajAcdGetList("fields");
    directory  = ajAcdGetDirectoryName("directory");
    indexdir   = ajAcdGetOutdirName("indexoutdir");
    filename   = ajAcdGetString("filenames");
    exclude    = ajAcdGetString("exclude");
    dbname     = ajAcdGetString("dbname");
    dbrs       = ajAcdGetString("dbresource");
    release    = ajAcdGetString("release");
    datestr    = ajAcdGetString("date");

    entry = embBtreeEntryNew();
    tmpstr = ajStrNew();
    
    idobj   = ajBtreeIdNew();
    priobj  = ajBtreePriNew();


    nfields = embBtreeSetFields(entry,fieldarray);
    embBtreeSetDbInfo(entry,dbname,dbrs,datestr,release,dbtype,directory,
		      indexdir);

    embBtreeGetRsInfo(entry);

    nfiles = embBtreeGetFiles(entry,directory,filename,exclude);
    embBtreeWriteEntryFile(entry);

    embBtreeOpenCaches(entry);


    typeexp = dbxfasta_getExpr(dbtype,&idtype);


    for(i=0;i<nfiles;++i)
    {
	ajListPop(entry->files,(void **)&thysfile);
	ajListPushApp(entry->files,(void *)thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	printf("Processing file %s\n",MAJSTRSTR(tmpstr));
	if(!(inf=ajFileNewIn(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);
	

	while(dbxfasta_NextEntry(entry,inf,typeexp,idtype))
	{
	    if(entry->do_id)
	    {
		ajStrToLower(&entry->id);
		ajStrAssS(&idobj->id,entry->id);
		idobj->dbno = i;
		idobj->offset = entry->fpos;
		idobj->dups = 0;
		ajBtreeInsertId(entry->idcache,idobj);
	    }

	    if(entry->do_accession)
                while(ajListPop(entry->ac,(void **)&word))
                {
		    ajStrToLower(&word);
                    ajStrAssS(&idobj->id,word);
                    idobj->dbno = i;
		    idobj->offset = entry->fpos;
		    idobj->dups = 0;
		    ajBtreeInsertId(entry->accache,idobj);
		    ajStrDel(&word);
                }

	    if(entry->do_sv)
                while(ajListPop(entry->sv,(void **)&word))
                {
		    ajStrToLower(&word);
                    ajStrAssS(&idobj->id,word);
                    idobj->dbno = i;
		    idobj->offset = entry->fpos;
		    idobj->dups = 0;
		    ajBtreeInsertId(entry->svcache,idobj);
		    ajStrDel(&word);
                }

	    if(entry->do_description)
                while(ajListPop(entry->de,(void **)&word))
                {
		    ajStrToLower(&word);
		    ajStrAssS(&priobj->id,entry->id);
                    ajStrAssS(&priobj->keyword,word);
                    priobj->treeblock = 0;
                    ajBtreeInsertKeyword(entry->decache, priobj);
		    ajStrDel(&word);
                }
	}
	
	ajFileClose(&inf);
    }
    

    embBtreeDumpParameters(entry);
    embBtreeCloseCaches(entry);
    

    embBtreeEntryDel(&entry);
    ajStrDel(&tmpstr);
    

    nfields = 0;
    while(fieldarray[nfields])
	ajStrDel(&fieldarray[nfields++]);
    AJFREE(fieldarray);


    ajBtreeIdDel(&idobj);
    ajBtreePriDel(&priobj);

    ajExit();

    return 0;
}




/* @funcstatic dbxfasta_NextEntry ********************************************
**
** Parse the next entry from a fasta file
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [u] inf [AjPFile] file object ptr
** @param [u] typeexp [AjPRegexp] regexp corresponding to idtype
** @param [r] idtype [ajint] the kind of parsing required
**
** @return [AjBool] ajTrue on success, ajFalse if EOF
** @@
******************************************************************************/

static AjBool dbxfasta_NextEntry(EmbPBtreeEntry entry, AjPFile inf,
				 AjPRegexp typeexp, ajint idtype)
{
    static AjBool init = AJFALSE;
    static AjPStr line = NULL;
    
    if(!init)
    {
        line = ajStrNew();
        init = ajTrue;
    }

    ajStrAssC(&line,"");

    while(*MAJSTRSTR(line) != '>')
    {
	entry->fpos = ajFileTell(inf);
	if(!ajFileReadLine(inf,&line))
	    return ajFalse;
    }


    dbxfasta_ParseFasta(entry, typeexp, idtype, line);

    return ajTrue;
}




/* @funcstatic dbxfasta_getExpr ***********************************************
**
** Compile regular expression
**
** @param [r] idformat [const AjPStr] type of ID line
** @param [w] type [ajint *] numeric type
** @return [AjPRegexp] ajTrue on success.
** @@
******************************************************************************/

static AjPRegexp dbxfasta_getExpr(const AjPStr idformat, ajint *type)
{
    AjPRegexp exp = NULL;

    if(ajStrMatchC(idformat,"simple"))
    {
	*type = FASTATYPE_SIMPLE;
	exp   = ajRegCompC("^>([.A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"idacc"))
    {
	*type = FASTATYPE_IDACC;
	exp   = ajRegCompC("^>([.A-Za-z0-9_-]+)+[ \t]+([A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"accid"))
    {
	*type = FASTATYPE_ACCID;
	exp   = ajRegCompC("^>([A-Za-z0-9_-]+)+[ \t]+([A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgid"))
    {
	*type = FASTATYPE_GCGID;
	exp   = ajRegCompC("^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgidacc"))
    {
	*type = FASTATYPE_GCGIDACC;
	exp   = ajRegCompC(
		     "^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgaccid"))
    {
	*type = FASTATYPE_GCGACCID;
	exp   = ajRegCompC(
		     "^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"ncbi"))
    {
	exp   = ajRegCompC("^>([A-Za-z0-9_-]+)"); /* dummy regexp */
	*type = FASTATYPE_NCBI;
    }
    else if(ajStrMatchC(idformat,"dbid"))
    {
	exp   = ajRegCompC("^>[A-Za-z0-9_-]+[ \t]+([A-Za-z0-9_-]+)");
	*type = FASTATYPE_DBID;
    }
    else
	return NULL;

    return exp;
}




/* @funcstatic dbxfasta_ParseFasta ********************************************
**
** Parse the ID, accession from a FASTA format sequence
**
** @param [u] entry [EmbPBtreeEntry] entry object ptr
** @param [u] typeexp [AjPRegexp] regular expression
** @param [r] idtype [ajint] type of id line
** @param [r] line [const AjPStr] fasta '>' line
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxfasta_ParseFasta(EmbPBtreeEntry entry, AjPRegexp typeexp,
				  ajint idtype, const AjPStr line)
{
    static AjPRegexp wrdexp = NULL;
    static AjPStr ac  = NULL;
    static AjPStr sv  = NULL;
    static AjPStr gi  = NULL;
    static AjPStr de  = NULL;

    static AjPStr tmpfd  = NULL;

    AjPStr str = NULL;
    


    if(!wrdexp)
	wrdexp = ajRegCompC("([A-Za-z0-9]+)");


    if(!ajRegExec(typeexp,line))
    {
	ajStrDelReuse(&ac);
	ajDebug("Invalid ID line [%S]",line);
	return ajFalse;
    }

    /*
    ** each case needs to set id, ac, sv, de
    ** using empty values if they are not found
    */
    
    ajStrAssC(&sv, "");
    ajStrAssC(&gi, "");
    ajStrAssC(&de, "");
    ajStrAssC(&ac, "");
    ajStrAssC(&entry->id, "");

    switch(idtype)
    {
    case FASTATYPE_SIMPLE:
	ajRegSubI(typeexp,1,&entry->id);
	ajStrAssS(&ac,entry->id);
	ajRegPost(typeexp, &de);
	break;
    case FASTATYPE_DBID:
	ajRegSubI(typeexp,1,&entry->id);
	ajStrAssS(&ac,entry->id);
	ajRegPost(typeexp, &de);
	break;
    case FASTATYPE_GCGID:
	ajRegSubI(typeexp,1,&entry->id);
	ajStrAssS(&ac,entry->id);
	ajRegPost(typeexp, &de);
	break;
    case FASTATYPE_NCBI:
	if(!ajSeqParseNcbi(line,&entry->id,&ac,&sv,&gi,
			   &de))
	    return ajFalse;
	break;
    case FASTATYPE_GCGIDACC:
	ajRegSubI(typeexp,1,&entry->id);
	ajRegSubI(typeexp,2,&ac);
	ajRegPost(typeexp, &de);
	break;
    case FASTATYPE_GCGACCID:
	ajRegSubI(typeexp,1,&ac);
	ajRegSubI(typeexp,2,&entry->id);
	ajRegPost(typeexp, &de);
	break;
    case FASTATYPE_IDACC:
	ajRegSubI(typeexp,1,&entry->id);
	ajRegSubI(typeexp,2,&ac);
	ajRegPost(typeexp, &de);
	break;
    case FASTATYPE_ACCID:
	ajRegSubI(typeexp,1,&ac);
	ajRegSubI(typeexp,2,&entry->id);
	ajRegPost(typeexp, &de);
	break;
    default:
	return ajFalse;
    }

    ajStrToLower(&entry->id);

    if(entry->do_accession && ajStrLen(ac))
    {
	str = ajStrNew();
	ajStrAssS(&str,ac);
	ajListPush(entry->ac,(void *)str);
    }

    if(ajStrLen(gi))
	ajStrAssS(&sv,gi);

    if(entry->do_sv && ajStrLen(sv))
    {
	str = ajStrNew();
	ajStrAssS(&str,sv);
	ajListPush(entry->ac,(void *)str);
    }
    
    if(entry->do_description && ajStrLen(de))
	while(ajRegExec(wrdexp,de))
	{
	    ajRegSubI(wrdexp, 1, &tmpfd);
	    str = ajStrNew();
	    ajStrAssS(&str,tmpfd);
	    ajListPush(entry->de,(void *)str);
	    ajRegPost(wrdexp, &de);
	}


    ajStrDelReuse(&ac);
    ajStrDelReuse(&sv);
    ajStrDelReuse(&gi);
    ajStrDelReuse(&de);

    return ajTrue;
}
