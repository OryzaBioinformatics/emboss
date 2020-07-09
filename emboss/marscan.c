/* @source marscan application
 **
 ** Finds MAR/SAR sites in nucleic sequences
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
#include "stdlib.h"


static AjBool getpos (AjPList l, ajint *thisprev, ajint otherprev,
	AjBool *stored_match, ajint *stored_dist, ajint *stored_this_pos,
	ajint *stored_other_pos, AjPFeatTable *tab, AjBool this_is_8);

static void stepdown (AjPList l16, AjPList l8, AjPFeatTable *tab);

static void output_stored_match(AjBool stored_match, ajint stored_dist,
	ajint stored_this_pos, ajint stored_other_pos, AjPFeatTable *tab,
	AjBool this_is_8);
	
static ajint    patRestrictStartCompare(const void *a, const void *b);


/* the maximum distance between a length 16 pattern and a length 8
pattern in a MRS */
#define MAXDIST 200


int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq;
    AjPFeatTabOut outf=NULL;
    AjPFeatTable tab=NULL;

    AjPStr pattern16=ajStrNewC("awwrtaannwwgnnnc");
    AjPStr opattern16=NULL;
    AjBool amino16;
    AjBool carboxyl16;
    ajint    type16=0;
    ajint    m16;
    ajint    plen16;
    ajint    *buf16=NULL;    
    EmbOPatBYPNode off16[AJALPHA];
    ajuint   *sotable16=NULL;
    ajuint   solimit16;
    AjPStr	   regexp16=NULL;
    ajint            **skipm16=NULL;
    ajint    mismatch16=1;	/* allow a single mismatch */
    AjPList l16;
    ajint    hits16=0;
    void   *tidy16=NULL;
    
    AjPStr pattern16rev=ajStrNewC("gnnncwwnnttaywwt");
    AjPStr opattern16rev=NULL;
    AjBool amino16rev;
    AjBool carboxyl16rev;
    ajint    type16rev=0;
    ajint    m16rev;
    ajint    plen16rev;
    ajint    *buf16rev=NULL;    
    EmbOPatBYPNode off16rev[AJALPHA];
    ajuint   *sotable16rev=NULL;
    ajuint   solimit16rev;
    AjPStr	   regexp16rev=NULL;
    ajint            **skipm16rev=NULL;
    ajint    mismatch16rev=1;	/* allow a single mismatch */
    AjPList l16rev;
    ajint    hits16rev=0;
    void   *tidy16rev=NULL;
    
    AjPStr pattern8=ajStrNewC("aataayaa");
    AjPStr opattern8=NULL;
    AjBool amino8;
    AjBool carboxyl8;
    ajint    type8=0;
    ajint    m8;
    ajint    plen8;
    ajint    *buf8=NULL;    
    EmbOPatBYPNode off8[AJALPHA];
    ajuint   *sotable8=NULL;
    ajuint   solimit8;
    AjPStr	   regexp8=NULL;
    ajint            **skipm8=NULL;
    ajint    mismatch8=0;
    AjPList l8;
    ajint    hits8=0;
    void   *tidy8=NULL;
    
    AjPStr pattern8rev=ajStrNewC("ttrttatt");
    AjPStr opattern8rev=NULL;
    AjBool amino8rev;
    AjBool carboxyl8rev;
    ajint    type8rev=0;
    ajint    m8rev;
    ajint    plen8rev;
    ajint    *buf8rev=NULL;    
    EmbOPatBYPNode off8rev[AJALPHA];
    ajuint   *sotable8rev=NULL;
    ajuint   solimit8rev;
    AjPStr	   regexp8rev=NULL;
    ajint            **skipm8rev=NULL;
    ajint    mismatch8rev=0;
    AjPList l8rev;
    ajint    hits8rev=0;
    void   *tidy8rev=NULL;
    
    AjPStr seqname=NULL;
    AjPStr text=NULL;
    
    ajint    i;
    ajint    begin;
    ajint    end;
    ajint    adj;

/* feature table stuff */    
    AjPFeatLexicon dict=NULL;


    embInit ("marscan", argc, argv);
    
    seqall   = ajAcdGetSeqall("sequence");
    outf     = ajAcdGetFeatout("outf");
    
    seqname = ajStrNew();
    opattern16=ajStrNew();
    opattern16rev=ajStrNew();
    opattern8=ajStrNew();
    opattern8rev=ajStrNew();


/* Copy original patterns for dear Henry */
    ajStrAssC(&opattern16, ajStrStr(pattern16));
    ajStrAssC(&opattern16rev, ajStrStr(pattern16rev));
    ajStrAssC(&opattern8, ajStrStr(pattern8));
    ajStrAssC(&opattern8rev, ajStrStr(pattern8rev));

    if(!(type16=embPatGetType(&pattern16, mismatch16, 0, &m16, &amino16,
		&carboxyl16)))
	ajFatal("Illegal pattern");
    embPatCompile(type16, pattern16, opattern16, &plen16, &buf16, off16,
		&sotable16, &solimit16, &m16, &regexp16, &skipm16, mismatch16);
    
    if(!(type16rev=embPatGetType(&pattern16rev, mismatch16rev, 0, &m16rev,
		&amino16rev, &carboxyl16rev)))
	ajFatal("Illegal pattern");
    embPatCompile(type16rev, pattern16rev, opattern16rev, &plen16rev,
		&buf16rev, off16rev, &sotable16rev, &solimit16rev, &m16rev,
		&regexp16rev, &skipm16rev, mismatch16rev);
    
    if(!(type8=embPatGetType(&pattern8, mismatch8, 0, &m8, &amino8,
		&carboxyl8)))
	ajFatal("Illegal pattern");
    embPatCompile(type8, pattern8, opattern8, &plen8, &buf8, off8,
		&sotable8, &solimit8, &m8, &regexp8, &skipm8, mismatch8);
    
    if(!(type8rev=embPatGetType(&pattern8rev, mismatch8rev, 0, &m8rev,
		&amino8rev, &carboxyl8rev)))
	ajFatal("Illegal pattern");
    embPatCompile(type8rev, pattern8rev, opattern8rev, &plen8rev,
		&buf8rev, off8rev, &sotable8rev, &solimit8rev, &m8rev,
		&regexp8rev, &skipm8rev, mismatch8rev);
    

    text = ajStrNew();
    

    while(ajSeqallNext(seqall, &seq))
    {
	l16 = ajListNew();
	l16rev = ajListNew();
	l8 = ajListNew();
	l8rev = ajListNew();

	ajStrAssC(&seqname, ajSeqName(seq));
	begin = ajSeqallBegin(seqall);
	end   = ajSeqallEnd(seqall);
	ajStrAssSubC(&text, ajSeqCharCopy(seq), begin-1, end-1);
	ajStrToUpper(&text);
	adj = begin+end+1;

	embPatFuzzSearch(type16, begin, pattern16, opattern16, seqname,
		text, &l16, plen16, mismatch16, amino16, carboxyl16, buf16,
		off16, sotable16, solimit16, regexp16, skipm16, &hits16, m16,
		&tidy16);
	embPatFuzzSearch(type16rev, begin, pattern16rev, opattern16rev,
		seqname, text, &l16rev, plen16rev, mismatch16rev, amino16rev,
		carboxyl16rev, buf16rev, off16rev, sotable16rev, solimit16rev,
		regexp16rev, skipm16rev, &hits16rev, m16rev, &tidy16rev);
	embPatFuzzSearch(type8, begin, pattern8, opattern8, seqname,
		text, &l8, plen8, mismatch8, amino8, carboxyl8, buf8, off8,
		sotable8, solimit8, regexp8, skipm8, &hits8, m8, &tidy8);
	embPatFuzzSearch(type8rev, begin, pattern8rev, opattern8rev,
		seqname, text, &l8rev, plen8rev, mismatch8rev, amino8rev,
		carboxyl8rev, buf8rev, off8rev, sotable8rev, solimit8rev,
		regexp8rev, skipm8rev, &hits8rev, m8rev, &tidy8rev);

	if((hits16||hits16rev) && (hits8 || hits8rev)) {

/* append reverse lists to forward lists and sort them by match position */
	  ajListPushList(l8, &l8rev);
	  ajListSort(l8, patRestrictStartCompare);

	  ajListPushList(l16, &l16rev);
	  ajListSort(l16, patRestrictStartCompare);

/* initialise the output feature table */
          dict = ajFeatGffDictionaryCreate();
          if(!tab)
            tab = ajFeatTabNew(seqname,dict);

/* find pairs of hits withing the required distance and output the results */
          stepdown (l16, l8, &tab);

	}
	
	
/* tidy up - (l8rev and l16rev have already been deleted in ajListPushList) */;
	ajListDel(&l16);
	ajListDel(&l8);
    }
    
    if(type16==6)
	for(i=0;i<m16;++i) AJFREE(skipm16[i]);
    if(type16rev==6)
	for(i=0;i<m16rev;++i) AJFREE(skipm16rev[i]);
    if(type8==6)
	for(i=0;i<m8;++i) AJFREE(skipm8[i]);
    if(type8rev==6)
	for(i=0;i<m8rev;++i) AJFREE(skipm8rev[i]);
    
    if(tidy16) AJFREE(tidy16);
    if(tidy16rev) AJFREE(tidy16rev);
    if(tidy8) AJFREE(tidy8);
    if(tidy8rev) AJFREE(tidy8rev);
    
    ajStrDel(&pattern16);
    ajStrDel(&pattern16rev);
    ajStrDel(&pattern8);
    ajStrDel(&pattern8rev);

    ajStrDel(&seqname);
    ajSeqDel(&seq);
    ajFeaturesWrite(outf, tab);

    ajExit();
    return 0;
}

/* @funcstatic getpos ********************************************
**
** gets the next position from a list and checks to see if we have a match
** within MAXDIST of the last match in the other list
**
** @param [r] l [AjPList] the list of matching positions 
** @param [rw] thisprev [int *] pointer to last stored position of this pattern
** @param [r] otherprev [ajint] last stored position of the other pattern
** @param [rw] stored_match [AjBool *] flag set to ajtrue if have stored match
** @param [rw] stored_dist [int *] distance between the patterns in the stored match
** @param [rw] stored_this_pos [int *] position of this pattern match in stored match
** @param [rw] stored_other_pos [int *] position of 8 pattern match in stored match
** @param [rw] tab [AjPFeatTable*] feature table
** @param [r] this_is_8 [AjBool] ajTrue is 'thisprev' refers to the length 8 pattern
** @return [AjBool] False if the list is empty
** @@
******************************************************************************/

static AjBool getpos (AjPList l, ajint *thisprev, ajint otherprev,
	AjBool *stored_match, ajint *stored_dist, ajint *stored_this_pos,
	ajint *stored_other_pos, AjPFeatTable *tab, AjBool this_is_8)
{

  EmbPMatMatch m;
  ajint dist;	/* distance between the two patterns */

  while (*thisprev <= otherprev) {
  	
/* if the list is empty, return ajFalse */
    if (!ajListPop(l, (void **)&m)) {
      return ajFalse;
    }

/* get position of next list element and store it */
    *thisprev = m->start;
    embMatMatchDel(&m);

/* get the absolute distance between the two patterns */
    dist = otherprev - *thisprev;
    if (dist < 0) {
      dist = -dist;
    }
   
/* otherprev is -1 if it hasn't got a position stored in it yet */
    if (otherprev == -1) {
      dist = MAXDIST + 1;
    }

/* if dist to other stored pos is within range */
    if (dist < MAXDIST) {

/* if we have a stored match output it */
      if (*stored_match) {
        if (dist < *stored_dist) {

/* store new match */
	  *stored_match = ajTrue;
	  *stored_dist = dist;
	  *stored_this_pos = *thisprev;
	  *stored_other_pos = otherprev;
        } else {
          output_stored_match(*stored_match, *stored_dist, *stored_this_pos, *stored_other_pos, tab, this_is_8);
          *stored_match = ajFalse;
        }
      } else {
/* store new match */
	  *stored_match = ajTrue;
	  *stored_dist = dist;
	  *stored_this_pos = *thisprev;
	  *stored_other_pos = otherprev;
      }
    } else {
      if (*stored_match) {
      	output_stored_match(*stored_match, *stored_dist, *stored_this_pos, *stored_other_pos, tab, this_is_8);
      	*stored_match = ajFalse;
      }
    }
  }
  
  return ajTrue;
  
}


/* @funcstatic stepdown ********************************************
**
** steps down the two lists of matches looking for hits within the required
** maximum distance
**
** @param [r] l16 [AjPList] List of length 16 hits (both forward and reverse)
** @param [r] l8 [AjPList] List of length 8 hits (both forward and reverse)
** @param [rw] tab [AjPFeatTable*] feature table
** @return [void]
** @@
******************************************************************************/

static void stepdown (AjPList l16, AjPList l8, AjPFeatTable *tab) 
{
	
  ajint prev16 = -1;	/* we have not got a stored position for length 16 */
  ajint prev8 = -1;	/* we have not got a stored position for length 8 */

  AjBool stored_match = ajFalse;	/* flag ajtrue if have stored match */
  ajint stored_dist;	/* distance between the patterns in the stored match */
  ajint stored_16_pos;	/* position of 16 pattern match in stored match */
  ajint stored_8_pos;	/* position of 8 pattern match in stored match */
  
  AjBool notend16=ajTrue;	/* flag for empty list of length 16 pattern matches*/
  AjBool notend8=ajTrue;	/* flag for empty list of length 8 pattern matches*/
  
  while (notend16 || notend8) {

    notend16 = getpos(l16, &prev16, prev8, &stored_match, &stored_dist, &stored_16_pos, &stored_8_pos, tab, ajFalse);

/* if the list of 8 pattern matches is empty and the 16's have gone past
the last 8 pattern match, stop searching */
    if (prev16 >= prev8 && !notend8) {
      notend16 = ajFalse;
    }

    notend8 = getpos(l8, &prev8, prev16, &stored_match, &stored_dist, &stored_8_pos, &stored_16_pos, tab, ajTrue);

/* if the list of 16 pattern matches is empty and the 8's have gone past
the last 16 pattern match, stop searching */
    if (prev8 >= prev16 && !notend16) {
      notend8 = ajFalse;
    }
  }

/* Both lists are empty. Output any remaining stored match */
  output_stored_match(stored_match, stored_dist, stored_8_pos, stored_16_pos, tab, ajTrue);

}

/* @funcstatic output_stored_match ********************************************
**
** Outputs the results of finding a match of the two patterns
**
** @param [r] stored_match [AjBool] flag set to ajtrue if have stored match
** @param [r] stored_dist [ajint] distance between the patterns in the stored match
** @param [r] stored_this_pos [ajint]  position of this pattern match in stored match
** @param [r] stored_other_pos [ajint] position of 8 pattern match in stored match
** @param [rw] tab [AjPFeatTable*] feature table
** @param [r] this_is_8 [AjBool] ajTrue is 'stored_this_pos' is for the length 8 pattern
** @return [void]
** @@
******************************************************************************/

static void output_stored_match(AjBool stored_match, ajint stored_dist,
ajint stored_this_pos, ajint stored_other_pos, AjPFeatTable *tab, 
AjBool this_is_8)
{

/* strand is set to unknown because the MAR/SAR recognition signature (MRS) is
not dependent on the strand(s) it is on */
  AjEFeatStrand strand=AjStrandUnknown;

  AjEFeatFrame frame=AjFrameUnknown;
  AjPStr score=NULL,source=NULL,type=NULL, desc=NULL, note=NULL;
  AjPFeature feature;
  AjPStr notestr=NULL;

  ajint a, b, s8, s16, tmp, end;

  if (!stored_match) return;
  	
  ajStrAssC(&source,"marscan");
  ajStrAssC(&type,"misc_signal");
  ajStrAssC(&score,"1.0");
  ajStrAssC(&note, "note");


/* order the start positions of the two patterns */
  a = stored_this_pos;
  b = stored_other_pos;
  if (a > b) {
    tmp = a;
    a = b;
    b = tmp;
    this_is_8 = ! this_is_8;
  }

/* get the start and end positions of the 8bp and 16bp patterns and get
the end position of the MRS = second pattern + length of second pattern -1 */
  if (this_is_8) {
    s8 = a;
    s16 = b;
    end = s16+15;
  } else {
    s8 = b;
    s16 = a;
    end = s8+7;
  }

  feature = ajFeatureNew(*tab, source, type,
    a, end, score, strand, frame, desc, 0, 0) ;

  ajFmtPrintS(&notestr, "MAR/SAR recognition site (MRS). 8bp pattern=%d..%d. 16bp pattern = %d..%d", a, a+7, b, b+15);
  ajFeatSetTagValue(feature, note, notestr, ajFalse);

/* tidy up - don't delete 'notestr' */
  ajStrDel(&source);
  ajStrDel(&type);
  ajStrDel(&score);
  ajStrDel(&note);
    

}
/* @funcstatic patRestrictStartCompare ****************************************
**      
** Sort pattern hits on the basis of start position
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


