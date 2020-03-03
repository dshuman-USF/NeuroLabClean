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



# a convenience function to automate splitting .daq files into .chan files.

if [ $# -lt 1 ] ; then
	echo usage: $0 recording number[s], such as "$0 001 002 003"
	exit
fi

echo "Start splitting operations"

for rec in "$@"
do
    echo "$rec"

   file1=${PWD##*/}_${rec}_1-64
   file2=${PWD##*/}_${rec}_65-128

   if [ -e $file1.daq ] ; then
      daq2_split $file1
   else
      echo "$file1.daq does not exists"
   fi

   if [ -e $file2.daq ] ; then
      daq2_split $file2
   else
      echo "$file2.daq does not exists"
   fi

done

