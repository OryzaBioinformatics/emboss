pkgdata_DATA = Application.java ApplicationFields.java ApplicationParam.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/parser/acd

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/parser/acd; \
	  ${INSTALL_DATA} *.class $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/parser/acd; \
	fi
