package ParseSoapOptions;

#
# parses a single string of the form "key1=value1 key2=value2"
# into a hash of keys and values
#

sub parse_soap_options {
    my ($soapstr) = @_;
    my @pairs = split(' ',$soapstr);
    my %form;
    foreach (@pairs) {
	my ($name, $value) = split(/=/, $_);
	$form{$name}=$value;
    }
    return %form;
}

1;
