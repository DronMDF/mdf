#
# Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

use strict;
use warnings;

package MDF::MSetup;

use lib $ENV{'MDF_REPOS'} . '/Script';

use MDF::Config;
use MDF::Version;
use MDF::File;

use File::Basename qw/basename/;

our @EXPORT = qw/FindMSetup FindRule/;

#-------------------------------------------------------------------------------
# Поиск скриптов и формирование имен пакетов.

# Возвращает хеш. - ключем хеша является которкое имя - которое будет 
# преобразовано в имя пакета, значением является полный путь к файлу.
sub LocateScripts($$) {
	my ($bases, $name) = @_;
	my %scripts;
	foreach my $base (@$bases) {
		my @files = (
			MDF::File->Find($base, basename $name . ".msetup", 0),
			#MDF::File->Find($base, basename $name . "\/.msetup", 0)
		);
		foreach my $file (@files) {
			# TODO: Сразу отбросить ненужные
			die "Duplicate tree entryes $file" if exists $scripts{$file};
			$scripts{$file} = "$base/$file" 
		}
	}
	return %scripts;
}

sub FindMSetup {
	my ($self, $name) = @_;

	return () unless (defined $name);

	my %scripts = LocateScripts([$ENV{'MDF_TREE'}], $name);
	my @msetup = keys %scripts;
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

#-------------------------------------------------------------------------------
# UNITCHECK {
# 	use MDF::Test;
# 
# 	# Формируем тестовое окружение
# 	my $root = "$ENV{'MDF_TEMP'}/msetup";
# 	system('mkdir', '-p', "$root/group/subgroup/project");
# 	system('touch', "$root/group/subgroup/test.msetup");
# 	system('touch', "$root/group/subgroup/project/.msetup");
# 
# 	# Скрипт по старому расположению
# 	my %scripts = LocateScripts([$root], 'test');
# 	my @script_name = keys %scripts;
# 	&assert_equal($script_name[0], 'group/subgroup/test.msetup');
# 	&assert_equal($scripts{'group/subgroup/test.msetup'}, "$root/group/subgroup/test.msetup");
# 
# 	# Скрипт по новому расположению (В работе пока не используется)
# 	# %scripts = LocateScripts(["$ENV{'MDF_TEMP'}/msetup"], 'project');
# 	# @script_name = keys %scripts;
# 	# is($script_name[0], 'group/subgroup/project/.msetup');
# 	# is($scripts{'group/subgroup/project/.msetup'}, "$ENV{'MDF_TEMP'}/msetup/group/subgroup/project/.msetup");
# 
# 	# чистка.
# 	system('rm', '-rf', $root);
# 	return 0;
# }

1;
