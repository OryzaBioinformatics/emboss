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

## PCRE stuff (evrsion 4.3)

POSIX_MALLOC_THRESHOLD = @POSIX_MALLOC_THRESHOLD@
PCRELIBVERSION = @PCRE_LIB_VERSION@
PCREPOSIXLIBVERSION = @PCRE_POSIXLIB_VERSION@

INCLUDES = -DAJAX_FIXED_ROOT=$(AJAX_FIXED_ROOT) -DPREFIX=\"$(prefix)\" -I$(top_srcdir)/plplot $(POSIX_MALLOC_THRESHOLD) 
#LDFLAGS = -no-undefined
LIBS =

AJAXSRC = ajacd.c ajalign.c ajarr.c ajassert.c \
ajbase.c \
ajcall.c ajcod.c \
ajdan.c ajexcept.c \
ajfeat.c ajfile.c ajfmt.c ajgraphxml.c ajjava.c ajlist.c \
ajmath.c ajmatrices.c ajmem.c ajmess.c ajnam.c ajnexus.c ajphylo.c \
ajrange.c ajreg.c ajreport.c \
ajseq.c ajseqabi.c ajseqdb.c ajseqread.c ajseqtype.c ajseqwrite.c \
ajsort.c ajstr.c ajsys.c \
ajtable.c ajtime.c ajtranslate.c ajutil.c ajvector.c ajxyz.c \
pcre.c pcre_study.c pcreposix.c


AJAXGSRC = ajgraph.c ajhist.c

noinst_HEADERS = ajax.h ajarch.h ajdefine.h \
ajacd.h ajalign.h ajarr.h ajassert.h \
ajbase.h ajcall.h ajcod.h \
ajdan.h ajexcept.h ajfeat.h ajfeatdata.h ajfile.h ajfmt.h \
ajgraph.h ajgraphxml.h ajgraphstruct.h ajhist.h ajjava.h ajlist.h \
ajmath.h ajmatrices.h ajmem.h ajmess.h ajnam.h ajnexus.h ajphylo.h \
ajrange.h ajreg.h ajreport.h \
ajseq.h ajseqabi.h ajseqdata.h ajseqdb.h ajseqread.h ajseqtype.h ajseqwrite.h \
ajsort.h ajstr.h ajsys.h \
ajtable.h ajtime.h ajtranslate.h ajutil.h ajvector.h ajxyz.h \
pcre.h pcre_chartables.c pcre_config.h pcre_get.c pcre_printint.c \
pcre_internal.h pcreposix.h


libajax_la_SOURCES = $(AJAXSRC)
libajax_la_LDFLAGS = -version-info 1:0:1
libajaxg_la_SOURCES = $(AJAXGSRC)
libajaxg_la_LDFLAGS = -version-info 1:0:1
