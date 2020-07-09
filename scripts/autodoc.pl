#!/usr/local/bin/perl -w

# Makes the Apps/index.html web page
# Creates the command lines include files: .ione, .ihelp, .itable
# Warns of missing documentation pages
# Updates the doc/programs/{text,html} files in the CVS tree

######################################################################
######################################################################
######################################################################
######################################################################
# SUBROUTINES
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


##################################################################
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

<table align=center bgcolor=\"#000070\" border=0 cellspacing=0 cellpadding=10>
<tr>
<td>

<a href=\"../index.html\" onmouseover=\"self.status='Go to the EMBOSS home page';return true\">
<img border=0 src=\"../emboss_icon.gif\" alt=\"\" width=55 height=55>
</a>

</td>

<td align=middle valign=middle>
<font face=\"Arial,Helvetica\" size=6 color=\"#ffffff\">
<H2>
EMBOSS: ";

}


##################################################################
sub header2 (*) {
    local (*OUT) = @_;
   
print OUT "
</H2>
</font>
</td>

</tr>
</table>
</td>

</tr>
</table>

<p><hr><p>

<!--END OF HEADER-->




";
}



##################################################################
sub footer (*) {
    local (*OUT) = @_;
print OUT "

</table>

</TD></TR></TABLE>
</CENTER>

</BODY>
</HTML>
";
}


##################################################################

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

<table align=center bgcolor=\"#000070\" border=0 cellspacing=0 cellpadding=10>
<tr>
<td>

<a href=\"http://www.uk.embnet.org/Software/EMBOSS/index.html\" onmouseover=\"self.status='Go to the EMBOSS home page';return true\">
<img border=0 src=\"emboss_icon.gif\" alt=\"\" width=55 height=55>
</a>

</td>

<td align=middle valign=middle>
<font face=\"Arial,Helvetica\" size=6 color=\"#ffffff\">
<H2>
EMBOSS: The Applications (programs)
</H2>


</font>
</td>

</tr>
</table>
</td>

</tr>
</table>

<p><hr><p>

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

##################################################################

sub indexfooter (*) {
	local (*OUT) = @_;

print OUT "

</table>




</TD></TR></TABLE>
</CENTER>

</BODY>
</HTML>
";
}

##################################################################
##################################################################
#
# Main routine
#
##################################################################
##################################################################

%progdone = ();
%progdir = ();

# read in from the EMBOSS application 'wossname'
# group names, application name and which application is in which groups

###################################################################
# check that we are on the CVS machine
require 'hostname.pl';
if (hostname() ne "tin") {
  die "This script should be executed on the CVS machine 'tin'\n";
}
###################################################################


open (PROGS, "wossname -auto |") || die "Cannot run wossname";
$grp = "";
while (<PROGS>) {
  if (/^\s*$/) {next}
    if (/^([A-Z0-9 ]+)$/) {
      $capgrp = $1;
      $grp = lc($capgrp);
      $grp =~ s/ +/_/g;
      $grpnames{$grp} = $capgrp;
      next;
    }
    if (/^(\S+) +(.*)/) {
      $progs{$1} = $2;
      $groups{$grp} .= "$1 ";
    } 
  }
  close PROGS;

# where the URL for the html pages is
  $url = "http://www.uk.embnet.org/Software/EMBOSS/Apps/";

# where the CVS tree program doc pages are
  $cvsdoc = "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/doc/programs/";

# where the web pages live
  $doctop = "/data/www/Software/EMBOSS";

# the directories containing web pages - EMBOSS and EMBASSY
  @doclist = (
	    "$doctop/Apps",
	    "$doctop/EMBASSY/MSE",
	    "$doctop/EMBASSY/PHYLIP",
	    "$doctop/EMBASSY/TOPO",
	    "$doctop/EMBASSY/MEME",
	    "$doctop/EMBASSY/EMNU",
  );

# filenames for cvs add and commit commands
  $cvsdochtmladd = '';
  $cvsdochtmlcommit = '';
  $cvsdoctextadd = '';
  $cvsdoctextcommit = '';

###################################################################

# open the index.html file we will be putting in the distribution
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
    foreach $x (sort (keys %progs)) {
#     print "\n$x '$progs{$x}'\n";

# add the non-EMBASSY entry in the index.html file
      if ($embassy eq "") {
        print INDEX
"<tr>
<td><a href=\"$x.html\">$x</a></td>
<td>$progs{$x}</td>
</tr>
";
      }

# check the documentation for this file exists
    if (-e "$docdir/$x.html") {
#     print "$x.html found\n";
      $progdone{$x} = 1;
# if this is an EMBASSY document, note which EMBASSY directory it is in
      if ($embassy ne "") {$progdir{$x} = $embassy}
    } elsif ($embassy eq "") {
# application's document is missing
      print "\n$x.html =missing=\n";
      print "Create a web page for this program? (y/n) ";
      $ans = <STDIN>;
      if ($ans =~ /^n/) {
        next;
      } elsif ($ans =~ /^y/) {
        $progdone{$x} = 1;
        system("cp $docdir/template.html.save $docdir/$x.html");
        system "perl -p -i -e 's/ProgramNameToBeReplaced/$x/g;' $docdir/$x.html";
	chmod 0664, "$docdir/$x.html";
	if (defined $ENV{'EDITOR'} && $ENV{'EDITOR'} ne "") {
	  system("$ENV{'EDITOR'} $docdir/$x.html");
	  open (INDEX2, ">> $docdir/index.html") || die "Cannot open $docdir/index.html\n";
	  print INDEX2 "

<tr><td><a href=\"$x.html\">$x</a></td><td>INSTITUTE</td><td>
$progs{$x}
</td></tr>
";
	  close (INDEX2);
	  system("$ENV{'EDITOR'} $docdir/index.html");
	} else {
	  print "*********************************

REMEMBER TO EDIT THESE FILES:
 $docdir/$x.html
 $docdir/index.html\n\n\n";
	}
      } else {
	die "Expecting an answer 'y' or 'n'";
      }
    } else {
# don't try to create documentation for an embassy program at present - should probably be added at some time
      next;
    }

# check on the existence of the one-line description include file
# for this application
    if (-e "$docdir/inc/$x.ione") {
#     print "$x.ione found\n";
# check to see if the one-line description has changed
      open(FH, ">x.x") || die "Can't open file x.x\n";
      print FH $progs{$x};
      close(FH);
      if (filediff (0, "$docdir/inc/$x.ione", "x.x")) {
	system "cp x.x $docdir/inc/$x.ione";
	chmod 0664, "$docdir/inc/$x.ione";
	print "$x.ione *replaced*\n";
	unlink "x.x";
      }
    } else {
# it doesn't exist, so create the new one-line description output
      open(FH, ">$docdir/inc/$x.ione") || die "Can't open file $docdir/inc/$x.ione\n";
      print FH $progs{$x};
      close(FH);
      chmod 0664, "$docdir/inc/$x.ione";
      print "$x.ione *created*\n";
    }

# check on the existence of the '-help' include file for this application
    if (-e "$docdir/inc/$x.ihelp") {
#     print "$x.ihelp found\n";
# check to see if the '-help' output has changed
      system "acdc $x -help 2> x.x";
      if (filediff (0, "$docdir/inc/$x.ihelp", "x.x")) {
	system "cp x.x $docdir/inc/$x.ihelp";
	chmod 0664, "$docdir/inc/$x.ihelp";
	print "$x.ihelp *replaced*\n";
	unlink "x.x";
        }
      } else {
# it doesn't exist, so create the new '-help' output
	system "acdc $x -help 2> $docdir/inc/$x.ihelp";
        chmod 0664, "$docdir/inc/$x.ihelp";
        print "$x.ihelp *created*\n";
      }

# check to see if the command table include file exists
      if (-e "$docdir/inc/$x.itable") {
#    print "$x.itable found\n";
	system "acdc $x -help -acdtable 2> x.x";
	if (filediff (0, "$docdir/inc/$x.itable", "x.x")) {
	  system "cp x.x $docdir/inc/$x.itable";
	  chmod 0664, "$docdir/inc/$x.itable";
	  print "$x.itable *replaced*\n";
          unlink "x.x";
        }
      } else {
# it doesn't exist, so create the new command table include file
	system "acdc $x -help -acdtable 2> $docdir/inc/$x.itable";
        chmod 0664, "$docdir/inc/$x.itable";
        print "$x.itable *created*\n";
      }


# check on the existence of the 'seealso' include file for this application
      if (-e "$docdir/inc/$x.isee") {
#       print "$x.isee found\n";
# check to see if the 'seealso' output has changed
	system "seealso $x -auto -html -post '.html' -out x.x";
        system "perl -p -i -e 's/SEE ALSO/See also/g;' x.x";
# don't report differences to stdout
	if (filediff (1, "$docdir/inc/$x.isee", "x.x")) {
	  system "cp x.x $docdir/inc/$x.isee";
	  chmod 0664, "$docdir/inc/$x.isee";
#	  print "$x.isee *replaced*\n";
          unlink "x.x";
        }
      } else {
# it doesn't exist, so create the new 'seealso' output
        system "seealso $x -auto -html -post '.html' -out $docdir/inc/$x.isee";
        system "perl -p -i -e 's/SEE ALSO/See also/g;' $docdir/inc/$x.isee";
        chmod 0664, "$docdir/inc/$x.isee";
        print "$x.isee *created*\n";
      }

# check to see if the CVS tree copy of the text documentation needs updating
      if (-e "$cvsdoc/text/$x.txt") {
# check to see if the text has changed
	system "lynx -dump -nolist $url/$x.html > x.x";
	if (filediff (0, "$cvsdoc/text/$x.txt", "x.x")) {
	  system "cp x.x $cvsdoc/text/$x.txt";
	  chmod 0664, "$cvsdoc/text/$x.txt";
#	  system "cvs commit -m'documentation updated' $cvsdoc/text/$x.txt";
	  $cvsdoctextcommit .= " $x.txt";
	  print "$x.txt *replaced*\n";
          unlink "x.x";
        }
      } else {
# it doesn't exist, so create the new text output
        system "lynx -dump -nolist $url/$x.html > $cvsdoc/text/$x.txt";
        chmod 0664, "$cvsdoc/text/$x.txt";
#       system "cvs add -m'documentation created' $cvsdoc/text/$x.txt";
        $cvsdoctextadd .= " $x.txt";
#       system "cvs commit -m'documentation created' $cvsdoc/text/$x.txt";
        $cvsdoctextcommit .= " $x.txt";
        print "$x.txt *created*\n";
      }

# check to see if the CVS tree copy of the html documentation needs updating
      if (-e "$cvsdoc/html/$x.html") {
# check to see if the html file has changed
      system "lynx -source $url/$x.html > x.x";
# change ../emboss_icon.gif and ../index.html to current directory
      system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
      system "perl -p -i -e 's#\.\.\/emboss_icon.gif#emboss_icon.gif#g;' x.x";
      if (filediff (0, "$cvsdoc/html/$x.html", "x.x")) {
	system "cp x.x $cvsdoc/html/$x.html";
	chmod 0664, "$cvsdoc/html/$x.html";
#	system "cvs commit -m'documentation updated' $cvsdoc/html/$x.html";
	$cvsdochtmlcommit .= " $x.html";
	print "$x.html *replaced*\n";
	unlink "x.x";
      }
    } else {
# it doesn't exist, so create the new html output
      system "lynx -source $url/$x.html > $cvsdoc/html/$x.html";
# change ../emboss_icon.gif and ../index.html to current directory
      system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' $cvsdoc/html/$x.html";
      system "perl -p -i -e 's#\.\.\/emboss_icon.gif#emboss_icon.gif#g;' $cvsdoc/html/$x.html";
      chmod 0664, "$cvsdoc/html/$x.html";
#     system "cvs add -m'documentation created' $cvsdoc/html/$x.html";
      $cvsdochtmladd .= " $x.html";
#     system "cvs commit -m'documentation created' $cvsdoc/html/$x.html";
      $cvsdochtmlcommit .= " $x.html";
      print "$x.html *created*\n";
    }

  }

}

# end the index.html file
indexfooter(INDEX);
close(INDEX);

# check to see if the index.html file has changed
if (filediff (1, "$cvsdoc/html/index.html", "i.i")) {    
  system "cp i.i $cvsdoc/html/index.html";
  chmod 0664, "$cvsdoc/html/index.html";
#  system "cvs commit -m'index.html updated' $cvsdoc/html/index.html";
  $cvsdochtmlcommit .= " index.html";
  print "index.html *replaced*\n";
  unlink "i.i";
}


# look at all applications and report the ones with missing documentation
foreach $x (sort (keys %progs)) {
    if ($progdone{$x}) {next}
    print "$x.html =missing=\n";
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
# change ../emboss_icon.gif and ../index.html to current directory
    system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
    system "perl -p -i -e 's#\.\.\/emboss_icon.gif#emboss_icon.gif#g;' x.x";
    if (filediff (1, "$cvsdoc/html/$filename\_group.html", "x.x")) {
      system "cp x.x $cvsdoc/html/$filename\_group.html";
      chmod 0664, "$cvsdoc/html/$filename\_group.html";
#     system "cvs commit -m'documentation updated' $cvsdoc/html/$filename\_group.html";
      $cvsdochtmlcommit .= " $filename\_group.html";
      print "$filename\_group.html *replaced*\n";
      unlink "x.x";
    }
  } else {
# it doesn't exist, so create the new html output
    system "lynx -source $url/$filename\_group.html > $cvsdoc/html/$filename\_group.html";
# change ../emboss_icon.gif and ../index.html to current directory
    system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' $cvsdoc/html/$filename\_group.html";
    system "perl -p -i -e 's#\.\.\/emboss_icon.gif#emboss_icon.gif#g;' $cvsdoc/html/$filename\_group.html";
    chmod 0664, "$cvsdoc/html/$filename\_group.html";
#   system "cvs add -m'documentation created' $cvsdoc/html/$filename\_group.html";
    $cvsdochtmladd .= " $filename\_group.html";
#   system "cvs commit -m'documentation created' $cvsdoc/html/$filename\_group.html";
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
# change ../emboss_icon.gif and ../index.html to current directory
  system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' x.x";
  system "perl -p -i -e 's#\.\.\/emboss_icon.gif#emboss_icon.gif#g;' x.x";
  if (filediff (1, "$cvsdoc/html/groups.html", "x.x")) {
    system "cp x.x $cvsdoc/html/groups.html";
    chmod 0664, "$cvsdoc/html/groups.html";
#   system "cvs commit -m'documentation updated' $cvsdoc/html/groups.html";
    $cvsdochtmlcommit .= " groups.html";
    print "groups.html *replaced*\n";
    unlink "x.x";
  }
} else {
# it doesn't exist, so create the new html output
  system "lynx -source $url/groups.html > $cvsdoc/html/groups.html";
# change ../emboss_icon.gif and ../index.html to current directory
  system "perl -p -i -e 's#\.\.\/index.html#index.html#g;' $cvsdoc/html/groups.html";
  system "perl -p -i -e 's#\.\.\/emboss_icon.gif#emboss_icon.gif#g;' $cvsdoc/html/groups.html";
  chmod 0664, "$cvsdoc/html/groups.html";
# system "cvs add -m'documentation created' $cvsdoc/html/groups.html";
  $cvsdochtmladd .= " groups.html";
# system "cvs commit -m'documentation created' $cvsdoc/html/groups.html";
  $cvsdochtmlcommit .= " groups.html";
  print "groups.html *created*\n";
}

######################################################################
# OK - we have updated all our files, now CVS add and CVS commit them 
######################################################################

chdir "$cvsdoc/html";

if ($cvsdochtmladd ne "") {
  system "cvs add -m'documentation created' $cvsdochtmladd";
}
if ($cvsdochtmlcommit ne "") {
  system "cvs commit -m'documentation created' $cvsdochtmlcommit";
}

chdir "$cvsdoc/text";

if ($cvsdoctextadd ne "") {
  system "cvs add -m'documentation created' $cvsdoctextadd";
}
if ($cvsdoctextcommit ne "") {
  system "cvs commit -m'documentation created' $cvsdoctextcommit";
}

print "Create make files\n";
chdir "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/scripts";
system("./makeMake.pl");	# no parameter == do text
system("./makeMake.pl html");

