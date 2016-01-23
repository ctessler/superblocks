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

	vout(1, "Reading blocklist " . $OPTS{blocklist} . "\n");
	open(BLOCKS, $OPTS{blocklist});
	my @blocks = <BLOCKS>;
	chomp(@blocks);
	close(BLOCKS);

	my (@unsorted, @sorted);
	@unsorted = grep { !/^''$/ } @blocks;
	@blocks = @unsorted;
	@unsorted = grep { defined($_) } @blocks;

	while (scalar(@unsorted) != 0) {
		print "Remaining breakpoints: " . scalar(@unsorted) . "\n";
		vout(1, "Opening an expect handle to tsim-leon3\n");
		my ($handle);
		use Expect;
		$handle = Expect->spawn("tsim-leon3", $OPTS{binary})
		    or die("Couldn't spawn tsim-leon3 $OPTS{binary}");
		$handle->log_stdout($OPTS{expect});
		$handle->expect(3, 'tsim>') or
		    die("Could not find tsim> prompt");
		vout(1, "Connected\n");

		# Set breakpoints
		my $breaks = gen_breakpoints(blocks => \@unsorted);
		if (!defined($breaks)) {
			@unsorted=();
			last;
		}
		vout(1, "Setting breakpoints\n");
		foreach my $cmd (split("\n", $breaks)) {
			print $handle "$cmd\n";
			$handle->expect(4, 'tsim>');
		}

		vout(1, "Finding the last breakpoint\n");
		# Find the last one
		my $index = last_bp(handle => $handle);
		if ($index == -1) {
			@unsorted=();
			last;
		}
		vout(1, "Last block is $index $unsorted[$index - 1]\n");
		$handle->close;

		# Remove the last one from unsorted, and put at the
		# end of sorted
		vout(1, "Removing $unsorted[$index - 1]\n");
		vout(1, join(" ", @unsorted). "\n");
		my $block = splice(@unsorted, $index - 1, 1);
		vout(1, join(" ", @unsorted). "\n");

		unshift(@sorted, $block);
	}


	#
	# Write the result
	#
	print "Writing to $OPTS{ofile}\n";
	open(OFILE, ">" . $OPTS{ofile});
	print OFILE join("\n", @sorted);
	print OFILE "\n";
	close(OFILE);

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
	$opts->{expect} = 0;
	$opts->{tsim} = "tsim-leon3";

	my $ok = GetOptions("output-file|o=s" => \$opts->{ofile},
			    "verbose|v+" => \$opts->{verbose},
			    "expect" => \$opts->{expect},
			    "tsim=s" => \$opts->{tsim}
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
	if (! -f $opts->{blocklist}) {
		pod2usage("No file " . $opts->{blocklist});
		return 0;
	}
	if (!defined($opts->{ofile})) {
		$opts->{ofile} = $opts->{binary} . "-ordered.txt"
	}

	vout(1, "Binary being used is " . $opts->{binary} . "\n");
	vout(1, "blocklist being used is " . $opts->{blocklist}
	     . "\n");

	return $ok;
}

#
# Generates the breakpoint commands
#
# $commands = gen_breakpoints(blocks => [ <addr1>, <addr2>, ... ]
#
sub gen_breakpoints {
	my (%args);
	%args = @_;

	my $command;
	foreach my $block (@{$args{blocks}}) {
		$command .= "break $block\n" if $block;
	}

	return $command;
}

#
# Finds the last breakpoint from those that are set
#
sub last_bp {
	my (%args, $handle);
	%args = @_;
	$handle = $args{handle};

	my ($running, $lastbp);
	$handle->clear_accum();
	print $handle "run\n";
	$running = 1;
	$lastbp = -1;

	while ($running == 1) {
		$handle->expect(3,
				[ qr/breakpoint\s+\d+/,
				  sub {
					  my $m = $handle->match();
					  if ($m =~ /(\d+)/) {
						  $lastbp = $1;
					  }
					  exp_continue;
				  }
				],
				[ qr/Program exited normally/,
				  sub {
					  $running = 0;
					  exp_continue;
				  }
				],
				[ qr/tsim>/ ]
		    );
		print $handle "c\n";
	}
	return $lastbp;
}


exit main();

__END__

=head1 NAME

order-blocks.pl - Orders the basic blocks based on their last visit.

=head1 SYNOPSIS

	order-blocks.pl [OPTION]... <binary> <block list> 

=head1 DESCRIPTION

Runs the binary image on the leon3 simulator setting a breakpoint at
each address of the block list. When the final block is discovered, it
is removed from the list and the process begins again. 

=over

=item --verbose|-v

Each --verbose or -v option will increase the verbosity of the output.

=item --tsim

Specify the path to tsim-leon3 if not in the users path

=item --output-file|-o <filename>

Name the output file, defaults to <binary>-order.txt

=back



# Local Variables:
# perl-indent-level: 8
# cperl-indent-level: 8
# End:
