pkgdata_DATA = AuthPopup.java CallAjax.java GetWossname.java \
JembossRun.java JembossSoapException.java PendingResults.java \
PrivateRequest.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/soap

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/soap; \
	  ${INSTALL_DATA} *.class $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/soap; \
	fi
