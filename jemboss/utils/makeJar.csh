#!/bin/csh
cd ..
/packages/java/jdk1.3/bin/jar cf Jemboss.jar images/* org/emboss/jemboss/*class resources/version resources/jemboss.properties resources/readme.txt org/emboss/jemboss/*/*class org/emboss/jemboss/*/*/*class uk/ac/mrc/hgmp/embreo/*class uk/ac/mrc/hgmp/embreo/*/*class
mv Jemboss.jar utils
