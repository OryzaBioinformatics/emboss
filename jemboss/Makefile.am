## Makefile.am for Jemboss
## Process this file with automake to produce Makefile.in in all directories.
AUTOMAKE_OPTIONS = gnits

SUBDIRS = api images lib org resources utils

# files with nonstandard names in this directory
EXTRA_DIST = LICENSE

all-local:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "Not compiling Jemboss"; \
	else \
	  (if test "${SHELL}" = "/bin/bash" ; then \
	  declare -x CLASSPATH="lib/JembossPrintAlignment.jar:lib/xerces.jar:lib/mail.jar:lib/activation.jar:lib/jakarta-regexp-1.2.jar:lib/jalview.jar:lib/jcert.jar:lib/jnet.jar:lib/jsse.jar:.:lib/axis/axis-ant.jar:lib/axis/axis.jar:lib/axis/commons-discovery.jar:lib/axis/commons-logging.jar:lib/axis/jaxrpc.jar:lib/axis/log4j-1.2.4.jar:lib/axis/saaj.jar:lib/axis/wsdl4j.jar:lib/grout.jar:$(srcdir)"; \
	  export CLASSPATH; \
	  javac -d . org/emboss/jemboss/Jemboss.java; \
	  else \
	  CLASSPATH="lib/JembossPrintAlignment.jar:lib/xerces.jar:lib/mail.jar:lib/activation.jar:lib/jakarta-regexp-1.2.jar:lib/jalview.jar:lib/jcert.jar:lib/jnet.jar:lib/jsse.jar:.:lib/axis/axis-ant.jar:lib/axis/axis.jar:lib/axis/commons-discovery.jar:lib/axis/commons-logging.jar:lib/axis/jaxrpc.jar:lib/axis/log4j-1.2.4.jar:lib/axis/saaj.jar:lib/axis/wsdl4j.jar:lib/grout.jar:$(srcdir)"; \
	  export CLASSPATH; \
	  javac -d . org/emboss/jemboss/Jemboss.java; \
	  fi); \
	fi

install-exec-hook:
	rm -f $(bindir)/runJemboss.csh; \
	echo "#!/bin/csh" >> $(bindir)/runJemboss.csh; \
	echo "setenv LD_LIBRARY_PATH $(libdir)" >> $(bindir)/runJemboss.csh; \
	echo "setenv CLASSPATH $(prefix)/share/$(PACKAGE)/jemboss/lib/JembossPrintAlignment.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/xerces.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/mail.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/activation.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jakarta-regexp-1.2.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jalview.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jcert.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jnet.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/jsse.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/axis-ant.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/axis.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/commons-discovery.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/commons-logging.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/jaxrpc.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/log4j-1.2.4.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/saaj.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/axis/wsdl4j.jar:$(prefix)/share/$(PACKAGE)/jemboss/lib/grout.jar:$(prefix)/share/$(PACKAGE)/jemboss" >> $(bindir)/runJemboss.csh; \
	echo "(cd $(prefix)/share/$(PACKAGE)/jemboss; java org/emboss/jemboss/Jemboss &)" >> $(bindir)/runJemboss.csh; \
	chmod 755 $(bindir)/runJemboss.csh;

pkgdata_DATA = LICENSE readme.html runJemboss.sh README

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss



# tar to pick up the other directories
# then remove any CVS subdirectories

dist-hook:
	tar cBf - api | ( cd $(distdir); tar xBf - ; rm -rf api/CVS ) 
