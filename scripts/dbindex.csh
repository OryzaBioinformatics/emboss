#!/bin/csh

set top = /scratch/pmr/local/share/EMBOSS/test

echo "$top"

cd test

cd swiss
echo "Indexing swiss"
dbiflat -fields="acnum,seqvn,des,keyword,taxon" \
    -dbname SWISSPROT -auto
diff entrynam.idx $top/swiss/

cd ../swnew
echo "Indexing swnew"
dbiflat -fields="acnum,seqvn,des,keyword,taxon" \
    -dbname SWISSNEW -auto
diff entrynam.idx $top/swnew/

cd ../embl
echo "Indexing embl"
dbiflat -fields="acnum,seqvn,des,keyword,taxon" \
    -dbname EMBL -idformat embl -auto
diff entrynam.idx $top/embl/

cd ../genbank
echo "Indexing genbank"
dbiflat -fields="acnum,seqvn,des,keyword,taxon" \
    -dbname GENBANK -idformat gb -filenames '*.seq' -auto
diff entrynam.idx $top/genbank/

cd ../gb
echo "Indexing gb"
dbigcg -fields="acnum,seqvn,des,keyword,taxon" \
    -dbname GENBANK -idformat genbank -auto
diff entrynam.idx $top/gb/

cd ../pir
echo "Indexing pir"
dbigcg -fields="acnum,seqvn,des,keyword,taxon" \
    -dbname PIR -idformat pir -auto
diff entrynam.idx $top/pir/

cd ../wormpep
echo "Indexing wormpep"
dbifasta -fields="acnum,des" \
    -dbname WORMPEP -filenames 'wormpep' -auto
diff entrynam.idx $top/wormpep/

