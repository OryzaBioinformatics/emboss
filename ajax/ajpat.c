/******************************************************************************
** @source AJAX PATTERN (ajax pattern and patternlist) functions
**
** These functions allow handling of patternlists.
**
** @author Copyright (C) 2004 Henrikki Almusa, Medicel Oy
** @version 0.9
** @modified Aug 10 Beta version
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/
#include "ajax.h"

/* @datastatic PatPRegTypes ***************************************************
**
** Regular expression pattern types
**
** @alias PatSTypes
** @alias PatOTypes
**
** @attr Name [char*] Type name
** @attr Desc [char*] Type description
** @@
******************************************************************************/

typedef struct PatSRegTypes
{
    char *Name;
    char *Desc;
} PatORegTypes;

#define PatPRegTypes PatORegTypes*

static PatORegTypes patRegTypes[] = {
/* "Name",        "Description" */
  {"string",      "General string pattern"},
  {"protein",     "Protein sequence pattern"},
  {"nucleotide",  "Nucleotide sequence pattern"},
  {NULL, NULL}
};

/* @func ajPatternSeqNewList **************************************************
**
** Constructor for a sequence pattern object. Sets all but compiled object.
** That is set with search function. Adds the pattern to a pattern list.
**
** @param [u] plist [AjPPatlistSeq] Pattern list
** @param [r] name [const AjPStr] Name of the pattern
** @param [r] pat [const AjPStr] Pattern as string
** @param [r] mismatch [ajint] mismatch value
** @return [AjPPatternSeq] New pattern object
** @@
******************************************************************************/
AjPPatternSeq ajPatternSeqNewList (AjPPatlistSeq plist,
				   const AjPStr name, const AjPStr pat,
				   ajint mismatch)
{
    AjPPatternSeq pthis;

    if (!ajStrGetLen(pat))
	return NULL;

    AJNEW0(pthis);

    if(ajStrGetLen(name))
	ajStrAssignS (&pthis->Name,name);
    else
	ajFmtPrintS(&pthis->Name, "pattern%d",
		    1+ajListLength(plist->Patlist));

    ajStrAssignS(&pthis->Pattern,pat);
    pthis->Protein  = plist->Protein;
    pthis->Mismatch = mismatch;

    ajPatlistAddSeq (plist,pthis);

    return pthis;
}

/* @func ajPatternRegexNewList ************************************************
**
** Constructor for a pattern object. Sets all but compiled object. That is set
** with search function. Adds the pattern to a pattern list.
**
** @param [u] plist [AjPPatlistRegex] Regular expression pattern list
** @param [r] name [const AjPStr] Name of the pattern
** @param [r] pat [const AjPStr] Pattern as string
** @return [AjPPatternRegex] New regular expression pattern object
** @@
******************************************************************************/
AjPPatternRegex ajPatternRegexNewList(AjPPatlistRegex plist,
				      const AjPStr name,
				      const AjPStr pat)
{
    AjPPatternRegex pthis;

    if (!ajStrGetLen(pat))
	return NULL;

    AJNEW0(pthis);

    if(ajStrGetLen(name))
	ajStrAssignS (&pthis->Name,name);
    else
	ajFmtPrintS(&pthis->Name, "regex%d",
		    1+ajListLength(plist->Patlist));

    ajStrAssignS  (&pthis->Pattern,pat);
    pthis->Type = plist->Type;

    if(plist->Type == AJ_PAT_TYPE_PRO)
    {
    }
    else if(plist->Type == AJ_PAT_TYPE_NUCL)
    {
    }

    pthis->Compiled = ajRegComp(pthis->Pattern);
    ajPatlistAddRegex (plist,pthis);

    return pthis;
}

/* @func ajPatternRegexDel ****************************************************
**
** Destructor for a regular expression pattern object
**
** @param [d] pthys [AjPPatternRegex*] Pattern object reference
** @return [void]
** @@
******************************************************************************/
void ajPatternRegexDel (AjPPatternRegex* pthys)
{
    AjPPatternRegex thys = *pthys;
    ajStrDel(&thys->Name);
    ajStrDel(&thys->Pattern);

    ajRegFree(&thys->Compiled);

    AJFREE (*pthys);

    return;
}

/* @func ajPatternSeqDel ******************************************************
**
** Destructor for a pattern object
**
** @param [d] pthys [AjPPatternSeq*] Pattern object reference
** @return [void]
** @@
******************************************************************************/
void ajPatternSeqDel (AjPPatternSeq* pthys)
{
    AjPPatternSeq thys = *pthys;
    ajStrDel(&thys->Name);
    ajStrDel(&thys->Pattern);

    if (&thys->Compiled)
	ajDebug ("Pattern still has compiled pattern in causing memory leak. ",
		 "Use embRemoveCompiledPatterns to remove it.\n");

    AJFREE (*pthys);

    return;
}

/* @func ajPatternSeqGetName **************************************************
**
** Returns the name of the pattern.
**
** @param [r] thys [const AjPPatternSeq] Pattern
** @return [const AjPStr] Name of the pattern. Real pointer in structure.
** @@
******************************************************************************/
const AjPStr ajPatternSeqGetName (const AjPPatternSeq thys)
{
    return thys->Name;
}

/* @func ajPatternRegexGetName ************************************************
**
** Returns the name of the pattern.
**
** @param [r] thys [const AjPPatternRegex] Pattern
** @return [const AjPStr] Name of the pattern. Real pointer in structure.
** @@
******************************************************************************/
const AjPStr ajPatternRegexGetName (const AjPPatternRegex thys)
{
    return thys->Name;
}

/* @func ajPatternSeqGetPattern ***********************************************
**
** Returns pattern in string format.
**
** @param [r] thys [const AjPPatternSeq] Pattern
** @return [const AjPStr] Pattern. Real pointer in structure.
** @@
******************************************************************************/
const AjPStr ajPatternSeqGetPattern (const AjPPatternSeq thys)
{
    return thys->Pattern;
}

/* @func ajPatternRegexGetPattern *********************************************
**
** Returns pattern in string format.
**
** @param [r] thys [const AjPPatternRegex] Pattern
** @return [const AjPStr] Pattern. Real pointer in structure.
** @@
******************************************************************************/
const AjPStr ajPatternRegexGetPattern (const AjPPatternRegex thys)
{
    return thys->Pattern;
}

/* @func ajPatternSeqGetCompiled **********************************************
**
** Returns void pointer to compiled pattern.
**
** @param [r] thys [const AjPPatternSeq] Pattern
** @return [AjPPatComp] Reference for compiled pattern
** @@
******************************************************************************/
AjPPatComp ajPatternSeqGetCompiled (const AjPPatternSeq thys)
{
    return thys->Compiled;
}

/* @func ajPatternRegexGetCompiled ********************************************
**
** Returns void pointer to compiled pattern. Compiles expression if not
** yet done.
**
** @param [r] thys [const AjPPatternRegex] Pattern
** @return [AjPRegexp] Reference for compiled pattern
** @@
******************************************************************************/
AjPRegexp ajPatternRegexGetCompiled (const AjPPatternRegex thys)
{
    return thys->Compiled;
}

/* @func ajPatternSeqGetProtein ***********************************************
**
** Returns true if the pattern is for a protein sequence.
**
** @param [r] thys [const AjPPatternSeq] Pattern
** @return [AjBool] ajTrue for a protein pattern
** @@
******************************************************************************/
AjBool ajPatternSeqGetProtein (const AjPPatternSeq thys)
{
    return thys->Protein;
}

/* @func ajPatternRegexGetType ************************************************
**
** Returns the type of the pattern.
**
** @param [r] thys [const AjPPatternRegex] Pattern
** @return [ajint] Type of the pattern.
** @@
******************************************************************************/
ajint ajPatternRegexGetType (const AjPPatternRegex thys)
{
    return thys->Type;
}

/* @func ajPatternSeqGetMismatch **********************************************
**
** Returns the mismatch of the pattern.
**
** @param [r] thys [const AjPPatternSeq] Pattern
** @return [ajint] Mismatch value of the pattern.
** @@
******************************************************************************/
ajint ajPatternSeqGetMismatch (const AjPPatternSeq thys)
{
    return thys->Mismatch;
}


/* @func ajPatternSeqSetCompiled **********************************************
**
** Sets the compiled pattern
**
** @param [u] thys [AjPPatternSeq] Pattern
** @param [u] pat [void *] Compiled pattern
** @return [void]
** @@
******************************************************************************/
void ajPatternSeqSetCompiled (AjPPatternSeq thys, void* pat)
{
    thys->Compiled = pat;
    return;
}

/* @func ajPatternRegexSetCompiled ********************************************
**
** Sets the compiled pattern
**
** @param [u] thys [AjPPatternRegex] Pattern
** @param [u] pat [AjPRegexp] Compiled pattern
** @return [void]
** @@
******************************************************************************/
void ajPatternRegexSetCompiled (AjPPatternRegex thys, AjPRegexp pat)
{
    thys->Compiled = pat;
    return;
}

/* @func ajPatternSeqDebug ****************************************************
**
** Constructor for a pattern list object
**
** @param [r] pat [const AjPPatternSeq] Pattern object
** @return [void]
** @@
******************************************************************************/
void ajPatternSeqDebug (const AjPPatternSeq pat)
{
    ajDebug ("patPatternSeqDebug:\n  name: %S\n  pattern: %S\n  protein: %B,"
             " mismatch: %d\n",
	     pat->Name, pat->Pattern, pat->Protein, pat->Mismatch);
    return;
}



/* @func ajPatternRegexDebug **************************************************
**
** Constructor for a pattern list object
**
** @param [r] pat [const AjPPatternRegex] Pattern object
** @return [void]
** @@
******************************************************************************/
void ajPatternRegexDebug (const AjPPatternRegex pat)
{
    ajDebug ("patPatternRegexDebug:\n  name: %S\n  pattern: %S\n  type: %d",
	     pat->Name, pat->Pattern, pat->Type);
    return;
}




/* @func ajPatlistRegexNew ****************************************************
**
** Constructor for a pattern list object
**
** @return [AjPPatlistRegex] New pattern list object
** @@
******************************************************************************/
AjPPatlistRegex ajPatlistRegexNew (void)
{
    AjPPatlistRegex pthis;

    AJNEW0(pthis);

    pthis->Patlist = ajListNew();
    pthis->Iter    = NULL;

    // ajDebug ("ajPatlistRegexNew size '%d'\n",ajListLength(pthis->Patlist));

    return pthis;
}

/* @func ajPatlistRegexNewType ************************************************
**
** Constructor for a pattern list object with a specified type
**
** @param [r] type [ajint] type value
** @return [AjPPatlistRegex] New pattern list object
** @@
******************************************************************************/
AjPPatlistRegex ajPatlistRegexNewType (ajint type)
{
    AjPPatlistRegex pthis;

    AJNEW0(pthis);

    pthis->Patlist = ajListNew();
    pthis->Iter    = NULL;
    pthis->Type = type;

    // ajDebug ("ajPatlistRegexNew size '%d'\n",ajListLength(pthis->Patlist));

    return pthis;
}

/* @func ajPatlistSeqNew ******************************************************
**
** Constructor for a pattern list object. Defaults to protein.
**
** @return [AjPPatlistSeq] New pattern list object
** @@
******************************************************************************/
AjPPatlistSeq ajPatlistSeqNew (void)
{
    AjPPatlistSeq pthis;

    AJNEW0(pthis);

    pthis->Patlist = ajListNew();
    pthis->Iter    = NULL;
    pthis->Protein = ajTrue;

    ajDebug ("ajPatlistSeqNew\n");

    return pthis;
}

/* @func ajPatlistSeqNewType **************************************************
**
** Constructor for a pattern list object
**
** @param [r] type [AjBool] True for a protein pattern
** @return [AjPPatlistSeq] New pattern list object
** @@
******************************************************************************/
AjPPatlistSeq ajPatlistSeqNewType(AjBool type)
{
    AjPPatlistSeq pthis;

    AJNEW0(pthis);

    pthis->Patlist = ajListNew();
    pthis->Iter    = NULL;
    pthis->Protein = type;

    ajDebug ("ajPatlistSeqNewType type '%d'\n", pthis->Protein);

    return pthis;
}

/* @func ajPatlistRegexDel ****************************************************
**
** Destructor for a pattern list object
**
** @param [d] pthys [AjPPatlistRegex*] Pattern list object reference
** @return [void]
** @@
******************************************************************************/
void ajPatlistRegexDel (AjPPatlistRegex* pthys)
{
    AjPPatlistRegex thys = NULL;
    AjPPatternRegex patternregex = NULL;

    thys = *pthys;

    while (ajListPop(thys->Patlist, (void **)&patternregex))
    {
	ajDebug("ajPatlistRegexDel list size: %d\n",
		ajListLength(thys->Patlist));
	ajPatternRegexDel(&patternregex);
    }
    if (thys->Iter)
	ajListIterFree (&thys->Iter);
    ajListFree(&thys->Patlist);

    AJFREE(*pthys);

    return;
}

/* @func ajPatlistSeqDel ******************************************************
**
** Destructor for a pattern list object
**
** @param [d] pthys [AjPPatlistSeq*] Pattern list object reference
** @return [void]
** @@
******************************************************************************/
void ajPatlistSeqDel (AjPPatlistSeq* pthys)
{
    AjPPatlistSeq thys = NULL;
    AjPPatternSeq patternseq = NULL;

    thys = *pthys;

    while (ajListPop(thys->Patlist, (void **)&patternseq))
	ajPatternSeqDel(&patternseq);
    if (thys->Iter)
	ajListIterFree (&thys->Iter);
    ajListFree(&thys->Patlist);

    AJFREE(*pthys);

    return;
}

/* @func ajPatlistSeqRead *****************************************************
**
** Parses a file into pattern list object. If there is not mismatch value on
** pattern in file, it is assumed to be 0.
**
** @param [r] patspec [const AjPStr] Pattern specification
** @param [r] patname [const AjPStr] Default pattern name prefix
** @param [r] protein [AjBool] ajTrue for protein patterns
** @param [r] mismatches [ajint] default number of mismatches
** @return [AjPPatlistSeq] Pattern list
** @@
******************************************************************************/
AjPPatlistSeq ajPatlistSeqRead (const AjPStr patspec,
				const AjPStr patname,
				AjBool protein, ajint mismatches)
{
    AjPPatlistSeq patlist = NULL;
    AjPPatternSeq pattern;
    AjPStr line = NULL;
    AjPStr name = NULL;
    AjPFile infile = NULL;
    AjPRegexp misreg = NULL;
    AjPStr patstr = NULL;
    AjPStr pat = NULL;
    ajint mismatch = 0;

    ajStrAssignS(&patstr, patspec);

    patlist = ajPatlistSeqNewType(protein);

    ajDebug("ajPatlistSeqRead patspec: '%S' patname: '%S' "
	    "protein: %B mismatches: %d\n",
	    patspec, patname, protein, mismatches);
    if(ajStrGetCharFirst(patstr) == '@') {
	ajStrCutStart(&patstr, 1);
	infile = ajFileNewIn(patstr);
	line = ajStrNew();
	name = ajStrNew();

	while (ajFileGetsTrim(infile,&line))
	{
	    AjPRegexp misreg = ajRegCompC("<mismatch=(\\d+)>");
	    if (ajStrGetCharFirst(line) == '>')
	    {
		if (ajStrGetLen(name))
		{
		    pattern = ajPatternSeqNewList(patlist,name,pat,
						  mismatch);
		    ajStrSetClear(&name);
		    ajStrSetClear(&pat);
		    mismatch=mismatches;
		}
		ajStrCutStart(&line,1);
		if (ajRegExec(misreg,line))
		{
		    ajRegSubI(misreg,1,&name);
		    ajStrToInt(name,&mismatch);
		    ajStrTruncateLen(&line,ajRegOffset(misreg));
		    ajStrTrimWhiteEnd(&line);
		}
		ajStrAssignS (&name,line);
		ajStrAssignEmptyS(&name, patname);
	    }
	    else
		ajStrAppendS (&pat,line);
	}
	pattern = ajPatternSeqNewList(patlist,name,pat,mismatch);
	ajFileClose(&infile);
	ajRegFree(&misreg);
    }
    else
    {
	pattern = ajPatternSeqNewList(patlist,patname,patstr,mismatches);
    }

    ajStrDel(&name);
    ajStrDel(&line);
    ajStrDel(&pat);
    ajStrDel(&patstr);

    return patlist;
}

/* @func ajPatlistRegexRead ***************************************************
**
** Parses a file of regular expressions into a pattern list object.
**
** @param [r] patspec [const AjPStr] Name of the file with patterns
** @param [r] patname [const AjPStr] Default pattern name prefix
** @param [r] type [ajint] Type of the patterns
** @param [r] upper [AjBool] Convert to upper case
** @param [r] lower [AjBool] Convert to lower case
** @return [AjPPatlistRegex] Pattern list
** @@
******************************************************************************/
AjPPatlistRegex ajPatlistRegexRead (const AjPStr patspec,
				    const AjPStr patname,
				    ajint type, AjBool upper, AjBool lower)
{
    AjPPatlistRegex patlist = NULL;
    AjPPatternRegex pattern;
    AjPStr line = NULL;
    AjPStr pat  = NULL;
    AjPStr name = NULL;
    AjPFile infile = NULL;

    patlist = ajPatlistRegexNewType(type);

    if(ajStrGetCharFirst(patspec) ==  '@')
    {
	infile = ajFileNewIn(patspec);
	line = ajStrNew();
	pat  = ajStrNew();
	name = ajStrNew();

	while (ajFileGetsTrim(infile,&line))
	{
	    if (ajStrFindC(line,">")>-1)
	    {
		if (ajStrGetLen(name))
		{
		    if(lower)
			ajStrFmtLower(&pat);
		    if(upper)
			ajStrFmtUpper(&pat);
		    pattern = ajPatternRegexNewList(patlist,name,pat);
		    ajStrSetClear(&name);
		    ajStrSetClear(&pat);
		}
		ajStrCutStart(&line,1);
		ajStrAssignS (&name,line);
	    }
	    else
		ajStrAppendS (&pat,line);
	}
	pattern = ajPatternRegexNewList(patlist,name,pat);
    }
    else
    {
	ajStrAssignS(&pat, patspec);
	if(lower)
	    ajStrFmtLower(&pat);
	if(upper)
	    ajStrFmtUpper(&pat);
	pattern = ajPatternRegexNewList(patlist,patname,pat);
    }

    ajFileClose(&infile);
    ajStrDel(&name);
    ajStrDel(&line);
    ajStrDel(&pat);

    return patlist;
}

/* @func ajPatlistSeqGetSize **************************************************
**
** Gets number of patterns from list.
**
** @param [r] thys [const AjPPatlistSeq] Pattern list object
** @return [ajint] Number of patterns
** @@
******************************************************************************/
ajint ajPatlistSeqGetSize (const AjPPatlistSeq thys)
{
    return ajListLength(thys->Patlist);
}

/* @func ajPatlistRegexGetSize ************************************************
**
** Gets number of patterns from list.
**
** @param [r] thys [const AjPPatlistRegex] Pattern list object
** @return [ajint] Number of patterns
** @@
******************************************************************************/
ajint ajPatlistRegexGetSize (const AjPPatlistRegex thys)
{
    return ajListLength(thys->Patlist);
}

/* @func ajPatlistSeqGetNext **************************************************
**
** Gets next available pattern from list.
**
** @param [u] thys [AjPPatlistSeq] Pattern list object
** @param [w] pattern [AjPPatternSeq*] Pattern object reference
** @return [AjBool] ajTrue if there was next object
** @@
******************************************************************************/
AjBool ajPatlistSeqGetNext (AjPPatlistSeq thys, AjPPatternSeq* pattern)
{
    if (!thys->Iter)
	thys->Iter = ajListIter(thys->Patlist);

    if (ajListIterMore(thys->Iter))
	*pattern = ajListIterNext (thys->Iter);
    else
    {
	ajPatlistSeqRewind(thys);
	return ajFalse;
    }

    return ajTrue;
}

/* @func ajPatlistRegexGetNext ************************************************
**
** Gets next available pattern from list.
**
** @param [u] thys [AjPPatlistRegex] Pattern list object
** @param [w] pattern [AjPPatternRegex*] Pattern object reference
** @return [AjBool] ajTrue if there was next object
** @@
******************************************************************************/
AjBool ajPatlistRegexGetNext (AjPPatlistRegex thys,
			      AjPPatternRegex* pattern)
{
    if (!thys->Iter)
	thys->Iter = ajListIter(thys->Patlist);

    if (ajListIterMore(thys->Iter))
	*pattern = ajListIterNext (thys->Iter);
    else
    {
	ajPatlistRegexRewind(thys);
	return ajFalse;
    }

    return ajTrue;
}



/* @func ajPatlistRegexRewind *************************************************
**
** Resets the pattern list iteration.
**
** @param [u] thys [AjPPatlistRegex] Pattern list object reference
** @return [void]
** @@
******************************************************************************/
void ajPatlistRegexRewind (AjPPatlistRegex thys)
{
    if (thys->Iter)
	ajListIterFree (&thys->Iter);
    thys->Iter=NULL;

    return;
}

/* @func ajPatlistSeqRewind ***************************************************
**
** Resets the pattern list iteration.
**
** @param [u] thys [AjPPatlistSeq] Pattern list object reference
** @return [void]
** @@
******************************************************************************/
void ajPatlistSeqRewind (AjPPatlistSeq thys)
{
    if (thys->Iter)
	ajListIterFree (&thys->Iter);
    thys->Iter=NULL;

    return;
}

/* @func ajPatlistRegexRemoveCurrent ******************************************
**
** Removes current pattern from pattern list. If looping has not started or
** pattern list has just been rewound then nothing is removed.
**
** @param [u] thys [AjPPatlistRegex] Pattern list from which to remove
** @return [void]
** @@
******************************************************************************/
void ajPatlistRegexRemoveCurrent (AjPPatlistRegex thys)
{
    if (!thys->Iter)
	return;

    ajListRemove (thys->Iter);
    ajListIterBackNext (thys->Iter);

    return;
}


/* @func ajPatlistSeqRemoveCurrent ********************************************
**
** Removes current pattern from pattern list. If looping has not started or
** pattern list has just been rewound then nothing is removed.
**
** @param [u] thys [AjPPatlistSeq] Pattern list from which to remove
** @return [void]
** @@
******************************************************************************/
void ajPatlistSeqRemoveCurrent (AjPPatlistSeq thys)
{
    if (!thys->Iter)
	return;

    ajListRemove (thys->Iter);
    ajListIterBackNext (thys->Iter);

    return;
}


/* @func ajPatlistAddSeq ******************************************************
**
** Adds pattern into patternlist
**
** @param [u] thys [AjPPatlistSeq] Pattern list object reference
** @param [u] pat [AjPPatternSeq] Pattern to be added
** @return [void]
** @@
******************************************************************************/
void ajPatlistAddSeq (AjPPatlistSeq thys, AjPPatternSeq pat)
{
    ajDebug ("ajPatlistAddSeq list size %d '%S' '%S' '%B' '%d'\n",
             ajListLength (thys->Patlist), pat->Name,
             pat->Pattern, pat->Protein, pat->Mismatch);
    ajListPushApp (thys->Patlist, pat);

    return;
}


/* @func ajPatlistAddRegex ****************************************************
**
** Adds pattern into patternlist
**
** @param [u] thys [AjPPatlistRegex] Pattern list object reference
** @param [u] pat [AjPPatternRegex] Pattern to be added
** @return [void]
** @@
******************************************************************************/
void ajPatlistAddRegex (AjPPatlistRegex thys, AjPPatternRegex pat)
{
    ajDebug ("ajPatlistAddRegex list size %d '%S' '%S' '%d'\n",
             ajListLength (thys->Patlist), pat->Name,
             pat->Pattern, pat->Type);
    ajListPushApp (thys->Patlist, pat);

    return;
}

/* @func ajPPatCompNew *******************************************************
**
** Create prosite pattern structure.
**
** @return [AjPPatComp] pattern structure
** @@
******************************************************************************/
AjPPatComp ajPPatCompNew ()
{
    AjPPatComp pthis;

    AJNEW0(pthis);

    pthis->pattern=ajStrNew();
    pthis->regex=ajStrNew();

    return pthis;
}

/* @func ajPPatCompDel *******************************************************
**
** Delete prosite pattern structure.
**
** @param [d] pthys [AjPPatComp*] Prosite pattern structure
**
** @return [void]
** @@
******************************************************************************/
void ajPPatCompDel (AjPPatComp *pthys)
{
    int i;

    AjPPatComp pthis = *pthys;
    ajStrDel(&pthis->pattern);
    ajStrDel(&pthis->regex);

    if(pthis->type==6)
	for(i=0;i<pthis->m;++i)
	    AJFREE(pthis->skipm[i]);

    AJFREE(*pthys);

    return;
}


/* @func ajPatternRegexType ***************************************************
**
** Returns type assocaited with a named type of regular expression
**
** @param [r] type [const AjPStr] Regular expression type
** @return [ajint] Type number, defaults to 0 (string)
******************************************************************************/

ajint ajPatternRegexType(const AjPStr type)
{
    ajint i = 0;

    while (patRegTypes[i].Name) {
	if(ajStrMatchCaseC(type, patRegTypes[i].Name))
	    return i;
    }
    return 0;
}




/* @func ajPatlistRegexDoc **************************************************
**
** Documents patterns to a formatted string
**
** @param [u] plist [AjPPatlistRegex] Pattern list object
** @param [w] pdoc [AjPStr*] Formatted string
** @return [ajint] Number of patterns
** @@
******************************************************************************/
ajint ajPatlistRegexDoc (AjPPatlistRegex plist, AjPStr* pdoc)
{
    AjPPatternRegex pat = NULL;

    ajFmtPrintS(pdoc, "%-12S %S\n", "Pattern_name", "Pattern");
    while (ajPatlistRegexGetNext(plist, &pat))
    {
	ajFmtPrintAppS(pdoc, "%-12S %S\n",
		       ajPatternRegexGetName(pat),
		       ajPatternRegexGetPattern(pat));
    }
    return ajListLength(plist->Patlist);
}




/* @func ajPatlistSeqDoc **************************************************
**
** Documents patterns to a formatted string
**
** @param [u] plist [AjPPatlistSeq] Pattern list object
** @param [w] pdoc [AjPStr*] kFormatted string
** @return [ajint] Number of patterns
** @@
******************************************************************************/
ajint ajPatlistSeqDoc (AjPPatlistSeq plist, AjPStr* pdoc)
{
    AjPPatternSeq pat = NULL;

    ajFmtPrintS(pdoc, "%-12s %8s %s\n",
		   "Pattern_name", "Mismatch", "Pattern");
    while (ajPatlistSeqGetNext(plist, &pat))
    {
	ajFmtPrintAppS(pdoc, "%-12S %8d %S\n",
		       ajPatternSeqGetName(pat),
		       ajPatternSeqGetMismatch(pat),
		       ajPatternSeqGetPattern(pat));
    }
    return ajListLength(plist->Patlist);
}
