#
# Copyright (c) 2000-2005 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#

package MDF::MPackage;

use lib $ENV{'MDF_REPOS'} . '/Script';

@EXPORT = qw/ GetFileList CheckInstalled SetProject GetProject FindProject /;

use MDF::Hash;
use MDF::File;

use Cwd qw / realpath /;
use File::Basename;

sub GetFileList {
	my (undef, $path) = @_;

	$path =~ realpath ($path);

	my @files = MDF::File->Find ($path, '', 0);
	my @filelist;

	foreach my $file (@files) {
		if ( -f "$path/$file" or -l "$path/$file") {
			# В список заносим только регулярные файлы и симлинки

			# TODO: В случае возникновения проблем вернет undef.
			push @filelist, MDF::Hash->File ($path, $file);
		}
	};

	return @filelist;
}

sub CheckInstalled {
	my (undef, $nm, $ver) = @_;

	return 0 unless (defined $nm and defined $ver);

	# Пока все просто, если файл есть - то пакет стоит
	return 1 if ( -e "$ENV{'MDF_ROOT'}/Config/System/Packages/$nm-$ver.mpackage");

	return 0;
}

sub SetProject {
	my (undef, $nm, $ver, @filelist) = @_;

	# Проверим есть ли директория
	my $pfile = "$ENV{'MDF_ROOT'}/Config/System/Packages/$nm-$ver.mpackage";
	system ('mkdir', '-p', dirname $pfile)
		and die $!;

	open PFILE, "> $pfile"
		or die $!;

	foreach (@filelist) {
		print PFILE $_, "\n";
	}

	close PFILE;
}

sub GetProject {
	my (undef, $nm, $ver, $remove) = @_;

	return () unless (defined $nm);

	my @pfile = MDF::File->Find ("$ENV{'MDF_ROOT'}/Config/System/Packages/",
		".mpackage", 1);

	for (my $i = 0; $i <= $#pfile; $i++) {
		if (defined $ver) {
			next unless ($pfile[$i] =~ /$nm-$ver\.mpackage$/);
 		} else {
			next unless ($pfile[$i] =~ /$nm-[\.\d]*.mpackage$/);
		}

		open PFILE, "< $pfile[$i]" or return ();
		my @filelist = <PFILE>;
		close PFILE;

		unlink $pfile[$i] if ($remove);
		return @filelist;
	}

	return ();
}

sub FindProject {
	my (undef, $nm) = @_;

	return () unless (defined $nm);

	my @pfile = MDF::File->Find ("$ENV{'MDF_ROOT'}/Config/System/Packages/",
		".mpackage", 0);
	my @mfile;

	foreach (@pfile) {
		s/.mpackage$//;
		if (m#/$nm-[\.\d]*$# or m#^$nm-[\.\d]*$#) {
			push @mfile, $_;
		}
	}

	if ($#mfile > 1) {
		# Это странно...
		print "MDF::MPackage: ??? inparticularly, select one of:\n";
		foreach (@mfile) {
			m#^(.*)-([\d\.]*)$#;
			my ($_nm, $_ver) = ($1, $2);
			while ($_nm =~ s#/(.*)/#-$1/#) {};	# Путь в имя!
			print "\t$_nm-$_ver\n";
		}
		return ();
	}

	foreach (@mfile) {
		m#^(.*)-([\d\.]*)$#;
		my ($_nm, $_ver) = ($1, $2);
		while ($_nm =~ s#/(.*)/#-$1/#) {};	# Путь в имя!
		return ($_nm, $_ver);
	}

	return ();
}

1;