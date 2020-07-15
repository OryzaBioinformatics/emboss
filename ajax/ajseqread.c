/*
** This is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU Library General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"
#include "limits.h"




static ajint seqMaxGcglines = 5000;
static AjPRegexp seqRegQryWild = NULL;

static AjPRegexp seqRegRawNonseq = NULL;
static AjPRegexp seqRegNbrfId  = NULL;
static AjPRegexp seqRegStadenId = NULL;
static AjPRegexp seqRegHennigBlank = NULL;
static AjPRegexp seqRegHennigSeq   = NULL;
static AjPRegexp seqRegHennigTop   = NULL;
static AjPRegexp seqRegHennigHead  = NULL;
static AjPRegexp seqRegFitchHead = NULL;
static AjPRegexp seqRegStockholmSeq  = NULL;
static AjPRegexp seqRegAbiDots = NULL;
static AjPRegexp seqRegMaseHead = NULL;
static AjPRegexp seqRegPhylipTop  = NULL;
static AjPRegexp seqRegPhylipHead = NULL;
static AjPRegexp seqRegPhylipSeq  = NULL;
static AjPRegexp seqRegPhylipSeq2 = NULL;

static AjPRegexp seqRegGcgDot = NULL;
static AjPRegexp seqRegGcgChk = NULL;
static AjPRegexp seqRegGcgLen = NULL;
static AjPRegexp seqRegGcgTyp = NULL;
static AjPRegexp seqRegGcgNam = NULL;
static AjPRegexp seqRegGcgMsf = NULL;
static AjPRegexp seqRegGcgMsflen = NULL;
static AjPRegexp seqRegGcgMsfnam = NULL;
static AjPRegexp seqRegGcgWgt = NULL;

static AjBool seqInFormatSet = AJFALSE;

static AjPStr seqFtFmtEmbl    = NULL;
static AjPStr seqFtFmtGenbank = NULL;
static AjPStr seqFtFmtGff     = NULL;
static AjPStr seqFtFmtPir     = NULL;
static AjPStr seqFtFmtSwiss   = NULL;
static AjPStr seqUsaTest      = NULL;
static AjPStr seqQryChr       = NULL;
static AjPStr seqQryDb        = NULL;
static AjPStr seqQryList      = NULL;
static AjPStr seqReadLine     = NULL;

static AjPRegexp seqRegUsaAsis  = NULL;
static AjPRegexp seqRegUsaDb    = NULL;
static AjPRegexp seqRegUsaFmt   = NULL;
static AjPRegexp seqRegUsaId    = NULL;
static AjPRegexp seqRegUsaList  = NULL;
static AjPRegexp seqRegUsaRange = NULL;
static AjPRegexp seqRegUsaWild  = NULL;



/* @datastatic SeqPInFormat ***************************************************
**
** Sequence input formats data structure
**
** @alias SeqSInFormat
** @alias SeqOInFormat
**
** @attr Name [char*] Format name
** @attr Desc [char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Try [AjBool] If true, try for an unknown input. Duplicate names
**                    and read-anything formats are set false
** @attr Nucleotide [AjBool] True if suitable for nucleotide
** @attr Protein [AjBool] True if suitable for protein
** @attr Feature [AjBool] True if includes parsable feature data
** @attr Gap [AjBool] True if allows gap characters
** @attr Multiset [AjBool] If true, supports multiple sequence sets
**                         If false, multiple sets must be in separate files
** @attr Read [(AjBool*)] Input function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct SeqSInFormat
{
    char *Name;
    char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool Nucleotide;
    AjBool Protein;
    AjBool Feature;
    AjBool Gap;
    AjBool Multiset;
    AjBool (*Read) (AjPSeq thys, AjPSeqin seqin);
} SeqOInFormat;

#define SeqPInFormat SeqOInFormat*




/* @datastatic SeqPMsfData ****************************************************
**
** MSF format alignment data, stored until written when output file is closed
**
** @alias SeqSMsfData
** @alias SeqOMsfData
**
** @attr Table [AjPTable] Ajax table of AjPMsfItem objects
** @attr Names [AjPStr*] Sequence names
** @attr Count [ajint] Undocumented
** @attr Nseq [ajint] Number of sequences
** @attr Bufflines [ajint] Undocumented
** @attr Nexus [AjPNexus] Nexus alignment data
** @@
******************************************************************************/

typedef struct SeqSMsfData
{
    AjPTable Table;
    AjPStr* Names;
    ajint Count;
    ajint Nseq;
    ajint Bufflines;
    AjPNexus Nexus;
} SeqOMsfData;

#define SeqPMsfData SeqOMsfData*




/* @datastatic SeqPMsfItem ****************************************************
 **
 ** MSF alignment output individual sequence data
 **
 ** @alias SeqSMsfItem
 ** @alias SeqOMsfItem
 **
 ** @attr Name [AjPStr] Sequence name
 ** @attr Len [ajint] Sequence length
 ** @attr Check [ajint] Sequence GCG checksum
 ** @attr Weight [float] Weight (default 1.0)
 ** @attr Seq [AjPStr] Sequence
 ** @@
*****************************************************************************/

typedef struct SeqSMsfItem
{
    AjPStr Name;
    ajint Len;
    ajint Check;
    float Weight;
    AjPStr Seq;
} SeqOMsfItem;

#define SeqPMsfItem SeqOMsfItem*


/* @datastatic SeqPStockholm **************************************************
**
** Ajax Stockholm object.
**
** @new stockholmNew Default constructor
** @delete stockholmDel Default destructor
**
** @attr id [AjPStr] identifier
** @attr ac [AjPStr] accession
** @attr de [AjPStr] description
** @attr au [AjPStr] author
** @attr al [AjPStr] Undocumented
** @attr tp [AjPStr] Undocumented
** @attr se [AjPStr] Undocumented
** @attr ga [ajint[2]] Undocumented
** @attr tc [float[2]] Undocumented
** @attr nc [float[2]] Undocumented
** @attr bm [AjPStr] Undocumented
** @attr ref [AjPStr] Undocumented
** @attr dc [AjPStr] Undocumented
** @attr dr [AjPStr] Undocumented
** @attr cc [AjPStr] Undocumented
** @attr sacons [AjPStr] Undocumented
** @attr sscons [AjPStr] Undocumented
** @attr gs [AjPStr] Undocumented
** @attr name [AjPStr*] Undocumented
** @attr str [AjPStr*] Undocumented
** @attr n [ajint] Undocumented
** @attr Count [ajint] Count
** @@
******************************************************************************/

typedef struct SeqSStockholm
{
    AjPStr id;
    AjPStr ac;
    AjPStr de;
    AjPStr au;
    AjPStr al;
    AjPStr tp;
    AjPStr se;
    ajint  ga[2];
    float  tc[2];
    float  nc[2];
    AjPStr bm;
    AjPStr ref;
    AjPStr dc;
    AjPStr dr;
    AjPStr cc;
    AjPStr sacons;
    AjPStr sscons;
    AjPStr gs;
    AjPStr *name;
    AjPStr *str;
    ajint  n;
    ajint  Count;
} SeqOStockholm;

#define SeqPStockholm SeqOStockholm*




/* @datastatic SeqPStockholmdata **********************************************
**
** Ajax Stockholm data object (individual sequences)
**
** @new stockholmdataNew Default constructor
** @delete stockholmdataDel Default destructor
**
** @attr id [AjPStr] identifier
** @attr ac [AjPStr] accession
** @attr de [AjPStr] description
** @attr au [AjPStr] author
** @attr al [AjPStr] Undocumented
** @attr tp [AjPStr] Undocumented
** @attr se [AjPStr] Undocumented
** @attr bm [AjPStr] Undocumented
** @attr sscons [AjPStr] Undocumented
** @attr sacons [AjPStr] Undocumented
** @attr ref [AjPStr] Undocumented
** @attr dc [AjPStr] Undocumented
** @attr dr [AjPStr] Undocumented
** @attr cc [AjPStr] Undocumented
** @attr gs [AjPStr] Undocumented
** @attr ga [float[2]] Undocumented
** @attr tc [float[2]] Undocumented
** @attr nc [float[2]] Undocumented
** @@
******************************************************************************/

typedef struct SeqSStockholmdata
{
    AjPStr id;
    AjPStr ac;
    AjPStr de;
    AjPStr au;
    AjPStr al;
    AjPStr tp;
    AjPStr se;
    AjPStr bm;
    AjPStr sscons;
    AjPStr sacons;
    AjPStr ref;
    AjPStr dc;
    AjPStr dr;
    AjPStr cc;
    AjPStr gs;
    float  ga[2];
    float  tc[2];
    float  nc[2];
} SeqOStockholmdata;

#define SeqPStockholmdata SeqOStockholmdata*




/* @datastatic SeqPSelexseq ***************************************************
**
** Ajax Selex object for #=SQ information.
**
** @new selexSQNew Default constructor
** @delete selexSQDel Default destructor
**
** @attr name [AjPStr] Object name
** @attr source [AjPStr] Source file
** @attr ac [AjPStr] accession
** @attr de [AjPStr] description
** @attr wt [float] weight (default 1.0)
** @attr start [ajint] start position
** @attr stop [ajint] end position
** @attr len [ajint] length
** @@
******************************************************************************/

typedef struct SeqSSelexseq
{
    AjPStr name;
    AjPStr source;
    AjPStr ac;
    AjPStr de;
    float  wt;
    ajint  start;
    ajint  stop;
    ajint  len;
}SeqOSelexseq;

#define SeqPSelexseq SeqOSelexseq*




/* @datastatic SeqPSelex ******************************************************
**
** Ajax Selex object.
**
** @new selexNew Default constructor
** @delete selexDel Default destructor
**
** @attr id [AjPStr] identifier
** @attr ac [AjPStr] accession
** @attr de [AjPStr] description
** @attr au [AjPStr] author
** @attr cs [AjPStr] Undocumented
** @attr rf [AjPStr] Undocumented
** @attr name [AjPStr*] Undocumented
** @attr str [AjPStr*] Undocumented
** @attr ss [AjPStr*] Undocumented
** @attr ga [float[2]] Undocumented
** @attr tc [float[2]] Undocumented
** @attr nc [float[2]] Undocumented
** @attr sq [SeqPSelexseq*] Selex sequence objects
** @attr n [ajint] Number of SeqPSelexseq sequence objects
** @attr Count [ajint] Count
** @@
******************************************************************************/

typedef struct SeqSSelex
{
    AjPStr id;
    AjPStr ac;
    AjPStr de;
    AjPStr au;
    AjPStr cs;
    AjPStr rf;
    AjPStr *name;
    AjPStr *str;
    AjPStr *ss;
    float  ga[2];
    float  tc[2];
    float  nc[2];
    SeqPSelexseq *sq;
    ajint  n;
    ajint  Count;
} SeqOSelex;

#define SeqPSelex SeqOSelex*




/* @datastatic SeqPSelexdata **************************************************
**
** Ajax Selex data object (individual sequences)
**
** @new selexdataNew Default constructor
** @delete selexdataDel Default destructor
**
** @attr id [AjPStr] identifier
** @attr ac [AjPStr] accession
** @attr de [AjPStr] description
** @attr au [AjPStr] author
** @attr cs [AjPStr] Undocumented
** @attr rf [AjPStr] Undocumented
** @attr name [AjPStr] Undocumented
** @attr str [AjPStr] Undocumented
** @attr ss [AjPStr] Undocumented
** @attr ga [float[2]] Undocumented
** @attr tc [float[2]] Undocumented
** @attr nc [float[2]] Undocumented
** @attr sq [SeqPSelexseq] Selex sequence object
** @@
******************************************************************************/

typedef struct SeqSSelexdata
{
    AjPStr id;
    AjPStr ac;
    AjPStr de;
    AjPStr au;
    AjPStr cs;
    AjPStr rf;
    AjPStr name;
    AjPStr str;
    AjPStr ss;
    float  ga[2];
    float  tc[2];
    float  nc[2];
    SeqPSelexseq sq;
} SeqOSelexdata;

#define SeqPSelexdata SeqOSelexdata*






/* @datastatic SeqPListUsa ****************************************************
**
** Usa processing list of USAs from a list file.
**
** Includes data from the original USA (@listfile)
**
** @alias SeqSListUsa
** @alias SeqOListUsa
**
** @attr Begin [ajint] Begin if defined in original USA
** @attr End [ajint] End if defined in original USA
** @attr Rev [AjBool] Reverse if defined in original USA
** @attr Format [ajint] Format number from original USA
** @attr Features [AjBool] if true, process features
** @attr Formatstr [AjPStr] Format name from original USA
** @attr Usa [AjPStr] Current USA
** @@
******************************************************************************/

typedef struct SeqSListUsa
{
    ajint Begin;
    ajint End;
    AjBool Rev;
    ajint Format;
    AjBool Features;
    AjPStr Formatstr;
    AjPStr Usa;
} SeqOListUsa;

#define SeqPListUsa SeqOListUsa*




enum fmtcode {FMT_OK, FMT_NOMATCH, FMT_BADTYPE, FMT_FAIL, FMT_EOF, FMT_EMPTY};




static AjBool     seqReadAbi(AjPSeq thys, AjPSeqin seqin);

static void       seqAccSave(AjPSeq thys, const AjPStr acc);
static ajint      seqAppend(AjPStr* seq, const AjPStr line);
static ajint      seqAppendCommented(AjPStr* seq, AjBool* incomment,
				     const AjPStr line);
static AjBool     seqClustalReadseq(const AjPStr seqReadLine,
				    const AjPTable msftable);
static AjBool     seqFindInFormat(const AjPStr format, ajint *iformat);
static AjBool     seqFormatSet(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqGcgDots(AjPSeq thys, const AjPSeqin seqin,
			     AjPStr* pline, ajint maxlines, ajint *len);
static void       seqGcgRegInit(void);
static AjBool     seqGcgMsfDots(AjPSeq thys, const AjPSeqin seqin,
				AjPStr* pline,
				ajint maxlines, ajint *len);
static AjBool     seqGcgMsfHeader(const AjPStr line, SeqPMsfItem* msfitem);
static AjBool     seqGcgMsfReadseq(const AjPStr seqReadLine,
				   const AjPTable msftable);
static AjBool     seqHennig86Readseq(const AjPStr seqReadLine,
				     const AjPTable msftable);
static AjBool     seqinUfoLocal(const AjPSeqin thys);
static void       seqListNoComment(AjPStr* text);
static AjBool     seqListProcess(AjPSeq thys, AjPSeqin seqin,
				 const AjPStr usa);
static void       seqMsfTabDel(const void **key, void **value, void *cl);
static void       seqMsfTabList(const void *key, void **value, void *cl);
static AjBool     seqPhylipReadseq(const AjPStr seqReadLine,
				   const AjPTable phytable,
				   const AjPStr token,
				   ajint len, ajint* ilen, AjBool* done);
static AjBool     seqQueryField(const AjPSeqQuery qry, const AjPStr field);
static AjBool     seqQueryFieldC(const AjPSeqQuery qry, const char* field);
static AjBool     seqQueryMatch(const AjPSeqQuery query, const AjPSeq thys);
static void       seqQryWildComp(void);
static AjBool     seqRead(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadAcedb(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadClustal(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadCodata(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadDbId(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadEmbl(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadExperiment(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadFasta(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadFitch(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadFmt(AjPSeq thys, AjPSeqin seqin,
			     ajint format);
static AjBool     seqReadGcg(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGenbank(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGff(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadHennig86(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadIg(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadJackknifer(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadMase(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadMega(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadMsf(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNbrf(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNcbi(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNexus(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPhylip(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPhylipnon(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadRaw(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadSelex(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadStockholm(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadStaden(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadStrider(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadSwiss(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadText(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadTreecon(AjPSeq thys, AjPSeqin seqin);
static void       seqSelexAppend(const AjPStr src, AjPStr *dest, ajint beg,
				 ajint end);
static void       seqSelexCopy(AjPSeq *thys, SeqPSelex selex, ajint n);
static AjBool     seqSelexHeader(SeqPSelex *thys, const AjPStr line, ajint n,
				 AjBool *named, ajint *sqcnt);
static void       seqSelexPos(const AjPStr line, ajint *begin, ajint *end);
static AjBool     seqSelexReadBlock(SeqPSelex *thys, AjBool *named, ajint n,
				    AjPStr *line, AjPFileBuff buff,
				    AjBool store, AjPStr *astr);
static AjBool     seqSetInFormat(const AjPStr format);
static void       seqSetName(AjPStr* name, const AjPStr str);
static void       seqSetNameFile(AjPStr* name, const AjPSeqin seqin);
static void       seqStockholmCopy(AjPSeq *thys, SeqPStockholm stock, ajint n);
static void       seqSvSave(AjPSeq thys, const AjPStr sv);
static void       seqTaxSave(AjPSeq thys, const AjPStr tax);
static void       seqTextSeq(AjPStr* textptr, const AjPStr seq);
static void       seqUsaListTrace(const AjPList list);
static AjBool     seqUsaProcess(AjPSeq thys, AjPSeqin seqin);
static void       seqUsaRestore(AjPSeqin seqin, const SeqPListUsa node);
static void       seqUsaSave(SeqPListUsa node, const AjPSeqin seqin);


static SeqPStockholm stockholmNew(ajint i);
static void         stockholmDel(SeqPStockholm *thys);

/*
static SeqPStockholmdata stockholmdataNew(void);
static void         stockholmdataDel(SeqPStockholmdata *thys);
static SeqPSelexdata seqSelexClone(const SeqPSelexdata thys);
static void         selexDel(SeqPSelex *thys);
static void         selexdataDel(SeqPSelexdata *thys);
static SeqPSelexdata selexdataNew(void);
static void         selexseqDel(SeqPSelexseq *thys);
*/

static SeqPSelex     selexNew(ajint n);
static SeqPSelexseq  selexseqNew(void);


/* static data that needs the function definitions and so must come later */

/* @funclist seqInFormatDef ***************************************************
**
** Functions to read each sequence format
**
******************************************************************************/

static SeqOInFormat seqInFormatDef[] = {
/* "Name",        "Description" */
/*     Alias,   Try,     Nucleotide, Protein   */
/*     Feature  Gap,     Multiset,ReadFunction */
  {"unknown",     "Unknown format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  AJTRUE,  seqReadText},	/* alias for text */
  {"gcg",         "GCG sequence format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadGcg}, /* do first, headers mislead */
  {"gcg8",        "GCG old (version 8) sequence format",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadGcg}, /* alias for gcg (8.x too) */
  {"embl",        "EMBL entry format",
       AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadEmbl},
  {"em",          "EMBL entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadEmbl},	/* alias for embl */
  {"swiss",       "Swissprot entry format",
       AJFALSE, AJTRUE,  AJFALSE, AJTRUE,
       AJTRUE,  AJTRUE,  AJFALSE, seqReadSwiss},
  {"sw",          "Swissprot entry format (alias)",
       AJTRUE,  AJFALSE, AJFALSE, AJTRUE,
       AJTRUE,  AJTRUE,  AJFALSE, seqReadSwiss}, /* alias for swiss */
  {"swissprot",   "Swissprot entry format(alias)",
       AJTRUE,  AJTRUE,  AJFALSE, AJTRUE,
       AJTRUE,  AJTRUE,  AJFALSE, seqReadSwiss},
  {"nbrf",        "NBRF/PIR entry format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadNbrf},	/* test before NCBI */
  {"pir",         "NBRF/PIR entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  AJFALSE, seqReadNbrf},	/* alias for nbrf */
  {"fasta",       "FASTA format including NCBI-style IDs",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadNcbi}, /* alias for ncbi,
						    preferred name */
  {"ncbi",        "FASTA format including NCBI-style IDs (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadNcbi}, /* test before pearson */
  {"pearson",     "Plain old fasta format with IDs not parsed further",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadFasta}, /* plain fasta - off by
						 default, can read bad files */
  {"genbank",     "Genbank entry format",
       AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  AJFALSE, seqReadGenbank},
  {"gb",          "Genbank entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  AJFALSE, seqReadGenbank}, /* alias for genbank */
  {"ddbj",        "Genbank/DDBJ entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  AJFALSE, seqReadGenbank}, /* alias for genbank */
  {"codata",      "Codata entry format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  AJFALSE, seqReadCodata},
  {"strider",     "DNA strider output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadStrider},
  {"clustal",     "Clustalw output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadClustal},
  {"aln",         "Clustalw output format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadClustal}, /* alias for clustal */
  {"phylip",      "Phylip interleaved and non-interleaved formats",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJTRUE,  seqReadPhylip},
  {"phylipnon",   "Phylip non-interleaved format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJTRUE,  seqReadPhylipnon}, /* tried by phylip */
  {"acedb",       "ACEDB sequence format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadAcedb},
  {"dbid",        "Fasta format variant with database name before ID",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadDbId},    /* odd fasta with id as
						       second token */
  {"msf",         "GCG MSF (mutiple sequence file) file format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadMsf},
  {"hennig86",    "Hennig86 output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadHennig86},
  {"jackknifer",  "Jackknifer output interleaved format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadJackknifer},
  {"nexus",       "Nexus/paup interleaved format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadNexus},
  {"paup",        "Nexus/paup interleaved format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadNexus}, /* alias for nexus */
  {"treecon",     "Treecon output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadTreecon},
  {"mega",        "Mega interleaved output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadMega},
  {"ig",          "Intelligenetics sequence format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadIg}, /* can read almost anything */
  {"staden",      "Old staden package sequence format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadStaden}, /* original staden format */
  {"text",        "Plain text",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadText}, /* can read almost anything */
  {"plain",       "Plain text (alias)",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadText},	/* alias for text */
  {"gff",         "GFF feature file with sequence in the header",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  AJFALSE, seqReadGff},
  {"stockholm",   "Stockholm (pfam) format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadStockholm},
  {"selex",       "Selex format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadSelex},
  {"pfam",        "Stockholm (pfam) format (alias)",
       AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadStockholm},
  {"fitch",       "Fitch program format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadFitch},
  {"mase",        "Mase program format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadMase},	/* like ig - off by default*/
  {"raw",         "Raw sequence with no non-sequence characters",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJFALSE, AJFALSE, seqReadRaw}, /* OK - only sequence chars
						allowed - but off by default*/
  {"experiment",  "Staden experiment file",
       AJFALSE, AJTRUE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadExperiment},
  {"abi",         "ABI trace file",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  AJFALSE, seqReadAbi},
  {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, NULL}
};



/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Sequence Input Constructors ***************************************
**
** All constructors return a new sequence input object by pointer. It
** is the responsibility of the user to first destroy any previous
** sequence input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
******************************************************************************/

/* @func ajSeqinNew ***********************************************************
**
** Creates a new sequence input object.
**
** @return [AjPSeqin] New sequence input object.
** @category new [AjPSeqin] Default constructor
** @@
******************************************************************************/

AjPSeqin ajSeqinNew(void)
{
    AjPSeqin pthis;

    AJNEW0(pthis);

    pthis->Name  = ajStrNew();
    pthis->Acc   = ajStrNew();
    pthis->Db    = ajStrNew();
    pthis->Full  = ajStrNew();
    pthis->Date  = ajStrNew();
    pthis->Desc  = ajStrNew();
    pthis->Doc   = ajStrNew();
    pthis->Rev   = ajFalse;
    pthis->Begin = 0;
    pthis->End   = 0;
    pthis->Usa   = ajStrNew();
    pthis->Ufo   = ajStrNew();
    pthis->List  = NULL;

    pthis->Inputtype = ajStrNew();
    pthis->Formatstr = ajStrNew();
    pthis->Filename  = ajStrNew();
    pthis->Entryname = ajStrNew();


    pthis->Format    = 0;
    pthis->Filebuff  = NULL;
    pthis->Search    = ajTrue;
    pthis->Single    = ajFalse;
    pthis->Features  = ajFalse;
    pthis->Upper     = ajFalse;
    pthis->Lower     = ajFalse;
    pthis->Text      = ajFalse;
    pthis->Count     = 0;
    pthis->Filecount = 0;
    pthis->Fileseqs  = 0;
    pthis->Query     = ajSeqQueryNew();
    pthis->Data      = NULL;
    pthis->Ftquery   = ajFeattabInNew(); /* empty object */
    pthis->multi     = ajFalse;
    pthis->multiset  = ajFalse;

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Sequence Input Destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence input object.
**
******************************************************************************/




/* @func ajSeqinDel ***********************************************************
**
** Deletes a sequence input object.
**
** @param [d] pthis [AjPSeqin*] Sequence input
** @return [void]
** @category delete [AjPSeqin] Default destructor
** @@
******************************************************************************/

void ajSeqinDel(AjPSeqin* pthis)
{
    AjPSeqin thys;

    ajDebug("ajSeqinDel called usa:'%S'\n", (*pthis)->Usa);

    thys = *pthis;

    ajStrDel(&thys->Name);
    ajStrDel(&thys->Acc);

    ajStrDel(&thys->Inputtype);

    ajStrDel(&thys->Db);
    ajStrDel(&thys->Full);
    ajStrDel(&thys->Date);
    ajStrDel(&thys->Desc);
    ajStrDel(&thys->Doc);

    ajListFree(&thys->List);

    ajStrDel(&thys->Usa);
    ajStrDel(&thys->Ufo);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Entryname);
    ajStrDel(&thys->Inseq);
    ajSeqQueryDel(&thys->Query);

    if(thys->Filebuff)
	ajFileBuffDel(&thys->Filebuff);

    if(thys->Fttable)
	ajFeattableDel(&thys->Fttable);

/*
    if(thys->Ftquery && ! thys->multi)
    {
	if(thys->Ftquery->Handle)
	    ajStrDel(&thys->Ftquery->Handle->File->Name);
	if(thys->Ftquery->Handle)
	    ajStrDel(&thys->Ftquery->Handle->File->Buff);
    }
*/

    if(thys->Ftquery)		/* this deletes filebuff stuff above anyway */
        ajFeattabInDel(&thys->Ftquery);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Sequence Input Modifiers ******************************************
**
** These functions use the contents of a sequence input object and
** update them.
**
******************************************************************************/




/* @func ajSeqinUsa ***********************************************************
**
** Creates or resets a sequence input object using a new Universal
** Sequence Address
**
** @param [u] pthis [AjPSeqin*] Sequence input object.
** @param [r] Usa [const AjPStr] USA
** @return [void]
** @category modify [AjPSeqin] Resets using a new USA
** @@
******************************************************************************/

void ajSeqinUsa(AjPSeqin* pthis, const AjPStr Usa)
{
    AjPSeqin thys;

    if(!*pthis)
	thys = *pthis = ajSeqinNew();
    else
    {
	thys = *pthis;
	ajSeqinClear(thys);
    }

    ajStrAssignS(&thys->Usa, Usa);

    return;
}




/* @func ajSeqinSetNuc ********************************************************
**
** Sets the type to be forced as nucleic for a sequence input object
**
** @param [u] seqin [AjPSeqin] Sequence input object to be set.
** @return [void]
** @@
******************************************************************************/

void ajSeqinSetNuc(AjPSeqin seqin)
{
    seqin->IsNuc = ajTrue;

    return;
}




/* @func ajSeqinSetProt *******************************************************
**
** Sets the type to be forced as protein for a sequence input object
**
** @param [u] seqin [AjPSeqin] Sequence input object to be set.
** @return [void]
** @@
******************************************************************************/

void ajSeqinSetProt(AjPSeqin seqin)
{
    seqin->IsProt = ajTrue;

    return;
}




/* @func ajSeqinSetRange ******************************************************
**
** Sets the start and end positions for a sequence input object
**
** @param [u] seqin [AjPSeqin] Sequence input object to be set.
** @param [r] ibegin [ajint] Start position. Negative values are from the end.
** @param [r] iend [ajint] End position. Negative values are from the end.
** @return [void]
** @category modify [AjPSeqin] Sets a sequence range for all input sequences
** @@
******************************************************************************/

void ajSeqinSetRange(AjPSeqin seqin, ajint ibegin, ajint iend)
{

    if(ibegin)
	seqin->Begin = ibegin;

    if(iend)
	seqin->End = iend;

    return;
}




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Input Assignments ****************************************
**
** These functions overwrite the sequence input object provided as the
** first argument.
**
******************************************************************************/




/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Input Operators ******************************************
**
** These functions use the contents of a sequence input object but do
** not make any changes.
**
******************************************************************************/




/* @func ajSeqAllRead *********************************************************
**
** Parse a USA Uniform Sequence Address into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPSeq object but leave the file open for
** future calls.
**
** @param [w] thys [AjPSeq] Sequence returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @category input [AjPSeq] Master sequence stream input, reads first
**           sequence from an open input stream.
** @@
******************************************************************************/

AjBool ajSeqAllRead(AjPSeq thys, AjPSeqin seqin)
{
    AjBool ret       = ajFalse;
    AjPStr tmpformat = NULL;
    SeqPListUsa node = NULL;
    AjBool listdata  = ajFalse;

    if(!seqInFormatSet)
    {
	/* we need a copy of the formatlist */
	if(ajNamGetValueC("format", &tmpformat))
	{
	    seqSetInFormat(tmpformat);
	    ajDebug("seqSetInFormat '%S' from EMBOSS_FORMAT\n", tmpformat);
	}
	ajStrDel(&tmpformat);
	seqInFormatSet = ajTrue;
    }

    if(!seqin->Filebuff)
    {
	/* First call. No file open yet ... */
	if(!seqUsaProcess(thys, seqin)	       /* ... so process the USA */
	   && !ajListLength(seqin->List))      /* not list with bad 1st item */
	    return ajFalse; /* if this fails, we read no sequence at all */
	if(ajListLength(seqin->List))
	    listdata = ajTrue;
    }


    ret = seqRead(thys, seqin); /* read the sequence */
    if(ret)			/* clone any specified DB or entryname */
    {
	if (ajStrGetLen(seqin->Db))
	{
	    ajDebug("++ajSeqallread set db: '%S' => '%S'\n",
		    seqin->Db, thys->Db);
	    ajStrAssignS(&thys->Db, seqin->Db);
	}
	if (ajStrGetLen(seqin->Entryname))
	{
	    ajDebug("++ajSeqallread set entryname: '%S' => '%S'\n",
		    seqin->Entryname, thys->Entryname);
	    ajStrAssignS(&thys->Entryname, seqin->Entryname);
	}

	if(!ajStrGetLen(thys->Type)) /* make sure the type is set */
	    ajSeqType(thys);
    }

    while(!ret && ajListLength(seqin->List))
    {
	/* Failed, but we have a list still - keep trying it */

        ajErr("Failed to read sequence '%S'", seqin->Usa);

	ajListPop(seqin->List, (void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Usa);
	ajSeqinUsa(&seqin, node->Usa);
	ajDebug("++SAVE (AGAIN) SEQIN '%S' %d..%d(%b) '%S' %d\n",
		seqin->Usa, seqin->Begin, seqin->End, seqin->Rev,
		seqin->Formatstr, seqin->Format);
	seqUsaRestore(seqin, node);

	ajStrDel(&node->Usa);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	/* must exit if this fails ... for bad list USAs */

	if(!seqUsaProcess(thys, seqin))
	    continue;

 	ret = seqRead(thys, seqin);
    }

    if(!ret)
    {
      if(listdata)
	ajErr("Failed to read sequence '%S'", seqin->Usa);
      return ajFalse;
    }

    if (seqin->List) {
	ajSeqinClearPos(seqin);
    }
    return ret;
}




/* @func ajSeqallFile *********************************************************
**
** Parse a USA Uniform Sequence Address
**
** Return the results in the AjPSeqall object but leave the file open for
** future calls.
**
** @param [r] usa [const AjPStr] sequence usa.
** @return [AjPSeqall] seqall object
** @@
******************************************************************************/

AjPSeqall ajSeqallFile(const AjPStr usa)
{
    AjPSeqall seqall = NULL;
    AjPSeqin  seqin  = NULL;
    AjPSeq    seq    = NULL;

    seqall = ajSeqallNew();

    seqin = seqall->Seqin;
    seqin->multi  = ajTrue;
    seqin->Single = ajFalse;
    seq = seqall->Seq;

    ajSeqinUsa(&seqin,usa);

    if(!ajSeqAllRead(seq,seqin))
    {
	ajSeqallDel(&seqall);
	return NULL;
    }

    return seqall;
}




/* @func ajSeqallNext *********************************************************
**
** Reads the next sequence into a sequence stream. For the first call this
** simply returns the sequence already loaded. For later calls a new
** sequence is read.
**
** @param [u] seqall [AjPSeqall] Sequence stream
** @param [w] retseq [AjPSeq*] Sequence
** @return [AjBool] ajTrue if a sequence was refound. ajFalse when all is done.
** @category input [AjPSeq] Master sequence stream input, reads next
**                         sequence from an open input stream.
** @category modify [AjPSeqall] Master sequence stream input,
**                 reads next sequence from an open input stream.
** @@
******************************************************************************/

AjBool ajSeqallNext(AjPSeqall seqall, AjPSeq* retseq)
{

    if(!seqall->Count)
    {
	seqall->Count = 1;
	if(seqall->Rev)
	    ajSeqSetRangeRev(seqall->Seq, seqall->Begin, seqall->End);
	else
	    ajSeqSetRange(seqall->Seq, seqall->Begin, seqall->End);

	/*
	seqall->Seq->Begin = seqall->Begin;
	seqall->Seq->End   = seqall->End;
	*/

	*retseq = seqall->Seq;
	seqall->Returned = ajTrue;
	return ajTrue;
    }


    if(ajSeqRead(seqall->Seq, seqall->Seqin))
    {
	seqall->Count++;
	if(seqall->Rev)
	    ajSeqSetRangeRev(seqall->Seq, seqall->Begin, seqall->End);
	else
	    ajSeqSetRange(seqall->Seq, seqall->Begin, seqall->End);

	*retseq = seqall->Seq;
	seqall->Returned = ajTrue;

	ajDebug("ajSeqallNext success\n");
	return ajTrue;
    }

    *retseq = NULL;
    ajDebug("ajSeqallNext failed\n");
    ajSeqallClear(seqall);

    return ajFalse;
}




/* @func ajSeqinClearPos ******************************************************
**
** Clears a Sequence input object position information as possibly read from
** a USA that included the begni, end and direction
**
** @param [u] thys [AjPSeqin] Sequence input
** @return [void]
** @@
******************************************************************************/

void ajSeqinClearPos(AjPSeqin thys)
{
    thys->Rev    = ajFalse;
    thys->Begin = 0;
    thys->End = 0;
    return;
}

/* @func ajSeqinClear *********************************************************
**
** Clears a Sequence input object back to "as new" condition, except
** for the USA list and the features setting which must be preserved.
**
** @param [w] thys [AjPSeqin] Sequence input
** @return [void]
** @category modify [AjPSeqin] Resets ready for reuse.
** @@
******************************************************************************/

void ajSeqinClear(AjPSeqin thys)
{

    ajDebug("ajSeqinClear called\n");

    ajStrSetClear(&thys->Name);
    ajStrSetClear(&thys->Acc);
    /* preserve thys->Inputtype */
    ajStrSetClear(&thys->Db);
    ajStrSetClear(&thys->Full);
    ajStrSetClear(&thys->Date);
    ajStrSetClear(&thys->Desc);
    ajStrSetClear(&thys->Doc);
    /* preserve thys->List */
    ajStrSetClear(&thys->Usa);
    ajStrSetClear(&thys->Ufo);
    ajStrSetClear(&thys->Formatstr);
    ajStrSetClear(&thys->Filename);
    ajStrSetClear(&thys->Entryname);
    ajStrSetClear(&thys->Inseq);

    /* preserve thys->Query */

    if(thys->Filebuff)
	ajFileBuffDel(&thys->Filebuff);

    if(thys->Filebuff)
	ajFatal("ajSeqinClear did not delete Filebuff");

    if(thys->Fttable)
    {
	ajFeattableDel(&thys->Fttable);
    }


/*
    if(thys->Ftquery && ! thys->multi)
    {
	if(thys->Ftquery->Handle)
	    ajStrDel(&thys->Ftquery->Handle->File->Name);
	if(thys->Ftquery->Handle)
	    ajStrDel(&thys->Ftquery->Handle->File->Buff);
   }
*/


    if(thys->Ftquery)  		/* this clears filebuff stuff above anyway */
        ajFeattabInClear(thys->Ftquery);

    ajSeqQueryClear(thys->Query);
    thys->Data = NULL;

    thys->Rev    = ajFalse;
    thys->Format = 0;

    thys->Search = ajTrue;
    thys->Single = ajFalse;

    /* keep thys->Features */
    /* thys->Features = ajFalse;*/

    thys->Count     = 0;
    thys->Filecount = 0;

    thys->Begin = 0;
    thys->End = 0;
    return;
}




/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section Sequence Input Casts **********************************************
**
** These functions examine the contents of a sequence input object and
** return some derived information. Some of them provide access to the
** internal components of a sequence input object. They are provided
** for programming convenience but should be used with caution.
**
******************************************************************************/




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence inputs **********************************************
**
** These functions read the sequence provdied by the first argument
**
******************************************************************************/




/* @func ajSeqRead ************************************************************
**
** If the file is not yet open, calls seqUsaProcess to convert the USA into
** an open file stream.
**
** Uses seqRead for the actual file reading.
**
** Returns the results in the AjPSeq object.
**
** @param [w] thys [AjPSeq] Sequence returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @category input [AjPSeq] Master sequence input, calls specific functions
**                  for file access type and sequence format.
** @@
******************************************************************************/

AjBool ajSeqRead(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr tmpformat = NULL;
    AjBool ret       = ajFalse;
    SeqPListUsa node = NULL;
    AjBool listdata  = ajFalse;

    if(!seqInFormatSet)
    {
	/* we need a copy of the formatlist */
	if(ajNamGetValueC("format", &tmpformat))
	{
	    seqSetInFormat(tmpformat);
	    ajDebug("seqSetInFormat '%S' from EMBOSS_FORMAT\n", tmpformat);
	}
	ajStrDel(&tmpformat);
	seqInFormatSet = ajTrue;
    }

    if(seqin->Filebuff)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajSeqRead: input file '%F' still there, try again\n",
		seqin->Filebuff->File);
	ret = seqRead(thys, seqin);
	ajDebug("ajSeqRead: open buffer  usa: '%S' returns: %B\n",
		seqin->Usa, ret);
    }
    else
    {
	/* (b) if we have a list, try the next USA in the list */
	if(ajListLength(seqin->List))
	{
	    listdata = ajTrue;
	    ajListPop(seqin->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Usa);
	    ajSeqinUsa(&seqin, node->Usa);
	    ajDebug("++SAVE SEQIN '%S' %d..%d(%b) '%S' %d\n",
		    seqin->Usa, seqin->Begin, seqin->End, seqin->Rev,
		    seqin->Formatstr, seqin->Format);
	    seqUsaRestore(seqin, node);

	    ajStrDel(&node->Usa);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajSeqRead: open list, try '%S'\n", seqin->Usa);
	    if(!seqUsaProcess(thys, seqin) && !ajListLength(seqin->List))
		return ajFalse;
	    ret = seqRead(thys, seqin);
	    ajDebug("ajSeqRead: list usa: '%S' returns: %B\n",
		    seqin->Usa, ret);
	}
	else
	{
	    ajDebug("ajSeqRead: no file yet - test USA '%S'\n", seqin->Usa);
	    /* (c) Must be a USA - decode it */
	    if(!seqUsaProcess(thys, seqin) && !ajListLength(seqin->List))
		return ajFalse;
	    if(ajListLength(seqin->List)) /* could be a new list */
		listdata = ajTrue;
	    ret = seqRead(thys, seqin);
	    ajDebug("ajSeqRead: new usa: '%S' returns: %B\n",
		    seqin->Usa, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListLength(seqin->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read sequence '%S'", seqin->Usa);

	listdata = ajTrue;
	ajListPop(seqin->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Usa);
	ajSeqinUsa(&seqin, node->Usa);
	ajDebug("++SAVE (AGAIN) SEQIN '%S' %d..%d(%b) '%S' %d\n",
		seqin->Usa, seqin->Begin, seqin->End, seqin->Rev,
		seqin->Formatstr, seqin->Format);
	seqUsaRestore(seqin, node);

	ajStrDel(&node->Usa);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!seqUsaProcess(thys, seqin))
	    continue;

	ret = seqRead(thys, seqin);
	ajDebug("ajSeqRead: list retry usa: '%S' returns: %B\n",
		seqin->Usa, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read sequence '%S'", seqin->Usa);

	return ajFalse;
    }


    /* if values are missing in the sequence object, we can use defaults
       from seqin or calculate where possible */

    /*ajDebug("++keep restored %d..%d (%b) '%S' %d\n",
	    seqin->Begin, seqin->End, seqin->Rev,
	    seqin->Formatstr, seqin->Format);*/
    /*ajDebug("ajSeqRead: thys->Db '%S', seqin->Db '%S'\n",
	    thys->Db, seqin->Db);*/
    /*ajDebug("ajSeqRead: thys->Name '%S'\n",
	    thys->Name);*/
    /*ajDebug("ajSeqRead: thys->Entryname '%S', seqin->Entryname '%S'\n",
	    thys->Entryname, seqin->Entryname);*/
    ajStrAssignEmptyS(&thys->Db, seqin->Db);
    ajStrAssignEmptyS(&thys->Entryname, seqin->Entryname);
    /*ajStrAssignEmptyS(&thys->Name, thys->Entryname); */
    ajDebug("ajSeqRead: thys->Name '%S'\n",
	    thys->Name);

    if(!ajStrGetLen(thys->Type))
	ajSeqType(thys);
    
    return ajTrue;
}




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Set Inputs ******************************************
**
** These functions read the sequence set object provided as the
** first argument.
**
******************************************************************************/




/* @func ajSeqsetRead *********************************************************
**
** Parse a USA Uniform Sequence Address into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Read all the sequences until done
**
** Return the results in the AjPSeqset object.
**
** @param [w] thys [AjPSeqset] Sequence set returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @category input [AjPSeqset] Master input routine for a sequence
**                set
** @@
******************************************************************************/

AjBool ajSeqsetRead(AjPSeqset thys, AjPSeqin seqin)
{
    AjPSeq seq;
    AjPList setlist;

    ajint iseq = 0;

    seq = ajSeqNew();

    ajDebug("ajSeqsetRead\n");

    if(!seqUsaProcess(seq, seqin))
	return ajFalse;

    ajStrAssignS(&thys->Usa, seqin->Usa);
    ajStrAssignS(&thys->Ufo, seqin->Ufo);
    thys->Begin = seqin->Begin;
    thys->End = seqin->End;

    setlist = ajListNew();

    ajDebug("ready to start reading format '%S' '%S' %d..%d\n",
	    seqin->Formatstr, seq->Formatstr, seqin->Begin, seqin->End);

    while(ajSeqRead(seq, seqin))
    {
	if (seqin->List)
	    ajSeqinClearPos(seqin);
	/*ajDebug("read name '%S' length %d format '%S' '%S' seqindata: %x\n",
	  seq->Entryname, ajSeqGetLen(seq),
	  seqin->Formatstr, seq->Formatstr, seqin->Data);*/
	ajStrAssignEmptyS(&seq->Db, seqin->Db);
	if(!ajStrGetLen(seq->Type))
	    ajSeqType(seq);

	ajDebug ("ajSeqsetRead read sequence %d %x '%s' %d..%d (%d)\n",
		 iseq, seq, ajSeqGetNameS(seq),
		 seq->Begin, seq->End, ajSeqGetLen(seq));
	/*ajSeqTrace(seq);*/
	iseq++;

	ajListPushApp(setlist, seq);

	/*ajDebug("appended to list\n");*/

	/* add to a list of sequences */

	seq = ajSeqNew();
	seqFormatSet(seq, seqin);
    }
    ajSeqDel(&seq);

    if(!iseq)
	return ajFalse;

    /* convert the list of sequences into a seqset structure */

    ajSeqsetFromList(thys, setlist);

    ajListFree(&setlist);

    ajDebug("ajSeqsetRead total %d sequences\n", iseq);

    return ajTrue;
}




/* @func ajSeqsetallRead ******************************************************
**
** Parse a USA Uniform Sequence Address into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Read all the sequences into sequence sets until done
**
** Start a new set for each multiple sequence input
**
** Return the results in the AjPList object with AjPSeqset nodes
**
** @param [w] thys [AjPList] List of sequence sets returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqsetallRead(AjPList thys, AjPSeqin seqin)
{
    AjPSeq seq;
    AjPList setlist;
    AjPSeqset seqset = NULL;

    ajint iseq = 0;

    seq = ajSeqNew();

    ajDebug("ajSeqsetallRead\n");

    if(!seqUsaProcess(seq, seqin))
	return ajFalse;

    setlist = ajListNew();
    seqset = ajSeqsetNew();

    ajStrAssignS(&seqset->Usa, seqin->Usa);
    ajStrAssignS(&seqset->Ufo, seqin->Ufo);
    seqset->Begin = seqin->Begin;
    seqset->End = seqin->End;


    ajDebug("ready to start reading format '%S' '%S' %d..%d\n",
	    seqin->Formatstr, seq->Formatstr, seqin->Begin, seqin->End);

    while(ajSeqRead(seq, seqin))
    {
	ajDebug("read name '%S' length %d format '%S' '%S' "
		"seqindata: %x multidone: %B\n",
		seq->Entryname, ajSeqGetLen(seq),
		seqin->Formatstr, seq->Formatstr,
		seqin->Data, seqin->multidone);
	ajStrAssignEmptyS(&seq->Db, seqin->Db);
	if(!ajStrGetLen(seq->Type))
	    ajSeqType(seq);

	/*ajDebug ("ajSeqsetallRead read sequence %d '%s' %d..%d\n",
	  iseq, ajSeqName(seq), seq->Begin, seq->End);*/
	/*ajSeqTrace(seq);*/
	iseq++;

	ajListPushApp(setlist, seq);

	/*ajDebug("appended to list\n");*/

	/* add to a list of sequences */

	seq = ajSeqNew();
	seqFormatSet(seq, seqin);
	if(seqin->multidone)
	{
	    ajSeqsetFromList(seqset, setlist);
	    ajListFree(&setlist);
	    ajListPushApp(thys, seqset);
	    ajDebug("ajSeqsetallRead multidone save set %d of %d sequences\n",
		    ajListLength(thys), ajSeqsetSize(seqset));
	    setlist = ajListNew();
	    seqset = ajSeqsetNew();
	}
    }
    ajSeqDel(&seq);

    if(!iseq)
	return ajFalse;

    /* convert the list of sequences into a seqset structure */

    if(ajListLength(setlist))
    {
	ajSeqsetFromList(seqset, setlist);
	ajListFree(&setlist);
	ajListPushApp(thys, seqset);
	seqset = NULL;
    }

    ajDebug("ajSeqsetallRead total %d sets of %d sequences\n",
	    ajListLength(thys), iseq);

    return ajTrue;
}




/* @func ajSeqsetFromList *****************************************************
**
** Builds a sequence set from a list of sequences
**
** @param [w] thys [AjPSeqset] Sequence set
** @param [r] list [const AjPList] List of sequence objects
** @return [ajint] Number of sequences in the set.
******************************************************************************/

ajint ajSeqsetFromList(AjPSeqset thys, const AjPList list)
{

    ajint i;
    AjIList iter;
    AjPSeq seq;

    ajDebug("ajSeqsetFromList length: %d\n", ajListLength(list));

    ajListTrace(list);

    thys->Size      = ajListLength(list);
    thys->Seq       = AJCALLOC0(thys->Size, sizeof(AjPSeq));
    thys->Seqweight = AJCALLOC0(thys->Size, sizeof(float));

    i = 0;
    iter = ajListIterRead(list);
    ajListIterTrace(iter);
    while((seq = (AjPSeq) ajListIterNext(iter)))
    {
	if(!i)
	{
	    thys->EType = seq->EType;
	    ajStrAssignS(&thys->Type, seq->Type);
	    thys->Format = seq->Format;
	    ajStrAssignS(&thys->Formatstr, seq->Formatstr);
	    ajStrAssignS(&thys->Filename, seq->Filename);
	    ajStrAssignS(&thys->Full, seq->Full);
	}
	thys->Seqweight[i] = seq->Weight;
	thys->Seq[i] = seq;
	thys->Totweight += seq->Weight;
	if(ajSeqGetLen(seq) > thys->Len)
	    thys->Len = ajSeqGetLen(seq);
	ajDebug("seq %d '%x'\n", i, seq);
	ajDebug("seq '%x' len: %d weight: %.3f\n",
		seq->Name, ajSeqGetLen(seq), thys->Seq[i]->Weight);
	i++;
    }
    ajListIterFree(&iter);

    return thys->Size;
}




/* @func ajSeqsetFromPair *****************************************************
**
** Builds a sequence set from a pair of sequences
**
** @param [w] thys [AjPSeqset] Sequence set
** @param [r] seqa [const AjPSeq] Sequence 1
** @param [r] seqb [const AjPSeq] Sequence 2
** @return [ajint] Number of sequences in the set.
******************************************************************************/

ajint ajSeqsetFromPair(AjPSeqset thys, const AjPSeq seqa, const AjPSeq seqb)
{

    ajSeqsetApp(thys, seqa);
    ajSeqsetApp(thys, seqb);

    return thys->Size;
}




/* @func ajSeqsetApp **********************************************************
**
** Adds a sequence to a sequence set
**
** @param [w] thys [AjPSeqset] Sequence set
** @param [r] seq [const AjPSeq] Sequence
** @return [ajint] Number of sequences in the set.
******************************************************************************/

ajint ajSeqsetApp(AjPSeqset thys, const AjPSeq seq)
{
    ajint iseq;

    iseq = thys->Size;

    ajDebug("ajSeqsetApp '%S' size %d len %d add '%S' len %d\n",
	    thys->Full, thys->Size, thys->Len,
	    seq->Full, ajSeqGetLen(seq));

    thys->Size ++;
    AJCRESIZE(thys->Seq, thys->Size);
    AJCRESIZE(thys->Seqweight, thys->Size);

    if(!iseq)
    {
	thys->EType = seq->EType;
	ajStrAssignEmptyS(&thys->Type, seq->Type);
	thys->Format = seq->Format;
	ajStrAssignEmptyS(&thys->Formatstr, seq->Formatstr);
	ajStrAssignEmptyS(&thys->Filename, seq->Filename);
	ajStrAssignEmptyS(&thys->Full, seq->Full);
    }

    thys->Seqweight[iseq] = seq->Weight;
    thys->Seq[iseq] = ajSeqNewS(seq);
    thys->Totweight += seq->Weight;
    if(ajSeqGetLen(seq) > thys->Len)
	thys->Len = ajSeqGetLen(seq);

    ajDebug("result '%S' size %d len\n",
	    thys->Full, thys->Size, thys->Len);

    return thys->Size;
}




/* @funcstatic seqReadFmt *****************************************************
**
** Tests whether a sequence can be read using the specified format.
** Then tests whether the sequence matches sequence query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @param [r] format [ajint] input format code
** @return [ajint] 0 if successful.
**                 1 if the query match failed.
**                 2 if the sequence type failed
**                 3 if it failed to read a sequence
** @@
** This is the only function that calls the appropriate Read function
** seqReadXxxxxx where Xxxxxxx is the supported sequence format.
**
** Some of the seqReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajint seqReadFmt(AjPSeq thys, AjPSeqin seqin,
			ajint format)
{
    ajDebug("++seqReadFmt format %d (%s) '%S' feat %B\n",
	    format, seqInFormatDef[format].Name,
	    seqin->Usa, seqin->Features);


    /* Calling funclist seqInFormatDef() */
    if(seqInFormatDef[format].Read(thys, seqin))
    {
	ajDebug("seqReadFmt success with format %d (%s)\n",
		format, seqInFormatDef[format].Name);
	seqin->Format = format;
	ajStrAssignC(&seqin->Formatstr, seqInFormatDef[format].Name);
	ajStrAssignC(&thys->Formatstr, seqInFormatDef[format].Name);
	ajStrAssignS(&thys->Db, seqin->Db);
	ajStrAssignS(&thys->Entryname, seqin->Entryname);
	ajStrAssignS(&thys->Filename, seqin->Filename);

	if(seqQueryMatch(seqin->Query, thys))
	{
	    ajStrAssignEmptyS(&thys->Entryname, thys->Name);

	    if(seqin->Features && !thys->Fttable)
	    {
		ajStrAssignEmptyS(&seqin->Ftquery->Seqname, thys->Entryname);
		seqin->Fttable = ajFeatUfoRead(seqin->Ftquery,
				  seqin->Ufo);
		if (!seqin->Fttable)
		{
		    ajDebug("seqReadFmt features input failed UFO: '%S'\n",
			    seqin->Ufo);
		    /*
		     **  GWW 21 Aug 2000 - don't warn about missing feature
		     **  tables
		     **/
		}
		else
		{
		    ajFeattableDel(&thys->Fttable);
		    /* ajFeattableTrace(seqin->Fttable); */
		    thys->Fttable = seqin->Fttable;
		    seqin->Fttable = NULL;
		}
	    }

	    if (!ajStrGetLen(thys->Seq))	/* empty sequence string! */
		return FMT_EMPTY;

	    if(ajSeqTypeCheckIn(thys, seqin))
	    {
		/* ajSeqinTrace(seqin); */
		if(seqin->Upper)
		    ajSeqFmtUpper(thys);
		if(seqin->Lower)
		    ajSeqFmtLower(thys);
		if(seqin->Begin)
		    thys->Begin = seqin->Begin;
		if(seqin->End)
		    thys->End = seqin->End;
		if(seqin->Rev)
		    thys->Rev = seqin->Rev;
		return FMT_OK;
	    }
	    else
		return FMT_BADTYPE;
	}
	ajDebug("query match failed, continuing ...\n");
	ajSeqClear(thys);
	return FMT_NOMATCH;
    }
    else
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFileBuffIsBuffered(seqin->Filebuff),
		ajFileBuffEof(seqin->Filebuff));
	if (!ajFileBuffIsBuffered(seqin->Filebuff) &&
	    ajFileBuffEof(seqin->Filebuff))
	    return FMT_EOF;
	ajFileBuffResetStore(seqin->Filebuff, seqin->Text, &thys->TextPtr);
	ajDebug("Format %d (%s) failed, file buffer reset by seqReadFmt\n",
		format, seqInFormatDef[format].Name);
	/* ajFileBuffTraceFull(seqin->Filebuff, 10, 10);*/
    }
    ajDebug("++seqReadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic seqRead ********************************************************
**
** Given data in a seqin structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqRead(AjPSeq thys, AjPSeqin seqin)
{
    ajint i;
    ajint stat;

    AjPFileBuff buff = seqin->Filebuff;

    ajSeqClear(thys);
    ajDebug("seqRead: cleared\n");

    if(seqin->Single && seqin->Count)
    {
	/*
	 ** we read one sequence at a time.
	 ** the first sequence was read by ACD
	 ** for the following ones we need to reset the AjPSeqin
	 **
	 ** Single is set by the access method
	 */

	ajDebug("seqRead: single access - count %d - call access"
		" routine again\n",
		seqin->Count);
	/* Calling funclist seqAccess() */
	if(!seqin->Query->Access->Access(seqin))
	{
	    ajDebug("seqRead: seqin->Query->Access->Access(seqin) "
		    "*failed*\n");
	    return ajFalse;
	}
    }

    ajDebug("seqRead: seqin format %d '%S'\n", seqin->Format,
	    seqin->Formatstr);

    seqin->Count++;

    if(!seqin->Filebuff)
	return ajFalse;

    if(!seqin->Format)
    {			   /* no format specified, try all defaults */

	for(i = 1; seqInFormatDef[i].Name; i++)
	{
	    if(!seqInFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

	    ajDebug("seqRead:try format %d (%s)\n",
		    i, seqInFormatDef[i].Name);

	    stat = seqReadFmt(thys, seqin, i);
	    switch(stat)
	    {
	    case FMT_OK:
		ajDebug("++seqRead OK, set format %d\n", seqin->Format);
		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("seqRead: (a1) seqReadFmt stat == BADTYPE *failed*\n");
		return ajFalse;
	    case FMT_FAIL:
		ajDebug("seqRead: (b1) seqReadFmt stat == FAIL *failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("seqRead: (c1) seqReadFmt stat==NOMATCH try again\n");
		break;
	    case FMT_EOF:
		ajDebug("seqRead: (d1) seqReadFmt stat == EOF *failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("Sequence '%S' has zero length, ignored",
		       ajSeqGetUsa(thys));
		ajDebug("seqRead: (e1) seqReadFmt stat==EMPTY try again\n");
		break;
	    default:
		ajDebug("unknown code %d from seqReadFmt\n", stat);
	    }
	    ajSeqClear(thys);

	    if(seqin->Format)
		break;			/* we read something */
	    ajFileBuffTrace(seqin->Filebuff);
	}

	if(!seqin->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("seqRead:all default formats failed, give up\n");
	    return ajFalse;
	}
	ajDebug("++seqRead set format %d\n", seqin->Format);
    }
    else
    {					/* one format specified */
	ajDebug("seqRead: one format specified\n");
	ajFileBuffNobuff(seqin->Filebuff);

	ajDebug("++seqRead known format %d\n", seqin->Format);
	stat = seqReadFmt(thys, seqin, seqin->Format);
	switch(stat)
	{
	case FMT_OK:
	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("seqRead: (a2) seqReadFmt stat == BADTYPE *failed*\n");
	    return ajFalse;
	case FMT_FAIL:
	    ajDebug("seqRead: (b2) seqReadFmt stat == FAIL *failed*\n");
	    return ajFalse;
	case FMT_NOMATCH:
	    ajDebug("seqRead: (c2) seqReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("seqRead: (d2) seqReadFmt stat == EOF *try again*\n");
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("Sequence '%S' has zero length, ignored",
		   ajSeqGetUsa(thys));
	    ajDebug("seqRead: (e2) seqReadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from seqReadFmt\n", stat);
	}

	ajSeqClear(thys); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("seqRead failed - try again with format %d '%s'\n",
	    seqin->Format, seqInFormatDef[seqin->Format].Name);

    ajDebug("Search:%B Data:%x ajFileBuffEmpty:%B\n",
	    seqin->Search, seqin->Data, ajFileBuffEmpty(buff));
    /* while(seqin->Search) */ /* need to check end-of-file to avoid repeats */
    while(seqin->Search && (seqin->Data ||!ajFileBuffEmpty(buff)))
    {
	stat = seqReadFmt(thys, seqin, seqin->Format);
	switch(stat)
	{
	case FMT_OK:
	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("seqRead: (a3) seqReadFmt stat == BADTYPE *failed*\n");
	    return ajFalse;
	case FMT_FAIL:
	    ajDebug("seqRead: (b3) seqReadFmt stat == FAIL *failed*\n");
	    return ajFalse;
	case FMT_NOMATCH:
	    ajDebug("seqRead: (c3) seqReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("seqRead: (d3) seqReadFmt stat == EOF *failed*\n");
	    return ajFalse;			/* we already tried again */
	case FMT_EMPTY:
	    ajWarn("Sequence '%S' has zero length, ignored",
		   ajSeqGetUsa(thys));
	    ajDebug("seqRead: (e3) seqReadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from seqReadFmt\n", stat);
	}
	ajSeqClear(thys); /* 1 : read, failed to match id/acc/query */
    }

    if(seqin->Format)
	ajDebug("seqRead: *failed* to read sequence %S using format %s\n",
		seqin->Usa, seqInFormatDef[seqin->Format].Name);
    else
	ajDebug("seqRead: *failed* to read sequence %S using any format\n",
		seqin->Usa);

    return ajFalse;
}




/* @funcstatic seqReadFasta ***************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the FASTA format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadFasta(AjPSeq thys, AjPSeqin seqin)
{
    AjPFileBuff buff;
    AjPStr id   = NULL;
    AjPStr acc  = NULL;
    AjPStr sv   = NULL;
    AjPStr desc = NULL;

    const char *cp;
    ajint bufflines = 0;
    ajlong fpos     = 0;
    ajlong fposb    = 0;
    AjBool ok       = ajTrue;
    AjPStr tmpline = NULL;

    ajDebug("seqReadFasta\n");

    buff = seqin->Filebuff;

    /* ajFileBuffTrace(buff); */

    ok = ajFileBuffGetStoreL(buff, &seqReadLine, &fpos,
			     seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    bufflines++;

    ajDebug("First line: %S\n", seqReadLine);
    if(ajStrGetCharPos(seqReadLine, 3) == ';') /* then it is really PIR format */
    {
	ajStrAssignSubS(&tmpline,seqReadLine, 3, -1);
	ajFmtPrintS(&seqReadLine, ">%S",tmpline);
	ajDebug("PIR format changed line to %S\n", seqReadLine);
	ajStrDel(&tmpline);
    }

    cp = ajStrGetPtr(seqReadLine);
    if(*cp != '>')
    {
	ajDebug("first line is not FASTA\n");
	ajFileBuffReset(buff);
	return ajFalse;
    }

    if(!ajSeqParseFasta(seqReadLine, &id, &acc, &sv, &desc))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }

    seqSetName(&thys->Name, id);

    if(ajStrGetLen(sv))
	seqSvSave(thys, sv);

    if(ajStrGetLen(acc))
	seqAccSave(thys, acc);

    ajStrAssignS(&thys->Desc, desc);
    ajStrDel(&id);
    ajStrDel(&acc);
    ajStrDel(&sv);
    ajStrDel(&desc);

    if(ajStrGetLen(seqin->Inseq))
    {				       /* we have a sequence to use */
        ajDebug("++fasta use Inseq '%S'\n", seqin->Inseq);
	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);

	ajFileBuffClear(buff, 0);
    }
    else
    {
	ok = ajFileBuffGetStoreL(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, ">"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    bufflines++;
	    ajDebug("++fasta append line '%S'\n", seqReadLine);
	    ok = ajFileBuffGetStoreL(buff, &seqReadLine, &fposb,
				     seqin->Text, &thys->TextPtr);
	}

	if(ok)
	    ajFileBuffClearStore(buff, 1,
				 seqReadLine, seqin->Text, &thys->TextPtr);
	else
	    ajFileBuffClear(buff, 0);
    }

    thys->Fpos = fpos;

    ajDebug("started at fpos %Ld ok: %B fposb: %Ld\n", fpos, ok, fposb);

    return ajTrue;
}




/* @funcstatic seqReadDbId ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the FASTA >db id format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadDbId(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPFileBuff buff;

    const char *cp;
    const AjPStr vacc = NULL;
    ajint bufflines = 0;
    ajlong fpos     = 0;
    ajlong fposb    = 0;
    AjBool ok       = ajTrue;

    ajDebug("seqReadDbId\n");

    buff = seqin->Filebuff;
    /* ajFileBuffTrace(buff); */

    ok = ajFileBuffGetStoreL(buff, &seqReadLine, &fpos,
			     seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    bufflines++;

    if(ajStrGetCharPos(seqReadLine, 3) == ';') /* then it is really PIR format */
	return ajFalse;

    cp = ajStrGetPtr(seqReadLine);
    if(*cp != '>')
    {
	ajDebug("first line is not FASTA\n");
	ajFileBuffReset(buff);
	return ajFalse;
    }

    ajStrTokenAssignC(&handle, seqReadLine, "> ");
    ajStrTokenNextParseC(&handle, " \t\n\r", &token);
    ajStrTokenNextParseC(&handle, " \t\n\r", &token);
    seqSetName(&thys->Name, token);

    ajStrTokenNextParse(&handle, &token);

    vacc = ajIsSeqversion(token);
    if(vacc)
    {
	seqSvSave(thys, token);
	seqAccSave(thys, vacc);
	ajStrTokenNextParseC(&handle, "\n\r", &thys->Desc);
    }
    else if(ajIsAccession(token))
    {
	seqAccSave(thys, token);
	ajStrTokenNextParseC(&handle, "\n\r", &thys->Desc);
    }
    else
    {
	ajStrAssignS(&thys->Desc, token);
	if(ajStrTokenNextParseC(&handle, "\n\r", &token))
	{
	    ajStrAppendC(&thys->Desc, " ");
	    ajStrAppendS(&thys->Desc, token);
	}
    }

    ajStrDel(&token);
    ajStrTokenDel(&handle);

    if(ajStrGetLen(seqin->Inseq))
    {				       /* we have a sequence to use */
	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);

	ajFileBuffClear(buff, 0);
    }
    else
    {
	ok = ajFileBuffGetStoreL(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, ">"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    bufflines++;
	    ok = ajFileBuffGetStoreL(buff, &seqReadLine, &fposb,
				     seqin->Text, &thys->TextPtr);
	}
	if(ok)
	    ajFileBuffClearStore(buff, 1,
				 seqReadLine, seqin->Text, &thys->TextPtr);
	else
	    ajFileBuffClear(buff, 0);
    }

    thys->Fpos = fpos;

    ajDebug("started at fpos %Ld ok: %B fposb: %Ld\n", fpos, ok, fposb);

    return ajTrue;
}




/* @funcstatic seqReadNbrf ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using NBRF format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadNbrf(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr token  = NULL;
    static AjPStr idline = NULL;
    static AjPStr tmpline = NULL;

    AjBool dofeat       = ajFalse;

    static AjPStrTok handle2 = NULL;
    static AjPStr    token2  = NULL;
    static AjPStr    seqReadLine2 = NULL;

    AjBool ok;
    AjPFileBuff buff;

    ajDebug("seqReadNbrf\n");

    buff = seqin->Filebuff;

    if(!token2)
    {
	token2 = ajStrNew();
	seqReadLine2 = ajStrNew();
    }

    if(!seqFtFmtPir)
	ajStrAssignC(&seqFtFmtPir, "pir");

    if(!seqRegNbrfId)
	seqRegNbrfId = ajRegCompC("^>(..)[>;]([^ \t\n]+)");

    if(!ajFileBuffGetStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    ajDebug("nbrf first line:\n%S", seqReadLine);

    if(!ajRegExec(seqRegNbrfId, seqReadLine))
    {
	ajFileBuffResetStore(buff, seqin->Text, &thys->TextPtr);
	return ajFalse;
    }
    ajRegSubI(seqRegNbrfId, 1, &token);
    ajRegSubI(seqRegNbrfId, 2, &thys->Name);
    ajDebug("parsed line name '%S' token '%S' token(1) '%c'\n",
	    thys->Name, token, ajStrGetCharFirst(token));
    ajStrAssignS(&idline, seqReadLine);

    /*
     ** token has the NBRF 2-char type. First char is the type
     ** and second char is Linear, Circular, or 1
     ** or, for GCG databases, this is just '>>'
     */

    switch(toupper((ajint) ajStrGetCharFirst(token)))
    {
    case 'P':
    case 'F':
	ajSeqSetProt(thys);
	break;
    case 'B':				/* used by DIANA */
    case 'D':				/* DNA */
    case 'R':				/* RNA */
	ajSeqSetNuc(thys);
	break;
    default:
	ajWarn("Unknown NBRF sequence type '%S'", token);
    }

    /* next line is the description, with no prefix */

    if(!ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr))
    {
	ajFileBuffResetStore(buff, seqin->Text, &thys->TextPtr);
	return ajFalse;
    }

    ajStrAssignS(&thys->Desc, seqReadLine);
    if(ajStrGetCharLast(thys->Desc) == '\n')
	ajStrCutEnd(&thys->Desc, 1);

    /* read on, looking for feature and sequence lines */

    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    while(ok && !ajStrPrefixC(seqReadLine, ">"))
    {
	if(ajStrGetCharPos(seqReadLine, 1) != ';')
	    seqAppend(&thys->Seq, seqReadLine);
	else
	{
	    if(ajStrPrefixC(seqReadLine, "C;Accession:"))
	    {
		ajStrAssignC(&seqReadLine2,ajStrGetPtr(seqReadLine)+13);
		ajStrTokenAssignC(&handle2,seqReadLine2, " ;\n\r");
		while(ajStrTokenNextParse(&handle2, &token2))
		    seqAccSave(thys, token2);
	    }

	    if(ajStrPrefixC(seqReadLine, "C;Species:"))
	    {
		ajStrAssignC(&seqReadLine2,ajStrGetPtr(seqReadLine)+11);
		ajStrTokenAssignC(&handle2,seqReadLine2, ";.\n\r");
		while(ajStrTokenNextParse(&handle2, &token2))
		{
		    seqTaxSave(thys, token2);
		}
	    }

	    if(ajStrGetCharFirst(seqReadLine) == 'R')
	    {		     /* skip reference lines with no prefix */
		while((ok=ajFileBuffGetStore(buff,&seqReadLine,
					     seqin->Text, &thys->TextPtr)))
		    if(ajStrGetCharPos(seqReadLine,1)==';' || ajStrGetCharFirst(seqReadLine)=='>')
			break;		/* X; line or next sequence */

		if(ok)
		    continue;
	    }
	    else if(ajStrGetCharFirst(seqReadLine) == 'F')
	    {				/* feature lines */
		if(seqinUfoLocal(seqin))
		{
		    if(!dofeat)
		    {
			dofeat = ajTrue;
			ajFeattabInDel(&seqin->Ftquery);
			seqin->Ftquery = ajFeattabInNewSS(seqFtFmtPir,
							  thys->Name,
							  "N");
			ajDebug("seqin->Ftquery Handle %x\n",
				seqin->Ftquery->Handle);
		    }
		    ajFileBuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		    /* ajDebug("NBRF FEAT saved line:\n%S", seqReadLine); */
		}
	    }
	}
	if(ok)
	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);

	/* SRS 7 and SRS 8.0 put an extra ID line in here */

	/* SRS 8.1 is even worse - it has a peculiar bug that repeats
	   the ID line but with a few digits in front, and then repeats the
	   description */
	if(ok && !ajStrGetLen(thys->Seq) && (ajStrFindAnyK(seqReadLine, '>') != -1))
	{
	    ajStrAssignS(&tmpline, seqReadLine);
	    ajStrTrimStartC(&tmpline,"0123456789");
	    if(ajStrMatchS(tmpline, idline))
	    {
		ok = ajFileBuffGetStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		if(!ajStrIsWhite(seqReadLine)) /* SRS 8.1 description line */
		    ok = ajFileBuffGetStore(buff, &seqReadLine,
					    seqin->Text, &thys->TextPtr);
	    }
	}

    }

    ajStrTrimEndC(&thys->Seq, "*");

    if(ok)
	ajFileBuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    else
	ajFileBuffClear(buff, 0);

    if(dofeat)
    {
	ajDebug("seqin->Ftquery Handle %x\n",
		seqin->Ftquery->Handle);
	ajFeattableDel(&seqin->Fttable);
	thys->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    ajStrDel(&idline);
    return ajTrue;
}




/* @funcstatic seqReadGcg *****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using GCG format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGcg(AjPSeq thys, AjPSeqin seqin)
{
    ajint bufflines      = 0;
    AjBool ok;

    ajint len     = 0;
    AjBool incomment = ajFalse;

    AjPFileBuff buff;

    buff = seqin->Filebuff;

    ok = ajFileBuffGetStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;
    bufflines++;

    ajDebug("seqReadGcg first line ok: %B\n", ok);

    /* test GCG 9.x file types if available */
    /* any type on the .. line will override this */

    if(ajStrPrefixC(seqReadLine, "!!NA_SEQUENCE"))
	ajSeqSetNuc(thys);
    else if(ajStrPrefixC(seqReadLine, "!!AA_SEQUENCE"))
	ajSeqSetProt(thys);

    if(!seqGcgDots(thys, seqin, &seqReadLine, seqMaxGcglines, &len))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }
    ajDebug("   Gcg dots read ok len: %d\n", len);


    while(ok &&  (ajSeqGetLen(thys) < len))
    {
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(ok)
	{
	    bufflines++;
	    seqAppendCommented(&thys->Seq, &incomment, seqReadLine);
	    ajDebug("line %d seqlen: %d ok: %B\n",
		    bufflines, ajSeqGetLen(thys), ok);
	}
    }
    ajDebug("lines: %d ajSeqGetLen : %d len: %d ok: %B\n",
	    bufflines, ajSeqGetLen(thys), len, ok);

    ajFileBuffClear(buff, 0);


    return ok;
}




/* @funcstatic seqReadNcbi ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using NCBI format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadNcbi(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr id        = NULL;
    AjPStr acc       = NULL;
    AjPStr sv        = NULL;
    AjPStr gi        = NULL;
    AjPStr desc      = NULL;

    AjPFileBuff buff;

    ajint bufflines = 0;
    AjBool ok;


    buff = seqin->Filebuff;

    ok = ajFileBuffGetStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    ajStrAssignC(&id,"");
    ajStrAssignC(&acc,"");
    ajStrAssignC(&sv,"");
    ajStrAssignC(&gi,"");
    ajStrAssignC(&desc,"");


    if(!ajSeqParseNcbi(seqReadLine,&id,&acc,&sv,&gi,&desc))
    {
	ajFileBuffReset(buff);
	ajStrDel(&id);
	ajStrDel(&acc);
	ajStrDel(&sv);
	ajStrDel(&gi);
	ajStrDel(&desc);
	return ajFalse;
    }

    ajDebug("parsed id '%S' acc '%S' sv '%S' gi '%S' desc '%S'\n",
	    id, acc, sv, gi, desc);
    if(ajStrGetLen(gi))
	ajStrAssignS(&thys->Gi, gi);

    if(ajStrGetLen(sv))
	seqSvSave(thys, sv);

    if(ajStrGetLen(acc))
	seqAccSave(thys, acc);

    seqSetName(&thys->Name, id);
    ajStrAssignS(&thys->Desc, desc);



    if(ajStrGetLen(seqin->Inseq))
    {				       /* we have a sequence to use */
	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);

	ajFileBuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    }
    else
    {
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, ">"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    bufflines++;
	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

	if(ok)
	    ajFileBuffClearStore(buff, 1,
				 seqReadLine, seqin->Text, &thys->TextPtr);
	else
	    ajFileBuffClear(buff, 0);
    }

    ajStrTokenDel(&handle);
    ajStrDel(&id);
    ajStrDel(&acc);
    ajStrDel(&sv);
    ajStrDel(&gi);
    ajStrDel(&desc);

    return ajTrue;
}




/* @funcstatic seqReadSelex ***************************************************
**
** Read a Selex file. (temporary)
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadSelex(AjPSeq thys, AjPSeqin seqin)
{
    AjPFileBuff buff  = seqin->Filebuff;
    AjPStr      line  = NULL;
    SeqPSelex    selex;
    ajint       n      = 0;
    const char  *p     = NULL;
    AjBool      ok     = ajFalse;
    AjBool      isseq  = ajFalse;
    AjBool      named  = ajFalse;
    AjBool      head   = ajTrue;
    ajint       sqcnt  = 0;
    ajint       i;
    char        c      = '\0';
    AjBool      first  = ajTrue;

    line = ajStrNew();


    if(seqin->Data)
	selex = seqin->Data;
    else
    {
	ajFileBuffBuff(buff);    /* must buffer to test sequences */

	/* First count the sequences, and get any header information */
	while(!isseq && (ok=ajFileBuffGet(buff,&line)))
	{
	    if(first)
	    {
		first=ajFalse;
		if(!ajStrPrefixC(line,"#"))
		{
		    ajStrDel(&line);
		    ajFileBuffReset(buff);
		    return ajFalse;
		}
	    }
	    ajStrRemoveWhite(&line);
	    p = ajStrGetPtr(line);
	    if(!*p || *p=='#')
		continue;
	    else
		isseq = ajTrue;
	}

	if(!ok && !isseq)
	    return ajFalse;
	++n;

	ok = ajTrue;
	while(ok && ajFileBuffGet(buff,&line))
	{
	    ajStrRemoveWhite(&line);
	    p = ajStrGetPtr(line);
	    if(*p=='#')
		continue;
	    if(!*p)
		ok = ajFalse;
	    else
		++n;
	}

	ajFileBuffClear(buff,-1);
	ajFileBuffReset(buff);
	buff->Fpos = 0;
	ajFileSeek(buff->File, 0L, 0);
	selex = selexNew(n);

	/* now read it for real */

	while(head && ajFileBuffGetStore(buff,&line,
					 seqin->Text, &thys->TextPtr))
	{
	    if(ajStrPrefixC(line,"#=RF") ||ajStrPrefixC(line,"#=CS"))
		break;

	    if(ajStrPrefixC(line,"#="))
	    {
		head=seqSelexHeader(&selex,line,n,&named,&sqcnt);
		continue;
	    }
	    c = *ajStrGetPtr(line);
	    if(c>='0')
		head = ajFalse;
	}

	/* Should now be at start of first block, whether RF or sequence */
	ajDebug("First Block Line: %S",line);

	ok = ajTrue;
	while(ok && !ajStrPrefixC(line, "# ID"))
	{
	    seqSelexReadBlock(&selex,&named,n,&line,buff,
			      seqin->Text, &thys->TextPtr);
	    ok = ajFileBuffGetStore(buff,&line,
				     seqin->Text, &thys->TextPtr);
	}
	if(ok)
	    ajFileBuffClearStore(buff, 1,
				 line, seqin->Text, &thys->TextPtr);
	else
	    ajFileBuffClear(buff, 0);

	seqin->Data = selex;
    }


    /* At this point the Selex structure is fully loaded */
    if(selex->Count >= selex->n)
    {
	seqin->Data = NULL;
	ajStrDel(&line);
	return ajFalse;
    }

    i = selex->Count;

    seqSelexCopy(&thys,selex,i);

    ++selex->Count;

    ajFileBuffClear(buff,0);

    ajStrDel(&line);

    return ajTrue;
}




/* @funcstatic seqReadStockholm ***********************************************
**
** Read a Stockholm file.
**
** @param [w] thys [AjPSeq] Stockholm input file
** @param [u] seqin [AjPSeqin] seqin object
** @return [AjBool] ajTrue if success
** @@
******************************************************************************/

static AjBool seqReadStockholm(AjPSeq thys, AjPSeqin seqin)
{
    AjPFileBuff buff  = seqin->Filebuff;
    AjPStr      line  = NULL;
    AjPStr      word  = NULL;
    AjPStr      token = NULL;
    AjPStr      post  = NULL;
    AjPStr      namstr = NULL;
    AjPStr      seqstr = NULL;
    AjBool      ok    = ajFalse;
    AjBool      bmf   = ajTrue;
    AjBool      dcf   = ajTrue;
    AjBool      drf   = ajTrue;
    AjBool      ccf   = ajTrue;
    AjBool      gsf   = ajTrue;
    AjBool      reff  = ajTrue;

    SeqPStockholm stock = NULL;

    ajint i     = 0;
    ajint n     = 0;
    ajlong lpos = 0L;
    ajint  scnt = 0;

    line = ajStrNew();

    ajDebug("seqReadStockholm EOF:%B Data:%x\n",
	    ajFileBuffEof(buff), seqin->Data);
    if(seqin->Data)
	stock = seqin->Data;
    else
    {
	ajFileBuffBuff(buff); 		/* must buffer to test sequences */
	ajFileBuffTraceFull(buff, 20, 0);
	lpos = ajFileTell(buff->File);
	ok=ajFileBuffGetStore(buff,&line, seqin->Text, &thys->TextPtr);

	if(!ok || !ajStrPrefixC(line,"# STOCKHOLM 1."))
	{
	    if (ok)
		ajDebug("Stockholm: bad first line: %S", line);
	    else
		ajDebug("Stockholm: no first line\n");
	    ajFileBuffReset(buff);
	    ajStrDel(&line);
	    return ajFalse;
	}

	ajDebug("Stockholm: good first line: %S", line);

	while(ok && (ajStrPrefixC(line, "#") || ajStrMatchC(line, "\n")))
	{
	    if(ajStrPrefixC(line,"#=GF SQ"))
	    {
		ajFmtScanS(line,"%*s%*s%d",&n);
		ajDebug("Stockholm: parsed SQ line of %d sequences\n", n);
	    }
	    ok=ajFileBuffGetStore(buff,&line, seqin->Text, &thys->TextPtr);
	    ajDebug("Stockholm: SQ search: %S", line);
	}

	if (!n)				/* no SQ line, count first block */
	{
	    while(ok && !ajStrMatchC(line, "\n"))
	    {
		n++;
		ok=ajFileBuffGetStore(buff,&line, seqin->Text, &thys->TextPtr);
		ajDebug("Stockholm: block %d read: %S", n, line);
	    }
	    ajDebug("Stockholm: read block of %d sequences\n", n);
	}
	ajFileSeek(buff->File,lpos,0);
	ajFileBuffClear(buff,-1);
	ajFileBuffReset(buff);

	/* Commented out by jison ... was causing incorrect parsing for input file
	   from HMMER tutorial. */
        /* ok=ajFileBuffGetStore(buff,&line,
	                       seqin->Text, &thys->TextPtr); */
	ok=ajFileBuffGetStore(buff,&line,
			       seqin->Text, &thys->TextPtr);
	stock = stockholmNew(n);

	ajDebug("Created stockholm data object size: %d\n", n);

	word  = ajStrNew();
	token = ajStrNew();
	post  = ajStrNew();

	if(!seqRegStockholmSeq)
	    seqRegStockholmSeq = ajRegCompC("^([^ \t\n]+)[ \t]+([^ \t\n]+)[ \t]+");
	while(ok && !ajStrPrefixC(line,"//"))
	{
	    if(ajRegExec(seqRegStockholmSeq,line))
	    {
		ajRegSubI(seqRegStockholmSeq,1,&word);
		ajRegSubI(seqRegStockholmSeq,2,&token);
		ajRegPost(seqRegStockholmSeq,&post);
		ajStrRemoveLastNewline(&post);

		if(!ajStrCmpC(word,"#=GF"))
		{
		    if(!ajStrCmpC(token,"ID"))
			ajStrAssignS(&stock->id,post);
		    else if(!ajStrCmpC(token,"AC"))
			ajStrAssignS(&stock->ac,post);
		    else if(!ajStrCmpC(token,"DE"))
			ajStrAssignS(&stock->de,post);
		    else if(!ajStrCmpC(token,"AU"))
			ajStrAssignS(&stock->au,post);
		    else if(!ajStrCmpC(token,"AL"))
			ajStrAssignS(&stock->al,post);
		    else if(!ajStrCmpC(token,"SE"))
			ajStrAssignS(&stock->se,post);
		    else if(!ajStrCmpC(token,"TP"))
			ajStrAssignS(&stock->se,post);
		    else if(!ajStrCmpC(token,"GA"))
			ajFmtScanS(post,"%d%d",&stock->ga[0],
				   &stock->ga[1]);
		    else if(!ajStrCmpC(token,"TC"))
			ajFmtScanS(post,"%f%f",&stock->tc[0],
				   &stock->tc[1]);
		    else if(!ajStrCmpC(token,"NC"))
			ajFmtScanS(post,"%f%f",&stock->nc[0],
				   &stock->nc[1]);
		    else if(!ajStrCmpC(token,"BM"))
		    {
			if(bmf)
			{
			    bmf = ajFalse;
			    ajStrAssignS(&stock->bm,line);
			}
			else
			    ajStrAppendS(&stock->bm,line);
		    }
		    else if(!ajStrCmpC(token,"DC"))
		    {
			if(dcf)
			{
			    dcf = ajFalse;
			    ajStrAssignS(&stock->dc,line);
			}
			else
			    ajStrAppendS(&stock->dc,line);
		    }
		    else if(!ajStrCmpC(token,"DR"))
		    {
			if(drf)
			{
			    drf = ajFalse;
			    ajStrAssignS(&stock->dr,line);
			}
			else
			    ajStrAppendS(&stock->dr,line);
		    }
		    else if(!ajStrCmpC(token,"CC"))
		    {
			if(ccf)
			{
			    ccf = ajFalse;
			    ajStrAssignS(&stock->cc,line);
			}
			else
			    ajStrAppendS(&stock->cc,line);
		    }
		    else if(*ajStrGetPtr(token)=='R')
		    {
			if(reff)
			{
			    reff = ajFalse;
			    ajStrAssignS(&stock->ref,line);
			}
			else
			    ajStrAppendS(&stock->ref,line);
		    }
		}

		if(!ajStrCmpC(word,"#=GS"))
		{
		    if(gsf)
		    {
			gsf = ajFalse;
			ajStrAssignS(&stock->gs,line);
		    }
		    else
			ajStrAppendS(&stock->gs,line);
		}

		if(!ajStrCmpC(word,"#=GC"))
		{
		    if(!ajStrCmpC(token,"SS_cons"))
			ajStrAssignS(&stock->sscons,post);
		    else if(!ajStrCmpC(token,"SA_cons"))
			ajStrAssignS(&stock->sacons,post);
		}

	    }
	    else if (!ajStrMatchC(line, "\n"))
	    {
		ajFmtScanS(line,"%S%S", &namstr,&seqstr);
		if(!ajStrGetLen(stock->name[scnt]))
		    ajStrAppendS(&stock->name[scnt], namstr);
		else
		{
		    if(!ajStrMatchS(namstr, stock->name[scnt]))
			ajWarn("Bad stockholm format found id %d '%S' expect '%S'",
			       scnt, namstr, stock->name[scnt]);
		}
		ajStrRemoveLastNewline(&seqstr);
		ajStrAppendS(&stock->str[scnt], seqstr);
		++scnt;
		if(scnt >= n)
		    scnt = 0;
	    }

	    ok = ajFileBuffGetStore(buff,&line,
				     seqin->Text, &thys->TextPtr);
	}
	while(ok && !ajStrPrefixC(line, "# STOCKHOLM 1."))
	{
	    ok = ajFileBuffGetStore(buff,&line,
				     seqin->Text, &thys->TextPtr);
	}
	if(ok)
	    ajFileBuffClearStore(buff, 1,
				 line, seqin->Text, &thys->TextPtr);
	else
	    ajFileBuffClear(buff, 0);

	ajStrDel(&word);
	ajStrDel(&token);
	ajStrDel(&post);
	ajStrDel(&namstr);
	ajStrDel(&seqstr);
	seqin->Data = stock;
    }


    /* At this point the Stockholm structure is fully loaded */
    if(stock->Count >= stock->n)
    {
	ajDebug("Stockholm count %d: All done\n", stock->Count);
	stockholmDel(&stock);
	seqin->Data = NULL;
	ajStrDel(&line);
	return ajFalse;
    }

    i = stock->Count;

    seqStockholmCopy(&thys,stock,i);


    ++stock->Count;



    


    ajFileBuffClear(buff,0);

    ajStrDel(&line);

    return ajTrue;
}




/* @funcstatic seqSelexCopy ***************************************************
**
** Copy Selex data to sequence object.
** Pad with gaps to make lengths equal.
**
** @param [w] thys [AjPSeq*] sequence object
** @param [u] selex [SeqPSelex] seqin containing selex info
** @param [r] n [ajint] index into selex object
** @return [void]
** @@
******************************************************************************/

static void seqSelexCopy(AjPSeq *thys, SeqPSelex selex, ajint n)
{
    AjPSeq pthis   = *thys;
    /*SeqPSelexdata sdata;*/

    ajStrAssignS(&pthis->Seq, selex->str[n]);
    ajStrAssignS(&pthis->Name, selex->name[n]);
    pthis->Weight = selex->sq[n]->wt;

/*
    if(!(*thys)->Selexdata)
	(*thys)->Selexdata = selexdataNew();

    sdata = (*thys)->Selexdata;

    ajStrAssignS(&sdata->id,selex->id);
    ajStrAssignS(&sdata->ac,selex->ac);
    ajStrAssignS(&sdata->de,selex->de);
    ajStrAssignS(&sdata->au,selex->au);
    ajStrAssignS(&sdata->cs,selex->cs);
    ajStrAssignS(&sdata->rf,selex->rf);
    ajStrAssignS(&sdata->name,selex->name[n]);
    ajStrAssignS(&sdata->str,selex->str[n]);
    ajStrAssignS(&sdata->ss,selex->ss[n]);

    sdata->ga[0] = selex->ga[0];
    sdata->ga[1] = selex->ga[1];
    sdata->tc[0] = selex->tc[0];
    sdata->tc[1] = selex->tc[1];
    sdata->nc[0] = selex->nc[0];
    sdata->nc[1] = selex->nc[1];

    ajStrAssignS(&sdata->sq->name,selex->sq[n]->name);

    ajStrAssignS(&sdata->sq->ac,selex->sq[n]->ac);
    ajStrAssignS(&sdata->sq->source,selex->sq[n]->source);
    ajStrAssignS(&sdata->sq->de,selex->sq[n]->de);

    sdata->sq->wt    = selex->sq[n]->wt;
    sdata->sq->start = selex->sq[n]->start;
    sdata->sq->stop  = selex->sq[n]->stop;
    sdata->sq->len   = selex->sq[n]->len;
*/
    return;
}




/* @funcstatic seqStockholmCopy ***********************************************
**
** Copy Stockholm data to sequence object.
** Pad with gaps to make lengths equal.
**
** @param [w] thys [AjPSeq*] sequence object
** @param [u] stock [SeqPStockholm] seqin containing selex info
** @param [r] n [ajint] index into stockholm object
** @return [void]
** @@
******************************************************************************/

static void seqStockholmCopy(AjPSeq *thys, SeqPStockholm stock, ajint n)
{
    AjPSeq pthis;
    /*SeqPStockholmdata sdata;*/

    pthis = *thys;

    ajStrAssignS(&pthis->Seq, stock->str[n]);
    ajStrAssignS(&pthis->Name, stock->name[n]);

/*
    if(!(*thys)->Stock)
	(*thys)->Stock = stockholmdataNew();

    sdata = (*thys)->Stock;

    ajStrAssignS(&sdata->id,stock->id);
    ajStrAssignS(&sdata->ac,stock->ac);
    ajStrAssignS(&sdata->de,stock->de);
    ajStrAssignS(&sdata->au,stock->au);
    ajStrAssignS(&sdata->al,stock->al);
    ajStrAssignS(&sdata->tp,stock->tp);
    ajStrAssignS(&sdata->se,stock->se);
    ajStrAssignS(&sdata->gs,stock->gs);
    ajStrAssignS(&sdata->dc,stock->dc);
    ajStrAssignS(&sdata->dr,stock->dr);
    ajStrAssignS(&sdata->cc,stock->cc);
    ajStrAssignS(&sdata->ref,stock->ref);
    ajStrAssignS(&sdata->sacons,stock->sacons);
    ajStrAssignS(&sdata->sscons,stock->sscons);
    sdata->ga[0] = stock->ga[0];
    sdata->ga[1] = stock->ga[1];
    sdata->tc[0] = stock->tc[0];
    sdata->tc[1] = stock->tc[1];
    sdata->nc[0] = stock->nc[0];
    sdata->nc[1] = stock->nc[1];
*/
    return;
}




/* @funcstatic seqSelexAppend *************************************************
**
** Append sequence and related Selex info to selex object.
** Pad with gaps to make lengths equal.
**
** @param [r] src [const AjPStr] source line from Selex file
** @param [w] dest [AjPStr*] Destination in Selex object
** @param [r] beg  [ajint] start of info in src
** @param [r] end  [ajint] end of info in src
** @return [void]
** @@
******************************************************************************/

static void seqSelexAppend(const AjPStr src, AjPStr *dest,
			   ajint beg, ajint end)
{
    const char *p = NULL;
    char c;
    ajint len;
    ajint i;
    ajint pad = 0;

    len = end-beg+1;
    p   = ajStrGetPtr(src);

    if(beg>=ajStrGetLen(src))
    {
	for(i=0;i<len;++i)
	    ajStrAppendK(dest,'-');
	return;
    }

    p += beg;
    pad = end - ajStrGetLen(src) + 2;

    while((c=*p) && *p!='\n')
    {
	if(c=='.' || c=='_' || c==' ')
	    c='-';
	ajStrAppendK(dest,c);
	++p;
    }

    for(i=0;i<pad;++i)
	ajStrAppendK(dest,'-');

    return;
}




/* @funcstatic seqSelexHeader *************************************************
**
** Load a Selex object with header information for a single line
**
** @param [w] thys [SeqPSelex*] Selex object
** @param [r] line [const AjPStr] Selex header line
** @param [r] n  [ajint] Number of sequences in Selex file
** @param [w] named  [AjBool*] Whether names of sequences have been read
** @param [w] sqcnt  [ajint*] Number of SQ names read
** @return [AjBool] ajTrue if the line contained header information
** @@
******************************************************************************/

static AjBool seqSelexHeader(SeqPSelex *thys, const AjPStr line, ajint n,
			     AjBool *named, ajint *sqcnt)
{
    SeqPSelex pthis;
    AjPStrTok token = NULL;
    AjPStr handle   = NULL;


    pthis = *thys;


    if(ajStrPrefixC(line,"#=ID"))
    {
	ajFmtScanS(line,"#=ID %S",&pthis->id);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=AC"))
    {
	ajFmtScanS(line,"#=AC %S",&pthis->ac);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=DE"))
    {
	ajStrAssignC(&pthis->de,ajStrGetPtr(line)+5);
	ajStrRemoveWhite(&pthis->de);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=AU"))
    {
	ajStrAssignC(&pthis->au,ajStrGetPtr(line)+5);
	ajStrRemoveWhite(&pthis->au);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=GA"))
    {
	ajFmtScanS(line,"%*s %f %f",&pthis->ga[0],&pthis->ga[1]);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=TC"))
    {
	ajFmtScanS(line,"%*s %f %f",&pthis->tc[0],&pthis->tc[1]);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=NC"))
    {
	ajFmtScanS(line,"%*s %f %f",&pthis->nc[0],&pthis->nc[1]);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=SQ"))
    {
	handle = ajStrNew();
	token = ajStrTokenNewC(line," \t\n");
	ajStrTokenNextParse(&token,&handle);

	ajStrTokenNextParse(&token,&pthis->sq[*sqcnt]->name);
	ajStrAssignS(&pthis->name[*sqcnt],pthis->sq[*sqcnt]->name);

	ajStrTokenNextParse(&token,&handle);
	ajStrToFloat(handle,&pthis->sq[*sqcnt]->wt);

	ajStrTokenNextParse(&token,&handle);
	ajStrAssignS(&pthis->sq[*sqcnt]->source,handle);

	ajStrTokenNextParse(&token,&handle);
	ajStrAssignS(&pthis->sq[*sqcnt]->ac,handle);

	ajStrTokenNextParse(&token,&handle);
	ajFmtScanS(handle,"%d..%d:%d",&pthis->sq[*sqcnt]->start,
		   &pthis->sq[*sqcnt]->stop,&pthis->sq[*sqcnt]->len);

	ajStrTokenNextParseC(&token,"\n",&handle);
	ajStrAssignS(&pthis->sq[*sqcnt]->de,handle);

	ajStrTokenDel(&token);
	ajStrDel(&handle);
	*named = ajTrue;
	++(*sqcnt);
	return ajTrue;
    }


    return ajFalse;
}




/* @funcstatic seqSelexPos ****************************************************
**
** Find start and end positions of sequence & related Selex information
**
** @param [r] line [const AjPStr] Selex sequence or related line
** @param [w] begin  [ajint*] start pos
** @param [w] end  [ajint*] end pos
** @return [void]
** @@
******************************************************************************/

static void seqSelexPos(const AjPStr line, ajint *begin, ajint *end)
{
    ajint pos = 0;
    ajint len = 0;

    const char  *p;

    /*
    **  Selex sequence info can start any number of spaces
    **  after the names so we need to find out where to
    **  start counting chars from and where to end
     */

    len  = ajStrGetLen(line) - 1;
    pos  = len -1;
    *end = (pos > *end) ? pos : *end;
    p = ajStrGetPtr(line);

    while(*p && *p!=' ')
	++p;
    while(*p && *p==' ')
	++p;
    if(p)
	pos = p - ajStrGetPtr(line);
    *begin = (pos < *begin) ? pos : *begin;


    return;
}




/* @funcstatic seqSelexReadBlock **********************************************
**
** Read a block of sequence information from a selex file
**
** @param [w] thys [SeqPSelex*] Selex object
** @param [w] named  [AjBool*] Whether names of sequences have been read
** @param [r] n  [ajint] Number of sequences in Selex file
** @param [u] line [AjPStr*] Line from Selex file
** @param [u] buff  [AjPFileBuff] Selex file buffer
** @param [r] store [AjBool] store if ajTrue
** @param [w] astr [AjPStr*] string to append to
** @return [AjBool] ajTrue if data was read.
** @@
******************************************************************************/

static AjBool seqSelexReadBlock(SeqPSelex *thys, AjBool *named, ajint n,
				AjPStr *line, AjPFileBuff buff,
				AjBool store, AjPStr *astr)
{
    SeqPSelex pthis;
    AjPStr *seqs = NULL;
    AjPStr *ss   = NULL;

    AjPStr rf = NULL;
    AjPStr cs = NULL;
    ajint  i;
    ajint  begin;
    ajint  end;
    AjBool ok;
    ajint  cnt;
    AjPStr tmp    = NULL;
    AjBool haverf = ajFalse;
    AjBool havecs = ajFalse;
    AjBool havess = ajFalse;

    pthis = *thys;

    begin = INT_MAX;
    end   = -(INT_MAX);

    tmp = ajStrNew();
    rf = ajStrNew();
    cs = ajStrNew();
    AJCNEW(seqs,n);
    AJCNEW(ss,n);
    for(i=0;i<n;++i)
    {
	seqs[i] = ajStrNew();
	ss[i]  = ajStrNew();
    }

    ok = ajTrue;
    cnt = 0;

    while(ok)
    {
	seqSelexPos(*line,&begin,&end);
	if(ajStrPrefixC(*line,"#=RF"))
	{
	    haverf=ajTrue;
	    ajStrAssignS(&rf,*line);
	}

	if(ajStrPrefixC(*line,"#=CS"))
	{
	    havecs=ajTrue;
	    ajStrAssignS(&cs,*line);
	}

	if(ajStrPrefixC(*line,"#=SS"))
	{
	    havess=ajTrue;
	    ajStrAssignS(&ss[--cnt],*line);
	    ++cnt;
	}

	if(!ajStrPrefixC(*line,"#"))
	{
	    if(!*named)
	    {
		ajFmtScanS(*line,"%S",&pthis->name[cnt]);
		ajStrAssignS(&pthis->sq[cnt]->name,pthis->name[cnt]);
	    }
	    else
	    {
		ajFmtScanS(*line,"%S",&tmp);
		if(!ajStrPrefixS(pthis->name[cnt],tmp))
		    ajWarn("Sequence names do not match [%S %S]",
			   pthis->name[cnt],tmp);
	    }

	    ajStrAssignS(&seqs[cnt],*line);
	    ++cnt;
	}

	ok = ajFileBuffGetStore(buff,line, store, astr);
	if(ajStrPrefixC(*line,"\n"))
	    ok = ajFalse;
    }


    if(haverf)
	seqSelexAppend(rf,&pthis->rf,begin,end);

    if(havecs)
	seqSelexAppend(cs,&pthis->cs,begin,end);

    for(i=0;i<n;++i)
    {
	seqSelexAppend(seqs[i],&pthis->str[i],begin,end);
	if(havess)
	    seqSelexAppend(ss[i],&pthis->ss[i],begin,end);
    }


    for(i=0;i<n;++i)
    {
	ajStrDel(&seqs[i]);
	ajStrDel(&ss[i]);
    }
    AJFREE(seqs);
    AJFREE(ss);

    ajStrDel(&rf);
    ajStrDel(&cs);
    ajStrDel(&tmp);

    *named = ajTrue;

    return ajTrue;
}




/* @funcstatic seqReadStaden **************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the old Staden package file format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadStaden(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr token  = NULL;
    ajint bufflines      = 0;
    AjPFileBuff buff;
    AjBool incomment = ajFalse;

    buff = seqin->Filebuff;

    if(!seqRegStadenId)
	seqRegStadenId = ajRegCompC("^[<]([^>-]+)[-]*[>]");

    if(!ajFileBuffGetStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;
    bufflines++;

    if(ajRegExec(seqRegStadenId, seqReadLine))
    {
	ajRegSubI(seqRegStadenId, 1, &token);
	seqSetName(&thys->Name, token);
	ajDebug("seqReadStaden name '%S' token '%S'\n",
		thys->Name, token);
	ajRegPost(seqRegStadenId, &token);
	seqAppendCommented(&thys->Seq, &incomment, token);
    }
    else
    {
	seqSetName(&thys->Name, seqin->Filename);
	seqAppendCommented(&thys->Seq, &incomment, seqReadLine);
    }

    while(ajFileBuffGetStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr))
    {
	seqAppendCommented(&thys->Seq, &incomment, seqReadLine);
	bufflines++;
    }

    ajFileBuffClear(buff, 0);

    if(!bufflines) return ajFalse;

    return ajTrue;
}




/* @funcstatic seqReadText ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using plain text format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadText(AjPSeq thys, AjPSeqin seqin)
{
    ajint bufflines      = 0;
    AjPFileBuff buff;

    ajDebug("seqReadText\n");

    buff = seqin->Filebuff;

    while(ajFileBuffGetStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr))
    {
	ajDebug("read '%S'\n", seqReadLine);
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
    }

    ajDebug("read %d lines\n", bufflines);
    ajFileBuffClear(buff, 0);

    if(!bufflines)
	return ajFalse;

    seqSetNameFile(&thys->Name, seqin);

    return ajTrue;
}




/* @funcstatic seqReadRaw *****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using raw format, which accepts only alphanumeric and whitespace
** characters or '-' for gap or '*' for a protein stop
** and rejects anything else.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadRaw(AjPSeq thys, AjPSeqin seqin)
{
    ajint bufflines      = 0;
    AjPFileBuff buff;
    ajDebug("seqReadRaw\n");

    buff = seqin->Filebuff;

    if(!seqRegRawNonseq)
	seqRegRawNonseq = ajRegCompC("[^A-Za-z0-9 \t\n\r*-]");

    while(ajFileBuffGetStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr))
    {
	ajDebug("read '%S'\n", seqReadLine);
	if(ajRegExec(seqRegRawNonseq, seqReadLine))
	{
	    ajDebug("seqReadRaw: Bad character found in line: %S\n",
		    seqReadLine);
	    ajFileBuffReset(buff);
	    ajStrAssignC(&thys->Seq,"");
	    return ajFalse;
	}
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
    }

    ajDebug("read %d lines\n", bufflines);
    ajFileBuffClear(buff, 0);

    if(!bufflines)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic seqReadIg ******************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using IntelliGenetics format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadIg(AjPSeq thys, AjPSeqin seqin)
{
    ajint bufflines      = 0;
    AjPFileBuff buff;
    AjBool ok = ajTrue;

    buff = seqin->Filebuff;

    do
    {
	/* skip comments with ';' prefix */
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
    } while(ok && ajStrPrefixC(seqReadLine, ";"));

    if(!ok)
	return ajFalse;

    ajStrAssignS(&thys->Name, seqReadLine);
    ajStrCutEnd(&thys->Name, 1);
    bufflines++;

    while(ajFileBuffGetStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr) &&
	  !ajStrPrefixC(seqReadLine, "\014"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
    }

    ajFileBuffClear(buff, 0);

    return ajTrue;
}




/* @funcstatic seqReadClustal *************************************************
**
** Tries to read input in Clustal ALN format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadClustal(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr seqstr = NULL;
    AjPStr name          = NULL;
    ajint bufflines      = 0;
    AjBool ok            = ajFalse;
    ajint iseq           = 0;
    AjPFileBuff buff     = seqin->Filebuff;
    AjPTable alntable    = NULL;
    SeqPMsfItem alnitem  = NULL;
    AjPList alnlist      = NULL;
    SeqPMsfData alndata  = NULL;

    ajint i;

    ajDebug("seqReadClustal seqin->Data %x\n", seqin->Data);

    if(!seqin->Data)
    {					/* start of file */
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
	if(!ok)
	    return ajFalse;

	ajDebug("first line:\n'%S'\n", seqReadLine);

	if(!ajStrPrefixC(seqReadLine, "CLUSTAL"))
	{
	    /* first line test */
	    ajFileBuffReset(buff);
	    return ajFalse;
	}

	ajDebug("first line OK: '%S'\n", seqReadLine);

	while(ok)
	{				/* skip blank lines */
	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;
	    if(!ajStrIsWhite(seqReadLine))
		break;
	}

	if(!ok)
	{
	    ajDebug("FAIL (blank lines only)\n");
	    ajFileBuffReset(buff);
	    return ajFalse;
	}

	seqin->Data = AJNEW0(alndata);
	alndata->Table = alntable = ajTableNew(0,ajStrTableCmp,ajStrTableHash);
	alnlist = ajListstrNew();
	seqin->Filecount = 0;

	/* first set - create table */
	ok = ajTrue;
	while(ok && ajStrExtractFirst(seqReadLine, &seqstr, &name))
	{
	    AJNEW0(alnitem);
	    ajStrAssignS(&alnitem->Name, name);
	    alnitem->Weight = 1.0;
	    seqAppend(&alnitem->Seq, seqstr);

	    iseq++;
	    ajDebug("first set %d: '%S'\n line: '%S'\n",
		    iseq, name, seqReadLine);

	    ajTablePut(alntable, name, alnitem);
	    name = NULL;
	    ajListstrPushApp(alnlist, alnitem->Name);

	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;
	}

	ajDebug("Header has %d sequences\n", iseq);
	ajListstrTrace(alnlist);
	ajTableTrace(alntable);
	ajTableMap(alntable, seqMsfTabList, NULL);

	alndata->Names = AJCALLOC(iseq, sizeof(*alndata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(alnlist, &alndata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, alndata->Names[i]);
	}
	ajListstrFree(&alnlist);

	while(ajFileBuffGetStore(buff, &seqReadLine,
				 seqin->Text, &thys->TextPtr))
	{				/* now read the rest */
	    bufflines++;
	    seqClustalReadseq(seqReadLine, alntable);
	}

	ajTableMap(alntable, seqMsfTabList, NULL);
	alndata->Nseq = iseq;
	alndata->Count = 0;
	alndata->Bufflines = bufflines;
	ajDebug("ALN format read %d lines\n", bufflines);
    }

    alndata = seqin->Data;
    alntable = alndata->Table;
    if(alndata->Count >= alndata->Nseq)
    {					/* all done */
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableMapDel(alntable, seqMsfTabDel, NULL);
	ajTableFree(&alntable);
	AJFREE(alndata->Names);
	AJFREE(alndata);
	seqin->Data = NULL;
	return ajFalse;
    }
    i = alndata->Count;
    ajDebug("returning [%d] '%S'\n", i, alndata->Names[i]);
    alnitem = ajTableGet(alntable, alndata->Names[i]);
    ajStrAssignS(&thys->Name, alndata->Names[i]);
    ajStrDel(&alndata->Names[i]);

    thys->Weight = alnitem->Weight;
    ajStrAssignS(&thys->Seq, alnitem->Seq);
    ajStrDel(&alnitem->Seq);

    alndata->Count++;

    return ajTrue;
}




/* @funcstatic seqClustalReadseq **********************************************
**
** Reads sequence name from first token on the input line, and appends
** the sequence data to that sequence in the alntable structure.
**
** @param [r] rdline [const AjPStr] Line from input file.
** @param [r] msftable [const AjPTable] MSF format sequence table.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqClustalReadseq(const AjPStr rdline, const AjPTable msftable)
{
    SeqPMsfItem msfitem;
    static AjPStr token     = NULL;
    static AjPStr seqstr    = NULL;

    if(!ajStrExtractFirst(rdline, &seqstr, &token))
	return ajFalse;

    msfitem = ajTableGet(msftable, token);
    if(!msfitem)
    {
	ajStrDel(&token);
	return ajFalse;
    }

    seqAppend(&msfitem->Seq, seqstr);

    return ajTrue;
}




/* @funcstatic seqReadPhylipnon ***********************************************
**
** Tries to read input in Phylip non-interleaved format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadPhylipnon(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr seqstr = NULL;
    static AjPStr tmpstr = NULL;
    ajint bufflines = 0;
    AjBool ok       = ajFalse;
    ajint iseq      = 0;
    ajint jseq      = 0;
    ajint len       = 0;
    ajint ilen      = 0;
    ajint maxlen    = 0;
    AjPFileBuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    SeqPMsfData phydata      = NULL;
    ajint i;
    AjBool done = ajFalse;

    ajDebug("seqReadPhylipnon seqin->Data %x\n", seqin->Data);

    buff = seqin->Filebuff;

    if(!seqRegPhylipTop)
	seqRegPhylipTop = ajRegCompC("^ *([0-9]+) +([0-9]+)");

    if(!seqRegPhylipHead)
	seqRegPhylipHead = ajRegCompC("^(..........) ?"); /* 10 chars */

    if(!seqin->Data)
    {					/* start of file */
	seqin->multidone = ajFalse;
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;
	bufflines++;

	ajDebug("first line:\n'%-20.20S'\n", seqReadLine);

	if(!ajRegExec(seqRegPhylipTop, seqReadLine))
	{				/* first line test */
	    ajFileBuffReset(buff);
	    return ajFalse;
	}

	ajRegSubI(seqRegPhylipTop, 1, &tmpstr);
	ajStrToInt(tmpstr, &iseq);
	ajDebug("seqRegPhylipTop1 '%S' %d\n", tmpstr, iseq);
	ajRegSubI(seqRegPhylipTop, 2, &tmpstr);
	ajStrToInt(tmpstr, &len);
	ajDebug("seqRegPhylipTop2 '%S' %d\n", tmpstr,len);
	ajDebug("first line OK: '%S' iseq: %d len: %d\n",
		seqReadLine, iseq, len);

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTableNew(0, ajStrTableCmp,
					       ajStrTableHash);
	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));
	seqin->Filecount = 0;

	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
	ilen = 0;
	while(ok && (jseq < iseq))
	{
	    /* first set - create table */
	    if(!ajRegExec(seqRegPhylipHead, seqReadLine))
	    {
		ajDebug("FAIL (not seqRegPhylipHead): '%S'\n", seqReadLine);
		ajFileBuffReset(buff);
		AJFREE(seqin->Data);
		return ajFalse;
	    }
	    ajDebug("line: '%S'\n", seqReadLine);
	    ajRegSubI(seqRegPhylipHead, 1, &tmpstr);
	    if(!ajStrIsWhite(tmpstr)) {
		/* check previous sequence */
		if(jseq)
		{
		    if(ilen != len)
		    {
			ajDebug("phylipnon format length mismatch at %d "
				"(length %d)\n",
				len, ilen);
			AJFREE(seqin->Data);
			return ajFalse;
		    }
		}
		/* new sequence */
		AJNEW0(phyitem);
		seqSetName(&phyitem->Name, tmpstr);
		ajStrAssignS(&phydata->Names[jseq], phyitem->Name);
		ajDebug("name: '%S' => '%S'\n", tmpstr, phyitem->Name);
		phyitem->Weight = 1.0;
		ajRegPost(seqRegPhylipHead, &seqstr);
		seqAppend(&phyitem->Seq, seqstr);
		ilen = ajStrGetLen(phyitem->Seq);
		if(ilen == len)
		    done = ajTrue;
		else if(ilen > len)
		{
		    ajDebug("Phylipnon format: sequence %S "
			    "header size %d exceeded\n",
			    phyitem->Name, len);
		    AJFREE(seqin->Data);
		    return ajFalse;
		}
		ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
		ajDebug("seq %d: (%d) '%-20.20S'\n", jseq, ilen, seqReadLine);
	    }
	    else {
		/* more sequence to append */
		if(seqPhylipReadseq(seqReadLine, phytable, phyitem->Name,
				    len, &ilen, &done))
		{
		    ajDebug("read to len %d\n", ilen);
		    if (done)
		    {
			if(!jseq)
			    maxlen = ilen;
			jseq++;
		    }
		}

	    }

	    if(jseq < iseq)
	    {
		ok = ajFileBuffGetStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		bufflines++;
	    }
	}
	if(ilen != len)
	{
	    ajDebug("phylipnon format final length mismatch at %d "
		    "(length %d)\n",
		    len, ilen);
	    return ajFalse;
	}

	ajDebug("Header has %d sequences\n", jseq);
	ajTableTrace(phytable);
	ajTableMap(phytable, seqMsfTabList, NULL);

	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = bufflines;
	ajDebug("PHYLIP format read %d lines\n", bufflines);
    }

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableGet(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);
    ajStrDel(&phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);
    ajStrDel(&phyitem->Seq);

    phydata->Count++;
    if(phydata->Count >=phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadPhylip multidone\n");
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableMapDel(phytable, seqMsfTabDel, NULL);
	ajTableFree(&phytable);
	AJFREE(phydata->Names);
	AJFREE(phydata);
	seqin->Data = NULL;
    }

    return ajTrue;
}




/* @funcstatic seqReadPhylip **************************************************
**
** Tries to read input in Phylip interleaved format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadPhylip(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr seqstr = NULL;
    static AjPStr tmpstr = NULL;
    ajint bufflines = 0;
    AjBool ok       = ajFalse;
    ajint iseq      = 0;
    ajint jseq      = 0;
    ajint len       = 0;
    ajint ilen      = 0;
    ajint maxlen    = 0;
    AjPFileBuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    AjPList phylist          = NULL;
    SeqPMsfData phydata      = NULL;
    ajint i;
    AjBool done = ajFalse;

    ajDebug("seqReadPhylip seqin->Data %x\n", seqin->Data);

    buff = seqin->Filebuff;
    ajFileBuffBuff(buff);    /* must buffer to test non-interleaved */

    if(!seqRegPhylipTop)
	seqRegPhylipTop = ajRegCompC("^ *([0-9]+) +([0-9]+)");

    if(!seqRegPhylipHead)
	seqRegPhylipHead = ajRegCompC("^(..........) ?"); /* 10 chars */

    if(!seqRegPhylipSeq)
	seqRegPhylipSeq = ajRegCompC("^[ \t\n\r]*$");

    if(!seqin->Data)
    {					/* start of file */
	seqin->multidone = ajFalse;
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	while (ok && ajStrIsWhite(seqReadLine))
	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);

	if(!ok)
	    return ajFalse;
	bufflines++;

	/* ajDebug("first line:\n'%-20.20S'\n", seqReadLine);*/

	if(!ajRegExec(seqRegPhylipTop, seqReadLine))
	{				/* first line test */
	    ajFileBuffResetStore(buff, seqin->Text, &thys->TextPtr);
	    return ajFalse;
	}

	ajRegSubI(seqRegPhylipTop, 1, &tmpstr);
	ajStrToInt(tmpstr, &iseq);
	ajRegSubI(seqRegPhylipTop, 2, &tmpstr);
	ajStrToInt(tmpstr, &len);
	/*ajDebug("first line OK: '%S' iseq: %d len: %d\n",
		seqReadLine, iseq, len);*/

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTableNew(0, ajStrTableCmp,
					       ajStrTableHash);
	phylist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
	ilen = 0;
	while(ok && (jseq < iseq))
	{
	    /* first set - create table */
	    if(!ajRegExec(seqRegPhylipHead, seqReadLine))
	    {
		ajDebug("FAIL (not seqRegPhylipHead): '%S'\n", seqReadLine);
		ajFileBuffResetStore(buff, seqin->Text, &thys->TextPtr);
		AJFREE(seqin->Data);
		return ajFalse;
	    }
	    /* ajDebug("line: '%S'\n", seqReadLine); */
	    AJNEW0(phyitem);
	    ajRegSubI(seqRegPhylipHead, 1, &tmpstr);
	    seqSetName(&phyitem->Name, tmpstr);
	    /* ajDebug("name: '%S' => '%S'\n", tmpstr, phyitem->Name); */
	    phyitem->Weight = 1.0;
	    ajRegPost(seqRegPhylipHead, &seqstr);
	    seqAppend(&phyitem->Seq, seqstr);
	    ilen = ajStrGetLen(phyitem->Seq);
	    if(ilen == len)
		done = ajTrue;
	    else if(ilen > len)
	    {
		/* ajDebug("Phylip format: sequence %S header size %d exceeded\n",
			phyitem->Name, len); */
		ajFileBuffResetStore(buff, seqin->Text, &thys->TextPtr);
		AJFREE(seqin->Data);
		return ajFalse;
	    }
	    ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
	    ajListstrPushApp(phylist, phyitem->Name);
	    if(!jseq)
		maxlen = ilen;
	    else
	    {
		if(ilen != maxlen)
		{
		    /* ajDebug("phylip format length mismatch in header "
			    "iseq: %d jseq: %d ilen: %d maxlen: %d",
			    iseq, jseq, ilen, maxlen);*/
		    ajFileBuffResetStore(buff, seqin->Text, &thys->TextPtr);
		    AJFREE(seqin->Data);
		    ajTableMapDel(phytable, seqMsfTabDel, NULL);
		    ajTableFree(&phytable);
		    if(seqReadPhylipnon(thys, seqin))
			return ajTrue;
		    else {
			ajWarn("phylip format length mismatch in header");
			return ajFalse;
		    }
		}
	    }
	    jseq++;
	    /* ajDebug("first set %d: (%d) '%-20.20S'\n",
	       jseq, ilen, seqReadLine); */

	    if(jseq < iseq)
	    {
		ok = ajFileBuffGetStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		bufflines++;
	    }
	}

	/* ajDebug("Header has %d sequences\n", jseq);*/
	ajListstrTrace(phylist);
	ajTableTrace(phytable);
	ajTableMap(phytable, seqMsfTabList, NULL);

	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(phylist, &phydata->Names[i]);
	    /* ajDebug("list [%d] '%S'\n", i, phydata->Names[i]); */
	}
	ajListstrFree(&phylist);

	if(ilen < len)
	{
	    jseq=0;
	    while(ajFileBuffGetStore(buff, &seqReadLine,
				     seqin->Text, &thys->TextPtr))
	    {				/* now read the rest */
		/* ajDebug("seqReadPhylip line '%S\n", seqReadLine); */
		bufflines++;
		if(seqPhylipReadseq(seqReadLine, phytable, phydata->Names[jseq],
				    len, &ilen, &done))
		{
		    if(!jseq)
			maxlen = ilen;
		    else
		    {
			if(ilen != maxlen)
			{
			   /*  ajDebug("phylip format length mismatch at %d "
				    "(length %d)\n",
				    maxlen, ilen); */
			    ajFileBuffResetStore(buff,
						 seqin->Text, &thys->TextPtr);
			    ajTableMapDel(phytable, seqMsfTabDel, NULL);
			    ajTableFree(&phytable);
			    AJFREE(phydata->Names);
			    AJFREE(seqin->Data);
			    /* ajDebug("File reset, try seqReadPhylipnon\n"); */
			    return seqReadPhylipnon(thys, seqin);
			}
		    }

		    jseq++;
		    if(jseq == iseq) jseq = 0;
		    if(!jseq && done)
		    {
			/* ajDebug("seqReadPhylip set done\n"); */
			break;
		    }
		    done = ajTrue;	/* for end-of-file */
		}
	    }
	    if(!done)
	    {
		ajFileBuffResetStore(buff, seqin->Text, &thys->TextPtr);
		ajTableMapDel(phytable, seqMsfTabDel, NULL);
		ajTableFree(&phytable);
		AJFREE(phydata->Names);
		AJFREE(seqin->Data);
		return seqReadPhylipnon(thys, seqin);
	    }

	    if(jseq)
	    {
		/* ajDebug("Phylip format %d sequences partly read at end\n",
			iseq-jseq); */
		ajFileBuffResetStore(buff, seqin->Text, &thys->TextPtr);
		ajTableMapDel(phytable, seqMsfTabDel, NULL);
		ajTableFree(&phytable);
		AJFREE(phydata->Names);
		AJFREE(seqin->Data);
		return seqReadPhylipnon(thys, seqin);
	    }
	}

	ajTableMap(phytable, seqMsfTabList, NULL);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = bufflines;
	/* ajDebug("PHYLIP format read %d lines\n", bufflines);*/
    }

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    /* ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]); */
    phyitem = ajTableGet(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);
    ajStrDel(&phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);
    ajStrDel(&phyitem->Seq);

    phydata->Count++;
    if(phydata->Count >= phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	/* ajDebug("seqReadPhylip multidone\n"); */
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableMapDel(phytable, seqMsfTabDel, NULL);
	ajTableFree(&phytable);
	AJFREE(phydata->Names);
	AJFREE(phydata);
	seqin->Data = NULL;
    }

    return ajTrue;
}




/* @funcstatic seqPhylipReadseq ***********************************************
**
** Reads sequence from the input line, and appends the sequence data
** to the named sequence in the phytable structure.
**
** @param [r] rdline [const AjPStr] Line from input file.
** @param [r] phytable [const AjPTable] MSF format sequence table.
** @param [r] token [const AjPStr] Name of sequence so it can append
** @param [r] len [ajint] Final length of each sequence (from file header)
** @param [w] ilen [ajint*] Length of each sequence so far
** @param [w] done [AjBool*] ajTrue if sequence was completed
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqPhylipReadseq(const AjPStr rdline, const AjPTable phytable,
			       const AjPStr token,
			       ajint len, ajint* ilen, AjBool* done)
{
    SeqPMsfItem phyitem;

    *done = ajFalse;
    if(!seqRegPhylipSeq2)
	seqRegPhylipSeq2 = ajRegCompC("[^ \t\n\r]");

    if(!ajRegExec(seqRegPhylipSeq2, rdline))
	return ajFalse;

    phyitem = ajTableGet(phytable, token);
    if(!phyitem)
    {
	ajDebug("seqPhylipReadseq failed to find '%S' in phytable\n",
		token);
	return ajFalse;
    }

    seqAppend(&phyitem->Seq, rdline);
    *ilen = ajStrGetLen(phyitem->Seq);

    if(*ilen == len)
	*done = ajTrue;
    else if(*ilen > len)
    {
	ajDebug("Phylip format error, sequence %S length %d exceeded\n",
		token, len);
	return ajFalse;
    }

    ajDebug("seqPhylipReadSeq '%S' len: %d ilen: %d done: %B\n",
	    token, len, *ilen, *done);

    return ajTrue;
}




/* @funcstatic seqReadHennig86 ************************************************
**
** Tries to read input in Hennig86 format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadHennig86(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr seqstr = NULL;
    static AjPStr tmpstr = NULL;
    ajint bufflines = 0;
    AjBool ok       = ajFalse;
    ajint iseq      = 0;
    ajint len       = 0;
    AjPFileBuff buff;
    AjPTable fmttable   = NULL;
    SeqPMsfItem fmtitem = NULL;
    AjPList fmtlist     = NULL;
    SeqPMsfData fmtdata = NULL;
    char *cp;

    ajint i;
    ajint jseq = 0;

    ajDebug("seqReadHennig86 seqin->Data %x\n", seqin->Data);

    buff = seqin->Filebuff;

    if(!seqRegHennigHead)
	seqRegHennigHead = ajRegCompC("[^1-4? \t]");

    if(!seqRegHennigTop)
	seqRegHennigTop = ajRegCompC("^ *([0-9]+) +([0-9]+)");

    if(!seqRegHennigBlank)
	seqRegHennigBlank = ajRegCompC("^[ \t\n\r]*$");

    if(!seqRegHennigSeq)
	seqRegHennigSeq = ajRegCompC("^([^ \t\n\r]+)");

    if(!seqin->Data)
    {
	/* start: load in file */
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;
	bufflines++;

	ajDebug("first line:\n'%S'\n", seqReadLine);

	if(!ajStrPrefixC(seqReadLine, "xread"))
	{
	    /* first line test */
	    ajFileBuffReset(buff);
	    return ajFalse;
	}

	ajDebug("first line OK: '%S'\n", seqReadLine);

	/* skip title line */
	for(i=0; i<2; i++)
	{
	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;

	    if(!ok)
	    {
		ajDebug("FAIL (bad header)\n");
		ajFileBuffReset(buff);
		return ajFalse;
	    }
	}

	if(!ajRegExec(seqRegHennigTop, seqReadLine))	/* first line test */
	    return ajFalse;

	ajRegSubI(seqRegHennigTop, 1, &tmpstr);
	ajStrToInt(tmpstr, &iseq);
	ajRegSubI(seqRegHennigTop, 2, &tmpstr);
	ajStrToInt(tmpstr, &len);
	ajDebug("first line OK: '%S' iseq: %d len: %d\n",
		seqReadLine, iseq, len);

	seqin->Data = AJNEW0(fmtdata);
	fmtdata->Table = fmttable = ajTableNew(0,ajStrTableCmp,ajStrTableHash);
	fmtlist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
	while(ok && (jseq < iseq))
	{				/* first set - create table */
	    if(!ajRegExec(seqRegHennigHead, seqReadLine))
	    {
		ajDebug("FAIL (not seqRegHennigHead): '%S'\n", seqReadLine);
		return ajFalse;
	    }
	    AJNEW0(fmtitem);
	    ajStrAssignS(&fmtitem->Name, seqReadLine);
	    fmtitem->Weight = 1.0;
	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;
	    while(ok && ajRegExec(seqRegHennigSeq, seqReadLine))
	    {
		ajRegPost(seqRegHennigSeq, &seqstr);
		for(cp = ajStrGetuniquePtr(&seqstr); cp; cp++)
		    switch(*cp)
		    {
		    case 0: *cp = 'A';break;
		    case 1: *cp = 'T';break;
		    case 2: *cp = 'G';break;
		    case 3: *cp = 'C';break;
		    default: *cp = '.';break;
		    }

		seqAppend(&fmtitem->Seq, seqstr);
	    }

	    ajTablePut(fmttable, ajStrNewS(fmtitem->Name), fmtitem);
	    ajListstrPushApp(fmtlist, fmtitem->Name);
	    jseq++;
	    ajDebug("first set %d: '%S'\n", jseq, seqReadLine);

	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;
	}

	ajDebug("Header has %d sequences\n", iseq);
	ajListstrTrace(fmtlist);
	ajTableTrace(fmttable);
	ajTableMap(fmttable, seqMsfTabList, NULL);

	fmtdata->Names = AJCALLOC(iseq, sizeof(*fmtdata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(fmtlist, &fmtdata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, fmtdata->Names[i]);
	}
	ajListstrFree(&fmtlist);

	while(ajFileBuffGetStore(buff, &seqReadLine,
				 seqin->Text, &thys->TextPtr))
	{				/* now read the rest */
	    bufflines++;
	    seqHennig86Readseq(seqReadLine, fmttable);
	}

	ajTableMap(fmttable, seqMsfTabList, NULL);
	fmtdata->Nseq = iseq;
	fmtdata->Count = 0;
	fmtdata->Bufflines = bufflines;
	ajDebug("... format read %d lines\n", bufflines);
    }

    /* processing entries */

    fmtdata = seqin->Data;
    fmttable = fmtdata->Table;
    if(fmtdata->Count >=fmtdata->Nseq)
    {					/* all done */
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableMapDel(fmttable, seqMsfTabDel, NULL);
	ajTableFree(&fmttable);
	AJFREE(fmtdata->Names);
	AJFREE(fmtdata);
	seqin->Data = NULL;
	return ajFalse;
    }
    i = fmtdata->Count;
    ajDebug("returning [%d] '%S'\n", i, fmtdata->Names[i]);
    fmtitem = ajTableGet(fmttable, fmtdata->Names[i]);
    ajStrAssignS(&thys->Name, fmtdata->Names[i]);
    ajStrDel(&fmtdata->Names[i]);

    thys->Weight = fmtitem->Weight;
    ajStrAssignS(&thys->Seq, fmtitem->Seq);
    ajStrDel(&fmtitem->Seq);

    fmtdata->Count++;

    return ajTrue;
}




/* @funcstatic seqHennig86Readseq *********************************************
**
** Reads sequence name from first token on the input line, and appends
** the sequence data to that sequence in the fmttable structure.
**
** @param [r] rdline [const AjPStr] Line from input file.
** @param [r] msftable [const AjPTable] MSF format sequence table.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqHennig86Readseq(const AjPStr rdline, const AjPTable msftable)
{
    static AjPRegexp seqexp = NULL;
    SeqPMsfItem msfitem;
    static AjPStr token  = NULL;
    static AjPStr seqstr = NULL;

    if(!seqexp)
	seqexp = ajRegCompC("^[^ \t\n\r]+"); /* must be at start of line */

    if(!ajRegExec(seqexp, rdline))
	return ajFalse;

    ajRegSubI(seqexp, 0, &token);
    msfitem = ajTableGet(msftable, token);
    if(!msfitem)
    {
	ajStrDel(&token);
	return ajFalse;
    }

    ajRegPost(seqexp, &seqstr);
    seqAppend(&msfitem->Seq, seqstr);

    return ajTrue;
}




/* @funcstatic seqReadTreecon *************************************************
**
** Tries to read input in Treecon format.
**
** Treecon is a windows program for tree drawing.
**
** Van de Peer, Y., De Wachter, R. (1994)
** TREECON for Windows: a software package for the construction and
** drawing of evolutionary trees for the Microsoft Windows environment.
** Comput. Applic. Biosci. 10, 569-570.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadTreecon(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr tmpstr = NULL;
    ajint bufflines = 0;
    AjBool ok       = ajFalse;
    ajint len       = 0;
    ajint ilen      = 0;
    ajint iseq;
    ajint i;
    AjPFileBuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    AjPList phylist          = NULL;
    SeqPMsfData phydata      = NULL;
    static AjPRegexp topexp  = NULL;
    static AjPRegexp seqexp  = NULL;

    buff = seqin->Filebuff;

    if(!topexp)
	topexp = ajRegCompC("^ *([0-9]+)");

    if(!seqexp)
	seqexp = ajRegCompC("^[ \t\n\r]*$");

    if(!seqin->Data)			/* first time - read the data */
    {
	iseq = 0;
	seqin->multidone = ajFalse;
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;
	bufflines++;

	if(!ajRegExec(topexp, seqReadLine))
	{				/* first line test */
	    ajFileBuffReset(buff);
	    return ajFalse;
	}

	ajRegSubI(topexp, 1, &tmpstr);
	ajStrToInt(tmpstr, &len);

	ajDebug("first line OK: '%S' len: %d\n",
		seqReadLine, len);

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTableNew(0, ajStrTableCmp,
					       ajStrTableHash);
	phylist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
	ilen = -1;
	while (ok)
	{
	   if (ilen < 0)
	   {
	       ajStrRemoveWhite(&seqReadLine);
	       if (!ajStrGetLen(seqReadLine))	/* empty line after a sequence */
	       {
		   ok = ajFileBuffGetStore(buff, &seqReadLine,
					   seqin->Text, &thys->TextPtr);
		   continue;
	       }
	       AJNEW0(phyitem);
	       phyitem->Weight = 1.0;
	       seqSetName(&phyitem->Name, seqReadLine);
	       ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
	       ajListstrPushApp(phylist, phyitem->Name);
	       iseq++;
	       ilen = 0;
	   }
	   else
	   {
	       ajStrRemoveWhiteExcess(&seqReadLine);
	       ilen += ajStrGetLen(seqReadLine);
	       seqAppend(&phyitem->Seq, seqReadLine);
	       
	       if (ilen > len)
	       {
		   ajDebug("Treecon format: '%S' too long, read %d/%d\n",
		    phyitem->Name, ilen, len);
		ajFileBuffReset(buff);
		AJFREE(seqin->Data);
		return ajFalse;
	       }
	       if (ilen == len)
	       {
		   ilen = -1;
	       }
	   }

	   ok = ajFileBuffGetStore(buff, &seqReadLine,
				   seqin->Text, &thys->TextPtr);
	}
	if (ilen >= 0)
	{
	    ajDebug("Treecon format: unfinished sequence '%S' read %d/%d\n",
		    phyitem->Name, ilen, len);
	    AJFREE(seqin->Data);
	    return ajFalse;
	}

	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(phylist, &phydata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, phydata->Names[i]);
	}
	ajListstrFree(&phylist);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = bufflines;
	ajDebug("Treecon format read %d lines\n", bufflines);


    }

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableGet(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);
    ajStrDel(&phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);
    ajStrDel(&phyitem->Seq);

    phydata->Count++;
    if(phydata->Count >=phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadTreecon multidone\n");
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableMapDel(phytable, seqMsfTabDel, NULL);
	ajTableFree(&phytable);
	AJFREE(phydata->Names);
	AJFREE(phydata);
	seqin->Data = NULL;
    }

    return ajTrue;
}




/* @funcstatic seqReadJackknifer **********************************************
**
** Tries to read input in Jackknifer format.
**
** The Jackknifer program by Farris is a parsimony program that also
** implements the jackknife method to test the reliability of branches. 
** The format is similar to the MEGA format.
**
** On the first line a title/description is placed in between single quotes.
** The alignment can be written in sequential or interleaved format,
** but the sequence names have to be placed between brackets.
** Also no blanks are allowed in the names.
** They should be replaced by underscores ( _ ).
** The file is ended by a semicolon.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadJackknifer(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr tmpstr = NULL;
    static AjPStr tmpname = NULL;
    ajint bufflines = 0;
    AjBool ok       = ajFalse;
    ajint iseq;
    ajint i;
    AjPFileBuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    AjPList phylist          = NULL;
    SeqPMsfData phydata      = NULL;
    static AjPRegexp topexp  = NULL;
    static AjPRegexp seqexp  = NULL;

    buff = seqin->Filebuff;

    if(!topexp)
	topexp = ajRegCompC("^'(.*)'\\s*$");

    if(!seqexp)
	seqexp = ajRegCompC("^[(]([^)]+)(.*)$");

    if(!seqin->Data)			/* first time - read the data */
    {
	iseq = 0;
	seqin->multidone = ajFalse;
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;
	bufflines++;

	if(!ajRegExec(topexp, seqReadLine))
	{				/* first line test */
	    ajFileBuffReset(buff);
	    return ajFalse;
	}
	ajDebug("JackKnifer format: First line ok '%S'\n", seqReadLine);

	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTableNew(0, ajStrTableCmp,
					       ajStrTableHash);
	phylist = ajListstrNew();
	seqin->Filecount = 0;

	while (ok)
	{
	    if (!ajStrGetLen(seqReadLine))	/* empty line after a sequence */
	    {
		ok = ajFileBuffGetStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		continue;
	    }
	    if (ajStrPrefixC(seqReadLine, ";"))
		break;			/* done */
	    if (ajStrPrefixC(seqReadLine, "("))
	    {
		if (!ajRegExec(seqexp, seqReadLine))
		{
		    ajDebug("JackKnifer format: bad (id) line\n");
		    AJFREE(seqin->Data);
		    return ajFalse;
		}

		ajRegSubI(seqexp, 1, &tmpstr);
		seqSetName(&tmpname, tmpstr);
		phyitem = ajTableGet(phytable, tmpname);
		if (!phyitem)
		{
		    ajDebug("JackKnifer format: new (id) '%S'\n", tmpname);
		    AJNEW0(phyitem);
		    phyitem->Weight = 1.0;
		    ajStrAssignS(&phyitem->Name,tmpname);
		    ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
		    ajListstrPushApp(phylist, phyitem->Name);
		    iseq++;
		}
		else
		{
		    ajDebug("JackKnifer format: More for (id) '%S'\n",
			    tmpname);

		}
		ajRegSubI(seqexp, 2, &tmpstr);
		ajStrAssignS(&seqReadLine, tmpstr);
	    }
	    seqAppend(&phyitem->Seq, seqReadLine);

	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				   seqin->Text, &thys->TextPtr);
	}

	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(phylist, &phydata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, phydata->Names[i]);
	}
	ajListstrFree(&phylist);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = bufflines;
	ajDebug("JackKnifer format read %d lines\n", bufflines);
    }

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableGet(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);
    ajStrDel(&phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);
    ajStrDel(&phyitem->Seq);

    phydata->Count++;
    if(phydata->Count >=phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadJackKnifer multidone\n");
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableMapDel(phytable, seqMsfTabDel, NULL);
	ajTableFree(&phytable);
	AJFREE(phydata->Names);
	AJFREE(phydata);
	seqin->Data = NULL;
    }

    return ajTrue;
}




/* @funcstatic seqReadNexus ***************************************************
**
** Tries to read input in Nexus format.
**
** Nexus files contain many things.
** All Nexus files begin with a #NEXUS line
** Data is in begin ... end blocks
** Sequence data is in a "begin character" block
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadNexus(AjPSeq thys, AjPSeqin seqin)
{
    ajint bufflines = 0;
    AjBool ok       = ajFalse;
    ajint iseq;
    ajint i;
    AjPFileBuff buff;
    AjPStr* seqs = NULL;
    AjPNexus nexus = NULL;

    SeqPMsfData phydata      = NULL;

    buff = seqin->Filebuff;

    if(!seqin->Data)			/* first time - read the data */
    {
	iseq = 0;
	seqin->multidone = ajFalse;

	ajFileBuffBuff(buff);

	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	ajDebug("Nexus format: Testing first line '%S'\n", seqReadLine);
	if(!ok)
	    return ajFalse;
	bufflines++;

	if(!ajStrPrefixCaseC(seqReadLine, "#NEXUS"))
	{				/* first line test */
	    ajFileBuffReset(buff);
	    return ajFalse;
	}
	ajDebug("Nexus format: First line ok '%S'\n", seqReadLine);

	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixCaseC(seqReadLine, "#NEXUS"))
	{
	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

	ajFileBuffReset(buff);

	AJNEW0(phydata);
	phydata->Nexus = ajNexusParse(buff);
	if (!phydata->Nexus)
	{
	    ajFileBuffReset(buff);
	    ajDebug("Failed to parse in nexus format\n");
	    return ajFalse;
	}
	phydata->Count = 0;
	phydata->Nseq = ajNexusGetNtaxa(phydata->Nexus);
	/* GetTaxa may fail if names are only defined in the sequences */
	phydata->Names = ajNexusGetTaxa(phydata->Nexus);
	seqin->Data = phydata;
	ajDebug("Nexus parsed %d sequences\n", phydata->Nseq);
    }

    phydata = seqin->Data;
    nexus = phydata->Nexus;

    i = phydata->Count;

    seqs = ajNexusGetSequences(nexus);
    if (!seqs)
    {
	ajNexusDel(&phydata->Nexus);
	AJFREE(phydata);
	seqin->Data = NULL;
	return ajFalse;
    }

    if (!phydata->Names)		/* finally set from the sequences */
	phydata->Names = ajNexusGetTaxa(phydata->Nexus);
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);

    ajStrAssignS(&thys->Name, phydata->Names[i]);

    thys->Weight = 1.0;
    ajStrAssignS(&thys->Seq, seqs[i]);

    phydata->Count++;
    if(phydata->Count >= phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadNexus multidone\n");
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableFree(&phydata->Table);		/* unused */
	phydata->Names = NULL;
	ajNexusDel(&phydata->Nexus);
	AJFREE(phydata);
	seqin->Data = NULL;
    }

    return ajTrue;
}


/* @funcstatic seqReadMega ****************************************************
**
** Tries to read input in Mega non-interleaved format.
**
** The Molecular Evolutionary Genetic Analysis program by
** Kumar, Tamura & Nei is a tree construction program
** based on distance- and parsimony methods.
**
** http://evolgen.biol.metro-u.ac.jp/MEGA/manual/DataFormat.html
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadMega(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr tmpstr = NULL;
    static AjPStr tmpname = NULL;
    static AjPStr prestr = NULL;
    static AjPStr poststr = NULL;
    ajint bufflines = 0;
    AjBool ok       = ajFalse;
    ajint iseq;
    ajint i;
    AjPFileBuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    AjPList phylist          = NULL;
    SeqPMsfData phydata      = NULL;
    static AjPRegexp featexp  = NULL;
    static AjPRegexp seqexp  = NULL;

    buff = seqin->Filebuff;

    if(!featexp)
	featexp = ajRegCompC("^(.*)\"[^\"]*\"(.*)$");

    if(!seqexp)
	seqexp = ajRegCompC("^#([^ \t\n\r]+)(.*)$");

    if(!seqin->Data)			/* first time - read the data */
    {
	iseq = 0;
	seqin->multidone = ajFalse;
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	ajDebug("Mega format: Testing first line '%S'\n", seqReadLine);
	if(!ok)
	    return ajFalse;
	bufflines++;

	if(!ajStrMatchCaseC(seqReadLine, "#MEGA\n"))
	{				/* first line test */
	    ajFileBuffReset(buff);
	    return ajFalse;
	}
	ajDebug("Mega format: First line ok '%S'\n", seqReadLine);

	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;
	bufflines++;

	if(!ajStrPrefixCaseC(seqReadLine, "TITLE"))
	{				/* first line test */
	    ajFileBuffReset(buff);
	    return ajFalse;
	}
	ajDebug("Mega format: Second line ok '%S'\n", seqReadLine);

	while(ok && !ajStrPrefixC(seqReadLine, "#"))
	{				/* skip comments in header */
	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}


	/*
        ** read through looking for #id
	** Some day we could stop at #mega and read multiple files
	*/

	
	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTableNew(0, ajStrTableCmp,
					       ajStrTableHash);
	phylist = ajListstrNew();
	seqin->Filecount = 0;

	while (ok)
	{
	    if (!ajStrGetLen(seqReadLine))	/* empty line after a sequence */
	    {
		ok = ajFileBuffGetStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		continue;
	    }
	    if (ajStrPrefixC(seqReadLine, "#"))
	    {
		if (!ajRegExec(seqexp, seqReadLine))
		{
		    ajDebug("Mega format: bad #id line\n");
		    AJFREE(seqin->Data);
		    return ajFalse;
		}

		ajRegSubI(seqexp, 1, &tmpstr);
		seqSetName(&tmpname, tmpstr);
		phyitem = ajTableGet(phytable, tmpname);
		if (!phyitem)
		{
		    ajDebug("Mega format: new #id '%S'\n", tmpname);
		    AJNEW0(phyitem);
		    phyitem->Weight = 1.0;
		    ajStrAssignS(&phyitem->Name,tmpname);
		    ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
		    ajListstrPushApp(phylist, phyitem->Name);
		    iseq++;
		}
		else
		{
		    ajDebug("Mega format: More for #id '%S'\n", tmpname);

		}
		ajRegSubI(seqexp, 2, &tmpstr);
		ajStrAssignS(&seqReadLine, tmpstr);
	    }
	    while (ajRegExec(featexp, seqReadLine))
	    {
		ajDebug("Quotes found: '%S'\n", seqReadLine);
		ajRegSubI(featexp, 1, &prestr);
		ajRegSubI(featexp, 2, &poststr);
		ajStrAssignS(&seqReadLine, prestr);
		ajStrAppendS(&seqReadLine, poststr);
		ajDebug("Quotes removed: '%S'\n", seqReadLine);
	    }
	    seqAppend(&phyitem->Seq, seqReadLine);

	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				   seqin->Text, &thys->TextPtr);
	}

	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(phylist, &phydata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, phydata->Names[i]);
	}
	ajListstrFree(&phylist);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = bufflines;
	ajDebug("Mega format read %d lines\n", bufflines);
    }

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableGet(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);
    ajStrDel(&phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);
    ajStrDel(&phyitem->Seq);

    phydata->Count++;
    if(phydata->Count >=phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadMega multidone\n");
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableMapDel(phytable, seqMsfTabDel, NULL);
	ajTableFree(&phytable);
	AJFREE(phydata->Names);
	AJFREE(phydata);
	seqin->Data = NULL;
    }

    return ajTrue;
}




/* @funcstatic seqReadCodata **************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using CODATA format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadCodata(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStrTok handle = NULL;
    static AjPStr token     = NULL;
    ajint bufflines         = 0;
    AjPFileBuff buff;
    AjBool ok = ajTrue;


    buff = seqin->Filebuff;

    if(!ajFileBuffGetStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    bufflines++;

    ajDebug("first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "ENTRY "))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }
    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    ajStrTokenNextParse(&handle, &token);	/* 'ENTRY' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName (&thys->Name, token);

    ok = ajFileBuffGetStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);

    while(ok && !ajStrPrefixC(seqReadLine, "SEQUENCE"))
    {
	bufflines++;
	if(ajStrPrefixC(seqReadLine, "ACCESSION "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'AC' */
	    ajStrTokenNextParse(&handle, &token); /* accnum */
	    seqAccSave(thys, token);
	}

	if(ajStrPrefixC(seqReadLine, "TITLE "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		bufflines++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");
		ajStrTokenNextParseC(&handle, "\n\r", &token);
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
		ok = ajFileBuffGetStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
	    }
	}
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ok = ajFileBuffGetStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    while(ok && !ajStrPrefixC(seqReadLine, "///"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }
    ajFileBuffClear(buff, 0);

    ajStrTokenReset(&handle);

    return ajTrue;
}




/* @funcstatic seqReadAcedb ***************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using ACEDB format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadAcedb(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    ajint bufflines         = 0;
    AjPFileBuff buff;
    AjBool ok = ajTrue;

    ajDebug("seqReadAcedb\n");

    buff = seqin->Filebuff;

    do
    {
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
    } while(ok &&
	    (ajStrPrefixC(seqReadLine, "//") || ajStrPrefixC(seqReadLine, "\n")));

    if(!ok)
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }
    ajDebug("first line:\n'%S'\n", seqReadLine);


    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    ajStrTokenNextParseC(&handle, " \t", &token); /* 'DNA' or 'Peptide'*/
    ajDebug("Token 1 '%S'\n", token);

    if(ajStrMatchCaseC(token, "Peptide"))
    {
	ajDebug("Protein\n");
	ajSeqSetProt(thys);
    }
    else if(ajStrMatchCaseC(token, "DNA"))
    {
	ajDebug("DNA\n");
	ajSeqSetNuc(thys);
    }
    else
    {
	ajDebug("unknown - failed\n");
	ajFileBuffReset(buff);
	ajStrTokenDel(&handle);
	ajStrDel(&token);
	return ajFalse;
    }

    ajStrTokenNextParseC(&handle, " \t\"", &token); /* : */
    if(!ajStrMatchC(token, ":"))
    {
	ajFileBuffReset(buff);
	ajStrTokenDel(&handle);
	ajStrDel(&token);
	return ajFalse;
    }

    ajStrTokenNextParseC(&handle, "\"", &token);	/* name */
    if(!ajStrGetLen(token))
    {
	ajFileBuffReset(buff);
	ajStrTokenDel(&handle);
	ajStrDel(&token);
	return ajFalse;
    }

    seqSetName(&thys->Name, token);

    /* OK, we have the name. Now look for the sequence */

    ok = ajFileBuffGetStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    while(ok && !ajStrPrefixC(seqReadLine,"\n"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajFileBuffClear(buff, 0);

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return ajTrue;
}


/* @funcstatic seqReadFitch *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using fitch format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadFitch(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr token     = NULL;
    AjPFileBuff buff;
    AjBool ok = ajTrue;
    ajint ilen = 0;

   if (!seqRegFitchHead)
	seqRegFitchHead = ajRegCompC("^(\\S+),\\s+(\\d+)\\s+bases\n");

    buff = seqin->Filebuff;

    ok = ajFileBuffGetStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if (!ajRegExec(seqRegFitchHead, seqReadLine))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }

    ajRegSubI(seqRegFitchHead, 1, &token);
    seqSetName(&thys->Name, token);

    ajRegSubI(seqRegFitchHead, 2, &token);
    ajStrToInt(token, &ilen);

    ok = ajFileBuffGetStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    while (ok && (ajStrGetLen(thys->Seq) < ilen))
    {
	seqAppend(&thys->Seq, seqReadLine);
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajStrDel(&token);
    ajFileBuffClear(buff, 0);
    return ajTrue;
}

/* @funcstatic seqReadMase ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using mase format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadMase(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr token     = NULL;
    AjPStr des     = NULL;
    AjPFileBuff buff;
    AjBool ok = ajTrue;

    if (!seqRegMaseHead)
	seqRegMaseHead = ajRegCompC("^(;+)");

    buff = seqin->Filebuff;

    ok = ajFileBuffGetStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if (!ajRegExec(seqRegMaseHead, seqReadLine))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }

    while (ok && ajRegExec(seqRegMaseHead, seqReadLine))
    {
	if (ajRegLenI(seqRegMaseHead, 1) == 1)
	{
	    ajRegPost(seqRegMaseHead, &token);
	    if (des)
		ajStrAppendK(&des, ' ');
	    ajStrAppendS(&des, token);
	}
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajStrRemoveWhite(&seqReadLine);
    seqSetName(&thys->Name, seqReadLine);
    ajStrRemoveWhite(&des);
    ajSeqAssignDescS(thys, des);

    ok = ajFileBuffGetStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    while (ok && !ajRegExec(seqRegMaseHead, seqReadLine))
    {
	seqAppend(&thys->Seq, seqReadLine);
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajStrDel(&token);
    ajStrDel(&des);
    if (ok)
	ajFileBuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    else
	ajFileBuffClear(buff, 0);

    return ajTrue;
}

/* @funcstatic seqReadStrider *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using DNA strider format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadStrider(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStrTok handle = NULL;
    static AjPStr token     = NULL;
    ajint bufflines         = 0;
    AjPFileBuff buff;
    AjBool ok = ajTrue;

    buff = seqin->Filebuff;

    do
    {
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(ajStrPrefixC(seqReadLine, "; DNA sequence"))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\t,\n");
	    ajStrTokenNextParse(&handle, &token); /* 'DNA' */
	    ajStrTokenNextParse(&handle, &token); /* sequence */
	    ajStrTokenNextParse(&handle, &token); /* entry name */
	}
	bufflines++;
    } while(ok && ajStrPrefixC(seqReadLine, ";"));

    if(!ok || !ajStrGetLen(token))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }

    seqSetName(&thys->Name, token);

    /* OK, we have the name. Now look for the sequence */

    while(ok && !ajStrPrefixC(seqReadLine, "//"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajFileBuffClear(buff, 0);

    ajStrTokenReset(&handle);

    return ajTrue;
}




/* @funcstatic seqReadMsf *****************************************************
**
** Tries to read input in MSF format. If successful, can repeat for the
** next call to return the second, third, ... sequence from the same file.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadMsf(AjPSeq thys, AjPSeqin seqin)
{
    ajint bufflines      = 0;
    ajint len;
    AjBool ok  = ajFalse;
    ajint iseq = 0;

    AjPFileBuff buff;
    AjPTable msftable   = NULL;
    SeqPMsfItem msfitem = NULL;
    AjPList msflist     = NULL;
    SeqPMsfData msfdata = NULL;

    ajint i;

    ajDebug("seqReadMsf seqin->Data %x\n", seqin->Data);

    buff = seqin->Filebuff;

    if(!seqin->Data)
    {
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;
	bufflines++;

	if(ajStrPrefixC(seqReadLine, "!!"))
	{
	    if(ajStrPrefixC(seqReadLine, "!!AA_MULTIPLE_ALIGNMENT"))
		ajSeqSetProt(thys);

	    if(ajStrPrefixC(seqReadLine, "!!NA_MULTIPLE_ALIGNMENT"))
		ajSeqSetNuc(thys);
	}

	if(!seqGcgMsfDots(thys, seqin, &seqReadLine, seqMaxGcglines, &len))
	{
	    ajDebug("seqGcgMsfDots failed\n");
	    ajFileBuffReset(buff);
	    return ajFalse;
	}

	seqin->Data = AJNEW0(msfdata);
	msfdata->Table = msftable = ajTableNew(0, ajStrTableCmp,
					       ajStrTableHash);
	msflist = ajListstrNew();
	seqin->Filecount = 0;
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;

	    if(seqGcgMsfHeader(seqReadLine, &msfitem))
	    {
		ajTablePut(msftable, ajStrNewS(msfitem->Name), msfitem);
		ajListstrPushApp(msflist, msfitem->Name);
		iseq++;
	    }
	}

	ajDebug("Header has %d sequences\n", iseq);
	ajListstrTrace(msflist);
	ajTableTrace(msftable);
	ajTableMap(msftable, seqMsfTabList, NULL);

	msfdata->Names = AJCALLOC(iseq, sizeof(*msfdata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(msflist, &msfdata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, msfdata->Names[i]);
	}
	ajListstrFree(&msflist);
	while(ajFileBuffGetStore(buff, &seqReadLine,
				 seqin->Text, &thys->TextPtr))
	{
	    bufflines++;
	    seqGcgMsfReadseq(seqReadLine, msftable);
	}

	ajTableMap(msftable, seqMsfTabList, NULL);
	msfdata->Nseq = iseq;
	msfdata->Count = 0;
	msfdata->Bufflines = bufflines;
	ajDebug("MSF format read %d lines\n", bufflines);
    }

    msfdata = seqin->Data;
    msftable = msfdata->Table;
    if(msfdata->Count >= msfdata->Nseq)
    {
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableMapDel(msftable, seqMsfTabDel, NULL);
	ajTableFree(&msftable);
	AJFREE(msfdata->Names);
	AJFREE(msfdata);
	seqin->Data = NULL;
	return ajFalse;
    }
    i = msfdata->Count;
    ajDebug("returning [%d] '%S'\n", i, msfdata->Names[i]);
    msfitem = ajTableGet(msftable, msfdata->Names[i]);
    ajStrAssignS(&thys->Name, msfdata->Names[i]);
    ajStrDel(&msfdata->Names[i]);

    thys->Weight = msfitem->Weight;
    ajStrAssignS(&thys->Seq, msfitem->Seq);
    ajStrDel(&msfitem->Seq);

    msfdata->Count++;

    return ajTrue;
}




/* @funcstatic seqGcgMsfReadseq ***********************************************
**
** Reads sequence name from first token on the input line, and appends
** the sequence data to that sequence in the msftable structure.
**
** @param [r] rdline [const AjPStr] Line from input file.
** @param [r] msftable [const AjPTable] MSF format sequence table.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqGcgMsfReadseq(const AjPStr rdline, const AjPTable msftable)
{
    SeqPMsfItem msfitem;
    AjPStr token     = NULL;
    AjPStr seqstr    = NULL;
    AjBool status;

    status = ajStrExtractWord(rdline, &seqstr, &token);
    if(!status)
    {
	ajStrDel(&token);
	ajStrDel(&seqstr);
	return ajFalse;
    }

    ajDebug("seqGcgMsfReadseq '%S' '%S'\n", token, seqstr);

    msfitem = ajTableGet(msftable, token);
    if(!msfitem)
    {
	ajStrDel(&token);
	ajStrDel(&seqstr);
	return ajFalse;
    }

    seqAppend(&msfitem->Seq, seqstr);

    ajStrDel(&token);
    ajStrDel(&seqstr);

    return ajTrue;
}




/* @funcstatic seqMsfTabList **************************************************
**
** Writes a debug report of the contents of an MSF table.
**
** @param [r] key [const void*] Standard argument, key from current table item
**                              which is a string for MSF internal tables.
** @param [r] value [void**] Standard argument, data from current table item,
**                           converted to an MSF internal table item.
** @param [r] cl [void*] Standard argument, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void seqMsfTabList(const void* key, void** value, void* cl)
{
    SeqPMsfItem msfitem;

    msfitem = (SeqPMsfItem) *value;

    ajDebug("key '%S' Name '%S' Seqlen %d\n",
	    key, msfitem->Name, ajStrGetLen(msfitem->Seq));

    return;
}




/* @funcstatic seqMsfTabDel ***************************************************
**
** Deletes entries from the MSF internal table. Called for each entry in turn.
**
** @param [d] key [const void**] Standard argument, table key.
** @param [d] value [void**] Standard argument, table data item.
** @param [r] cl [void*] Standard argument, usually NULL
** @return [void]
** @@
******************************************************************************/

static void seqMsfTabDel(const void** key, void** value, void* cl)
{
    SeqPMsfItem msfitem;
    AjPStr keystr;

    keystr = (AjPStr) *key;
    msfitem = (SeqPMsfItem) *value;

    ajStrDel(&keystr);
    AJFREE(msfitem);

    *key = NULL;
    *value = NULL;

    return;
}




/* @funcstatic seqReadSwiss ***************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using SWISS format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadSwiss(AjPSeq thys, AjPSeqin seqin)
{
    static AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    ajint bufflines         = 0;
    AjBool ok;
    AjPFileBuff buff;
    static AjPStr tmpstr = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr liststr;			/* for lists, do not delete */


    buff = seqin->Filebuff;

    if(!seqFtFmtSwiss)
	ajStrAssignC(&seqFtFmtSwiss, "swissprot");

    if(!ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr))
	return ajFalse;

    bufflines++;

    /* for GCG formatted databases */

    while(ajStrPrefixC(seqReadLine, "WP "))
    {
	if(!ajFileBuffGetStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;
	bufflines++;
    }

    /* extra blank lines */

    while(ajStrIsWhite(seqReadLine))
    {
	if(!ajFileBuffGetStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;
	bufflines++;
    }

    ajDebug("seqReadSwiss first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "ID   "))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }
    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    ajStrTokenNextParse(&handle, &token);	/* 'ID' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName(&thys->Name, token);

    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    while(ok && !ajStrPrefixC(seqReadLine, "SQ   "))
    {
	bufflines++;

	/* check for Staden Experiment format instead */
	if(ajStrPrefixC(seqReadLine, "EN   ") ||
	   ajStrPrefixC(seqReadLine, "TN   ") ||
	   ajStrPrefixC(seqReadLine, "EX   ") )
	{
	    ajFileBuffReset(buff);
	    return ajFalse;;
	}

	if(ajStrPrefixC(seqReadLine, "AC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'AC' */
	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	if(ajStrPrefixC(seqReadLine, "DE   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    if(ajStrGetLen(thys->Desc))
	    {
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
	    }
	    else
		ajStrAssignS(&thys->Desc, token);
	}

	if(ajStrPrefixC(seqReadLine, "KW   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'KW' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		ajListstrPushApp(thys->Keylist, liststr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "OS   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OS' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr);
		ajStrDel(&tmpstr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "OC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OC' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr);
		ajStrDel(&tmpstr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "FT   "))
	    if(seqinUfoLocal(seqin))
	    {
		if(!dofeat)
		{
		    dofeat = ajTrue;
		    ajFeattabInDel(&seqin->Ftquery);
		    seqin->Ftquery = ajFeattabInNewSS(seqFtFmtSwiss,
						      thys->Name, "N");
		    ajDebug("seqin->Ftquery ftfile %x\n",
			    seqin->Ftquery->Handle);
		}
		ajFileBuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		/* ajDebug("SWISS FEAT saved line:\n%S", seqReadLine); */
	    }

	ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    }

    if(dofeat)
    {
 	ajDebug("EMBL FEAT TabIn %x\n", seqin->Ftquery);
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    if(ajStrGetLen(seqin->Inseq))
    {
	/* we have a sequence to use */
	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	{
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);
	    ajFmtPrintAppS(&thys->TextPtr, "//\n");
	}
    }
    else
    {
	/* read the sequence and terminator */
	ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    bufflines++;
	    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	}
    }

    ajFileBuffClear(buff, 0);

    ajStrDel(&token);

    ajStrTokenReset(&handle);

    return ajTrue;
}




/* @funcstatic seqReadEmbl ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using EMBL format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadEmbl(AjPSeq thys, AjPSeqin seqin)
{

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    ajint bufflines         = 0;
    AjBool ok;
    AjPFileBuff buff;
    AjPStr tmpstr = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr liststr;			/* for lists, do not delete */

    buff = seqin->Filebuff;

    if(!seqFtFmtEmbl)
	ajStrAssignC(&seqFtFmtEmbl, "embl");

    if(!ajFileBuffGetStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    bufflines++;

    /* for GCG formatted databases */

    while(ajStrPrefixC(seqReadLine, "WP "))
    {
	if(!ajFileBuffGetStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;
	bufflines++;
    }

    /* extra blank lines */

    while(ajStrIsWhite(seqReadLine))
    {
	if(!ajFileBuffGetStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;
	bufflines++;
    }

    ajDebug("seqReadEmbl first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "ID   "))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }

    if(seqin->Text)
	ajStrAssignC(&thys->TextPtr,ajStrGetPtr(seqReadLine));

    ajDebug("seqReadEmbl ID line found\n");
    ajStrTokenAssignC(&handle, seqReadLine, " ;\t\n\r");
    ajStrTokenNextParse(&handle, &token);	/* 'ID' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName(&thys->Name, token);

    ajStrTokenNextParse(&handle, &token);	/* entry name */
    if(ajStrMatchC(token, "SV"))
    {
	ajStrTokenNextParse(&handle, &token);	/* SV */
	ajStrInsertK(&token, 0, '.');
	ajStrInsertS(&token, 0, thys->Name);
	seqSvSave(thys, token);
    }

    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    while(ok && !ajStrPrefixC(seqReadLine, "SQ   "))
    {
	bufflines++;

	/* check for Staden Experiment format instead */
	if(ajStrPrefixC(seqReadLine, "EN   ") ||
	   ajStrPrefixC(seqReadLine, "TN   ") ||
	   ajStrPrefixC(seqReadLine, "EX   ") )
	{
	    ajFileBuffReset(buff);
	    return ajFalse;;
	}

	if(ajStrPrefixC(seqReadLine, "AC   ") ||
	   ajStrPrefixC(seqReadLine, "PA   ") ) /* emblcds database format */
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'AC' */
	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	if(ajStrPrefixC(seqReadLine, "SV   ") ||
	   ajStrPrefixC(seqReadLine, "IV   ") ) /* emblcds database format */
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'SV' */
	    ajStrTokenNextParse(&handle, &token); /* version */
	    seqSvSave(thys, token);
	}

	if(ajStrPrefixC(seqReadLine, "DE   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    if(ajStrGetLen(thys->Desc))
	    {
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
	    }
	    else
		ajStrAssignS(&thys->Desc, token);
	}

	if(ajStrPrefixC(seqReadLine, "KW   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'KW' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		ajListstrPushApp(thys->Keylist, liststr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "OS   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OS' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr);
		ajStrDel(&tmpstr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "OC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OC' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr);
		ajStrDel(&tmpstr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "FT   "))
	    if(seqinUfoLocal(seqin))
	    {
		if(!dofeat)
		{
		    dofeat = ajTrue;
		    ajFeattabInDel(&seqin->Ftquery);
		    seqin->Ftquery = ajFeattabInNewSS(seqFtFmtEmbl,
						      thys->Name, "N");
		    /* ajDebug("seqin->Ftquery Handle %x\n",
		       seqin->Ftquery->Handle); */
		}
		ajFileBuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		/* ajDebug("EMBL FEAT saved line:\n%S", seqReadLine); */
	    }

	ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    }



    if(dofeat)
    {
	/* ajDebug("EMBL FEAT TabIn %x\n", seqin->Ftquery); */
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    if(ajStrGetLen(seqin->Inseq))
    {
	/* we have a sequence to use */
	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	{
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);
	    ajFmtPrintAppS(&thys->TextPtr, "//\n");
	}
    }
    else
    {
	/* read the sequence and terminator */
	ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    bufflines++;
	    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	}
    }

    ajFileBuffClear(buff, 0);

    ajStrDel(&token);

    ajStrTokenDel(&handle);


    /* ajSeqTrace(thys); */

    return ajTrue;
}




/* @funcstatic seqReadExperiment **********************************************
**
** Given data in a sequence structure, tries to read everything needed
** using Staden experiment format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadExperiment(AjPSeq thys, AjPSeqin seqin)
{

    static AjPStrTok handle  = NULL;
    static AjPStrTok handle2 = NULL;
    static AjPStr token      = NULL;
    static AjPStr token2     = NULL;
    ajint bufflines          = 0;
    AjBool ok;
    AjPFileBuff buff;
    static AjPStr tmpstr = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr liststr;			/* for lists, do not delete */
    AjPStr accvalstr = NULL;
    ajint i;
    ajint j;
    ajint ilen;
    AjBool avok;

    buff = seqin->Filebuff;

    if(!seqFtFmtEmbl)
	ajStrAssignC(&seqFtFmtEmbl, "embl");

    if(!ajFileBuffGetStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    bufflines++;

    ajDebug("seqReadExperiment first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "ID   "))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }

    if(seqin->Text)
	ajStrAssignC(&thys->TextPtr,ajStrGetPtr(seqReadLine));

    ajDebug("seqReadExperiment ID line found\n");
    ajStrTokenAssignC(&handle, seqReadLine, " \n\r\t");
    ajStrTokenNextParse(&handle, &token);	/* 'ID' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName(&thys->Name, token);

    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    while(ok && !ajStrPrefixC(seqReadLine, "SQ"))
    {
	bufflines++;

	if(ajStrPrefixC(seqReadLine, "EX   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'EX'*/
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /*  experiment description */
	    if(ajStrGetLen(thys->Desc))
	    {
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
	    }
	    else
		ajStrAssignS(&thys->Desc, token);
	}

	if(ajStrPrefixC(seqReadLine, "AV   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'AV' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    if(ajStrGetLen(accvalstr))
	    {
		ajStrAppendC(&accvalstr, " ");
		ajStrAppendS(&accvalstr, token);
	    }
	    else
		ajStrAssignS(&accvalstr, token);
	}

	/* standard EMBL records are allowed */

	if(ajStrPrefixC(seqReadLine, "AC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'AC' */
	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	if(ajStrPrefixC(seqReadLine, "SV   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'SV' */
	    ajStrTokenNextParse(&handle, &token); /* version */
	    seqSvSave(thys, token);
	}

	if(ajStrPrefixC(seqReadLine, "DE   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    if(ajStrGetLen(thys->Desc))
	    {
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
	    }
	    else
		ajStrAssignS(&thys->Desc, token);
	}

	if(ajStrPrefixC(seqReadLine, "KW   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'KW' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		ajListstrPushApp(thys->Keylist, liststr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "OS   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OS' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr);
		ajStrDel(&tmpstr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "OC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OC' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr);
		ajStrDel(&tmpstr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "FT   "))
	    if(seqinUfoLocal(seqin))
	    {
		if(!dofeat)
		{
		    dofeat = ajTrue;
		    ajFeattabInDel(&seqin->Ftquery);
		    seqin->Ftquery = ajFeattabInNewSS(seqFtFmtEmbl,
						      thys->Name, "N");
		    /* ajDebug("seqin->Ftquery Handle %x\n",
		       seqin->Ftquery->Handle); */
		}
		ajFileBuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		/* ajDebug("EMBL FEAT saved line:\n%S", seqReadLine); */
	    }

	ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    }

    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    while(ok && !ajStrPrefixC(seqReadLine, "//"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
	ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
				&thys->TextPtr);
    }
    ajDebug("Sequence read %d bases\n", ajStrGetLen(thys->Seq));

    while(ok && !ajStrPrefixC(seqReadLine, "ID   "))
    {
	bufflines++;
	ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
				&thys->TextPtr);
    }

    if(ok)
	ajFileBuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    else
	ajFileBuffClear(buff, 0);

    if(dofeat)
    {
	/* ajDebug("EMBL FEAT TabIn %x\n", seqin->Ftquery); */
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    if(ajStrGetLen(accvalstr))
    {
	ilen = ajStrGetLen(thys->Seq);
	AJCNEW0(thys->Accuracy,ilen);
	ajStrTokenAssignC(&handle, accvalstr, " ");
	avok = ajTrue;
	for(i=0;i<ilen;i++)
	{
	    if(!ajStrTokenNextParse(&handle, &token))
	    {
		ajWarn("Missing accuracy for base %d in experiment format\n",
		       i+1);
		avok = ajFalse;
		break;
	    }
	    ajStrTokenAssignC(&handle2, token, ",");
	    while(ajStrTokenNextParse(&handle2, &token2))
	    {
		if(ajStrToInt(token2, &j))
		{
		    if(j > thys->Accuracy[i])
			thys->Accuracy[i] = j;
		}
		else
		{
		    ajWarn("Bad accuracy '%S' for base %d "
			   "in experiment format\n",
			   token, i+1);
		    avok = ajFalse;
		    break;
		}
	    }
	    ajDebug("Accval[%d] %3d '%S'\n", i+1, thys->Accuracy[i], token);
	}
	if(!avok)
	    AJFREE(thys->Accuracy);
	ajStrTokenDel(&handle);
	ajStrTokenDel(&handle2);
	ajStrDel(&token);
	ajStrDel(&token2);
	ajStrDel(&accvalstr);
    }

    ajStrDelStatic(&token);

    ajStrTokenReset(&handle);


    /* ajSeqTrace(thys); */

    return ajTrue;
}




/* @funcstatic seqReadGenbank *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using Genbank format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGenbank(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    ajint bufflines         = 0;
    AjBool ok;
    AjBool done = ajFalse;
    AjPFileBuff buff;
    AjPStr tmpstr = NULL;
    AjBool dofeat        = ajFalse;
    AjPSeqQuery qry;
    AjPStr liststr;			/* for lists, do not delete */

    ajDebug("seqReadGenbank\n");

    buff = seqin->Filebuff;
    qry  = seqin->Query;

    if(!seqFtFmtGenbank)
	ajStrAssignC(&seqFtFmtGenbank, "genbank");

    if(!ajFileBuffGetStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;
    bufflines++;

    ok = ajTrue;

    /* for GCG formatted databases */

    if(ajStrPrefixC(seqReadLine, "WPCOMMENT"))
    {
	ok = ajFileBuffGetStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr);
	bufflines++;
	while(ok && ajStrPrefixC(seqReadLine, " "))
	{
	    ok = ajFileBuffGetStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;
	}
    }

    if(!ok)
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }

    /* This loop necessary owing to headers on GB distro files */
    if(ajStrFindC(seqReadLine,"Genetic Sequence Data Bank") >= 0)
	while(!ajStrPrefixC(seqReadLine, "LOCUS"))
	{
	    if(!ajFileBuffGetStore(buff, &seqReadLine,
				   seqin->Text, &thys->TextPtr))
		return ajFalse;
	    bufflines++;
	}

    if(!ajStrPrefixC(seqReadLine, "LOCUS"))
    {
	ajDebug("failed - LOCUS not found - first line was\n%S\n", seqReadLine);
	ajFileBuffReset(buff);
	return ajFalse;
    }

    if(seqin->Text)
	ajStrAssignC(&thys->TextPtr,ajStrGetPtr(seqReadLine));

    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    ajStrTokenNextParse(&handle, &token);	/* 'ID' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName(&thys->Name, token);

    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    while(ok &&
	  !ajStrPrefixC(seqReadLine, "ORIGIN") &&
	  !ajStrPrefixC(seqReadLine, "BASE COUNT"))
    {
	done = ajFalse;
	bufflines++;
	if(ajStrPrefixC(seqReadLine, "ACCESSION"))
	{
	    ajDebug("accession found\n");

	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'ACCESSION' */
	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	if(ajStrPrefixC(seqReadLine, "VERSION"))
	{
	    ajDebug("seqversion found\n");

	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'VERSION' */
	    ajStrTokenNextParse(&handle, &token);
	    seqSvSave(thys, token);
	    if(ajStrTokenNextParseC(&handle, ": \n\r", &token)) /* GI: */
	    {
		ajStrTokenNextParse(&handle, &token);
		ajStrAssignS(&thys->Gi, token);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "FEATURES"))
	    if(seqinUfoLocal(seqin))
	    {
		ajDebug("features found\n");
		if(!dofeat)
		{
		    dofeat = ajTrue;
		    ajFeattabInDel(&seqin->Ftquery);
		    seqin->Ftquery = ajFeattabInNewSS(seqFtFmtGenbank,
						      thys->Name, "N");
		    ajDebug("seqin->Ftquery Handle %x\n",
			    seqin->Ftquery->Handle);
		    /* ajDebug("GENBANK FEAT first line:\n%S", seqReadLine); */
		}
		ajFileBuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		ok = ajFileBuffGetStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		done = ajTrue;
		while(ok && ajStrPrefixC(seqReadLine, " "))
		{
		    bufflines++;
		    ajFileBuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		    /* ajDebug("GENBANK FEAT saved line:\n%S", seqReadLine); */
		    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
					    &thys->TextPtr);
		}
	    }


	if(ajStrPrefixC(seqReadLine, "DEFINITION"))
	{
	    ajDebug("definition found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DEFINITION' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    ajStrAssignS(&thys->Desc, token);
	    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;
	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		bufflines++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");
		ajStrTokenNextParseC(&handle, "\n\r", &token);
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
		ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "KEYWORDS"))
	{
	    ajDebug("keywords found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'KEYWORDS' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		ajListstrPushApp(thys->Keylist, liststr);
	    }

	    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;
	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		bufflines++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");
		while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
		{
		    liststr = ajStrNewS(token);
		    ajStrTrimWhite(&liststr);
		    ajListstrPushApp(thys->Keylist, liststr);
		}
		ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "  ORGANISM"))
	{
	    ajDebug("organism found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'ORGANISM' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr);
	    }

	    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;
	    while(ok && ajStrPrefixC(seqReadLine, "    "))
	    {
		bufflines++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");
		while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
		{
		    ajStrAssignS(&tmpstr, token);
		    ajStrTrimWhite(&tmpstr);
		    seqTaxSave(thys, tmpstr);
		}
		ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	if(!done)
	    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
    }

    if(dofeat)
    {
	ajDebug("GENBANK FEAT TabIn %x\n", seqin->Ftquery);
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    if(ajStrGetLen(seqin->Inseq))
    {
	/* we have a sequence to use */
	ajDebug("Got an Inseq sequence\n");
	if(ajStrMatchC(qry->Method,"gcg"))
	    while(ok && !ajStrPrefixC(seqReadLine,"ORIGIN"))
		ok = ajFileBuffGetStore(buff,&seqReadLine, seqin->Text,
					&thys->TextPtr);

	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	{
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);
	    ajFmtPrintAppS(&thys->TextPtr, "//\n");
	}
    }
    else
    {
	/* read the sequence and terminator */
	ajDebug("sequence start at '%S'\n", seqReadLine);
	while(!ajStrPrefixC(seqReadLine,"ORIGIN") &&
	      !ajStrPrefixC(seqReadLine,"BASE COUNT"))
	    if(!ajFileBuffGetStore(buff,&seqReadLine,
				   seqin->Text, &thys->TextPtr))
		break;
	ok = ajFileBuffGetStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    if(!ajStrPrefixC(seqReadLine, "ORIGIN") &&
	       !ajStrPrefixC(seqReadLine,"BASE COUNT"))
		seqAppend(&thys->Seq, seqReadLine);
	    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    bufflines++;
	}
    }

    if(!ajStrMatchC(qry->Method,"gcg"))
	while(ok && !ajStrPrefixC(seqReadLine,"//"))
	    ok = ajFileBuffGetStore(buff,&seqReadLine,
				    seqin->Text, &thys->TextPtr);


    ajFileBuffClear(buff, 0);

    ajStrTokenDel(&handle);
    ajStrDel(&token);
    ajStrDel(&tmpstr);

    return ajTrue;
}




/* @funcstatic seqReadGff *****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using GFF format.
**
** GFF only offers the sequence, and the type, with the DNA, RNA and
** Protein and End-xxx headers. GFF allows other header lines to be defined,
** so EMBOSS can add more lines for accession number and description
**
** GFF also defines Type and sequence-region headers, but they only
** provide information that is also in the DNA, RNA or Protein header
** and these are required for sequence storage so we ignore the alternatives.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGff(AjPSeq thys, AjPSeqin seqin)
{

    static AjPRegexp typexp = NULL;
    ajint bufflines         = 0;
    AjBool ok;
    AjBool isseq            = ajFalse;
    AjPFileBuff buff;
    AjPFileBuff ftfile   = NULL;
    AjBool dofeat        = ajFalse;
    static AjPStr typstr;
    static AjPStr verstr = NULL;	/* copy of version line */
    static AjPStr outstr = NULL;	/* generated Type line */

    buff = seqin->Filebuff;

    if(!typexp)
	typexp = ajRegCompC("^##([DR]NA|Protein) +([^ \t\r\n]+)");

    if(!seqFtFmtGff)
	ajStrAssignC(&seqFtFmtGff, "gff");

    ok = ajFileBuffGetStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    bufflines++;

    ajDebug("seqReadGff first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "##gff-version "))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }
    ajStrAssignS(&verstr, seqReadLine);

    if(seqin->Text)
	ajStrAssignS(&thys->TextPtr,seqReadLine);

    ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    while(ok && ajStrPrefixC(seqReadLine, "##"))
    {
	if(ajRegExec(typexp, seqReadLine))
	{
	    isseq = ajTrue;
	    ajRegSubI(typexp, 1, &typstr);
	    ajRegSubI(typexp, 2, &thys->Name);
	    if(ajStrMatchC(typstr, "Protein"))
		ajSeqSetProt(thys);
	    else
		ajSeqSetNuc(thys);
	    ajFmtPrintS(&outstr, "##Type %S %S", typstr, thys->Name);
	}
	else if(ajStrPrefixC(seqReadLine, "##end-"))
	    isseq = ajFalse;
	else if(isseq)
	    seqAppend(&thys->Seq, seqReadLine);

	ok = ajFileBuffGetStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    }

    if(!ajSeqGetLen(thys))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }

    /* do we want the features now? */

    if(ok & seqinUfoLocal(seqin))
    {
	dofeat = ajTrue;
	ftfile = ajFileBuffNew();
	ajFileBuffLoadS(ftfile, verstr);
	ajFileBuffLoadS(ftfile, outstr);
	while(ok && !ajStrPrefixC(seqReadLine, "##"))
	{
	    ajFileBuffLoadS(ftfile, seqReadLine);
	    /* ajDebug("GFF FEAT saved line:\n%S", seqReadLine); */
	    ok = ajFileBuffGetStore(buff,&seqReadLine,seqin->Text,&thys->TextPtr);
	}
    }

    if(dofeat)
    {
	ajFeattabInDel(&seqin->Ftquery);
	seqin->Ftquery = ajFeattabInNewSSF(seqFtFmtGff, thys->Name,
					   ajStrGetPtr(seqin->Type), ftfile);
	ajDebug("GFF FEAT TabIn %x\n", seqin->Ftquery);
	ftfile = NULL;		  /* now copied to seqin->FeattabIn */
	ajFeattableDel(&seqin->Fttable);
	seqin->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(seqin->Fttable); */
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = seqin->Fttable;
	seqin->Fttable = NULL;
    }

    ajFileBuffClear(buff, 0);

    return ajTrue;
}




/* @funcstatic seqReadAbi *****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using ABI format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadAbi(AjPSeq thys, AjPSeqin seqin)
{
    AjPFileBuff buff;
    AjBool  ok      = ajFalse;
    ajlong baseO    = 0L;
    ajlong numBases = 0L;
    AjPStr sample   = NULL;
    AjPStr smpl     = NULL;
    AjPFile fp;
    ajint filestat;

    buff = seqin->Filebuff;
    fp = ajFileBuffFile(buff);

    ajDebug("seqReadAbi file %F\n", fp);

    /* ajFileBuffTraceFull(buff, 10, 10); */

    if(ajFileBuffEnd(buff))
	return ajFalse;

    if(!ajSeqABITest(fp))
    {
        ajDebug("seqReadAbi ajSeqABITest failed on %F\n", fp);
	ajFileBuffResetPos(buff);
	return ajFalse;
    }

    if(seqin->Text)
	ajWarn("Failed to read text from binary ABI file %F", fp);

    filestat = ajFileSeek(fp,0L,0);
    ajDebug("filestat %d\n", filestat);

    numBases = ajSeqABIGetNBase(fp);
    /* Find BASE tag & get offset                    */
    baseO = ajSeqABIGetBaseOffset(fp);
    /* Read in sequence         */
    ok = ajSeqABIReadSeq(fp,baseO,numBases,&thys->Seq);
    if(!ok) {
	ajFileSeek(fp,filestat,0);
	ajFileBuffResetPos(buff);
	return ajFalse;
    }

    sample = ajStrNew();
    ajSeqABISampleName(fp, &sample);

    /* replace dots in the sample name with underscore */
    if(!seqRegAbiDots)
	seqRegAbiDots = ajRegCompC("^(.*)[.](.*)$");
    smpl = ajStrNew();

    while(ajRegExec(seqRegAbiDots,sample))
    {
	ajStrSetClear(&sample);
	ajRegSubI(seqRegAbiDots,1,&smpl);
	ajStrAppendC(&smpl,"_");
	ajStrAppendS(&sample,smpl);
	ajRegSubI(seqRegAbiDots,2,&smpl);
	ajStrAppendS(&sample,smpl);
    }

    ajStrAssignC(&thys->Name,ajStrGetPtr(sample));

    ajSeqSetNuc(thys);

    ajFileBuffClear(buff, -1);
    buff->File->End=ajTrue;

    ajStrDel(&smpl);
    ajStrDel(&sample);

    return ajTrue;
}




/* @func ajSeqPrintInFormat ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajSeqPrintInFormat(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# sequence input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Nuc   Can read nucleotide input\n");
    ajFmtPrintF(outf, "# Pro   Can read protein input\n");
    ajFmtPrintF(outf, "# Feat  Can read feature annotation\n");
    ajFmtPrintF(outf, "# Gap   Can read gap characters\n");
    ajFmtPrintF(outf, "# Mset  Can read seqsetall (multiple seqsets)\n");
    ajFmtPrintF(outf, "# Name         Alias Try  Nuc  Pro Feat  Gap MSet "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");
    for(i=0; seqInFormatDef[i].Name; i++)
    {
	if(full || !seqInFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B  %3B  %3B  %3B  %3B  %3B \"%s\"\n",
			seqInFormatDef[i].Name,
			seqInFormatDef[i].Alias,
			seqInFormatDef[i].Try,
			seqInFormatDef[i].Nucleotide,
			seqInFormatDef[i].Protein,
			seqInFormatDef[i].Feature,
			seqInFormatDef[i].Gap,
			seqInFormatDef[i].Multiset,
			seqInFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @funcstatic seqFindInFormat ************************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Sets iformat as the recognized format, and returns ajTrue.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqFindInFormat(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajint i = 0;

    /* ajDebug("seqFindInFormat '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; seqInFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, seqInFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, seqInFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", seqInFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajSeqFormatTest ******************************************************
**
** tests whether a named format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
** @@
******************************************************************************/

AjBool ajSeqFormatTest(const AjPStr format)
{
    ajint i;

    for(i=0; seqInFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, seqInFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @funcstatic seqSetInFormat *************************************************
**
** Steps through a list of default formats, setting the Try value for
** each known format to ajTrue if it is in the list, and ajFalse
** if not.
**
** @param [r] format [const AjPStr] Format list, punctuated by whitespace
**                                  or commas
** @return [AjBool] ajTrue if all formats were accepted
** @@
******************************************************************************/

static AjBool seqSetInFormat(const AjPStr format)
{
    ajint i;
    AjPStr fmtstr           = NULL;
    static AjPStrTok handle = NULL;
    ajint ifound;
    AjBool ret              = ajTrue;

    for(i=0; seqInFormatDef[i].Name; i++)
	seqInFormatDef[i].Try = ajFalse;

    ajDebug("seqSetInformat '%S'\n", format);

    ajStrTokenAssignC(&handle, format, " \t\n\r,;:");
    while(ajStrTokenNextParseC(&handle, " \t\n\r,;:", &fmtstr))
    {
	ifound = 0;
	for(i=0; seqInFormatDef[i].Name; i++)
	    if(ajStrMatchCaseC(fmtstr, seqInFormatDef[i].Name))
	    {
		/* ajDebug("found '%S' %d\n", fmtstr, i); */
		seqInFormatDef[i].Try = ajTrue;
		ifound = 1;
		break;
	    }

	if(!ifound)
	{
	    /* ajDebug("not found '%S'\n", fmtstr); */

	    ajErr("Input format '%S' not known", fmtstr);
	    ret = ajFalse;
	}
    }

    ajStrTokenReset(&handle);

    return ret;
}




/* @funcstatic seqAppend ******************************************************
**
** Appends sequence characters in the input line to a growing sequence.
** Non sequence characters are simply ignored.
**
** @param [u] pseq [AjPStr*] Sequence as a string
** @param [r] line [const AjPStr] Input line.
** @return [ajint] Sequence length to date.
** @@
******************************************************************************/

static ajint seqAppend(AjPStr* pseq, const AjPStr line)
{
    AjPStr tmpstr = NULL;
    ajint ret = 0;

    ajStrAssignS(&tmpstr, line);
    ajStrKeepSetAlphaC(&tmpstr, "*.~?#+-");
    ajStrAppendS(pseq, tmpstr);

/*
    while(ajStrGetLen(tmpstr) && ajRegExec(seqexp, tmpstr))
    {
	ajRegSubI(seqexp, 0, &token);
	ajStrAppendS(pseq, token);
	i += ajStrGetLen(token);
	ajRegPost(seqexp, &tmpstr);
    }
*/
    ret = ajStrGetLen(*pseq);
    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic seqAppendCommented *********************************************
**
** Appends sequence characters in the input line to a growing sequence.
** Non sequence characters are simply ignored.
**
** This version of seqAppend removes comments in the angle brackets style
** used first by Staden and then later by GCG.
**
** @param [u] pseq [AjPStr*] Sequence as a string
** @param [u] incomment [AjBool*] Currently processing a comment
** @param [r] line [const AjPStr] Input line.
** @return [ajint] Sequence length to date.
** @@
******************************************************************************/

static ajint seqAppendCommented(AjPStr* pseq, AjBool* incomment,
				const AjPStr line)
{
    AjPStr tmpstr = NULL;
    ajint i;
    ajint ret = 0;

    ajStrAssignS(&tmpstr, line);
    ajStrKeepSetAlphaC(&tmpstr, "*.~?#+-<>");

    ajDebug("seqAppendCommented %B '%S'\n", *incomment, tmpstr);
    while(ajStrGetLen(tmpstr))
    {
	/* if we are in a comment, look for the end of it */
	/* Staden comments are <comment> */
	/* GCG comments are <comment< or >comment> */

	/* there should be no case of >comment< 
	   but in a broken file we can't tell */

	/* so we test for both kinds of angle brackets at both ends */

	if(*incomment)
	{
	    i = ajStrFindAnyC(tmpstr, "<>");
	    if(i >= 0)			/* comment ends in this line */
	    {
		ajStrCutStart(&tmpstr, i+1);
		*incomment = ajFalse;
	    }
	    else
	    {
		ajStrAssignC(&tmpstr, "");	/* all comment */
	    }
	}
	else
	{
	    i = ajStrFindAnyC(tmpstr, "<>");
	    if(i >= 0)			/* comment starts in this line */
	    {
		if(i)
		    ajStrAppendSubS(pseq, tmpstr, 0, i-1);
		ajDebug("before comment saved '%S'\n", *pseq);
		ajStrCutStart(&tmpstr, i+1);
		*incomment = ajTrue;
	    }
	    else
	    {
		ajStrAppendS(pseq, tmpstr);
		ajDebug("all saved '%S'\n", *pseq);
		ajStrAssignC(&tmpstr, "");
	    }
	}
	if(ajStrGetLen(tmpstr))
	    ajDebug("continuing %B '%S'\n", *incomment, tmpstr);
	else
	    ajDebug("done %B '%S'\n", *incomment, tmpstr);
    }

    

/*
    while(ajStrGetLen(tmpstr) && ajRegExec(seqexp, tmpstr))
    {
	ajRegSubI(seqexp, 0, &token);
	ajStrAppendS(pseq, token);
	i += ajStrGetLen(token);
	ajRegPost(seqexp, &tmpstr);
    }
*/
    ret = ajStrGetLen(*pseq);
    ajStrDel(&tmpstr);

    return ret;
}

/* @funcstatic seqGcgRegInit **************************************************
**
** Initialises regular expressions for GCG and MSF format parsing
**
**
** @return [void]
******************************************************************************/

static void seqGcgRegInit(void)
{
    if(!seqRegGcgDot)
	seqRegGcgDot = ajRegCompC("[.][.]");

    if(!seqRegGcgChk)
	seqRegGcgChk = ajRegCompC("[Cc][Hh][Ee][Cc][Kk]:[ \t]*([0-9]+)");

    if(!seqRegGcgLen)
	seqRegGcgLen = ajRegCompC("[Ll][Ee][Nn][Gg][Tt][Hh]:[ \t]*([0-9]+)");

    if(!seqRegGcgTyp)
	seqRegGcgTyp = ajRegCompC("[Tt][Yy][Pp][Ee]:[ \t]*([NP])");

    if(!seqRegGcgNam)
	seqRegGcgNam = ajRegCompC("[^ \t>]+");

    if(!seqRegGcgMsf)
	seqRegGcgMsf = ajRegCompC("[Mm][Ss][Ff]:[ \t]*([0-9]+)");

    if(!seqRegGcgMsflen)
	seqRegGcgMsflen = ajRegCompC("[Ll][Ee][Nn]:[ \t]*([0-9]+)");

    if(!seqRegGcgWgt)
	seqRegGcgWgt = ajRegCompC("[Ww][Ee][Ii][Gg][Hh][Tt]:[ \t]*([0-9.]+)");

    if(!seqRegGcgMsfnam)
	seqRegGcgMsfnam = ajRegCompC("[Nn][Aa][Mm][Ee]:[ \t]*([^ \t]+)");

    return;
}

/* @funcstatic seqGcgDots *****************************************************
**
** Looks for the ".." line in the header of a GCG format sequence.
** Care is needed to make sure this is not an MSF header which
** has a very similar format.
**
** Data found on the header line is extracted and returned.
**
** The number of lines searched is limited to avoid parsing large data
** files that are not in GCG format. The user should set this limit to
** be large enough to handle large EMBL/Genbank annotations
**
** @param [u] thys [AjPSeq] Sequence.
** @param [r] seqin [const AjPSeqin] Sequence input.
** @param [u] pline [AjPStr*] Input buffer.
** @param [r] maxlines [ajint] Maximum number of lines to read before giving up
** @param [w] len [ajint*] Length of sequence read.
** @return [AjBool] ajTrue on success. ajFalse on failure or aborting.
** @@
******************************************************************************/

static AjBool seqGcgDots(AjPSeq thys, const  AjPSeqin seqin,
			 AjPStr* pline,
			 ajint maxlines, ajint* len)
{
    static AjPStr token = NULL;
    ajint check  = 0;
    ajint nlines = 0;

    AjPFileBuff buff;

    buff = seqin->Filebuff;

    seqGcgRegInit();

    while(nlines < maxlines)
    {
	if(nlines++)
	    if(!ajFileBuffGetStore(buff, pline,
				   seqin->Text, &thys->TextPtr))
		return ajFalse;

	if(nlines > maxlines)
	    return ajFalse;

	if(!ajRegExec(seqRegGcgDot, *pline))
	    continue;

	ajDebug("seqGcgDots   .. found\n'%S'\n", *pline);
	if(!ajRegExec(seqRegGcgChk, *pline))	/* checksum required */
	    return ajFalse;

	if(ajRegExec(seqRegGcgMsf, *pline))	/* oops - it's an MSF file */
	    return ajFalse;

	ajRegSubI(seqRegGcgChk, 1, &token);
	ajStrToInt(token, &check);

	ajDebug("   checksum %d\n", check);

	if(ajRegExec(seqRegGcgLen, *pline))
	{
	    ajRegSubI(seqRegGcgLen, 1, &token);
	    ajStrToInt(token, len);
	    ajDebug("   length %d\n", *len);
	}

	if(ajRegExec(seqRegGcgNam, *pline))
	{
	    ajRegSubI(seqRegGcgNam, 0, &thys->Name);
	    ajDebug("   name '%S'\n", thys->Name);
	}

	if(ajRegExec(seqRegGcgTyp, *pline))
	{
	    ajRegSubI(seqRegGcgTyp, 1, &thys->Type);
	    ajDebug("   type '%S'\n", thys->Type);
	}

	ajStrDel(&token);
	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic seqGcgMsfDots **************************************************
**
** Looks for the ".." line in the header of an MSF format sequence.
** Care is needed to make sure this is not a simple GCG header which
** has a very similar format.
**
** Data found on the header line is extracted and returned.
**
** The number of lines searched is limited to avoid parsing large data
** files that are not in GCG format. The user should set this limit to
** be large enough to handle large EMBL/Genbank annotations
**
** @param [u] thys [AjPSeq] Sequence.
** @param [r] seqin [const AjPSeqin] Sequence input.
** @param [u] pline [AjPStr*] Input buffer.
** @param [r] maxlines [ajint] Maximum number of lines to read before giving up
** @param [w] len [ajint*] Length of sequence read.
** @return [AjBool] ajTrue on success. ajFalse on failure or aborting.
** @@
******************************************************************************/

static AjBool seqGcgMsfDots(AjPSeq thys, const AjPSeqin seqin, AjPStr* pline,
			    ajint maxlines, ajint* len)
{
    AjPStr token = NULL;
    ajint check  = 0;
    ajint nlines = 0;

    AjPFileBuff buff;

    buff = seqin->Filebuff;

    ajDebug("seqGcgMsfDots maxlines: %d\nline: '%S'\n", maxlines,*pline);

    seqGcgRegInit();

    while(nlines < maxlines)
    {
	if(nlines++)
	    if(!ajFileBuffGetStore(buff, pline,
				   seqin->Text, &thys->TextPtr))
		return ajFalse;

	ajDebug("testing line %d\n'%S'\n", nlines,*pline);
	if(nlines > maxlines)
	    return ajFalse;

	if(!ajRegExec(seqRegGcgDot, *pline))
	    continue;

	/* dots found. This must be the line if this is MSF format */

	if(!ajRegExec(seqRegGcgChk, *pline))	/* check: is required */
	    return ajFalse;

	if(!ajRegExec(seqRegGcgMsf, *pline)) /* MSF: len required for GCG*/
	    return ajFalse;


	ajRegSubI(seqRegGcgMsf, 1, &token);
	ajStrToInt(token, len);

	ajRegSubI(seqRegGcgChk, 1, &token);
	ajStrToInt(token, &check);

	if(ajRegExec(seqRegGcgNam, *pline))
	    ajRegSubI(seqRegGcgNam, 0, &thys->Name);

	if(ajRegExec(seqRegGcgTyp, *pline))
	    ajRegSubI(seqRegGcgTyp, 1, &thys->Type);

	ajStrDel(&token);
	ajDebug("seqGcgMsfDots '%S' '%S' len: %d check: %d\n",
		thys->Name, thys->Type, *len, check);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic seqGcgMsfHeader ************************************************
**
** Parses data from a line of an MSF file header. The header stores
** names and other data for all sequences in the file. Each file
** is definied on a separate line. The results are stored
** in the MSF internal table. The sequence data is read later in the
** input file and added to the table.
**
** @param [r] line [const AjPStr] Input line.
** @param [u] pmsfitem [SeqPMsfItem*] MSF internal table item.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqGcgMsfHeader(const AjPStr line, SeqPMsfItem* pmsfitem)
{
    AjPStr name         = NULL;	/* NOTE: not static. New each time for list */
    AjPStr token = NULL;
    SeqPMsfItem msfitem = NULL;

    ajDebug("seqGcgMsfHeader '%S'\n", line);

    if(!ajRegExec(seqRegGcgMsfnam, line))
	return ajFalse;

    ajRegSubI(seqRegGcgMsfnam, 1, &name);
    /*ajDebug("Name found\n");*/

    if(!ajRegExec(seqRegGcgChk, line))
	return ajFalse;

    /*ajDebug("Check found\n");*/

    *pmsfitem = AJNEW0(msfitem);
    msfitem->Name = name;

    ajRegSubI(seqRegGcgChk, 1, &token);
    ajStrToInt(token, &msfitem->Check);

    if(ajRegExec(seqRegGcgMsflen, line))
    {
	ajRegSubI(seqRegGcgMsflen, 1, &token);
	ajStrToInt(token, &msfitem->Len);
    }
    else
	msfitem->Len = 0;

    msfitem->Seq = ajStrNewRes(msfitem->Len+1);

    if(ajRegExec(seqRegGcgWgt, line))
    {
	ajRegSubI(seqRegGcgWgt, 1, &token);
	ajStrToFloat(token, &msfitem->Weight);
    }
    else
	msfitem->Weight = 1.0;

    ajDebug("MSF header name '%S' check %d len %d weight %.3f\n",
	    msfitem->Name, msfitem->Check, msfitem->Len, msfitem->Weight);

    ajStrDel(&token);

    return ajTrue;
}




/* @funcstatic seqUsaProcess **************************************************
**
** Converts a USA Universal Sequence Address into an open file.
**
** First tests for "[n:n:r]" range and sets this if it is found
**
** Then tests for asis:: in which the "filename" is really the sequence
** and no format is needed.
**
** Then tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using seqListProcess which in turn invokes seqUsaProcess
** until a valid USA is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and sequence reading
** will have to scan for the entry/entries we need.
**
** @param [u] thys [AjPSeq] Sequence to be read.
** @param [u] seqin [AjPSeqin] Sequence input structure.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqUsaProcess(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr tmpstr  = NULL;

    AjPSeqQuery qry;

    AjBool fmtstat   = ajFalse;	 /* status returns from regex tests */
    AjBool regstat   = ajFalse;
    AjBool dbstat    = ajFalse;
    AjBool accstat   = ajFalse;	/* return status from reading something */
    AjBool liststat  = ajFalse;
    AjBool asisstat  = ajFalse;
    AjBool rangestat = ajFalse;
#ifdef __CYGWIN__
    AjPStr usatmp    = NULL;
#endif

    qry = seqin->Query;

    ajStrDel(&qry->Field);    /* clear it. we test this for regstat */

    ajDebug("++seqUsaProcess '%S' %d..%d(%b) '%S' %d \n",
	    seqin->Usa, seqin->Begin, seqin->End, seqin->Rev,
	    seqin->Formatstr, seqin->Format);

    if(!seqRegUsaFmt)
	seqRegUsaFmt = ajRegCompC("^([A-Za-z0-9]*)::(.*)$");
    /* \1 format letters and numbers only */
    /* \2 remainder (filename, etc.)*/

    if(!seqRegUsaDb)
	seqRegUsaDb = ajRegCompC("^([A-Za-z][A-Za-z0-9_]+)([-]([A-Za-z]+))?"
			   "([:{]([^}]*)}?)?$");

    /* \1 dbname (start with a letter, then alphanumeric) */
    /* \2 -id or -acc etc. */
    /* \3 qry->Field (id or acc etc.) */
    /* \4 :qry->QryString */
    /* \5 qry->QryString */

    if(!seqRegUsaId)		 /* \1 is filename \4 is the qry->QryString */
#ifndef WIN32
	seqRegUsaId = ajRegCompC("^([^|]+[|]|[^:{%]+)"
			   "(([:{%])(([^:}]+):)?([^:}]*)}?)?$");
#else
	/* Windows file names can start with e.g.: 'C:\' */
	/* But allow e.g. 'C:/...', for Staden spin */
	seqRegUsaId = ajRegCompC ("^(([a-zA-Z]:[\\\\/])?[^:{%]+)"
				  "(([:{%])(([^:}]+):)?([^:}]*)}?)?$");
#endif


    if(!seqRegUsaList)	 /* \1 is filename \3 is the qry->QryString */
	seqRegUsaList = ajRegCompC("^(@|[Ll][Ii][Ss][Tt]:+)(.+)$");

    if(!seqRegUsaAsis)	 /* \1 is filename \3 is the qry->QryString */
	seqRegUsaAsis = ajRegCompC("^[Aa][Ss][Ii][Ss]:+(.+)$");

    if(!seqRegUsaWild)
	seqRegUsaWild = ajRegCompC("(.*[*].*)");
    /* \1 wildcard query */

    if(!seqRegUsaRange)    /* \1 is rest of USA \2 start \3 end \5 reverse*/
	seqRegUsaRange = ajRegCompC("(.*)[[](-?[0-9]*):(-?[0-9]*)(:([Rr])?)?[]]$");

    ajStrAssignS(&seqUsaTest, seqin->Usa);
    /* Strip any leading spaces */
    ajStrTrimC(&seqUsaTest," \t\n");

#ifdef __CYGWIN__
    if(*(ajStrGetPtr(seqUsaTest)+1)==':')
    {
	usatmp = ajStrNew();
        ajFmtPrintS(&usatmp,"/cygdrive/%c/%s",*ajStrGetPtr(seqUsaTest),
		    ajStrGetPtr(seqUsaTest)+2);
        ajStrAssignRef(&seqUsaTest,usatmp);
        ajStrDel(&usatmp);
    }
#endif

    ajDebug("USA to test: '%S'\n\n", seqUsaTest);

    rangestat = ajRegExec(seqRegUsaRange, seqUsaTest);
    if(rangestat)
    {
	ajRegSubI(seqRegUsaRange, 2, &tmpstr);
	if(ajStrGetLen(tmpstr))
	    ajStrToInt(tmpstr, &seqin->Begin);
	ajRegSubI(seqRegUsaRange, 3, &tmpstr);
	if(ajStrGetLen(tmpstr))
	    ajStrToInt(tmpstr, &seqin->End);
	ajRegSubI(seqRegUsaRange, 5, &tmpstr);
	if(ajStrGetLen(tmpstr))
	    seqin->Rev = ajTrue;
	ajStrDel(&tmpstr);
	ajRegSubI(seqRegUsaRange, 1, &seqUsaTest);
	ajDebug("range found [%d:%d:%b]\n",
		seqin->Begin, seqin->End, seqin->Rev);
    }

    asisstat = ajRegExec(seqRegUsaAsis, seqUsaTest);
    if(asisstat)
    {
	ajRegSubI(seqRegUsaAsis, 1, &qry->Filename);
	ajStrAssignC(&qry->Formatstr, "text");
	ajStrAssignS(&seqin->Formatstr, qry->Formatstr);
	seqFormatSet(thys, seqin);
	ajDebug("asis sequence '%S'\n", qry->Filename);
	return ajSeqAccessAsis(seqin);
    }

    liststat = ajRegExec(seqRegUsaList, seqUsaTest);
    fmtstat = ajRegExec(seqRegUsaFmt, seqUsaTest);
    ajDebug("format regexp: %B list:%B\n", fmtstat, liststat);

    if(fmtstat && !liststat)
    {
	ajRegSubI(seqRegUsaFmt, 1, &qry->Formatstr);
	/* default unknown */
	ajStrAssignEmptyC(&qry->Formatstr, seqInFormatDef[0].Name);
	ajRegSubI(seqRegUsaFmt, 2, &seqUsaTest);
	ajDebug("found format %S\n", qry->Formatstr);
	if(seqFindInFormat(qry->Formatstr, &seqin->Format))
	    ajStrAssignS(&seqin->Formatstr, qry->Formatstr);
	else
	    ajDebug("unknown format '%S'\n", qry->Formatstr);
    }
    else
	ajDebug("no format specified in USA\n");

    ajDebug("\n");

    seqFormatSet(thys, seqin);

    liststat = ajRegExec(seqRegUsaList, seqUsaTest);
    if(liststat)
    {
	ajRegSubI(seqRegUsaList, 2, &seqQryList);
	ajDebug("list found @%S fmt:%B range:%B\n",
		seqQryList, fmtstat, rangestat);
	if(seqin->Count && fmtstat)
	    ajWarn("List includes another list and format. "
		   "Results undefined\n");
	if(seqin->Count && rangestat)
	    ajWarn("List includes another list with range. "
		   "Results undefined\n");
	return seqListProcess(thys, seqin, seqQryList);
    }

    regstat = ajRegExec(seqRegUsaDb, seqUsaTest);
    ajDebug("dbname dbexp: %B\n", regstat);

    if(regstat)
    {
	/* clear it if this was really a file */	
	ajRegSubI(seqRegUsaDb, 3, &qry->Field);
	ajRegSubI(seqRegUsaDb, 1, &seqQryDb);
	if(!ajNamDatabase(seqQryDb))
	{
	    ajDebug("unknown dbname %S, try filename\n", seqQryDb);
	    regstat = ajFalse;
	}
    }

    if(regstat)
    {
	ajRegSubI(seqRegUsaDb, 5, &qry->QryString);
	ajStrAssignS(&qry->DbName, seqQryDb);
	ajDebug("found dbname '%S' level: '%S' qry->QryString: '%S'\n",
		qry->DbName, qry->Field, qry->QryString);
	dbstat = ajNamDbData(qry);

	if(dbstat && ajStrGetLen(qry->QryString))
	{
	    /* ajDebug("  qry->QryString %S\n", qry->QryString); */
	    if(ajStrGetLen(qry->Field))
	    {
		ajDebug("  db QryString '%S' Field '%S'\n",
			qry->QryString, qry->Field);
		if(ajStrMatchCaseC(qry->Field, "id"))
		    ajStrAssignS(&qry->Id, qry->QryString);
		else if(ajStrMatchCaseC(qry->Field, "acc"))
		    ajStrAssignS(&qry->Acc, qry->QryString);
		else
		{
		    if(!seqQueryField(qry, qry->Field))
		    {
			ajErr("USA '%S' query field '%S' not defined"
			      " for database '%S'",
			      seqUsaTest, qry->Field, qry->DbName);
			return ajFalse;
		    }

		    if(ajStrMatchCaseC(qry->Field, "sv"))
			ajStrAssignS(&qry->Sv, qry->QryString);
		    else if(ajStrMatchCaseC(qry->Field, "gi"))
			ajStrAssignS(&qry->Gi, qry->QryString);
		    else if(ajStrMatchCaseC(qry->Field, "des"))
			ajStrAssignS(&qry->Des, qry->QryString);
		    else if(ajStrMatchCaseC(qry->Field, "org"))
			ajStrAssignS(&qry->Org, qry->QryString);
		    else if(ajStrMatchCaseC(qry->Field, "key"))
			ajStrAssignS(&qry->Key, qry->QryString);
		    else
		    {
			ajErr("USA '%S' query level '%S' not supported",
			      seqUsaTest, qry->Field);
			return ajFalse;
		    }
		}
	    }
	    else
	    {
		ajStrAssignS(&qry->Id, qry->QryString);
		ajStrAssignS(&qry->Acc, qry->QryString);
		if(seqQueryFieldC(qry, "sv"))
		    ajStrAssignS(&qry->Sv, qry->QryString);
	    }
	}
	ajSeqQueryStarclear(qry);
	dbstat = ajNamDbQuery(qry);
	if(dbstat)
	{
	    ajDebug("database type: '%S' format '%S'\n",
		    qry->DbType, qry->Formatstr);
	    if(seqFindInFormat(qry->Formatstr, &seqin->Format))
		ajStrAssignS(&seqin->Formatstr, qry->Formatstr);
	    else
		ajDebug("unknown format '%S'\n", qry->Formatstr);

	    ajDebug("use access method '%S'\n", qry->Method);
	    qry->Access = ajSeqMethod(qry->Method);
	    if(!qry->Access)
	    {
		ajErr("Access method '%S' unknown", qry->Method);
		return ajFalse;
	    }
	    else
	    {
		/* ajDebug("trying access method '%S'\n", qry->Method); */

		/* Calling funclist seqAccess() */
		accstat = qry->Access->Access(seqin);
		if(accstat)
		    return ajTrue;

		ajDebug("Database '%S' : access method '%s' failed\n",
			qry->DbName, qry->Access->Name);
		return ajFalse;
	    }
	}
	else
	{
	    ajErr("no access method available for '%S'", seqUsaTest);
	    return ajFalse;
	}
    }
    else
	ajDebug("no dbname specified\n");

    ajDebug("\n");

    /* no database name, try filename */

    if(!dbstat)
    {
	regstat = ajRegExec(seqRegUsaId, seqUsaTest);
	ajDebug("entry-id regexp: %B\n", regstat);

	if(regstat)
	{
#ifndef WIN32
	    ajRegSubI(seqRegUsaId, 1, &qry->Filename);
	    ajRegSubI(seqRegUsaId, 3, &seqQryChr);
	    ajRegSubI(seqRegUsaId, 5, &qry->Field);
	    ajRegSubI(seqRegUsaId, 6, &qry->QryString);
#else
	    ajRegSubI (seqRegUsaId, 1, &qry->Filename);
	    ajRegSubI (seqRegUsaId, 4, &seqQryChr);
	    ajRegSubI (seqRegUsaId, 6, &qry->Field);
	    ajRegSubI (seqRegUsaId, 7, &qry->QryString);
#endif
	    ajDebug("found filename %S\n", qry->Filename);
	    if(ajStrMatchC(seqQryChr, "%")) {
		ajStrToLong(qry->QryString, &qry->Fpos);
		accstat = ajSeqAccessOffset(seqin);
		if(accstat)
		    return ajTrue;
	    }
	    else
	    {
		if(ajStrGetLen(qry->QryString))
		{
		    ajDebug("file QryString '%S' Field '%S' seqQryChr '%S'\n",
			    qry->QryString, qry->Field, seqQryChr);
		    if(ajStrGetLen(qry->Field)) /* set by dbname above */
		    {
			/* ajDebug("    qry->Field %S\n", qry->Field); */
			if(ajStrMatchCaseC(qry->Field, "id"))
			    ajStrAssignS(&qry->Id, qry->QryString);
			else if(ajStrMatchCaseC(qry->Field, "acc"))
			    ajStrAssignS(&qry->Acc, qry->QryString);
			else if(ajStrMatchCaseC(qry->Field, "sv"))
			    ajStrAssignS(&qry->Sv, qry->QryString);
			else if(ajStrMatchCaseC(qry->Field, "gi"))
			    ajStrAssignS(&qry->Gi, qry->QryString);
			else if(ajStrMatchCaseC(qry->Field, "des"))
			    ajStrAssignS(&qry->Des, qry->QryString);
			else if(ajStrMatchCaseC(qry->Field, "org"))
			    ajStrAssignS(&qry->Org, qry->QryString);
			else if(ajStrMatchCaseC(qry->Field, "key"))
			    ajStrAssignS(&qry->Key, qry->QryString);
			else  /* assume it was part of the filename */
			{
			    ajErr("Unknown query field '%S' in USA '%S'",
				  qry->Field, seqUsaTest);
			    return ajFalse;
			}
		    }
		    else
		    {
			ajStrAssignS(&qry->Id, qry->QryString);
			ajStrAssignS(&qry->Acc, qry->QryString);
		    }
		}
		accstat = ajSeqAccessFile(seqin);
		if(accstat)
		    return ajTrue;
	    }
	    ajErr("Failed to open filename '%S'", qry->Filename);
	    return ajFalse;
	}
	else			  /* dbstat and regstat both failed */
	    ajDebug("no filename specified\n");

	ajDebug("\n");
    }

    return accstat;
}




/* @funcstatic seqQueryField **************************************************
**
** Checks whether a query field is defined for a database as a "fields:"
** string in the database definition.
**
** @param [r] qry [const AjPSeqQuery] Sequence query object
** @param [r] field [const AjPStr] field name
** @return [AjBool] ajTrue if the field is defined
******************************************************************************/

static AjBool seqQueryField(const AjPSeqQuery qry, const AjPStr field)
{

    return seqQueryFieldC(qry, ajStrGetPtr(field));
}




/* @funcstatic seqQueryFieldC *************************************************
**
** Checks whether a query field is defined for a database as a "fields:"
** string in the database definition.
**
** @param [r] qry [const AjPSeqQuery] Sequence query object
** @param [r] field [const char*] field name
** @return [AjBool] ajTrue if the field is defined
******************************************************************************/

static AjBool seqQueryFieldC(const AjPSeqQuery qry, const char* field)
{

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;

    ajDebug("seqQueryFieldC usa '%s' fields '%S'\n", field, qry->DbFields);
    ajStrTokenAssignC(&handle, qry->DbFields, " ");
    while(ajStrTokenNextParse(&handle, &token))
    {
	ajDebug("seqQueryField test '%S'\n", token);
	if(ajStrMatchCaseC(token, field))
	{
	    ajDebug("seqQueryField match '%S'\n", token);
	    ajStrTokenDel(&handle);
	    ajStrDel(&token);
	    return ajTrue;
	}
    }

    ajStrTokenDel(&handle);
    ajStrDel(&token);
    return ajFalse;
}




/* @funcstatic seqUsaRestore **************************************************
**
** Restores a sequence input specification from a SeqPListUsa node
**
** @param [w] seqin [AjPSeqin] Sequence input object
** @param [r] node [const SeqPListUsa] Usa list node
** @return [void]
******************************************************************************/

static void seqUsaRestore(AjPSeqin seqin, const SeqPListUsa node)
{

    seqin->Begin    = node->Begin;
    seqin->End      = node->End;
    seqin->Rev      = node->Rev;
    seqin->Format   = node->Format;
    seqin->Features = node->Features;
    ajStrAssignS(&seqin->Formatstr, node->Formatstr);

    return;
}




/* @funcstatic seqUsaSave *****************************************************
**
** Saves a sequence input specification in a SeqPListUsa node
**
** @param [w] node [SeqPListUsa] Usa list node
** @param [r] seqin [const AjPSeqin] Sequence input object
** @return [void]
******************************************************************************/

static void seqUsaSave(SeqPListUsa node, const AjPSeqin seqin)
{
    node->Begin    = seqin->Begin;
    node->End      = seqin->End;
    node->Rev      = seqin->Rev;
    node->Format   = seqin->Format;
    node->Features = seqin->Features;
    ajStrAssignS(&node->Formatstr, seqin->Formatstr);

    return;
}




/* @funcstatic seqUsaListTrace ************************************************
**
** Traces the nodes in a USA list
**
** @param [r] list [const AjPList] The USA list
** @return [void]
******************************************************************************/

static void seqUsaListTrace(const AjPList list)
{
    AjIList iter;
    SeqPListUsa node;
    ajint i = 0;

    iter = ajListIterRead(list);

    ajDebug("SeqUsaListTrace %d nodes\n", ajListLength(list));
    while(ajListIterMore(iter))
    {
	node = (SeqPListUsa) ajListIterNext(iter);
	ajDebug("%3d: '%S' %4d..%d (%b) '%S' %d\n",
		++i, node->Usa, node->Begin, node->End, node->Rev,
		node->Formatstr, node->Format);
    }

    ajListIterFree(&iter);
    ajDebug("...Done...\n");

    return;
}




/* @funcstatic seqListProcess *************************************************
**
** Processes a file of USAs.
** This function is called by, and calls, seqUsaProcess. There is
** a depth check to avoid infinite loops, for example where a list file
** refers to itself.
**
** This function produces a list (AjPList) of USAs with all list references
** expanded into lists of USAs.
**
** Because USAs in a list can have their own begin, end and reverse settings
** the prior setting are stored with each USA in the list node so that they
** can be restored after.
**
** @param [u] seq [AjPSeq] Sequence
** @param [u] seqin [AjPSeqin] Sequence input
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqListProcess(AjPSeq seq, AjPSeqin seqin, const AjPStr listfile)
{
    AjPList list  = NULL;
    AjPFile file  = NULL;
    AjPStr token  = NULL;
    static AjPStrTok handle = NULL;
    AjBool ret       = ajFalse;
    SeqPListUsa node = NULL;

    static ajint depth    = 0;
    static ajint MAXDEPTH = 16;

    depth++;
    ajDebug("++seqListProcess %S depth %d Rev: %B\n",
	    listfile, depth, seqin->Rev);
    if(depth > MAXDEPTH)
	ajFatal("USA List too deep");

    if(!ajListLength(seqin->List))
	seqin->List = ajListNew();

    list = ajListNew();

    file = ajFileNewIn(listfile);
    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;
	return ret;
    }
    while(ajFileGetsTrim(file, &seqReadLine))
    {
	seqListNoComment(&seqReadLine);
	if(ajStrGetLen(seqReadLine))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \t\n\r");
	    ajStrTokenNextParse(&handle, &token);
	    /* ajDebug("Line  '%S'\ntoken '%S'\n", seqReadLine, token); */
	    if(ajStrGetLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0(node);
	        ajStrAssignS(&node->Usa, token);
	        seqUsaSave(node, seqin);
	        ajListPushApp(list, node);
	    }
	    ajStrDel(&token);
	    token = NULL;
	}
    }
    ajFileClose(&file);

    ajDebug("Trace seqin->List\n");
    seqUsaListTrace(seqin->List);
    ajDebug("Trace new list\n");
    seqUsaListTrace(list);
    ajListPushList(seqin->List, &list);

    ajDebug("Trace combined seqin->List\n");
    seqUsaListTrace(seqin->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(seqin->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Usa);
	ajSeqinUsa(&seqin, node->Usa);
	seqUsaRestore(seqin, node);
	ajStrDel(&node->Usa);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with usa '%S'\n", seqin->Usa);
	ret = seqUsaProcess(seq, seqin);
    }

    ajStrTokenReset(&handle);
    depth--;
    ajDebug("++seqListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic seqListNoComment ***********************************************
**
** Strips comments from a character string (a line from an ACD file).
** Comments are blank lines or any text following a "#" character.
**
** @param [u] text [AjPStr*] Line of text from input file.
** @return [void]
** @@
******************************************************************************/

static void seqListNoComment(AjPStr* text)
{
    ajint i;
    char *cp;

    i = ajStrGetLen(*text);

    if(!i)				/* empty string */
	return;

    cp = strchr(ajStrGetPtr(*text), '#');
    if(cp)
    {					/* comment found */
	*cp = '\0';
	ajStrSetValid(text);
    }

    return;
}




/* @funcstatic seqFormatSet ***************************************************
**
** Sets the input format for a sequence using the sequence input object's
** defined format, or a default from variable 'EMBOSS_FORMAT'.
**
** @param [u] thys [AjPSeq] Sequence.
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqFormatSet(AjPSeq thys, AjPSeqin seqin)
{

    if(ajStrGetLen(seqin->Formatstr))
    {
	ajDebug("... input format value '%S'\n", seqin->Formatstr);
	if(seqFindInFormat(seqin->Formatstr, &seqin->Format))
	{
	    ajStrAssignS(&thys->Formatstr, seqin->Formatstr);
	    thys->Format = seqin->Format;
	    ajDebug("...format OK '%S' = %d\n", seqin->Formatstr,
		    seqin->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n", seqin->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @funcstatic seqinUfoLocal **************************************************
**
** Tests whether a sequence input object will read features from the
** sequence input file. The alternative is to use a separate UFO.
**
** @param [r] thys [const AjPSeqin] Sequence input object.
** @return [AjBool] ajTrue if the features will be read from the sequence
** @@
******************************************************************************/

static AjBool seqinUfoLocal(const AjPSeqin thys)
{
    if(thys->Features && ! ajStrGetLen(thys->Ufo))
	return ajTrue;

    return ajFalse;
}




/* @funcstatic seqSetName *****************************************************
**
** Sets the name for a sequence object by applying simple conversion
** rules to the input which could be, for example, the name from a
** FASTA format file.
**
** @param [u] name [AjPStr*] Sequence name derived.
** @param [r] str [const AjPStr] User supplied name.
** @return [void]
** @@
******************************************************************************/

static void seqSetName(AjPStr* name, const AjPStr str)
{
    AjPStrTok split = NULL;
    AjPStr token = NULL;

    if(ajStrIsWord(str))
    {
	ajDebug("seqSetName word '%S'\n", str);
	split = ajStrTokenNewC(str, ":");
	while(ajStrTokenNextParse(&split, &token))
	{
	    if(ajStrGetLen(token))
	       ajStrAssignS(name, token);
	}
	ajStrTokenDel(&split);
    }
    else
    {
	ajDebug("seqSetName non-word '%S'\n", str);
	ajStrAssignS(name, str);
	ajStrRemoveWhite(name);
	ajStrExchangeKK(name, ' ', '_');
	ajDebug("seqSetName cleaned '%S'\n", *name);
    }

    ajDebug("seqSetName '%S' result: '%S'\n", str, *name);
    ajStrDel(&token);
    return;
}




/* @funcstatic seqSetNameFile *************************************************
**
** Sets the name for a sequence object by applying simple conversion
** rules to the input source file..
**
** @param [u] name [AjPStr*] Sequence name derived.
** @param [r] seqin [const AjPSeqin] Sequence input object
** @return [void]
** @@
******************************************************************************/

static void seqSetNameFile(AjPStr* name, const AjPSeqin seqin)
{
    AjPStr tmpname = NULL;

    ajStrAssignS(&tmpname, seqin->Filename);

    seqSetName(name, tmpname);
    if(seqin->Count > 1)
	ajFmtPrintAppS(name, "_%3d", seqin->Count);

    ajDebug("seqSetNameFile '%S' result: '%S'\n", tmpname, *name);
    ajStrDel(&tmpname);

    return;
}




/* @funcstatic seqAccSave *****************************************************
**
** Adds an accession number to the stored list for a sequence.
** The first accession number is also saved as the primary number.
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] acc [const AjPStr] Accession number
** @return [void]
** @@
******************************************************************************/

static void seqAccSave(AjPSeq thys, const AjPStr acc)
{
    AjPStr liststr;	    /* do not free - it is stored in a list */

    liststr = ajStrNewS(acc);
    ajListstrPushApp(thys->Acclist, liststr);

    if(!ajStrGetLen(thys->Acc))
	ajStrAssignS(&thys->Acc, acc);

    return;
}




/* @funcstatic seqTaxSave *****************************************************
**
** Adds an organism taxonomy level to the stored list for a sequence.
** The first is also saved as the primary 'Tax' (should be the species).
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] tax [const AjPStr] Organism taxonomy
** @return [void]
** @@
******************************************************************************/

static void seqTaxSave(AjPSeq thys, const AjPStr tax)
{
    AjPStr liststr;	    /* do not free - it is stored in a list */

    liststr = ajStrNewS(tax);
    ajListstrPushApp(thys->Taxlist, liststr);

    if(!ajStrGetLen(thys->Tax))
	ajStrAssignS(&thys->Tax, tax);

    return;
}




/* @funcstatic seqSvSave ******************************************************
**
** Adds a sequence version number to the stored data for a sequence.
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] sv [const AjPStr] SeqVersion number
** @return [void]
** @@
******************************************************************************/

static void seqSvSave(AjPSeq thys, const AjPStr sv)
{
    if(!ajStrGetLen(thys->Sv))
	ajStrAssignS(&thys->Sv, sv);

    return;
}




/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Sequence Query Constructors ***************************************
**
** All constructors return a new sequence query object by pointer. It
** is the responsibility of the user to first destroy any previous
** sequenceoutput object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
******************************************************************************/




/* @func ajSeqQueryNew ********************************************************
**
** Creates a new sequence query object
**
** @return [AjPSeqQuery] New sequence query object.
** @category new [AjPSeqQuery] Default constructor
** @@
******************************************************************************/

AjPSeqQuery ajSeqQueryNew(void)
{
    AjPSeqQuery pthis;

    AJNEW0(pthis);

    pthis->DbName = ajStrNew();
    pthis->DbAlias= ajStrNew();
    pthis->Id     = ajStrNew();
    pthis->Acc    = ajStrNew();
    pthis->Sv     = ajStrNew();
    pthis->Gi     = ajStrNew();
    pthis->Des    = ajStrNew();
    pthis->Org    = ajStrNew();
    pthis->Key    = ajStrNew();

    pthis->Wild   = ajFalse;

    pthis->Method      = ajStrNew();
    pthis->Formatstr   = ajStrNew();
    pthis->IndexDir    = ajStrNew();
    pthis->Directory   = ajStrNew();
    pthis->Filename    = ajStrNew();
    pthis->Application = ajStrNew();
    pthis->Field       = ajStrNew();

    pthis->Type    = QRY_UNKNOWN;
    pthis->Access  = NULL;
    pthis->QryData = NULL;
    pthis->Fpos    = NULLFPOS;
    pthis->QryDone = ajFalse;

    return pthis;
}




/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Sequence Query Destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence query object.
**
******************************************************************************/




/* @func ajSeqQueryDel ********************************************************
**
** Deletes a sequence query object
**
** @param [d] pthis [AjPSeqQuery*] Address of sequence query object
** @return [void]
** @category delete [AjPSeqQuery] Default destructor
** @@
******************************************************************************/

void ajSeqQueryDel(AjPSeqQuery* pthis)
{
    AjPSeqQuery thys;

    ajDebug("ajSeqQueryDel db:'%S' id:'%S'\n", (*pthis)->DbName, (*pthis)->Id);

    thys = *pthis;

    ajStrDel(&thys->DbName);
    ajStrDel(&thys->DbAlias);
    ajStrDel(&thys->DbType);
    ajStrDel(&thys->Id);
    ajStrDel(&thys->Acc);
    ajStrDel(&thys->Des);
    ajStrDel(&thys->Key);
    ajStrDel(&thys->Org);
    ajStrDel(&thys->Sv);
    ajStrDel(&thys->Gi);
    ajStrDel(&thys->Method);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->IndexDir);
    ajStrDel(&thys->Directory);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Exclude);
    ajStrDel(&thys->DbFields);
    ajStrDel(&thys->DbProxy);
    ajStrDel(&thys->DbHttpVer);
    ajStrDel(&thys->Field);
    ajStrDel(&thys->QryString);
    ajStrDel(&thys->Application);
 
    if(thys->QryData)
    {
	if(thys->Access->AccessFree)
	    thys->Access->AccessFree(thys);

	AJFREE(thys->QryData);
    }

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Query Assignments ****************************************
**
** These functions overwrite the sequence query object provided as
** the first argument.
**
******************************************************************************/




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Sequence Query Modifiers ******************************************
**
** These functions use the contents of a sequence query object and
** update them.
**
******************************************************************************/




/* @func ajSeqQueryClear ******************************************************
**
** Resets a Sequence query object to a clean state for reuse
**
** @param [u] thys [AjPSeqQuery] Sequence query object
** @return [void]
** @category modify [AjPSeqQuery] Clears all contents
** @@
******************************************************************************/

void ajSeqQueryClear(AjPSeqQuery thys)
{

    ajStrSetClear(&thys->DbName);
    ajStrSetClear(&thys->DbAlias);
    ajStrSetClear(&thys->Id);
    ajStrSetClear(&thys->Acc);
    ajStrSetClear(&thys->Sv);
    ajStrSetClear(&thys->Gi);
    ajStrSetClear(&thys->Des);
    ajStrSetClear(&thys->Org);
    ajStrSetClear(&thys->Key);
    ajStrSetClear(&thys->Method);
    ajStrSetClear(&thys->Formatstr);
    ajStrSetClear(&thys->IndexDir);
    ajStrSetClear(&thys->Directory);
    ajStrSetClear(&thys->Filename);
    ajStrSetClear(&thys->Application);

    thys->Type   = QRY_UNKNOWN;
    thys->Access = NULL;
    if(thys->QryData)
	AJFREE(thys->QryData);

    thys->QryDone = ajFalse;

    return;
}




/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Query Operators ******************************************
**
** These functions use the contents of a sequence query object but do
** not make any changes.
**
******************************************************************************/




/* @funcstatic seqQueryMatch **************************************************
**
** Compares a sequence to a query and returns true if they match.
**
** @param [r] thys [const AjPSeqQuery] Sequence query.
** @param [r] seq [const AjPSeq] Sequence.
** @return [AjBool] ajTrue if the sequence matches the query.
** @category use [AjPSeqQuery] Compares an AjPSeq to a query.
** @@
******************************************************************************/

static AjBool seqQueryMatch(const AjPSeqQuery thys, const AjPSeq seq)
{
    AjBool tested = ajFalse;
    AjIList iter  = NULL;
    AjPStr accstr;			/* from list, do not delete */
    AjPStr keystr;			/* from list, do not delete */
    AjPStr taxstr;			/* from list, do not delete */
    AjBool ok = ajFalse;

    ajDebug("seqQueryMatch '%S' id '%S' acc '%S' Sv '%S' Gi '%S' Des '%S'"
	    " Key '%S' Org '%S'\n",
	    seq->Name, thys->Id, thys->Acc, thys->Sv, thys->Gi,
	    thys->Des, thys->Key, thys->Org);

    if(!thys)			   /* no query to test, that's fine */
	return ajTrue;

    if(thys->QryDone)			/* do we need to test here? */
	return ajTrue;

    /* test the query field(s) */

    if(ajStrGetLen(thys->Id))
    {
	if(ajStrMatchWildS(seq->Name, thys->Id))
	    return ajTrue;

	ajDebug("id test failed\n");
	tested = ajTrue;
	ok = ajFalse;
    }

    if(ajStrGetLen(thys->Sv)) /* test Sv and Gi */
    {
	if(ajStrMatchWildS(seq->Sv, thys->Sv))
	    return ajTrue;

	ajDebug("sv test failed\n");
	tested = ajTrue;
	ok = ajFalse;
    }

    if(ajStrGetLen(thys->Gi)) /* test Sv and Gi */
    {
	if(ajStrMatchWildS(seq->Gi, thys->Gi))
	    return ajTrue;

	ajDebug("gi test failed\n");
	tested = ajTrue;
	ok = ajFalse;
    }

    if(!ajStrGetLen(thys->Acc))
    {
	/*ajDebug("No accession number to test\n");*/
    }
    else if(ajListLength(seq->Acclist))
    {		   /* accession number test - check the entire list */
	iter = ajListIterRead(seq->Acclist);
	while(ajListIterMore(iter))
	{
	    accstr = ajListIterNext(iter);
	    ajDebug("... try accession '%S' '%S'\n", accstr,
		    thys->Acc);

	    if(ajStrMatchWildS(accstr, thys->Acc))
		return ajTrue;
	}
	tested = ajTrue;
	ajDebug("acc test failed\n");
	ajListIterFree(&iter);
    }

    if(!ajStrGetLen(thys->Org))
    {
	/*ajDebug("No taxonomy to test\n"); */
    }
    else if(ajListLength(seq->Taxlist))
    {			   /* taxonomy test - check the entire list */
	iter = ajListIterRead(seq->Taxlist);
	while(ajListIterMore(iter))
	{
	    taxstr = ajListIterNext(iter);
	    ajDebug("... try organism '%S' '%S'\n", taxstr,
		    thys->Org);

	    if(ajStrMatchWildS(taxstr, thys->Org))
		return ajTrue;
	}
	tested = ajTrue;
	ajDebug("org test failed\n");
	ajListIterFree(&iter);
    }
    else
    {
	ajDebug("org test failed - nothing to test\n");
	return ajFalse;
    }

    if(!ajStrGetLen(thys->Key))
    {
	/*ajDebug("No keyword to test\n");*/
    }
    else if(ajListLength(seq->Keylist))
    {			    /* keyword test - check the entire list */
	iter = ajListIterRead(seq->Keylist);
	while(ajListIterMore(iter))
	{
	    keystr = ajListIterNext(iter);
	    ajDebug("... try keyword '%S' '%S'\n", keystr,
		    thys->Key);

	    if(ajStrMatchWildS(keystr, thys->Key))
		return ajTrue;
	}
	tested = ajTrue;
	ajDebug("key test failed\n");
	ajListIterFree(&iter);
    }
    else
    {
	ajDebug("key test failed - nothing to test\n");
	return ajFalse;
    }

    if(!ajStrGetLen(thys->Des))
    {
	/*ajDebug("No description to test\n");*/
	ok = ajFalse;
    }
    else if(ajStrGetLen(seq->Desc))
    {			     /* description test - check the string */
	ajDebug("... try description '%S' '%S'\n", seq->Desc,
		thys->Des);

	if(ajStrMatchWildWordS(seq->Desc, thys->Des))
	    return ajTrue;

	tested = ajTrue;
	ajDebug("des test failed\n");
	ajListIterFree(&iter);
    }
    else
    {
	ajDebug("des test failed - nothing to test\n");
	return ajFalse;
    }

    if(!tested)		    /* nothing to test, so accept it anyway */
	return ajTrue;

    ajDebug("result: %B\n", ok);

    return ok;
}




/* @func ajSeqQueryWild *******************************************************
**
** Tests whether a query includes wild cards in any element,
** or can return more than one entry (keyword and some other search terms
** will find multiple entries)
**
** @param [u] qry [AjPSeqQuery] Query object.
** @return [AjBool] ajTrue if query had wild cards.
** @category modify [AjPSeqQuery] Tests whether a query includes wildcards
** @@
******************************************************************************/

AjBool ajSeqQueryWild(AjPSeqQuery qry)
{

    if(!seqRegQryWild)
	seqQryWildComp();

    ajDebug("ajSeqQueryWild id '%S' acc '%S' sv '%S' gi '%S' des '%S'"
	    " org '%S' key '%S'\n",
	    qry->Id, qry->Acc, qry->Sv, qry->Gi, qry->Des, qry->Org, qry->Key);

    if(ajRegExec(seqRegQryWild, qry->Id))
    {
	ajDebug("wild query Id '%S'\n", qry->Id);
	qry->Wild = ajTrue;
	return ajTrue;
    }

    if(ajStrGetLen(qry->Acc))
    {
	if(strpbrk(qry->Acc->Ptr,"*?"))
	    qry->Wild = ajTrue;
	
        if(!ajStrGetLen(qry->Id))
	{
	    ajDebug("wild (has, but no Id) query Acc '%S'\n", qry->Acc);
	    return ajTrue;
	}
	else if(ajRegExec(seqRegQryWild, qry->Id))
        {
	    ajDebug("wild query Acc '%S'\n", qry->Acc);
	    return ajTrue;
	}
    }

    if(ajStrGetLen(qry->Sv))
    {
	if(strpbrk(qry->Sv->Ptr,"*?"))
	    qry->Wild = ajTrue;

	ajDebug("wild (has) query Sv '%S'\n", qry->Sv);
	return ajTrue;
    }

    if(ajStrGetLen(qry->Gi))
    {
	if(strpbrk(qry->Gi->Ptr,"*?"))
	    qry->Wild = ajTrue;

	if(!ajStrIsNum(qry->Gi))
	{
	    ajDebug("wild (has) query Gi '%S'\n", qry->Gi);
	    return ajTrue;
	}
    }

    if(ajStrGetLen(qry->Des))
    {
	if(strpbrk(qry->Des->Ptr,"*?"))
	    qry->Wild = ajTrue;

	ajDebug("wild (has) query Des '%S'\n", qry->Des);
	return ajTrue;
    }

    if(ajStrGetLen(qry->Org))
    {
	if(strpbrk(qry->Org->Ptr,"*?"))
	    qry->Wild = ajTrue;

	ajDebug("wild (has) query Org '%S'\n", qry->Org);
	return ajTrue;
    }

    if(ajStrGetLen(qry->Key))
    {
	if(strpbrk(qry->Key->Ptr,"*?"))
	    qry->Wild = ajTrue;

	ajDebug("wild (has) query Key '%S'\n", qry->Key);
	return ajTrue;
    }

    ajDebug("no wildcard in stored qry\n");

    return ajFalse;
}




/* @func ajSeqQueryStarclear **************************************************
**
** Clears elements of a query object if they are simply "*" because this
** is equivalent to a null string.
**
** @param [u] qry [AjPSeqQuery] Query object.
** @return [void]
** @category modify [AjPSeqQuery] Clears fully wild elements of a query
**                 because empty elements are the same.
** @@
******************************************************************************/

void ajSeqQueryStarclear(AjPSeqQuery qry)
{
    if(ajStrMatchC(qry->Id, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Id '%S'\n", qry->Id);
	ajStrSetClear(&qry->Id);
    }

    if(ajStrMatchC(qry->Acc, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Acc '%S'\n", qry->Acc);
	ajStrSetClear(&qry->Acc);
    }

    if(ajStrMatchC(qry->Sv, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Sv '%S'\n", qry->Sv);
	ajStrSetClear(&qry->Sv);
    }

    if(ajStrMatchC(qry->Gi, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Gi '%S'\n", qry->Gi);
	ajStrSetClear(&qry->Gi);
    }

    if(ajStrMatchC(qry->Des, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Des '%S'\n", qry->Des);
	ajStrSetClear(&qry->Des);
    }

    if(ajStrMatchC(qry->Org, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Org '%S'\n", qry->Org);
	ajStrSetClear(&qry->Org);
    }

    if(ajStrMatchC(qry->Key, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Key '%S'\n", qry->Key);
	ajStrSetClear(&qry->Key);
    }

    return;
}




/* @func ajSeqQueryIs *********************************************************
**
** Tests whether any element of a query has been set. Elements which
** are simply '*' are cleared as this has the same meaning.
**
** @param [r] qry [const AjPSeqQuery] Query object.
** @return [AjBool] ajTrue if query should be made. ajFalse if the query
**                  includes all entries.
** @category use [AjPSeqQuery] Tests whether a query has been defined
** @@
******************************************************************************/

AjBool ajSeqQueryIs(const AjPSeqQuery qry)
{

    if(ajStrGetLen(qry->Id))
	return ajTrue;
    if(ajStrGetLen(qry->Acc))
	return ajTrue;
    if(ajStrGetLen(qry->Sv))
	return ajTrue;
    if(ajStrGetLen(qry->Gi))
	return ajTrue;
    if(ajStrGetLen(qry->Des))
	return ajTrue;
    if(ajStrGetLen(qry->Org))
	return ajTrue;
    if(ajStrGetLen(qry->Key))
	return ajTrue;

    return ajFalse;
}




/* @funcstatic seqQryWildComp *************************************************
**
** Compiles the reqular expressions for testing wild cards in queries.
** These are held in static storage and built once only if needed.
**
** @return [void]
** @@
******************************************************************************/

static void seqQryWildComp(void)
{
    if(!seqRegQryWild)
	seqRegQryWild = ajRegCompC("[*?]");

    return;
}




/* @func ajSeqQueryTrace ******************************************************
**
** Debug calls to trace the data in a sequence query object.
**
** @param [r] thys [const AjPSeqQuery] Sequence query object.
** @return [void]
** @@
******************************************************************************/

void ajSeqQueryTrace(const AjPSeqQuery thys)
{
    ajDebug( "  Query Trace\n");

    if(ajStrGetLen(thys->DbName))
	ajDebug( "    DbName: '%S'\n", thys->DbName);

    if(ajStrGetLen(thys->DbAlias))
	ajDebug( "    DbAlias: '%S'\n", thys->DbAlias);

    if(ajStrGetLen(thys->DbType))
	ajDebug( "    DbType: '%S' (%d)\n", thys->DbType, thys->Type);
    ajDebug( "   QryDone: %B\n", thys->QryDone);

    if(ajStrGetLen(thys->Id))
	ajDebug( "    Id: '%S'\n", thys->Id);

    if(ajStrGetLen(thys->Acc))
	ajDebug( "    Acc: '%S'\n", thys->Acc);

    if(ajStrGetLen(thys->Sv))
	ajDebug( "    Sv: '%S'\n", thys->Sv);

    if(ajStrGetLen(thys->Gi))
	ajDebug( "    Gi: '%S'\n", thys->Gi);

    if(ajStrGetLen(thys->Des))
	ajDebug( "    Des: '%S'\n", thys->Des);

    if(ajStrGetLen(thys->Key))
	ajDebug( "    Key: '%S'\n", thys->Key);

    if(ajStrGetLen(thys->Org))
	ajDebug( "    Org: '%S'\n", thys->Org);

    if(ajStrGetLen(thys->Method))
	ajDebug( "    Method: '%S'\n", thys->Method);

    if(ajStrGetLen(thys->Formatstr))
	ajDebug( "    Formatstr: '%S'\n", thys->Formatstr);

    if(ajStrGetLen(thys->IndexDir))
	ajDebug( "    IndexDir: '%S'\n", thys->IndexDir);

    if(ajStrGetLen(thys->Directory))
	ajDebug( "    Directory: '%S'\n", thys->Directory);

    if(ajStrGetLen(thys->Filename))
	ajDebug( "    Filename: '%S'\n", thys->Filename);

    if(ajStrGetLen(thys->Exclude))
	ajDebug( "    Exclude: '%S'\n", thys->Exclude);

    if(ajStrGetLen(thys->Application))
	ajDebug( "    Application: '%S'\n", thys->Application);

    if(thys->Access)
	ajDebug( "    Access: exists\n");

    if(thys->QryData)
	ajDebug( "    QryData: exists\n");

    return;
}




/* ==================================================================== */
/* ============================ Casts ================================= */
/* ==================================================================== */

/* @section Sequence Query Casts **********************************************
**
** These functions examine the contents of a sequence query object
** and return some derived information. Some of them provide access to
** the internal components of a sequence query object. They are
** provided for programming convenience but should be used with
** caution.
**
******************************************************************************/




/* @func ajSeqParseFasta ******************************************************
**
** Parse an NCBI format fasta line. Return id acc sv and description
**
** @param [r] instr [const AjPStr]   fasta line.
** @param [w] id [AjPStr*]   id.
** @param [w] acc [AjPStr*]  accession number.
** @param [w] sv [AjPStr*]  sequence version number.
** @param [w] desc [AjPStr*] description.
** @return [AjBool] ajTrue if fasta format
** @@
******************************************************************************/

AjBool ajSeqParseFasta(const AjPStr instr, AjPStr* id, AjPStr* acc,
		       AjPStr* sv, AjPStr* desc)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    AjBool ok = ajFalse;

    ajDebug("ajSeqParseFasta '%S'\n", instr);

    if(!ajStrPrefixC(instr, ">"))
	return ajFalse;

    ajStrAssignS(&str, instr);

    ajStrTokenAssignC(&handle, str, "> ");
    ajStrTokenNextParseC(&handle, " \t\n\r", id);

    ok = ajStrTokenNextParse(&handle, &token);

    if(ok && ajIsSeqversion(token))
    {
        ajStrAssignS(acc, ajIsSeqversion(token));
	ajStrAssignS(sv, token);
	ajStrTokenNextParseC(&handle, "\n\r", desc);
    }
    else if(ok && ajIsAccession(token))
    {
	ajStrAssignS(acc, token);
        ajStrAssignC(sv, "");
	ajStrTokenNextParseC(&handle, "\n\r", desc);
    }
    else if(ok)
    {
        ajStrAssignC(acc, "");
        ajStrAssignC(sv, "");
	ajStrAssignS(desc, token);
	if(ajStrTokenNextParseC(&handle, "\n\r", &token))
	{
	    ajStrAppendC(desc, " ");
	    ajStrAppendS(desc, token);
	}
    }

    ajStrDel(&token); /* duplicate of accession or description */
    ajStrTokenDel(&handle);
    ajStrDel(&str);
    ajDebug("result id: '%S' acc: '%S' desc: '%S'\n", *id, *acc, *desc);

    return ajTrue;
}




/* @func ajSeqParseNcbi *******************************************************
**
** Parse an NCBI format fasta line. Return id acc and description.
**
** Tries to cope with the amazing variety of identifiers NCBI inflicts
** on us all - see the BLAST document README.formatdb from NCBI for
** some of the gory detail, and look at some real files for clues
** to what can really happen. Sadly,' real files' also includes
** internal IDs in blast databases reformatted by formatdb.
**
** @param [r] instr [const AjPStr]   fasta line.
** @param [w] id [AjPStr*]   id.
** @param [w] acc [AjPStr*]  accession number.
** @param [w] sv [AjPStr*]  sequence version number.
** @param [w] gi [AjPStr*]  GI version number.
** @param [w] desc [AjPStr*] description.
** @return [AjBool] ajTrue if ncbi format
** @@
******************************************************************************/

AjBool ajSeqParseNcbi(const AjPStr instr, AjPStr* id, AjPStr* acc,
		      AjPStr* sv, AjPStr* gi, AjPStr* desc)
{
    AjPStrTok idhandle = NULL;
    AjPStrTok handle   = NULL;
    AjPStr idstr       = NULL;
    AjPStr reststr     = NULL;
    AjPStr prefix      = NULL;
    AjPStr token       = NULL;
    AjPStr numtoken    = NULL;
    AjPStr str         = NULL;
    const AjPStr vacc  = NULL;
    const char *q;
    ajint  i;
    ajint  nt;
    AjBool ret = ajFalse;

    /* NCBI's list of standard identifiers June 2001
     ** ftp://ncbi.nlm.nih.gov/blast/db/README.formatdb
     **
     ** Database Name                         Identifier Syntax
     **
     ** GenBank                               gb|accession|locus
     ** EMBL Data Library                     emb|accession|locus
     ** DDBJ, DNA Database of Japan           dbj|accession|locus
     ** SWISS-PROT                            sp|accession|entry name
     ** NCBI Reference Sequence               ref|accession|locus
     **
     ** General database identifier           gnl|database|identifier
     ** BLAST formatdb                        gnl|BL_ORD_ID|number
     **   (prefix for normal FASTA header - remove)
     **
     ** NBRF PIR                              pir||entry
     ** Protein Research Foundation           prf||name
     **   (Japanese SEQDB protein DB)
     **
     ** Brookhaven Protein Data Bank          pdb|entry|chain
     **
     ** Patents                               pat|country|number
     **
     ** GenInfo Backbone Id                   bbs|number
     ** Local Sequence identifier             lcl|identifier
     **
     ** GenInfo identifier prefix             gi|gi_identifier
     **   (prefix - remove)
     */

    ajDebug("ajSeqParseNcbi '%S'\n", instr);

    if(ajStrGetCharPos(instr, 3) == ';')  /* then it is really PIR format */
    {
	ajDebug("ajSeqParseNcbi failed: this is PIR format\n");
	return ajFalse;
    }

    ajStrAssignS(&str, instr);

    /* ajDebug("id test %B %B\n",
       !strchr(MAJSTRGETPTR(str), (ajint)'|'),
       (*MAJSTRGETPTR(str)!='>')); */

    /* Line must start with '>', and include '|' bar, hopefully in the ID */

    if(*MAJSTRGETPTR(str)!='>')
    {
	ajDebug("ajSeqParseNcbi failed: no '>' at start\n");
	ajStrDel(&str);
	return ajFalse;
    }

    /* pick out the ID */

    ajStrTokenAssignC(&idhandle,str,"> \t\r\n");
    ajStrTokenNextParse(&idhandle, &idstr);
    ajStrTokenNextParseC(&idhandle, "\r\n", &reststr);
    ajStrTokenDel(&idhandle);

    /* check we have an ID */

    if(!ajStrGetLen(idstr))
    {
	ajDebug("No ID string found - but try FASTA\n");
	ret = ajSeqParseFasta(str, id, acc, sv, desc);
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	return ret;
    }

    /* NCBI ids always have | somewhere. Else we try a simple FASTA format */

    if(!strchr(MAJSTRGETPTR(idstr),(ajint)'|'))
    {
	ajDebug("trying ajSeqParseFasta\n");
	ret = ajSeqParseFasta(str, id, acc, sv, desc);
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	return ret;
    }

    ajStrTokenAssignC(&handle,idstr,"|");

    ajStrTokenNextParse(&handle, &prefix);
    q = MAJSTRGETPTR(prefix);

    /* ajDebug(" idstr: '%S'\n", idstr); */
    /* ajDebug("prefix: '%S'\n", prefix); */

    if(!strncmp(q,"gi",2))
    {
        /* ajDebug("gi prefix\n"); */
	ajStrTokenNextParse(&handle, &token);
	ajStrAssignS(gi, token);
	if(! ajStrTokenNextParse(&handle, &prefix))
	{
	    /* we only have a gi prefix */
	    /* ajDebug("*only* gi prefix\n"); */
	    ajStrAssignS(id, token);
	    ajStrAssignC(acc, "");
	    ajStrAssignS(desc, reststr);
	    /* ajDebug("found pref: '%S' id: '%S', acc: '%S' "
	       "desc: '%S'\n",
	       prefix, *id, *acc, *desc); */
	    ajStrDel(&str);
	    ajStrDel(&idstr);
	    ajStrDel(&reststr);
	    ajStrDel(&prefix);
	    ajStrDel(&token);
	    ajStrTokenDel(&handle);
	    return ajTrue;
	}

	/* otherwise we continue to parse the rest */
	q = MAJSTRGETPTR(prefix);
	/* ajDebug("continue with '%S'\n", prefix); */
    }


    /*
     * This next routine and associated function could be used if
     * whatever is appended to gnl lines is consistent
     */

    if(!strncmp(MAJSTRGETPTR(idstr),"gnl|BL_ORD_ID|",14))
    {
        /* ajDebug("gnl|BL_ORD_ID stripping\n"); */
	ajStrTokenNextParse(&handle, &token); /* BL_ORD_ID */
	ajStrTokenNextParse(&handle, &numtoken); /* number */
	ajStrInsertC(&reststr, 0, ">");

	if(ajSeqParseNcbi(reststr,id,acc,sv,gi,desc))
	{
	    /* recursive ... */
	    /* ajDebug("ajSeqParseNcbi recursive success\n"); */
	    /* ajDebug("found pref: '%S' id: '%S', acc: '%S' "
	       "sv: '%S' desc: '%S'\n",
	       prefix, *id, *acc, *sv, *desc); */
	    ajStrDel(&str);
	    ajStrDel(&idstr);
	    ajStrDel(&reststr);
	    ajStrDel(&prefix);
	    ajStrDel(&numtoken);
	    ajStrDel(&token);
	    ajStrTokenDel(&handle);
	    return ajTrue;
	}
        /* ajDebug("ajSeqParseNcbi recursive failed - use gnl id\n"); */
	ajStrAssignS(id,numtoken);
	ajStrAssignC(acc,"");
	/* ajDebug("found pref: '%S' id: '%S', acc: '%S' "
	   "sv: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *sv, *desc); */
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajTrue;
    }

    /* works for NCBI formatdb reformatted blast databases
     ** still checking for any misformatted databases elsewhere */

    if(!strcmp(q,"bbs") || !strcmp(q,"lcl"))
    {
        /* ajDebug("bbs or lcl prefix\n"); */
	ajStrTokenNextParse(&handle, id);
	ajStrAssignC(acc,"");
	ajStrAssignS(desc, reststr);
	/* ajDebug("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajTrue;
    }

    if(!strcmp(q,"gnl") || !strcmp(q,"pat"))
    {
        /* ajDebug("gnl or pat or pdb prefix\n"); */
	ajStrTokenNextParse(&handle, &token);
	ajStrTokenNextParse(&handle, id);
	ajStrAssignC(acc,"");		/* no accession number */
	ajStrAssignS(desc, reststr);
	/* ajDebug("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajTrue;
    }


    if(!strcmp(q,"pdb"))
    {
        /* ajDebug("gnl or pat or pdb prefix\n"); */
	ajStrTokenNextParse(&handle, id);
	if(ajStrTokenNextParse(&handle, &token))
	{
	    /* chain identifier to append */
	    ajStrAppendS(id, token);
	}
	ajStrAssignC(acc,"");		/* no accession number */
	ajStrAssignS(desc, reststr);
	/* ajDebug("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajTrue;
    }


    if(!strcmp(q,"gb") || !strcmp(q,"emb") || !strcmp(q,"dbj")
       || !strcmp(q,"sp") || !strcmp(q,"ref"))
    {
        /* ajDebug("gb,emb,dbj,sp,ref prefix\n"); */
	ajStrTokenNextParse(&handle, &token);
	vacc = ajIsSeqversion(token);
	if(vacc)
	{
	    ajStrAssignS(sv,token);
	    ajStrAssignS(acc,vacc);
	}
	else if(ajIsAccession(token))
	    ajStrAssignS(acc,token);

	if(!ajStrTokenNextParse(&handle, id))
	{
	    /* no ID, reuse accession token */
	    ajStrAssignS(id, token);
	}
	ajStrAssignS(desc, reststr);
	/* ajDebug("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajTrue;
    }


    if(!strcmp(q,"pir") || !strcmp(q,"prf"))
    {
        /* ajDebug("pir,prf prefix\n"); */
	ajStrTokenNextParse(&handle, id);
	ajStrAssignS(desc, reststr);
	ajStrAssignC(acc, "");
	/* ajDebug("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajTrue;
    }


    /* else assume that the last two barred tokens contain [acc]|id */

    /* ajDebug("No prefix accepted - try the last 2 fields\n"); */

    nt = ajStrParseCountC(idstr,"|");

    /* ajDebug("Barred tokens - %d found\n", nt); */

    if(nt < 2)
    {
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajFalse;
    }

    /* restart parsing with only bars */

    ajStrTokenAssignC(&handle,str,"|");
    for(i=0;i<nt-2;++i)
	ajStrTokenNextParse(&handle, &token);

    ajStrTokenNextParse(&handle, &token);
    /* ajDebug("token acc: '%S'\n", token); */
    vacc = ajIsSeqversion(token);
    if(vacc)
    {
	ajStrAssignS(sv,token);
	ajStrAssignS(acc,vacc);
    }
    else if(ajIsAccession(token))
	ajStrAssignS(acc,token);

    ajStrTokenNextParseC(&handle, " \n\t\r", &token);
    /* ajDebug("token id: '%S'\n", token); */

    ajStrAssignS(id,token);

    ajStrTokenNextParseC(&handle, "\n\r", &token);
    ajStrAssignS(desc, token);
    ajStrTokenDel(&handle);
    ajStrDel(&token);
    /* ajDebug("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
       prefix, *id, *acc, *desc); */

    ajStrDel(&str);
    ajStrDel(&idstr);
    ajStrDel(&reststr);
    ajStrDel(&prefix);
    ajStrDel(&numtoken);
    ajStrDel(&token);

    return ajTrue;
}




/* @func ajSeqGetFromUsa ******************************************************
**
** Returns a sequence given a USA
**
** @param [r] thys [const AjPStr] USA
** @param [r] protein [AjBool] True if protein
** @param [w] seq [AjPSeq*] sequence
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajSeqGetFromUsa(const AjPStr thys, AjBool protein, AjPSeq *seq)
{
    AjPSeqin seqin;
    AjBool ok;

    seqin        = ajSeqinNew();
    seqin->multi = ajFalse;
    seqin->Text  = ajFalse;

    if(!protein)
	ajSeqinSetNuc(seqin);
    else
	ajSeqinSetProt(seqin);

    ajSeqinUsa(&seqin, thys);
    ok = ajSeqRead(*seq, seqin);
    ajSeqinDel(&seqin);

    if(!ok)
	return ajFalse;

    return ajTrue;
}




/* @func ajSeqsetGetFromUsa *****************************************
**
** Return a seqset given a usa
**
** @param [r] thys [const AjPStr] usa
** @param [w] seq [AjPSeqset*] seqset
** @return [AjBool] ajTrue on success
******************************************************************************/

AjBool ajSeqsetGetFromUsa(const AjPStr thys, AjPSeqset *seq)
{
    AjPSeqin seqin;
    AjBool ok;

    seqin        = ajSeqinNew();
    seqin->multi = ajTrue;
    seqin->Text  = ajFalse;

    ajSeqinUsa(&seqin, thys);
    ok = ajSeqsetRead(*seq, seqin);
    ajSeqinDel(&seqin);


    if(!ok)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic seqTextSeq *****************************************************
**
** Saves a sequence from a string into the text output pointer
**
** Could do some extra formatting here (left margin, numbering)
** but as the EMBOSS formats are not too fussy that can wait.
**
** @param [w] textptr [AjPStr*] Text output
** @param [r] seq [const AjPStr] sequence as a string
** @return [void]
******************************************************************************/

static void seqTextSeq(AjPStr* textptr, const AjPStr seq)
{
    ajint i;
    ajint istart;
    ajint iend;
    ajint ilen;
    static ajint iwidth;
    static AjPStr tmpstr = NULL;

    ilen = ajStrGetLen(seq);
    iwidth = 60;

    for(i=0; i < ilen; i += iwidth)
    {
	istart = i;
	iend = AJMIN(ilen-1, istart+iwidth-1);
	ajStrAssignSubS(&tmpstr, seq, istart, iend);
	ajFmtPrintAppS(textptr, "%S\n", tmpstr);
    }

    return;
}




/* @func ajSeqReadExit ********************************************************
**
** Cleans up sequence reading internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajSeqReadExit(void)
{
    ajRegFree(&seqRegQryWild);

    /* USA processing regular expressions */
    ajRegFree(&seqRegUsaAsis);
    ajRegFree(&seqRegUsaDb);
    ajRegFree(&seqRegUsaFmt);
    ajRegFree(&seqRegUsaId);
    ajRegFree(&seqRegUsaList);
    ajRegFree(&seqRegUsaRange);
    ajRegFree(&seqRegUsaWild);

    /* sequence reading regular expressions */
    ajRegFree(&seqRegGcgDot);
    ajRegFree(&seqRegGcgChk);
    ajRegFree(&seqRegGcgLen);
    ajRegFree(&seqRegGcgNam);
    ajRegFree(&seqRegGcgTyp);
    ajRegFree(&seqRegGcgMsf);
    ajRegFree(&seqRegGcgMsflen);
    ajRegFree(&seqRegGcgMsfnam);
    ajRegFree(&seqRegGcgWgt);
    ajRegFree(&seqRegNbrfId);
    ajRegFree(&seqRegStadenId);
    ajRegFree(&seqRegHennigBlank);
    ajRegFree(&seqRegHennigSeq);
    ajRegFree(&seqRegHennigTop);
    ajRegFree(&seqRegHennigHead);
    ajRegFree(&seqRegFitchHead);
    ajRegFree(&seqRegStockholmSeq);
    ajRegFree(&seqRegAbiDots);
    ajRegFree(&seqRegRawNonseq);
    ajRegFree(&seqRegPhylipTop);
    ajRegFree(&seqRegPhylipHead);
    ajRegFree(&seqRegPhylipSeq);
    ajRegFree(&seqRegPhylipSeq2);

    /* sequence reading strings */
    ajStrDel(&seqFtFmtEmbl);
    ajStrDel(&seqFtFmtGenbank);
    ajStrDel(&seqFtFmtGff);
    ajStrDel(&seqFtFmtPir);
    ajStrDel(&seqFtFmtSwiss);
    ajStrDel(&seqUsaTest);
    ajStrDel(&seqQryChr);
    ajStrDel(&seqQryDb);
    ajStrDel(&seqQryList);

    ajStrDel(&seqReadLine);

    return;
}
/* @func ajSeqinTrace *********************************************************
**
** Debug calls to trace the data in a sequence input object.
**
** @param [r] thys [const AjPSeqin] Sequence input object.
** @return [void]
** @@
******************************************************************************/

void ajSeqinTrace(const AjPSeqin thys)
{
    ajDebug("Sequence input trace\n");
    ajDebug( "====================\n\n");
    ajDebug( "  Name: '%S'\n", thys->Name);

    if(ajStrGetLen(thys->Acc))
	ajDebug( "  Accession: '%S'\n", thys->Acc);

    if(ajStrGetLen(thys->Inputtype))
	ajDebug( "  Inputtype: '%S'\n", thys->Inputtype);

    if(ajStrGetLen(thys->Desc))
	ajDebug( "  Description: '%S'\n", thys->Desc);

    if(ajStrGetLen(thys->Inseq))
	ajDebug( "  Inseq len: %d\n", ajStrGetLen(thys->Inseq));

    if(thys->Rev)
	ajDebug( "     Rev: %B\n", thys->Rev);

    if(thys->Begin)
	ajDebug( "   Begin: %d\n", thys->Begin);

    if(thys->End)
	ajDebug( "     End: %d\n", thys->End);

    if(ajStrGetLen(thys->Db))
	ajDebug( "  Database: '%S'\n", thys->Db);

    if(ajStrGetLen(thys->Full))
	ajDebug( "  Full name: '%S'\n", thys->Full);

    if(ajStrGetLen(thys->Date))
	ajDebug( "  Date: '%S'\n", thys->Date);

    if(ajListLength(thys->List))
	ajDebug( "  List: (%d)\n", ajListLength(thys->List));

    if(thys->Filebuff)
	ajDebug( "  Filebuff: %F (%Ld)\n",
		ajFileBuffFile(thys->Filebuff),
		ajFileTell(ajFileBuffFile(thys->Filebuff)));

    if(ajStrGetLen(thys->Usa))
	ajDebug( "  Usa: '%S'\n", thys->Usa);

    if(ajStrGetLen(thys->Ufo))
	ajDebug( "  Ufo: '%S'\n", thys->Ufo);

    if(thys->Fttable)
	ajDebug( "  Fttable: exists\n");

    if(thys->Ftquery)
	ajDebug( "  Ftquery: exists\n");

    if(ajStrGetLen(thys->Formatstr))
	ajDebug( "  Input format: '%S' (%d)\n", thys->Formatstr,
		thys->Format);

    if(ajStrGetLen(thys->Filename))
	ajDebug( "  Filename: '%S'\n", thys->Filename);

    if(ajStrGetLen(thys->Entryname))
	ajDebug( "  Entryname: '%S'\n", thys->Entryname);

    if(thys->Search)
	ajDebug( "  Search: %B\n", thys->Search);

    if(thys->Single)
	ajDebug( "  Single: %B\n", thys->Single);

    if(thys->Features)
	ajDebug( "  Features: %B\n", thys->Features);

    if(thys->IsNuc)
	ajDebug( "  IsNuc: %B\n", thys->IsNuc);

    if(thys->IsProt)
	ajDebug( "  IsProt: %B\n", thys->IsProt);

    if(thys->Count)
	ajDebug( "  Count: %d\n", thys->Count);

    if(thys->Filecount)
	ajDebug( "  Filecount: %d\n", thys->Filecount);

    if(thys->Fpos)
	ajDebug( "  Fpos: %l\n", thys->Fpos);

    if(thys->Query)
	ajSeqQueryTrace(thys->Query);

    if(thys->Data)
	ajDebug( "  Data: exists\n");

    if(ajStrGetLen(thys->Doc))
	ajDebug( "  Documentation:...\n%S\n", thys->Doc);

    return;
}




/* @funcstatic stockholmNew ***************************************************
**
** Creates and initialises a Stockholm object.
**
** @param [r] i [ajint] Number of sequences
** @return [SeqPStockholm] New sequence object.
** @@
******************************************************************************/

static SeqPStockholm stockholmNew(ajint i)
{
    SeqPStockholm thys = NULL;

    AJNEW0(thys);

    thys->id  = ajStrNew();
    thys->ac  = ajStrNew();
    thys->de  = ajStrNew();
    thys->au  = ajStrNew();
    thys->al  = ajStrNew();
    thys->tp  = ajStrNew();
    thys->se  = ajStrNew();
    thys->bm  = ajStrNew();
    thys->dc  = ajStrNew();
    thys->dr  = ajStrNew();
    thys->cc  = ajStrNew();
    thys->gs  = ajStrNew();
    thys->ref = ajStrNew();
    thys->sacons  = ajStrNew();
    thys->sscons  = ajStrNew();

    thys->n = i;

    AJCNEW0(thys->name,i);
    AJCNEW0(thys->str,i);

    for(i=0;i<thys->n;++i)
    {
	thys->name[i] = ajStrNew();
	thys->str[i]  = ajStrNew();
    }

    return thys;
}




/* #funcstatic stockholmdataNew ***********************************************
**
** Creates and initialises a Stockholm data object.
**
** #return [SeqPStockholmdata] New sequence object.
** ##
******************************************************************************/

/*static SeqPStockholmdata stockholmdataNew(void)
{
    SeqPStockholmdata thys = NULL;

    AJNEW0(thys);

    thys->id  = ajStrNew();
    thys->ac  = ajStrNew();
    thys->de  = ajStrNew();
    thys->au  = ajStrNew();
    thys->al  = ajStrNew();
    thys->tp  = ajStrNew();
    thys->se  = ajStrNew();
    thys->bm  = ajStrNew();
    thys->dc  = ajStrNew();
    thys->dr  = ajStrNew();
    thys->cc  = ajStrNew();
    thys->gs  = ajStrNew();
    thys->ref = ajStrNew();
    thys->sacons  = ajStrNew();
    thys->sscons  = ajStrNew();

    return thys;
}*/


/* @funcstatic stockholmDel ***************************************************
**
** Deletes a Stockholm object.
**
** @param [d] Pseq [SeqPStockholm*] Stockholm object
** @return [void]
** @@
******************************************************************************/

static void stockholmDel(SeqPStockholm *Pseq)
{
    SeqPStockholm pthis = NULL;
    ajint i;

    if(!Pseq)
	return;
    pthis = *Pseq;
    if(!pthis)
	return;

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);
    ajStrDel(&pthis->au);
    ajStrDel(&pthis->al);
    ajStrDel(&pthis->tp);
    ajStrDel(&pthis->se);
    ajStrDel(&pthis->bm);
    ajStrDel(&pthis->dc);
    ajStrDel(&pthis->dr);
    ajStrDel(&pthis->cc);
    ajStrDel(&pthis->gs);
    ajStrDel(&pthis->ref);
    ajStrDel(&pthis->sacons);
    ajStrDel(&pthis->sscons);

    for(i=0;i<pthis->n;++i)
    {
	ajStrDel(&pthis->name[i]);
	ajStrDel(&pthis->str[i]);
    }

    AJFREE(pthis->name);
    AJFREE(pthis->str);
    AJFREE(*Pseq);

    return;
}




/* #funcstatic stockholmdataDel ***********************************************
**
** Deletes a Stockholm data object.
**
** #param [d] Pseq [SeqPStockholmdata*] Stockholm object
** #return [void]
** ##
******************************************************************************/

/*static void stockholmdataDel(SeqPStockholmdata *Pseq)
{
    SeqPStockholmdata pthis = NULL;

    if(!Pseq)
	return;
    pthis = *Pseq;
    if(!pthis)
	return;

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);
    ajStrDel(&pthis->au);
    ajStrDel(&pthis->al);
    ajStrDel(&pthis->tp);
    ajStrDel(&pthis->se);
    ajStrDel(&pthis->bm);
    ajStrDel(&pthis->dc);
    ajStrDel(&pthis->dr);
    ajStrDel(&pthis->cc);
    ajStrDel(&pthis->gs);
    ajStrDel(&pthis->ref);
    ajStrDel(&pthis->sacons);
    ajStrDel(&pthis->sscons);

    AJFREE(*Pseq);

    return;
}*/


/* @funcstatic selexNew *******************************************************
**
** Creates and initialises a selex #=SQ line object.
**
** @param [r] n [ajint] Number of sequences
** @return [SeqPSelex] New sequence object.
** @@
******************************************************************************/

static SeqPSelex selexNew(ajint n)
{
    SeqPSelex thys = NULL;
    ajint    i;

    AJNEW0(thys);
    thys->id = ajStrNew();
    thys->ac = ajStrNew();
    thys->de = ajStrNew();
    thys->au = ajStrNew();
    thys->cs = ajStrNew();
    thys->rf = ajStrNew();
    thys->n  = n;

    AJCNEW(thys->name,n);
    AJCNEW(thys->str,n);
    AJCNEW(thys->ss,n);
    AJCNEW(thys->sq,n);

    for(i=0;i<n;++i)
    {
	thys->name[i] = ajStrNew();
	thys->str[i]  = ajStrNew();
	thys->ss[i]   = ajStrNew();
	thys->sq[i]   = selexseqNew();
    }

    return thys;
}




/* @funcstatic selexseqNew ****************************************************
**
** Creates and initialises a selex #=SQ line object.
**
** @return [SeqPSelexseq] New sequence object.
** @@
******************************************************************************/

static SeqPSelexseq selexseqNew()
{
    SeqPSelexseq thys = NULL;

    AJNEW0(thys);

    thys->name   = ajStrNew();
    thys->source = ajStrNew();
    thys->ac     = ajStrNew();
    thys->de     = ajStrNew();

    return thys;
}




/* #funcstatic selexdataNew ***************************************************
**
** Creates and initialises a selex #=SQ line object.
**
** #return [SeqPSelexdata] New sequence object.
** ##
******************************************************************************/

/*static SeqPSelexdata selexdataNew(void)
{
    SeqPSelexdata thys = NULL;

    AJNEW0(thys);
    thys->id = ajStrNew();
    thys->ac = ajStrNew();
    thys->de = ajStrNew();
    thys->au = ajStrNew();
    thys->cs = ajStrNew();
    thys->rf = ajStrNew();

    thys->name = ajStrNew();
    thys->str  = ajStrNew();
    thys->ss   = ajStrNew();
    thys->sq   = selexseqNew();

    return thys;
}*/


/* #funcstatic selexseqDel ****************************************************
**
** Deletes a Selex object.
**
** #param [d] Pseq [SeqPSelexseq*] Selex #=SQ object
** #return [void]
** ##
******************************************************************************/

/*static void selexseqDel(SeqPSelexseq *Pseq)
{
    SeqPSelexseq pthis;

    pthis = *Pseq;

    if(!Pseq || !pthis)
	return;

    ajStrDel(&pthis->name);
    ajStrDel(&pthis->source);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);

    AJFREE(pthis);
    *Pseq = NULL;

    return;
}*/




/* #funcstatic selexDel *******************************************************
**
** Deletes a Selex object.
**
** #param [d] Pseq [SeqPSelex*] Selex object
** #return [void]
** ##
******************************************************************************/

/*static void selexDel(SeqPSelex *Pseq)
{
    SeqPSelex pthis;
    ajint    i;
    ajint    n;

    pthis = *Pseq;

    if(!Pseq || !pthis)
	return;

    n = pthis->n;
    for(i=0;i<n;++i)
    {
	ajStrDel(&pthis->name[i]);
	ajStrDel(&pthis->str[i]);
	ajStrDel(&pthis->ss[i]);
	selexseqDel(&pthis->sq[i]);
    }

    if(n)
    {
	AJFREE(pthis->name);
	AJFREE(pthis->str);
	AJFREE(pthis->ss);
	AJFREE(pthis->sq);
    }

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);
    ajStrDel(&pthis->au);
    ajStrDel(&pthis->cs);
    ajStrDel(&pthis->rf);

    AJFREE(pthis);
    *Pseq = NULL;

    return;
}*/




/* #funcstatic selexdataDel ***************************************************
**
** Deletes a Selex data object.
**
** #param [d] Pseq [SeqPSelexdata*] Selex data object
** #return [void]
** ##
******************************************************************************/

/*static void selexdataDel(SeqPSelexdata *Pseq)
{
    SeqPSelexdata pthis;

    pthis = *Pseq;

    if(!Pseq || !pthis)
	return;


    ajStrDel(&pthis->name);
    ajStrDel(&pthis->str);
    ajStrDel(&pthis->ss);
    selexseqDel(&pthis->sq);

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);
    ajStrDel(&pthis->au);
    ajStrDel(&pthis->cs);
    ajStrDel(&pthis->rf);

    AJFREE(pthis);
    *Pseq = NULL;

    return;
}*/




/* #funcstatic seqSelexClone *************************************************
**
** Clone a Selexdata object
**
** #param [r] thys [const SeqPSelexdata] selex data object
**
** #return [SeqPSelexdata] New selex data object.
** ##
******************************************************************************/

/*static SeqPSelexdata seqSelexClone(const SeqPSelexdata thys)
{
    SeqPSelexdata pthis;

    pthis = selexdataNew();

    ajStrAssignS(&pthis->id, thys->id);
    ajStrAssignS(&pthis->ac, thys->ac);
    ajStrAssignS(&pthis->de, thys->de);
    ajStrAssignS(&pthis->au, thys->au);
    ajStrAssignS(&pthis->cs, thys->cs);
    ajStrAssignS(&pthis->rf, thys->rf);
    ajStrAssignS(&pthis->name, thys->name);
    ajStrAssignS(&pthis->str, thys->str);
    ajStrAssignS(&pthis->ss, thys->ss);

    pthis->ga[0] = thys->ga[0];
    pthis->ga[1] = thys->ga[1];
    pthis->tc[0] = thys->tc[0];
    pthis->tc[1] = thys->tc[1];
    pthis->nc[0] = thys->nc[0];
    pthis->nc[1] = thys->nc[1];

    ajStrAssignS(&pthis->sq->name, thys->sq->name);
    ajStrAssignS(&pthis->sq->source, thys->sq->source);
    ajStrAssignS(&pthis->sq->ac, thys->sq->ac);
    ajStrAssignS(&pthis->sq->de, thys->sq->de);

    pthis->sq->wt    = thys->sq->wt;
    pthis->sq->start = thys->sq->start;
    pthis->sq->stop  = thys->sq->stop;
    pthis->sq->len   = thys->sq->len;


    return pthis;
}*/




