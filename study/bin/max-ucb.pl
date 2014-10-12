#!/usr/bin/perl
use strict;
use warnings;
use Getopt::Long
    qw(:config no_bundling no_ignore_case no_auto_abbrev auto_help);
use Pod::Usage;

use File::Temp qw/tempfile tempdir/;
use POSIX ":sys_wait_h";

use vars qw/%OPTS/;

#
# False entry point for perl
#
sub main { 
	my $ok = arguments(result => \%OPTS);
	if (!$ok) {
		return -1;
	}

	my %icache = read_cache($OPTS{I});
	my %dcache = read_cache($OPTS{D});

	my $max = maxed(icache => \%icache, dcache => \%dcache);

	print "Maximum UCB : $max\n";
	return 0;
}


sub read_cache {
	my $file = shift;
	
	my %cache;
	open(CACHE, "$file") || return ();
	<CACHE>; # Get rid of the header.
	foreach my $line (<CACHE>) {
		chomp($line);
		my @values = split(/\s+/, $line);
		my $point = shift @values;
		for (my $i =0 ; $i < scalar(@values); $i++) {
			$cache{$point}->{$i + $point + 1} = $values[$i];
		}
	}
	close(CACHE);

	vout(1, "Read $file\n");
	vout(1, "Program Point [Later Point:UCBS] ... \n");
	foreach my $point (sort {$a <=> $b} keys %cache) {
		vout(1, "$point");
		foreach my $later
		    (sort {$a <=> $b} keys $cache{$point}) {
			    vout(1, " [$later:"
				 . $cache{$point}->{$later}
				 . "]");
		}
		vout(1, "\n");
	}

	return %cache;
}

sub maxed {
	my (%args, %i, %d);
	%args = @_;
	%i = %{$args{icache}};
	%d = %{$args{dcache}};

	my $max = 0;
	my $point = 0;
	foreach my $point (keys %i) {
		foreach my $later (keys $i{$point}) {
			my $sum = $i{$point}->{$later} +
			    $d{$point}->{$later};

			if ($max < $sum) {
				vout(1, "Max UCBS ($point, $later)" .
				     " = $sum\n");
				$max = $sum;
			}
		}
	}

	return $max;
}
	

#
# Prints a message if the verbosity level is appropriate.
#
# Usage:
#     vout($level, @message);
#
sub vout {
	my ($level, @m);
	$level = shift;
	@m = @_;

	if ($level > $OPTS{verbose}) {
		return;
	}

	print @m;
}


#
# Parses the command line arguments.
#
# Usage:
#     $ok = arguments(result => \%opts)
#
sub arguments {
	my (%args, $opts);
	%args = @_;
	$opts = $args{result};

	$opts->{verbose} = 0;

	my $ok = GetOptions("verbose|v+" => \$opts->{verbose});

	$opts->{'I'} = shift @ARGV;
	$opts->{'D'} = shift @ARGV;

	if (!defined($opts->{I})) {
		pod2usage("No instruction cache given");
		return 0;
	}
	if (!defined($opts->{D})) {
		pod2usage("No data cache given");
		return 0;
	}
	vout(1, "Instruction cache file: " . $opts->{I} . "\n");
	vout(1, "Data cache file: " . $opts->{D} . "\n");

	if (! -f $opts->{I}) {
		pod2usage("Instruction cache file does not exist");
		return 0;
	}

	if (! -f $opts->{D}) {
		pod2usage("Data cache file does not exist");
		return 0;
	}

	return $ok;
}



exit main();

__END__

=head1 NAME

    max-ucb.pl -- calculates the program point with the greatest sum
    of shared UCBs as a sum of the shared UCBs in the instruction and
    data cache

=head1 SYNOPSIS

    max-ucb.pl [OPTIONS] imatrix.txt dmatrix.txt

=head1 DESCRIPTION

Finds the program point with the most shared UCBs as a sum of shared
UCB counts from the instruction and data caches.

=head1 OPTIONS

=over

=item --verbose|-v

Each --verbose or -v option will increase the verbosity of the output.

=back

# Local Variables:
# perl-indent-level: 8
# End:
