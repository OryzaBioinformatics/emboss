pkgdata_DATA = Database.java ProgList.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/startup

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/startup; \
	for i in *.class; do \
	  ${INSTALL_DATA} $$i $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/startup; \
	done; \
	fi
