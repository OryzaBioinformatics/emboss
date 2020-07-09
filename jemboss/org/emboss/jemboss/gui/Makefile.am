SUBDIRS = filetree form sequenceChooser startup


pkgdata_DATA = AdvancedOptions.java BuildProgramMenu.java \
ResultsMenuBar.java SetUpMenuBar.java ShowResultSet.java \
ShowSavedResults.java SwingWorker.java LaunchJalView.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui; \
	  ${INSTALL_DATA} *.class $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui; \
	fi

