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
echo >! ../efunc.check
echo >! ../edata.out
echo >! ../edata.check

foreach x ($edir/ajax/*.c)
  embossdoccheck.pl $x >> ../efunc.check
  embossdoc.pl $x >> ../efunc.out
end
cat *.srs >! ../efunc.dat
\cp *html $wdir/Ajax/

\rm *html
\rm *.srs

foreach x ($edir/nucleus/*.c)
  embossdoccheck.pl $x >> ../efunc.check
  embossdoc.pl $x >> ../efunc.out
end
cat *.srs >> ../efunc.dat
\cp *html $wdir/Nucleus/

\rm *html
\rm *.srs

foreach x ($edir/emboss/*.c)
  embossdoccheck.pl $x >> ../efunc.check
  embossdoc.pl $x >> ../efunc.out
end
cat *.srs >> ../efunc.dat
\cp *html $wdir/Appsource/

\rm *html
\rm *.srs

foreach x ($edir/ajax/*.h)
  embossdatacheck.pl $x >> ../edata.check
  embossdatadoc.pl $x >> ../edata.out
end
foreach x ($edir/nucleus/*.h)
  embossdatacheck.pl $x >> ../edata.check
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
srsbuild efunc -rel '2.1.0'

srsbuild edata -nn
srsbuild edata -rel '2.1.0'

srsbuild -l efunc
srsbuild -l edata
