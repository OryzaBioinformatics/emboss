#!/usr/local/bin/perl -w

$infile = "stdin";

if ($ARGV[0]) {
  $infile = $ARGV[0];
}

$infile =~ m"^([^.]+)"o;

# $fpref = $1;

open IN, $infile || die "Cannot open input file $infile";

while (<IN>) {
  $allsrc .= $_;
}

print "\n";
print "============================\n";
print ".. File $infile\n";
print "============================\n";
@presrc = split (m"^typedef\s+struct\s+\S*\s*[{][^}]+[}]\s*[^;]+;\s*$"gosm, $allsrc);
$ip = 0;
$dalias="";

while ($allsrc =~ m"^typedef\s+struct\s+(\S*)\s*[{][^}]+[}]\s*([^;]+);\s*$"gosm) {
  $dnam = $1;
  $dalias = $2;
  @anam = (split(/[ \t,*]+/, $dalias));
  if (!defined($dnam) || $dnam eq "") {
    $dnam = $anam[$#anam];
  }
  $presrc = $presrc[$ip];

  if ($presrc =~ m"[\n][/][*]\s+[@]data[staic]*\s+(\S+)([^/*][^*]*[*]+)*[/]\s*$"osm) {
    $hnam = $1;
    $ok = 0;
    foreach $nam (@anam) {
      if ($hnam eq $nam) {$ok = 1}
    }
    if (!$ok && $dnam ne $hnam) {
      print "Bad docheader for $hnam precedes $dnam\n";
    }
  }
  else {
    print "Bad or missing docheader for $dnam\n";
  }
  $ip++;
}

close IN;
