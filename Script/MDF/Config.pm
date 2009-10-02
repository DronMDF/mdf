#
# Copyright (c) 2000-2005 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#

package	MDF::Config;

@EXPORT = qw/ Refresh /;

sub Refresh {
	my $system = `uname`;
	chomp $system;
	
	if ( $system eq 'FreeBSD' ) {
		$ENV{'MAKE'} = 'gmake';
	} else {
		$ENV{'MAKE'} = 'make';
	}
	
	# Чтение из конфигурации
	if ( -f "$ENV{'MDF_ROOT'}/Config/MDF.Config" ) {
		open CFG, "$ENV{'MDF_ROOT'}/Config/MDF.Config"
			or die "Cannot read config file!";
	
		my $config = join '', <CFG>;
		close CFG;

		# фикс на досовый перевод строки
		$config =~ s/\r//g;
	
		# Вырежем комментарии
		$config =~ s/#.*\n//g;
	
		# Это все мягкие переносы строк съедим
		$config =~ s/\\\n//g;
	
		# Теперь все переменные заэкспортируем!
		while ($config =~ s/(\w+)\s*=\s*"(.*?)"//) {
			my ($var, $val) = ($1, $2);
			$val =~ s/\$\((\w+?)\)/$ENV{$1}/g;
		
			$ENV{$var} = $val;
		}
	}
	
	$ENV{'MAKEFLAGS'} = "" unless defined $ENV{'MAKEFLAGS'};
}

sub CheckVar {
	my ($name, $fatal) = @_;

	unless ( defined $ENV{$name} ) {
		die "MDF::Config: Environment var $name is not set!\n" if ( $fatal );
		return 0;
	}

	system ('mkdir', '-p', $ENV{$name}) unless ( -d $ENV{$name} );
	return 1;
}

CheckVar ('MDF_ROOT', 1);
CheckVar ('MDF_REPOS', 1);
CheckVar ('MDF_FILES', 1);
CheckVar ('MDF_TEMP', 1);

if (CheckVar ('MDF_TREE', 0) == 0) {
	$ENV{'MDF_TREE'} = $ENV{'MDF_REPOS'} . '/Tree';
}

&Refresh;

1;
