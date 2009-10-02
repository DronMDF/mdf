#!/usr/bin/perl -w
#
# Copyright (c) 2000-2006 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#

use strict;

use Getopt::Long;
use Term::ANSIColor;

sub cdie
{
	my ($msg) = @_;
	die color('red'), "lt_includegen: ", $msg, color('reset');
}

my $outfile;

GetOptions ("output|o=s" => \$outfile);

# Читаем источник.
open INC, "< $ARGV[0]"
	or cdie "open source file '$ARGV[0]' failed";
my $body = join '', <INC>;
close INC;

# Обрабатываем

# #pragma once[(name)]
if ( $body =~ /\n[ \t]*?#\s*pragma\s+once\s*\(\s*(\w+)\s*\)[ \t;]*?\n/ ) {
	$body = "$`\n#ifndef\t$1\n#define\t$1\n$'\n#endif\t// $1\n";
} elsif ( $body =~ /\n[ \t]*?#\s*pragma\s+once[ \t;]*?\n/ ) {
	my ($pre, $post) = ($`, $');

	cdie "Not specified output file" unless (defined $outfile);

	$outfile =~ /\/([^\s\/]*?)$/;
	my $value = "__" . uc $1 . "__INCLUDEGEN";
	$value =~ s/\./_/g;
	$body = "$pre\n#ifndef\t$value\n#define\t$value\n$post\n#endif\t// $value\n";
}

# #pragma prototypes
if ( $body =~ /\n[ \t]*?#\s*pragma\s+prototypes[ \t;]*?\n/ ) {
	$body = "$`\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n" .
		"#ifdef __cplusplus\n}\n#endif\n$'";
}

# Пишем результат.
if (defined $outfile) {
	open INC, "> $outfile"
		or cdie "create target file '$outfile' failed";

	print INC $body;
	close INC;
} else {
	# выводим на stdout
	print $body;
}
