#!/bin/csh -f
#
# 
#
setenv LIB ./lib

setenv CLASSPATH :$LIB/axis/axis.jar:$LIB/axis/jaxrpc.jar:$LIB/axis/saaj.jar:$LIB/axis/commons-logging.jar:$LIB/axis/commons-discovery.jar:$LIB/jakarta-regexp-1.2.jar:$LIB/jalview.jar:$LIB/jnet.jar:$LIB/jcert.jar:$LIB/jsse.jar:.:$LIB/axis/servlet.jar:$LIB/xerces.jar

#
#Commented vars are required for standalone use
#
#setenv EMBOSS_INSTALL /packages/emboss/STABLE/lib
#setenv LD_LIBRARY_PATH $EMBOSS_INSTALL
#
#For MacOSX:
#setenv DYLD_LIBRARY_PATH $EMBOSS_INSTALL/lib
#
#For HPUX:
#setenv SHLIB_PATH $EMBOSS_INSTALL/lib
#
#Add local to run as 'standalone':
#java org.emboss.jemboss.Jemboss local &
#
java org.emboss.jemboss.Jemboss &
