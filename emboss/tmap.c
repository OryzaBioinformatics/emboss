/*

 +-------------------------------+
 | Program       tmap.c          |
 | Edition 46    1994-04-21      |
 | Copyright (c) Bengt Persson   |
 +-------------------------------+------------------------------+
 | This program predicts transmembrane segments in proteins,    |
 | utilising the algorithm described in:                        |
 | "Persson, B. & Argos, P. (1994) Prediction of transmembrane  |
 | segments in proteins utilsing multiple sequence alignments   |
 | J. Mol. Biol. 237, 182-192.                                  |
 +--------------------------------------------------------------+
 | Users of this program are kindly asked to cite the above     |
 | reference in publications (or other types of presentation).  |
 +--------------------------------------------------------------+
 | Questions, suggestions and comments are welcomed             |
 | electronically (see below). Best wishes and good luck!       |
 +--------------------------------------------------------------+
 | E-mail addresses:  Persson@EMBL-Heidelberg.DE                |
 |                    Argos@EMBL-Heidelberg.DE                  |
 +--------------------------------------------------------------+

 Converted for EMBOSS by Ian longden il@sanger.ac.uk (18/6/99)

*/


#include "emboss.h"
#include <limits.h>
#include <float.h>

#define NUMBER 300
#define LENGTH 6000

#define UTGAVA "46"

#define TM_NUMBER 100

#define MAX_PROF 10    /* Max antal profiler */
#define NOLLVARDE FLT_MIN /*-10.000*/
#define MAXHIT 200  /* Max antal pred. TM-segment */

#define N_SPANN 4    
#define M_SPANN 21
#define C_SPANN 4
#define N_FORLANGNING 10
#define C_FORLANGNING 10
#define N_FOSFAT 4
#define C_FOSFAT 4

#define FORLC 8
#define FORLN 8

#define START_E_KORR 2  
#define STOPP_E_KORR 2  

#define M_KORT2_LIMIT 8  
#define M_KORT3_LIMIT 16 
#define M_LANG1_LIMIT 29 

#define ALI_MINIMUM  0.30            /* Proportion of sequences that should be present at a position */
				     /* in order to utilise the data from that position.             */
#define E_SPANN_MIN 20 
#define E_SPANN_MAX 33 
#define E_STST_DIFF 6 

#define GAP '-'



int count,weight;

float profile[MAX_PROF][LENGTH+1];


/* P values and spans */

int pp_antal=2;
int span[2] = { 15, 4 };

float P[2][26] = {


/* Pm values */

 { 1.409446,
 0.000000,
 1.068500,
 0.192356,
 0.174588,
 1.965858,
 1.058479,
 0.587963,
 1.990336,
 0.000000,
 0.180983,
 1.701726,
 1.500664,
 0.433590,
 0.000000,
 0.518571,
 0.344232,
 0.239737,
 0.774442,
 0.828131,
 0.000000,
 1.694256,
 1.314157,
 0.000000,
 0.979187,
 0.000000},

/* Pe values */

 {0.887866,
 0.000000,
 0.842097,
 0.739931,
 0.804004,
 1.102175,
 0.919923,
 1.117477,
 1.103394,
 0.000000,
 1.178047,
 0.997766,
 1.171823,
 1.103455,
 0.000000,
 0.881061,
 0.889218,
 1.519044,
 0.919717,
 0.881105,
 0.000000,
 0.869741,
 1.450220,
 0.000000,
 1.314105,
 0.000000} };


char s[NUMBER][LENGTH];            /* Sequences  */
int relc[LENGTH], reln[LENGTH];

int nr,pos,poss;

int start[TM_NUMBER], stopp[TM_NUMBER];

float norm_skillnad[NUMBER];

int tm_number,tm_segment[TM_NUMBER][2];
int npos[MAXHIT],cpos[MAXHIT];

int pred_mode[TM_NUMBER];
int e_spann_min, e_spann_max;

float mx_limit, me_limit;

int ali_ok[LENGTH];

/*
 * Function prototypes
 */

void profile2(int prof, int antal, int poss, int span);

float length1(int nr, int start, int stopp);

void present3p(int antal, int *npos, int *cpos, int poss, int nr, AjPSeqset seqset, AjPFile outfile);

int peak1(int start, int stopp, float *parameter);

int vec_to_stst(int *vec, int *start, int *stopp, int length);

void weights(char [][LENGTH], int, int, float *);

void refpos2(int, int);

float summa1(int start, int stopp, float *parameter);

void plot1(char *, int, char [][60], int , int , int [][2]);
int pred1(float, float, float, int);
int pred1a(float, float, float, int);

int insert_in_vector(int *start, int *stopp, int max, int starttmp, int stopptmp, int *pred, int predparameter);
int tm_in_vector(int *start, int *stopp, int max, int starttmp, int stopptmp);

void align_rel(int antal, int poss, int span);


void plot2(AjPGraph mult){
  AjPGraphData graph=NULL;
  float max=-10.0,min=10.0;
  int i,j;
  
  for(j=0;j<pp_antal;j++){
    for (i=1; i<=poss; i++) {
      if(profile[j][i] != FLT_MIN){
	if(profile[j][i] > max)
	  max = profile[j][i];
	if(profile[j][i] < min)
	  min = profile[j][i]; 
      }
    }
    graph = ajGraphxyDataNew();
    ajGraphDataxySetTypeC(graph,"Overlay 2D Plot");
    
    ajGraphxyAddDataCalcPtr(graph,poss,0.0,1.0,&profile[j][0]);

    ajGraphDataxySetMaxima(graph,0.,(float)poss,min,max);

    ajGraphxyDataSetXtitleC(graph,"Residue no.");
    ajGraphxyDataSetYtitleC(graph,"");
    ajGraphxyDataSetTitleC(graph,"");
    ajGraphxyDataSetSubtitleC(graph,"");
    
    ajGraphxyAddGraph(mult,graph);
  }
  ajGraphxySetLineType(graph, 2);
  ajGraphxySetGaps(mult,AJTRUE);
  ajGraphxySetOverLap(mult,AJTRUE);

  
  if(min > 0.0)
    min=min*0.9;
  else
    min=min*1.1;

  max=max*1.1;

  ajGraphxySetMaxMin(mult,0.0,(float)poss,min,max);
  ajGraphxyTitleC(mult,"Tmap");

  max=max *0.95;
  
  for (i=1; i<=tm_number; i++) {
/*    ajGraphRectFill((float)tm_segment[i][0],max,(float)tm_segment[i][1],
		    max+((max-min)*0.01));*/
    ajGraphObjAddRect(mult,(float)tm_segment[i][0],max,(float)tm_segment[i][1],
		      max+((max-min)*0.01),BLACK,1);
  }  
  ajGraphxyDisplay(mult,AJFALSE);
  ajGraphCloseWin();
  ajExit();
}

/* Start of main program */

int main(int argc, char *argv[])
{
  AjPSeqset seqset;
  AjPGraph mult;
  AjPFile outfile;
  int i,j;
  /*  int n_spann,  m_spann,  c_spann;*/
  float m_limit, ml_limit, e_limit;

  (void) ajGraphInit("tmap", argc, argv);

  seqset = ajAcdGetSeqset("msf");
  mult = ajAcdGetGraphxy ("graph");
  outfile = ajAcdGetOutfile("outfile");

/* 
   22 March 2000 - GWW
   EMBOSS programs shouldn't write to stdout, unless the user specifies it.
*/  
  ajFmtPrintF( outfile, "Program TMAP, version %s, to predict transmembrane segments from .msf file.\n",UTGAVA);
  ajFmtPrintF( outfile, "The program reads a multiple alignment file of the GCG multiple sequence format\n");
  ajFmtPrintF( outfile, "and predicts membrane-spanning regions according to the algorithm in\n");
  ajFmtPrintF( outfile, "Persson & Argos (1994), J. Mol. Biol. 237, 182-192.\n\n");


  /*  n_spann=N_SPANN;
  m_spann=M_SPANN;
  c_spann=C_SPANN;*/
  
  e_spann_min=E_SPANN_MIN;
  e_spann_max=E_SPANN_MAX;

  m_limit=1.23; 
  ml_limit=1.17; 
  e_limit=1.07;  

  mx_limit=1.18;
  me_limit=1.10;


  nr = ajSeqsetSize (seqset);
  poss = ajSeqsetLen (seqset);

  nr--;

  for(i=0;i<=nr;i++){
    char *temp;

    ajSeqsetToUpper(seqset);
    temp = ajSeqsetSeq (seqset, i);
    for(j=0;j<poss;j++){
      s[i][j+1] =  temp[j];
    }
  }

  /*    stand_nr= -1;*/

  if(!nr)
      norm_skillnad[0]=1;
  else
    weights(s,poss,nr,norm_skillnad);
  
    for (j=0; j<pp_antal; j++) {
      align_rel(nr,poss,span[j]);
      profile2(j,nr, poss, span[j]);
    }

    tm_number=pred1(m_limit,ml_limit,e_limit,nr);

    present3p(tm_number, npos, cpos, poss, nr, seqset, outfile);
   
    for (j=1; j<=tm_number; j++) {
      tm_segment[j][0]=npos[j]+N_SPANN;
      tm_segment[j][1]=cpos[j]-C_SPANN;
    }


  plot2(mult);

  ajFileOutClose(&outfile);

  exit (0);

} /* END OF MAIN */






/*
 * refpos2, utg. 44
 * ----------------
 * Transforms the positional numbers of alignment into those of a reference sequence
 * in each alignment.
 *
 * Global variables:
 * -----------------
 * relc[] and reln[] - vectors containing the "real" positional numbers 
 *
 */

void refpos2(int refnr, int poss)

{
  int i,temp;

  for (i=0; i<LENGTH; i++)
    relc[i]=reln[i]=0;

  temp=1;
    for (i=1; i<=poss; i++)  {
      reln[i]=temp;
      if (s[refnr][i]!=GAP)
	++temp;
    }

  temp=0;
    for (i=1; i<=poss; i++)  {
      if (s[refnr][i]!=GAP)
	++temp;
      relc[i]=temp;
    }


}


/*
 * all_charged
 * -----------
 * Calculates if all residues at an alignment position are identical and one of DEKRQN
 * 
 * Returns 1 if so, otherwise 0
 */

int all_charged(int pos, int nr)
  {
    int i,likhet;
    for (i=1,likhet=1; i<=nr; i++) { 
      if (s[0][pos]!=s[i][pos])
	likhet=0;
    }
    if (likhet==1){
      if ( (s[0][pos]=='D') || (s[0][pos]=='E') || (s[0][pos]=='K') || (s[0][pos]=='R') )
	return 1;
      else
	return 0;
    }
    else
      return 0;
  }





/*
 * length1
 * -------
 *
 * Calculates the real medium length of TM segment
 * Eliminates from the calculations sequences with less than 4 positions
 *
 */

float length1(int nr, int start, int stopp)
{
  int i,j,l,ll;
  int correct_sequence[MAXHIT], nr_correct;

/* First, check for sequences with less than 10 a. a. residues */
  for (i=0; i<MAXHIT; i++)
    correct_sequence[i]=0;
  nr_correct=0;

  for (i=0; i<=nr; i++) {
    for (j=start, l=0; j<=stopp; j++)
      if (s[i][j]!=GAP) l++;
    if (l>=10) {
      correct_sequence[i]=1;
      nr_correct++;
    }
  }

/* Second, check for lengths among the sequences that contain >=10 a. a. residues */
  for (i=0, ll=0; i<=nr; i++) 
    if (correct_sequence[i]==1) {
      for (j=start, l=0; j<=stopp; j++)
	if (s[i][j]!=GAP) l++;
      ll+=l;
    }
  
  if (nr_correct!=0)
    return (float)ll/nr_correct;
  else
    return 0;
  
}


/*
 * present3p, utg. 44
 * ------------------
 * Presents results from predictions
 */

void present3p(int antal, int *npos, int *cpos,  int poss, int nr, AjPSeqset seqset, AjPFile outfile)

{
  int i,j;
 
  ajFmtPrintF( outfile,"RESULTS from program TMAP, edition %s'\n\n",UTGAVA); 

  ajFmtPrintF( outfile,"Numbers give: a) number of transmembrane segment\n");
  ajFmtPrintF( outfile,"              b) start of TM segment (alignment position / residue number)\n");
  ajFmtPrintF( outfile,"              c) end of TM segment (alignment position / residue number)\n");
  ajFmtPrintF( outfile,"              d) length of TM segment within parentheses\n\n");

  ajFmtPrintF( outfile,"PREDICTED TRANSMEMBRANE SEGMENTS FOR ALIGNMENT \n\n");
  for (i=1; i<=antal; i++) 
    ajFmtPrintF( outfile,"  TM %2d: %4d - %4d  (%4.1f)\n",  i,npos[i],cpos[i],length1(nr,npos[i],cpos[i]));
  ajFmtPrintF( outfile,"\n\n");

  for (j=0; j<=nr; j++) {
    refpos2(j, poss);
    ajFmtPrintF( outfile,"PREDICTED TRANSMEMBRANE SEGMENTS FOR PROTEIN %s\n\n",ajStrStr(ajSeqsetName(seqset, j)));
    for (i=1; i<=antal; i++) 
      ajFmtPrintF( outfile,"  TM %2d: %4d - %4d (%d)\n",i,reln[npos[i]],relc[cpos[i]],relc[cpos[i]]-reln[npos[i]]+1);
    ajFmtPrintF( outfile,"\n\n");
  }
 
} /* present3p */



  

/*
 * pred1
 * -----
 * Prediction algorithm
 * Returns number of transmembrane segments
 *
 * Global variables
 * npos[]
 * cpos[]
 */

int pred1(float m_limit, float ml_limit, float e_limit, int nr)
{
  int i,j,k;
  int tm_ant;

  int flag,length;

  int start[MAXHIT], stopp[MAXHIT], hitposs[LENGTH];

  int avstand,mitt,start0;

  int start_e_pos[MAXHIT], stopp_e_pos[MAXHIT];

  float sum;
  
  int count,count2,tempN,tempC;

  int starttmp,stopptmp,temp;


  for (i=0; i<TM_NUMBER; i++)
    pred_mode[i]=0;



  
  /* Find peak values */
  for (i=1; i<=poss; i++) 
    if (profile[0][i]>m_limit) 
      hitposs[i]=1;
    else
      hitposs[i]=0;


  /* Smoothing: Disregard 1 or 2 consequtive positions in vector hitposs[] */
  for (i=3; i<=poss-1; i++) 
    if ( (hitposs[i-2]==1) && (hitposs[i+1]==1) )
      hitposs[i]=hitposs[i-1]=1;
  for (i=2; i<=poss-1; i++) 
    if ( (hitposs[i-1]==1) && (hitposs[i+1]==1) )
      hitposs[i]=1;



  /* Transform hitposs[] to TM vector */
  for (i=0; i<MAXHIT; i++) 
    start[i]=stopp[i]=npos[i]=cpos[i]=0;
  tm_ant=vec_to_stst(hitposs,start,stopp,poss);

  for (i=1; i<=tm_ant; i++)
    pred_mode[i]=pred_mode[i] | 1;


  /* Remove start[] & stopp[] with length <=M_KORT2_LIMIT */
  for (i=1; i<=tm_ant; i++) 
    if (stopp[i]-start[i]<M_KORT2_LIMIT-1) {


      /* Correct for if strictly conserved charges are present, thus reducing the mean value */

      count2=0;

      count=stopp[i]-start[i]+1;
      for (j=stopp[i]+1; j<=poss && profile[0][j]>mx_limit; j++) 
	if (profile[0][j]>mx_limit)
	  count++;
      tempC=j-1;
      for (j=start[i]-1; j>0 && profile[0][j]>mx_limit; j--) 
	if (profile[0][j]>mx_limit)
	  count++;
      tempN=j+1;

      mitt=(tempC-tempN)/2 + tempN;
	  
      count2=0;
      if (count>8) 
	if (mitt>8)
	  for (j=mitt-8; j<=mitt+8 && j<=poss; j++) 
	    if (all_charged(j,nr))
	    count2++;


      /* ... this was not the case ... */

      if (count2==0) {
	for (j=i; j<=tm_ant-1; j++) {
	  start[j]=start[j+1];
	  stopp[j]=stopp[j+1];
	  pred_mode[j]=pred_mode[j+1];
	}
	i--;
	tm_ant--;
      }
      
    }






  /* 3.
     Starting in 'start[]' and 'stopp[]', expand N- and C-terminally
     - each step is taken in the direction of highest profile[0] value
     - as long 'over_limit' is true
     until langd=M_SPAN
     */




  stopp[0]=0;
  start[tm_ant+1]=pos;


  for (i=1; i<=tm_ant; i++) {
    flag=1;
    while (flag) {
      if ( (profile[0][start[i]-1]>ml_limit) || (profile[0][stopp[i]+1]>ml_limit) ){
	if ( profile[0][start[i]-1] > profile[0][stopp[i]+1] ) {
	  if ( (start[i]>1) && (profile[0][start[i]-1]>ml_limit) && (start[i]>stopp[i-1]+FORLC) ){ 
	    start[i]--;
	  }
	  else {
	    if ( (stopp[i]<poss) && (profile[0][stopp[i]+1]>ml_limit) && (stopp[i]<start[i+1]-FORLN) ) {
	      stopp[i]++;
	    }
	    else
	      flag=0;
	  }
	}
	else 
	  if ( (stopp[i]<poss) && (profile[0][stopp[i]+1]>ml_limit) && (stopp[i]<start[i+1]-FORLN) ) {
	    stopp[i]++;
	  }
	  else{
	    if ( (start[i]>1) && (profile[0][start[i]-1]>ml_limit) && (start[i]>stopp[i-1]+FORLC) ) 
	      start[i]--;
	    else 
	      flag=0;
	  }
      }
      else
	flag=0;
    }
  }    



  /* 
   * 4.
   * Elongate with N_FOSFAT and C_FOSFAT, respectively, to correct for that the prediction hitherto
   * has been focused to find only the hydrophobic portion of the transmembrane segment
   */

  for (i=1; i<=tm_ant; i++) {
    if (start[i]>N_FOSFAT) 
      start[i]-=N_FOSFAT;
    if (stopp[i]<poss-C_FOSFAT)
      stopp[i]+=C_FOSFAT;
  }




  /* 
   * 5.1. 
   * Search for Pe values
   */

  for (i=1; i<=tm_ant; i++) {
    mitt=start[i]+(stopp[i]-start[i])/2;
    start_e_pos[i]=peak1(mitt-20,mitt,profile[1]);
    stopp_e_pos[i]=peak1(mitt,mitt+20,profile[1]);


  /* Use the Pe values, if they are good */

  if ( (profile[1][start_e_pos[i]]>=e_limit) && (profile[1][stopp_e_pos[i]]>=e_limit) ) {
    if ( (length1(nr,start_e_pos[i],stopp_e_pos[i])>=E_SPANN_MIN) && 
	 (length1(nr,start_e_pos[i],stopp_e_pos[i])<=E_SPANN_MAX) ) 
      {
	start[i]=start_e_pos[i]-START_E_KORR;
	stopp[i]=stopp_e_pos[i]+STOPP_E_KORR;
      }

  } 
  else {

    if (profile[1][start_e_pos[i]]>=e_limit) 
      if (abs(start_e_pos[i]-start[i])<E_STST_DIFF) 
	start[i]=start_e_pos[i]-START_E_KORR;

    if (profile[1][stopp_e_pos[i]]>=e_limit)
      if (abs(stopp_e_pos[i]-stopp[i])<E_STST_DIFF) 
	stopp[i]=stopp_e_pos[i]+STOPP_E_KORR;
  }



  } /* i, E-varden */



  /* Check for Pe values */

  for (i=20; i<=poss-20; i++) {

    starttmp=peak1(i-19,i,profile[1]);
    stopptmp=peak1(i+1,i+20,profile[1]);
    temp=0;

    if ( (profile[1][starttmp]>1.15) && (profile[1][stopptmp]>1.15) )
      if ( (length1(nr,starttmp,stopptmp)>=E_SPANN_MIN /* >17 */ ) && 
	  (length1(nr,starttmp,stopptmp)<=e_spann_max /*<30*/) )
	for (j=starttmp+2, temp=1; j<=stopptmp-2; j++)
	  if (profile[0][j]<1.08)  /* <1.08 gor att helix 7 i gt74 hittas, >=1.09 gar ej */
	    temp=0;
    
    if (temp==1) 
      if (!(tm_in_vector(start,stopp,tm_ant,starttmp,stopptmp)))
	tm_ant=insert_in_vector(start,stopp,tm_ant,starttmp,stopptmp,pred_mode,16);

  }


  /* 
   * 6.2. 
   * Correction for overlap
   */
  
  for (i=2; i<=tm_ant; i++) 
    if (start[i]<stopp[i-1]) {
      stopp[i-1]=stopp[i];
      for (j=i; j<tm_ant; j++) {
	start[j]=start[j+1];
	stopp[j]=stopp[j+1];
	pred_mode[j]=pred_mode[j+1];
      }
      tm_ant--;
      i--;
    }



  /* 
   * 6.3.
   * Divide segments long enough to contain several tm segments 
   */

  for (i=1; i<=tm_ant; i++) {
    length=length1(nr,start[i],stopp[i]);
    for (j=10; j>=2; j--) {
      if (length>=j*M_SPANN+(j-1)*(N_SPANN+C_SPANN-1) ) { 

	/* For att korrigera for turn precis i membrankanten kan langdkriteriet 
	   justeras litet grand: */
	/* N_SPANN+C_SPANN   missar en helix i k-kanalerna */
	/* N_SPANN+C_SPANN-2 ger en extra helix i am       */
	/* N_SPANN+C_SPANN-1 fungerar i bada fallen ...    */

	/* dela upp tm_segmentet symmetriskt, ty enklast
	   kan finjusteras senare .... */
	
	/* gor plats for de nya segmenten */
	tm_ant+=j-1;
	for (k=tm_ant; k>=i+1; k--) {
	  start[k]=start[k-(j-1)];
	  stopp[k]=stopp[k-(j-1)];
	  pred_mode[k]=pred_mode[k-(j-1)];
	}

	avstand=length/j;
	start0=start[i];



	for (k=1; k<=j; k++) {
	  mitt=avstand/2+avstand*(k-1);
	  start[i+k-1]=start0+mitt-N_FORLANGNING;
	  stopp[i+k-1]=start0+mitt+C_FORLANGNING;
	  pred_mode[i+k-1]=pred_mode[i+k-1] | 8;
	}
	
	j=0;
      } /* if */
    } /* j */
  } /* i */




  /* 
   * 7.
   * Remove too short segments
   */

  for (i=1; i<=tm_ant; i++) 
    if ( (length1(nr,start[i],stopp[i]) < M_KORT3_LIMIT ) ) {
      for (j=i; j<=tm_ant-1; j++) {
	start[j]=start[j+1];
	stopp[j]=stopp[j+1];
	pred_mode[j]=pred_mode[j+1];
      }
      i--;
      tm_ant--;
    }



  /* 
   * 8.
   * Shorten segments, longer than M_SPANN aa 
   */
  
  for (i=1; i<=tm_ant; i++) 
    if ( (length1(nr,start[i],stopp[i])>M_LANG1_LIMIT) ) {
      /* Forst kolla om segmentet tangerar forutvarande */
      if (i>1)
	if (start[i]<stopp[i-1])
	  start[i]=stopp[i-1];

      sum=0;
      start0=start[i];
      for (j=start[i]; j<=stopp[i]-M_LANG1_LIMIT+1; j++) 
	if ( summa1(j, j+M_LANG1_LIMIT-1, profile[0]) > sum) {
	  sum = summa1(j, j+M_LANG1_LIMIT-1, profile[0]);
	  start0=j;
	}
      start[i]=start0;
      stopp[i]=start0+M_LANG1_LIMIT-1;
      pred_mode[i]=pred_mode[i] | 128;
    }

  for (i=1; i<=tm_ant; i++) {
    npos[i]=start[i];
    cpos[i]=stopp[i];
  }


  return tm_ant;
}



/*
 * peak1
 * -----
 * Finds peak value in the vector 'parameter'
 * and returns position of peak value
 */

int peak1(int start, int stopp, float *parameter)
{
  int i,maxpos=0;
  float maximum;

  maximum=0;
  for (i=start; i<=stopp; i++)
    if (parameter[i]>maximum) {
      maxpos=i;
      maximum=parameter[i];
    }
  return maxpos;
}





#define OVERLAPP 2
#define MAX_TM_LANGD 30        
#define MIN_TM_LANGD 25



/*
 *  summa1
 *  ------
 *  Sums the values for 'parameter' in span 'start' to 'stopp'
 */

float summa1(int start, int stopp, float *parameter)
{
  float summa=0;
  int i;

  for (i=start; i<=stopp; i++)
    summa+=parameter[i];

  return summa;
}



/*
 * tm_in_vector
 * ------------
 * Checks if segment already in TM vector
 */

int tm_in_vector(int *start, int *stopp, int max, int starttmp, int stopptmp)
{
  int i,temp;

  temp=0;
  for (i=1; i<=max; i++)
    if ( (abs(start[i]-starttmp)<7) && (abs(stopp[i]-stopptmp)<7) )
      temp=1;

  return temp;
}


/* insert_in_vector
 * ----------------
 * Insert segment in TM vector
 */

int insert_in_vector(int *start, int *stopp, int max, int starttmp, int stopptmp, int *pred, int predparameter)
{
  int i,j;

  for (i=1; i<=max-1; i++)
    if (starttmp>start[i])
      if (starttmp<start[i+1]) {
	for (j=max; j>=i+1; j--) {
	  start[j+1]=start[j];
	  stopp[j+1]=stopp[j];
	  pred[j+1]=pred[j];
	}
	start[i+1]=starttmp;
	stopp[i+1]=stopptmp;
	pred[i+1]=predparameter;
	return max+1;
      }

  /* starttmp<=start[i] */

  for (j=max; j>=1; j--) {
    start[j+1]=start[j];
    stopp[j+1]=stopp[j];
    pred[j+1]=pred[j];
  }
  start[1]=starttmp;
  stopp[1]=stopptmp;
  pred[1]=predparameter;
  return max+1;

}
  


/*
 * vec_to_stst
 * -----------
 * Transfers information in vector vec[] to start[] and stopp[]
 * Returns number of elements in start[] and stopp[]
 */

int vec_to_stst(int *vec, int *start, int *stopp, int length)
{


  int flagga,i,index;

  flagga=0;
  index=0;

  for (i=1; i<=length; i++) {
    if ( (vec[i]==1) && (flagga==0) ) {
      flagga=1;
      start[++index]=i;
    }
    if ( (vec[i]==0) && (flagga==1) ) {
      flagga=0;
      stopp[index]=i-1;
    }
  }
  if (flagga==1)
    stopp[index]=length;

  return index;
}








/*
 *  weights
 *  -------
 *  Calculates number of differences between sequence 'testnr' and all 
 *  other sequences (Ref. Vingron & Argos, CABIOS 5 (1989) 115-121).  
 *
 *  s[][]   - sekvensmatris
 *  poss    - antal positioner i sekvenserna
 *  nr      - max nr av sekvenserna
 *  norm_sk - vektor for vikterna
 *
 */


void weights(char s[][LENGTH], int poss, int nr, float *norm_sk)
{
  int i,j,testnr;
  int skillnad[NUMBER+1];
  float summa;
  
  for (testnr=0; testnr<=nr; testnr++) 
    for (i=0; i<=nr; i++) 
      if (i!=testnr) 
	for (j=1, skillnad[testnr]=0; j<=poss; j++) 
	  if (s[testnr][j]!=s[i][j]) 
	    skillnad[testnr]++;

  /* 
   * Normalize 'skillnad[]' 
   */
  for (i=0, j=LENGTH*NUMBER; i<=nr; i++)
    if (skillnad[i]<j) j=skillnad[i];
  for (i=0; i<=nr; i++) norm_sk[i]=(float)skillnad[i]/j;

  /*
   * Satt  Summa av skillnad[] till 1
   */
  
  for (i=0, summa=0; i<=nr; i++)
    summa+=norm_sk[i];
  for (i=0; i<=nr; i++)
  norm_sk[i]=norm_sk[i]/summa;

} /* weights */





/*
 * profile2
 * --------
 * Calculates mean values of 'P[]' over 'span' a.a.
 * and stores the result in 'profile[]'.
 *
 * Calculates on all sequences of the alignment
 * Ignores gaps
 *
 * prof  - number of profile
 * antal - number of sequences in alignment
 * poss  - number of positions in sequence
 * span  - length of span for profile to be calculated upon
 * s[][] - sequence data
 * P[]   - values for profile 
 * profile[] - result vector, result stored in pos closest
 *             to centre of segment
 */

void profile2(int prof, int antal, int poss, int span)
{
  int bin=0,count,i,j,nr;	     
  int flagga[LENGTH+1];
  float prof_temp;
  float summa_vikt[LENGTH+1];

 
  for (i=1; i<=poss; i++) {
    profile[prof][i]= 0.0;
    flagga[i]=0;
    summa_vikt[i]=0;
  }

  for (nr=0; nr<=antal; nr++)
    for (i=1; i<=poss-span+1; i++) {
      if ( (s[nr][i]>='A') && (s[nr][i]<='Z') ) {
	for (j=0, count=0, prof_temp=0; count<span && i+j<=poss; j++) 
	  if ( (s[nr][i+j]>='A') && (s[nr][i+j]<='Z') ) {
	    prof_temp+=P[prof][s[nr][i+j]-'A'] * norm_skillnad[nr];
	    if (count==span/2) 
	      bin=i+j;
	    count++;
	  }
	if (count==span) {
	  flagga[bin]=1;
	  profile[prof][bin]+=prof_temp/count;
	  summa_vikt[bin]+=norm_skillnad[nr];
	}
      }
    }

  for (i=1; i<=poss; i++) 
    if ( (flagga[i]==0) || (ali_ok[i]==0) )
      profile[prof][i]= NOLLVARDE;
    else
      profile[prof][i]=profile[prof][i]/summa_vikt[i];

}





/*
 * align_rel
 * ---------
 */

void align_rel(int antal, int poss, int span)

{
  int nr,ok,pos;

  for (pos=1; pos<=poss-span+1; pos++) {
    for (nr=0,ok=0; nr<=antal; nr++)
      if (s[nr][pos]!=GAP)
	ok++;
    if ( (float)ok/(float)nr > ALI_MINIMUM ) 
      ali_ok[pos]=1;
    else
      ali_ok[pos]=0;
  }
}



