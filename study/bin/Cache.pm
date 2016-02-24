# Local Variables:
# mode: cperl
# cperl-indent-level: 8
# perl-indent-level: 8
# cperl-indent-parens-as-block: t
# End:
package Cache;
use strict;
use warnings;
use CacheLine;
use Moose;

has 'name' => (
	is => 'rw',
	isa => 'Str',
	default => '',
	handles => {
		nameSet => 'set',
		nameGet => 'get',
	}
);

has 'line' => (
	is => 'rw',
	isa => 'ArrayRef[CacheLine]',
	traits => ['Array'],
	default => sub { [] },
	handles => {
		lines => 'elements',
		lineCount => 'count',
		linePush => 'push',
		linePop => 'pop',
		lineDelete => 'delete',
		lineSet => 'set',
		lineGet => 'get',
		clear => 'clear',
	}
);

# $cache->import(filename)
sub importFile {
	my ($self, $filename, $fh);
	($self, $filename) = @_;
	$self->clear();

	open($fh, $filename);

	while (my $line = <$fh>) {
		chomp($line);
		my @fields = split(/\s+/, $line);
		my $line = new CacheLine();
		$line->addressPush(map { '0x' . $_ } @fields[4,5,6,7]);
		$self->linePush($line);
	}
	close($fh);
}

#
# Removes cache lines with no value
#
sub removeBlanks {
	my ($self);
	$self = shift;

	for (my $i = 0; $i < $self->lineCount; $i++) {
		my $line = $self->lineGet($i);
		if (!$line->nonZero()) {
			$self->lineDelete($i);
			$i--;
		}
	}
}

#
# Counts the number of empty cache lines
#
sub emptyCount {
	my ($self, $rv);
	$rv = 0;
	$self = shift;

	for (my $i = 0; $i < $self->lineCount; $i++) {
		my $line = $self->lineGet($i);
		if (!$line->nonZero()) {
			$rv++;
		}
	}
	return $rv;
}

sub toString {
	my ($self, $str);
	$self = shift;

	my $cache_line = 0;
	foreach my $line ($self->lines()) {
		$str .= sprintf ("Line [ %03i ] ", $cache_line++);
		foreach my $addr ($line->addresses()) {
			$addr =~ s/0x//;
			$str .= sprintf("%-12s", $addr);
		}
		$str .= "\n";
	}
	return $str;
}

sub copy {
	my $self = shift;

	my $r = new Cache();
	foreach my $line ($self->lines()) {
		$r->linePush($line->copy());
	}

	return $r;
}

#
# Calculates the intersection of two caches
#
# Usage:
#   $newCache = $cache->intersect($anotherCache);
sub intersect {
	my ($self, $other, $cache, $zstr);
	($self, $other) = @_;
	$cache = new Cache();

	# Do the zero string once.
	$zstr = $self->lineGet(0)->zeroStr();
	# Make the array once too
	my @zs;
	for ($self->lineGet(0)->addresses()) {
		push @zs, $zstr;
	}

	my $count = $self->lineCount();
	for (my $i = 0; $i < $count; $i++) {
		my ($ref, $cand, $line);
		$ref = $self->lineGet($i);
		$cand = $other->lineGet($i);

		$line = new CacheLine();

		if ($ref->equals($cand)) {
			$line->addressPush($ref->addresses());
		} else {
			$line->addressPush(@zs);
		}
		$cache->linePush($line);
	}

	return $cache;
}

#
# Calculates the conflicts of two caches
#
# The resulting conflict cache takes the values from this cache
#
# Usage:
#   $newCache = $cache->conflicts($anotherCache)
#
sub conflicts {
	my ($self, $other, $cache);
	($self, $other) = @_;
	$cache = new Cache();

	for (my $i = 0; $i < $self->lineCount(); $i++) {
		my ($ref, $cand, $line);
		$ref = $self->lineGet($i);
		$cand = $other->lineGet($i);

		if ($ref->nonZero() && $cand->nonZero()) {
			$line = $ref->copy();
		} else {
			$line = new CacheLine();
		}
		$cache->linePush($line);
	}
	return $cache;
}

#
# Calculates the union of two caches
#
# Usage:
#    $newCache = $cache->union($anotherCache);
#
sub union {
	my ($self, $other, $cache);
	($self, $other) = @_;
	$cache = new Cache();

	# For each cache line
	for (my $i = 0; $i < $other->lineCount(); $i++) {
		my ($s, $o, $line);
		$s = $self->lineGet($i);
		$o = $other->lineGet($i);

		$line = new CacheLine();
		for (my $j=0; $j < $o->addressCount(); $j++) {
			my ($addr, $zero);
			$zero = $o->zeroStr();
			$addr = $o->addressGet($j);

			if ($addr eq $zero) {
				if ($s && $s->addressCount() > $j) {
					$addr = $s->addressGet($j);
				}
			}
			# $addr may still be zero
			$line->addressPush($addr);
		}
		$cache->linePush($line);

	}
	return $cache;
}

no Moose;


1
