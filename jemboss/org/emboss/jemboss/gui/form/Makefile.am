pkgdata_DATA = BuildJembossForm.java ListFilePanel.java SectionPanel.java \
SetInFileCard.java SetOutFileCard.java TextFieldFloat.java \
TextFieldInt.java TextFieldSink.java myComboPopup.java \
MultiLineToolTipUI.java PlafMacros.java ReportFormat.java \
Separator.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/form

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/form; \
	for i in *.class; do \
	  ${INSTALL_DATA} $$i $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/form; \
	done; \
	fi
