## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libnucleus.la

if ISSHARED
if ISAIXIA64
AM_CFLAGS = -Wl,-G -lm -L../ajax/.libs -lajax
else
AM_CFLAGS =
endif
endif

if ISCYGWIN
AM_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias -Wl,--export-all-symbols -Wl,--enable-auto-import -L../plplot -L../ajax -lplplot -lajax
endif


INCLUDES = -I$(top_srcdir)/plplot $(X_CFLAGS) -I$(srcdir)/ -I$(top_srcdir)/ajax

LIBS =

NUCLSRC =  embaln.c embcom.c embcons.c embdata.c embdbi.c embdmx.c \
	   embest.c embexit.c embgotoh.c embgroup.c embiep.c embindex.c \
	   embinit.c embmat.c embmisc.c embmol.c \
	   embnmer.c embpat.c embprop.c embread.c \
           embpdb.c embdomain.c embsig.c \
	   embshow.c embword.c

libnucleus_la_SOURCES = $(NUCLSRC)

libnucleus_la_LDFLAGS =  -version-info 1:0:1

include_HEADERS = emboss.h \
		 embaln.h embcom.h embcons.h embdata.h embdbi.h embdmx.h \
		 embest.h embexit.h embgotoh.h embgroup.h embiep.h embindex.h \
		 embinit.h embmat.h embmisc.h \
		 embmol.h embnmer.h embpat.h embprop.h \
                 embpdb.h embdomain.h embsig.h \
                 embread.h embshow.h embword.h 
