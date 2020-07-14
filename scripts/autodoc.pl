#!/usr/local/bin/perl -w

# Makes the Apps/index.html web page
# Creates the command lines include files: .ione, .ihelp, .itable
# Warns of missing documentation pages
# Updates the doc/programs/{text,html} files in the CVS tree



##################################################################
# 
# Definitions and global variables
# 
##################################################################


  my %progdone = ();	# key=program name, value = set to 1 if documentation exists
  my %progdir = ();	# key=program name, value = EMBASSY name if EMBASSY program

  my $embassy;		# name of current EMBASSY directory being done, "" if not an EMBASSY program
  my $docdir;		# name of directory holding the set of EMBASSY programs being done

# read in from the EMBOSS application 'wossname'
# group names, application name and which application is in which groups
  my %grpnames;	# hash of key=lowercase group name, value = uppercase group description
  my %progs;	# hash of key=program name, value = description
  my %groups;	# hash of key=lowercase group name, value = program names

# where the URL for the html pages is
  my $url = "http://www.uk.embnet.org/Software/EMBOSS/Apps/";

# where the CVS tree program doc pages are
  my $cvsdoc = "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/doc/programs/";

# where the CVS tree scripts are
  my $scripts = "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/scripts";
 
# where the web pages live
  my $doctop = "/data/www/Software/EMBOSS";

# the directories containing web pages - EMBOSS and EMBASSY
  my @doclist = (
	    "$doctop/Apps",
	    "$doctop/EMBASSY/MSE",
	    "$doctop/EMBASSY/PHYLIP",
	    "$doctop/EMBASSY/TOPO",
	    "$doctop/EMBASSY/MEME",
	    "$doctop/EMBASSY/EMNU",
	    "$doctop/EMBASSY/DOMAINATRIX",
  );

# Filenames for cvs add and commit commands.
# These hold a list of the names of files to be added/committed
# to the text or html documentation directories
# This is done at the end of the script
  my $cvsdochtmladd = '';
  my $cvsdochtmlcommit = '';
  my $cvsdoctextadd = '';
  my $cvsdoctextcommit = '';



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
# 	$silent	- false = print the output of diff
# 	$afile - first filename
# 	$bfile - second filename
# 
# Warning: 
#	uses temporary filename 'z.z'
# 
######################################################################
sub filediff ( $$$ ) {
    my ($silent, $afile, $bfile) = @_;
    system ("diff $afile $bfile > z.z");
    if ( -s "z.z" ) {
	if (!$silent) {
	    print "$afile ** differences **\n";
            open (DIF, "z.z") || die "cannot open diff output file";
	    while (<DIF>) { print "> $_";}
            close DIF;
	}
        unlink "z.z";
	return 1;
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


<!--#include file=\"header1.inc\" -->\n";

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
<A HREF=\"http://www.uk.embnet.org/Software/EMBOSS/index.html\" ONMOUSEOVER=\"self.status='Go to the EMBOSS home page';return true\">
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
# 	%grpnames - hash of key=lowercase group name, value = uppercase group description
# 	%progs - hash of key=program name, value = program description
#	%groups - hash of key=lowercase group name, value = program names
# 
######################################################################

sub getprogramnames ( ) {

  my $prog;	# program name
  my $capgrp;	# uppcase group name
  my $grp;	# lowercase group name

  open (PROGS, "wossname -auto |") || die "Cannot run wossname";
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

  my ($thisprogram, $docdir) = @_;
  my $ans;
  my $indexfile = "$docdir/index.html";

# application's document is missing
      print "\n$thisprogram.html =missing=\n";
      print "Create a web page for this program? (y/n) ";
      $ans = <STDIN>;

      if ($ans =~ /^y/) {
        system("cp $docdir/template.html.save $docdir/$thisprogram.html");
        system "perl -p -i -e 's/ProgramNameToBeReplaced/$thisprogram/g;' $docdir/$thisprogram.html";
	chmod 0664, "$docdir/$thisprogram.html";
	if (defined $ENV{'EDITOR'} && $ENV{'EDITOR'} ne "") {
	  system("$ENV{'EDITOR'} $docdir/$thisprogram.html");
	  open (INDEX2, ">> $indexfile") || die "Cannot open $indexfile\n";
	  print INDEX2 "

<tr><td><a href=\"$thisprogram.html\">$thisprogram</a></td><td>INSTITUTE</td><td>
$progs{$thisprogram}
</td></tr>
";
	  close (INDEX2);
	  system("$ENV{'EDITOR'} $docdir/index.html");
	} else {
	  print "*********************************

YOU DO NOT HAVE AN EDITOR DEFINED
REMEMBER TO EDIT THESE FILES:
 $docdir/$thisprogram.html
 $indexfile\n\n\n";
	}
        return 1;
      } else {
	return 0;
      }
}

######################################################################
# 
# Name: checkincludefile
# 
# Description: 
# 	This checks for the existance of one of several types of include file
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
      if (filediff (0, "$docdir/inc/$thisprogram.$ext", "x.x")) {
	system "cp x.x $docdir/inc/$thisprogram.$ext";
	print "$thisprogram.$ext *replaced*\n";
      }
    } else {
# it doesn't exist, so create the new include file
      system "cp x.x $docdir/inc/$thisprogram.$ext";
      print "$thisprogram.$ext *created*\n";
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
    if ($docdir =~ /EMBASSY\/(.*)/) {
      $embassy = $1;
    } else {
      $embassy = "";
    }

# look at all applications alphabetically
    foreach $thisprogram (sort (keys %progs)) {
      print "\n$thisprogram '$progs{$thisprogram}'\n";

# if this is a non-EMBASSY program then add it to the index.html file
      if ($embassy eq "") {
        print INDEX
"<tr>
<td><a href=\"$thisprogram.html\">$thisprogram</a></td><td>$progs{$thisprogram}</td>
</tr>\n";
      }

# check the documentation for this file exists and is not a symbolic link 
# (EMBASSY program docs have symbolic links to the main EMBOSS doc directory)
      if (-e "$docdir/$thisprogram.html" && ! -l "$docdir/$thisprogram.html") {
#       print "$thisprogram.html found\n";
# if this is an EMBASSY document, note which EMBASSY directory it is in
        if ($embassy ne "") {$progdir{$thisprogram} = $embassy}
      } elsif ($embassy eq "") {
# optionally create the documentation and edit it, or abort and do the next program
        if (!createnewdocumentation($thisprogram, $docdir)) {next;}
      } else {
# don't try to create documentation for an embassy program at present - should probably be added at some time
        next;
      }

# note whether we now have a documentation file or not
      if (-e "$docdir/$thisprogram.html") {
        $progdone{$thisprogram} = 1;
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
      if ($embassy eq "") {
        system "seealso $thisprogram -auto -noembassy -html -post '.html' -out x.x";
      } else {
        system "seealso $thisprogram -auto -html -post '.html' -out x.x";
      }
      system "perl -p -i -e 's/SEE ALSO/See also/g;' x.x";
      checkincludefile($thisprogram, $docdir, 'isee');

# create the '.usage', '.input' and '.output' include files
      system "$scripts/makeexample.pl $thisprogram";

# check to see if the CVS tree copy of the text documentation needs updating
      if (-e "$cvsdoc/text/$thisprogram.txt") {
# check to see if the text has changed
	system "lynx -dump -nolist $url/$thisprogram.html > x.x";
	if (filediff (0, "$cvsdoc/text/$thisprogram.txt", "x.x")) {
	  system "cp x.x $cvsdoc/text/$thisprogram.txt";
	  chmod 0664, "$cvsdoc/text/$thisprogram.txt";
	  $cvsdoctextcommit .= " $thisprogram.txt";
	  print "$thisprogram.txt *replaced*\n";
          unlink "x.x";
        }
      } else {
# it doesn't exist, so create the new text output
        system "lynx -dump -nolist $url/$thisprogram.html > $cvsdoc/text/$thisprogram.txt";
        chmod 0664, "$cvsdoc/text/$thisprogram.txt";
        $cvsdoctextadd .= " $thisprogram.txt";
        $cvsdoctextcommit .= " $thisprogram.txt";
        print "$thisprogram.txt *created*\n";
      }

# check to see if the CVS tree copy of the html documentation needs updating
      if (-e "$cvsdoc/html/$thisprogram.html") {
# check to see if the html file has changed
      system "lynx -source $url/$thisprogram.html > x.x";
# change ../emboss_icon.jpg and ../index.html to current directory
      system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
      system "perl -p -i -e 's#\.\.\/emboss_icon.jpg#emboss_icon.jpg#g;' x.x";
      if (filediff (0, "$cvsdoc/html/$thisprogram.html", "x.x")) {
	system "cp x.x $cvsdoc/html/$thisprogram.html";
	chmod 0664, "$cvsdoc/html/$thisprogram.html";
	$cvsdochtmlcommit .= " $thisprogram.html";
	print "$thisprogram.html *replaced*\n";
	unlink "x.x";
      }
    } else {
# it doesn't exist, so create the new html output
      system "lynx -source $url/$thisprogram.html > $cvsdoc/html/$thisprogram.html";
# change ../emboss_icon.jpg and ../index.html to current directory
      system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' $cvsdoc/html/$thisprogram.html";
      system "perl -p -i -e 's#\.\.\/emboss_icon.jpg#emboss_icon.jpg#g;' $cvsdoc/html/$thisprogram.html";
      chmod 0664, "$cvsdoc/html/$thisprogram.html";
      $cvsdochtmladd .= " $thisprogram.html";
      $cvsdochtmlcommit .= " $thisprogram.html";
      print "$thisprogram.html *created*\n";
    }

  }
}

# end the index.html file
indexfooter(INDEX);
close(INDEX);

# check to see if the index.html file has changed
if (filediff (1, "$cvsdoc/html/index.html", "i.i")) {    
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

$docdir = "$doctop/Apps";

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
  $name = ucfirst($name);
  $name =~ s/[Dd]na/DNA/;
  $name =~ s/[Rr]na/RNA/;
  $name =~ s/Cpg/CpG/;
  $desc = $name;

# this group's name is too long for the Makefile 
  $filename = $g;
  $filename =~ s/restriction_enzymes/re/;


  print SUM "<tr><td><A HREF=\"$filename\_group.html\">$name<A></td><td>$desc</td></tr>\n";
# print "$filename '$groups{$g}' '$grpnames{$g}'\n";

  open ( GRP, ">y.y") || die "cannot open temporary group file";

  header1 (GRP);
  print GRP "$grpnames{$g}";
  header2 (GRP);

  print GRP "
<table border cellpadding=4 bgcolor=\"#FFFFF0\">

<tr><th>Program name</th><th>Description</th></tr>
";

  foreach $p (split (/\s+/, $groups{$g})) {
#   print "$g : '$p'\n";
    if ($progdir{$p}) {
      print GRP "
<tr>
<td><a href=\"../EMBASSY/$progdir{$p}/$p.html\">$p</a></td>
<td>
$progs{$p}
</td>
</tr>
";

    } else {
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
    if (filediff (1, "$docdir/$filename\_group.html", "y.y")) {
      print "$filename\_group.html group file *differences*\n";
      system "cp y.y $docdir/$filename\_group.html";
      chmod 0664, "$docdir/$filename\_group.html";
      print "$filename\_group.html *replaced*\n";
      unlink "y.y";
    }
  } else {
    system "cp y.y $docdir/$filename\_group.html";
    chmod 0664, "$docdir/$filename\_group.html";
    print "$filename\_group.html *created*\n";
    unlink "y.y";
  }


# check to see if the CVS tree copy of the html groups documentation needs updating
  if (-e "$cvsdoc/html/$filename\_group.html") {
# check to see if the html file has changed
    system "lynx -source $url/$filename\_group.html > x.x";
# change ../emboss_icon.jpg and ../index.html to current directory
    system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
    system "perl -p -i -e 's#\.\.\/emboss_icon.jpg#emboss_icon.jpg#g;' x.x";
    if (filediff (1, "$cvsdoc/html/$filename\_group.html", "x.x")) {
      system "cp x.x $cvsdoc/html/$filename\_group.html";
      chmod 0664, "$cvsdoc/html/$filename\_group.html";
      $cvsdochtmlcommit .= " $filename\_group.html";
      print "$filename\_group.html *replaced*\n";
      unlink "x.x";
    }
  } else {
# it doesn't exist, so create the new html output
    system "lynx -source $url/$filename\_group.html > $cvsdoc/html/$filename\_group.html";
# change ../emboss_icon.jpg and ../index.html to current directory
    system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' $cvsdoc/html/$filename\_group.html";
    system "perl -p -i -e 's#\.\.\/emboss_icon.jpg#emboss_icon.jpg#g;' $cvsdoc/html/$filename\_group.html";
    chmod 0664, "$cvsdoc/html/$filename\_group.html";
    $cvsdochtmladd .= " $filename\_group.html";
    $cvsdochtmlcommit .= " $filename\_group.html";
    print "$filename\_group.html *created*\n";
  }
}

print SUM "

</table>
";

footer(SUM);

close SUM;

if (filediff (0, "$docdir/groups.html", "g.g")) {
  system "cp g.g $docdir/groups.html";
  chmod 0664, "$docdir/groups.html";
  print "groups.html *replaced*\n";
  unlink "g.g";
}


# check to see if the CVS tree copy of the html group documentation needs updating
if (-e "$cvsdoc/html/groups.html") {
# check to see if the html file has changed
  system "lynx -source $url/groups.html > x.x";
# change ../emboss_icon.jpg and ../index.html to current directory
  system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
  system "perl -p -i -e 's#\.\.\/emboss_icon.jpg#emboss_icon.jpg#g;' x.x";
  if (filediff (1, "$cvsdoc/html/groups.html", "x.x")) {
    system "cp x.x $cvsdoc/html/groups.html";
    chmod 0664, "$cvsdoc/html/groups.html";
    $cvsdochtmlcommit .= " groups.html";
    print "groups.html *replaced*\n";
    unlink "x.x";
  }
} else {
# it doesn't exist, so create the new html output
  system "lynx -source $url/groups.html > $cvsdoc/html/groups.html";
# change ../emboss_icon.jpg and ../index.html to current directory
  system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' $cvsdoc/html/groups.html";
  system "perl -p -i -e 's#\.\.\/emboss_icon.jpg#emboss_icon.jpg#g;' $cvsdoc/html/groups.html";
  chmod 0664, "$cvsdoc/html/groups.html";
  $cvsdochtmladd .= " groups.html";
  $cvsdochtmlcommit .= " groups.html";
  print "groups.html *created*\n";
}

######################################################################
# OK - we have updated all our files, now CVS add and CVS commit them 
######################################################################

chdir "$cvsdoc/html";

if ($cvsdochtmladd ne "") {
  print "cvs add -m'documentation added' $cvsdochtmladd\n";
  system "cvs add -m'documentation added' $cvsdochtmladd";
}
if ($cvsdochtmlcommit ne "") {
  print "cvs commit -m'documentation commited' $cvsdochtmlcommit\n";
  system "cvs commit -m'documentation commited' $cvsdochtmlcommit";
}

chdir "$cvsdoc/text";

if ($cvsdoctextadd ne "") {
  print "cvs add -m'documentation added' $cvsdoctextadd\n";
  system "cvs add -m'documentation added' $cvsdoctextadd";
}
if ($cvsdoctextcommit ne "") {
  print "cvs commit -m'documentation commited' $cvsdoctextcommit";
  system "cvs commit -m'documentation commited' $cvsdoctextcommit";
}

print "Create make files\n";
system("$scripts/makeMake.pl");	# no parameter == do text
system("$scripts/makeMake.pl html");





