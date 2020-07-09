## Makefile.am for EMBOSS
## Process this file with automake to produce Makefile.in in all directories.
AUTOMAKE_OPTIONS = gnits

SUBDIRS = plplot ajax nucleus emboss test doc jemboss

# AJAX_FIXED_ROOT = \"`pwd`/emboss/acd\"

# files with nonstandard names in this directory
EXTRA_DIST = COMPAT LICENSE KNOWN_BUGS ONEWS PROBLEMS FAQ depcomp

# tar to pick up the other directories
# then remove any CVS subdirectories

dist-hook:
	tar cBf - scripts | ( cd $(distdir); tar xBf - ; ) 
	tar cBf - test | ( cd $(distdir); tar xBf - ; ) 
	tar cBf - emboss/acd | ( cd $(distdir); tar xBf - ; cd emboss/acd; ) 
	tar cBf - emboss/data | ( cd $(distdir); tar xBf - ; cd emboss/data; ) 
	tar cBf - plplot/lib | ( cd $(distdir); tar xBf - ; ) 
	tar cBf - doc | ( cd $(distdir); tar xBf - ; ) 
	tar cBf - jemboss | ( cd $(distdir); tar xBf - ; find jemboss -name Makefile -exec rm -rf {} \; )
