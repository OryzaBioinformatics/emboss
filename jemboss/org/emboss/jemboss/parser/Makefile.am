SUBDIRS = acd

pkgdata_DATA = AcdFunResolve.java  AcdVariableResolve.java  AcdVarResolve.java \
Ajax.java  AjaxUtil.java  Dependent.java  ParseAcd.java

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/parser

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/parser; \
	  ${INSTALL_DATA} *.class $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/parser; \
	fi
