# Local Variables:
# mode: cperl
# cperl-indent-level: 8
# perl-indent-level: 8
# cperl-indent-parens-as-block: t
# End:
package CacheLine;
use strict;
use warnings;

use Moose;

has 'address' => (
	is => 'rw',
	isa => 'ArrayRef[Str]',
	traits => ['Array'],
	default => sub { [ ] },
	handles => {
		addressCount => 'count',
		addresses => 'elements',
		addressSet => 'set',
		addressGet => 'get',
		addressPush => 'push',
		addressPop => 'pop',
	}
);

#
# Returns true if any of the addresses in the cache line are nonzero
#
sub nonZero {
	my ($self);
	$self = shift;

	foreach my $address ($self->addresses()) {
		return 1 if ($address ne "0x00000000");
	}
	return 0;
}

sub zeroStr {
	return "0x00000000";
}

#
#
#
sub equals {
	my ($self, $other);
	($self, $other) = @_;

	if ($self->addressCount() != $other->addressCount()) {
		return 0;
	}

	for (my $i = 0; $i < $self->addressCount(); $i++) {
		my ($l, $r);
		$l = $self->addressGet($i);
		$r = $other->addressGet($i);
		if ($l ne $r) {
			return 0;
		}
	}
	return 1;
}

sub copy {
	my $self = shift;

	my $r = new CacheLine();
	$r->addressPush($self->addresses());

	return $r;
}
no Moose;
1
