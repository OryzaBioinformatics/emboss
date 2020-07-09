#include "ajax.h"
#include "embmat.h"
#include "stdlib.h"
#include "limits.h"


static void matPushHitInt(AjPStr *n, EmbPMatPrints *m, AjPList *l, ajint pos,
			       ajint score, ajint elem, ajint hpe, ajint hpm);




/* @funcstatic matPushHitInt *********************************************
**
** Put a matching protein matrix (EmbPMatPrints) on the heap
** as an EmbPMatMatch structure
**
** @param [r] n [AjPStr *] Sequence name
** @param [r] m [EmbPMatPrints *] Matching fingerprint element
** @param [w] l [AjPList *] List to push hits to
** @param [r] pos [ajint] Sequence position of element
** @param [r] score [ajint] Score of element
** @param [r] elem [ajint] Element number (0 to n)
** @param [r] hpe [ajint] Hits per element (so far)
** @param [r] hpm [ajint] Hits per motif (so far)
**
** @return [void]
******************************************************************************/

static void matPushHitInt(AjPStr *n, EmbPMatPrints *m, AjPList *l, ajint pos,
			       ajint score, ajint elem, ajint hpe, ajint hpm)
{
    EmbPMatMatch mat;

    AJNEW0 (mat);
    mat->seqname=ajStrNewC(ajStrStr(*n));
    mat->cod=ajStrNewC(ajStrStr((*m)->cod));
    mat->acc=ajStrNewC(ajStrStr((*m)->acc));
    mat->tit=ajStrNewC(ajStrStr((*m)->tit));
    mat->pat=ajStrNew();
    mat->n = (*m)->n;
    mat->len = (*m)->len[elem];
    mat->thresh = (*m)->thresh[elem];
    mat->max = (*m)->max[elem];
    mat->element = elem;
    mat->start   = pos;
    mat->score   = score;
    mat->hpe     = hpe;
    mat->hpm     = hpm;
    mat->all	 = ajFalse;
    mat->ordered = ajFalse;
    

    ajListPush(*l,(void *)mat);

    return;
}




/* @func embMatMatchDel *******************************************************
**
** Deallocate a MatMatch structure
**
** @param [w] s [EmbPMatMatch *] Structure to delete
**
** @return [void]
******************************************************************************/

void embMatMatchDel(EmbPMatMatch *s)
{

    ajStrDel(&(*s)->seqname);
    ajStrDel(&(*s)->cod);
    ajStrDel(&(*s)->acc);
    ajStrDel(&(*s)->tit);
    ajStrDel(&(*s)->pat);
    ajStrDel(&(*s)->iso);
    AJFREE(*s);
    *s = NULL;

    return;
}



/* @func embMatPrintsInit ************************************************
**
** Initialise file pointer to the EMBOSS PRINTS data file
**
** @param [w] fp [AjPFile *] file pointer
** @return [void]
******************************************************************************/

void embMatPrintsInit(AjPFile *fp)
{

    ajFileDataNewC(PRINTS_MAT,fp);
    if(!*fp)
	ajFatal("prints.mat file not found. Create it with printsextract.");
    return;
}



/* @func embMatProtReadInt **********************************************
**
** Fill a protein matrix structure (EmbPMatPrints) from a data file
** Gets next entry.
**
** @param [r] fp [AjPFile *] data file pointer
** @param [r] s [EmbPMatPrints *] matrix structure to populate
**
** @return [AjBool] True if structure filled
******************************************************************************/

AjBool embMatProtReadInt(AjPFile *fp, EmbPMatPrints *s)
{
    AjPStr line;

    ajint i;
    ajint j;
    ajint m;
    char *p;

    line = ajStrNewC("#");

    p=ajStrStr(line);
    while(!*p || *p=='#' || *p=='!' || *p=='\n')
    {
	if(!ajFileReadLine(*fp,&line))
	{
	    ajStrDel(&line);
	    return ajFalse;
	}
	p=ajStrStr(line);
    }
    
    AJNEW0 (*s);

    (*s)->cod = ajStrNew();
    (void) ajStrAss(&(*s)->cod,line);
    
    (void) ajFileReadLine(*fp,&line);
    (*s)->acc = ajStrNew();
    (void) ajStrAss(&(*s)->acc,line);
    (void) ajFileReadLine(*fp,&line);
    (void) ajStrToInt(line,&(*s)->n);
    (void) ajFileReadLine(*fp,&line);
    (*s)->tit = ajStrNew();
    (void) ajStrAss(&(*s)->tit,line);

    AJCNEW ((*s)->len, (*s)->n);
    AJCNEW ((*s)->max, (*s)->n);
    AJCNEW ((*s)->thresh, (*s)->n);
    AJCNEW ((*s)->matrix, (*s)->n);
    
    for(m=0;m<(*s)->n;++m)
    {
	(void) ajFileReadLine(*fp,&line);
	(void) ajStrToInt(line,&(*s)->len[m]);
	(void) ajFileReadLine(*fp,&line);
	(void) ajStrToInt(line,&(*s)->thresh[m]);
	(void) ajFileReadLine(*fp,&line);
	(void) ajStrToInt(line,&(*s)->max[m]);
	for(i=0;i<26;++i)
	{
	    AJCNEW((*s)->matrix[m][i], (*s)->len[m]);
	    (void) ajFileReadLine(*fp,&line);
	    p=ajStrStr(line);
	    for(j=0;j<(*s)->len[m];++j)
	    {
		if(!j) p=strtok(p," ");
		else   p=strtok(NULL," ");
		(void) sscanf(p,"%d",&(*s)->matrix[m][i][j]);
	    }
	}
    }
    (void) ajFileReadLine(*fp,&line);
    ajStrDel(&line);
    return ajTrue;
}



/* @func embMatProtDelInt **********************************************
**
** Deallocate a protein matrix structure (EmbPMatPrints)
**
** @param [w] s [EmbPMatPrints *] matrix structure
**
** @return [void]
******************************************************************************/

void embMatProtDelInt(EmbPMatPrints *s)
{
    ajint n;
    ajint i;
    ajint j;
    
    n = (*s)->n;
    
    for(i=0;i<n;++i)
	for(j=0;j<26;++j) AJFREE((*s)->matrix[i][j]);

    AJFREE((*s)->matrix);
    AJFREE((*s)->len);
    AJFREE((*s)->thresh);
    AJFREE((*s)->max);

    ajStrDel(&(*s)->cod);
    ajStrDel(&(*s)->acc);
    ajStrDel(&(*s)->tit);
    AJFREE(*s);
}



/* @func embMatProtScanInt **********************************************
**
** Scan a protein sequence with a fingerprint
**
** @param [r] s [AjPStr *] Sequence
** @param [r] n [AjPStr *] name of sequence
** @param [r] m [EmbPMatPrints *] Fingerprint matrix
** @param [w] l [AjPList *] List to push hits to
** @param [w] all [AjBool *] Set if all elements match
** @param [w] ordered [AjBool *] Set if all elements are in order
** @param [r] overlap [AjBool] True if overlaps are allowed
**
** @return [ajint] number of hits
******************************************************************************/

ajint embMatProtScanInt(AjPStr *s, AjPStr *n, EmbPMatPrints *m, AjPList *l,
			   AjBool *all, AjBool *ordered, AjBool overlap)
{
    EmbPMatMatch mm;
    AjPStr t;
    char   *p;
    char   *q;
    ajint slen;
    ajint score;
    ajint mlen;
    ajint elem;
    ajint minpc;
    ajint maxscore;
    ajint limit;
    ajint sum;
    ajint hpe;
    ajint hpm;
    
    ajint lastelem;
    ajint lastpos;
    ajint op;
    
    ajint i;
    ajint j;
    
    t = ajStrNewC(ajStrStr(*s));
    (void) ajStrToUpper(&t);
    p = q = ajStrStr(t);
    slen = ajStrLen(t);
    for(i=0;i<slen;++i,++p) *p=ajSysItoC(ajAZToInt((ajint)*p));
    p=q;
    
    *all = *ordered = ajTrue;
    lastelem = lastpos = INT_MAX;

    hpm=0;
    
    for(elem=(*m)->n - 1;elem >= 0;--elem)
    {
	hpe=0;
	
	mlen     = (*m)->len[elem];
	minpc    = (*m)->thresh[elem];
	maxscore = (*m)->max[elem];

	limit = slen-mlen;
	for(i=0;i<limit;++i)
	{
	    sum=0;
	    for(j=0;j<mlen;++j)
		sum+=(*m)->matrix[elem][(ajint) p[i+j]][j];
	    score=(sum*100)/maxscore;
	    if(score>=minpc)
	    {
		if(elem<lastelem && *ordered)
		{
		    if(lastelem == INT_MAX)
		    {
			lastelem=elem;
			lastpos=i;
		    }
		    else
		    {
			lastelem=elem;
			op=i;
			if(!overlap) op+=mlen;
			if(op >= lastpos) *ordered=ajFalse;
			lastpos=i;
		    }
		}
		
		++hpe;
		++hpm;
		matPushHitInt(n,m,l,i,score,elem,hpe,hpm);
	    }
	}
	if(!hpe) *all=ajFalse;
    }

    if(hpm)
    {
	(void) ajListPop(*l,(void **)&mm);
	if(*all)
	{
	    mm->all = ajTrue;
	    if(*ordered) mm->ordered=ajTrue;
	    else mm->ordered=ajFalse;
	}
	else
	{
	    mm->all=ajFalse;
	    if(*ordered) mm->ordered=ajTrue;
	    else mm->ordered=ajFalse;
	}
	ajListPush(*l,(void *)mm);
    }

    ajStrDel(&t);
    return hpm;
}
