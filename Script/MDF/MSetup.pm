#
# Copyright (c) 2000-2005 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#

package	MDF::MSetup;

use lib $ENV{'MDF_REPOS'} . '/Script';

use MDF::Config;
use MDF::Version;
use MDF::File;

use File::Basename qw/ basename /;

@EXPORT = qw/ FindRule FindMSetup /;

# Разборки с .msetup
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

sub FindRuleTest {
	my @Test = FindRule (undef, 'Test', undef);
	return 0 unless (
		defined $Test[0] and $Test[0] eq 'Test/Test' and
		defined $Test[1] and $Test[1] eq '1.1.1' and
		defined $Test[2] and $Test[2] =~ /^\s*Test;\s*$/);

	@Test = FindRule (undef, 'Test', '1.1.0');
	return 0 unless (
		defined $Test[0] and $Test[0] eq 'Test/Test' and
		defined $Test[1] and $Test[1] eq '1.1.1' and
		defined $Test[2] and $Test[2] =~ /^\s*Test;\s*$/);

	@Test = FindRule (undef, 'Test', '1.1.1');
	return 0 unless (
		defined $Test[0] and $Test[0] eq 'Test/Test' and
		defined $Test[1] and $Test[1] eq '1.1.1' and
		defined $Test[2] and $Test[2] =~ /^\s*Test;\s*$/);

	return 1;
}

unless (&FindRuleTest) {
	die "MDF::MSetup: Failed!\n";
}

1;
