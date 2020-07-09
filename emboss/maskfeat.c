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


static void maskfeat_FeatSeqMask (AjPSeq seq, AjPStr type, AjPStr maskchar);


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


  (void) embInit ("maskfeat", argc, argv);

  seqall = ajAcdGetSeqall ("sequence");
  seqout = ajAcdGetSeqout ("outseq");
  type = ajAcdGetString ("type");
  maskchar = ajAcdGetString ("maskchar");

  while (ajSeqallNext(seqall, &seq))
  {
/* mask the regions */
    (void) maskfeat_FeatSeqMask (seq, type, maskchar);
  
    (void) ajSeqAllWrite (seqout, seq);
  }

  (void) ajSeqWriteClose (seqout);

  ajExit ();
  return 0;
}




/* @funcstatic maskfeat_FeatSeqMask *****************************************
**
** Masks features of a sequence
**
** @param [u] seq [AjPSeq] sequence
** @param [r] type [AjPStr] types of features to mask as wildcarded string
** @param [r] maskchar [AjPStr] character to mask with 
** @return [void] 
** @@
******************************************************************************/


static void maskfeat_FeatSeqMask (AjPSeq seq, AjPStr type, AjPStr maskchar)
{
    AjIList    iter = NULL ;
    AjPFeature gf   = NULL ;
    AjPStr str = NULL;
    AjPFeattable feat;
    char whiteSpace[] = " \t\n\r,;";	/* skip whitespace and , ; */
    AjPStrTok tokens;
    AjPStr key=NULL;
      
    /* get the feature table of the sequence */
    feat = ajSeqGetFeat(seq);
    
    (void) ajStrAss (&str, ajSeqStr(seq));
                  
    /* For all features... */
                    
    if (feat->Features)
    {
	iter = ajListIter(feat->Features) ;
	while(ajListIterMore(iter))
	{
	    gf = ajListIterNext (iter) ;
	    tokens = ajStrTokenInit(type, whiteSpace);
	    while (ajStrToken( &key, &tokens, NULL))
		if (ajStrMatchWild(gf->Type, key))
		    (void) ajStrMask (&str, gf->Start-1, gf->End-1,
				      *ajStrStr(maskchar));

	    (void) ajStrTokenClear( &tokens);
	    (void) ajStrDel(&key);
	}
	ajListIterFree(iter) ;
    }
     
    (void) ajSeqReplace(seq, str);

    /* tidy up */
    (void) ajStrDel(&str);
    (void) ajStrDel(&key);

    return;
}

  


