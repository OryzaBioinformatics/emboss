#include "emboss.h"
#include <math.h>
#include <stdlib.h>

/* Bill Pearson's include files */

char amino[]="GAVLISTDENQKHRFYWCMP";

ajint helix[20][17]=
{
  { -5,-10,-15,-20,-30,-40,-50,-60,-86,-60,-50,-40,-30,-20,-15,-10, -5},
  { 5, 10, 15, 20, 30, 40, 50, 60, 65, 60, 50, 40, 30, 20, 15, 10,  5},
  { 0,  0,  0,  0,  0,  0,  0,  5, 10, 14, 10,  5,  0,  0,  0,  0,  0},
  { 0,  5, 10, 15, 20, 25, 28, 30, 32, 30, 28, 25, 20, 15, 10,  5,  0},
  { 5, 10, 15, 20, 25, 20, 15, 10,  6,  0,-10,-15,-20,-25,-20,-10, -5},
  { 0, -5,-10,-15,-20,-25,-30,-35,-39,-35,-30,-25,-20,-15,-10, -5,  0},
  { 0,  0,  0, -5,-10,-15,-20,-25,-26,-25,-20,-15,-10, -5,  0,  0,  0},
  { 0, -5,-10,-15,-20,-15,-10,  0,  5, 10, 15, 20, 20, 20, 15, 10,  5},
  { 0,  0,  0,  0, 10, 20, 60, 70, 78, 78, 78, 78, 78, 70, 60, 40, 20},
  { 0,  0,  0,  0,-10,-20,-30,-40,-51,-40,-30,-20,-10,  0,  0,  0,  0},
  { 0,  0,  0,  0,  5, 10, 20, 20, 10,-10,-20,-20,-10, -5,  0,  0,  0},
  { 20, 40, 50, 55, 60, 60, 50, 30, 23, 10,  5,  0,  0,  0,  0,  0,  0},
  { 10, 20, 30, 40, 50, 50, 50, 30, 12,-20,-10,  0,  0,  0,  0,  0,  0},
  {  0,  0,  0,  0,  0,  0,  0,  0, -9,-15,-20,-30,-40,-50,-50,-30,-10},
  {  0,  0,  0,  0,  0,  5, 10, 15, 16, 15, 10,  5,  0,  0,  0,  0,  0},
  { -5,-10,-15,-20,-25,-30,-35,-40,-45,-40,-35,-30,-25,-20,-15,-10, -5},
  { -10,-20,-40,-50,-50,-10,  0, 10, 12, 10,  0,-10,-50,-50,-40,-20,-10},
  {  0,  0,  0,  0,  0,  0, -5,-10,-13,-10, -5,  0,  0,  0,  0,  0,  0},
  { 10, 20, 25, 30, 35, 40, 45, 50, 53, 50, 45, 40, 35, 30, 25, 20, 10},
  { -10,-20,-40,-60,-80,-100,-120,-140,-77,-60,-30,-20,-10, 0, 0, 0,  0}
};

ajint extend [20][17]=
{
  { 10, 20, 30, 40, 40, 20,  0,-20,-42,-20,  0, 20, 40, 40, 30, 20,-10},
  {  0,  0,  0,  0, -5,-10,-15,-20,-23,-20,-15,-10, -5,  0,  0,  0,  0},
  {  0,  0,-10,-20,  0, 20, 40, 60, 68, 60, 40, 20,  0,-20,-10,  0,  0},
  {  0,  0,  0,  0,  0,  5, 10, 20, 23, 20, 10,  5,  0,  0,  0,  0,  0},
  {  0,-10,-20,-10,  0, 20, 40, 60, 67, 60, 40, 20,  0,-10,-20,-10,  0},
  {  0, 10, 20, 10,  0, -5,-10,-15,-17,-15,-10, -5,  0, 10, 20, 10,  0},
  {  5, 10, 15, 20, 15, 15, 10, 10, 13, 10, 10, 15, 15, 20, 15, 10,  5},
  {  0,  5, 10, 15, 20,  0,-20,-30,-44,-30,-20,  0,  0,  0,  0,  0,  0},
  {-10,-15,-20,-25,-30,-35,-40,-45,-50,-55,-60,-60,-50,-40,-30,-20,-10},
  { 10, 30, 50, 30, 20,  0,-15,-30,-41,-30,-15,  0, 20, 50, 30, 50, 10},
  {  0,  0,  0,  0,  0, -5,-10,  0, 12, 20, 30, 40, 50, 50, 40, 30, 15},
  { -5,-10,-15,-20,-30,-40,-50,-40,-33,-20,-10,  0, 10, 10,  0,  0,  0},
  {-10,-20,-40,-20,-10,  0,-10,-20,-25,-35,-30,-25,-20,-15,-10, -5,  0},
  {  0,  0,  0,  0,  0,  0,  0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0},
  {  0,  0,  0,  0,  0,  5, 10, 20, 26, 10,-10,-30,-60,-65,-60,-40,-20},
  {  0,  5, 10, 15, 20, 25, 30, 35, 40, 35, 30, 25, 20, 15, 10,  5,  0},
  {  0,  0,  0,  0,  0,-10,-10,-10,-10,-10,-10,-15,-20,-25,-30,-20,-10},
  {  0,  0,  0,  0,  0, 10, 20, 30, 44, 30, 20, 10,  0,  0,  0,  0,  0},
  {-10,-20,-30,-40,-40,-30,  0, 10, 23, 10,  0,-30,-40,-40,-30,-20,-10},
  { 10, 20, 30, 30, 20, 10,  0,-10,-18,-20,-10, 10, 30, 40, 30, 20, 10}
};

ajint turns[20][17]=
{
 {  0,  0,  0,  0, 10, 30, 55, 55, 57, 40,  0,  0,  0,  0,  0,  0,  0},
 {  0,  0,  0,-10,-20,-30,-40,-50,-50,-40,-30,-20,-10,  0,  0,  0,  0},
 {  0,  0,  0,  0,-10,-20,-30,-40,-60,-40,-30,-20,-10,  0,  0,  0,  0},
 {  0,  0,  0,-10,-20,-30,-40,-50,-56,-20,-10,  0,  0,  0,  0,  0,  0},
 {  0,  0,  0,  0,  0,-10,-20,-30,-46,-40,-10,  0,  0, 20, 30, 20, 10},
 {  0,-10,-20,-20, 10, 15, 20, 25, 26, 25, 20, 15, 10,  0,  0,  0,  0},
 {  0, 10, 20, 20, 20, 15, 18,  5,  3,  5, 10, 15, 20, 20, 20, 10,  0},
 {  0,  0,  0,  0,  0,  0,  5, 10, 31, 10,  5,  0,  0,  0,  0,  0,  0},
 {  0, -5,-10,-15,-20,-30,-40,-45,-47,-20,  0, 10,  5,  0,  0,  0,  0},
 {  0,  0,  0, 10, 20, 30, 35, 40, 42, 40, 35, 30, 20, 10,  5,  0,  0},
 { 10, 20, 30, 25, 20, 15, 10,  5,  4, 20, 30, 40, 50, 60, 50, 40, 20},
 {-10,-20,-30,-40,-25,-10,  0, 10, 10, 10,  0,-20,-30,-20,-10, -5,  0},
 {  0,  0,  0,  0,  0,  0,  0,  0, -3,  0, 10, 20, 30, 20, 10,  0,  0},
 {  0,  0,  0,  0,  0,  0,  0, 10, 21, 30, 40, 30, 20, 10,  0,  0,  0},
 {  0,  0,  0,  0,  0, -5,-10,-15,-18,-15,  0, 15, 30, 25, 20, 10,  0},
 {  0,  0,  0,  5, 15, 15, 20, 25, 29, 25, 20, 15, 15,  5,  0,  0,  0},
 {  0,  0,  0, 10, 20, 30, 40, 80, 36,-30, 30, 40, 50, 60, 70, 40, 20},
 { 20, 40, 50, 60, 60, 55, 50, 45, 44, 40, 35, 30, 25, 20, 15, 10,  5},
 { -5,-15,-20,-25,-30,-35,-40,-45,-48,-45,-40,-35,-30,-25,-20,-15, -5},
 { 10, 20, 30, 40, 50, 70, 10,-90, 36, 90, 10,  0,  0,  0,  0,  0,  0}
};

ajint coil[20][17]=
{
  {  0,  0,  0,  0, 10, 30, 40, 45, 49, 45, 40, 30, 10,  0,  0,  0,  0},
  {  0,  0,  0,  0, -5,-10,-20,-25,-25,-25,-20,-15,-10, -5,  0,  0,  0},
  {  0,  0,  0,  0,-10,-20,-25,-30,-35,-30,-25,-20,-10,  0,  0,  0,  0},
  {  0,  0,  0,-10,-20,-30,-40,-30,-20,-20,-10,  0,  0,  0,  0,  0,  0},
  {  0,  0,  0,  0,  0,-10,-20,-30,-33,-30,-10,  0, 10, 20, 30, 20,  0},
  {  0,-10,-20,-20, 10, 15, 20, 25, 50, 25, 20, 15, 10,  0,  0,  0,  0},
  {  0, 10, 20, 30, 20, 15, 10, 15, 17, 15, 10, 15, 20, 30, 20, 10,  0},
  {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
  {  0,  0, 10, 20, 40, 20,  0,-10,-44,-40,-20,-10,  0,  0,  0,  0,  0},
  {  0,  0,  0, 10, 20, 30, 35, 40, 46, 40, 35, 30, 20, 10,  0,  0,  0},
  { 10, 20, 30, 25, 20, 15, 10,  0, -5, 20, 30, 40, 50, 60, 50, 40, 20},
  {-10,-20,-30,-40,-25,-20,-10, -8, -8,  0,  0,-20,-30,-20,-10, -5,  0},
  {  0,  0,  0,  0,  0,  0,  0, 10, 16, 15, 10, 10, 10, 10,  5,  0,  0},
  {  0,  0,  0,  0,  0,  0,  0,  0,-12,  0, 20, 30, 20, 10,  0,  0,  0},
  {  0,  0,  0,  0,  0, -5,-10,-20,-41,-20,  0, 15, 30, 25, 20, 10,  0},
  {  0,  0,  0,  0,  0,  0,  0,  0, -6,  0,  0,  0,  0,  0,  0,  0,  0},
  {  0,  0,  0, 10, 20, 30, 40, 20, 12, 20, 30, 40, 50, 60, 70, 40, 20},
  {  0,  0,  0,  0,  0,  0,-10,-30,-47,-30,-10,  0,  0,  0,  0,  0,  0},
  {  0, -5,-10,-15,-20,-25,-30,-40,-41,-40,-30,-25,-20,-15,-10, -5,  0},
  {  0,  0, 10, 20, 30, 40, 50, 55, 58, 50, 10,  0,  0,  0,  0,  0,  0}
};

/*	ascii.gbl	ascii translation to amino acids */
/*	modified 10-Mar-1987 for B, Z	*/

#define NA 124
#define EL 125
#define ES 126

/*      0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15	*/
ajint aascii[]={
	EL,NA,NA,NA,NA,NA,NA,NA,NA,NA,EL,NA,NA,EL,NA,NA,
	NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,
	NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,
	NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,
	ES, 0,20, 4, 3, 6,13, 7, 8, 9,NA,11,10,12, 2,NA,
	14, 5, 1,15,16,NA,19,17,22,18,21,NA,NA,NA,NA,NA,
	NA, 0,20, 4, 3, 6,13, 7, 8, 9,NA,11,10,12, 2,NA,
	14, 5, 1,15,16,NA,19,17,22,18,21,NA,NA,NA,NA,NA};

ajint *sascii;
#define AAMASK 127

ajint nascii[]={
/*	 0  1  2  3  5  6  7  8  9 10 11 12 13 14 15 15
	 @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
	 P  Q  R  S  T  U  V  W  X  Y  Z		*/
	EL,NA,NA,NA,NA,NA,NA,NA,NA,NA,EL,NA,NA,EL,NA,NA,
	NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,
	NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,16,NA,NA,
	NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,NA,
	ES, 0,14, 1,11,NA,NA, 2,12,NA,NA,10,NA, 7,15,NA,
	 5, 6, 5, 9, 3, 4,13, 8,15, 6,NA,NA,NA,NA,NA,NA,
	NA, 0,14, 1,11,NA,NA, 2,12,NA,NA,10,NA, 7,15,NA,
	 5, 6, 5, 9, 3, 4,13, 8,15, 6,NA,NA,NA,NA,NA,NA};


/*	20-June-1986	universal pam file */

ajint gdelval= -12;
ajint del_set=0;

ajint ggapval= -2;
ajint gap_set=0;

ajint gshift = -30;
ajint shift_set=0;

#define EOSEQ 31
#define MAXSQ 32

char qsqnam[]={"aa"};
char sqnam[]={"aa"};
char sqtype[]={"protein"};

char *sq;
char aa[MAXSQ] = {"ARNDCQEGHILKMFPSTWYVBZX"};

ajint naa = 23;
ajint nsq;

ajint haa[MAXSQ] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,2,6,0};
ajint *hsq;

ajint apam250[450] = {
  2,
 -2, 6,
  0, 0, 2,
  0,-1, 2, 4,
 -2,-4,-4,-5,12,
  0, 1, 1, 2,-5, 4,
  0,-1, 1, 3,-5, 2, 4,
  1,-3, 0, 1,-3,-1, 0, 5,
 -1, 2, 2, 1,-3, 3, 1,-2, 6,
 -1,-2,-2,-2,-2,-2,-2,-3,-2, 5,
 -2,-3,-3,-4,-6,-2,-3,-4,-2, 2, 6,
 -1, 3, 1, 0,-5, 1, 0,-2, 0,-2,-3, 5,
 -1, 0,-2,-3,-5,-1,-2,-3,-2, 2, 4, 0, 6,
 -4,-4,-4,-6,-4,-5,-5,-5,-2, 1, 2,-5, 0, 9,
  1, 0,-1,-1,-3, 0,-1,-1, 0,-2,-3,-1,-2,-5, 6,
  1, 0, 1, 0, 0,-1, 0, 1,-1,-1,-3, 0,-2,-3, 1, 2,
  1,-1, 0, 0,-2,-1, 0, 0,-1, 0,-2, 0,-1,-3, 0, 1, 3,
 -6, 2,-4,-7,-8,-5,-7,-7,-3,-5,-2,-3,-4, 0,-6,-2,-5,17,
 -3,-4,-2,-4, 0,-4,-4,-5, 0,-1,-1,-4,-2, 7,-5,-3,-3, 0,10,
  0,-2,-2,-2,-2,-2,-2,-1,-2, 4, 2,-2, 2,-1,-1,-1, 0,-6,-2, 4,
  0,-1, 2, 3,-4, 1, 2, 0, 1,-2,-3, 1,-2,-5,-1, 0, 0,-5,-3,-2, 2,
  0, 0, 1, 3,-5, 3, 3,-1, 2,-2,-3, 0,-2,-5, 0, 0,-1,-6,-4,-2, 2, 3,
 -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

ajint abl50[] = {
  5,
 -2, 7,
 -1,-1, 7,
 -2,-2, 2, 8,
 -1,-4,-2,-4,13,
 -1, 1, 0, 0,-3, 7,
 -1, 0, 0, 2,-3, 2, 6,
  0,-3, 0,-1,-3,-2,-3, 8,
 -2, 0, 1,-1,-3, 1, 0,-2,10,
 -1,-4,-3,-4,-2,-3,-4,-4,-4, 5,
 -2,-3,-4,-4,-2,-2,-3,-4,-3, 2, 5,
 -1, 3, 0,-1,-3, 2, 1,-2, 0,-3,-3, 6,
 -1,-2,-2,-4,-2, 0,-2,-3,-1, 2, 3,-2, 7,
 -3,-3,-4,-5,-2,-4,-3,-4,-1, 0, 1,-4, 0, 8,
 -1,-3,-2,-1,-4,-1,-1,-2,-2,-3,-4,-1,-3,-4,10,
  1,-1, 1, 0,-1, 0,-1, 0,-1,-3,-3, 0,-2,-3,-1, 5,
  0,-1, 0,-1,-1,-1,-1,-2,-2,-1,-1,-1,-1,-2,-1, 2, 5,
 -3,-3,-4,-5,-5,-1,-3,-3,-3,-3,-2,-3,-1, 1,-4,-4,-3,15,
 -2,-1,-2,-3,-3,-1,-2,-3, 2,-1,-1,-2, 0, 4,-3,-2,-2, 2, 8,
  0,-3,-3,-4,-1,-3,-3,-4,-4, 4, 1,-3, 1,-1,-3,-2, 0,-3,-1, 5,
 -2,-1, 4, 5,-3, 0, 1,-1, 0,-4,-4, 0,-3,-4,-2, 0, 0,-5,-3,-4, 5,
 -1, 0, 0, 1,-3, 4, 5,-2, 0,-3,-3, 1,-1,-4,-1, 0,-1,-2,-2,-3, 2, 5,
 -1,-1,-1,-1,-2,-1,-1,-2,-1,-1,-1,-1,-1,-2,-2,-1, 0,-3,-1,-1,-1,-1,-1};

ajint abl62[] = {
  4,
 -1, 5,
 -2, 0, 6,
 -2,-2, 1, 6,
  0,-3,-3,-3, 9,
 -1, 1, 0, 0,-3, 5,
 -1, 0, 0, 2,-4, 2, 5,
  0,-2, 0,-1,-3,-2,-2, 6,
 -2, 0, 1,-1,-3, 0, 0,-2, 8,
 -1,-3,-3,-3,-1,-3,-3,-4,-3, 4,
 -1,-2,-3,-4,-1,-2,-3,-4,-3, 2, 4,
 -1, 2, 0,-1,-3, 1, 1,-2,-1,-3,-2, 5,
 -1,-1,-2,-3,-1, 0,-2,-3,-2, 1, 2,-1, 5,
 -2,-3,-3,-3,-2,-3,-3,-3,-1, 0, 0,-3, 0, 6,
 -1,-2,-2,-1,-3,-1,-1,-2,-2,-3,-3,-1,-2,-4, 7,
  1,-1, 1, 0,-1, 0, 0, 0,-1,-2,-2, 0,-1,-2,-1, 4,
  0,-1, 0,-1,-1,-1,-1,-2,-2,-1,-1,-1,-1,-2,-1, 1, 5,
 -3,-3,-4,-4,-2,-2,-3,-2,-2,-3,-2,-3,-1, 1,-4,-3,-2,11,
 -2,-2,-2,-3,-2,-1,-2,-3, 2,-1,-1,-2,-1, 3,-3,-2,-2, 2, 7,
  0,-3,-3,-3,-1,-2,-2,-3,-3, 3, 1,-2, 1,-1,-2,-2, 0,-3,-1, 4,
 -2,-1, 3, 4,-3, 0, 1,-1, 0,-3,-4, 0,-3,-3,-2, 0,-1,-4,-3,-3, 4,
 -1, 0, 0, 1,-3, 3, 4,-2, 0,-3,-3, 1,-1,-3,-1, 0,-1,-3,-2,-2, 1, 4,
  0,-1,-1,-1,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2, 0, 0,-2,-1,-1,-1,-1,-1};

/*	DNA alphabet

	A, C, G, T U
	R, Y
	M (A or C)	6
	W (A or T)	7
	S (C or G)	8
	K (G or T)	9
	D (not C)	10
	H (not G)	11
	V (not T)	12
	B (not A)	13
	N X 		14
*/

char nt[MAXSQ]={"ACGTURYMWSKDHVBNX"};

ajint nnt = 17;

ajint hnt[MAXSQ] = {0,1,2,3,3,0,1,0,0,1,2,0,0,0,1,0,0};

ajint npam[450] = {
/*       A  C  G  T  U  R  Y  M  W  S  K  D  H  V  B  N  X  */
	 5,						/* A */
	-4, 5,						/* C */
	-4,-4, 5,					/* G */
	-4,-4,-4, 5,					/* T */
	-4,-4,-4, 5, 5,					/* U */
	 2,-1, 2,-1,-1, 2,				/* R (A G)*/
	-1, 2,-1, 2, 2,-2, 2,				/* Y (C T)*/
	 2, 2,-1,-1,-1,-1,-1, 2,			/* M (A C)*/
	 2,-1,-1, 2, 2, 1, 1, 1, 2,			/* W (A T)*/
	-1, 2, 2,-1,-1, 1, 1, 1,-1, 2,			/* S (C G)*/
	-1,-1, 2, 2, 2, 1, 1,-1, 1, 1, 2,		/* K (G T)*/
	 1,-2, 1, 1, 1, 1,-1,-1, 1,-1, 1, 1,		/* D (!C) */
	 1, 1,-2, 1, 1,-1, 1, 1, 1,-1,-1,-1, 1,		/* H (!G) */
	 1, 1, 1,-2,-1, 1,-1, 1,-1, 1,-1,-1,-1, 1,	/* V (!T) */
	-2, 1, 1, 1, 1,-1, 1,-1,-1, 1, 1,-1,-1,-1, 1,	/* B (!A) */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, /* N */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}; /* X */
/*       A  C  G  T  U  R  Y  M  W  S  K  D  H  V  B  N  */

ajint *pam;
ajint pam2[MAXSQ][MAXSQ];
ajint pamh1[MAXSQ];		/* used for kfact replacement */


static void garnier_report(AjPReport report, AjPFeattable TabRpt,
			   AjPSeq seqobj,
			   ajint from, ajint to, char *seq,
			   ajint begin, ajint Idc);
static void garnier_do(AjPFile outf, ajint s, ajint len, char *seq, char *name,
		       ajint begin, ajint Idc);
static void garnier_makemap (char *input, ajint *map, ajint n);


/* @prog garnier **************************************************************
**
** Predicts protein secondary structure
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   outf=NULL;
    AjPReport report=NULL;
    AjPFeattable TabRpt=NULL;
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    
    ajint begin;
    ajint end;
    ajint len;
    ajint Idc;

    embInit("garnier",argc,argv);
    
    seqall    = ajAcdGetSeqall("sequencea");
    Idc       = ajAcdGetInt("idc");
    report    = ajAcdGetReport("outfile");

    
    substr = ajStrNew();


    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);

	TabRpt = ajFeattableNewSeq(seq);

	strand = ajSeqStrCopy(seq);
	ajStrToUpper(&strand);

	ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);

	len=ajStrLen(substr);

	garnier_report(report, TabRpt, seq, 0,len, ajStrStr(substr),begin,Idc);
	if (outf)
	  garnier_do(outf,0,len,ajStrStr(substr),ajSeqName(seq),begin,Idc);

	ajStrDel(&strand);

	ajReportWrite (report, TabRpt, seq);
	ajFeattableDel(&TabRpt);
    }
    
    
    ajSeqDel(&seq);
    ajStrDel(&substr);
    if (outf)
      ajFileClose(&outf);
    (void) ajReportClose(report);

    ajExit();
    return 0;
}

/* @funcstatic garnier_do ****************************************************
**
** Undocumented.
**
** @param [?] outf [AjPFile] Undocumented
** @param [?] from [ajint] Undocumented
** @param [?] to [ajint] Undocumented
** @param [?] seq [char*] Undocumented
** @param [?] name [char*] Undocumented
** @param [?] begin [ajint] Undocumented
** @param [?] Idc [ajint] Undocumented
** @@
******************************************************************************/


static void garnier_do(AjPFile outf, ajint from, ajint to, char *seq,
		       char *name, ajint begin, ajint Idc)
{
    char *refstr="\n Please cite:\n Garnier, Osguthorpe and Robson (1978) J. Mol. Biol. 120:97-120\n";
    
    ajint i;
    ajint end;
    ajint amap[20];
    ajint nna=20;
    ajint parr[4];
    char carr[]="HETC";
    char *type;
    ajint iarr[4];
    ajint dharr[]=
    {
	0,158,-75,-100
    }
    ;
    ajint dsarr[]=
    {
	0,50,-88,-88
    }
    ;
    ajint n0;
    ajint j, k, l0, l1, idc, dcs, dch, lastk;
    float fn0;
    
    idc=Idc;
    end=to-from+1;
    n0=end;

/* GWW - 28 Sept 2001 - changed 'type' to dynamic allocation */
    type = AJALLOC0(n0*sizeof(char));

    sascii = aascii;
    
    if (idc<=0) dcs=dch=0;
    else if (idc <4) 
    {
	dch=dharr[idc];
	dcs=0;
    }
    else if (idc<=6)
    {
	dch=0;
	dcs=dsarr[idc-3];
    }
    else dcs=dch=0;
    
    garnier_makemap(amino,amap,nna);  

    
/* copy from garnier.c original */
    --n0;  /* AJB: Added as n0 was one greater than the sequence length */
    
  for (i=0; i<n0; i++)  {
/*      ajDebug("seq[%d] '%c' %x", i, seq[i], seq[i]);
*/      
    seq[i] = amap[aascii[(ajint)seq[i]]];
/*
    ajDebug(" -> %x\n", seq[i]);
*/
  }	
    
  for(k=0;k<4;++k) iarr[k]=0;

  lastk = 0;
  for (i=0; i<n0; i++) {
    parr[0]=helix[(ajint)seq[i]][8];
    parr[1]=extend[(ajint)seq[i]][8];
    parr[2]=turns[(ajint)seq[i]][8];
    parr[3]=coil[(ajint)seq[i]][8];

    for (j=1; j<9; j++) {
      if ((i-j)>=0) {
        parr[0] += helix[(ajint)seq[i-j]][8+j];
        parr[1] += extend[(ajint)seq[i-j]][8+j];
        parr[2] += turns[(ajint)seq[i-j]][8+j];
        parr[3] += coil[(ajint)seq[i-j]][8+j];
      }
      if ((i+j)<n0) {
        parr[0] += helix[(ajint)seq[i+j]][8-j];
        parr[1] += extend[(ajint)seq[i+j]][8-j];
        parr[2] += turns[(ajint)seq[i+j]][8-j];
        parr[3] += coil[(ajint)seq[i+j]][8-j];
      }
    }
    parr[0] -= dch;
    parr[1] -= dcs;

    k = 0;

    for (j=1; j<4; j++) if (parr[j]>parr[k]) k=j;
    if (parr[lastk]>=parr[k]) k=lastk;
    lastk = k;
    type[i]=carr[k];
    iarr[k]++;
  }

    ajFmtPrintF(outf," GARNIER plot of %s, %3d aa; DCH = %d, DCS = %d\n",
         name,n0,dch,dcs);
    ajFmtPrintF(outf,"%s\n",refstr);


  l1 = n0/60 + 1;
  for (l0=0; l0<l1; l0++) {
    ajFmtPrintF(outf,"       ");
    for (i=l0*60+9; i<n0 && i<(l0+1)*60; i+=10)
      ajFmtPrintF(outf,"    .%5d",i+1);
    ajFmtPrintF(outf,"\n       ");
    for (i=l0*60; i<n0 && i<(l0+1)*60; i++) 
      ajFmtPrintF(outf,"%c",amino[(ajint)seq[i]]);
    ajFmtPrintF(outf,"\n helix ");
    for (i=l0*60; i<n0 && i<(l0+1)*60; i++)
      ajFmtPrintF(outf,"%c",(type[i]=='H')?'H':' ');
    ajFmtPrintF(outf,"\n sheet ");
    for (i=l0*60; i<n0 && i<(l0+1)*60; i++)
      ajFmtPrintF(outf,"%c",(type[i]=='E')?'E':' ');
    ajFmtPrintF(outf,"\n turns ");
    for (i=l0*60; i<n0 && i<(l0+1)*60; i++)
      ajFmtPrintF(outf,"%c",(type[i]=='T')?'T':' ');
    ajFmtPrintF(outf,"\n coil  ");
    for (i=l0*60; i<n0 && i<(l0+1)*60; i++)
      ajFmtPrintF(outf,"%c",(type[i]=='C')?'C':' ');
    ajFmtPrintF(outf,"\n\n");
  }
  ajFmtPrintF(outf," Residue totals: H:%3d   E:%3d   T:%3d   C:%3d\n",
         iarr[0],iarr[1],iarr[2],iarr[3]);
  fn0 = (float)(n0-16)/100.0;
  ajFmtPrintF(outf,"        percent: H: %4.1f E: %4.1f T: %4.1f C: %4.1f\n",
         (float)iarr[0]/fn0,(float)iarr[1]/fn0,(float)iarr[2]/fn0,
         (float)iarr[3]/fn0);
    ajFmtPrintF(outf,"-----------------------------------------------"
		"---------------------\n\n");

    AJFREE(type);
    
    return;
}


/* @funcstatic garnier_report *************************************************
**
** Undocumented.
**
** @param [?] report [AjPReport] Undocumented
** @param [?] TabRpt [AjPFeattable] Undocumented
** @param [?] seqobj [AjPSeq] Undocumented
** @param [?] from [ajint] Undocumented
** @param [?] to [ajint] Undocumented
** @param [?] seq [char*] Undocumented
** @param [?] begin [ajint] Undocumented
** @param [?] Idc [ajint] Undocumented
** @@
******************************************************************************/


static void garnier_report(AjPReport report, AjPFeattable TabRpt,
			   AjPSeq seqobj,
			   ajint from, ajint to, char *seq,
			   ajint begin, ajint Idc)
{
    char *refstr="\n Please cite:\n Garnier, Osguthorpe and Robson (1978) J. Mol. Biol. 120:97-120\n";
    
    ajint i;
    ajint end;
    ajint amap[20];
    ajint nna=20;
    ajint parr[4];
    char carr[]="HETC";
    char type[5000];
    ajint iarr[4];
    ajint dharr[]=
    {
	0,158,-75,-100
    }
    ;
    ajint dsarr[]=
    {
	0,50,-88,-88
    }
    ;
    ajint n0;
    ajint j, k, l0, l1, idc, dcs, dch, lastk;
    float fn0;
    AjPStr tmpStr=NULL;
    AjPStr strHelix=NULL;
    AjPStr strExtend=NULL;
    AjPStr strTurns=NULL;
    AjPStr strCoil=NULL;
    AjPFeature gf=NULL;
    char testch = ' ';

    if (!strHelix) {
      ajStrAssC (&strHelix, "helix");
      ajStrAssC (&strExtend, "strand");
      ajStrAssC (&strTurns, "turn");
      ajStrAssC (&strCoil, "coil");
    }

    idc=Idc;
    end=to-from+1;
    n0=end;
      
    sascii = aascii;
    
    if (idc<=0) dcs=dch=0;
    else if (idc <4) 
    {
	dch=dharr[idc];
	dcs=0;
    }
    else if (idc<=6)
    {
	dch=0;
	dcs=dsarr[idc-3];
    }
    else dcs=dch=0;
    
    garnier_makemap(amino,amap,nna);  

    
/* copy from garnier.c original */
    --n0;  /* AJB: Added as n0 was one greater than the sequence length */
    
  for (i=0; i<n0; i++)  {
/*      ajDebug("seq[%d] '%c' %x", i, seq[i], seq[i]);
*/      
    seq[i] = amap[aascii[(ajint)seq[i]]];
/*
    ajDebug(" -> %x\n", seq[i]);
*/
  }	
    
  for(k=0;k<4;++k) iarr[k]=0;

  lastk = 0;
  for (i=0; i<n0; i++) {
    parr[0]=helix[(ajint)seq[i]][8];
    parr[1]=extend[(ajint)seq[i]][8];
    parr[2]=turns[(ajint)seq[i]][8];
    parr[3]=coil[(ajint)seq[i]][8];

    for (j=1; j<9; j++) {
      if ((i-j)>=0) {
        parr[0] += helix[(ajint)seq[i-j]][8+j];
        parr[1] += extend[(ajint)seq[i-j]][8+j];
        parr[2] += turns[(ajint)seq[i-j]][8+j];
        parr[3] += coil[(ajint)seq[i-j]][8+j];
      }
      if ((i+j)<n0) {
        parr[0] += helix[(ajint)seq[i+j]][8-j];
        parr[1] += extend[(ajint)seq[i+j]][8-j];
        parr[2] += turns[(ajint)seq[i+j]][8-j];
        parr[3] += coil[(ajint)seq[i+j]][8-j];
      }
    }
    parr[0] -= dch;
    parr[1] -= dcs;

    k = 0;

    for (j=1; j<4; j++) if (parr[j]>parr[k]) k=j;
    if (parr[lastk]>=parr[k]) k=lastk;
    lastk = k;
    type[i]=carr[k];
    iarr[k]++;
  }

  ajStrAssC (&tmpStr, "");
  ajFmtPrintAppS (&tmpStr,
		  "DCH = %d, DCS = %d\n",
		  dch,dcs);
  ajFmtPrintAppS (&tmpStr, "%s\n",refstr);

  ajReportSetHeader (report, tmpStr);
  
  /*
  l1 = n0/60 + 1;
  for (l0=0; l0<l1; l0++) {
    ajFmtPrintF(outf,"       ");
    for (i=l0*60+9; i<n0 && i<(l0+1)*60; i+=10)
      ajFmtPrintF(outf,"    .%5d",i+1);
    ajFmtPrintF(outf,"\n       ");
    for (i=l0*60; i<n0 && i<(l0+1)*60; i++) 
      ajFmtPrintF(outf,"%c",amino[(ajint)seq[i]]);
    ajFmtPrintF(outf,"\n helix ");
    for (i=l0*60; i<n0 && i<(l0+1)*60; i++)
      ajFmtPrintF(outf,"%c",(type[i]=='H')?'H':' ');
    ajFmtPrintF(outf,"\n sheet ");
    for (i=l0*60; i<n0 && i<(l0+1)*60; i++)
      ajFmtPrintF(outf,"%c",(type[i]=='E')?'E':' ');
    ajFmtPrintF(outf,"\n turns ");
    for (i=l0*60; i<n0 && i<(l0+1)*60; i++)
      ajFmtPrintF(outf,"%c",(type[i]=='T')?'T':' ');
    ajFmtPrintF(outf,"\n coil  ");
    for (i=l0*60; i<n0 && i<(l0+1)*60; i++)
      ajFmtPrintF(outf,"%c",(type[i]=='C')?'C':' ');
    ajFmtPrintF(outf,"\n\n");
  }
  */

  testch = ' ';
  l0=1;
  l1=0;

  for (i=0; i<=n0; i++) {
    if (i==n0 || type[i] != testch) {
      if (i) {
	switch (testch) {
	case 'H':
	  gf = ajFeatNewProt (TabRpt, NULL, strHelix, l0, i, 0.0);
	  ajFmtPrintS(&tmpStr, "*helix H");
	  ajFeatTagAdd(gf,  NULL, tmpStr);
	  break;
	case 'E':
	  gf = ajFeatNewProt (TabRpt, NULL, strExtend, l0, i, 0.0);
	  ajFmtPrintS(&tmpStr, "*sheet E");
	  ajFeatTagAdd(gf,  NULL, tmpStr);
	  break;
	case 'T':
	  gf = ajFeatNewProt (TabRpt, NULL, strTurns, l0, i, 0.0);
	  ajFmtPrintS(&tmpStr, "*turns T");
	  ajFeatTagAdd(gf,  NULL, tmpStr);
	  break;
	case 'C':
	  gf = ajFeatNewProt (TabRpt, NULL, strCoil, l0, i, 0.0);
	  ajFmtPrintS(&tmpStr, "*coil C");
	  ajFeatTagAdd(gf,  NULL, tmpStr);
	  break;
	default:
	  break;
	}
	l0=i+1;
      }
      if (i < n0) testch = type[i];
    }

  }



  ajStrAssC (&tmpStr, "");

  ajFmtPrintAppS (&tmpStr,
		  " Residue totals: H:%3d   E:%3d   T:%3d   C:%3d\n",
		  iarr[0],iarr[1],iarr[2],iarr[3]);
  fn0 = (float)(n0-16)/100.0;
  ajFmtPrintAppS (&tmpStr,
		  "        percent: H: %4.1f E: %4.1f T: %4.1f C: %4.1f\n",
		  (float)iarr[0]/fn0,(float)iarr[1]/fn0,(float)iarr[2]/fn0,
		  (float)iarr[3]/fn0);

  ajReportSetTail (report, tmpStr);

    ajStrDel(&tmpStr);
    ajStrDel(&strExtend);
    ajStrDel(&strHelix);
    ajStrDel(&strTurns);
    ajStrDel(&strCoil);

  return;
}

/* @funcstatic garnier_makemap ***********************************************
**
** Undocumented.
**
** @param [?] input [char*] Undocumented
** @param [?] map [ajint*] Undocumented
** @param [?] n [ajint] Undocumented
** @@
******************************************************************************/


static void garnier_makemap (char *input, ajint *map, ajint n)
{
    ajint i;
    
    for (i=0;i<n;i++) 
    {
	map[aascii[(ajint)input[i]]]=i;
	/*
	 *      ajDebug("i: %d map...[i] %x input[i] %x '%c'\n",
	 *	i, map[aascii[input[i]]], input[i], input[i]);
	 */	
    }

    return;
}
