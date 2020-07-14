## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libnucleus.la

if ISSHARED
if ISAIXIA64
AM_CFLAGS = -Wl,-G -lm -L../ajax/.libs -lajax
else
AM_CFLAGS =
endif
endif

INCLUDES = -I$(top_srcdir)/plplot $(X_CFLAGS) -I$(srcdir)/ -I$(top_srcdir)/ajax

LIBS =

NUCLSRC =  embaln.c embcom.c embcons.c embdata.c embdbi.c \
	   embest.c embexit.c embgroup.c embiep.c embindex.c \
	   embinit.c embmat.c embmisc.c embmol.c \
	   embnmer.c embpat.c embprop.c embread.c \
	   embshow.c embword.c embxyz.c

libnucleus_la_SOURCES = $(NUCLSRC)

libnucleus_la_LDFLAGS =  -version-info 1:0:1

noinst_HEADERS = emboss.h \
		 embaln.h embcom.h embcons.h embdata.h embdbi.h \
		 embest.h embexit.h embgroup.h embiep.h embindex.h \
		 embinit.h embmat.h embmisc.h \
		 embmol.h embnmer.h embpat.h embprop.h \
                 embread.h embshow.h embword.h embxyz.h
