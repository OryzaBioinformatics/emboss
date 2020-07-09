#!/bin/csh
cd ..
jar cf Jemboss.jar images/* org/emboss/jemboss/*class resources/version resources/jemboss.properties resources/readme.txt org/emboss/jemboss/*/*class org/emboss/jemboss/*/*/*class uk/ac/mrc/hgmp/embreo/*class uk/ac/mrc/hgmp/embreo/*/*class org/emboss/jemboss/*java org/emboss/jemboss/*/*java org/emboss/jemboss/*/*/*java
mv Jemboss.jar utils
