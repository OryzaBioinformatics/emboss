package EmbreoFile;

require ParseSoapOptions;

#
# standard methods
#

sub name {
    return "Embreo File Manager";
}
sub appversion {
    return "1";
}
sub about {
    return "A facility to list, transfer, create and delete files.";
}
sub pubservice {
    return "Y";
}
sub helpurl {
    return "";
}
sub abouturl {
    return "";
}
sub docurl {
    return "";
}
sub servicedesc {
    $desc{'name'}=&name;
    $desc{'version'}=&version;
    $desc{'about'}=&about;
    $desc{'public'}=&public;
    $desc{'helpurl'}=&helpurl;
    $desc{'abouturl'}=&abouturl;
    $desc{'docurl'}=&docurl;
    return (%desc);
}


#
# EmbreoFile methods
#
# the $options variable has a set of key-value pairs in which the key and
# value are separated by = and the pairs separated by spaces
#

sub get_file {
    my ($class,$options,$file) = @_;
    my %soapopts = &ParseSoapOptions::parse_soap_options($options);
    my $cdir = map_root($soapopts{'fileroot'});
    if ( -d $cdir ) {
	chdir("$cdir");
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="Root directory $soapopts{'fileroot'} not found";
	$returndata{'contents'}="Root directory $soapopts{'fileroot'} [ $cdir ] not found";
	return (%returndata);
    }
    my $outdata = "";
    $returndata{'status'}="0";
    $returndata{'msg'}="";
    if ( -d $file ) {
	$returndata{'msg'}="Directory listing follows";
	$returndata{'contents'} = `/bin/ls -1l $file`;
    } elsif ( -f $file ) {
	#open (OFILE, "<$file");
	#while (<OFILE>)
	#{
	#    $outdata = "$outdata"."$_";
	#}
	#close OFILE;
	$outdata = `/bin/cat $file`;
	$returndata{'status'}="0";
	$returndata{'msg'}="File follows";
	$returndata{'contents'} = "$outdata";
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="File $file not found";
    }
    return (%returndata);
}

sub put_file {
    my ($class,$options,$file,$data) = @_;
    my %soapopts = &ParseSoapOptions::parse_soap_options($options);
    my $cdir = map_root($soapopts{'fileroot'});
    if ( -d $cdir ) {
	chdir("$cdir");
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="Root directory $soapopts{'fileroot'} not found";
	$returndata{'contents'}="Root directory $soapopts{'fileroot'} [ $cdir ] not found";
	return (%returndata);
    }
    $returndata{'status'}="0";
    $returndata{'msg'}="";
    if ( -f $file ) {
	$returndata{'status'}="1";
	$returndata{'msg'}="Failed - file already exists";
    } else {
	open(FH,">$file");
	write FH, $data;
	close(FH);
	$returndata{'status'}="0";
	$returndata{'msg'}="Copy complete";
    }
    return (%returndata);
}

sub test_file {
    my ($class,$options,$file) = @_;
    my %soapopts = &ParseSoapOptions::parse_soap_options($options);
    my $cdir = map_root($soapopts{'fileroot'});
    if ( -d $cdir ) {
	chdir("$cdir");
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="Root directory $soapopts{'fileroot'} not found";
	$returndata{'contents'}="Root directory $soapopts{'fileroot'} [ $cdir ] not found";
	return (%returndata);
    }
    $returndata{'status'}="0";
    $returndata{'msg'}="";
    if ( -f $file ) {
	$returndata{'status'}="0";
	$returndata{'msg'}="File found successfully";
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="No such file";
    }
    return (%returndata);
}

sub directory_longls {
    my ($class,$options,$dir) = @_;
    my %soapopts = &ParseSoapOptions::parse_soap_options($options);
    my $cdir = map_root($soapopts{'fileroot'});
    if ( -d $cdir ) {
	chdir("$cdir");
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="Root directory $soapopts{'fileroot'} not found";
	$returndata{'contents'}="Root directory $soapopts{'fileroot'} [ $cdir ] not found";
	return (%returndata);
    }
    $returndata{'status'}="0";
    $returndata{'msg'}="";
    if ( -d $dir ) {
	$returndata{'status'}="0";
	$returndata{'msg'}="Directory listing follows";
	chdir $dir;
	open(LS, "/bin/ls -l |");
	my $fnum=0;
	while (<LS>) {
	    chomp;
	    @ent=split();
	    unless ($_ =~ /^total /) {
		$returndata{"$ent[$#ent]"}=$_;
		$fnum++;
	    }
	}
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="No such Directory $dir";
    }
    return (%returndata);
}

sub directory_shortls {
    my ($class,$options,$dir) = @_;
    my %soapopts = &ParseSoapOptions::parse_soap_options($options);
    my $cdir = map_root($soapopts{'fileroot'});
    if ( -d $cdir ) {
	chdir("$cdir");
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="Root directory $soapopts{'fileroot'} not found";
	$returndata{'contents'}="Root directory $soapopts{'fileroot'} [ $cdir ] not found";
	return (%returndata);
    }
    $returndata{'status'}="0";
    $returndata{'msg'}="";
    if ( -d $dir ) {
	$returndata{'status'}="0";
	$returndata{'msg'}="Directory listing follows";
	chdir $dir;
	my $filelist=`/bin/ls -1`;
	my $dirlist = `/bin/ls -1F | grep '/$'`;
	$dirlist =~ s:/::g;
	$returndata{'list'}=$filelist;
	$returndata{'dirlist'}=$dirlist;
    } else {
	$returndata{'status'}="1";
	$returndata{'msg'}="No such Directory $dir";
    }
    return (%returndata);
}

sub embreo_roots {
    $returndata{'status'}="0";
    $returndata{'msg'}="";
    $returndata{'default-root'}="HOME";
    $returndata{'HOME'}="/people/$ENV{'LOGNAME'}";
    if ( -d "/archive/$ENV{'LOGNAME'}/." ) {
	$returndata{'ARCHIVE'} = "/archive/$ENV{'LOGNAME'}";
    }
    if ( -d "/data/scratch/$ENV{'LOGNAME'}/." ) {
	$returndata{'SCRATCH'} = "/data/scratch/$ENV{'LOGNAME'}";
    }
    return (%returndata);
}

sub map_root {
    my ($rootarg) = @_;
    my $rootdir = "";
    if ($rootarg eq "HOME") {
	$rootdir = "/people/$ENV{'LOGNAME'}";
    } elsif ($rootarg eq "ARCHIVE") {
	$rootdir = "/archive/$ENV{'LOGNAME'}";
    } elsif ($rootarg eq "SCRATCH") {
	$rootdir = "/data/scratch/$ENV{'LOGNAME'}";
    }
}

1;
