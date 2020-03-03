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

# This script copies channel files listed in a "do not clean" list to the
# target directory.  It extracts the recording number REC from the prefix name
# and assumes the src dir is split.REC/ and the destination is clean.REC/.  The
# reason would use this is that while spike train signal sources need
# to be run through the data cleaning processing, some channels, such as blood
# pressure do not.  Downstream processing expects to see all files in the same
# directory.  This copies the raw data files into the destination directory to
# a final file name.  It assumes a prefix format of:
#   YYYY-MM-DD_REC
#  where YYYY-MM-DD is the year, month, date, and REC is the recording number
#  001, 002, etc.

echo got $# args

if [ $# -ne 2 ] && [ $# -ne 3 ] ; then
	echo usage: $0 filename_prefix chanlist_filename [--no_r]
	exit
fi


if [ $# -eq 3 ] && [ $3 != '--no_r' ] ; then
	echo The flag $3 is not --no_r
	exit
fi


prefix=$1

while read -a line; do
  len=${#line[@]}
  wholename=$1
  basename=${wholename%_*}
  recno=${wholename#*_}
  destdir="clean."$recno
  srcdir="split."$recno

echo "Copy files that do not need cleaning to $destdir"

# it is very likely do_clean_data2.m has already created destdir,
# but try to create it and make sure it exists
  mkdir $destdir -m 777 &> /dev/null
  if [ ! -d $destdir ] ; then
    echo "Error creating $destdir, exiting. . ."
    exit 2
  fi

  for ((chan=0; chan<len; chan++));
    do
      destname=`printf "%s/%s_%03d_%02d.chan" $destdir $basename $recno ${line[chan]}`
      if [ $# -eq 3 ] && [ $3 == '--no_r' ] ; then
         srcname=`printf "%s/%s_%02d.chan" $srcdir $1 ${line[chan]}`
      else
         srcname=`printf "%s/%s_r_%02d.chan" $srcdir $1 ${line[chan]}`
      fi
      cp -v $srcname  $destname
    done
done < $2




