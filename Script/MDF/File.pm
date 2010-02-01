#
# Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

use strict;
use warnings;

package	MDF::File;

use lib $ENV{'MDF_REPOS'} . '/Script';

use File::Find;
use Cwd qw/ realpath /;

our @EXPORT = qw/ Find /;

my @filelist;
my $basedir;
my $pattern;

sub FindProc {
	if ($File::Find::name =~ /$pattern$/ )	{
		push @filelist, $File::Find::name;
	}
}

sub Find {
	my $fullnames;
	(undef, $basedir, $pattern, $fullnames) = @_;

	return () unless ( -e $basedir );

	@filelist = ();
	$basedir =  realpath ($basedir);

	find ({ wanted=>\&FindProc, follow=>1}, ($basedir));

	unless ($fullnames) {
		for (my $i = 0; $i <= $#filelist; $i++) {
			$filelist[$i] =~ s#$basedir/##;
		}
	}

	return @filelist;
}

# UNITCHECK {
# 	use MDF::Test;
# 
# 	#$td->ok(0);
# 	# Формируем тестовое окружение
# 	# my $root = "$ENV{'MDF_TEMP'}/file";
# 	# system('mkdir', '-p', "$root/a/b/c/d");
# 	# system('touch', "$root/a/b/c/file1.test");
# 	# system('touch', "$root/a/b/c/d/.test");
# 	# 
# 	# my @flist = Find($root, "file1.test");
# 	# ok(scalar @flist == 1);
# 	# is($flist[0], "a/b/c/file1.test");
# 	# 
# 	# system('rm', '-rf', $root);
# 	return 0;
# }

1;
