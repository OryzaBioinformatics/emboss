#!/usr/bin/perl -w

$errfile = 0;
$errfunc = 0;
$errcount = 0;
$errtotcount = 0;
$totcount = 0;
$totfile = 0;
$filelib = "unknown";
$filename = "unknown";

open (LOG, ">embossdocreport.log") || die "Cannot open embossdocreport.log";

%badfiles = ();
%badtotfiles = ();

while (<>) {
    $newfunc = 0;
    $newfile = 0;
    if (/^Function (\S+)/) {
	$funcline = $_;
	$funcname = $1;
	$newfunc = 1;
    }
    elsif (/^Static function (\S+)/) {
	$funcline = $_;
	$funcname = $1;
	$newfunc = 1;
    }
    elsif (/^Macro (\S+)/) {
	$funcline = $_;
	$funcname = $1;
	$newfunc = 1;
    }
    elsif (/^Data type (\S+)/) {
	$funcline = $_;
	$funcname = $1;
	$newfunc = 1;
    }
    elsif (/^Static data type (\S+)/) {
	$funcline = $_;
	$funcname = $1;
	$newfunc = 1;
    }

    elsif (/^Typedef data type (\S+)/) {
	$funcline = $_;
	$funcname = $1;
	$newfunc = 1;
    }

    if (/^set pubout \'([^\']+)\' lib \'([^\']+)\'/) {
	$newfileline = $_;
	$newfilename = $1;
	$newfilelib = $2;
	$newfile = 1;
	if (/ type \'([^\']+)\'/) {
	    $newfiletype = $1;
	}
	else {
	    $newfiletype = "";
	}
    }

    if ($newfunc || $newfile) {
	if ($errcount) {
	    $errtotcount += $errcount;
	    $totcount += $errcount;
	    $errfunc++;
	    $errfile++;
	$errcount = 0;
	}
    }
    if ($newfile) {
	if ($errfile) {
	    $badfiles{$filelib."_".$filename}=$errfile;
	    $badtotfiles{$filelib."_".$filename}=$errtotcount;
	    $totfile++;
	}
	$errfile = 0;
	$errtotcount = 0;
	$filename = $newfilename;
	if ($newfiletype ne "") {
	    if ($newfiletype eq "include") {$filename .= ".h"}
	    elsif ($newfiletype eq "source") {$filename .= ".c"}
	    else {$filename .= ".$newfiletype"}
	}
	$filelib = $newfilelib;
    }


    if (/^bad/) {
	if (!$errcount) {
	    if (!$errfile) {
		print LOG "=============================\n";
		print LOG $newfileline;
	    }
	    print LOG "  ".$funcline;
	}
	$errcount++;
	print LOG "    ".$_;
    }
}

if ($errcount) {
    $totcount += $errcount;
    $errfunc++;
    if (!$errfile) {
	$totfile++;
    }
    $errfile++;
}

if ($errfile) {$badfiles{$filelib."_".$filename}=$errfile}

foreach $x (sort (keys (%badfiles))) {
    printf "%4d %4d %s\n", $badtotfiles{$x}, $badfiles{$x}, $x;
}

print STDERR "$totcount errors in $errfunc functions in $totfile files\n";
close LOG;
