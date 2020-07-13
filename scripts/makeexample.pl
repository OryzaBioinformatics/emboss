#!/usr/local/bin/perl -w

# This is a utility to create the usage, input and output
# example HTML include files for the EMBOSS application documentation.
#
# The 'scripts/qatest.pl' script should be run with the argument '-kk' 
# before makeexample.pl is run in order to create the example usage,
# input and output files.




use File::Basename;




###################################################################
#
# Some useful definitions
#
###################################################################
# where the URL for the html pages is
#$url = "http://www.uk.embnet.org/Software/EMBOSS/Apps/";

# where the qa directories are
$qa = "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/test/qa/";

# where the web pages and include files live
$docdir = "/data/www/Software/EMBOSS/Apps/";
$incdir = "/data/www/Software/EMBOSS/Apps/inc";


# some HTML
$p = "<p>";
$bold = "<b>";
$unbold = "</b>";

# maximum number of lines of a file to be displayed
$MaxLines = 100;

# names of test databases
@testdbs = (
	'tsw',
	'tswnew',
	'twp',
	'tembl',
	'tpir',
);

# colours for backgrounds of usage examples, input and output files
$usagecolour = "#CCFFFF"; # light cyan
$inputcolour = "#FFDDFF"; # light violet
$outputcolour = "#DDFFDD"; # light green



###################################################################
# initialise the outputs
$USAGE = "";
$INPUT = "";
$OUTPUT = "";

@testdbsoutput = ();
@inputfilesshown = ();
@outputfilesshown = ();

# debug
#open (OUT, "> /people/gwilliam/jjj.html") || die "Can't open debug OUT\n";

###################################################################
# get the name of the application
if ($#ARGV != 0) {
	print "Name of the program >";
	$application = <STDIN>;
} else {
	$application = $ARGV[0];
}
chomp $application;
if (!defined $application || $application eq "") {die "No program specified\n";}

###################################################################
# check for any qa '*-ex' example directories
@dirs = glob("$qa/$application-ex*");

# if there are none, look for '*-keep' directories
if ($#dirs == -1) {
    @dirs = glob("$qa/$application-keep");
}

# are there any results directories?
if ($#dirs == -1) {
    errorexit("No qa results directories were found for $application");
}

# sort the directory names
@dirs = sort @dirs;

###################################################################
# get next example directory
$count = 0;
foreach $dir (@dirs) {
    print "Doing $dir\n";
    $count++;

###################################################################
# initialise some output messages for this example
    $testdbcomment = "";
    $commandline = "";
    $usagecomment = "";
    $inputcomment = "";
    $outputcomment = "";

###################################################################
# read in 'testdef' file of qa commands
    $testfile = "$dir/testdef";
    open (TESTDEF, "< $testfile") || errorexit("Couldn't open file $testfile");
    @testdef = <TESTDEF>;
    close (TESTDEF);

###################################################################
# read in 'stderr' file of prompts
    $promptfile = "$dir/stderr";
    open (PROMPTS, "< $promptfile" ) || errorexit("Couldn't open file $promptfile");
    @prompts = <PROMPTS>;
    close (PROMPTS);

###################################################################
# read in 'stdin' file of responses to prompts
    $answerfile = "$dir/stdin";
    open (ANSWERS, "< $answerfile") || errorexit("Couldn't open file $answerfile");
    @answers = <ANSWERS>;
    close (ANSWERS);

###################################################################
# read in 'stdout' file of results written to screen
    $resultsfile = "$dir/stdout";
    open (RESULTS, "< $resultsfile") || errorexit("Couldn't open file $resultsfile");
    @results = <RESULTS>;
    close (RESULTS);

###################################################################
# get list of other results files
    @outfiles = glob("$dir/*");

# remove stderr, stdin, stdout, testdef, testlog from this list
    @outfiles = grep !/stdin|stderr|stdout|testdef|testlog/, @outfiles;

###################################################################
# parse 'CL, UC, IC, OC' lines from 'testdef'
    foreach $line (@testdef) {
        if ($line =~ /^CL\s+(.+)/) {$commandline .= "$1 ";}
        if ($line =~ /^UC\s+(.+)/) {$usagecomment .= "$1 ";}
        if ($line =~ /^IC\s+(.+)/) {$inputcomment .= "$1 ";}
        if ($line =~ /^OC\s+(.+)/) {$outputcomment .= "$1 ";}
    }

###################################################################
# change any ampersands or angle brackets to HTML codes
    $commandline =~ s/&/&amp;/g;
    $commandline =~ s/</&lt;/g;
    $commandline =~ s/>/&gt;/g;

    $usagecomment =~ s/&/&amp;/g;
    $usagecomment =~ s/</&lt;/g;
    $usagecomment =~ s/>/&gt;/g;

    $inputcomment =~ s/&/&amp;/g;
    $inputcomment =~ s/</&lt;/g;
    $inputcomment =~ s/>/&gt;/g;

    $outputcomment =~ s/&/&amp;/g;
    $outputcomment =~ s/</&lt;/g;
    $outputcomment =~ s/>/&gt;/g;

###################################################################
# change newlines to <br>
    $commandline =~ s/\n/<br>\n/g;
    $usagecomment =~ s/\n/<br>\n/g;
    $inputcomment =~ s/\n/<br>\n/g;
    $outputcomment =~ s/\n/<br>\n/g;

###################################################################
# see if any of the arguments of CL are real input files
    @infiles = ();
    @inusas = ();

# split on space, >, <, = - all are possible before a file name
    foreach $f (split /\s|\>|\<|\=/, $commandline) {
        chomp $f;
# ignore qualifiers - words starting with a '-'
        if ($f =~ /^-/) {next;}
# split on '::' to get files embedded in a format::file USA
        if ($f =~ /\:\:/) {
print "CL line=$f\n";
            @fm = split /\:\:/, $f;
            $f = pop @fm;
print "CL f=$f\n";
        }
# deal with '@' in list files
        $f =~ s/\@//;
# check to see if this is an output file existing in the -ex directory
        if (grep /^$f$/, @outfiles) {
            next;
        }

# check to see if it is a real file (not a directory)
        if (-f "$dir/$f" && ! -d "$dir/$f") {
# we assume all files in the *-ex directory are output files
# check for '.' or '/' at start of path
            if ($f =~ /^\./ || $f =~ m#^\/#)  {
#print "displaying infile=$f\n";
                push @infiles, $f;
            }
        } else {
           push @inusas, $f; 
        }
    }

###################################################################
# see if any of the answers to prompts are real files
# split on '::' to get files embedded in a format::file USA
    foreach $line (@answers) {
        chomp $line;
	if (length $line) {
#print "line=$line\n";
            @usas = split /\:\:/, $line;
            $f = pop @usas;
#print "f=$f\n";
# deal with '@' in list files
            $f =~ s/\@//;
# check to see if this is an output file existing in the -ex directory
            if (grep /^$f$/, @outfiles) {
                next;
            }
# check to see if it is a real file (not a directory)
#print "test for file: $dir/$f\n";
            if (-f "$dir/$f" && ! -d "$dir/$f") {
# we assume all files in the *-ex directory are output files
# check for '.' or '/' at start of path
                if ($f =~ /^\./ || $f =~ m#^\/#)  {
#print "displaying infile=$f\n";
                    push @infiles, $f;
                }
            } else {
               push @inusas, $line; 
            }
        }
    }

###################################################################
# see if any of the answers to prompts are lists of real files
# split on ',' to get files embedded in a file list 'file1,file2,file3'
    foreach $line (@answers) {
        chomp $line;
	if (length $line) {
            @filelist = split /,\s*/, $line;
            foreach $f (@filelist) {
# deal with '@' in list files
                $f =~ s/\@//;
# if ':' in it then ignore - not a file
                if ($f =~ /\:/) {
#print "ignoring $f in list of input files\n";
                    next;
                }
# check to see if it is a real file (not a directory)
                if (-f "$dir/$f" && ! -d "$dir/$f") {
# we assume all files in the *-ex directory are output files
# check for '.' or '/' at start of path
                    if ($f =~ /^\./ || $f =~ m#^\/#)  {push @infiles, $f;}
                } else {
                   push @inusas, $line; 
                }
            }
        }
    }

###################################################################
# see if we use the test databases anywhere in the command line or answers
    foreach $f (@answers, split (/\s|\>|\<|\=/, $commandline)) {
        chomp $f;
        @d = split (/\:/, $f);
        foreach $d (@d) {
            if ($d =~ /\*/) {next;}
            if (grep /^$d$/, @testdbs) {
# check we have not made any other comments about this test database
                if (! grep /$d/, @testdbsoutput) {
                    if ($d eq "tembl") {
                        $type = "nucleic acid";
                    } else {
                        $type = "protein";
                    }
                    $testdbcomment .= "\n'$f' is a sequence entry in the example $type database '$d'\n$p\n";
                    push @testdbsoutput, $d;
                }
	    }
        }
    }


###################################################################
# usage documentation
###################################################################

# example count
    if ($count == 1) {
        $USAGE .= qq|<b>Here is a sample session with $application</b>\n$p\n|;
    } else {
        $USAGE .= qq|$p\n<b>Example $count</b>\n$p\n|;
    }

# usage comment
    if ($usagecomment ne "") {
        $USAGE .= qq|$usagecomment\n$p\n|;
    }

# blank line
    $USAGE .= qq|\n$p\n|;

# start table (light cyan)
    $USAGE .= qq|<table width="90%"><tr><td bgcolor="$usagecolour"><pre>\n\n|;

# application name and command line
    print "Doing:\n% $application $commandline\n";
    $USAGE .= qq|% $bold$application $commandline$unbold\n|;

# intercalate prompts and answers
    @pr = ();
    foreach $line (@prompts) {
        push @pr, split /([^\s]+: )/, $line;
    }
    foreach $line (@pr) {
        $USAGE .= qq|$line|;
# change ':'s in warning messages so that they don't look like prompts
# although if we get a warning, things are probably going badly wrong anyway
#print "prompt=$line\n";
        $line =~ s/Warning:/Warning :/;
        $line =~ s/Error:/Error :/;
        $line =~ s/Fatal:/Fatal :/;
# print the answer in bold
#print "$line\n";
        if ($line =~ /[^\s]: $/) {
            $ans = shift @answers;
#print "prompt=$line\n";
#print "ans=$ans\n";
            $USAGE .= "$bold$ans$unbold\n";
        }
    }

# have we used all of our answers?
    if ($#answers != -1) {
        print "WARNING **** 
application '$application' example $count hasn't used", $#answers+1, "answers\n";
    }

# display any results found in the 'stdout' file
    $USAGE .= qq|\n|;
    foreach $r (@results) {$USAGE .= qq|$r|;}
# end stdout results with another blank line
    if ($#results > -1) {$USAGE .= qq|\n|;}

# end table
    $USAGE .= qq|</pre></td></tr></table>$p\n|;

# blank line
    $USAGE .= qq|$p\n|;

###################################################################
# input documentation
###################################################################

# If the command line of answers contain tembl, tsw etc.
# add a command about these being test databases.
# only add this comment once
    $I = $testdbcomment;

# input comment
    $I .= $inputcomment;

# foreach input file
    foreach $file (@infiles) {
#print "input file=$file\n";

# if this some sort of binary file?
        if (checkBinary("$dir/$file")) {

            $I .= $p . "<h3>File: $file</h3>\n";
            $I .= $p . "This file contains non-printing characters and so cannot be displayed here.\n";

# normal file that can be displayed
        } else {

# has the name has been used before
            if (! grep /^$file$/, @inputfilesshown) { 
                push @inputfilesshown, $file;

# no - contruct new display of file
# add example number to the list of examples that use this file
                $I .= displayFile($file, "$dir/$file", $inputcolour);
            }
        }
    }

# the @inusas list of inputs are possible USAs, but have not been checked.
# see if they have a ':' in them and display using entret.
# If it has an '*' then ignore it,
    foreach $f (@inusas) {
        if ($f =~ /\:/ && $f !~ /\*/ && $f !~ /http\:/) {
# has the name has been used before
            if (! grep /^$f$/, @inputfilesshown) { 
                push @inputfilesshown, $f;

                $I .= displayEntry($f, $inputcolour);
            }
        }
    }


# If any new files were output for this example, 
    if (length $I) {
        $INPUT .= qq|\n<a name="input.$count"></a>\n<h3>Input files for usage example |;
        if ($count > 1) {$INPUT .= "$count";}
        $INPUT .= "</h3>\n";
        $INPUT .= $I;

# add a link from the usage
        $USAGE .= qq|<a href="#input.$count">Go to the input files for this example</a><br>|;
    }



###################################################################
# output documentation
###################################################################

    $O = "";

# output comment
    $O .= $outputcomment;

# foreach output file
    foreach $path (@outfiles) {
#print "output file=$path\n";

        $file = basename($path);

# if this a .gif, .ps or .png graphics file?
        if ($file =~ /\.gif$|\.ps$|\.png$/) {

# convert .ps file to gif
            $giffile = "";
            $origfile = $file;
            if ($file =~ /\.ps$/) {
                $giffile = $file;
                $giffile =~ s/\.ps/.gif/;
# add -delay to see the first page of an animated gif for 10 mins
		system("2>&1 /usr/local/bin/convert -delay 65535 -rotate '-90<' $path $giffile >/dev/null");
                $file = $giffile;
                $path = $giffile;
            }

# rename file to be unique - application name . example count . given file name
	    $newfile = "$application.$count.$file";
            system("cp $path $docdir/$newfile");
            chmod 0664, "$docdir/$newfile";

# tidy up
            if ($giffile ne "") {
                unlink $giffile;
            }

# display the graphics file
            $O .= $p . "<h3>Graphics File: $origfile</h3>\n";
            $O .= $p . qq|<img src="$newfile" alt="[$application results]">\n|;

# if this some other binary file?
        } elsif (checkBinary($path)) {

            $O .= $p . "<h3>File: $file</h3>\n";
            $O .= $p . "This file contains non-printing characters and so cannot be displayed here.\n";

# normal file that can be displayed
        } else {

# has the name has been used before (match to end of path in @outputfilesshown)
            @o = grep /$file$/, @outputfilesshown;
            if ($#o == -1) { 
                push @outputfilesshown, $path;

# no - contruct new display of file
#print "displaying $file\n";
                $O .= displayFile($file, $path, $outputcolour);
            } else {
# do a diff of the two files
                $ofile = pop @o;
                system ("diff  $ofile $path> diff.tmp");
                if ( -s "diff.tmp" ) {
# it is a different file
                    $O .= displayFile($file, $path, $outputcolour);
                }
                unlink "diff.tmp";
            }
        }
    }

# If any new files were output for this example, 
    if (length $O) {
        $OUTPUT .= qq|\n<a name="output.$count"></a>\n<h3>Output files for usage example |;
        if ($count > 1) {$OUTPUT .= "$count";}
        $OUTPUT .= "</h3>\n";
        $OUTPUT .= $O;

# add a link from the usage
        $USAGE .= qq|<a href="#output.$count">Go to the output files for this example</a>$p|;
    }

# force a blank line to be at the end of the usage
    $USAGE .= "$p\n";

}

###################################################################
# create the usage, input and output documentation files

writeUsage($USAGE);
writeInput($INPUT);
writeOutput($OUTPUT);

# debug
#print OUT "USAGE=\n$USAGE\n";
#print OUT "INPUT = \n$INPUT";
#print OUT "OUTPUT = \n$OUTPUT";
#close(OUT);

exit(0);

###################################################################
###################################################################



###################################################################
# Name:		errorexit
# Function:	writes dummy files and exits with an error message
# Args:		string - error message
# Returns:	exits - no return
###################################################################
sub errorexit {
    my $msg = $_[0];

    my $usage = $msg;
    my $input = $msg;
    my $output = $msg;

    writeUsage($usage);
    writeInput($input);
    writeOutput($output);

    print "$msg\n";
    exit 1;
}

###################################################################
# Name:        writeUsage
# Function:    writes usage file
# Args:        string - example usage
# Returns:    
###################################################################
sub writeUsage {
    my $usage = $_[0];

    my $out = "$incdir/$application.usage";
    open (OUT, "> $out") || die "Can't open $out";
    print OUT $usage;
    close(OUT);
    chmod 0664, $out;	# rw-rw-r--
}

###################################################################
# Name:		writeInput
# Function:	writes input file
# Args:		string - example input
# Returns:	
###################################################################
sub writeInput {
    my $input = $_[0];

    my $out = "$incdir/$application.input";
    open (OUT, "> $out") || die "Can't open $out";
    print OUT $input;
    close(OUT);
    chmod 0664, $out;	# rw-rw-r--
}

###################################################################
# Name:		writeOutput
# Function:	writes output file
# Args:		string - example output
# Returns:	
###################################################################
sub writeOutput {
    my $output = $_[0];

    my $out = "$incdir/$application.output";
    open (OUT, "> $out") || die "Can't open $out";
    print OUT $output;
    close(OUT);
    chmod 0664, $out;	# rw-rw-r--
}

###################################################################
# Name:		displayEntry
# Function:	returns a string used to display a database entry
# Args:		string - USA
#		string - colour
# Returns:	string - HTML formatted entry contents
###################################################################
sub displayEntry {
    my $usa = $_[0];
    my $colour = $_[1];
    my $res = "";

#print "In displayEntry($usa)\n";

# if the 'file name' contains 'http:' then ignore it :-)
    if ($usa =~ /http\:/) {
      return $res;
    }

# if the USA has a single ':', use entret, else it is a file and we use seqret
    if ($usa !~ /\:\:/ && $usa =~ /\:/) {
       system ("entret $usa z.z -auto");
    } elsif ($usa =~ /\:\:/) {
       system ("seqret $usa z.z -auto");
    }

    $res = displayFile($usa, "z.z", $colour);

    unlink "z.z";

    return $res;
}

###################################################################
# Name:		displayFile
# Function:	returns a string used to display a file
# Args:		string - file name as used in the example usage
#		string - path to file
#		string - colour
# Returns:	string - HTML formatted files contents
###################################################################
sub displayFile {
    my $file = $_[0];
    my $path = $_[1];
    my $colour = $_[2];

    my $result = "";
    my @lines;
    my $count;

#print "In displayFile($file)\n";

# if the 'file name' contains 'http:' then ignore it :-)
    if ($file =~ /http\:/) {
      return $result;
    }

# if the file has the name 'z.z' (used by displayEntry) then say in the
# title that it is a database entry rather than a file
    if ($path eq "z.z") {
        $result = $p . "<h3>Database entry: $file</h3>\n";
    } else {
        $result = $p . "<h3>File: $file</h3>\n";
    }

# start table
    $result .= qq|<table width="90%"><tr><td bgcolor="$colour">\n|;

# if not an .html file, put it in a PRE block
    if ($file !~ /.html$/) {
        $result .= qq|<pre>\n|;
    }

    open (F, "< $path") || die "Can't open input file $path";
    @lines = <F>;
    close (F);

# convert <, >, & to HTML codes if the file is not a .html file
    if ($path !~ /.html$/) {
        foreach my $l (@lines) {
            $l =~ s/&/&amp;/g;
            $l =~ s/</&lt;/g;
            $l =~ s/>/&gt;/g;
        }
    }

# if file is too long, cut out the middle bit;
    if ($#lines > $MaxLines) {
        for ($count = 0; $count < $MaxLines/2; $count++) {
            $result .= $lines[$count];
        }
        $result .= "\n\n<font color=red>  [Part of this file has been deleted for brevity]</font>\n\n";
        for ($count = $#lines - $MaxLines/2; $count <= $#lines; $count++) {
            $result .= $lines[$count];
        }

    } else {
        $result .= join "", @lines;
    }

# if not an .html file, put it in a PRE block
    if ($file !~ /.html$/) {
        $result .= qq|</pre>\n|;
    }

# end table
    $result .= qq|</td></tr></table>$p\n|;

    return $result;
}

###################################################################
# Name:		checkBinary
# Function:	checks to see if a file has non-printing characters in
# Args:		string - path to file
# Returns:	bool - true if file is binary
###################################################################
sub checkBinary {
    my $file = $_[0];

    my $count;
    my $buf;
    my $c;

    open(B, "< $file") || die "Can't open file $file\n";
# get the first 1Kb characters
    read B, $buf, 1024;
    close(B);

#my @n = unpack('C*', $buf);
#for ($count = 0; $count <= $#n; $count++) {
#    $c = pop @n;
#print "c=$c\n"; 
#    if ($c >= 32 && $c <= 126) {next;}
#    if ($c == ord("\t") || $c == ord("\n") || $c == ord("\r")) {next;}
#
#print "Binary\n";
#        return 1;
#    }
#
#print "Not Binary\n";
#    return 0;
	    

    for ($count = 0; $count < length $buf; $count++) {
        $c = ord(substr($buf, $count, 1));
        if ($c >= 32 && $c <= 126) {next;}
	if ($c == ord("\t") || $c == ord("\n") || $c == ord("\r")) {next;}
        return 1;
    }

    return 0;
}
