## Process this file with automake to produce Makefile.in

SUBDIRS = lib

lib_LTLIBRARIES = libplplot.la

if ISSHARED
if ISAIXIA64

if AMPNG
AM_CFLAGS = -Wl,-G -lgd -lpng -lz -lX11 -lm
else
AM_CFLAGS = -Wl,-G -lX11
endif

else
AM_CFLAGS =
endif
endif

if ISCYGWIN
AM_LDFLAGS = -no-undefined -Wl,--add-stdcall-alias -Wl,--export-all-symbols -Wl,--enable-auto-import -lX11 -lgd -lpng -lz
endif


INCLUDES = $(X_CFLAGS) -I$(srcdir)/ -I/usr/include/gd -DPREFIX=\"$(prefix)\" -DEMBOSS_TOP=\"$(EMBOSS_TOP)\"
LIBS =

PLPLOTSRC = pdfutils.c plargs.c plbox.c plcont.c plcore.c \
plctrl.c plcvt.c pldtik.c plfill.c plhist.c plline.c plmap.c \
plot3d.c plpage.c plsdef.c plshade.c plsym.c pltick.c \
plvpor.c plwind.c plstripc.c dg300.c hpgl.c impress.c \
linuxvga.c ljiip.c ljii.c next.c null.c pbm.c plbuf.c \
plmeta.c ps.c tek.c xfig.c xwin.c gdpng.c


libplplot_la_SOURCES = $(PLPLOTSRC)

libplplot_la_LDFLAGS = -version-info 5:0:0

include_HEADERS = drivers.h pdf.h plcore.h plevent.h plplotP.h plxwd.h \
metadefs.h plConfig.h pldebug.h plplot.h plstrm.h plDevs.h



