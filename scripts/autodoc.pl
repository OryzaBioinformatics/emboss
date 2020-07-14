#!/usr/bin/perl -w

# Makes the apps/index.html web page
# Creates the command lines include files: .ione, .ihelp, .itable
# Warns of missing documentation pages
# Updates the doc/programs/{text,html} files in the CVS tree

###############################################################################
#
# To be done
# ==========
# programs/html version to have separate embassy directories
#
# check emboss.cvs for files that need to be added - do not assume
# the cvs add will be run
#
# Why did domainatrix files get updated as zero length when the tests failed?
#
# Skeleton version for sourceforge apps directory (for Jemboss)
#
# embassy docs - /apps/ redirect (checked)
#     and embassy/*/ main pages (not yet checked?)
#
# plotorf copies plotorf.gif 3 times in 2 tests
#
# Changes needed for SourceForge version
# ======================================
# flag failed test cases so we don't use their results
# need to update index.html for embassy packages
# need to be better at identifying and fixing embassy missing documentation
# Need to check for EFUNC and EDATA HTML file updates with main server
#     including the index.html pages
###############################################################################

##################################################################
# 
# Definitions and global variables
# 
##################################################################


my %progdone = ();  # key=program name,
                    # value = set to 1 if documentation exists
my %progdir = ();   # key=program name,
                    # value = EMBASSY name if EMBASSY program

my $embassy;	    # name of current EMBASSY directory being done,
                    # "" if not an EMBASSY program
my $docdir;	    # name of directory holding the set of EMBASSY programs
                    # being done

# read in from the EMBOSS application 'wossname'
# group names, application name and which application is in which groups
my %grpnames;	    # hash of key=lowercase group name,
                    # value = uppercase group description
my %progs;	    # hash of key=program name, value = description
my %groups;	    # hash of key=lowercase group name, value = program names
my %embassyprogs;   # hash of embassy package names for each program
my %missingdoc;     # hash of missing program documentation.
                    # Value is the directory it should be in

# where the URL for the html pages is
my $url = "http://emboss.sourceforge.net/apps";
my $urlembassy = "http://emboss.sourceforge.net/embassy";

# where the original distribution lives
my $distribtop = "./";

# where the installed package lives
my $installtop = "./";

open (VERS, "embossversion -full -auto|") || die "Cannot run embossversion";
while (<VERS>) {
    if(/InstallDirectory: +(\S+)/) {$installtop = $1}
    if(/BaseDirectory: +(\S+)/) {$distribtop = $1}
}
close VERS;

# where the CVS tree program doc pages are
my $cvsdoc = "$distribtop/doc/programs";

# where the CVS tree scripts are
my $scripts = "$distribtop/scripts";
 
# where the web pages live
my $doctop = "$distribtop/doc/sourceforge";

my @embassylist = ("appendixd",
		   "domainatrix",
		   "domalign",
		   "domsearch",
		   "emnu",
		   "esim4",
		   "hmmer",
		   "meme",
		   "mse",
		   "myemboss",
		   "phylip",
		   "phylipnew",
		   "reconstruct",
		   "signature",
		   "structure",
		   "topo",
		   );

# the directories containing web pages - EMBOSS and EMBASSY
my @doclist = (
	       "$doctop/apps"
	       );
foreach $x (@embassylist) {
    push @doclist, "$doctop/embassy/$x";
}

# Filenames for cvs add and commit commands.
# These hold a list of the names of files to be added/committed
# to the text or html documentation directories
# This is done at the end of the script
my $cvsdochtmladd = '';
my $cvsdochtmlcommit = '';
my $cvsdoctextadd = '';
my $cvsdoctextcommit = '';
my $badlynx = 0;

$doccreate = "";

if ($#ARGV >= 0) {
    $argdoccreate = $ARGV[0];
    if ($argdoccreate =~ /^[-]create/i) {$doccreate = "Y"}
}

$cvscommit = $doccreate;

######################################################################
######################################################################
# 
# SUBROUTINES
# 
######################################################################
######################################################################


######################################################################
# 
# Name: filediff
# 
# Description: 
#	runs diff on two files and returns 1 if they differ
# 
# Args: 
# 	$silent	- 0: silent, else print the output of diff
#                 1: to stdout 2: to autodoc.log
# 	$afile - first filename
# 	$bfile - second filename
# 
# Warning: 
#	uses temporary filename 'z.z'
# 
######################################################################
sub filediff ( $$$ ) {
    my ($silent, $afile, $bfile) = @_;
    if ($silent == 2) {
	if ((-s $afile) !=  (-s $bfile)) {
	  print LOG "$afile " . (-s $afile) . ", $bfile " . (-s $bfile) . "\n";
	}
    }
    system ("diff -b $afile $bfile > z.z");
    $s = (-s "z.z");
    if ($s) {
	if ($silent == 1) {
	    print "$afile ** differences ** size:$s\n";
            open (DIF, "z.z") || die "cannot open diff output file";
	    while (<DIF>) { print "> $_";}
            close DIF;
	}
	elsif ($silent == 2) {
	    print LOG "$afile ** differences ** size:$s\n";
            open (DIF, "z.z") || die "cannot open diff output file";
	    while (<DIF>) { print LOG "> $_";}
            close DIF;
	}
        unlink "z.z";
	return $s;
    }
    unlink "z.z";
    return 0;
}


######################################################################
# 
# Name: header1
# 
# Description: 
#	prints out the first part of the HTML header text (before title)
# 
# Args: 
# 	*OUT - filehandle to print to
# 
# 
######################################################################
sub header1 (*) {
    local (*OUT) = @_;

    print OUT "
<HTML>

<HEAD>
  <TITLE>
  EMBOSS
  </TITLE>
</HEAD>
<BODY BGCOLOR=\"#FFFFFF\" text=\"#000000\">


<!--#include file=\"header1.inc\" -->
";

}


######################################################################
# 
# Name: header2
# 
# Description: 
#	prints out the second part of the HTML header text (after title)
# 
# Args: 
# 	*OUT - filehandle to print to
# 
# 
######################################################################
sub header2 (*) {
    local (*OUT) = @_;

    print OUT "
<!--#include file=\"header2.inc\" -->

<!--END OF HEADER-->




";
}



######################################################################
# 
# Name: footer
# 
# Description: 
#	ends an HTML page
# 
# Args: 
# 	*OUT - filehandle to print to
# 
# 
######################################################################
sub footer (*) {
    local (*OUT) = @_;
    print OUT "

</BODY>
</HTML>
";
}


######################################################################
# 
# Name: indexheader
# 
# Description: 
# 	prints out the header and text at the start of the file
#	containing the table of applications.
#
# Args: 
#	*OUT - filehandle to print to 
# 
# 
######################################################################
sub indexheader (*) {
    local (*OUT) = @_;

    print OUT "
<HTML>

<HEAD>
  <TITLE>
  EMBOSS: The Applications (programs)
  </TITLE>
</HEAD>
<BODY BGCOLOR=\"#FFFFFF\" text=\"#000000\">



<table align=center border=0 cellspacing=0 cellpadding=0>
<tr><td valign=top>
<A HREF=\"http://emboss.sourceforge.net/\" ONMOUSEOVER=\"self.status='Go to the EMBOSS home page';return true\">
<img border=0 src=\"emboss_icon.jpg\" alt=\"\" width=150 height=48></a>
</td>
<td align=left valign=middle>
<b><font size=\"+6\"> 
The Applications (programs)
</font></b>
</td></tr>
</table>
<br>&nbsp;
<p>




The programs are listed in alphabetical order, Look at the individual
applications or go to the 
<a href=\"groups.html\">GROUPS</a>
page. 
<p>


<h3><A NAME=\"current\">Applications</A> in the <a
href=\"ftp://ftp.uk.embnet.org/pub/EMBOSS/\">current release</a></h3>

<table border cellpadding=4 bgcolor=\"#FFFFF0\">

<tr>
<th>Program name</th>
<th>Description</th>
</tr>

";

}

######################################################################
# 
# Name: indexfooter
# 
# Description: 
# 	print out the end of the table for the file
#       containing the table of applications. 
#
# Args: 
# 	*OUT - filehandle to print to
# 
# 
######################################################################
sub indexfooter (*) {
    local (*OUT) = @_;

    print OUT "

</table>




</BODY>
</HTML>
";
}

######################################################################
# 
# Name: getprogramnames
# 
# Description: 
# 	Runs wossname to get the EMBOSS programs on the path
#	together with their groups
#
# Args: 
#   *** These are all global variables ***
# 	%grpnames - hash of key=lowercase group name,
#                        value = uppercase group description
# 	%progs - hash of key=program name, value = program description
#	%groups - hash of key=lowercase group name, value = program names
# 
######################################################################

sub getprogramnames ( ) {

    my $prog;	# program name
    my $capgrp;	# uppcase group name
    my $grp;	# lowercase group name

    open (PROGS, "wossname -noembassy -auto |") ||
	die "Cannot run wossname";
    while ($prog = <PROGS>) {
	if ($prog =~ /^\s*$/) {	# ignore blank lines
	    next;
	} elsif ($prog =~ /^([A-Z0-9 ]+)$/) {	# uppcase means a group name
	    $capgrp = $1;			
	    $grp = lc($capgrp);
	    $grp =~ s/ +/_/g;		# lowercase one-word group_name
	    $grpnames{$grp} = $capgrp;
#      print "Group $grp = $capgrp\n";
	} elsif ($prog =~ /^(\S+) +(.*)/) {
	    $progs{$1} = $2;		
	    $groups{$grp} .= "$1 ";
#      print "Program in $grp = $1\n";
	}
    }
    close PROGS;

    foreach $e(@embassylist) {
	open (PROGS, "wossname -showembassy $e -auto |") ||
	    die "Cannot run wossname";
	while ($prog = <PROGS>) {
	    if ($prog =~ /^\s*$/) {	# ignore blank lines
		next;
	    } elsif ($prog =~ /^([A-Z0-9 ]+)$/) {  # uppcase means a group name
		$capgrp = $1;			
		$grp = lc($capgrp);
		$grp =~ s/ +/_/g;		# lowercase one-word group_name
		$grpnames{$grp} = $capgrp;
#      print "Group $grp = $capgrp\n";
	    } elsif ($prog =~ /^(\S+) +(.*)/) {
		$progs{$1} = $2;		
		$groups{$grp} .= "$1 ";
		$embassyprogs{$1} = $e;
#      print "Program in $grp = $1\n";
	    }
	}
    }
    close PROGS;

}


######################################################################
# 
# Name: createnewdocumentation
# 
# Description: 
# 	Asks if the user wishes to create and edit new documentation
#	for a program.
#	If so, the template file is copied and the user's favorite
#	editor is started.
#
# Args: 
# 	$thisprogram - the name of the program
#	$docdir - the location of the web pages
#	
#
# Returns:
#	1 if the document is created and edited
#	0 if no document is created
# 
######################################################################

sub createnewdocumentation ( $$ ) {

    my ($thisprogram, $progdocdir) = @_;
    my $ans;
    my $indexfile = "$progdocdir/index.html";

# application's document is missing
    print LOG "createnewdocumentation missing $progdocdir/$thisprogram.html\n";
    print "\n$progdocdir/$thisprogram.html =missing=\n";
    print STDERR "\n$progdocdir/$thisprogram.html =missing=\n";
    if ($doccreate) {
	print STDERR "Create a web page for this program? (y/n) ";
	$ans = <STDIN>;
    }
    else {
	$ans = "skip";
	print STDERR "Create later - run with create on commandline\n";
    }
    if ($ans =~ /^y/) {
        system("cp $progdocdir/template.html.save $progdocdir/$thisprogram.html");
        system "perl -p -i -e 's/ProgramNameToBeReplaced/$thisprogram/g;' $progdocdir/$thisprogram.html";
	chmod 0664, "$progdocdir/$thisprogram.html";
	if (defined $ENV{'EDITOR'} && $ENV{'EDITOR'} ne "") {
	    print STDERR "Generated $thisprogram.html:
	    Fill in the description section
	    Describe the input and output
	    Add notes, references
";
	    system("$ENV{'EDITOR'} $progdocdir/$thisprogram.html");
	    open (INDEX2, ">> $indexfile") || die "Cannot open $indexfile\n";
	    print INDEX2 "

<tr><td><a href=\"$thisprogram.html\">$thisprogram</a></td><td>INSTITUTE</td><td>
$progs{$thisprogram}
</td></tr>
";
	    close (INDEX2);
	    print STDERR "Edit $progdocdir/index.html:
	    Look for $thisprogram line (at end)
	    Replace INSTITUTE for $thisprogram
	    Move $thisprogram line to be in alphabetic order
";
	    system("$ENV{'EDITOR'} $progdocdir/index.html");
	}
	else {
	    print "*********************************

YOU DO NOT HAVE AN EDITOR DEFINED
REMEMBER TO EDIT THESE FILES:
 $progdocdir/$thisprogram.html
 $indexfile\n\n\n";
	}
        return 1;
    }
    else {
	$missingdoc{$thisprogram} = $progdocdir;
	return 0;
    }
}

######################################################################
# 
# Name: createnewdocumentationembassy
# 
# Description: 
# 	Asks if the user wishes to create and edit new documentation
#	for an embassy program.
#	If so, the top level redirect is created, and then 
#       the embassy template file is copied and the user's favorite
#	editor is started.
#
# Args: 
# 	$thisprogram - the name of the program
#	$progdocdir - the location of the web pages
#	
#
# Returns:
#	1 if the document is created and edited
#	0 if no document is created
# 
######################################################################

sub createnewdocumentationembassy ( $$ ) {

    my ($thisprogram, $progdocdir) = @_;
    my $ans;
    my $indexfile = "$progdocdir/index.html";

# application's document is missing
    print LOG "createnewdocumentationembassy missing $progdocdir/$thisprogram.html\n";
    print "\n$progdocdir/$thisprogram.html =missing=\n";
    print STDERR "\n$progdocdir/$thisprogram.html =missing=\n";
    print STDERR "Create a web page for this program? (y/n) ";
    if ($doccreate) {
	print STDERR "Create a web page for this program? (y/n) ";
	$ans = <STDIN>;
    }
    else {
	$ans = "skip";
	print STDERR "Create later - run with create on commandline\n";
    }
    if ($ans =~ /^y/) {
        system("cp $progdocdir/template.html.save $progdocdir/$thisprogram.html");
        system "perl -p -i -e 's/ProgramNameToBeReplaced/$thisprogram/g;' $progdocdir/$thisprogram.html";
	chmod 0664, "$progdocdir/$thisprogram.html";
	if (defined $ENV{'EDITOR'} && $ENV{'EDITOR'} ne "") {
	    print STDERR "Generated $thisprogram.html:
	    Fill in the description section
	    Describe the input and output
	    Add notes, references
";
	    system("$ENV{'EDITOR'} $progdocdir/$thisprogram.html");
	    open (INDEX2, ">> $indexfile") || die "Cannot open $indexfile\n";
	    print INDEX2 "

<tr><td><a href=\"$thisprogram.html\">$thisprogram</a></td><td>INSTITUTE</td><td>
$progs{$thisprogram}
</td></tr>
";
	    close (INDEX2);
	    print STDERR "Edit $progdocdir/index.html:
	    Look for $thisprogram line (at end)
	    Replace INSTITUTE for $thisprogram
";
	    system("$ENV{'EDITOR'} $progdocdir/index.html");
	}
	else {
	    print "*********************************

YOU DO NOT HAVE AN EDITOR DEFINED
REMEMBER TO EDIT THESE FILES:
 $progdocdir/$thisprogram.html
 $indexfile\n\n\n";
	}
        return 1;
    }
    else {
	$missingdoc{$thisprogram} = $progdocdir;
	return 0;
    }
}

######################################################################
# 
# Name: createnewdoclinkembassy
# 
# Description: 
# 	Asks if the user wishes to create and edit new documentation
#	for an embassy program.
#	If so, the top level redirect is created, and then 
#       the embassy template file is copied and the user's favorite
#	editor is started.
#
# Args: 
# 	$thisprogram - the name of the program
#	$progdocdir - the location of the web pages
#	
#
# Returns:
#	1 if the document is created and edited
#	0 if no document is created
# 
######################################################################

sub createnewdoclinkembassy ( $$$ ) {

    my ($thisprogram, $thisembassy, $progdocdir) = @_;
    my $ans;
    my $indexfile = "$progdocdir/index.html";

# application's document is missing
    print LOG "createnewdocumentationembassy missing $progdocdir/$thisprogram.html\n";
    print "\n$progdocdir/$thisprogram.html =missing=\n";
    print STDERR "\n$progdocdir/$thisprogram.html =missing=\n";
    print STDERR "Create a web page for this program? (y/n) ";
    if ($doccreate) {
	print STDERR "Create a web page for this program? (y/n) ";
	$ans = <STDIN>;
    }
    else {
	$ans = "skip";
	print STDERR "Create later - run with create on commandline\n";
    }
    if ($ans =~ /^y/) {
        system("cp $progdocdir/template-embassy.html.save $progdocdir/$thisprogram.html");
        system "perl -p -i -e 's/ProgramNameToBeReplaced/$thisprogram/g;' $progdocdir/$thisprogram.html";
        system "perl -p -i -e 's/EmbassyNameToBeReplaced/$thisembassy/g;' $progdocdir/$thisprogram.html";
	chmod 0664, "$progdocdir/$thisprogram.html";
	print STDERR "Generated $thisprogram.html: no need to edit\n";
	if (defined $ENV{'EDITOR'} && $ENV{'EDITOR'} ne "") {
	    system("$ENV{'EDITOR'} $progdocdir/$thisprogram.html");
	    open (INDEX2, ">> $indexfile") || die "Cannot open $indexfile\n";
	    print INDEX2 "

<tr><td><a href=\"../embassy/$thisembassy/$thisprogram.html\">$thisprogram</a></td><td>INSTITUTE</td><td>
$progs{$thisprogram}
</td></tr>
";
	    close (INDEX2);
	    print STDERR "Edit $progdocdir/index.html:
	    Look for $thisprogram line (at end)
	    Replace INSTITUTE for $thisprogram
	    Move $thisprogram line to $thisembassy section
";
	    system("$ENV{'EDITOR'} $progdocdir/index.html");
	}
	else {
	    print "*********************************

YOU DO NOT HAVE AN EDITOR DEFINED
REMEMBER TO EDIT THESE FILES:
 $progdocdir/$thisprogram.html
 $indexfile\n\n\n";
	}
        return 1;
    }
    else {
	$missingdoc{$thisprogram} = $progdocdir;
	return 0;
    }
}

######################################################################
# 
# Name: checkincludefile
# 
# Description: 
# 	This checks for the existence of one of several types of include file
#	If the file doesn't exist, it is created from the 'x.x' file.
#	If the file exists, a new one is created and checked to see if it
#	is different to the old one, if different, it is updated
#	This assumes that the file 'x.x' has just been set up with the
#	new include file contents.
#
# Args: 
# 	$thisprogram - the name of the program
#	$docdir - the location of the web pages
#	$ext - extension of the include file
# 
# 
######################################################################

sub checkincludefile ( $$$ ) {

    my ($thisprogram, $docdir, $ext) = @_;


    if (-e "$docdir/inc/$thisprogram.$ext") {
#     print "$thisprogram.$ext found\n";
# check to see if the include file has changed
	if (filediff (2, "$docdir/inc/$thisprogram.$ext", "x.x")) {
	    system "cp x.x $docdir/inc/$thisprogram.$ext";
	    print "$thisprogram.$ext *replaced*\n";
	    print LOG "$thisprogram.$ext *replaced*\n";
	}
    }
    else {
# it doesn't exist, so create the new include file
	system "cp x.x $docdir/inc/$thisprogram.$ext";
	print "$thisprogram.$ext *created*\n";
	print LOG "$thisprogram.$ext *created*\n";
    }
    chmod 0664, "$docdir/inc/$thisprogram.$ext";
    unlink "x.x";
}



##################################################################
##################################################################
#
# Main routine
#
##################################################################
##################################################################

open(LOGEX, ">makeexample.log") || die "Cannot open makeexample.log";
close (LOGEX);

open(LOG, ">autodoc.log") || die "Cannot open autodoc.log";

# get the program and group names
getprogramnames();

# open the file 'i.i'
# This will be copied to the file 'index.html' at the end of the script
# if all goes well.
# 'index.html' is the file we will be putting in the distribution.
open (INDEX, "> i.i") || die "Cannot open i.i\n";
indexheader(INDEX);


# look at all directories in our documentation list
foreach $docdir (@doclist) {
    if ($docdir =~ /embassy\/(.*)/) {
	$embassy = $1;
	print LOG "embassy $embassy\n";
    }
    else {
	$embassy = "";
    }

# look at all applications alphabetically
    foreach $thisprogram (sort (keys %progs)) {
	if ($embassy eq "") {
	    if (defined($embassyprogs{$thisprogram})) {next}
	}
	elsif (!defined($embassyprogs{$thisprogram})) {next}
	else {
	    if ($embassyprogs{$thisprogram} ne $embassy) {next}
	}
	print "\n$thisprogram '$progs{$thisprogram}'\n";
	print LOG "\n$thisprogram '$progs{$thisprogram}'\n";
	
# if this is a non-EMBASSY program then add it to the index.html file
	if (!defined($embassyprogs{$thisprogram})) {
	    $progdocdir = $docdir;
	    print INDEX
"<tr>
<td><a href=\"$thisprogram.html\">$thisprogram</a></td><td>$progs{$thisprogram}</td>
</tr>\n";
	}
	else {
# update the embassy index here -
# or just use the %embassyprogs array to make a list?
	    $progdocdir = "$doctop/embassy/$embassyprogs{$thisprogram}";
	}

# check the documentation for this file exists and is not a symbolic link 
	if (-e "$progdocdir/$thisprogram.html") {
###	  print "$progdocdir/$thisprogram.html found\n";
# if this is an EMBASSY document, note which EMBASSY directory it is in
	    if ($embassyprogs{$thisprogram}) {
		$progdir{$thisprogram} = $embassyprogs{$thisprogram};
	    }
	}
	elsif (!defined($embassyprogs{$thisprogram})) {
# optionally create the documentation and edit it,
#	  or abort and do the next program
###	  print "$progdocdir/$thisprogram.html missing - EMBOSS\n";
	    if (!createnewdocumentation($thisprogram, $progdocdir)) {next;}
	}
	else {
###	  print "$progdocdir/$thisprogram.html missing - EMBASSY $embassyprogs{$thisprogram}\n";
	    print STDERR "Missing embassy documentation $progdocdir/$thisprogram.html\n";
	    print STDERR "docdir: $docdir\n";
	    print STDERR "progdocdir: $progdocdir\n";
	    print STDERR "embassyprogs: $embassyprogs{$thisprogram}\n";
	    if (!createnewdocumentationembassy($thisprogram, $progdocdir)) {next;}
	}

# note whether we now have a documentation file or not
	if (-e "$progdocdir/$thisprogram.html") {
	    $progdone{$thisprogram} = 1;
	}
	else {
	    print "++ Missing main docs: $thisprogram ++ \n";
	}

# For embassy, also check the embassy specific directory
	if (defined($embassyprogs{$thisprogram})) {
	    if (-e "$docdir/$thisprogram.html") {
###	  print "$docdir/$thisprogram.html found\n";
		$progdir{$thisprogram} = $embassyprogs{$thisprogram};
	    }
	    else {
###	      print "$docdir/$thisprogram.html missing - EMBASSY $embassyprogs{$thisprogram}\n";
		print STDERR "Missing embassy link $docdir/$thisprogram.html\n";
		print STDERR "docdir: $docdir\n";
		print STDERR "embassyprogs: $embassyprogs{$thisprogram}\n";
		if (!createnewdoclinkembassy($thisprogram, $embassyprogs{$thisprogram}, $docdir)) {next;}
	    }
	}

# check on the existence of the one-line description include file
# for this application
	open(FH, ">x.x") || die "Can't open file x.x\n";
	print FH $progs{$thisprogram};
	close(FH);
	checkincludefile($thisprogram, $docdir, 'ione');

# check on the existence of the '-help' include file for this application
	system "acdc $thisprogram -help -verbose 2> x.x";
	checkincludefile($thisprogram, $docdir, 'ihelp');


# check to see if the command table include file exists
	system "acdtable $thisprogram 2> x.x";
	checkincludefile($thisprogram, $docdir, 'itable');


# check on the existence of the 'seealso' include file for this application
# if this is not an EMBASSY program, then we don't want to include EMBASSY
# programs in the SEE ALSO file
	if (!defined($embassyprogs{$thisprogram})) {
	    system "seealso $thisprogram -auto -noembassy -html -post '.html' -out x.x";
	    open (X, "x.x") || die "Cannot open x.x";
	    $text = "";
	    while (<X>) {
		if (/\"([^\/.]+)\.html/) {
		    $app = $1;
		    if (defined($embassyprogs{$app})) {
			$apppack = $embassyprogs{$app};
			s/\"([^\/.]+)\.html/\"..\/embassy\/$apppack\/$app.html/;
		    }
		}
		$text .= $_;
	    }
	    close (X);
	    open (X, ">x.x") || die "Cannot open x.x for output";
	    print X $text;
	    close X;
	}
	else {
	    system "seealso $thisprogram -auto -html -post '.html' -out x.x";
	    open (X, "x.x") || die "Cannot open x.x";
	    $text = "";
	    while (<X>) {
		if (/\"([^\/.]+)\.html/) {
		    $app = $1;
		    if (defined($embassyprogs{$app})) {
			$apppack = $embassyprogs{$app};
			if ($apppack ne $embassyprogs{$thisprogram}) {
			    s/\"([^\/.]+)\.html/\"..\/$apppack\/$app.html/;
			}
		    }
		    else {
			s/\"([^\/.]+)\.html/\"..\/..\/apps\/$app.html/;
		    }
		}
		$text .= $_;
	    }
	    close (X);
	    open (X, ">x.x") || die "Cannot open x.x for output";
	    print X $text;
	    close X;
	}
	system "perl -p -i -e 's/SEE ALSO/See also/g;' x.x";
	checkincludefile($thisprogram, $progdocdir, 'isee');

# create the '.usage', '.input' and '.output' include files
	if ($embassy eq "") {
	    $docurl = $url;
	    $mkstatus = system "$scripts/makeexample.pl $thisprogram";
	}
	else {
	    $docurl = "$urlembassy/$embassy";
	    $mkstatus = system "$scripts/makeexample.pl $thisprogram $embassy";
	}
	if ($mkstatus) {
	    print STDERR "$thisprogram: makeexample.pl status $mkstatus\n";
	}

# check to see if the CVS tree copy of the text documentation needs updating
	if (-e "$cvsdoc/text/$thisprogram.txt") {
# check to see if the text has changed
	    $status = system "lynx -dump -nolist $docurl/$thisprogram.html > x.x";
	    if ($status) {
		$badlynx++;
		print "lynx error $status $docurl/$thisprogram.html";
	    }
	    elsif (filediff (2, "$cvsdoc/text/$thisprogram.txt", "x.x")) {
		system "cp x.x $cvsdoc/text/$thisprogram.txt";
		chmod 0664, "$cvsdoc/text/$thisprogram.txt";
		$cvsdoctextcommit .= " $thisprogram.txt";
		print "$thisprogram.txt *replaced*\n";
		print LOG "$thisprogram.txt *replaced*\n";
		unlink "x.x";
	    }
	}
	else {
# it doesn't exist, so create the new text output
	    $status = system "lynx -dump -nolist $docurl/$thisprogram.html > $cvsdoc/text/$thisprogram.txt";
	    if ($status) {
		$badlynx++;
		print "lynx error $status $docurl/$thisprogram.html";
	    }
	    else {
		chmod 0664, "$cvsdoc/text/$thisprogram.txt";
		$cvsdoctextadd .= " $thisprogram.txt";
		$cvsdoctextcommit .= " $thisprogram.txt";
		print "$thisprogram.txt *created*\n";
		print LOG "$thisprogram.txt *created*\n";
	    }
	}

# check to see if the CVS tree copy of the html documentation needs updating
	if (-e "$cvsdoc/html/$thisprogram.html") {
# check to see if the html file has changed
	    $status = system "lynx -source $docurl/$thisprogram.html > x.x";
	    if ($status) {
		$badlynx++;
		print "lynx error $status $docurl/$thisprogram.html";
	    }
	    else {
# change ../emboss_icon.jpg and ../index.html to current directory
		system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
		system "perl -p -i -e 's#/images/emboss_icon.jpg#emboss_icon.jpg#g;' x.x";
		if (filediff (2, "$cvsdoc/html/$thisprogram.html", "x.x")) {
		    system "cp x.x $cvsdoc/html/$thisprogram.html";
		    chmod 0664, "$cvsdoc/html/$thisprogram.html";
		    $cvsdochtmlcommit .= " $thisprogram.html";
		    print "$thisprogram.html *replaced*\n";
		    print LOG "$thisprogram.html *replaced*\n";
		    unlink "x.x";
		}
	    }
	}
	else {
# it doesn't exist, so create the new html output
	    $status = system "lynx -source $docurl/$thisprogram.html > $cvsdoc/html/$thisprogram.html";
	    if ($status) {
		$badlynx++;
		print "lynx error $status $docurl/$thisprogram.html";
	    }
	    else {
# change ../emboss_icon.jpg and ../index.html to current directory
		system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' $cvsdoc/html/$thisprogram.html";
		system "perl -p -i -e 's#\/images/emboss_icon.jpg#emboss_icon.jpg#g;' $cvsdoc/html/$thisprogram.html";
		chmod 0664, "$cvsdoc/html/$thisprogram.html";
		$cvsdochtmladd .= " $thisprogram.html";
		$cvsdochtmlcommit .= " $thisprogram.html";
		print "$thisprogram.html *created*\n";
		print LOG "$thisprogram.html *created*\n";
	    }
	}
    }
}

# end the index.html file
indexfooter(INDEX);
close(INDEX);

# check to see if the index.html file has changed
if (filediff (2, "$cvsdoc/html/index.html", "i.i")) {    
    system "cp i.i $cvsdoc/html/index.html";
    unlink "i.i";
    chmod 0664, "$cvsdoc/html/index.html";
    $cvsdochtmlcommit .= " index.html";
    print "index.html *replaced*\n";
}


# look at all applications and report the ones with missing documentation
foreach $thisprogram (sort (keys %progs)) {
    if ($progdone{$thisprogram}) {next}
    print "$thisprogram.html =missing=\n";
}


#############################
#
# NOW PROCESS THE GROUPS FILE
#
#############################

open (GRPSTD, "$installtop/share/EMBOSS/acd/groups.standard") ||
    die "Cannot open $installtop/share/EMBOSS/acd/groups.standard";
while (<GRPSTD>) {
    if (/^\#/) {next}
    if (/^([^ ]+) (.*)/) {
	$gname = $1;
	$gdesc = ucfirst(lc($2));
	$gname =~ s/:/ /g;
	$gname =~ s/_/ /g;
	$gname = ucfirst(lc($gname));
	$gname =~ s/[Dd]na/DNA/;
	$gname =~ s/[Rr]na/RNA/;
	$gname =~ s/[Cc]pg/CpG/;
	$gname =~ s/Hmm/HMM/i;
	$gdesc =~ s/[Dd]na/DNA/;
	$gdesc =~ s/[Rr]na/RNA/;
	$gdesc =~ s/[Cc]pg/CpG/;
	$gdesc =~ s/Hmm/HMM/i;
	$grpdef{$gname} = $gdesc;
    }
}
close GRPSTD;

$docdir = "$doctop/apps";

open ( SUM, ">g.g") || die "cannot open temporary groups summary file";

header1(SUM);
print SUM "Application Groups";
header2(SUM);


print SUM "
<table border cellpadding=4 bgcolor=\"#FFFFF0\">

<tr><th>Group</th><th>Description</th></tr>

";

foreach $g (sort (keys %groups)) {
# change the capitalisation on a few group names - most are lowercase
    $name = $g;
    $name =~ s/_/ /g;
    $name = ucfirst(lc($name));
    $name =~ s/[Dd]na/DNA/;
    $name =~ s/[Rr]na/RNA/;
    $name =~ s/Cpg/CpG/i;
    $name =~ s/Hmm/HMM/i;
    if(!defined($grpdef{$name})){
	print STDERR "Unknown group '$name'\n";
	$grpdef{$name} = $name;
    }
    $desc = $grpdef{$name};

# this group's name is too long for the Makefile 
    $filename = $g;
    $filename =~ s/restriction_enzymes/re/;


    print SUM "<tr><td><A HREF=\"$filename\_group.html\">$name</A></td><td>$desc</td></tr>\n";
# print "$filename '$groups{$g}' '$grpnames{$g}'\n";

    open ( GRP, ">y.y") || die "cannot open temporary group file";

    header1 (GRP);
    print GRP "$grpnames{$g}";
    header2 (GRP);

    print GRP "
$desc
<p>

<table border cellpadding=4 bgcolor=\"#FFFFF0\">

<tr><th>Program name</th><th>Description</th></tr>
";

    foreach $p (split (/\s+/, $groups{$g})) {
#   print "$g : '$p'\n";
	if ($progdir{$p}) {
	    print GRP "
<tr>
<td><a href=\"../embassy/$progdir{$p}/$p.html\">$p</a></td>
<td>
$progs{$p}
</td>
</tr>
";

	}
	else {
	    print GRP "
<tr>
<td><a href=\"$p.html\">$p</a></td>
<td>
$progs{$p}
</td>
</tr>
";
	}
    }

    print GRP "

</table>

";

    footer (GRP);

    close GRP;

    if (-e "$docdir/$filename\_group.html") {
	if (filediff (2, "$docdir/$filename\_group.html", "y.y")) {
	    print "$filename\_group.html group file *differences*\n";
	    system "cp y.y $docdir/$filename\_group.html";
	    chmod 0664, "$docdir/$filename\_group.html";
	    print "$filename\_group.html *replaced*\n";
	    print LOG "$filename\_group.html *created*\n";
	    unlink "y.y";
	}
    }
    else {
	system "cp y.y $docdir/$filename\_group.html";
	chmod 0664, "$docdir/$filename\_group.html";
	print "$filename\_group.html *created*\n";
	print LOG "$filename\_group.html *created*\n";
	unlink "y.y";
    }


# check to see if the CVS tree copy of the html groups documentation needs updating
    if (-e "$cvsdoc/html/$filename\_group.html") {
# check to see if the html file has changed
	$status = system "lynx -source $url/$filename\_group.html > x.x";
	if ($status) {
	    $badlynx++;
	    print "lynx error $status $url/$filename\_group.html";
	}
	else {
# change ../emboss_icon.jpg and ../index.html to current directory
	    system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
	    system "perl -p -i -e 's#\/images/emboss_icon.jpg#emboss_icon.jpg#g;' x.x";
	    if (filediff (2, "$cvsdoc/html/$filename\_group.html", "x.x")) {
		system "cp x.x $cvsdoc/html/$filename\_group.html";
		chmod 0664, "$cvsdoc/html/$filename\_group.html";
		$cvsdochtmlcommit .= " $filename\_group.html";
		print "$filename\_group.html *replaced*\n";
		unlink "x.x";
	    }
	}
    }
    else {
# it doesn't exist, so create the new html output
	$status = system "lynx -source $url/$filename\_group.html > $cvsdoc/html/$filename\_group.html";
	if ($status) {
	    $badlynx++;
	    print "lynx error $status $url/$filename\_group.html";
	}
	else {
# change ../emboss_icon.jpg and ../index.html to current directory
	    system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' $cvsdoc/html/$filename\_group.html";
	    system "perl -p -i -e 's#\/images/emboss_icon.jpg#emboss_icon.jpg#g;' $cvsdoc/html/$filename\_group.html";
	    chmod 0664, "$cvsdoc/html/$filename\_group.html";
	    $cvsdochtmladd .= " $filename\_group.html";
	    $cvsdochtmlcommit .= " $filename\_group.html";
	    print "$filename\_group.html *created*\n";
	}
    }
}

print SUM "

</table>
";

footer(SUM);

close SUM;

if (filediff (2, "$docdir/groups.html", "g.g")) {
    system "cp g.g $docdir/groups.html";
    chmod 0664, "$docdir/groups.html";
    print "groups.html *replaced*\n";
    unlink "g.g";
}


# check to see if the CVS tree copy of the html group documentation needs updating
if (-e "$cvsdoc/html/groups.html") {
# check to see if the html file has changed
    $status = system "lynx -source $url/groups.html > x.x";
    if ($status) {
	$badlynx++;
	print "lynx error $status $url/groups.html";
    }
    else {
# change ../emboss_icon.jpg and ../index.html to current directory
	system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
	system "perl -p -i -e 's#\/images/emboss_icon.jpg#emboss_icon.jpg#g;' x.x";
	if (filediff (2, "$cvsdoc/html/groups.html", "x.x")) {
	    system "cp x.x $cvsdoc/html/groups.html";
	    chmod 0664, "$cvsdoc/html/groups.html";
	    $cvsdochtmlcommit .= " groups.html";
	    print "groups.html *replaced*\n";
	    unlink "x.x";
	}
    }
}
else {
# it doesn't exist, so create the new html output
    $status = system "lynx -source $url/groups.html > $cvsdoc/html/groups.html";
    if ($status) {
	$badlynx++;
	print "lynx error $status $url/groups.html";
    }
    else {
# change ../emboss_icon.jpg and ../index.html to current directory
	system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' $cvsdoc/html/groups.html";
	system "perl -p -i -e 's#\/images/emboss_icon.jpg#emboss_icon.jpg#g;' $cvsdoc/html/groups.html";
	chmod 0664, "$cvsdoc/html/groups.html";
	$cvsdochtmladd .= " groups.html";
	$cvsdochtmlcommit .= " groups.html";
	print "groups.html *created*\n";
    }
}

######################################################################
# OK - we have updated all our files, now CVS add and CVS commit them 
######################################################################

chdir "$cvsdoc/html";

if ($cvsdochtmladd ne "") {
    print "cvs add -m'documentation added' $cvsdochtmladd\n";
    print STDERR "cvs add -m'documentation added' $cvsdochtmladd\n";
    if ($cvscommit) {
	system "cvs add -m'documentation added' $cvsdochtmladd";
    }
}
if ($cvsdochtmlcommit ne "") {
    print "cvs commit -m'documentation committed' $cvsdochtmlcommit\n";
    print STDERR "cvs commit -m'documentation committed' $cvsdochtmlcommit\n";
    if ($cvscommit) {
	system "cvs commit -m'documentation committed' $cvsdochtmlcommit";
    }
}
else {
    print STDERR "HTML docs unchanged\n";
}

chdir "$cvsdoc/text";

if ($cvsdoctextadd ne "") {
    print "cvs add -m'documentation added' $cvsdoctextadd\n";
    print STDERR "cvs add -m'documentation added' $cvsdoctextadd\n";
    if ($cvscommit) {
	system "cvs add -m'documentation added' $cvsdoctextadd";
    }
}
if ($cvsdoctextcommit ne "") {
    print "cvs commit -m'documentation committed' $cvsdoctextcommit\n";
    print STDERR "cvs commit -m'documentation committed' $cvsdoctextcommit\n";
    if ($cvscommit) {
	system "cvs commit -m'documentation committed' $cvsdoctextcommit";
    }
}
else {
    print STDERR "TEXT docs unchanged\n";
}

# No need to make these makefiles ... now they use wildcards
#print "Create make files\n";
#system("$scripts/makeMake.pl");	       # no parameter == do text
#system("$scripts/makeMake.pl html");   # any parameter == do html

print "\n";
print LOG "\n";
print "==================================\n";
print LOG "==================================\n";
print "Lynx errors: $badlynx\n";
print LOG "Lynx errors: $badlynx\n";
foreach $x (sort(keys(%missingdoc))) {
    print "Missing: $missingdoc{$x}/$x.html\n";
    print LOG "Missing: $missingdoc{$x}/$x.html\n";
}
