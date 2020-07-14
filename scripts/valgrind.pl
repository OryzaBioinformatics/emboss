#!/usr/local/bin/perl -w

# Runs valgrind, a Linux utility to check for memory leaks.
# Alternatives are third (3rd degree) on OSF
# or purify (purify.pl) on Irix and Solaris
#
# Valgrind is free software, with documentation online at
# http://devel-home.kde.org/~sewardj/docs/index.html
#
# Valgrind requires EMBOSS built with shared libraries

sub runtest ($) {
    my ($name) = @_;
    print "Run valgrind test $name\n";
    my $defbytes = 0;
    my $posbytes = 0;
    my $rembytes = 0;
    my $errcount = 0;
    my $timeout = 300;
    my $timealarm = 0;

    if (defined($tests{$name})) {
	if (defined($testcheck{$name})) {
	    $myvalgpath = "../../emboss/";
	}
	else {
	    $myvalgpath = $valgpath;
	}
	    print "Running valgrind $valgopts $myvalgpath$tests{$name}\n";

	eval {
	    $status = 0;
	    alarm($timeout);
	    $sysstat = system ("valgrind $valgopts $myvalgpath$tests{$name} 9> valgrind/$name.valgrind" );
	    alarm(0);
	    $status = $sysstat >> 8;
	};

	if ($@) {			# error from eval block
	    if ($@ =~ /memtest timeout/) {	# timeout signal handler
		$timealarm = 1;		# set timeout flag and continue
	    }
	    else {			# other signal - fail
		die;
	    }
	}
    
	if ($timealarm) {
	    print STDERR "Valgrind test $name timed out\n";
	    return -1;
	}

	open (TEST, "valgrind/$name.valgrind") ||
	    die "Failed to open valgrind/$name.valgrind";
	while (<TEST>) {
	    if (/definitely lost: +(\d+) bytes in (\d+) blocks/) {
		$defbytes=$1;
		#$defblock=$2;
	    }
	    if (/possibly lost: +(\d+) bytes in (\d+) blocks/) {
		$posbytes=$1;
		#$posblock=$2;
	    }
	    if (/still reachable: +(\d+) bytes in (\d+) blocks/) {
		$rembytes=$1;
		#$remblock=$2;
	    }
	    if (/ERROR SUMMARY: +(\d+) errors from (\d+) contexts/) {
		$errcount=$1;
		#$remblock=$2;
	    }
	}
	if ($status) {
	    print STDERR "Valgrind test $name returned status $status\n";
	}
	else {
	    if ($errcount){
		print STDERR "Valgrind test $name errors $errcount\n";
	    }
	    elsif ($defbytes){
		print STDERR "Valgrind test $name leak $defbytes (possibly $posbytes) bytes, still reachable $rembytes bytes\n";
	    }
	    elsif (defined($rembytes)) {
		print STDERR "Valgrind test $name OK (still reachable $rembytes bytes)\n";
	    }
	    else {
		print STDERR "Valgrind test $name OK (all clean)\n";
	    }
	}
	return $status;
    }
    else {
	print STDERR "ERROR: Unknown test $name \n";
	return -1;
    }

}

%tests = ();
%testcheck = ();

if (defined($ENV{EVALGRIND})) {
    $valgpath = $ENV{EVALGRIND}."/";
}
else {
    $valgpath = "";
}

open (MEMTEST, "../memtest.dat");
while (<MEMTEST>) {
    if (/^[#]/) {next}
    if (/(\S+) += +(\S.*)/) {
	$tests{$1}="$2";
    }
    if (/(\S+) +=test= +(\S.*)/) {
	$tests{$1}="$2";
	$testcheck{$1} = 1;
    }
}
close MEMTEST;

$valgopts = "--leak-check=yes --show-reachable=yes --num-callers=15 --verbose --logfile-fd=9 --error-limit=no --leak-resolution=high";
## --leak-check=yes       Test for memory leaks at end
## --show-reachable=yes   Show allocated memory still reachable
## --num-callers=15       Backtrace 15 functions - use more if needed
## --verbose              
## --logfile-fd=9         Write to file 9 for redirect (default 2 is stderr)
## --error-limit=no       Don't stop after 300 errors
## --leak-resolution=high Report alternate backtraces

@dotest = @ARGV;

$SIG{ALRM} = sub { print STDERR "+++ timeout handler\n"; die "memtest timeout" };
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
	elsif ($arg =~ /block=(\d+)/) {
	    $block=$1;
	    $i=0;
	    $blocksize=10;
	    $blockcount=0;
	    foreach $x (sort (keys (%tests))) {
		if (!$i) {
		    $blockcount++;
		}
		$i++;
		if ($i >= $blocksize) {$i=0}
		if ($blockcount == $block) {
		    runtest($x);
		}
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
