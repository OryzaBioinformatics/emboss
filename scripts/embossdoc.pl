#!/usr/local/bin/perl -w

use English;

sub srsref {
    return "<a href=\"/srs6bin/cgi-bin/wgetz?-e+[EFUNC-ID:$_[0]]\">$_[0]</a>";
}
sub srsdref {
    return "<a href=\"/srs6bin/cgi-bin/wgetz?-e+[EDATA-ID:$_[0]]\">$_[0]</a>";
}

sub secttest($$) {
    my ($sect, $ctype) = @_;
    my $stype = "";
    if ($sect =~ /Constructors$/) {$stype = "new"}
    elsif ($sect =~ /Destructors$/) {$stype = "delete"}
    elsif ($sect =~ /Assignments$/) {$stype = "assign"}
    elsif ($sect =~ /Iterators$/) {$stype = "iterate"}
    elsif ($sect =~ /Modifiers$/) {$stype = "modify"}
    elsif ($sect =~ /Casts$/) {$stype = "cast"}
    elsif ($sect =~ /Input$/) {$stype = "input"}
    elsif ($sect =~ /Output$/) {$stype = "output"}
    elsif ($sect =~ /Miscellaneous$/) {$stype = "use"}
    if ($stype eq "") {return $stype}
    if ($stype ne $ctype) {
	print "bad category '$ctype' in section '$sect'\n";
    }
    return $stype;
}

sub testnew($$) {
    my ($tdata, $ttype) = @_;
    if ($tdata ne $ttype) {
	print "bad category new - return type '$ttype' datatype '$tdata'\n";
    }
}

sub testdelete($$\@\@) {
    my ($tdata, $ttype, $tcast, $tcode) = @_;
    if ($ttype ne "void") {
	print "bad category delete - return type '$ttype' non-void\n";
    }
    if ($#{$tcast} < 0) {
	print "bad category delete - parameter missing\n";
	return 0;
    }
    $tx = ${$tcode}[0];
    if ($#{$tcast} > 0) {
	print "bad category delete - only one parameter allowed\n";
	return 0;
    }
    if (${$tcast}[0] ne "$tdata\*") {
	$tc = ${$tcast}[0];
	print "bad category delete - only parameter '$tc' must be '$tdata\*'\n";
    }
    if ($tx !~ /[d]/) {
	print "bad category delete - code1 '$tx' not 'd'\n";
    }
}

sub testassign($$\@\@) {
    my ($tdata, $ttype, $tcast, $tcode) = @_;
    if ($#{$tcast} < 0) {
	print "bad category assign - no parameters\n";
    }
    $tc = ${$tcast}[0];
    $tx = ${$tcode}[0];
    if ($tc ne "$cdata\*") {
	print "bad category assign - parameter1 '$tc' not '$tdata\*'\n";
    }
    if ($tx !~ /[w]/) {
	print "bad category assign - code1 '$tx' not 'w'\n";
    }
    if ($tx !~ /[D]/) {
	print "bad category assign - code1 '$tx' not 'D'\n";
    }
}

sub testmodify($$\@\@) {
    my ($tdata, $ttype, $tcast, $tcode) = @_;
    if ($#{$tcast} < 0) {
	print "bad category modify - no parameters\n";
    }
    $tc = ${$tcast}[0];
    $tx = ${$tcode}[0];
    if ($tc ne "$cdata" && $tc ne "$cdata\*") {
	print "bad category modify - parameter1 '$tc' not '$tdata\*'\n";
    }
    if ($tx !~ /[wu]/) {
	print "bad category modify - code1 '$tx' not 'w' or 'u'\n";
    }
}

sub testcast($$\@\@) {
    my ($tdata, $ttype, $tcast, $tcode) = @_;
    if ($#{$tcast} < 0) {
	print "bad category cast - no parameters\n";
	return 0;
    }
    if ($#{$tcast} == 0 && $ttype eq "void") {
	print "bad category cast - one parameter and returns void\n";
    }
    $tc = ${$tcast}[0];
    $tx = ${$tcode}[0];
    if ($tc ne "const $tdata") {
	print "bad category cast - parameter1 '$tc' not 'const $tdata'\n";
    }
    if ($tx !~ /[r]/) {
	print "bad category cast - code1 '$tx' not 'r'\n";
    }
}

sub testuse($\@\@) {
    my ($tdata, $tcast, $tcode) = @_;
    if ($#{$tcast} < 0) {
	print "bad category use - no parameters\n";
	return 0;
    }
    $qpat = qr/^const $tdata[*]*$/;
    $qpat2 = qr/^$tdata[*]* const[ *]*$/;
    $tc = ${$tcast}[0];
    $tx = ${$tcode}[0];
    $tc =~ s/^CONST /const /go;
    if ($tc !~ $qpat && $tc !~ $qpat2 && $tc ne "const void*") {
	print "bad category use - parameter1 '$tc' not 'const $tdata'\n";
    }
    if ($tx !~ /[r]/) {
	print "bad category use - code1 '$tx' not 'r'\n";
    }
}

sub testiterate($$$\@) {
    my ($tdata, $ttype, $tdesc, $tcast, $tcode) = @_;
    my ($itertype) = ($tdesc =~ /(^\S+)\s+iterator/);
    if (!$itertype) {
	print "bad category iterator - no type in description\n";
    }
    else {
	$tc = ${$tcast}[0];
	if ($ttype ne $itertype &&
	    $tc ne "$itertype" &&
	    $tc ne "$itertype\*") {
	    print "bad category iterate - type '$itertype' not referenced\n";
	}
    }
}

sub testinput($\@\@) {
    my ($tdata, $tcast, $tcode) = @_;
    my $ok = 0;
    my $i = 0;
    if ($#{$tcast} < 0) {
	print "bad category input - no parameters\n";
	return 0;
    }
	
    for ($i=0; $i <= $#{$tcast}; $i++) {
	$tc = ${$tcast}[$i];
	$tx = ${$tcode}[$i];
	if (($tc eq "$tdata" || $tc eq "$tdata*")&& ($tx =~ /[wu]/)) {
	    $ok = 1;
	}
    }
    if (!$ok) {
	print "bad category input - no parameter '$tdata' with code 'w' or 'u'\n";
    }
}

sub testoutput($\@\@) {
    my ($tdata, $tcast, $tcode) = @_;
    my $ok = 0;
    my $i = 0;
    if ($#{$tcast} < 0) {
	print "bad category output - no parameters\n";
	return 0;
    }
    for ($i=0; $i <= $#{$tcast}; $i++) {
	$tc = ${$tcast}[$i];
	$tx = ${$tcode}[$i];
	if ($tc eq "$tdata" || $tc eq "const $tdata") {
	    if  ($tx =~ /[ru]/) {
		$ok = 1;
	    }
	}
    }
    if (!$ok) {
	print "bad category output - no parameter (const) '$tdata' and code 'r' or 'u'\n";
    }
}

$pubout = "public";
$local = "local";
$infile = "";
$lib = "unknown";

### test is whether to test the return etc.
### body is whether to print the body code

%test = ("func" => 1, "funcstatic" => 1, "funclist" => 0, "prog" => 0);
%body = ("func" => 1, "funcstatic" => 1, "funclist" => 1, "prog" => 1);

%categs = ("new" => 1, "delete" => 1, "assign" => 1, "modify" => 1,
	   "cast" => 1, "use" => 1, "iterate" => 1,
	   "input" => 1, "output" => 1);
%ctot = ();
if ($ARGV[0]) {$infile = $ARGV[0];}
if ($ARGV[1]) {$lib = $ARGV[1];}

foreach $x ("short", "int", "long", "float", "double", "char",
	    "size_t", "time_t",
	    "unsigned", "unsigned char",
	    "unsigned short", "unsigned int",
	    "unsigned long", "unsigned long int") {
    $simpletype{$x} = 1;
}

foreach $x ("ajshort", "ajint", "ajuint", "ajlong", "ajulong",
	    "jobject", "jstring", "jboolean", "jclass", "jint", "jbyteArray",
	    "AjBool", "AjStatus", "BOOL", "AjEnum", "PLFLT", "PLINT",
	    "VALIST") {
    $simpletype{$x} = 1;
}

foreach $x ("CallFunc", "AjMessVoidRoutine", "AjMessOutRoutine") {
    $functype{$x} = 1;
}

$source = "";

if ($infile) {
    (undef, $dir, $pubout) = ($infile =~ /^(([^\/.]*)\/)*([^\/.]+)(\.\S+)?$/);
##    ($dummy, $dir, $pubout) = ($infile =~ /(([^\/.]+)\/)?([^\/.]+)(\.\S+)?$/);
    $local = $pubout;
    if ($dir) {$lib = $dir}
    print "set pubout '$pubout' lib '$lib'\n";
    open (INFILE, "$infile") || die "Cannot open $infile";
    while (<INFILE>) {$source .= $_}
}
else {
    while (<>) {$source .= $_}
}

open (HTML, ">$pubout.html");
open (HTMLB, ">$local\_static.html");
open (SRS, ">$pubout.srs");

$file = "$pubout\.c";
$title = "$file";

print HTML  "<html><head><title>$title</title></head>\n";
print HTML  "<body bgcolor=\"#ffffff\">\n";
print HTMLB "<html><head><title>$title</title></head>\n";
print HTMLB "<body bgcolor=\"#ffffff\">\n";

print HTML  "<h1>$file</h1>\n";
print HTMLB "<h1>$file</h1>\n";

$sect = $lastfsect = $laststatfsect = "";

##############################################################
## $source is the entire source file as a string with newlines
## step through each comment
## looking for extended JavaDoc style formatting
## $ccfull is the comment
## $rest is the rest of the file
##############################################################

while ($source =~ m"[/][*][^*]*[*]+([^/*][^*]*[*]+)*[/]"gos) {
    $ccfull = $&;
    $rest = $POSTMATCH;

    ($cc) = ($ccfull =~ /^..\s*(.*\S)*\s*..$/gos);
    if (defined($cc)) {
	$cc =~ s/[* ]*\n[* ]*/\n/gos;
    }
    else {
	$cc = "";
    }
    $type = "";
    $acnt = 0;
    $rtype = "";
    $ismacro = 0;
    $isprog = 0;
    $islist = 0;
    @largs = ();
    @savecode = ();
    @savevar = ();
    @savecast = ();
    while ($cc =~ m/@((\S+)([^@]+))/gos) {
	$data = $1;
	$token = $2;
	#print "<$token>\n";
	#print "$data\n";

	if ($token eq "section")  {
	    $OFILE = HTML;
	    ($sect, $srest) = ($data =~ /\S+\s+([^*\n]+)\s*(.*)/gos);
	    $sect =~ s/\s+/ /gos;
	    $sect =~ s/^ //gos;
	    $sect =~ s/ $//gos;
	    $srest =~ s/>/\&gt;/gos;
	    $srest =~ s/</\&lt;/gos;
	    $srest =~ s/\n\n/\n<p>\n/gos;
	    $srest =~ s/{([^\}]+)}/<a href="#$1">$1<\/a>/gos;
	    print "Section $sect\n";
	}

	if ($token eq "func" || $token eq "prog")  {
	    $ismacro = 0;
	    $isprog = 0;
	    if ($token eq "prog") {$isprog = 1}
	    $OFILE = HTML;
	    if ($sect ne $lastfsect) {
		print $OFILE "<hr><h2><a name=\"$sect\">\n";
		print $OFILE "$sect</a></h2>\n";
		print $OFILE "$srest\n";
		$lastfsect = $sect;
	    }
	    $type = $token;
	    ($name, $frest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    ($ftype,$fname, $fargs) =
		$rest =~ /^\s*([^\(\)]*\S)\s+(\S+)\s*[\(]\s*([^{]*)[)]\s*[\{]/os;
	    if ($isprog && $fname eq "main") {$fname = $pubout}
	    print "Function $name\n";
	    print $OFILE "<hr><h3><a name=\"$name\">\n";
	    print $OFILE "Function</a> ".srsref($name)."</h3>\n";
	    $srest = $frest;
	    $frest =~ s/>/\&gt;/gos;
	    $frest =~ s/</\&lt;/gos;
	    $frest =~ s/\n\n/\n<p>\n/gos;
	    print $OFILE "$frest\n";

	    print SRS "ID $name\n";
	    print SRS "TY public\n";
	    print SRS "MO $pubout\n";
	    print SRS "LB $lib\n";
	    print SRS "XX\n";

	    $ftype =~ s/\s+/ /gos;
	    $ftype =~ s/ \*/\*/gos;
	    $fname =~ s/^[\(]//gos;
	    $fname =~ s/[\)]$//gos;
	    if ($fname =~ /^Java_org.*Ajax_([^_]+)$/) {
		$fname = "Ajax.".$1;
		if ($ftype =~ /JNIEXPORT+\s+(\S+)\s+JNICALL/) {
		    $ftype = $1;
		}
	    }
	    if ($isprog && $ftype ne "int") {print "bad main type (not int)\n"}
	    if (!$ftype) {print "bad function definition\n"}
	    if ($fname ne $name) {print "bad function name <$name> <$fname>\n"}
	    if (!$frest) {print "bad function '$name', no description\n"}
	
	    $srest =~ s/\n\n+$/\n/gos;
	    $srest =~ s/\n\n\n+/\n\n/gos;
	    $srest =~ s/\n([^\n])/\nDE $1/gos;
	    $srest =~ s/\n\n/\nDE\n/gos;
	    $srest =~ s/>/\&gt;/gos;
	    $srest =~ s/</\&lt;/gos;
	    print SRS "DE $srest";
	    print SRS "XX\n";

	    $fargs =~ s/\s+/ /gos;    # all whitespace is one space
	    $fargs =~ s/ ,/,/gos;   # no space before comma
	    $fargs =~ s/, /,/gos;   # no space after comma
	    $fargs =~ s/ *(\w+) *((\[[^\]]*\])+)/$2 $1/gos;   # [] before name
	    $fargs =~ s/(\*+)(\S)/$1 $2/g;  # put space after run of *
	    $fargs =~ s/ \*/\*/gos;         # no space before run of *
	    $fargs =~ s/ [\(]\* (\w+)[\)]/* $1/gos;  # remove fn arguments
	    $fargs =~ s/(\w+)\s?[\(][^\)]+[\)],/function $1,/gos; # ditto
	    $fargs =~ s/(\w+)\s?[\(][^\)]+[\)]$/function $1/gos;  # ditto
	    $fargs =~ s/\s*\(\*(\w+)[^\)]*\)/\* $1/gs;
#           print "**functype <$ftype> fname <$fname> fargs <$fargs>\n";
	    @largs = split(/,/, $fargs);
#           foreach $x (@largs) {
#	        print "<$x> ";
#           }
#           print "\n";
#           print "-----------------------------\n";
	}

	if ($token eq "funcstatic")  {
	    $ismacro = 0;
	    $isprog = 0;
	    $OFILE = HTMLB;
	    if ($sect ne $laststatfsect) {
		print $OFILE "<hr><h2><a name=\"$sect\">\n";
		print $OFILE "$sect</a></h2>\n";
		print $OFILE "$srest\n";
		$laststatfsect = $sect;
	    }
	    $type = $token;
	    ($name, $frest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    ($ftype,$fname, $fargs) =
		$rest =~ /^\s*static\s+([^\(\)]*\S)\s+(\S+)\s*[\(]\s*([^{]*)[)]\s*[\{]/os;
	    print "Static function $name\n";
	    print $OFILE "<h3><a name=\"$name\">\n";
	    print $OFILE "Static function</a> ".srsref($name)."</h3>\n";
	    $srest = $frest;
	    $frest =~ s/>/\&gt;/gos;
	    $frest =~ s/</\&lt;/gos;
	    $frest =~ s/\n\n/\n<p>\n/gos;
	    print $OFILE "$frest\n";

	    print SRS "ID $name\n";
	    print SRS "TY static\n";
	    print SRS "MO $pubout\n";
	    print SRS "LB $lib\n";
	    print SRS "XX\n";

	    $ftype =~ s/\s+/ /gos;
	    $ftype =~ s/ \*/\*/gos;
	    if (!$ftype) {print "bad static function definition\n"}
	    if ($fname ne $name) {print "bad function name <$name> <$fname>\n"}
	    if (!$frest) {print "bad function '$name', no description\n"}

	    $srest =~ s/\n\n+$/\n/gos;
	    $srest =~ s/\n\n\n+/\n\n/gos;
	    $srest =~ s/\n([^\n])/\nDE $1/gos;
	    $srest =~ s/\n\n/\nDE\n/gos;
	    $srest =~ s/>/\&gt;/gos;
	    $srest =~ s/</\&lt;/gos;
	    print SRS "DE $srest";
	    print SRS "XX\n";

	    $fargs =~ s/\s+/ /gos;    # all whitespace is one space
	    $fargs =~ s/ ,/,/gos;   # no space before comma
	    $fargs =~ s/, /,/gos;   # no space after comma
	    $fargs =~ s/ *(\w+) *((\[[^\]]*\])+)/$2 $1/gos;   # [] before name
	    $fargs =~ s/(\*+)(\S)/$1 $2/g;  # put space after run of *
	    $fargs =~ s/ \*/\*/gos;         # no space before run of *
	    $fargs =~ s/ [\(]\* (\w+)[\)]/* $1/gos;  # remove fn arguments
	    $fargs =~ s/(\w+)\s?[\(][^\)]+[\)],/function $1,/gos;  # ditto
	    $fargs =~ s/(\w+)\s?[\(][^\)]+[\)]$/function $1/gos;  # ditto
	    $fargs =~ s/\s*\(\*(\w+)[^\)]*\)/\* $1/gs;
	    @largs = split(/,/, $fargs);
	}

	if ($token eq "macro")  {
	    $ismacro = 1;
	    $OFILE = HTML;
	    if ($sect ne $lastfsect) {
		print $OFILE "<hr><h2><a name=\"$sect\">\n";
		print $OFILE "$sect</a></h2>\n";
		print $OFILE "$srest\n";
		$lastfsect = $sect;
	    }

	    $type = $token; 
	    ($name, $mrest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    $fname = $name;
	    print "Macro $name\n";
	    ### print "args '$margs'\n";
	    print $OFILE "<hr><h3><a name=\"$name\">\n";
	    print $OFILE "Macro</a> ".srsref($name)."</h3>\n";
	    $srest = $mrest;
	    $mrest =~ s/>/\&gt;/gos;
	    $mrest =~ s/</\&lt;/gos;
	    $mrest =~ s/\n\n/\n<p>\n/gos;
	    print $OFILE "$mrest\n";

	    print SRS "ID $name\n";
	    print SRS "TY macro\n";
	    print SRS "MO $pubout\n";
	    print SRS "LB $lib\n";
	    print SRS "XX\n";

#           $ftype =~ s/\s+/ /gos;
#           $ftype =~ s/ \*/\*/gos;
#           if (!$ftype) {print "bad macro definition\n"}
#           if ($fname ne $name) {print "bad macro name <$name> <$fname>\n"}
#           if (!$frest) {print "bad macro '$name', no description\n"}

	    $srest =~ s/\n\n+$/\n/gos;
	    $srest =~ s/\n\n\n+/\n\n/gos;
	    $srest =~ s/\n([^\n])/\nDE $1/gos;
	    $srest =~ s/\n\n/\nDE\n/gos;
	    $srest =~ s/>/\&gt;/gos;
	    $srest =~ s/</\&lt;/gos;
	    print SRS "DE $srest";
	    print SRS "XX\n";
	}

	if ($token eq "funclist")  {
	    $ismacro = 0;
	    $isprog = 0;
	    $islist = 1;
	    $OFILE = HTMLB;
	    if ($sect ne $laststatfsect) {
		print $OFILE "<hr><h2><a name=\"$sect\">\n";
		print $OFILE "$sect</a></h2>\n";
		print $OFILE "$srest\n";
		$laststatfsect = $sect;
	    }
	    $type = $token;
	    ($name, $mrest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
	    print "Function list $name\n";
	    print $OFILE "<hr><h3><a name=\"$name\">\n";
	    print $OFILE "Macro</a> ".srsref($name)."</h3>\n";
	    $srest = $mrest;
	    $mrest =~ s/>/\&gt;/gos;
	    $mrest =~ s/</\&lt;/gos;
	    $mrest =~ s/\n\n/\n<p>\n/gos;
	    print $OFILE "$mrest\n";

	    print SRS "ID $name\n";
	    print SRS "TY list\n";
	    print SRS "MO $pubout\n";
	    print SRS "LB $lib\n";
	    print SRS "XX\n";

	    $srest =~ s/\n\n+$/\n/gos;
	    $srest =~ s/\n\n\n+/\n\n/gos;
	    $srest =~ s/\n([^\n])/\nDE $1/gos;
	    $srest =~ s/\n\n/\nDE\n/gos;
	    $srest =~ s/>/\&gt;/gos;
	    $srest =~ s/</\&lt;/gos;
	    print SRS "DE $srest";
	    print SRS "XX\n";
	}

	if ($token eq "param")  {
	    if (!$intable) {
		print $OFILE "<p><table border=3>\n";
		print $OFILE "<tr><th>RW</th><th>Name</th><th>Type</th><th>Description</th></tr>\n";
		$intable = 1;
	    }
	    ($code,$var,$cast, $prest) = ($data =~ m/[\[]([^\]]+)[\]]\s*(\S*)\s*[\[]([^\]]+[\]]?)[\]]\s*(.*)/gos);
	    if (!defined($code)) {
		print STDERR "bad \@param syntax:\n$data";
		next;
	    }

#           print "code: <$code> var: <$var> cast: <$cast>\n";
#           print "-----------------------------\n";
	    $cast =~ s/ \*/\*/gos;         # no space before run of *
	    $cast =~ s/\{/\[/gos;	# brackets fixed
	    $cast =~ s/\}/\]/gos;	# brackets fixed

	    if ($code !~ /^[rwufdv?][CDENPU]*$/) { # deleted OSU (all unused)
		print "bad code <$code> var: <$var>\n";
	    }

	    if ($ismacro) {               # No code to test for macros
	    }
	    else {
		$curarg = $largs[$acnt];
		if (!defined($curarg)) {
		    print "bad argument \#$acnt not found in prototype for <$var>\n";
		}
		else {
		    ($tcast,$tname) = ($curarg =~ /(\S.*\S)\s+(\S+)/);
		    if (!$tname) {
			$tcast = $curarg;
			if (!$var && $curarg eq "...") {
			    $var = $tname = "vararg";
			}
		    }
		    $castfix = $cast;
		    $castfix =~ s/^CONST +//go;
		    if (!$isprog && ($castfix ne $tcast)) {
			print "bad cast for $tname <$cast> <$tcast>\n";
		    }
		    if (!$isprog && ($var ne $tname)) {
			print "bad var <$var> <$tname>\n";
		    }
		}
	    }
	    $acnt++;

	    push @savecode, $code;
	    push @savevar,  $var;
	    push @savecast, $cast;

	    $drest = $prest;
	    $drest =~ s/\n\n+$/\n/gos;
	    $drest =~ s/\n\n\n+/\n\n/gos;
	    $drest =~ s/\n([^\n])/\nPD $1/gos;
	    $drest =~ s/\n\n/\nPD\n/gos;
	    $drest =~ s/>/\&gt;/gos;
	    $drest =~ s/</\&lt;/gos;
	    print SRS "PN [$acnt]\n";
	    print SRS "PA $code $var $cast\n";
	    print SRS "PD $drest";
	    print SRS "PX\n";

	    if (!$prest) {print "bad \@param '$var', no description\n"}
	    print $OFILE "<tr><td>$code</td><td>$var</td><td>$cast</td><td>$prest</td></tr>\n";

	    if ($simpletype{$cast}) {
# Simple C types (not structs)
# and EMBOSS types that resolve to simple types
		if ($code !~ /r/) {
		    print "bad \@param '$var' pass by value, code '$code'\n";
		}
	    }
	    elsif ($functype{$cast}) {
# Known function types - C and EMBOSS-specific
		if ($code !~ /f/) {
		    print "bad \@param '$var' function type '$cast', code '$code'\n";
		}
	    }
	    elsif ($cast =~ / function$/) {
# other function types
		if ($code !~ /f/) {
		    print "bad \@param '$var' function type '$cast', code '$code'\n";
		}
	    }
	    elsif ($cast =~ /^const .*[*][*]/) {
# Tricky - we can be read-only
# or we can set to any const char* string (for example)
# e.g. pcre error pointers
# but can be d (e.g. in ajTableMapDel functions)
		if ($code !~ /[rwud]/) {
		    print "bad \@param '$var' const ** but code '$code'\n";
		}
	    }
	    elsif ($cast =~ /^const /) {
#If it starts const - except const type ** (see above) - it is const
# One exception: pcre has a "const int*" array that is set
		if ($cast =~ /const[^a-z].*[*]/)
		{
		    if ($code !~ /[rwud]/) {
			print "bad \@param '$var' const($cast) but code '$code'\n";
		    }
		}
		elsif ($code !~ /r/) {
		    print "bad \@param '$var' const but code '$code'\n";
		}
	    }
	    elsif ($cast =~ / const[^a-z]/) {
# also if it has an internal const
# For example char* const argv[] is "char* const[]"
# One exception: pcre has a "register const uschar*" array that is set
		if ($cast =~ / const[^a-z].*[*]/)
		{
		    if ($code !~ /[rwud]/) {
			print "bad \@param '$var' const($cast) but code '$code'\n";
		    }
		}
		elsif ($code !~ /r/) {
			print "bad \@param '$var' const($cast) but code '$code'\n";
		}
	    }
	    elsif ($cast =~ / const$/) {
# For char* const (so far no examples)
# There could be exceptions - but not yet!
		if ($code !~ /r/) {
		    print "bad \@param '$var' const($cast) but code '$code'\n";
		}
	    }
	    elsif ($cast =~ /^[.][.][.]$/) {
# varargs can be ...
		if ($code !~ /v/) {
		    print "bad \@param '$var' type '...' but code '$code'\n";
		}
	    }
	    elsif ($cast =~ /^va_list$/) {
# varargs can also be va_list down the list
# we did use 'a' for this instead of 'v' but it is too confusing
		if ($code !~ /v/) {
		    print "bad \@param '$var' type '$cast' but code '$code'\n";
		}
	    }
	    elsif ($cast =~ /^void[*]$/) {
# hard to check - can be read, write, update or delete
		if ($code =~ /[?]/) {
		    print "bad \@param '$var' code '$code'\n";
		}
	    }
	    elsif ($cast =~ /^void[*]+$/) {
# hard to check - can be read, write, update or delete
# Note: maybe we can put a placeholder in the @param cast
		if ($code =~ /[?]/) {
		    print "bad \@param '$var' code '$code'\n";
		}
	    }
	    elsif ($cast =~ /[\]]$/) {
# hard to check - can be read, write, update or delete
# because we can't use const for these
# Note: maybe we can put a placeholder in the @param cast
		if ($code =~ /[?]/) {
		    print "bad \@param '$var' code '$code'\n";
		}
		if ($code =~ /r/) {
		    if ($cast =~ /^CONST +/) {
			$cast =~ s/^CONST +//o;
		    }
		    else
		    {
			print "bad \@param '$var' code '$code' but '$cast'\n";
		    }
		}
	    }
	    elsif ($cast =~ /[*]+$/) {
# hard to check - can be read, write, update or delete
# because we can't use const for these
# Note: maybe we can put a placeholder in the @param cast
		if ($code =~ /[?]/) {
		    print "bad \@param '$var' code '$code'\n";
		}
		if ($code =~ /r/) {
		    if ($cast =~ /^CONST +/) {
			$cast =~ s/^CONST +//o;
		    }
		    else
		    {
			print "bad \@param '$var' code '$code' but '$cast'\n";
		    }
		}
	    }
	    else {
# Standard checks for anything else
		if ($code =~ /r/) {
		    print "bad \@param '$var' code '$code' but not const\n";
		}
		if ($code =~ /[?]/) {
		    print "bad \@param '$var' code '$code'\n";
		}
	    }
	}

	if ($token eq "return")  {
	    if (!$intable) {
		print $OFILE "<p><table border=3>\n";
		print $OFILE "<tr><th>RW</th><th>Name</th><th>Type</th><th>Description</th></tr>\n";
		$intable = 1;
	    }
	    ($rtype, $rrest) = ($data =~ /\S+\s+\[([^\]]+)\]\s*(.*)/gos);
	    if (!$ismacro && !$isprog && $rtype ne $ftype) {
		print "bad return type <$rtype> <$ftype>\n";
	    }
	    if (!$rrest && $rtype ne "void") {
		print "bad \@return [$rtype], no description\n";
	    }
	    $rrest =~ s/>/\&gt;/gos;
	    $rrest =~ s/</\&lt;/gos;
	    print $OFILE "<tr><td>\&nbsp;</td><td>RETURN</td><td>$rtype</td><td>$rrest</td></tr>\n";
	    print $OFILE "</table><p>\n";
	    $intable = 0;

	    $drest = $rrest;
	    $drest =~ s/^$/\n/gos;  # make sure we have something
	    $drest =~ s/\n\n+$/\n/gos;
	    $drest =~ s/\n\n\n+/\n\n/gos;
	    $drest =~ s/\n([^\n])/\nRD $1/gos;
	    $drest =~ s/\n\n/\nRD\n/gos;
	    $drest =~ s/>/\&gt;/gos;
	    $drest =~ s/</\&lt;/gos;
	    print SRS "RT $rtype\n";
	    print SRS "RD $drest";
	    print SRS "RX\n";
	}

	if ($token eq "category")  {
	    if (!$intable) {
		print $OFILE "<p><table border=3>\n";
		print $OFILE "<tr><th>Datatype</th><th>Category</th><th>Description</th></tr>\n";
		$intable = 1;
	    }
	    ($ctype, $cdata, $crest) = ($data =~ /\S+\s+(\S+)\s+\[([^\]]+)\]\s*(.*)/gos);
	    if (!$crest) {
		print "bad \@category [$ctype], no description\n";
	    }

	    $crest =~ s/\s+/ /gos;
	    $crest =~ s/^ //gos;
	    $crest =~ s/ $//gos;
	    $crest =~ s/>/\&gt;/gos;
	    $crest =~ s/</\&lt;/gos;
	    print $OFILE "<tr><td>$cdata</td><td>$ctype</td><td>$crest</td></tr>\n";
	    print $OFILE "</table><p>\n";
	    $intable = 0;

	    $drest = $crest;
	    $drest =~ s/^$/\n/gos;  # make sure we have something
	    $drest =~ s/\n\n+$/\n/gos;
	    $drest =~ s/\n\n\n+/\n\n/gos;
	    $drest =~ s/\n([^\n])/\nCD $1/gos;
	    $drest =~ s/\n\n/\nCD\n/gos;
	    $drest =~ s/>/\&gt;/gos;
	    $drest =~ s/</\&lt;/gos;
	    print SRS "CT $rtype\n";
	    print SRS "CD $drest";
	    print SRS "CX\n";

	    print "category $ctype [$cdata] $fname $pubout $lib : $crest\n";
	    $ctot{$ctype}++;
	    secttest($sect,$ctype);

	    if (!defined($categs{$ctype})) {
		print "bad \@category [$ctype], unknown type\n";
	    }
	    elsif ($ctype eq "new") {
		testnew($cdata,$rtype);
	    }
	    elsif  ($ctype eq "delete") {
		testdelete($cdata, $rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "assign") {
		testassign($cdata,$rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "modify") {
		testmodify($cdata,$rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "cast") {
		testcast($cdata,$rtype,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "use") {
		testuse($cdata,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "iterate") {
		testiterate($cdata,$rtype,$crest,@savecast);
	    }
	    elsif  ($ctype eq "input") {
		testinput($cdata,@savecast,@savecode);
	    }
	    elsif  ($ctype eq "output") {
		testoutput($cdata,@savecast,@savecode);
	    }
	    else {
		print "bad category type '$ctype' - no validation\n";
	    }
	}

	if ($token eq "cc")  {
	    next;
	}

	if ($token eq "@")  {
	    last;
	}

    }
    if ($type) {
#       print "acnt: $acnt largs: $#largs\n";
#       print "type $type test $test{$type}\n";
	if ($test{$type}) {
	    if ($acnt == $#largs) {
		if ($largs[$#largs] ne "void") {
		    print "bad last argument: $largs[$#largs]\n";
		}
	    }
	    if ($acnt < $#largs) {   # allow one remaining
		$w=$#largs+1;
		print "bad \@param list $acnt found $w wanted\n";
	    }
	    if (!$rtype && $ftype ne "void") {print "bad missing \@return\n"}
	    print "=============================\n";
	}
	print SRS "//\n";

##############################################################
## do we want to save what follows the comment?
## Yes, for functions (and static functions) and main programs
## $rest is what follows the comment
##############################################################

	if (defined($body{$type}) && $body{$type} == 1) {

# body is the code up to a '}' at the start of a line

	    ($body) = ($rest =~ /(.*?\n\}[^\n]*\n)/os);
	    print SRS $body;
	}

	if (defined($test{$type}) && $test{$type} == 2) {

# body is the code up to a line that doesn't end with '\'

	    ($body) = ($rest =~ /\s*(\n\#define\s+[^(\n]+\s*[(][^)\n]*[)].*?[^\\])$/os);
	    print SRS "==FUNCLIST\n$body\n==ENDLIST\n";
	    print SRS "==REST\n$rest\n==ENDREST\n";
	}
    }
}

print HTML "</body></html>\n";
print HTMLB "</body></html>\n";
close HTML;
close HTMLB;
