/* @source distmat application
**
**   Calculates the evolutionary distance matrix for a set of 
** aligned sequences. Measures the pairwise evolutionary 
** distances between aligned sequences. Distances are expressed
** as substitutions per 100 bases or a.a.'s.
**
** Methods to correct for multiple substitutions at a site:
**  Nucleic Acid- Kimura's 2 parameter, Tajima-Nei, Jin-Nei
**                Gamma distance or Tamura methods.
**  Protein     - Kimura method.
**  Nucleic Acid or Protein - Jukes-Cantor method
**
**
** @author: Copyright (C) Tim Carver (tcarver@hgmp.mrc.ac.uk)
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
*****************************************************************************/


#include "emboss.h"
#include "math.h"
#define IUBFILE "Ebases.iub"

AjIUB aj_base_iubS[256];      /* base letters and their alternatives */



static AjPFloat2d calc_match(char** seqcharptr, ajint len, ajint nseqs,
                             AjBool ambig, AjBool nuc, AjPFloat2d* gap);
static AjPFloat2d uncorrected(AjPFloat2d match, AjPFloat2d gap, float gapwt,
                              ajint len, ajint nseqs, AjBool nuc);

/* correction methods for multiple subst. */
static AjPFloat2d JukesCantor(AjPFloat2d match, AjPFloat2d gap, float gapwt,
                              ajint mlen, ajint nseqs, AjBool nuc);
static AjPFloat2d Kimura(char** seqcharptr, ajint len, ajint nseqs);
static AjPFloat2d KimuraProt(char** seqcharptr, ajint mlen, ajint nseqs);
static AjPFloat2d Tamura(char** seqcharptr, ajint len, ajint nseqs);
static AjPFloat2d TajimaNei(char** seqcharptr, AjPFloat2d match,
                            ajint mlen, ajint nseqs, AjBool nuc);
static AjPFloat2d JinNei(char** seqcharptr, ajint len, ajint nseqs, 
                         AjBool calc_a, float var_a);

/* output routine and misc routines */
static void outputDist(AjPFile outf, ajint nseqs, ajint mlen, AjPSeqset seqset,
                       AjPFloat2d match, AjPFloat2d gap, float gapwt,
                       ajint method, AjBool ambig, AjBool nuc, ajint posn,
                       ajint incr);
static float checkambigNuc(char m1, char m2);
static float checkambigProt(ajint t1, ajint t2);
static void checkSubs(ajint t1, ajint t2, ajint* trans, ajint* tranv);
static void checkRY(ajint t1, ajint t2, ajint* trans, ajint* tranv);

static char** getSeq(AjPSeqset seqset, ajint nseqs, ajint mlen, ajint incr,
                     ajint posn, ajint* len);


int main (int argc, char * argv[])
{
    ajint nseqs;
    ajint mlen;
    ajint len;
    ajint i;
    ajint method;
    ajint incr;
    ajint posn;

    float gapwt;
    float var_a;

    char  *p;
    char **seqcharptr;

    AjPSeqset seqset=NULL;
    AjPFloat2d match=NULL;
    AjPFloat2d matchDist=NULL;
    AjPFloat2d gap=NULL;
    AjPStr* methodlist;
    AjPFile outf=NULL;
    
    AjBool nuc=ajFalse;
    AjBool ambig;
    AjBool calc_a;




    embInit ("distmat", argc, argv);

    seqset = ajAcdGetSeqset ("msf");

    if(ajSeqsetIsNuc(seqset))             /* nucleic acid seq */
       nuc = ajTrue;
    else if ( ajSeqsetIsProt(seqset))
       nuc = ajFalse;
    else
       ajExit();


    outf   = ajAcdGetOutfile("outf");     /* output filename  */
    ambig  = ajAcdGetBool("ambiguous");
    gapwt  = ajAcdGetFloat("gapweight");
    if(nuc)
      methodlist = ajAcdGetList("nucmethod");
    else
      methodlist = ajAcdGetList("protmethod");
    posn   = ajAcdGetInt("position");
    calc_a = ajAcdGetBool("calculatea");
    var_a  = ajAcdGetFloat("parametera");


    incr = 1;                            /* codons to analyse */
    if(posn >= 1 && posn <= 3 && nuc)
    {
      incr = 3;
      posn--;
    }     
    else if(posn == 123)
      posn = 0;
    else if(posn == 23 || posn == 13 || posn != 12)
      ajFatal("Choose base positions 1, 2, 3, 12, or 123");

    (void) ajStrToInt(methodlist[0], &method);

    nseqs = ajSeqsetSize(seqset);
    if(nseqs<2)
      ajFatal("Insufficient sequences (%d) to create a matrix",nseqs);

    mlen = ajSeqsetLen(seqset);
    for(i=0;i<nseqs;++i)                 /* check seqs are same length */
    {
      p = ajSeqsetSeq(seqset,i);
      if(strlen(p)!=mlen)
          ajWarn("Sequence lengths are not equal!");
      ajSeqsetToUpper(seqset);
    }

    seqcharptr = getSeq(seqset,nseqs,mlen,incr,posn,&len);

    /* look at pairs of seqs for matches */
    if(method == 0 || method == 1 || method == 4 )
      match = calc_match(seqcharptr,len,nseqs,ambig,nuc,
                         &gap);


    /* No correction made for multiple subst. */
    if(method == 0)
      matchDist = uncorrected(match,gap,gapwt,len,nseqs,nuc);

    /* adjust for multiple substs */
    else if(method == 1)                              /* for nucl. & prot. */
      matchDist = JukesCantor(match,gap,gapwt,len,nseqs,nuc);
    else if(method == 2)
      if(nuc)
        matchDist = Kimura(seqcharptr,len,nseqs);
      else
        matchDist = KimuraProt(seqcharptr,mlen,nseqs);
    else if(method == 3)                               /* for nucl. */
      matchDist = Tamura(seqcharptr,len,nseqs);
    else if(method == 4)
      matchDist = TajimaNei(seqcharptr,match,mlen,nseqs,nuc);
    else if(method == 5)
      matchDist = JinNei(seqcharptr,len,nseqs,calc_a,var_a);

    outputDist(outf,nseqs,mlen,seqset,matchDist,gap,gapwt,
               method,ambig,nuc,posn,incr);

    /* free allocated memory */
    for(i=0;i<nseqs;i++)
      ajCharFree(seqcharptr[i]);
    AJFREE(seqcharptr);

    if(method == 0 || method == 1 || method == 4 )
        ajFloat2dDel(&gap);

    ajExit ();
    return 0;

}




/* @funcstatic Tamura ***************************************************
**
** Tamura distance - nucleic acids only.
**
** K Tamura, Mol. Biol. Evol. 1992, 9, 678.
**
** @param [r] seqcharptr [char**] Array of sequences as C strings 
** @param [r] len [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @return [AjPFloat2d] corrected distance matrix
**
*************************************************************************/
static AjPFloat2d Tamura(char** seqcharptr, ajint len, ajint nseqs)
{

    ajint i;
    ajint j;
    ajint k;

    ajint t1;
    ajint t2;
    ajint m;
    ajint trans;
    ajint tranv;

    float P;
    float Q;
    float D;
    float C;
    float geecee;
    float cgap;

    AjPInt2d Ptrans=NULL;
    AjPInt2d Qtranv=NULL;
    AjPInt2d score=NULL;

    AjPFloat2d matDist=NULL;
    AjPFloat2d gap=NULL;
    AjPFloat2d GC=NULL;

    Ptrans  = ajInt2dNew();
    Qtranv  = ajInt2dNew();
    score   = ajInt2dNew();

    matDist = ajFloat2dNew();
    GC      = ajFloat2dNew();
    gap     = ajFloat2dNew();


    /* initialise array */
    for(i=0;i<nseqs;++i)
    {
      for(j=0;j<nseqs;++j)
      {
          ajFloat2dPut(&matDist,i,j,0.);
          ajFloat2dPut(&GC,i,j,0.);
          ajFloat2dPut(&gap,i,j,0.);
          ajInt2dPut(&Ptrans,i,j,0);
          ajInt2dPut(&Qtranv,i,j,0);
          ajInt2dPut(&score,i,j,0);
      }
    }

    /* calc GC content for each seq for each pair */
    /* of seq - ignoring gap posns in both seqs   */
    for(i=0;i<nseqs;++i)
    {
      for(j=0; j<nseqs; j++)
      {
        for(k=0;k<len;++k)
        {
          cgap = ajFloat2dGet(gap,i,j);
          geecee = ajFloat2dGet(GC,i,j);

          t1 = toupper((int) seqcharptr[j][k]);
          t2 = toupper((int) seqcharptr[i][k]);


          if(strchr("-NXWRYMKBVDH",t1) || strchr("-NXWRYMKBVDH",t2))
             ++cgap;
          else if(strchr("GCS",t2))
             ++geecee;

          ajFloat2dPut(&GC,i,j,geecee);
          ajFloat2dPut(&gap,i,j,cgap);
        }
      }
    }

    /* fraction GC content */
    for(i=0;i<nseqs;++i)
    {
      for(j=0; j<nseqs; j++)
      {
        cgap = ajFloat2dGet(gap,i,j);
        geecee = ajFloat2dGet(GC,i,j)/((float)len-cgap);
        ajFloat2dPut(&GC,i,j,geecee);
      }
    }

    /* calc transition & transversion subst.'s */
    for(i=0;i<nseqs;i++)               
    {
      for(j=i+1;j<nseqs;j++)            
      {
        for(k=0; k< len; k++)
        {
          t1 = toupper((int) seqcharptr[i][k]);
          t2 = toupper((int) seqcharptr[j][k]);
          if(!strchr("-",t2) && !strchr("-",t1))
          {
            trans = ajInt2dGet(Ptrans,i,j);
            tranv = ajInt2dGet(Qtranv,i,j);

            checkSubs(t1,t2,&trans,&tranv);
            ajInt2dPut(&Ptrans,i,j,trans); 
            ajInt2dPut(&Qtranv,i,j,tranv);
            m = ajInt2dGet(score,i,j)+1;
            ajInt2dPut(&score,i,j,m);
          }
        }
      }
    }


    /* calc distance matrix */
    for(i=0;i<nseqs;i++)             
    {
      for(j=i+1;j<nseqs;j++)            
      {
        C = ajFloat2dGet(GC,j,i)+ajFloat2dGet(GC,i,j);
        C = C - (2*ajFloat2dGet(GC,j,i)*ajFloat2dGet(GC,i,j));
        P = (float)ajInt2dGet(Ptrans,i,j)/(float)ajInt2dGet(score,i,j);
        Q = (float)ajInt2dGet(Qtranv,i,j)/(float)ajInt2dGet(score,i,j);

        if(P != 0.)
          P = P/C;
        D = -(C*log(1-P-Q)) - (0.5*(1-C)*log(1-2*Q));
        ajFloat2dPut(&matDist,i,j,D); 
      }
    }
  
    ajInt2dDel(&Ptrans);
    ajInt2dDel(&Qtranv);
    ajInt2dDel(&score);
    ajFloat2dDel(&GC);
    ajFloat2dDel(&gap);

    return matDist;   

}




/* @funcstatic Kimura ***************************************************
**
** Kimura 2-parameter distance - nucleic acid only.
**
** M Kimura, J. Mol. Evol., 1980, 16, 111.
**
** @param [r] seqcharptr [char**] Array of sequences as C strings 
** @param [r] len [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @return [AjPFloat2d] corrected distance matrix
**
*************************************************************************/

static AjPFloat2d Kimura(char** seqcharptr, ajint len, ajint nseqs)
{
    ajint i;
    ajint j;
    ajint k;

    ajint t1;
    ajint t2;

    float P;
    float Q;
    float D;

    ajint m;
    ajint trans;
    ajint tranv;

    AjPFloat2d matDist=NULL;
    AjPInt2d Ptrans=NULL;
    AjPInt2d Qtranv=NULL;
    AjPInt2d match=NULL;



    matDist = ajFloat2dNew();
    Ptrans  = ajInt2dNew();
    Qtranv  = ajInt2dNew();
    match   = ajInt2dNew();

 
    /* initialise array */
    for(i=0;i<nseqs;++i)
    {
      for(j=i+1;j<nseqs;++j)
      {
          ajFloat2dPut(&matDist,i,j,0.);
          ajInt2dPut(&Ptrans,i,j,0);
          ajInt2dPut(&Qtranv,i,j,0);
          ajInt2dPut(&match,i,j,0);
      }
    }

    /* calc transition & transversion subst.'s */
    for(i=0;i<nseqs;i++)               
    {
      for(j=i+1;j<nseqs;j++)            
      {
        for(k=0; k< len; k++)
        {
          t1 = toupper((int) seqcharptr[i][k]);
          t2 = toupper((int) seqcharptr[j][k]);

          if(!strchr("-",t2) && !strchr("-",t1))
          {
            trans = ajInt2dGet(Ptrans,i,j);
            tranv = ajInt2dGet(Qtranv,i,j);

            checkSubs(t1,t2,&trans,&tranv);
            checkRY(t1,t2,&trans,&tranv);
            ajInt2dPut(&Ptrans,i,j,trans); 
            ajInt2dPut(&Qtranv,i,j,tranv);
            m = ajInt2dGet(match,i,j)+1;
            ajInt2dPut(&match,i,j,m);
          }
        }
      }
    }


    /* calc distance matrix */
    for(i=0;i<nseqs;i++)             
    {
      for(j=i+1;j<nseqs;j++)            
      {
        P = (float)ajInt2dGet(Ptrans,i,j)/(float)ajInt2dGet(match,i,j);
        Q = (float)ajInt2dGet(Qtranv,i,j)/(float)ajInt2dGet(match,i,j);
        D = -0.5*log((1-(2*P)-Q)*sqrt(1-(2*Q)));
     
        ajFloat2dPut(&matDist,i,j,D); 
      }
    }
  
    ajInt2dDel(&Ptrans);
    ajInt2dDel(&Qtranv);
    ajInt2dDel(&match);

    return matDist;   

}



/* @funcstatic KimuraProt ***********************************************
**
** Kimura protein distance
**
** @param [r] seqcharptr [char**] Array of sequences as C strings 
** @param [r] mlen [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @return [AjPFloat2d] corrected distance matrix
**
*************************************************************************/

static AjPFloat2d KimuraProt(char** seqcharptr, ajint mlen, ajint nseqs)
{
    ajint i;
    ajint j;
    ajint k;
    ajint mi;

    char m1;
    char m2;

    float D;
    float m;

    AjPFloat2d matDist=NULL;
    AjPFloat2d match=NULL;
    AjPInt2d scored=NULL;



    matDist = ajFloat2dNew();
    match   = ajFloat2dNew();
    scored  = ajInt2dNew();

 
    /* initialise array */
    for(i=0;i<nseqs;++i)
    {
      for(j=i+1;j<nseqs;++j)
      {
          ajFloat2dPut(&matDist,i,j,0.);
          ajFloat2dPut(&match,i,j,0);
          ajInt2dPut(&scored,i,j,0);
      }
    }

    /* calc matches */
    for(i=0;i<nseqs;i++)               
    {
      for(j=i+1;j<nseqs;j++)            
      {
        for(k=0; k< mlen; k++)
        {
          m1 = seqcharptr[i][k];
          m2 = seqcharptr[j][k];
          if(m1 != '-' && m2 != '-')
          {
            
            m = ajFloat2dGet(match,i,j)+
                checkambigProt(toupper((int) m1),toupper((int) m2));
            ajFloat2dPut(&match,i,j,m);
            mi = ajInt2dGet(scored,i,j)+1;
            ajInt2dPut(&scored,i,j,mi);
          }
        }
      }
    }


    /* calc distance matrix */
    for(i=0;i<nseqs;i++)             
    {
      for(j=i+1;j<nseqs;j++)            
      {
        D = 1.-(ajFloat2dGet(match,i,j)/
                (float)ajInt2dGet(scored,i,j));
        D = -log(1-D-(0.2*D*D));
        ajFloat2dPut(&matDist,i,j,D); 
      }
    }
  
    ajInt2dDel(&scored);
    ajFloat2dDel(&match);

    return matDist;   

}


/* @funcstatic calc_match *************************************************
**
** Sum the no. of matches between each pair of sequence in an 
** alignment.
**
** @param [r] seqcharptr [char**] Array of sequences as C strings 
** @param [r] len [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @param [r] ambig [AjBool] Ambiguity codes
** @param [r] nuc [AjBool] Nucleotide
** @param [r] gap [AjPFloat2d*] Gaps
** @return [AjPFloat2d] corrected distance matrix
**
*************************************************************************/

static AjPFloat2d calc_match(char** seqcharptr, ajint len, ajint nseqs,
                             AjBool ambig, AjBool nuc, AjPFloat2d* gap)
{
 
    ajint i;
    ajint j;
    ajint k;

    char m1;
    char m2;

    float m;
    AjPFloat2d match=NULL;



    match = ajFloat2dNew();
    *gap  = ajFloat2dNew();


    /* initialise arrays */
    for(i=0;i<nseqs;++i)
    {
      for(j=0;j<nseqs;++j)
      {
          ajFloat2dPut(&match,i,j,0);
          ajFloat2dPut(gap,i,j,0);
      }
    }


    for(i=0;i<nseqs;i++)          
    { 
      for(j=i+1;j<nseqs;j++)       
      {
        for(k=0; k< len; k++)
        {
          m1 = seqcharptr[i][k];
          m2 = seqcharptr[j][k];
          if(ambig && nuc)                /* using -ambiguous */
          {
            m = checkambigNuc(m1,m2);
            m = ajFloat2dGet(match,i,j)+m;
            ajFloat2dPut(&match,i,j,m);
          }
          else if( ambig && !nuc && m1 != '-' && m2 != '-') 
          {
            m = checkambigProt(toupper((int) m1),toupper((int) m2));
            m = ajFloat2dGet(match,i,j)+m;
            ajFloat2dPut(&match,i,j,m);
          }
          else if( m2 == m1 && m1 != '-' )
          {
            m = ajFloat2dGet(match,i,j)+1;
            ajFloat2dPut(&match,i,j,m); 
          }

          if( m1 == '-' || m2 == '-' )    /* gap in seq */
          {
            m = ajFloat2dGet(*gap,i,j)+1;
            ajFloat2dPut(gap,i,j,m);
          }
        }
      }
    }


    return match;

}



/* @funcstatic uncorrected ***********************************************
**
** No correction for multiple substitutions is used in the calculation
** of the distance matrix.
**
**        D = p-distance = 1 - (matches/(posns_scored + gaps*gap_penalty))
**
** @param [r] match [AjPFloat2d] Matches
** @param [r] gap [AjPFloat2d] Gaps
** @param [r] gapwt [float] Gap weight
** @param [r] len [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @param [r] nuc [AjBool] Nucleotide
** @return [AjPFloat2d] uncorrected distance matrix D
**
*************************************************************************/

static AjPFloat2d uncorrected(AjPFloat2d match, AjPFloat2d gap, float gapwt,
                              ajint len, ajint nseqs, AjBool nuc)
{

    ajint i;
    ajint j;

    float m;
    float g;
    float D;

    AjPFloat2d matchUn=NULL;


    matchUn = ajFloat2dNew();
    for(i=0;i<nseqs;++i)
        for(j=0;j<nseqs;++j)
            ajFloat2dPut(&matchUn,i,j,0);

    for(i=0;i<nseqs;i++)       
    {
      for(j=i+1;j<nseqs;j++)    
      {
        m = ajFloat2dGet(match,i,j);    /* no. matches */
        g = ajFloat2dGet(gap,i,j);      /* no. gaps    */

        D = 1 - (m/((float)len-g+(g*gapwt)));

        ajFloat2dPut(&matchUn,i,j,D);
      }
    }

    return matchUn;

}



/* @funcstatic TajimaNei ************************************************
**
** Tajima-Nei correction used for multiple substitutions in the calc
** of the distance matrix. Nucleic acids only.
**
**  D = p-distance = 1 - (matches/(posns_scored + gaps)
**
**  distance = -b * ln(1-D/b) 
**
** Tajima and Nei, Mol. Biol. Evol. 1984, 1, 269.
**
** @param [r] seqcharptr [char**] Array of sequences as C strings
** @param [r] match [AjPFloat2d] Matches
** @param [r] mlen [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @param [r] nuc [AjBool] Nucleotide
** @return [AjPFloat2d] corrected distance matrix
**
*************************************************************************/
static AjPFloat2d TajimaNei(char** seqcharptr, AjPFloat2d match, 
                            ajint mlen, ajint nseqs, AjBool nuc)
{

    ajint i;
    ajint j;
    ajint l;
    ajint ti;
    ajint tj;

    ajint bs;
    ajint bs1;
    ajint pair;
    ajint val;

    AjPInt2d len=NULL;
    AjPInt3d pfreq=NULL;
    AjPInt3d cbase=NULL; 

    float fi;
    float fj;
    float fij;
    float fij2;
    float ci1,ci2;
    float cj1,cj2;
    float slen;
    float h;
    float m;
    float D;
    float b;

    AjPFloat2d matchTN=NULL;


    len   = ajInt2dNew();     /* scored length for pairs */
    pfreq = ajInt3dNew();     /* pair freq between seq's */
    cbase = ajInt3dNew();     /* no. of bases */
    matchTN = ajFloat2dNew();



    for(i=0;i<nseqs;++i)
    {
      for(j=0;j<nseqs;++j)
      {
        ajFloat2dPut(&matchTN,i,j,0);
        ajInt2dPut(&len,i,j,0);
        for(pair=0;pair<6;pair++)
          ajInt3dPut(&pfreq,i,j,pair,0);

        for(bs=0;bs<4;bs++)
          ajInt3dPut(&cbase,i,j,bs,0);
      }
    }

    /* calc content of each seq  - ignoring gaps */
    for(i=0;i<nseqs;i++)    
    {
       for(j=0;j<nseqs;j++)
       {
         for(l=0; l< mlen; l++)
         {
           ti = toupper((int) seqcharptr[i][l]);

           if(!strchr("-NXWMKBVDH",ti) )
           {
             tj = toupper((int) seqcharptr[j][l]);

             if(!strchr("-NXWMKBVDH",tj))
             {
               slen = ajInt2dGet(len,i,j)+1;
               ajInt2dPut(&len,i,j,slen);
               if(strchr("G",ti))
               {
                 val = ajInt3dGet(cbase,i,j,3)+1;
                 ajInt3dPut(&cbase,i,j,3,val);
               }
               else if(strchr("C",ti))
               {
                 val = ajInt3dGet(cbase,i,j,2)+1;
                 ajInt3dPut(&cbase,i,j,2,val);
               }
               else if(strchr("T",ti)) 
               {
                 val = ajInt3dGet(cbase,i,j,1)+1;
                 ajInt3dPut(&cbase,i,j,1,val);
               }
               else if(strchr("A",ti))
               {
                 val = ajInt3dGet(cbase,i,j,0)+1;
                 ajInt3dPut(&cbase,i,j,0,val);
               }
             }
         
             if(ti != tj)
             {
         
               if( (strchr("A",ti) && strchr("T",tj)) ||      
                   (strchr("T",ti) && strchr("A",tj)) )
               {                                          /* AT pair */
                 val = ajInt3dGet(pfreq,i,j,0)+1;
                 ajInt3dPut(&pfreq,i,j,0,val);
               }
               else if( (strchr("A",ti) && strchr("C",tj)) || 
                        (strchr("C",ti) && strchr("A",tj)) )
               {                                          /* AC pair */      
                 val = ajInt3dGet(pfreq,i,j,1)+1;
                 ajInt3dPut(&pfreq,i,j,1,val);
               }
               else if( (strchr("A",ti) && strchr("G",tj)) || 
                        (strchr("G",ti) && strchr("A",tj)) )
               {                                          /* AG pair */     
                 val = ajInt3dGet(pfreq,i,j,2)+1;
                 ajInt3dPut(&pfreq,i,j,2,val);
               }
               else if( (strchr("T",ti) && strchr("C",tj)) || 
                        (strchr("C",ti) && strchr("T",tj)) )
               {                                          /* TC pair */      
                 val = ajInt3dGet(pfreq,i,j,3)+1;
                 ajInt3dPut(&pfreq,i,j,3,val);
               }
               else if( (strchr("T",ti) && strchr("G",tj)) ||  
                        (strchr("G",ti) && strchr("T",tj)) )
               {                                          /* TG pair */      
                 val = ajInt3dGet(pfreq,i,j,4)+1;
                 ajInt3dPut(&pfreq,i,j,4,val);
               }                                               
               else if( (strchr("C",ti) && strchr("G",tj)) || 
                        (strchr("G",ti) && strchr("C",tj)) )
               {                                          /* CG pair */     
                 val = ajInt3dGet(pfreq,i,j,5)+1;
                 ajInt3dPut(&pfreq,i,j,5,val);
               }
             }
           }
         }
       }
    }

    /* calc distance matrix */
    for(i=0;i<nseqs;i++)       
    {
      for(j=i+1;j<nseqs;j++) 
      {
        slen = (float)ajInt2dGet(len,i,j);   

        fij2 = 0.;
        for(bs=0;bs<4;bs++)
        {
          fi  = (float)ajInt3dGet(cbase,i,j,bs);
          fj  = (float)ajInt3dGet(cbase,j,i,bs);
          fij = 0.;
          if(fi != 0. && fj != 0.)
            fij = (float)(fi+fj)/(2.*slen);
          fij2 += fij*fij;
        }

        pair = 0;
        h = 0.;
        for(bs=0;bs<3;bs++)
        {
          for(bs1=bs+1;bs1<4;bs1++)
          {
            fij = (float)ajInt3dGet(pfreq,i,j,pair)/slen;
            ci1 = (float)ajInt3dGet(cbase,j,i,bs);
            cj1 = (float)ajInt3dGet(cbase,i,j,bs);
            ci2 = (float)ajInt3dGet(cbase,j,i,bs1);
            cj2 = (float)ajInt3dGet(cbase,i,j,bs1);

            if(fij !=0.)
               h += (0.5*fij*fij)/((ci1+cj1)/(2.*slen) * (ci2+cj2)/(2.*slen));

            pair++;
          }
        }
         
        m = ajFloat2dGet(match,i,j);    /* no. matches */
        D = 1. - m/slen;
        b = 0.5*(1-fij2+((D*D)/h));

        ajFloat2dPut(&matchTN,i,j, (-b*log(1.-(D/b))) );
      }
    }


    ajInt2dDel(&len);
    ajInt3dDel(&cbase);
    ajInt3dDel(&pfreq);

    return matchTN;

}





/* @funcstatic JinNei ***********************************************
**
**  Nucleic acids only.
**
**  Jin and Nei, Mol. Biol. Evol. 82, 7, 1990.
**
** @param [r] seqcharptr [char**] Array of sequences as C strings
** @param [r] mlen [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @param [r] calc_a [AjBool] Calculation
** @param [r] var_a [float] Variable
** @return [AjPFloat2d] corrected distance matrix
*************************************************************************/
static AjPFloat2d JinNei(char** seqcharptr, ajint mlen, ajint nseqs, 
                         AjBool calc_a, float var_a)
{

    ajint i;
    ajint j;
    ajint k;

    ajint t1;
    ajint t2;
    ajint trans;
    ajint tranv;
    ajint slen=0;

    float av;
    float var;
    float dist;
    float P;
    float Q;

    AjPFloat2d matDist=NULL;
    AjPFloat2d cval=NULL;
    AjPFloat2d avL=NULL;

    AjPInt2d Ptrans=NULL;
    AjPInt2d Qtranv=NULL;
    AjPInt2d len=NULL;


    matDist = ajFloat2dNew();
    cval    = ajFloat2dNew();
    avL     = ajFloat2dNew();
    Ptrans  = ajInt2dNew();
    Qtranv  = ajInt2dNew();
    len     = ajInt2dNew();

    /* initialise array */
    for(i=0;i<nseqs;++i)
    {
      for(j=i+1;j<nseqs;++j)
      {
        ajFloat2dPut(&matDist,i,j,0.);
        ajInt2dPut(&Ptrans,i,j,0);
        ajInt2dPut(&Qtranv,i,j,0);
        ajInt2dPut(&len,i,j,0);
        ajFloat2dPut(&avL,i,j,0);
      }
    }


    /* calc transition & transversion subst.'s */
    for(i=0;i<nseqs;i++)
    {
      for(j=i+1;j<nseqs;j++)
      {

        av = ajFloat2dGet(avL,i,j);
        for(k=0; k< mlen; k++)
        {
          t1 = toupper((int) seqcharptr[i][k]);
          t2 = toupper((int) seqcharptr[j][k]);
          if(!strchr("-",t1) && !strchr("-",t2))
          {
            slen = ajInt2dGet(len,i,j)+1;
            ajInt2dPut(&len,i,j,slen);

            trans = 0;
            tranv = 0;
            checkSubs(t1,t2,&trans,&tranv);
            checkRY(t1,t2,&trans,&tranv);
            av+= ((float)trans + (2.*(float)tranv));

            trans+= ajInt2dGet(Ptrans,i,j);
            tranv+= ajInt2dGet(Qtranv,i,j);

            ajInt2dPut(&Ptrans,i,j,trans);
            ajInt2dPut(&Qtranv,i,j,tranv);
          }
        }
        ajFloat2dPut(&avL,i,j,av/slen);
      }
    }


    if(calc_a)       /* calc inverse of coeff of variance */
    {
      for(i=0;i<nseqs;i++)
      {
        for(j=i+1;j<nseqs;j++)
        {
          slen = (float)ajInt2dGet(len,i,j);
          av = (float)( ajInt2dGet(Ptrans,i,j)+
                     (2*ajInt2dGet(Qtranv,i,j)) )/slen;

          av = ajFloat2dGet(avL,i,j);
          var = 0.;

          for(k=0; k< mlen; k++)
          {
            t1 = toupper((int) seqcharptr[i][k]);
            t2 = toupper((int) seqcharptr[j][k]);
            if(!strchr("-",t1) && !strchr("-",t2))
            {
              trans = 0;
              tranv = 0;
              checkSubs(t1,t2,&trans,&tranv);
              checkRY(t1,t2,&trans,&tranv);
 
              var+= (av-(float)(trans+(2*tranv)))*
                    (av-(float)(trans+(2*tranv)));
            }
          }
          var = var/slen;
          ajFloat2dPut(&cval,i,j,(av*av)/var);
        }
      }
    }
    
    /* calc the dist matrix */
    for(i=0;i<nseqs;i++)       
    {
      for(j=i+1;j<nseqs;j++)    
      {
        slen = ajInt2dGet(len,i,j); 
        P = (float)ajInt2dGet(Ptrans,i,j)/slen;
        Q = (float)ajInt2dGet(Qtranv,i,j)/slen;
        if(calc_a)
          var = ajFloat2dGet(cval,i,j);
        else
          var = var_a;

        dist = 0.5*var*( pow(1.-(2*P)-Q,-1./var) +
                    (0.5*pow(1.-(2*Q),-1./var)) - 1.5 );

        ajFloat2dPut(&matDist,i,j,dist);
      }
    }

    ajFloat2dDel(&cval);
    ajFloat2dDel(&avL);
    ajInt2dDel(&Ptrans);
    ajInt2dDel(&Qtranv);
    ajInt2dDel(&len);

    return matDist;

}




/* @funcstatic JukesCantor ***********************************************
**
** Use the Jukes-Cantor method to correct for multiple substitutions in
** the calculation of the distance matrix.
**
**        D = p-distance = 1 - (matches/(posns_scored + gaps*gap_penalty))
**
** distance = -b * ln(1-D/b)        b = 3/4    nucleic acid
**                                    = 19/20  protein
** 
** "Phylogenetic Inference", Swoffrod, Olsen, Waddell and Hillis, 
** in Mol. Systematics, 2nd ed, 1996, Ch 11. Derived from "Evolution
** of Protein Molecules", Jukes & Cantor, in Mammalian Prot. Metab.,
** III, 1969, pp. 21-132.
**
** @param [r] match [AjPFloat2d] Matches
** @param [r] gap [AjPFloat2d] Gaps
** @param [r] gapwt [float] Gap weight
** @param [r] mlen [ajint] Length
** @param [r] nseqs [ajint] Number of sequences
** @param [r] nuc [AjBool] Nucleotide
** @return [AjPFloat2d] corrected distance matrix
**
*************************************************************************/

static AjPFloat2d JukesCantor(AjPFloat2d match, AjPFloat2d gap,
                      float gapwt, ajint mlen, ajint nseqs, AjBool nuc)
{

    ajint i;
    ajint j;

    float m;
    float g;
    float b;
    float D;

    AjPFloat2d matchJC=NULL;


    b = 19./20.;
    if(nuc)
      b = 3./4.;

    matchJC = ajFloat2dNew();
    for(j=0;j<nseqs;++j)
        for(i=0;i<nseqs;++i)
            ajFloat2dPut(&matchJC,j,i,0);


    for(j=0;j<nseqs;j++)         
    { 
      for(i=j;i<nseqs;i++)      
      {
        m = ajFloat2dGet(match,j,i);    /* no. matches */
        g = ajFloat2dGet(gap,j,i);      /* no. gaps    */

        D = 1 - (m/((float)mlen-g+(g*gapwt)));

        ajFloat2dPut(&matchJC,j,i, (-b * log(1. - (D/b))) );
      }
    }


    return matchJC;

}



/* @funcstatic checkRY **************************************************
**
** Check substitutions (not found by checkSubs) involving abiguity codes 
** R (A or G) & Y (C or T) for transitions & transversions.
**
** @param [r] t1 [ajint] Transition score
** @param [r] t2 [ajint] Transversion score
** @param [r] trans [ajint*] Transitions
** @param [r] tranv [ajint*] Transversions
** @return [void]
**
*************************************************************************/
static void checkRY(ajint t1, ajint t2, ajint* trans, ajint* tranv)
{

    if(strchr("R",t1))
    {
      if(strchr("AGR",t2))          /* transitions */
        ++*trans;
      else if(strchr("CTUY",t2))    /* transversion */
        ++*tranv;
    }
    else if (strchr("AG",t1))
    {
      if(strchr("R",t2))            /* transitions */
        ++*trans;
      else if(strchr("Y",t2))       /* transversion */
        ++*tranv;
    }
    else if (strchr("Y",t1))
    {
      if(strchr("CTUY",t2))         /* transitions */
        ++*trans;
      else if(strchr("AGR",t2))     /* transversion */
        ++*tranv;
    }
    else if (strchr("CTU",t1))
    {
      if(strchr("Y",t2))            /* transitions */
        ++*trans;
      else if(strchr("R",t2))       /* transversion */
        ++*tranv;
    }

    return;

}


/* @funcstatic checkSubs ************************************************
**
** Check substitutions for transitions & transversions (ignores 
** ambiguity codes).
**
** @param [r] t1 [ajint] Transition score
** @param [r] t2 [ajint] Transversion score
** @param [r] trans [ajint*] Transitions
** @param [r] tranv [ajint*] Transversions
** @return [void]
**
*************************************************************************/
static void checkSubs(ajint t1, ajint t2, ajint* trans, ajint* tranv)
{

    
    if(strchr("A",t1))
    {
      if(strchr("G",t2))            /* transitions */
        ++*trans;
      else if(strchr("CTU",t2))     /* transversion */
        ++*tranv;
    }
    else if (strchr("G",t1))
    {
      if(strchr("A",t2))            /* transitions */
        ++*trans;
      else if(strchr("CTU",t2))     /* transversion */
        ++*tranv;
    }
    else if(strchr("C",t1))
    {                             
      if(strchr("TU",t2))           /* transitions */
        ++*trans;
      else if(strchr("AG",t2))      /* transversion */
        ++*tranv;
    }
    else if(strchr("T",t1))      
    {
      if(strchr("CU",t2))           /* transitions */
        ++*trans;
      else if(strchr("AG",t2))      /* transversion */
        ++*tranv;
    }
    else if(strchr("U",t1))        
    {
      if(strchr("TC",t2))           /* transitions */  
        ++*trans;
      else if(strchr("AG",t2))      /* transversion */
        ++*tranv;
    }


    return;
}



/* @funcstatic checkambigProt *******************************************
**
** Check amino acid ambiguity codes  to estimate the distance score.
**
** @param [r] t1 [ajint] Transition score
** @param [r] t2 [ajint] Transversion score
** @return [float] Estimated distance score
**
*************************************************************************/
static float checkambigProt(ajint t1, ajint t2)
{
    float n;

   
    n = 0.;

    if( !strchr("X",t1) && t1 == t2 )
      n = 1.0;
    else if( ((strchr("B",t1) && strchr("DN",t2)) ||
              (strchr("B",t2) && strchr("DN",t1))) )
      n = 0.5;
    else if( ((strchr("Z",t1) && strchr("EQ",t2)) ||
              (strchr("Z",t2) && strchr("EQ",t1))) )
      n = 0.5;
    else if( strchr("X",t1) && strchr("X",t2) )
      n = 0.0025;
    else if( strchr("X",t1) || strchr("X",t2) )
      n = 0.05;

    return n;

}

/* @funcstatic checkambigNuc ********************************************
**
** Check ambiguity codes (IUB) to estimate the distance score.
**
** @param [r] m1 [char] First base to compare
** @param [r] m2 [char] Second base to compare
** @return [float] estimated match 
**
*************************************************************************/
static float checkambigNuc(char m1, char m2)
{
    AjPStr b1 = NULL;
    AjPStr b2 = NULL;
    AjPStr b = NULL;
    AjPRegexp rexp = NULL;
    AjBool pmatch = ajFalse;

    float i;
    float n;
    ajint len1;
    ajint len2;


    ajBaseInit();

    len1 = ajStrLen(aj_base_iubS[(int)m1].list)-1;
    len2 = ajStrLen(aj_base_iubS[(int)m2].list)-1;

    b1 = ajStrNew();
    ajStrAssI(&b1,aj_base_iubS[(int)m1].list,len1);

    b2 = ajStrNew();
    ajStrAssI(&b2,aj_base_iubS[(int)m2].list,len2);

    /* for each base code in 1 cf. base code */
    /* for seq 2 to see if there is a match  */
    for(i = 0;i < len1;i++)
    {
      b = ajStrNew();
      ajStrAssSub(&b,b1,i,i);
      rexp = ajRegComp(b);

      if(ajRegExec(rexp,b2))
        pmatch = ajTrue;
      ajRegFree(&rexp);
      ajStrDel(&b);
    }

  
    ajStrDel(&b1);
    ajStrDel(&b2);

    if(pmatch) 
      n = (1./len1)*(1./len2);
    else
      n = 0.;

    return n;

}



/* @funcstatic getSeq ***************************************************
**
** Get the part of the sequences that the distances are calculated from.
** i.e. codon positions 1, 2, 3 or 1 & 2.
**
** @param [r] seqset [AjPSeqset] Sequence set object
** @param [r] nseqs [ajint] Number of sequences
** @param [r] mlen [ajint] Length
** @param [r] incr [ajint] Increment
** @param [r] posn [ajint] Position
** @param [r] len [ajint*] length of longest sequence
** @return [char**] Sequences as an array of C strings
**
*************************************************************************/
static char** getSeq(AjPSeqset seqset, ajint nseqs, ajint mlen, ajint incr,
                     ajint posn, ajint* len)
{
    ajint i;
    ajint j;
    ajint count;

    AjBool onetwo=ajFalse;
    char*  pseqset;
    char** pseq;


    /* positions 1 & 2 used to score distances */
    if(posn == 12)
    {
      onetwo = ajTrue;
      posn   = 0;
      incr   = 3;
    }

    *len = 0;
    for(j=posn;j<mlen;j+=incr)
    {
      *len+=1;
      if(onetwo) *len+=1;
    }

    AJCNEW(pseq,nseqs);
    for(i=0;i<nseqs;i++)                  /* get seq as char* */
    {
      pseqset =  ajSeqsetSeq(seqset,i);
      pseq[i] = ajCharNewL(*len);

      count = 0;
      for(j=posn;j<mlen;j+=incr)
      {
        (void) strncpy(pseq[i]+count,pseqset+j,1);
        count++;
        if(onetwo)
        {
          (void) strncpy(pseq[i]+count,pseqset+j+1,1);
          count++;
        }
      }
      ajDebug("SEQ %d: %s",i,pseq[i]);
    }

    return pseq;

}


/* @funcstatic outputDist ***********************************************
**
** Output the distance matrix
**
** @param [r] outf [AjPFile] Output file
** @param [r] nseqs [ajint] Number of sequences
** @param [r] mlen [ajint] Length
** @param [r] seqset [AjPSeqset] Sequence set object
** @param [r] match [AjPFloat2d] Matches
** @param [r] gap [AjPFloat2d] Gaps
** @param [r] gapwt [float] Gap weight
** @param [r] method [ajint] Method
** @param [r] ambig [AjBool] Ambiguities
** @param [r] nuc [AjBool] Nucleotide
** @param [r] posn [ajint] Position
** @param [r] incr [ajint] Incvrement
** @return [void]
**
*************************************************************************/
static void outputDist(AjPFile outf, ajint nseqs, ajint mlen, AjPSeqset seqset,
                       AjPFloat2d match, AjPFloat2d gap, float gapwt, 
                       ajint method, AjBool ambig, AjBool nuc, ajint posn,
                       ajint incr)
{

    ajint i;
    ajint j;
    float D;

    if(posn == 0)
    {
      if(incr ==3)
       posn = posn+1;
      else
       posn = 123;
    }

    /* print title and parameters */  
    ajFmtPrintF(outf,"Distance Matrix\n---------------\n\n");
    if(method == 0)
       ajFmtPrintF(outf,"Uncorrected for Multiple Substitutions\n");
    else if(method == 1)
       ajFmtPrintF(outf,"Using the Jukes-Cantor correction method\n");
    else if(method == 2)
       ajFmtPrintF(outf,"Using the Kimura correction method\n");
    else if(method == 3)
       ajFmtPrintF(outf,"Using the Tamura correction method\n");
    else if(method == 4)
       ajFmtPrintF(outf,"Using the Tajima-Nei correction method\n");
    else if(method == 5)
       ajFmtPrintF(outf,"Using the Jin-Nei correction method\n");

    if(ambig)
       ajFmtPrintF(outf,"Using ambiguity codes\n");
    if(nuc)
       ajFmtPrintF(outf,"Using base positions %d in the codon\n",posn);
    ajFmtPrintF(outf,"Gap weighting is %f\n\n",gapwt);

    /* print matrix */

    for(j=0;j<nseqs;j++)
       ajFmtPrintF(outf,"\t    %d",j+1);
    ajFmtPrintF(outf,"\n");

    /* Output distance matrix */
    for(j=0;j<nseqs;j++)
    {
      ajFmtPrintF(outf,"\t");
      for(i=0;i<nseqs;i++)
      {
        if (i >= j)
        {
          if(i==j)
            D = 0.;
          else
            D=ajFloat2dGet(match,j,i);

          D=D*100.;
          if (D < 10.)
            ajFmtPrintF(outf,"  %.2f\t",D);
          else if (D < 100.)
            ajFmtPrintF(outf," %.2f\t",D);
          else
            ajFmtPrintF(outf,"%.2f\t",D);
          }
          else
            ajFmtPrintF(outf,"\t");
      }
      ajFmtPrintF(outf,"\t%S %d",ajSeqsetName(seqset,j),j+1);
      ajFmtPrintF(outf,"\n");
    }

    return;

}



