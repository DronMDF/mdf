#!/usr/bin/perl -w
#
# Copyright (c) 2000-2006 Андрей Валяев (dron@infosec.ru)
# All rights reserved.
#
# Created: 15/09/06 10:56:01
#

use warnings;
use strict;

use XML::SAX::ParserFactory;
use Getopt::Long;

use Term::ANSIColor;

package MDF::UnitGen;
use base qw(XML::SAX::Base);

# Опции командной строки
my $targetdir = undef;
my $makepub = undef;
my $protopub = undef;
my $protopub_local = undef;
my $cflags_local = undef;
my $makefile = 0;
my $list = 0;

Getopt::Long::GetOptions (
	'targetdir=s'	=> \$targetdir,
	'makepub=s'	=> \$makepub,
	'makefile' 	=> \$makefile,
	'protopub=s'	=> \$protopub,
	'help'		=> \&help,
	'list'	=> \$list,
);

my ($unit, $unittype, $unitlang, $prototype);
my ($copyright, $license);
my ($inbody, $inprebody, $test_postfix, $predef) = (undef, 0, undef, undef);
my (%body, %prebody, $prebody_global);
my (%depend, %link);
my ($link_test, $depend_test) = (undef, undef);

my $handler = new MDF::UnitGen;

my $filename;

foreach my $file (@ARGV) {
	my $parser = XML::SAX::ParserFactory->parser ( Handler => $handler );
	$filename = $file;
	$parser->parse_file ($file);
}

exit;

sub cdie
{
	my ($msg) = @_;
	print STDERR Term::ANSIColor::color('bold red'), "$filename: ",
		Term::ANSIColor::color('dark red'), "$msg\n",
		Term::ANSIColor::color('reset');

	die;
}

### Вспомогательные функци
sub help
{
	print "Options: \n";
	print "\t--targetdir <path>\ttarget directory;\n";
	print "\t--makefile <file>\tGenerate makefile;\n";
	print "\t--makepub <file>\tPublish makefile;\n";
	print "\t--protopub <file>\tPublish unit prototype;\n";
	print "\t--list\tUnit list;\n";
	exit;
}

sub GenerateUnit
{
	my ($name) = @_;

	my $path = "$targetdir/$unit" . ($name eq 'mdf:unit' ? "": "_test_" . $name). ".c";

	cdie ("$path already exists!") if (-e $path);

	open FILE, "> $path"
		or cdie ("create '$path' failed");

	print FILE "// This file automatically generated\n";
	print FILE "// Copyright (c) by dron (dron\@infosec.ru)\n\n";

	print FILE $prebody_global, "\n" if defined $prebody_global;
	print FILE $prebody{$name}, "\n\n" if defined $prebody{$name};

	if ($name eq 'mdf:unit') {
		# Тело функци.
		print FILE $prototype, "\n";
		print FILE "{\n";
		print FILE $body{$name}, "\n";
		print FILE "}\n";
	} else {
		# Специальная функция для вывода строки на stderr для linux
		print FILE "static inline void testerror (char *str)\n";
		print FILE "{\n";
		print FILE "\tfor (int i = 0; str[i] != 0; i++)\n";
		print FILE "\t{\n";
		print FILE "\t\t__asm__ __volatile__ ( \"int \$0x80\"\n";
		print FILE "\t\t\t: : \"a\" (4), \"b\" (2), \"c\" (str + i), \"d\" (1));\n";
		print FILE "\t}\n";
		print FILE "}\n\n";

		# модуль тестирования
		print FILE "int test_$name (void)\n";
		print FILE "{\n";
		print FILE $body{$name}, "\n";
		print FILE "\treturn 0;\n";
		print FILE "}\n\n";
		print FILE "int main (int argc, char **argv)\n";
		print FILE "{\n";
		print FILE "\treturn test_$name();\n";
		print FILE "}\n";
	}

	close FILE;
}

sub GenerateMakefile
{
	my ($name) = @_;
	my $path = "$targetdir/$name";

	cdie ("$path Already exists!") if (-e $path);

	$cflags_local = '' unless defined $cflags_local;

	open FILE, "> $path"
		or cdie ("create '$path' failed");

	print FILE "# This file automatically generated\n";
	print FILE "# Copyright (c) by dron (dron\@infosec.ru)\n\n";

	# Правило для тестовой сборки юнита.
	print FILE $unit, "_test.o : $unit.c";
		print FILE " $depend_test" if (defined $depend_test);
		print FILE "\n";
	print FILE "\t", '$(TEST_CC) -std=c99 -nostdinc ' . $cflags_local .
			' $(TEST_CFLAGS) -c -o $@ $<', "\n\n";

	# Правила для тестовых модулей
	foreach my $tn (keys %body) {
		next if ($tn eq 'mdf:unit');

		print FILE $unit, "_test_$tn :";
			print FILE " $unit", "_test_$tn.c $unit", "_test.o";
			print FILE " $depend{$tn}" if (defined $depend{$tn});
			print FILE "\n";

		# Линкуем с определенными юнитами.
		print FILE "\t", '$(TEST_CC) -std=c99 -nostdinc ' . $cflags_local .
				' $(TEST_CFLAGS) -o $@ $<';
			# линкуем с отладочным юнитом.
			print FILE " $unit", '_test.o';
			print FILE " $link{$tn}" if (defined $link{$tn});
			print FILE " $link_test" if (defined $link_test);
			print FILE "\n";

		print FILE "\t", './$@', "\n\n";
	}

	# Правило для релизного юнита
	print FILE $unit, '.o : ', $unit, '.c ';
		# Список тестов
		foreach my $tn (keys %body) {
			print FILE $unit . "_test_" . $tn . ' '
				if ($tn ne 'mdf:unit');
		}
		print FILE "\n";
	print FILE "\t", '$(ECHO) Compile $@...', "\n";
	print FILE "\t", '$(CC) -std=c99 ' . $cflags_local .
			' $(CFLAGS) -c -o $@ $<', "\n\n";

	close FILE;

	# от юнита к юниту могут меняться.
	$cflags_local = undef;
}

sub PublishMakefile
{
	my ($name) = @_;
	my $path = $makepub;

	unless (-e $path) {
		open MF, "> $path" or
			cdie ("create '$path' failed");

		print MF "# This file automatically generated\n";
		print MF "# Copyright (c) by dron (dron\@infosec.ru)\n\n";
		close MF;
	}

	# Заменяем абсолютные пути на относительные
	my $dir = $targetdir;
	unless ($dir =~ s/$ENV{'MDF_TEMP'}/\$(MDF_TEMP)/) {
		$dir =~ s/$ENV{'HOME'}/\$(HOME)/;
	}

	open MF, ">> $path" or
		cdie ("append '$path' failed");

	print MF "include $dir/$name\n";
	close MF;
}

sub PublishPrototype
{
	my ($proto) = @_;

	open (IFN, "+< $proto")
		or cdie ("Problem with publist prototype in '$proto'");

	my $incbody = join '', <IFN>;

	# Вставляем прототип
	if ($incbody =~ /extern\s"C"\s\{\n#endif\n(.*)#ifdef\s__cplusplus\n\}/s ) {
		$incbody = "$`extern \"C\" \{\n#endif\n$1" .
			"$prototype;\n#ifdef __cplusplus\n\}$'";
	} else {
		cdie ("No prototype section in '$proto'");
	}

	seek (IFN, 0, 0);
	print IFN  $incbody;
	close IFN;
}

### Реализация SAX хандлера.
sub start_element
{
	my ($self, $params) = @_;
	my $name = $params->{Name};

	if ($list) {
		# Режим списка юнитов - только показываем имя.
		if ($name eq 'mdf:unit') {
			print $params->{Attributes}->{'{}name'}->{Value}, "\n";
		}
		return;
	}

	return if ($name eq 'mdf:unitset');

	# Копирайт может быть один на несклько юнитов.
	if ($name eq 'mdf:copyright') {
		$copyright = $params->{Attributes}->{'{}author'}->{Value} . " <" .
			$params->{Attributes}->{'{}email'}->{Value} . ">";
		$license = $params->{Attributes}->{'{}license'}->{Value};
		return;
	}

	if ((not defined $unit) and ($name eq 'mdf:unit'))
	{
		$unit = $params->{Attributes}->{'{}name'}->{Value};
		$unittype = $params->{Attributes}->{'{}type'}->{Value};
		$unitlang = $params->{Attributes}->{'{}lang'}->{Value};

		# Чистка
		%body = ();
		%prebody = ();
		%depend = ();
		%link = ();

		$prebody_global = undef;
		$predef = undef;
		$inbody = undef;
		$link_test = undef;
		$depend_test = undef;
		return;
	}

	cdie ("Illegal tag <$name>") unless (defined $unit);

	if ($name eq 'mdf:include') {
		my $ifile = $params->{Attributes}->{'{}file'}->{Value};
		my $iprefix = $params->{Attributes}->{'{}prefix'}->{Value};
		my $iproto = $params->{Attributes}->{'{}protopub'}->{Value};

		my $iline= '#include <' . $ifile .">\n";

		if (defined $inbody) {
			cdie ("protopub in body") if (defined $iproto and $iproto eq 'yes');

			# Инклюд персонально для теста
			$prebody{$inbody} .= $iline;

			# TODO: Пути перенести в командную строку.
		} else {
			# инклюд для всех
			$prebody_global .= $iline;

			if (defined $iproto and $iproto eq 'yes') {
				cdie ("protopub redefined")
					if (defined $protopub_local);

				$protopub_local = $iprefix . '/' . $ifile;
				$protopub_local =~ s/\$\(([^\)\s]+)\)/$ENV{$1}/g;
			}
		}

		if (defined $iprefix) {
			$cflags_local = '' unless defined $cflags_local;
			$cflags_local .= " -I$iprefix";
		}
		return;
	}

	# TODO: Возможно прототипы стоит хранить независимо от языка?
	if ($name eq 'mdf:prototype') {
		$prototype =
			$params->{Attributes}->{'{}type'}->{Value} . " $unit (";
		return;
	}

	if ($name eq 'mdf:arg') {
		$prototype =~ s/([^(]+)$/$1, /;

		my $type = $params->{Attributes}->{'{}type'}->{Value};
		my $name = $params->{Attributes}->{'{}name'}->{Value};

		$prototype .= $type . " " unless ($name eq '...');
		$prototype .= $name;
		return;
	}

 	if ($name eq 'mdf:body')
	{
		cdie ("recursive body/test") if (defined $inbody);

		$inbody = 'mdf:unit';
		$body{$inbody} = '';
		return;
	}

 	if ($name eq 'mdf:test')
	{
		cdie ("recursive body/test") if (defined $inbody);

		$inbody = $params->{Attributes}->{'{}name'}->{Value};
		$inbody = 'main' unless (defined $inbody);
		cdie ("Illegal test name 'mdf:unit'") if ($inbody eq 'mdf:unit');

		$body{$inbody} = '';
		return;
	}

	cdie ("Illegal tag <$name>") unless (defined $inbody);

	if ($name eq 'mdf:testexp')
	{
		cdie ("mdf:testexp in mdf:body") if ($inbody eq 'mdf:unit');

		my $op = $params->{Attributes}->{'{}op'}->{Value};
		my $value = $params->{Attributes}->{'{}value'}->{Value};
		my $id = $params->{Attributes}->{'{}id'}->{Value};

		$id = "undef" unless (defined $id);

		my $error = "testerror (\"expression '$id' in test '$inbody' in unit '$unit' has failed!\\n\"); return -1";

		if ($op eq 'eq') {
			$body{$inbody} .= "\n\tif ($value != (";
			$test_postfix = ")) { $error; }\n";
			return;
		}

		if ($op eq 'ne') {
			$body{$inbody} .= "\n\tif ($value == (";
			$test_postfix = ")) { $error; }\n";
			return;
		}

		if ($op eq 'le') {
			$body{$inbody} .= "\n\tif ($value <= (";
			$test_postfix = ")) { $error; }\n";
			return;
		}

		if ($op eq 'gt') {
			$body{$inbody} .= "\n\tif ($value >= (";
			$test_postfix = ")) { $error; }\n";
			return;
		}

		if ($op eq 'streq') {
			$body{$inbody} .= "\n\t{\tconst char *value = \"$value\";" .
					"\n\t\tconst char *exp = (char *)(";
			$test_postfix = ");\n\t\tfor (int p = 0; ; p++) {" .
					"\n\t\tif (exp[p] != value[p]) { $error; }" .
					"\n\t\tif (value[p] == 0) break; } }\n";
			return;
		}

		if ($op eq 'memeq') {
			my $valuesize;

			if ($value =~ /^\s*0?x[\da-fA-F]{2}(\s*,\s*0?x[\da-fA-F]{2})*$/ ) {
				# Шестнадцатеричная последовательность
				$value = "{ $value }";
				$valuesize = "sizeof(value)";
			} else {
				# В протичном случае интерпретируем как строку
				$value = "\"$value\"";
				$valuesize = "sizeof(value) - 1";	# Не учитываем терминатор
			}

			$body{$inbody} .= "\n\t{\tconst char value[] = $value;" .
					"\n\t\tconst char *exp = (char *)(";
			$test_postfix = ");\n\t\tfor (int p = 0; p < ($valuesize); p++) {" .
					"\n\t\tif (exp[p] != value[p]) { $error; } } }\n";
			return;
		}

		cdie ("Illegal op $op in <$name>");
	}

	if ($name eq 'mdf:depend') {
		my $depunit = $params->{Attributes}->{'{}unit'}->{Value};
		$depunit .= "_test";

		my $deplink = $params->{Attributes}->{'{}link'}->{Value};
		$deplink = 'yes' unless (defined $deplink);

		if ($inbody eq 'mdf:unit') {
			$depend_test = "" unless (defined $depend_test);
			$depend_test .= " $depunit.o";
		} else {
			$depend{$inbody} = '' unless (defined $depend{$inbody});
			$depend{$inbody} .= " $depunit.o"
		}

		if ($deplink eq 'yes') {
			if ($inbody eq 'mdf:unit') {
				$link_test = "" unless (defined $link_test);
				$link_test .= " $depunit.o"
			} else {
				$link{$inbody} = '' unless (defined $link{$inbody});
				$link{$inbody} .= " $depunit.o";
			}
		}

		return;
	}

	if ($name eq 'mdf:predef') {
		$predef = 1;
		return;
	}

	cdie ("Illegal tag <$name>");
}

sub characters
{
	my ($self, $params) = @_;

	if (defined $inbody) {
		if ($predef) {
			$prebody{$inbody} .= $params->{Data};
		} else {
			$body{$inbody} .= $params->{Data};
		}
		return;
	}
}

sub end_element
{
	my ($self, $params) = @_;
	my $name = $params->{Name};

	if (defined $unit and $name eq 'mdf:unit') {
 		# сгенерировать файло

		foreach my $uname (keys %body) {
			&GenerateUnit ($uname);

			if ($uname eq 'mdf:unit') {
				# Публикация прототипа.
				if (defined $protopub_local) {
					&PublishPrototype($protopub_local);
					$protopub_local = undef;
					if (defined $protopub and
						$protopub ne $protopub_local)
					{
						&PublishPrototype($protopub)
					}
				} else {
					if (defined $protopub) {
						&PublishPrototype($protopub)
					}
				}


			}
		}

		if ($makefile) {
			my $mfn = "Makefile.$unit";
			# Создание мейкфайла.
			&GenerateMakefile ($mfn);

			if (defined $makepub) {
				# Публикация мейкфайла.
				&PublishMakefile ($mfn);
			}
		}

		# Подчистим следы старого юнита.
		$unit = undef;
		return;
	}

	if ($name eq 'mdf:prototype') {
		$prototype .= ")";
		return;
	}

	return unless (defined $inbody);

 	if (($name eq 'mdf:body') or ($name eq 'mdf:test')) {
		$inbody = undef;
		return;
	}

	if ($name eq 'mdf:testexp') {
		$body{$inbody} .= $test_postfix;
		$test_postfix = undef;
		return;
	}

	return unless (defined $predef);

	if ($name eq 'mdf:predef') {
		$predef = undef;
		return;
	}
}
