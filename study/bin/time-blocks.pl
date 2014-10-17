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

	my (@trimmed);
	@trimmed = grep { !/^''$/ } @blocks;
	@blocks = @trimmed;
	@trimmed = grep { defined($_) } @blocks;

	my ($lastp, @result);

	vout(1, "Opening an expect handle to tsim-leon3\n");
	my ($handle, $hok);
	use Expect;
	$handle = Expect->spawn("tsim-leon3", $OPTS{binary});
	$handle->log_stdout($OPTS{verbose} >= 2 ? 1 : 0);
	$hok = $handle->expect(3, 'tsim>');
	vout(1, "Connected.\n");

	my $breakps = gen_breakpoints(blocks => \@trimmed);
	if (!defined($breakps)) {
		last;
	}

	vout(1, "Setting breakpoints $breakps\n");
	print $handle $breakps;
	$handle->expect(3, 'tsim>');

	my %result = last_bp(handle => $handle);
	my %delta = delta_cycles(\%result);

	@blocks = sort {$a <=> $b} grep { /\d+/ } keys %delta;

	foreach my $bp (sort { $a <=> $b } @blocks) {
		print "$bp $trimmed[$bp - 1] cycles: $delta{$bp}\n";
	}


	print "Removing first block, due to inclusion of initialization cycles\n";
	shift @blocks;

	my $sum = 0;
	foreach my $bp (sort { $a <=> $b } @blocks) {
		$sum += $delta{$bp};
	}

	# Write the results to files.
	# Number of cycles for the program
	print "Writing execution time to $OPTS{efile}\n";
	open(EFILE, ">$OPTS{efile}");
	print EFILE "WCET of task: $result{c}\n";
	print EFILE "WCET of timed blocks: $sum\n";
	close(EFILE);

	# Reduced (and correct number) of basic blocks
	print "Writing redacted ordered block set to $OPTS{bfile}\n";
	open(OFILE, ">$OPTS{bfile}");
	foreach my $b (@blocks) {
		print OFILE "$trimmed[$b - 1]\n";
	}
	close(OFILE);

	# The cycle time of each block
	print "Writing execution time of each block to $OPTS{cfile}\n";
	open(CFILE, ">$OPTS{cfile}");
	foreach my $b (@blocks) {
		print CFILE "$trimmed[$b - 1] $delta{$b}\n";
	}
	close(CFILE);

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
	$opts->{tsim} = "tsim-leon3";

	my $ok = GetOptions("output-blocks|b=s" => \$opts->{bfile},
			    "output-cycles|c=s" => \$opts->{cfile},
			    "output-execution|e=s" => \$opts->{efile},
			    "verbose|v+" => \$opts->{verbose},
			    "tsim=s" => $opts->{tsim}
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
	if (!defined($opts->{cfile})) {
		$opts->{cfile} = $opts->{binary} . "-cycles.txt"
	}

	if (!defined($opts->{bfile})) {
		$opts->{bfile} = $opts->{binary} . "-observed.txt"
	}

	if (!defined($opts->{efile})) {
		$opts->{efile} = $opts->{binary} . "-c.txt"
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
	my (%args, $handle, %result);
	%args = @_;
	$handle = $args{handle};

	# Start the program, should stop at the first breakpoint.
	$handle->clear_accum();
	print $handle "run\n";
	my ($running, $lastbp, $prevbp, $last_cycle);
	$running = 1;
	$lastbp = 0;
	$prevbp = 0;

	vout(1, "Gathering block order and cycle counts\n");
	vout(1, "\t");
	while ($running) {
		$handle->expect(10,
				[ qr/breakpoint\s+\d+\s+[^(at)]/,
				  sub {
					  my $m = $handle->match();
					  if ($m =~ /(\d+)/) {
						  $lastbp = $1;
					  }
				  }
				],
				[ qr/Program exited normally/,
				  sub {
					  $running = 0;
				  }
				]
		    );
		if (!$running) {
			last;
		}
		my $cycles = get_cycles(handle => $handle);
		vout(1, ".") if (($cycles % 100) == 0);
		if ($lastbp != $prevbp) {
			# First time hitting the new block
			$result{$lastbp} = $cycles;
		}
		$prevbp = $lastbp;

		print $handle "c\n";
	}

	$result{c} = get_cycles(handle => $handle);
	vout(1, "\nFinished after $result{c} cycles\n");
	vout(1, "Result:\n");
	for my $key (keys %result) {
		vout(1, "$key: $result{$key}\n");
	}

	return %result;
}

sub get_cycles {
	my (%args, $handle);
	%args = @_;
	$handle = $args{handle};

	print $handle "perf\n";

	my $cycles = 0;
	$handle->expect(3,
			[qr/Cycles.*:\s+\d+/,
			 sub {
				 my $m = $handle->match();
				 if ($m =~ /(\d+)/) {
					 $cycles = $1;
				 }
			 }
			],
	    );
	$handle->expect('trsim>');

	return $cycles;
}


#
# The last_bp records the cycles in the a result hash where
# %result = ( 1 => cycle count[1],
#             2 => cycle count[2] ... )
#
# What we want is the delta between the two
#
# %delta = delta_cycles(\%result)
#        = ( 1 => cycle count[1] - 0,
#            2 => cycle count[2] - cycle count[1], ... )
#
sub delta_cycles(\%) {
	my $href = shift;

	my %delta = ();
	my @keys = grep { /\d+/ } keys %$href;
	foreach my $key (sort {$a <=> $b} @keys) {
		my $prev = 0;
		my $i = 1;
		while ((($key - $i) >= 1) && ($prev == 0)) {
			$prev = $href->{$key - $i}
			    if defined($href->{$key - $i});
			$i++;
		}
		$delta{$key} = $href->{$key} - $prev;

		my $msg = "Breakpoint $key, previous $prev\n";
		$msg .= "\t" . $href->{$key} . " - $prev = $delta{$key}\n";
		vout(1, $msg);
	}
	return %delta;
}


exit main();

__END__

=head1 NAME

time-blocks.pl - Finds the number of cycles required for each basic block

=head1 SYNOPSIS

	time-blocks.pl [OPTION]... <binary> <ordered block list> 

=head1 DESCRIPTION

Runs the binary image on the leon3 simulator setting a breakpoint at
each address of the block list. The number of cycles between
breakpoints determines the cycles required for each block.

=over

=item --verbose|-v

Each --verbose or -v option will increase the verbosity of the output.

=item --tsim

Specify the path to tsim-leon3 if not in the users path

=item --output-cycles|-c <filename>

Name the output file for the cycles, defaults to <binary>-cycles.txt

=item --output-blocks|-b <filename>

Name the output file for the redacted set of blocks, defaults to
<binary>-observed.txt 

=item --output-execution|-e <filename>

Name the output file for the number of cycles the binary takes to
complete, defaults to <binary>-c.txt

=back



# Local Variables:
# perl-indent-level: 8
# cperl-indent-level: 8
# End:
