#!/usr/bin/perl
use warnings;
use Getopt::Long
    qw(:config no_bundling no_ignore_case no_auto_abbrev auto_help);
use Pod::Usage;

use vars qw/%OPTS/;

#
# False entry point for perl
#
#
sub main {
	my (%opts, $ok);
	$ok = arguments(result => \%OPTS);
	return -1 if (!$ok);

	open(BLOCKS, $OPTS{blocklist});

	my $count = 0;
	foreach my $block (<BLOCKS>) {
		chomp($block);
		$count++;
		my $str = sprintf("%02i", $count);
		vout(0, "Working on Basic Block $str [$block]\n");
		create_cache(%OPTS, prefix => $str, address => $block);
	}

	close(BLOCKS);
	return 0;
}

#
# Parses the command line arguments.
#
# Usage:
#     $ok = arguments(result => \%opts)
#
# Side Effects
#     %opts = ( iterations => $num,
#               address => $cache_addr,
#               verbose => $num );
#
sub arguments {
	my (%args, $opts);
	%args = @_;
	$opts = $args{result};

	$opts->{verbose} = 0;
	$opts->{tsim} = "tsim-leon3";
	$opts->{dest} = "caches";
	$opts->{lastcache} = "last-cache.pl";

	my $ok = GetOptions("verbose|v+" => \$opts->{verbose},
			    "tsim=s" => \$opts->{tsim},
			    "dest=s" => \$opts->{dest}
	    );

	$opts->{binary} = shift @ARGV;
	if (!defined($opts->{binary})) {
		pod2usage("No binary given");
		return 0;
	}

	$opts->{blocklist} = shift @ARGV;
	if (!defined($opts->{blocklist})) {
		pod2usage("No blocklist given");
		return 0;
	}

	vout(1, "Binary being used is " . $opts->{binary} . "\n");
	vout(1, "Blocklist being used is " . $opts->{blocklist} . "\n");
	vout(1, "Destination directory is " . $opts->{dest} . "\n");
	vout(1, "last-cache path is " . $opts->{lastcache} . "\n");

	if (-f $opts->{dest} ||
	    -d $opts->{dest}) {
		vout(0, $opts->{dest} . " exists, aborting\n");
		return 0;
	}
	mkdir $opts->{dest};

	return $ok;
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
# create_cache
#
# Creates the cache files for a binary, and a basic block
#
# Usage:
#     create_cache(prefix => $str,
#                  tsim => $path,
#                  lastcache => $path,
#                  dest => $dir_path,
#                  address => $addr);
#
sub create_cache {
	my %args = @_;
	my ($lastcache, $tsim, $binary, $address, $pfx, $dest);
	
	($lastcache, $tsim, $binary, $address, $pfx, $dest) =
	    @args{'lastcache', 'tsim', 'binary', 'address', 'prefix', 'dest'};

	my $cmd = "$lastcache --tsim=$tsim $binary $address\n";

	open(CMD, "$cmd |");
	my (@icache, @dcache);
	while (my $line = <CMD>) {
		next if ($line =~ /icache/);
		last if ($line =~ /dcache/);
		push @icache, $line;
	}
	while (my $line = <CMD>) {
		last if ($line =~ /^tsim/);
		push @dcache, $line;
	}
	close(CMD);


	my ($ipath, $dpath);
	$ipath = "$dest/icache.dat.$pfx.$address";
	$dpath = "$dest/dcache.dat.$pfx.$address";

	open(ICACHE, "> $ipath");
	print ICACHE @icache;
	close(ICACHE);

	open(DCACHE, "> $dpath");
	print DCACHE @dcache;
	close(DCACHE);
}

exit main();


#
# POD
#

__END__

=head1 NAME

collect-caches.pl -- Collects the cache contents for an ordered list
  of basic blocks

=head1 SYNOPSIS

	collect-caches.pl [OPTIONS] <binary> <blocklist>

=head1 DESCRIPTION

Each line the text file C<blocklist> must contain one address that
represents the end of a basic block. This program will collect the
cache contents at the final visit of the basic block, dropping them in
a new directory.

=over

=item --verbose|-v

Each --verbose or -v option will increase the verbosity of the output.

=item --tsim

Specify the path to tsim-leon3 if not in the users path

=item --dest

Directory where the output files will go, default is caches

=item --lastcache

Path to lastcache perlscript

=back

=cut

#
# File Local Variables
#

# Local Variables:
# perl-indent-level: 8
# End:
