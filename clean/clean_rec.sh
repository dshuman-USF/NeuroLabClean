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

# a utility script to automate cleaning the chan files from a single recording

# Before running this, you should edit the chanlist* files to include or
# exclude the channels you want

# Note that we also handle the noclean files, which should just be copied from
# the split dir to the clean dir.

# To support old datamax files that have been split by daq_split, not
# daq2_split, use the optional --no_r flag after the recording number.  This is
# handed to down-stream software so it will not expect a _r_ in the raw
# filenames.


if [ $# -ne 1 -a $# -ne 2 ] ; then
	echo usage: $0 single recording number, such as 001, with optional --no_r flag.
	echo such as $0 001 --no_r
	exit
fi

if [ $# -eq 2 ] && [ $2 != '--no_r' ] ; then
	echo The flag $2 is not --no_r
	exit
fi


echo "Start cleaning operations"

if [ -e chanlist_$1_1 ] ; then
   echo "** Start new cleaning operation for ${PWD##*/}_$1 $2 **" >> chanlist_$1_1.log
   do_clean_data.sh ${PWD##*/}_$1 chanlist_$1_1 $2 >> chanlist_$1_1.log 2>&1 &
fi

if [ -e chanlist_$1_2 ] ; then
   echo "** Start new cleaning operation for ${PWD##*/}_$1 $2 **" >> chanlist_$1_2.log
   do_clean_data.sh ${PWD##*/}_$1 chanlist_$1_2 $2 >> chanlist_$1_2.log 2>&1 &
fi

if [ -e chanlist_$1_3 ] ; then
  echo "** Start new cleaning operation for ${PWD##*/}_$1 $2 **" >> chanlist_$1_3.log
  do_clean_data.sh ${PWD##*/}_$1 chanlist_$1_3 $2 >> chanlist_$1_3.log 2>&1 &
fi

if [ -e chanlist_$1_4 ] ; then
  echo "** Start new cleaning operation for ${PWD##*/}_$1 $2 **" >> chanlist_$1_4.log
  do_clean_data.sh ${PWD##*/}_$1 chanlist_$1_4 $2 >> chanlist_$1_4.log 2>&1 &
fi

if [ -e chanlist_$1_5 ] ; then
  echo "** Start new cleaning operation for ${PWD##*/}_$1 $2 **" >> chanlist_$1_5.log
  do_clean_data.sh ${PWD##*/}_$1 chanlist_$1_5 $2 >> chanlist_$1_5.log 2>&1 &
fi

if [ -e chanlist_$1_6 ] ; then
  echo "** Start new cleaning operation for ${PWD##*/}_$1 $2 **" >> chanlist_$1_6.log
  do_clean_data.sh ${PWD##*/}_$1 chanlist_$1_6 $2 >> chanlist_$1_6.log 2>&1 &
fi

if [ -e chanlist_$1_7 ] ; then
  echo "** Start new cleaning operation for ${PWD##*/}_$1 $2 **" >> chanlist_$1_7.log
  do_clean_data.sh ${PWD##*/}_$1 chanlist_$1_7 $2 >> chanlist_$1_7.log 2>&1 &
fi

if [ -e chanlist_$1_8 ] ; then
  echo "** Start new cleaning operation for ${PWD##*/}_$1 $2 **" >> chanlist_$1_8.log
  do_clean_data.sh ${PWD##*/}_$1 chanlist_$1_8 $2 >> chanlist_$1_8.log 2>&1 &
fi

# 9 and 10 may exist but are empty by default
if [ -s chanlist_$1_9 ] ; then
   echo "** Start new cleaning operation for ${PWD##*/}_$1 $2 **" >> chanlist_$1_9.log
   do_clean_data.sh ${PWD##*/}_$1 chanlist_$1_9 $2 >> chanlist_$1_9.log 2>&1 &
fi

if [ -s chanlist_$1_10 ] ; then
   echo "** Start new cleaning operation for ${PWD##*/}_$1 $2 **" >> chanlist_$1_10.log
   do_clean_data.sh ${PWD##*/}_$1 chanlist_$1_10 $2 >> chanlist_$1_10.log 2>&1 &
fi

if [ -e nocleanlist_$1 ] ; then
  echo "** Processing files in the no clean list ${PWD##*/}_$1 $2 **" >> nocleanlist_$1.log
  do_noclean_data.sh ${PWD##*/}_$1 nocleanlist_$1 $2 >> nocleanlist_$1.log 2>&1 &
fi

echo "Then cleaning processes have been started in parallel in the background."
echo "You can view progress by viewing the channel log files, for example, type:"
echo "tail -f chanlist_$1_1.log"
echo
