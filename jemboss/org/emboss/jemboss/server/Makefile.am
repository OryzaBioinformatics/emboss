pkgdata_DATA = JembossServer.java JembossFileServer.java  JembossThread.java \
JembossAuthServer.java JembossFileAuthServer.java \
TestPublicServer.java TestPrivateServer.java AppendToLogFileThread.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/server

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/server; \
	  if test -f JembossServer.class ; then \
	    ${INSTALL_DATA} *.class $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/server; \
	  fi \
	fi
