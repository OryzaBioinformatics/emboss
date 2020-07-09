#!/usr/local/bin/perl5 -w
#
# SOAP CGI server for EMBREO
#
# requires perl5 with the SOAP::LITE module
#
# to customize, change the directory name and list of modules
# in the dispatch_to line to match which modules and where you
# have them installed
#

use SOAP::Transport::HTTP;

SOAP::Transport::HTTP::CGI
  -> dispatch_to('/packages/soap/Modules','EmbreoFile','EmbossSoap')
  -> handle;
