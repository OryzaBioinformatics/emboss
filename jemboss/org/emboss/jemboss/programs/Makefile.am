pkgdata_DATA = BatchUpdateTimer.java JembossProcess.java ListFile.java \
ResultList.java ResultsUpdateTimer.java RunEmbossApplication.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/programs

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/programs; \
	  ${INSTALL_DATA} *.class $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/programs; \
	fi
