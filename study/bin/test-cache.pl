#!/usr/bin/perl
use warnings;
use strict;
use Data::Dumper;

use File::Basename;
use File::Spec;
use Cwd;

BEGIN {
	#
	# Local Modules
	#
	my $file = Cwd::abs_path(__FILE__);
	$file = File::Spec->canonpath($file);
	my $dirname = dirname($file);

	push @INC, "$dirname"; # bin
}

use Cache;

#
# Parses the command line arguments.
#
# Usage:
#     $ok = arguments(result => \%opts)
#
# Side Effects
#     %opts = ( verbose => $num );
#
sub arguments {
	my (%args, $opts);
	%args = @_;
	$opts = $args{result};

	my $set_file = sub {
	    $opts->{file} = shift;
	};

	use Getopt::Long
	  qw(:config no_bundling no_ignore_case no_auto_abbrev auto_help);

	my $ok = GetOptions("verbose|v+" => \$opts->{verbose},
			    '<>' => $set_file);

	if (!defined($opts->{file})) {
		print("An input file is required\n");
		return undef;
	}
	return $ok;
}

sub main {
	my %OPTS;
	my $ok = arguments(result => \%OPTS);
	if (!$ok) {
		return -1;
	}
	my $name = $OPTS{file};

	my $cache = new Cache();
	$cache->importFile($OPTS{file});

	print $cache->toString();

	return 0;
}

exit main();

# Local Variables:
# mode: cperl
# cperl-indent-level: 8
# perl-indent-level: 8
# cperl-indent-parens-as-block: t
# End:
