#!/bin/bash
filename=""

while [ "$1" != "" ]; do
	key="$1"
	case $key in 
		-i)
			filename="$2"
			shift
			;;
		*)
			m="$1"
			;;
	esac
	shift
done	
		
if [ "$filename" != "" ];
then
	./chordprc_o $m < $filename
else
	./chordprc_o $m
fi