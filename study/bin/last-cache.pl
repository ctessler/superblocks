#!/usr/bin/perl
use strict;
use warnings;
use Getopt::Long
    qw(:config no_bundling no_ignore_case no_auto_abbrev auto_help);
use Pod::Usage;

use File::Temp qw/tempfile tempdir/;
use POSIX ":sys_wait_h";

use vars qw/$ITERS %OPTS/;
$ITERS = 100;

#
# Finds the last data and instruction cache contents for a given
# memory location.
#
# The process is brute force. Setting a break ponit at the address,
# running to the break point, dumping the cache contents, and
# repeating. This is done $ITERS times.
#


#
# False entry point for perl
#
sub main { 
	my $ok = arguments(result => \%OPTS);
	if (!$ok) {
		return -1;
	}

	my ($batch_fh, $batch_name);
	($batch_fh, $batch_name) = tempfile(TMPDIR => 1, SUFFIX => '.bat');

	vout(1, "Writing data to $batch_name\n");

	gen_batch(fh => $batch_fh,
		  addr => $OPTS{address},
		  iterations => $OPTS{iterations});

        vout(1, "Data is ready in $batch_name\n");

	my @output = capture_cache(%OPTS, batch => $batch_name);

	vout(1, "Removing $batch_name");
	File::Temp::unlink0($batch_fh, $batch_name);
	File::Temp::cleanup();

	vout(0, @output);

	return 0;
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
# Populates the batch script
#
# Usage:
#     gen_batch(fh => $fh, 
#               addr => $address,
#               iterations => $iters);
#
sub gen_batch {
	my %args = @_;
	my $fh = $args{fh};

	print $fh "break " . $args{addr} . "\n";
	print $fh "run\n";
	for (1 .. $args{iterations}) {
		print $fh "reg\n";
		print $fh "icache\n";
		print $fh "dcache\n";
		print $fh "c\n";
	}
	print $fh "quit\n";
	
	$fh->flush();
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

	$opts->{iterations} = $ITERS;
	$opts->{verbose} = 0;
	$opts->{tsim} = "tsim-leon3";

	my $ok = GetOptions("iterations=i" => \$opts->{iterations},
			    "verbose|v+" => \$opts->{verbose},
			    "tsim=s" => $opts->{tsim}
	    );

	$opts->{binary} = shift @ARGV;
	if (!defined($opts->{binary})) {
		pod2usage("No binary given");
		return 0;
	}

	$opts->{address} = shift @ARGV;
	if (!defined($opts->{address})) {
		pod2usage("No address given");
		return 0;
	}
	

	vout(1, "Binary being used is " . $opts->{binary} . "\n");
	vout(1, "Address being checked is " . $opts->{address} . "\n");

	return $ok;
}


#
# Captures the cache information
#
# Usage:
#     capture_cache(tsim-path => $path,
#                   binary => $path,
#                   batch => $path,
#                   address => $addr,
#    );
#
sub capture_cache {
	my %args = @_;
	my (%kids, $cmd, @out);
	
	local $SIG{CHLD} = 
	    sub {
		    local ($!, $?);
		    my $pid = waitpid(-1, WNOHANG);
		    return if $pid == -1;
		    return unless defined $kids{$pid};

		    print "\nSIGCHLD CALLED\n";

		    delete $kids{$pid};
	    };

	$cmd = "tsim-leon3 " . $args{binary} . " -c " . $args{batch} . "\n";
	vout(1, "tsim command is : $cmd");
	     
	open(SIM, "$cmd |");
	@out = <SIM>;
	close(SIM);
	vout(2, @out);

	my ($first, @last_caches, $last_pc, $done);
	$first = 1;
	$last_pc = lc($args{address});
	while (my $line = shift @out) {
		next if ($line !~ /^tsim> reg/);
		# In register section
		my $cur_pc;
		while ($line = shift @out) {
			if ($line =~ /^tsim> icache/) {
				last;
			}
			if ($line =~ /\s+pc:\s+(\w+)\s+/) {
				$cur_pc = $1;
			} else {
				next;
			}
		}

		my @cur_cache;
		while (my $line = shift @out) {
			last if $line =~ /^tsim> c/;
			next if $line =~ /^tsim> dcache/;
			push @cur_cache, $line;
			
		}
		if ($first) {
			@last_caches = @cur_cache;
			$first = 0;
		}

		vout(1, "last_pc: $last_pc vs cur_pc: 0x$cur_pc\n");
		if ("0x$cur_pc" ne $last_pc) {
			last;
		}
		@last_caches = @cur_cache;
	}

	vout(2, "LAST CACHES\n", @last_caches);

	return @last_caches;
}





exit main();

__END__

=head1 NAME

last-cache.pl - Gathers the instruction and data cache state for the last
    invocation of an address

=head1 SYNOPSIS

	last-cache.pl [OPTION]... <binary> <address>

=head1 DESCRIPTION

Runs the binary image on the leon3 simulator setting a breakpoint at
address. After stopping at the breakpoint C<--iterations> number of times, the
instruction and data cache state is recorded.

=over

=item --iterations

The number of iterations to stop at the breakpoint, the default is 100.

=item --verbose|-v

Each --verbose or -v option will increase the verbosity of the output.

=item --tsim

Specify the path to tsim-leon3 if not in the users path

=back

# Local Variables:
# perl-indent-level: 8
# End:
