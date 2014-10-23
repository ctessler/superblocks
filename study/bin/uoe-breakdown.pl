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

	do {
		my $usage = breakdown();
		print sprintf("BRT: %3d U: %.2f\n", $OPTS{brt}, $usage);
		if (!$OPTS{table}) {
			$OPTS{brt} = 0;
		} else {
			$OPTS{brt} -= 10;
		}
	} while ($OPTS{brt} > 0);

	return 0;
}

sub breakdown {
	my @TASKS;
	foreach my $file (@{$OPTS{files}}) {
		my %task = read_file($file);
		$task{g} = $OPTS{brt} * $task{g};
		push @TASKS, \%task;
		my $str = sprintf("%-10s", $file);
		vout(1, "$str\tWCET: $task{c}\tUCBS: $task{g} (BRT)\n");
	}

	# u is the utilization parameter being modified
	my ($u, $gu, $usage, $done, $dbg);
	$u = scalar(@TASKS);

	# $prev - previous utilization
	# $mod - how much to adjust $u by
	my ($prev, $mod);
	($prev, $mod) = (0, .25);
	for ($done = 0; !$done; ) {
		# We're going to let perl's inaccuracy determine the
		# end point
		($gu, $usage, $dbg) = calc_u(u => $u, tasks => \@TASKS);
		qprint("Constant: $u, CRPDu: $gu, U: $usage\n");
		$done = 1 if $usage == $prev;

		if ($gu > 1) {
			$u += $mod;
		} else {
			$mod /= 2;
			$u -= $mod;
		}

		$prev = $usage;
	}
	foreach my $dbg (@$dbg) {
		vout(1, $dbg);
	}
	return $usage;
}

sub read_file {
	my $file = shift;

	my %result;
	open (FILE, $file) || return ();
	$result{name} = $file;
	foreach my $line (<FILE>) {
		chomp($line);

		if ($line =~ /WCET:\s+(\d+)/) {
			$result{c} = $1;
		}

		if ($line =~ /UCBS:\s+(\d+)/) {
			$result{g} = $1;
		}
	}
	close(FILE);

	if (!defined($result{c})) {
		pod2usage("$file does not contain WCET");
		return ();
	}

	if (!defined($result{g})) {
		pod2usage("$file does not contain UCBS");
		return ();
	}

	my $str = sprintf("%-10s", $file);
	vout(1, "$str\tWCET: $result{c}\tUCBS: $result{g}\n");

	return %result;
}

#
# ($CRPDu, $NOCRPDu) = calc_u(u => $u, tasks => \@TASKS);
#
sub calc_u {
	my (%args, $u, @t, @dbg);
	%args = @_;
	$u = $args{u};
	@t = @{$args{tasks}};

	# Calculate the current periods
	foreach my $task (@t) {
		my $period = $u * $task->{c};
		$task->{p} = $period;
	}

	# Calculate the utilization $gsum utilization with CRPD, $usum
	# utilization without CRPD
	my ($gsum, $usum); 
	($gsum, $usum) = (0, 0);

	foreach my $task (@t) {
		# calc aff($task, @t);
		# my @aff = map { $_->{p} > $task->{p} ? $_ : () } @t;
		my @aff = map { $_ != $task ? $_ : () } @t;
		my $maxucb = 0;
		foreach my $a (@aff) {
			if ($a->{g} > $maxucb) {
				vout(3, $task->{name} .
				     " UCB: " . $a->{g} .
				     "\n");
				$maxucb = $a->{g};
			}
		}

		push @dbg,
		  sprintf("%-10s\tWCET:%-10i BRT*UCBmax:%-10i WCET+BRT*UCBmax:%-10i\n",
			  $task->{name}, $task->{c}, $maxucb,
			  $task->{c}, $maxucb);

		my $gu = ($task->{c} + $maxucb) / $task->{p};
		my $pu = $task->{c} / $task->{p};
		$usum += $pu;
		$gsum += $gu;
	}

	return ($gsum, $usum, \@dbg);
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
# Displays a message only if it is not quiet
#
# Usage:
#     qprint(@messages);
sub qprint {
	return if $OPTS{quiet};
	print @_;
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

	$opts->{quiet} = 0;
	$opts->{verbose} = 0;
	$opts->{brt} = 3; # default

	my $ok = GetOptions("verbose|v+" => \$opts->{verbose},
			    "quiet|q" => \$opts->{quiet},
			    "brt|b=i" => \$opts->{brt},
			    "table|t" => \$opts->{table}
	    );

	vout(1, "Starting BRT is " . $opts->{brt} . " cycles\n");

	push @{$opts->{files}}, @ARGV;
	vout(1, "Task files:\n\t" . join("\n\t", @{$opts->{files}}) . "\n");

	return $ok;
}



exit main();

__END__

=head1 NAME

uoe-breakdown.pl -- calculates the breakdown utilization of the task
    set using the UCB only approach.

=head1 SYNOPSIS

    uoe-breakdown.pl [OPTIONS] <t1.txt> <t2.txt> ... <tn.txt>

=head1 DESCRIPTION

Calculates the breakdown utilization of the task set using the UCB
only approach under EDF.

=head1 TASK FILE DESCRIPTION

Each task file is comprised of two parameters. WCET is measured in
cycles, UCBS are cache line counts.

> cat task1.txt
WCET: 46000
UCBS: 190

=head1 OPTIONS

=over

=item --verbose|-v

Each --verbose or -v option will increase the verbosity of the output.

=item --brt|-b

The number of cycles to use for the block reload time, default is 3.

=item --quiet|-q

Disables all output except the final breakdown utilization

=item --table|-t

Outputs a table of decreasing block BRT values instead of a single
value.

=back

# Local Variables:
# perl-indent-level: 8
# End:
