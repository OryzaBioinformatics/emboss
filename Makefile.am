## Makefile.am for EMBOSS
## Process this file with automake to produce Makefile.in in all directories.
AUTOMAKE_OPTIONS = gnits

SUBDIRS = plplot ajax nucleus emboss test doc

# AJAX_FIXED_ROOT = \"`pwd`/emboss/acd\"

# files with nonstandard names in this directory
EXTRA_DIST = COMPAT LICENSE KNOWN_BUGS ONEWS PROBLEMS

# tar to pick up the other directories
# then remove any CVS subdirectories

dist-hook:
	tar cBf - scripts | ( cd $(distdir); tar xBf - ; rm -rf scripts/CVS ) 
	tar cBf - test | ( cd $(distdir); tar xBf - ; rm -rf test/CVS test/*/CVS ) 
	tar cBf - emboss/acd | ( cd $(distdir); tar xBf - ; cd emboss/acd; rm -rf CVS ) 
	tar cBf - emboss/data | ( cd $(distdir); tar xBf - ; cd emboss/data; rm -rf CVS */CVS */*/CVS ) 
	tar cBf - plplot/lib | ( cd $(distdir); tar xBf - ; cd emboss/data; rm -rf CVS */CVS */*/CVS ) 
	tar cBf - doc | ( cd $(distdir); tar xBf - ; cd emboss/data; rm -rf CVS */CVS */*/CVS ) 
