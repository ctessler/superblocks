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

use vars qw/@NAMES/;
sub add_cache {
	push @NAMES, shift;
}

sub usage {

}

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

	use Getopt::Long
	  qw(:config no_bundling no_ignore_case no_auto_abbrev auto_help);

	$opts->{data} = 1;
	$opts->{instruction} = 1;
	my $ok = GetOptions("verbose|v+" => \$opts->{verbose},
			    "data|d!" => \$opts->{data},
			    "instruction|i!" => \$opts->{instruction},
			    "<>" => \&add_cache);

	if (scalar(@NAMES) != 2) {
		pod2usage("Two cache files are required");
	}

	return $ok;
}

#
# False entry point for perl
#
sub main {
	my %opts;
	my $ok = arguments(result => \%opts);
	if (!$ok) {
		return -1;
	}

	my $left = new Cache();
	$left->importFile($NAMES[0]);

	my $right = new Cache();
	$right->importFile($NAMES[1]);

	my $conflicts = $left->conflicts($right);

	$conflicts->removeBlanks();
	print "Conflicts: " . $conflicts->lineCount() . "\n";
	return 0;
}

exit main();

__END__

=head1 NAME

    ucb-ecb-count.pl -- calculates the UCB-ECB count between two cache states.

=head1 SYNOPSIS

    ucb-ecb-count.pl ucb.dat ecb.dat

=head1 DESCRIPTION

    ucb-ecb-count.pl finds the conflicts between two cache states. It assumes
    that any value in a cache line from one state will evict a line in the
    same position in the other cache state.



# Local Variables:
# mode: cperl
# cperl-indent-level: 8
# perl-indent-level: 8
# cperl-indent-parens-as-block: t
# End:
