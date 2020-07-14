/* @source splitter application
**
** Split a sequence into (overlapping) smaller sequences
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** @Modified: Rewritten for more intuitive overlaps (ableasby@hgmp.mrc.ac.uk)
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



static void splitter_write(AjPSeqout seqout, AjPSeq subseq, ajint start,
			   ajint end, const AjPSeq seq);
static void splitter_AddSubSeqFeat(AjPFeattable ftable, ajint start,
                                   ajint end, const AjPSeq oldseq);
static void splitter_ProcessChunk (AjPSeqout seqout, const AjPSeq seq,
                                   ajint start, ajint end, const AjPStr name,
                                   AjBool feature);
static void splitter_MakeSubSeqName (AjPStr * name_ptr, const AjPSeq seq,
                                     ajint start, ajint end);




/* @prog splitter *************************************************************
**
** Split a sequence into (overlapping) smaller sequences
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq;
    ajint size;
    ajint overlap;
    ajint len;
    ajint pos;
    AjBool addover;
    AjBool source;
    AjBool feature;
    AjPStr outseq_name = ajStrNew();

    AjIList iter = NULL;
    AjPFeattable old_feattable = NULL;
    AjPFeature gf = NULL;
    const AjPStr type = NULL;
    AjPStr origid = NULL;

    ajint start;
    ajint end;

    embInit("splitter", argc, argv);

    seqout  = ajAcdGetSeqoutall("outseq");
    seqall  = ajAcdGetSeqall("sequence");
    size    = ajAcdGetInt("size");
    overlap = ajAcdGetInt("overlap");
    addover = ajAcdGetBool("addoverlap");
    source  = ajAcdGetBool ("source");
    feature = ajAcdGetBool ("feature");

    if (!feature) {
      source = AJFALSE;
    }

    while(ajSeqallNext(seqall, &seq))
    {
	ajSeqTrim(seq);

	len = ajSeqGetLen(seq);
	pos = 0;

        if (source)
          {
            old_feattable = ajSeqGetFeatCopy(seq);
            iter = ajListIterRead(old_feattable->Features);

            while(ajListIterMore(iter)) {
              gf = ajListIterNext (iter);
              type = ajFeatGetType(gf);
              origid = ajStrNewC("origid");

              if (ajStrMatchC(type, "source")) {
                if (ajFeatGetTag(gf,origid,1,&outseq_name)) {
                  splitter_ProcessChunk (seqout,seq,
					 ajFeatGetStart(gf)-1,
					 ajFeatGetEnd(gf)-1,
                                         outseq_name, feature);
                }
              }
            }
	    ajFeattableDel(&old_feattable);
            ajListIterFree(&iter);
          }
        else
          {
            ajStrAssignC(&outseq_name, "");

            if (!addover)
	    {
		while(pos+size <= len-1)
		{
		    start = pos;
		    end = pos+size-1;
		    splitter_MakeSubSeqName (&outseq_name, seq, start, end);
		    splitter_ProcessChunk (seqout, seq, start, end,
					   outseq_name, feature);
		    pos += size-overlap;
		}
             }
	    else
	    {
		while(pos+size+overlap < len-1)
		{
		    start = pos;
		    end = pos+size+overlap-1;
		    splitter_MakeSubSeqName (&outseq_name, seq, start, end);
		    splitter_ProcessChunk (seqout, seq, start, end,
					   outseq_name, feature);
		    pos += size;
		}
            }
	    splitter_MakeSubSeqName(&outseq_name, seq, pos, len-1);
	    splitter_ProcessChunk (seqout, seq, pos, len-1,
				   outseq_name, feature);
            ajStrDel (&outseq_name);
          }
      }

    ajSeqWriteClose(seqout);
    ajSeqallDel(&seqall);
    ajSeqoutDel(&seqout);
    ajSeqDel(&seq);
    ajStrDel(&origid);
    ajStrDel(&outseq_name);

    ajExit();

    return 0;
}




/* @funcstatic splitter_write  ************************************************
**
** Write out split sequence
**
** @param [u] default_seqout [AjPSeqout] Output object
** @param [u] subseq [AjPSeq] sequence to write
** @param [r] start [ajint] start offset
** @param [r] end [ajint] end offset
** @param [r] seq [const AjPSeq] original trimmed sequence
** @return [void]
** @@
******************************************************************************/

static void splitter_write(AjPSeqout default_seqout,
                           AjPSeq subseq, ajint start,
			   ajint end, const AjPSeq seq)
{
  /* set the description of the subsequence */
  ajSeqAssignDescS(subseq, ajSeqGetDescS(seq));

  /* set the type of the subsequence */
  ajSeqType(subseq);

  ajSeqAllWrite (default_seqout, subseq);

  return;
}

/* @funcstatic splitter_MakeSubSeqName ****************************************
**
** Undocumented
**
** @param [w] name_ptr [AjPStr*] Undocumented
** @param [r] seq [const AjPSeq] Undocumented
** @param [r] start [ajint] Undocumented
** @param [r] end [ajint] Undocumented
**
******************************************************************************/

static void splitter_MakeSubSeqName (AjPStr * name_ptr,
                                     const AjPSeq seq, ajint start,
                                     ajint end)
{
  AjPStr value = ajStrNew();

  /* create a nice name for the subsequence */
  ajStrAssignS(name_ptr, ajSeqGetNameS(seq));
  ajStrAppendC(name_ptr, "_");
  ajStrFromInt(&value, ajSeqGetBegin(seq)+start);
  ajStrAppendS(name_ptr, value);
  ajStrAppendC(name_ptr, "-");
  ajStrFromInt(&value, ajSeqGetBegin(seq)+end);
  ajStrAppendS(name_ptr, value);

  ajStrDel(&value);
}

/* @funcstatic splitter_ProcessChunk ******************************************
**
** Undocumented
**
** @param [u] seqout [AjPSeqout] Undocumented
** @param [r] seq [const AjPSeq] Undocumented
** @param [r] start [ajint] Undocumented
** @param [r] end [ajint] Undocumented
** @param [r] name [const AjPStr] Undocumented
** @param [r] feature [AjBool] Undocumented
**
******************************************************************************/

static void splitter_ProcessChunk (AjPSeqout seqout, const AjPSeq seq,
                                   ajint start, ajint end, const AjPStr name,
                                   AjBool feature)
{
  AjPStr str = ajStrNew();

  AjPFeattable new_feattable = NULL;
  AjPSeq subseq = ajSeqNew ();

  new_feattable = ajFeattableNew(name);
  subseq->Fttable = new_feattable;
  ajFeattableSetNuc(new_feattable);

  ajStrAssignSubC(&str,ajSeqGetSeqC(seq),start,end);
  ajSeqAssignSeqS(subseq,str);
  if (feature)
    splitter_AddSubSeqFeat(subseq->Fttable,start,end,seq);
  ajSeqAssignNameS(subseq, name);
  splitter_write(seqout,subseq,start,end,seq);

  ajStrDel(&str);
  ajSeqDel(&subseq);

  return;
}



/* @funcstatic splitter_AddSubSeqFeat *****************************************
**
** Undocumented
**
** @param [u] ftable [AjPFeattable] Undocumented
** @param [r] start [ajint] Undocumented
** @param [r] end [ajint] Undocumented
** @param [r] oldseq [const AjPSeq] Undocumented
**
******************************************************************************/

static void splitter_AddSubSeqFeat(AjPFeattable ftable, ajint start,
                                   ajint end, const AjPSeq oldseq)
{
  AjPFeattable old_feattable = NULL;
  AjIList iter = NULL;

  old_feattable = ajSeqGetFeatCopy(oldseq);
  iter = ajListIterRead(old_feattable->Features);

  while(ajListIterMore(iter)) {
    AjPFeature gf = ajListIterNext (iter);

    AjPFeature copy = NULL;

    copy = ajFeatCopy(gf);

    if (((ajFeatGetEnd(copy) < start + 1) &&
        (copy->End2 == 0 || copy->End2 < start + 1)) ||
        ((ajFeatGetStart(copy) > end + 1) &&
        (copy->Start2 == 0 || copy->Start2 > end + 1))) {
      continue;
    }

    copy->Start = copy->Start - start;
    copy->End = copy->End - start;

    if (copy->Start2 > 0)
      copy->Start2 = copy->Start2 - start;

    if (copy->End2 > 0)
      copy->End2 = copy->End2 - start;

    ajFeatTrimOffRange (copy, 0, 1, end - start + 1, AJTRUE, AJTRUE);

    ajFeattableAdd(ftable, copy);
  }
  ajFeattableDel(&old_feattable);
  ajListIterFree(&iter);
}
