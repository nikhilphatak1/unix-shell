#!/usr/bin/perl
use 5.16.0;
use warnings FATAL => 'all';

use Test::Simple tests => 17;

system("mkdir -p tmp");

my @scripts = glob("tests/*.in");

for my $script (@scripts) {
    system("rm -f tmp/output");
    system("timeout -k 5 10 ./shell $script > tmp/output");

    my $correct = $script;
    $correct =~ s/\.in$/.out/;
    my $diff = `diff $correct tmp/output`;

    ok($diff eq "", $script);
    if ($diff ne "") {
        $diff =~ s/^/# /mg;
        print $diff;
    }
}

sub check_errors {
    my ($errs) = @_;
    chomp $errs;
    if ($errs ne "") {
        $errs =~ s/^/# /mg;
        warn $errs;
        return 0;
    }
    else {
        return 1;
    }
}

sub clang_check {
    my $errs = `clang-check -analyze *.c -- 2>&1`;
    return check_errors($errs);
}

sub valgrind() {
    system(qq{valgrind -q --leak-check=full --log-file=valgrind.out ./shell tests/01-echo.in > main.out});
    return `cat -n valgrind.out` eq "";
}

ok(clang_check(), "clang check");
ok(valgrind(), "valgrind");

my $procs = `ps | grep memcheck`;
chomp $procs;

ok($procs eq "", "no stray processes after tests");
if ($procs ne "") {
    system("killall memcheck-amd64-");
}
