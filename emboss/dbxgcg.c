/* @source dbxgcg application
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




#define GCGTYPE_OTHER 0
#define GCGTYPE_ID 1
#define GCGTYPE_ACC 2
#define GCGTYPE_DES 3
#define GCGTYPE_KEY 4
#define GCGTYPE_TAX 5
#define GCGTYPE_VER 6





static AjBool dbxgcg_ParseEmbl(EmbPBtreeEntry entry, AjPFile infr,
			       AjPStr *reflibstr);
static AjBool dbxgcg_ParseGenbank(EmbPBtreeEntry entry, AjPFile infr,
				  AjPStr *reflibstr);
static AjBool dbxgcg_ParsePir(EmbPBtreeEntry entry, AjPFile infr,
			      AjPStr *reflibstr);

static AjBool dbxgcg_NextEntry(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype);

static ajlong dbxgcg_gcggetent(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype);
static ajlong dbxgcg_pirgetent(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype);

static ajlong dbxgcg_gcgappent(AjPFile infr, AjPFile infs,
			       AjPRegexp rexp, AjPRegexp sexp,
			       AjPStr* libstr);





/* @datastatic DbxgcgPParser *************************************************
**
** Parser definition structure
**
** @alias DbxgcgSParser
** @alias DbxgcgOParser
**
** @attr Name [char*] Parser name
** @attr GcgType [AjBool] Gcg type parser if true, PIR type if false
** @attr Parser [(AjBool*)] Parser function
** @@
******************************************************************************/

typedef struct DbxgcgSParser
{
    char* Name;
    AjBool GcgType;
    AjBool (*Parser) (EmbPBtreeEntry entry, AjPFile infr, AjPStr *reflibstr);
} DbxgcgOParser;
#define DbxgcgPParser DbxgcgOParser*




static DbxgcgOParser parser[] =
{
    {"EMBL", AJTRUE, dbxgcg_ParseEmbl},
    {"SWISS", AJTRUE, dbxgcg_ParseEmbl},
    {"GENBANK", AJTRUE, dbxgcg_ParseGenbank},
    {"PIR", AJFALSE, dbxgcg_ParsePir},
    {NULL, 0, NULL}
};





/* @prog dbxgcg **************************************************************
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
    AjPStr refname = NULL;
    AjPStr seqname = NULL;
    AjPStr thysfile = NULL;
    
    ajint i;
    AjPFile infs = NULL;
    AjPFile infr = NULL;

    AjPStr word = NULL;
    
    AjPBtId  idobj  = NULL;
    AjPBtPri priobj = NULL;
    

    embInit("dbxgcg", argc, argv);

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
    for(i=0; i<nfiles; ++i)
    {
	ajListPop(entry->files,(void **) &seqname);
	refname = ajStrNew();
	ajStrAssS(&refname,seqname);
	ajFileNameExtC(&seqname,"seq");
	ajFileNameExtC(&refname,"ref");
	ajListPushApp(entry->files,(void *)seqname);
	ajListPushApp(entry->reffiles,(void *)refname);
    }
    

    embBtreeWriteEntryFile(entry);

    embBtreeOpenCaches(entry);



    for(i=0;i<nfiles;++i)
    {
	ajListPop(entry->files,(void **)&thysfile);
	ajListPushApp(entry->files,(void *)thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	printf("Processing file %s\n",MAJSTRSTR(tmpstr));
	if(!(infs=ajFileNewIn(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);

	ajListPop(entry->reffiles,(void **)&thysfile);
	ajListPushApp(entry->files,(void *)thysfile);
	ajFmtPrintS(&tmpstr,"%S%S",entry->directory,thysfile);
	if(!(infr=ajFileNewIn(tmpstr)))
	    ajFatal("Cannot open input file %S\n",tmpstr);
	

	while(dbxgcg_NextEntry(entry,infs,infr,dbtype))
	{
	    if(entry->do_id)
	    {
		ajStrToLower(&entry->id);
		ajStrAssS(&idobj->id,entry->id);
		idobj->dbno = i;
		idobj->offset = entry->fpos;
		idobj->refoffset = entry->reffpos;
		idobj->dups = 0;
		ajBtreeInsertId(entry->idcache,idobj);
	    }

	    if(entry->do_accession)
	    {
                while(ajListPop(entry->ac,(void **)&word))
                {
		    ajStrToLower(&word);
                    ajStrAssS(&idobj->id,word);
                    idobj->dbno = i;
		    idobj->offset = entry->fpos;
		    idobj->refoffset = entry->reffpos;
		    idobj->dups = 0;
		    ajBtreeInsertId(entry->accache,idobj);
		    ajStrDel(&word);
                }
	    }

	    if(entry->do_sv)
	    {
                while(ajListPop(entry->sv,(void **)&word))
                {
		    ajStrToLower(&word);
                    ajStrAssS(&idobj->id,word);
                    idobj->dbno = i;
		    idobj->offset = entry->fpos;
		    idobj->refoffset = entry->reffpos;
		    idobj->dups = 0;
		    ajBtreeInsertId(entry->svcache,idobj);
		    ajStrDel(&word);
                }
	    }

	    if(entry->do_keyword)
	    {
                while(ajListPop(entry->kw,(void **)&word))
                {
		    ajStrToLower(&word);
		    ajStrAssS(&priobj->id,entry->id);
                    ajStrAssS(&priobj->keyword,word);
                    priobj->treeblock = 0;
                    ajBtreeInsertKeyword(entry->kwcache, priobj);
		    ajStrDel(&word);
                }
	    }

	    if(entry->do_description)
	    {
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

	    if(entry->do_taxonomy)
	    {
                while(ajListPop(entry->tx,(void **)&word))
                {
		    ajStrToLower(&word);
		    ajStrAssS(&priobj->id,entry->id);
                    ajStrAssS(&priobj->keyword,word);
                    priobj->treeblock = 0;
                    ajBtreeInsertKeyword(entry->txcache, priobj);
		    ajStrDel(&word);
                }
	    }
	}
	
	ajFileClose(&infs);
	ajFileClose(&infr);
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




/* @funcstatic dbxgcg_NextEntry ***********************************************
**
** Returns next database entry as an EmbPEntry object
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infs [AjPFile] sequence file
** @param [u] infr [AjPFile] reference file
** @param [r] dbtype [const AjPStr] Id format in GCG file
** @return [AjBool] ajTrue if successful read
** @@
******************************************************************************/

static AjBool dbxgcg_NextEntry(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype)
{
    static AjPStr tmpstr = NULL;
    char *p;

    entry->reffpos = ajFileTell(infr);
    entry->fpos    = ajFileTell(infs);

    if(!dbxgcg_gcggetent(entry, infs, infr, dbtype) &&
       !dbxgcg_pirgetent(entry, infs, infr, dbtype))
	return ajFalse;

    ajDebug("id '%S' seqfpos:%d reffpos:%d\n",
	    entry->id, entry->fpos, entry->reffpos);

    ajStrAssC(&tmpstr,ajStrStr(entry->id));

    if(ajStrSuffixC(entry->id,"_0") ||
       ajStrSuffixC(entry->id,"_00") ||
       ajStrSuffixC(entry->id,"_000"))
    {
	p  = strrchr(ajStrStr(tmpstr),'_');
	*p = '\0';
	ajStrAssC(&entry->id,ajStrStr(tmpstr));
    }


    return ajTrue;
}




/* @funcstatic dbxgcg_gcggetent ***********************************************
**
** get a single entry from the GCG database files
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infs [AjPFile] sequence file
** @param [u] infr [AjPFile] reference file
** @param [r] dbtype [const AjPStr] Id format in GCG file
** @return [ajlong] Sequence length
** @@
******************************************************************************/

static ajlong dbxgcg_gcggetent(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype)
{
    static AjPStr gcgtype   = NULL;
    static ajlong gcglen;
    static AjPStr gcgdate   = NULL;
    ajlong rblock;
    static AjPStr reflibstr = NULL;
    ajint i;
    static AjPStr tmpstr  = NULL;
    static ajint called   = 0;
    static ajint iparser  = -1;
    static AjPRegexp rexp = NULL;
    static AjPRegexp sexp = NULL;
    static AjPStr rline = NULL;
    static AjPStr sline = NULL;

    if(!called)
    {
	for(i=0; parser[i].Name; i++)
	    if(ajStrMatchC(dbtype, parser[i].Name))
	    {
		iparser = i;
		break;
	    }

	if(iparser < 0)
	    ajFatal("dbtype '%S' unknown", dbtype);

	ajDebug("dbtype '%S' Parser %d\n", dbtype, iparser);
	called = 1;
    }

    if(!parser[iparser].GcgType)
	return 0;

    if(!rexp)
	rexp = ajRegCompC("^>>>>([^ \t\n]+)");

    if(!sexp)
	sexp = ajRegCompC("^>>>>([^ \t]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)"
			  "[ \t]+([^ \t]+)[ \t]+([0-9]+)");

    ajStrAssC(&sline, "");

    /* check for seqid first line */
    while(ajStrChar(sline,0)!='>')
    {
	if(!ajFileGets(infs, &sline))
	    return 0;			/* end of file */

	ajDebug("... read until next seq %Ld '%S'\n",
		ajFileTell(infs), sline);
    }

    ajDebug("dbxgcg_gcggetent .seq (%S) %Ld '%S'\n",
	    dbtype, ajFileTell(infs), sline);

    /* get the encoding/sequence length info */
    if(!ajRegExec(sexp, sline))
    {
        ajDebug("dbxgcg_gcggetent sequence expression FAILED\n");
	return 0;
    }

    ajRegSubI(sexp, 1, &entry->id);		/* Entry ID returned */

    ajRegSubI(sexp, 2, &gcgdate);
    ajRegSubI(sexp, 3, &gcgtype);
    ajRegSubI(sexp, 5, &tmpstr);
    ajStrToLong(tmpstr, &gcglen);

    ajDebug("new entry '%S' date:'%S' type:'%S' len:'%S'=%Ld\n",
	    entry->id, gcgdate, gcgtype, tmpstr, gcglen);

    ajStrAssC(&rline, "");

    ajDebug("dbxgcg_gcggetent .ref (%S) %Ld '%S'\n",
	    dbtype, ajFileTell(infr), rline);

    /* check for refid first line */
    while(ajStrChar(rline,0)!='>')
    {
	if(!ajFileGets(infr, &rline))
	{
	    ajErr("ref ended before seq");
	    break;			/* end of file */
	}
	ajDebug("... read until next ref %Ld '%S'\n", ajFileTell(infr), rline);
    }

    /* get the encoding/sequence length info */

    ajRegExec(rexp, rline);
    ajRegSubI(rexp, 1, &reflibstr);

    parser[iparser].Parser(entry, infr, &reflibstr);/* writes alistfile data */

    /* get the description line */
    ajFileGets(infs, &sline);

    /* seek to the end of the sequence; +1 to jump over newline */
    if(ajStrChar(gcgtype,0)=='2')
    {
	rblock = (gcglen+3)/4;
	ajFileSeek(infs,rblock+1,SEEK_CUR);
    }
    else
	ajFileSeek(infs,gcglen+1,SEEK_CUR);

    /*
    **  for big entries, need to append until we have all the parts.
    **  They are named with _0 on the first part, _1 on the second and so on.
    **  or _00 on the first part, _01 on the second and so on.
    **  We can look for the "id_" prefix.
    */

    if(!ajStrSuffixC(entry->id, "_0") &&
       !ajStrSuffixC(entry->id,"_00") &&
       !ajStrSuffixC(entry->id,"_000"))
	return gcglen;

    gcglen += dbxgcg_gcgappent(infr, infs, rexp, sexp,
			       &entry->id);

    return gcglen;
}




/* @funcstatic dbxgcg_pirgetent ***********************************************
**
** get a single entry from the PIR database files
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infs [AjPFile] sequence file
** @param [u] infr [AjPFile] reference file
** @param [r] dbtype [const AjPStr] Id format in GCG file
** @return [ajlong] Sequence length
** @@
******************************************************************************/

static ajlong dbxgcg_pirgetent(EmbPBtreeEntry entry, AjPFile infs,
			       AjPFile infr, const AjPStr dbtype)
{
    static AjPStr reflibstr = NULL;
    ajint i;
    static ajint called  = 0;
    static ajint iparser = -1;
    static AjPRegexp pirexp = NULL;
    ajlong gcglen;
    static AjPStr rline = NULL;
    static AjPStr sline = NULL;
    ajlong spos = 0;

    if(!called)
    {
	for(i=0; parser[i].Name; i++)
	    if(ajStrMatchC(dbtype, parser[i].Name))
	    {
		iparser = i;
		break;
	    }

	if(iparser < 0)
	    ajFatal("dbtype '%S' unknown", dbtype);
	ajDebug("dbtype '%S' Parser %d\n", dbtype, iparser);
	called = 1;
    }

    if(parser[iparser].GcgType)
	return 0;

    if(!pirexp)
	pirexp = ajRegCompC("^>..;([^ \t\n]+)");

    ajStrAssC(&sline, "");
    ajStrAssC(&rline, "");

    /* skip to seqid first line */
    while(ajStrChar(sline,0)!='>')
	if(!ajFileGets(infs, &sline))
	    return 0;			/* end of file */

    ajDebug("dbxgcg_pirgetent .seq (%S) %Ld '%S' \n",
	    dbtype, ajFileTell(infs), sline);

    ajRegExec(pirexp, sline);

    /* skip to refid first line */
    while(ajStrChar(rline,0)!='>')
	if(!ajFileGets(infr, &rline))
	{
	    ajErr("ref ended before seq"); /* end of file */
	    break;
	}

    /* get the encoding/sequence length info */

    ajRegExec(pirexp, rline);
    ajRegSubI(pirexp, 1, &reflibstr);
    ajRegSubI(pirexp, 1, &entry->id);

    ajDebug("dbigcg_pirgetent seqid '%S' spos: %Ld\n",
	    entry->id, ajFileTell(infs));
    ajDebug("dbxgcg_pirgetent refid '%S' spos: %Ld\n",
	    entry->id, ajFileTell(infr));

    parser[iparser].Parser(entry, infr, &reflibstr);/* writes alistfile data */

    /* get the description line */
    ajFileGets(infs, &sline);
    gcglen = 0;

    /* seek to the end of the sequence; +1 to jump over newline */
    while(ajStrChar(sline,0)!='>')
    {
	spos = ajFileTell(infs);
	if(!ajFileGets(infs, &sline))
	{
	    spos = 0;
	    break;
	}
	gcglen += ajStrLen(sline);
    }

    if(spos)
	ajFileSeek(infs, spos, 0);

    ajDebug("dbxgcg_pirgetent end spos %Ld line '%S'\n", spos, sline);

    return gcglen;
}




/* @funcstatic dbxgcg_gcgappent ***********************************************
**
** Go to end of a split GCG entry
**
** @param [u] infr [AjPFile] Reference file
** @param [u] infs [AjPFile] Sequence file
** @param [u] rexp [AjPRegexp] Regular expression to find ID in ref file
** @param [u] sexp [AjPRegexp] Regular expression to find ID in seq file
** @param [w] libstr [AjPStr*] ID
** @return [ajlong] Sequence length for this section
** @@
******************************************************************************/

static ajlong dbxgcg_gcgappent(AjPFile infr, AjPFile infs,
			       AjPRegexp rexp, AjPRegexp sexp,
			       AjPStr* libstr)
{
    static AjPStr reflibstr = NULL;
    static AjPStr seqlibstr = NULL;
    static AjPStr testlibstr = NULL;
    ajint ilen;
    static AjPStr tmpstr = NULL;
    static AjPStr rline  = NULL;
    static AjPStr sline  = NULL;

    AjBool isend;
    const char *p;
    char *q;
    ajlong rpos;
    ajlong spos;

    /*
    ** keep reading until the end of entry is reached
    ** and return the extra number of bases
    */

    if(!testlibstr)
	testlibstr = ajStrNew();

    ajStrAssS(&tmpstr,*libstr);

    ajDebug("dbi_gcgappent '%S'\n", tmpstr);

    p = ajStrStr(tmpstr);
    q = strrchr(p,'_');
    *q = '\0';


    ajFmtPrintS(&testlibstr, "%s_",p);
    ilen = ajStrLen(testlibstr);

    isend = ajFalse;

    while(!isend)
    {
        spos = ajFileTell(infs);
	ajFileGets(infs,&sline);
	while(strncmp(ajStrStr(sline),">>>>",4))
	{
	    spos = ajFileTell(infs);
	    if(!ajFileGets(infs, &sline))
	    {
		ajDebug("end of file on seq\n");
		return 1L;
	    }
	}

	ajRegExec(sexp, sline);
	ajRegSubI(sexp, 1, &seqlibstr);

	rpos = ajFileTell(infr);
	ajFileGets(infr, &rline);

	while(ajStrChar(rline,0)!='>')
	{
	    rpos = ajFileTell(infr);
	    if(!ajFileGets(infr, &rline))
	    {
		ajDebug("ref ended before seq\n");
		ajErr("ref ended before seq\n");
		break;
	    }
	}

	ajRegExec(rexp, rline);
	ajRegSubI(rexp, 1, &reflibstr);

	if(ajStrNCmpO(reflibstr, testlibstr, ilen) ||
	   ajStrNCmpO(seqlibstr, testlibstr, ilen))
	    isend = ajTrue;

	ajDebug("gcgappent %B test: '%S' seq: '%S' ref: '%S'\n",
		isend, testlibstr, seqlibstr, reflibstr);
    }

    ajDebug("gcgappent done at seq: '%S' ref: '%S'\n", seqlibstr, reflibstr);

    ajStrAssC(libstr,p);

    ajFileSeek(infr, rpos, 0);
    ajFileSeek(infs, spos, 0);

    return 1L;
}




/* @funcstatic dbxgcg_ParseEmbl ***********************************************
**
** Parse the ID, accession from an EMBL or SWISSPROT entry
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infr [AjPFile] reference file
** @param [w] id [AjPStr*] ID
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxgcg_ParseEmbl(EmbPBtreeEntry entry, AjPFile infr,
			       AjPStr *id)
{
    static AjPRegexp typexp = NULL;
    static AjPRegexp idexp  = NULL;
    static AjPRegexp verexp = NULL;
    static AjPRegexp wrdexp = NULL;
    static AjPRegexp phrexp = NULL;
    static AjPRegexp taxexp = NULL;
    static AjPStr tmpstr  = NULL;
    static AjPStr tmpline = NULL;
    static AjPStr tmpfd   = NULL;
    static AjPStr typStr  = NULL;
    AjPStr tmpacnum = NULL;
    ajint lineType;
    ajlong rpos;
    static AjPStr rline = NULL;

    AjPStr str = NULL;
    

    if(!typexp)
	typexp = ajRegCompC("^([A-Z][A-Z]) +");

    if(!wrdexp)
	wrdexp = ajRegCompC("([A-Za-z0-9_]+)");

    if(!verexp)
	verexp = ajRegCompC("([A-Za-z0-9]+[.][0-9]+)");

    if(!phrexp)
	phrexp = ajRegCompC(" *([^;.\n\r]+)");

    if(!taxexp)
	taxexp = ajRegCompC(" *([^;.\n\r()]+)");

    if(!idexp)
	idexp = ajRegCompC("^ID   ([^ \t]+)");

    rpos = ajFileTell(infr);
    while(ajFileGets(infr, &rline))
    {
	if(ajStrChar(rline,0) == '>')
	    break;
	
        rpos = ajFileTell(infr);
	ajStrAssS(&tmpstr,rline);

	if(ajRegExec(typexp, tmpstr))
	{
	    ajRegSubI(typexp, 1, &typStr);
	    if(ajStrMatchC(typStr, "ID"))
		lineType = GCGTYPE_ID;
	    else if(ajStrMatchC(typStr, "SV"))
		lineType = GCGTYPE_VER;
	    else if(ajStrMatchC(typStr, "AC"))
		lineType = GCGTYPE_ACC;
	    else if(ajStrMatchC(typStr, "DE"))
		lineType = GCGTYPE_DES;
	    else if(ajStrMatchC(typStr, "KW"))
		lineType = GCGTYPE_KEY;
	    else if(ajStrMatchC(typStr, "OS"))
		lineType = GCGTYPE_TAX;
	    else if(ajStrMatchC(typStr, "OC"))
		lineType = GCGTYPE_TAX;
	    else
		lineType=GCGTYPE_OTHER;

	    if(lineType != GCGTYPE_OTHER)
		ajRegPost(typexp, &tmpline);
	}
	else
	    lineType = GCGTYPE_OTHER;

	if(lineType == GCGTYPE_ID)
	{
	    ajRegExec(idexp, rline);
	    ajRegSubI(idexp, 1, id);
	    ajDebug("++id '%S'\n", *id);
	    continue;
	}

	if(lineType == GCGTYPE_ACC && entry->do_accession)
	{
	    while(ajRegExec(wrdexp, tmpline))
	    {
		ajRegSubI(wrdexp, 1, &tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);

		if(!tmpacnum)
		    ajStrAssS(&tmpacnum, tmpfd);

		str = ajStrNew();
		ajStrAssS(&str,tmpfd);
		ajListPush(entry->ac,(void *)str);

		ajRegPost(wrdexp, &tmpline);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_DES && entry->do_description)
	{
	    while(ajRegExec(wrdexp, tmpline))
	    {
		ajRegSubI(wrdexp, 1, &tmpfd);
		ajDebug("++des '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssS(&str,tmpfd);
		ajListPush(entry->de,(void *)str);

		ajRegPost(wrdexp, &tmpline);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_VER && entry->do_sv)
	{
	    while(ajRegExec(verexp, tmpline))
	    {
		ajRegSubI(verexp, 1, &tmpfd);
		ajDebug("++sv '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssS(&str,tmpfd);
		ajListPush(entry->sv,(void *)str);

		ajRegPost(verexp, &tmpline);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_KEY && entry->do_keyword)
	{
	    while(ajRegExec(phrexp, tmpline))
	    {
		ajRegSubI(phrexp, 1, &tmpfd);
		ajRegPost(phrexp, &tmpline);
		ajStrChompEnd(&tmpfd);
		if(!ajStrLen(tmpfd))
		    continue;
		ajDebug("++key '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssS(&str,tmpfd);
		ajListPush(entry->kw,(void *)str);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_TAX && entry->do_taxonomy)
	{
	    while(ajRegExec(taxexp, tmpline))
	    {
		ajRegSubI(taxexp, 1, &tmpfd);
		ajRegPost(taxexp, &tmpline);
		ajStrChompEnd(&tmpfd);
		if(!ajStrLen(tmpfd))
		    continue;
		ajDebug("++tax '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssS(&str,tmpfd);
		ajListPush(entry->tx,(void *)str);
	    }
	    continue;
	}
    }

    if(rpos)
        ajFileSeek(infr, rpos, 0);

    ajStrDel(&tmpacnum);

    return ajFalse;
}




/* @funcstatic dbxgcg_ParseGenbank ********************************************
**
** Parse the ID, accession from a Genbank entry
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infr [AjPFile] reference file
** @param [w] id [AjPStr*] ID
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbxgcg_ParseGenbank(EmbPBtreeEntry entry, AjPFile infr,
			       AjPStr *id)
{
    static AjPRegexp typexp = NULL;
    static AjPRegexp morexp = NULL;
    static AjPRegexp wrdexp = NULL;
    static AjPRegexp phrexp = NULL;
    static AjPRegexp taxexp = NULL;
    static AjPRegexp verexp = NULL;
    ajlong rpos = 0;
    static AjPStr tmpstr  = NULL;
    static AjPStr tmpline = NULL;
    static AjPStr rline   = NULL;
    static AjPStr tmpfd   = NULL;
    static AjPStr typStr  = NULL;
    ajint lineType=GCGTYPE_OTHER;

    AjPStr str = NULL;
    
    if(!typexp)
	typexp = ajRegCompC("^(  )?([A-Z]+)");

    if(!morexp)
	morexp = ajRegCompC("^            ");

    if(!wrdexp)
	wrdexp = ajRegCompC("([A-Za-z0-9_]+)");

    if(!phrexp)
	phrexp = ajRegCompC(" *([^;.\n\r]+)");

    if(!taxexp)
	taxexp = ajRegCompC(" *([^;.\n\r()]+)");

    if(!verexp)
	verexp = ajRegCompC("([A-Za-z0-9]+)( +GI:([0-9]+))?");

    while(ajFileGets(infr, &rline))
    {
	if(ajStrChar(rline,0) == '>')
	    break;
	
        rpos = ajFileTell(infr);
	ajStrAssS(&tmpstr,rline);

	if(ajRegExec(typexp, tmpstr))
	{
	    ajRegSubI(typexp, 2, &typStr);
	    if(ajStrMatchC(typStr, "LOCUS"))
		lineType = GCGTYPE_ID;
	    else if(ajStrMatchC(typStr, "VERSION"))
		lineType = GCGTYPE_VER;
	    else if(ajStrMatchC(typStr, "ACCESSION"))
		lineType = GCGTYPE_ACC;
	    else if(ajStrMatchC(typStr, "DEFINITION"))
		lineType = GCGTYPE_DES;
	    else if(ajStrMatchC(typStr, "KEYWORDS"))
		lineType = GCGTYPE_KEY;
	    else if(ajStrMatchC(typStr, "ORGANISM"))
		lineType = GCGTYPE_TAX;
	    else
		lineType=GCGTYPE_OTHER;

	    if(lineType != GCGTYPE_OTHER)
		ajRegPost(typexp, &tmpline);
	    ajDebug("++type line %d\n", lineType);
	}
	else if(lineType != GCGTYPE_OTHER && ajRegExec(morexp, rline))
	{
	    ajRegPost(morexp, &tmpline);
	    ajDebug("++more line %d\n", lineType);
	}
	else
	    lineType = GCGTYPE_OTHER;

	if(lineType == GCGTYPE_ID)
	{
	    ajRegExec(wrdexp, tmpline);
	    ajRegSubI(wrdexp, 1, id);
	}
	else if(lineType == GCGTYPE_ACC && entry->do_accession)
	{
	    while(ajRegExec(wrdexp, tmpline))
	    {
		ajRegSubI(wrdexp, 1, &tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssS(&str,tmpfd);
		ajListPush(entry->ac,(void *)str);

		ajRegPost(wrdexp, &tmpline);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_DES && entry->do_description)
	{
	    while(ajRegExec(wrdexp, tmpline))
	    {
	        ajRegSubI(wrdexp, 1, &tmpfd);
		ajDebug("++des '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssS(&str,tmpfd);
		ajListPush(entry->de,(void *)str);

		ajRegPost(wrdexp, &tmpline);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_KEY && entry->do_keyword)
	{
	    while(ajRegExec(phrexp, tmpline))
	    {
	        ajRegSubI(phrexp, 1, &tmpfd);
		ajRegPost(phrexp, &tmpline);
		ajStrChompEnd(&tmpfd);
		if(!ajStrLen(tmpfd))
		    continue;
		ajDebug("++key '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssS(&str,tmpfd);
		ajListPush(entry->kw,(void *)str);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_TAX && entry->do_taxonomy)
	{
	    while(ajRegExec(taxexp, tmpline))
	    {
	        ajRegSubI(taxexp, 1, &tmpfd);
		ajRegPost(taxexp, &tmpline);
		ajStrChompEnd(&tmpfd);
		if(!ajStrLen(tmpfd))
		    continue;
		ajDebug("++tax '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssS(&str,tmpfd);
		ajListPush(entry->tx,(void *)str);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_VER && entry->do_sv)
	{
	    if(ajRegExec(verexp, tmpline))
	    {
		ajRegSubI(verexp, 1, &tmpfd);
		ajDebug("++ver '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssS(&str,tmpfd);
		ajListPush(entry->sv,(void *)str);

		ajRegSubI(verexp, 3, &tmpfd);
		if(!ajStrLen(tmpfd))
		    continue;
		ajDebug("++ver gi: '%S'\n", tmpfd);

		str = ajStrNew();
		ajStrAssS(&str,tmpfd);
		ajListPush(entry->sv,(void *)str);
	    }
	    continue;
	}

    }

    if(rpos)
	ajFileSeek(infr, rpos, 0);

    return ajFalse;
}




/* @funcstatic dbxgcg_ParsePir ************************************************
**
** Parse the ID, accession from a PIR entry
**
** @param [u] entry [EmbPBtreeEntry] b+tree entry pointer
** @param [u] infr [AjPFile] reference file
** @param [w] id [AjPStr*] ID
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool dbxgcg_ParsePir(EmbPBtreeEntry entry, AjPFile infr,
			       AjPStr *id)
{
    static AjPRegexp idexp  = NULL;
    static AjPRegexp acexp  = NULL;
    static AjPRegexp ac2exp = NULL;
    static AjPRegexp keyexp = NULL;
    static AjPRegexp taxexp = NULL;
    static AjPRegexp tax2exp = NULL;
    static AjPRegexp wrdexp = NULL;
    static AjPRegexp phrexp = NULL;
    ajlong rpos;
    static AjPStr tmpstr  = NULL;
    static AjPStr tmpline = NULL;
    static AjPStr rline   = NULL;
    static AjPStr tmpfd   = NULL;

    AjPStr str = NULL;

    if(!wrdexp)
	wrdexp = ajRegCompC("([A-Za-z0-9_]+)");

    if(!idexp)
	idexp = ajRegCompC("^>..;([^;.\n\r]+)");

    if(!phrexp)				/* allow . for "sp." */
	phrexp = ajRegCompC(" *([^,;\n\r]+)");

    if(!tax2exp)				/* allow . for "sp." */
	tax2exp = ajRegCompC(" *([^,;\n\r()]+)");

    if(!acexp)
	acexp = ajRegCompC("^C;Accession:");

    if(!ac2exp)
	ac2exp = ajRegCompC("([A-Za-z0-9]+)");

    if(!taxexp)
	taxexp = ajRegCompC("^C;Species:");

    if(!keyexp)
	keyexp = ajRegCompC("^C;Keywords:");

    rpos = ajFileTell(infr);

    ajDebug("++id '%S'\n", *id);


    ajFileGets(infr, &rline);
    ajDebug("line-2 '%S'\n", rline);
    if(entry->do_description)
    {
	while(ajRegExec(wrdexp, rline))
	{
	    ajRegSubI(wrdexp, 1, &tmpfd);
	    ajDebug("++des '%S'\n", tmpfd);

	    str = ajStrNew();
	    ajStrAssS(&str,tmpfd);
	    ajListPush(entry->de,(void *)str);

	    ajRegPost(wrdexp, &rline);
	}
    }

    while(ajStrChar(rline,0)!='>')
    {
        rpos = ajFileTell(infr);
	ajStrAssS(&tmpstr,rline);

	if(ajRegExec(acexp, rline))
	{
	    ajRegPost(acexp, &tmpline);
	    while(ajRegExec(ac2exp, tmpline))
	    {
		ajRegSubI(ac2exp, 1, &tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);

		if(entry->do_accession)
		{
		    str = ajStrNew();
		    ajStrAssS(&str,tmpfd);
		    ajListPush(entry->ac,(void *)str);
		}

		ajRegPost(ac2exp, &tmpline);
	    }
	}

	if(entry->do_keyword)
	{
	    if(ajRegExec(keyexp, rline))
	    {
		ajRegPost(keyexp, &tmpline);
		while(ajRegExec(phrexp, tmpline))
		{
		    ajRegSubI(phrexp, 1, &tmpfd);
		    ajDebug("++key '%S'\n", tmpfd);
		    ajStrChompEnd(&tmpfd);

		    str = ajStrNew();
		    ajStrAssS(&str,tmpfd);
		    ajListPush(entry->kw,(void *)str);

		    ajRegPost(phrexp, &tmpline);
		}
	    }
	}

	if(entry->do_taxonomy)
	{
	    if(ajRegExec(taxexp, rline))
	    {
		ajRegPost(taxexp, &tmpline);
		while(ajRegExec(tax2exp, tmpline))
		{
		    ajRegSubI(tax2exp, 1, &tmpfd);
		    ajStrChompEnd(&tmpfd);
		    ajDebug("++tax '%S'\n", tmpfd);

		    str = ajStrNew();
		    ajStrAssS(&str,tmpfd);
		    ajListPush(entry->tx,(void *)str);

		    ajRegPost(tax2exp, &tmpline);
		}
	    }
	}

	if(!ajFileGets(infr, &rline))
	{
	    rpos = 0;
	    break;
	}
    }

    if(rpos)
	ajFileSeek(infr, rpos, 0);

    return ajFalse;
}
