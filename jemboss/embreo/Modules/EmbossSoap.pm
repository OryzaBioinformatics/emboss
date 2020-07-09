package EmbossSoap;

use POSIX qw(strftime);

require ParseSoapOptions;

#
# standard defines
#

my $EMBDIR = "/packages/emboss/STABLE/";
my $EMBBINDIR = "$EMBDIR"."bin/";
my $ACDDIR = "$EMBDIR"."share/EMBOSS/acd/";
my $logname = $ENV{'REMOTE_USER'} || getpwuid($<) || "nobody";
my $TMPROOT = "/data/tmp/SOAP/emboss/".$logname;

#
# standard methods
#

sub name {
    return "The EMBOSS Application Suite";
}
sub appversion {
    my $embexe = "$EMBBINDIR".embossversion;
    my $ver = `$embexe`;
    return "Emboss version $ver";
}
sub about {
    return "A facility to run any EMBOSS application.";
}
sub pubservice {
    return "Y";
}
sub helpurl {
    return "";
}
sub abouturl {
    return "http://www.uk.embnet.org/Software/EMBOSS/";
}
sub docurl {
    return "";
}
sub servicedesc {
    $desc{'name'}=&name;
    $desc{'appversion'}=&appversion;
    $desc{'about'}=&about;
    $desc{'pubservice'}=&pubservice;
    $desc{'helpurl'}=&helpurl;
    $desc{'abouturl'}=&abouturl;
    $desc{'docurl'}=&docurl;
    $desc{'embreo.debug'}="true";
    return (%desc);
}

#
# regular methods
#

sub show_acd {
    my ($class,$prog) = @_;
    $returndata{'status'}="0";
    $returndata{'msg'}="OK";
    my $embexe = "$EMBBINDIR"."$prog";
    if (-x $embexe) {
	if (! -d $TMPROOT) {
	    mkdir("$TMPROOT");
	}
	chdir("$TMPROOT");
	system("$embexe -acdpretty");
	my $outdata="";
	open (OFILE, "<$prog.acdpretty");
	while (<OFILE>)
	{
	    $outdata .= $_;
	}
	close OFILE;
	$returndata{'acd'}=$outdata;
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="Application not found";
    }
    return (%returndata);
}

sub show_progs {
    my ($class,$args) = @_;
    $returndata{'status'}="0";
    $ENV{'PATH'}="/usr/bin:$EMBBINDIR:/packages/clustal";
    $returndata{'msg'}="basic";
    if ( $args eq "extended" ) {
	$returndata{'msg'}="extended";
	open FF, "$EMBBINDIR/wossname -alphabetic -auto | /bin/grep -v 'ALPHABETIC' |";
	while (<FF>) {
	    ($progname,$rest) = split(" ",$_,2);
	    chomp $progname;
	    chomp $rest;
	    if(length($progname)>0){
		$returndata{$progname} = "$rest";
	    }
	}
	close FF;
    } elsif ( $args eq "wossname" ) {
	$returndata{'wossname'} = `$EMBBINDIR/wossname -alphabetic -auto`;
    } else {
	my $acdproglist = `$EMBBINDIR/wossname -alphabetic -auto | /bin/grep -v 'ALPHABETIC' | /bin/cut -d' ' -f1`;
	chomp $acdproglist;
	$returndata{'proglist'}="$acdproglist";
    }
    return (%returndata);
}

#
# a list of applications ordered by category
# if there is no appropriate or meaningful taxonomy, return non-zero status
# the msg is used as the title of the root of the taxonomy
#
# extended format uses a : to separate hierarchical menus
#
sub list_groups {
    my ($class,$args) = @_;
    $returndata{'status'}="0";
    $returndata{'msg'}="EMBOSS";
    $ENV{'PATH'}="/usr/bin:$EMBBINDIR:/packages/clustal";
    my $grpname="";
    if ( $args eq "extended" ) {
	open FF, "$EMBBINDIR/wossname -colon -auto |";
    } elsif ( $args eq "wossname" ) {
	$returndata{'wossname'} = `$EMBBINDIR/wossname -colon -auto`;
	return (%returndata);
    } else {
	open FF, "$EMBBINDIR/wossname -auto |";
    }
    while (<FF>) {
	if (/^[A-Z][A-Z0-9: ]*$/) {
	    $grpname=$_;
	    chomp $grpname;
	    $returndata{$grpname} = "";
	} else {
	    if (!/^$/) {
		($prog1,$rest) = split(" ",$_,2);
		chomp $prog1;
		$returndata{$grpname} .= " $prog1";
	    }
	}
    }
    close FF;
    return (%returndata);
}

sub show_help {
    my ($class,$prog) = @_;
    my $embexe = "$EMBBINDIR"."tfm";
    $returndata{'status'}="0";
    $returndata{'msg'}="OK";
    $ENV{'PATH'}="/usr/bin:$EMBBINDIR:/packages/clustal";
    #
    # produce html output once tfm does it correctly
    # and once the client can actually handle it
    #
    #my $helptext = `$embexe -html -auto -nomore $prog`;
    #$helptext =~ s#<!--.*?-->##gis;
    my $helptext = `$embexe -auto -nomore $prog`;
    $returndata{'helptext'}="$helptext";
    return (%returndata);
}

#
# this one is emboss-specific
#
sub show_db {
    my ($class,$prog) = @_;
    my $embexe = "$EMBBINDIR"."showdb";
    $returndata{'status'}="0";
    $returndata{'msg'}="OK";
    $ENV{'PATH'}="/usr/bin:$EMBBINDIR:/packages/clustal";
    #
    my $helptext = `$embexe -auto`;
    $returndata{'showdb'}="$helptext";
    return (%returndata);
}

sub run_prog {
    #my ($class,$prog,$args,%inputfiles) = @_;
    my ($class,$prog,$args,$inhash) = @_;
    my %inputfiles = %{$inhash};
    my %soapopts = &ParseSoapOptions::parse_soap_options($args);
    #foreach my $optkey (keys %soapopts) {
    #	print $optkey, ":", $soapopts{$optkey},"\n";
    #}
    my $embexe = "$EMBBINDIR"."$prog";
    umask 0077;
    if (! -d $TMPROOT) {
	mkdir("$TMPROOT");
    }
    chdir("$TMPROOT");
    #
    # make a name based on the app, date, and something random
    #
    ($progname,$rest) = split(" ",$prog,2);
    my $newtmpstr = sprintf "%.32g", int(rand(100000000));
    my $now_date = strftime "%b-%d-%Y-%T", localtime;
    my $newtmpdir = "$progname"."."."$now_date"."."."$newtmpstr";
    mkdir("$newtmpdir");
    chdir("$newtmpdir");
    #
    # parse the input command
    # disallow absolute command pathnames
    # and pipes or multiple commands
    #
    if ( $progname =~ m#/#) {
	$returndata{'status'}="1";
	$returndata{'msg'}="Invalid command name $progname";
	return (%returndata);
    }
    if ( $prog =~ m#;|\||\`#) {
	$returndata{'status'}="1";
	$returndata{'msg'}="Invalid command $prog";
	return (%returndata);
    }
    $returndata{'cmd'}="$prog\nSOAP parameters: $args";
    my $safe_prog = $prog;
    #
    $now_string = localtime;
    #
    # we were passed some input files, create them
    #
    # save list of input files in .infiles
    # save description of this run in .desc
    #
    # it's up to the client to pass us a sane URL
    # if we get a bad one, we fail rather than try to fix it
    #
    open(DESC,">.desc");
    print DESC "EMBOSS run details\n\n";
    print DESC "Application: $progname\nArguments: $rest\n\n";
    print DESC "Started at $now_string\n\nInput files:\n";
    open(IFILELIST,">.infiles");
    foreach my $input_file (keys %inputfiles) {
	my $safe_file = $input_file;
	if ( $safe_file =~ m#/|\s|;|\||\`#) {
	    $returndata{'status'}="1";
	    $returndata{'msg'}="Invalid file name $safe_file";
	    return (%returndata);
	}
	open(IFILE, ">$safe_file");
	print IFILE $inputfiles{$input_file};
	close IFILE;
	print IFILELIST "$safe_file\n";
	print DESC "$safe_file\n";
    }
    close IFILELIST;
    close DESC;
    $returndata{'cmd'}.="\n$safe_prog";
    $returndata{'status'}="0";
    $returndata{'msg'}="OK";
    #
    # at this point we switch between batch and interactive
    # create a script file in both cases
    #
    open(SCRIPTFILE,">.scriptfile");
    print SCRIPTFILE "#!/usr/local/bin/perl5\n";
    print SCRIPTFILE "#\n# EmbossSoap command script\n";
    print SCRIPTFILE "#\n# Automatically generated $now_string\n#\n";
    print SCRIPTFILE "\n";
    print SCRIPTFILE "\$ENV{'PATH'}=\"/usr/bin:$EMBBINDIR:/packages/clustal\";\n";
    print SCRIPTFILE "\$ENV{'PLPLOT_LIB'}=\"/packages/emboss/STABLE/share/EMBOSS/\";\n";
    print SCRIPTFILE "\$ENV{'EMBOSS_DATA'}=\"/packages/emboss/STABLE/share/EMBOSS/\";\n";
    print SCRIPTFILE "\n";
    $currentdir = `/bin/pwd`;
    chomp $currentdir;
    print SCRIPTFILE "chdir(\"$currentdir\");\n";
    print SCRIPTFILE "\n";
    print SCRIPTFILE "system(\"$safe_prog 1>stdout 2>stderr\");\n";
    print SCRIPTFILE "\n";
    print SCRIPTFILE "\$exit_code = \$? >> 8;\n";
    print SCRIPTFILE "\n";
    print SCRIPTFILE "if (\$exit_code == 0) { unlink(\"stderr\");}\n";
    print SCRIPTFILE "\n";
    print SCRIPTFILE "\$now_string = localtime;\n";
    print SCRIPTFILE "open(IFILE, \">.finished\");\n";
    print SCRIPTFILE "print IFILE \"\$now_string\";\n";
    print SCRIPTFILE "close IFILE;\n";
    print SCRIPTFILE "\n";
    close SCRIPTFILE;
    #
    # if batch, submit into the queue
    #
    if (defined $soapopts{'mode'}) {
	if ($soapopts{'mode'} eq "batch") {
	    system("/data/jobqueue/bin/jq_submit2 .scriptfile soap");
	    $returndata{'job_submitted'} = "Job $newtmpdir submitted.\n";
	    $returndata{'jobid'}="$newtmpdir";
	    my $descfile=".desc";
	    if ( -r "$descfile") {
		$returndata{'description'}=`/bin/cat $descfile`;
		$returndata{'description'}.="\nApplication pending\n";
	    } else {
		$returndata{'description'}="Application pending\n";
	    }
	    return (%returndata);
	}
    }
    #
    # environment emboss needs
    #
    $ENV{'PATH'}="/usr/bin:$EMBBINDIR:/packages/clustal";
    $ENV{'PLPLOT_LIB'}="/packages/emboss/STABLE/share/EMBOSS/";
    $ENV{'EMBOSS_DATA'}="/packages/emboss/STABLE/share/EMBOSS/";
    #
    # actually run the command
    # need to save output
    #
    # explicitly direct stdout and stderr
    #
    system("$safe_prog 1>stdout 2>stderr");
    $exit_code = $? >> 8;
    $returndata{'cmd'}.="\nstatus code $exit_code";
    #
    # unfortunately emboss writes to stderr in the normal course
    # of events, so zap the error file unless we have a non-zero status
    #
    if ($exit_code == 0) { unlink("stderr");}
    #
    $now_string = localtime;
    open(IFILE, ">.finished");
    print IFILE "$now_string";
    close IFILE;
    #
    # file-return loop
    # just return all non-empty files that were produced
    #
    # should clean up the files we had passed over
    #
    while (<*>) {
	my $outdata="";
	my $thisfile="$_";
	if ( -s $thisfile ) {
	    open (OFILE, "<$thisfile");
	    while (<OFILE>)
	    {
		$outdata .= $_;
	    }
	    close OFILE;
	    $returndata{$thisfile}=$outdata;
	}
    }
    #
    # clean up after ourselves
    #
    chdir("/tmp");
    #system("/bin/rm -fr $newtmpdir");
    return (%returndata);
}

sub show_saved_results {
    my ($class,$dir) = @_;
    if (($dir =~ m#^/#)||($dir =~ m#^\.#)||($dir =~ m#\./#)||($dir =~ m#/\.#)||($dir =~ m#\s|;|\||\`#)){
	$returndata{'status'}="1";
	$returndata{'msg'}="Invalid directory name $dir";
	return (%returndata);
    }
    chdir("$TMPROOT");
    #
    # must check that the passed directory is sane
    #
    if ( -d $dir) {
	chdir("$dir");
	$returndata{'status'}="0";
	$returndata{'msg'}="OK";
	while (<*>) {
	    my $outdata="";
	    my $thisfile="$_";
	    if ( -s $thisfile ) {
		open (OFILE, "<$thisfile");
		while (<OFILE>)
		{
		    $outdata .= $_;
		}
		close OFILE;
		$returndata{$thisfile}=$outdata;
	    }
	}
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="Unable to find result set $dir\n";
    }
    #
    # send back an annotation file if present
    #
    if ( -s ".Annotation" ) {
	my $outdata="";
	open (OFILE,"<.Annotation");
	while (<OFILE>)
	{
	    $outdata .= $_;
	}
	close OFILE;
	$returndata{'Annotation'}=$outdata;
    }
    chdir("/tmp");
    return (%returndata);
}

sub delete_saved_results {
    my ($class,$dir) = @_;
    if (($dir =~ m#^/#)||($dir =~ m#^\.#)||($dir =~ m#\./#)||($dir =~ m#/\.#)||($dir =~ m#\s|;|\||\`#)){
	$returndata{'status'}="1";
	$returndata{'msg'}="Invalid directory name $dir";
	return (%returndata);
    }
    chdir("$TMPROOT");
    if ( -d $dir ) {
	$returndata{'status'}="0";
	$returndata{'msg'}="Results deleted successfully.";
	system("/bin/rm -fr $dir");
	#$returndata{'debug'}="rm -fr $dir";
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="Unable to find results directory $dir.";
    }
    chdir("/tmp");
    return (%returndata);
}

sub annotate_results {
    my ($class,$dir,$annotation) = @_;
    if (($dir =~ m#^/#)||($dir =~ m#^\.#)||($dir =~ m#\./#)||($dir =~ m#/\.#)||($dir =~ m#\s|;|\||\`#)){
	$returndata{'status'}="1";
	$returndata{'msg'}="Invalid directory name $dir";
	return (%returndata);
    }
    chdir("$TMPROOT");
    if ( -d $dir ) {
	chdir("$dir");
	$returndata{'status'}="0";
	if ( -f ".Annotation" ) {
	    $returndata{'msg'}="Annotation updated successfully.";
	} else {
	    $returndata{'msg'}="Annotation saved successfully.";
	}
	open(AFILE,">.Annotation");
	print AFILE $annotation;
	close AFILE;
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="Unable to find results directory $dir.";
    }
    chdir("/tmp");
    return (%returndata);
}

sub list_saved_results {
    my ($class,$dir) = @_;
    chdir("$TMPROOT");
    $returndata{'status'}="0";
    $returndata{'msg'}="OK";
    while (<*>) {
	my $resdir="$_";
	my $descfile="$resdir/.desc";
	if ( -r "$descfile") {
	    $returndata{'list'}.="$resdir\n";
	    $returndata{$resdir}=`/bin/cat $descfile`;
	    if ( -r "$resdir/.finished" ) {
		$returndata{$resdir}.="\nApplication completed";
		$returndata{$resdir}.=`/bin/cat $resdir/.finished`;
	    } else {
		$returndata{$resdir}.="\nApplication pending\n";
	    }
	}
    }
    chomp $returndata{'list'};
    chdir("/tmp");
    return (%returndata);
}

sub update_result_status {
    my ($class,$prog,$args,$inhash) = @_;
    my %inputfiles = %{$inhash};
    $returndata{'status'} = "0";
    $returndata{'msg'} = "OK";
    chdir("$TMPROOT");
    foreach my $id (keys %inputfiles) {
	my $proj = $inputfile{$id};
	#projects currently aren't supported
	if ( -f "$id/.finished" ) {
	    $returndata{"$id"} = "complete";
	    $iddesc = "$id"."-description";
	    my $descfile = "$id/.desc";
	    $returndata{"$iddesc"}=`/bin/cat $descfile`;
	    $returndata{"$iddesc"}.="\nApplication completed";
	    $returndata{"$iddesc"}.=`/bin/cat $id/.finished`;
	} else {
	    $returndata{"$id"} = "pending";
	}
    }
    return (%returndata);
}

1;
