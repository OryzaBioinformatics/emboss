#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajphylo_h
#define ajphylo_h

/* @data AjPPhyloDist *********************************************************
**
** Ajax phylogeny distance matrix
**
** Input can be square (all values) or lower-triangular (diagonal and below)
** or upper-triangular (diagonal and above). We can count values for the
** first 2 species to identify the format.
**
** S-format allows degree of replication for each distance (integer)
** we can check for this (twice as many numbers) otherwise we set the
** replicates to 1.
** 
** @alias AjSPhyloDist
** @alias AjOPhyloDist
**
** @@
******************************************************************************/

typedef struct AjSPhyloDist
{
    ajint Size;
    AjBool HasReplicates;     /* Has (some) replicates data in file */
    AjBool HasMissing;			/* Has missing data in file */
    AjPStr* Names;			/* Row names, NULL at end */
    float* Data;	      /* Distance matrix Size*Size diag=0.0 */
    ajint* Replicates;	       /* Replicate count default=1 missing=0 */
} AjOPhyloDist;

#define AjPPhyloDist AjOPhyloDist*

/* @data AjPPhyloFreq *********************************************************
**
** Ajax phylogeny frequencies.
**
** For continuous data there are always 2 alleles
** For gene frequency data there can be more than 2 alleles
**
** @alias AjSPhyloFreq
** @alias AjOPhyloFreq
**
** @@
******************************************************************************/

typedef struct AjSPhyloFreq
{
    ajint Size;
    ajint Loci;				/* Number of loci per name*/
    ajint Len;				/* Number of values per name
					 may be more than 1 per locus */
    AjBool ContChar;			/* Continuous character data */
    AjBool Within;		   /* Individual data within species*/
    AjPStr* Names;			/* Row names array size is Size */

/* row grouping - multiple individual values for one 'species' */
/* ContChar data only, otherwise NULL */
    ajint* Species;		/* Species number 1, 2, 3 for each value
				   array size is Len */
    ajint* Individuals;			/* Allele countNumber of individuals1
					   1 or more per species
					   array size is Loci */

/* column grouping - multiple frequence values for alleles of a locus */
    ajint* Locus;		/* Locus number 1, 2, 3 for each value
				   array size is Len */
    ajint* Allele;			/* Allele count 2 or more per locus
					 array size is Loci */
    float* Data;			/* Frequency for each allele
					 for each Name*/
} AjOPhyloFreq;

#define AjPPhyloFreq AjOPhyloFreq*

/* @data AjPPhyloProp *********************************************************
**
** Ajax phylogeny properties: weights, ancestral states, factors.
**
** Basically, all of these are one value per position
**
** Weights are converted to integers 0-9, A=10 Z=35 by phylip
** There are programs that can use multiple weights
** We can handle this by making all of these multiple,
**  and using ACD to limit them to 1 for nonm-weight data.
**
** Ancestral states are character data
**
** Factors are multi-state character data where the factor character changes
** when moving to a new character. Without this, all factors are assumed to
** be different. The default would be to make each character distinct by
** alternating 12121212 or to use 12345678901234567890.
**
** We can, in fact, convert any input string into this format for factors
** but probably we can leave them unchanged.
**
** @alias AjSPhyloProperty
** @alias AjOPhyloProperty
**
** @@
******************************************************************************/

typedef struct AjSPhyloProp
{
    ajint Len;				/* string length */
    ajint Size;				/* number of strings */
    AjBool IsWeight;			/* is phylip weight values */
    AjBool IsFactor;			/* is phylip factor values */
    AjPStr* Str;			/* The original string(s) */
} AjOPhyloProp;

#define AjPPhyloProp AjOPhyloProp*

/* @data AjPPhyloState ********************************************************
**
** Ajax discrete state data.
**
** Basically, all of these are one value per position
**
** States have a limited character set, usually defined through ACD
**
** @alias AjSPhyloState
** @alias AjOPhyloState
**
** @@
******************************************************************************/

typedef struct AjSPhyloState
{
    ajint Len;				/* string length */
    ajint Size;				/* number of strings */
    ajint Count;		  /* number of enzymes for restriction data */
    AjPStr Characters;			/* The allowed state characters */
    AjPStr* Names;			/* The names */
    AjPStr* Str;			/* The original string(s) */
} AjOPhyloState;

#define AjPPhyloState AjOPhyloState*

/* @data AjPPhyloTree *********************************************************
**
** Ajax phylogeny trees
**
** For programs that read multiple tree inputs we use an array,
** and let ACD limit the others to 1 tree.
**
** @alias AjSPhyloTree
** @alias AjOPhyloTree
**
** @@
******************************************************************************/

typedef struct AjSPhyloTree
{
    AjBool Multifurcated;		/* Multifurcating (..(a,b,c)..) */
    AjBool BaseTrifurcated;		/* 3-way base (a,b,c) */
    AjBool BaseBifurcated;		/* Rooted 2-way base (a,b) */
    AjBool BaseQuartet;			/* Unrooted quartet ((a,b),(c,d)); */
    AjPStr Tree;			/* Newick tree */
} AjOPhyloTree;

#define AjPPhyloTree AjOPhyloTree*


void           ajPhyloDistDel (AjPPhyloDist* pthis);
AjPPhyloDist   ajPhyloDistNew (void);
AjPPhyloDist   ajPhyloDistRead (AjPStr filename, ajint size, AjBool missing);
void           ajPhyloDistTrace (AjPPhyloDist thys);

void           ajPhyloFreqDel (AjPPhyloFreq* pthis);
AjPPhyloFreq   ajPhyloFreqNew (void);
AjPPhyloFreq   ajPhyloFreqRead (AjPStr filename, AjBool contchar,
				AjBool genedata, AjBool indiv);
void           ajPhyloFreqTrace (AjPPhyloFreq thys);

void           ajPhyloPropDel (AjPPhyloProp* pthis);
ajint          ajPhyloPropGetSize (AjPPhyloProp thys);
AjPPhyloProp   ajPhyloPropNew (void);
AjPPhyloProp   ajPhyloPropRead (AjPStr filename, AjPStr propchars,
			       ajint len, ajint size);

void           ajPhyloStateDel (AjPPhyloState* pthis);
AjPPhyloState  ajPhyloStateNew (void);
AjPPhyloState* ajPhyloStateRead (AjPStr filename, AjPStr statechars);

void           ajPhyloTreeDel (AjPPhyloTree* pthis);
AjPPhyloTree   ajPhyloTreeNew (void);
AjPPhyloTree*  ajPhyloTreeRead (AjPStr filename, ajint size);

#endif

#ifdef __cplusplus
}
#endif
