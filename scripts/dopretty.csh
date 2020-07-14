#!/bin/csh -f

foreach x (*.acd)

echo $x:r

cp $x ~/acdsave
acdpretty $x:r
~/hgmp/dosection.pl  $x:r.acdpretty >! $x:r.acd
cp $x:r.acd ~/local/share/EMBOSS/acd/

end
