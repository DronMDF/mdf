#
# Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

package	MDF::MSetup;

use lib $ENV{'MDF_REPOS'} . '/Script';

use MDF::Config;
use MDF::Version;
use MDF::File;

use File::Basename qw/basename/;

@EXPORT = qw/FindMSetup FindRule/;

#-------------------------------------------------------------------------------
# Поиск скриптов и формирование имен пакетов.

sub LocateScripts {
	my (undef, $base, $name) = @_;
	return MDF::File->Find($base, basename $name . ".msetup", 0);
}

sub FindMSetup {
	my (undef, $name) = @_;

	return () unless (defined $name);

	my @msetup = MDF::File->Find ($ENV{'MDF_TREE'}, basename $name . ".msetup", 0);
	my ($nm, $msetup);

 	foreach my $ms_org (@msetup) {
		my $ms = $ms_org;
		$ms =~ s#.msetup$##;
		while ($ms =~ s#/(.*)/#-$1/#) {}	# Путь преобразуем в группу
		if ($ms eq $name or $ms =~ /\/$name$/ ) {
			# Если пакет уже нашелся - любое повторное совпадение - неверно!
			die "MSetup.pm: inconcrete name\n\t$nm\n\t$ms\n"
				if (defined $msetup);

			($nm, $msetup) = ($ms, $ms_org);
		}
	}

	return ($nm, $msetup);
}

#-------------------------------------------------------------------------------
# Разбор содержимого скрипта

sub FindRule {
	my (undef, $name) = @_;
	my ($nm, $msetup) = FindMSetup (undef, $name);
	my ($version, $rules);

	return () unless (defined $msetup);

	open MS, "< $ENV{'MDF_TREE'}/$msetup" or
		return ();

	my $ms_content = join '', (<MS>);
	while ($ms_content =~ s/Release\s+([\d\.]*)\s+{\s+(.*?)\s+}//is) {
		my ($ver_l, $rul_l) = ($1, $2);

		($version, $rules) = ($ver_l, $rul_l)
			if (MDF::Version->Valid ($ver_l, $version));
	}
	close MS;

	$rules = '' unless defined $rules;
	return ($nm, $version, $rules);
}

# ------------------------------------------------------------------------------
# Тестирование

use MDF::Test;

# Формируем тестовое окружение
system('mkdir', '-p', "$ENV{'MDF_TEMP'}/msetup/group/subgroup/project");
system('touch', "$ENV{'MDF_TEMP'}/msetup/group/subgroup/test.msetup");

my @scripts = LocateScripts(undef, "$ENV{'MDF_TEMP'}/msetup", "test");
is($scripts[0], "group/subgroup/test.msetup");

system('rm', '-rf', "$ENV{'MDF_TEMP'}/msetup");

1;
