#!/bin/sh

cd src
echo "Starting compiler"
huc -DDEBUG -s -02 test.c
if [ "$?" == "0" ]
then
	echo "Compile successful - starting assembler"
	pceas -s l0 test.s
	if [ "$?" == "0" ]
	then
		echo "Assembly successful"
	else
		echo "Assembly not successful"
	fi
else
	echo "Compile not successful"	
fi
cd -
