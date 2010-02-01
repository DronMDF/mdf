#
# Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

use strict;
use warnings;

package	MDF::Test;

our @EXPORT = qw/&assert &assert_equal/;

sub assert {
	my ($expr) = @_;
	die "assert failed" if (!$expr);
}

sub assert_equal {
	my ($a, $b) = @_;
	die "assert failed" if ($a ne $b);
}

1;
