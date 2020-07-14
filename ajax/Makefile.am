## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libajax.la libajaxg.la

if ISSHARED
if ISAIXIA64
if NEEDAJAX
AM_CFLAGS = -Wl,-G -L../plplot/.libs -L.libs -lajax -lplplot -lm
else
AM_CFLAGS = -Wl,-G -L../plplot/.libs -L.libs -lplplot -lm
endif
else
AM_CFLAGS =
endif
endif

INCLUDES = -DAJAX_FIXED_ROOT=$(AJAX_FIXED_ROOT) -DPREFIX=\"$(prefix)\" -I$(top_srcdir)/plplot
#LDFLAGS = -no-undefined
LIBS =

AJAXSRC = ajacd.c ajalign.c ajarr.c ajassert.c \
ajbase.c \
ajcall.c ajcod.c \
ajdan.c ajexcept.c \
ajfeat.c ajfile.c ajfmt.c ajjava.c ajlist.c \
ajmath.c ajmatrices.c ajmem.c ajmess.c ajnam.c \
ajposreg.c ajrange.c ajreg.c ajreport.c \
ajseq.c ajseqabi.c ajseqdb.c ajseqread.c ajseqtype.c ajseqwrite.c \
ajsort.c ajstr.c ajsys.c \
ajtable.c ajtime.c ajtranslate.c ajutil.c ajxyz.c \
hsp_debug.c hsp_regcomp.c hsp_regerror.c \
hsp_regexec.c hsp_regfree.c hsp_split.c \
hsregerror.c hsregexp.c hsregsub.c 

AJAXGSRC = ajgraph.c ajhist.c

noinst_HEADERS = ajax.h ajarch.h ajdefine.h \
ajacd.h ajalign.h ajarr.h ajassert.h \
ajbase.h ajcall.h ajcod.h \
ajdan.h ajexcept.h ajfeat.h ajfeatdata.h ajfile.h ajfmt.h \
ajgraph.h ajgraphstruct.h ajhist.h ajjava.h ajlist.h \
ajmath.h ajmatrices.h ajmem.h ajmess.h ajnam.h \
ajposreg.h ajrange.h ajreg.h ajreport.h \
ajseq.h ajseqabi.h ajseqdata.h ajseqdb.h ajseqread.h ajseqtype.h ajseqwrite.h \
ajsort.h ajstr.h ajsys.h \
ajtable.h ajtime.h ajtranslate.h ajutil.h ajxyz.h \
hsregmagic.h hsregexp.h \
hsp_cclass.h hsp_cname.h hsp_engine.c hsp_engine.h \
hsp_regcomp.h hsp_regerror.h \
hsp_regex.h hsp_regex2.h hsp_utils.h 


libajax_la_SOURCES = $(AJAXSRC)
libajax_la_LDFLAGS = -version-info 1:0:1
libajaxg_la_SOURCES = $(AJAXGSRC)
libajaxg_la_LDFLAGS = -version-info 1:0:1



