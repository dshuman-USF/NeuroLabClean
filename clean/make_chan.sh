#!/bin/bash

#Copyright 2005-2020 Kendall F. Morris
#
# This file is part of the USF Neural Recording Cleaning suite.
#
#    The USF Neural Recording Cleaning Simulator suite is free software: you
#    can redistribute it and/or modify it under the terms of the GNU General
#    Public License as published by the Free Software Foundation, either
#    version 3 of the License, or (at your option) any later version.
#
#    The suite is distributed in the hope that it will be useful, but WITHOUT
#    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#    more details.
#
#    You should have received a copy of the GNU General Public License along
#    with the suite.  If not, see <https://www.gnu.org/licenses/>.

# rude and crude, make a first stab at the chan files that the cleaner wants
# Edit the file if you want to leave channels out.
# run this in the dir where the .daq files live.

# Modification history
# 13-Sep-2013  Add channel 123 to chanlist 6
#              Add in question to determine what chan config to use.
#              The 64-128 set of chans were recently reorganized to move and
#              add in 2 more chans.  We use these now and not any from the 1-64
#              chans
# 


if [ $# -lt 1 ] ; then
	echo usage: $0 recording number[s], such as "$0 001 002 003"
	exit
fi

echo "Enter 1 to create chan lists for nerves 1-5 on chans 57-60, 123 (pre August 2013)"
echo "Enter 2 to create chan lists for nerves 1-7 on chans 113-119 (post August 2013)"
read choice
if [ $choice != 1 ] && [ $choice != 2 ] ; then
  echo "Error:  Enter 1 or 2"
  exit
fi


for rec in "$@"
do

   echo "Make channel cleaning include/exclude files for recording ${rec}"
   echo "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 199 198" > chanlist_${rec}_1
   echo "17 18 19 20 21 22 23 24 299 298" > chanlist_${rec}_2
   echo "25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 399 398" > chanlist_${rec}_3
   echo "41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 499 498" > chanlist_${rec}_4
   echo "65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 599 598" > chanlist_${rec}_5
   if [ $choice = 1 ] ; then
      echo "Creating pre August 2013 chan lists"
      echo "57 58 59 60 123 699 698" > chanlist_${rec}_6
   else
      echo "Creating post August 2013 chan lists"
      echo "113 114 115 116 117 118 119 699 698" > chanlist_${rec}_6
   fi
   echo "81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 799 798" > chanlist_${rec}_7
   echo "97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 899 898" > chanlist_${rec}_8
   echo -n "" > chanlist_${rec}_9
   echo -n "" > chanlist_${rec}_10
   echo "61 62 63 64 120 121 122 123 124 125 126 127 128" > nocleanlist_$rec

done

echo "You may want to edit these to adjust the channels"
echo

