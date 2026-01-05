#!/bin/bash
a=$1
b=$2
c=$3

echo "a-b = $(($a - $b))"
echo "a+b = $(($a + $b))"
echo "c/b = $(($c / $b))"
echo "c*b = $(($c * $b))"
result=$((($a - $b) * ($b - $a)))
echo "(a-b)*(b-a) = $result"
