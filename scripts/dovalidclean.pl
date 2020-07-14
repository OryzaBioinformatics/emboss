#!/usr/local/bin/perl -w

$name = shift @ARGV;

$cnt=0;
while (<>) {
    if (!$cnt++) {print "$name\n"}
    s/ \S+\/([^\/.]+[.]acd) / $1 /;
    print;
}

if ($cnt) {print "\n"}
