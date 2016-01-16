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

use vars qw/$EXNAME/;

sub set_exname {
	$EXNAME = shift;
	$EXNAME .= "-" if $EXNAME ne "";
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

	set_exname("");
	$opts->{data} = 1;
	$opts->{instruction} = 1;
	my $ok = GetOptions("verbose|v+" => \$opts->{verbose},
			    "data|d!" => \$opts->{data},
			    "instruction|i!" => \$opts->{instruction},
			    "<>" => \&set_exname);

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

	my @iCaches = grep {/icache.dat./} <./*>; #/ perl Q_Q
	my @dCaches = grep {/dcache.dat./} <./*>; #/ perl Q_Q

	extract_ecb(prefix => 'icache', caches => \@iCaches)
	  if $opts{instruction};
	extract_ecb(prefix => 'dcache', caches => \@dCaches)
	  if $opts{data};

	extract_ucbs(prefix => 'icache', caches => \@iCaches)
	  if $opts{instruction};
	extract_ucbs(prefix => 'dcache', caches => \@dCaches)
	  if $opts{data};

	return 0;
}

#
# Saves the max ECB information as a cache state file, dumped as $pfx.ecb.dat
# suitable for comparison to $pfx.ucb.dat
#
sub extract_ecb {
	my (%args, $pfx, $names);
	%args = @_;
	$pfx = $args{prefix};
	$names = $args{caches};

	printf("Extracting ECBs for $pfx\n");

	my ($maxc, $ecb);
	$maxc = 0;
	foreach my $name (@$names) {
		my $cache = new Cache();
		$cache->importFile($name);

		my $reduced = new Cache();
		$reduced = $cache->copy();
		$reduced->removeBlanks();

		if ($reduced->lineCount() >= $maxc) {
			$maxc = $reduced->lineCount();
			$ecb = $cache->copy();
		}
	}

	my $outname = sprintf("$EXNAME%s.ecb.dat", $pfx);
	printf("Writing ECBs to $outname\n");
	open FILE, ">$outname";
	print FILE $ecb->toString();
	close FILE
}

#
# Save the max UCB information as a cache state file, dumped as $pfx.ucb.dat
# suitable for comparison to $pfx.ecb.dat
#
sub extract_ucbs {
	my (%args, $pfx, $names);
	%args = @_;
	$pfx = $args{prefix};
	$names = $args{caches};

	printf("Finding max UCB state for $pfx\n");
	my @caches = ();
	foreach my $name (@$names) {
		my $cache = new Cache();
		$cache->importFile($name);
		push @caches, $cache;
	}

	# $p - potential preemption point
	my ($maxc, $ucb);
	$maxc = 0;
	for (my $p = 1; $p < scalar(@caches); $p++) {
		# $l - last preemption point
		my $str = "$p/" . (scalar(@caches) - 1);
		print "$pfx: UCB search at block: $str ... \n";
		for (my $l = 0; $l < $p; $l++) {
			my $cache = $caches[$p - 1]; # last 
			for my $idx ($l .. $p - 1) {
				$cache = $cache->intersect($caches[$l]);
			}
			my $reduced = $cache->copy();
			$reduced->removeBlanks();
			my $candc = $reduced->lineCount();
			if ($candc > $maxc) {
				print "$pfx: New larger UCB count: $candc\n";
				$maxc = $candc;
				$ucb = $cache->copy();
			}
		}
	}

	my $outname = sprintf("$EXNAME%s.ucb.dat", $pfx);
	printf("Writing UCBs to $outname\n");
	open FILE, ">$outname";
	print FILE $ucb->toString();
	close FILE

}

exit main();

# Local Variables:
# mode: cperl
# cperl-indent-level: 8
# perl-indent-level: 8
# cperl-indent-parens-as-block: t
# End:
