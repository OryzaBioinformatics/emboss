/* @source msbar application
**
** Mutate sequence beyond all recognition
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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
#include <ctype.h>	/* for tolower, toupper */




static void msbar_blockmutn(AjPStr *str, AjBool isnuc, AjPStr *blocklist,
			    ajint min, ajint max, AjBool inframe);
static void msbar_codonmutn(AjPStr *str, AjBool isnuc, AjPStr *codonlist,
			    AjBool inframe);
static void msbar_pointmutn(AjPStr *str, AjBool isnuc, AjPStr *pointlist);
static void msbar_Insert(AjPStr *str, AjBool isnuc, ajint start, ajint end);
static void msbar_Move(AjPStr *str, ajint start, ajint end, ajint destination);
static void msbar_Duplicate(AjPStr *str, ajint start, ajint end);





/* @prog msbar ***************************************************************
**
** Mutate sequence beyond all recognition
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq = NULL;

    AjBool isnuc;
    AjPStr str=NULL;

    AjPStr *pointlist;
    AjPStr *codonlist;
    AjPStr *blocklist;

    ajint i;
    ajint count;
    ajint min;
    ajint max;
    AjBool inframe;

    (void) embInit ("msbar", argc, argv);

    seqall = ajAcdGetSeqall ("sequence");
    seqout = ajAcdGetSeqoutall ("outseq");

    pointlist = ajAcdGetList ("point");
    codonlist = ajAcdGetList ("codon");
    blocklist = ajAcdGetList ("block");

    count = ajAcdGetInt ("count");
    inframe = ajAcdGetBool ("inframe");
    min = ajAcdGetInt ("minimum");
    max = ajAcdGetInt ("maximum");

    str = ajStrNew();

    while (ajSeqallNext(seqall, &seq))
    {
	/* is this a protein or nucleic sequence? */
	isnuc = ajSeqIsNuc(seq);

	/* get a copy of the sequence string */
	(void) ajStrAss (&str, ajSeqStr(seq));

	/* seed the random number generator */
	(void) ajRandomSeed();

	/* do the mutation operations */
	for (i=0; i<count; i++)
	{
	    (void) msbar_blockmutn(&str, isnuc, blocklist, min, max, inframe);
	    (void) msbar_codonmutn(&str, isnuc, codonlist, inframe);
	    (void) msbar_pointmutn(&str, isnuc, pointlist);

	}

	(void) ajSeqReplace(seq, str);
	(void) ajSeqAllWrite (seqout, seq);
    }

    (void) ajSeqWriteClose (seqout);

    ajStrDel(&str);
    
    ajExit ();
    return 0;
}




/* @funcstatic msbar_blockmutn ***********************************************
**
** Undocumented.
**
** @param [?] str [AjPStr*] Undocumented
** @param [?] isnuc [AjBool] Undocumented
** @param [?] blocklist [AjPStr*] Undocumented
** @param [?] min [ajint] Undocumented
** @param [?] max [ajint] Undocumented
** @param [?] inframe [AjBool] Undocumented
** @@
******************************************************************************/

static void msbar_blockmutn(AjPStr *str, AjBool isnuc, AjPStr *blocklist,
			    ajint min, ajint max, AjBool inframe)
{
    ajint i=-1;
    ajint rposstart;
    ajint rposend;
    ajint rpos2;
    ajint opt;

    while (blocklist[++i])
    {
	(void) ajDebug("Next block mutation operation = %S\n", blocklist[i]);

	/* None */
	if (!ajStrCmpC(blocklist[i], "0"))
	    return;
      
	/* get the option value */
	(void) ajStrToInt(blocklist[i], &opt);

	/* if we want 'Any', then choose which one */
	if (opt == 1)
	{
	    opt = ajRandomNumberD() * 5;
	    opt += 2;
	}

	/*
	 *  get random block start and end positions in the sequence
	 *  (0 to ajStrLen - 1)
	 */
	if (inframe)
	{
	    if (min < 3) min = 3;
	    rposstart = ajRandomNumberD() * (double)ajStrLen(*str)/3;
	    rposend = rposstart + (min/3) + ajRandomNumberD() *
		(double)((max - min)/3);
	    rposstart *= 3;
	    rposend *= 3;
	    rposend--;
	}
	else
	{
	    rposstart = ajRandomNumberD() * (double)ajStrLen(*str);
	    rposend = rposstart + min + ajRandomNumberD() *
		(double)(max - min);
	}

    
	if (opt == 2)
	{
	    /* Insert */
	    (void) ajDebug("block insert from %d to %d\n", rposstart, rposend);
	    (void) msbar_Insert(str, isnuc, rposstart, rposend);
	}
    
	if (opt == 3)
	{
	    /* Delete */
	    (void) ajDebug("block deletion from %d to %d\n", rposstart,
			   rposend);
	    (void) ajStrCut(str, rposstart, rposend);
	}
    
	if (opt == 4)
	{
	    /* Change */
	    (void) ajDebug("block change from %d to %d\n", rposstart, rposend);
	    (void) ajStrCut(str, rposstart, rposend);
	    (void) msbar_Insert(str, isnuc, rposstart, rposend);      
	}
    
	if (opt == 5)
	{
	    /* Duplication */
	    (void) ajDebug("block duplication from %d to %d\n", rposstart,
			   rposend);
	    (void) msbar_Duplicate(str, rposstart, rposend);
	}
    
	if (opt == 6)
	{
	    /* Move */
	    if (inframe)
	    {
		rpos2 = ajRandomNumberD() * (double)(ajStrLen(*str)/3);
		rpos2 *= 3;
	    }
	    else
		rpos2 = ajRandomNumberD() * (double)ajStrLen(*str);

	    (void) ajDebug("block move from %d to %d to position %d\n",
			   rposstart, rposend, rpos2);
	    (void) msbar_Move(str, rposstart, rposend, rpos2);
	}
    }

    return;
}


/* @funcstatic msbar_codonmutn ***********************************************
**
** Undocumented.
**
** @param [?] str [AjPStr*] Undocumented
** @param [?] isnuc [AjBool] Undocumented
** @param [?] codonlist [AjPStr*] Undocumented
** @param [?] inframe [AjBool] Undocumented
** @@
******************************************************************************/

static void msbar_codonmutn(AjPStr *str, AjBool isnuc, AjPStr *codonlist,
			    AjBool inframe)
{
    ajint rpos;
    ajint rpos2;
    ajint i=-1;
    ajint opt;

    while (codonlist[++i])
    {
	(void) ajDebug("Next codon mutation operation = %S\n", codonlist[i]);

	/* None */
	if (!ajStrCmpC(codonlist[i], "0")) return;
      
	/* get the option value */
	(void) ajStrToInt(codonlist[i], &opt);

	/* if we want 'Any', then choose which one */
	if (opt == 1)
	{
	    opt = ajRandomNumberD() * 5;
	    opt += 2;
	}


	/* get a random position in the sequence (0 to ajStrLen - 1) */
	if (inframe)
	{
	    rpos = ajRandomNumberD() * (double)(ajStrLen(*str)/3);
	    rpos *= 3;
	}
	else
	    rpos = ajRandomNumberD() * (double)ajStrLen(*str);


	if (opt == 2)
	{
	    /* Insert */
	    (void) ajDebug("codon insert at %d\n", rpos);
	    (void) msbar_Insert(str, isnuc, rpos, rpos+2);      
	}
    
	if (opt == 3)
	{
	    /* Delete */
	    (void) ajDebug("codon deletion at %d\n", rpos);
	    (void) ajStrCut(str, rpos, rpos+2);
	}

	if (opt == 4)
	{
	    /* Change */
	    (void) ajDebug("codon change at %d\n", rpos);
	    (void) ajStrCut(str, rpos, rpos+2);
	    (void) msbar_Insert(str, isnuc, rpos, rpos+2);      
	} 
    
	if (opt == 5)
	{
	    /* Duplication */
	    (void) ajDebug("codon duplication at %d\n", rpos);
	    (void) msbar_Duplicate(str, rpos, rpos+2);

	} 
    
	if (opt == 6)
	{
	    /* Move */
	    if (inframe)
	    {
		rpos2 = ajRandomNumberD() * (double)(ajStrLen(*str)/3);
		rpos2 *= 3;
	    }
	    else
		rpos2 = ajRandomNumberD() * (double)ajStrLen(*str);

	    (void) ajDebug("codon move from %d to %d\n", rpos, rpos2);
	    (void) msbar_Move(str, rpos, rpos+2, rpos2);
	}


    }

    return;
}




/* @funcstatic msbar_pointmutn ***********************************************
**
** Undocumented.
**
** @param [?] str [AjPStr*] Undocumented
** @param [?] isnuc [AjBool] Undocumented
** @param [?] pointlist [AjPStr*] Undocumented
** @@
******************************************************************************/

static void msbar_pointmutn(AjPStr *str, AjBool isnuc, AjPStr *pointlist)
{
    ajint i=-1;
    ajint rpos, rpos2;
    ajint opt;

    while (pointlist[++i])
    {
	(void) ajDebug("Next point mutation operation = %S\n", pointlist[i]);
	/* None */
	if (!ajStrCmpC(pointlist[i], "0")) return;
      
	/* get the option value */
	(void) ajStrToInt(pointlist[i], &opt);

	/* if we want 'Any', then choose which one */
	if (opt == 1)
	{
	    opt = ajRandomNumberD() * 5;
	    opt += 2;
	}

	/* get a random position in the sequence (0 to ajStrLen - 1) */
	rpos = ajRandomNumberD() * (double)ajStrLen(*str);

	if (opt == 2)
	{
	    /* Insert */
	    (void) ajDebug("Point insert at %d\n", rpos);
	    (void) msbar_Insert(str, isnuc, rpos, rpos);      

	} 

	if (opt == 3)
	{
	    /* Delete */
	    (void) ajDebug("Point deletion at %d\n", rpos);
	    (void) ajStrCut(str, rpos, rpos);

	} 
    
	if (opt == 4)
	{
	    /* Change */
	    (void) ajDebug("Point change at %d\n", rpos);
	    (void) ajStrCut(str, rpos, rpos);
	    (void) msbar_Insert(str, isnuc, rpos, rpos);      

	} 
    
	if (opt == 5)
	{
	    /* Duplication */
	    (void) ajDebug("Point duplication at %d\n", rpos);
	    (void) msbar_Duplicate(str, rpos, rpos);

	} 

	if (opt == 6)
	{
	    /* Move */
	    rpos2 = ajRandomNumberD() * (double)ajStrLen(*str);
	    (void) ajDebug("Point move from %d to %d\n", rpos, rpos2);
	    (void) msbar_Move(str, rpos, rpos, rpos2);
	}
    }

    return;
}




/* @funcstatic msbar_Insert ***************************************************
**
** Undocumented.
**
** @param [?] str [AjPStr*] Undocumented
** @param [?] isnuc [AjBool] Undocumented
** @param [?] start [ajint] Undocumented
** @param [?] end [ajint] Undocumented
** @@
******************************************************************************/

static void msbar_Insert(AjPStr *str, AjBool isnuc, ajint start, ajint end)
{
    char nuc[] = "ACGT";
    char prot[] = "ARNDCQEGHILKMFPSTWYV";
    AjPStr ins=NULL;
    ajint count = end - start +1;
    ajint r;

    while (count--)
    {
	if (isnuc)
	{
	    r = ajRandomNumberD() * (double)strlen(nuc);
	    (void) ajStrAppK(&ins, nuc[r]);
	}
	else
	{
	    r = ajRandomNumberD() * (double)strlen(prot);
	    (void) ajStrAppK(&ins, prot[r]);      
	}
    }
    (void) ajDebug("Inserting %S at %d\n", ins, start);
    (void) ajStrInsert(str, start, ins);
    (void) ajStrDel(&ins);

    return;
}

/* @funcstatic msbar_Move ****************************************************
**
** Undocumented.
**
** @param [?] str [AjPStr*] Undocumented
** @param [?] start [ajint] Undocumented
** @param [?] end [ajint] Undocumented
** @param [?] destination [ajint] Undocumented
** @@
******************************************************************************/

static void msbar_Move(AjPStr *str, ajint start, ajint end, ajint destination)
{
    AjPStr mov=NULL;

    (void) ajStrAss(&mov, *str);
    (void) ajStrSub(&mov, start, end);
    (void) ajStrInsert(str, destination, mov);
    (void) ajStrDel(&mov);

    return;
}

/* @funcstatic msbar_Duplicate ***********************************************
**
** Undocumented.
**
** @param [?] str [AjPStr*] Undocumented
** @param [?] start [ajint] Undocumented
** @param [?] end [ajint] Undocumented
** @@
******************************************************************************/

static void msbar_Duplicate(AjPStr *str, ajint start, ajint end)
{

    (void) msbar_Move(str, start, end, start);
    return;
}
