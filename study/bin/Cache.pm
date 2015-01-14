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
	}
);

# $cache->import(filename)
sub importFile {
	my ($self, $filename, $fh);
	($self, $filename) = @_;

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
# Calculates the intersection of two caches
#
# Usage:
#   $newCache = $cache->intersect($anotherCache);
sub intersect {
	my ($self, $other, $cache);
	($self, $other) = @_;
	$cache = new Cache();

	for (my $i = 0; $i < $self->lineCount(); $i++) {
		my ($ref, $cand, $line);
		$ref = $self->lineGet($i);
		$cand = $other->lineGet($i);

		$line = new CacheLine();

		my $use = 0;
		if ($ref->equals($cand)) {
			$use = 1;
		}
		foreach my $addr ($ref->addresses()) {
			$addr = $ref->zeroStr() if !$use;
			$line->addressPush($addr);
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
