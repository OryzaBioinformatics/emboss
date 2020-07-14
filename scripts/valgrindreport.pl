#!/usr/local/bin/perl -w

open (DAT, "../memtest.dat") || die "Cannot open file memtest.dat";
%cmd = ();

while (<DAT>) {
    if (/^[\#]/) {next}
    if (/^(\S+) = (.*)/) {$cmd{$1} = $2};
    if (/^(\S+) =test= (.*)/) {$cmd{$1} = $2};
}

while (<>) {
    if (/^Valgrind test (\S+)/) {
	$name = $1;
	$leak = $status = 0;
	if (/^Valgrind test (\S+) OK/) {next}
	elsif (/^Valgrind test (\S+) returned status (\d+)/) {
	    $status = $2;
	    print "\n";
	    print "$name status $status\n";
	    print "\% $cmd{$name}\n";
	    print "$txt";
	}
	elsif (/^Valgrind test (\S+) leak ([^,]+)/) {
	    $leak = $2;
	    print "\n";
	    print "$name leak $leak\n";
	    print "\% $cmd{$name}\n";
	}
	$txt = "";
    }
    else {$txt .= $_}
}
