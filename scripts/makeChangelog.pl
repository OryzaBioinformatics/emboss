#!/usr/local/bin/perl -w

# This is a utility to convert the ChangeLog file to HTML

###################################################################
#
# Some useful definitions
#
###################################################################

# where the CVS tree ChangeLog file is
my $cvsfile = "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/ChangeLog";

# where the web page file is
my $htmlfile = "/data/www/Software/EMBOSS/Doc/ChangeLog.html";


my @headings;	# list of heading titles
my $line;
my $i;
my $count;
my $pre_flag;

open (IN, "< $cvsfile") || die "Can't open $cvsfile\n";
open (OUT, "> $htmlfile") || die "Can't open $htmlfile\n";


# start HTML
print OUT qq|<HTML>

|;

# write warning note
print OUT qq|<!-- 

***                         DO NOT EDIT THIS FILE                        ***
*** IT IS AUTOMATICALLY PRODUCED BY THE EMBOSS SCRIPT 'makeChangelog.pl' ***

-->

|;


# write header stuff
print OUT qq|
<HEAD>
  <TITLE>
  EMBOSS: Change Log
  </TITLE>
</HEAD>
<BODY BGCOLOR="#FFFFFF" text="#000000">

<!--#include file="header1.inc" -->
Change Log
<!--#include file="header2.inc" -->  

|;

# parse source for headings
@headings = ();
while ($line = <IN>) {
    if ($line !~ /^\s/) {
	push @headings, $line
    }
}

# write contents list
print OUT qq|<H2>Contents</H2>
<UL>
|;
$count = 0;
foreach $i (@headings) {
    print OUT qq|<LI><A HREF="#$count">$i</A></LI>|;
    $count++;
}
print OUT qq|</UL>
|;

# parse source for text
seek IN, 0, 0;	# go back to start of file

$count = 0;
$pre_flag = 0;
while ($line = <IN>) {

# heading found
    if ($line !~ /^\s/) {
	if ($pre_flag) {
            print OUT qq|</PRE>|;
            $pre_flag = 0;
        }
	print OUT qq|<H2><A NAME="$count">$line</A></H2>|;
        $count++;
        next;
    }

# blank line is a paragraph end
    if ($line =~ /^\s*$/ || $line =~ /^\t*$/) {
	if ($pre_flag) {
            print OUT qq|</PRE>|;
            $pre_flag = 0;
        }
	print OUT qq|<P>|;
        next;
    }

# indent of more than a TAB is the start of a <PRE> block
    if ($line =~ /^\t\s+/ || $line =~ /^\t\t+/) {
        if (! $pre_flag) {
            $pre_flag = 1;
            print OUT qq|<PRE>|;
        }
        print OUT qq|$line|;
        next;
    }

# else just print the line
    if ($pre_flag) {
        print OUT qq|</PRE>|;
        $pre_flag = 0;
    }
    print OUT qq|$line|;

}



# end HTML
print OUT qq|</BODY>
</HTML>
|;

close (OUT);
close (IN);



print "Done.\n";
    