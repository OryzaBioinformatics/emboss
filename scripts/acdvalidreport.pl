#!/usr/bin/perl -w

%knownmsg = (
   "^Calculated standard value for \\S+" => "Calculated standard value",
   "^Calculated additional value for \\S+" => "Calculated additional value",
   "^Standard qualifier '\\S+' in section '\\S+'" => "Standard badsection",
   "^Additional qualifier '\\S+' in section '\\S+'" => "Additional badsection",
   "^Advanced qualifier '\\S+' in section '\\S+'" => "Advanced badsection",
   "^First \\S+ (\\S+ )?qualifier '\\S+' is not" => "Bad first qualname",
   "^\\S+ (\\S+ )?qualifier '\\S+' is not" => "Bad qualname",
   "^No knowntype specified for" => "Knowntype missing",
   "^Pattern but no knowntype specified for" => "Pattern without knowntype",
   "^Knowntype '[^']+' not defined" => "Knowntype undefined",
   "^Knowntype '[^']+' defined for type '[^']+', used" => "Knowntype wrong type",
   "^Expected \\S+ (\\S+ )?qualifier is '\\S+' found '\\S+'" => "Bad expected qualname",
   "^Unexpected information value for" => "Unexpected information",
   "^\\S+ string for '\\S+' starts in lower case" => "String start lowercase",
   "^\\S+ string for '\\S+' starts non-alphabetic" => "String start nonalpha",
   "Section info '[^']+' expected, case mismatch" => "Section info badcase",
   "^Qualifier '\\S+' type '\\S+' not in section" => "Qualifier badsection",
   "^No section defined for qualifier '\\S+'" => "No section",
   "^Sub level section '\\S+' should be under" => "Subsection badsection",
   "Parameter defined as false" => "Parameter false",
   "Section '\\S+' not defined in sections[.]standard file" => "Section undefined",
#   "" => "",
   "Dummy message" => "dummy"
	     );

%knownexp = ();

foreach $x (sort (keys (%knownmsg))) {
    $xname = $knownmsg{$x};
    $knownexp{$xname} = qr/$x/;
}

while (<>) {
#    if (/^(\S+)$/) {
#	$applname = $1;
#    }

    if (/^(Error|Warning): File ([^.]+)[.]acd line \d+: (.*)/) {
	$type = $1;
	$file = $2;
	$message = $3;
	$found=0;
	foreach $x (keys (%knownexp)) {
	    if ($message =~ /$knownexp{$x}/) {
		if ($type eq "Error") {$y = "* $x"}
		else {$y = $x}
		$countknown{$y}++;
#		print "$file: Message '$y' $countknown{$y}: $message\n";
		$found=1;
		last;
	    }
	}
	if (!$found) {
	    $countmsg{$message}++;
	    print "$file: Message '++other++' $countmsg{$message}: $message\n";
	}
    }
}

foreach $x (sort (keys ( %countknown ) ) ) {
    printf "%4d %s\n", $countknown{$x}, $x;
}
