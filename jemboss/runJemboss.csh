#!/bin/csh -f
setenv CLASSPATH :lib/soap.jar:lib/xerces.jar:lib/mail.jar:lib/activation.jar:lib/jakarta-regexp-1.2.jar:lib/jalview.jar:embreo/embreo-full.jar:.:
#setenv LD_LIBRARY_PATH /usr/local/lib
java org/emboss/jemboss/Jemboss &
