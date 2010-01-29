#
# Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

package	MDF::MSetup;

use Test::Builder;

@EXPORT = qw(ok is);

my $builder = Test::Builder->new;
$builder->output("/dev/null");

END {
	$builder->done_testing();
}

sub ok($;$) {
	$builder->ok(@_) or exit(-1);
}

sub is($$;$) {
	$builder->is_eq(@_) or exit(-1);
}

1;
