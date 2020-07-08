#!/usr/local/bin/perl

$pubout = "public";
$locout = "local";
$infile = "";
$lib = "unknown";

if ($ARGV[0]) {$infile = $ARGV[0];}
if ($ARGV[1]) {$lib = $ARGV[1];}

$source = "";
$lasttoken = "";

if ($infile) {
    ($file) = ($infile =~ /([^\/]+)$/);
    ($dummy, $dir, $pubout) = ($infile =~ /(([^\/.]+)\/)?([^\/.]+)(\.\S+)?$/);
    $local = $pubout;
    if ($dir) {$lib = $dir}
    print "use pubout '$pubout' lib '$lib'\n";
    open (INFILE, "$infile") || die "Cannot open $infile";
    while (<INFILE>) {$source .= $_}
}
else {
    $file = "xxx.c";
    while (<>) {$source .= $_}
}

open (HTML, ">$pubout.html");
open (HTMLB, ">$pubout_static.html");
open (SRS, ">$pubout.srsdata");

$title = "$infile";

print HTML  "<html><head><title>$title</title></head><body bgcolor=\"#ffffff\">\n";
print HTMLB  "<html><head><title>$title</title></head><body bgcolor=\"#ffffff\">\n";

print HTML  "<h1>$pubout</h1>\n";
print HTMLB  "<h1>$pubout</h1>\n";

foreach $x ("new", "delete", "del", "ass", "mod", "use", "set", "cast", "use", "other") {
    $tables{$x} = 1;
}

while ($source =~ m"[/][*][^*]*[*]+([^/*][^*]*[*]+)*[/]"gos) {
  $ccfull = $&;
  $rest = $';

  ($cc) = ($ccfull =~ /^..\s*(.*\S)*\s*..$/gos);
  $cc =~ s/[* ]*\n[* ]*/\n/gos;

  $type = "";
  $acnt = 0;
  $rtype = "";
  @largs = ();
  while ($cc =~ m/@((\S+)([^@]+))/gos) {
    $data = $1;
    $token = $2;

    if ($token ne $lasttoken) {
	if ($tables{$lasttoken}) {print $OFILE "</table>\n"}
	$intable = 0;
    }
    if ($token eq "data")  {
      $OFILE = HTML;
      $type = $token;
      ($name, $frest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
      print "Data type $name\n";
      print $OFILE "<hr><h2>Data type ".srsdref($name)."</h2>\n";
      $srest = $frest;
      $frest =~ s/\n\n/\n<p>\n/gos;
      print $OFILE "$frest\n";

      print SRS "ID $name\n";
      print SRS "TY public\n";
      print SRS "MO $pubout\n";
      print SRS "LB $lib\n";
      print SRS "XX\n";

      if (!$frest) {print "bad data type '$name', no description\n"}

      $srest =~ s/\n\n+$/\n/gos;
      $srest =~ s/\n\n\n+/\n\n/gos;
      $srest =~ s/\n([^\n])/\nDE $1/gos;
      $srest =~ s/\n\n/\nDE\n/gos;
      print SRS "DE $srest";
      print SRS "XX\n";

    }

    if ($token eq "datastatic")  {
      $OFILE = HTMLB;
      $type = $token;
      ($name, $frest) = ($data =~ /\S+\s+(\S+)\s*(.*)/gos);
      print "Static data type $name\n";
      print $OFILE "<h2>Static data type $name</h2>\n";
      $srest = $frest;
      $frest =~ s/\n\n/\n<p>\n/gos;
      print $OFILE "$frest\n";

      print SRS "ID $name\n";
      print SRS "TY static\n";
      print SRS "MO $pubout\n";
      print SRS "LB $lib\n";
      print SRS "XX\n";

       if (!$frest) {print "bad data type '$name', no description\n"}

      $srest =~ s/\n\n+$/\n/gos;
      $srest =~ s/\n\n\n+/\n\n/gos;
      $srest =~ s/\n([^\n])/\nDE $1/gos;
      $srest =~ s/\n\n/\nDE\n/gos;
      print SRS "DE $srest";
      print SRS "XX\n";

    }

    if ($token eq "new")  {
      if (!$intable) {
	print $OFILE "<h3>Constructor(s)</h3>\n";
	print $OFILE "<p><table border=3>\n";
	print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
	$intable = 1;
      }
      ($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

      $drest = $prest;
      $drest =~ s/\n\n+$/\n/gos;
      $drest =~ s/\n\n\n+/\n\n/gos;
      $drest =~ s/\n([^\n])/\nND $1/gos;
      $drest =~ s/\n\n/\nND\n/gos;
      print SRS "NN $fname\n";
      print SRS "ND $drest";
      print SRS "NX\n";

      if (!$prest) {print "bad new spec '$fname', no description\n"}
      print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
    }

    if ($token eq "delete" || $token eq "del")  {
      if (!$intable) {
	print $OFILE "<h3>Destructor(s)</h3>\n";
	print $OFILE "<p><table border=3>\n";
	print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
	$intable = 1;
      }
      ($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

      $drest = $prest;
      $drest =~ s/\n\n+$/\n/gos;
      $drest =~ s/\n\n\n+/\n\n/gos;
      $drest =~ s/\n([^\n])/\nDD $1/gos;
      $drest =~ s/\n\n/\nDD\n/gos;
      print SRS "DN $fname\n";
      print SRS "DD $drest";
      print SRS "DX\n";

      if (!$prest) {print "bad delete spec '$fname', no description\n"}
      print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
    }

    if ($token eq "use")  {
      if (!$intable) {
	print $OFILE "<h3>Operator(s)</h3>\n";
	print $OFILE "<p><table border=3>\n";
	print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
	$intable = 1;
      }
      ($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

      $drest = $prest;
      $drest =~ s/\n\n+$/\n/gos;
      $drest =~ s/\n\n\n+/\n\n/gos;
      $drest =~ s/\n([^\n])/\nOD $1/gos;
      $drest =~ s/\n\n/\nOD\n/gos;
      print SRS "ON $fname\n";
      print SRS "OD $drest";
      print SRS "OX\n";

      if (!$prest) {print "bad use spec '$fname', no description\n"}
      print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
    }

    if ($token eq "ass" || $token eq "set")  {
      if (!$intable) {
	print $OFILE "<h3>Assignment(s)</h3>\n";
	print $OFILE "<p><table border=3>\n";
	print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
	$intable = 1;
      }
      ($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

      $drest = $prest;
      $drest =~ s/\n\n+$/\n/gos;
      $drest =~ s/\n\n\n+/\n\n/gos;
      $drest =~ s/\n([^\n])/\nAD $1/gos;
      $drest =~ s/\n\n/\nAD\n/gos;
      print SRS "AN $fname\n";
      print SRS "AD $drest";
      print SRS "AX\n";

      if (!$prest) {print "bad assignment spec '$fname', no description\n"}
      print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
    }

    if ($token eq "mod")  {
      if (!$intable) {
	print $OFILE "<h3>Modifier(s)</h3>\n";
	print $OFILE "<p><table border=3>\n";
	print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
	$intable = 1;
      }
      ($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

      $drest = $prest;
      $drest =~ s/\n\n+$/\n/gos;
      $drest =~ s/\n\n\n+/\n\n/gos;
      $drest =~ s/\n([^\n])/\nMD $1/gos;
      $drest =~ s/\n\n/\nMD\n/gos;
      print SRS "MN $fname\n";
      print SRS "MD $drest";
      print SRS "MX\n";

      if (!$prest) {print "bad modifier spec '$fname', no description\n"}
      print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
    }

    if ($token eq "cast")  {
      if (!$intable) {
	print $OFILE "<h3>Cast(s)</h3>\n";
	print $OFILE "<p><table border=3>\n";
	print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
	$intable = 1;
      }
      ($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

      $drest = $prest;
      $drest =~ s/\n\n+$/\n/gos;
      $drest =~ s/\n\n\n+/\n\n/gos;
      $drest =~ s/\n([^\n])/\nCD $1/gos;
      $drest =~ s/\n\n/\nCD\n/gos;
      print SRS "CN $fname\n";
      print SRS "CD $drest";
      print SRS "CX\n";

      if (!$prest) {print "bad cast spec '$fname', no description\n"}
      print $OFILE "<tr><td>".srsref($fname)."</td><td>$prest</td></tr>\n";
    }

    if ($token eq "other")  {
      if (!$intable) {
	print $OFILE "<h3>Other related data structure(s)</h3>\n";
	print $OFILE "<p><table border=3>\n";
	print $OFILE "<tr><th>Name</th><th>Description</th></tr>\n";
	$intable = 1;
      }
      ($fname,$prest) = ($data =~ m/\S+\s*(\S*)\s*(.*)/gos);

      $drest = $prest;
      $drest =~ s/\n\n+$/\n/gos;
      $drest =~ s/\n\n\n+/\n\n/gos;
      $drest =~ s/\n([^\n])/\nRD $1/gos;
      $drest =~ s/\n\n/\nRD\n/gos;
      $drest =~ s/^$/\n/gos;
      print SRS "RN $fname\n";
      print SRS "RD $drest";
      print SRS "RX\n";

      if (!$prest) {print "bad other spec '$fname', no description\n"}
      print $OFILE "<tr><td>".srsdref($fname)."</td><td>$prest</td></tr>\n";
    }

    if ($token eq "@")  {
	break;
    }

    $lasttoken = $token;
  }
  if ($type) {
    print "=============================\n";
    print SRS "//\n";

#    ($body) = ($rest =~ /(.*?\n\}[^\n]*\n)/gos);
#    print SRS $body;
  }

}

print HTML "</body></html>\n";
print HTMLB "</body></html>\n";
close HTML;
close HTMLB;

sub srsref {
    return "<a href=\"/srs5bin/cgi-bin/wgetz?-e+[EFUNC-ID:$_[0]]\">$_[0]</a>";
}
sub srsdref {
    return "<a href=\"/srs5bin/cgi-bin/wgetz?-e+[EDATA-ID:$_[0]]\">$_[0]</a>";
}
