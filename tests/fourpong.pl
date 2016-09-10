#!/usr/bin/perl

use threads;

sub pong {
	qx{../build/ensipong};
}


$th1 = threads->new(\&pong);
$th2 = threads->new(\&pong);
$th3 = threads->new(\&pong);
$th4 = threads->new(\&pong);

$th1->join;
$th2->join;
$th3->join;
$th4->join;
