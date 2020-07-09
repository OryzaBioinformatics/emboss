#!/usr/local/bin/perl -w

##########################
# THINGS TO DO
##########################
# Timeout (TI line to set) default 60secs, diffseq needs more
# Preprocess (PP line) command
# Postprocess (QQ line) command - e.g. testing database builds, reusing output
#
##########################
# THINGS TO DO
##########################
#
# EMBOSS_RC variable to read and extra .embossrc file (for new test dbs)
#

# EMBOSS QA Test Processing
#
# Test line types:
# ID Test name (for the test directory)
# AP Application name (for the command line and for statistics)
# CL Command line (rest of the command line)
# ER error return code
# ## Comment (required double #)
# CC Comment (used in commenting on failed tests, e.g. "Requires SRS"
# DL success or all or keep - whether to delete the files afterways
# IN Line(s) of standard input
# FI File name (stdout and stderr assumed to exist and be empty unless stated)
# FP File pattern - /regexp/ to be found. Optional count to check exact number.
# FZ [<=>]number File size test. Implicit test for zero size stdout/stderr unless stated
# FC [<=>]number File linecount test
# // End of test entry
# 
# Return codes: see %retcode definition
#
#
# Note:
# timeout fails if the program reaches EOF on stdin
# apparently because it waits for the user to enter something when there
# is no piped input. Fixed by always providing piped stdin, usually empty.
# 

###################################################################
# runtest
#
# parses the test definition, runs the test, check the results
# stores error reports for the caller
###################################################################

sub runtest ($) {

  my ($testdef) = @_;		# we pass in the full definition
  # print $testdef;
  my $ifile = 0;
  my $ipatt = 0;
  my $ret = 0;
  my $odata = "";
  my $ip = $iq = 0;
  my $i = $j = $k = 0;
  my $testerr = "";
  my $cmdline = "";
  my $testret = 0;
  my $testid = "";
  my $testin = "";
  my $timeout = $timeoutdef;
  my $ppcmd = "";
  my $qqcmd = "";

# these are globals, used by the caller

  $globaltestdelete=$defdelete;	# global cleanup of test directory
  $globalcomment = ""; # global comment in case of expected failure

# parse the test definition (EMBL-style 2 character prefixes)

  foreach $line  (split (/^/, $testdef)) {
    ###print "<$line>\n";
    chomp $line;

# first line of the definition - initialise variables

    if ($line =~ /^ID\s+(\S+)/) {
      $testid = $1;
      $testin = "";
      $cmdline = "";
      $ifile=0;
      print LOG "Test <$testid>\n";
      $sysstat = system( "rm -rf $testid");
      $status = $sysstat >> 8;
      if ($status) {
	$testerr = "failed to delete old directory $testid, status $status\n";
	print STDERR $testerr;
	print LOG $testerr;
      }
      mkdir ("$testid", 0777);
      open (SAVEDEF, ">$1/testdef") || die "Cannot save $1/testdef";
      print SAVEDEF $testdef;
      close (SAVEDEF);
    }

# other lines

    elsif ($line =~ /^\#\#/) {next}
    elsif ($line =~ /^CC\s*(.*)/) {$globalcomment .= "** $1\n"}
    elsif ($line =~ /^TI\s+(\d+)/) {$timeout = $1}
    elsif ($line =~ /^ER\s+(\d+)/) {$testret = $1}
    elsif ($line =~ /^AP\s+(\S+)/) {$testapp = $1}
    elsif ($line =~ /^DL\s+(success|keep|all)/) {$globaltestdelete = $1}
    elsif ($line =~ /^PP\s*(.*)/) {$ppcmd .= "$1 ; "}
    elsif ($line =~ /^QQ\s*(.*)/) {$qqcmd .= " ; $1"}
    elsif ($line =~ /^IN\s*(.*)/) {$testin .= "$1\n"}
    elsif ($line =~ /^CL\s+(.*)/) {
      if ($cmdline ne "") {$cmdline .= " "}
      $cmdline .= $1;
    }

# filename - must be unique

    elsif ($line =~ /^FI\s+(\S+)/) {
      $filename = $1;
      if (defined($outfile{$filename})) {
	$testerr = "$retcode{16} $testid/$filename\n";
	print STDERR $testerr;
	print LOG $testerr;
	return 16;
      }
      $outfile{$filename} = $ifile;
      $outfilepatt{$ifile} = $ipatt;
      print LOG "Known file [$ifile] <$1>\n";
      $ifile++;
      $filezero{$ifile} = 0;
    }

# file pattern(s) - can be many for each file

    elsif ($line =~ /^FP\s+((\d+)\s+)?(([si]+)\s+)?\/(.*)\/$/) {
      if (defined($2)) {$patcount{$ipatt}=$2}
      if (defined($4)) {$patcode{$ipatt}=$4}
      $pat = $5;
      $pattest{$ipatt} = $pat;
      $ipatt++;
    }

# line count - maximum one for each file

    elsif ($line =~ /^FC\s+([<>=]\s*\d+)/) {
      $countpatt = $1;
      if (defined($outcount{$ifile-1})) {
	$testerr = "$retcode{14} $testid/$filename\n";
	print STDERR $testerr;
	print LOG $testerr;
	return 14;
      }
      $outcount{$ifile-1} = $countpatt;
      if ($countpatt =~ /^[=]\s*0$/) {$filezero{$ifile-1}=1}
    }

# file size (greater/less/equal) - maximum one for each file

    elsif ($line =~ /^FZ\s+([<>=]\s*\d+)/) {
      $sizepatt = $1;
      if (defined($outsize{$ifile-1})) {
	$testerr = "$retcode{15} $line: $line\n";
	print STDERR $testerr;
	print LOG $testerr;
	return 15;
      }
      $outsize{$ifile-1} = $sizepatt;
      if ($sizepatt =~ /^[=]\s*0$/) {$filezero{$ifile-1}=1}
    }

# end of test definition

    elsif ($line =~ /^\/\/$/) {next}

# fall through for any unknown lines (bad prefix, or failed to match regexp)

    else {
      $testerr = "$retcode{1} $line: $line\n";
      print STDERR $testerr;
      print LOG $testerr;
      return 1;
    }
  }

# cd to the test directory (created when ID was parsed)

  chdir $testid;

# set up stdin always
# we need to hit EOF if it tries to read when there is no input

  open (TESTIN, ">stdin");
  if ($testin ne "") {
    print TESTIN $testin;
  }
  close TESTIN;
  $stdin = "< stdin";

  $timealarm=0;
  $starttime = time();

# run the test with a timeout (default 60 seconds) to catch infinite loops
# The easiest infinite loop is an unexpected prompt, which waits on stdin

  eval {
    alarm($timeout);
    $sysstat = system("$ppcmd $testapp $cmdline > stdout 2> stderr $stdin $qqcmd");
    alarm(0);
    $status = $sysstat >> 8;
  };


  if ($@) {			# error from eval block
    if ($@ =~ /qatest timeout/) {	# timeout signal handler
      $timealarm = 1;		# set timeout flag and continue
    }
    else {			# other signal - fail
      die;
    }
  }

# Report any timeout

  if ($timealarm) {
    $testerr = "$retcode{11} ($timeout secs) '$testapp $cmdline $stdin', status $status/$testret\n";
    print STDERR $testerr;
    print LOG $testerr;
    chdir ("..");
    return 11;
  }

# report any failed system call

  elsif ($status) {
    if ($status != $testret) {
      $testerr = "$retcode{2} '$testapp $cmdline $stdin', status $status/$testret\n";
      print STDERR $testerr;
      print LOG $testerr;
      chdir ("..");
      return 2;
    }
  }

# report any run that succeeded where a failure was expected

  else {
    if ($testret) {
      $testerr = "$retcode{5} '$testapp $cmdline $stdin', status $status/$testret\n";
      print STDERR $testerr;
      print LOG $testerr;
      chdir ("..");
      return 5;
    }
  }

# We have a sucessful run, no timeouts.
# Check it did what we wanted it to

# Note the run time

  $endtime = time();
  $runtime = $endtime - $starttime;

# Check for a core dump

  if (-e "core") {
    $testerr = "$retcode{12} $testid\n";
    print STDERR $testerr;
    print LOG $testerr;
    chdir ("..");
    return 12;
  }

# Read the file names in the test directory

  opendir (DIR, ".");
  @allfiles = readdir(DIR);
  closedir DIR;

# Check the other files in the test directory

  foreach $file (@allfiles) {
    if ($file eq ".") {next}	# current directory
    if ($file eq "..") {next}	# parent directory
    if ($file eq "stdin") {next} # stdin we created
    if ($file eq "testdef") {next} # test definition

# stdout and stderr are present (system call creates them)
# and expected to be empty unless the test definition say otherwise
# this tests they are empty if they are not defined
# otherwise they fall through to normal file testing

    if ($file eq "stdout" || $file eq "stderr") {
      if (!defined($outfile{$file})){ # not in test definition
	$size = -s $file;
	print LOG "Test empty file $testid/$file\n";
	if (testnum("=0", $size)) {
	  $testerr = "$retcode{6} $testid/$file\n";
	  print STDERR $testerr;
	  print LOG $testerr;
	  chdir ("..");
	  return 6;
	}
	next;
      }
    }

# This file was not defined

    if (!defined($outfile{$file})) {
      $testerr = "$retcode{3} <$testid/$file>\n";
      print STDERR $testerr;
      print LOG $testerr;
      chdir ("..");
      return 3;
    }

# File is defined, check its properties

    else {
      $i =  $outfile{$file};
      print LOG "file [$i] <$file>\n";

# File size defined - test it

      if (defined($outsize{$i})) {
	$size = -s $file;
	print LOG "Test size $size '$outsize{$i}' $testid/$file\n";
	if (testnum($outsize{$i}, $size)) {
	  $testerr = "$retcode{7} $size '$outsize{$i}' $testid/$file\n";
	  print STDERR $testerr;
	  print LOG $testerr;
	  chdir ("..");
	  return 7;
	}
      }

# read the output file - to test patterns and line count

      open (OFIL, $file) || die "Cannot open $testid/$file";
      $odata = "";
      $linecount=0;
      while (<OFIL>) {$odata .= $_; $linecount ++;}
      close OFIL;

# Test line count (FC)

      if (defined($outcount{$i})) {
	print LOG "Test linecount $linecount '$outcount{$i}' $testid/$file\n";
	if (testnum($outcount{$i}, $linecount)) {
	  $testerr = "$retcode{8} $linecount '$outcount{$i}' $testid/$file\n";
	  print STDERR $testerr;
	  print LOG $testerr;
	  chdir ("..");
	  return 8;
	}
      }

# check how many patterns were defined for this file
# first is indicated by $outfilepatt
# total number is given by $outfilepatt for the next file
# (or total for last file)

      $ip = $outfilepatt{$i};
      $j = $i + 1;
      if ($j >= $ifile) {$iq = $ipatt}
      else {$iq = $outfilepatt{$j}}
      ###print LOG "Patterns $ip .. ", $iq-1, "\n";

# test whether we should know about file patterns
# we ignore stderr, which can contain user prompts
# but we test stdout which should be empty or have output

      if ($ip >= $iq) {
	if (!$filezero{$i}) {
	  if ($file ne "stderr") {
	    $testerr = "$retcode{17} $testid/$file\n";
	    print STDERR $testerr;
	    print LOG $testerr;
	    chdir ("..");
	    return 17;
	  }
	}
      }

# loop through each pattern testing the saved data (in $odata)

      for ($k=$ip; $k < $iq; $k++) {

# Some patterns have a trailing code s (\n matching '.') i (case insensitive)
# Not clear whether these work correctly in perl 5.005
# these are compiled with qr//mi and so on

	if (defined($patcode{$k})) {
	  ##print STDERR "special /m$patcode{$k} pattern '$pattest{$k}'\n";
	  if ($patcode{$k} eq "s") {$qpat = qr/$pattest{$k}/ms}
	  elsif ($patcode{$k} eq "i") {$qpat = qr/$pattest{$k}/mi}
	  elsif ($patcode{$k} eq "is") {$qpat = qr/$pattest{$k}/mis}
	  elsif ($patcode{$k} eq "si") {$qpat = qr/$pattest{$k}/mis}
	}

# simple pattern, compile using qr//m

	else {
	  ##print STDERR "standard /m pattern '$pattest{$k}'\n";
	  $qpat = qr/$pattest{$k}/m;
	}

# We need to check how often the pattern was found

	if (defined($patcount{$k})) {

# We want to find the pattern exactly $pcount times
# (though $pcount can be zero)

	  $pcount = $patcount{$k};
	  ##print STDERR "Test pattern [pat $k] '$pattest{$k}' ($pcount times) $testid/$file\n";
	  print LOG "Test pattern [pat $k] '$pattest{$k}' ($pcount times) $testid/$file\n";

# Count the pattern

	  $pc = 0;
	  while ($odata =~ /$qpat/g) {
	    $pc++;
	  }

# We did not want to find it (defined as count=0)

	  if ($pc && !$pcount) {
	    print LOG "$retcode{9} [pat $k] '$pattest{$k}' $testid/$file\n";
	    $testerr = "$retcode{9} [pat $k] '$pattest{$k}' $testid/$file\n";
	    print STDERR $testerr;
	    print LOG $testerr;
	    chdir ("..");
	    return 9;
	  }

# We should find it $pcount times

	  elsif ($pc != $pcount) {
	    print LOG "$retcode{10} [pat $k] '$pattest{$k}' found $pc/$pcount times $testid/$file\n";
	    $testerr = "$retcode{10} [pat $k] '$pattest{$k}' found $pc/$pcount times $testid/$file\n";
	    print STDERR $testerr;
	    print LOG $testerr;
	    chdir ("..");
	    return 10;
	  }
	}

# We only need to check whether the pattern exists

	else {
	  print LOG "Test pattern [pat $k] '$pattest{$k}' $testid/$file\n";
	  ###print STDERR "Test pattern [pat $k] '$pattest{$k}' $testid/$file\n";
	  if ($odata !~ $qpat) {
	    print LOG "$retcode{4} [pat $k] '$pattest{$k}' $testid/$file\n";
	    print LOG "\$odata: '\n$odata'\n";
	    $testerr = "$retcode{4} [pat $k] '$pattest{$k}' $testid/$file\n";
	    print STDERR $testerr;
	    print LOG $testerr;
	    chdir ("..");
	    return 4;
	  }
	}

# done with checking this pattern

      }

# done with checking all patterns

    }

  }

# done with checking all files in the test directory

# write status information to testlog file in the test directory

  open (TESTLOG, ">testlog") || die "Cannot open $testid/testlog";
  print TESTLOG $testerr;
  print TESTLOG "Runtime $runtime seconds\n";
  close TESTLOG;
  chdir "..";

# return the return code (zero, because we return the failure codes above)

  return $ret;
}


#########################################################################
# testnum
#
# test greater/less/equal for file linecount (FC) and size (FZ) tests
# $test is "> nnn" "< nnn" or "= nnn"
# $val is the actual number to test against
# Returns 0 for success, 1 for failure
#########################################################################

sub testnum ($$) {
  my ($test, $val) = @_;
  my ($oper, $num) = ($test =~ /([<>=])\s*(\d+)/);
  if ($oper eq "=") {
    if ($val == $num) {return 0}
  }
  elsif ($oper eq ">") {
    if ($val > $num) {return 0}
  }
  elsif ($oper eq "<") {
    if ($val < $num) {return 0}
  }

  return 1;
}

#########################################################################
# MAIN PROGRAM
#
# For each test in qatest.dat, call runtest
#########################################################################

$id = "";
$lastid = "";
$testdef = "";
$tcount=0;
$tfail=0;

$timeoutdef=60;			# default timeout in seconds
$defdelete="success";		# success, all, keep
$globaltestdelete=$defdelete;
$globalcomment = "";

$allstarttime = time();

$SIG{ALRM} = sub { print STDERR "+++ timeout handler\n"; die "qatest timeout" };

# predefined return codes for the runtest function
# '99' is only a placeholder for ease of inserting new codes

%retcode = (
	    "1" => "Bad definition line ",
	    "2" => "Failed to run",
	    "3" => "Unknown file",
	    "4" => "Failed pattern",
	    "5" => "Unexpected success",
	    "6" => "Not empty file",
	    "7" => "Failed size",
	    "8" => "Failed linecount",
	    "9" => "Failed unwanted pattern",
	    "10" => "Failed counted pattern",
	    "11" => "Timeout",
	    "12" => "CORE DUMP",
	    "13" => "Duplicate test id",
	    "14" => "Duplicate file linecount",
	    "15" => "Duplicate test file size",
	    "16" => "Duplicate filename definition",
	    "17" => "No patterns to test file contents",
            "99" => "Testing"
);

# The relative path ios fixed, as are the paths of files in the qatest.dat
# file, so best to keep everything running in the test/qa directory

open (IN, "../qatest.dat") || die "Cannot open qatest.dat";
open (LOG, ">qatest.log") || die "Cannot open qatest.log";

# make qatest.log unbuffered and be sure to reset the current filehandle
$fh = select LOG; $|=1; select $fh;

while (<IN>) {

# Save a test when we reach an ID line (ignore any comments in between tests)

  if (/^ID\s+(\S+)/) {
    $lastid = $id;
    $id = $1;
    $testdef = "";
  }
  $testdef .= $_;

# end of definition - fire up the test

  if (/^\/\//) {
    $tcount++;
    $result = runtest ($testdef);

# check the results

# (1) look out for duplicate tests - which overwrite a previous directory

    if (defined($saveresult{$id})) {
      print STDERR "$id duplicate test name\n";
      print LOG "$id duplicate test name\n";
      if (!$result) {$result = 13}
    }

    $saveresult{$id} = $result;

# (2) $result is 0 for success, or a code in %retcode

    if ($result) {

# test definitions can have CC comments for expected failures (e.g. SRS needed)

      if ($globalcomment ne "") {
	print STDERR "$globalcomment";
	print LOG "$globalcomment";
      }

# Report the error code (to log and to user)

      print STDERR "$id test failed code $result $retcode{$result}\n\n";
      print LOG "$id test failed code $result $retcode{$result}\n";
      $tfail++;

# Usually we keep failed tests (unless delete is set to 'all')

      if ($globaltestdelete eq "all") {
	$sysstat = system( "rm -rf $id");
	$status = $sysstat >> 8;
	if ($status) {
	  $testerr = "failed to delete old directory $id, status $status\n";
	  print STDERR $testerr;
	  print LOG $testerr;
	}
      }

    }

# (3) successful completion

    else {

# Note to log (silence to user)

      print LOG "test $id success\n";

# usually we delete sucessful results (unless delete is set to 'keep')

      if ($globaltestdelete ne "keep") {
	$sysstat = system( "rm -rf $id");
	$status = $sysstat >> 8;
	if ($status) {
	  $testerr = "failed to delete old directory $id, status $status\n";
	  print STDERR $testerr;
	  print LOG $testerr;
	}
      }

    }

# new line in the log file before the next test

    print LOG "\n";

# clear any global hashes befoire defining the next test

    undef %outfile;
    undef %outfilepatt;
    undef %pattest;
    undef %patcount;
    undef %patcode;
    undef %outcount;
    undef %outsize;
    undef %filezero;

  }
}

# Final summary

$tpass = $tcount - $tfail;
$allendtime = time();
$alltime = $allendtime - $allstarttime;

print STDERR "Tests total: $tcount pass: $tpass fail: $tfail\n";

print STDERR "Time: $alltime seconds\n";
print LOG "Time: $alltime seconds\n";

exit;