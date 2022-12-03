#!/bin/sh
echo "the name of the script is $0"
echo "the first argument is $1"
echo "the second argument is $2"
echo "a list of all the arguments is $*"
echo "the script places the date into a temporary file called $1_$$"
date > $1.$$ # redirect the output of date
