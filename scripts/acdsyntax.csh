#!/bin/csh

../emboss/entrails -auto -full entrails-full.txt 
grep 'ajNamGetValue' ../ajax/*.c >! acdsyntax.getvalue
../scripts/acdstats.pl >! acdsyntax.acdstats 
../scripts/acdsyntax.pl entrails-full.txt 
cp test.html ~/public_html/acdsyntax.html
#rm acdsyntax.getvalue
