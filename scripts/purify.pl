#!/usr/local/bin/perl -w

sub runtest ($) {
  my ($name) = @_;
  print "Test $name\n";

  if (defined($tests{$name})) {
  print "Running $purepath$tests{$name}\n";
    $status = system ("$purepath$tests{$name}");
    
    if ($status) {
      print STDERR "Purify test $name returned status $status\n";
    }
    else {
      print STDERR "Purify test $name OK\n";
    }
  return $status;
  }
  else {
      print STDERR "ERROR: Unknown test $name \n";
      return -1;
  }

}

%tests = (
	  "feat1" => "seqretallfeat -auto tembl:hsfau",
	  "featmany" => "seqretallfeat -auto 'tembl:hsf*'",
	  "set1" => "seqretset -auto tsw:opsd_human",
	  "setmany" => "seqretset -auto 'tsw:opsd_*'",
	  "embl1" => "seqret -auto tembl:hsfau",
	  "emblmany" => "seqret -auto 'tembl:hsf*'",
	  "wossname" => "wossname -search codon -auto",
	  "wordmatch" => "wordmatch tembl:hsfau tembl:hsfau1 -auto",
          "wordcount" => "wordcount tembl:hsfau -auto",
	  "wobble" => "wobble tembl:hsfau -auto",
	  "water" => "water tsw:opsd_human tsw:opsd_xenla -auto",
	  "vectorstrip" => "vectorstrip tembl:hsfau -vectors $ENV{EPURE}/../test/data/data.vectorstrip -vectorfile -auto",
	  "trimseq" => "trimseq tembl:hsfau -window 20 -percent 10 -auto",
	  "transeq" => "transeq -frame r tembl:hsfau -auto",
	  "tmap" => "tmap tsw:opsd_human -auto",
	  "tfm" => "tfm tfm -nomore",
	  "textsearch" => "textsearch 'tsw:*' opsin -auto",
	  "syco" => "syco -plot tembl:hsfau -cfile Echicken.cut -auto",
#	  "swissparse" => "swissparse -keyfile $ENV{EPURE}/../test/data/test.terms -spfile $ENV{EPURE}/../test/data/test.seq -auto",
	  "supermatcher" => "supermatcher tembl:hsfau tembl:hsfau1 -auto",
	  "stretcher" => "stretcher tembl:hsfau tembl:hsfau1 -auto",
	  "splitter" => "splitter -size 500 tembl:hsfau -auto",
	  "silent" => "silent tembl:hsfau -sbeg 31 -send 50 -auto",
	  "siggen" => "siggen -algpath $ENV{EPURE}/../test/data -algextn .align -conpath $ENV{EPURE}/../test/data -conextn .con -sparsity 10 -randomize N -datafile EBLOSUM62 -scoreseq Y -scorencon Y -scoreccon Y -scoreboth N -postsim Y -auto",
	  "sigcleave" => "sigcleave tsw:opsd_human -auto",
	  "shuffleseq" => "shuffleseq tembl:hsfau -auto",
	  "showseq" => "showseq tembl:hsfau -auto",
	  "showorf" => "showorf tembl:hsfau -auto",
	  "showfeat" => "showfeat tembl:hsfau -auto",
	  "showdb" => "showdb",
	  "showalign" => "showalign $ENV{EPURE}/../test/data/globins.msf -auto",
	  "seqtofeat" => "seqtofeat tsw:opsd_human -pattern VIA -auto",
	  "seqretsplit" => "seqretsplit 'tembl:hsfau*' -auto",
	  "seqretset" => "seqretset $ENV{EPURE}/../test/data/globins.msf -auto",
	  "seqretfeat" => "seqretfeat tembl:hsfau -auto",
	  "seqretallfeat" => "seqretallfeat 'tembl:hsfau*' -auto",
	  "seqretall" => "seqretall 'tembl:hsfau*' -auto",
	  "seqret" => "seqret tsw:opsd_human -auto",
	  "seqmatchall" => "seqmatchall 'tembl:hsfau*' -auto",
	  "seqinfo" => "seqinfo tsw:opsd_human -auto",
	  "seealso" => "seealso fuzznuc",
	  "scope" => "scope -inf $ENV{EPURE}/../test/data/test.scopraw -auto",
	  "scopalign" => "scopalign -scopf $ENV{EPURE}/../test/data/test.scopnr -path $ENV{EPURE}/../test/data",
	  "revseq" => "revseq tembl:hsfau -auto",
	  "restrict" => "restrict tembl:hsfau -auto",
	  "restover" => "restover tembl:hsfau -seqcomp acgt -auto",
	  "remap"    => "remap -auto",
	  "recoder" => "recoder tembl:hsfau -sbeg 31 -send 50 -auto",
	  "prophecy" => "prophecy $ENV{EPURE}/../test/data/globins.msf -type g -auto",
	  "prophet" => "prophet tsw:100k_rat $ENV{EPURE}/../test/data/prophecy.gribskov -auto",
	  "profit" => "profit $ENV{EPURE}/../test/data/globins.prophecy tsw:100k_rat -auto",
	  "primersearch" => "primersearch 'tembl:*' $ENV{EPURE}/../test/data/primers -auto",
	  "prima" => "prima tembl:hsfau -auto",
	  "prettyseq" => "prettyseq tembl:hsfau -auto",
	  "prettyplot" => "prettyplot $ENV{EPURE}/../test/data/globins.msf -auto",
	  "preg" => "preg 'tsw:*' -pattern kat -auto",
	  "polydot" => "polydot 'tembl:hsfa*' -auto",
	  "plotorf" => "plotorf 'tembl:hsfau' -auto",
	  "plotcon" => "plotcon $ENV{EPURE}/../test/data/globins.msf -auto",
	  "pepwindowall" => "pepwindowall tsw:opsd_human -auto",
	  "pepwindow" => "pepwindow tsw:opsd_human -auto",
	  "pepwheel" => "pepwheel tsw:opsd_human -auto",
	  "pepstats" => "pepstats tsw:opsd_human -auto",
	  "pepnet" => "pepnet tsw:opsd_human -auto",
	  "pepcoil" => "pepcoil tsw:opsd_human -auto",
	  "patmattest" => "patmattest tsw:opsd_human -expr kat -auto",
	  "patmatmotifs" => "patmatmotifs tsw:opsd_human -auto",
	  "patmatdb" => "patmatdb 'tsw:*' -motif kat -auto",
	  "pasteseq" => "pasteseq tembl:hsfau tembl:hsfau1 -pos 200 -auto",
	  "palindrome" => "palindrome tembl:hsfau1 -min 4 -auto",
	  "oddcomp" => "oddcomp 'tsw:*' -comp $ENV{EPURE}/../test/data/oddcomp.comp -auto",
	  "octanol" => "octanol tsw:opsd_human -auto",
	  "nthseq" => "nthseq $ENV{EPURE}/..test/swiss/seq.dat -num 5 -auto",
	  "nrscope" => "nrscope -scopin $ENV{EPURE}/../test/data/test.scopembl -scopout $ENV{EPURE}/../test/data/test.scopnr -dpdb $ENV{EPURE}/../test/data -extn .pxyz -thresh 95.0 -datafile EBLOSUM62 -gapopen 10 -gapextend 0.5 -errf nrscope.log",
	  "notseq" => "notseq 'tsw:*' -exclude 'ops*' -auto",
	  "noreturn" => "noreturn $ENV{EPURE}/../test/data/oddcomp.comp -auto",
	  "newseq" => "newseq -name fred -desc test -type p -seq acdefg -auto",
	  "newcpgseek" => "newcpgseek tembl:xl23808 -auto",
	  "newcpgreport" => "newcpgreport tembl:xl23808 -auto",
	  "needle" => "needle tembl:hsfau tembl:hsfau1 -auto",
	  "msbar" => "msbar tembl:hsfau -auto",
	  "merger" => "merger tembl:hsfau tembl:hsfau1 -auto",
	  "megamerger" => "megamerger tembl:hsfau tembl:hsfau1 -auto",
	  "matcher" => "matcher tembl:hsfau tembl:hsfau1 -auto",
	  "maskseq" => "maskseq tembl:hsfau -regions '20-50' -auto",
	  "maskfeat" => "maskfeat tembl:hsfau -auto",
	  "marscan" => "marscan tembl:hsfau -auto",
	  "lindna" => "lindna -inp $ENV{EPURE}/../test/data/data.linp -auto",
	  "isochore" => "isochore tembl:hsfau -auto",
	  "intconv" => "intconv $ENV{EPURE}/../test/data/data.linp -auto",
	  "infoseq" => "infoseq tsw:opsd_human",
	  "infoalign" => "infoalign $ENV{EPURE}/../test/data/globins.msf -auto",
	  "iep" => "iep tsw:opsd_human -auto",
	  "hmoment" => "hmoment tsw:opsd_human -auto",
	  "getorf" => "getorf tembl:hsfau -auto",
	  "geecee" => "geecee tembl:hsfau -auto",
	  "garnier" => "garnier tsw:opsd_human -auto",
	  "fuzztran" => "fuzztran tembl:hsfau -patt kat -auto",
	  "fuzzpro" => "fuzzpro tsw:opsd_human -patt MMNKQFR -auto",
	  "fuzznuc" => "fuzznuc tembl:hsfau -patt ggcaggcgcgc -auto",
	  "freak" => "freak tembl:hsfau -auto",
	  "findkm" => "findkm $ENV{EPURE}/../test/data/test.findkm -auto",
	  "extractseq" => "extractseq tembl:hsfau -region '20-50' -auto",
	  "etandem" => "etandem tembl:hsfau -auto",
	  "est2genome" => "est2genome tembl:hsfau tembl:hsfau1 -auto",
	  "equicktandem" => "equicktandem tembl:hsfau -auto",
	  "entret" => "entret tembl:hsfau -auto",
	  "entrails" => "entrails -auto",
	  "emowse" => "emowse 'tsw:*fugru'  $ENV{EPURE}/../test/data/test.mowse -auto",
	  "embossversion" => "embossversion",
	  "embossdata" => "embossdata -fetch -file Eamino.dat",
	  "einverted" => "einverted tembl:hsfau -auto",
	  "dreg" => "dreg tembl:hsfau -pattern ggcaggcgcgc -auto",
	  "dottup" => "dottup tembl:hsfau tembl:hsfau1 -auto",
	  "dotpath" => "dotpath tembl:hsfau tembl:hsfau1 -auto",
	  "dotmatcher" => "dotmatcher tembl:hsfau tembl:hsfau1 -auto",
	  "domainer" => "domainer -scop $ENV{EPURE}/../test/data/test.scopembl -cpdb $ENV{EPURE}/../test/data/ -cpdbscop $ENV{EPURE}/../test/data/ -cpdbextn .pxyz -pdbscop $ENV{EPURE}/../test/data/ -pdbextn .ent -cpdberrf domainer.log1 -pdberrf domainer.log2",
	  "distmat" => "distmat $ENV{EPURE}/../test/data/globins.msf -auto",
	  "digest" => "digest tsw:opsd_human -auto",
	  "diffseq" => "diffseq tembl:hsfa tembl:hsfau1 -auto",
	  "descseq" => "descseq -desc 'Test' tsw:opsd_human -auto",
	  "degapseq" => "degapseq $ENV{EPURE}/../test/data/globins.msf -auto",
	  "dbigcg" => "dbigcg -dir $ENV{EPURE}/../test/embl -file '*.seq' -index . -id embl -db embl -auto",
	  "dbiflat" => "dbiflat -dir $ENV{EPURE}/../test/swiss -file seq.dat -index . -id swiss -db swiss -auto",
	  "dbifasta" => "dbifasta -dir $ENV{EPURE}/../test/wormpep -file wormpep -index . -id idacc -db worm -auto",
	  "dbiblast" => "dbiblast -dir $ENV{EPURE}/../test/wormpep -file wormpep -index . -db worm -auto",
	  "dan" => "dan tembl:hsfau -auto",
	  "cutseq" => "cutseq tembl:hsfau -from 20 -to 500 -auto",
	  "cusp" => "cusp tembl:hsfau -auto",
	  "cpgreport" => "cpgreport tembl:hsfau -auto",
	  "cpgplot" => "cpgplot tembl:hsfau -auto",
#	  "contacts" => "contacts -cpdb $ENV{EPURE}/../test/data -cpdbextn .pxyz -con $ENV{EPURE}/../test/data -conextn .con -thresh 1.0 -vdwf Evdw.dat -conerrf contacts.log",
	  "cons" => "cons $ENV{EPURE}/../test/data/globins.msf -auto",
	  "cutgextract" => "cutgextract $ENV{EPURE}/../test/data",
	  "compseq" => "compseq tsw:opsd_human -auto",
	  "coderet" => "coderet tembl:hsfau -auto",
	  "codcmp"  => "codcmp -auto",
	  "cirdna" => "cirdna -inp $ENV{EPURE}/../test/data/data.cirp -auto",
	  "chips" => "chips tembl:hsfau -auto",
	  "checktrans" => "checktrans $ENV{EPURE}/../test/data/paamir.pep -auto",
	  "charge" => "charge tsw:opsd_human -auto",
	  "chaos" => "chaos tembl:hsfau -auto",
	  "cai" => "cai tembl:hsfau -auto",
	  "banana" => "banana tembl:hsfau -auto",
	  "backtranseq" => "backtranseq tsw:opsd_human -auto",
	  "antigenic" => "antigenic tsw:opsd_human -auto",
	  "acdc" => "acdc embossversion",
	  "abiview" => "abiview $ENV{EPURE}/../test/data/abiview.abi -auto"
);

if (defined($ENV{EPURE})) {
  $purepath = "$ENV{EPURE}/";
}
else {
  $purepath = "";
}

@dotest = @ARGV;

foreach $name (@dotest) {
  if ($name =~ /^-(\S+)$/) {
    $arg = $1;
    if ($arg eq "all") {
      foreach $x (sort (keys (%tests))) {
	runtest($x);
      }
      exit;
    }
    elsif ($arg eq "list") {
      foreach $x (sort (keys (%tests))) {
	printf "%-15s %s\n", $x, $tests{$x};
      }
      exit;
    }
    else {
      print STDERR "Invalid argument $name (ignored)\n";
      next;
    }
  }
  runtest ($name);
}


exit();
