#!/bin/bash

# rude and crude, make a first stab at the chan files that the cleaner wants
# Edit the file if you want to leave channels out.
# run this in the dir where the .daq files live.

if [ $# -lt 1 ] ; then
	echo usage: $0 recording number[s], such as "$0 001 002 003"
	exit
fi


for rec in "$@"
do

   echo "Make channel cleaning include/exclude files for recording ${rec}"
   echo "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 199 198" > chanlist_${rec}_1
   echo "17 18 19 20 21 22 23 24 299 298" > chanlist_${rec}_2
   echo "25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 399 398" > chanlist_${rec}_3
   echo "41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 499 498" > chanlist_${rec}_4
   echo "65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 00 599 598" > chanlist_${rec}_5
   echo "57 58 59 60 119 120 121 122 123 699 698" > chanlist_${rec}_6
   echo NOTE:  chanlist_${rec}_7 and chanlist_${rec}_8 have not been created since the Neptune array is not available.

#TODO no hardware for these channels yet, so don't include them (yet)
#echo "81 82 83 84 85 86 87 88 89 90 91 92 93 95 96 95 96 799 798" > chanlist7
#echo "97 98 99 100 101 102 103 104 105 106 107 108 109 110 110 112 899 898" > chanlist8

   echo "61 62 63 64 124 125 126 127 128" > nocleanlist_$rec

done

echo "You may want to edit these to adjust the channels"
echo
