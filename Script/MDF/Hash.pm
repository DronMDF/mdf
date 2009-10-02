#
# Copyright (c) 2000-2005 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#

package	MDF::Hash;

@EXPORT = qw/ File /;

use Digest::SHA1;
use Cwd qw / realpath /;

sub File {
	my (undef, $base, $file) = @_;
	my $fullname = ($base ? ($base . '/') : '') . $file;

	if ( -l $fullname ) {
		my $realpath = realpath ($fullname);

		unless ( $realpath =~ s/^$base\///i ) {
			print "MDF::Hash::File: Bad symlink: '$_' -> '$realpath'\n";
			return;
		}

		return $file .
			":link=" . $realpath;
	}

	if ( -f $fullname ) {
		open FILE, "< $fullname";
		my $hash = Digest::SHA1->new;
		$hash->addfile (*FILE);

		close FILE;
		return $file .
			":size=" . (stat("$fullname"))[7] .
			":sha1=" . $hash->b64digest;
	}

	return;
}
