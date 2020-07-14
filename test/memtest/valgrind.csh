#!/bin/csh

setenv EMBOSSRC ../
setenv EMBOSS_OUTDIRECTORY valgrind
(../../scripts/valgrind.pl -all >! valgrind.out) >&! valgrind.err
grep '^Valgrind test' valgrind.err
