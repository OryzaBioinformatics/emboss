## Makefile.am for Jemboss
## Process this file with automake to produce Makefile.in in all directories.
AUTOMAKE_OPTIONS = gnits

SUBDIRS = api embreo images lib org resources utils

# files with nonstandard names in this directory
EXTRA_DIST = LICENSE

all-local:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "Not compiling Jemboss"; \
	else \
	  (if test "${SHELL}" != "/bin/sh" ; then \
	  declare -x CLASSPATH=":lib/soap.jar:lib/xerces.jar:lib/mail.jar:lib/activation.jar:lib/jakarta-regexp-1.2.jar:embreo/embreo-full.jar:lib/jalview.jar:.:"; \
	  export CLASSPATH; \
	  javac org/emboss/jemboss/Jemboss.java; \
	  else \
	  CLASSPATH=":lib/soap.jar:lib/xerces.jar:lib/mail.jar:lib/activation.jar:lib/jakarta-regexp-1.2.jar:embreo/embreo-full.jar:lib/jalview.jar:.:"; \
	  export CLASSPATH; \
	  javac org/emboss/jemboss/Jemboss.java; \
	  fi); \
	fi

install-exec-hook:
	rm -f $(bindir)/runJemboss.csh; \
	echo "#!/bin/csh" >> $(bindir)/runJemboss.csh; \
	echo "setenv LD_LIBRARY_PATH $(libdir)" >> $(bindir)/runJemboss.csh; \
	echo "setenv CLASSPATH $(prefix)/share/$(PACKAGE)/jemboss/lib/soap.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/xerces.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/mail.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/activation.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jakarta-regexp-1.2.jar:$(prefix)/share/$(PACKAGE)/jemboss/embreo/embreo-full.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jalview.jar:." >> $(bindir)/runJemboss.csh; \
	echo "(cd $(prefix)/share/$(PACKAGE)/jemboss; java org/emboss/jemboss/Jemboss &)" >> $(bindir)/runJemboss.csh; \
	chmod 755 $(bindir)/runJemboss.csh;

pkgdata_DATA = LICENSE interface.html readme.html runJemboss.csh setup.html

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss



# tar to pick up the other directories
# then remove any CVS subdirectories

dist-hook:
	tar cBf - api | ( cd $(distdir); tar xBf - ; rm -rf api/CVS ) 
