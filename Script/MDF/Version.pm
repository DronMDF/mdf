package MDF::Version;

@EXPORT = qw/ Valid /;

# Разборки с версиями
sub ExpandVersion {
	my ($v) = @_;
	return 0 unless (defined $v);
	
	my @vf = split /\./, $v;
	return 0 unless (defined $vf[0]);

	my $vi;
	$vi = $vf[0];
	$vi *= 100; $vi += $vf[1] if (defined $vf[1]);
	$vi *= 1000; $vi += $vf[2] if (defined $vf[2]);
	$vi *= 100; $vi += $vf[3] if (defined $vf[3]);

	return $vi;
}

# v1 должна быть больше или равно v2 тогда валидно
sub Valid {
	my (undef, $v1, $v2) = @_;
	return 0 if (ExpandVersion ($v1) < ExpandVersion ($v2));
	return 1;
}

sub ValidVersionTest {
	return 0 unless (&ExpandVersion ("1.1.1.1") == 10100101);
	return 0 unless (&ExpandVersion ("1") == 10000000);

	return 0 unless (&Valid(undef, "1.0.0", "0.0.0"));
	return 0 unless (&Valid(undef, "1.100.101", "1.100.100"));
	return 0 unless (&Valid(undef, "1.100.100", "1.100.100"));
	return 0 if (&Valid(undef, "0.0.0", "1.0.0"));
	return 1;
}

print "ValidVersion: Failed!\n" unless &ValidVersionTest;

