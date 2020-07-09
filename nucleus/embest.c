/********************************************************************
** @source NUCLEUS EST alignment functions
**
** @author Copyright (C) 1996 Richard Mott
** @author Copyright (C) 1998 Peter Rice revised for EMBOSS
** @version 4.0 
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
********************************************************************/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "emboss.h"
#include "embest.h"
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

typedef struct EstSKeyValue {
  float key;
  ajint value;
} EstOKeyValue, *EstPKeyValue;

typedef struct EstSCoord{
  ajint left, right;
} EstOCoord, *EstPCoord;

typedef struct EstSSavePair
{
  ajint col, row;
} EstOSavePair, *EstPSavePair;

#define LIMIT_RPAIR_SIZE 10000

static EstPSavePair rpair=NULL;
static ajint rpairs=0;
static ajint rpair_size=0;
static ajint rpairs_sorted=0;
static ajint limit_rpair_size=LIMIT_RPAIR_SIZE;
static ajint lsimmat[256][256];
static AjBool verbose;
static AjBool debug;
static ajint indentation;
static float estRand3 (ajint *idum);
static char* estShuffleSeq( char *s, ajint *seed );
static ajint estPairRemember( ajint col, ajint row );
static ajint estSavePairCmp( const void *a, const void *b );
static void estPairInit ( ajint max_bytes );
static void estPairFree(void);
static ajint estDoNotForget( ajint col, ajint row );
static void estIndent( void);
static ajint  estAlignMidpt ( AjPSeq est, AjPSeq genome,
				ajint match, ajint mismatch,
				ajint gap_penalty, ajint intron_penalty,
				ajint splice_penalty,
				AjPSeq splice_sites,
				ajint middle, ajint *gleft, ajint *gright );

static EmbPEstAlign estAlignRecursive ( AjPSeq est, AjPSeq genome,
				    ajint match, ajint mismatch,
				    ajint gap_penalty, ajint intron_penalty,
				    ajint splice_penalty,
				    AjPSeq splice_sites,
				    float max_area, ajint init_path );

static void estWriteMsp( AjPFile ofile, ajint *matches, ajint *len, ajint *tsub,
	       AjPSeq genome, ajint gsub, ajint gpos, AjPSeq est,
	       ajint esub, ajint epos, ajint reverse, ajint gapped);

/* @func embEstSetDebug *******************************************************
**
** Sets debugging calls on
**
** @return [void]
******************************************************************************/

void embEstSetDebug (void) {
  debug = ajTrue;
}

/* @func embEstSetVerbose *******************************************************
**
** Sets verbose debugging calls on
**
** @return [void]
******************************************************************************/

void embEstSetVerbose (void) {
  verbose = ajTrue;
}

static char* estShuffleSeq( char *s, ajint *seed );

/* @func  embEstGetSeed *******************************************************
**
** Returns a seed for the random number generator, using the system clock.
**
** @return [ajint] seed.
** @@
******************************************************************************/

ajint embEstGetSeed(void) {

    time_t *tloc;
    ajint seed;

    tloc = NULL;

    seed = ((ajint)time(tloc))% 100000; 

    (void) ajDebug("! seed = %d\n", seed);
    return seed;
}

/* @func embEstMatInit ********************************************************
**
** Comparison matrix initialisation.
**
** @param [r] match [ajint] Match code
** @param [r] mismatch [ajint] Mismatch penalty
** @param [r] gap [ajint] Gap penalty
** @param [r] neutral [ajint] Score for ambiguous base positions.
** @param [r] pad_char [char] Pad character for gaps in input sequences
**
** @return [void]
** @@
******************************************************************************/

void embEstMatInit(ajint match, ajint mismatch, ajint gap,
		   ajint neutral, char pad_char) {

  ajint c1, c2;
  
  for(c1=0;c1<256;c1++)
    for(c2=0;c2<256;c2++)
      {
	if ( c1 == c2 )
	  {
	    if ( c1 != '*' && c1 != 'n' &&  c1 != 'N' && c1 != '-' )
	      lsimmat[c1][c2] = match;
	    else
	      lsimmat[c1][c2] = 0;
	  }
	else
	  {
	    if ( c1 == pad_char || c2 == pad_char )
	      lsimmat[c1][c2] = lsimmat[c2][c1] = -gap;
	    else if( c1 == 'n' || c2 == 'n' || c1 == 'N' || c2 == 'N' ) 
	      lsimmat[c1][c2] = lsimmat[c2][c1] = neutral;
	    else
	      lsimmat[c1][c2] = lsimmat[c2][c1] = -mismatch;
	  }
      }

  for(c1=0;c1<256;c1++)
    {
      c2 = tolower(c1);
      lsimmat[c1][c2] = lsimmat[c1][c1];
      lsimmat[c2][c1] = lsimmat[c1][c1];
    }
}

/* @func embEstFindSpliceSites *************************************************
**
** Finds all putative DONOR and ACCEPTOR splice sites in the genomic sequence.
**
** Returns a sequence object whose "dna" should be interpreted as an
** array indicating what kind (if any) of splice site can be found at
** each sequence position.
**     
**     DONOR    sites are NNGTNN last position in exon
**
**     ACCEPTOR sites are NAGN last position in intron
**
**     if forward==1 then search fot GT/AG
**     else               search for CT/AC
**
** @param [r] genome [AjPSeq] Genomic sequence
** @param [r] forward [ajint] Boolean. 1 = forward direction
**
** @return [AjPSeq] Sequence of bitmask codes for splice sites.
** @@
******************************************************************************/

AjPSeq embEstFindSpliceSites (AjPSeq genome, ajint forward ) {

  AjPSeq sites = ajSeqNew();
  ajint pos;
  ajint genomelen = ajSeqLen(genome);
  char *s = ajSeqChar(genome);
  char *sitestr = ajSeqCharCopy( genome );

  for(pos=0;pos<genomelen;pos++)
    sitestr[pos] = NOT_A_SITE;

  if ( forward ) { /* gene is in forward direction 
		      -splice consensus is gt/ag */
    for(pos=1;pos<genomelen-2;pos++)
      {
	if ( tolower((ajint) s[pos]) == 'g' &&
	     tolower((ajint) s[pos+1]) == 't' ) /* donor */
	  sitestr[pos-1] = ajSysItoC((ajuint) sitestr[pos-1] | (ajuint) DONOR); /* last position in exon */
	if ( tolower((ajint) s[pos]) == 'a' &&
	     tolower((ajint) s[pos+1]) == 'g' ) /* acceptor */
	  sitestr[pos+1]  = ajSysItoC((ajuint) sitestr[pos+1] | (ajuint) ACCEPTOR); /* last position in intron */
      }
    (void) ajSeqAssNameC(sites, "forward"); /* so that other functions know */
  }
  else { /* gene is on reverse strand so splice consensus looks like ct/ac */
    for(pos=1;pos<genomelen-2;pos++)
      {
	if ( tolower((ajint) s[pos]) == 'c' &&
	     tolower((ajint) s[pos+1]) == 't' ) /* donor */
	  sitestr[pos-1] = ajSysItoC((ajuint) sitestr[pos-1] | (ajuint) DONOR); /* last position in exon */
	if ( tolower((ajint) s[pos]) == 'a' &&
	     tolower((ajint) s[pos+1]) == 'c' ) /* acceptor */
	  sitestr[pos+1] = ajSysItoC((ajuint) sitestr[pos+1] | (ajuint) ACCEPTOR); /* last position in intron */
      }
    (void) ajSeqAssNameC(sites,"reverse"); /* so that other functions know */
  }
 (void)  ajSeqAssSeqC (sites, sitestr);
  AJFREE (sitestr);
  return sites;
}

/* @func embEstShuffleSeq ******************************************************
**
** Shuffle the sequence.
**
** @param [r] seq [AjPSeq] Original sequence
** @param [r] in_place [ajint] Boolean 1=shuffle in place
** @param [r] seed [ajint*] Random number seed.
**
** @return [AjPSeq] shuffled sequence.
** @@
******************************************************************************/

AjPSeq embEstShuffleSeq( AjPSeq seq, ajint in_place, ajint *seed ) {

  AjPSeq shuffled;
  if ( ! in_place )
    shuffled = ajSeqNewS ( seq );
  else
    shuffled = seq;

  estShuffleSeq( ajSeqChar(shuffled),  seed );

  return shuffled;
}
/* @funcstatic estShuffleSeq **************************************************
**
** in-place shuffle of a string
**
** @param [r] s [char*] String
** @param [r] seed [ajint*] Seed
**
** @return [char*] shuffled string.
** @@
******************************************************************************/

static char* estShuffleSeq( char *s, ajint *seed ) {

/* in-place shuffle of a string */

  EstPKeyValue tmp;
  ajint n;
  ajint len = strlen(s);

  AJCNEW (tmp, len);

  for(n=0;n<len;n++)
    {
      tmp[n].key = estRand3(seed);
      tmp[n].value = s[n];
    }

  for(n=0;n<len;n++)
    s[n] = ajSysItoC(tmp[n].value);

  AJFREE (tmp);

  return s;
}

/* @funcstatic estRand3 *******************************************************
**
** Random number generator.
**
** @param [r] idum [ajint*] Seed
**
** @return [float] Random flaoting point number.
** @@
******************************************************************************/

static float estRand3 (ajint *idum) {

  static ajint inext,inextp;
  static long ma[56];
  static ajint iff=0;
  long mj,mk;
  ajint i,ii,k;
  float ZZ;

  static ajint MBIG=1000000000;
  static ajint MSEED=161803398;
  static ajint MZ=0;
  ajint FAC = ((float)1.0/(float)MBIG);
	
  if (*idum < 0 || iff == 0) {
    iff=1;
    mj=MSEED-(*idum < 0 ? -*idum : *idum);
    mj %= MBIG;
    ma[55]=mj;
    mk=1;
    for (i=1;i<=54;i++) {
      ii=(21*i) % 55;
      ma[ii]=mk;
      mk=mj-mk;
      if (mk < MZ) mk += MBIG;
      mj=ma[ii];
    }
    for (k=1;k<=4;k++)
      for (i=1;i<=55;i++) {
	ma[i] -= ma[1+(i+30) % 55];
	if (ma[i] < MZ) ma[i] += MBIG;
      }
    inext=0;
    inextp=31;
    *idum=1;
  }
  if (++inext == 56) inext=1;
  if (++inextp == 56) inextp=1;
  mj=ma[inext]-ma[inextp];
  if (mj < MZ) mj += MBIG;
  ma[inext]=mj;
/*	return mj*FAC; */
  ZZ = mj*FAC;
  ZZ = (ZZ < (float)0.0 ? -ZZ : ZZ );
  ZZ = (ZZ > (float)1.0 ? ZZ-(ajint)ZZ : ZZ);
  return(ZZ);
	
}

/* @func embEstFreeAlign ******************************************************
**
** Free a genomic EST alignment structure
**
** @param [r] ge [EmbPEstAlign*] Genomic EST alignment data structure
**
** @return [void]
** @@
******************************************************************************/

void embEstFreeAlign ( EmbPEstAlign *ge ) {

  if ( *ge )
    {
      if ( (*ge)->align_path )
	AJFREE ( (*ge)->align_path );
      AJFREE (*ge);
    }
}
     
/* @func embEstPrintAlign ****************************************************
**
** Print the alignment
**
** @param [r] ofile [AjPFile] Output file
** @param [r] genome [AjPSeq] Genomic sequence
** @param [r] est [AjPSeq] EST sequence
** @param [r] ge [EmbPEstAlign] Genomic EST alignment
** @param [r] width [ajint] Output width (in bases)
**
** @return [void]
** @@
******************************************************************************/

void embEstPrintAlign(AjPFile ofile, AjPSeq genome, AjPSeq est,
		      EmbPEstAlign ge, ajint width ) {

  ajint gpos, epos, pos, len, i, j, max, m;
  char *gbuf;
  char *ebuf;
  char *sbuf;
  char *genomeseq = ajSeqChar(genome);
  char *estseq = ajSeqChar(est);

  ajint *gcoord, *ecoord;
  ajint namelen = strlen(ajSeqName(genome)) > strlen(ajSeqName(est)) ?
    strlen(ajSeqName(genome)) : strlen(ajSeqName(est))  ;
  char format[256];

  (void) sprintf(format, "%%%ds %%6d ", namelen );
  if ( ofile ) {
    ajFmtPrintF(ofile, "\n");
    len = ajSeqLen(genome) + ajSeqLen(est) + 1;
      
    AJCNEW (gbuf, len);
    AJCNEW (ebuf, len);
    AJCNEW (sbuf, len);

    AJCNEW (gcoord, len);
    AJCNEW (ecoord, len);
      
    gpos = ge->gstart;
    epos = ge->estart;
    len = 0;
    for(pos=0;pos<ge->len;pos++) {
      ajint way = ge->align_path[pos];
      if ( way == DIAGONAL  ) {	/* diagonal */
	gcoord[len] = gpos;
	ecoord[len] = epos;
	gbuf[len] = genomeseq[gpos++];
	ebuf[len] = estseq[epos++];
	m = lsimmat[(ajint)gbuf[len]][(ajint)ebuf[len]];

/* MATHOG, the triple form promotes char to arithmetic type, which 
generates warnings as it might be able overflow the char type.  This is
equivalent but doesn't trigger any compiler noise 
	      sbuf[len] = (char) ( m > 0 ? '|' : ' ' );
*/

	if(m>0) sbuf[len] = '|';
	else    sbuf[len] = ' ';
	len++;
      }
      else if ( way == DELETE_EST ) {
	gcoord[len] = gpos;
	ecoord[len] = epos;
	gbuf[len] = '-';
	ebuf[len] = estseq[epos++];
	sbuf[len] = ' ';
	len++;
      }
      else if ( way == DELETE_GENOME ) {
	gcoord[len] = gpos;
	ecoord[len] = epos;
	gbuf[len] = genomeseq[gpos++];
	ebuf[len] = '-';
	sbuf[len] = ' ';
	len++;
      }
      else if ( way <= INTRON ) {
	      /* want enough space to print the first 5 and last 5
		 bases of the intron, plus a string containing the
		 intron length */

	ajint intron_width = ge->align_path[pos+1];
	ajint half_width = intron_width > 10 ? 5 : intron_width/2;
	ajint g = gpos-1;
	char number[30];
	ajint numlen;
	(void) sprintf(number," %d ", intron_width );
	numlen = strlen(number);

	for(j=len;j<len+half_width;j++) {
	  g++;
	  gcoord[j] = gpos-1;
	  ecoord[j] = epos-1;
	  gbuf[j] = ajSysItoC(tolower((ajint) genomeseq[g]));
	  ebuf[j] = '.';
	  if ( way == FORWARD_SPLICED_INTRON )
	    sbuf[j] = '>';
	  else if ( way == REVERSE_SPLICED_INTRON )
	    sbuf[j] = '<';
	  else 
	    sbuf[j] = '?';
	}
	len = j;
	for(j=len;j<len+numlen;j++) {
	  gcoord[j] = gpos-1;
	  ecoord[j] = epos-1;
	  gbuf[j] = '.';
	  ebuf[j] = '.';
	  sbuf[j] = number[j-len];
	}
	len = j;
	g = gpos + intron_width - half_width-1;
	for(j=len;j<len+half_width;j++) {
	  g++;
	  gcoord[j] = gpos-1;
	  ecoord[j] = epos-1;
	  gbuf[j] = ajSysItoC(tolower((ajint) genomeseq[g]));
	  ebuf[j] = '.';
	  if ( way == FORWARD_SPLICED_INTRON )
	    sbuf[j] = '>';
	  else if ( way == REVERSE_SPLICED_INTRON )
	    sbuf[j] = '<';
	  else 
	    sbuf[j] = '?';
	}

	gpos += ge->align_path[++pos];
	len = j;
      }
    }
      
    for(i=0;i<len;i+=width) {
      max = ( i+width > len ? len : i+width );

      ajFmtPrintF(ofile, format, ajSeqName(genome), gcoord[i]+1 );
      for(j=i;j<max;j++)
	ajFmtPrintF(ofile, "%c",  gbuf[j]);
      ajFmtPrintF(ofile," %6d\n", gcoord[j-1]+1 );

      for(j=0;j<namelen+8;j++)
	ajFmtPrintF(ofile, " ");
      for(j=i;j<max;j++)
	ajFmtPrintF(ofile,"%c", sbuf[j]);
      ajFmtPrintF(ofile,  "\n");

      ajFmtPrintF(ofile, format, ajSeqName(est), ecoord[i]+1 );
      for(j=i;j<max;j++)
	ajFmtPrintF(ofile, "%c", ebuf[j]);
      ajFmtPrintF(ofile," %6d\n\n", ecoord[j-1]+1 );
    }

    ajFmtPrintF( ofile, "\nAlignment Score: %d\n", ge->score );

    AJFREE (gbuf);
    AJFREE (ebuf);
    AJFREE (sbuf);
    AJFREE (gcoord);
    AJFREE (ecoord);
  }
  return;
}  

/* @func embEstAlignNonRecursive *****************************************
**
** Modified Smith-Waterman/Needleman to align an EST or mRNA to a Genomic
** sequence, allowing for introns.
**
** The recursion is
**     
**     {  S[gpos-1][epos]   - gap_penalty
**     
**     {  S[gpos-1][epos-1] + D[gpos][epos]
**     
**     S[gpos][epos] = max {  S[gpos][epos-1]   - gap_penalty
**     
**     {  C[epos]           - intron_penalty 
**     
**     {  0 (optional, only if ! needleman )
**     
**     C[epos] = max{ S[gpos][epos], C[epos] }
**     
**     S[gpos][epos] is the score of the best path to the cell gpos, epos 
**     C[epos] is the score of the best path to the column epos
**     
**     
** @param [r] est [AjPSeq] Sequence of EST
** @param [r] genome [AjPSeq] Sequence of genomic region
** @param [r] match [ajint] Match score
** @param [r] mismatch [ajint] Mismatch penalty (positive)
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] splice_sites [AjPSeq] Marked splice sites.
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
** @param [r] backtrack [ajint] Boolean.
**     If backtrack is 0 then only the start and end points and the score
**     are computed, and no path matrix is allocated.
** @param [r] needleman [ajint] Boolean 1 = global alignment 0 = local alignment
** @param [r] init_path [ajint] Type of initialization for the path.
**     If init_path  is DIAGONAL then the boundary conditions are adjusted  
**     so that the optimal path enters the cell (0,0) diagonally. Otherwise
**     it enters from the left (ie as a deletion in the EST)
**
** @return [EmbPEstAlign] Resulting genomic EST alignment
** @@
******************************************************************************/

EmbPEstAlign embEstAlignNonRecursive ( AjPSeq est, AjPSeq genome,
				       ajint match, ajint mismatch,
				       ajint gap_penalty, ajint intron_penalty,
				       ajint splice_penalty,
				       AjPSeq splice_sites,
				       ajint backtrack, ajint needleman,
				       ajint init_path ) {

  AjPSeq gdup=NULL, edup=NULL;
  char* splice_sites_str = ajSeqChar(splice_sites);
  unsigned char **ppath=NULL, *path=NULL;
  ajint *score1, *score2;
  ajint *s1, *s2, *s3;
  ajint *best_intron_score, *best_intron_coord;
  ajint e_len_pack = ajSeqLen(est)/4+1;
  ajint gpos, epos;
  ajint emax = -1, gmax = -1;
  ajint max_score = 0;
  ajint diagonal, delete_genome, delete_est, intron;
  char *gseq, *eseq, g;
  ajint max, total=0;
  ajint p, pos;
  ajint *temp_path=NULL;
  ajint is_acceptor;
  EmbPEstAlign ge;
  EstPCoord start1=NULL, start2=NULL, t1=NULL, t2=NULL, t3;
  EstPCoord best_intron_start=NULL;
  EstOCoord best_start;
  ajint splice_type=0;

  unsigned char direction;
  unsigned char diagonal_path[4] = { 1, 4, 16, 64 };
  unsigned char delete_est_path[4] = { 2, 8, 32, 128 };
  unsigned char delete_genome_path[4] = { 3, 12, 48, 192 };
  unsigned char mask[4] = { 3, 12, 48, 192 };

  /* path is encoded as 2 bits per cell:
     
     00 intron
     10 diagonal
     01 delete_est
     11 delete_genome
     
     */

  /* the backtrack path, packed 4 cells per byte */

  char dbgmsg[512] = "<undefined>\n";

  if (debug) {
    ajDebug ("embEstAlignNonRecursive\n");
    ajDebug ("   backtrack:%d needleman:%d, init_path:%d\n",
	    backtrack, needleman, init_path);
  }

  AJNEW0 (ge);

  if ( backtrack )
    {
      AJCNEW (ppath, ajSeqLen(genome));
      for(gpos=0;gpos<ajSeqLen(genome);gpos++)
	AJCNEW (ppath[gpos], e_len_pack);      
      AJCNEW (temp_path,  ajSeqLen(genome)+ajSeqLen(est));
    }
  else
    {
      AJCNEW (start1,ajSeqLen( est)+1);
      AJCNEW (start2, ajSeqLen(est)+1);
      AJCNEW (best_intron_start, ajSeqLen(est));

      t1 = start1+1;
      t2 = start2+1;
    }

  AJCNEW (score1, ajSeqLen(est)+1);
  AJCNEW (score2, ajSeqLen(est)+1);
  
  s1 = score1+1;
  s2 = score2+1;

  AJCNEW (best_intron_coord, ajSeqLen(est)+1);
  AJCNEW (best_intron_score, ajSeqLen(est)+1);

  gdup = ajSeqNewS(genome);
  edup = ajSeqNewS(est);
  ajSeqToLower (gdup);
  ajSeqToLower(edup);
  gseq = ajSeqChar(gdup);
  eseq = ajSeqChar(edup);
  
  if ( ! backtrack ) {/* initialise the boundaries for the start points */
    for(epos=0;epos<ajSeqLen(est);epos++)
      {
	t1[epos].left = 0;
	t1[epos].right = epos;
	t2[epos].left = 0;	/* try initializing t2 explicitly */
	t2[epos].right = epos;	/* otherwise it gets missed on first pass */
	best_intron_start[epos] = t1[epos];
      }
  }
  if ( needleman ) {
    for(epos=0;epos<ajSeqLen(est);epos++)
      {
	s1[epos] = MINUS_INFINITY;
	best_intron_score[epos] = MINUS_INFINITY;
      }
  }
  
  for(gpos=0;gpos<ajSeqLen(genome);gpos++) /* loop thru GENOME sequence */
    {
      s3 = s1; s1 = s2; s2 = s3;
      
      g = gseq[gpos];

      if ( backtrack )
	path = ppath[gpos];
      else
	{
	  t3 = t1; t1 = t2; t2 = t3;
	  t2[-1].left = gpos;	/* set start1[0] to (gpos,0) */
	  t1[-1].left = gpos;	/* set start1[0] to (gpos,0) */
	  t1[-1].right = 0;
	}

      if ( splice_sites && (splice_sites_str[gpos] & ACCEPTOR ) )
	is_acceptor = 1;	/* gpos is last base of putative intron */
      else
	is_acceptor = 0;

/* initialisation */

      if ( needleman )
	{
	  if ( init_path == DIAGONAL || gpos > 0  )
	    s1[-1] = MINUS_INFINITY;
	  else
	    s1[-1] = 0;
	}
      else
	s1[-1] = 0;
      
      for(epos=0;epos<ajSeqLen(est);epos++) /* loop thru EST sequence */
	{
	  /* align est and genome */

	  diagonal = s2[epos-1] + lsimmat[(ajint)g][(ajint)eseq[epos]];
	  
	  /* single deletion in est */

	  delete_est = s1[epos-1] - gap_penalty;

	  /* single deletion in genome */

	  delete_genome = s2[epos] - gap_penalty;

	  /* intron in genome, possibly modified by
	     donor-acceptor splice sites */

	  if ( is_acceptor &&
	      (splice_sites_str[best_intron_coord[epos]] & DONOR ) )
	    intron = best_intron_score[epos] - splice_penalty;
	  else
	    intron = best_intron_score[epos] - intron_penalty;
	    
	  if ( delete_est > delete_genome )
	    max = delete_est;
	  else
	    max = delete_genome;

	  if ( diagonal > max )
	    max = diagonal;
	  
	  if ( intron > max )
	    max = intron;

	  if ( needleman || max > 0 ) /* save this score */
	    {
	      if ( max == diagonal ) /* match extension */
		{
		  s1[epos] = diagonal;
		  if ( backtrack ) {
/*		    path[epos/4] |= diagonal_path[epos%4]; <mod> */
		    path[epos/4] =  ajSysItoUC((ajuint) path[epos/4] | (ajuint) diagonal_path[epos%4]);
		  }
		  else
		    {
		      if ( t2[epos-1].left == -1 ) /* SW start */
			{
			  t1[epos].left = gpos;
			  t1[epos].right = epos;
			  if (debug && t1[epos].left == 10126)
			    (void) sprintf (dbgmsg,
				     "t1[%d].left = gpos:%d\n",
				     epos, gpos);
			}
		      else {	/* continue previous match */
			t1[epos] = t2[epos-1];
			if (debug && t1[epos].left == 10126)
			  (void) sprintf (dbgmsg,
				   "t1[%d] = t2[epos-1] left:%d right:%d gpos: %d(a)\n",
				   epos, t1[epos].left, t1[epos].right, gpos);
		      }
		    }
		}
	      else if ( max == delete_est ) /* (continue) gap in EST */
		{
		  s1[epos] = delete_est;
		  if ( backtrack ) { /* <mod> */
		    path[epos/4]  =  ajSysItoUC((ajuint) path[epos/4] | (ajuint) delete_est_path[epos%4]);
		  }
		  else {
		    t1[epos] = t1[epos-1];
		    if (debug && t1[epos].left == 10126)
		      (void) sprintf (dbgmsg,
			       "t1[%d] = t2[epos-1] left:%d (b)\n",
			       epos, t1[epos].left);
		  }
		}
	      else if ( max == delete_genome ) /* (continue) gap in GENOME */
		{
		  s1[epos] = delete_genome;
		  if ( backtrack ) { /* <mod> */
		    path[epos/4] = ajSysItoUC((ajuint) path[epos/4] | (ajuint) delete_genome_path[epos%4]);
		  }
		  else {
		    t1[epos] = t2[epos];
		    if (debug && t1[epos].left == 10126)
		      (void) sprintf (dbgmsg,
			       "t1[%d] = t2[epos] left:%d\n",
			       epos, t1[epos].left);
		  }
		}
	      else		/* Intron */
		{
		  s1[epos] = intron;
		  if ( ! backtrack )
		    t1[epos] = best_intron_start[epos];
		}
	    }
	  else			/* not worth saving (SW with score < 0 ) */
	    {
	      s1[epos] = 0;
	      if ( ! backtrack )
		{
		  t1[epos].left = -1;
		  t1[epos].right = -1;
		}
	    }
	  if ( best_intron_score[epos] < s1[epos] )
	    {
	      /* if ( intron > 0 ) */ /* will only need to store
		                         if this path is positive */
		if ( backtrack )
		  if ( estDoNotForget(epos,
				     best_intron_coord[epos]) == 0 )
				/* store the previous path just
				   in case we need it */

		    {		/* error - stack ran out of memory. Clean up
				   and return NULL */

		      ajErr ("stack ran out of memory, returning NULL");

		      AJFREE (score1);
		      AJFREE (score2);
		      AJFREE (eseq);
		      AJFREE (gseq);
		      AJFREE (best_intron_score);
		      AJFREE (best_intron_coord);
		      AJFREE (temp_path);
		      for(gpos=0;gpos<ajSeqLen(genome);gpos++)
			AJFREE (ppath[gpos]);
		      AJFREE (ppath);
		      estPairFree();
		      AJFREE (ge);

		      return NULL;
		    }

	      best_intron_score[epos] = s1[epos];
	      best_intron_coord[epos] = gpos;
	      if ( ! backtrack )
		best_intron_start[epos] = t1[epos];
	    }

	  if ( ! needleman && max_score < s1[epos] )
	    {
	      max_score = s1[epos];
	      emax = epos;
	      gmax = gpos;
	      if ( ! backtrack ) {
		best_start = t1[epos];
		if (debug)
		  ajDebug ("max_score: %d best_start = t1[%d] left:%d right:%d\n",
			  max_score, epos, best_start.left, best_start.right);
		if (debug)
		  ajDebug ("t1 from :%s\n", dbgmsg);
	      }
	    }
	}
    }

  /* back track */

  if ( needleman )
    {
      ge->gstop = ajSeqLen(genome)-1;
      ge->estop = ajSeqLen(est)-1;
      ge->score = s1[ge->estop];
    }
  else
    {
      ge->gstop = gmax;
      ge->estop = emax;
      ge->score = max_score;
    }

  if ( backtrack )
    {
      pos = 0;
      
      epos = ge->estop;
      gpos = ge->gstop;
      total = 0;

      /* determine the type of spliced intron (forward or reversed) */
      
      if ( splice_sites ) {
	if ( ! strcmp( ajSeqName(splice_sites), "forward") )
	  splice_type = FORWARD_SPLICED_INTRON;
	else if ( ! strcmp( ajSeqName(splice_sites), "reverse") )
	  splice_type = REVERSE_SPLICED_INTRON;
	else 
	  splice_type = INTRON; /* This is really an error - splice_sites
				   MUST have a direction */
      }
	  
      while( ( needleman || total < max_score) && epos >= 0 && gpos >= 0 )
	{
	  direction = ajSysItoUC(( (ajuint)ppath[gpos][epos/4] & (ajuint)mask[epos%4] ) >> (2*(epos%4))); 
	  temp_path[pos++] = direction;
	  if ( (ajuint) direction == INTRON ) /* intron */
	    {
	      ajint gpos1;

	      if ( gpos-best_intron_coord[epos]  <= 0 )
		{
 		  if ( verbose ) 
		    (void) ajWarn("NEGATIVE intron gpos: %d %d\n",
				  gpos, gpos-best_intron_coord[epos] ); 
		  gpos1 = estPairRemember(epos, gpos ); 
		}
	      else
		{
		  gpos1 = best_intron_coord[epos];	      
		}

	      if ( splice_sites && (splice_sites_str[gpos] & ACCEPTOR ) &&
		  ( splice_sites_str[gpos1] & DONOR ) )
		{
		  total -= splice_penalty;
		  temp_path[pos-1] = splice_type; /* make note that this
						     is a proper intron */
		}
	      else
		{
		  total -= intron_penalty;
		}

	      temp_path[pos++] = gpos-gpos1; /* intron this far */
	      gpos = gpos1;
	    }
	  else if ( (ajuint) direction == DIAGONAL ) /* diagonal */
	    {
	      total += lsimmat[(ajint)gseq[gpos]][(ajint)eseq[epos]];
	      epos--;
	      gpos--;
	    }
	  else if ( (ajuint) direction == DELETE_EST ) /* delete_est */
	    {
	      total -= gap_penalty;
	      epos--;
	    }
	  else			/* delete_genome */
	    {
	      total -= gap_penalty;
	      gpos--;
	    }
	}
      
      gpos++;
      epos++;
      
      
      ge->gstart = gpos;
      ge->estart = epos;
      ge->len = pos;
      if (debug)
	ajDebug ("gstart = gpos (a) : %d\n", ge->gstart);
      
      AJCNEW (ge->align_path, ge->len);

      /* reverse the ge so it starts at the beginning of the sequences */


      for(p=0;p<ge->len;p++)
	{
	  if ( temp_path[p] > INTRON ) /* can be INTRON or
					  FORWARD_SPLICED_INTRON or
					  REVERSE_SPLICED_INTRON */
	    ge->align_path[pos-p-1] = temp_path[p];
	  else
	    {
	      ge->align_path[pos-p-2] = temp_path[p];
	      ge->align_path[pos-p-1] = temp_path[p+1];
	      p++;
	    }
	}
    }
  else
    {
      ge->gstart = best_start.left;
      ge->estart = best_start.right;
      if (debug)
	ajDebug ("gstart = best_start.left : %d\n", ge->gstart);
    }

  AJFREE (score1);
  AJFREE (score2);
  AJFREE (eseq);
  AJFREE (gseq);
  AJFREE (best_intron_score);
  AJFREE (best_intron_coord);
  ajSeqDel(&gdup);
  ajSeqDel(&edup);

  if ( backtrack )
    {
      AJFREE (temp_path);
      
      for(gpos=0;gpos<ajSeqLen(genome);gpos++)
	AJFREE (ppath[gpos]);
      AJFREE (ppath);
      estPairFree();
    }
  else
    {
      AJFREE (start1);
      AJFREE (start2);
      AJFREE (best_intron_start);
    }

  if ( verbose )
    {
      estIndent();
      (void) ajDebug("non-recursive score %d total: %d gstart %d estart %d "
		     "gstop %d estop %d\n",
		     ge->score, total, ge->gstart, ge->estart,
		     ge->gstop, ge->estop );
    }

  return ge;
}

/* @func embEstAlignLinearSpace ******************************************
**
** Align EST sequence to genomic in linear space
**
** @param [r] est [AjPSeq] Sequence of EST
** @param [r] genome [AjPSeq] Sequence of genomic region
** @param [r] match [ajint] Match score
** @param [r] mismatch [ajint] Mismatch penalty (positive)
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] splice_sites [AjPSeq] Marked splice sites.
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
** @param [r] megabytes [float] Maximum memory allowed in Mbytes for
**        alignment by standard methods.
**
** @return [EmbPEstAlign] Genomic EST alignment
** @@
******************************************************************************/

EmbPEstAlign embEstAlignLinearSpace ( AjPSeq est, AjPSeq genome,
					  ajint match, ajint mismatch,
					  ajint gap_penalty, ajint intron_penalty,
					  ajint splice_penalty,
					  AjPSeq splice_sites,
					  float megabytes ) {

  EmbPEstAlign ge, rge;
  AjPSeq genome_subseq, est_subseq, splice_subseq;
  float area;
  float max_area = megabytes*(float)1.0e6;

  estPairInit( (ajint)((float)1.0e6*megabytes) );

  area = ((float)ajSeqLen(genome)+(float)1.0)*((float)ajSeqLen(est)+(float)1.0)/(float)4; /* divide by 4
							      as we pack 4
							      cells per byte */

  ajDebug("area %.6f max_area %.6f\n", area, max_area);
/* sequences small enough to align by standard methods ?*/

  if ( area <= max_area ) 
  {
    ajDebug("call embEstAlignNonRecursive\n");
     return embEstAlignNonRecursive ( est, genome, match, mismatch,
					 gap_penalty, intron_penalty,
					 splice_penalty, splice_sites,
					 1, 0, DIAGONAL );
  }
/* need to recursively split */

  /* first do a Smith-Waterman without backtracking to find
     the start and end of the alignment */
      
  ge = embEstAlignNonRecursive( est, genome, match, mismatch,
				gap_penalty, intron_penalty,
				splice_penalty, splice_sites,
				0, 0, DIAGONAL );  
  
  /* extract subsequences corresponding to the aligned regions */

  if ( verbose ) {
     estIndent();
     (void) ajDebug ("sw alignment score %d gstart %d estart %d "
		     "gstop %d estop %d\n", ge->score, ge->gstart,
		     ge->estart, ge->gstop, ge->estop ); 
  }
  genome_subseq = ajSeqNewS(genome);
  est_subseq = ajSeqNewS(est);
  ajSeqSetRange (genome_subseq, ge->gstart, ge->gstop);
  ajSeqSetRange (est_subseq, ge->estart, ge->estop);
  ajSeqTrim(genome_subseq);
  ajSeqTrim(est_subseq);

  if ( splice_sites ) {
    splice_subseq = ajSeqNewS(splice_sites);
    ajSeqSetRange (splice_subseq, ge->gstart, ge->gstop );
    ajSeqTrim(splice_subseq);
  }
  else
    splice_subseq = NULL;

  /* recursively do the alignment */

  rge = estAlignRecursive( est_subseq, genome_subseq, match,
			   mismatch, gap_penalty, intron_penalty,
			   splice_penalty, splice_subseq, max_area,
			   DIAGONAL );  

  ge->len = rge->len;
  ge->align_path = rge->align_path;

  AJFREE (rge);
  ajSeqDel(&genome_subseq);
  ajSeqDel(&est_subseq);
  ajSeqDel(&splice_subseq);
      
  return ge;
}

/* @funcstatic estAlignRecursive *********************************************
**
** Modified Smith-Waterman/Needleman to align an EST or mRNA to a Genomic
**     sequence, allowing for introns
**
** @param [r] est [AjPSeq] Sequence of EST
** @param [r] genome [AjPSeq] Sequence of genomic region
** @param [r] match [ajint] Match score
** @param [r] mismatch [ajint] Mismatch penalty (positive)
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] splice_sites [AjPSeq] Marked splice sites.
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
** @param [r] max_area [float] Maximum memory available for alignment
**            by standard method (allowing 4 bases per byte).
**            Otherwise sequences are split and aligned recursively.
** @param [r] init_path [ajint] Type of initialization for the path.
**     If init_path  is DIAGONAL then the boundary conditions are adjusted  
**     so that the optimal path enters the cell (0,0) diagonally. Otherwise
**     it enters from the left (ie as a deletion in the EST)
**
** @return [EmbPEstAlign] Resulting genomic EST alignment
** @@
******************************************************************************/

static EmbPEstAlign estAlignRecursive ( AjPSeq est, AjPSeq genome,
				 ajint match, ajint mismatch,
				 ajint gap_penalty, ajint intron_penalty,
				 ajint splice_penalty,
				 AjPSeq splice_sites, float max_area,
				 ajint init_path ) {

  ajint middle, gleft, gright, score, i, j;
  AjPSeq left_splice=NULL, right_splice=NULL;
  AjPSeq left_genome, right_genome;
  AjPSeq left_est, right_est;
  EmbPEstAlign left_ge, right_ge, ge;
  float area;
  ajint split_on_del;

  if (debug)
    ajDebug ("estAlignRecursive\n");

  area = ((float)ajSeqLen(genome)+(float)1.0)*((float)ajSeqLen(est)+(float)1.0)/(float)4; /* divide by 4 as
							      we pack 4 cells
							      per byte */

  if ( area <= max_area )	/* sequences small enough to align
				   by standard methods */
    {
      if ( verbose ) {
	estIndent();
	(void) ajDebug("using non-recursive alignment %d %d   %g %g\n",
		      ajSeqLen(genome), ajSeqLen(est), area, max_area );
      }
      ge = embEstAlignNonRecursive( est, genome, match, mismatch,
				       gap_penalty, intron_penalty,
				       splice_penalty, splice_sites,
				       1, 1, DIAGONAL );

      if ( ge != NULL ) { /* success */
	if (debug)
	  ajDebug ("success returns ge gstart:%d estart:%d gstop:%d estop:%d\n",
		  ge->gstart, ge->estart, ge->gstop, ge->estop);
	return ge;
      }
      else /* failure because we ran out of memory */
	{
	  indentation -= 3;
	  if ( verbose ) {
	    estIndent();
	    (void) ajDebug("Stack memory overflow ... splitting\n");
	  }
	}
    }
  /* need to recursively split */

  if ( verbose ) {
    estIndent();
    (void) ajDebug("splitting genome and est\n");
  }

  middle = ajSeqLen(est)/2;
  
  score = estAlignMidpt( est, genome, match, mismatch, gap_penalty,
			      intron_penalty, splice_penalty, splice_sites,
			      middle, &gleft, &gright );
  if ( verbose ) {
    estIndent();
    (void) ajDebug("score %d middle %d gleft %d gright %d\n",
		   score, middle, gleft, gright );
  }
  split_on_del =  ( gleft == gright );
  
  
  /* split genome */

  left_genome = ajSeqNewS(genome);
  right_genome = ajSeqNewS(genome);
  ajSeqSetRange (left_genome,  0, gleft );
  ajSeqSetRange (right_genome, gright, ajSeqLen(genome)-1);
  ajSeqTrim (left_genome);
  ajSeqTrim(right_genome);
  if ( splice_sites )
    {
      left_splice = ajSeqNewS(genome);
      right_splice = ajSeqNewS(genome);
      ajSeqSetRange (left_splice,  0, gleft );
      ajSeqSetRange (right_splice, gright, ajSeqLen(genome)-1);
      ajSeqTrim (left_splice);
      ajSeqTrim(right_splice);
    }
  /* split est */
  
  left_est = ajSeqNewS(est);
  right_est = ajSeqNewS(est);
  ajSeqSetRange (left_est,  0, middle );
  ajSeqSetRange (right_est, middle+1, ajSeqLen(est)-1);
  ajSeqTrim (left_est);
  ajSeqTrim(right_est);

  /* align left and right parts separately */

  if ( verbose ) {
    estIndent();
    (void) ajDebug ("LEFT\n");
  }
  left_ge = estAlignRecursive( left_est, left_genome, match, mismatch,
			       gap_penalty, intron_penalty,
			       splice_penalty, left_splice, max_area,
			       DIAGONAL );  

  if ( verbose ) {
    estIndent();
    (void) ajDebug ("RIGHT\n");
  }
  right_ge = estAlignRecursive ( right_est, right_genome, match,
				 mismatch, gap_penalty, intron_penalty,
				 splice_penalty, right_splice, max_area,
				 DIAGONAL );  


      /* merge the alignments */

  AJNEW0 (ge);
  ge->score = left_ge->score + right_ge->score;
  ge->gstart = 0;
  ge->estart = 0;
  ge->gstop = ajSeqLen(genome)-1;
  ge->estop = ajSeqLen(est)-1;

  ge->len = left_ge->len+right_ge->len;
  AJCNEW (ge->align_path, ge->len);

  for(i=0,j=0;j<left_ge->len;i++,j++)
    ge->align_path[i] = left_ge->align_path[j];
	  
  if ( split_on_del ) /* merge on an est deletion */
    {
      estIndent();
      (void) ajDebug ("split_on_del\n");
      ge->align_path[i++] = DELETE_EST;
      for(j=1;j<right_ge->len;i++,j++) /* omit first symbol on
					  right-hand alignment */
	ge->align_path[i] = right_ge->align_path[j];
    }
  else
    for(j=0;j<right_ge->len;i++,j++)
      ge->align_path[i] = right_ge->align_path[j];
	    
  ajSeqDel(&left_est);
  ajSeqDel(&right_est);
  ajSeqDel(&left_genome);
  ajSeqDel(&right_genome);
  if ( splice_sites ) {
    ajSeqDel(&left_splice);
    ajSeqDel(&right_splice);
  }
  embEstFreeAlign(&left_ge);
  embEstFreeAlign(&right_ge );

  indentation -= 3;
  if (verbose)
    (void) ajDebug ("end returns ge gstart:%d estart:%d gstop:%d estop:%d\n",
		    ge->gstart, ge->estart, ge->gstop, ge->estop);
  return ge;
}

/* @funcstatic estAlignMidpt *************************************************
**
** Modified Needleman-Wunsch to align an EST or mRNA to a Genomic
** sequence, allowing for introns. The recursion is
**
**     {  S[gpos-1][epos]   - gap_penalty
**     
**     {  S[gpos-1][epos-1] + D[gpos][epos]
**     
**     S[gpos][epos] = max {  S[gpos][epos-1]   - gap_penalty
**     
**     {  C[epos]           - intron_penalty 
**     
**     C[epos] = max{ S[gpos][epos], C[epos] }
**     
**     S[gpos][epos] is the score of the best path to the cell gpos, epos 
**     C[epos] is the score of the best path to the column epos
**     
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
**     
**     NB: IMPORTANT:
**     
**     The input sequences are assumed to be subsequences chosen so
**     that they align end-to end, with NO end gaps. Call
**     non_recursive_est_to_genome() to get the initial max scoring
**     segment and chop up the sequences appropriately.
**     
**     The return value is the alignment score.
**
**     If the alignment crosses middle by a est deletion (ie horizontally) then
**              gleft == gright
**     
** @param [r] est [AjPSeq] Sequence of EST
** @param [r] genome [AjPSeq] Sequence of genomic region
** @param [r] match [ajint] Match score
** @param [r] mismatch [ajint] Mismatch penalty (positive)
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] splice_sites [AjPSeq] Marked splice sites.
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
** @param [r] middle [ajint] Sequence mid point position.
**     This Function does not compute the path, instead it finds the
**     genome coordinates where the best path crosses epos=middle, so this
**     should be called recursively to generate the complete alignment in
**     linear space.
** @param [r] gleft [ajint*] genome left coordinate at the crossing point.
**     If the alignment crosses middle in a diagonal fashion then
**              gleft+1 == gright
** @param [r] gright [ajint*] genome right coordinate at the crossing point.
**     If the alignment crosses middle in a diagonal fashion then
**              gleft+1 == gright
**
** @return [ajint] alignment score
** @@
******************************************************************************/

static ajint estAlignMidpt ( AjPSeq est, AjPSeq genome, ajint match,
			   ajint mismatch, ajint gap_penalty, ajint intron_penalty,
			   ajint splice_penalty, AjPSeq splice_sites,
			   ajint middle, ajint *gleft, ajint *gright ) {

  AjPSeq gdup=NULL, edup=NULL;
  ajint *score1, *score2;
  ajint *s1, *s2, *s3;
  ajint *best_intron_score, *best_intron_coord;
  ajint gpos, epos;
  ajint score;
  ajint diagonal, delete_genome, delete_est, intron;
  char *gseq, *eseq, g;
  ajint max;
  ajint is_acceptor;
  EstPCoord m1, m2, m3;
  EstPCoord midpt1, midpt2, best_intron_midpt;
  char *splice_sites_str = ajSeqChar(splice_sites);

  AJCNEW (score1, ajSeqLen(est)+1);
  AJCNEW (score2, ajSeqLen(est)+1);
  
  s1 = score1+1;
  s2 = score2+1;


  AJCNEW (midpt1, ajSeqLen(est)+1);
  AJCNEW (midpt2, ajSeqLen(est)+1);
  
  m1 = midpt1+1;
  m2 = midpt2+1;

  AJCNEW (best_intron_coord, ajSeqLen(est)+1);
  AJCNEW (best_intron_score, ajSeqLen(est)+1);
  AJCNEW (best_intron_midpt, ajSeqLen(est)+1);

  
  gdup = ajSeqNewS(genome);
  edup = ajSeqNewS(est);
  ajSeqToLower (gdup);
  ajSeqToLower (edup);
  gseq = ajSeqChar(gdup);
  eseq = ajSeqChar(edup);

  middle++;


  /* initialise the boundary: We want the alignment to start at [0,0] */

  for(epos=0;epos<ajSeqLen(est);epos++)
    {
      s1[epos] = MINUS_INFINITY;
      best_intron_score[epos] = MINUS_INFINITY;
    }

  for(gpos=0;gpos<ajSeqLen(genome);gpos++)
    {
      s3 = s1; s1 = s2; s2 = s3;
      m3 = m1; m1 = m2; m2 = m3;

      g = gseq[gpos];

      if ( splice_sites && ( splice_sites_str[gpos] & ACCEPTOR ) )
	is_acceptor = 1;	/* gpos is last base of putative intron */
      else
	is_acceptor = 0;

      
/* boundary conditions */
      
      s1[-1] = MINUS_INFINITY;

/* the meat */

      for(epos=0;epos<ajSeqLen(est);epos++)
	{
	  /* align est and genome */

	  diagonal = s2[epos-1] + lsimmat[(ajint)g][(ajint)eseq[epos]];
	  
	  /* single deletion in est */

	  delete_est = s1[epos-1] - gap_penalty;

	  /* single deletion in genome */

	  delete_genome = s2[epos] - gap_penalty;

	  /* intron in genome, possibly modified by
	     donor-acceptor splice sites */

	  if ( is_acceptor &&
	      (splice_sites_str[best_intron_coord[epos]] & DONOR ) )
	    intron = best_intron_score[epos] - splice_penalty;
	  else
	    intron = best_intron_score[epos] - intron_penalty;
	    
	  if ( delete_est > delete_genome )
	    max = delete_est;
	  else
	    max = delete_genome;

	  if ( diagonal > max )
	    max = diagonal;
	  
	  if ( intron > max )
	    max = intron;

	  if ( max == diagonal )
	    {
	      s1[epos] = diagonal;
	      if ( epos == middle ) 
		{
		  m1[epos].left = gpos-1;
		  m1[epos].right = gpos;
		}
	      else
		m1[epos] = m2[epos-1];
	    }
	  else if ( max == delete_est )
	    {
	      s1[epos] = delete_est;
	      if ( epos == middle )
		{
		  m1[epos].left = gpos;
		  m1[epos].right = gpos;
		}
	      else
		m1[epos] = m1[epos-1];
	    }
	  else if ( max == delete_genome )
	    {
	      s1[epos] = delete_genome;
	      m1[epos] = m2[epos];
	    }
	  else			/* intron */
	    {
	      s1[epos] = intron;
	      m1[epos] = best_intron_midpt[epos];
	    }

	  if ( best_intron_score[epos] < s1[epos] )
	    {
	      best_intron_score[epos] = s1[epos];
	      best_intron_coord[epos] = gpos;
	      best_intron_midpt[epos] = m1[epos];
	    }
	}
    }

  *gleft = m1[ajSeqLen(est)-1].left;
  *gright = m1[ajSeqLen(est)-1].right;
  score = s1[ajSeqLen(est)-1];

  if ( verbose ) {
    estIndent();
    (void) ajDebug ("midpt score %d middle %d gleft %d gright %d "
		    "est %d genome %d\n",
		    score, middle-1, *gleft, *gright, ajSeqLen(est),
		    ajSeqLen(genome) );
  }

  AJFREE (score1);
  AJFREE (score2);
  AJFREE (midpt1);
  AJFREE (midpt2);
  AJFREE (eseq);
  AJFREE (gseq);
  AJFREE (best_intron_score);
  AJFREE (best_intron_coord);

  return score;
}



/* @funcstatic estPairRemember ************************************************
**
** Recall saved pair values for row and column
**
** @param [r] col [ajint] Current column
** @param [r] row [ajint] Current row
**
** @return [ajint] Row number
** @@
******************************************************************************/

static ajint estPairRemember( ajint col, ajint row ) {

  EstOSavePair rp;
  ajint left, right, middle, d;
  ajint bad;

  if ( ! rpairs_sorted ) {
    qsort( rpair, rpairs, sizeof(EstOSavePair), estSavePairCmp );
    rpairs_sorted = 1;
  }

  rp.col = col;
  rp.row = row;

  left = 0;
  right = rpairs-1;

  ajDebug ("estPairRemember left: %d right: %d rp rp.col rp.row\n",
	   left, right, rp, rp.col, rp.row);
  
/* MATHOG, changed flow somewhat, added "bad" variable, inverted some
tests ( PLEASE CHECK THEM!  I did this because the original version
had two ways to drop into the failure code, but one of them was only
evident after careful reading of the code.  */

  if ( (estSavePairCmp(&rpair[left],&rp ) > 0 ) ||
       (estSavePairCmp(&rpair[right],&rp ) < 0 ) ) {
       bad = 1; /*MATHOG, preceding test's logic was inverted */
  }
  else {
    bad = 0;
    while( right-left > 1 ) {	/* binary check for row/col */
      middle = (left+right)/2;
      d = estSavePairCmp( &rpair[middle], &rp );
      if ( d < 0 ) 
	left = middle;
      else if ( d >= 0 )
	right = middle;
    }
     ajDebug (
	      "col %d row %d found right: col %d row %d left: col %d row %d\n",
	      col, row, rpair[right].col, rpair[right].row, rpair[left].col,
	      rpair[left].row );

/* any of these fail indicates failure */
/*MATHOG, next test's logic was inverted */
    if ( estSavePairCmp( &rpair[right], &rp ) < 0 ||
	 rpair[left].col != col ||
         rpair[left].row >= row ) {
      ajDebug("estPairRemember => bad2\n");
      ajDebug("estSavePairCmp( %d+%d, %d+%d) %d\n",
	      rpair[right].col, rpair[right].row,
	      rp.col, rp.row,
	      estSavePairCmp( &rpair[right], &rp ));
      ajDebug("rpair[left].col %d %d\n", rpair[left].col, col);
      ajDebug("rpair[left].row %d %d\n", rpair[left].row, row);
      bad = 2;
    }
  }

 /* error - this should never happen  */

  if(bad != 0){
    (void) ajFatal("ERROR in estPairRemember() left: %d (%d %d) right: %d (%d %d) "
		   "col: %d row: %d, bad: %d\n",
		   left, rpair[left].col, rpair[left].row, right,
		   rpair[right].col, rpair[right].row, col, row, bad);
  }

  return rpair[left].row;
}

/* @funcstatic estSavePairCmp ************************************************
**
** Compare two EstPSavePair values. Return the column difference, or if
** the columns are the same, return the row difference.
**
** A value of zero means the two RPAIRS are identical.
**
** @param [r] a [const void*] First value
** @param [r] b [const void*] Second value
**
** @return [ajint] difference.
** @@
******************************************************************************/

static ajint estSavePairCmp( const void *a, const void *b ) {

  EstPSavePair A = (EstPSavePair)a;
  EstPSavePair B = (EstPSavePair)b;
  ajint n = A->col - B->col;

  if ( n == 0 )
    n = A->row - B->row;

  return n;
}

/* @funcstatic estPairInit ***************************************************
**
** Initialise the rpair settings
**
** @param [r] max_bytes [ajint] Maximum memory size (bytes)
**
** @return [void]
** @@
******************************************************************************/

static void estPairInit ( ajint max_bytes ) {

  limit_rpair_size = max_bytes/sizeof(EstPSavePair);
  estPairFree();
}

/* @funcstatic estPairFree **************************************************
**
** Free the rpairs data structure
**
** @return [void]
** @@
******************************************************************************/

static void estPairFree(void) {

  (void) ajDebug("estPairFree: rpairs: %d rpair: %x\n", rpairs, rpair);
  if ( rpair ) AJFREE (rpair);
  rpair = NULL;
  rpair_size = 0;
  rpairs = 0;
  rpairs_sorted = 0;
}

/* @funcstatic estDoNotForget *************************************************
**
** Saving rpairs row and column values.
**
** @param [r] col [ajint] Current column
** @param [r] row [ajint] Current row
**
** @return [ajint] o upon error.
** @@
******************************************************************************/

static ajint estDoNotForget( ajint col, ajint row ) {

  if ( rpairs >= limit_rpair_size ) {
    ajErr ("rpairs %d beyond maximum %d", rpairs+1, limit_rpair_size);
    ajErr ("increase space threshold to repeat this search");
    return 0; /* failure - ran out of memory */
  }
  if ( rpairs >= rpair_size ) {
    rpair_size = ( rpairs == 0 ? 10000 : 2*rpairs );

    if ( rpair_size > limit_rpair_size ) /* enforce the limit */
      rpair_size = limit_rpair_size;

    AJCRESIZE (rpair, rpair_size); 
    if (verbose) {
      (void) ajDebug ("rpairs %d allocated rpair_size %d rpair: %x\n",
	       rpairs, rpair_size, rpair);
      (void) ajDebug ("test rpair[0] %x col %d row %d\n",
		      &rpair[0], rpair[0].col, rpair[0].row);
      (void) ajDebug ("test rpair[%d] %x col %d row %d\n",
		      rpairs, &rpair[rpairs],
		      rpair[rpairs].col, rpair[rpairs].row);
      (void) ajDebug ("test rpair[%d] %x col %d row %d\n",
		      rpair_size-1, &rpair[rpair_size-1],
		      rpair[rpair_size-1].col, rpair[rpair_size-1].row);
    }
  }
  rpair[rpairs].col = col;
  rpair[rpairs].row = row;

  rpairs++;
  rpairs_sorted = 0;

  return 1; /* success */
}

/* @funcstatic estIndent ******************************************************
**
** Indent report by printing spaces to standard output.
**
** @return [void]
** @@
******************************************************************************/

static void estIndent( void) {

  ajint n = indentation;

  while(n--)
    (void) fputc(' ',stdout);
}

/* @func embEstOutBlastStyle *************************************************
**
** output in blast style.
**
** @param [r] blast [AjPFile] Output file
** @param [r] genome [AjPSeq] Genomic sequence
** @param [r] est [AjPSeq] EST sequence
** @param [r] ge [EmbPEstAlign] Genomic EST alignment
** @param [r] match [ajint] Match score
** @param [r] mismatch [ajint] Mismatch penalty
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] gapped [ajint] Boolean. 1 = write a gapped alignment
** @param [r] reverse [ajint] Boolean. 1 = reverse alignment
**
** @return [void]
** @@
******************************************************************************/

void embEstOutBlastStyle ( AjPFile blast, AjPSeq genome, AjPSeq est,
			EmbPEstAlign ge, ajint match, ajint mismatch,
			ajint gap_penalty, ajint intron_penalty,
			ajint splice_penalty, ajint gapped, ajint reverse  ) {

  ajint gsub, gpos, esub, epos, tsub, p;
  ajint matches=0, len=0, m;
  ajint total_matches=0, total_len=0;
  float percent;
  char *genomestr = ajSeqChar(genome);
  char *eststr = ajSeqChar(est);

  if (verbose)
    ajDebug ("debugging set to %d\n", debug);

  gsub = gpos = ge->gstart;
  esub = epos = ge->estart;

  if (verbose) {
    ajDebug("blast_style_output\n");
    ajDebug("gsub %d esub %d\n", gsub, esub);
  }

  if ( blast )
    {
      tsub = 0;
      for(p=0;p<ge->len;p++)
	if ( ge->align_path[p] <= INTRON )
	  {
	    estWriteMsp( blast, &matches, &len, &tsub, genome, gsub, gpos,
		      est, esub, epos, reverse, gapped);
	    if ( gapped )
	      {
		if ( ge->align_path[p] == INTRON )
		  {
		      ajFmtPrintF( blast,
			      "?Intron  %5d %5.1f %5d %5d %-12s\n",
			      -intron_penalty, (float) 0.0, gpos+1,
			      gpos+ge->align_path[p+1], ajSeqName(genome) ); 
		  }
		else /* proper intron */
		  {
		    if ( ge->align_path[p] == FORWARD_SPLICED_INTRON )
		      ajFmtPrintF( blast,
			      "+Intron  %5d %5.1f %5d %5d %-12s\n",
			      -splice_penalty, (float) 0.0, gpos+1,
			      gpos+ge->align_path[p+1], ajSeqName(genome) ); 
		    else
		      ajFmtPrintF( blast,
			      "-Intron  %5d %5.1f %5d %5d %-12s\n",
			      -splice_penalty, (float) 0.0, gpos+1,
			      gpos+ge->align_path[p+1], ajSeqName(genome) ); 

		  }
	      }

	    gpos += ge->align_path[++p];
	    esub = epos;
	    gsub = gpos;
	  }
	else if ( ge->align_path[p] == DIAGONAL )
	  {
	    m = lsimmat[(ajint)genomestr[(ajint)gpos]][(ajint)eststr[(ajint)epos]];
	    tsub += m;
	    if ( m > 0 )
	      {
		matches++;
		total_matches++;
	      }
	    len++;
	    total_len++;
	    gpos++;
	    epos++;
	  }
	else if ( ge->align_path[p] == DELETE_EST )
	  {
	    if ( gapped )
	      {
		tsub -= gap_penalty;
		epos++;
		len++;
		total_len++;
	      }
	    else
	      {
		estWriteMsp( blast, &matches, &len, &tsub, genome, gsub,
			  gpos, est, esub, epos, reverse, gapped );
		epos++;
		esub = epos;
		gsub = gpos;
	      }
	  }
	else if ( ge->align_path[(ajint)p] == DELETE_GENOME )
	  {
	    if ( gapped )
	      {
		tsub -= gap_penalty;
		gpos++;
		total_len++;
		len++;
	      }
	    else
	      {
		estWriteMsp( blast, &matches, &len, &tsub, genome, gsub,
			   gpos, est, esub, epos, reverse, gapped );
		gpos++;
		esub = epos;
		gsub = gpos;
	      }
	  }
      estWriteMsp( blast, &matches, &len, &tsub, genome, gsub, gpos, est,
		 esub, epos, reverse, gapped );

      if ( gapped )
	{
	  if ( total_len > 0 )
	    percent = (total_matches/(float)(total_len))*(float)100.0;
	  else
	    percent = (float) 0.0;

	  if ( reverse )
	    ajFmtPrintF( blast,
		     "\nSpan     %5d %5.1f %5d %5d %-12s %5d %5d %-12s  %S\n",
		     ge->score, percent, ge->gstart+1, ge->gstop+1,
		     ajSeqName(genome), ajSeqLen(est)-ge->estart,
		     ajSeqLen(est)-ge->estop,
		     ajSeqName(est), ajSeqGetDesc(est) );
	  else
	    ajFmtPrintF( blast,
		    "\nSpan     %5d %5.1f %5d %5d %-12s %5d %5d %-12s  %S\n",
		    ge->score, percent, ge->gstart+1, ge->gstop+1,
		    ajSeqName(genome), ge->estart+1, ge->estop+1,
                    ajSeqName(est), ajSeqGetDesc(est) );
	}

    }
}
/* @funcstatic estWriteMsp ***************************************************
**
** write out the MSP (maximally scoring pair).
**
** @param [r] ofile [AjPFile] Output file
** @param [r] matches [ajint*] Number of matches found
** @param [r] len [ajint*] Length of alignment
** @param [r] tsub [ajint*] Score
** @param [r] genome [AjPSeq] Genomic sequence
** @param [r] gsub [ajint] Genomic start position
** @param [r] gpos [ajint] Genomic end position
** @param [r] est [AjPSeq] EST sequence
** @param [r] esub [ajint] EST start position
** @param [r] epos [ajint] EST end position
** @param [r] reverse [ajint] Boolean 1=reverse the EST sequence
** @param [r] gapped [ajint] Boolean 1=full gapped alignment
**                         0=display ungapped segment
**
** @return [void]
** @@
******************************************************************************/

static void estWriteMsp( AjPFile ofile, ajint *matches, ajint *len, ajint *tsub,
	       AjPSeq genome, ajint gsub, ajint gpos, AjPSeq est,
	       ajint esub, ajint epos, ajint reverse, ajint gapped) {

  float percent;

  if ( *len > 0 )
    percent = (*matches/(float)(*len))*(float)100.0;
  else
    percent = (float) 0.0;

  if ( percent > 0 )
    {
      if ( gapped )
	ajFmtPrintF( ofile, "Exon     " );
      else
	ajFmtPrintF( ofile, "Segment  " );
      if ( reverse )
	ajFmtPrintF(ofile, "%5d %5.1f %5d %5d %-12s %5d %5d %-12s  %S\n", 
		*tsub, percent, gsub+1, gpos, ajSeqName(genome),
		ajSeqLen(est)-esub,  ajSeqLen(est)-epos+1, ajSeqName(est), ajSeqGetDesc(est) );
      else
	ajFmtPrintF(ofile, "%5d %5.1f %5d %5d %-12s %5d %5d %-12s  %S\n",
		*tsub, percent, gsub+1, gpos, ajSeqName(genome),
		esub+1, epos, ajSeqName(est), ajSeqGetDesc(est) );
    }
  *matches = *len = *tsub = 0;
}

