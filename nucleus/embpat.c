/* @source embpat.c
**
** General routines for pattern matching.
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

#include "ajax.h"
#include "embpat.h"
#include "embmat.h"
#include "stdlib.h"
#include "limits.h"

static void   patRestrictPushHit(EmbPPatRestrict *enz, AjPList *l, ajint pos,
				 ajint begin, ajint len, AjBool forward);
static ajint    patRestrictStartCompare(const void *a, const void *b);
static ajint    patRestrictCutCompare(const void *a, const void *b);
static ajint    patRestrictNameCompare(const void *a, const void *b);

static void   patAminoCarboxyl(AjPStr *s, AjBool *amino, AjBool *carboxyl);
static AjBool patParenTest(char *p, AjBool *repeat, AjBool *range);
static AjBool patExpandRepeat(AjPStr *s);
static void   patIUBTranslate(AjPStr *pat);
static AjBool patBruteClass(char *p, char c);
static AjBool patBruteCompl(char *p, char c);
static AjBool patBruteIsRange(char *t, ajint *x, ajint *y);
static AjBool patBruteCharMatch(char *t, char c);
static ajint    patBruteNextPatChar(char *t, ajint ppos);
static AjBool patOUBrute(char *seq, char *pat, ajint spos, ajint ppos, ajint mm,
			ajint omm, ajint level, AjPList *l, AjBool carboxyl,
			ajint begin, ajint *count, AjPStr *name, ajint st);

static void patStringFree(void **x, void *cl) {
  ajint **ptr = (ajint **)x;

  AJFREE(*ptr);
}

/* @func embPatSeqCreateRegExp *************************************************
**
** Create a regular expression for a string and substitute the chars for
** Nucleotides or proteins as needed.
**
** @param [r] thys [AjPStr] string to create reg expr from.
** @param [r] protein [AjBool] is it a protein.
** 
** @return [AjPStr] the new regular expression.
******************************************************************************/

AjPStr embPatSeqCreateRegExp(AjPStr thys, AjBool protein) {
  return embPatSeqCreateRegExpC(ajStrStr(thys), protein);
}

/* @func embPatSeqCreateRegExpC ************************************************
**
** Create a regular expression for a string and substitute the chars for
** Nucleotides or proteins as needed.
**
** @param [r] ptr [char *] text to create reg expr from.
** @param [r] protein [AjBool] is it a protein.
** 
** @return [AjPStr] the new regular expression.
******************************************************************************/

AjPStr embPatSeqCreateRegExpC(char *ptr, AjBool protein) {

  char *nucpatternmatch[] = {
    "[Aa]", "[CcGgTtUu]", "[Cc]", "[AaGgTtUu]", /* abcd */
    "", "", "[Gg]", "[AaCcTtUu]", /* efgh */  
    "", "", "", "",		/* ijkl */
    "[AaCc]", "[A-Za-z]", "", "", /* mnop */
    "", "[AaGg]", "[GgCc]", "[TtUu]", /* qrst */
    "[TtUu]","[AaCcGg]", "[AaTtUu]", "[A-Za-z]", /* uvwx */
    "[CTU]", "" };		/* yz */


  char *protpatternmatch[] = { "[Aa]","[DdNn]","[Cc]","[Dd]", /* abcd */
			       "[Ee]","[Ff]","[Gg]","[AaCcTtUu]", /* efgh */
			       "[Ii]","","[Kk]","[Ll]",	/* ijkl */
			       "[Mm]","[A-Za-z]","","[Pp]", /* mnop */
			       "[Qq]","[Rr]","[Ss]","[Tt]", /* qrst */
			       "[Uu]","[Vv]","[Ww]","[A-Za-z]",	/* uvwx */
			       "[Yy]","[EeQq]" }; /* yz */
  AjPStr regexp = 0;
  ajint match;
  char match2[2] ="";


  regexp = ajStrNewL(strlen(ptr) * 4); /* just a rough guess */

  while(*ptr != '\0'){
    if((*ptr > 64 && *ptr < 91) || (*ptr > 96 && *ptr < 123)){
      if(*ptr > 91)
	match = ((ajint) *ptr) - 97;
      else
	match = ((ajint) *ptr) - 65;
      if(protein)
	(void) ajStrAppC(&regexp,protpatternmatch[match]);
      else
	(void) ajStrAppC(&regexp,nucpatternmatch[match]);
    }
    else{
      match2[0] = *ptr;
      (void) ajStrAppC(&regexp,match2);
    }
    ptr++;
  }
  return regexp;
}

/* @func embPatSeqMatchFind *********************************************
**
** Find all the regular expression matches of reg in the string string.
**
** @param [r] seq [AjPSeq] Sequence to be searched.
** @param [r] reg [AjPStr] regular expression string.
**
** @return [EmbPPatMatch] Results of the pattern matching.
**
******************************************************************************/

EmbPPatMatch embPatSeqMatchFind(AjPSeq seq, AjPStr reg){
  return embPatSeqMatchFindC(seq, ajStrStr(reg));
}

/* @func embPatSeqMatchFindC *********************************************
**
** Find all the regular expression matches of reg in the string string.
**
** @param [r] seq [AjPSeq] Sequence to be searched.
** @param [r] reg [char*] regular expression text.
**
** @return [EmbPPatMatch] Results of the pattern matching.
**
******************************************************************************/

EmbPPatMatch embPatSeqMatchFindC(AjPSeq seq, char *reg){
  AjPStr regexp = NULL;
  AjBool protein;
  EmbPPatMatch results;

  protein = ajSeqIsProt(seq);

  regexp = embPatSeqCreateRegExpC(reg,protein);
    
  results = embPatMatchFind(regexp, ajSeqStr(seq));

  ajStrDel(&regexp);

  return results;
  
}

/* @func embPatMatchFind *********************************************
**
** Find all the regular expression matches of reg in the string string.
**
** @param [r] regexp [AjPStr] Regular expression string.
** @param [r] string   [AjPStr] String to be searched.
**
** @return [EmbPPatMatch] Results of the pattern matching.
**
******************************************************************************/
EmbPPatMatch embPatMatchFind(AjPStr regexp, AjPStr string){
  return embPatMatchFindC(regexp, ajStrStr(string));
}


/* @func embPatMatchFindC *********************************************
**
** Find all the regular expression matches of reg in the string string.
**
** @param [r] regexp [AjPStr] Regular expression string.
** @param [r] sptr   [char *] String to be searched.
**
** @return [EmbPPatMatch] Results of the pattern matching.
**
******************************************************************************/

EmbPPatMatch embPatMatchFindC(AjPStr regexp, char *sptr){
  AjPRegexp regcomp = NULL;
  EmbPPatMatch results; 
  AjPList poslist = ajListNew();
  AjPList lenlist = ajListNew();
  AjIList iter;
  ajint *pos,*len,posi,i;
  char *ptr = sptr;
  regcomp = ajRegComp(regexp);

  AJNEW(results);

  while(*sptr != '\0' && ajRegExecC(regcomp,sptr)){
    /*    ajRegTrace(regcomp);*/
    AJNEW (pos);
    *pos = posi = ajRegOffsetC(regcomp);
    AJNEW (len);
    *len = ajRegLenI(regcomp,0);
    *pos +=sptr-ptr;
    ajListAppend(poslist, ajListNodesNew(pos, NULL));
    ajListAppend(lenlist, ajListNodesNew(len, NULL));
    sptr += posi+1;
  }
	
  ajRegFree(&regcomp);
  results->number  = ajListLength(poslist);
  if(results->number){
    AJCNEW(results->start, results->number);
    AJCNEW(results->len, results->number);
    
    i=0;
    iter=ajListIter(poslist);
    while (ajListIterMore(iter)) {
      results->start[i] = *(ajint *) ajListIterNext (iter);
      i++;
    }
    ajListIterFree (iter);
    
    i=0;
    iter=ajListIter(lenlist);
    while (ajListIterMore(iter)) {
      results->len[i] = *(ajint *) ajListIterNext (iter);
      i++;
    }
    ajListIterFree (iter);
    
    ajListMap(poslist,patStringFree, NULL);
    ajListMap(lenlist,patStringFree, NULL);
    ajListFree(&poslist);
    ajListFree(&lenlist);

  }
  else{
    ajListFree(&poslist);
    ajListFree(&lenlist);
  }
  return results;
}

/* @func  embPatMatchGetLen***************************************************
**
** Returns the length from the pattern match structure for index'th item.
**
** @param [r] data [EmbPPatMatch] results of match.
** @param [r] index   [ajint] index to structure.
**
** @return [ajint] returns -1 if not available.
**
******************************************************************************/

ajint embPatMatchGetLen(EmbPPatMatch data, ajint index){

  if(data->number <= index || index < 0)
    return -1;
  return data->len[index];
}

/* @func  embPatMatchGetEnd***************************************************
**
** Returns the End point for the pattern match structure for index'th item.
**
** @param [r] data [EmbPPatMatch] results of match.
** @param [r] index   [ajint] index to structure.
**
** @return [ajint] returns -1 if not available.
**
******************************************************************************/

ajint embPatMatchGetEnd(EmbPPatMatch data, ajint index){

  if(data->number <= index || index < 0)
    return -1;
  return data->len[index]+data->start[index]-1;
}

/* @func   embPatMatchGetNumber***************************************************
**
** Returns the number of  pattern matchs in the structure.
**
** @param [r] data [EmbPPatMatch] results of match.
**
** @return [ajint] returns -1 if not available.
**
******************************************************************************/

ajint embPatMatchGetNumber(EmbPPatMatch data){

  return data->number;
}

/* @func  embPatMatchGetStart***************************************************
**
** Returns the start position from the pattern match structure for
** index'th item.
**
** @param [r] data [EmbPPatMatch] results of match.
** @param [r] index   [ajint] index to structure.
**
** @return [ajint] returns -1 if not available.
**
******************************************************************************/

ajint embPatMatchGetStart(EmbPPatMatch data, ajint index){

  if(data->number <= index || index < 0)
    return -1;
  return data->start[index];
}

/* @func embPatMatchDel ******************************************
**
** Free all the memory from the pattern match search.
**
** @param [rw] pthis [EmbPPatMatch*] results to be freed.
** @return [void]
******************************************************************************/

void embPatMatchDel(EmbPPatMatch* pthis){
  EmbPPatMatch thys = pthis ? *pthis : 0;

  if (!pthis) return;
  if (!*pthis) return;

  if(thys->number){
    AJFREE(thys->start);    
    AJFREE(thys->len);
  }
  AJFREE(*pthis);
}





/* @func embPatPosSeqCreateRegExp **********************************************
**
** Create a posix regular expression for a string and substitute the chars
** for Nucleotides or proteins as needed.
**
** @param [r] thys [AjPStr] string to create reg expr from.
** @param [r] protein [AjBool] is it a protein.
** 
** @return [AjPStr] the new regular expression.
******************************************************************************/

AjPStr embPatPosSeqCreateRegExp(AjPStr thys, AjBool protein) {
  return embPatPosSeqCreateRegExpC(ajStrStr(thys), protein);
}

/* @func embPatPosSeqCreateRegExpC *********************************************
**
** Create a posix regular expression for a string and substitute the chars for
** Nucleotides or proteins as needed.
**
** @param [r] ptr [char *] text to create reg expr from.
** @param [r] protein [AjBool] is it a protein.
** 
** @return [AjPStr] the new regular expression.
******************************************************************************/

AjPStr embPatPosSeqCreateRegExpC(char *ptr, AjBool protein)
{
    return embPatSeqCreateRegExpC(ptr,protein);
}


/* @func embPatPosSeqMatchFind *********************************************
**
** Find all the regular expression matches of reg in the string string.
**
** @param [r] seq [AjPSeq] Sequence to be searched.
** @param [r] reg [AjPStr] regular expression string.
**
** @return [EmbPPatMatch] Results of the pattern matching.
**
******************************************************************************/

EmbPPatMatch embPatPosSeqMatchFind(AjPSeq seq, AjPStr reg){
  return embPatPosSeqMatchFindC(seq, ajStrStr(reg));
}

/* @func embPatPosSeqMatchFindC *********************************************
**
** Find all the posix regular expression matches of reg in the string string.
**
** @param [r] seq [AjPSeq] Sequence to be searched.
** @param [r] reg [char*] regular expression text.
**
** @return [EmbPPatMatch] Results of the pattern matching.
**
******************************************************************************/

EmbPPatMatch embPatPosSeqMatchFindC(AjPSeq seq, char *reg){
  AjPStr regexp = NULL;
  AjBool protein;
  EmbPPatMatch results;

  protein = ajSeqIsProt(seq);

  regexp = embPatPosSeqCreateRegExpC(reg,protein);
    
  results = embPatPosMatchFind(regexp, ajSeqStr(seq));

  ajStrDel(&regexp);

  return results;
  
}

/* @func embPatPosMatchFind *********************************************
**
** Find all the posix regular expression matches of reg in the string string.
**
** @param [r] regexp [AjPStr] Regular expression string.
** @param [r] string   [AjPStr] String to be searched.
**
** @return [EmbPPatMatch] Results of the pattern matching.
**
******************************************************************************/
EmbPPatMatch embPatPosMatchFind(AjPStr regexp, AjPStr string){
  return embPatPosMatchFindC(regexp, ajStrStr(string));
}


/* @func embPatPosMatchFindC *********************************************
**
** Find all the posix regular expression matches of reg in the string string.
**
** @param [r] regexp [AjPStr] Regular expression string.
** @param [r] sptr   [char *] String to be searched.
**
** @return [EmbPPatMatch] Results of the pattern matching.
**
******************************************************************************/

EmbPPatMatch embPatPosMatchFindC(AjPStr regexp, char *sptr){
  AjPPosRegexp regcomp = NULL;
  EmbPPatMatch results = NULL;
  AjPList poslist = ajListNew();
  AjPList lenlist = ajListNew();
  AjIList iter;
  ajint *pos,*len,posi,i;
  char *ptr = sptr;
  regcomp = ajPosRegComp(regexp);

  AJNEW(results);

  while(*sptr != '\0' && ajPosRegExecC(regcomp,sptr)){
    /*    ajPosRegTrace(regcomp);*/
    AJNEW (pos);
    *pos = posi = ajPosRegOffsetC(regcomp);
    AJNEW (len);
    *len = ajPosRegLenI(regcomp,0);
    *pos +=sptr-ptr;
    ajListAppend(poslist, ajListNodesNew(pos, NULL));
    ajListAppend(lenlist, ajListNodesNew(len, NULL));
    sptr += posi+1;
  }
	
  ajPosRegFree(&regcomp);
  results->number  = ajListLength(poslist);
  if(results->number){
    AJCNEW(results->start, results->number);
    AJCNEW(results->len, results->number);
    
    i=0;
    iter=ajListIter(poslist);
    while (ajListIterMore(iter)) {
      results->start[i] = *(ajint *) ajListIterNext (iter);
      i++;
    }
    ajListIterFree (iter);
    
    i=0;
    iter=ajListIter(lenlist);
    while (ajListIterMore(iter)) {
      results->len[i] = *(ajint *) ajListIterNext (iter);
      i++;
    }
    ajListIterFree (iter);
    
    ajListMap(poslist,patStringFree, NULL);
    ajListMap(lenlist,patStringFree, NULL);
    ajListFree(&poslist);
    ajListFree(&lenlist);

  }
  else{
    ajListFree(&poslist);
    ajListFree(&lenlist);
  }
  return results;
}

/* @func  embPatPosMatchGetLen*************************************************
**
** Returns the length from the posix pattern match structure for index'th item.
**
** @param [r] data [EmbPPatMatch] results of match.
** @param [r] index   [ajint] index to structure.
**
** @return [ajint] returns -1 if not available.
**
******************************************************************************/

ajint embPatPosMatchGetLen(EmbPPatMatch data, ajint index)
{
    return embPatMatchGetLen(data,index);
}


/* @func  embPatPosMatchGetEnd*************************************************
**
** Returns the End point for the posix pattern match structure for index'th
** item.
**
** @param [r] data [EmbPPatMatch] results of match.
** @param [r] index   [ajint] index to structure.
**
** @return [ajint] returns -1 if not available.
**
******************************************************************************/

ajint embPatPosMatchGetEnd(EmbPPatMatch data, ajint index)
{
    return embPatMatchGetEnd(data,index);
}




/* @func   embPatPosMatchGetNumber*********************************************
**
** Returns the number of posix pattern matches in the structure.
**
** @param [r] data [EmbPPatMatch] results of match.
**
** @return [ajint] returns -1 if not available.
**
******************************************************************************/

ajint embPatPosMatchGetNumber(EmbPPatMatch data)
{
    return embPatMatchGetNumber(data);
}



/* @func  embPatPosMatchGetStart***********************************************
**
** Returns the start position from the posix pattern match structure for
** index'th item.
**
** @param [r] data [EmbPPatMatch] results of match.
** @param [r] index   [ajint] index to structure.
**
** @return [ajint] returns -1 if not available.
**
******************************************************************************/

ajint embPatPosMatchGetStart(EmbPPatMatch data, ajint index)
{
    return embPatMatchGetStart(data,index);
}


/* @func embPatPosMatchDel ******************************************
**
** Free all the memory from a posix pattern match search.
**
** @param [rw] pthis [EmbPPatMatch*] results to be freed.
** @return [void]
******************************************************************************/

void embPatPosMatchDel(EmbPPatMatch* pthis)
{
    embPatMatchDel(pthis);
}



/* @func embPatPrositeToRegExp       ******************************************
**
** Convert a prosite pattern to a regular expression
**
** @param [r] s [AjPStr *] prosite pattern
** @return [AjPStr] regular expression
******************************************************************************/

AjPStr embPatPrositeToRegExp(AjPStr *s)
{
    AjPStr t;
    AjPStr c;
    AjBool isnt;
    AjBool isct;
    
    char   *p;
    static char *aa="ACDEFGHIKLMNPQRSTVWY";
    static char ch[2];
    ajint len;
    ajint i;
    
    t = ajStrNewC("");
    len=ajStrLen(*s);
    if(!len) return t;

    c = ajStrNew();
    (void) ajStrAss(&c, *s);
    (void) ajStrToUpper(&c);
    (void) ajStrClean(&c);
    ch[1]='\0';
    
    p = ajStrStr(c);
    for(i=isnt=isct=0;i<len;++i)
    {
	if(p[i]=='>') isct=1;
	if(p[i]=='<') isnt=1;
    }
    if(isnt) (void) ajStrAppC(&t,"^");

    while(*p)
    {
	if(*p=='?')
	{
	    (void) ajStrAppC(&t,"[^#]");
	    ++p;
	    continue;
	}
	
	if(*p=='X')
	{
	    (void) ajStrAppC(&t,"[^BJOUXZ]");
	    ++p;
	    continue;
	}
	if(*p=='(')
	{
	    ++p;
	    (void) ajStrAppC(&t,"{");
	    while(*p != ')')
	    {
		if(!*p)
		    ajFatal("Unmatched '(' in %s\n",ajStrStr(*s));
		if(*p=='<' || *p=='>')
		{
		    ++p;
		    continue;
		}
		*ch= *p;
		(void) ajStrAppC(&t,ch);
		++p;
	    }
	    (void) ajStrAppC(&t,"}");
	    ++p;
	    continue;
	}
	if(*p=='[')
	{
	    while(*p != ']')
	    {
		if(!*p)
		    ajFatal("Unmatched '[' in %s\n",ajStrStr(*s));
		if(*p=='<' || *p=='>')
		{
		    ++p;
		    continue;
		}
		*ch= *p;
		(void) ajStrAppC(&t,ch);
		++p;
	    }
	    /*if(*(p+1)!='(')*/ (void) ajStrAppC(&t,"]");
	    ++p;
	    continue;
	}
	if(*p=='{')
	{
	    ++p;
	    (void) ajStrAppC(&t,"[^");
	    while(*p != '}')
	    {
		if(!*p)
		    ajFatal("Unmatched '{' in %s\n",ajStrStr(*s));
		if(*p=='<' || *p=='>')
		{
		    ++p;
		    continue;
		}
		*ch= *p;
		(void) ajStrAppC(&t,ch);
		++p;
	    }
	    /*if(*(p+1)!='(')*/ (void) ajStrAppC(&t,"]");
	    ++p;
	    continue;
	}

	if(strchr(aa,*p))
	{
	    *ch= *p;
	    (void) ajStrAppC(&t,ch);
	    ++p;
	    continue;
	}
	
	if(!(*p==' ' || *p=='-' || *p=='>' || *p=='<'))
	    ajFatal("Unrecognised character in %s\n",ajStrStr(*s));
	++p;
    }

    if(isct) (void) ajStrAppC(&t,"$");
    (void) ajStrAssC(&c,ajStrStr(t));
    ajStrDel(&t);
    return c;
}



/* @func embPatRestrictNew ****************************************************
**
** Create a new restriction object
** 
** @return [EmbPPatRestrict] the allocated object
******************************************************************************/

EmbPPatRestrict embPatRestrictNew(void)
{
    EmbPPatRestrict thys;

    AJNEW0 (thys);

    thys->cod  = ajStrNew();
    thys->pat  = ajStrNew();
    thys->bin  = ajStrNew();
    thys->org  = ajStrNew();
    thys->iso  = ajStrNew();
    thys->meth = ajStrNew();
    thys->sou  = ajStrNew();
    thys->sup  = ajStrNew();

    return thys;
}




/* @func embPatRestrictDel ****************************************************
**
** Delete a restriction object
**
** @param [r] thys [EmbPPatRestrict *] restriction object
** @return [void]
******************************************************************************/

void embPatRestrictDel(EmbPPatRestrict *thys)
{
    ajStrDel(&(*thys)->cod);
    ajStrDel(&(*thys)->pat);
    ajStrDel(&(*thys)->bin);
    ajStrDel(&(*thys)->org);
    ajStrDel(&(*thys)->iso);
    ajStrDel(&(*thys)->meth);
    ajStrDel(&(*thys)->sou);
    ajStrDel(&(*thys)->sup);
    AJFREE (*thys);
}




/* @func embPatRestrictReadEntry **********************************************
**
** Read next restriction enzyme from re file
**
** @param [w] re [EmbPPatRestrict *] restriction object to fill
** @param [r] inf [AjPFile *] input file pointer
** @return [AjBool] True if read successful
******************************************************************************/

AjBool embPatRestrictReadEntry(EmbPPatRestrict *re, AjPFile *inf)
{
    AjPStr line;
    AjBool ret;
    char   *p=NULL;
    ajint    i;
    
    line = ajStrNew();
    while((ret=ajFileReadLine(*inf,&line)))
    {
	p = ajStrStr(line);
	if(!(!*p || *p=='#' || *p=='!')) break;
    }

    if(!ret)
    {
	ajStrDel(&line);
	return ajFalse;
    }


    p=ajSysStrtok(p,"\t \n");
    (void) ajStrAssC(&(*re)->cod,p);
    p=ajSysStrtok(NULL,"\t \n");
    (void) ajStrAssC(&(*re)->pat,p);
    (void) ajStrAssC(&(*re)->bin,p);

    p=ajSysStrtok(NULL,"\t \n");
    (void) sscanf(p,"%d",&(*re)->len);
    p=ajSysStrtok(NULL,"\t \n");
    (void) sscanf(p,"%d",&(*re)->ncuts);
    p=ajSysStrtok(NULL,"\t \n");
    (void) sscanf(p,"%d",&(*re)->blunt);
    p=ajSysStrtok(NULL,"\t \n");
    (void) sscanf(p,"%d",&(*re)->cut1);
    p=ajSysStrtok(NULL,"\t \n");
    (void) sscanf(p,"%d",&(*re)->cut2);
    p=ajSysStrtok(NULL,"\t \n");
    (void) sscanf(p,"%d",&(*re)->cut3);
    p=ajSysStrtok(NULL,"\t \n");
    (void) sscanf(p,"%d",&(*re)->cut4);

    for(i=0,p=ajStrStr((*re)->bin);i<(*re)->len;++i)
	*(p+i)=ajAZToBinC(*(p+i));

    ajStrDel(&line);
    
    return ajTrue;
}



/* @funcstatic patRestrictPushHit *******************************************
**
** Put a matching restriction enzyme on the heap
** as an EmbPMatMatch structure
**
** @param [r] enz [EmbPPatRestrict *] Enyme information
** @param [w] l [AjPList *] List to add to
** @param [r] pos [ajint] Sequence match position
** @param [r] begin [ajint] Sequence offset
** @param [r] len [ajint] Sequence length
** @param [r] forward [AjBool] True if forward strand
**
** @return [void]
******************************************************************************/

static void patRestrictPushHit(EmbPPatRestrict *enz, AjPList *l, ajint pos,
			      ajint begin, ajint len, AjBool forward)
{

    EmbPMatMatch hit;
    ajint v;
    
    AJNEW0(hit);

    hit->seqname = ajStrNew();
    hit->cod = ajStrNewC(ajStrStr((*enz)->cod));
    hit->pat = ajStrNewC(ajStrStr((*enz)->pat));
    hit->acc = ajStrNew();
    hit->tit = ajStrNew();
    hit->iso = ajStrNew();
    hit->len = (*enz)->len;

    if(forward)
    {
	hit->forward = 1;
	hit->start = pos+begin;
	hit->cut1 = pos+begin+(*enz)->cut1-1;
	hit->cut2 = pos+begin+(*enz)->cut2-1;
	if(hit->cut1>len+begin-1) hit->cut1-=len;
	if(hit->cut2>len+begin-1) hit->cut2-=len;

 	if((*enz)->cut1<1) ++hit->cut1;
	if((*enz)->cut2<1) ++hit->cut2;

	if(hit->cut1<1) hit->cut1+=len;
	if(hit->cut2<1) hit->cut2+=len;


	if((*enz)->ncuts == 4)
	{
	    hit->cut3 = pos+begin+(*enz)->cut3-1;
	    hit->cut4 = pos+begin+(*enz)->cut4-1;
	    if(hit->cut3>len+begin-1) hit->cut3-=len;
	    if(hit->cut4>len+begin-1) hit->cut4-=len;
	}
	else hit->cut3 = hit->cut4 = 0;
    }
    else
    {
	hit->forward=0;
	hit->start = len+begin-pos-1;
	hit->cut1 = len+begin-pos-(*enz)->cut1-1;
	hit->cut2 = len+begin-pos-(*enz)->cut2-1;

	if((*enz)->cut1<1) --hit->cut1;
	if((*enz)->cut2<1) --hit->cut2;

	if(hit->cut1<1) hit->cut1+=len;
	if(hit->cut2<1) hit->cut2+=len;

	if(hit->cut1>len+begin-1) hit->cut1-=len;
	if(hit->cut2>len+begin-1) hit->cut2-=len;

	if((*enz)->ncuts == 4)
	{
	    hit->cut3 = len+begin-pos-(*enz)->cut3-1;
	    hit->cut4 = len+begin-pos-(*enz)->cut4-1;
	    if(hit->cut3<0) hit->cut3+=len;
	    if(hit->cut4<0) hit->cut4+=len;
	}
	else hit->cut3 = hit->cut4 = 0;

	/* Reverse them to refer to forward strand */
	v=hit->cut1;
	hit->cut1=hit->cut2;
	hit->cut2=v;
	v=hit->cut3;
	hit->cut3=hit->cut4;
	hit->cut4=v;
    }

    ajListPush(*l,(void *) hit);

    return;
}




/* @func embPatRestrictScan ***************************************************
**
** Scan a sequence with a restriction object
**
** @param [r] enz [EmbPPatRestrict *] Enyme information
** @param [r] substr [AjPStr *] Sequence as ASCII
** @param [r] binstr [AjPStr *] Sequence as binary IUB
** @param [r] revstr [AjPStr *] Sequence as ASCII reversed
** @param [r] binrev [AjPStr *] Sequencd as binary IUB reversed
** @param [r] len [ajint] Length of sequence
** @param [r] ambiguity [AjBool] Allow ambiguity (binary search)
** @param [r] plasmid [AjBool] Allow circular DNA
** @param [r] min [ajint] Minimum # of matches allowed
** @param [r] max [ajint] Maximum # of matches
** @param [r] begin [ajint] Sequence offset
** @param [w] l [AjPList *] List to push hits to
**
** @return [ajint] Number of matches
******************************************************************************/

ajint embPatRestrictScan(EmbPPatRestrict *enz, AjPStr *substr, AjPStr *binstr,
		       AjPStr *revstr, AjPStr *binrev, ajint len,
		       AjBool ambiguity, AjBool plasmid, ajint min,
		       ajint max, ajint begin, AjPList *l)
{
    ajint limit;
    ajint i;
    ajint j;
    ajint hits;
    ajint rhits=0;
    char *p;
    char *q;
    char *t;
    ajint  mincut;
    ajint  maxcut;
    AjBool forward;
    ajint  v;
    AjPList tx=NULL;
    AjPList ty=NULL;
    EmbPMatMatch m=NULL;
    EmbPMatMatch z=NULL;
    
    if(plasmid) limit=len;
    else limit=len-(*enz)->len+1;

    mincut=AJMIN((*enz)->cut1,(*enz)->cut2);
    if((*enz)->ncuts==4)
    {
	mincut=AJMIN(mincut,(*enz)->cut3);
	mincut=AJMIN(mincut,(*enz)->cut4);
    }
    maxcut=AJMAX((*enz)->cut1,(*enz)->cut2);
    if((*enz)->ncuts==4)
    {
	maxcut=AJMAX(maxcut,(*enz)->cut3);
	maxcut=AJMAX(maxcut,(*enz)->cut4);
    }

    tx = ajListNew();
    ty = ajListNew();
    

    if(ambiguity)
    {
	p = ajStrStr(*binstr);
	t = ajStrStr((*enz)->bin);

	forward = ajTrue;
	for(i=0,hits=0;i<limit;++i)
	{
	    for(j=0,q=t;j<(*enz)->len;++j,++q)
	    {
		v=*(p+i+j);
		if(!(*q & v) || v==15) break;
	    }
	    if(j==(*enz)->len && !plasmid && (i+(*enz)->cut1>=len ||
					      i+(*enz)->cut2>=len))
		continue;

	    if(j==(*enz)->len && (plasmid || i+mincut+1>0) && i<limit)
	    {
		++hits;
		patRestrictPushHit(enz,&tx,i,begin,len,forward);
	    }
	
	}

	forward=ajFalse;
	p=ajStrStr(*binrev);
	for(i=0;i<limit;++i)
	{
	    for(j=0,q=t;j<(*enz)->len;++j,++q)
	    {
		v=*(p+i+j);
		if(!(*q & v) || v==15) break;
	    }
	    if(j==(*enz)->len && !plasmid && (i+(*enz)->cut1>=len ||
					      i+(*enz)->cut2>=len))
		continue;

	    if(j==(*enz)->len && (plasmid || i+mincut+1>0) && i<limit)
	    {
		++hits;
		patRestrictPushHit(enz,&tx,i,begin,len,forward);
	    }
	
	}

    }
    else
    {
	p = ajStrStr(*substr);
	t = ajStrStr((*enz)->pat);
	forward = ajTrue;
	for(i=0,hits=0;i<limit;++i)
	{
	    for(j=0,q=t;j<(*enz)->len;++j,++q)
	    {
		v=*(p+i+j);
		if(*q != v || v=='N') break;
	    }
	    if(j==(*enz)->len && !plasmid && (i+(*enz)->cut1>=len ||
					      i+(*enz)->cut2>=len))
		continue;
	    if(j==(*enz)->len && (plasmid || i+mincut+1>0) && i<limit)
	    {
		++hits;
		patRestrictPushHit(enz,&tx,i,begin,len,forward);
	    }
	
	}

	forward=ajFalse;
	p=ajStrStr(*revstr);
	for(i=0;i<limit;++i)
	{
	    for(j=0,q=t;j<(*enz)->len;++j,++q)
	    {
		v=*(p+i+j);
		if(*q != v || v=='N') break;
	    }
	    if(j==(*enz)->len && !plasmid && (i+(*enz)->cut1>=len ||
					      i+(*enz)->cut2>=len))
		continue;
	    if(j==(*enz)->len && (plasmid || i+mincut+1>0) && i<limit)
	    {
		++hits;
		patRestrictPushHit(enz,&tx,i,begin,len,forward);
	    }
	}
    }	


    if(hits)
    {
	ajListSort(tx,patRestrictCutCompare);
	for(i=0,rhits=0,v=0;i<hits;++i)
	{
	    (void) ajListPop(tx,(void **)&m);
	    if(m->cut1 != v)
	    {
		(void) ajListPush(ty,(void *)m);
		++rhits;
		v = m->cut1;
	    }
	    else
	    {
		if(i)
		{
		    if(m->forward)
		    {
			ajListPop(ty,(void **)&z);
			ajListPush(ty,(void *)m);
			m=z;
		    }
		}
		embMatMatchDel(&m);
	    }
	}

	if(rhits<min || rhits>max)
	{
	    while(ajListPop(ty,(void **)&m));
	    ajListDel(&tx);
	    ajListDel(&ty);
	    return 0;
	}
	else
	{
	    while(ajListPop(ty,(void **)&m))
		ajListPush(*l,(void *)m);
	    hits=rhits;
	}
    }

    ajListDel(&tx);
    ajListDel(&ty);
    
    return hits;
}




/* @func embPatKMPInit ******************************************************
**
** Initialise a Knuth-Morris-Pratt pattern.
**
** @param [r] pat [AjPStr *] pattern
** @param [r] len [ajint] length of pattern
** @param [w] next [int *] offset table
**
** @return [void]
******************************************************************************/

void embPatKMPInit(AjPStr *pat, ajint len, ajint *next)
{
    ajint i;
    ajint k;
    ajint t;
    char *p;
    
    p=ajStrStr(*pat);
    t=len-1;
    
    i=0;
    k = -1;
    next[0] = -1;
    while(i<t)
    {
	while(k>-1 && p[i]!=p[k]) k=next[k];
	++i;
	++k;
	if(p[i]==p[k]) next[i]=next[k];
	else next[i]=k;
    }

    return;
}




/* @func embPatKMPSearch *****************************************************
**
** Perform a Knuth-Morris-Pratt search
**
** @param [r] str [AjPStr *] string to search
** @param [r] pat [AjPStr *] pattern to use
** @param [r] slen [ajint] length of string
** @param [r] plen [ajint] length of pattern
** @param [r] next [int *] array from embPatKMPInit
** @param [r] start [ajint] position within str to start search
**
** @return [ajint] Index of match in str or -1 if not found
******************************************************************************/

ajint embPatKMPSearch(AjPStr *str, AjPStr *pat, ajint slen, ajint plen, ajint *next,
		   ajint start)
{
    ajint i;
    ajint j;
    char *p;
    char *q;

    p = ajStrStr(*str);
    q = ajStrStr(*pat);
    
    i=start;
    j=0;
    while(i<slen && j<plen)
    {
	while(j>=0 && p[i]!=q[j]) j=next[j];
	++i;
	++j;
    }
    
    if(j==plen) return i-plen;
    return -1;
}




/* @func embPatBMHInit ******************************************************
**
** Initialise a Boyer-Moore-Horspool pattern.
**
** @param [r] pat [AjPStr *] pattern
** @param [r] len [ajint] pattern length
** @param [w] skip [int *] offset table
**
** @return [void]
******************************************************************************/

void embPatBMHInit(AjPStr *pat, ajint len, ajint *skip)
{
    ajint i;
    ajint t;
    char *p;

    p=ajStrStr(*pat);

    t=len-1;
    for(i=0;i<AJALPHA;++i) skip[i]=t;
    for(i=0;i<t;++i) skip[(ajint)p[i]]=t-i;

    return;
}




/* @func embPatBMHSearch *****************************************************
**
** Perform a Boyer-Moore-Horspool search
**
** @param [r] str [AjPStr *] string to search
** @param [r] pat [AjPStr *] pattern to use
** @param [r] slen [ajint] length of string
** @param [r] plen [ajint] length of pattern
** @param [r] skip [int *] array from embPatBMHInit
** @param [r] start [ajint] position within str to start search
** @param [r] left [AjBool] has to match the start
** @param [r] right [AjBool] has to match the end
** @param [w] l [AjPList *] list to push to
** @param [r] name [AjPStr *] name of entry
** @param [r] begin [ajint] offset in orig sequence
**
** @return [ajint] number of hits
******************************************************************************/

ajint embPatBMHSearch(AjPStr *str, AjPStr *pat, ajint slen, ajint plen, ajint *skip,
		   ajint start, AjBool left, AjBool right, AjPList *l,
		   AjPStr *name, ajint begin)
{
    ajint i;
    ajint j;
    ajint k=0;
    char *p;
    char *q;
    AjBool flag;
    ajint count;
    
    if(left && start) return 0;

    p = ajStrStr(*str);
    q = ajStrStr(*pat);

    flag = ajTrue;
    count = 0;
    
    i=start+(plen-1);
    j=plen-1;

    while(flag)
    {
	while(j>=0 && i<slen)
	{
	    k=i;
	    while(j>=0 && p[k]==q[j])
	    {
		--k;
		--j;
	    }
	    if(j>=0)
	    {
		i+=skip[(ajint)p[i]];
		j=plen-1;
	    }
	}

	if(j<0)
	{
	    if(left && k+1) return 0;
	    if(!right || (right && k+1+plen==slen))
	    {
		++count;
		embPatPushHit(*l,name,k+1,plen,begin,0);
	    }
	    i=start+(plen-1)+k+2;
	    j=plen-1;
	       
	}
	else flag=ajFalse;
    }
    
    
    return count;
}




/* @func embPatBYPInit ******************************************************
**
** Initialise a Baeza-Yates,Perleberg pattern.
**
** @param [r] pat [AjPStr *] pattern
** @param [r] len [ajint] pattern length
** @param [w] offset [EmbPPatBYPNode] character index
** @param [w] buf [int *] mismatch count
**
** @return [void]
******************************************************************************/

void embPatBYPInit(AjPStr *pat, ajint len, EmbPPatBYPNode offset, ajint *buf)
{
    ajint i;
    ajint j;
    
    char *p;
    EmbPPatBYPNode op;
    
    p=ajStrStr(*pat);

    for(i=0;i<AJALPHA;++i)
    {
	offset[i].offset = -1;
	offset[i].next = NULL;
	buf[i]=len;
    }
    
    for(i=0,j=AJALPHA>>1;i<len;++i,++p)
    {
	buf[i]=AJALPHA;
	if(offset[(ajint)*p].offset == -1) offset[(ajint)*p].offset = len-i-1;
	else
	{
	    op=offset[(ajint)*p].next;
	    offset[(ajint)*p].next=&offset[j++];
	    offset[(ajint)*p].next->offset = len-i-1;
	    offset[(ajint)*p].next->next = op;
	}
    }

    return;
}



/* @func embPatPushHit *******************************************************
**
** Put a matching BYP search hit on the heap
** as an EmbPMatMatch structure
**
** @param [w] l [AjPList] list to push to
** @param [r] name [AjPStr *] string name
** @param [r] pos [ajint] Sequence match position
** @param [r] plen [ajint] pattern length
** @param [r] begin [ajint] Sequence offset
** @param [r] mm [ajint] number of mismatches
**
** @return [void]
******************************************************************************/

void embPatPushHit(AjPList l, AjPStr *name, ajint pos, ajint plen,
		 ajint begin, ajint mm)
{
    EmbPMatMatch hit;

    AJNEW0 (hit);

    hit->seqname = ajStrNewC(ajStrStr(*name));
    hit->len = plen;
    hit->cod = ajStrNew();
    hit->pat = ajStrNew();
    hit->acc = ajStrNew();
    hit->tit = ajStrNew();
    hit->start = pos+begin;
    hit->mm = mm;
    hit->end = pos+begin+plen-1;    
    ajListPush(l,(void *) hit);

    return;
}




/* @func embPatBYPSearch ******************************************************
**
** Perform a Baeza-Yates,Perleberg search.
**
** @param [r] str [AjPStr *] search string
** @param [r] name [AjPStr *] search string
** @param [r] begin [ajint] sequence offset
** @param [r] slen [ajint] string length
** @param [r] plen [ajint] pattern length
** @param [r] mm [ajint] allowed mismatches (Hamming distance)
** @param [r] offset [EmbPPatBYPNode] character index
** @param [r] buf [int *] mismatch count array
** @param [w] l [AjPList] list to push hits to
** @param [r] amino [AjBool] if true, match at amino terminal end
** @param [r] carboxyl [AjBool] if true, match at carboxyl terminal end
** @param [r] pat [AjPStr] original pattern
**
** @return [ajint] number of matches
******************************************************************************/

ajint embPatBYPSearch(AjPStr *str, AjPStr *name, ajint begin, ajint slen,
		   ajint plen, ajint mm, EmbPPatBYPNode offset, ajint *buf,
		   AjPList l, AjBool amino, AjBool carboxyl, AjPStr pat)
{
    char *p;
    char *q;
    ajint  i;
    ajint  t;
    EmbPPatBYPNode off;
    ajint count;
    
    p=ajStrStr(*str);
    ajStrToUpper(&pat);
    q=ajStrStr(pat);
    
    count = mm;
    for(i=0;i<plen;++i)
	if(*q++!=*p++)
	    if(--count<0)
		break;
    if(count>=0)
    {
	embPatPushHit(l,name,0,plen,begin,mm-count);
	count=1;
    }
    else count=0;

    p=ajStrStr(*str);

    for(i=0;i<slen;++i)
    {
	if((t=(off=&offset[(ajint)*p++])->offset)>=0)
	{
	    buf[(i+t)&AJMOD256]--;
	    for(off=off->next;off!=NULL;off=off->next)
		buf[(i+off->offset)&AJMOD256]--;
	}
	if(buf[i&AJMOD256]<=mm)
	{
	    if(amino && i-plen+1!=0) return 0;
	    if(!carboxyl || (carboxyl && i+1==slen))
	    {
		++count;
		embPatPushHit(l,name,i-plen+1,plen,begin,buf[i&AJMOD256]);
	    }
	}
	
	buf[i&AJMOD256]=plen;
    }

    return count;
}


/* @funcstatic patAminoCarboxyl *******************************************
**
** Checks for start and/or end angle bracket markers
** Removes them from the string and sets bools accordingly
**
** @param [rw] s [AjPStr *] pattern
** @param [w] amino [AjBool *] set if start marker (left angle bracket)
** @param [w] carboxyl [AjBool *] set if end marker (right angle bracket)
** 
** @return [void]
******************************************************************************/

static void patAminoCarboxyl(AjPStr *s, AjBool *amino, AjBool *carboxyl)
{
    AjPStr t;
    char *p;
    char c[2];

    t=ajStrNewC("");
    p=ajStrStr(*s);
    c[1]='\0';
    
    while(*p)
    {
	if(*p==' ' || *p=='-' || *p=='.')
	{
	    ++p;
	    continue;
	}
	if(*p=='<')
	{
	    *amino=ajTrue;
	    ++p;
	    continue;
	}
	if(*p=='>')
	{
	    *carboxyl=ajTrue;
	    ++p;
	    continue;
	}
	
	*c=*p;
	(void) ajStrAppC(&t,c);
	++p;
    }
    (void) ajStrAssC(s,ajStrStr(t));
    ajStrDel(&t);
    return;
}


/* @funcstatic patParenTest *********************************************
**
** Checks parenthesis grammer. Sets repeat and range bools
**
** @param [r] p [char *] pattern
** @param [w] repeat [AjBool *] set if any parenthesis e.g. (3)
** @param [w] range [AjBool *] set if range e.g. (5,8)
** 
** @return [AjBool] True if grammar correct
******************************************************************************/

static AjBool patParenTest(char *p, AjBool *repeat, AjBool *range)
{
    ajint i;
    
    *repeat=ajTrue;
    p=p+2;
    if(sscanf(p,"%d",&i)!=1)
    {
	ajWarn("Illegal pattern. Missing repeat number");
	return ajFalse;
    }
	    
    while(*p)
    {
	if(*p==')') break;
	if(*p=='('||*p=='['||*p=='{'||*p=='}'||*p==']' ||
	   isalpha((ajint)*p))
	{
	    ajWarn("Illegal pattern. Nesting not allowed");
	    return ajFalse;
	}
	if(*p==',')
	{
	    *range=ajTrue;
	    ++p;
	    if(sscanf(p,"%d",&i)!=1)
	    {
		ajWarn("Illegal pattern. Missing range number");
		return ajFalse;
	    }
	    continue;
	}
	++p;
    }
    if(!*p)
    {
	ajWarn("Illegal pattern. Missing parenthesis");
	return ajFalse;
    }
    return ajTrue;
}    



/* @funcstatic patExpandRepeat *********************************************
**
** Expand repeats e.g. [ABC](2) to [ABC][ABC]
**
** @param [rw] s [AjPStr *] pattern
**
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool patExpandRepeat(AjPStr *s)
{
    AjPStr t;
    char *p;
    char *q;
    ajint count;
    ajint i;
    char c[2];

    t=ajStrNewC("");
    p=ajStrStr(*s);
    c[1]='\0';

    while(*p)
    {
	if(*p=='[' || *p=='{')
	{
	    q=p;
	    while(!(*p==']' || *p=='}')) ++p;
	    if(*(p+1)!='(') count=1;
	    else (void) sscanf(p+2,"%d",&count);
	    if(count<=0)
	    {
		ajWarn("Illegal pattern. Bad repeat count");
		return ajFalse;
	    }
	    for(i=0;i<count;++i)
	    {
		p=q;
		while(!(*p==']'||*p=='}'))
		{
		    *c=*p;
		    (void) ajStrAppC(&t,c);
		    ++p;
		}
		*c=*p;
		(void) ajStrAppC(&t,c);
	    }
	    if(*(p+1)=='(')
		while(*p!=')') ++p;
	    
	    ++p;
	    continue;
	}

	if(*p=='(')
	{
	    (void) sscanf(p+1,"%d",&count);
	    if(count<=0)
	    {
		ajWarn("Illegal pattern. Bad range number");
		return ajFalse;
	    }
	    *c=*(p-1);
	    for(i=1;i<count;++i) (void) ajStrAppC(&t,c);
	    while(*p!=')') ++p;
	    ++p;
	    continue;
	}

	*c=*p;
	(void) ajStrAppC(&t,c);
	++p;
    }

    (void) ajStrAssC(s,ajStrStr(t));
    ajStrDel(&t);
    return ajTrue;
}


/* @funcstatic patIUBTranslate *********************************************
**
** Convert IUB symbols to classes e.g. S to [GC]
**
** @param [rw] pat [AjPStr *] pattern
**
** @return [void]
******************************************************************************/

static void patIUBTranslate(AjPStr *pat)
{
    AjPStr t;
    char   *p;
    char   c[2];
    
    t=ajStrNewC(ajStrStr(*pat));
    p=ajStrStr(t);
    (void) ajStrClear(pat);
    c[1]='\0';
    
    while(*p)
    {
	if(*p=='B')
	{
	    (void) ajStrAppC(pat,"[TGC]");
	    ++p;
	    continue;
	}
	if(*p=='D')
	{
	    (void) ajStrAppC(pat,"[TGA]");
	    ++p;
	    continue;
	}
	if(*p=='H')
	{
	    (void) ajStrAppC(pat,"[TCA]");
	    ++p;
	    continue;
	}
	if(*p=='K')
	{
	    (void) ajStrAppC(pat,"[TG]");
	    ++p;
	    continue;
	}
	if(*p=='M')
	{
	    (void) ajStrAppC(pat,"[CA]");
	    ++p;
	    continue;
	}
	if(*p=='R')
	{
	    (void) ajStrAppC(pat,"[GA]");
	    ++p;
	    continue;
	}
	if(*p=='S')
	{
	    (void) ajStrAppC(pat,"[GC]");
	    ++p;
	    continue;
	}
	if(*p=='V')
	{
	    (void) ajStrAppC(pat,"[GCA]");
	    ++p;
	    continue;
	}
	if(*p=='W')
	{
	    (void) ajStrAppC(pat,"[TA]");
	    ++p;
	    continue;
	}
	if(*p=='Y')
	{
	    (void) ajStrAppC(pat,"[TC]");
	    ++p;
	    continue;
	}
	*c=*p;
	(void) ajStrAppC(pat,c);
	++p;
    }

    
    ajStrDel(&t);
    return;
}




/* @func embPatClassify *******************************************************
**
** Classify patterns according to type
**
** @param [rw] pat     [AjPStr *] pattern
** @param [w] amino    [AjBool*] set if must match start of sequence
** @param [w] carboxyl [AjBool*] set if must match end of sequence
** @param [w] fclass   [AjBool*] set if class e.g. [ABC]
** @param [w] ajcompl  [AjBool*] set if complement e.g. {ABC}
** @param [w] dontcare [AjBool*] set if X (protein) or N (DNA)
** @param [w] range    [AjBool*] set if range specified e.g. (3,10)
** @param [r] protein  [AjBool] true if protein false if DNA
**
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool embPatClassify(AjPStr *pat, AjBool *amino, AjBool *carboxyl,
		     AjBool *fclass, AjBool *ajcompl, AjBool *dontcare,
		     AjBool *range, AjBool protein)
{
    char *p;
    AjBool repeat;
    
    repeat=*amino=*carboxyl=*fclass=*ajcompl=*dontcare=*range=ajFalse;

    (void) ajStrClean(pat);
    patAminoCarboxyl(pat,amino,carboxyl);
    
    (void) ajStrToUpper(pat);

    if(!protein)
	patIUBTranslate(pat);
    p=ajStrStr(*pat);    
    
    while(*p)
    {
	if(isalpha((ajint)*p))
	{
	    if( (*p=='X' && protein) || (*p=='N' && !protein))
	    {
		*p='?';
		*dontcare=ajTrue;
	    }
	    
	    if(*(p+1)=='(')
	    {
		if(!patParenTest(p,&repeat,range)) return ajFalse;
		while(*p!=')') ++p;
	    }
	    ++p;
	    continue;
	}
	


	if(*p=='[')
	{
	    *fclass=ajTrue;
	    ++p;
	    while(*p)
	    {
		if(*p==']')break;
		if(*p=='('||*p=='['||*p=='{'||*p=='}'||*p==')')
		{
		    ajWarn("Illegal pattern. Nesting not allowed");
		    return ajFalse;
		}
		if(!isalpha((ajint)*p))
		{
		    ajWarn("Illegal pattern. Non alpha character");
		    return ajFalse;
		}
		if((protein&&*p=='X')||(!protein&&*p=='N'))
		{
		    ajWarn("Illegal pattern. Dontcare in []");
		    return ajFalse;
		}
		
		++p;
	    }
	    if(!*p)
	    {
		ajWarn("Illegal pattern. Missing ]");
		return ajFalse;
	    }
	    
	    if(*(p+1)=='(')
	    {
		if(!patParenTest(p,&repeat,range)) return ajFalse;
		while(*p!=')') ++p;
	    }
	    
	    ++p;
	    continue;
	}
	
	if(*p=='{')
	{
	    *ajcompl=ajTrue;
	    ++p;
	    while(*p)
	    {
		if(*p=='}')break;
		if(*p=='('||*p=='['||*p=='{'||*p==']'||*p==')')
		{
		    ajWarn("Illegal pattern. Nesting not allowed");
		    return ajFalse;
		}
		if(!isalpha((ajint)*p))
		{
		    ajWarn("Illegal pattern. Non alpha character");
		    return ajFalse;
		}
		if((protein&&*p=='X')||(!protein&&*p=='N'))
		{
		    ajWarn("Illegal pattern. Dontcare in {}");
		    return ajFalse;
		}
		
		++p;
	    }
	    if(!*p)
	    {
		ajWarn("Illegal pattern. Missing }");
		return ajFalse;
	    }
	    
	    if(*(p+1)=='(')
	    {
		if(!patParenTest(p,&repeat,range)) return ajFalse;
		while(*p!=')') ++p;
	    }
	    
	    ++p;
	    continue;
	}

	ajWarn("Illegal character [%c]",*p);
	return ajFalse;
    }

    if(repeat && !*range)
	if(!patExpandRepeat(pat)) return ajFalse;

    return ajTrue;
}






/* @func embPatSOInit ******************************************************
**
** Initialise a Shift-Or pattern.
**
** @param [r] pat [AjPStr *] pattern
** @param [w] table [ajuint *] SO table
** @param [w] limit [ajuint *] match limit
**
** @return [void]
******************************************************************************/

void embPatSOInit(AjPStr *pat, ajuint *table, ajuint *limit)
{
    ajuint  i;
    char *p;
    
    if(ajStrLen(*pat)>AJWORD)
	ajFatal("Pattern too ajlong for Shift-OR search");


    for(i=0;i<AJALPHA2;++i) table[i] = ~0;
    *limit=0;
    for(i=1,p=ajStrStr(*pat);*p;i<<=AJBPS,++p)
    {
	table[(ajint)*p] &= ~i;
	*limit |=  i;
    }

    *limit = ~(*limit>>AJBPS);
}

    
/* @func embPatSOSearch ******************************************************
**
** Perform a Shift-OR search.
**
** @param [r] str [AjPStr *] search string
** @param [r] name [AjPStr *] search string
** @param [r] first [ajuint] first char of pattern
** @param [r] begin [ajint] sequence offset
** @param [r] plen [ajint] pattern length
** @param [r] table [ajuint *] SO table
** @param [r] limit [ajuint] SO limit
** @param [w] l [AjPList] list to push hits to
** @param [r] amino [AjBool] must match start
** @param [r] carboxyl [AjBool] must match end
**
** @return [ajint] number of matches
******************************************************************************/

ajint embPatSOSearch(AjPStr *str, AjPStr *name, ajuint first, ajint begin,
		  ajint plen, ajuint *table, ajuint limit, AjPList l,
		  AjBool amino, AjBool carboxyl)
{
    register ajuint state;
    register ajuint initial;
    char *p;
    char *q;
    ajint pos;
    
    ajint matches;
    ajint slen;
    
    p=q=ajStrStr(*str);
    slen = ajStrLen(*str);
    matches = 0;
    initial = ~0;

    do
    {
	while(*p && *p != first) ++p;
	state = initial;
	do
	{
	    state = (state<<AJBPS) | table[(ajint)*p];
	    if(state < limit)
	    {
		pos=(p-q)-plen+1;
		if(amino && pos) return 0;
		if(!carboxyl || (carboxyl && pos==slen-plen))
		{
		    ++matches;
		    embPatPushHit(l,name,pos,plen,begin,0);
		}
	    }
	    ++p;
	}
	while(state!=initial);
    }
    while(*(p-1));

    return matches;
}





/* @func embPatBYGCInit ******************************************************
**
** Initialise a Baeza-Yates Gonnet class pattern.
**
** @param [r] pat [AjPStr *] pattern
** @param [w] m [int *] real pattern length
** @param [w] table [ajuint *] SO table
** @param [w] limit [ajuint *] match limit
**
** @return [void]
******************************************************************************/

void embPatBYGCInit(AjPStr *pat, ajint *m, ajuint *table,
		    ajuint *limit)
{
    char *p;
    char *q;
    ajuint initval;
    ajuint shift;
    ajint i;
    
    p=q=ajStrStr(*pat);
    initval = ~0;
    shift = 1;
    *m=0;
    *limit=0;
    
    while(*p)
    {
	if(*p=='?') initval &= ~shift;
	else if(*p=='{')
	{
	    initval &= ~shift;
	    while(*p!='}') ++p;
	}
	else if(*p=='[') while(*p!=']') ++p;
	++p;
	++*m;
	*limit |= shift;
	shift<<=AJBPS;
    }

    for(i=0;i<AJALPHA2;++i) table[i]=initval;

    p=q;

    shift=1;
    while(*p)
    {
	if(*p=='{')
	{
	    ++p;
	    while(*p!='}')
	    {
		table[(ajint)*p]|=shift;
		++p;
	    }
	}
	else if(*p=='[')
	{
	    ++p;
	    while(*p!=']')
	    {
		table[(ajint)*p]&=~shift;
		++p;
	    }
	}
	else if(*p!='?') table[(ajint)*p]&=~shift;

	shift<<=AJBPS;
	++p;
    }

    *limit = ~(*limit>>AJBPS);
}


/* @func embPatBYGSearch ******************************************************
**
** Perform a Baeza-Yates Gonnet search.
**
** @param [r] str [AjPStr *] search string
** @param [r] name [AjPStr *] search string
** @param [r] begin [ajint] sequence offset
** @param [r] plen [ajint] pattern length
** @param [r] table [ajuint *] SO table
** @param [r] limit [ajuint] SO limit
** @param [w] l [AjPList] list to push hits to
** @param [r] amino [AjBool] must match start
** @param [r] carboxyl [AjBool] must match end
**
** @return [ajint] number of matches
******************************************************************************/

ajint embPatBYGSearch(AjPStr *str, AjPStr *name, ajint begin, ajint plen,
		      ajuint *table, ajuint limit, AjPList l,
		      AjBool amino, AjBool carboxyl)
{
    register ajuint state;
    register ajuint initial;
    char *p;
    char *q;
    ajint pos;
    
    ajint matches;
    ajint slen;
    
    p=q=ajStrStr(*str);
    slen = ajStrLen(*str);
    matches = 0;
    initial = ~0;

    do
    {
	state = initial;
	do
	{
	    state = (state<<AJBPS) | table[(ajint)*p];
	    if(state < limit)
	    {
		pos=(p-q)-plen+1;
		if(amino && pos) return 0;
		if(!carboxyl || (carboxyl && pos==slen-plen))
		{
		    ++matches;
		    embPatPushHit(l,name,pos,plen,begin,0);
		}
	    }
	    ++p;
	}
	while(state!=initial && *p);
    }
    while(p-q<slen);

    return matches;
}




/* @func embPatTUInit ******************************************************
**
** Initialise a Tarhio-Ukkonen search
**
** @param [r] pat [AjPStr *] pattern
** @param [w] skipm [int **] mismatch skip array
** @param [r] m [ajint] real pattern length
** @param [r] k [ajint] allowed mismatches
**
** @return [void]
******************************************************************************/

void embPatTUInit(AjPStr *pat, ajint **skipm, ajint m, ajint k)
{
    char *p;
    ajint i;
    ajint j;
    ajint x;
    
    ajint ready[AJALPHA];
    
    p=ajStrStr(*pat);

    for(i=0;i<AJALPHA;++i)
    {
	ready[i]=m;
	for(j=m-k-1;j<m;++j) skipm[j][i]=m-k-1;
    }

    for(j=m-2;j>-1;--j)
    {
	x=AJMAX(j+1,m-k-1);
	for(i=ready[(ajint)p[j]]-1;i>=x;--i) skipm[i][(ajint)p[j]]=i-j;
	ready[(ajint)p[j]]=x;
    }
}



/* @func embPatTUSearch ******************************************************
**
** Perform a Tarhio-Ukkonen search
**
** @param [r] pat [AjPStr *] pattern
** @param [r] text [AjPStr *] text to search (incl ajcompl/class)
** @param [r] slen [ajint] length of text
** @param [r] skipm [int **] mismatch skip array
** @param [r] m [ajint] real pattern length
** @param [r] k [ajint] allowed mismatches
** @param [r] begin [ajint] text offset
** @param [w] l [AjPList] list to push to
** @param [r] amino [AjBool] true if text start
** @param [r] carboxyl [AjBool] true if text end
** @param [r] name [AjPStr *] name of text
**
** @return [ajint] number of hits
******************************************************************************/

ajint embPatTUSearch(AjPStr *pat, AjPStr *text, ajint slen, ajint **skipm, ajint m,
		  ajint k, ajint begin, AjPList l, AjBool amino,
		  AjBool carboxyl, AjPStr *name)
{
    ajint i;
    ajint j;
    ajint h;
    ajint mm;
    ajint skip;
    char *p;
    char *q;
    ajint  matches;
    
    p=ajStrStr(*pat);
    q=ajStrStr(*text);

    matches=0;

    i=m-1;
    while(i<slen)
    {
	h=i;
	j=m-1;
	skip=m-k;
	mm=0;
	while(j>-1 && mm<=k)
	{
	    if(j>=m-k-1) skip=AJMIN(skip,(ajint)skipm[j][(ajint)q[h]]);
	    if(q[h]!=p[j]) ++mm;
	    --h;
	    --j;
	}
	if(mm<=k)
	{
	    if(amino && h+1) return 0;
	    if(!carboxyl || (carboxyl && h+1==slen-m))
	    {
		++matches;
		embPatPushHit(l,name,h+1,m,begin,mm);
	    }
	}
	i+=skip;
    }
    return matches;
}



/* @func embPatTUBInit *****************************************************
**
** Initialise a Tarhio-Ukkonen-Bleasby search
**
** @param [r] pat [AjPStr *] pattern
** @param [w] skipm [int **] mismatch skip array
** @param [r] m [ajint] real pattern length
** @param [r] k [ajint] allowed mismatches
** @param [r] plen [ajint] full pattern length (incl ajcompl & class)
**
** @return [void]
******************************************************************************/

void embPatTUBInit(AjPStr *pat, ajint **skipm, ajint m, ajint k, ajint plen)
{
    char *p;
    char *q;
    char *s;
    ajint i;
    ajint j;
    ajint x;
    ajint z;
    ajint flag;
    ajint ready[AJALPHA];
    
    p=ajStrStr(*pat);

    for(i=0;i<AJALPHA;++i)
    {
	ready[i]=m;
	for(j=m-k-1;j<m;++j) skipm[j][i]=m-k-1;
    }

    p+=plen-1;
    if(*p=='}' || *p==']')
    {
	while(*p!='{' && *p!='[') --p;
	--p;
    }
    else --p;
    
    for(j=m-2;j>-1;--j)
    {
	x=AJMAX(j+1,m-k-1);
	if(*p=='?')
	{
	    for(z='A';z<='Z';++z)
	    {
		for(i=ready[z]-1;i>=x;--i) skipm[i][z]=i-j;
		ready[z]=x;
	    }
	    --p;
	    continue;
	}
	if(*p==']')
	{
	    --p;
	    while(*p!='[')
	    {
		for(i=ready[(ajint)*p]-1;i>=x;--i) skipm[i][(ajint)*p]=i-j;
		ready[(ajint)*p]=x;
		--p;
	    }
	    --p;
	    continue;
	}
	if(*p=='}')
	{
	    s=--p;
	    for(z='A';z<='Z';++z)
	    {
		q=s;
		flag=0;
		while(*q!='{')
		{
		    if(*q==z)
		    {
			flag=1;
			break;
		    }
		    --q;
		}
		if(!flag)
		{
		    for(i=ready[z]-1;i>=x;--i) skipm[i][z]=i-j;
		    ready[z]=x;
		}
	    }
	    while(*p!='{') --p;
	    --p;
	    continue;
	}

	for(i=ready[(ajint)*p]-1;i>=x;--i) skipm[i][(ajint)*p]=i-j;
	ready[(ajint)*p]=x;
	--p;
    }
    
}



/* @func embPatTUBSearch *****************************************************
**
** Perform a Tarhio-Ukkonen-Bleasby search
**
** @param [r] pat [AjPStr *] pattern
** @param [r] text [AjPStr *] text to search (incl ajcompl/class)
** @param [r] slen [ajint] length of text
** @param [r] skipm [int **] mismatch skip array
** @param [r] m [ajint] real pattern length
** @param [r] k [ajint] allowed mismatches
** @param [r] begin [ajint] text offset
** @param [w] l [AjPList] list to push to
** @param [r] amino [AjBool] true if text start
** @param [r] carboxyl [AjBool] true if text end
** @param [r] name [AjPStr *] name of text
** @param [r] plen [ajint] total pattern length
**
** @return [ajint] number of hits
******************************************************************************/

ajint embPatTUBSearch(AjPStr *pat,AjPStr *text, ajint slen, ajint **skipm, ajint m,
		   ajint k, ajint begin, AjPList l, AjBool amino,
		   AjBool carboxyl, AjPStr *name, ajint plen)
{
    ajint i;
    ajint j;
    ajint h;
    ajint mm;
    ajint skip;
    char *p;
    char *q;
    char *s;
    
    ajint  matches;
    ajint  a;
    ajint  flag;
    
    s=ajStrStr(*pat);
    q=ajStrStr(*text);

    matches=0;

    i=m-1;
    while(i<slen)
    {
	h=i;
	j=m-1;
	p = s+plen-1;
	skip=m-k;
	mm=0;
	while(j>-1 && mm<=k)
	{
	    if(j>=m-k-1) skip=AJMIN(skip,skipm[j][(ajint)q[(ajint)h]]);
	    a=q[h];
	    if(*p!='?')
	    {
		if(*p==']')
		{
		    flag=0;
		    --p;
		    while(*p!='[')
		    {
			if(a==*p) flag=1;
			--p;
		    }
		    if(!flag) ++mm;
		}
		else if(*p=='}')
		{
		    flag=0;
		    --p;
		    while(*p!='{')
		    {
			if(a==*p) flag=1;
			--p;
		    }
		    if(flag) ++mm;
		}
		else
		{
		    if(a!=*p) ++mm;
		}
	    }
	    --p;
	    --h;
	    --j;
	}

	if(mm<=k)
	{
	    if(amino && h+1) return 0;
	    if(!carboxyl || (carboxyl && h+1==slen-m))
	    {
		++matches;
		embPatPushHit(l,name,h+1,m,begin,mm);
	    }
	}
	i+=skip;
    }
    return matches;
}



/* @funcstatic patBruteClass *******************************************
**
** Test if character matches one of a class
**
** @param [r] p [char *] pattern class e.g. [abc]
** @param [r] c [char] character to match
**
** @return [AjBool] true if match success
******************************************************************************/

static AjBool patBruteClass(char *p, char c)
{
    char *s;

    s=p+1;
    while(*s!=']') if(*s++==c) return ajTrue;
    return ajFalse;
}



/* @funcstatic patBruteCompl *******************************************
**
** Test if character matches one of a complement
**
** @param [r] p [char *] pattern complement e.g. {abc}
** @param [r] c [char] character to match
**
** @return [AjBool] true if character not in complement
******************************************************************************/

static AjBool patBruteCompl(char *p, char c)
{
    char *s;

    s=p+1;
    while(*s!='}') if(*s++==c) return ajFalse;
    return ajTrue;
}



/* @funcstatic patBruteIsRange *******************************************
**
** Check if character, class or complement has a range e.g. [abc](3,6)
**
** @param [r] t [char *] pattern
** @param [w] x [int *] first range value
** @param [w] y [int *] second range value
**
** @return [AjBool] true if there is a range & sets x/y accordingly
******************************************************************************/

static AjBool patBruteIsRange(char *t, ajint *x, ajint *y)
{
    char *u;

    if((*t >='A' && *t<='Z') || *t=='?')
    {
	if(*(t+1)=='(')
	{
	    if(sscanf(t+2,"%d,%d",x,y)!=2)
	    {
		(void) sscanf(t+2,"%d",x);
		*y=*x;
	    }
	    
	    return ajTrue;
	}
	return ajFalse;
    }

    u=t;
    
    if(*t=='[')
    {
	while(*u!=']') ++u;
	if(*(u+1)=='(')
	{
	    if(sscanf(u+2,"%d,%d",x,y)!=2)
	    {
		(void) sscanf(u+2,"%d",x);
		*y=*x;
	    }

	    return ajTrue;
	}
	return ajFalse;
    }

    while(*u!='}') ++u;
    if(*(u+1)=='(')
    {
	if(sscanf(u+2,"%d,%d",x,y)!=2)
	{
	    (void) sscanf(u+2,"%d",x);
	    *y=*x;
	}

	return ajTrue;
    }
    return ajFalse;
}



/* @funcstatic patBruteCharMatch *******************************************
**
** Check if text character matches a char/class or complement
**
** @param [r] t [char *] text
** @param [r] c [char] character to match
**
** @return [AjBool] true if match
******************************************************************************/

static AjBool patBruteCharMatch(char *t, char c)
{
    if(!c) return ajFalse;

    if(*t=='?') return ajTrue;
    
    if(*t>='A' && *t<='Z')
    {
	if(*t==c) return ajTrue;
	return ajFalse;
    }
    if(*t=='[')
    {
	if(patBruteClass(t,c)) return ajTrue;
	return ajFalse;
    }

    if(patBruteCompl(t,c)) return ajTrue;
    return ajFalse;
}



/* @funcstatic patBruteNextPatChar *****************************************
**
** Get index of next char/class/complement
**
** @param [r] t [char *] text
** @param [r] ppos [ajint] current index
**
** @return [ajint] next index
******************************************************************************/

static ajint patBruteNextPatChar(char *t, ajint ppos)
{
    if(t[ppos]=='{')
	while(t[ppos]!='}') ++ppos;
    if(t[ppos]=='[')
	while(t[ppos]!=']') ++ppos;
    ++ppos;
    if(t[ppos]=='(')
    {
	while(t[ppos]!=')') ++ppos;
	++ppos;
    }

    return ppos;
}


    
/* @funcstatic patOUBrute *****************************************************
**
** Match pattern to current sequence position
**
** @param [r] seq [char *] text
** @param [r] pat [char *] pattern
** @param [r] spos [ajint] sequence index
** @param [r] ppos [ajint] pattern index
** @param [r] mm [ajint] mismatches left
** @param [r] omm [ajint] allowed mismatches
** @param [r] level [ajint] level of recursion
** @param [w] l [AjPList *] list on which to push hits
** @param [r] carboxyl [AjBool] true if pattern must only match end of text
** @param [r] begin [ajint] text offset
** @param [w] count [int *] hit counter
** @param [r] name [AjPStr *] text entry name
** @param [r] st [ajint] original text index
**
** @return [AjBool] true if hit found (Care! Function recursive.)
******************************************************************************/
    
static AjBool patOUBrute(char *seq, char *pat, ajint spos, ajint ppos, ajint mm,
			ajint omm, ajint level, AjPList *l, AjBool carboxyl,
			ajint begin, ajint *count, AjPStr *name, ajint st)
{
    char *t;
    ajint x;
    ajint y;
    ajint i;
    
    if(level==1000)
	ajFatal("Pattern too complex: 1000 levels of recursion");
    
    
    while(pat[ppos])
    {
	t=pat+ppos;
	if(!seq[spos]) return ajFalse;

	if(!patBruteIsRange(t,&x,&y))
	{
	    if(!patBruteCharMatch(t,seq[spos])) --mm;
	    if(mm<0) return ajFalse;
	    ppos=patBruteNextPatChar(pat,ppos);
	    ++spos;
	    continue;
	}

	for(i=0;i<x;++i)
	{
	    if(!patBruteCharMatch(t,seq[spos++])) --mm;
	    if(mm<0 || !seq[spos-1]) return ajFalse;
	}

	ppos=patBruteNextPatChar(pat,ppos);
	for(i=0;i<y-x;++i)
	{
	    (void) patOUBrute(seq,pat,spos,ppos,mm,omm,level,l,carboxyl,
			      begin,count,name,st);
	    if(!patBruteCharMatch(t,seq[spos])) return ajFalse;
	    ++spos;	    
	}
    }
    
    if(!carboxyl || (carboxyl && !seq[spos]))
    {
	*count+=1;
	embPatPushHit(*l,name,st,spos-st/*+1*/,begin,omm-mm);

    }

    return ajTrue;
}



/* @func embPatBruteForce ****************************************************
**
** Match pattern to a sequence 
**
** @param [r] seq [AjPStr *] text
** @param [r] pat [AjPStr *] pattern
** @param [r] amino [AjBool] true if must match start
** @param [r] carboxyl [AjBool] true if must match end
** @param [w] l [AjPList *] list on which to push hits
** @param [r] begin [ajint] text offset
** @param [r] mm [ajint] allowed mismatches
** @param [r] name [AjPStr *] text entry name
**
** @return [ajint] number of hits
******************************************************************************/

ajint embPatBruteForce(AjPStr *seq, AjPStr *pat, AjBool amino, AjBool carboxyl,
		    AjPList *l, ajint begin, ajint mm, AjPStr *name)
{
    char *s;
    char *p;
    ajint  sum;
    ajint  len;
    ajint  i;
    ajint  count;
    

    sum=count=0;
    s = ajStrStr(*seq);
    p = ajStrStr(*pat);

    if(amino)
    {
	(void) patOUBrute(s,p,0,0,mm,mm,1,l,carboxyl,begin,&count,name,0);
	return count;
    }

    len=strlen(s);
    for(i=0;i<len;++i)
    {
	(void) patOUBrute(s,p,i,0,mm,mm,1,l,carboxyl,begin,&count,name,i);
	sum+=count;
	count=0;
    }
    
    return sum;
}


/* @func  embPatVariablePattern ***********************************************
**
** Match variable pattern against constant text.
** Used for matching many patterns against one sequence.
**
** @param [rw] pattern [AjPStr *] pattern to match
** @param [r] opattern [AjPStr] read-only pattern
** @param [r] text [AjPStr] text to scan
** @param [r] patname [AjPStr] ID or AC of pattern
** @param [w] l [AjPList] list on which to push hits
** @param [r] mode [ajint] 1 for protein, 0 for nucleic acid
** @param [r] mismatch [ajint] allowed mismatches
** @param [r] begin [ajint] text offset
**
** @return [ajint] number of hits
** @@
******************************************************************************/

ajint embPatVariablePattern (AjPStr *pattern, AjPStr opattern, AjPStr text,
			  AjPStr patname, AjPList l, ajint mode,
			  ajint mismatch, ajint begin)
{
    ajint plen;
    ajint slen=0;

    AjBool amino;
    AjBool carboxyl;
    AjBool fclass;
    AjBool ajcompl;
    AjBool dontcare;
    AjBool range;
    ajint    *buf;
    ajint    hits;
    ajint    m;
    ajint    n;
    ajint    i;
    ajint    start;
    ajint    end;
    
    EmbOPatBYPNode off[AJALPHA];

    ajuint *sotable=NULL;
    ajuint solimit;

    EmbPPatMatch ppm;
    AjPStr	   regexp;

    ajint **skipm;
    

    
    if(!embPatClassify(pattern,&amino,&carboxyl,&fclass,&ajcompl,&dontcare,
		      &range,mode))
	ajFatal("Illegal pattern");
    
    plen = ajStrLen(*pattern);

    /*
     *  Select type of search depending on pattern
     */

    if(!range && !dontcare && !fclass && !ajcompl && !mismatch && plen>4)
    {
	/* Boyer Moore Horspool is the choice for ajlong exact patterns */
	plen = ajStrLen(*pattern);
	AJCNEW (buf, AJALPHA);
	embPatBMHInit(pattern,plen,buf);
	hits=embPatBMHSearch(&text,pattern,ajStrLen(text),
			    ajStrLen(*pattern),buf,0,amino,carboxyl,&l,
			    &patname,begin);
        AJFREE (buf);
	return hits;
    }
    
    else if(mismatch && !dontcare && !range && !fclass && !ajcompl)
    {
	/* Baeza-Yates Perleberg for exact patterns plus don't cares */
	plen = ajStrLen(*pattern);
	AJCNEW (buf, AJALPHA);
	embPatBYPInit(pattern,plen,off,buf);
	hits=embPatBYPSearch(&text,&patname,begin,
			    ajStrLen(text),plen,mismatch,off,buf,l,
			    amino,carboxyl,opattern);
	AJFREE (buf);
	return hits;    
    }


    if(!range && !dontcare && !fclass && !ajcompl && !mismatch)
    {
	/* Shift-OR is the choice for small exact patterns */
	AJCNEW (sotable, AJALPHA2);
	(void) embPatSOInit(pattern,sotable,&solimit);
	hits=embPatSOSearch(&text,&patname,*ajStrStr(*pattern),
			   begin,plen,sotable,solimit,l,
			   amino,carboxyl);
	AJFREE (sotable);
	return hits;
    }


    /* Next get m, the real pattern length                                  */
    /* May as well set up a class search as well tho' it needs free's later */
    AJCNEW (sotable, AJALPHA2);
    embPatBYGCInit(pattern,&m,sotable,&solimit);


    if(!range && (fclass || ajcompl) && !mismatch && m<=AJWORD)
    {
	/*
	 *  Baeza-Yates Gonnet for classes and dontcares.
	 *  No mismatches or ranges. Patterns less than (e.g.) 32
	 *  Uses Shift-OR search engine
         */
	AJFREE (sotable);
	AJCNEW (sotable, AJALPHA2);
	embPatBYGCInit(pattern,&m,sotable,&solimit);
	plen=m;
	hits=embPatBYGSearch(&text,&patname,
			   begin,plen,sotable,solimit,l,
			   amino,carboxyl);
	/*	tidy = (void *) sotable;*/
	AJFREE (sotable);
	return hits;
    }
    
	

    if(!mismatch && (range || m>AJWORD))
    {
	/*
	 *  Henry Spencer for ranges and simple classes longer than
         *  e.g. 32. No mismatches allowed
         */
	AJFREE (sotable);
	regexp=embPatPrositeToRegExp(&opattern);
	ppm=embPatPosMatchFind(regexp,text);
	n=embPatPosMatchGetNumber(ppm);
	for(i=0;i<n;++i)
	{
	    start=embPatPosMatchGetStart(ppm,i);
	    end=embPatPosMatchGetEnd(ppm,i);
	    if(amino && start)
	    {
		n=0;
		break;
	    }
	    if(!carboxyl || (carboxyl && start==slen-(end-start+1)))
		embPatPushHit(l,&patname,start,end-start+1,
			     begin,0);
	}
	embPatMatchDel(&ppm);
	hits=n;
	ajStrDel(&regexp);
	return hits;
    }
    

    if(mismatch && !range && (fclass || ajcompl))
    {
	/* Try a Tarhio-Ukkonen-Bleasby         */
	
	AJFREE (sotable);
	
	AJCNEW (skipm, m);
	for(i=0;i<m;++i)
	    AJCNEW (skipm[i], AJALPHA);

	embPatTUBInit(pattern,skipm,m,mismatch,plen);
	hits = embPatTUBSearch(pattern,&text,ajStrLen(text),skipm,
			      m,mismatch,begin,
			      l,amino,carboxyl,&patname,plen);
	for(i=0;i<m;++i) AJFREE (skipm[i]);
	AJFREE (skipm);
	return hits;
    }

    /*
     *  No choice left but to do a Bleasby recursive brute force
     */
    hits = embPatBruteForce(&text,pattern,amino,carboxyl,&l,
			   begin,mismatch,&patname);
    AJFREE (sotable);
    return hits;
}



/* @func embPatRestrictRestrict ***********************************************
**
** Cut down the number of restriction enzyme hits from ajPatRestrictScan
** Notably double reporting of symmetric palindromes and reporting
** of isoschizomers. Also provides an optional alphabetic sort.
**
** If we don't allow isoschizomers, then names of all isoschizomers
** found will be added to the string 'iso' in the returned list of
** EmbPMatMatch structures.  If 'isos' is AjTrue then they will be left alone.
**
** @param [rw] l [AjPList *] list of hits from ajPatRestrictScan
** @param [r] hits [ajint] number of hits from ajPatRestrictScan
** @param [r] isos [AjBool] Allow isoschizomers
** @param [r] alpha [AjBool] Sort alphabetically
**
** @return [ajint] adjusted number of hits
** @@
******************************************************************************/

ajint embPatRestrictRestrict (AjPList *l, ajint hits, AjBool isos,
	AjBool alpha)
{
    EmbPMatMatch m=NULL;
    EmbPMatMatch archetype=NULL; /* archetype of a set of isoschizomers */
    AjPStr  ps=NULL;
    AjPList tlist=NULL;
    AjPList newlist=NULL;

    ajint i;
    ajint v;
    ajint tc=0;
    ajint nc=0;
    ajint cut1=0;
    ajint cut2;
    ajint cut3;
    ajint cut4;
    ajint pos=0;

    ps=ajStrNew();
    tlist=ajListNew();
    newlist=ajListNew();


    /* Remove Mirrors for each enzyme separately */
    ajListSort(*l,patRestrictNameCompare);
    tc=nc=0;

    if(hits)
    {
	ajListPop(*l,(void **)&m);
	ajStrAss(&ps,m->cod);
	ajListPush(*l,(void *)m);
    }
    
    while(ajListPop(*l,(void **)&m))
    {
	if(!ajStrCmpO(m->cod,ps))
	{
	    ajListPush(tlist,(void *)m);
	    ++tc;
	}
	else
	{
	    ajStrAss(&ps,m->cod);
	    ajListPush(*l,(void *)m);
	    ajListSort(tlist,patRestrictStartCompare);
	    ajListSort(tlist,patRestrictCutCompare);
	    cut1=cut2=INT_MAX;
	    for(i=0;i<tc;++i)
	    {
		ajListPop(tlist,(void **)&m);
		if(cut1!=m->cut1)
		{
		    cut1=m->cut1;
		    ajListPush(newlist,(void *)m);
		    ++nc;
		}
		else
		    embMatMatchDel(&m);
	    }
	    tc=0;
	}
    }
    ajListSort(tlist,patRestrictStartCompare);
    ajListSort(tlist,patRestrictCutCompare);
    cut1=cut2=INT_MAX;
    for(i=0;i<tc;++i)
    {
	ajListPop(tlist,(void **)&m);
	if(cut1!=m->cut1)
	{
	    cut1=m->cut1;
	    ajListPush(newlist,(void *)m);
	    ++nc;
	}
	else
	    embMatMatchDel(&m);
    }
    

    hits=nc;
    ajListDel(l);
    ajListDel(&tlist);
    *l=ajListNew();
    tlist=ajListNew();
    

    if(!isos)
    {
	/* Now remove Isoschizomers         */
	/* Keep only first alphabetical one */
	ajListSort(newlist,patRestrictStartCompare);
	if(hits)
	{
	    ajListPop(newlist,(void **)&m);
	    pos=m->start;
	    ajListPush(newlist,(void *)m);
	}
	tc=nc=0;

	while(ajListPop(newlist,(void **)&m))
	{
	    if(pos==m->start)
	    {
/* push groups of RE's that share the same start position onto tlist to
be checked later to see if they are isoschizomers */
		ajListPush(tlist,(void *)m);
		++tc;
	    }
	    else
	    {
		pos=m->start;
	    
		ajListPush(newlist,(void *)m);

		/*
		 * Now for list of all enz's which cut at same pos 
		 *  sorted by Name
		 */
		ajListSort(tlist,patRestrictNameCompare);

		/*
		 * Now loop rejecting, for each left in the list,
		 * anything similar
		 */
/* check for isoschizomers in the group sharing the previous 'pos' here */
		while(tc)
		{
		    ajListPop(tlist,(void **)&m);
		    cut1=m->cut1;
		    cut2=m->cut2;
		    cut3=m->cut3;
		    cut4=m->cut4;
		    ajStrAssC(&ps,ajStrStr(m->pat));
/* first one of the group is not an isoschizomer, by definition, so return it */
		    ajListPush(*l,(void *)m);
		    archetype = m;
		    ++nc;
		    --tc;
		

		    for(i=0,v=0;i<tc;++i)
		    {
			ajListPop(tlist,(void **)&m);
			   
			if(m->cut1!=cut1 || m->cut2!=cut2 || m->cut3!=cut3 ||
			   m->cut4!=cut4 || ajStrCmpO(ps,m->pat))
			{
			    ajListPushApp(tlist,(void *)m);
			    ++v;
			}
		    
			else 
			{
/* same cut sites and pattern at the RE just pushed onto 'l', so is an
isoschizomer - add its name to the archetype's list of isoschizomers and
delete */
			    if (ajStrLen(archetype->iso) > 0) {
			        ajStrAppC(&archetype->iso, ",");
			    }
			    ajStrApp(&archetype->iso, m->cod);
			    embMatMatchDel(&m);
			}
		    }
		    tc=v;
		}
	    }
	}



	ajListSort(tlist,patRestrictNameCompare);
	while(tc)
	{
	    ajListPop(tlist,(void **)&m);
	    cut1=m->cut1;
	    cut2=m->cut2;
	    cut3=m->cut3;
	    cut4=m->cut4;
	    ajStrAssC(&ps,ajStrStr(m->pat));
/* first one of the group is not an isoschizomer, by definition, so return it */
	    ajListPush(*l,(void *)m);
	    archetype = m;
	    ++nc;
	    --tc;
		

	    for(i=0,v=0;i<tc;++i)
	    {
		ajListPop(tlist,(void **)&m);
		   
		if(m->cut1!=cut1 || m->cut2!=cut2 || m->cut3!=cut3 ||
		   m->cut4!=cut4 || ajStrCmpO(ps,m->pat))
		{
		    ajListPushApp(tlist,(void *)m);
		    ++v;
		}
		else
		{
/* same cut sites and pattern as the RE just pushed onto 'l', so is an
isoschizomer - add its name to the archetype's list of isoschizomers and
delete */
		    if (ajStrLen(archetype->iso) > 0) {
		        ajStrAppC(&archetype->iso, ",");
		    }
		    ajStrApp(&archetype->iso, m->cod);
		    embMatMatchDel(&m);
		}
	    }
	    tc=v;
	}
	hits=nc;
	ajListDel(&tlist);
	ajListDel(&newlist);

    }
    else
	*l=newlist;
    
    
    

    /* Finally sort on position of recognition sequence and print */
    ajListSort(*l,patRestrictStartCompare);

    if(alpha)
	ajListSort(*l,patRestrictNameCompare);

    ajStrDel(&ps);
    
    return hits;
}





/* @funcstatic patRestrictStartCompare ****************************************
**
** Sort restriction site hits on the basis of start position
**
** @param [r] a [const void *] First EmbPMatMatch hit
** @param [r] b [const void *] Second EmbPMatMatch hit
**
** @return [ajint] 0 if a and b are equal
**               -ve if a is less than b,
**               +ve if a is greater than b
******************************************************************************/

static ajint patRestrictStartCompare(const void *a, const void *b)
{
    return (*(EmbPMatMatch *)a)->start - (*(EmbPMatMatch *)b)->start;
}

/* @funcstatic patRestrictCutCompare ****************************************
**
** Sort restriction site hits on the basis of cut position
**
** @param [r] a [const void *] First EmbPMatMatch hit
** @param [r] b [const void *] Second EmbPMatMatch hit
**
** @return [ajint] 0 if a and b are equal
**               -ve if a is less than b,
**               +ve if a is greater than b
******************************************************************************/

static ajint patRestrictCutCompare(const void *a, const void *b)
{
    return (*(EmbPMatMatch *)a)->cut1 - (*(EmbPMatMatch *)b)->cut1;
}

/* @funcstatic patRestrictNameCompare ****************************************
**
** Sort restriction site hits on the basis of enzyme name
**
** @param [r] a [const void *] First EmbPMatMatch hit
** @param [r] b [const void *] Second EmbPMatMatch hit
**
** @return [ajint] 0 if a and b are equal
**               -ve if a is less than b,
**               +ve if a is greater than b
******************************************************************************/

static ajint patRestrictNameCompare(const void *a, const void *b)
{
    return strcmp (ajStrStr((*(EmbPMatMatch *)a)->cod),
		   ajStrStr((*(EmbPMatMatch *)b)->cod));
}


/* @func embPatRestrictMatch ***********************************************
**
** Main Restriction function. Scans sequence and rejects unwanted
** cutters
**
** @param [r] seq [AjPSeq] sequence
** @param [r] begin [ajint] start position in sequence
** @param [r] end [ajint] end position in sequence
** @param [r] enzfile [AjPFile] file pointer to .enz file
** @param [r] enzymes [AjPStr] comma separated list of REs or NULL for all
** @param [r] sitelen [ajint] minimum length of recognition site
** @param [r] plasmid [AjBool] Circular DNA
** @param [r] ambiguity [AjBool] Allow ambiguities
** @param [r] min [ajint] minimum number of true cuts
** @param [r] max [ajint] maximum number of true cuts
** @param [r] blunt [AjBool] Allow blunt cutters
** @param [r] sticky [AjBool] Allow sticky cutters
** @param [r] commercial [AjBool] Allow Only report REs with a supplier
** @param [w] l [AjPList *] list for (EmbPMatMatch) hits
**
** @return [ajint] number of hits
** @@
******************************************************************************/

ajint embPatRestrictMatch(AjPSeq seq, ajint begin, ajint end, AjPFile enzfile,
			AjPStr enzymes, ajint sitelen, AjBool plasmid,
			AjBool ambiguity, ajint min, ajint max, AjBool blunt,
			AjBool sticky, AjBool commercial, AjPList *l)
{
    AjBool hassup;
    AjBool isall=ajTrue;
    AjPStr  name;
    AjPStr  strand;
    AjPStr  substr;
    AjPStr  revstr;
    AjPStr  binstr;
    AjPStr  binrev;
    AjPStr  *ea;
    
    EmbPPatRestrict enz;
    
    
    ajint len;
    ajint plen;
    ajint i;
    ajint hits;
    ajint ne;
    
    char *p;
    char *q;

    
    name   = ajStrNew();
    substr = ajStrNew();
    revstr = ajStrNew();
    binstr = ajStrNew();
    binrev = ajStrNew();

    enz = embPatRestrictNew();
    

    ne = 0;
    if(!enzymes)
	isall = ajTrue;
    else
    {
	ne = ajArrCommaList(enzymes,&ea);
	for(i=0;i<ne;++i)
	{
	    ajStrCleanWhite(&ea[i]);
	    ajStrToUpper(&ea[i]);
	}
	if(ajStrMatchCaseC(ea[0],"all"))
	    isall = ajTrue;
	else
	    isall = ajFalse;
    }
    

    
    (void) ajFileSeek(enzfile,0L,0);
    *l = ajListNew();
    (void) ajStrAssC(&name,ajSeqName(seq));
    strand = ajSeqStr(seq);
    ajStrToUpper(&strand);
    (void) ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);
    len = plen = ajStrLen(substr);
    (void) ajStrAssSubC(&revstr,ajStrStr(strand),begin-1,end-1);
    ajSeqReverseStr(&revstr);
    
    (void) ajStrAssC(&binstr,ajStrStr(substr));
    (void) ajStrAssC(&binrev,ajStrStr(revstr));
    
    if(plasmid)
    {
	plen <<= 1;
	(void) ajStrAppC(&substr,ajStrStr(substr));
	(void) ajStrAppC(&binstr,ajStrStr(binstr));

	(void) ajStrAppC(&revstr,ajStrStr(revstr));
	(void) ajStrAppC(&binrev,ajStrStr(binrev));
    }

    q = ajStrStr(binrev);
    p = ajStrStr(binstr);
    for(i=0;i<plen;++i,++p,++q)
    {
	*p = (char)ajAZToBin(*p);
	*q = (char)ajAZToBin(*q);
    }
    

    hits = 0;
    while(embPatRestrictReadEntry(&enz,&enzfile))
    {
	if(!enz->ncuts) continue;
	if(enz->len < sitelen) continue;
	if(!blunt && enz->blunt) continue;
	if(!sticky && !enz->blunt) continue;

	p = ajStrStr(enz->pat);
	if(*p>='A' && *p<='Z') hassup = ajTrue;
	else hassup=ajFalse;
	
	if(!hassup && isall && commercial) continue;

	ajStrToUpper(&enz->pat);

	if(!isall)
	{
	    for(i=0;i<ne;++i)
		if(ajStrMatchCase(ea[i],enz->cod))
		    break;
	    if(i==ne) continue;
	}

	hits += embPatRestrictScan(&enz,&substr,&binstr,&revstr,&binrev,len,
				   ambiguity,plasmid,min,max,begin,l);
    }



    for(i=0;i<ne;++i)
	ajStrDel(&ea[i]);
    if(ne)
	AJFREE (ea);
    
    ajStrDel(&name);
    ajStrDel(&substr);
    ajStrDel(&revstr);
    ajStrDel(&binstr);
    ajStrDel(&binrev);
    embPatRestrictDel(&enz);

    return hits;
}


/* @func embPatGetType ***********************************************
**
** Return the type of a pattern
**
** @param [r] pattern [AjPStr*] pattern
** @param [r] mismatch [ajint] number of allowed mismatches
** @param [r] protein [AjBool] true if protein
** @param [w] m [int*] real length of pattern
** @param [w] left [AjBool*] must match left begin
** @param [r] right [AjBool*] must match right
**
** @return [ajint] type of pattern
** @@
******************************************************************************/

ajint embPatGetType(AjPStr *pattern, ajint mismatch, AjBool protein, ajint *m,
		  AjBool *left, AjBool *right)
{
    AjBool fclass;
    AjBool compl;
    AjBool dontcare;
    AjBool range;
    ajint    plen;
    ajint    type;
    char   *p;
    char   *q;
    

    if(!embPatClassify(pattern,left,right,&fclass,&compl,&dontcare,
		       &range,protein))
	return 0;
    
    /* Get real pattern length */
    p = ajStrStr(*pattern);
    *m = 0;
    while(*p)
    {
	if(*p=='{')
	    while(*p!='}')
		++p;
	else if(*p=='[')
	    while(*p!=']')
		++p;
	++p;
	++*m;
    }
    

    plen = ajStrLen(*pattern);
    type = 0;
    
    /*
     *  Select type of search depending on pattern
     */

    if(!range && !dontcare && !fclass && !compl && !mismatch && plen>AJWORD)
    {
	/* Boyer Moore Horspool is the choice for ajlong exact patterns */
	type = 1;
    }
    else if(mismatch && !dontcare && !range && !fclass && !compl &&
	    plen<AJALPHA/2)
    {
	/* Baeza-Yates Perleberg for exact patterns plus mismatches */
	type = 2;
    }
    else if(!range && !dontcare && !fclass && !compl && !mismatch &&
	    plen<=AJWORD)
    {
	/* Shift-OR is the choice for small exact patterns */
	type = 3;
    }
    else if(!range && (fclass || compl || dontcare) && !mismatch && *m<=AJWORD)
    {
	/*
	 *  Baeza-Yates Gonnet for classes and dontcares.
	 *  No mismatches or ranges. Patterns less than (e.g.) 32
         */
	type = 4;
    }
    else if(!mismatch && (range || *m>AJWORD))
    {
        q = ajStrStr(*pattern);
	while(*q && *q!='?')
	    ++q;
	if(*q=='?')
	    type=7;
	else
	    type = 5;
    }
    else if(mismatch && !range && (fclass || compl))
    {
	/* Try a Tarhio-Ukkonen-Bleasby         */
	type = 6;
    }
    else if((mismatch && range) || !type)
    {
	/*
         *  No choice left but to do a Bleasby recursive brute force
         */
	type = 7;
    }
    
    return type;
}


/* @func embPatCompile ***********************************************
**
** Compile a pattern classified by embPatGetType
**
** @param [r] type [ajint] pattern type
** @param [r] pattern [AjPStr] processed pattern
** @param [r] opattern [AjPStr] original pattern
** @param [w] plen [int*] pattern length
** @param [w] buf [int**] buffer for BMH search
** @param [w] off [EmbOPatBYPNode*] offset buffer for B-Y/P search
** @param [w] sotable [ajuint**] buffer for SHIFT-OR
** @param [w] solimit [int*] limit for SHIFT-OR
** @param [w] m [int*] real length of pattern (from embPatGetType)
** @param [w] regexp [AjPStr *] Henry Spencer regexp string
** @param [w] skipm [int***] skip buffer for Tarhio-Ukkonen
** @param [r] mismatch [ajint] number of allowed mismatches
**
** @return [void]
** @@
******************************************************************************/

void embPatCompile(ajint type, AjPStr pattern, AjPStr opattern, ajint* plen,
		   ajint** buf, EmbOPatBYPNode* off, ajuint** sotable,
		   ajint* solimit, ajint* m, AjPStr* regexp, ajint*** skipm,
		   ajint mismatch)
{
    ajint i=0;
    

    *plen = ajStrLen(pattern);

    switch(type)
    {
    case 1:
	AJCNEW(*buf,AJALPHA);
	embPatBMHInit(&pattern,*plen,*buf);
	break;
    case 2:
	AJCNEW(*buf,AJALPHA);
	embPatBYPInit(&pattern,*plen,off,*buf);
	break;
    case 3:
	AJCNEW(*sotable,AJALPHA2);
	embPatSOInit(&pattern,*sotable,solimit);
	*m = *plen;
	break;
    case 4:
	AJCNEW(*sotable,AJALPHA2);
	embPatBYGCInit(&pattern,m,*sotable,solimit);
	break;
    case 5:
	*regexp = embPatPrositeToRegExp(&opattern);
	break;
    case 6:
	AJCNEW(*skipm,*m);
	for(i=0;i<*m;++i)
	    AJCNEW((*skipm)[i],AJALPHA);
	embPatTUBInit(&pattern,*skipm,*m,mismatch,*plen);
	break;
    case 7:
	break;
    default:
	ajFatal("embPatCompile: Cannot compile pattern");
	break;
    }

    return;
}


/* @func embPatFuzzSearch ***********************************************
**
** Fuzzy search after embPatGetType and embPatCompile
**
** @param [r] type [ajint] pattern type
** @param [r] begin [ajint] text displacement (1=start)
** @param [r] pattern [AjPStr] processed pattern
** @param [r] opattern [AjPStr] original pattern
** @param [r] name [AjPStr] name associated with text
** @param [r] text [AjPStr] text
** @param [w] l [AjPList*] list to push hits onto
** @param [w] plen [int*] pattern length
** @param [r] mismatch [ajint] number of allowed mismatches
** @param [r] left [AjBool] must match left
** @param [r] right [AjBool] must match right
** @param [r] buf [int*] buffer for BMH search
** @param [r] off [EmbOPatBYPNode*] offset buffer for B-Y/P search
** @param [r] sotable [ajuint*] buffer for SHIFT-OR
** @param [r] solimit [ajint] limit for SHIFT-OR
** @param [r] regexp [AjPStr] Henry Spencer regexp string
** @param [r] skipm [int**] skip buffer for Tarhio-Ukkonen-Bleasby
** @param [w] hits [int*] number of hits
** @param [r] m [int*] real pat length (from embPatGetType/embPatCompile)
** @param [w] tidy [void**] data to free
**
** @return [void]
** @@
******************************************************************************/

void embPatFuzzSearch(ajint type, ajint begin, AjPStr pattern,
		      AjPStr opattern, AjPStr name, AjPStr text, AjPList *l,
		      ajint plen, ajint mismatch, AjBool left, AjBool right,
		      ajint *buf, EmbOPatBYPNode *off, ajuint *sotable,
		      ajint solimit, AjPStr regexp, ajint **skipm,
		      ajint *hits, ajint m, void **tidy)
{
    EmbPPatMatch ppm;
    ajint n;
    ajint i;
    ajint start;
    ajint end;
    

    switch (type)
    {
    case 1:
	*hits=embPatBMHSearch(&text,&pattern,ajStrLen(text),
			      ajStrLen(pattern),buf,0,left,right,l,
			      &name,begin);
	*tidy = (void *) buf;
	break;

    case 2:
	for(i=0;i<AJALPHA;++i) buf[i]=plen;
	for(i=0;i<plen;++i) buf[i]=AJALPHA;
	*hits=embPatBYPSearch(&text,&name,begin,
			      ajStrLen(text),plen,mismatch,off,buf,*l,
			      left,right,opattern);
	*tidy = (void *) buf;
	break;

    case 3:
	*hits=embPatSOSearch(&text,&name,*ajStrStr(pattern),
			     begin,plen,sotable,solimit,*l,
			     left,right);
	*tidy = (void *) sotable;
	break;

    case 4:
	plen=m;
	*hits=embPatBYGSearch(&text,&name,
			      begin,plen,sotable,solimit,*l,
			      left,right);
	*tidy = (void *) sotable;
	break;

    case 5:
	ppm=embPatPosMatchFind(regexp,text);
	n=embPatPosMatchGetNumber(ppm);
	for(i=0;i<n;++i)
	{
	    start=embPatPosMatchGetStart(ppm,i);
	    end=embPatPosMatchGetEnd(ppm,i);
	    if(left && start)
	    {
		n=0;
		break;
	    }
	    if(!right || (right && start==ajStrLen(text)-
			     (end-start+1)))
		embPatPushHit(*l,&name,start,end-start+1,
			      begin,0);

	}
	embPatMatchDel(&ppm);
	*hits=n;
	break;

    case 6:
	*hits = embPatTUBSearch(&pattern,&text,ajStrLen(text),skipm,
				m,mismatch,begin,
				*l,left,right,&name,plen);
	*tidy = (void *) skipm;
	break;

    case 7:
	*hits = embPatBruteForce(&text,&pattern,left,right,l,
				 begin,mismatch,&name);
	break;
	    
    default:
	ajFatal("Can't handle pattern type %s\n",ajStrStr(opattern));
	break;
    }
    
    return;
}
