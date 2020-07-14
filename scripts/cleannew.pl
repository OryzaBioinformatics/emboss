#!/usr/bin/perl -w

@validfile = ("acdvalid.txt", "acdvalidreport.txt",
	      "myall.csh", "myconfig.csh", "mydoc.csh", "myembassyall.csh",
	      "myembassyconfig.csh",
	      "src-embassy", "summary.log", "x",
	      );

@validbase = ( "aclocal.m4", "autom4te.cache",
	      "config.cache", "config.log", "config.status",
	      "configure", "configure.lineno", "libtool", "so_locations",
	       "Makefile", "Makefile.in"
	       );
@memtestfiles = ( "hsfau.diffseq", "HSFAU.diffgff", "HSFAU1.diffgff",
		  "hsfau.fasta", "hsfau.merger", "division.lkp",
		  "entrynam.idx", "acnum.trg", "acnum.hit", "Eamino.dat",
		  "valgrind.txt", "valgrind.summary", "valgrind.result",
		  "valgrind.out", "valgrind.err"
		  );

%knownfile = ();
%knownbase = ();
%progs = ();

foreach $x (@validfile) { $knownfile{$x} = 1 }
foreach $x (@validbase) { $knownbase{$x} = 1 }
foreach $x (@memtestfiles) { $memtest{$x} = 1 }

open (VERS, "embossversion -full -auto|") || die "Cannot run embossversion";
while (<VERS>) {
#    if(/InstallDirectory: +(\S+)/) {$installdir = $1}
    if(/BaseDirectory: +(\S+)/) {$basedir = $1}
}
close VERS;

open (PROGS, "wossname -alpha -auto|") || die "Cannot run wossname";

while (<PROGS>) {
    if(/^([a-z]\S+)\s+(\S+)/) { $progs{$1} = $2; }
}
close PROGS;

$check = 0;
open (CHECK, "$basedir/emboss/Makefile.am") || die "Cannot open emboss/Makefile.am";
while (<CHECK>) {
    if (/^check_PROGRAMS/) {$check = 1;}
    if ($check) {
	if (!/\S/) {$check = 0;}
    }
    if (!$check) {next}
    foreach $x (split) {
	if ($x eq "check_PROGRAMS") {next}
	if ($x eq "=") {next}
	if ($x eq "\\") {next}
#	print "x: $x\n";
	$progs{$x} = "check";
    }
}
close CHECK;

$progs{"jembossctl"} = "jemboss"; # no ACD file, missed by wossname

# Process the input file (cvs update output)
# Look for lines that start with '?'
# check for files that could reasonably be there
# report everything else as a directory (d) or file.

while (<>) {
    if (!/^[?] (\S+)/) {next}
    $file = $1;
    if($file =~ /[\/]([^\/]+)$/) { $base = $1 }
    else { $base = $file }
#    print "$file '$base'\n";
    if(defined($knownfile{$file})) {next}
    if(defined($knownbase{$base})) {next}
    if ($file =~ /[.]o$/) {next}
    if ($file =~ /^test\/qa\/\S+/) {
	if (-d "$basedir/$file") {next}
    }
    if ($file =~ /^jemboss\/org\/emboss\/jemboss\/\S+/) {next}
    if ($file =~ /^test\/memtest\/valgrind\/\S+/) {next}
    if ($file =~ /^test\/memtest\/(\S+)/) {
	if(defined($memtest{$1})) {next}
    }
    if ($base =~ /^Makefile$/) {next}
    if ($base =~ /^Makefile[.]in$/) {next}
    if ($base =~ /^[.]deps$/) {next}
    if ($base =~ /^[.]libs$/) {next}
    if($base =~ /[.]lo$/) {next}
    if($base =~ /[.]la$/) {next}
    if ($file =~ /^emboss\/([^\/]+)$/) {
	if(defined($progs{$1})) {next}
    }
    if ($file =~ /^embassy\/[^\/]+\/src\/([^\/]+)$/) {
	if(defined($progs{$base})) {next}
    }
    if ($file =~ /^embassy\/[^\/]+\/source\/([^\/]+)$/) {
	if(defined($progs{$base})) {next}
    }
#    print "$file '$base'\n";
    if (-d "$basedir/$file") {
	print "d $file\n";
	next;
    }
    print;
}
