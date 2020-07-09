#!/usr/local/bin/perl -w

# creates and commits the Makefile.am for the text and html doc directories
# the default is to do the text directory.
# any parameter causes it to do the html directory

my @list = ();
my $line = "";
my $flag = 0;
my $flag2 = 0;
my $dir;  
my $html;
my $cvsdoc;

# where the CVS tree program doc pages are
  $cvsdoc = "/packages/emboss_dev/$ENV{'USER'}/emboss/emboss/doc/programs/";

if ($#ARGV >= 0) {
  $html = 1;
  print "Doing HTML directory\n";
} else {
  $html = 0;
  print "Doing TEXT directory\n";
}


if ($html) {
  $dir = "html";
} else {
  $dir = "text";
}

chdir "$cvsdoc/$dir";
 
open(M, ">make.temp") || die "Can't open make.temp\n";
   
foreach $file (glob("*.gif"), glob("*.html"), glob("*.txt")) {  
#print ">$file<\n";
  if (length($line) + length($file) +1 > 60) {
    if ($flag) {
      print M " \\\n               ";
    } else {
      if ($flag2) {
        print M "\npkgdata2_DATA = ";
        $flag = 1;
      } else {
        print M "pkgdata_DATA = ";
        $flag = 1;
        $flag2 = 1;
      }
    }
    print M "$line";
    $line = $file;
  } else {
    $line .= " $file";
  } 
  if ($file =~ /^lindna.html/ || $file =~ /^lindna.txt/) {
#    print "Found lindna - breaking in half here\n";
    $flag = 0;
  }
}
print M " \\\n               $line\n\n";
print M "pkgdatadir=\$(prefix)/share/\$(PACKAGE)/doc/programs/$dir
pkgdata2dir=\$(prefix)/share/\$(PACKAGE)/doc/programs/$dir\n";


close (M);

# copy make.text to be Makefile.am
system("mv make.temp Makefile.am");

# cvs commit it
system("cvs commit -m'new makefile' Makefile.am");


