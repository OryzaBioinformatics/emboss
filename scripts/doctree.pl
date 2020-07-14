#!/usr/bin/perl -w

# Converts the EMBOSS Web pages on the Sanger Centre Web server
# to a version that is easy to distribute and can be included
# in the CVS source tree
#
# Files are copied, replaced or converted. If they woudl change the
# current CVS version of the file then the change is committed.
#
# Copied:
#    Selected files (*.gif) are simply copied
#
# Replaced:
#    Header files of various kinds are replaced by an alternative version
#    Include file (*.ihelp *.itable *.iauth) are merged with the .shtml files
#
# Converted:
#    For most HTML files:
#       #include files are replaced with the full file contents
#       Links are made relative even if they were absolute
#
# Diagnostics
#    Information messages start with '#' (comment) and '=' (unchanged)
#    Error messages start with '**'
#    WebLint messages start with 'y.y'

$topsrc = "/nfs/WWW/htdocs/Software/EMBOSS";
$tophome = "/nfs/WWW/htdocs/";
$topdir = "/nfs/disk42/pmr/WWW/EMBOSS";
$topurl = "http://www.hgmp.mrc.ac.uk/Software/EMBOSS";
$totfile=0;
$totdir=0;
$dofile = 0;
$docopy=0;
$doskip=0;
$doignore=0;

print "Using topdir $topdir\n";
print "Using topurl $topurl\n";

dirtest ($topsrc, "");

print "$totfile files in $totdir directories\n";
print "  skipped: $doskip\n";
print "   copied: $docopy\n";
print "  ignored: $doignore\n";
print "processed: $dofile\n";
exit;

sub dirtest ($$) {

    local ($src, $subsrc) = @_;
    local ($file) = "";
    local (*DIR);
    opendir (DIR, "$src");

    while (defined($file = readdir(DIR))) {
	if ($file =~ /^[.#]/) {print "# ignore $src/$file\n";$doignore++;next}
	if (-d "$src/$file") {
	    $totdir++;
	    print "# directory $file\n";
	    dirtest ("$src/$file", "$subsrc/$file");
	}
	else {fileprocess ("$src/$file", "$subsrc")}
    }
    closedir DIR;

    return;
}

sub urlprocess ($$$) {
    local ($url, $dir, $name) = @_;
    local ($type) = 0;
    local ($full) = "";
    while (!$type) {
	if ($url =~ 'mailto:') {
	    print "=email address $url\n";
	    $type = 81;
	    next;
	}
	if ($url =~ '^ftp:') {
	    print "=ftp server $url\n";
	    $type = 82;
	    next;
	}
	if ($url =~ '^http://([^/]+)/(.*)') {
	    print "= remote  $url\n";
	    $type = 71;
	    next;
	}
	if ($url =~ '^http://www.sanger.ac.uk/(.*)') {
	    print "= full sanger $url\n";
	    $type = 31;
	    $full = $tophome.$1;
	    next;
	}
	if ($url =~ 'cgi-bin') {
	    print "= CGI $url\n";
	    $type = 35;
	    next;
	}
	if ($url =~ /^\/$/) {
	    print "= Sanger top, *replace* $url\n";
	    $type = 38;
	    $full = "$tophome/";
	    $url = "http://www.sanger.ac.uk/";
	    next;
	}
	if ($url =~ '^/(.+)') {
	    print "= top page *replace* $url\n";
	    $type = 32;
	    $full = $tophome.$1;
	    $url = "http://www.sanger.ac.uk/$1";
	    next;
	}
	if ($url =~ /\/$/) {
	    print "+ dir page $url\n";
	    $type = 33;
	    $full = $dir.$url."index.shtml";
	    $url .= "index.html";
	    next;
        }
        if ($url =~ '/') {
	    print "= subdir page $url\n";
	    $full = $dir.$url;
	    if ($url =~ '(.*)[.]shtml(.*)') {$url = "$1.html$2"}
	    $type = 34;
	    next;
	}
        if ($url =~ '(.*)[.]shtml(.*)') {
	    print "+ shtml page $url\n";
	    $type = 34;
	    $url = "$1.html$2";
	    next;
	}
	print "=local page $url\n";
	$type = 30;
	next;
    }

    if ($full ne "") {
	if (-e "$full") {print "# file $full\n"}
	else {print "** no such file $full\ncited in $dir$name\n"}
    }

    return $url;
}

sub cgiprocess ($$$*) {
    local ($cgifile,$dir,$name, *HOUT) = @_;
    local ($htxt) = "";
    local ($hurl) = "";

    if ($cgifile eq "/cgi-bin/header2") {
	if ($vars{title} ne "EMBOSS") {print STDERR "using title $vars{title}\n"}
	print HOUT "
<HTML>

<HEAD>
  <TITLE>
  $vars{title}
  </TITLE>
</HEAD>
<BODY BGCOLOR=\"#FFFFFF\" text=\"#000000\">
";

# open table for page header

	print HOUT "
<TABLE BORDER=\"1\" WIDTH=\"100%\">
 <TR valign=\"top\">
";

#	print STDERR "# navigator: $vars{navigator3}\n";
#	print STDERR "## navigator3\n";

	$test = $vars{navigator3};
	while ($test =~ /[ \n]*([^\n,;]+);([^,]*),/g) {
	    $htxt = $1;
	    $hurl = $2;
	    $hurl =~ s/\/Software\/EMBOSS\///;
	    $htxt =~ s/<\/?b>//;
	    $htxt =~ s/<.*>//;
#	    print STDERR "## '$htxt' => '$hurl'\n";
	}

# Left header - local stuff

print HOUT "
  <TD WIDTH=\"30%\" ALIGN=\"LEFT\" VALIGN=\"TOP\">
    <TT><FONT FACE=\"Arial,Helvetica,sans-serif\" SIZE=\"-1\"> |
";
#<A HREF=\"index.html\">EMBOSS&nbsp;Docs</A> |

#	print STDERR "# navigator2: $vars{navigator2}\n";
#	print STDERR "## navigator2\n";
	$test = $vars{navigator2};
	while ($test =~ /\s*([^\n,;]+);([^,]*),/g) {
	    $htxt = $1;
	    $hurl = $2;
	    $hurl =~ s/\/Software\/EMBOSS\///;
	    $htxt =~ s/<\/?b>//g;
	    $htxt =~ s/<[^>]*>//g;
	    $htxt =~ s/ +/\&nbsp;/g;
	    if ($hurl eq "") {$hurl="index.html"}
	    print STDERR "## HEAD '$htxt' => '$hurl'\n";
	    print HOUT "<A HREF=\"$hurl\">$htxt</A> |\n";
	}
	print HOUT "</FONT></TT>
     </TD>
";

# Central header - global EMBOSS stuff

	print HOUT "
  <TD ALIGN=\"CENTER\" VALIGN=\"TOP\">
    <TT><FONT FACE=\"Arial,Helvetica,sans-serif\" SIZE=\"-1\"> |
";

#	print STDERR "# navigator: $vars{navigator}\n";
#	print STDERR "## navigator\n";
	$test = $vars{navigator};
	while ($test =~ /\s*([^\n,;]+);([^,]*),/g) {
	    $htxt = $1;
	    $hurl = $2;
	    $hurl =~ s/\/Software\/EMBOSS\///;
	    $htxt =~ s/<\/?b>/g/;
	    $htxt =~ s/<.*>//;
	    $htxt =~ s/ +/\&nbsp;/g;
#	    print STDERR "## '$htxt' => '$hurl'\n";
	    print HOUT "<A HREF=\"$hurl\">$htxt</A> |\n";
	}
	print HOUT "
</FONT></TT>
  </TD>
";

# Right header - Logo

	print HOUT "
  <TD WIDTH=\"55\" ALIGN=\"RIGHT\" VALIGN=\"TOP\">
      <IMG WIDTH=\"55\" HEIGHT=\"55\" ALT=\"\" BORDER=0 SRC=\"http://www.hgmp.mrc.ac.uk/Software/EMBOSS/emboss_icon.gif\">
  </TD>
";
	print HOUT "
 </TR>
</TABLE>
  <P>
";

# open table for page content

	print HOUT "
<CENTER>
<TABLE BORDER=\"0\" CELLPADDING=\"0\" CELLSPACING=\"0\" WIDTH=\"95%\">
<TR VALIGN=\"top\">
<TD>
";
#	print STDERR "$cgifile reprinted\n";
	return;
    }
    if ($cgifile eq "/cgi-bin/footer") {

# page content ends here
# close table for page content

	print HOUT "
</TD></TR></TABLE>
</CENTER>
";

#  table for page footer

#	if ($vars{author} ne "pmr") {print STDERR "using author $vars{author}\n#"}
#	print HOUT "
# <HR ALIGN=\"CENTER\" WIDTH=\"90%\">
#
#
#<TABLE BORDER=\"0\" WIDTH=\"100%\">
# <TR>
#  <TD ALIGN=LEFT>
#   <I>
#   last modified : 12-Jun-2000, 12:23 PM
#   </I>
#  </TD>
#
#  <TD ALIGN=RIGHT>
#   <A HREF=\"http://www.sanger.ac.uk/Users/$vars{author}/\">Peter Rice</A>
#   <I>(<A HREF=\"mailto:$vars{author}\@sanger.ac.uk\">$vars{author}\@sanger.ac.uk</A>)</I>
#  </TD>
# </TR>
#</TABLE>
#";

# closing BODY and HTML tags

	print HOUT "
</BODY>
</HTML>

";
#	print STDERR "$cgifile reprinted\n";
	return;
    }
#    print STDERR "= cgi file $cgifile in $dir$name\n";
    print "= cgi file $cgifile in $dir$name\n";
    return;
}

sub incprocess ($$**) {
    local ($dir, $incfile, *OUT, *HOUT) = @_;
    local (*IN);
    local ($i) = 0;
    local ($incfname) = "$dir$incfile";
    local ($varname) = "";
    local ($modtext) = "";
    local ($modname) = "";
    local ($modtime) = "";

    if ($incfile =~ /(.*)header([^\/]*)[.]shtml$/) {
	$incfile = $1."header".$2.".xhtml";
	$incfname = "$dir$incfile";
#	print STDERR "** rename header to $incfile\n";
    }
    if ($incfile =~ /^\/(.*)/) {
	$incfname = "$tophome$1";
    }

#    print STDERR "** include file $incfname\n";
    print "include file $incfname\n";
    print OUT "$incfname ... ";
    open (IN, "$incfname") || die "Cannot open include file $incfname";
    $i = 0;
    while (<IN>) {
	if (/^<!--#config/) {while (! /-->/ ){$_=<IN>} next}
	if (/^<!--#if/) {while (! /-->/ ){$_=<IN>} next}
	if (/<!--#flastmod virtual=\"([^\"]+)\"/) {
	    $modname = "$dir$1";
	    $modtime = -M $modname;
	    print STDERR "* flastmod $modname $modtime\n";
	    print "* flastmod $modname $modtime\n";
	    print HOUT "<i>modtime unknown</i>\n";
	    next;
	}
	if (/^<!--#set var=\"[^\"]+\"/) {
	    $settext = $_;
	    while (! /-->/ ){
		$_=<IN>;
		$settext .= $_;
	    }
	    if ($settext =~ /var=\"(\S+)\" +value=\"(.*)\"[ \t\n]*-->/gos) {
#		print STDERR "** set '$1' = '$2'\n";
		$vars{$1} = $2;
		$varname = $1;
		$modtext = $2;
		$modtext =~ s/[\n]/\n##/g;
		print "# set '$varname' = '$modtext'\n";
	    }
	    else {
		print STDERR "** trouble parsing '$settext'\n";
	    }
	    next;
	}
	if (/^<!--#endif/) {next}
	if (/^<!--AUTHOR:/) {next}
	if (/^<!--#exec +cgi *= *\"([^\"]+)\"/) {
	    $cgifile = $1;
#	    print STDERR "** cgi $cgifile from $dir$name\n";
	    print "# cgi $cgifile from $dir$name\n";
	    cgiprocess ($cgifile, $dir, $name, HOUT);
	    $cgi++;
	    next;
	}
	if (/^<!--#include +virtual *= *\"([^\"]+)\"/i) {
	    $incfile2 = $1;
#	    print STDERR "** include virtual $incfile2 from $incfname\n";
	    print "# include virtual $incfile2 from $incfname\n";
	    incprocess ($dir, $incfile2, OUT, HOUT);
	    $inc++;
	    next;
	}
	if (/^<!--#include +file *= *\"([^\"]+)\"/i) {
	    $incfile2 = $1;
#	    print STDERR "** include file $incfile2 from $incfname\n";
	    print "# include file $incfile2 from $incfname\n";
	    incprocess ($dir, $incfile2, OUT, HOUT);
	    $inc++;
	    next;
	}
	if (/^<!--#include/i ) {
	    print "* include unknown type found '$_'\n";
	    next;
	}
	$i++;
	print HOUT;
    }
    close IN;
    print OUT "$i lines\n";
    return;
}

sub fileprocess ($$) {
    local ($file, $subdir) = @_;
    local (*IN);
    local (*OUT);
    local (*HOUT);
    local ($i) = 0;
    local ($inc) = 0;
    local ($cgi) = 0;
    local ($dir) = "";
    local ($name) = "";
    local ($incfile) = "";
    local ($cgifile) = "";
    local ($url) = "";
    local ($h2) = 0;
    local ($h2bg) = 0;
    local ($newname) = "";
    local ($line) = "";
    local ($urlcnt) = 0;
    local ($ipos) = 0;
    local ($more) = 0;
    local ($newurl) = "";
    local ($varname) = "";
    local ($modtext) = "";
    local ($modname) = "";
    local ($modtime) = "";
    local ($saveline) = "";

    %vars = ("title" => "EMBOSS unknown",
	     "author" => "peter.rice"
	     );

    $totfile++;

    ($dir, $name) = ($file =~ /(.*\/)?([^\/]+)$/);

    #print "process $file ($name)\n";

    $newname = "$topdir$subdir/$name";
    $newname =~ s/shtml$/html/;
#    print STDERR "new name '$newname\n";

    if (! -d "$topdir$subdir") {
	print STDERR "unknown directory '$topdir$subdir'\n";
	mkdir ("$topdir$subdir", 0777);
    }

    if ($name =~ /~$/ )        {print "- skip $name\n";$doskip++;return}
    if ($name =~ /^header/ )   {print "- skip $name\n";$doskip++;return}
    if ($name =~ /^footer/ )   {print "- skip $name\n";$doskip++;return}
    if ($name =~ /[.]ihelp$/)  {print "- skip $name\n";$doskip++;return}
    if ($name =~ /[.]itable$/) {print "- skip $name\n";$doskip++;return}
    if ($name =~ /[.]ihtml$/)  {print "- skip $name\n";$doskip++;return}
    if ($name =~ /[.]gif$/)    {print "+ copy $name\n";$docopy++;
				filecopy ("$file","$newname");
				return}
    if ($name =~ /[.]seq$/)    {print "+ copy $name\n";$docopy++;
				filecopy ("$file","$newname");
				return}
    if ($name =~ /[.]em$/)    {print "+ copy $name\n";$docopy++;
				filecopy ("$file","$newname");
				return}
    if ($name =~ /[.]gcg$/)    {print "+ copy $name\n";$docopy++;
				filecopy ("$file","$newname");
				return}
    if ($name =~ /mylist$/)    {print "+ copy $name\n";$docopy++;
				filecopy ("$file","$newname");
				return}

    print "# process $name\n";
    $dofile++;

    if (! -d "$topdir$subdir") {
	print STDERR "** still unknown directory '$topdir$subdir'\n";
    }
    else {
	if (! -e "$newname") {
	    print STDERR "unknown file '$newname'\n";
	}
    }

    open (IN, $file) || die "failed to read $file";
    open (OUT, ">z.z") || die "failed to open z.z";
    open (HOUT, ">y.y") || die "failed to open y.y";

    while (<IN>) {
	$i++;
	if (/\/h2>/) {
	    $h2++;
	}
	if (/<table width=\"100%\" +cellspacing=\"0\" +cellpadding=\"0\" *>/i) {
	    $saveline = $_;
	    print "Saving line $saveline\n";
	    $line = <IN>;
	    print "Next line $line\n";
	    $more = 1;
	    if ($line !~ /class=\"?h2bg/i) {
		print HOUT $saveline;
		print HOUT $line;
	    }
	    else {  
		while ($more && ($line = <IN>)) {
		    if ($line =~ /<\/table>(.*)/i) {
			print HOUT $line;
			$more = 0;
			next;
		    }
		    if ($line =~ /<[Hh][0-9][^>]*>(.*)<\/[Hh][0-9][^>]*>/) {
			print "@ header '$1'\n";
			print HOUT "
<table CELLSPACING=0 CELLPADDING=0 BGCOLOR=\"#000070\" width=\"100%\">
<tr valign=\"top\">
<th ALIGN=CENTER><br>
<font face=\"Arial,Helvetica\" size=6 COLOR=\"#FFFFFF\">
$1
</font><br><br></th></tr>
";
		    }
		}
	    }
	    next;
	}
	if (/class=\"?h2bg/) {
	    $h2bg++;
	    print "* h2bg used in $dir$name\n";
	}
	if (/<table.* COLS=1 /) {s/ COLS=1//}
	if (/NOSAVE >/) {s/NOSAVE >/>/}
	if (/NOSAVE>/) {s/NOSAVE>/>/}
	if (/(<A NAME=[^>]+>)(<\/A>)/) {s/(<A NAME=[^>]+>)/$1\&nbsp;/}
	if (/^<!--#config/) {while (! /-->/ ){$_=<IN>} next}
	if (/^<!--#if/) {while (! /-->/ ){$_=<IN>} next}
	if (/<!--#flastmod virtual=\"([^\"]+)\"/) {
	    $modname = "$dir$1";
	    $modtime = -M $modname;
	    print STDERR "* flastmod $modname $modtime\n";
	    print "* flastmod $modname $modtime\n";
	    print HOUT "<i>modtime unknown</i>\n";
	    next;
	}
	if (/^<!--#set var=\"[^\"]+\"/) {
	    $settext = $_;
	    while (! /-->/ ){
		$_=<IN>;
		$settext .= $_;
	    }
	    if ($settext =~ /var=\"(\S+)\" +value=\"(.*)\"[ \t\n]*-->/gos) {
#		print STDERR "** set '$1' = '$2'\n";
		print "# set '$1' = '$2'\n";
		$vars{$1} = $2;
		$varname = $1;
		$modtext = $2;
		$modtext =~ s/[\n]/\n##/g;
		print "# set '$1' = '$modtext'\n";
	    }
	    else {
		print STDERR "** trouble parsing '$settext'\n";
	    }
	    next;
	}
	if (/#include +virtual *= *\"([^\"]+)\"/i) {
	    $incfile = $1;
	    print "# include virtual $incfile from $dir$name\n";
	    incprocess ($dir, $incfile, OUT, HOUT);
	    $inc++;
	    next;
	}
	if (/#include +file *= *\"([^\"]+)\"/i) {
	    $incfile = $1;
	    print "# include file $incfile from $dir$name\n";
	    incprocess ($dir, $incfile, OUT, HOUT);
	    $inc++;
	    next;
	}
	if (/^<!--#endif/) {next}
	if (/^<!--AUTHOR:/) {next}
	if (/#exec +cgi *= *\"([^\"]+)\"/) {
	    $cgifile = $1;
	    print "# cgi $cgifile from $dir$name\n";
	    cgiprocess ($cgifile, $dir, $name, HOUT);
	    $cgi++;
	    next;
	}
	if (/href=[ \n]*$/i) {	# split line
	    print "** split HREF in $dir$name $_";
	    print STDERR "** split HREF in $dir$name $_";
	}
	$line = $_;
        $urlcnt = 0;
	while ($line =~ /href=([^\" >]+)/ig) { # quote those URLs
	    $url = $1;
	    $line =~ s/href=$url/href=\"$url\"/i;
#	    print STDERR "quoted $line";
	}
	while ($line =~ /href=\"([^\"]+)\"/ig) { # URL
	    $url = $1;
#	    if (!$urlcnt) {print STDERR "\nURL(s) in line\n$line"}
	    print "# url $url from $dir$name \n";
	    $ipos = pos($line);
	    $newurl = urlprocess ($url, $dir, $name);
	    if ($url ne $newurl) {
		print "+ URL updated '$url' => '$newurl'\n";
#		print STDERR "+ URL updated '$url' => '$newurl'\n";
		$line =~ s/href=\"$url\"/href=\"$newurl\"/i;
#		print "# line => $line";
#		print STDERR "# line => $line";
		pos($line) = $ipos;
	    }
	    $urlcnt++;
	}
	print HOUT $line;
    }
    if ($h2 > $h2bg) {print "# Header h2 $h2 lines in $dir$name\n"}
    print OUT "$name $incfile ... $inc includes, $cgi cgi calls, $i lines\n";
    print "# $name $incfile ... $inc includes, $cgi cgi calls, $i lines\n";
    close IN;
    close OUT;
    close HOUT;
    print STDERR "# weblint $dir$name\n";
    print "========\n";
    print "# weblint $dir$name\n";
#    open (X, "y.y");while(<X>){print STDERR} close X;
# for 1.9.3 can use -d body-colors,
    open (WL, "weblint -d heading-order -x netscape y.y|") || die "Cannot start weblint for $dir$name";
    while (<WL>) {
	if (/empty container element <P>\.$/) {next}
	if (/empty container element <PRE>\.$/) {next}
	print;
    }
    close WL;
    print "========\n";
    if (-e "$newname") {
	if (filediff ("$newname", "y.y")) {
	    print STDERR "** $newname differences\n";
	    system "cp y.y $newname";
	}
	else {
	    print STDERR "== $newname unchanged\n";
	}
    }
    else {
	print STDERR "** new file $newname\n";
	system "cp y.y $newname";
    }
    return;
}

sub filediff ( $$ ) {
    my ($afile, $bfile) = @_;
    system ("diff $afile $bfile > a.a");
    if ( -s "a.a" ) {
	print "$afile ** differences **\n";
        # open (DIF, "a.a") || die "cannot open diff output file";
	# while (<DIF>) { print "> $_" } # print the differences
        # close DIF;
	return 1;
    }
    return 0;
}


sub filecopy ( $$ ) {
    my ($afile, $bfile) = @_;
    if (-e $bfile && !filediff ($afile, $bfile)) {return 0}
    print "* copy $afile\n";
    print STDERR "* copy $afile\n";
    system ("cp $afile $bfile");
    return 1;
}
