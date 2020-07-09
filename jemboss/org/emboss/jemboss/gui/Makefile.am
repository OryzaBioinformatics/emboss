SUBDIRS = filetree form sequenceChooser startup

pkgdata_DATA = AdvancedOptions.java ResultsMenuBar.java ShowResultSet.java \
BuildProgramMenu.java SequenceData.java ShowSavedResults.java \
LaunchJalView.java SequenceList.java SwingWorker.java \
MemoryComboBox.java SetUpMenuBar.java  

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui; \
	  ${INSTALL_DATA} *.class $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui; \
	fi

