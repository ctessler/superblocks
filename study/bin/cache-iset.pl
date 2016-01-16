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
#     %opts = ( iterations => $num,
#               address => $cache_addr,
#               verbose => $num );
#
sub arguments {
	my (%args, $opts);
	%args = @_;
	$opts = $args{result};

	$opts->{data} = 1;
	$opts->{instruction} = 1;
	use Getopt::Long
	  qw(:config no_bundling no_ignore_case no_auto_abbrev auto_help);

	my $ok = GetOptions("verbose|v+" => \$opts->{verbose},
			    "data|d!" => \$opts->{data},
			    "instruction|i!" => \$opts->{instruction},
	    );
	return $ok;
}


sub do_instruction {
	my @inCaches = grep {/icache.dat./} <./*>; #/ perl Q_Q

	my @iCaches;
	foreach my $fname (@inCaches) {
		my $cache = new Cache(name => $fname);
		$cache->importFile($fname);
		push(@iCaches, $cache);
	}

	print "Processing " . scalar(@iCaches) . " instruction cache snapshots\n";

	print "Calculating UCBs for instruction caches\n";
	my %iUCBs = calcUCBs(@iCaches);
	my $iUCBu = UCBMax(@iCaches);

	#
	# Instruction Cache
	#
	print "\nInstruction Cache UCB Estimates\n";
	dispTaskUCBs($iUCBu);
	dispUCBs(%iUCBs);

	print "\nInstruction Cache Minimums\n";
	dispMINs(minUCBs(%iUCBs));

	print "\nInstruction Cache Maximums\n";
	dispMAXs(maxUCBs(%iUCBs));

	print "\nInstruction Cache Matrix\n";
	matrixUCBs(%iUCBs);

	print "\nInstruction Cache plot\n";
	gnuplotUCBs(min => 'icache.min.dat',
		    max => 'icache.max.dat',
		    file => 'icache.png',
		    title => 'Instruction Cache');

	#
	# Instruction Cache
	#
	open ICACHETASKUCB, '>iucb_bound.txt';
	select ICACHETASKUCB; $| = 1;
	dispTaskUCBs($iUCBu);
	select STDOUT;
	close ICACHETASKUCB;

	open ICACHEMIN, '>icache.min.dat';
	select ICACHEMIN; $| = 1;
	dispMINs(minUCBs(%iUCBs));
	select STDOUT;
	close ICACHEMIN;

	open ICACHEMAX, '>icache.max.dat';
	select ICACHEMAX; $| = 1;
	dispMAXs(maxUCBs(%iUCBs));
	select STDOUT;
	close ICACHEMAX;

	open ICACHEMAT, '>icache.matrix';
	select ICACHEMAT; $| = 1;
	matrixUCBs(%iUCBs);
	select STDOUT;
	close ICACHEMAT;

	open ICACHEPLOT, '>icache.p';
	select ICACHEPLOT; $| = 1;
	gnuplotUCBs(min => 'icache.min.dat',
		    max => 'icache.max.dat',
		    file => 'icache.png',
		    title => 'Instruction Cache');
	select STDOUT;
	close ICACHEPLOT;

	open ICACHEPLOT, '>icache-eps.p';
	select ICACHEPLOT; $| = 1;
	plotEPS(min => 'icache.min.dat',
		max => 'icache.max.dat',
		file => 'icache.eps',
		title => 'Instruction Cache');
	select STDOUT;
	close ICACHEPLOT;

	return $iUCBu;
}

sub do_data {
	my @daCaches = grep {/dcache.dat./} <./*>; #/
	print "Processing " . scalar(@daCaches) . " data cache snapshots\n";

	my @dCaches;
	foreach my $fname (@daCaches) {
		my $cache = new Cache(name => $fname);
		$cache->importFile($fname);
		push(@dCaches, $cache);
	}

	print "Calculating UCBs for data caches\n";
	my %dUCBs = calcUCBs(@dCaches);
	my $dUCBu = UCBMax(@dCaches);

	#
	# Data Cache
	#
	print "\nData Cache UCB Estimates\n";
	dispTaskUCBs($dUCBu);
	dispUCBs(%dUCBs);

	print "\nData Cache Minimums\n";
	dispMINs(minUCBs(%dUCBs));

	print "\nData Cache Maximums\n";
	dispMAXs(maxUCBs(%dUCBs));

	print "\nData Cache Matrix\n";
	matrixUCBs(%dUCBs);

	print "\nData Cache plot\n";
	gnuplotUCBs(min => 'dcache.min.dat',
		    max => 'dcache.max.dat',
		    file => 'dcache.png',
		    title => 'Data Cache');


	#
	# Data Cache
	#
	open DCACHETASKUCB, '>ducb_bound.txt';
	select DCACHETASKUCB; $| = 1;
	dispTaskUCBs($dUCBu);
	select STDOUT;
	close DCACHETASKUCB;

	open DCACHEMIN, '>dcache.min.dat';
	select DCACHEMIN; $| = 1;
	dispMINs(minUCBs(%dUCBs));
	select STDOUT;
	close DCACHEMIN;

	open DCACHEMAX, '>dcache.max.dat';
	select DCACHEMAX; $| = 1;
	dispMAXs(maxUCBs(%dUCBs));
	select STDOUT;
	close DCACHEMAX;

	open DCACHEMAT, '>dcache.matrix';
	select DCACHEMAT; $| = 1;
	matrixUCBs(%dUCBs);
	select STDOUT;
	close DCACHEMAT;

	open DCACHEPLOT, '>dcache.p';
	select DCACHEPLOT; $| = 1;
	gnuplotUCBs(min => 'dcache.min.dat',
		    max => 'dcache.max.dat',
		    file => 'dcache.png',
		    title => 'Data Cache');
	select STDOUT;
	close DCACHEPLOT;

	open DCACHEPLOT, '>dcache-eps.p';
	select DCACHEPLOT; $| = 1;
	plotEPS(min => 'dcache.min.dat',
		max => 'dcache.max.dat',
		file => 'dcache.eps',
		title => 'Data Cache');
	select STDOUT;
	close DCACHEPLOT;

	return $dUCBu;
}

# False entrypoint for perl
sub main {
	my %OPTS;
	my $ok = arguments(result => \%OPTS);
	if (!$ok) {
		return -1;
	}

	print "Performing instruction caches: $OPTS{instruction}\n";
	print "Performing data caches: $OPTS{data}\n";

	my ($iucb, $ducb);
	$iucb = $ducb = 0;

	if ($OPTS{instruction}) {
		$iucb = do_instruction();
	}

	if ($OPTS{data}) {
		$ducb = do_data();
	}


	if (!$OPTS{data} || !$OPTS{instruction}) {
		# Poison the task total data
		$iucb = $ducb = -1;
	}
	#
	# Task Totals
	#
	open CACHETASKUCB, '>ucb_bound.txt';
	select CACHETASKUCB; $| = 1;
	dispTaskUCB($iucb, $ducb);
	select STDOUT;
	close CACHETASKUCB;

	return 0;
}

#
# Calculates the UCBs given an ordered list of cache snapshots
#
# Usage: %ucbs = calcUCBs($cache1, $cache2, ... $cacheN);
#
# $ucbs{$ponential_point}{$given_point} = $cache
#
# The ucbs are organized by the current point being considered, that
# is if point 4 is being considered for preemption, there are three
# previous potential premption points 1 through 3. The cost of
# preempting at 4 given the last preemption was at two will be
# $ucbs{4}{2}
#
sub calcUCBs {
	my (@caches, %ucbs);
	@caches = @_;

	use POSIX;
	$| = 1;

	# $p - potential preemption point
	for (my $p = 1; $p < scalar(@caches); $p++) {
		# $l - last preemption point
		my $str = "$p/" . (scalar(@caches) - 1);
		print "\b" x length($str);
		print "$str";
		for (my $l = 0; $l < $p; $l++) {
			my $cache = $caches[$p - 1]; # last 
			for my $idx ($l .. $p - 1) {
				$cache = $cache->intersect($caches[$l]);
			}
			$cache->removeBlanks();
			$ucbs{$p + 1}->{$l + 1} = $cache;
		}
	}
	print "\n";
	return %ucbs;
}

sub UCBMax {
	my (@caches, $total);
	@caches = @_;

	my $union = new Cache();
	foreach my $cache (@caches) {
		$union = $union->union($cache);
	}

	$union->removeBlanks();
	return $union->lineCount();
}

#
# Displays the results of calcUCBS
#
# Usage: dispUCBs(calcUCBs(@caches));
#
sub dispUCBs {
	my (%ucbs);
	%ucbs = @_;

	for my $point (sort{ $a <=> $b } keys(%ucbs)) {
		printf("Potential Point: %02d\n", $point);
		for my $last (sort { $a <=> $b } keys(%{$ucbs{$point}})) {
			my $cache = $ucbs{$point}->{$last};
			printf("\tGiven: %02d UCBs: %02d\n", 
			       $last, $cache->lineCount());
		}
	}
}

#
# Displays the UCBs in matrix form
#
#
#    1    2    3    4
# 1  -    
# 2       -
# 3            -
# 4                 -
sub matrixUCBs {
	my (%ucbs);
	%ucbs = @_;
	
	my @keys = (1, sort {$a <=> $b} keys(%ucbs));

	print "   ";
	foreach my $point (@keys) {
		printf("%02d  ", $point);
	}
	print "\n";

	foreach my $point (@keys) {
		printf("%02d ", $point);
		foreach my $later (@keys) {
			if ($later <= $point) {
				printf("    ");
				next;
			}
			printf("%03d ", $ucbs{$later}->{$point}->lineCount());
		}
		print "\n";
	}
}



#
# Finds the minimum UCB cost when preempting at each point.
#
# Usage: %minUCB = minUCBs(calcUCBs(@caches));
#
# $minUCB{$given_point} = [$count, $later_point];
#
# The minUCBs are organized with the keys as the previous preemption
# point, the $later_point is the program point with the minimum UCB
# $count.
#
sub minUCBs {
	my (%ucbs, %minUCBs);
	%ucbs = @_;

	for my $given_point (1 .. scalar(keys(%ucbs))) {
		my $point = $given_point + 1;
		my $min = $ucbs{$point}->{$given_point}->lineCount();
		
		for my $later_point (sort {$a <=> $b} keys (%ucbs)) {
			if (!exists($ucbs{$later_point}->{$given_point})) {
				next;
			}
			my $count = $ucbs{$later_point}->{$given_point}->lineCount();
			if ($min > $count) {
				$min = $count;
				$point = $later_point;
			}
		}
		$minUCBs{$given_point} = [$min, $point];
	}

	return %minUCBs;
}
	

#
# Displays the minimum UCB points
#
sub dispMINs {
	my (%mins);
	%mins = @_;

	printf("Given\tSmallest\tUCBs\nPoint\tPoint\n");
	for my $givenp (sort {$a <=> $b} keys(%mins)) {
		printf("%02d\t%02d\t\t%03d\n", $givenp, $mins{$givenp}->[1],
		       $mins{$givenp}->[0]);
	}
}

#
# Finds the maximum UCB cost when preempting at each point.
#
# Usage: %maxUCB = maxUCBs(calcUCBs(@caches));
#
# $maxUCB{$given_point} = [$count, $later_point];
#
# The maxUCBs are organized with the keys as the previous preemption
# point, the $later_point is the program point with the maximum UCB
# $count.
#
sub maxUCBs {
	my (%ucbs, %maxUCBs);
	%ucbs = @_;

	for my $given_point (1 .. scalar(keys(%ucbs))) {
		my $point = $given_point + 1;
		my $max = $ucbs{$point}->{$given_point}->lineCount();

		for my $later_point (sort {$a <=> $b} keys (%ucbs)) {
			if (!exists($ucbs{$later_point}->{$given_point})) {
				next;
			}
			my $count = $ucbs{$later_point}->{$given_point}->lineCount();
			if ($max < $count) {
				$max = $count;
				$point = $later_point;
			}
		}
		$maxUCBs{$given_point} = [$max, $point];
	}

	return %maxUCBs;
}

#
# Displays the maximum UCB points
#
sub dispMAXs {
	my (%maxs);
	%maxs = @_;

	printf("Given\tGreatest\tUCBs\nPoint\tPoint\n");
	for my $givenp (sort {$a <=> $b} keys(%maxs)) {
		printf("%02d\t%02d\t\t%03d\n", $givenp, $maxs{$givenp}->[1],
		       $maxs{$givenp}->[0]);
	}
}

#
# Displays the task UCBs
#
sub dispTaskUCBs {
	printf("Task UCB bound: %d\n", $_[0])
}

#
# Displays the tasks total ucbs
#
sub dispTaskUCB {
	printf("Task UCB bound: %d\n", $_[0] + $_[1]);
}

sub gnuplotUCBs {
	my (%args, $file, $title, $min, $max);
	%args = @_;
	($min, $max, $title, $file) =
	    @args{'min', 'max', 'title', 'file' };

	print "set term pngcairo dashed\n";
	gnuplotCore(@_);
	return;
}

sub plotEPS {
	my (%args, $file, $title, $min, $max);
	%args = @_;
	($min, $max, $title, $file) =
	    @args{'min', 'max', 'title', 'file' };

	print "set term postscript eps color\n";
	gnuplotCore(@_);
	return;
}

sub gnuplotCore {
	my (%args, $file, $title, $min, $max);
	%args = @_;
	($min, $max, $title, $file) =
	    @args{'min', 'max', 'title', 'file'};

	my $msg;
	($msg = <<EOT) =~ s/^\s+//gm;
	    set title "$title"
	    set output "$file"
	    set xlabel "Program Point"
	    set ylabel "UCBs Shared with Subsequent Program Point"
	    set xtics 0,1
	    set xrange []
	    set yrange []
	    # Minimum Line Style
	    set linestyle 1 lt 1 lc rgb "black"

	    # Maximum Line Style
	    set linestyle 2 lt 2 lc rgb "blue"

	    plot \\
	        '$min' using 1:3:2 with labels offset 0,-1 tc ls 1 notitle , \\
		'$min' using 1:3 ls 1 pt 7 notitle, \\
		'$min' using 1:3 title "Minimum" with lines ls 1 , \\
		\\
		'$max' using 1:3:2 with labels offset 0,1 tc ls 2 notitle, \\
		'$max' using 1:3 ls 2 pt 7 notitle, \\
		'$max' using 1:3 title "Maximum" with lines ls 2 , \\
EOT

	print $msg;
}	

exit main();
