/* @source union application
**
** Read a list of sequences, combine them into one sequence and write it
**
** @author: Copyright (C) Peter Rice
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

static ajulong union_GetOverlap (AjPSeq first,
                                 AjPSeq second);

static void union_CopyFeatures (const AjPFeattable old_feattable,
                                AjPFeattable new_feattable,
                                ajulong offset,
                                const AjPFeature source_feature);

/* @prog kmrunion *************************************************************
**
** Reads sequence fragments and builds one sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq    = NULL;
    AjPSeq uniseq = NULL;
    AjPStr unistr = NULL;
    AjBool first = ajTrue;
    AjBool feature;
    AjBool source;
    AjBool findoverlap;
    AjPFeattable new_feattable = NULL;
    ajulong offset = 0;
    AjPSeq prev_seq = NULL;
    AjPStr overlap_file_name = NULL;
    AjPFile overlap_file = NULL;

    AjPFeattable old_feattable = NULL;
    AjPFeature source_feature = NULL;

    AjPStr source_str = ajStrNew();
    AjPStr type_str = ajStrNew();
    char strand='+';
    ajint frame=0;
    float score = 0.0;
    ajulong overlap_base_count = 0;

    int source_group = -1000000;

    embInit ("kmrunion", argc, argv);

    feature = ajAcdGetBool ("feature");
    source  = ajAcdGetBool ("source");
    /* if true, search for overlaps */
    findoverlap = ajAcdGetBool ("findoverlap");
    /* file to write overlap base counts to */
    overlap_file_name = ajAcdGetString ("overlapfile");

    if (findoverlap && ajStrLen(overlap_file_name) > 0) {
      overlap_file = ajFileNewOut(overlap_file_name);
    }
    seqout = ajAcdGetSeqout("outseq");
    seqall = ajAcdGetSeqall("sequence");

    while(ajSeqallNext(seqall, &seq))
    {

      ajStrAssC(&source_str, "union");
      ajStrAssC(&type_str, "source");

      if (first) {
	uniseq = ajSeqNewS(seq);

        if (feature) {
          new_feattable = ajFeattableNewSeq(seq);
          uniseq->Fttable = new_feattable;
        }
      }

      ajSeqTrim(seq);

      if (findoverlap) {
        if (!first) {
          overlap_base_count = union_GetOverlap (prev_seq, seq);
        }

        if (overlap_file) {
          ajFmtPrintF (overlap_file, "%Ld\n", overlap_base_count);
        }
      }

      if (feature) {
        old_feattable = ajSeqCopyFeat(seq);
        source_feature = NULL;

        if (source) {
          source_feature = ajFeatNew(new_feattable, source_str, type_str,
                                     ajStrLen(unistr) -
                                     overlap_base_count + 1,
                                     ajStrLen(unistr) + ajSeqLen (seq) -
                                     overlap_base_count,
                                     score, strand, frame);
          ajFeatTagAddCC (source_feature, "origid", ajStrStr(seq->Name));
          /* FIXME */
          source_feature->Group = source_group++;
        }

        if (old_feattable) {
          union_CopyFeatures(old_feattable, new_feattable,
                             offset - overlap_base_count, source_feature);
        }

        offset += ajSeqLen (seq) - overlap_base_count;
      }

      ajStrAppSub(&unistr, ajSeqStr(seq), overlap_base_count,
                  ajSeqLen (seq) - 1);

      if (!first) {
        ajSeqDel (&prev_seq);
      }

      first = ajFalse;

      prev_seq = ajSeqNewS(seq);

      ajStrDel(&source_str);
      ajStrDel(&type_str);
    }

    ajSeqReplace (uniseq, unistr);
/*
    if (feature)
	seqout->Features = AJTRUE;
*/
    ajSeqWrite (seqout, uniseq);
    ajSeqWriteClose (seqout);

    if (overlap_file) {
      ajFileClose (&overlap_file);
    }

    ajExit();

    return 0;
}

/* FIXME - replace with an EMBOSS library call
** returns the number of bases by which two sequences overlap */

static ajulong union_GetOverlap (AjPSeq first_seq, AjPSeq second_seq)
{
  const AjPStr first_seq_str = ajSeqStr(first_seq);
  const AjPStr second_seq_str = ajSeqStr(second_seq);

  int i = ajSeqLen(first_seq);

  const char * first_str = ajStrStr(first_seq_str);
  const char * second_str = ajStrStr(second_seq_str);

  if (i > ajStrLen(second_seq_str)) {
    i = ajStrLen(second_seq_str);
  }

  for (; i >= 0 ; --i) {
    if (memcmp(&first_str[ajStrLen(first_seq_str)-i], second_str, i) == 0) {
      return i;
    }
  }

  return 0;
}


/* @funcstatic union_CopyFeatures  ********************************************
**
** Copy features with an offset
**
** @param [r] old_feattable [const AjPFeattable] input feature table
** @param [w] new_feattable [AjPFeattable] output feature table
** @param [r] offset [ajulong] amount to offset all feature locations by
** @param [r] source_feature [const AjPFeature] Source feature to update
** @return [void]
** @@
******************************************************************************/
static void union_CopyFeatures (const AjPFeattable old_feattable,
                                AjPFeattable new_feattable,
                                ajulong offset,
                                const AjPFeature source_feature)
{
  AjPStr outseq_name = ajStrNew();
  AjPFeature gf = NULL;
  AjPStr type = NULL;
  AjPFeature copy = NULL;
  AjPStr source_feature_type = NULL;

  ajulong new_length;
  AjIList iter;

  new_length = ajListLength(new_feattable->Features);
  iter = ajListIterRead(old_feattable->Features);

  while(ajListIterMore(iter)) {
     gf = ajListIterNext (iter);
     type = ajFeatGetType(gf);

     copy = ajFeatCopy (gf);
    
    /* FIXME */
     copy->Start += offset;
     copy->End += offset;
     copy->Start2 += offset;
     copy->End2 += offset;
    
    if (source_feature != NULL) {
      source_feature_type = ajFeatGetType(source_feature);

      if (ajStrMatch (type, source_feature_type) &&
          ajFeatGetStart(copy) == ajFeatGetStart (source_feature) &&
          ajFeatGetEnd(copy) == ajFeatGetEnd (source_feature)) {

        AjPStr origid = ajStrNewC("origid");

        if (ajFeatGetTag(gf,origid,1,&outseq_name)) {
          /* don't duplicate source features if there is one already */
          continue;
        }
      }
    }
    
    /* FIXME */
    copy->Group += new_length;
    ajFeattableAdd(new_feattable, copy);
  }
}
