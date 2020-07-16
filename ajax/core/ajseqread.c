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
#include <limits.h>
#include <math.h>
#include <errno.h>

#include "ajseqbam.h"

#ifdef WIN32
#define fileno _fileno
#endif

AjPTable seqDbMethods = NULL;

static ajint seqMaxGcglines = 5000;
static AjPRegexp seqRegQryWild = NULL;

static AjPRegexp seqRegTreeconTop  = NULL;
static AjPRegexp seqRegMegaCommand = NULL;
static AjPRegexp seqRegMegaFeat = NULL;
static AjPRegexp seqRegMegaSeq  = NULL;
static AjPRegexp seqRegJackTop  = NULL;
static AjPRegexp seqRegJackSeq  = NULL;
static AjPRegexp seqRegGffTyp = NULL;
static AjPRegexp seqRegGff3Typ = NULL;
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
static AjPStr seqFtFmtRefseq  = NULL;
static AjPStr seqFtFmtRefseqp = NULL;
static AjPStr seqFtFmtGff     = NULL;
static AjPStr seqFtFmtPir     = NULL;
static AjPStr seqFtFmtSwiss   = NULL;
static AjPStr seqUsaTest      = NULL;
static AjPStr seqQryChr       = NULL;
static AjPStr seqQryDb        = NULL;
static AjPStr seqQryList      = NULL;
static AjPStr seqReadLine     = NULL;
static AjPStr seqSaveLine     = NULL;
static AjPStr seqSaveLine2    = NULL;
static AjPStr seqAppendRestStr = NULL;
static AjPStr seqAppendTmpSeq = NULL;
static AjPStr seqQualStr      = NULL;

static AjPRegexp seqRegUsaAsis  = NULL;
static AjPRegexp seqRegUsaDb    = NULL;
static AjPRegexp seqRegUsaFmt   = NULL;
static AjPRegexp seqRegUsaId    = NULL;
static AjPRegexp seqRegUsaList  = NULL;
static AjPRegexp seqRegUsaRange = NULL;
static AjPRegexp seqRegUsaWild  = NULL;
static AjBool seqRegUsaInitDone = AJFALSE;
static AjBool seqDoWarnAppend = AJFALSE;

static float seqQualPhred[] = { 0.0,
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 1-8 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 9-16 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 17-24 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 25-32 */
   0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,   8.0,  9.0, /* 33-42 */
  10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0,  18.0, 19.0, /* 43-52 */
  20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0,  28.0, 29.0, /* 53-62 */
  30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0,  38.0, 39.0, /* 63-72 */
  40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0,  48.0, 49.0, /* 73-82 */
  50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0,  58.0, 59.0, /* 83-92 */
  60.0, 61.0, 62.0, 63.0, 64.0, 65.0, 66.0, 67.0,  68.0, 69.0, /* 93-102 */
  70.0, 71.0, 72.0, 73.0, 74.0, 75.0, 76.0, 77.0,  78.0, 79.0, /* 103-112 */
  80.0, 81.0, 82.0, 83.0, 84.0, 85.0, 86.0, 87.0,  88.0, 89.0, /* 113-122 */
  90.0, 91.0, 92.0, 93.0};	/* 123-126 */

static double seqQualSolexa[] = {
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 0-7 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 8-15 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 16-23 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 24-31 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 32-39 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 40-47 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 48-55 */
   0.0,  0.0,  0.0, 1.193310,                     /* 56-59 */
  1.455405,  1.764349,  2.124426,  2.539019,  3.010300, /* 60-64 */
  3.539019,  4.124426,  4.764349,  5.455405,  6.193310, /* 65-69 */
  6.973228,  7.790097,  8.638920,  9.514969, 10.413927, /* 70-74 */
 11.331956, 12.265724, 13.212384, 14.169543, 15.135209, /* 75-79 */
 16.107742, 17.085800, 18.068291, 19.054333, 20.043214, /* 80-84 */
 21.034361, 22.027316, 23.021712, 24.017255, 25.013712, /* 85-89 */
 26.010895, 27.008657, 28.006878, 29.005464, 30.004341, /* 90-94 */
 31.003448, 32.002739, 33.002176, 34.001729, 35.001373, /* 95-99 */
 36.001091, 37.000866, 38.000688, 39.000547, 40.000434, /* 100-104 */
 41.000345, 42.000274, 43.000218, 44.000173, 45.000137, /* 105-109 */
 46.000109, 47.000087, 48.000069, 49.000055, 50.000043, /* 110-114 */
 51.000034, 52.000027, 53.000022, 54.000017, 55.000014, /* 115-119 */
 56.000011, 57.000009, 58.000007, 59.000005, 60.000004, /* 120-124 */
 61.000003, 62.000003};	/* 125-126 */


    
static float seqQualIllumina[] = {
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 0-7 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 8-15 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 16-23 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 24-31 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 32-39 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 40-47 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 48-55 */
   0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, /* 56-63 */
   0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,   8.0,  9.0, /* 64-73 */
  10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0,  18.0, 19.0, /* 74-83 */
  20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0,  28.0, 29.0, /* 84-93 */
  30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0,  38.0, 39.0, /* 94-103 */
  40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0,  48.0, 49.0, /* 104-113 */
  50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0,  58.0, 59.0, /* 114-123 */
  60.0, 61.0, 62.0};	/* 124-126 */




/* @datastatic SeqPInFormat ***************************************************
**
** Sequence input formats data structure
**
** @alias SeqSInFormat
** @alias SeqOInFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Try [AjBool] If true, try for an unknown input. Duplicate names
**                    and read-anything formats are set false
** @attr Nucleotide [AjBool] True if suitable for nucleotide
** @attr Protein [AjBool] True if suitable for protein
** @attr Feature [AjBool] True if includes parsable feature data
** @attr Gap [AjBool] True if allows gap characters
** @attr Read [(AjBool*)] Input function, returns ajTrue on success
** @attr Multiset [AjBool] If true, supports multiple sequence sets
**                         If false, multiple sets must be in separate files
** @attr Binary [AjBool] Binary file format
** @@
******************************************************************************/

typedef struct SeqSInFormat
{
    const char *Name;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool Nucleotide;
    AjBool Protein;
    AjBool Feature;
    AjBool Gap;
    AjBool (*Read) (AjPSeq thys, AjPSeqin seqin);
    AjBool Multiset;
    AjBool Binary;
} SeqOInFormat;

#define SeqPInFormat SeqOInFormat*




/* @datastatic SeqPMsfData ****************************************************
**
** Sequence alignment data, stored until written when output file is closed
**
** @alias SeqSMsfData
** @alias SeqOMsfData
**
** @attr Table [AjPTable] Ajax table of AjPMsfItem objects
** @attr Names [AjPStr*] Sequence names
** @attr Count [ajuint] Undocumented
** @attr Nseq [ajuint] Number of sequences
** @attr Nexus [AjPNexus] Nexus alignment data
** @attr Gene [AjPStr] Gene name
** @attr Domain [AjPStr] Domain name
** @attr NextGene [AjPStr] Next block gene name
** @attr NextDomain [AjPStr] Next block domain name
** @attr Bufflines [ajuint] Number of buffered lines read
** @attr CommentDepth [ajint] Comment depth
** @attr Resume [AjBool] Resume processing
** @attr Identchar [char] Identity character
** @attr Indelchar [char] Gap character
** @attr Misschar [char] Gap character
** @attr Seqtype [char] Sequence type N:nucleotide P:protein
** @@
******************************************************************************/

typedef struct SeqSMsfData
{
    AjPTable Table;
    AjPStr* Names;
    ajuint Count;
    ajuint Nseq;
    AjPNexus Nexus;
    AjPStr Gene;
    AjPStr Domain;
    AjPStr NextGene;
    AjPStr NextDomain;
    ajuint Bufflines;
    ajint CommentDepth;
    AjBool Resume;
    char Identchar;
    char Indelchar;
    char Misschar;
    char Seqtype;
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
 ** @attr Desc [AjPStr] Sequence description
 ** @attr Len [ajuint] Sequence length
 ** @attr Check [ajuint] Sequence GCG checksum
 ** @attr Seq [AjPStr] Sequence
 ** @attr Weight [float] Weight (default 1.0)
 ** @attr Padding [char[4]] Padding to alignment boundary
 ** @@
*****************************************************************************/

typedef struct SeqSMsfItem
{
    AjPStr Name;
    AjPStr Desc;
    ajuint Len;
    ajuint Check;
    AjPStr Seq;
    float Weight;
    char Padding[4];
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
** @attr ga [ajuint[2]] Undocumented
** @attr tc [float[2]] Undocumented
** @attr nc [float[2]] Undocumented
** @attr bm [AjPStr] Undocumented
** @attr ref [AjPStr] Undocumented
** @attr dc [AjPStr] Undocumented
** @attr dr [AjPStr] Undocumented
** @attr cc [AjPStr] Undocumented
** @attr sacons [AjPStr] Undocumented
** @attr sqcons [AjPStr] Undocumented
** @attr sscons [AjPStr] Undocumented
** @attr gs [AjPStr] Undocumented
** @attr name [AjPStr*] Undocumented
** @attr str [AjPStr*] Undocumented
** @attr n [ajuint] Undocumented
** @attr Count [ajuint] Count
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
    ajuint  ga[2];
    float  tc[2];
    float  nc[2];
    AjPStr bm;
    AjPStr ref;
    AjPStr dc;
    AjPStr dr;
    AjPStr cc;
    AjPStr sacons;
    AjPStr sqcons;
    AjPStr sscons;
    AjPStr gs;
    AjPStr *name;
    AjPStr *str;
    ajuint  n;
    ajuint  Count;
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
** @attr sacons [AjPStr] Undocumented
** @attr sqcons [AjPStr] Undocumented
** @attr sscons [AjPStr] Undocumented
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
    AjPStr sacons;
    AjPStr sqcons;
    AjPStr sscons;
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
** @attr start [ajuint] start position
** @attr stop [ajuint] end position
** @attr len [ajuint] length
** @@
******************************************************************************/

typedef struct SeqSSelexseq
{
    AjPStr name;
    AjPStr source;
    AjPStr ac;
    AjPStr de;
    float  wt;
    ajuint  start;
    ajuint  stop;
    ajuint  len;
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
** @attr n [ajuint] Number of SeqPSelexseq sequence objects
** @attr Count [ajuint] Count
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
    ajuint  n;
    ajuint  Count;
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
** @attr Format [ajuint] Format number from original USA
** @attr Formatstr [AjPStr] Format name from original USA
** @attr Usa [AjPStr] Current USA
** @attr Features [AjBool] if true, process features
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct SeqSListUsa
{
    ajint Begin;
    ajint End;
    AjBool Rev;
    ajuint Format;
    AjPStr Formatstr;
    AjPStr Usa;
    AjBool Features;
    char Padding[4];
} SeqOListUsa;

#define SeqPListUsa SeqOListUsa*




enum fmtcode {FMT_OK, FMT_NOMATCH, FMT_BADTYPE, FMT_FAIL, FMT_EOF, FMT_EMPTY};




static AjBool     seqReadAbi(AjPSeq thys, AjPSeqin seqin);

static void       seqAccSave(AjPSeq thys, const AjPStr acc);
static ajuint     seqAppend(AjPStr* seq, const AjPStr line);
static ajuint     seqAppendK(AjPStr* seq, char ch);
static const AjPStr seqAppendWarn(AjPStr* seq, const AjPStr line);
static ajuint     seqAppendCommented(AjPStr* seq, AjBool* incomment,
				     const AjPStr line);
static AjBool     seqClustalReadseq(const AjPStr rdLine,
				    const AjPTable msftable);
static AjBool     seqDefine(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqFindInFormat(const AjPStr format, ajint *iformat);
static AjBool     seqFormatSet(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqGcgDots(AjPSeq thys, const AjPSeqin seqin,
			     AjPStr* pline, ajuint maxlines, ajuint *len);
static void       seqGcgRegInit(void);
static AjBool     seqGcgMsfDots(AjPSeq thys, const AjPSeqin seqin,
				AjPStr* pline,
				ajuint maxlines, ajuint *len);
static AjBool     seqGcgMsfHeader(const AjPStr line, SeqPMsfItem* msfitem);
static AjBool     seqGcgMsfReadseq(const AjPStr rdline,
				   const AjPTable msftable);
static AjBool     seqHennig86Readseq(const AjPStr rdline,
				     const AjPTable msftable);
static AjBool     seqinUfoLocal(const AjPSeqin thys);
static void       seqListNoComment(AjPStr* text);
static AjBool     seqListProcess(AjPSeq thys, AjPSeqin seqin,
				 const AjPStr usa);
static void       seqMsfDataDel(SeqPMsfData* pthys);
static void       seqMsfDataTrace(const SeqPMsfData thys);
static void       seqMsfItemDel(SeqPMsfItem* pthys);
static void       seqMsfTabDel(void **key, void **value, void *cl);
static void       seqMsfTabList(const void *key, void **value, void *cl);
static AjBool     seqPhylipReadseq(const AjPStr rdline,
				   const AjPTable phytable,
				   const AjPStr token,
				   ajuint len, ajuint* ilen, AjBool* done);
static AjBool     seqQueryField(const AjPSeqQuery qry, const AjPStr field);
static AjBool     seqQueryFieldC(const AjPSeqQuery qry, const char* field);
static AjBool     seqQueryMatch(const AjPSeqQuery query, const AjPSeq thys);
static void       seqQryWildComp(void);
static AjBool     seqRead(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadAce(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadAcedb(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadBam(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadBiomart(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadClustal(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadCodata(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadDbId(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadEmbl(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadEnsembl(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadExperiment(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadFasta(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadFastq(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadFastqIllumina(AjPSeq thys, AjPSeqin seqin);
/*static AjBool     seqReadFastqInt(AjPSeq thys, AjPSeqin seqin);*/
static AjBool     seqReadFastqSanger(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadFastqSolexa(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadFitch(AjPSeq thys, AjPSeqin seqin);
static ajuint     seqReadFmt(AjPSeq thys, AjPSeqin seqin,
			     ajuint format);
static AjBool     seqReadGcg(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGenbank(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGenpept(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGifasta(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGff(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGff3(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadHennig86(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadIg(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadIgstrict(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadJackknifer(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadMase(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadMega(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadMsf(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNbrf(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNcbi(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNexus(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPdb(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPdbseq(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPdbnuc(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPdbnucseq(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPhylip(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPhylipnon(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadRaw(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadRefseq(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadRefseqp(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadSam(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadSelex(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadStockholm(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadStaden(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadStrider(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadSwiss(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadText(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadTreecon(AjPSeq thys, AjPSeqin seqin);
static void       seqSelexAppend(const AjPStr src, AjPStr *dest, ajuint beg,
				 ajuint end);
static void       seqSelexCopy(AjPSeq *thys, SeqPSelex selex, ajuint n);
static AjBool     seqSelexHeader(SeqPSelex *thys, const AjPStr line,
				 AjBool *named, ajuint *sqcnt);
static void       seqSelexPos(const AjPStr line, ajuint *begin, ajuint *end);
static AjBool     seqSelexReadBlock(SeqPSelex *thys, AjBool *named, ajuint n,
				    AjPStr *line, AjPFilebuff buff,
				    AjBool store, AjPStr *astr);
static AjBool     seqSetInFormat(const AjPStr format);
static void       seqSetName(AjPSeq thys, const AjPStr str);
static void       seqitemSetName(SeqPMsfItem thys, const AjPStr str);
static void       seqnameSetName(AjPStr *name, const AjPStr str);
static void       seqSetNameFile(AjPSeq thys, const AjPSeqin seqin);
static void       seqSetNameNospace(AjPStr* name, const AjPStr str);
static void       seqStockholmCopy(AjPSeq *thys, SeqPStockholm stock, ajint n);
static void       seqSvSave(AjPSeq thys, const AjPStr sv);
static void       seqTaxSave(AjPSeq thys, const AjPStr tax, ajuint level);
static void       seqTaxidSaveI(AjPSeq thys, ajuint tax);
static void       seqTaxidSaveS(AjPSeq thys, const AjPStr tax);
static void       seqTextSeq(AjPStr* textptr, const AjPStr seq);
static void       seqUsaListTrace(const AjPList list);
static AjBool     seqUsaProcess(AjPSeq thys, AjPSeqin seqin);
static void       seqUsaRegInit(void);
static void       seqUsaRestore(AjPSeqin seqin, const SeqPListUsa node);
static void       seqUsaSave(SeqPListUsa node, const AjPSeqin seqin);

static void       seqqualAppendWarn(AjPStr* seq, const AjPStr line);

static SeqPStockholm stockholmNew(ajuint i);
static void         stockholmDel(SeqPStockholm *thys);

static void         selexDel(SeqPSelex *thys);
static void         selexseqDel(SeqPSelexseq *thys);

/*
static SeqPStockholmdata stockholmdataNew(void);
static void         stockholmdataDel(SeqPStockholmdata *thys);
static SeqPSelexdata seqSelexClone(const SeqPSelexdata thys);
static SeqPSelexdata selexdataNew(void);
static void         selexdataDel(SeqPSelexdata *thys);
*/

static SeqPSelex     selexNew(ajuint n);
static SeqPSelexseq  selexseqNew(void);


/* static data that needs the function definitions and so must come later */




/* @funclist seqInFormatDef ***************************************************
**
** Functions to read each sequence format
**
** New documentation on sequence formats:
** http://www.megasoftware.net/mega4.pdf pages 55 onwards (sections 4.1, 4.2)
** describe MEGA, some other formats, simple XML (name and seq)
**
** The SeqIO program supports some non-EMBOSS formats:
** http://biowulf.nih.gov/apps/seqio_docs/seqio_user.html
** notably FASTA-output, BLAST-output
** and has its own rules for database definitions (BioSeq)
** and database references
**
** For XML formats see Paul Gordon's list at
** http://www.visualgenomics.ca/gordonp/xml/
**
******************************************************************************/

static SeqOInFormat seqInFormatDef[] =
{
/* "Name",        "Description" */
/*     Alias,   Try,     Nucleotide, Protein   */
/*     Feature  Gap,     ReadFunction, Multiset, Binary */
  {"unknown",     "Unknown format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadText, AJFALSE, AJFALSE}, /* alias for text */
  {"gcg",         "GCG sequence format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadGcg, AJFALSE, AJFALSE}, /* do 1st,
                                                           headers mislead */
  {"gcg8",        "GCG old (version 8) sequence format",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadGcg, AJFALSE, AJFALSE}, /* alias for gcg
                                                           (8.x too) */
  {"embl",        "EMBL entry format",
       AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  seqReadEmbl, AJFALSE, AJFALSE},
  {"em",          "EMBL entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  seqReadEmbl, AJFALSE, AJFALSE}, /* alias for embl */
  {"swiss",       "Swissprot entry format",
       AJFALSE, AJTRUE,  AJFALSE, AJTRUE,
       AJTRUE,  AJTRUE,  seqReadSwiss, AJFALSE, AJFALSE},
  {"sw",          "Swissprot entry format (alias)",
       AJTRUE,  AJFALSE, AJFALSE, AJTRUE,
       AJTRUE,  AJTRUE,  seqReadSwiss, AJFALSE, AJFALSE}, /* alias for swiss */
  {"swissprot",   "Swissprot entry format(alias)",
       AJTRUE,  AJTRUE,  AJFALSE, AJTRUE,
       AJTRUE,  AJTRUE,  seqReadSwiss, AJFALSE, AJFALSE},
  {"nbrf",        "NBRF/PIR entry format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadNbrf, AJFALSE, AJFALSE}, /* test before NCBI */
  {"pir",         "NBRF/PIR entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadNbrf, AJFALSE, AJFALSE}, /* alias for nbrf */
  {"pdb",         "PDB protein databank format ATOM lines",
       AJFALSE, AJTRUE,  AJFALSE, AJTRUE,
       AJFALSE, AJFALSE, seqReadPdb, AJFALSE, AJFALSE},
  {"pdbseq",         "PDB protein databank format SEQRES lines",
       AJFALSE, AJTRUE,  AJFALSE, AJTRUE,
       AJFALSE, AJFALSE, seqReadPdbseq, AJFALSE, AJFALSE},
  {"pdbnuc",         "PDB protein databank format nucleotide ATOM lines",
       AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
       AJFALSE, AJFALSE, seqReadPdbnuc, AJFALSE, AJFALSE},
  {"pdbnucseq",         "PDB protein databank format nucleotide SEQRES lines",
       AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
       AJFALSE, AJFALSE, seqReadPdbnucseq, AJFALSE, AJFALSE},
  {"fasta",       "FASTA format including NCBI-style IDs",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadNcbi, AJFALSE, AJFALSE}, /* alias for ncbi,
						    preferred name */
  {"ncbi",        "FASTA format including NCBI-style IDs (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadNcbi, AJFALSE, AJFALSE}, /* test before
                                                            pearson */
  {"gifasta",     "FASTA format including NCBI-style GIs (alias)",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadGifasta, AJFALSE, AJFALSE}, /* NCBI with GI
                                                               as ID*/
  {"pearson",     "Plain old fasta format with IDs not parsed further",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadFasta, AJFALSE, AJFALSE}, /* plain fasta - off
                                                             by default, can
                                                             read bad files */
  {"fastq",       "FASTQ short read format ignoring quality scores",
       AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,
       AJFALSE, AJFALSE, seqReadFastq, AJFALSE, AJFALSE},
  {"fastq-sanger", "FASTQ short read format with phred quality",
       AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
       AJFALSE, AJFALSE, seqReadFastqSanger, AJFALSE, AJFALSE},
  {"fastq-illumina","FASTQ Illumina 1.3 short read format",
       AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
       AJFALSE, AJFALSE, seqReadFastqIllumina, AJFALSE, AJFALSE},
  {"fastq-solexa",  "FASTQ Solexa/Illumina 1.0 short read format",
       AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
       AJFALSE, AJFALSE, seqReadFastqSolexa, AJFALSE, AJFALSE},
/*
**  {"fastq-int",  "FASTQ short read format with integer Solexa scores",
**       AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
**       AJFALSE, AJFALSE, seqReadFastqInt, AJFALSE, AJFALSE},
*/
  {"sam",         "Sequence Alignment/Map (SAM) format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE, seqReadSam, AJFALSE, AJFALSE},
  {"genbank",     "Genbank entry format",
       AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  seqReadGenbank, AJFALSE, AJFALSE},
  {"gb",          "Genbank entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  seqReadGenbank, AJFALSE, AJFALSE}, /* alias for
                                                               genbank */
  {"ddbj",        "Genbank/DDBJ entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  seqReadGenbank, AJFALSE, AJFALSE}, /* alias for
                                                               genbank */
  {"refseq",      "Refseq entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  seqReadRefseq, AJFALSE, AJFALSE}, /* alias for
                                                              genbank */
  {"refseqp",     "Refseq protein entry format",
       AJFALSE, AJFALSE, AJFALSE, AJTRUE,       /* genbank format proteins */
       AJTRUE,  AJTRUE,  seqReadRefseqp, AJFALSE, AJFALSE},
  {"genpept",     "Refseq protein entry format (alias)",
       AJFALSE, AJFALSE, AJFALSE, AJTRUE,
       AJFALSE, AJTRUE,  seqReadGenpept, AJFALSE, AJFALSE},
  {"codata",      "Codata entry format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadCodata, AJFALSE, AJFALSE},
  {"strider",     "DNA strider output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadStrider, AJFALSE, AJFALSE},
  {"clustal",     "Clustalw output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadClustal, AJFALSE, AJFALSE},
  {"aln",         "Clustalw output format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadClustal, AJFALSE, AJFALSE}, /* alias for
                                                               clustal */
  {"phylip",      "Phylip interleaved and non-interleaved formats",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadPhylip, AJTRUE, AJFALSE},
  {"phylipnon",   "Phylip non-interleaved format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadPhylipnon, AJTRUE, AJFALSE}, /* tried by
                                                                phylip */
  {"ace",         "ACE sequence format",
       AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,
       AJFALSE, AJTRUE,  seqReadAce, AJFALSE, AJFALSE},
  {"consed",         "ACE sequence format",
       AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE,
       AJFALSE, AJTRUE,  seqReadAce, AJFALSE, AJFALSE}, /* alias for ace */
  {"acedb",       "ACEDB sequence format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadAcedb, AJFALSE, AJFALSE},
  {"dbid",        "Fasta format variant with database name before ID",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadDbId, AJFALSE, AJFALSE}, /* odd fasta with id as
                                                            second token */
  {"msf",         "GCG MSF (multiple sequence file) file format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadMsf, AJFALSE, AJFALSE},
  {"hennig86",    "Hennig86 output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadHennig86, AJFALSE, AJFALSE},
  {"jackknifer",  "Jackknifer interleaved and non-interleaved formats",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadJackknifer, AJFALSE, AJFALSE},
  {"nexus",       "Nexus/paup interleaved format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadNexus, AJFALSE, AJFALSE},
  {"paup",        "Nexus/paup interleaved format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadNexus, AJFALSE, AJFALSE}, /* alias for nexus */
  {"treecon",     "Treecon output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadTreecon, AJFALSE, AJFALSE},
  {"mega",        "Mega interleaved and non-interleaved formats",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadMega, AJFALSE, AJFALSE},
  {"igstrict",    "Intelligenetics sequence format strict parser",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadIgstrict, AJFALSE, AJFALSE},
  {"ig",          "Intelligenetics sequence format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadIg, AJFALSE, AJFALSE}, /* can read almost
                                                          anything */
  {"staden",      "Old staden package sequence format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadStaden, AJFALSE, AJFALSE},/* original staden
                                                             format */
  {"text",        "Plain text",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadText, AJFALSE, AJFALSE},/* can read almost
                                                           anything */
  {"plain",       "Plain text (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadText, AJFALSE, AJFALSE}, /* alias for text */
  {"gff2",         "GFF feature file with sequence in the header",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadGff, AJFALSE, AJFALSE},
  {"gff3",         "GFF3 feature file with sequence",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadGff3, AJFALSE, AJFALSE},
  {"gff",         "GFF3 feature file with sequence",
       AJTRUE,  AJFALSE,  AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadGff3, AJFALSE, AJFALSE},
  {"stockholm",   "Stockholm (pfam) format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadStockholm, AJFALSE, AJFALSE},
  {"pfam",        "Stockholm (pfam) format (alias)",
       AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadStockholm, AJFALSE, AJFALSE},
  {"selex",       "Selex format",                /* can read almost anything */
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadSelex, AJFALSE, AJFALSE},
  {"fitch",       "Fitch program format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadFitch, AJFALSE, AJFALSE},
  {"biomart",       "Biomart tab-delimited results",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadBiomart, AJFALSE, AJFALSE},
  {"mase",        "Mase program format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadMase, AJFALSE, AJFALSE}, /* like ig - off by
                                                            default */
  {"raw",         "Raw sequence with no non-sequence characters",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJFALSE, seqReadRaw, AJFALSE, AJFALSE}, /* OK - only sequence
                                                           chars allowed - but
                                                           off by default */
  {"experiment",  "Staden experiment file",
       AJFALSE, AJTRUE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadExperiment, AJFALSE, AJFALSE},
  {"abi",         "ABI trace file",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadAbi, AJFALSE, AJTRUE},
  {"bam",         "Binary Sequence Alignment/Map (BAM) format",
       AJFALSE, AJTRUE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE, seqReadBam, AJFALSE, AJTRUE},
  {"ensembl",     "Ensembl SQL format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJTRUE, AJTRUE, seqReadEnsembl, AJFALSE, AJFALSE},
  {NULL, NULL, 0, 0, 0, 0, 0, 0, NULL, 0, 0}
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
    SeqPListUsa node = NULL;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("ajSeqinDel called usa:'%S'\n", thys->Usa);


    ajStrDel(&thys->Name);
    ajStrDel(&thys->Acc);

    ajStrDel(&thys->Inputtype);

    ajStrDel(&thys->Db);
    ajStrDel(&thys->Full);
    ajStrDel(&thys->Date);
    ajStrDel(&thys->Desc);
    ajStrDel(&thys->Doc);

    while(ajListGetLength(thys->List))
    {
	ajListPop(thys->List, (void**) &node);
	ajStrDel(&node->Usa);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
    }

    ajListFree(&thys->List);

    ajStrDel(&thys->Usa);
    ajStrDel(&thys->Ufo);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Entryname);
    ajStrDel(&thys->Inseq);
    ajSeqQueryDel(&thys->Query);

    if(thys->Filebuff)
	ajFilebuffDel(&thys->Filebuff);

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
	if(!seqUsaProcess(thys, seqin)	      /* ... so process the USA */
	   && !ajListGetLength(seqin->List))  /* not list with bad 1st item */
	    return ajFalse; /* if this fails, we read no sequence at all */

	if(ajListGetLength(seqin->List))
	    listdata = ajTrue;
    }


    ret = seqRead(thys, seqin); /* read the sequence */

    if(ret)			/* clone any specified DB or entryname */
    {
	if (ajStrGetLen(seqin->Db))
	{
	    ajDebug("++ajSeqallRead set db: '%S' => '%S'\n",
		    seqin->Db, thys->Db);
	    ajStrAssignS(&thys->Db, seqin->Db);
	}

	if (ajStrGetLen(seqin->Entryname))
	{
	    ajDebug("++ajSeqallRead set entryname: '%S' => '%S'\n",
		    seqin->Entryname, thys->Entryname);
	    ajStrAssignS(&thys->Entryname, seqin->Entryname);
	}

	if(!ajStrGetLen(thys->Type)) /* make sure the type is set */
	    ajSeqType(thys);
    }

    while(!ret && ajListGetLength(seqin->List))
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

    if (seqin->List)
	ajSeqinClearPos(seqin);

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

	seqall->Totseqs++;
	seqall->Totlength += ajSeqGetLenTrimmed(seqall->Seq);;

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

	seqall->Totseqs++;
	seqall->Totlength += ajSeqGetLenTrimmed(seqall->Seq);;

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
** a USA that included the begin, end and direction
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
	ajFilebuffDel(&thys->Filebuff);

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
    thys->ChunkEntries = ajFalse;

    /* keep thys->Features */
    /* thys->Features = ajFalse;*/

    thys->Count     = 0;
    thys->Filecount = 0;

    thys->Begin = 0;
    thys->End = 0;

    thys->Records = 0;

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
** These functions read the sequence provided by the first argument
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
	if(ajListGetLength(seqin->List))
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

	    if(!seqUsaProcess(thys, seqin) && !ajListGetLength(seqin->List))
		return ajFalse;

	    ret = seqRead(thys, seqin);
	    ajDebug("ajSeqRead: list usa: '%S' returns: %B\n",
		    seqin->Usa, ret);
	}
	else
	{
	    ajDebug("ajSeqRead: no file yet - test USA '%S'\n", seqin->Usa);

	    /* (c) Must be a USA - decode it */
	    if(!seqUsaProcess(thys, seqin) && !ajListGetLength(seqin->List))
		return ajFalse;

	    if(ajListGetLength(seqin->List)) /* could be a new list */
		listdata = ajTrue;

	    ret = seqRead(thys, seqin);
	    ajDebug("ajSeqRead: new usa: '%S' returns: %B\n",
		    seqin->Usa, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(seqin->List))
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


    seqDefine(thys, seqin);

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

    ajuint iseq = 0;

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

    while(!seqin->multidone && ajSeqRead(seq, seqin))
    {
	if (seqin->List)
	    ajSeqinClearPos(seqin);
	/*ajDebug("read name '%S' length %d format '%S' '%S' seqindata: %x\n",
	  seq->Entryname, ajSeqGetLen(seq),
	  seqin->Formatstr, seq->Formatstr, seqin->Data);*/
	ajStrAssignEmptyS(&seq->Db, seqin->Db);

	if(!ajStrGetLen(seq->Type))
	    ajSeqType(seq);

	if(thys->Rev)
	    ajSeqSetRangeRev(seq, thys->Begin, thys->End);
	else
	    ajSeqSetRange(seq, thys->Begin, thys->End);

	ajDebug ("ajSeqsetRead read sequence %d %x '%S' %d..%d (%d) "
                 "Rev:%B Reversed:%B\n",
		 iseq, seq, ajSeqGetNameS(seq),
		 seq->Begin, seq->End, ajSeqGetLen(seq),
		 seq->Rev, seq->Reversed);

	/*ajSeqTrace(seq);*/
	iseq++;

	ajListPushAppend(setlist, seq);

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
    AjPSeq seq = NULL;
    AjPList setlist = NULL;
    AjPSeqset seqset = NULL;

    ajuint iseq = 0;

    seq = ajSeqNew();

    ajDebug("ajSeqsetallRead\n");

    if(!seqUsaProcess(seq, seqin))
	return ajFalse;



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
	  iseq, ajSeqGetNameC(seq), seq->Begin, seq->End);*/
	/*ajSeqTrace(seq);*/
	iseq++;

	if(!setlist)
	    setlist = ajListNew();

	ajListPushAppend(setlist, seq);

	/*ajDebug("appended to list\n");*/

	/* add to a list of sequences */

	seq = ajSeqNew();
	seqFormatSet(seq, seqin);

	if(seqin->multidone)
	{
	    seqset = ajSeqsetNew();
	    ajStrAssignS(&seqset->Usa, seqin->Usa);
	    ajStrAssignS(&seqset->Ufo, seqin->Ufo);
	    seqset->Begin = seqin->Begin;
	    seqset->End = seqin->End;

	    ajSeqsetFromList(seqset, setlist);
	    ajListFree(&setlist);
	    ajListPushAppend(thys, seqset);
	    ajDebug("ajSeqsetallRead multidone save set %u of %u sequences\n",
		    ajListGetLength(thys), ajSeqsetGetSize(seqset));
	    seqset = NULL;
	}
    }

    ajSeqDel(&seq);

    if(!iseq)
	return ajFalse;

    /* convert the list of sequences into a seqset structure */

    if(ajListGetLength(setlist))
    {
	seqset = ajSeqsetNew();
	ajStrAssignS(&seqset->Usa, seqin->Usa);
	ajStrAssignS(&seqset->Ufo, seqin->Ufo);
	seqset->Begin = seqin->Begin;
	seqset->End = seqin->End;

	ajSeqsetFromList(seqset, setlist);
	ajListFree(&setlist);
	ajListPushAppend(thys, seqset);
	seqset = NULL;
    }

    ajDebug("ajSeqsetallRead total %d sets of %d sequences\n",
	    ajListGetLength(thys), iseq);

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

    ajuint i;
    AjIList iter;
    AjPSeq seq;

    ajDebug("ajSeqsetFromList length: %d\n", ajListGetLength(list));

    /*ajListTrace(list);*/

    thys->Size      = ajListGetLength(list);
    thys->Seq       = AJCALLOC0(thys->Size, sizeof(AjPSeq));
    thys->Seqweight = AJCALLOC0(thys->Size, sizeof(float));

    i = 0;
    iter = ajListIterNewread(list);
    ajListIterTrace(iter);

    while((seq = (AjPSeq) ajListIterGet(iter)))
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

	/*	ajDebug("seq %d '%x'\n", i, seq);*/
	ajDebug("seq '%x' len: %d weight: %.3f\n",
		seq->Name, ajSeqGetLen(seq), thys->Seq[i]->Weight);
	i++;
    }
    ajListIterDel(&iter);

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
    ajuint iseq;

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
    thys->Seq[iseq] = ajSeqNewSeq(seq);
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
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the sequence type failed
**                  3 if it failed to read a sequence
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

static ajuint seqReadFmt(AjPSeq thys, AjPSeqin seqin,
			ajuint format)
{
    ajDebug("++seqReadFmt format %d (%s) '%S' feat %B\n",
	    format, seqInFormatDef[format].Name,
	    seqin->Usa, seqin->Features);

    seqin->Records = 0;

    /* Calling funclist seqInFormatDef() */
    if(seqInFormatDef[format].Read(thys, seqin))
    {
	ajDebug("seqReadFmt success with format %d (%s)\n",
		format, seqInFormatDef[format].Name);
        ajDebug("id: '%S' len: %d\n",
                thys->Name, ajStrGetLen(thys->Seq));
	seqin->Format = format;
	ajStrAssignC(&seqin->Formatstr, seqInFormatDef[format].Name);
	ajStrAssignC(&thys->Formatstr, seqInFormatDef[format].Name);
	ajStrAssignEmptyS(&thys->Db, seqin->Db);
	ajStrAssignS(&thys->Entryname, seqin->Entryname);
	ajStrAssignS(&thys->Filename, seqin->Filename);

	if(seqQueryMatch(seqin->Query, thys))
	{
	    ajStrAssignEmptyS(&thys->Entryname, thys->Name);

	    if(seqin->Features && !thys->Fttable)
	    {
		ajStrAssignEmptyS(&seqin->Ftquery->Seqname, thys->Entryname);
		seqin->Fttable = ajFeattableNewReadUfo(seqin->Ftquery,
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

            if (!ajStrGetLen(thys->Seq))      /* empty sequence string! */
                return FMT_EMPTY;

	    if(ajSeqTypeCheckIn(thys, seqin))
	    {
                if (!ajStrGetLen(thys->Seq))  /* removed all remaining chars */
                    return FMT_EMPTY;

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
		ajFilebuffIsBuffered(seqin->Filebuff),
		ajFilebuffIsEof(seqin->Filebuff));

	if (!ajFilebuffIsBuffered(seqin->Filebuff) &&
	    ajFilebuffIsEof(seqin->Filebuff))
	    return FMT_EOF;

	ajFilebuffResetStore(seqin->Filebuff, seqin->Text, &thys->TextPtr);
	ajDebug("Format %d (%s) failed, file buffer reset by seqReadFmt\n",
		format, seqInFormatDef[format].Name);
	/* ajFilebuffTraceFull(seqin->Filebuff, 10, 10);*/
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
    ajuint i;
    ajuint istat = 0;
    ajuint jstat = 0;

    AjPFilebuff buff = seqin->Filebuff;
    AjBool regfile = ajFalse;
    AjBool ok;

    ajSeqClear(thys);
    ajDebug("seqRead: cleared\n");

    if(seqin->Single && seqin->Count)
    {
	/*
	** One sequence at a time is read.
	** The first sequence was read by ACD
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
        buff = seqin->Filebuff;
    }

    ajDebug("seqRead: seqin format %d '%S'\n", seqin->Format,
	    seqin->Formatstr);

    seqin->Count++;

    if(!seqin->Filebuff)
	return ajFalse;

    ok = ajFilebuffIsBuffered(seqin->Filebuff);

    if(!seqInFormatDef[seqin->Format].Binary)
    {
        while(ok)
        {				/* skip blank lines */
            ok = ajBuffreadLine(seqin->Filebuff, &seqReadLine);
            
            if(!ajStrIsWhite(seqReadLine))
            {
                ajFilebuffClear(seqin->Filebuff,1);
                break;
            }
        }
    }

    if(!seqin->Format)
    {			   /* no format specified, try all defaults */

        regfile = ajFileIsFile(ajFilebuffGetFile(seqin->Filebuff));

	for(i = 1; seqInFormatDef[i].Name; i++)
	{
	    if(!seqInFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

            if(seqInFormatDef[i].Binary && !regfile)
            {
                ajDebug("seqRead: binary stdin skip format %d (%s)\n",
                        i, seqInFormatDef[i].Name);
                continue;
            }

	    ajDebug("seqRead:try format %d (%s)\n",
		    i, seqInFormatDef[i].Name);

	    istat = seqReadFmt(thys, seqin, i);

	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++seqRead OK, set format %d\n", seqin->Format);
		seqDefine(thys, seqin);

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
		       ajSeqGetUsaS(thys));
		ajDebug("seqRead: (e1) seqReadFmt stat==EMPTY try again\n");
		break;
	    default:
		ajDebug("unknown code %d from seqReadFmt\n", stat);
	    }

	    ajSeqClear(thys);

	    if(seqin->Format)
		break;			/* we read something */

            ajFilebuffTrace(seqin->Filebuff);
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
	ajFilebuffSetUnbuffered(seqin->Filebuff);

	ajDebug("++seqRead known format %d\n", seqin->Format);
	istat = seqReadFmt(thys, seqin, seqin->Format);

	switch(istat)
	{
	case FMT_OK:
	    seqDefine(thys, seqin);

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
            if(seqin->Records)
                ajErr("Error reading file '%F' with format '%s': "
                      "end-of-file before end of data "
                      "(read %u records)",
                      ajFilebuffGetFile(seqin->Filebuff),
                      seqInFormatDef[seqin->Format].Name,
                      seqin->Records);
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("Sequence '%S' has zero length, ignored",
		   ajSeqGetUsaS(thys));
	    ajDebug("seqRead: (e2) seqReadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from seqReadFmt\n", stat);
	}

	ajSeqClear(thys); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("seqRead failed - try again with format %d '%s' code %d\n",
	    seqin->Format, seqInFormatDef[seqin->Format].Name, istat);

    ajDebug("Search:%B Chunk:%B Data:%x ajFileBuffEmpty:%B\n",
	    seqin->Search, seqin->ChunkEntries,
            seqin->Data, ajFilebuffIsEmpty(buff));

    if(ajFilebuffIsEmpty(buff) && seqin->ChunkEntries)
    {
	if(!seqin->Query->Access->Access(seqin))
            return ajFalse;
        buff = seqin->Filebuff;
    }


    /* need to check end-of-file to avoid repeats */
    while(seqin->Search && (seqin->Data || !ajFilebuffIsEmpty(buff)))
    {
	jstat = seqReadFmt(thys, seqin, seqin->Format);

	switch(jstat)
	{
	case FMT_OK:
	    seqDefine(thys, seqin);

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
	    if(istat != FMT_EMPTY)
                ajWarn("Sequence '%S' has zero length, ignored",
                       ajSeqGetUsaS(thys));
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
    AjPFilebuff buff;
    AjPStr id   = NULL;
    AjPStr acc  = NULL;
    AjPStr sv   = NULL;
    AjPStr desc = NULL;

    const char *cp;
    ajlong fpos     = 0;
    ajlong fposb    = 0;
    AjBool ok       = ajTrue;
    AjPStr tmpline = NULL;
    const AjPStr badstr = NULL;

    ajDebug("seqReadFasta\n");

    buff = seqin->Filebuff;

    /* ajFilebuffTrace(buff); */

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fpos,
			     seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    seqin->Records++;

    ajDebug("First line: %S\n", seqReadLine);

    /* If ; then it is really PIR format */
    if(ajStrGetCharPos(seqReadLine, 3) == ';')
    {
	ajStrAssignSubS(&tmpline,seqReadLine, 4, -1);
	ajFmtPrintS(&seqReadLine, ">%S",tmpline);
	ajDebug("PIR format changed line to %S\n", seqReadLine);
	ajStrDel(&tmpline);
    }

    cp = ajStrGetPtr(seqReadLine);

    if(*cp != '>')
    {
	ajDebug("first line is not FASTA\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(!ajSeqParseFasta(seqReadLine, &id, &acc, &sv, &desc))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    /* we know we will succeed from here ... no way to return ajFalse */

    ajFilebuffSetUnbuffered(buff);

    seqSetNameNospace(&thys->Name, id);

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

	ajFilebuffClear(buff, 0);
    }
    else
    {
	ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, ">"))
	{
	    badstr = seqAppendWarn(&thys->Seq, seqReadLine);

	    if(badstr)
		ajWarn("Sequence '%S' has bad character(s) '%S'",
			   thys->Name, badstr);
	    seqin->Records++;
	    ajDebug("++fasta append line '%S'\n", seqReadLine);
	    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				     seqin->Text, &thys->TextPtr);
	}

	if(ok)
	    ajFilebuffClearStore(buff, 1,
				 seqReadLine, seqin->Text, &thys->TextPtr);
	else
	    ajFilebuffClear(buff, 0);
    }

    thys->Fpos = fpos;

    ajDebug("started at fpos %Ld ok: %B fposb: %Ld\n", fpos, ok, fposb);

    return ajTrue;
}




/* @funcstatic seqReadFastq ***************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the FASTQ format, but ignores quality values.
**
** See the more specific fastq formats for parsers that read and process
** the quality scores.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadFastq(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjPStr id   = NULL;
    AjPStr acc  = NULL;
    AjPStr sv   = NULL;
    AjPStr desc = NULL;

    ajuint seqlen = 0;
    /*AjPStr qualstr = NULL;*/
    char minqual;
    char maxqual;
    char comqual;

    const char *cp;
    ajlong fpos     = 0;
    ajlong fposb    = 0;
    AjBool ok       = ajTrue;
    const AjPStr badstr = NULL;

    ajDebug("seqReadFastq\n");

    buff = seqin->Filebuff;

    /* ajFilebuffTrace(buff); */

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fpos,
			     seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    seqin->Records++;

    ajDebug("First line: %S\n", seqReadLine);

    cp = ajStrGetPtr(seqReadLine);

    if(*cp != '@')
    {
	ajDebug("first line is not FASTQ\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(!ajSeqParseFastq(seqReadLine, &id, &desc))
    {
	ajDebug("first line did not parse as FASTQ\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    seqSetNameNospace(&thys->Name, id);

    if(ajStrGetLen(sv))
	seqSvSave(thys, sv);

    if(ajStrGetLen(acc))
	seqAccSave(thys, acc);

    ajStrAssignS(&thys->Desc, desc);
    ajStrDel(&id);
    ajStrDel(&acc);
    ajStrDel(&sv);
    ajStrDel(&desc);

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);
    while(ok &&
          !ajStrPrefixC(seqReadLine, "+"))
    {
        badstr = seqAppendWarn(&thys->Seq, seqReadLine);

        if(badstr)
            ajWarn("Sequence '%S' has bad character(s) '%S'",
                   thys->Name, badstr);
        seqin->Records++;
        ajDebug("++fastq append line '%S'\n", seqReadLine);
        ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
                                    seqin->Text, &thys->TextPtr);
        ajDebug("++fastq sequence %4u '%S'\n",
                ajStrGetLen(thys->Seq), thys->Seq);
    }

    if(!ok)
    {
	ajDebug("failed to find quality scores\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    seqlen = ajStrGetLen(thys->Seq);

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);

    ajStrAssignClear(&seqQualStr);

    while(ok &&
          ((ajStrGetLen(seqQualStr) < seqlen) ||
           ajStrGetCharFirst(seqReadLine) !=  '@'))
    {
        seqqualAppendWarn(&seqQualStr, seqReadLine);

        seqin->Records++;
        ajDebug("++fastq append qualities '%S'\n", seqReadLine);
        ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
                                    seqin->Text, &thys->TextPtr);
        ajDebug("++fastq qualities %3u '%S'\n",
                ajStrGetLen(seqQualStr), seqQualStr);
    }

    minqual = ajStrGetAsciiLow(seqQualStr);
    maxqual = ajStrGetAsciiHigh(seqQualStr);
    comqual = ajStrGetAsciiCommon(seqQualStr);

    if(ajStrGetLen(seqQualStr) != seqlen)
    {
	ajDebug("length mismatch seq: %u quality: %u\n",
                seqlen, ajStrGetLen(seqQualStr));
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(ok)
        ajFilebuffClearStore(buff, 1,
                             seqReadLine, seqin->Text, &thys->TextPtr);
    else
        ajFilebuffClear(buff, 0);

    thys->Fpos = fpos;

    ajDebug("started at fpos %Ld ok: %B fposb: %Ld\n", fpos, ok, fposb);

    ajDebug("quality characters %d..%d (%d) '%c' '%c' (%c)\n",
            (int) minqual, (int) maxqual, (int) comqual,
            minqual, maxqual, comqual);

    return ajTrue;
}




/* @funcstatic seqReadFastqSanger *********************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the FASTQ format, and interprets Sanger (phred) scores.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadFastqSanger(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjPStr id   = NULL;
    AjPStr acc  = NULL;
    AjPStr sv   = NULL;
    AjPStr desc = NULL;

    ajuint seqlen = 0;

/*
**    char minqual;
**    char maxqual;
**    char comqual;
*/

    const char *cp;
    ajint iqual;
    ajlong fpos     = 0;
    ajlong fposb    = 0;
    AjBool ok       = ajTrue;
    const AjPStr badstr = NULL;

    /*    ajint amin = 0; */
    ajint qmin = 33;
    ajint qmax = 126;
    ajuint i;
    ajuint cntseq = 0;
    ajuint cntqual = 0;
    ajuint cntnewline = 0;

    /* ajDebug("seqReadFastqSanger\n"); */

    buff = seqin->Filebuff;

    /* ajFilebuffTrace(buff); */

    ok = ajBuffreadLinePosStore(buff, &seqSaveLine, &fpos,
			     seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    seqin->Records++;

    /* ajDebug("First line: %S\n", seqSaveLine); */

    cp = MAJSTRGETPTR(seqSaveLine);

    if(*cp != '@')
    {
        /* ajDebug("first line is not FASTQ\n"); */
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(!ajSeqParseFastq(seqSaveLine, &id, &desc))
    {
        /* ajDebug("first line did not parse as FASTQ\n"); */
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    seqSetNameNospace(&thys->Name, id);

    if(MAJSTRGETLEN(sv))
	seqSvSave(thys, sv);

    if(MAJSTRGETLEN(acc))
	seqAccSave(thys, acc);

    ajStrAssignS(&thys->Desc, desc);
    ajStrDel(&id);
    ajStrDel(&acc);
    ajStrDel(&sv);
    ajStrDel(&desc);

    i = MAJSTRGETLEN(seqSaveLine) - 1;
    while(ajStrGetCharPos(seqSaveLine, i) == '\n' ||
          ajStrGetCharPos(seqSaveLine, i) == '\r')
    {
        cntnewline++;
        i--;
    }
    
    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);
    while(ok &&
          ajStrGetCharFirst(seqReadLine) != '+')
    {
        cntseq += MAJSTRGETLEN(seqReadLine) - cntnewline;
        badstr = seqAppendWarn(&thys->Seq, seqReadLine);

        if(badstr)
            ajWarn("Sequence '%S' has bad character(s) '%S'",
                   thys->Name, badstr);
        seqin->Records++;
        /* ajDebug("++fastq append line '%S'\n", seqReadLine); */
        ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
                                    seqin->Text, &thys->TextPtr);
        /* ajDebug("++fastq sequence %4u '%S'\n",
	           ajStrGetLen(thys->Seq), thys->Seq); */
    }

    if(!ok)
    {
        /* ajDebug("failed to find quality scores\n"); */
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }
    if(MAJSTRGETLEN(seqReadLine) > (cntnewline+1))
    {
        ajStrPasteCountK(&seqReadLine, 0,'@', 1);
        if(!ajStrMatchS(seqReadLine, seqSaveLine))
        {
            ajStrPasteCountK(&seqReadLine, 0,'+', 1);
            ajWarn("Mismatch in file '%F' + line "
                   "does not match first line '%.*S' '%.*S'",
                   ajFilebuffGetFile(buff),
                   MAJSTRGETLEN(seqSaveLine) - cntnewline, seqSaveLine,
                   MAJSTRGETLEN(seqReadLine) - cntnewline,  seqReadLine);
        }        
    }
    
    seqlen = MAJSTRGETLEN(thys->Seq);

    if(seqlen < cntseq)
    {
        ajWarn("FASTQ format '%F' sequence '%S' "
               "sequence skipped %u character(s)",
               ajFilebuffGetFile(buff), thys->Name, cntseq - seqlen);
    }
    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);

    ajStrAssignClear(&seqQualStr);
    ajStrAssignClear(&seqSaveLine2);

    while(ok &&
          ((MAJSTRGETLEN(seqQualStr) < seqlen) ||
           ajStrGetCharFirst(seqReadLine) != '@'))
    {
        if((ajStrGetCharFirst(seqReadLine) == '@') &&
           !MAJSTRGETLEN(seqSaveLine2))
            ajStrAssignS(&seqSaveLine2, seqReadLine);
        
        cntqual += MAJSTRGETLEN(seqReadLine) - cntnewline;
        seqqualAppendWarn(&seqQualStr, seqReadLine);

        seqin->Records++;
        /* ajDebug("++fastq append qualities '%S'\n", seqReadLine); */
        ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
                                    seqin->Text, &thys->TextPtr);
        /* ajDebug("++fastq qualities %3u '%S'\n",
	           ajStrGetLen(seqQualStr), seqQualStr); */
    }

/*
**    minqual = ajStrGetAsciiLow(seqQualStr);
**    maxqual = ajStrGetAsciiHigh(seqQualStr);
**    comqual = ajStrGetAsciiCommon(seqQualStr);
*/

    if(MAJSTRGETLEN(seqQualStr) != seqlen)
    {
      	ajWarn("FASTQ quality length mismatch '%F' '%S' "
               "expected: %u found: %u",
	       ajFilebuffGetFile(buff), thys->Name,
	       seqlen, ajStrGetLen(seqQualStr));
        if((MAJSTRGETLEN(seqQualStr) > seqlen) &&
           MAJSTRGETLEN(seqSaveLine2))
        {
            ajStrTrimEndC(&seqSaveLine2, "\n\r");
            ajWarn("(Possible short quality record before '%S')",
                   seqSaveLine2);
        }
    }
    if(MAJSTRGETLEN(seqQualStr) < cntqual)
    {
        ajWarn("FASTQ format '%F' sequence '%S' "
               "quality skipped %u character(s)",
               ajFilebuffGetFile(buff), thys->Name,
               cntqual - MAJSTRGETLEN(seqQualStr));
    }
    

    if(ok)
        ajFilebuffClearStore(buff, 1,
                             seqReadLine, seqin->Text, &thys->TextPtr);
    else
        ajFilebuffClear(buff, 0);

    thys->Fpos = fpos;

    /* ajDebug("started at fpos %Ld ok: %B fposb: %Ld\n", fpos, ok, fposb); */

    /* ajDebug("Sanger: %d..%d (%d)\n",
               (ajint) minqual, (ajint) maxqual, (ajint) comqual); */
    
    cp = MAJSTRGETPTR(seqQualStr);
    i=0;

    if(seqlen > thys->Qualsize)
        AJCRESIZE(thys->Accuracy, seqlen);

    thys->Qualsize = seqlen;

    if(MAJSTRGETLEN(seqQualStr) > thys->Qualsize)
        AJCRESIZE(thys->Accuracy, MAJSTRGETLEN(seqQualStr));

    /*
    ** Sanger uses Phred quality calculated from error probability p
    ** Qp = -10 log (p)
    **
    ** For Sanger (phred) p = 1 / 10**(Q/10)
    ** 10: p=0.1 20: p=0.01 etc.
    */

    while (*cp)
    {
        iqual = *cp++;
        if(iqual < qmin)
	{
            ajWarn("FASTQ-SANGER '%F' sequence '%S' "
                   "quality value '%c' too low",
                   ajFilebuffGetFile(buff), thys->Name,
                   (char) iqual);
            iqual = qmin;
	}
        if(iqual > qmax)
	{
	    ajWarn("FASTQ-SANGER '%F' sequence '%S' "
                   "quality value '%c' too high",
                   ajFilebuffGetFile(buff), thys->Name,
                   (char) iqual);
	    iqual = qmax;
        }
        thys->Accuracy[i++] = seqQualPhred[iqual];
    }


/*
**    ajDebug("quality characters %d..%d (%d) '%c' '%c' (%c) "
**            "scores %d..%d (%d)\n",
**            (int) minqual, (int) maxqual, (int) comqual,
**            minqual, maxqual, comqual,
**            (amin + minqual - qmin), (amin + maxqual - qmin),
**            (amin + comqual - qmin));
*/

    ajStrAssignClear(&seqQualStr);

    return ajTrue;
}




/* #funcstatic seqReadFastqInt ************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the FASTQ numeric format, and interprets integer Solexa scores.
**
** #param [w] thys [AjPSeq] Sequence object
** #param [u] seqin [AjPSeqin] Sequence input object
** #return [AjBool] ajTrue on success
** ##
******************************************************************************/

/*
static AjBool seqReadFastqInt(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjPStrTok handle  = NULL;
    AjPStr id   = NULL;
    AjPStr acc  = NULL;
    AjPStr sv   = NULL;
    AjPStr desc = NULL;

    ajuint seqlen = 0;
    AjPStr qualstr = NULL;

    const char *cp;
    ajlong fpos     = 0;
    ajlong fposb    = 0;
    AjBool ok       = ajTrue;

    const AjPStr badstr = NULL;

    ajuint i;
    AjBool badwarn = ajFalse;
    double sval;
    double pval;
    double qval;

    ajDebug("seqReadFastqInt\n");

    buff = seqin->Filebuff;

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fpos,
			     seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    seqin->Records++;

    ajDebug("First line: %S\n", seqReadLine);

    cp = ajStrGetPtr(seqReadLine);

    if(*cp != '@')
    {
	ajDebug("first line is not FASTQ\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(!ajSeqParseFastq(seqReadLine, &id, &desc))
    {
	ajDebug("first line did not parse as FASTQ\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    seqSetNameNospace(&thys->Name, id);

    if(ajStrGetLen(sv))
	seqSvSave(thys, sv);

    if(ajStrGetLen(acc))
	seqAccSave(thys, acc);

    ajStrAssignS(&thys->Desc, desc);
    ajStrDel(&id);
    ajStrDel(&acc);
    ajStrDel(&sv);
    ajStrDel(&desc);

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);
    while(ok &&
          !ajStrPrefixC(seqReadLine, "+"))
    {
        badstr = seqAppendWarn(&thys->Seq, seqReadLine);

        if(badstr)
            ajWarn("Sequence '%S' has bad character(s) '%S'",
                   thys->Name, badstr);
        seqin->Records++;
        ajDebug("++fastq append line '%S'\n", seqReadLine);
        ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
                                    seqin->Text, &thys->TextPtr);
        ajDebug("++fastq sequence %4u '%S'\n",
                ajStrGetLen(thys->Seq), thys->Seq);
    }

    if(!ok)
    {
	ajDebug("failed to find quality scores\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    seqlen = ajStrGetLen(thys->Seq);

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);

    i=0;
    if(seqlen > thys->Qualsize)
    {
        AJCRESIZE(thys->Accuracy, seqlen);
        thys->Qualsize = seqlen;
    }

    ajStrAssignClear(&seqQualStr);
    while(ok &&
          (!ajStrPrefixC(seqReadLine, "@")))
    {
	ajStrTokenAssignC(&handle, seqReadLine, " ,\n\r\t");
        while(ajStrTokenNextParse(&handle, &seqQualStr))
        {
            if(i >= seqlen){
                if(!badwarn)
                    ajWarn("Bad quality '%S' for base %d "
                       "in fastq-int format\n",
                       qualstr, i);
                badwarn = ajTrue;
            }
            else if(!ajStrToDouble(seqQualStr, &sval))
            {
                if(!badwarn)
                    ajWarn("Bad quality '%S' for base %d "
			   "in fastq-int format\n",
			   qualstr, i);
                badwarn = ajTrue;
                i++;
            }
            else
            {
                pval = pow(10.0, (sval / -10.0));
                qval = pval / (1.0 + pval);
                thys->Accuracy[i++] = -10.0 * log10(qval);
            }
        }
        seqin->Records++;
        ajDebug("++fastq append qualities '%S'\n", seqReadLine);
        ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
                                    seqin->Text, &thys->TextPtr);
    }

    if(i != seqlen)
    {
	ajWarn("length mismatch seq: %u quality: %u\n",
                seqlen, i);
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(ok)
        ajFilebuffClearStore(buff, 1,
                             seqReadLine, seqin->Text, &thys->TextPtr);
    else
        ajFilebuffClear(buff, 0);

    thys->Fpos = fpos;

    ajDebug("started at fpos %Ld ok: %B fposb: %Ld\n", fpos, ok, fposb);

    ajStrTokenDel(&handle);

    return ajTrue;
}
*/




/* @funcstatic seqReadFastqIllumina ********************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the FASTQ format, and processes phred quality scores
** with Illumina encoding.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadFastqIllumina(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjPStr id   = NULL;
    AjPStr acc  = NULL;
    AjPStr sv   = NULL;
    AjPStr desc = NULL;

    ajuint seqlen = 0;
    /*AjPStr qualstr = NULL;*/
/*
**    char minqual;
**    char maxqual;
**    char comqual;
*/

    const char *cp;
    ajint iqual;
    ajlong fpos     = 0;
    ajlong fposb    = 0;
    AjBool ok       = ajTrue;
    const AjPStr badstr = NULL;

    /*ajint amin = 0;*/
    ajint qmin = 64;
    ajint qmax = 126;
    ajuint i;

    ajDebug("seqReadFastqIllumina\n");

    buff = seqin->Filebuff;

    /* ajFilebuffTrace(buff); */

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fpos,
			     seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    seqin->Records++;

    ajDebug("First line: %S\n", seqReadLine);

    cp = ajStrGetPtr(seqReadLine);

    if(*cp != '@')
    {
	ajDebug("first line is not FASTQ\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(!ajSeqParseFastq(seqReadLine, &id, &desc))
    {
	ajDebug("first line did not parse as FASTQ\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    seqSetNameNospace(&thys->Name, id);

    if(ajStrGetLen(sv))
	seqSvSave(thys, sv);

    if(ajStrGetLen(acc))
	seqAccSave(thys, acc);

    ajStrAssignS(&thys->Desc, desc);
    ajStrDel(&id);
    ajStrDel(&acc);
    ajStrDel(&sv);
    ajStrDel(&desc);

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);
    while(ok &&
          !ajStrPrefixC(seqReadLine, "+"))
    {
        badstr = seqAppendWarn(&thys->Seq, seqReadLine);

        if(badstr)
            ajWarn("Sequence '%S' has bad character(s) '%S'",
                   thys->Name, badstr);
        seqin->Records++;
        ajDebug("++fastq append line '%S'\n", seqReadLine);
        ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
                                    seqin->Text, &thys->TextPtr);
        ajDebug("++fastq sequence %4u '%S'\n",
                ajStrGetLen(thys->Seq), thys->Seq);
    }

    if(!ok)
    {
	ajDebug("failed to find quality scores\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    seqlen = ajStrGetLen(thys->Seq);

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);

    ajStrAssignClear(&seqQualStr);

    while(ok &&
          ((ajStrGetLen(seqQualStr) < seqlen) ||
           ajStrGetCharFirst(seqReadLine) != '@'))
    {
        seqqualAppendWarn(&seqQualStr, seqReadLine);

        seqin->Records++;
        ajDebug("++fastq append qualities '%S'\n", seqReadLine);
        ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
                                    seqin->Text, &thys->TextPtr);
        ajDebug("++fastq qualities %3u '%S'\n",
                ajStrGetLen(seqQualStr), seqQualStr);
    }

/*
**    minqual = ajStrGetAsciiLow(seqQualStr);
**    maxqual = ajStrGetAsciiHigh(seqQualStr);
**    comqual = ajStrGetAsciiCommon(seqQualStr);
*/

    if(ajStrGetLen(seqQualStr) != seqlen)
    {
	ajDebug("length mismatch seq: %u quality: %u\n",
                seqlen, ajStrGetLen(seqQualStr));
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(ok)
        ajFilebuffClearStore(buff, 1,
                             seqReadLine, seqin->Text, &thys->TextPtr);
    else
        ajFilebuffClear(buff, 0);

    thys->Fpos = fpos;

    /*ajDebug("started at fpos %Ld ok: %B fposb: %Ld\n", fpos, ok, fposb);*/

    /*ajDebug("Illumina: %d..%d (%d)\n",
      (ajint) minqual, (ajint) maxqual, (ajint) comqual);*/
    
    cp = ajStrGetPtr(seqQualStr);
    i=0;

    if(seqlen > thys->Qualsize)
	AJCRESIZE(thys->Accuracy, seqlen);

    thys->Qualsize = seqlen;

    /*
    ** Illumina uses Phred quality calculated from error probability p
    ** Qp = -10 log (p)
    **
    ** For Sanger (phred) p = 1 / 10**(Q/10)
    ** 10: p=0.1 20: p=0.01 etc.
    */

    while (*cp)
    {
        iqual = *cp++;
        if(iqual < qmin)
	{
            ajWarn("FASTQ-ILLUMINA quality value too low '%F' '%S' '%c'",
                   ajFilebuffGetFile(buff), thys->Name,
                   (char) iqual);
            iqual = qmin;
	}
        if(iqual > qmax)
	{
	    ajWarn("FASTQ-ILLUMINA quality value too high '%F' '%S' '%c'",
                   ajFilebuffGetFile(buff), thys->Name,
                   (char) iqual);
	    iqual = qmax;
        }
        thys->Accuracy[i++] = seqQualIllumina[iqual];
    }

/*
**    ajDebug("quality characters %d..%d (%d) '%c' '%c' (%c) "
**            "scores %d..%d (%d)\n",
**            (int) minqual, (int) maxqual, (int) comqual,
**            minqual, maxqual, comqual,
**            (amin + minqual - qmin), (amin + maxqual - qmin),
**            (amin + comqual - qmin));
*/

    return ajTrue;
}





/* @funcstatic seqReadFastqSolexa ********************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the FASTQ format, and processes Illumina/Solexa quality scores.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadFastqSolexa(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjPStr id   = NULL;
    AjPStr acc  = NULL;
    AjPStr sv   = NULL;
    AjPStr desc = NULL;

    ajuint seqlen = 0;
    /*AjPStr qualstr = NULL;*/

/*
**    char minqual;
**    char maxqual;
**    char comqual;
*/

    const char *cp;
    ajint iqual;
    ajlong fpos     = 0;
    ajlong fposb    = 0;
    AjBool ok       = ajTrue;
    const AjPStr badstr = NULL;

    /*ajint amin = 0;*/
    ajint qmin = 59;
    ajint qmax = 126;
    ajuint i;
/*
**    double sval;
**    double pval;
**    double qval;
*/

    /*ajDebug("seqReadFastqSolexa\n");*/

    buff = seqin->Filebuff;

    /* ajFilebuffTrace(buff); */

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fpos,
			     seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    seqin->Records++;

    /*ajDebug("First line: %S\n", seqReadLine);*/

    cp = ajStrGetPtr(seqReadLine);

    if(*cp != '@')
    {
	ajDebug("first line is not FASTQ\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(!ajSeqParseFastq(seqReadLine, &id, &desc))
    {
	ajDebug("first line did not parse as FASTQ\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    seqSetNameNospace(&thys->Name, id);

    if(ajStrGetLen(sv))
	seqSvSave(thys, sv);

    if(ajStrGetLen(acc))
	seqAccSave(thys, acc);

    ajStrAssignS(&thys->Desc, desc);
    ajStrDel(&id);
    ajStrDel(&acc);
    ajStrDel(&sv);
    ajStrDel(&desc);

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);
    while(ok &&
          !ajStrPrefixC(seqReadLine, "+"))
    {
        badstr = seqAppendWarn(&thys->Seq, seqReadLine);

        if(badstr)
            ajWarn("Sequence '%S' has bad character(s) '%S'",
                   thys->Name, badstr);
        seqin->Records++;
        ajDebug("++fastq append line '%S'\n", seqReadLine);
        ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
                                    seqin->Text, &thys->TextPtr);
        ajDebug("++fastq sequence %4u '%S'\n",
                ajStrGetLen(thys->Seq), thys->Seq);
    }

    if(!ok)
    {
	ajDebug("failed to find quality scores\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    seqlen = ajStrGetLen(thys->Seq);

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);

    ajStrAssignClear(&seqQualStr);

    while(ok &&
          ((ajStrGetLen(seqQualStr) < seqlen) ||
           ajStrGetCharFirst(seqReadLine) != '@'))
    {
        seqqualAppendWarn(&seqQualStr, seqReadLine);

        seqin->Records++;
        ajDebug("++fastq append qualities '%S'\n", seqReadLine);
        ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
                                    seqin->Text, &thys->TextPtr);
        ajDebug("++fastq qualities %3u '%S'\n",
                ajStrGetLen(seqQualStr), seqQualStr);
    }

/*
**    minqual = ajStrGetAsciiLow(seqQualStr);
**    maxqual = ajStrGetAsciiHigh(seqQualStr);
**    comqual = ajStrGetAsciiCommon(seqQualStr);
*/

    if(ajStrGetLen(seqQualStr) != seqlen)
    {
	ajDebug("length mismatch seq: %u quality: %u\n",
                seqlen, ajStrGetLen(seqQualStr));
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(ok)
        ajFilebuffClearStore(buff, 1,
                             seqReadLine, seqin->Text, &thys->TextPtr);
    else
        ajFilebuffClear(buff, 0);

    thys->Fpos = fpos;

    /*ajDebug("started at fpos %Ld ok: %B fposb: %Ld\n", fpos, ok, fposb);*/

    /*ajDebug("Solexa: %d..%d (%d)\n",
      (ajint) minqual, (ajint) maxqual, (ajint) comqual);*/
    
    cp = ajStrGetPtr(seqQualStr);
    i=0;

    if(seqlen > thys->Qualsize)
        AJCRESIZE(thys->Accuracy, seqlen);

    thys->Qualsize = seqlen;

    /*
    ** Sanger uses Phred quality calculated from error probability p
    ** Qp = -10 log (p)
    ** Solexa adjusts for the probability of error
    ** Qs = -10 log ((p/(1-p))
    **
    ** For Sanger (phred) p = 1 / 10**(Q/10)
    ** 10: p=0.1 20: p=0.01 etc.
    **
    ** For Solexa (Illumina) ps = p / (1+p) where p is the phred probability
    ** calculation which we use as an intermediate value
    */

    while (*cp)
    {
        iqual = *cp++;
        if(iqual < qmin)
	{
            ajWarn("FASTQ-SOLEXA quality value too low '%F' '%S' '%c'",
                   ajFilebuffGetFile(buff), thys->Name,
                   (char) iqual);
            iqual = qmin;
	}
        if(iqual > qmax)
	{
	    ajWarn("FASTQ-SOLEXA quality value too high '%F' '%S' '%c'",
                   ajFilebuffGetFile(buff), thys->Name,
                   (char) iqual);
	    iqual = qmax;
        }
        thys->Accuracy[i++] = (float) seqQualSolexa[iqual];
    }
/*
**    ajDebug("quality characters %d..%d (%d) '%c' '%c' (%c) "
**            "scores %d..%d (%d)\n",
**            (int) minqual, (int) maxqual, (int) comqual,
**            minqual, maxqual, comqual,
**            (amin + minqual - qmin), (amin + maxqual - qmin),
**            (amin + comqual - qmin));
*/

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
    AjPFilebuff buff;

    const char *cp;
    const AjPStr vacc = NULL;
    ajlong fpos     = 0;
    ajlong fposb    = 0;
    AjBool ok       = ajTrue;

    ajDebug("seqReadDbId\n");

    buff = seqin->Filebuff;
    /* ajFilebuffTrace(buff); */

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fpos,
			     seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    seqin->Records++;

    /* If ; then it is really PIR format */
    if(ajStrGetCharPos(seqReadLine, 3) == ';')
	return ajFalse;

    cp = ajStrGetPtr(seqReadLine);

    if(*cp != '>')
    {
	ajDebug("first line is not FASTA\n");
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    ajStrTokenAssignC(&handle, seqReadLine, "> ");
    ajStrTokenNextParseC(&handle, " \t\n\r", &token);
    ajStrTokenNextParseC(&handle, " \t\n\r", &token);
    seqSetName(thys, token);

    ajStrTokenNextParse(&handle, &token);

    vacc = ajSeqtestIsSeqversion(token);
    if(vacc)
    {
	seqSvSave(thys, token);
	seqAccSave(thys, vacc);
	ajStrTokenNextParseC(&handle, "\n\r", &thys->Desc);
    }
    else if(ajSeqtestIsAccession(token))
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

	ajFilebuffClear(buff, 0);
    }
    else
    {
        /* we know we will succeed from here ... no way to return ajFalse */

        ajFilebuffSetUnbuffered(buff);

	ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, ">"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    seqin->Records++;
	    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				     seqin->Text, &thys->TextPtr);
	}

	if(ok)
	    ajFilebuffClearStore(buff, 1,
				 seqReadLine, seqin->Text, &thys->TextPtr);
	else
	    ajFilebuffClear(buff, 0);
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
    AjPStr token  = NULL;
    AjPStr idline = NULL;
    AjPStr tmpline = NULL;

    AjBool dofeat       = ajFalse;

    AjPStrTok handle2 = NULL;
    AjPStr    token2  = NULL;
    AjPStr    seqReadLine2 = NULL;

    AjBool ok;
    AjPFilebuff buff;

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

    if(!ajBuffreadLineStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    ajDebug("nbrf first line:\n%S", seqReadLine);

    if(!ajRegExec(seqRegNbrfId, seqReadLine))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	return ajFalse;
    }

    ajRegSubI(seqRegNbrfId, 1, &token);
    ajRegSubI(seqRegNbrfId, 2, &thys->Name);
    ajDebug("parsed line name '%S' token '%S' token(1) '%c'\n",
	    thys->Name, token, ajStrGetCharFirst(token));
    ajStrAssignSubS(&idline, seqReadLine, 4, -1);

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

    if(!ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    ajStrAssignS(&thys->Desc, seqReadLine);

    if(ajStrGetCharLast(thys->Desc) == '\n')
	ajStrCutEnd(&thys->Desc, 1);

    /* read on, looking for feature and sequence lines */

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

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
		    seqTaxSave(thys, token2, 1);
	    }

	    if(ajStrGetCharFirst(seqReadLine) == 'R')
	    {		     /* skip reference lines with no prefix */
		while((ok=ajBuffreadLineStore(buff,&seqReadLine,
					     seqin->Text, &thys->TextPtr)))
		    if(ajStrGetCharPos(seqReadLine,1)==';' ||
		       ajStrGetCharFirst(seqReadLine)=='>')
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

		    ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		    /* ajDebug("NBRF FEAT saved line:\n%S", seqReadLine); */
		}
	    }
	}
	if(ok)
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);

	/* SRS 7 and SRS 8.0 put an extra ID line in here */

	/* SRS 8.1 is even worse - it has a peculiar bug that repeats
	   the ID line but with a few digits in front, and then repeats the
	   description */

	/* just for another oddity ... the extra ID line always starts >P1;
	   even if the protein is a fragment */

	if(ok && !ajStrGetLen(thys->Seq) &&
	   (ajStrFindAnyK(seqReadLine, '>') != -1))
	{
	    ajStrAssignS(&tmpline, seqReadLine);
	    ajStrTrimStartC(&tmpline,"0123456789");
	    ajStrCutStart(&tmpline, 4);

	    if(ajStrMatchS(tmpline, idline))
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);

		if(!ajStrIsWhite(seqReadLine)) /* SRS 8.1 description line */
		    ok = ajBuffreadLineStore(buff, &seqReadLine,
					    seqin->Text, &thys->TextPtr);
	    }
	}

    }

    if(ajStrGetCharLast(thys->Seq) == '*')
	ajStrCutEnd(&thys->Seq, 1);

    if(ok)
	ajFilebuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    else
	ajFilebuffClear(buff, 0);

    if(dofeat)
    {
	ajDebug("seqin->Ftquery Handle %x\n",
		seqin->Ftquery->Handle);
	ajFeattableDel(&seqin->Fttable);
	thys->Fttable = ajFeattableNewRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    ajStrDel(&idline);
    ajStrDel(&tmpline);
    ajStrDel(&seqReadLine2);
    ajStrTokenDel(&handle2);
    ajStrDel(&token);
    ajStrDel(&token2);

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
    AjBool ok;

    ajuint len     = 0;
    AjBool incomment = ajFalse;

    AjPFilebuff buff;

    buff = seqin->Filebuff;

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    while (ok && ajStrIsWhite(seqReadLine))
        ok = ajBuffreadLineStore(buff, &seqReadLine,
                                 seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    seqin->Records++;

    ajDebug("seqReadGcg first line ok: %B\n'%S'\n", ok, seqReadLine);

    /* test GCG 9.x file types if available */
    /* any type on the .. line will override this */

    if(ajStrPrefixC(seqReadLine, "!!NA_SEQUENCE"))
	ajSeqSetNuc(thys);
    else if(ajStrPrefixC(seqReadLine, "!!AA_SEQUENCE"))
	ajSeqSetProt(thys);

    if(!seqGcgDots(thys, seqin, &seqReadLine, seqMaxGcglines, &len))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	return ajFalse;
    }

    ajDebug("   Gcg dots read ok len: %d\n", len);

    while(ok &&  (ajSeqGetLen(thys) < len))
    {
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(ok)
	{
	    seqin->Records++;
	    seqAppendCommented(&thys->Seq, &incomment, seqReadLine);
	    ajDebug("line %d seqlen: %d ok: %B\n",
		    seqin->Records, ajSeqGetLen(thys), ok);
	}
    }

    ajDebug("lines: %d ajSeqGetLen : %d len: %d ok: %B\n",
	    seqin->Records, ajSeqGetLen(thys), len, ok);

    ajFilebuffClear(buff, 0);


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
    AjPStr db        = NULL;
    AjPStr desc      = NULL;

    AjPFilebuff buff;

    AjBool ok;
    const AjPStr badstr = NULL;

    buff = seqin->Filebuff;

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    ajStrAssignClear(&id);
    ajStrAssignClear(&acc);
    ajStrAssignClear(&sv);
    ajStrAssignClear(&gi);
    ajStrAssignClear(&desc);


    if(!ajSeqParseNcbi(seqReadLine,&id,&acc,&sv,&gi,&db,&desc))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	ajStrDel(&id);
	ajStrDel(&acc);
	ajStrDel(&sv);
	ajStrDel(&gi);
	ajStrDel(&db);
	ajStrDel(&desc);

	return ajFalse;
    }

    ajDebug("parsed id '%S' acc '%S' sv '%S' gi '%S' db '%S' (%S) desc '%S'\n",
	    id, acc, sv, gi, db, thys->Setdb, desc);

    ajStrAssignS(&thys->Setdb, db);
    ajDebug("set setdb '%S' db '%S'\n", thys->Setdb, thys->Db);

    if(ajStrGetLen(gi))
	ajStrAssignS(&thys->Gi, gi);

    if(ajStrGetLen(sv))
	seqSvSave(thys, sv);

    if(ajStrGetLen(acc))
	seqAccSave(thys, acc);

    seqSetName(thys, id);
    ajStrAssignS(&thys->Desc, desc);


    if(ajStrGetLen(seqin->Inseq))
    {				       /* we have a sequence to use */
	ajStrAssignS(&thys->Seq, seqin->Inseq);

	if(seqin->Text)
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);

	ajFilebuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    }
    else
    {
        /* we know we will succeed from here ... no way to return ajFalse */

        ajFilebuffSetUnbuffered(buff);

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, ">"))
	{
	    badstr = seqAppendWarn(&thys->Seq, seqReadLine);

	    if(badstr)
		ajWarn("Sequence '%S' has bad character(s) '%S'",
			   thys->Name, badstr);
	    seqin->Records++;
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

	if(ok)
	    ajFilebuffClearStore(buff, 1,
				 seqReadLine, seqin->Text, &thys->TextPtr);
	else
	    ajFilebuffClear(buff, 0);
    }

    ajDebug("seqReadNcbi Setdb '%S' Db '%S'\n", thys->Setdb, thys->Db);
    ajStrTokenDel(&handle);
    ajStrDel(&id);
    ajStrDel(&acc);
    ajStrDel(&sv);
    ajStrDel(&gi);
    ajStrDel(&db);
    ajStrDel(&desc);

    return ajTrue;
}




/* @funcstatic seqReadGifasta *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using NCBI format. However, unlike NCBI format it uses the GI number
** as the sequence ID
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGifasta(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr id        = NULL;
    AjPStr acc       = NULL;
    AjPStr sv        = NULL;
    AjPStr gi        = NULL;
    AjPStr db        = NULL;
    AjPStr desc      = NULL;

    AjPFilebuff buff;

    AjBool ok;


    buff = seqin->Filebuff;

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    ajStrAssignClear(&id);
    ajStrAssignClear(&acc);
    ajStrAssignClear(&sv);
    ajStrAssignClear(&gi);
    ajStrAssignClear(&desc);


    if(!ajSeqParseNcbi(seqReadLine,&id,&acc,&sv,&gi,&db,&desc) ||
       !ajStrGetLen(gi))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	ajStrDel(&id);
	ajStrDel(&acc);
	ajStrDel(&sv);
	ajStrDel(&gi);
	ajStrDel(&desc);

	return ajFalse;
    }

    ajDebug("parsed id '%S' acc '%S' sv '%S' gi '%S' db '%S' (%S) desc '%S'\n",
	    id, acc, sv, gi, db, thys->Setdb, desc);

    ajStrAssignS(&thys->Gi, gi);
    
    ajStrAssignS(&thys->Setdb, db);
    ajDebug("set setdb '%S' db '%S'\n", thys->Setdb, thys->Db);

    if(ajStrGetLen(sv))
	seqSvSave(thys, sv);

    if(ajStrGetLen(acc))
	seqAccSave(thys, acc);

    seqSetName(thys, gi);
    ajStrAssignS(&thys->Desc, desc);


    if(ajStrGetLen(seqin->Inseq))
    {				       /* we have a sequence to use */
	ajStrAssignS(&thys->Seq, seqin->Inseq);

	if(seqin->Text)
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);

	ajFilebuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    }
    else
    {
        /* we know we will succeed from here ... no way to return ajFalse */

        ajFilebuffSetUnbuffered(buff);

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);



	while(ok && !ajStrPrefixC(seqReadLine, ">"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    seqin->Records++;
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

	if(ok)
	    ajFilebuffClearStore(buff, 1,
				 seqReadLine, seqin->Text, &thys->TextPtr);
	else
	    ajFilebuffClear(buff, 0);
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
** Read a Selex file. Assumed a comment on the first line but this may
** not be true.
**
** This format can read anything that looks like a block of "name sequence"
** data. The names are even allowed to change in later blocks.
**
** The format was used by HMMER, but that package now prefers the better
** annotated "Stockholm" format used by Pfam and Rfam.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadSelex(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff  = seqin->Filebuff;
    AjPStr      line  = NULL;
    SeqPSelex    selex;
    ajuint       n      = 0;
    const char  *p     = NULL;
    AjBool      ok     = ajFalse;
    AjBool      isseq  = ajFalse;
    AjBool      named  = ajFalse;
    AjBool      head   = ajTrue;
    ajuint       sqcnt  = 0;
    ajuint       i;
    char        c      = '\0';
    AjBool      first  = ajTrue;

    line = ajStrNew();


    if(seqin->Data)
	selex = seqin->Data;
    else
    {
	ajFilebuffSetBuffered(buff);    /* must buffer to test sequences */

	/* First count the sequences, and get any header information */
	while(!isseq && (ok=ajBuffreadLine(buff,&line)))
	{
	    if(first)
	    {
		first=ajFalse;

		if(!ajStrPrefixC(line,"#"))
		{
		    ajStrDel(&line);
		    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

		    return ajFalse;
		}
	    }

	    ajStrRemoveWhiteExcess(&line);
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

	while(ok && ajBuffreadLine(buff,&line))
	{
	    ajStrRemoveWhiteExcess(&line);
	    p = ajStrGetPtr(line);

	    if(*p=='#')
		continue;

	    if(!*p)
		ok = ajFalse;
	    else
		++n;
	}

	ajFilebuffClear(buff,-1);
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	buff->Fpos = 0;
	ajFileSeek(buff->File, 0L, 0);
	selex = selexNew(n);

	/* now read it for real */

	while(head && ajBuffreadLineStore(buff,&line,
					 seqin->Text, &thys->TextPtr))
	{
	    if(ajStrPrefixC(line,"#=RF") ||ajStrPrefixC(line,"#=CS"))
		break;

	    if(ajStrPrefixC(line,"#="))
	    {
		head=seqSelexHeader(&selex,line,&named,&sqcnt);
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
	    ok = ajBuffreadLineStore(buff,&line,
				     seqin->Text, &thys->TextPtr);
	    ajDebug("... in loop ok: %B\n", ok);
	}

	ajDebug(" Block done. More data (ok): %B\n", ok);

	if(ok)
	    ajFilebuffClearStore(buff, 1,
				 line, seqin->Text, &thys->TextPtr);
	else
	    ajFilebuffClear(buff, 0);

	seqin->Data = selex;
    }


    /* At this point the Selex structure is fully loaded */
    if(selex->Count >= selex->n)
    {
	selexDel(&selex);
	seqin->Data = NULL;
	ajStrDel(&line);

	return ajFalse;
    }

    i = selex->Count;

    seqSelexCopy(&thys,selex,i);

    ++selex->Count;

    ajFilebuffClear(buff,0);

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
    AjPFilebuff buff  = seqin->Filebuff;
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

    ajuint i     = 0;
    ajuint n     = 0;
    ajuint  scnt = INT_MAX;

    line = ajStrNew();

    ajDebug("seqReadStockholm EOF:%B Data:%x\n",
	    ajFilebuffIsEof(buff), seqin->Data);
    if(seqin->Data)
	stock = seqin->Data;
    else
    {
	ajFilebuffSetBuffered(buff); /* must buffer to test sequences */
	ok=ajBuffreadLineStore(buff, &line, seqin->Text, &thys->TextPtr);
        ajStrTrimWhiteEnd(&line);

	if(!ok || !ajStrPrefixC(line,"# STOCKHOLM 1."))
	{
	    if (ok)
		ajDebug("Stockholm: bad first line: %S", line);
	    else
		ajDebug("Stockholm: no first line\n");

	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	    ajStrDel(&line);

	    return ajFalse;
	}

	ajDebug("Stockholm: good first line: %S", line);

	while(ok && (ajStrPrefixC(line, "#") || !ajStrGetLen(line)))
	{
	    if(ajStrPrefixC(line,"#=GF SQ"))
	    {
		ajFmtScanS(line,"%*s%*s%d",&n);
		ajDebug("Stockholm: parsed SQ line of %d sequences\n", n);
	    }

	    ok=ajBuffreadLineStore(buff, &line, seqin->Text, &thys->TextPtr);
            ajStrTrimWhiteEnd(&line);
	    ajDebug("Stockholm: SQ search: %S\n", line);
	}

	if (!n)				/* no SQ line, count first block */
	{
	    while(ok && ajStrGetLen(line))
	    {
		if(!ajStrPrefixC(line, "#") &&
                   !ajStrMatchC(line, "//"))
                    n++;

		ok=ajBuffreadLineStore(buff,&line,
                                       seqin->Text, &thys->TextPtr);
                ajStrTrimWhiteEnd(&line);
		ajDebug("Stockholm: block %d read: %S\n", n, line);
	    }

	    ajDebug("Stockholm: read block of %d sequences\n", n);
	}

	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	ok=ajBuffreadLineStore(buff,&line,
			       seqin->Text, &thys->TextPtr);
        ajStrTrimWhiteEnd(&line);
	stock = stockholmNew(n);

	ajDebug("Created stockholm data object size: %d\n", n);

	word  = ajStrNew();
	token = ajStrNew();
	post  = ajStrNew();

	if(!seqRegStockholmSeq)
	    seqRegStockholmSeq = ajRegCompC("^([^ \t\n]+)[ \t]+"
                                            "([^ \t\n]+)[ \t]+");
	while(ok && !ajStrPrefixC(line,"//"))
	{
	    if(ajRegExec(seqRegStockholmSeq,line))
	    {
		ajRegSubI(seqRegStockholmSeq,1,&word);
		ajRegSubI(seqRegStockholmSeq,2,&token);
		ajRegPost(seqRegStockholmSeq,&post);
		ajStrRemoveLastNewline(&post);

                ajDebug("Stockholm: regex word '%S' token '%S' post '%S'\n",
                        word, token, post);
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

		else if(!ajStrCmpC(word,"#=GS"))
		{
		    if(gsf)
		    {
			gsf = ajFalse;
			ajStrAssignS(&stock->gs,line);
		    }
		    else
			ajStrAppendS(&stock->gs,line);
		}

		else if(!ajStrCmpC(word,"#=GC"))
		{
		    if(!ajStrCmpC(token,"SS_cons"))
			ajStrAssignS(&stock->sscons,post);
		    else if(!ajStrCmpC(token,"SA_cons"))
			ajStrAssignS(&stock->sacons,post);
		    else if(!ajStrCmpC(token,"seq_cons"))
			ajStrAssignS(&stock->sqcons,post);
		}
	    }
	    else if (ajStrGetLen(line))
	    {
                if(ajStrParseCount(line) > 1)
                {
                    ++scnt;

                    if(scnt >= n)
                        scnt = 0;

                    ajFmtScanS(line,"%S%S", &namstr,&seqstr);
                    ajDebug("Stockholm: scnt: %d namstr '%S' seqstr '%S'\n",
                            scnt,namstr,seqstr);

                    if(!ajStrGetLen(stock->name[scnt]))
                        ajStrAppendS(&stock->name[scnt], namstr);
                    else
                    {
                        if(!ajStrMatchS(namstr, stock->name[scnt]))
                            ajWarn("Bad stockholm format found id %d '%S' "
                                   "expect '%S'",
                                   scnt, namstr, stock->name[scnt]);
                    }

                    ajStrRemoveLastNewline(&seqstr);
                    ajStrAppendS(&stock->str[scnt], seqstr);
                }
                else
                {
                    ajStrRemoveLastNewline(&line);
                    ajStrAppendS(&stock->str[scnt], line);
                }
                
	    }

	    ok = ajBuffreadLineStore(buff,&line,
				     seqin->Text, &thys->TextPtr);
            ajStrTrimWhiteEnd(&line);
	}

	while(ok && !ajStrPrefixC(line, "# STOCKHOLM 1."))
	    ok = ajBuffreadLineStore(buff,&line,
				     seqin->Text, &thys->TextPtr);

	if(ok)
	    ajFilebuffClearStore(buff, 1,
				 line, seqin->Text, &thys->TextPtr);
	else
	    ajFilebuffClear(buff, 0);

	ajStrDel(&word);
	ajStrDel(&token);
	ajStrDel(&post);
	ajStrDel(&namstr);
	ajStrDel(&seqstr);
	seqin->Data = stock;

        ajFilebuffClear(buff,0);
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

    ajDebug("Stockholm returning %d/%d '%S' len: %d\n",
            stock->Count, stock->n, ajSeqGetNameS(thys),ajSeqGetLen(thys));

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
** @param [r] n [ajuint] index into selex object
** @return [void]
** @@
******************************************************************************/

static void seqSelexCopy(AjPSeq *thys, SeqPSelex selex, ajuint n)
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
    ajStrAssignS(&sdata->sqcons,stock->sqcons);
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
** @param [r] beg  [ajuint] start of info in src
** @param [r] end  [ajuint] end of info in src
** @return [void]
** @@
******************************************************************************/

static void seqSelexAppend(const AjPStr src, AjPStr *dest,
			   ajuint beg, ajuint end)
{
    const char *p = NULL;
    char c;
    ajuint len;
    ajuint i;
    ajuint pad = 0;

    len = end-beg+1;
    p   = ajStrGetPtr(src);

    ajDebug("seqSelexAppend srclen: %u beg: %u end: %u src '%S'\n",
	    ajStrGetLen(src), beg, end, src);

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
** @param [w] named  [AjBool*] Whether names of sequences have been read
** @param [w] sqcnt  [ajuint*] Number of SQ names read
** @return [AjBool] ajTrue if the line contained header information
** @@
******************************************************************************/

static AjBool seqSelexHeader(SeqPSelex *thys, const AjPStr line,
			     AjBool *named, ajuint *sqcnt)
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
	ajStrRemoveWhiteExcess(&pthis->de);

	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=AU"))
    {
	ajStrAssignC(&pthis->au,ajStrGetPtr(line)+5);
	ajStrRemoveWhiteExcess(&pthis->au);

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
** @param [w] begin  [ajuint*] start pos
** @param [w] end  [ajuint*] end pos
** @return [void]
** @@
******************************************************************************/

static void seqSelexPos(const AjPStr line, ajuint *begin, ajuint *end)
{
    ajuint pos = 0;
    ajuint len = 0;

    const char  *p;

    /*
    **  Selex sequence info can start any number of spaces
    **  after the names so we need to find out where to
    **  start counting chars from and where to end
     */

    len  = ajStrGetLen(line) - 1;

    if(!len) 
    {
        *begin=0;
        *end=0;

        return;
    }
    
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

    ajDebug("seqSelexPos len:%u pos:%u begin:%u end:%u\n",
            len, pos, *begin, *end);

    return;
}




/* @funcstatic seqSelexReadBlock **********************************************
**
** Read a block of sequence information from a selex file
**
** @param [w] thys [SeqPSelex*] Selex object
** @param [w] named  [AjBool*] Whether names of sequences have been read
** @param [r] n  [ajuint] Number of sequences in Selex file
** @param [u] line [AjPStr*] Line from Selex file
** @param [u] buff  [AjPFilebuff] Selex file buffer
** @param [r] store [AjBool] store if ajTrue
** @param [w] astr [AjPStr*] string to append to
** @return [AjBool] ajTrue if data was read.
** @@
******************************************************************************/

static AjBool seqSelexReadBlock(SeqPSelex *thys, AjBool *named, ajuint n,
				AjPStr *line, AjPFilebuff buff,
				AjBool store, AjPStr *astr)
{
    SeqPSelex pthis;
    AjPStr *seqs = NULL;
    AjPStr *ss   = NULL;

    AjPStr rf = NULL;
    AjPStr cs = NULL;
    ajuint  i;
    ajuint  begin=0;
    ajuint  end=0;
    AjBool ok;
    ajuint  cnt;
    AjPStr tmp    = NULL;
    AjBool haverf = ajFalse;
    AjBool havecs = ajFalse;
    AjBool havess = ajFalse;

    pthis = *thys;

    begin = INT_MAX;
    end   = 0;

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


    while(ajStrPrefixC(*line,"\n"))
        ok = ajBuffreadLineStore(buff,line, store, astr);
   
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
		    ajWarn("Selex format sequence names do not match "
                           "['%S' '%S']",
			   pthis->name[cnt],tmp);
	    }

	    ajStrAssignS(&seqs[cnt],*line);
	    ++cnt;
	}

	ok = ajBuffreadLineStore(buff,line, store, astr);

	if(ajStrPrefixC(*line,"\n"))
	    ok = ajFalse;
    }

    ajDebug("selexReadBlock block done line '%S' n: %u rf:%B cs:%B ss:%B\n",
	    *line, n, haverf, havecs, havess);

    if(cnt != n)
        ajWarn("Selex format expected %u sequences in block, found %u",
               n, cnt);
    if(cnt > n)
        cnt = n;

    if(haverf)
	seqSelexAppend(rf,&pthis->rf,begin,end);

    if(havecs)
	seqSelexAppend(cs,&pthis->cs,begin,end);

    for(i=0;i<cnt;++i)
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
    AjPStr token  = NULL;
    AjPFilebuff buff;
    AjBool incomment = ajFalse;

    buff = seqin->Filebuff;

    if(!seqRegStadenId)
	seqRegStadenId = ajRegCompC("^[<]([^>-]+)[-]*[>]");

    if(!ajBuffreadLineStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    seqin->Records++;

    if(ajRegExec(seqRegStadenId, seqReadLine))
    {
	ajRegSubI(seqRegStadenId, 1, &token);
	seqSetName(thys, token);
	ajDebug("seqReadStaden name '%S' token '%S'\n",
		thys->Name, token);
	ajRegPost(seqRegStadenId, &token);
	seqAppendCommented(&thys->Seq, &incomment, token);
	ajStrDel(&token);
    }
    else
    {
	seqSetName(thys, seqin->Filename);
	seqAppendCommented(&thys->Seq, &incomment, seqReadLine);
    }

    while(ajBuffreadLineStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr))
    {
	seqAppendCommented(&thys->Seq, &incomment, seqReadLine);
	seqin->Records++;
    }

    if(!seqin->Records)         /* but we have read at least 1 line */
        return ajFalse;

    ajFilebuffClear(buff, 0);

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
    AjPFilebuff buff;

    ajDebug("seqReadText\n");

    buff = seqin->Filebuff;

    while(ajBuffreadLineStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr))
    {
	ajDebug("read '%S'\n", seqReadLine);
	seqAppend(&thys->Seq, seqReadLine);
	seqin->Records++;
    }

    ajDebug("read %d lines\n", seqin->Records);
    ajFilebuffClear(buff, 0);

    if(!seqin->Records)
	return ajFalse;

    seqSetNameFile(thys, seqin);

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
    AjPFilebuff buff;
    ajDebug("seqReadRaw\n");

    buff = seqin->Filebuff;

    if(!seqRegRawNonseq)
	seqRegRawNonseq = ajRegCompC("[^A-Za-z0-9 \t\n\r*-]");

    while(ajBuffreadLineStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr))
    {
	ajDebug("read '%S'\n", seqReadLine);

	if(ajRegExec(seqRegRawNonseq, seqReadLine))
	{
	    ajDebug("seqReadRaw: Bad character found in line: %S\n",
		    seqReadLine);
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	    ajStrAssignClear(&thys->Seq);

	    return ajFalse;
	}
	seqAppend(&thys->Seq, seqReadLine);
	seqin->Records++;
        ajDebug("read %d lines\n", seqin->Records);
    }

    ajFilebuffClear(buff, 0);

    if(!seqin->Records)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic seqReadIgstrict *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using IntelliGenetics format.
**
** Requires a trailing number at the end of the sequence
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadIgstrict(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjBool endnum = ajFalse;
    AjBool ok = ajTrue;

    buff = seqin->Filebuff;

    do
    {
        if(seqin->Records){
            ajStrCutStart(&seqReadLine, 1); /* trim the semi colon */
            ajStrRemoveWhiteExcess(&seqReadLine);
            if(ajStrGetLen(thys->Desc))
                ajStrAppendK(&thys->Desc, ' ');
            ajStrAppendS(&thys->Desc, seqReadLine);
        }
	/* skip comments with ';' prefix */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(ok)
            seqin->Records++;
    } while(ok && ajStrPrefixC(seqReadLine, ";"));

    if(!ok)
    {
        ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	return ajFalse;
    }

    seqSetName(thys, seqReadLine);
    seqin->Records++;

    while(ajBuffreadLineStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr) &&
	  !ajStrPrefixC(seqReadLine, ";"))
    {
        ajStrRemoveWhiteExcess(&seqReadLine);
        if(ajStrSuffixC(seqReadLine, "1"))
            endnum = ajTrue;
        else if(ajStrSuffixC(seqReadLine, "2"))
            endnum = ajTrue;
        else
            endnum = ajFalse;
	seqAppend(&thys->Seq, seqReadLine);
	seqin->Records++;
    }

    if(!endnum)
    {
        ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
        return ajFalse;
    }
    
    if(ajStrPrefixC(seqReadLine, ";"))
        ajFilebuffClear(buff, 1);
    else
        ajFilebuffClear(buff, 0);

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
    AjPFilebuff buff;
    AjBool ok = ajTrue;

    buff = seqin->Filebuff;

    do
    {
        if(seqin->Records){
            ajStrCutStart(&seqReadLine, 1); /* trim the semi colon */
            ajStrRemoveWhiteExcess(&seqReadLine);
            if(ajStrGetLen(thys->Desc))
                ajStrAppendK(&thys->Desc, ' ');
            ajStrAppendS(&thys->Desc, seqReadLine);
        }
	/* skip comments with ';' prefix */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(ok)
            seqin->Records++;
    } while(ok && ajStrPrefixC(seqReadLine, ";"));

    if(!ok)
    {
        ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	return ajFalse;
    }

    /* we know we will succeed from here ... no way to return ajFalse */

    ajFilebuffSetUnbuffered(buff);

    seqSetName(thys, seqReadLine);
    seqin->Records++;

    while(ajBuffreadLineStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr) &&
	  !ajStrPrefixC(seqReadLine, ";"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	seqin->Records++;
    }

    if(ajStrPrefixC(seqReadLine, ";"))
        ajFilebuffClear(buff, 1);
    else
        ajFilebuffClear(buff, 0);

    return ajTrue;
}




/* @funcstatic seqReadPdb **************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using PDB protein databank format using ATOM records.
**
** See seqReadPdbseq for parsing the SEQRES records
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadPdb(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjPStr name  = NULL;
    AjPStr alnname  = NULL;
    AjPStr token = NULL;
    AjPStr chain = NULL;
    AjPTable alntable    = NULL;
    SeqPMsfItem alnitem  = NULL;
    AjPList alnlist      = NULL;
    SeqPMsfData alndata  = NULL;
    char aa;
    ajuint nseq = 0;
    ajuint i;
    AjBool ok = ajTrue;
    AjPStr aa3 = NULL;
    ajuint iaa = 0;
    ajuint lastaa = 0;
    AjPStr model = NULL;

    buff = seqin->Filebuff;

    ajDebug("seqReadPdb seqin->Data %x\n", seqin->Data);

    if(!seqin->Data)
    {					/* start of file */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	seqin->Records++;

	ajDebug("first line:\n'%S'\n", seqReadLine);

	if(!ajStrPrefixC(seqReadLine, "HEADER    "))
	{
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	ajStrAssignSubS(&name,seqReadLine, 62, 71);
	ajStrTrimWhite(&name);

	ajDebug("first line OK name '%S'\n", name);

	seqin->Data = AJNEW0(alndata);
	alndata->Table = alntable = ajTablestrNew();
	alnlist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);

	while(ok && !ajStrMatchC(seqReadLine, "END"))
	{
	    seqin->Records++;

	    if(ajStrPrefixC(seqReadLine, "MODEL"))
	    {
                ajStrAssignSubS(&model, seqReadLine, 7, 14);
                ajStrTrimWhite(&model);
            }
	    else if(ajStrPrefixC(seqReadLine, "ATOM"))
	    {
		if(!alnitem)
		    AJNEW0(alnitem);

		ajStrKeepRange(&seqReadLine, 0,71);

		ajStrAssignSubS(&aa3, seqReadLine, 17, 19);
		ajStrAssignSubS(&chain, seqReadLine, 21, 21);
		ajStrAssignSubS(&token, seqReadLine, 22, 25);
		ajStrToUint(token, &iaa);

		if(iaa > lastaa)
		{
		    if(ajResidueFromTriplet(aa3,&aa))
			seqAppendK(&alnitem->Seq, aa);
		    lastaa = iaa;
		}

	    }

	    else if(ajStrPrefixC(seqReadLine, "TER"))
	    {
                if(alnitem && !ajStrGetLen(alnitem->Seq))
                {
                    ajTableRemoveKey(alntable, alnitem->Name,
                                     (void**) &alnname);
                    ajStrDel(&alnname);
                    seqMsfItemDel(&alnitem);
                }
                else 
                {
                    nseq++;
                    ajFmtPrintS(&token, "%S-%S", name, chain);

                    if(ajStrGetLen(model))
                        ajStrAppendS(&token, model);

                    seqitemSetName(alnitem, token);
                    ajStrAssignS(&alnname, alnitem->Name);
                    alnitem->Weight = 1.0;
                    ajTablePut(alntable, alnname, alnitem);
                    alnname = NULL;
                    ajListstrPushAppend(alnlist, ajStrNewS(alnitem->Name));
                    alnitem = NULL;
                }

                lastaa = 0;
	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

        ajStrDel(&token);
        ajStrDel(&name);
        ajStrDel(&chain);
        ajStrDel(&aa3);
        ajStrDel(&model);

        if(!nseq)
        {
            seqMsfDataDel((SeqPMsfData*) &seqin->Data);
            ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	ajDebug("PDB Entry has %d sequences\n", nseq);
	ajListstrTrace(alnlist);
	ajTableTrace(alntable);
	ajTableMap(alntable, seqMsfTabList, NULL);

	alndata->Names = AJCALLOC(nseq, sizeof(*alndata->Names));

	for(i=0; i < nseq; i++)
	{
	    ajListstrPop(alnlist, &alndata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, alndata->Names[i]);
	}

	ajListstrFreeData(&alnlist);

	ajTableMap(alntable, seqMsfTabList, NULL);
	alndata->Nseq = nseq;
	alndata->Count = 0;
	alndata->Bufflines = seqin->Records;
	ajDebug("PDB format read %d lines\n", seqin->Records);
    }

    alndata = seqin->Data;
    alntable = alndata->Table;

    if(alndata->Count >= alndata->Nseq)
    {					/* all done */
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);

	return ajFalse;
    }

    i = alndata->Count;
    ajDebug("returning [%d] '%S'\n", i, alndata->Names[i]);
    alnitem = ajTableFetch(alntable, alndata->Names[i]);
    ajStrAssignS(&thys->Name, alndata->Names[i]);

    thys->Weight = alnitem->Weight;
    ajStrAssignS(&thys->Seq, alnitem->Seq);

    alndata->Count++;

    return ajTrue;
}




/* @funcstatic seqReadPdbseq **************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using PDB protein databank format using the SEQRES records.
**
** This is the original sequence, see seqReadPdb for parsing the ATOM records
** which give the sequence observed in the structure.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadPdbseq(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjPStrTok handle = NULL;
    AjPStr name  = NULL;
    AjPStr alnname  = NULL;
    AjPStr token = NULL;
    AjPStr chain = NULL;
    AjPTable alntable    = NULL;
    SeqPMsfItem alnitem  = NULL;
    AjPList alnlist      = NULL;
    SeqPMsfData alndata  = NULL;
    char aa;
    ajuint iseq = 0;
    ajuint nseq = 0;
    ajuint i;
    AjBool ok = ajTrue;

    buff = seqin->Filebuff;

    ajDebug("seqReadPdbseq seqin->Data %x\n", seqin->Data);

    if(!seqin->Data)
    {					/* start of file */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	seqin->Records++;

	ajDebug("first line:\n'%S'\n", seqReadLine);

	if(!ajStrPrefixC(seqReadLine, "HEADER    "))
	{
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	ajStrAssignSubS(&name,seqReadLine, 62, 71);
	ajStrTrimWhite(&name);

	ajDebug("first line OK name '%S'\n", name);

	seqin->Data = AJNEW0(alndata);
	alndata->Table = alntable = ajTablestrNew();
	alnlist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);

	while(ok && !ajStrMatchC(seqReadLine, "END"))
	{
	    seqin->Records++;

	    if(ajStrPrefixC(seqReadLine, "SEQRES"))
	    {
		ajStrKeepRange(&seqReadLine, 0,71);
		ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
		ajStrTokenNextParse(&handle, &token);	/* 'SEQRES' */

		ajStrTokenNextParse(&handle, &token);	/* number */
		ajStrToUint(token, &iseq);

		ajStrTokenNextParse(&handle, &chain);	/* chain letter */

		if(iseq == 1)
		{
                    if(alnitem && !ajStrGetLen(alnitem->Seq))
                    {
                        nseq--;
                        ajListstrPopLast(alnlist, &alnname);
                        ajTableRemoveKey(alntable, alnitem->Name,
                                         (void**) &alnname);
                        ajStrDel(&alnname);
                        seqMsfItemDel(&alnitem);
                    }
                    
		    nseq++;
		    ajFmtPrintS(&token, "%S-%S", name, chain);
                    AJNEW0(alnitem);
		    seqitemSetName(alnitem, token);
		    ajStrAssignS(&alnname, alnitem->Name);
		    alnitem->Weight = 1.0;
		    ajTablePut(alntable, alnname, alnitem);
		    alnname = NULL;
		    ajListstrPushAppend(alnlist, ajStrNewS(alnitem->Name));
		}

		while(ajStrTokenNextParse(&handle, &token))
		    if(ajResidueFromTriplet(token,&aa))
			seqAppendK(&alnitem->Seq, aa);
	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

        if(alnitem && !ajStrGetLen(alnitem->Seq))
        {
            nseq--;
            ajListstrPopLast(alnlist, &alnname);
            ajTableRemoveKey(alntable, alnitem->Name,
                             (void**) &alnname);
            ajStrDel(&alnname);
            seqMsfItemDel(&alnitem);
        }

	if(!nseq)
	{
            ajStrDel(&token);
            ajStrDel(&name);
            ajStrDel(&chain);
            seqMsfDataDel((SeqPMsfData*) &seqin->Data);
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

        
	ajDebug("PDB Entry has %d sequences\n", nseq);
	ajListstrTrace(alnlist);
	ajTableTrace(alntable);
	ajTableMap(alntable, seqMsfTabList, NULL);

	alndata->Names = AJCALLOC(nseq, sizeof(*alndata->Names));

	for(i=0; i < nseq; i++)
	{
	    ajListstrPop(alnlist, &alndata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, alndata->Names[i]);
	}

	ajListstrFreeData(&alnlist);

	ajTableMap(alntable, seqMsfTabList, NULL);
	alndata->Nseq = nseq;
	alndata->Count = 0;
	alndata->Bufflines = seqin->Records;
	ajDebug("PDBSEQ format read %d lines\n", seqin->Records);
    }

    alndata = seqin->Data;
    alntable = alndata->Table;
    if(alndata->Count >= alndata->Nseq)
    {					/* all done */
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);
	return ajFalse;
    }

    i = alndata->Count;
    ajDebug("returning [%d] '%S'\n", i, alndata->Names[i]);
    alnitem = ajTableFetch(alntable, alndata->Names[i]);
    ajStrAssignS(&thys->Name, alndata->Names[i]);

    thys->Weight = alnitem->Weight;
    ajStrAssignS(&thys->Seq, alnitem->Seq);

    alndata->Count++;

    ajStrDel(&token);
    ajStrDel(&name);
    ajStrDel(&chain);

    return ajTrue;
}




/* @funcstatic seqReadPdbnuc **************************************************
**
** Given nucleotide data in a sequence structure,
** tries to read everything needed using PDB protein databank format
** using the SEQRES records.
**
** This is the sequence observed in the structure. See seqReadPdbnucseq
** for parsing the SEQRES records which give the original sequence.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadPdbnuc(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjPStr name  = NULL;
    AjPStr alnname  = NULL;
    AjPStr token = NULL;
    AjPStr chain = NULL;
    AjPTable alntable    = NULL;
    SeqPMsfItem alnitem  = NULL;
    AjPList alnlist      = NULL;
    SeqPMsfData alndata  = NULL;
    char aa;
    ajuint nseq = 0;
    ajuint i;
    AjBool ok = ajTrue;
    AjPStr aa3 = NULL;
    ajuint iaa = 0;
    ajuint lastaa = 0;
    AjPStr model = NULL;

    buff = seqin->Filebuff;

    ajDebug("seqReadPdbnuc seqin->Data %x\n", seqin->Data);

    if(!seqin->Data)
    {					/* start of file */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	seqin->Records++;

	ajDebug("first line:\n'%S'\n", seqReadLine);

	if(!ajStrPrefixC(seqReadLine, "HEADER    "))
	{
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	ajStrAssignSubS(&name,seqReadLine, 62, 71);
	ajStrTrimWhite(&name);

	ajDebug("first line OK name '%S'\n", name);

	seqin->Data = AJNEW0(alndata);
	alndata->Table = alntable = ajTablestrNew();
	alnlist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);

	while(ok && !ajStrMatchC(seqReadLine, "END"))
	{
	    seqin->Records++;

	    if(ajStrPrefixC(seqReadLine, "MODEL"))
	    {
                ajStrAssignSubS(&model, seqReadLine, 7, 14);
                ajStrTrimWhite(&model);
            }
	    else if(ajStrPrefixC(seqReadLine, "ATOM"))
	    {
		if(!alnitem)
		    AJNEW0(alnitem);

		ajStrKeepRange(&seqReadLine, 0,71);

		ajStrAssignSubS(&aa3, seqReadLine, 18, 19);
		ajStrAssignSubS(&chain, seqReadLine, 21, 21);
		ajStrAssignSubS(&token, seqReadLine, 22, 25);
		ajStrToUint(token, &iaa);

                if(iaa > lastaa)
		{
		    if(ajBaseFromDoublet(aa3,&aa))
			seqAppendK(&alnitem->Seq, aa);

		    lastaa = iaa;
		}

	    }

	    else if(ajStrPrefixC(seqReadLine, "TER"))
	    {
                if(!ajStrGetLen(alnitem->Seq))
                {
                    ajDebug("TER seqlen zero\n"); 
                    ajTableRemoveKey(alntable, alnitem->Name,
                                     (void**) &alnname);
                    ajStrDel(&alnname);
                    seqMsfItemDel(&alnitem);
                }
                else 
                {
                    nseq++;
                    ajFmtPrintS(&token, "%S-%S", name, chain);

                    if(ajStrGetLen(model))
                        ajStrAppendS(&token, model);

                    seqitemSetName(alnitem, token);
                    ajStrAssignS(&alnname, alnitem->Name);
                    alnitem->Weight = 1.0;
                    ajTablePut(alntable, alnname, alnitem);
                    alnname = NULL;
                    ajListstrPushAppend(alnlist, ajStrNewS(alnitem->Name));
                    alnitem = NULL;
                 }
                lastaa = 0;
	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

        ajStrDel(&token);
        ajStrDel(&name);
        ajStrDel(&chain);
        ajStrDel(&model);

        if(alnitem && !ajStrGetLen(alnitem->Seq))
        {
            ajListstrPopLast(alnlist, &alnname);
            ajTableRemoveKey(alntable, alnitem->Name,
                             (void**) &alnname);
            ajStrDel(&alnname);
            seqMsfItemDel(&alnitem);
        }

        if(!nseq)
        {
            seqMsfDataDel((SeqPMsfData*) &seqin->Data);
            ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	ajDebug("PDB Entry has %d sequences\n", nseq);
	ajListstrTrace(alnlist);
	ajTableTrace(alntable);
	ajTableMap(alntable, seqMsfTabList, NULL);

	alndata->Names = AJCALLOC(nseq, sizeof(*alndata->Names));

	for(i=0; i < nseq; i++)
	{
	    ajListstrPop(alnlist, &alndata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, alndata->Names[i]);
	}

	ajListstrFreeData(&alnlist);

	ajTableMap(alntable, seqMsfTabList, NULL);
	alndata->Nseq = nseq;
	alndata->Count = 0;
	alndata->Bufflines = seqin->Records;
	ajDebug("PDB format read %d lines\n", seqin->Records);
    }

    alndata = seqin->Data;
    alntable = alndata->Table;

    if(alndata->Count >= alndata->Nseq)
    {					/* all done */
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);

	return ajFalse;
    }

    i = alndata->Count;
    ajDebug("returning [%d] '%S'\n", i, alndata->Names[i]);
    alnitem = ajTableFetch(alntable, alndata->Names[i]);
    ajStrAssignS(&thys->Name, alndata->Names[i]);

    thys->Weight = alnitem->Weight;
    ajStrAssignS(&thys->Seq, alnitem->Seq);

    alndata->Count++;

    return ajTrue;
}




/* @funcstatic seqReadPdbnucseq ***********************************************
**
** Given nucleotide data in a sequence structure,
** tries to read everything needed using PDB protein databank format
** using the SEQRES records.
**
** This is the original sequence, see seqReadPdbnuc for parsing the ATOM records
** which give the sequence observed in the structure.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadPdbnucseq(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjPStrTok handle = NULL;
    AjPStr name  = NULL;
    AjPStr alnname  = NULL;
    AjPStr token = NULL;
    AjPStr chain = NULL;
    AjPTable alntable    = NULL;
    SeqPMsfItem alnitem  = NULL;
    AjPList alnlist      = NULL;
    SeqPMsfData alndata  = NULL;
    char aa;
    ajuint iseq = 0;
    ajuint nseq = 0;
    ajuint i;
    AjBool ok = ajTrue;

    buff = seqin->Filebuff;

    ajDebug("seqReadPdbnucseq seqin->Data %x\n", seqin->Data);

    if(!seqin->Data)
    {					/* start of file */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	seqin->Records++;

	ajDebug("first line:\n'%S'\n", seqReadLine);

	if(!ajStrPrefixC(seqReadLine, "HEADER    "))
	{
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	ajStrAssignSubS(&name,seqReadLine, 62, 71);
	ajStrTrimWhite(&name);

	ajDebug("first line OK name '%S'\n", name);

	seqin->Data = AJNEW0(alndata);
	alndata->Table = alntable = ajTablestrNew();
	alnlist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);

	while(ok && !ajStrMatchC(seqReadLine, "END"))
	{
	    seqin->Records++;

	    if(ajStrPrefixC(seqReadLine, "SEQRES"))
	    {
		ajStrKeepRange(&seqReadLine, 0,71);
		ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
		ajStrTokenNextParse(&handle, &token);	/* 'SEQRES' */

		ajStrTokenNextParse(&handle, &token);	/* number */
		ajStrToUint(token, &iseq);

		ajStrTokenNextParse(&handle, &chain);	/* chain letter */

		if(iseq == 1)
		{
                    if(alnitem && !ajStrGetLen(alnitem->Seq))
                    {
                        nseq--;
                        ajListstrPopLast(alnlist,&alnname);
                        ajTableRemoveKey(alntable, alnitem->Name,
                                         (void**) &alnname);
                        ajStrDel(&alnname);
                        seqMsfItemDel(&alnitem);
                    }
                    
		    nseq++;
		    ajFmtPrintS(&token, "%S-%S", name, chain);
                    AJNEW0(alnitem);
		    seqitemSetName(alnitem, token);
		    ajStrAssignS(&alnname, alnitem->Name);
		    alnitem->Weight = 1.0;
		    ajTablePut(alntable, alnname, alnitem);
		    alnname = NULL;
		    ajListstrPushAppend(alnlist, ajStrNewS(alnitem->Name));
		}

		while(ajStrTokenNextParse(&handle, &token))
		    if(ajBaseFromDoublet(token,&aa))
			seqAppendK(&alnitem->Seq, aa);
	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

        if(alnitem && !ajStrGetLen(alnitem->Seq))
        {
            nseq--;
            ajListstrPopLast(alnlist,&alnname);
            ajTableRemoveKey(alntable, alnitem->Name,
                             (void**) &alnname);
            ajStrDel(&alnname);
            seqMsfItemDel(&alnitem);
        }

	if(!nseq)
	{
            ajStrDel(&token);
            ajStrDel(&name);
            ajStrDel(&chain);
            seqMsfDataDel((SeqPMsfData*) &seqin->Data);
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

        
	ajDebug("PDB Entry has %d sequences\n", nseq);
	ajListstrTrace(alnlist);
	ajTableTrace(alntable);
	ajTableMap(alntable, seqMsfTabList, NULL);

	alndata->Names = AJCALLOC(nseq, sizeof(*alndata->Names));

	for(i=0; i < nseq; i++)
	{
	    ajListstrPop(alnlist, &alndata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, alndata->Names[i]);
	}

	ajListstrFreeData(&alnlist);

	ajTableMap(alntable, seqMsfTabList, NULL);
	alndata->Nseq = nseq;
	alndata->Count = 0;
	alndata->Bufflines = seqin->Records;
	ajDebug("PDBNUCSEQ format read %d lines\n", seqin->Records);
    }

    alndata = seqin->Data;
    alntable = alndata->Table;

    if(alndata->Count >= alndata->Nseq)
    {					/* all done */
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);

	return ajFalse;
    }

    i = alndata->Count;
    ajDebug("returning [%d] '%S'\n", i, alndata->Names[i]);
    alnitem = ajTableFetch(alntable, alndata->Names[i]);
    ajStrAssignS(&thys->Name, alndata->Names[i]);

    thys->Weight = alnitem->Weight;
    ajStrAssignS(&thys->Seq, alnitem->Seq);

    alndata->Count++;

    ajStrDel(&token);
    ajStrDel(&name);
    ajStrDel(&chain);

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
    AjPStr seqstr        = NULL;
    AjPStr name          = NULL;
    AjBool ok            = ajFalse;
    ajuint iseq          = 0;
    AjPFilebuff buff     = seqin->Filebuff;
    AjPTable alntable    = NULL;
    SeqPMsfItem alnitem  = NULL;
    AjPList alnlist      = NULL;
    SeqPMsfData alndata  = NULL;

    ajuint i;

    ajDebug("seqReadClustal seqin->Data %x\n", seqin->Data);

    if(!seqin->Data)
    {					/* start of file */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	seqin->Records++;

	if(!ok)
	    return ajFalse;

	ajDebug("first line:\n'%S'\n", seqReadLine);

	if(!ajStrPrefixC(seqReadLine, "CLUSTAL"))
	{
	    /* first line test */
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	ajDebug("first line OK: '%S'\n", seqReadLine);

	while(ok)
	{				/* skip blank lines */
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    seqin->Records++;

	    if(!ajStrIsWhite(seqReadLine))
		break;
	}

	if(!ok)
	{
	    ajDebug("FAIL (blank lines only)\n");
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	seqin->Data = AJNEW0(alndata);
	alndata->Table = alntable = ajTablestrNew();
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
	    ajListstrPushAppend(alnlist, ajStrNewS(alnitem->Name));

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    seqin->Records++;
	}

	ajStrDel(&seqstr);

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

	ajListstrFreeData(&alnlist);

	while(ajBuffreadLineStore(buff, &seqReadLine,
				 seqin->Text, &thys->TextPtr))
	{				/* now read the rest */
	    seqin->Records++;
	    seqClustalReadseq(seqReadLine, alntable);
	}

	ajTableMap(alntable, seqMsfTabList, NULL);
	alndata->Nseq = iseq;
	alndata->Count = 0;
	alndata->Bufflines = seqin->Records;
	ajDebug("ALN format read %d lines\n", seqin->Records);
    }

    alndata = seqin->Data;
    alntable = alndata->Table;

    if(alndata->Count >= alndata->Nseq)
    {					/* all done */
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);

	return ajFalse;
    }

    i = alndata->Count;
    ajDebug("returning [%d] '%S'\n", i, alndata->Names[i]);
    alnitem = ajTableFetch(alntable, alndata->Names[i]);
    ajStrAssignS(&thys->Name, alndata->Names[i]);

    thys->Weight = alnitem->Weight;
    ajStrAssignS(&thys->Seq, alnitem->Seq);

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
    AjPStr token     = NULL;
    AjPStr seqstr    = NULL;

    if(!ajStrExtractFirst(rdline, &seqstr, &token))
	return ajFalse;

    msfitem = ajTableFetch(msftable, token);
    ajStrDel(&token);

    if(!msfitem)
    {
	ajStrDel(&seqstr);

	return ajFalse;
    }

    seqAppend(&msfitem->Seq, seqstr);
    ajStrDel(&seqstr);

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
    AjPStr seqstr = NULL;
    AjPStr tmpstr = NULL;
    AjBool ok       = ajFalse;
    ajuint iseq      = 0;
    ajuint jseq      = 0;
    ajuint len       = 0;
    ajuint ilen      = 0;
    AjPFilebuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    SeqPMsfData phydata      = NULL;
    ajuint i;
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
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;

	seqin->Records++;

	ajDebug("first line:\n'%-20.20S'\n", seqReadLine);

	if(!ajRegExec(seqRegPhylipTop, seqReadLine))
	{				/* first line test */
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	ajRegSubI(seqRegPhylipTop, 1, &tmpstr);
	ajStrToUint(tmpstr, &iseq);
	ajDebug("seqRegPhylipTop1 '%S' %d\n", tmpstr, iseq);
	ajRegSubI(seqRegPhylipTop, 2, &tmpstr);
	ajStrToUint(tmpstr, &len);
	ajDebug("seqRegPhylipTop2 '%S' %d\n", tmpstr,len);
	ajDebug("first line OK: '%S' iseq: %d len: %d\n",
		seqReadLine, iseq, len);
	ajStrDel(&tmpstr);

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTablestrNew();
	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	seqin->Records++;
	ilen = 0;

	while(ok && (jseq < iseq))
	{
	    /* first set - create table */
	    if(!ajRegExec(seqRegPhylipHead, seqReadLine))
	    {
		ajDebug("FAIL (not seqRegPhylipHead): '%S'\n", seqReadLine);
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
		seqMsfDataDel((SeqPMsfData*) &seqin->Data);

		return ajFalse;
	    }

	    ajDebug("line: '%S'\n", seqReadLine);
	    ajRegSubI(seqRegPhylipHead, 1, &tmpstr);

	    if(!ajStrIsWhite(tmpstr))
            {
		/* check previous sequence */
		if(jseq)
		{
		    if(ilen != len)
		    {
			ajDebug("phylipnon format length mismatch at %d "
				"(length %d)\n",
				len, ilen);
			seqMsfDataDel((SeqPMsfData*) &seqin->Data);
			ajStrDel(&tmpstr);
			return ajFalse;
		    }
		}

		/* new sequence */
		AJNEW0(phyitem);
		seqitemSetName(phyitem, tmpstr);
		ajStrAssignS(&phydata->Names[jseq], phyitem->Name);
		ajDebug("name: '%S' => '%S'\n", tmpstr, phyitem->Name);
		phyitem->Weight = 1.0;
		ajRegPost(seqRegPhylipHead, &seqstr);
		seqAppend(&phyitem->Seq, seqstr);
		ajStrDel(&seqstr);
		ilen = ajStrGetLen(phyitem->Seq);

		if(ilen == len)
		    done = ajTrue;
		else if(ilen > len)
		{
		    ajDebug("Phylipnon format: sequence %S "
			    "header size %d exceeded\n",
			    phyitem->Name, len);
		    seqMsfDataDel((SeqPMsfData*) &seqin->Data);
		    seqMsfItemDel(&phyitem);
		    ajStrDel(&tmpstr);

		    return ajFalse;
		}

		ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
		ajDebug("seq %d: (%d) '%-20.20S'\n", jseq, ilen, seqReadLine);
	    }
	    else
            {
		/* more sequence to append */
		if(seqPhylipReadseq(seqReadLine, phytable, phyitem->Name,
				    len, &ilen, &done))
		{
		    ajDebug("read to len %d\n", ilen);

		    if (done)
			jseq++;
		}

	    }
	    ajStrDel(&tmpstr);

	    if(jseq < iseq)
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		seqin->Records++;
	    }
	}

	if(ilen != len)
	{
	    ajDebug("phylipnon format final length mismatch at %d "
		    "(length %d)\n",
		    len, ilen);
	    seqMsfDataDel((SeqPMsfData*) &seqin->Data);

	    return ajFalse;
	}

	ajDebug("Header has %d sequences\n", jseq);
	ajTableTrace(phytable);
	ajTableMap(phytable, seqMsfTabList, NULL);

	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = seqin->Records;
	ajDebug("PHYLIP format read %d lines\n", seqin->Records);
    }

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableFetch(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);
    ajStrDel(&phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);
    ajStrDel(&phyitem->Seq);

    phydata->Count++;

    if(phydata->Count >= phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajFilebuffClear(seqin->Filebuff, 0);
	ajDebug("seqReadPhylip multidone\n");
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);
    }

    ajStrDel(&seqstr);
    ajStrDel(&tmpstr);
    seqMsfDataTrace(seqin->Data);

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
    AjPStr seqstr = NULL;
    AjPStr tmpstr = NULL;
    AjBool ok       = ajFalse;
    ajuint iseq      = 0;
    ajuint jseq      = 0;
    ajuint len       = 0;
    ajuint ilen      = 0;
    ajuint maxlen    = 0;
    AjPFilebuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    AjPList phylist          = NULL;
    SeqPMsfData phydata      = NULL;
    ajuint i;
    AjBool done = ajFalse;

    ajDebug("seqReadPhylip seqin->Data %x\n", seqin->Data);

    buff = seqin->Filebuff;
    ajFilebuffSetBuffered(buff);    /* must buffer to test non-interleaved */

    if(!seqRegPhylipTop)
	seqRegPhylipTop = ajRegCompC("^ *([0-9]+) +([0-9]+)");

    if(!seqRegPhylipHead)
	seqRegPhylipHead = ajRegCompC("^(..........) ?"); /* 10 chars */

    if(!seqRegPhylipSeq)
	seqRegPhylipSeq = ajRegCompC("^[ \t\n\r]*$");

    if(!seqin->Data)
    {					/* start of file */
	seqin->multidone = ajFalse;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	while (ok && ajStrIsWhite(seqReadLine))
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);

	if(!ok)
	    return ajFalse;

	seqin->Records++;

	/* ajDebug("first line:\n'%-20.20S'\n", seqReadLine);*/

	if(!ajRegExec(seqRegPhylipTop, seqReadLine))
	{				/* first line test */
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	ajRegSubI(seqRegPhylipTop, 1, &tmpstr);
	ajStrToUint(tmpstr, &iseq);
	ajRegSubI(seqRegPhylipTop, 2, &tmpstr);
	ajStrToUint(tmpstr, &len);
	ajStrDel(&tmpstr);
	/*ajDebug("first line OK: '%S' iseq: %d len: %d\n",
		seqReadLine, iseq, len);*/

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTablestrNew();
	phylist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	seqin->Records++;
	ilen = 0;

	while(ok && (jseq < iseq))
	{
	    /* first set - create table */
	    if(!ajRegExec(seqRegPhylipHead, seqReadLine))
	    {
		ajDebug("FAIL (not seqRegPhylipHead): '%S'\n", seqReadLine);
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
		seqMsfDataDel((SeqPMsfData*) &seqin->Data);

		return ajFalse;
	    }

	    /* ajDebug("line: '%S'\n", seqReadLine); */
	    AJNEW0(phyitem);
	    ajRegSubI(seqRegPhylipHead, 1, &tmpstr);
	    seqitemSetName(phyitem, tmpstr);
	    ajStrDel(&tmpstr);
	    /* ajDebug("name: '%S' => '%S'\n", tmpstr, phyitem->Name); */
	    phyitem->Weight = 1.0;
	    ajRegPost(seqRegPhylipHead, &seqstr);
	    seqAppend(&phyitem->Seq, seqstr);
	    ajStrDel(&seqstr);
	    ilen = ajStrGetLen(phyitem->Seq);

	    if(ilen == len)
		done = ajTrue;
	    else if(ilen > len)
	    {
		ajDebug("Phylip format: sequence %S header size %d exceeded\n",
			phyitem->Name, len);
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

		seqMsfDataDel((SeqPMsfData*) &seqin->Data);
		seqMsfItemDel(&phyitem);

		ajListstrFreeData(&phylist);

		return ajFalse;
	    }

	    if(ajStrIsWhite(phyitem->Name) ||
	       ajTableFetch(phytable, phyitem->Name))
	    {
                ajFilebuffSetBuffered(buff);
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
		ajDebug("phytable repeated name '%S'\n",
			phyitem->Name);

		seqMsfDataDel((SeqPMsfData*) &seqin->Data);
		seqMsfItemDel(&phyitem);

		ajListstrFreeData(&phylist);

		return seqReadPhylipnon(thys, seqin);
	    }

	    ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
	    ajListstrPushAppend(phylist, ajStrNewS(phyitem->Name));
	    ajDebug("added '%S' list:%u table:%u\n",
		    phyitem->Name, ajListGetLength(phylist),
		    ajTableGetLength(phytable));

	    if(!jseq)
		maxlen = ilen;
	    else
	    {
		if(ilen != maxlen)
		{
		    ajDebug("phylip format length mismatch in header "
			    "iseq: %d jseq: %d ilen: %d maxlen: %d\n",
			    iseq, jseq, ilen, maxlen);
		    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
		    ajDebug("phytable deleted size:%u\n",
			    ajTableGetLength(phytable));
		    seqMsfDataDel((SeqPMsfData*) &seqin->Data);
		    ajListstrFreeData(&phylist);

		    if(seqReadPhylipnon(thys, seqin))
			return ajTrue;
		    else
                    {
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
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		seqin->Records++;
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

	ajListstrFreeData(&phylist);

	if(ilen < len)
	{
	    jseq=0;

	    while(ajBuffreadLineStore(buff, &seqReadLine,
				     seqin->Text, &thys->TextPtr))
	    {				/* now read the rest */
		/* ajDebug("seqReadPhylip line '%S\n", seqReadLine); */
		seqin->Records++;

		if(seqPhylipReadseq(seqReadLine, phytable,
				    phydata->Names[jseq],
				    len, &ilen, &done))
		{
		    if(!jseq)
			maxlen = ilen;
		    else
		    {
			if(ilen != maxlen)
			{
			    ajDebug("phylip format length mismatch at %d "
				    "(length %d)\n",
				    maxlen, ilen);
                            ajFilebuffSetBuffered(buff);
			    ajFilebuffResetStore(buff,
						 seqin->Text, &thys->TextPtr);
			    seqMsfDataDel((SeqPMsfData*) &seqin->Data);
			    ajDebug("File reset, try seqReadPhylipnon\n");

			    return seqReadPhylipnon(thys, seqin);
			}
		    }

		    jseq++;

		    if(jseq == iseq)
                        jseq = 0;

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
		ajDebug("seqReadPhylip read failed, try seqReadPhylipnon\n");
                ajFilebuffSetBuffered(buff);
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
		seqMsfDataDel((SeqPMsfData*) &seqin->Data);

		return seqReadPhylipnon(thys, seqin);
	    }

	    if(jseq)
	    {
		ajDebug("Phylip format %d sequences partly read at end\n",
			iseq-jseq);
                ajFilebuffSetBuffered(buff);
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
		seqMsfDataDel((SeqPMsfData*) &seqin->Data);

		return seqReadPhylipnon(thys, seqin);
	    }
	}

	ajTableMap(phytable, seqMsfTabList, NULL);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = seqin->Records;
	/* ajDebug("PHYLIP format read %d lines\n", seqin->Records);*/
    }

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    /* ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]); */
    phyitem = ajTableFetch(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);

    phydata->Count++;

    if(phydata->Count >= phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadPhylip multidone\n");
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);
    }

    seqMsfDataTrace(seqin->Data);

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
** @param [r] len [ajuint] Final length of each sequence (from file header)
** @param [w] ilen [ajuint*] Length of each sequence so far
** @param [w] done [AjBool*] ajTrue if sequence was completed
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqPhylipReadseq(const AjPStr rdline, const AjPTable phytable,
			       const AjPStr token,
			       ajuint len, ajuint* ilen, AjBool* done)
{
    SeqPMsfItem phyitem;

    *done = ajFalse;

    if(!seqRegPhylipSeq2)
	seqRegPhylipSeq2 = ajRegCompC("[^ \t\n\r]");

    if(!ajRegExec(seqRegPhylipSeq2, rdline))
	return ajFalse;

    phyitem = ajTableFetch(phytable, token);

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
    AjPStr seqstr = NULL;
    AjPStr tmpstr = NULL;
    AjBool ok       = ajFalse;
    ajuint iseq      = 0;
    ajuint len       = 0;
    AjPFilebuff buff;
    AjPTable fmttable   = NULL;
    SeqPMsfItem fmtitem = NULL;
    AjPList fmtlist     = NULL;
    SeqPMsfData fmtdata = NULL;
    char *cp;

    ajuint i;
    ajuint jseq = 0;

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
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;

	seqin->Records++;

	ajDebug("first line:\n'%S'\n", seqReadLine);

	if(!ajStrPrefixC(seqReadLine, "xread"))
	{
	    /* first line test */
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	ajDebug("first line OK: '%S'\n", seqReadLine);

	/* skip title line */
	for(i=0; i<2; i++)
	{
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    seqin->Records++;

	    if(!ok)
	    {
		ajDebug("FAIL (bad header)\n");
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

		return ajFalse;
	    }
	}

	if(!ajRegExec(seqRegHennigTop, seqReadLine))	/* first line test */
	    return ajFalse;

	ajRegSubI(seqRegHennigTop, 1, &tmpstr);
	ajStrToUint(tmpstr, &iseq);
	ajRegSubI(seqRegHennigTop, 2, &tmpstr);
	ajStrToUint(tmpstr, &len);
	ajDebug("first line OK: '%S' iseq: %d len: %d\n",
		seqReadLine, iseq, len);
	ajStrDel(&tmpstr);

	seqin->Data = AJNEW0(fmtdata);
	fmtdata->Table = fmttable = ajTablestrNew();
	fmtlist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	seqin->Records++;

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
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    seqin->Records++;

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

	    ajStrDel(&seqstr);

	    ajTablePut(fmttable, ajStrNewS(fmtitem->Name), fmtitem);
	    ajListstrPushAppend(fmtlist, ajStrNewS(fmtitem->Name));
	    jseq++;
	    ajDebug("first set %d: '%S'\n", jseq, seqReadLine);

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    seqin->Records++;
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

	ajListstrFreeData(&fmtlist);

	while(ajBuffreadLineStore(buff, &seqReadLine,
				 seqin->Text, &thys->TextPtr))
	{				/* now read the rest */
	    seqin->Records++;
	    seqHennig86Readseq(seqReadLine, fmttable);
	}

	ajTableMap(fmttable, seqMsfTabList, NULL);
	fmtdata->Nseq = iseq;
	fmtdata->Count = 0;
	fmtdata->Bufflines = seqin->Records;
	ajDebug("... format read %d lines\n", seqin->Records);
    }

    /* processing entries */

    fmtdata = seqin->Data;
    fmttable = fmtdata->Table;

    if(fmtdata->Count >=fmtdata->Nseq)
    {					/* all done */
	ajFilebuffClear(seqin->Filebuff, 0);
	ajTableMapDel(fmttable, seqMsfTabDel, NULL);
	ajTableFree(&fmttable);
	AJFREE(fmtdata->Names);
	AJFREE(fmtdata);
	seqin->Data = NULL;

	return ajFalse;
    }

    i = fmtdata->Count;
    ajDebug("returning [%d] '%S'\n", i, fmtdata->Names[i]);
    fmtitem = ajTableFetch(fmttable, fmtdata->Names[i]);
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
    SeqPMsfItem msfitem;
    AjPStr token  = NULL;
    AjPStr seqstr = NULL;

    if(!seqRegHennigSeq)
	seqRegHennigSeq = ajRegCompC("^[^ \t\n\r]+"); /* must be line start */

    if(!ajRegExec(seqRegHennigSeq, rdline))
	return ajFalse;

    ajRegSubI(seqRegHennigSeq, 0, &token);
    msfitem = ajTableFetch(msftable, token);
    ajStrDel(&token);

    if(!msfitem)
	return ajFalse;

    ajRegPost(seqRegHennigSeq, &seqstr);
    seqAppend(&msfitem->Seq, seqstr);

    ajStrDel(&seqstr);

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
    AjPStr tmpstr = NULL;
    AjBool ok       = ajFalse;
    ajint len       = 0;
    ajint ilen      = 0;
    ajuint iseq;
    ajuint i;
    AjPFilebuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    AjPList phylist          = NULL;
    SeqPMsfData phydata      = NULL;

    buff = seqin->Filebuff;

    if(!seqRegTreeconTop)
	seqRegTreeconTop = ajRegCompC("^ *([0-9]+)");

    if(!seqin->Data)			/* first time - read the data */
    {
	iseq = 0;
	seqin->multidone = ajFalse;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;

	seqin->Records++;

	if(!ajRegExec(seqRegTreeconTop, seqReadLine))
	{				/* first line test */
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	ajRegSubI(seqRegTreeconTop, 1, &tmpstr);
	ajStrToInt(tmpstr, &len);
	ajStrDel(&tmpstr);
	ajDebug("first line OK: '%S' len: %d\n",
		seqReadLine, len);

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTablestrNew();
	phylist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	seqin->Records++;
	ilen = UINT_MAX;

	while (ok)
	{
	   if (ilen < 0)
	   {
	       ajStrRemoveWhiteExcess(&seqReadLine);

	       if (!ajStrGetLen(seqReadLine))	/* empty line after sequence */
	       {
		   ok = ajBuffreadLineStore(buff, &seqReadLine,
					   seqin->Text, &thys->TextPtr);
		   continue;
	       }

	       AJNEW0(phyitem);
	       phyitem->Weight = 1.0;
	       seqitemSetName(phyitem, seqReadLine);
	       ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
	       ajListstrPushAppend(phylist, ajStrNewS(phyitem->Name));
	       iseq++;
	       ilen = 0;
	   }
	   else
	   {
	       ajStrRemoveWhite(&seqReadLine);
	       ilen += ajStrGetLen(seqReadLine);
	       seqAppend(&phyitem->Seq, seqReadLine);
	       
	       if (ilen > len)
	       {
		   ajDebug("Treecon format: '%S' too long, read %d/%d\n",
		    phyitem->Name, ilen, len);
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
		seqMsfDataDel((SeqPMsfData*)&seqin->Data);

		return ajFalse;
	       }

	       if (ilen == len)
		   ilen = -1;
	   }

	   ok = ajBuffreadLineStore(buff, &seqReadLine,
				   seqin->Text, &thys->TextPtr);
	}

	if (ilen >= 0)
	{
	    ajDebug("Treecon format: unfinished sequence '%S' read %d/%d\n",
		    phyitem->Name, ilen, len);
	    seqMsfDataDel((SeqPMsfData*)&seqin->Data);

	    return ajFalse;
	}

	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));

	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(phylist, &phydata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, phydata->Names[i]);
	}

	ajListstrFreeData(&phylist);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = seqin->Records;
	ajDebug("Treecon format read %d lines\n", seqin->Records);


    }

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableFetch(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);

    phydata->Count++;

    if(phydata->Count >=phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadTreecon multidone\n");
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*)&seqin->Data);
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
    AjPStr tmpstr = NULL;
    AjPStr tmpname = NULL;
    AjBool ok       = ajFalse;
    ajuint iseq;
    ajuint i;
    AjPFilebuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    AjPList phylist          = NULL;
    SeqPMsfData phydata      = NULL;

    buff = seqin->Filebuff;

    if(!seqRegJackTop)
	seqRegJackTop = ajRegCompC("^'(.*)'\\s*$");

    if(!seqRegJackSeq)
	seqRegJackSeq = ajRegCompC("^[(]([^)]+)(.*)$");

    if(!seqin->Data)			/* first time - read the data */
    {
	iseq = 0;
	seqin->multidone = ajFalse;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;

	seqin->Records++;

	if(!ajRegExec(seqRegJackTop, seqReadLine))
	{				/* first line test */
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

	ajDebug("JackKnifer format: First line ok '%S'\n", seqReadLine);

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTablestrNew();
	phylist = ajListstrNew();
	seqin->Filecount = 0;

	while (ok)
	{
	    if (!ajStrGetLen(seqReadLine))	/* empty line after sequence */
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		continue;
	    }

	    if (ajStrPrefixC(seqReadLine, ";"))
		break;			/* done */

	    if (ajStrPrefixC(seqReadLine, "("))
	    {
		if (!ajRegExec(seqRegJackSeq, seqReadLine))
		{
		    ajDebug("JackKnifer format: bad (id) line\n");
		    seqMsfDataDel((SeqPMsfData*)&seqin->Data);

		    return ajFalse;
		}

		ajRegSubI(seqRegJackSeq, 1, &tmpstr);
		seqnameSetName(&tmpname, tmpstr);
		phyitem = ajTableFetch(phytable, tmpname);

		if (!phyitem)
		{
		    ajDebug("JackKnifer format: new (id) '%S'\n", tmpname);
		    AJNEW0(phyitem);
		    phyitem->Weight = 1.0;
		    ajStrAssignS(&phyitem->Name,tmpname);
		    ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
		    ajListstrPushAppend(phylist, ajStrNewS(phyitem->Name));
		    iseq++;
		}
		else
		    ajDebug("JackKnifer format: More for (id) '%S'\n",
			    tmpname);

		ajRegSubI(seqRegJackSeq, 2, &tmpstr);
		ajStrAssignS(&seqReadLine, tmpstr);
	    }

	    seqAppend(&phyitem->Seq, seqReadLine);

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				   seqin->Text, &thys->TextPtr);
	}

	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));

	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(phylist, &phydata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, phydata->Names[i]);
	}

	ajListstrFreeData(&phylist);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = seqin->Records;
	ajDebug("JackKnifer format read %d lines\n", seqin->Records);
    }

    ajStrDel(&tmpstr);
    ajStrDel(&tmpname);

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableFetch(phytable, phydata->Names[i]);
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
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*)&seqin->Data);
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
    AjBool ok       = ajFalse;
    ajuint i;
    ajuint j;
    AjPFilebuff buff;
    AjPStr* seqs = NULL;
    AjPNexus nexus = NULL;

    SeqPMsfData phydata      = NULL;

    buff = seqin->Filebuff;

    if(!seqin->Data)			/* first time - read the data */
    {
	seqin->multidone = ajFalse;

	ajFilebuffSetBuffered(buff);

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	ajDebug("Nexus format: Testing first line '%S'\n", seqReadLine);

	if(!ok)
	    return ajFalse;

	seqin->Records++;

	if(!ajStrPrefixCaseC(seqReadLine, "#NEXUS"))
	{				/* first line test */
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	    return ajFalse;
	}

	ajDebug("Nexus format: First line ok '%S'\n", seqReadLine);

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);

	while(ok && !ajStrPrefixCaseC(seqReadLine, "#NEXUS"))
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);

	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	AJNEW0(phydata);
	phydata->Nexus = ajNexusParse(buff);

	if (!phydata->Nexus)
	{
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	    ajDebug("Failed to parse in nexus format\n");

	    return ajFalse;
	}

	phydata->Count = 0;
	phydata->Nseq = ajNexusGetNtaxa(phydata->Nexus);
	/* GetTaxa may fail if names are only defined in the sequences */
	seqs = ajNexusGetTaxa(phydata->Nexus);
	phydata->Names = AJCALLOC(phydata->Nseq, sizeof(*phydata->Names));

	for(j=0;j<phydata->Nseq;j++)
	    ajStrAssignS(&phydata->Names[j], seqs[j]);

	seqin->Data = phydata;
	ajDebug("Nexus parsed %d sequences\n", phydata->Nseq);
    }

    phydata = seqin->Data;
    nexus = phydata->Nexus;

    i = phydata->Count;

    seqs = ajNexusGetSequences(nexus);

    if (!seqs)
    {
	seqMsfDataDel((SeqPMsfData*)&seqin->Data);

	return ajFalse;
    }

    if (!phydata->Names)		/* finally set from the sequences */
    {
	seqs = ajNexusGetTaxa(phydata->Nexus);
	phydata->Names = AJCALLOC(phydata->Nseq, sizeof(*phydata->Names));

	for(j=0;j<phydata->Nseq;j++)
	    ajStrAssignS(&phydata->Names[j], seqs[j]);
    }

    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);

    ajStrAssignS(&thys->Name, phydata->Names[i]);

    thys->Weight = 1.0;
    ajStrAssignS(&thys->Seq, seqs[i]);

    phydata->Count++;

    if(phydata->Count >= phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadNexus multidone\n");
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*)&seqin->Data);
    }

    return ajTrue;
}




/* @funcstatic seqReadMega ****************************************************
**
** Tries to read input in Mega interleaved or non-interleaved format.
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
    AjPStr tmpstr = NULL;
    AjPStr tmpdesc = NULL;
    AjPStr tmpname = NULL;
    AjPStr prestr = NULL;
    AjPStr poststr = NULL;
    AjBool ok       = ajFalse;
    ajuint iseq = 0;
    ajuint i;
    AjPFilebuff buff;
    AjBool istitle = ajFalse;
    AjBool isformat = ajFalse;
    AjBool iscommand = ajFalse;
    AjBool resume = ajFalse;
    AjPStr genestr = NULL;
    AjPStr domainstr = NULL;
    AjPStr nextgenestr = NULL;
    AjPStr nextdomainstr = NULL;

    ajint ipos;
    ajint istart;
    ajint ilast;
    char ichar;
    
    AjPStr formatType = NULL;
    AjPStr formatValue = NULL;

    char identchar = '.';
    char indelchar = '-';
    char misschar = '?';
    char seqtype = ' ';

    char* cp;
    const char *cq;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    SeqPMsfItem firstitem    = NULL;
    AjPList phylist          = NULL;
    SeqPMsfData phydata      = NULL;

    AjPSeqGene seqgene = NULL;

    buff = seqin->Filebuff;

    if(!seqRegMegaCommand)
	seqRegMegaCommand = ajRegCompC("([^ =!]+)=([^ ;]+)");

    if(!seqRegMegaFeat)
	seqRegMegaFeat = ajRegCompC("^(.*)\"[^\"]*\"(.*)$");

    if(!seqRegMegaSeq)
	seqRegMegaSeq = ajRegCompC("^#([^ \t\n\r]+)(.*)$");

    if(seqin->Data)
    {
        phydata = seqin->Data;
        if(seqin->multidone)
            resume = phydata->Resume;
    }

    if(!seqin->Data ||          /* first time - read the data */
       (seqin->multidone && resume)) /* resuming gene/domain block */
    {
	iseq = 0;
	seqin->multidone = ajFalse;

        if(!seqin->Data)
        {
            ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
            ajDebug("Mega format: Testing first line '%S'\n", seqReadLine);

            if(!ok)
                return ajFalse;

            seqin->Records++;

            if(!ajStrPrefixCaseC(seqReadLine, "#MEGA"))
            {				/* first line test */
                ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

                return ajFalse;
            }

            ajDebug("Mega format: First line ok '%S'\n", seqReadLine);

            ok = ajBuffreadLineStore(buff, &seqReadLine,
                                     seqin->Text, &thys->TextPtr);
            if(!ok)
                return ajFalse;

            seqin->Records++;

            if(!ajStrPrefixCaseC(seqReadLine, "TITLE") &&
               !ajStrPrefixCaseC(seqReadLine, "!TITLE"))
            {				/* first line test */
                ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

                return ajFalse;
            }
            ajStrAssignSubS(&tmpdesc, seqReadLine, 6, -1);
            ajStrTrimStartC(&tmpdesc, ": \t");
            ajStrTrimEndC(&tmpdesc, "; \t\n\r");

            if(ajStrGetCharFirst(seqReadLine) == '!')
            {
                istitle = ajTrue;
                if(ajStrFindAnyK(seqReadLine, ';') != -1)
                    istitle = ajFalse;
            }
        
            ajDebug("Mega format: Second line ok '%S'\n", seqReadLine);

            isformat = ajFalse;

            while(ok && !ajStrPrefixC(seqReadLine, "#"))

            {				/* skip comments in header */
                if(iscommand)
                {
                    if(ajStrFindAnyK(seqReadLine, ';') != -1)
                        iscommand = ajFalse;
                }

                else if(istitle)
                {
                    ajStrAssignS(&tmpstr, seqReadLine);
                    ajStrTrimStartC(&tmpstr, ": \t");
                    ajStrTrimEndC(&tmpstr, "; \t\n\r");
                    ajStrAppendK(&tmpdesc, ' ');
                    ajStrAppendS(&tmpdesc, tmpstr);
                    if(ajStrFindAnyK(seqReadLine, ';') != -1)
                        istitle = ajFalse;
                }

                else
                {
                    if(ajStrPrefixCaseC(seqReadLine, "!FORMAT"))
                        isformat = ajTrue;

                    if(isformat)
                    {
                        ajDebug("Format line: %S", seqReadLine);
                        ajStrAssignS(&tmpstr, seqReadLine);

                        while(ajRegExec(seqRegMegaCommand, tmpstr))
                        {
                            ajRegSubI(seqRegMegaCommand, 1, &formatType);
                            ajRegSubI(seqRegMegaCommand, 2, &formatValue);
                            if(ajStrPrefixCaseC(formatType, "indel"))
                                indelchar = ajStrGetCharFirst(formatValue);
                            if(ajStrPrefixCaseC(formatType, "ident"))
                                identchar = ajStrGetCharFirst(formatValue);
                            if(ajStrPrefixCaseC(formatType, "match"))
                                identchar = ajStrGetCharFirst(formatValue);
                            if(ajStrPrefixCaseC(formatType, "miss"))
                                misschar = ajStrGetCharFirst(formatValue);
                            if(ajStrPrefixCaseC(formatType, "DataType"))
                                seqtype = ajStrGetCharFirst(formatValue);
                            ajRegPost(seqRegMegaCommand, &tmpstr);
                            ajDebug("'%S' = '%S' (%S) indel '%c' ident '%c' "
                                    "missing  '%c'\n",
                                    formatType, formatValue, tmpstr,
                                    indelchar, identchar, misschar);
                        }
                    
                        if(ajStrFindAnyK(seqReadLine, ';') == -1)
                            isformat = ajFalse;
                    }
            
                    else
                    {
                        if(ajStrGetCharFirst(seqReadLine) == '!')
                        {
                            ajStrAssignS(&tmpstr, seqReadLine);
                            while(ajRegExec(seqRegMegaCommand, tmpstr))
                            {
                                ajRegSubI(seqRegMegaCommand, 1, &formatType);
                                ajRegSubI(seqRegMegaCommand, 2, &formatValue);
                                if(ajStrMatchCaseC(formatType, "gene"))
                                {
                                    ajStrAssignS(&genestr, formatValue);
				    ajDebug("command: Gene='%S'\n",
					    formatValue);
                                }
                    
                                if(ajStrMatchCaseC(formatType, "domain"))
                                {
                                    ajStrAssignS(&domainstr, formatValue);
				    ajDebug("command: Domain='%S'\n",
					    formatValue);
                                }
                                ajRegPost(seqRegMegaCommand, &tmpstr);
                            }

                            if(ajStrFindAnyK(seqReadLine, ';') == -1)
                                iscommand = ajTrue;
                        }
                    }
                }

                ok = ajBuffreadLineStore(buff, &seqReadLine,
                                         seqin->Text, &thys->TextPtr);

                seqin->Records++;
            }
            ajStrDel(&tmpstr);

            if(isformat || istitle || iscommand)
            {
                ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

                return ajFalse;
            }

            /*
            ** read through looking for #id
            ** Some day we could stop at #mega and read multiple files
            */

            seqin->Data = AJNEW0(phydata);
            phydata->Table = phytable = ajTablestrNew();
            phylist = ajListstrNew();
            seqin->Filecount = 0;

            phydata->Identchar = identchar;
            phydata->Indelchar = indelchar;
            phydata->Misschar = misschar;
            phydata->Seqtype = seqtype;
        }

        /*
        ** Resume from here
        */

        if(resume)
        {
            ok = ajBuffreadLineStore(buff, &seqReadLine,
                                         seqin->Text, &thys->TextPtr);

            seqin->Records++;
            resume = ajFalse;
            phydata->Resume = ajFalse;
            ajTableMapDel(phydata->Table, seqMsfTabDel, NULL);
            phylist = ajListstrNew();
            phytable = phydata->Table;
            ajStrAssignS(&phydata->Gene, phydata->NextGene);
            ajStrAssignS(&phydata->Domain, phydata->NextDomain);
            ajStrAssignClear(&phydata->NextGene);
            ajStrAssignClear(&phydata->NextDomain);
        }

	while (ok)
	{
            ipos = ajStrFindAnyC(seqReadLine, "[]");
            istart = 0;
            ichar = ' ';
            while((ipos != -1) ||
                  (phydata->CommentDepth &&
                   (istart < (ajint) ajStrGetLen(seqReadLine))))
            {
                ilast = ipos;
                if(ipos > -1)
                    ichar = ajStrGetCharPos(seqReadLine, ipos);
                if(!phydata->CommentDepth)
                {
                    istart = ipos;
                }
                    
                if(ichar == '[')
                    phydata->CommentDepth++;
                else if((ichar == ']') && phydata->CommentDepth)
                    phydata->CommentDepth--;

                ajStrCutRange(&seqReadLine, istart, ilast);
                ipos = ajStrFindAnyC(seqReadLine, "[]");
                ichar = ' ';
            }
            
            /* empty line after a sequence */
	    if (!ajStrGetLen(seqReadLine))
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		continue;
	    }

	    if (ajStrPrefixC(seqReadLine, "!"))
            {
                iscommand = ajTrue;
            }

            if(!iscommand)
            {
                if(ajStrPrefixC(seqReadLine, "#"))
                {
                    if (!ajRegExec(seqRegMegaSeq, seqReadLine))
                    {
                        ajDebug("Mega format: bad #id line\n");
                        seqMsfDataDel((SeqPMsfData*)&seqin->Data);

                        return ajFalse;
                    }

                    ajRegSubI(seqRegMegaSeq, 1, &tmpstr);
                    seqnameSetName(&tmpname, tmpstr);
                    phyitem = ajTableFetch(phytable, tmpname);

                    if (!phyitem)
                    {
                        AJNEW0(phyitem);
                        phyitem->Weight = 1.0;
                        ajStrAssignS(&phyitem->Name,tmpname);
                        ajStrAssignS(&phyitem->Desc, tmpdesc);
                        ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
                        ajListstrPushAppend(phylist, ajStrNewS(phyitem->Name));
                        iseq++;
                    }
                    else
                        ajDebug("Mega format: More for #id '%S'\n", tmpname);

                    ajRegSubI(seqRegMegaSeq, 2, &tmpstr);
                    ajStrAssignS(&seqReadLine, tmpstr);
                }

                while (ajRegExec(seqRegMegaFeat, seqReadLine))
                {
                    ajDebug("Quotes found: '%S'\n", seqReadLine);
                    ajRegSubI(seqRegMegaFeat, 1, &prestr);
                    ajRegSubI(seqRegMegaFeat, 2, &poststr);
                    ajStrAssignS(&seqReadLine, prestr);
                    ajStrAppendS(&seqReadLine, poststr);
                    ajDebug("Quotes removed: '%S'\n", seqReadLine);
                }

                seqAppend(&phyitem->Seq, seqReadLine);
		ajDebug("Append '%S' len %u\n",
			phyitem->Name, ajStrGetLen(phyitem->Seq));
            }

            else
            {
                ajStrAssignS(&tmpstr, seqReadLine);

                while(ajRegExec(seqRegMegaCommand, tmpstr))
                {
                    ajRegSubI(seqRegMegaCommand, 1, &formatType);
                    ajRegSubI(seqRegMegaCommand, 2, &formatValue);
                    if(ajStrMatchCaseC(formatType, "gene"))
                    {
                        if(iseq)
                            resume = ajTrue;
                        ajStrAssignS(&nextgenestr, formatValue);
			ajDebug("command: Gene='%S'\n",
				formatValue);
                    }

                    if(ajStrMatchCaseC(formatType, "domain"))
                    {
                        if(iseq)
                            resume = ajTrue;
                        ajStrAssignS(&nextdomainstr, formatValue);
			ajDebug("command: Domain='%S'\n",
				formatValue);
                    }
                    ajRegPost(seqRegMegaCommand, &tmpstr);
                }
                if(ajStrFindAnyK(seqReadLine, ';') != -1)
                    iscommand = ajFalse;
            }
            
            if(resume)
                break;

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				   seqin->Text, &thys->TextPtr);
            seqin->Records++;

	}

        if(phydata->Names)
            AJCRESIZE0(phydata->Names, phydata->Nseq, iseq);
        else
            phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));

	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(phylist, &phydata->Names[i]);
	}

	ajListstrFreeData(&phylist);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = seqin->Records;
    }

    ajStrDel(&formatType);
    ajStrDel(&formatValue);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpname);
    ajStrDel(&tmpdesc);
    ajStrDel(&prestr);
    ajStrDel(&poststr);

    phydata = seqin->Data;
    phytable = phydata->Table;

    firstitem = ajTableFetch(phytable, phydata->Names[0]);
    i = phydata->Count;
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableFetch(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);
    if(i)
        ajStrDel(&phydata->Names[i]);

    if(ajStrGetLen(genestr))
        ajStrAssignS(&phydata->Gene, genestr);

    if(ajStrGetLen(domainstr))
        ajStrAssignS(&phydata->Domain, domainstr);

    if(resume)
    {
        phydata->Resume = ajTrue;
        if(ajStrGetLen(nextgenestr))
            ajStrAssignS(&phydata->NextGene, nextgenestr);
        else
            ajStrAssignClear(&phydata->NextGene);
        if(ajStrGetLen(nextdomainstr))
            ajStrAssignS(&phydata->NextDomain, nextdomainstr);
        else
            ajStrAssignClear(&phydata->NextDomain);
    }

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Desc, phyitem->Desc);
    ajStrAssignS(&thys->Seq, phyitem->Seq);
    if(i)
        ajStrDel(&phyitem->Seq);
    if(ajStrGetLen(phydata->Gene))
    {
        seqgene = ajSeqgeneNewName(phydata->Gene);
        ajSeqAddGene(thys, seqgene);
        seqgene = NULL;
    }

    if(strchr("nNrRdD", phydata->Seqtype))
        ajSeqSetNuc(thys);
    else if(strchr("pP", phydata->Seqtype))
        ajSeqSetProt(thys);

    cp = ajStrGetuniquePtr(&thys->Seq);
    cq = ajStrGetPtr(firstitem->Seq);

    while(*cp) 
    {
        if(*cp == phydata->Indelchar)
            *cp = '-';
        else if (*cp == phydata->Identchar)
            *cp = *cq;

        cp++;
        cq++;
    }
    
    phydata->Count++;

    if(phydata->Count >= phydata->Nseq)
    {
	seqin->multidone = ajTrue;

        ajStrDel(&phydata->Names[0]);
        ajStrDel(&firstitem->Seq);
        if(!phydata->Resume)
        {
            ajFilebuffClear(seqin->Filebuff, 0);
            seqMsfDataDel((SeqPMsfData*)&seqin->Data);
        }
    }

    ajStrDel(&genestr);
    ajStrDel(&nextgenestr);
    ajStrDel(&domainstr);
    ajStrDel(&nextdomainstr);
    ajStrDel(&formatType);
    ajStrDel(&formatValue);

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
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPFilebuff buff;
    AjBool ok = ajTrue;


    buff = seqin->Filebuff;

    if(!ajBuffreadLineStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    seqin->Records++;

    ajDebug("first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "ENTRY "))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    ajStrTokenNextParse(&handle, &token);	/* 'ENTRY' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName(thys, token);

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);

    while(ok && !ajStrPrefixC(seqReadLine, "SEQUENCE"))
    {
	seqin->Records++;

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
		seqin->Records++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");
		ajStrTokenNextParseC(&handle, "\n\r", &token);
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
	    }
	}

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);

    while(ok && !ajStrPrefixC(seqReadLine, "///"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	seqin->Records++;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajFilebuffClear(buff, 0);

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return ajTrue;
}




/* @funcstatic seqReadAce ***************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using ACE format as defined by the consed assembly editor.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadAce(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPFilebuff buff;
    AjBool ok = ajTrue;
    ajuint icontig;
    ajuint iseq;
    AjPTable acetable        = NULL;
    SeqPMsfItem aceitem      = NULL;
    SeqPMsfData acedata      = NULL;
    ajuint i;

    ajDebug("seqReadAcedb\n");

    buff = seqin->Filebuff;

    if(!seqin->Data)
    {					/* start of file */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
                                 seqin->Text, &thys->TextPtr);
        if(!ok)
	    return ajFalse;

        ajDebug("first line:\n'%S'\n", seqReadLine);

        ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
        ajStrTokenNextParseC(&handle, " \t", &token); /* 'AS ncontig nseq' */
        ajDebug("Token 1 '%S'\n", token);

        if(!ajStrMatchCaseC(token, "AS"))
        {
            ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
            ajStrDel(&token);
            ajStrTokenDel(&handle);
            return ajFalse;
        }

        ajStrTokenNextParseC(&handle, " \t", &token); /* number of contigs */
        ajStrToUint(token, &icontig);
        ajStrTokenNextParseC(&handle, " \t", &token); /* number of reads */
        ajStrToUint(token, &iseq);

        seqin->Data = AJNEW0(acedata);
        acedata->Table = acetable = ajTablestrNew();
        seqin->Filecount = 0;

        /*
        ** read sequence from CO (* for gap)
        ** read accuracy from BQ (no quality for gaps)
        **
        ** Read with gaps
        */
    }
    
    acedata = seqin->Data;
    acetable = acedata->Table;

    i = acedata->Count;
    /* ajDebug("returning [%d] '%S'\n", i, acedata->Names[i]); */
    aceitem = ajTableFetch(acetable, acedata->Names[i]);
    ajStrAssignS(&thys->Name, acedata->Names[i]);

    thys->Weight = aceitem->Weight;
    ajStrAssignS(&thys->Seq, aceitem->Seq);

    acedata->Count++;

    if(acedata->Count >= acedata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadAce Multidone\n");
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);
    }

    ajSeqSetNuc(thys);
    
    ajFilebuffClear(buff, 0);

    ajStrTokenDel(&handle);
    ajStrDel(&token);

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
    AjPFilebuff buff;
    AjBool ok = ajTrue;

    ajDebug("seqReadAcedb\n");

    buff = seqin->Filebuff;

    do
    {
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(ok)
            seqin->Records++;
    } while(ok &&
	    (ajStrPrefixC(seqReadLine, "//") ||
             ajStrPrefixC(seqReadLine, "\n")));

    if(!ok)
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

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
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	ajStrTokenDel(&handle);
	ajStrDel(&token);

	return ajFalse;
    }

    ajStrTokenNextParseC(&handle, " \t\"", &token); /* : */

    if(!ajStrMatchC(token, ":"))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	ajStrTokenDel(&handle);
	ajStrDel(&token);

	return ajFalse;
    }

    ajStrTokenNextParseC(&handle, "\"", &token);	/* name */

    if(!ajStrGetLen(token))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	ajStrTokenDel(&handle);
	ajStrDel(&token);

	return ajFalse;
    }

    /* we know we will succeed from here ... no way to return ajFalse */

    ajFilebuffSetUnbuffered(buff);

    seqSetName(thys, token);

    /* OK, we have the name. Now look for the sequence */

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    while(ok && !ajStrPrefixC(seqReadLine,"\n"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	seqin->Records++;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajFilebuffClear(buff, 0);

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return ajTrue;
}




/* @funcstatic seqReadBiomart *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using BioMart tab-delimited format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadBiomart(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr token     = NULL;
    AjPFilebuff buff;
    AjPStrTok handle = NULL;
    AjBool ok = ajTrue;
    ajuint ifields = 0;
    ajuint i;

    buff = seqin->Filebuff;

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
        return ajFalse;

    ajDebug("seqReadBiomart record '%S'%u\n",
            seqReadLine);

    ifields = ajStrCalcCountK(seqReadLine, '\t');
    ++ifields;
    
    ajDebug("fields: %u\n", ifields);

    if(ifields < 2) 
        return ajFalse;

    seqin->Records++;
    
    ajStrTokenAssignC(&handle, seqReadLine, "\t\n");

    ajStrTokenNextParseNoskip(&handle,&token); /* sequence */
    seqAppend(&thys->Seq, token);

    ajStrTokenNextParseNoskip(&handle,&token); /* identifier*/
    seqSetName(thys, token);

    for(i = 2; i < ifields; i++)
    {
        ajStrTokenNextParseNoskip(&handle,&token); /* non-sequence*/

        if(ajStrGetLen(token))
        {
            if(i > 2)
                ajStrAppendK(&thys->Desc, ' ');

            ajStrAppendS(&thys->Desc, token);
        }
    }

    ajFilebuffClear(buff, 0);

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
    AjPStr token     = NULL;
    AjPFilebuff buff;
    AjBool ok = ajTrue;
    ajuint ilen = 0;

   if (!seqRegFitchHead)
	seqRegFitchHead = ajRegCompC("^(\\S+),\\s+(\\d+)\\s+bases\n");

    buff = seqin->Filebuff;

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    ajDebug("seqReadFitch first line '%S'%u\n",
            seqReadLine);

    if (!ajRegExec(seqRegFitchHead, seqReadLine))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    ajRegSubI(seqRegFitchHead, 1, &token);
    seqSetName(thys, token);

    ajRegSubI(seqRegFitchHead, 2, &token);
    ajStrToUint(token, &ilen);

    ajDebug("seqReadFitch header name '%S' bases %u\n",
            thys->Name, ilen);

    /* we know we will succeed from here ... no way to return ajFalse */

    ajFilebuffSetUnbuffered(buff);

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);

    while (ok && (ajStrGetLen(thys->Seq) < ilen))
    {
	seqAppend(&thys->Seq, seqReadLine);
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
        ajDebug("seqReadFitch new length %u '%S'\n",
                ajStrGetLen(thys->Seq), seqReadLine);
    }

    ajStrDel(&token);

    if(ok)
        ajFilebuffClear(buff, 1);
    else
        ajFilebuffClear(buff, 0);

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
    AjPFilebuff buff;
    AjBool ok = ajTrue;

    if (!seqRegMaseHead)
	seqRegMaseHead = ajRegCompC("^(;+)");

    buff = seqin->Filebuff;

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ajRegExec(seqRegMaseHead, seqReadLine))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    /* we know we will succeed from here ... no way to return ajFalse */

    ajFilebuffSetUnbuffered(buff);

    while(ok && ajRegExec(seqRegMaseHead, seqReadLine))
    {
	if(ajRegLenI(seqRegMaseHead, 1) == 1)
	{
	    ajRegPost(seqRegMaseHead, &token);

	    if(des)
		ajStrAppendK(&des, ' ');

	    ajStrAppendS(&des, token);
	}

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajStrRemoveWhiteExcess(&seqReadLine);
    seqSetName(thys, seqReadLine);
    ajStrRemoveWhiteExcess(&des);
    ajSeqAssignDescS(thys, des);

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    while(ok && !ajRegExec(seqRegMaseHead, seqReadLine))
    {
	seqAppend(&thys->Seq, seqReadLine);
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajStrDel(&token);
    ajStrDel(&des);

    if(ok)
	ajFilebuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    else
	ajFilebuffClear(buff, 0);

    return ajTrue;
}




/* @funcstatic seqReadBam ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using binary alignment/map (BAM) format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/


static AjBool seqReadBam(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjPFile infile;
    ajuint i;
    ajuint ilen;
    ajuint ntargets;
    char bambuf [4] = "   ";
    char* refname;
    char** targetname;
    ajuint *targetlen;
    int status;
    ajuint namelen = 0;
    AjPSeqBam b = NULL;
    AjPSeqBamCore c;
    ajint ret = 0;
    struct bamdata
    {
        ajuint Count;
        ajuint Nref;
        AjPSeqBamBgzf gzfile;
        AjPSeqBam bam;
    } *bamdata = NULL;
    static AjBool called = ajFalse;
    static AjBool bigendian = ajFalse;
    unsigned char* d;
    ajuint dpos;
    int cigop;
    ajuint cigend;
    ajuint cigint;
    AjPStr cigarstr = NULL;
    AjPStr namestr = NULL;
    AjPStr seqstr = NULL;
    AjPStr qualstr = NULL;
    AjPStr tagstr = NULL;
    unsigned char dp;
    ajint filestat;

    if(!called)
    {
        called = ajTrue;
        bigendian = ajUtilGetBigendian();
        ajDebug("ajSeqReadBam bam bigendian: %B\n", bigendian);
    }

    buff = seqin->Filebuff;
    infile = ajFilebuffGetFile(buff);

    if(!seqin->Data)
    {
        ajFileTrace(infile);
        ajFilebuffTrace(buff);
        AJNEW0(bamdata);

        /* reset to beginning of file -
        ** has at least been tested for blank lines */
        filestat = ajFileSeek(infile, 0L, SEEK_SET);

        /* reopen BAM file as a BGZF file */
        bamdata->gzfile = ajSeqBamBgzfOpenfd(fileno(ajFilebuffGetFileptr(buff)),
                                             "rb");
        ajDebug("gzfile %x  fd:%d file:%x ubs:%d cbs:%d blen:%d boff:%d cache:%d open:'%c'\n",
               bamdata->gzfile, bamdata->gzfile->file_descriptor,
               bamdata->gzfile->file,
               bamdata->gzfile->uncompressed_block_size,
               bamdata->gzfile->compressed_block_size,
               bamdata->gzfile->block_length, bamdata->gzfile->block_offset,
               bamdata->gzfile->cache_size,
               bamdata->gzfile->open_mode);

	status = ajSeqBamBgzfEof(bamdata->gzfile);

	if (status < 0 && (errno == ESPIPE))
        {
            /*
            ** If the file is a pipe, checking the EOF marker will *always* fail
            ** with ESPIPE.  Suppress the error message in this case.
            */
	}
	else if (status <= 0)
        {
            /* seqReadBam EOF marker is absent */
            ajSeqBamBgzfClose(bamdata->gzfile);
            AJFREE(bamdata);
            ajFileSeek(infile,filestat,0);
            ajFilebuffResetPos(buff);
            ajFileTrace(infile);
            ajFilebuffTrace(buff);
            return ajFalse;
        }

        status = ajSeqBamBgzfRead(bamdata->gzfile, bambuf, 4);

        if(status < 0)
        {
            ajSeqBamBgzfClose(bamdata->gzfile);
            AJFREE(bamdata);
            ajFileSeek(infile,filestat,0);
            ajFilebuffResetPos(buff);
            return ajFalse;
        }

        ajDebug("gzfile %x  fd:%d file:%x ubs:%d cbs:%d blen:%d "
               "boff:%d cache:%d open:'%c'\n",
               bamdata->gzfile, bamdata->gzfile->file_descriptor,
               bamdata->gzfile->file,
               bamdata->gzfile->uncompressed_block_size,
               bamdata->gzfile->compressed_block_size,
               bamdata->gzfile->block_length,
               bamdata->gzfile->block_offset, bamdata->gzfile->cache_size,
               bamdata->gzfile->open_mode);

        if (strncmp(bambuf, "BAM\001", 4))
        {
            ajSeqBamBgzfClose(bamdata->gzfile);
            AJFREE(bamdata);
            ajFileSeek(infile,filestat,0);
            ajFilebuffResetPos(buff);
            return ajFalse;
        }

        /* BAM header */

	/* read plain text and the number of reference sequences */
        status = ajSeqBamBgzfRead(bamdata->gzfile, &ilen, 4);

        if(status != 4)
            ajErr("seqReadBam failed to read reference length %F", infile);

        if(bigendian)
            ajByteRevUint(&ilen);

        refname = ajCharNewRes(ilen+1);
        ajSeqBamBgzfRead(bamdata->gzfile, refname, ilen);
        ajDebug("bam text %u '%s'\n", ilen, refname);

        ajSeqBamBgzfRead(bamdata->gzfile, &ntargets, 4);

        if(bigendian)
            ajByteRevUint(&ntargets);

        ajDebug("bam targets %u\n", ntargets);
        bamdata->Nref = ntargets;

        /* read reference sequence names and lengths */
        targetname = AJCALLOC(ntargets, sizeof(char*));
        targetlen = AJCALLOC(ntargets, sizeof(ajuint));

        for(i=0; i < ntargets; i++) 
        {
            ajSeqBamBgzfRead(bamdata->gzfile, &namelen, 4);

            if(bigendian)
                ajByteRevUint(&namelen);

            targetname[i] = ajCharNewRes(namelen+1);
            ajSeqBamBgzfRead(bamdata->gzfile, targetname[i], namelen);
            ajSeqBamBgzfRead(bamdata->gzfile, &targetlen[i], 4);

            if(bigendian)
                ajByteRevUint(&targetlen[i]);

            ajDebug("bam target[%u] %u '%s'\n", i, targetlen[i], targetname[i]);
        }

        bamdata->bam = (AjPSeqBam)calloc(1, sizeof(AjOSeqBam));
        seqin->Data = bamdata;
    }

    /* next BAM record */

    bamdata = seqin->Data;
    b = bamdata->bam;
    ret = ajSeqBamRead(bamdata->gzfile, b);
    if(ret < -1)
        ajErr("seqReadBam truncated file return %d\n", ret);

    if(ret == -1)
    {
        ajSeqBamBgzfClose(bamdata->gzfile);
	ajFilebuffClear(seqin->Filebuff, 0);
	/*seqMsfDataDel((SeqPMsfData*)&seqin->Data);*/
        free(bamdata->bam->data); free(bamdata->bam);

        AJFREE(seqin->Data);
	return ajFalse;
    }

    c = &b->core;
    ajDebug("rID: %d pos: %d bin: %hd mapQual: %d read_name_len: %d"
            " flag_nc: %hd cigar_len: %hd read_len: %d"
            " mate_rID: %d mate_pos: %d ins_size: %d\n",
            c->tid, c->pos, c->bin, c->qual, c->l_qname,
            c->flag, c->n_cigar, c->l_qseq,
            c->mtid, c->mpos, c->isize);
    ajDebug("l_aux: %d data_len:%d m_data:%d\n",
            b->l_aux, b->data_len, b->m_data);
    d = b->data;
    dpos = 0;
    ajStrAssignC(&namestr, (const char*) &d[dpos]);
    ajSeqSetName(thys, namestr);
    ajDebug("read name: %p '%s'\n", dpos, &d[dpos]);
    dpos += (c->l_qname); /* l_qname includes trailing null */
    ajStrAssignC(&cigarstr, "");
    ajDebug("start of cigar %p\n", dpos);

    for(i=0; i < c->n_cigar; i++)
    {
        memcpy(&cigint, &d[dpos], 4);
        cigop = cigint & BAM_CIGAR_MASK;
        cigend = cigint >> BAM_CIGAR_SHIFT;

        ajFmtPrintAppS(&cigarstr, " %u%c",
                       cigend, cigarcode[cigop]);
        dpos += 4;
    }

    ajDebug("cigar: %p %S\n", dpos, cigarstr);

    ajStrAssignC(&seqstr, "");
    for(i=0; i < (ajuint) c->l_qseq; i++)
    {
        ajStrAppendK(&seqstr,
                     bam_nt16_rev_table[MAJSEQBAMSEQI(&d[dpos], i)]);
    }
    dpos += (c->l_qseq+1)/2;
    ajDebug("seq: %p '%S'\n", dpos, seqstr);

    ajStrAssignS(&thys->Seq, seqstr);

    if(d[dpos] == 0xFF)
    {
        AJFREE(thys->Accuracy);
        thys->Qualsize = 0;
        ajDebug("qual: MISSING\n");
        dpos++;
    }
    else
    {
        ajStrAssignC(&qualstr, "");

        if(thys->Qualsize < (ajuint) c->l_qseq)
        {
            AJCRESIZE(thys->Accuracy, c->l_qseq);
            thys->Qualsize = c->l_qseq;
        }

        for(i=0; i < (ajuint) c->l_qseq; i++)
        {
            ajFmtPrintAppS(&qualstr, " %02x", 33+d[dpos]);
            thys->Accuracy[i] = (float) (33 + d[dpos++]);
        }

        ajDebug("qual: %p %S\n", dpos, qualstr);
    }

    ajStrAssignC(&tagstr, "");

    while (dpos < (ajuint) b->data_len)
    {
        ajStrAppendK(&tagstr, ' ');
        ajStrAppendK(&tagstr, d[dpos++]);
        ajStrAppendK(&tagstr, d[dpos++]);
        ajStrAppendK(&tagstr, ':');
        dp = d[dpos++];
        ajStrAppendK(&tagstr, dp);
        ajStrAppendK(&tagstr, ':');

        if (dp == 'Z' || dp == 'H')
        {
            ajFmtPrintAppS(&tagstr,"%s", &d[dpos]);
            while(d[dpos])
                dpos++;
            dpos++;
        }
        else if (dp == 'f')
        {
            ajFmtPrintAppS(&tagstr,"%f", (float) *(&d[dpos]));
            dpos += 4;
        }
        else if (dp == 'd')
        {
            ajFmtPrintAppS(&tagstr,"%lf", (double) *(&d[dpos]));
            dpos += 8;
        }
        else if (dp == 'A')
        {
            ajFmtPrintAppS(&tagstr,"%c", &d[dpos++]);
        }
        else if (dp == 'c')
        {
            ajFmtPrintAppS(&tagstr,"%d",
                           (ajint) (signed char) d[dpos++]);
        }
        else if (dp == 's')
        {
            ajFmtPrintAppS(&tagstr,"%hd",
                           (ajshort) *(&d[dpos]));
            dpos += 2;
        }
        else if (dp == 'i')
        {
            ajFmtPrintAppS(&tagstr,"%d",
                           (ajint) *(&d[dpos]));
            dpos += 4;
        }
        else if (dp == 'C')
        {
            ajFmtPrintAppS(&tagstr,"%u",
                           (ajuint) d[dpos++]);
        }
        else if (dp == 'S')
        {
            ajFmtPrintAppS(&tagstr,"%hu",
                           (ajushort) *(&d[dpos]));
            dpos += 2;
        }
        else if (dp == 'I')
        {
            ajFmtPrintAppS(&tagstr,"%u",
                           (ajuint) d[dpos]);
            dpos += 4;
        }
        else {
            ajWarn("Unknown BAM aux type char(%d) '%c'", (ajint) dp, dp);
            ajFmtPrintAppS(&tagstr,"???");
        }
    }

    ajDebug("tags: %p '%S'\n", dpos, tagstr);

    seqin->Records++;

    bamdata->Count++;

    return ajTrue;
}





/* @funcstatic seqReadSam ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using sequence alignment/map (SAM) format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadSam(AjPSeq thys, AjPSeqin seqin)
{
    AjBool ok = ajTrue;
    AjPFilebuff buff;
    AjPFile infile = NULL;
    AjPStrTok handle = NULL;
    AjPStr token = NULL;
    AjBool badformat = ajFalse;
    ajuint seqlen = 0;
    const char *cp;
    ajuint i;
    ajint iqual;
    ajint qmin = 33;
    ajint qmax = 126;
    ajuint flags;

    buff = seqin->Filebuff;
    infile = ajFilebuffGetFile(buff);

    /* === header section === */
    
    ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    if(seqin->Count == 1)
    {
        while(ok && ajStrGetCharFirst(seqReadLine) == '@')
        {
            ajStrTokenAssignC(&handle, seqReadLine, "\t");
            ajStrTokenNextParse(&handle,&token);
            switch(ajStrGetCharPos(token, 1))
            {
                case 'H':
                    if(!ajStrMatchC(token, "@HD"))
                        badformat = ajTrue;
                    break;
                case 'S':
                    if(!ajStrMatchC(token, "@SQ"))
                        badformat = ajTrue;
                    break;
                case 'R':
                    if(!ajStrMatchC(token, "@RG"))
                        badformat = ajTrue;
                    break;
                case 'P':
                    if(!ajStrMatchC(token, "@PG"))
                        badformat = ajTrue;
                    break;
                case 'C':
                    if(!ajStrMatchC(token, "@CO"))
                        badformat = ajTrue;
                    break;
                default:
                    badformat = ajTrue;
                    break;
            }
            if(badformat) 
            {
                ajErr("bad sam format header record '%S'", seqReadLine);
                return ajFalse;
            }
            ok = ajBuffreadLineStore(buff, &seqReadLine,
                                     seqin->Text, &thys->TextPtr);
        }
    }

    if(!ok)
        return ajFalse;

    if(ajStrParseCountC(seqReadLine, "\t") < 11)
        return ajFalse;

    seqin->Records++;

    ajStrTokenAssignC(&handle, seqReadLine, "\t\n");

    ajStrTokenNextParseNoskip(&handle,&token); /* QNAME */
    ajDebug("QNAME '%S'\n", token);
    seqSetNameNospace(&thys->Name, token);

    ajStrTokenNextParseNoskip(&handle,&token); /* FLAG */
    ajDebug("FLAG  '%S'\n", token);
    if(ajStrGetLen(token)){
        if(!ajStrToUint(token, &flags))
            return ajFalse;
    }
    ajDebug("flags %x\n", flags);
    
    ajStrTokenNextParseNoskip(&handle,&token); /* RNAME */
    ajDebug("RNAME '%S'\n", token);
    if(ajStrGetLen(token))
        seqAccSave(thys, token);
    
    ajStrTokenNextParseNoskip(&handle,&token); /* POS */
    ajDebug("POS   '%S'\n", token);
    if(ajStrGetLen(token)){
        if(!ajStrToUint(token, &flags))
            return ajFalse;
    }

    ajStrTokenNextParseNoskip(&handle,&token); /* MAPQ */
    ajDebug("MAPQ  '%S'\n", token);

    ajStrTokenNextParseNoskip(&handle,&token); /* CIGAR */
    ajDebug("CIGAR '%S'\n", token);

    ajStrTokenNextParseNoskip(&handle,&token); /* MRNM */
    ajDebug("MRNM  '%S'\n", token);

    ajStrTokenNextParseNoskip(&handle,&token); /* MPOS */
    ajDebug("MPOS  '%S'\n", token);
    if(ajStrGetLen(token)){
        if(!ajStrToUint(token, &flags))
            return ajFalse;
    }

    ajStrTokenNextParseNoskip(&handle,&token); /* ISIZE */
    ajDebug("ISIZE '%S'\n", token);
    if(ajStrGetLen(token)){
        if(!ajStrToUint(token, &flags))
            return ajFalse;
    }

    ajStrTokenNextParseNoskip(&handle,&token); /* SEQ */
    ajDebug("SEQ   '%S'\n", token);
    seqAppend(&thys->Seq, token);
    seqlen = MAJSTRGETLEN(token);

    ajStrTokenNextParseNoskip(&handle,&token); /* QUAL */
    ajDebug("QUAL  '%S'", token);

    if(ajStrCmpC(token,"*")!=0)
    {
    if(MAJSTRGETLEN(token) != seqlen)
    {
      	ajWarn("SAM quality length mismatch '%F' '%S' "
               "expected: %u found: %u '%S' '%S'",
	       infile, thys->Name,
	       seqlen, ajStrGetLen(seqQualStr), thys->Seq, token);
        return ajFalse;
    }
    
    cp = MAJSTRGETPTR(token);
    i=0;

    if(seqlen > thys->Qualsize)
        AJCRESIZE(thys->Accuracy, seqlen);

    thys->Qualsize = seqlen;

    if(MAJSTRGETLEN(token) > thys->Qualsize)
        AJCRESIZE(thys->Accuracy, MAJSTRGETLEN(seqQualStr));

    while (*cp)
    {
        iqual = *cp++;
        if(iqual < qmin)
	{
            ajWarn("SAM '%F' sequence '%S' "
                   "quality value %d '%c' too low",
                   infile, thys->Name,
                   (ajint) (cp - MAJSTRGETPTR(token)), (char) iqual);
            iqual = qmin;
	}
        if(iqual > qmax)
	{
	    ajWarn("SAM '%F' sequence '%S' "
                   "quality value '%c' too high",
                   infile, thys->Name,
                   (char) iqual);
	    iqual = qmax;
        }
        thys->Accuracy[i++] = seqQualPhred[iqual];
    }
    }




/* @HD header VN:
    ** 
    */




/* @SQ sequence dictionary SN: LN:
    **
    */




/* @RG read group ID: SM:
    **
    */




/* @PG program name ID:
    **
    */




/* @CO comment
    **
    */

    /* === alignment section === */

    /* 11 fields then (tag:vtype:value)... */

    
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
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPFilebuff buff;
    AjBool ok = ajTrue;

    buff = seqin->Filebuff;

    do
    {
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);

	if(ok)
        {
            seqin->Records++;
            if(ajStrPrefixC(seqReadLine, "; DNA sequence"))
            {
                ajStrTokenAssignC(&handle, seqReadLine, " ;\t,\n");
                ajStrTokenNextParse(&handle, &token); /* 'DNA' */
                ajStrTokenNextParse(&handle, &token); /* sequence */
                ajStrTokenNextParse(&handle, &token); /* entry name */
            }
        }
        
    } while(ok && ajStrPrefixC(seqReadLine, ";"));

    ajStrTokenDel(&handle);

    if(!ok || !ajStrGetLen(token))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	ajStrDel(&token);

	return ajFalse;
    }

    /* we know we will succeed from here ... no way to return ajFalse */

    ajFilebuffSetUnbuffered(buff);

    seqSetName(thys, token);

    /* OK, we have the name. Now look for the sequence */

    while(ok && !ajStrPrefixC(seqReadLine, "//"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	seqin->Records++;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajFilebuffClear(buff, 0);
    ajStrDel(&token);

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
    ajuint len;
    AjBool ok  = ajFalse;
    ajuint iseq = 0;

    AjPFilebuff buff;
    AjPTable msftable   = NULL;
    SeqPMsfItem msfitem = NULL;
    AjPList msflist     = NULL;
    SeqPMsfData msfdata = NULL;

    ajuint i;

    ajDebug("seqReadMsf seqin->Data %x\n", seqin->Data);

    buff = seqin->Filebuff;

    if(!seqin->Data)
    {
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;

	seqin->Records++;

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
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	    return ajFalse;
	}

        /* we know we will succeed from here ... no way to return ajFalse */

        ajFilebuffSetUnbuffered(buff);

	seqin->Data = AJNEW0(msfdata);
	msfdata->Table = msftable = ajTablestrNew();
	msflist = ajListstrNew();
	seqin->Filecount = 0;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	seqin->Records++;

	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    seqin->Records++;

	    if(seqGcgMsfHeader(seqReadLine, &msfitem))
	    {
		ajTablePut(msftable, ajStrNewS(msfitem->Name), msfitem);
		ajListstrPushAppend(msflist, ajStrNewS(msfitem->Name));
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

	ajListstrFreeData(&msflist);

	while(ajBuffreadLineStore(buff, &seqReadLine,
				 seqin->Text, &thys->TextPtr))
	{
	    seqin->Records++;
	    seqGcgMsfReadseq(seqReadLine, msftable);
	}

	ajTableMap(msftable, seqMsfTabList, NULL);
	msfdata->Nseq = iseq;
	msfdata->Count = 0;
	msfdata->Bufflines = seqin->Records;
	ajDebug("MSF format read %d lines\n", seqin->Records);
    }

    msfdata = seqin->Data;
    msftable = msfdata->Table;

    if(msfdata->Count >= msfdata->Nseq)
    {
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*)&seqin->Data);

	return ajFalse;
    }

    i = msfdata->Count;
    ajDebug("returning [%d] '%S'\n", i, msfdata->Names[i]);
    msfitem = ajTableFetch(msftable, msfdata->Names[i]);
    ajStrAssignS(&thys->Name, msfdata->Names[i]);

    thys->Weight = msfitem->Weight;
    ajStrAssignS(&thys->Seq, msfitem->Seq);

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

    msfitem = ajTableFetch(msftable, token);

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




/* @funcstatic seqMsfDataDel **************************************************
**
** Destructor for SeqPMsfData objects
**
** @param [d] pthys [SeqPMsfData*] MSF data object
** @return [void]
** @@
******************************************************************************/

static void seqMsfDataDel(SeqPMsfData* pthys)
{
    SeqPMsfData thys;
    ajuint i;

    if(!pthys)
        return;

    if(!*pthys)
        return;

    thys = *pthys;

    ajDebug("seqMsfDataDel Nseq:%u Count:%u Table:%u Nexus:%u\n",
	    thys->Nseq, thys->Count, ajTableGetLength(thys->Table),
	    ajNexusGetNtaxa(thys->Nexus));

    for(i=0; i < thys->Nseq; i++)
    {
	ajStrDel(&thys->Names[i]);
    }

    
    AJFREE(thys->Names);

    ajNexusDel(&thys->Nexus);
    ajStrDel(&thys->Gene);
    ajStrDel(&thys->Domain);
    ajStrDel(&thys->NextGene);
    ajStrDel(&thys->NextDomain);
    ajTableMapDel(thys->Table, seqMsfTabDel, NULL);
    ajTableFree(&thys->Table);

    AJFREE(*pthys);

    return;
}




/* @funcstatic seqMsfItemDel **************************************************
**
** Destructor for SeqPMsfItem objects
**
** @param [d] pthys [SeqPMsfItem*] MSF item object
** @return [void]
** @@
******************************************************************************/

static void seqMsfItemDel(SeqPMsfItem* pthys)
{
    SeqPMsfItem thys;

    if(!pthys)
        return;

    if(!*pthys)
        return;

    thys = *pthys;

    ajStrDel(&thys->Name);
    ajStrDel(&thys->Desc);
    ajStrDel(&thys->Seq);

    AJFREE(*pthys);

    return;
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

    (void) cl;

    msfitem = (SeqPMsfItem) *value;

    ajDebug("key '%S' Name '%S' Seqlen %d\n",
	    key, msfitem->Name, ajStrGetLen(msfitem->Seq));

    return;
}




/* @funcstatic seqMsfDataTrace ************************************************
**
** Debug trace report for SeqPMsfData objects
**
** @param [r] thys [const SeqPMsfData] MSF data object
** @return [void]
** @@
******************************************************************************/

static void seqMsfDataTrace(const SeqPMsfData thys)
{
    ajuint i;

    if(!thys)
    {
	ajDebug("seqMsfDataTrace <null>\n");
	return;
    }

    ajDebug("seqMsfDataTrace Nseq:%u Count:%u Table:%u Nexus:%u\n",
	    thys->Nseq, thys->Count, ajTableGetLength(thys->Table),
	    ajNexusGetNtaxa(thys->Nexus));

    for(i=0; i < thys->Nseq; i++)
	if(i < thys->Count)
	    ajDebug("* [%u] '%S'\n", i, thys->Names[i]);
	else
	    ajDebug("  [%u] '%S'\n", i, thys->Names[i]);

    ajTableMap(thys->Table, seqMsfTabList, NULL);

    return;
}




/* @funcstatic seqMsfTabDel ***************************************************
**
** Deletes entries from the MSF internal table. Called for each entry in turn.
**
** @param [d] key [void**] Standard argument, table key.
** @param [d] value [void**] Standard argument, table data item.
** @param [r] cl [void*] Standard argument, usually NULL
** @return [void]
** @@
******************************************************************************/

static void seqMsfTabDel(void** key, void** value, void* cl)
{
    SeqPMsfItem msfitem;
    AjPStr keystr;

    (void) cl;

    keystr = (AjPStr) *key;
    msfitem = (SeqPMsfItem) *value;

    ajStrDel(&keystr);

    seqMsfItemDel(&msfitem);

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
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjBool ok;
    AjPFilebuff buff;
    AjPStr tmpstr = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr liststr;			/* for lists, do not delete */
    AjPStr datestr = NULL;
    AjPStr datetype = NULL;
    AjPStr relstr = NULL;
    AjPStr taxstr = NULL;
    AjPStr cmtstr = NULL;		/* stored in AjPSeq - do not delete */
    ajuint icount = 0;
    AjPSeqRef  seqref  = NULL;
    AjPSeqXref xref    = NULL;
    AjPSeqGene seqgene = NULL;
    AjPSeqDesc desctop = NULL;
    AjPSeqDesc descmaster = NULL;
    AjPSeqSubdesc subdesc = NULL;
    AjBool descistop = ajTrue;
    AjBool isdescflag = ajFalse;
    AjPStr *Pdescstr = NULL;
    AjPStr newdescstr = NULL;
    AjPStr genetoken = NULL;
    ajuint refnum;
    AjBool isnewgene = ajFalse;
    AjBool isgenetoken = ajFalse;
    AjIList iter;
    AjIList itb;
    AjIList itc;

/*
** To be done: 12-Feb-09
** input line wrapping test GN,
** continue lines for OS
**
** New line types:
**    OH organism host: list of tax ids
**
** CC line blocks -!- TOPIC:
** can do this by parsing the stored comment block
**
** DR lines - can parse out the details
*/
    
    buff = seqin->Filebuff;

    if(!seqFtFmtSwiss)
	ajStrAssignC(&seqFtFmtSwiss, "swissprot");

    if(!ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr))
	return ajFalse;

    seqin->Records++;

    /* for GCG formatted databases */

    while(ajStrPrefixC(seqReadLine, "WP "))
    {
	if(!ajBuffreadLineStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;

	seqin->Records++;
    }

    /* extra blank lines */

    while(ajStrIsWhite(seqReadLine))
    {
	if(!ajBuffreadLineStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;

	seqin->Records++;
    }

    ajDebug("seqReadSwiss first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "ID   "))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    ajStrTokenNextParse(&handle, &token);	/* 'ID' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName(thys, token);

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    if(!thys->Fulldesc)
        thys->Fulldesc = ajSeqdescNew();

    while(ok && !ajStrPrefixC(seqReadLine, "SQ   "))
    {
	seqin->Records++;

	/* check for Staden Experiment format instead */
	if(ajStrPrefixC(seqReadLine, "EN   ") ||
	   ajStrPrefixC(seqReadLine, "TN   ") ||
	   ajStrPrefixC(seqReadLine, "EX   ") )
	{
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	    ajStrTokenDel(&handle);
	    ajStrDel(&token);

	    return ajFalse;;
	}

	else if(ajStrPrefixC(seqReadLine, "AC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'AC' */

	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	else if(ajStrPrefixC(seqReadLine, "DE   "))
	{
            if(!desctop)
            {
                desctop = thys->Fulldesc;
                descmaster = thys->Fulldesc;
                Pdescstr = &thys->Desc;
            }

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DE' */

	    while(ajStrTokenNextParseC(&handle, " ;\n\r", &token))
            {
                if(ajStrGetCharLast(token) == ':')
                {
                    isdescflag = ajFalse;

                    if(ajStrPrefixC(token, "RecName:"))
                    {
                        Pdescstr = &descmaster->Name;
                        descistop = ajTrue;
                    }
                    else if(ajStrPrefixC(token, "AltName:"))
                    {
                        subdesc = ajSeqsubdescNew();
                        descistop = ajFalse;
                        Pdescstr = &subdesc->Name;
                        ajListPushAppend(descmaster->AltNames, subdesc);
                    }
                    else if(ajStrPrefixC(token, "SubName:"))
                    {
                        subdesc = ajSeqsubdescNew();
                        descistop = ajFalse;
                        Pdescstr = &subdesc->Name;
                        ajListPushAppend(descmaster->SubNames, subdesc);
                   }
                    else if(ajStrPrefixC(token, "Includes:"))
                    {
                        descmaster = ajSeqdescNew();
                        descistop = ajTrue;
                        ajListPushAppend(thys->Fulldesc->Includes, descmaster);
                        Pdescstr = &descmaster->Name;
                    }
                    else if(ajStrPrefixC(token, "Contains:"))
                    {
                        descmaster = ajSeqdescNew();
                        descistop = ajTrue;
                        ajListPushAppend(thys->Fulldesc->Contains, descmaster);
                        Pdescstr = &descmaster->Name;
                    }
                    else if(ajStrPrefixC(token, "Flags:"))
                        isdescflag = ajTrue;
                    else
                    {
                        ajDebug("Swissprot DE line UNKNOWN token '%S'\n",
                                token);

                        if(ajStrGetLen(*Pdescstr))
                            ajStrAppendK(Pdescstr, ' ');

                        ajStrAppendS(Pdescstr, token);
                    }
                }
                else if(ajStrPrefixC(token, "Full="))
                {
                    if(descistop)
                    {
                        Pdescstr = &descmaster->Name;
                    }
                    else
                    {
                        Pdescstr = &subdesc->Name;
                    }

                    ajStrAssignSubS(Pdescstr, token, 5, -1);
                }
                else if(ajStrPrefixC(token, "Short="))
                {
                    newdescstr = ajStrNewC("");
                    Pdescstr = &newdescstr;

                    if(descistop)
                        ajListstrPushAppend(descmaster->Short, newdescstr);
                    else
                        ajListstrPushAppend(subdesc->Short, newdescstr);

                    ajStrAssignSubS(Pdescstr, token, 6, -1);
                }
                else if(ajStrPrefixC(token, "EC="))
                {
                    newdescstr = ajStrNewC("");
                    Pdescstr = &newdescstr;

                    if(descistop)
                        ajListstrPushAppend(descmaster->EC, newdescstr);
                    else
                        ajListstrPushAppend(subdesc->EC, newdescstr);

                    ajStrAssignSubS(Pdescstr, token, 3, -1);
                    xref = ajSeqxrefNewDbC(*Pdescstr, "ENZYME", XREF_EC);
                    ajSeqAddXref(thys, xref);
                    xref = NULL;
                }
                else if(ajStrPrefixC(token, "Allergen="))
                {
                    newdescstr = ajStrNewC("");
                    Pdescstr = &newdescstr;
                    ajListstrPushAppend(subdesc->Allergen, newdescstr);
                    ajStrAssignSubS(Pdescstr, token, 9, -1);
                    xref = ajSeqxrefNewDbC(*Pdescstr, "Allergen", XREF_DESC);
                    ajSeqAddXref(thys, xref);
                    xref = NULL;
                }
                else if(ajStrPrefixC(token, "Biotech="))
                {
                    newdescstr = ajStrNewC("");
                    Pdescstr = &newdescstr;
                    ajListstrPushAppend(subdesc->Biotech, newdescstr);
                    ajStrAssignSubS(Pdescstr, token, 8, -1);
                }
                else if(ajStrPrefixC(token, "CD_antigen="))
                {
                    newdescstr = ajStrNewC("");
                    Pdescstr = &newdescstr;
                    ajListstrPushAppend(subdesc->Cdantigen, newdescstr);
                    ajStrAssignSubS(Pdescstr, token, 11, -1);
                    xref = ajSeqxrefNewDbC(*Pdescstr, "CD_Antigen", XREF_DESC);
                    ajSeqAddXref(thys, xref);
                    xref = NULL;
                }
                else if(ajStrPrefixC(token, "INN="))
                {
                    newdescstr = ajStrNewC("");
                    Pdescstr = &newdescstr;
                    ajListstrPushAppend(subdesc->Inn, newdescstr);
                    ajStrAssignSubS(Pdescstr, token, 4, -1);
                }
                else 
                {
                    if(isdescflag)
                    {
                        if(ajStrMatchC(token,"Precursor"))
                            thys->Fulldesc->Precursor = ajTrue;
                        else if(ajStrMatchC(token,"Fragments"))
                            thys->Fulldesc->Fragments = 2;
                        else if(ajStrMatchC(token,"Fragment"))
                            thys->Fulldesc->Fragments = 1;
                        else
                            ajDebug("unknown description flag text '%S'\n",
                                    token);
                    }
                    else 
                    {
                        if(ajStrGetLen(*Pdescstr))
                            ajStrAppendK(Pdescstr, ' ');

                        ajStrAppendS(Pdescstr, token);
                    }
                }
            }
        }

        /* needs a little work for wrapped lines - save token and
        ** append rather than set at the current level
        */

	else if(ajStrPrefixC(seqReadLine, "GN   "))
	{
            if(!seqgene)
            {
                isnewgene = ajTrue;
                seqgene = ajSeqgeneNew();
            }

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'GN' */
	    ajStrTokenNextParseC(&handle, ";=\n\r", &token);

            if(ajStrMatchC(token, "and")) /* test 'and' between genes */
            {
                isnewgene = ajTrue;
                seqgene = ajSeqgeneNew();
            }
            else
            {
                while(ajStrGetLen(token))
                {
                    isgenetoken = ajTrue;
                    ajStrTrimWhite(&token);

                    if(ajStrMatchC(token, "Name"))
                    {
                        ajStrTokenNextParseC(&handle, ";\n\r", &tmpstr);
                        ajSeqgeneSetName(seqgene, tmpstr);
                    }
                    else if (ajStrMatchC(token, "Synonyms"))
                    {
                        ajStrTokenNextParseC(&handle, ";\n\r", &tmpstr);
                        ajSeqgeneSetSynonyms(seqgene, tmpstr);
                    }
                    else if (ajStrMatchC(token, "OrderedLocusNames"))
                    {
                        ajStrTokenNextParseC(&handle, ";\n\r", &tmpstr);
                        ajSeqgeneSetOln(seqgene, tmpstr);
                    }
                    else if (ajStrMatchC(token, "ORFNames"))
                    {
                        ajStrTokenNextParseC(&handle, ";\n\r", &tmpstr);
                        ajSeqgeneSetOrf(seqgene, tmpstr);
                    }
                    else
                    {
                        isgenetoken = ajFalse;
                        ajDebug("Swissnew GN line unexpected '%S' (%S)",
                               token, genetoken);

                        if(ajStrMatchC(genetoken, "Name"))
                            ajSeqgeneAppendName(seqgene, token);
                        else if (ajStrMatchC(genetoken, "Synonyms"))
                            ajSeqgeneAppendSynonyms(seqgene, token);
                        else if (ajStrMatchC(genetoken,
                                             "OrderedLocusNames"))
                            ajSeqgeneAppendOln(seqgene, token);
                        else if (ajStrMatchC(genetoken, "ORFNames"))
                            ajSeqgeneAppendOrf(seqgene, token);
                    }

                    ajStrTokenNextParseC(&handle, "=;\n\r", &token);

                    if(isgenetoken)
                        ajStrAssignS(&genetoken, token);
                }

                if(isnewgene)
                {
                    isnewgene = ajFalse;
                    ajSeqAddGene(thys, seqgene);
                }
                /* keep seqgene so we can add to it if the line wraps */
            }
	}

	else if(ajStrPrefixC(seqReadLine, "PE   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* PE */
	    ajStrTokenNextParseC(&handle, "\n\r", &token);

            if(ajStrGetLen(token))
                ajStrAssignS(&thys->Evidence, token);
        }
    

	else if(ajStrPrefixC(seqReadLine, "KW   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'KW' */

	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		ajSeqAddKey(thys, liststr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "OS   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OS' */

	    while(ajStrTokenNextParseC(&handle, "\n\r", &token))
	    {
                if(ajStrGetLen(taxstr))
                    ajStrAppendK(&taxstr, ' ');

		ajStrAppendS(&taxstr, token);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "OC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OC' */

	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 0);
		ajStrDel(&tmpstr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "OG   "))
	{
            ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OG' */
	    ajStrTokenNextParse(&handle, &token);
            ajStrAssignS(&tmpstr, token);

            while(ajStrTokenNextParse(&handle, &token))
            {
                ajStrAppendK(&tmpstr, ' ');
                ajStrAppendS(&tmpstr, token);
            }
            
            if(ajStrGetCharLast(tmpstr) == '.')
                ajStrCutEnd(&tmpstr, 1);

            seqTaxSave(thys, tmpstr, 2);
	}

	else if(ajStrPrefixC(seqReadLine, "OX   "))
	{
            ajStrTokenAssignC(&handle, seqReadLine, " =;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OX' */
	    ajStrTokenNextParse(&handle, &token);
            ajStrAssignS(&tmpstr, token);

            if(ajStrMatchC(token, "NCBI_TaxID"))
            {
                ajStrTokenNextParse(&handle, &token);
		seqTaxidSaveS(thys, token);
                xref = ajSeqxrefNewDbC(token, "taxon", XREF_TAX);
                ajSeqAddXref(thys, xref);
                xref = NULL;
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "CC   "))
	{
	    ajStrAssignSubS(&token, seqReadLine, 5, -1);

	    if(ajStrGetLen(cmtstr))
            {
		ajStrAppendC(&cmtstr, "\n");

                if(ajStrPrefixC(token, "-!- ") ||
                   (ajStrPrefixC(token, "--------") &&
                    ajStrPrefixC(cmtstr, "-!- ")))
                {
                    ajSeqAddCmt(thys, cmtstr);
                    cmtstr = NULL;
                }
            }

	    ajStrAppendS(&cmtstr, token);
        }

	else if(ajStrPrefixC(seqReadLine, "DR   "))
	{
            AJNEW0(xref);
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'DR' */
	    ajStrTokenNextParseC(&handle, ";\n\r", &token); /* dbname */
	    ajStrAssignS(&xref->Db, token);
            ajStrTrimWhite(&token);
	    ajStrTokenNextParse(&handle, &token); /* primary */
            ajStrTrimWhite(&token);
	    ajStrAssignS(&xref->Id, token);
	    ajStrTokenNextParse(&handle, &token); /* secondary*/

            if(!ajStrGetLen(token))
            {
                if(ajStrGetCharLast(xref->Id) == '.')
                    ajStrCutEnd(&xref->Id, 1);
            }
            else
            {
                if(ajStrGetCharLast(token) == '.')
                    ajStrCutEnd(&token, 1);
                ajStrTrimWhite(&token);
                ajStrAssignS(&xref->Secid, token);

                ajStrTokenNextParse(&handle, &token); /* secondary*/

                if(!ajStrGetLen(token))
                {
                    if(ajStrGetCharLast(xref->Secid) == '.')
                        ajStrCutEnd(&xref->Secid, 1);
                }
                else
                {
                    if(ajStrGetCharLast(token) == '.')
                        ajStrCutEnd(&token, 1);
                    ajStrTrimWhite(&token);
                    ajStrAssignS(&xref->Terid, token);

                    ajStrTokenNextParse(&handle, &token); /* secondary*/

                    if(!ajStrGetLen(token))
                    {
                        if(ajStrGetCharLast(xref->Terid) == '.')
                            ajStrCutEnd(&xref->Terid, 1);
                    }
                    else
                    {
                        if(ajStrGetCharLast(token) == '.')
                            ajStrCutEnd(&token, 1);
                        ajStrTrimWhite(&token);
                        ajStrAssignS(&xref->Quatid, token);
                    }
                }
            }
            xref->Type = XREF_DR;
	    ajSeqAddXref(thys, xref);
	}

	else if(ajStrPrefixC(seqReadLine, "RN   "))
	{
            if(seqref)
            {
                ajSeqrefStandard(seqref);
                ajSeqAddRef(thys, seqref);
            }

	    seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RN' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* [num] */
	    ajStrAssignSubS(&tmpstr, token, 1, -2);
	    ajStrToUint(tmpstr, &refnum);
	    ajSeqrefSetnumNumber(seqref, refnum);
	}

	else if(ajStrPrefixC(seqReadLine, "RG   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RG' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* groupname */
	    ajSeqrefAppendGroupname(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RX   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RX' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* xref */
	    ajSeqrefAppendXref(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RP   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RP' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* position */
	    ajSeqrefAppendPosition(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RA   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RA' */
	    ajStrTokenNextParseC(&handle, "\n\r;", &token); /* authors */
	    ajSeqrefAppendAuthors(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RT   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RT' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* title */

	    if(!ajStrMatchC(token, ";"))
		ajSeqrefAppendTitle(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RL   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RL' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* authors */
	    ajSeqrefAppendLocation(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RC   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RC' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* comment */
	    ajSeqrefAppendComment(seqref, token);
	}

        else if(ajStrPrefixC(seqReadLine, "FT   "))
        {
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

		ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		/* ajDebug("SWISS FEAT saved line:\n%S", seqReadLine); */
	    }
        }

	else if(ajStrPrefixC(seqReadLine, "DT   "))
	{
	    if(!thys->Date)
		thys->Date = ajSeqdateNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " (),.\n\r");
	    icount = 0;

	    while(ajStrTokenNextParse(&handle, &token))
	    {
		icount++;

		if(icount==2)
		    ajStrAssignS(&datestr, token);
		else if(icount == 3)
		    ajStrAssignS(&datetype, token);
		else if(icount == 5)
		    ajStrAssignS(&relstr, token);
            }

            if(ajStrMatchC(datetype, "integrated"))
            {
                ajSeqdateSetCreateS(thys->Date, datestr);
                ajStrAssignS(&thys->Date->CreVer, relstr);
            }
            else if (ajStrMatchC(datetype, "sequence"))
            {
                ajSeqdateSetModseqS(thys->Date, datestr);
                ajStrAssignS(&thys->Date->SeqVer, relstr);
            }
            else if (ajStrMatchC(datetype, "entry"))
            {
                ajSeqdateSetModifyS(thys->Date, datestr);
                ajStrAssignS(&thys->Date->ModVer, relstr);
	    }
            else 
            {
                ajDebug("unknown datetype '%S' '%S'",
                       datetype, seqReadLine);
            }
	}

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    if(ajStrGetLen(taxstr)) 
    {
        ajStrTrimWhite(&taxstr);

        if(ajStrGetCharLast(taxstr) == '.')
            ajStrCutEnd(&taxstr, 1);
        seqTaxSave(thys, taxstr, 1);
    }

    if(seqref)                  /* clean up the last reference */
    {
        ajSeqrefStandard(seqref);
        ajSeqAddRef(thys, seqref);
        seqref = NULL;
    }

    if(ajStrGetLen(cmtstr))
    {
        ajSeqAddCmt(thys, cmtstr);
        cmtstr = NULL;
    }

    if(dofeat)
    {
 	ajDebug("EMBL FEAT TabIn %x\n", seqin->Ftquery);
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeattableNewRead(seqin->Ftquery);
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
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);

	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    seqin->Records++;
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	}
    }
    ajStrDel(&tmpstr);    

    if(!ajStrGetLen(thys->Desc))
    {
        ajStrAssignS(&thys->Desc, thys->Fulldesc->Name);

        iter = ajListIterNewread(thys->Fulldesc->Short);

        while((tmpstr = (AjPStr) ajListIterGet(iter)))
        {
            if(ajStrGetLen(tmpstr))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);
        }
        
        ajListIterDel(&iter);

        iter = ajListIterNewread(thys->Fulldesc->EC);

        while((tmpstr = (AjPStr) ajListIterGet(iter)))
        {
            if(ajStrGetLen(tmpstr))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);
        }
        
        ajListIterDel(&iter);

        iter = ajListIterNewread(thys->Fulldesc->AltNames);

        while((subdesc = (AjPSeqSubdesc) ajListIterGet(iter)))
        {
            if(ajStrGetLen(subdesc->Name))
            {
                ajFmtPrintAppS(&thys->Desc, " (%S)", subdesc->Name);
            }

            itb = ajListIterNewread(subdesc->Inn);
            while((tmpstr = (AjPStr) ajListIterGet(itb)))
            {
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);
            }
            
            ajListIterDel(&itb);

            itb = ajListIterNewread(subdesc->Short);

            while((tmpstr = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

            ajListIterDel(&itb);

            itb = ajListIterNewread(subdesc->EC);

            while((tmpstr = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);
            ajListIterDel(&itb);

            itb = ajListIterNewread(subdesc->Allergen);

            while((tmpstr = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);
            ajListIterDel(&itb);

            itb = ajListIterNewread(subdesc->Biotech);

            while((tmpstr = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);
            ajListIterDel(&itb);

            itb = ajListIterNewread(subdesc->Cdantigen);

            while((tmpstr = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

            ajListIterDel(&itb);
        }
        
        ajListIterDel(&iter);

        iter = ajListIterNewread(thys->Fulldesc->SubNames);

        while((subdesc = (AjPSeqSubdesc) ajListIterGet(iter)))
        {
            ajFmtPrintAppS(&thys->Desc, " (%S)", subdesc->Name);

            itb = ajListIterNewread(subdesc->Short);

            while((tmpstr = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

            ajListIterDel(&itb);

            itb = ajListIterNewread(subdesc->EC);

            while((tmpstr = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

            ajListIterDel(&itb);
        }

        ajListIterDel(&iter);

        iter = ajListIterNewread(thys->Fulldesc->Includes);

        while((desctop = (AjPSeqDesc) ajListIterGet(iter)))
        {
            ajFmtPrintAppS(&thys->Desc, " (%S)", desctop->Name);
            itb = ajListIterNewread(desctop->Short);

            while((tmpstr = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

            ajListIterDel(&itb);

            itb = ajListIterNewread(desctop->EC);

            while((tmpstr = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

            ajListIterDel(&itb);

            itb = ajListIterNewread(desctop->AltNames);

            while((subdesc = (AjPSeqSubdesc) ajListIterGet(itb)))
            {
                if(ajStrGetLen(subdesc->Name))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", subdesc->Name);

                itc = ajListIterNewread(subdesc->Inn);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                itc = ajListIterNewread(subdesc->Short);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                itc = ajListIterNewread(subdesc->EC);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                itc = ajListIterNewread(subdesc->Allergen);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                itc = ajListIterNewread(subdesc->Biotech);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                itc = ajListIterNewread(subdesc->Cdantigen);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);
            }

            ajListIterDel(&itb);
        
            itb = ajListIterNewread(desctop->SubNames);

            while((subdesc = (AjPSeqSubdesc) ajListIterGet(itb)))
            {
                ajFmtPrintAppS(&thys->Desc, " (%S)", subdesc->Name);

                itc = ajListIterNewread(subdesc->Short);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                itc = ajListIterNewread(subdesc->EC);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

            }

            ajListIterDel(&itb);

        }

        ajListIterDel(&iter);

        iter = ajListIterNewread(thys->Fulldesc->Contains);

        while((desctop = (AjPSeqDesc) ajListIterGet(iter)))
        {
            ajFmtPrintAppS(&thys->Desc, " (%S)", desctop->Name);

            itb = ajListIterNewread(desctop->Short);

            while((tmpstr = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

            ajListIterDel(&itb);

            itb = ajListIterNewread(desctop->EC);

            while((tmpstr = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

            ajListIterDel(&itb);

            itb = ajListIterNewread(desctop->AltNames);

            while((subdesc = (AjPSeqSubdesc) ajListIterGet(itb)))
            {
                if(ajStrGetLen(subdesc->Name))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", subdesc->Name);

                itc = ajListIterNewread(subdesc->Inn);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                itc = ajListIterNewread(subdesc->Short);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                itc = ajListIterNewread(subdesc->EC);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                itc = ajListIterNewread(subdesc->Allergen);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                itc = ajListIterNewread(subdesc->Biotech);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                itc = ajListIterNewread(subdesc->Cdantigen);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

            }

            ajListIterDel(&itb);
        
            itb = ajListIterNewread(desctop->SubNames);

            while((subdesc = (AjPSeqSubdesc) ajListIterGet(itb)))
            {
                ajFmtPrintAppS(&thys->Desc, " (%S)", subdesc->Name);
                itc = ajListIterNewread(subdesc->Short);

                itc = ajListIterNewread(subdesc->Cdantigen);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                itc = ajListIterNewread(subdesc->EC);

                itc = ajListIterNewread(subdesc->Cdantigen);

                while((tmpstr = (AjPStr) ajListIterGet(itc)))
                    ajFmtPrintAppS(&thys->Desc, " (%S)", tmpstr);

                ajListIterDel(&itc);

                }

            ajListIterDel(&itb);
        }

        ajListIterDel(&iter);

        if(thys->Fulldesc->Fragments || thys->Fulldesc->Precursor)
        {
            if(thys->Fulldesc->Fragments == 1)
                ajFmtPrintAppS(&thys->Desc, " (Fragment)");

            if(thys->Fulldesc->Fragments == 2)
                ajFmtPrintAppS(&thys->Desc, " (Fragments)");

            if(thys->Fulldesc->Precursor)
                ajFmtPrintAppS(&thys->Desc, " (Precursor)");
        }
        if(ajStrGetCharFirst(thys->Desc) == ' ')
            ajStrCutStart(&thys->Desc, 1);

        tmpstr = NULL;
    }

    ajSeqSetProt(thys);

    ajSeqreflistGetXrefs(thys->Reflist, &thys->Xreflist);

    ajFilebuffClear(buff, 0);
    ajStrDel(&token);
    ajStrDel(&datestr);
    ajStrDel(&datetype);
    ajStrDel(&relstr);
    ajStrDel(&taxstr);
    ajStrDel(&tmpstr);
    ajStrDel(&genetoken);
    ajStrTokenDel(&handle);

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
    AjBool ok;
/*    AjBool okdate; */
    AjPFilebuff buff;
    AjPStr tmpstr = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr liststr;			/* for lists, do not delete */
    AjPStr datestr = NULL;
    AjPStr relstr = NULL;
    AjPStr cmtstr = NULL;		/* stored in AjPSeq - do not delete */
    ajuint icount;
    AjPSeqRef seqref = NULL;
    AjPSeqXref xref  = NULL;
    ajuint refnum;
    ajuint seqlen=1024;
    ajuint tmplen;
    ajuint itmp;
    ajuint i;
    ajuint taxid = 0;

    buff = seqin->Filebuff;

    if(!seqFtFmtEmbl)
	ajStrAssignC(&seqFtFmtEmbl, "embl");

    if(!ajBuffreadLineStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    /* for GCG formatted databases */

    while(ajStrPrefixC(seqReadLine, "WP "))
    {
	if(!ajBuffreadLineStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;
        seqin->Records++;
    }

    /* extra blank lines */

    while(ajStrIsWhite(seqReadLine))
    {
	if(!ajBuffreadLineStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;
    }

    ajDebug("seqReadEmbl first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "ID   "))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }
    seqin->Records++;

    if(seqin->Text)
	ajStrAssignC(&thys->TextPtr,ajStrGetPtr(seqReadLine));

    ajDebug("seqReadEmbl ID line found\n");
    ajStrTokenAssignC(&handle, seqReadLine, " ;\t\n\r");
    ajStrTokenNextParse(&handle, &token);	/* 'ID' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName(thys, token);

    ajStrTokenNextParse(&handle, &token);	/* SV for new syntax */

    if(ajStrMatchC(token, "SV"))	/* new post-2006 EMBL line */
    {
	ajStrTokenNextParse(&handle, &token);	/* SV */
	ajStrInsertK(&token, 0, '.');
	ajStrInsertS(&token, 0, thys->Name);
	seqSvSave(thys, token);

	ajStrTokenNextParse(&handle, &token); /* linear or circular */

	if(ajStrMatchC(token, "circular"))
	    thys->Circular = ajTrue;

	ajStrTokenNextParseC(&handle, ";\t\n\r", &token);
	ajStrTrimWhite(&token);
	ajSeqmolSetEmbl(&thys->Molecule, token);

	ajStrTokenNextParse(&handle, &token);
	ajStrTrimWhite(&token);
	ajStrAssignS(&thys->Class, token);

	ajStrTokenNextParse(&handle, &token);
	ajStrTrimWhite(&token);
	ajStrAssignS(&thys->Division, token);

	ajStrTokenNextParse(&handle, &token);
	ajStrTrimEndC(&token, "BP.");
	ajStrTrimWhite(&token);
	ajStrToUint(token, &seqlen);
    }
    else		     /* test for a SwissProt/SpTrEMBL entry */
    {
	if(ajStrFindC(seqReadLine, " PRT; ")>= 0  ||
	   ajStrFindC(seqReadLine, " Unreviewed; ") >= 0 ||
	   ajStrFindC(seqReadLine, " Reviewed; ") >= 0 ||
	   ajStrFindC(seqReadLine, " Preliminary; ") >= 0 
	   )
	{
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	    ajStrTokenDel(&handle);
	    ajStrDel(&token);

	    return ajFalse;
	}
    }

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    while(ok && !ajStrPrefixC(seqReadLine, "SQ"))
    {
	seqin->Records++;

	/* check for Staden Experiment format instead */
	if(ajStrPrefixC(seqReadLine, "EN   ") ||
	   ajStrPrefixC(seqReadLine, "TN   ") ||
	   ajStrPrefixC(seqReadLine, "EX   ") )
	{
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	    ajStrDel(&token);

	    return ajFalse;;
	}

	else if(ajStrPrefixC(seqReadLine, "FH   ") ||
		ajStrPrefixC(seqReadLine, "AH   "))
	    ok = ajTrue;		/* ignore these lines */

	else if(ajStrPrefixC(seqReadLine, "AC   ") ||
	   ajStrPrefixC(seqReadLine, "PA   ") ) /* emblcds database format */
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'AC' */

	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	else if(ajStrPrefixC(seqReadLine, "SV   ") ||
	   ajStrPrefixC(seqReadLine, "IV   ") ) /* emblcds database format */
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'SV' */
	    ajStrTokenNextParse(&handle, &token); /* version */
	    seqSvSave(thys, token);
	}

	else if(ajStrPrefixC(seqReadLine, "DE   "))
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
		ajSeqAddKey(thys, liststr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "OS   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OS' */

	    /* maybe better remove . from this, and trim from end */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 1);
		ajStrDel(&tmpstr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "OC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OC' */

	    /* maybe better remove . from this, and trim from end */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 0);
		ajStrDel(&tmpstr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "OG   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OG' */

	    /* maybe better remove . from this, and trim from end */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 2);
		ajStrDel(&tmpstr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "CC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'CC' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* comment */

	    if(ajStrGetLen(cmtstr))
		ajStrAppendC(&cmtstr, "\n");
	    ajStrAppendS(&cmtstr, token);

/* trying to keep comments in one long string with embedded returns
** probably fails for long comments - and also fails for contact details
** which have very short comment lines
** switch to just keeping original lines */

/*
	    if(ajStrGetLen(cmtstr))
	    {
		if(ajStrGetLen(token))
		{
		    if(ajStrGetCharLast(cmtstr) != '\n')
			ajStrAppendK(&cmtstr, ' ');
		    ajStrAppendS(&cmtstr, token);
		}
		else
		{
		    if(ajStrGetCharLast(cmtstr) != '\n')
			ajStrAppendK(&cmtstr, '\n');
		    ajStrAppendC(&cmtstr, " \n");
		}
	    }
	    else
		ajStrAssignS(&cmtstr, token);
	    if(ajStrGetCharLast(token) == '.')
		ajStrAppendK(&cmtstr, '\n');
*/
	}

	else if(ajStrPrefixC(seqReadLine, "DR   "))
	{
            AJNEW0(xref);
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'DR' */
	    ajStrTokenNextParseC(&handle, ";\n\r", &token); /* dbname */
	    ajStrAssignS(&xref->Db, token);
            ajStrTrimWhite(&token);
	    ajStrTokenNextParse(&handle, &token); /* primary */
            ajStrTrimWhite(&token);
	    ajStrAssignS(&xref->Id, token);
	    ajStrTokenNextParse(&handle, &token); /* secondary*/

            if(!ajStrGetLen(token))
            {
                if(ajStrGetCharLast(xref->Id) == '.')
                    ajStrCutEnd(&xref->Id, 1);
            }
            else
            {
                if(ajStrGetCharLast(token) == '.')
                    ajStrCutEnd(&token, 1);
                ajStrTrimWhite(&token);
                ajStrAssignS(&xref->Secid, token);

                ajStrTokenNextParse(&handle, &token); /* secondary*/

                if(!ajStrGetLen(token))
                {
                    if(ajStrGetCharLast(xref->Secid) == '.')
                        ajStrCutEnd(&xref->Secid, 1);
                }
                else
                {
                    if(ajStrGetCharLast(token) == '.')
                        ajStrCutEnd(&token, 1);
                    ajStrTrimWhite(&token);
                    ajStrAssignS(&xref->Terid, token);

                    ajStrTokenNextParse(&handle, &token); /* secondary*/

                    if(!ajStrGetLen(token))
                    {
                        if(ajStrGetCharLast(xref->Terid) == '.')
                            ajStrCutEnd(&xref->Terid, 1);
                    }
                    else
                    {
                        if(ajStrGetCharLast(token) == '.')
                            ajStrCutEnd(&token, 1);
                        ajStrTrimWhite(&token);
                        ajStrAssignS(&xref->Quatid, token);
                    }
                }
            }
            xref->Type = XREF_DR;
	    ajSeqAddXref(thys, xref);
	}

	else if(ajStrPrefixC(seqReadLine, "RN   "))
	{
	    seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RN' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* [num] */
	    ajStrAssignSubS(&tmpstr, token, 1, -2);
	    ajStrToUint(tmpstr, &refnum);
	    ajSeqrefSetnumNumber(seqref, refnum);
	}

	else if(ajStrPrefixC(seqReadLine, "RG   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RG' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* groupname */
	    ajSeqrefAppendGroupname(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RX   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RX' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* xref */
	    ajSeqrefAppendXref(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RP   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RP' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* position */
	    ajSeqrefAppendPosition(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RA   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RA' */
	    ajStrTokenNextParseC(&handle, "\n\r;", &token); /* authors */
	    ajSeqrefAppendAuthors(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RT   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RT' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* title */

	    if(!ajStrMatchC(token, ";"))
		ajSeqrefAppendTitle(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RL   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RL' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* authors */
	    ajSeqrefAppendLocation(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RC   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RC' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* comment */
	    ajSeqrefAppendComment(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "FT   "))
	{
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

		ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		/* ajDebug("EMBL FEAT saved line:\n%S", seqReadLine); */
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "DT   "))
	{
	    if(!thys->Date)
		thys->Date = ajSeqdateNew();

	    ajStrTokenAssignC(&handle, seqReadLine, " (),");
	    icount = 0;

	    while(ajStrTokenNextParse(&handle, &token))
	    {
		icount++;

		if(icount==2)
		    ajStrAssignS(&datestr, token);
		else if(icount==4)
		    ajStrAssignS(&relstr, token);
		else if(icount==5)
		{
		    if(ajStrMatchC(token, "Created"))
		    {
			ajSeqdateSetCreateS(thys->Date, datestr);
			ajStrAssignS(&thys->Date->CreRel, relstr);
		    }
		}
		else if(icount==8)
		{
			ajSeqdateSetModifyS(thys->Date, datestr);
			ajStrAssignS(&thys->Date->ModRel, relstr);
			ajStrAssignS(&thys->Date->ModVer, token);
		}
	    }
	}


	else if(ajStrPrefixC(seqReadLine, "XX"))
	{
	    if(seqref)
	    {
                ajSeqrefStandard(seqref);
		ajSeqAddRef(thys, seqref);
		seqref = NULL;
	    }
	    if(ajStrGetLen(cmtstr))
	    {
                ajSeqAddCmt(thys, cmtstr);
		cmtstr = NULL;
	    }

	}

/* ignored line types */

/* other line types */
/*
	if(ajStrPrefixC(seqReadLine, "RN   "))
	if(ajStrPrefixC(seqReadLine, "RC   "))
	if(ajStrPrefixC(seqReadLine, "RP   "))
	if(ajStrPrefixC(seqReadLine, "RX   "))
	if(ajStrPrefixC(seqReadLine, "RG   "))
	if(ajStrPrefixC(seqReadLine, "RA   "))
	if(ajStrPrefixC(seqReadLine, "RT   "))
	if(ajStrPrefixC(seqReadLine, "RL   "))
	if(ajStrPrefixC(seqReadLine, "AS   "))
	if(ajStrPrefixC(seqReadLine, "CO   "))
	if(ajStrPrefixC(seqReadLine, "CC   "))
*/

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    /* now we are on the SQ line - or there was nothing */

    ajStrTokenAssignC(&handle, seqReadLine, " ");
    ajStrTokenNextParse(&handle, &token); /* 'SQ' */
    ajStrTokenNextParse(&handle, &token); /* 'Sequence' */
    ajStrTokenNextParse(&handle, &token); /* len */
    ajStrToUint(token, &tmplen);

    if(tmplen > seqlen)
	seqlen = tmplen;

    ajStrTokenNextParse(&handle, &token); /* BP; */
    tmplen = 0;

    for(i=0;i<4;i++)
    {
	ajStrTokenNextParse(&handle, &token); /* count */
	ajStrToUint(token, &itmp);
	ajStrTokenNextParse(&handle, &token); /* 'A' 'C' 'G' 'T' 'other' */
	tmplen += itmp;
    }

    if(tmplen > seqlen)
	seqlen = tmplen;

    if(dofeat)
    {
	/* ajDebug("EMBL FEAT TabIn %x\n", seqin->Ftquery); */
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeattableNewRead(seqin->Ftquery);
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
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	ajStrSetRes(&thys->Seq, seqlen+1);

	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    seqin->Records++;
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	}
    }

    if(!ajSeqIsNuc(thys))
    {
        ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
        ajStrDel(&tmpstr);
        ajStrDel(&token);
        ajStrDel(&datestr);
        ajStrDel(&relstr);
        ajStrTokenDel(&handle);

        return ajFalse;
    }
    
    ajSeqSetNuc(thys);

    if(ajFeattableGetSize(thys->Fttable))
    {
        ajFeattableGetXrefs(thys->Fttable, &thys->Xreflist, &taxid);
        if(taxid)
            seqTaxidSaveI(thys, taxid);
    }
    
    ajSeqreflistGetXrefs(thys->Reflist, &thys->Xreflist);

    if(!taxid)
        taxid = ajSeqGetTaxid(thys);

    ajFilebuffClear(buff, 0);

    ajStrDel(&tmpstr);
    ajStrDel(&token);
    ajStrDel(&datestr);
    ajStrDel(&relstr);

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

    AjPStrTok handle  = NULL;
    AjPStrTok handle2 = NULL;
    AjPStr token      = NULL;
    AjPStr token2     = NULL;
    AjBool ok;
    AjPFilebuff buff;
    AjPStr tmpstr = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr liststr;			/* for lists, do not delete */
    AjPStr accvalstr = NULL;
    ajuint i;
    ajint  ja;
    ajuint ilen;

    buff = seqin->Filebuff;

    if(!seqFtFmtEmbl)
	ajStrAssignC(&seqFtFmtEmbl, "embl");

    if(!ajBuffreadLineStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    seqin->Records++;

    ajDebug("seqReadExperiment first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "ID   "))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(seqin->Text)
	ajStrAssignC(&thys->TextPtr,ajStrGetPtr(seqReadLine));

    ajDebug("seqReadExperiment ID line found\n");
    ajStrTokenAssignC(&handle, seqReadLine, " \n\r\t");
    ajStrTokenNextParse(&handle, &token);	/* 'ID' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName(thys, token);

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    while(ok && !ajStrPrefixC(seqReadLine, "SQ"))
    {
	seqin->Records++;

	if(ajStrPrefixC(seqReadLine, "EX   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'EX'*/
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /*  expt. desc. */

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
		ajSeqAddKey(thys, liststr);
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
		seqTaxSave(thys, tmpstr, 1);
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
		seqTaxSave(thys, tmpstr, 0);
	    }
	}

	ajStrDel(&tmpstr);

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
		ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		/* ajDebug("EMBL FEAT saved line:\n%S", seqReadLine); */
	    }

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    while(ok && !ajStrPrefixC(seqReadLine, "//"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	seqin->Records++;
	ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				&thys->TextPtr);
    }
    ajDebug("Sequence read %d bases\n", ajStrGetLen(thys->Seq));

    while(ok && !ajStrPrefixC(seqReadLine, "ID   "))
    {
	seqin->Records++;
	ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				&thys->TextPtr);
    }

    if(ok)
	ajFilebuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    else
	ajFilebuffClear(buff, 0);

    if(dofeat)
    {
	/* ajDebug("EMBL FEAT TabIn %x\n", seqin->Ftquery); */
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeattableNewRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    if(ajStrGetLen(accvalstr))
    {
	ilen = ajStrGetLen(thys->Seq);
        if(ilen > thys->Qualsize)
        {
            AJCRESIZE(thys->Accuracy, ilen);
            thys->Qualsize = ilen;
        }

	ajStrTokenAssignC(&handle, accvalstr, " ");

	for(i=0;i<ilen;i++)
	{
            thys->Accuracy[i] = INT_MIN;
	    if(!ajStrTokenNextParse(&handle, &token))
	    {
		ajWarn("Missing accuracy for base %d in experiment format\n",
		       i+1);
		break;
	    }

	    ajStrTokenAssignC(&handle2, token, ",");

	    while(ajStrTokenNextParse(&handle2, &token2))
	    {
		if(ajStrToInt(token2, &ja))
		{
		    if(ja > thys->Accuracy[i])
			thys->Accuracy[i] = (float) ja;
		}
		else
		{
		    ajWarn("Bad accuracy '%S' for base %d "
			   "in experiment format\n",
			   token, i+1);
		    break;
		}
	    }
	    ajDebug("Accval[%d] %3d '%S'\n", i+1, thys->Accuracy[i], token);
	}
    }

    ajStrDel(&token);
    ajStrDel(&token2);
    ajStrDel(&accvalstr);

    ajStrTokenDel(&handle);
    ajStrTokenDel(&handle2);


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
    AjBool ok;
    AjBool done = ajFalse;
    AjPFilebuff buff;
    AjPStr tmpstr = NULL;
    AjPStr tmpstr2 = NULL;
    AjBool dofeat        = ajFalse;
    AjPSeqQuery qry;
    AjPStr liststr;			/* for lists, do not delete */
    AjPSeqRef seqref = NULL;
    ajuint refnum;
    ajuint seqlen = 1024;
    ajint i;
    ajint nfields;
    ajuint taxid = 0;

    ajDebug("seqReadGenbank\n");

    buff = seqin->Filebuff;
    qry  = seqin->Query;

    if(!seqFtFmtGenbank)
	ajStrAssignC(&seqFtFmtGenbank, "genbank");

    if(!ajBuffreadLine(buff, &seqReadLine))
	return ajFalse;

    ajDebug("++seqReadGenbank first line '%S'\n", seqReadLine);

    ok = ajTrue;

    /* extra blank lines */

    while(ajStrIsWhite(seqReadLine))
    {
	if(!ajBuffreadLineStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;
    }

    /* for GCG formatted databases */

    if(ajStrPrefixC(seqReadLine, "WPCOMMENT"))
    {
	ok = ajBuffreadLine(buff, &seqReadLine);
	seqin->Records++;

	while(ok && ajStrPrefixC(seqReadLine, " "))
	{
	    ok = ajBuffreadLine(buff, &seqReadLine);
	    seqin->Records++;
	}
    }

    /* This loop necessary owing to headers on GB distro files */
    if(ajStrFindC(seqReadLine,"Genetic Sequence Data Bank") >= 0)
	while(ok && !ajStrPrefixC(seqReadLine, "LOCUS"))
	{
	    ok = ajBuffreadLine(buff, &seqReadLine);
	    seqin->Records++;
	}

    if(!ok)
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(!ajStrPrefixC(seqReadLine, "LOCUS"))
    {
	ajDebug("failed - LOCUS not found - first line was\n%S\n",
		seqReadLine);
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }
    seqin->Records++;

    nfields = ajStrParseCountC(seqReadLine, " \n\r");

    if(nfields == 9) 
    {
        ajFilebuffSetBuffered(buff);
        ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

        return seqReadGenpept(thys,seqin);
    }
    
    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    i=0;

    while(ajStrTokenNextParse(&handle, &token))
    {
	switch(++i)
	{
	case 1:
	    break;
	case 2:
	    seqSetName(thys, token);
	    break;
	case 3:
	    ajStrToUint(token, &seqlen);
	    break;
	case 4:
	    if(ajStrMatchC(token, "aa"))
            {
                ajFilebuffSetBuffered(buff);
                ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
                ajStrDel(&token);
                ajStrTokenDel(&handle);

                ajDebug("first line %d aa pass to refseqp '%S'\n",
                        buff->Pos, seqReadLine);
                return seqReadRefseqp(thys,seqin);
            }
	    if(!ajStrMatchC(token, "bp"))
		ajWarn("bad Genbank LOCUS line '%S'", seqReadLine);
	    break;
	case 5:
	    ajSeqmolSetGb(&thys->Molecule, token);
	    break;
	case 6:
	    if(ajStrMatchC(token, "circular"))
		thys->Circular = ajTrue;
	    break;
	case 7:
	    ajSeqdivSetGb(&thys->Division, token);
	    ajSeqclsSetGb(&thys->Class, token);
	    break;
	case 8:
	    if(!thys->Date)
		thys->Date = ajSeqdateNew();
	    ajSeqdateSetModifyS(thys->Date, token);
	    break;
	default:
	    break;
	}
    }

    if(seqin->Text)
	ajStrAssignC(&thys->TextPtr,ajStrGetPtr(seqReadLine));

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    while(ok &&
	  !ajStrPrefixC(seqReadLine, "//") &&
	  !ajStrPrefixC(seqReadLine, "ORIGIN") &&
	  !ajStrPrefixC(seqReadLine, "BASE COUNT"))
    {
	done = ajFalse;
	seqin->Records++;

	if(ajStrPrefixC(seqReadLine, "DEFINITION"))
	{
	    ajDebug("definition found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DEFINITION' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    ajStrAssignS(&thys->Desc, token);
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		seqin->Records++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");
		ajStrTokenNextParseC(&handle, "\n\r", &token);
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "ACCESSION"))
	{
	    ajDebug("accession found\n");

	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'ACCESSION' */

	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	else if(ajStrPrefixC(seqReadLine, "VERSION"))
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

	else if(ajStrPrefixC(seqReadLine, "SOURCE"))
	{
	    ajDebug("source found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'SOURCE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* source */
	    ajStrAssignS(&thys->Tax, token);
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		seqin->Records++;
/* process organism lines */
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "FEATURES"))
	{
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

		ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		done = ajTrue;

		while(ok && ajStrPrefixC(seqReadLine, " "))
		{
		    seqin->Records++;
		    ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		    /* ajDebug("GENBANK FEAT saved line:\n%S", seqReadLine); */
		    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					    &thys->TextPtr);
		}
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "REFERENCE"))
	{
            ajDebug("reference found\n");
	    seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'REFERENCE' */
	    ajStrTokenNextParse(&handle, &token); /* number */
	    ajStrToUint(token, &refnum);
	    ajSeqrefSetnumNumber(seqref, refnum);
	    ajStrAssignClear(&tmpstr2);

	    while (ajStrTokenNextParse(&handle, &token))
	    {
		if(ajStrMatchC(token, "(bases"))
                    continue;

		if(ajStrMatchC(token, "to"))
                    continue;

		if(!ajStrGetLen(tmpstr2))
		    ajStrAssignS(&tmpstr2, token);

		if(ajStrSuffixC(token, ")"))
		{
		    ajStrTrimEndC(&token, ")");
		    ajStrAppendK(&tmpstr2, '-');
		    ajStrAppendS(&tmpstr2, token);
		}
	    }

	    ajSeqrefSetPosition(seqref, tmpstr2);

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
		seqin->Records++;
	    }

	    ajSeqrefStandard(seqref);
	    ajSeqAddRef(thys, seqref);
	    seqref = NULL;
	}

	else if(ajStrPrefixC(seqReadLine, "KEYWORDS"))
	{
            ajDebug("keywords found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'KEYWORDS' */

	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		ajSeqAddKey(thys, liststr);
	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		seqin->Records++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");

		while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
		{
		    liststr = ajStrNewS(token);
		    ajStrTrimWhite(&liststr);
		    ajSeqAddKey(thys, liststr);
		}

		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "  ORGANISM"))
	{
	    ajDebug("organism found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'ORGANISM' */

	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 0);
	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, "    "))
	    {
		seqin->Records++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");

		while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
		{
		    ajStrAssignS(&tmpstr, token);
		    ajStrTrimWhite(&tmpstr);
		    seqTaxSave(thys, tmpstr, 0);
		}

		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	if(!done)
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
    }

    if(dofeat)
    {
	ajDebug("GENBANK FEAT TabIn %x\n", seqin->Ftquery);
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeattableNewRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    if(ajStrGetLen(seqin->Inseq))
    {
	/* we have a sequence to use */
	ajDebug("Got an Inseq sequence\n");

	if(ajStrMatchC(qry->Method,"gcg"))
	    while(ok && !ajStrPrefixC(seqReadLine,"ORIGIN"))
		ok = ajBuffreadLineStore(buff,&seqReadLine, seqin->Text,
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

	while(ok &&
              !ajStrPrefixC(seqReadLine,"//") &&
	      !ajStrPrefixC(seqReadLine,"ORIGIN") &&
              !ajStrPrefixC(seqReadLine,"BASE COUNT"))
        {
            ok = ajBuffreadLineStore(buff,&seqReadLine,
                                     seqin->Text, &thys->TextPtr);
            if(!ok)
                break;
        }

        if(ok && !ajStrPrefixC(seqReadLine,"//"))
            ok = ajBuffreadLineStore(buff, &seqReadLine,
                                     seqin->Text, &thys->TextPtr);

	ajStrSetRes(&thys->Seq, seqlen+1);

	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    if(!ajStrPrefixC(seqReadLine, "ORIGIN") &&
	       !ajStrPrefixC(seqReadLine,"BASE COUNT"))
		seqAppend(&thys->Seq, seqReadLine);

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    seqin->Records++;
	}
    }

    if(!ajStrMatchC(qry->Method,"gcg"))
	while(ok && !ajStrPrefixC(seqReadLine,"//"))
	    ok = ajBuffreadLineStore(buff,&seqReadLine,
				    seqin->Text, &thys->TextPtr);


    if(ajFeattableGetSize(thys->Fttable))
    {
        ajFeattableGetXrefs(thys->Fttable, &thys->Xreflist, &taxid);
        if(taxid)
            seqTaxidSaveI(thys, taxid);
    }
    
    if(!taxid)
        taxid = ajSeqGetTaxid(thys);

    ajFilebuffClear(buff, 0);

    ajStrTokenDel(&handle);
    ajStrDel(&token);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpstr2);

    return ajTrue;
}




/* @funcstatic seqReadRefseq *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using Refseq format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadRefseq(AjPSeq thys, AjPSeqin seqin)
{
    return seqReadGenbank(thys, seqin);
}




/* @funcstatic seqReadGenpept *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using Genpept format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGenpept(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjBool ok;
    AjBool done = ajFalse;
    AjPFilebuff buff;
    AjPStr tmpstr = NULL;
    AjPStr tmpstr2 = NULL;
    AjBool dofeat        = ajFalse;
    AjPSeqQuery qry;
    AjPStr liststr;			/* for lists, do not delete */
    AjPSeqRef seqref = NULL;
    ajuint refnum;
    ajuint seqlen = 1024;
    ajint i;
    ajint nfields;

    ajDebug("seqReadGenpept\n");

    buff = seqin->Filebuff;
    qry  = seqin->Query;

    if(!ajBuffreadLine(buff, &seqReadLine))
	return ajFalse;

    seqin->Records++;

    ok = ajTrue;

    /* extra blank lines */

    while(ajStrIsWhite(seqReadLine))
    {
	if(!ajBuffreadLineStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;

	seqin->Records++;
    }

    /* for GCG formatted databases */

    if(ajStrPrefixC(seqReadLine, "WPCOMMENT"))
    {
	ok = ajBuffreadLine(buff, &seqReadLine);
	seqin->Records++;

	while(ok && ajStrPrefixC(seqReadLine, " "))
	{
	    ok = ajBuffreadLine(buff, &seqReadLine);
	    seqin->Records++;
	}
    }

    /* This loop necessary owing to headers on GB distro files */
    if(ajStrFindC(seqReadLine,"Genetic Sequence Data Bank") >= 0)
    {
	while(ok && !ajStrPrefixC(seqReadLine, "LOCUS"))
	{
	    ok = ajBuffreadLine(buff, &seqReadLine);
	    seqin->Records++;
	}
    }

    if(!ok)
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(!ajStrPrefixC(seqReadLine, "LOCUS"))
    {
	ajDebug("failed - LOCUS not found - first line was\n%S\n",
		seqReadLine);
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    nfields = ajStrParseCountC(seqReadLine, " \n\r");

    if(nfields == 8) 
    {
        ajFilebuffSetBuffered(buff);
        ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

        return seqReadRefseqp(thys,seqin);
    }
    
    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    i=0;

    while(ajStrTokenNextParse(&handle, &token))
    {
	switch(++i)
	{
	case 1:
	    break;
	case 2:
	    seqSetName(thys, token);
	    break;
	case 3:                 /* genbank locus name */
	    break;
	case 4:
	    ajStrToUint(token, &seqlen);
	    break;
	case 5:
	    if(!ajStrMatchC(token, "aa"))
		ajWarn("bad Genpept LOCUS line '%S'", seqReadLine);
	    break;
	case 6:
	    ajSeqmolSetGb(&thys->Molecule, token);
	    break;
	case 7:
	    if(ajStrMatchC(token, "circular"))
		thys->Circular = ajTrue;
	    break;
	case 8:
	    ajSeqdivSetGb(&thys->Division, token);
	    ajSeqclsSetGb(&thys->Class, token);
	    break;
	case 9:
	    if(!thys->Date)
		thys->Date = ajSeqdateNew();
	    ajSeqdateSetModifyS(thys->Date, token);
	    break;
	default:
	    break;
	}
    }

    if(seqin->Text)
	ajStrAssignC(&thys->TextPtr,ajStrGetPtr(seqReadLine));

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    while(ok &&
	  !ajStrPrefixC(seqReadLine, "ORIGIN") &&
	  !ajStrPrefixC(seqReadLine, "BASE COUNT"))
    {
	done = ajFalse;
	seqin->Records++;

	if(ajStrPrefixC(seqReadLine, "DEFINITION"))
	{
	    ajDebug("definition found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DEFINITION' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    ajStrAssignS(&thys->Desc, token);
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		seqin->Records++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");
		ajStrTokenNextParseC(&handle, "\n\r", &token);
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "ACCESSION"))
	{
	    ajDebug("accession found\n");

	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'ACCESSION' */

	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	else if(ajStrPrefixC(seqReadLine, "VERSION"))
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

	else if(ajStrPrefixC(seqReadLine, "SOURCE"))
	{
	    ajDebug("source found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'SOURCE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* source */
	    ajStrAssignS(&thys->Tax, token);
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		seqin->Records++;
/* process organism lines */
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "REFERENCE"))
	{
            ajDebug("reference found\n");
	    seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'REFERENCE' */
	    ajStrTokenNextParse(&handle, &token); /* number */
	    ajStrToUint(token, &refnum);
	    ajSeqrefSetnumNumber(seqref, refnum);
	    ajStrAssignClear(&tmpstr2);

	    while (ajStrTokenNextParse(&handle, &token))
	    {
		if(ajStrMatchC(token, "(bases"))
                    continue;

		if(ajStrMatchC(token, "to"))
                    continue;

		if(!ajStrGetLen(tmpstr2))
		    ajStrAssignS(&tmpstr2, token);

		if(ajStrSuffixC(token, ")"))
		{
		    ajStrTrimEndC(&token, ")");
		    ajStrAppendK(&tmpstr2, '-');
		    ajStrAppendS(&tmpstr2, token);
		}
	    }

	    ajSeqrefSetPosition(seqref, tmpstr2);

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
		seqin->Records++;
	    }

	    ajSeqrefStandard(seqref);
	    ajSeqAddRef(thys, seqref);
	    seqref = NULL;
	}

	else if(ajStrPrefixC(seqReadLine, "KEYWORDS"))
	{
            ajDebug("keywords found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'KEYWORDS' */

	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		ajSeqAddKey(thys, liststr);
	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		seqin->Records++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");

		while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
		{
		    liststr = ajStrNewS(token);
		    ajStrTrimWhite(&liststr);
		    ajSeqAddKey(thys, liststr);
		}

		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "  ORGANISM"))
	{
	    ajDebug("organism found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'ORGANISM' */

	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 0);
	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, "    "))
	    {
		seqin->Records++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");

		while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
		{
		    ajStrAssignS(&tmpstr, token);
		    ajStrTrimWhite(&tmpstr);
		    seqTaxSave(thys, tmpstr, 0);
		}

		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	if(!done)
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
    }

    if(dofeat)
    {
	ajDebug("GENPEPT FEAT TabIn %x\n", seqin->Ftquery);
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeattableNewRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    if(ajStrGetLen(seqin->Inseq))
    {
	/* we have a sequence to use */
	ajDebug("Got an Inseq sequence\n");

	if(ajStrMatchC(qry->Method,"gcg"))
	    while(ok && !ajStrPrefixC(seqReadLine,"ORIGIN"))
		ok = ajBuffreadLineStore(buff,&seqReadLine, seqin->Text,
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
	    if(!ajBuffreadLineStore(buff,&seqReadLine,
				   seqin->Text, &thys->TextPtr))
		break;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	ajStrSetRes(&thys->Seq, seqlen+1);

	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    if(!ajStrPrefixC(seqReadLine, "ORIGIN") &&
	       !ajStrPrefixC(seqReadLine,"BASE COUNT"))
		seqAppend(&thys->Seq, seqReadLine);
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    seqin->Records++;
	}
    }

    if(!ajStrMatchC(qry->Method,"gcg"))
	while(ok && !ajStrPrefixC(seqReadLine,"//"))
	    ok = ajBuffreadLineStore(buff,&seqReadLine,
				    seqin->Text, &thys->TextPtr);


    ajFilebuffClear(buff, 0);

    ajStrTokenDel(&handle);
    ajStrDel(&token);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpstr2);

    return ajTrue;
}




/* @funcstatic seqReadRefseqp *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using Refseq protein format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadRefseqp(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjBool ok;
    AjBool done = ajFalse;
    AjPFilebuff buff;
    AjPStr tmpstr = NULL;
    AjPStr tmpstr2 = NULL;
    AjBool dofeat        = ajFalse;
    AjPSeqQuery qry;
    AjPStr liststr;			/* for lists, do not delete */
    AjPSeqRef seqref = NULL;
    ajuint refnum;
    ajuint seqlen = 1024;
    ajint i;

    ajDebug("seqReadRefseqp\n");

    buff = seqin->Filebuff;
    qry  = seqin->Query;

    if(!seqFtFmtRefseqp)
	ajStrAssignC(&seqFtFmtRefseqp, "refseqp");

    if(!ajBuffreadLine(buff, &seqReadLine))
	return ajFalse;

    ajDebug("++seqReadRefseqp  %d first line '%S'\n", buff->Pos, seqReadLine);

    seqin->Records++;

    ok = ajTrue;

    /* extra blank lines */

    while(ajStrIsWhite(seqReadLine))
    {
	if(!ajBuffreadLineStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;

	seqin->Records++;
    }

    /* for GCG formatted databases */

    if(ajStrPrefixC(seqReadLine, "WPCOMMENT"))
    {
	ok = ajBuffreadLine(buff, &seqReadLine);
	seqin->Records++;

	while(ok && ajStrPrefixC(seqReadLine, " "))
	{
	    ok = ajBuffreadLine(buff, &seqReadLine);
	    seqin->Records++;
	}
    }

    if(!ok)
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(!ajStrPrefixC(seqReadLine, "LOCUS"))
    {
	ajDebug("failed - LOCUS not found - first line was\n%S\n",
		seqReadLine);
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	return ajFalse;
    }

    if(seqin->Text)
	ajStrAssignC(&thys->TextPtr,ajStrGetPtr(seqReadLine));

    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    i=0;

    while(ajStrTokenNextParse(&handle, &token))
    {
	switch(++i)
	{
            case 1:             /* 'LOCUS' */
                break;
            case 2:             /* locus name */
                seqSetName(thys, token);
                break;
            case 3:             /* length */
                ajStrToUint(token, &seqlen);
                break;
            case 4:             /* 'aa' */
                if(!ajStrMatchC(token, "aa"))
                    ajWarn("bad RefseqP LOCUS line '%S'", seqReadLine);
                break;
            case 5:
                ajSeqdivSetGb(&thys->Division, token);
                ajSeqclsSetGb(&thys->Class, token);
                break;
            case 6:
                if(!thys->Date)
                    thys->Date = ajSeqdateNew();
                ajSeqdateSetModifyS(thys->Date, token);
                break;
            default:
                break;
	}
    }

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    while(ok &&
	  !ajStrPrefixC(seqReadLine, "ORIGIN") &&
	  !ajStrPrefixC(seqReadLine, "BASE COUNT"))
    {
	done = ajFalse;
	seqin->Records++;

	if(ajStrPrefixC(seqReadLine, "DEFINITION"))
	{
	    ajDebug("definition found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DEFINITION' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    ajStrAssignS(&thys->Desc, token);
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		seqin->Records++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");
		ajStrTokenNextParseC(&handle, "\n\r", &token);
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "ACCESSION"))
	{
	    ajDebug("accession found\n");

	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'ACCESSION' */

	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	else if(ajStrPrefixC(seqReadLine, "VERSION"))
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

	else if(ajStrPrefixC(seqReadLine, "SOURCE"))
	{
	    ajDebug("source found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'SOURCE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* source */
	    ajStrAssignS(&thys->Tax, token);
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		seqin->Records++;
/* process organism lines */
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "FEATURES"))
	{
	    if(seqinUfoLocal(seqin))
	    {
		ajDebug("features found\n");

		if(!dofeat)
		{
		    dofeat = ajTrue;
		    ajFeattabInDel(&seqin->Ftquery);
		    seqin->Ftquery = ajFeattabInNewSS(seqFtFmtRefseqp,
						      thys->Name, "N");
		    ajDebug("seqin->Ftquery Handle %x\n",
			    seqin->Ftquery->Handle);
		    /* ajDebug("REFSEQP FEAT first line:\n%S", seqReadLine); */
		}

		ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		done = ajTrue;

		while(ok && ajStrPrefixC(seqReadLine, " "))
		{
		    seqin->Records++;
		    ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		    /* ajDebug("REFSEQP FEAT saved line:\n%S", seqReadLine); */
		    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					    &thys->TextPtr);
		}
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "REFERENCE"))
	{
	    ajDebug("reference found\n");
	    seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'REFERENCE' */
	    ajStrTokenNextParse(&handle, &token); /* number */
	    ajStrToUint(token, &refnum);
	    ajSeqrefSetnumNumber(seqref, refnum);
	    ajStrAssignClear(&tmpstr2);

	    while (ajStrTokenNextParse(&handle, &token))
	    {
		if(ajStrMatchC(token, "(residues"))
                    continue;

		if(ajStrMatchC(token, "to"))
                    continue;

		if(!ajStrGetLen(tmpstr2))
		    ajStrAssignS(&tmpstr2, token);

		if(ajStrSuffixC(token, ")"))
		{
		    ajStrTrimEndC(&token, ")");
		    ajStrAppendK(&tmpstr2, '-');
		    ajStrAppendS(&tmpstr2, token);
		}
	    }

	    ajSeqrefSetPosition(seqref, tmpstr2);

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
		seqin->Records++;
	    }

	    ajSeqrefStandard(seqref);
	    ajSeqAddRef(thys, seqref);
	    seqref = NULL;
	}

	else if(ajStrPrefixC(seqReadLine, "KEYWORDS"))
	{
	    ajDebug("keywords found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'KEYWORDS' */

	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		ajSeqAddKey(thys, liststr);
	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		seqin->Records++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");

		while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
		{
		    liststr = ajStrNewS(token);
		    ajStrTrimWhite(&liststr);
		    ajSeqAddKey(thys, liststr);
		}

		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "  ORGANISM"))
	{
	    ajDebug("organism found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'ORGANISM' */

	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 0);
	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;

	    while(ok && ajStrPrefixC(seqReadLine, "    "))
	    {
		seqin->Records++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");

		while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
		{
		    ajStrAssignS(&tmpstr, token);
		    ajStrTrimWhite(&tmpstr);
		    seqTaxSave(thys, tmpstr, 0);
		}

		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	if(!done)
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
    }

    if(dofeat)
    {
	ajDebug("REFSEQP FEAT TabIn %x\n", seqin->Ftquery);
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeattableNewRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    if(ajStrGetLen(seqin->Inseq))
    {
	/* we have a sequence to use */
	ajDebug("Got an Inseq sequence\n");

	if(ajStrMatchC(qry->Method,"gcg"))
	    while(ok && !ajStrPrefixC(seqReadLine,"ORIGIN"))
		ok = ajBuffreadLineStore(buff,&seqReadLine, seqin->Text,
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
	    if(!ajBuffreadLineStore(buff,&seqReadLine,
				   seqin->Text, &thys->TextPtr))
		break;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	ajStrSetRes(&thys->Seq, seqlen+1);

	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    if(!ajStrPrefixC(seqReadLine, "ORIGIN") &&
	       !ajStrPrefixC(seqReadLine,"BASE COUNT"))
		seqAppend(&thys->Seq, seqReadLine);

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    seqin->Records++;
	}
    }

    if(!ajStrMatchC(qry->Method,"gcg"))
	while(ok && !ajStrPrefixC(seqReadLine,"//"))
	    ok = ajBuffreadLineStore(buff,&seqReadLine,
				    seqin->Text, &thys->TextPtr);

    ajFilebuffClear(buff, 0);
    ajDebug("++last line %d '%S'\n", buff->Pos, seqReadLine);

    ajStrTokenDel(&handle);
    ajStrDel(&token);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpstr2);

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
    AjBool ok;
    AjBool isseq            = ajFalse;
    AjPFilebuff buff;
    AjPFilebuff ftfile   = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr typstr = NULL;
    AjPStr verstr = NULL;	/* copy of version line */
    AjPStr outstr = NULL;	/* generated Type line */

    buff = seqin->Filebuff;

    if(!seqRegGffTyp)
	seqRegGffTyp = ajRegCompC("^##([DR]NA|Protein) +([^ \t\r\n]+)");

    if(!seqFtFmtGff)
	ajStrAssignC(&seqFtFmtGff, "gff");

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    seqin->Records++;

    ajDebug("seqReadGff first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "##gff-version "))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    ajStrAssignS(&verstr, seqReadLine);

    if(seqin->Text)
	ajStrAssignS(&thys->TextPtr,seqReadLine);

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    while(ok && ajStrPrefixC(seqReadLine, "##"))
    {
	if(ajRegExec(seqRegGffTyp, seqReadLine))
	{
	    isseq = ajTrue;
	    ajRegSubI(seqRegGffTyp, 1, &typstr);
	    ajRegSubI(seqRegGffTyp, 2, &thys->Name);
	    ajFmtPrintS(&outstr, "##Type %S %S", typstr, thys->Name);
	}
	else if(ajStrPrefixC(seqReadLine, "##end-"))
	    isseq = ajFalse;
	else if(isseq)
	    seqAppend(&thys->Seq, seqReadLine);

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    if(!ajSeqGetLen(thys))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	return ajFalse;
    }

    /* do we want the features now? */

    if(ok & seqinUfoLocal(seqin))
    {
	dofeat = ajTrue;
	ftfile = ajFilebuffNewNofile();
	ajFilebuffLoadS(ftfile, verstr);
	ajFilebuffLoadS(ftfile, outstr);

	while(ok && !ajStrPrefixC(seqReadLine, "##"))
	{
	    ajFilebuffLoadS(ftfile, seqReadLine);
	    /* ajDebug("GFF FEAT saved line:\n%S", seqReadLine); */
	    ok = ajBuffreadLineStore(buff,&seqReadLine,seqin->Text,
				    &thys->TextPtr);
	}
    }

    if(dofeat)
    {
	ajFeattabInDel(&seqin->Ftquery);
	seqin->Ftquery = ajFeattabInNewSSF(seqFtFmtGff, thys->Name,
					   ajStrGetPtr(seqin->Type), ftfile);
	ajDebug("GFF FEAT TabIn %x type: '%S'\n",
                seqin->Ftquery, seqin->Type);
	ftfile = NULL;		  /* now copied to seqin->FeattabIn */
	ajFeattableDel(&seqin->Fttable);
	seqin->Fttable = ajFeattableNewRead(seqin->Ftquery);
	/* ajFeattableTrace(seqin->Fttable); */
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = seqin->Fttable;
	seqin->Fttable = NULL;
    }

    if(ajStrMatchC(typstr, "Protein"))
      ajSeqSetProt(thys);
    else if(ajSeqIsNuc(thys))
      ajSeqSetNuc(thys);
    else
      ajSeqSetProt(thys);


    ajFilebuffClear(buff, 0);

    ajStrDel(&typstr);
    ajStrDel(&verstr);
    ajStrDel(&outstr);

    return ajTrue;
}




/* @funcstatic seqReadGff3 ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using GFF3 format.
**
** GFF3 is far stricter than GFF2 but does include a sequence in FASTA format
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

static AjBool seqReadGff3(AjPSeq thys, AjPSeqin seqin)
{
    AjBool ok;
    AjPFilebuff buff;
    AjPFilebuff ftfile   = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr verstr = NULL;	/* copy of version line */
    AjPStr outstr = NULL;	/* generated Type line */
    AjPStr typstr = NULL;

    buff = seqin->Filebuff;

    if(!seqFtFmtGff)
	ajStrAssignC(&seqFtFmtGff, "gff3");

    if(!seqRegGff3Typ)
	seqRegGff3Typ = ajRegCompC("^#([DR]NA|Protein) +([^ \t\r\n]+)");

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    seqin->Records++;

    ajDebug("seqReadGff3 first line '%S'\n", seqReadLine);

    ajStrRemoveWhiteExcess(&seqReadLine);

    if(!ajStrMatchC(seqReadLine, "##gff-version 3"))
    {
	ajDebug("bad gff3 version line '%S'\n", seqReadLine);
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    ajStrAssignS(&verstr, seqReadLine);

    if(seqin->Text)
	ajStrAssignS(&thys->TextPtr,seqReadLine);

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    while(ok && ajStrPrefixC(seqReadLine, "#"))
    {
	if(ajStrPrefixC(seqReadLine, "##sequence-region"))
	{
	}
	else if(ajStrPrefixC(seqReadLine, "##feature-ontology"))
	{
	}
	else if(ajStrPrefixC(seqReadLine, "##attribute-ontology"))
	{
	}
	else if(ajStrPrefixC(seqReadLine, "##source-ontology"))
	{
	}
	else if(ajStrPrefixC(seqReadLine, "###"))
	{
	}
	else if(ajStrPrefixC(seqReadLine, "##FASTA"))
	{
	    break;
	}
	else if(ajStrPrefixC(seqReadLine, "##"))
	{
	    ajWarn("GFF3: Unrecognized header directive '%S'",
		   seqReadLine);
	}

	if(ajRegExec(seqRegGff3Typ, seqReadLine))
	{
	    ajRegSubI(seqRegGff3Typ, 1, &typstr);
	    ajRegSubI(seqRegGff3Typ, 2, &thys->Name);
	    ajFmtPrintS(&outstr, "#!Type %S %S", typstr, thys->Name);
	}

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    /* do we want the features now? */

    if(ok & seqinUfoLocal(seqin))
    {
	dofeat = ajTrue;

	ftfile = ajFilebuffNewNofile();
	ajFilebuffLoadS(ftfile, verstr);
	ajFilebuffLoadS(ftfile, outstr);
    }

    while(ok && !ajStrPrefixC(seqReadLine, "##"))
    {
	if(dofeat)
	    ajFilebuffLoadS(ftfile, seqReadLine);
	/* ajDebug("GFF FEAT saved line:\n%S", seqReadLine); */
	ok = ajBuffreadLineStore(buff,&seqReadLine,seqin->Text,
				&thys->TextPtr);
    }

    if(!ajStrPrefixC(seqReadLine, "##FASTA")) /* no sequence at end */
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }
	
    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(ok)
    {
	if(ajStrPrefixC(seqReadLine, ">"))
	{
	    ajStrCutStart(&seqReadLine, 1);
	    ajStrExtractFirst(seqReadLine, &thys->Desc, &thys->Name);
	}

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    while(ok && !ajStrPrefixC(seqReadLine, "##"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    if(!ajSeqGetLen(thys))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

	return ajFalse;
    }

    if(dofeat)
    {
	ajFeattabInDel(&seqin->Ftquery);
	seqin->Ftquery = ajFeattabInNewSSF(seqFtFmtGff, thys->Name,
					   ajStrGetPtr(seqin->Type), ftfile);
	ajDebug("GFF FEAT TabIn %x\n", seqin->Ftquery);
	ftfile = NULL;		  /* now copied to seqin->FeattabIn */
	ajFeattableDel(&seqin->Fttable);
	seqin->Fttable = ajFeattableNewRead(seqin->Ftquery);
	/* ajFeattableTrace(seqin->Fttable); */
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = seqin->Fttable;
	seqin->Fttable = NULL;
    }


    if(ajStrMatchC(typstr, "Protein"))
      ajSeqSetProt(thys);
    else if(ajSeqIsNuc(thys))
      ajSeqSetNuc(thys);
    else
      ajSeqSetProt(thys);

    ajFilebuffClear(buff, 0);

    ajStrDel(&typstr);
    ajStrDel(&verstr);
    ajStrDel(&outstr);

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
    AjPFilebuff buff;
    AjBool  ok      = ajFalse;
    ajlong baseO    = 0L;
    ajlong pconO    = 0L;
    ajlong numBases = 0L;
    AjPStr sample   = NULL;
    AjPStr smpl     = NULL;
    AjPFile fp;
    ajint filestat;

    buff = seqin->Filebuff;
    fp = ajFilebuffGetFile(buff);

    ajDebug("seqReadAbi file %F\n", fp);

    /* ajFilebuffTraceFull(buff, 10, 10); */

    if(ajFilebuffIsEnded(buff))
	return ajFalse;

    if(!ajSeqABITest(fp))
    {
        ajDebug("seqReadAbi ajSeqABITest failed on %F\n", fp);
	ajFilebuffResetPos(buff);

	return ajFalse;
    }

    if(seqin->Text)
	ajWarn("Failed to read text from binary ABI file %F", fp);

    filestat = ajFileSeek(fp,0L,0);
    ajDebug("filestat %d\n", filestat);

    numBases = ajSeqABIGetNBase(fp);

    ok = ajFalse;

    /* Find BASE tag & get offset                    */
    baseO = ajSeqABIGetBaseOffset(fp);
    /* Read in sequence         */
    if(baseO)
        ok = ajSeqABIReadSeq(fp,baseO,numBases,&thys->Seq);

    if(!ok)
    {
	ajFileSeek(fp,filestat,0);
	ajFilebuffResetPos(buff);

	return ajFalse;
    }

    ok = ajFalse;
    
    pconO = ajSeqABIGetConfidOffset(fp);
    if(numBases > (ajlong) thys->Qualsize)
    {
        AJCRESIZE(thys->Accuracy, numBases);
        thys->Qualsize = numBases;
    }
    if(pconO)
        ok = ajSeqABIReadConfid(fp, pconO, numBases, thys->Accuracy);

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

    ajFilebuffClear(buff, -1);
    buff->File->End=ajTrue;

    ajStrDel(&smpl);
    ajStrDel(&sample);

    return ajTrue;
}




/* @funcstatic seqReadEnsembl *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using Ensembl SQL access.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadEnsembl(AjPSeq thys, AjPSeqin seqin)
{
    AjPSeq seq = NULL;

    ajDebug("seqReadEnsembl\n"
            "  thys %p\n"
            "  seqin %p\n",
            thys,
            seqin);

    /* If the AJAX Sequence Input data member is empty, try one more. */

    if(!seqin->Data)
        seqin->Query->Access->Access(seqin);

    if(seqin->Data)
    {
        seq = (AjPSeq) seqin->Data;

        ajDebug("seqReadEnsembl got sequence %p\n", seq);

        /*
        ** TODO: It would be good to have an ajSeqAssignSeq function in the
        ** AJAX core library to assign members of a AjPSeq structure to
        ** another one.
        */

        /* For the moment only the name and the sequence need assigning. */

        ajStrAssignS(&thys->Name, seq->Name);
        ajStrAssignS(&thys->Seq, seq->Seq);

        seqin->Records++;

        ajSeqDel(&seq);

        seqin->Data = NULL;

        return ajTrue;
    }

    return ajFalse;
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
    ajuint i = 0;

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

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajSeqPrintbookInFormat ************************************************
**
** Reports the internal data structures as a Docbook table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajSeqPrintbookInFormat(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported sequence formats are summarised "
                "in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Output format</emphasis> (format name), "
                "<emphasis>Sngl</emphasis> "
                "(indicates whether each sequence is written to a new file. "
                "This behaviour is the default and can be set by the "
                "<option>-ossingle</option> command line qualifier.  "
                "<emphasis>Save</emphasis> (indicates that sequence data is "
                "stored internally and written when the output is closed. "
                "This is needed for 'interleaved' formats such as Phylip "
                "and MSF), <emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), "
                "<emphasis>Nuc</emphasis> (\"true\" indicates nucleotide "
                "sequence data may be represented), <emphasis>Pro</emphasis> "
                "(\"true\" indicates protein sequence data may be represented, "
                "<emphasis>Feat</emphasis> (whether the format includes "
                "feature annotation data. "
                "EMBOSS can also read feature data from a separate "
                "feature file).  "
                "<emphasis>Gap</emphasis> (whether the format supports "
                "sequence data with gap characters, for example the results "
                "of an alignment), "
                "<emphasis>Mset</emphasis> (\"true\" indicates that more "
                "than one set of sequences can be stored in a single file. "
                "This is used by, for example, phylogenetic analysis "
                "applications to store many versions of a multiple alignment "
                "for statistical analysis) and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Input sequence formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Try</th>\n");
    ajFmtPrintF(outf, "      <th>Nuc</th>\n");
    ajFmtPrintF(outf, "      <th>Pro</th>\n");
    ajFmtPrintF(outf, "      <th>Feat</th>\n");
    ajFmtPrintF(outf, "      <th>Gap</th>\n");
    ajFmtPrintF(outf, "      <th>Mset</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; seqInFormatDef[i].Name; i++)
    {
	if(!seqInFormatDef[i].Alias)
        {
            namestr = ajStrNewC(seqInFormatDef[i].Name);
            ajListPush(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; seqInFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],seqInFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            seqInFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqInFormatDef[j].Try);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqInFormatDef[j].Nucleotide);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqInFormatDef[j].Protein);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqInFormatDef[j].Feature);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqInFormatDef[j].Gap);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqInFormatDef[j].Multiset);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            seqInFormatDef[j].Desc);
                ajFmtPrintF(outf, "    </tr>\n");
            }
        }
    }
        

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    names = NULL;
    ajListstrFreeData(&fmtlist);

    return;
}




/* @func ajSeqPrinthtmlInFormat ************************************************
**
** Reports the internal data structures as an HTML table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajSeqPrinthtmlInFormat(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Input Format</th><th>Auto</th>\n");
    ajFmtPrintF(outf, "<th>Nuc</th><th>Pro</th><th>Feat</th><th>Gap</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; seqInFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, seqInFormatDef[i].Name);

	if(!seqInFormatDef[i].Alias)
        {
            for(j=i+1; seqInFormatDef[j].Name; j++)
            {
                if(seqInFormatDef[j].Read == seqInFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, " %s", seqInFormatDef[j].Name);
                    if(!seqInFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               seqInFormatDef[j].Name, seqInFormatDef[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td>\n",
                        namestr,
			seqInFormatDef[i].Try);
            ajFmtPrintF(outf, "<td>%B</td><td>%B</td><td>%B</td><td>%B</td>\n",
                        seqInFormatDef[i].Nucleotide,
			seqInFormatDef[i].Protein,
			seqInFormatDef[i].Feature,
                        seqInFormatDef[i].Gap);
            ajFmtPrintF(outf, "<td>%B</td><td>\n%s\n</td></tr>\n",
			seqInFormatDef[i].Multiset,
			seqInFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajSeqPrintwikiInFormat ************************************************
**
** Reports the internal data structures as a wiki table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajSeqPrintwikiInFormat(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Try!!Nuc!!Pro!!Feat!!Gap!!MSet!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; seqInFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, seqInFormatDef[i].Name);

	if(!seqInFormatDef[i].Alias)
        {
            for(j=i+1; seqInFormatDef[j].Name; j++)
            {
                if(seqInFormatDef[j].Read == seqInFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s", seqInFormatDef[j].Name);
                    if(!seqInFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               seqInFormatDef[j].Name, seqInFormatDef[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%B||%B||%B||%B||%B||%s\n",
			namestr,
			seqInFormatDef[i].Try,
			seqInFormatDef[i].Nucleotide,
			seqInFormatDef[i].Protein,
			seqInFormatDef[i].Feature,
			seqInFormatDef[i].Gap,
			seqInFormatDef[i].Multiset,
			seqInFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "|}\n\n");
    ajStrDel(&namestr);

    return;
}




/* @funcstatic seqFindInFormat ************************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Sets iformat as the recognised format, and returns ajTrue.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqFindInFormat(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

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
    ajuint i;

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
    ajuint i;
    AjPStr fmtstr     = NULL;
    AjPStrTok handle  = NULL;
    ajuint ifound;
    AjBool ret        = ajTrue;

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

    ajStrTokenDel(&handle);

    return ret;
}




/* @funcstatic seqAppend ******************************************************
**
** Appends sequence characters in the input line to a growing sequence.
** Non sequence characters are simply ignored.
**
** @param [u] pseq [AjPStr*] Sequence as a string
** @param [r] line [const AjPStr] Input line.
** @return [ajuint] Sequence length to date.
** @@
******************************************************************************/

static ajuint seqAppend(AjPStr* pseq, const AjPStr line)
{
    AjPStr tmpstr = NULL;
    ajuint ret = 0;

    ajStrAssignS(&tmpstr, line);
    ajStrKeepSetAlphaC(&tmpstr, "*.~?#+-");
    ajStrAppendS(pseq, tmpstr);

    ret = ajStrGetLen(*pseq);
    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic seqAppendK *****************************************************
**
** Appends single sequence character in the input line to a growing sequence.
** Non sequence characters are simply ignored.
**
** @param [u] pseq [AjPStr*] Sequence as a string
** @param [r] ch [char] Input character.
** @return [ajuint] Sequence length to date.
** @@
******************************************************************************/

static ajuint seqAppendK(AjPStr* pseq, char ch)
{
    AjPStr tmpstr = NULL;
    ajuint ret = 0;

    ajStrAssignK(&tmpstr, ch);
    ajStrKeepSetAlphaC(&tmpstr, "*.~?#+-");
    ajStrAppendS(pseq, tmpstr);

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
** @return [ajuint] Sequence length to date.
** @@
******************************************************************************/

static ajuint seqAppendCommented(AjPStr* pseq, AjBool* incomment,
				const AjPStr line)
{
    AjPStr tmpstr = NULL;
    ajint i;
    ajuint ret = 0;

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
		ajStrAssignClear(&tmpstr);	/* all comment */
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
		ajStrAssignClear(&tmpstr);
	    }
	}

	if(ajStrGetLen(tmpstr))
	    ajDebug("continuing %B '%S'\n", *incomment, tmpstr);
	else
	    ajDebug("done %B '%S'\n", *incomment, tmpstr);
    }

    ret = ajStrGetLen(*pseq);
    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic seqAppendWarn ***************************************************
**
** Appends sequence characters in the input line to a growing sequence.
**
** Non sequence characters are reported in the return value
** if EMBOSS_SEQWARN is set
**
** @param [u] pseq [AjPStr*] Sequence as a string
** @param [r] line [const AjPStr] Input line.
** @return [const AjPStr] Any rejected non-space characters
** @@
******************************************************************************/

static const AjPStr seqAppendWarn(AjPStr* pseq, const AjPStr line)
{
    AjPStr tmpstr = NULL;

    if(!seqAppendRestStr)
    {
	if(ajNamGetValueC("seqwarn", &tmpstr))
	    ajStrToBool(tmpstr, &seqDoWarnAppend);
        seqAppendRestStr = ajStrNew();
    }

    ajStrAssignS(&seqAppendTmpSeq, line);

    if(seqDoWarnAppend)
    {
	ajStrKeepSetAlphaRestC(&seqAppendTmpSeq, "*.~?#+-", &seqAppendRestStr);
	ajStrAppendS(pseq, seqAppendTmpSeq);

	ajStrDelStatic(&seqAppendTmpSeq);

	if(!ajStrGetLen(seqAppendRestStr))
	    return NULL;

	return seqAppendRestStr;
    }

    ajStrKeepSetAlphaC(&seqAppendTmpSeq, "*.~?#+-");
    ajStrAppendS(pseq, seqAppendTmpSeq);

    ajStrDelStatic(&seqAppendTmpSeq);

    return NULL;
}




/* @funcstatic seqqualAppendWarn ***********************************************
**
** Appends sequence quality characters in the input line to a growing string.
**
** Non sequence characters are reported in the return value
** if EMBOSS_SEQWARN is set
**
** @param [u] Pqual [AjPStr*] Quality values as a string
** @param [r] line [const AjPStr] Input line.
** @return [void]
** @@
******************************************************************************/

static void seqqualAppendWarn(AjPStr* Pqual, const AjPStr line)
{
    ajStrAssignS(&seqAppendTmpSeq, line);

    ajStrKeepSetAscii(&seqAppendTmpSeq, 33, 126);
    ajStrAppendS(Pqual, seqAppendTmpSeq);

    ajStrDelStatic(&seqAppendTmpSeq);

    return;
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
** @param [u] Pline [AjPStr*] Input buffer.
** @param [r] maxlines [ajuint] Maximum number of lines to read
**                              before giving up
** @param [w] len [ajuint*] Length of sequence read.
** @return [AjBool] ajTrue on success. ajFalse on failure or aborting.
** @@
******************************************************************************/

static AjBool seqGcgDots(AjPSeq thys, const  AjPSeqin seqin,
			 AjPStr* Pline,
			 ajuint maxlines, ajuint* len)
{
    AjPStr token  = NULL;
    ajuint check  = 0;
    ajuint nlines = 0;

    AjPFilebuff buff;

    buff = seqin->Filebuff;

    seqGcgRegInit();

    while(nlines < maxlines)
    {
	if(nlines++)
	    if(!ajBuffreadLineStore(buff, Pline,
				   seqin->Text, &thys->TextPtr))
		return ajFalse;

	if(nlines > maxlines)
	    return ajFalse;

	if(!ajRegExec(seqRegGcgDot, *Pline))
	    continue;

	ajDebug("seqGcgDots   .. found\n'%S'\n", *Pline);

	if(!ajRegExec(seqRegGcgChk, *Pline))	/* checksum required */
	    return ajFalse;

	if(ajRegExec(seqRegGcgMsf, *Pline))	/* oops - it's an MSF file */
	    return ajFalse;

	ajRegSubI(seqRegGcgChk, 1, &token);
	ajStrToUint(token, &check);

	ajDebug("   checksum %d\n", check);

	if(ajRegExec(seqRegGcgLen, *Pline))
	{
	    ajRegSubI(seqRegGcgLen, 1, &token);
	    ajStrToUint(token, len);
	    ajDebug("   length %d\n", *len);
	}

	if(ajRegExec(seqRegGcgNam, *Pline))
	{
	    ajRegSubI(seqRegGcgNam, 0, &thys->Name);
	    ajDebug("   name '%S'\n", thys->Name);
	}

	if(ajRegExec(seqRegGcgTyp, *Pline))
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
** @param [u] Pline [AjPStr*] Input buffer.
** @param [r] maxlines [ajuint] Maximum number of lines to read
**                              before giving up
** @param [w] len [ajuint*] Length of sequence read.
** @return [AjBool] ajTrue on success. ajFalse on failure or aborting.
** @@
******************************************************************************/

static AjBool seqGcgMsfDots(AjPSeq thys, const AjPSeqin seqin, AjPStr* Pline,
			    ajuint maxlines, ajuint* len)
{
    AjPStr token = NULL;
    ajuint check  = 0;
    ajuint nlines = 0;

    AjPFilebuff buff;

    buff = seqin->Filebuff;

    ajDebug("seqGcgMsfDots maxlines: %d\nline: '%S'\n", maxlines,*Pline);

    seqGcgRegInit();

    while(nlines < maxlines)
    {
	if(nlines++)
	    if(!ajBuffreadLineStore(buff, Pline,
                                    seqin->Text, &thys->TextPtr))
		return ajFalse;

	ajDebug("testing line %d\n'%S'\n", nlines,*Pline);

	if(nlines > maxlines)
	    return ajFalse;

	if(!ajRegExec(seqRegGcgDot, *Pline))
	    continue;

	/* dots found. This must be the line if this is MSF format */

	if(!ajRegExec(seqRegGcgChk, *Pline))	/* check: is required */
	    return ajFalse;

	if(!ajRegExec(seqRegGcgMsf, *Pline)) /* MSF: len required for GCG*/
	    return ajFalse;


	ajRegSubI(seqRegGcgMsf, 1, &token);
	ajStrToUint(token, len);

	ajRegSubI(seqRegGcgChk, 1, &token);
	ajStrToUint(token, &check);

	if(ajRegExec(seqRegGcgNam, *Pline))
	    ajRegSubI(seqRegGcgNam, 0, &thys->Name);

	if(ajRegExec(seqRegGcgTyp, *Pline))
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
** is defined on a separate line. The results are stored
** in the MSF internal table. The sequence data is read later in the
** input file and added to the table.
**
** @param [r] line [const AjPStr] Input line.
** @param [u] Pmsfitem [SeqPMsfItem*] MSF internal table item.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqGcgMsfHeader(const AjPStr line, SeqPMsfItem* Pmsfitem)
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

    *Pmsfitem = AJNEW0(msfitem);
    msfitem->Name = name;

    ajRegSubI(seqRegGcgChk, 1, &token);
    ajStrToUint(token, &msfitem->Check);

    if(ajRegExec(seqRegGcgMsflen, line))
    {
	ajRegSubI(seqRegGcgMsflen, 1, &token);
	ajStrToUint(token, &msfitem->Len);
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




/* @funcstatic seqUsaRegInit **************************************************
**
** Initialised regular expressions for parsing USAs
**
** @return [void]
******************************************************************************/

static void seqUsaRegInit(void)
{
    if(seqRegUsaInitDone)
        return;

    if(!seqRegUsaFmt)
	seqRegUsaFmt = ajRegCompC("^([A-Za-z0-9-]*)::(.*)$");
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

    if(!seqRegUsaId)
#ifndef WIN32
        /* \1 is filename \5 is the qry->Field \6 is the qry->QryString */
	seqRegUsaId = ajRegCompC("^([^|]+[|]|[^:{%]+)"
			   "(([:{%])(([^:}]+):)?([^:}]*)}?)?$");
#else
	/* Windows file names can start with e.g.: 'C:\' */
	/* But allow e.g. 'C:/...', for Staden spin */

        /* \1 is filename \6 is the qry->Field \7 is the qry->QryString */
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

    seqRegUsaInitDone = ajTrue;

    return;
}




/* @func ajSeqUsaGetBase *******************************************************
**
** Extracts the base part from a USA, suitable for use in fetching other
**sequences from the same source
**
** @param [r] usa [const AjPStr] Original USA
** @param [u] Pbaseusa [AjPStr*] Base part of USA
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajSeqUsaGetBase(const AjPStr usa, AjPStr* Pbaseusa)
{
    AjPStr tmpstr  = NULL;

    AjBool regstat   = ajFalse;
#ifdef __CYGWIN__
    AjPStr usatmp    = NULL;
#endif

    seqUsaRegInit();
    
    ajStrAssignC(Pbaseusa, "");

    ajStrAssignS(&seqUsaTest, usa);

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

    /* trim any range */

    if(ajRegExec(seqRegUsaRange, seqUsaTest))
    {
	ajRegPre(seqRegUsaRange, &tmpstr);
        ajStrAssignS(&seqUsaTest, tmpstr);
    }

    /* no base for an ASIS:: USA */

    if(ajRegExec(seqRegUsaAsis, seqUsaTest))
        return ajFalse;

    /* no base for a listfile USA */

    if(ajRegExec(seqRegUsaList, seqUsaTest))
        return ajFalse;

    if(ajRegExec(seqRegUsaFmt, seqUsaTest))
    {
	ajRegSubI(seqRegUsaFmt, 1, &tmpstr);
        ajStrAppendS(Pbaseusa, tmpstr);
        ajStrAppendC(Pbaseusa, "::");
	ajRegSubI(seqRegUsaFmt, 2,&tmpstr);
        ajStrAssignS(&seqUsaTest, tmpstr);
    }

    regstat = ajRegExec(seqRegUsaDb, seqUsaTest);

    if(regstat)
    {
	ajRegSubI(seqRegUsaDb, 1, &tmpstr);
	if(!ajNamDatabase(tmpstr))
            regstat = ajFalse;
    }

    if(regstat)
        ajStrAppendS(Pbaseusa, tmpstr);
    else
    {
        if(ajRegExec(seqRegUsaId, seqUsaTest))
	{
#ifndef WIN32
	    ajRegSubI(seqRegUsaId, 1, &tmpstr);
#else
	    ajRegSubI(seqRegUsaId, 1, &tmpstr);
#endif
	    ajDebug("found filename %S\n", tmpstr);
            ajStrAppendS(Pbaseusa, tmpstr);
        }
        
    }
    ajStrDel(&tmpstr);

    if(!ajStrGetLen(*Pbaseusa))
        return ajFalse;
        
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

    seqUsaRegInit();
    
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
	ajRegSubI(seqRegUsaDb, 3, &qry->Field);
	ajRegSubI(seqRegUsaDb, 1, &seqQryDb);

	/* clear it if this was really a file */	

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
		else if(qry->HasAcc && ajStrMatchCaseC(qry->Field, "acc"))
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

		if(qry->HasAcc)
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
	    qry->Access = ajCallTableGetS(seqDbMethods,qry->Method);

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

	    if(ajStrMatchC(seqQryChr, "%"))
            {
		ajStrToLong(qry->QryString, &qry->Fpos);
		accstat = ajSeqAccessOffset(seqin);

		if(accstat)
		    return ajTrue;
	    }
	    else
	    {
		if(ajStrGetLen(qry->QryString))
		{
		    ajDebug("file QryString '%S' Field '%S' hasAcc:%B seqQryChr '%S'\n",
			    qry->QryString, qry->Field, qry->HasAcc, seqQryChr);

		    if(ajStrGetLen(qry->Field)) /* set by dbname above */
		    {
			/* ajDebug("    qry->Field %S\n", qry->Field); */
			if(ajStrMatchCaseC(qry->Field, "id"))
			    ajStrAssignS(&qry->Id, qry->QryString);
			else if(qry->HasAcc &&
				ajStrMatchCaseC(qry->Field, "acc"))
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
		    else        /* no specific field */
		    {
			ajStrAssignS(&qry->Id, qry->QryString);

			if(qry->HasAcc && ajSeqtestIsAccession(qry->QryString))
			    ajStrAssignS(&qry->Acc, qry->QryString);

                        if(ajSeqtestIsSeqversion(qry->QryString))
                            ajStrAssignS(&qry->Sv, qry->QryString);
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
    ajStrTokenAssignC(&handle, qry->DbFields, "\t ,;\n\r");

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
    ajuint i = 0;

    iter = ajListIterNewread(list);

    ajDebug("SeqUsaListTrace %d nodes\n", ajListGetLength(list));

    while(!ajListIterDone(iter))
    {
	node = (SeqPListUsa) ajListIterGet(iter);
	ajDebug("%3d: '%S' %4d..%d (%b) '%S' %d\n",
		++i, node->Usa, node->Begin, node->End, node->Rev,
		node->Formatstr, node->Format);
    }

    ajListIterDel(&iter);
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
** the prior settings are stored with each USA in the list node so that they
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
    AjPStrTok handle = NULL;
    AjBool ret       = ajFalse;
    SeqPListUsa node = NULL;

    static ajint depth    = 0;
    static ajint MAXDEPTH = 16;

    depth++;
    ajDebug("++seqListProcess %S depth %d Rev: %B\n",
	    listfile, depth, seqin->Rev);

    if(depth > MAXDEPTH)
	ajFatal("USA List too deep");

    if(!ajListGetLength(seqin->List))
	seqin->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &seqReadLine))
    {
	seqListNoComment(&seqReadLine);

	if(ajStrGetLen(seqReadLine))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \t\n\r");
	    ajStrTokenNextParse(&handle, &token);
	    /* ajDebug("Line  '%S'\n");*/
	    /* ajDebug("token '%S'\n", seqReadLine, token); */

	    if(ajStrGetLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0(node);
	        ajStrAssignS(&node->Usa, token);
	        seqUsaSave(node, seqin);
	        ajListPushAppend(list, node);
	    }

	    ajStrDel(&token);
	    token = NULL;
	}
    }

    ajFileClose(&file);
    ajStrDel(&token);

    ajDebug("Trace seqin->List\n");
    seqUsaListTrace(seqin->List);
    ajDebug("Trace new list\n");
    seqUsaListTrace(list);
    ajListPushlist(seqin->List, &list);

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

    ajStrTokenDel(&handle);
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
    ajuint i;
    char *cp;

    i = ajStrGetLen(*text);

    if(!i)				/* empty string */
	return;

    MAJSTRGETUNIQUESTR(text);
    
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
** @param [u] thys [AjPSeq] Sequence object
** @param [r] str [const AjPStr] User supplied name.
** @return [void]
** @@
******************************************************************************/

static void seqSetName(AjPSeq thys, const AjPStr str)
{
    AjPStrTok split = NULL;
    AjPStr token = NULL;

    if(!ajStrGetLen(str))
    {
        ajSeqSetNameMulti(thys, NULL);
    }
    else if(ajStrIsWord(str))
    {
	ajDebug("seqSetName word '%S'\n", str);
	split = ajStrTokenNewC(str, ":");

	while(ajStrTokenNextParse(&split, &token))
	    if(ajStrGetLen(token))
	       ajStrAssignS(&thys->Name, token);

	ajStrTokenDel(&split);
    }
    else
    {
	ajDebug("seqSetName non-word '%S'\n", str);
	ajStrAssignS(&thys->Name, str);
	ajStrRemoveWhiteExcess(&thys->Name);
	ajStrExchangeKK(&thys->Name, ' ', '_');
	ajDebug("seqSetName cleaned '%S'\n", thys->Name);
    }

    ajDebug("seqSetName '%S' result: '%S'\n", str, thys->Name);
    ajStrDel(&token);

    return;
}




/* @funcstatic seqitemSetName **************************************************
**
** Sets the name for a multiple sequence item object by applying simple
** conversion rules to the input which could be, for example, the name from a
** FASTA format file.
**
** @param [u] thys [SeqPMsfItem] Sequence item object
** @param [r] str [const AjPStr] User supplied name.
** @return [void]
** @@
******************************************************************************/

static void seqitemSetName(SeqPMsfItem thys, const AjPStr str)
{
    AjPStrTok split = NULL;
    AjPStr token = NULL;

    if(ajStrIsWord(str))
    {
	ajDebug("seqitemSetName word '%S'\n", str);
	split = ajStrTokenNewC(str, ":");

	while(ajStrTokenNextParse(&split, &token))
	    if(ajStrGetLen(token))
	       ajStrAssignS(&thys->Name, token);

	ajStrTokenDel(&split);
    }
    else
    {
	ajDebug("seqitemSetName non-word '%S'\n", str);
	ajStrAssignS(&thys->Name, str);
	ajStrRemoveWhiteExcess(&thys->Name);
	ajStrExchangeKK(&thys->Name, ' ', '_');
	ajDebug("seqitemSetName cleaned '%S'\n", thys->Name);
    }

    ajDebug("seqitemSetName '%S' result: '%S'\n", str, thys->Name);
    ajStrDel(&token);

    return;
}




/* @funcstatic seqnameSetName *************************************************
**
** Sets the name usable by a sequence object by applying simple conversion
** rules to the input which could be, for example, the name from a
** FASTA format file.
**
** @param [u] name [AjPStr*] Sequence name derived.
** @param [r] str [const AjPStr] User supplied name.
** @return [void]
** @@
******************************************************************************/

static void seqnameSetName(AjPStr *name, const AjPStr str)
{
    AjPStrTok split = NULL;
    AjPStr token = NULL;

    if(ajStrIsWord(str))
    {
	ajDebug("seqnameSetName word '%S'\n", str);
	split = ajStrTokenNewC(str, ":");

	while(ajStrTokenNextParse(&split, &token))
	    if(ajStrGetLen(token))
	       ajStrAssignS(name, token);

	ajStrTokenDel(&split);
    }
    else
    {
	ajDebug("seqnameSetName non-word '%S'\n", str);
	ajStrAssignS(name, str);
	ajStrRemoveWhiteExcess(name);
	ajStrExchangeKK(name, ' ', '_');
	ajDebug("seqnameSetName cleaned '%S'\n", *name);
    }

    ajDebug("seqnameSetName '%S' result: '%S'\n", str, *name);
    ajStrDel(&token);

    return;
}




/* @funcstatic seqSetNameNospace **********************************************
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

static void seqSetNameNospace(AjPStr* name, const AjPStr str)
{
    ajStrAssignS(name, str);

    if(!ajStrIsWord(str))
    {
	ajDebug("seqSetNameNospace non-word '%S'\n", str);
	ajStrRemoveWhiteExcess(name);
	ajStrExchangeKK(name, ' ', '_');
	ajDebug("seqSetNameNospace cleaned '%S'\n", *name);
    }

    ajDebug("seqSetNameNospace '%S' result: '%S'\n", str, *name);

    return;
}




/* @funcstatic seqSetNameFile *************************************************
**
** Sets the name for a sequence object by applying simple conversion
** rules to the input source file..
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] seqin [const AjPSeqin] Sequence input object
** @return [void]
** @@
******************************************************************************/

static void seqSetNameFile(AjPSeq thys, const AjPSeqin seqin)
{
    AjPStr tmpname = NULL;

    ajStrAssignS(&tmpname, seqin->Filename);

    seqSetName(thys, tmpname);

    if(seqin->Count > 1)
	ajFmtPrintAppS(&thys->Name, "_%3d", seqin->Count);

    ajDebug("seqSetNameFile '%S' result: '%S'\n", tmpname, thys->Name);
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
    if(!thys->Acclist)
        thys->Acclist = ajListstrNew();

    ajListstrPushAppend(thys->Acclist, ajStrNewS(acc));

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
** @param [r] level [ajuint] 0: taxon level 1: species 2: organelle
** @return [void]
** @@
******************************************************************************/

static void seqTaxSave(AjPSeq thys, const AjPStr tax, ajuint level)
{
    AjPStr newstr = NULL;

    switch(level)
    {
        case 1:
            if(!ajStrGetLen(thys->Tax))
                ajStrAssignS(&thys->Tax, tax);
            break;
        case 2:
            if(!ajStrGetLen(thys->Organelle))
                ajStrAssignS(&thys->Organelle, tax);
            break;
        default:
            if(!thys->Taxlist)
                thys->Taxlist = ajListstrNew();
            newstr = ajStrNewS(tax);
            ajListstrPushAppend(thys->Taxlist, newstr);
            break;
    }

    return;
}




/* @funcstatic seqTaxidSaveI ***************************************************
**
** Adds an organism NCBI taxonomy id to the stored list for a sequence.
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] tax [ajuint] Organism NCBI taxonomy id
** @return [void]
** @@
******************************************************************************/

static void seqTaxidSaveI(AjPSeq thys, ajuint tax)
{
    if(tax && !ajStrGetLen(thys->Taxid))
        ajStrFromUint(&thys->Taxid, tax);

    return;
}




/* @funcstatic seqTaxidSaveS ***************************************************
**
** Adds an organism NCBI taxonomy id to the stored list for a sequence.
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] tax [const AjPStr] Organism NCBI taxonomy id
** @return [void]
** @@
******************************************************************************/

static void seqTaxidSaveS(AjPSeq thys, const AjPStr tax)
{
    if(!ajStrGetLen(thys->Taxid))
        ajStrAssignS(&thys->Taxid, tax);

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
** sequence output object. The target pointer does not need to be
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
    pthis->HasAcc  = ajTrue;

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
    ajStrDel(&thys->DbIdentifier);
    ajStrDel(&thys->DbSequence);
    ajStrDel(&thys->DbFilter);
    ajStrDel(&thys->DbReturn);
    ajStrDel(&thys->DbAccession);
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
	    " Key '%S' Org '%S' Case %B Done %B\n",
	    seq->Name, thys->Id, thys->Acc, thys->Sv, thys->Gi,
	    thys->Des, thys->Key, thys->Org, thys->CaseId, thys->QryDone);

    if(!thys)			   /* no query to test, that's fine */
	return ajTrue;

    if(thys->QryDone)			/* do we need to test here? */
	return ajTrue;

    /* test the query field(s) */

    if(ajStrGetLen(thys->Id))
    {
	if(thys->CaseId)
	{
	    if(ajStrMatchWildS(seq->Name, thys->Id))
		return ajTrue;
	}
	else
	{
	    if(ajStrMatchWildCaseS(seq->Name, thys->Id))
		return ajTrue;
	}

	ajDebug("id test failed\n");
	tested = ajTrue;
	ok = ajFalse;
    }

    if(ajStrGetLen(thys->Sv)) /* test Sv and Gi */
    {
	if(ajStrMatchWildCaseS(seq->Sv, thys->Sv))
	    return ajTrue;

	ajDebug("sv test failed\n");
	tested = ajTrue;
	ok = ajFalse;
    }

    if(ajStrGetLen(thys->Gi)) /* test Sv and Gi */
    {
	if(ajStrMatchWildCaseS(seq->Gi, thys->Gi))
	    return ajTrue;

	ajDebug("gi test failed\n");
	tested = ajTrue;
	ok = ajFalse;
    }

    if(!ajStrGetLen(thys->Acc))
    {
	/*ajDebug("No accession number to test\n");*/
    }
    else if(ajListGetLength(seq->Acclist))
    {		   /* accession number test - check the entire list */
	iter = ajListIterNewread(seq->Acclist);

	while(!ajListIterDone(iter))
	{
	    accstr = ajListIterGet(iter);
	    ajDebug("... try accession '%S' '%S'\n", accstr,
		    thys->Acc);

	    if(ajStrMatchWildCaseS(accstr, thys->Acc))
	    {
		ajListIterDel(&iter);

		return ajTrue;
	    }
	}

	tested = ajTrue;
	ajDebug("acc test failed\n");
	ajListIterDel(&iter);
    }

    if(!ajStrGetLen(thys->Org))
    {
	/*ajDebug("No taxonomy to test\n"); */
    }
    else if(ajListGetLength(seq->Taxlist))
    {			   /* taxonomy test - check the entire list */
	iter = ajListIterNewread(seq->Taxlist);

	while(!ajListIterDone(iter))
	{
	    taxstr = ajListIterGet(iter);
	    ajDebug("... try organism '%S' '%S'\n", taxstr,
		    thys->Org);

	    if(ajStrMatchWildCaseS(taxstr, thys->Org))
	    {
		ajListIterDel(&iter);

		return ajTrue;
	    }
	}

	tested = ajTrue;
	ajDebug("org test failed\n");
	ajListIterDel(&iter);
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
    else if(ajListGetLength(seq->Keylist))
    {			    /* keyword test - check the entire list */
	iter = ajListIterNewread(seq->Keylist);

	while(!ajListIterDone(iter))
	{
	    keystr = ajListIterGet(iter);
	    ajDebug("... try keyword '%S' '%S'\n", keystr,
		    thys->Key);

	    if(ajStrMatchWildCaseS(keystr, thys->Key))
	    {
		ajListIterDel(&iter);

		return ajTrue;
	    }
	}

	tested = ajTrue;
	ajDebug("key test failed\n");
	ajListIterDel(&iter);
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

	if(ajStrMatchWildWordCaseS(seq->Desc, thys->Des))
	    return ajTrue;

	tested = ajTrue;
	ajDebug("des test failed\n");
	ajListIterDel(&iter);
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
** Compiles the regular expressions for testing wild cards in queries.
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

    if(ajStrGetLen(thys->Id))
	ajDebug( "    Id: '%S'\n", thys->Id);

    if(ajStrGetLen(thys->Acc))
	ajDebug( "    Acc: '%S'\n", thys->Acc);

    if(ajStrGetLen(thys->Des))
	ajDebug( "    Des: '%S'\n", thys->Des);

    if(ajStrGetLen(thys->Key))
	ajDebug( "    Key: '%S'\n", thys->Key);

    if(ajStrGetLen(thys->Org))
	ajDebug( "    Org: '%S'\n", thys->Org);

    if(ajStrGetLen(thys->Sv))
	ajDebug( "    Sv: '%S'\n", thys->Sv);

    if(ajStrGetLen(thys->Gi))
	ajDebug( "    Gi: '%S'\n", thys->Gi);

    ajDebug( "    Case-sensitive Id: '%B'\n", thys->CaseId);
    ajDebug( "   Has accession: %B\n", thys->HasAcc);

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

    if(ajStrGetLen(thys->DbFields))
	ajDebug( "    DbFields: '%S'\n", thys->DbFields);

    if(ajStrGetLen(thys->DbProxy))
	ajDebug( "    DbProxy: '%S'\n", thys->DbProxy);

    if(ajStrGetLen(thys->DbHttpVer))
	ajDebug( "    DbHttpVer: '%S'\n", thys->DbHttpVer);

    if(ajStrGetLen(thys->Field))
	ajDebug( "    Field: '%S'\n", thys->Field);

    if(ajStrGetLen(thys->QryString))
	ajDebug( "    QryString: '%S'\n", thys->QryString);

    if(ajStrGetLen(thys->Application))
	ajDebug( "    Application: '%S'\n", thys->Application);

    if(ajStrGetLen(thys->Application))
	ajDebug( "    Application: '%S'\n", thys->Application);

    ajDebug( "   Fpos: %ld\n", thys->Fpos);
    ajDebug( "   QryDone: %B\n", thys->QryDone);
    ajDebug( "   Wildcard in query: %B\n", thys->Wild);

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
    AjPStr token2    = NULL;
    AjPStr str       = NULL;
    AjBool ok = ajFalse;

    ajDebug("ajSeqParseFasta '%S'\n", instr);

    if(!ajStrPrefixC(instr, ">"))
	return ajFalse;

    ajStrAssignS(&str, instr);

    ajStrTokenAssignC(&handle, str, "> ");
    ajStrTokenNextParseC(&handle, " \t\n\r", id);

    ok = ajStrTokenNextParse(&handle, &token);
    ajStrAssignS(&token2, token);
    ajStrRemoveSetC(&token2, "()");

    if(ok && ajSeqtestIsSeqversion(token2))
    {
        ajStrAssignS(acc, ajSeqtestIsSeqversion(token2));
	ajStrAssignS(sv, token2);
	ajStrTokenNextParseC(&handle, "\n\r", desc);
    }
    else if(ok && ajSeqtestIsAccession(token2))
    {
	ajStrAssignS(acc, token2);
        ajStrAssignClear(sv);
	ajStrTokenNextParseC(&handle, "\n\r", desc);
    }
    else if(ok)
    {
        ajStrAssignClear(acc);
        ajStrAssignClear(sv);
	ajStrAssignS(desc, token);

	if(ajStrTokenNextParseC(&handle, "\n\r", &token))
	{
	    ajStrAppendC(desc, " ");
	    ajStrAppendS(desc, token);
	}
    }

    ajStrDel(&token); /* duplicate of accession or description */
    ajStrDel(&token2);
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
** to what can really happen. Sadly,'real files' also includes
** internal IDs in blast databases reformatted by formatdb.
**
** @param [r] instr [const AjPStr]   fasta line.
** @param [w] id [AjPStr*]   id.
** @param [w] acc [AjPStr*]  accession number.
** @param [w] sv [AjPStr*]  sequence version number.
** @param [w] gi [AjPStr*]  GI version number.
** @param [w] db [AjPStr*]  NCBI database name
** @param [w] desc [AjPStr*] description.
** @return [AjBool] ajTrue if ncbi format
** @@
******************************************************************************/

AjBool ajSeqParseNcbi(const AjPStr instr, AjPStr* id, AjPStr* acc,
		      AjPStr* sv, AjPStr* gi, AjPStr* db, AjPStr* desc)
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
    ajuint  i;
    ajuint  nt;
    AjBool ret = ajFalse;

    ajStrAssignClear(db);

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

    ajStrAssignClear(id);
    ajStrTokenAssignC(&handle,idstr,"|");

    ajStrTokenNextParse(&handle, &prefix);
    q = MAJSTRGETPTR(prefix);

    ajDebug(" idstr: '%S'\n", idstr);
    ajDebug("prefix: '%S'\n", prefix);

    if(!strncmp(q,"gi",2))
    {
        /* ajDebug("gi prefix\n"); */
	ajStrTokenNextParse(&handle, &token);
	ajStrAssignS(gi, token);

	if(! ajStrTokenNextParse(&handle, &prefix))
	{
	    /* we only have a gi prefix */
	    ajDebug("*only* gi prefix\n");
	    ajStrAssignS(id, token);
	    ajStrAssignClear(acc);
	    ajStrAssignS(desc, reststr);
	    ajDebug("found pref: '%S' id: '%S', acc: '%S' "
	       "desc: '%S'\n",
	       prefix, *id, *acc, *desc);
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
	ajDebug("continue with '%S'\n", prefix);
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

	if(ajSeqParseNcbi(reststr,id,acc,sv,gi,db,desc))
	{
	    ajStrAssignEmptyS(db, token);
	    /* recursive ... */
	    ajDebug("ajSeqParseNcbi recursive success '%S'\n", reststr);
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
        ajDebug("ajSeqParseNcbi recursive failed '%S' - use gnl id\n",
		reststr);
	ajStrAssignS(id,numtoken);
	ajStrAssignClear(acc);
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
     ** still checking for any mis-formatted databases elsewhere */

    if(!strcmp(q,"bbs") || !strcmp(q,"lcl"))
    {
	if(!strcmp(q, "lcl"))
	    ajStrAssignS(db, prefix);

        /* ajDebug("bbs or lcl prefix\n"); */
	ajStrTokenNextParse(&handle, id);
	ajStrAssignClear(acc);
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
	/* ajDebug("gnl or pat prefix\n"); */
	ajStrTokenNextParse(&handle, &token);
        if(!strcmp(q,"gnl"))
	   ajStrAssignS(db, token);
	ajStrTokenNextParse(&handle, id);
	ajStrAssignClear(acc);		/* no accession number */
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
	ajStrAssignS(db, prefix);
        /* ajDebug("gnl or pat or pdb prefix\n"); */
	ajStrTokenNextParse(&handle, id);

	if(ajStrTokenNextParse(&handle, &token))
	{
	    /* chain identifier to append */
	    ajStrAppendS(id, token);
	}

	ajStrAssignClear(acc);		/* no accession number */
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
       || !strcmp(q,"tpd") || !strcmp(q,"tpd") || !strcmp(q,"tpg")
       || !strcmp(q,"sp") || !strcmp(q,"ref"))
    {
        /* ajDebug("gb,emb,dbj,sp,ref prefix\n"); */
	ajStrAssignS(db, prefix);
	ajStrTokenNextParse(&handle, &token);
	vacc = ajSeqtestIsSeqversion(token);

	if(vacc)
	{
	    ajStrAssignS(sv,token);
	    ajStrAssignS(acc,vacc);
	}
	else if(ajSeqtestIsAccession(token))
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
	ajStrAssignS(db, prefix);
        /* ajDebug("pir,prf prefix\n"); */
	ajStrTokenNextParse(&handle, id);
	ajStrAssignS(desc, reststr);
	ajStrAssignClear(acc);
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

    ajDebug("No prefix accepted - try the last 2 fields\n");

    nt = ajStrParseCountC(idstr,"|");

    if(ajStrGetCharLast(idstr) == '|')
      nt++;

    ajDebug("Barred tokens - %d found\n", nt);

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

    ajStrTokenAssignC(&handle,idstr,"|");

    for(i=0;i<nt-2;++i)
      ajStrTokenNextParse(&handle, &token);

    ajStrAssignS(db, token);
    ajStrTokenNextParse(&handle, &token);
    ajDebug("token acc: '%S'\n", token);
    vacc = ajSeqtestIsSeqversion(token);

    if(vacc)
    {
	ajStrAssignS(sv,token);
	ajStrAssignS(acc,vacc);
	ajStrAssignS(id,vacc);
    }
    else if(ajSeqtestIsAccession(token))
    {
        ajStrAssignS(acc,token);
        ajStrAssignS(id,token);
    }
    else
    {
        ajStrAssignS(id,token);
    }
    

    if(ajStrTokenNextParseC(&handle, " \n\t\r", &token))
    {
       ajDebug("token id: '%S'\n", token);

       if(ajStrGetLen(token))
	 ajStrAssignS(id,token);
    }

    ajStrTokenNextParseC(&handle, "\n\r", &token);
    ajStrAssignS(desc, reststr);
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




/* @func ajSeqParseFastq ******************************************************
**
** Parse a fastq id line. Return id acc sv and description
**
** @param [r] instr [const AjPStr]   fastq line.
** @param [w] id [AjPStr*]   id.
** @param [w] desc [AjPStr*] description.
** @return [AjBool] ajTrue if fastq format
** @@
******************************************************************************/

AjBool ajSeqParseFastq(const AjPStr instr, AjPStr* id, AjPStr* desc)
{
    AjPStr str       = NULL;

    /*ajDebug("ajSeqParseFastq '%S'\n", instr);*/

    if(!ajStrPrefixC(instr, "@"))
	return ajFalse;

    ajStrExtractWord(instr, desc, &str);
    ajStrTrimC(desc, "\n");
    ajStrAssignSubS(id, str, 1, -1);

    ajStrDel(&str);
    return ajTrue;
}




/* @func ajSeqGetFromUsa ******************************************************
**
** Returns a sequence given a USA
**
** @param [r] thys [const AjPStr] USA
** @param [r] protein [AjBool] True if protein
** @param [u] seq [AjPSeq] sequence
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajSeqGetFromUsa(const AjPStr thys, AjBool protein, AjPSeq seq)
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
    ok = ajSeqRead(seq, seqin);
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
    ajuint i;
    ajuint istart;
    ajuint iend;
    ajuint ilen;
    ajuint iwidth;
    AjPStr tmpstr = NULL;

    ilen = ajStrGetLen(seq);
    iwidth = 60;

    for(i=0; i < ilen; i += iwidth)
    {
	istart = i;
	iend = AJMIN(ilen-1, istart+iwidth-1);
	ajStrAssignSubS(&tmpstr, seq, istart, iend);
	ajFmtPrintAppS(textptr, "%S\n", tmpstr);
    }

    ajStrDel(&tmpstr);

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

    ajRegFree(&seqRegTreeconTop);
    ajRegFree(&seqRegMegaCommand);
    ajRegFree(&seqRegMegaFeat);
    ajRegFree(&seqRegMegaSeq);
    ajRegFree(&seqRegJackTop);
    ajRegFree(&seqRegJackSeq);
    ajRegFree(&seqRegGffTyp);
    ajRegFree(&seqRegGff3Typ);
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
    ajRegFree(&seqRegMaseHead);
    ajRegFree(&seqRegPhylipTop);
    ajRegFree(&seqRegPhylipHead);
    ajRegFree(&seqRegPhylipSeq);
    ajRegFree(&seqRegPhylipSeq2);

    /* sequence reading strings */
    ajStrDel(&seqFtFmtEmbl);
    ajStrDel(&seqFtFmtGenbank);
    ajStrDel(&seqFtFmtRefseq);
    ajStrDel(&seqFtFmtRefseqp);
    ajStrDel(&seqFtFmtGff);
    ajStrDel(&seqFtFmtPir);
    ajStrDel(&seqFtFmtSwiss);
    ajStrDel(&seqUsaTest);
    ajStrDel(&seqQryChr);
    ajStrDel(&seqQryDb);
    ajStrDel(&seqQryList);
    ajStrDel(&seqAppendRestStr);
    ajStrDel(&seqAppendTmpSeq);
    ajStrDel(&seqQualStr);

    ajStrDel(&seqReadLine);
    ajStrDel(&seqSaveLine);
    ajStrDel(&seqSaveLine2);
    ajTableFree(&seqDbMethods);

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

    if(ajListGetLength(thys->List))
	ajDebug( "  List: (%d)\n", ajListGetLength(thys->List));

    if(thys->Filebuff)
	ajDebug( "  Filebuff: %F (%Ld)\n",
		ajFilebuffGetFile(thys->Filebuff),
		ajFileResetPos(ajFilebuffGetFile(thys->Filebuff)));

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
** @param [r] i [ajuint] Number of sequences
** @return [SeqPStockholm] New sequence object.
** @@
******************************************************************************/

static SeqPStockholm stockholmNew(ajuint i)
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
    thys->sqcons  = ajStrNew();
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
    thys->sqcons  = ajStrNew();
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
    ajuint i;

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
    ajStrDel(&pthis->sqcons);
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
    ajStrDel(&pthis->sqcons);
    ajStrDel(&pthis->sscons);

    AJFREE(*Pseq);

    return;
}*/




/* @funcstatic selexNew *******************************************************
**
** Creates and initialises a selex #=SQ line object.
**
** @param [r] n [ajuint] Number of sequences
** @return [SeqPSelex] New sequence object.
** @@
******************************************************************************/

static SeqPSelex selexNew(ajuint n)
{
    SeqPSelex thys = NULL;
    ajuint    i;

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

static SeqPSelexseq selexseqNew(void)
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




/* @funcstatic selexseqDel ****************************************************
**
** Deletes a Selex object.
**
** @param [d] Pseq [SeqPSelexseq*] Selex #=SQ object
** @return [void]
** **
******************************************************************************/

static void selexseqDel(SeqPSelexseq *Pseq)
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
}




/* @funcstatic selexDel *******************************************************
**
** Deletes a Selex object.
**
** @param [d] Pseq [SeqPSelex*] Selex object
** @return [void]
** @@
******************************************************************************/

static void selexDel(SeqPSelex *Pseq)
{
    SeqPSelex pthis;
    ajuint    i;
    ajuint    n;

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
}




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




/* @funcstatic seqDefine ******************************************************
**
** Make sure all sequence object attributes are defined
** using values from the sequence input object if needed
**
** @param [w] thys [AjPSeq] Sequence returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqDefine(AjPSeq thys, AjPSeqin seqin)
{

    /* if values are missing in the sequence object, we can use defaults
       from seqin or calculate where possible */

    /*ajDebug("seqDefine: thys->Db '%S', seqin->Db '%S'\n",
      thys->Db, seqin->Db);*/
    /*ajDebug("seqDefine: thys->Name '%S' type: %S\n",
      thys->Name, thys->Type);*/
    /*ajDebug("seqDefine: thys->Entryname '%S', seqin->Entryname '%S'\n",
      thys->Entryname, seqin->Entryname);*/

    /* assign the dbname and entryname if defined in the seqin object */
    if(ajStrGetLen(seqin->Db))
      ajStrAssignS(&thys->Db, seqin->Db);

    if(ajStrGetLen(seqin->Entryname))
        ajStrAssignEmptyS(&thys->Entryname, seqin->Entryname);

    if(ajStrGetLen(thys->Entryname))
      ajStrAssignS(&thys->Name, thys->Entryname);

    /*ajDebug("seqDefine: returns thys->Name '%S' type: %S\n",
      thys->Name, thys->Type);*/

    if(!ajStrGetLen(thys->Type))
    {
        if(thys->Format) 
        {
            if(seqInFormatDef[thys->Format].Nucleotide &&
               !seqInFormatDef[thys->Format].Protein)
                ajSeqSetNuc(thys);

            if(!seqInFormatDef[thys->Format].Nucleotide &&
               seqInFormatDef[thys->Format].Protein)
                ajSeqSetProt(thys);
        }
    }
    
    if(!ajStrGetLen(thys->Type))
	ajSeqType(thys);
    
    return ajTrue;
}




/* @section ASIS Sequence Access **********************************************
**
** These functions manage the ASIS sequence access methods.
**
******************************************************************************/




/* @func ajSeqAccessAsis ******************************************************
**
** Reads a sequence using the 'filename' as the sequence data.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqAccessAsis(AjPSeqin seqin)
{
    AjPSeqQuery qry;

    qry = seqin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("ASIS access: no sequence");

	return ajFalse;
    }

    ajDebug("ajSeqAccessAsis %S\n", qry->Filename);

    ajFilebuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFilebuffNewLine(qry->Filename);

    if(!seqin->Filebuff)
    {
	ajDebug("Asis access: unable to use sequence '%S'\n", qry->Filename);

	return ajFalse;
    }

    ajStrAssignC(&seqin->Filename, "asis");
    /*ajFilebuffTrace(seqin->Filebuff);*/

    return ajTrue;
}




/* @section File Access *******************************************************
**
** These functions manage the sequence file access methods.
**
******************************************************************************/




/* @func ajSeqAccessFile ******************************************************
**
** Reads a sequence from a named file.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqAccessFile(AjPSeqin seqin)
{
    AjPSeqQuery qry;

    qry = seqin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("FILE access: no filename");

	return ajFalse;
    }

    ajDebug("ajSeqAccessFile %S\n", qry->Filename);

    /* ajStrTraceT(qry->Filename, "qry->Filename (before):"); */

    ajFilebuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFilebuffNewNameS(qry->Filename);

    if(!seqin->Filebuff)
    {
	ajDebug("FILE access: unable to open file '%S'\n", qry->Filename);

	return ajFalse;
    }

    /* ajStrTraceT(seqin->Filename, "seqin->Filename:"); */
    /* ajStrTraceT(qry->Filename, "qry->Filename (after):"); */

    ajStrAssignS(&seqin->Filename, qry->Filename);

    return ajTrue;
}




/* @func ajSeqAccessOffset ****************************************************
**
** Reads a sequence from a named file, at a given offset within the file.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqAccessOffset(AjPSeqin seqin)
{
    AjPSeqQuery qry;

    qry = seqin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("FILE access: no filename");

	return ajFalse;
    }

    ajDebug("ajSeqAccessOffset %S %Ld\n", qry->Filename, qry->Fpos);

    /* ajStrTraceT(qry->Filename, "qry->Filename (before):"); */

    ajFilebuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFilebuffNewNameS(qry->Filename);

    if(!seqin->Filebuff)
    {
	ajDebug("OFFSET access: unable to open file '%S'\n", qry->Filename);

	return ajFalse;
    }

    ajFileSeek(ajFilebuffGetFile(seqin->Filebuff), qry->Fpos, 0);
    /* ajStrTraceT(seqin->Filename, "seqin->Filename:"); */
    /* ajStrTraceT(qry->Filename, "qry->Filename (after):"); */
    ajStrAssignS(&seqin->Filename, qry->Filename);

    return ajTrue;
}





/* @func ajSeqtableGetDb ******************************************************
**
** returns the table in which sequence database access details are registered
**
** @return [AjPTable] Access functions hash table
** @@
******************************************************************************/

AjPTable ajSeqtableGetDb(void)
{
    if(!seqDbMethods)
        seqDbMethods = ajCallTableNew();
    return seqDbMethods;
}





/* @func ajSeqMethodTest ******************************************************
**
** Tests for a named method for sequence reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(seqDbMethods, method))
      return ajTrue;

    return ajFalse;
}




