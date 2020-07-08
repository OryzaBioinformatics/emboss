#!/bin/csh -f

if ($#argv != 2) then
  echo "usage:"
  echo "embossdoc.csh srctop wwwtop"
endif

set edir = $argv[1]
set wdir = $argv[2]


\rm -rf x/
mkdir x
cd x
echo >! ../efunc.out
echo >! ../edata.out
foreach x ($edir/ajax/*.c)
  embossdoc.pl $x >> ../efunc.out
end
cat *.srs >! ../efunc.dat
\cp *html $wdir/Ajax/

\rm *html
\rm *.srs

foreach x ($edir/nucleus/*.c)
  embossdoc.pl $x >> ../efunc.out
end
cat *.srs >> ../efunc.dat
\cp *html $wdir/Nucleus/

\rm *html
\rm *.srs

foreach x ($edir/emboss/*.c)
  embossdoc.pl $x >> ../efunc.out
end
cat *.srs >> ../efunc.dat
\cp *html $wdir/Appsource/

\rm *html
\rm *.srs

foreach x ($edir/ajax/*.h)
  embossdatadoc.pl $x >> ../edata.out
end
cat *.srsdata >! ../edata.dat
\cp *html $wdir/Data/

\rm *html
\rm *.srsdata


cd ..
\rm -rf x/

source ~/srs6/etc/prep_srs

srsbuild efunc -nn
srsbuild efunc -c
srsbuild efunc -rel '0.0.4'

srsbuild edata -nn
srsbuild edata -c
srsbuild edata -rel '0.0.4'
