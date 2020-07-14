install-exec-hook:
	  @if test "${JAVA_OK}" = "yes" ; then \
	          (if test -d $(prefix)/share/$(PACKAGE)/jemboss/resources; then \
			echo "resources exists"; \
		   else \
			mkdir $(prefix)/share/$(PACKAGE)/jemboss/resources; \
		   fi); \
	          rm -f $(prefix)/share/$(PACKAGE)/jemboss/resources/acdstore.jar; \
		  cd $(EMBOSS_TOP)/emboss/acd; \
		  jar cvf $(prefix)/share/$(PACKAGE)/jemboss/resources/acdstore.jar *.acd; \
	  fi


pkgdata_DATA = jemboss.properties readme.txt readmeAlign.txt version filemgr.html seqList.html

pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/resources
