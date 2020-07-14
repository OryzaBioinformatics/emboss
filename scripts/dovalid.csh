#!/bin/csh

foreach x ( ~/local/share/EMBOSS/acd/*.acd )
  set y = $x:t
  acdvalid $y:r |& ~/hgmp/scripts/dovalidclean.pl $y:r
end
