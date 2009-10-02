#
# Copyright (c) 2000-2005 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#

package	MDF::File;

use lib $ENV{'MDF_REPOS'} . '/Script';

use File::Find;
use Cwd qw/ realpath /;

@EXPORT = qw/ Find /;

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

1;
