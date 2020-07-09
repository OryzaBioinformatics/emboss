SUBDIRS = gui parser programs server soap

pkgdata_DATA = Jemboss.java JembossLogo.java JembossParams.java JembossJarUtil.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	  $(top_srcdir)/mkinstalldirs $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss; \
	for i in *.class; do \
	  ${INSTALL_DATA} $$i $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss; \
	done; \
	fi
