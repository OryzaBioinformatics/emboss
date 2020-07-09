pkgdata_DATA = DragTree.java FileEditorDisplay.java FileNode.java \
FileSave.java FileSaving.java RemoteDragTree.java \
RemoteFileNode.java RemoteFileTreePanel.java
pkgdatadir=$(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/filetree

install-exec-hook:
	@if test "${JAVA_OK}" != "yes" ; then \
	  echo "No class files to copy"; \
	else \
	$(top_srcdir)/mkinstalldirs $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/filetree; \
	  ${INSTALL_DATA} *.class $(prefix)/share/$(PACKAGE)/jemboss/org/emboss/jemboss/gui/filetree; \
	fi
