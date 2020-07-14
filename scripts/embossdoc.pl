#!/usr/local/bin/perl

use English;

$pubout = "public";
$locout = "local";
$infile = "";
$lib = "unknown";

### test is whether to test the return etc.
### body is whether to print the body code

%test = ("func" => 1, "funcstatic" => 1, "funclist" => 0, "prog" => 0);
%body = ("func" => 1, "funcstatic" => 1, "funclist" => 1, "prog" => 1);

if ($ARGV[0]) {$infile = $ARGV[0];}
if ($ARGV[1]) {$lib = $ARGV[1];}

$source = "";

if ($infile) {
    ($dummy, $dir, $pubout) = ($infile =~ /^(([^\/.]*)\/)*([^\/.]+)(\.\S+)?$/);
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

print HTML  "<html><head><title>$title</title></head><body bgcolor=\"#ffffff\">\n";
print HTMLB "<html><head><title>$title</title></head><body bgcolor=\"#ffffff\">\n";

print HTML  "<h1>$file</h1>\n";
print HTMLB "<h1>$file</h1>\n";

$sect = $lastfsect = $laststatfsect = "";

##############################################################
## $source is the entire source file as a string with newlines
## step through each comment
## looking for extended JavaDoc style formatting
##############################################################

while ($source =~ m"[/][*][^*]*[*]+([^/*][^*]*[*]+)*[/]"gos) {
  $ccfull = $&;
  $rest = $POSTMATCH;

  ($cc) = ($ccfull =~ /^..\s*(.*\S)*\s*..$/gos);
  $cc =~ s/[* ]*\n[* ]*/\n/gos;

  $type = "";
  $acnt = 0;
  $rtype = "";
  $ismacro = 0;
  $isprog = 0;
  $islist = 0;
  @largs = ();
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
      $srest =~ s/{([^}]+)}/<a href="#$1">$1<\/a>/gos;
      print "Section $sect\n";
    }

    if ($token eq "func" || $token eq "prog")  {
      $ismacro = 0;
      $isprog = 0;
      if ($token eq "prog") {$isprog = 1}
      $OFILE = HTML;
      if ($sect NE $lastfsect) {
        print $OFILE "<hr><h2><a name=\"$sect\">\n";
        print $OFILE "$sect</a></h2>\n";
        print $OFILE "$srest\n";
	$lastfsect = $sect
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
      $fname =~ s/^[(]//gos;
      $fname =~ s/[)]$//gos;
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
      $fargs =~ s/ *(\w+) *((\[[^\]]*\])+)/$2 $1/gos;   # put [] before name
      $fargs =~ s/(\*+)(\S)/$1 $2/g;  # put space after run of *
      $fargs =~ s/ \*/\*/gos;         # no space before run of *
      $fargs =~ s/ [\(]\* (\w+)[\)]/* $1/gos;  # remove function arguments
      $fargs =~ s/(\w+)\s?[\(][^\)]+[\)],/function $1,/gos;  # remove function arguments
      $fargs =~ s/(\w+)\s?[\(][^\)]+[\)]$/function $1/gos;  # remove function arguments
      $fargs =~ s/\s*\(\*(\w+)[^\)]*\)/\* $1/gs;
#      print "**functype <$ftype> fname <$fname> fargs <$fargs>\n";
      @largs = split(/,/, $fargs);
#      foreach $x (@largs) {
#	print "<$x> ";
#      }
#      print "\n";
#      print "-----------------------------\n";
    }

    if ($token eq "funcstatic")  {
      $ismacro = 0;
      $isprog = 0;
      $OFILE = HTMLB;
      if ($sect NE $laststatfsect) {
        print $OFILE "<hr><h2><a name=\"$sect\">\n";
        print $OFILE "$sect</a></h2>\n";
        print $OFILE "$srest\n";
	$laststatfsect = $sect
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

      $xfargs = $fargs;
      $fargs =~ s/\s+/ /gos;    # all whitespace is one space
      $fargs =~ s/ ,/,/gos;   # no space before comma
      $fargs =~ s/, /,/gos;   # no space after comma
      $fargs =~ s/ *(\w+) *((\[[^\]]*\])+)/$2 $1/gos;   # put [] before name
      $fargs =~ s/(\*+)(\S)/$1 $2/g;  # put space after run of *
      $fargs =~ s/ \*/\*/gos;         # no space before run of *
      $fargs =~ s/ [\(]\* (\w+)[\)]/* $1/gos;  # remove function arguments
      $fargs =~ s/(\w+)\s?[\(][^\)]+[\)],/function $1,/gos;  # remove function arguments
      $fargs =~ s/(\w+)\s?[\(][^\)]+[\)]$/function $1/gos;  # remove function arguments
      $fargs =~ s/\s*\(\*(\w+)[^\)]*\)/\* $1/gs;
#      if ($fname =~ /tmap/) {
#	print "**static <$ftype> fname <$fname> fargs <$fargs> xfargs <$xfargs>\n"; 
#      }
      @largs = split(/,/, $fargs);
#      foreach $x (@largs) {
#	print "<$x> ";
#      }
#      print "\n";
#      print "-----------------------------\n";
    }

    if ($token eq "macro")  {
      $ismacro = 1;
      $OFILE = HTML;
      if ($sect NE $lastfsect) {
        print $OFILE "<hr><h2><a name=\"$sect\">\n";
        print $OFILE "$sect</a></h2>\n";
        print $OFILE "$srest\n";
	$lastfsect = $sect
      }

      $type = $token; 
      ($name, $mrest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
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

#      $ftype =~ s/\s+/ /gos;
#      $ftype =~ s/ \*/\*/gos;
#      if (!$ftype) {print "bad macro definition\n"}
#      if ($fname ne $name) {print "bad macro name <$name> <$fname>\n"}
#      if (!$frest) {print "bad macro '$name', no description\n"}

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
      if ($sect NE $laststatfsect) {
        print $OFILE "<hr><h2><a name=\"$sect\">\n";
        print $OFILE "$sect</a></h2>\n";
        print $OFILE "$srest\n";
	$laststatfsect = $sect
      }
      $type = $token;
      ($name, $frest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
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
#      print "code: <$code> var: <$var> cast: <$cast>\n";
#      print "-----------------------------\n";
      $cast =~ s/ \*/\*/gos;         # no space before run of *
      $cast =~ s/\{/\[/gos;	# brackets fixed
      $cast =~ s/\}/\]/gos;	# brackets fixed

      if ($code !~ /^[rwufdpv?][CENOPSU]*$/) {
          print "bad code <$code> var: <$var>\n";
      }
      $curarg = @largs[$acnt];
      ($tcast,$tname) = ($curarg =~ /(\S.*\S)\s+(\S+)/);
      if (!$tname) {
	  $tcast = $curarg;
	  if (!$var && $curarg eq "...") {$var = $tname = "vararg"}
      }
      if (!$ismacro && !$isprog && ($cast ne $tcast)) {
	print "bad cast <$cast> <$tcast>\n";
      }
      if (!$ismacro && !$isprog && ($var ne $tname)) {
	print "bad var <$var> <$tname>\n";
      }
      $acnt++;

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

    if ($token eq "@")  {
	break;
    }

  }
  if ($type) {
#    print "acnt: $acnt largs: $#largs\n";
#      print "type $type test $test{$type}\n";
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

    if ($body{$type} == 1) {

# body is the code up to a '}' at the start of a line

	($body) = ($rest =~ /(.*?\n\}[^\n]*\n)/os);
	print SRS $body;

    }

    if ($test{$type} == 2) {

# body is the code up to a line that doesn't end with '\'

      ($body) = ($rest =~ /\s*(\n#define\s+[^(\n]+\s*[(][^)\n]*[)].*?[^\\])$/os);
	print SRS "==FUNCLIST\n$body\n==ENDLIST\n";
	print SRS "==REST\n$rest\n==ENDREST\n";

    }
  }

}

print HTML "</body></html>\n";
print HTMLB "</body></html>\n";
close HTML;
close HTMLB;

sub srsref {
    return "<a href=\"/srs6bin/cgi-bin/wgetz?-e+[EFUNC-ID:$_[0]]\">$_[0]</a>";
}
sub srsdref {
    return "<a href=\"/srs6bin/cgi-bin/wgetz?-e+[EDATA-ID:$_[0]]\">$_[0]</a>";
}
