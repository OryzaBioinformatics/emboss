/* @source maskseq application
**
** Mask off features of a sequence
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




static void maskfeat_FeatSeqMask(AjPSeq seq, const AjPStr type, 
				 const AjPStr maskchar, AjBool tolower);

static void maskfeat_StrToLower(AjPStr *str, ajint begin, ajint end);




/* @prog maskfeat *************************************************************
**
** Mask off features of a sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeq seq;
    AjPSeqout seqout;
    AjPStr type;
    AjPStr maskchar;
    AjBool tolower;

    embInit("maskfeat", argc, argv);

    seqall   = ajAcdGetSeqall("sequence");
    seqout   = ajAcdGetSeqout("outseq");
    type     = ajAcdGetString("type");
    maskchar = ajAcdGetString("maskchar");
    tolower  = ajAcdGetToggle("tolower");

    while(ajSeqallNext(seqall, &seq))
    {
	/* mask the regions */
	maskfeat_FeatSeqMask(seq, type, maskchar, tolower);

	ajSeqAllWrite(seqout, seq);
    }

    ajSeqWriteClose(seqout);

    ajExit();

    return 0;
}




/* @funcstatic maskfeat_FeatSeqMask *******************************************
**
** Masks features of a sequence
**
** @param [u] seq [AjPSeq] sequence
** @param [r] type [const AjPStr] types of features to mask as
**                                wildcarded string
** @param [r] maskchar [const AjPStr] character to mask with
** @param [r] tolower [AjBool] if True then 'mask' by changing to lower-case
** @return [void]
** @@
******************************************************************************/


static void maskfeat_FeatSeqMask(AjPSeq seq, const AjPStr type, 
				 const AjPStr maskchar, AjBool tolower)
{
    AjIList    iter = NULL ;
    AjPFeature gf   = NULL ;
    AjPStr str = NULL;
    const AjPFeattable feat;
    char whiteSpace[] = " \t\n\r,;|";	/* skip whitespace and , ; | */
    AjPStrTok tokens;
    AjPStr key = NULL;
    AjBool lower;


    /*
    ** want lower-case if 'tolower' or 'maskchar' is null
    ** or it is the SPACE character
    */
    lower = (tolower || ajStrLen(maskchar) == 0 || ajStrMatchC(maskchar, " "));


    /* get the feature table of the sequence */
    feat = ajSeqGetFeat(seq);

    ajStrAssS(&str, ajSeqStr(seq));

    /* For all features... */

    if(feat && feat->Features)
    {
	iter = ajListIterRead(feat->Features) ;
	while(ajListIterMore(iter))
	{
	    gf = ajListIterNext(iter) ;
	    tokens = ajStrTokenInit(type, whiteSpace);
	    while(ajStrToken( &key, &tokens, NULL))
		if(ajStrMatchWild(gf->Type, key))
		{
		    if(lower)
			maskfeat_StrToLower(&str, gf->Start-1, gf->End-1);
		    else
		        ajStrMask(&str, gf->Start-1, gf->End-1,
				  *ajStrStr(maskchar));
		}

	    ajStrTokenClear( &tokens);
	    ajStrDel(&key);
	}
	ajListIterFree(&iter);
    }

    ajSeqReplace(seq, str);


    ajStrDel(&str);
    ajStrDel(&key);

    return;
}




/* @funcstatic maskfeat_StrToLower *******************************************
**
** Lower-case a part of a sequence string
**
** @param [u] str [AjPStr *] sequence string
** @param [r] begin [ajint] start position to be masked
** @param [r] end [ajint] end position to be masked
** @return [void]
** @@
******************************************************************************/

static void maskfeat_StrToLower(AjPStr *str, ajint begin, ajint end) 
{
	
    AjPStr substr = ajStrNew();
    
    /* extract the region and lowercase */
    ajStrAppSub(&substr, *str, begin, end);
    ajStrToLower(&substr);

    /* remove and replace the lowercased region */
    ajStrCut(str, begin, end);
    ajStrInsert(str, begin, substr);
                                                         
    ajStrDel(&substr);

    return;
}
