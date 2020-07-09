pkgdata_DATA = AuthPopup.java JembossSoapException.java \
CallAjax.java PendingResults.java \
GetWossname.java PrivateRequest.java \
JembossRun.java ServerSetup.java \
FileRoots.java FileList.java PublicRequest.java \
GetACD.java GetHelp.java ShowDB.java MakeFileSafe.java

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/soap

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/soap; \
	for i in *.class; do \
	  ${INSTALL_DATA} $$i $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/soap; \
	done; \
	fi
