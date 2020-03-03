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

# Make a first stab at a label file that the spikesorter expects
# Simply adds strings to a .lbl file.
# Edit the file if you want to leave channels out.
# Uses the first .chan file as the name for label if there is no arg,
# otherwise uses arg.



if [ $# -eq 0 ] ; then
  declare -a files=(`ls 2>/dev/null *.chan`)
  len=${#files[@]}
  if [ ${len} -eq 0 ] ; then
     echo There do not seem to be any chan files in this directory
     echo Label file has not been created
     exit
  fi
  wholename=${files[0]}
  basename=${wholename%_*}
  echo ${wholename}
  echo ${basename}
  outfile=${basename}.lbl
elif [ $# -ne 1 ] ; then
	echo usage: $0 recording basename, such as 2012-03-12_001
	exit
else
   outfile=$1.lbl
fi

echo
echo "Create label file: $outfile"

echo Pegasus - 1  > $outfile
echo Pegasus - 2 >> $outfile
echo Pegasus - 3 >> $outfile
echo Pegasus - 4 >> $outfile
echo Pegasus - 5 >> $outfile
echo Pegasus - 6 >> $outfile
echo Pegasus - 7 >> $outfile
echo Pegasus - 8 >> $outfile
echo Pegasus - 9 >> $outfile
echo Pegasus - 10 >> $outfile
echo Pegasus - 11 >> $outfile
echo Pegasus - 12 >> $outfile
echo Pegasus - 13 >> $outfile
echo Pegasus - 14 >> $outfile
echo Pegasus - 15 >> $outfile
echo Pegasus - 16 >> $outfile
echo Pegasus - 17 >> $outfile
echo Pegasus - 18 >> $outfile
echo Pegasus - 19 >> $outfile
echo Pegasus - 20 >> $outfile
echo Pegasus - 21 >> $outfile
echo Pegasus - 22 >> $outfile
echo Pegasus - 23 >> $outfile
echo Pegasus - 24 >> $outfile
echo Medusa - 1 >> $outfile
echo Medusa - 2 >> $outfile
echo Medusa - 3 >> $outfile
echo Medusa - 4 >> $outfile
echo Medusa - 5 >> $outfile
echo Medusa - 6 >> $outfile
echo Medusa - 7 >> $outfile
echo Medusa - 8 >> $outfile
echo Medusa - 9 >> $outfile
echo Medusa - 10 >> $outfile
echo Medusa - 11 >> $outfile
echo Medusa - 12 >> $outfile
echo Medusa - 13 >> $outfile
echo Medusa - 14 >> $outfile
echo Medusa - 15 >> $outfile
echo Medusa - 16 >> $outfile
echo Medusa - 17 >> $outfile
echo Medusa - 18 >> $outfile
echo Medusa - 19 >> $outfile
echo Medusa - 20 >> $outfile
echo Medusa - 21 >> $outfile
echo Medusa - 22 >> $outfile
echo Medusa - 23 >> $outfile
echo Medusa - 24 >> $outfile
echo Medusa - 25 >> $outfile
echo Medusa - 26 >> $outfile
echo Medusa - 27 >> $outfile
echo Medusa - 28 >> $outfile
echo Medusa - 29 >> $outfile
echo Medusa - 30 >> $outfile
echo Medusa - 31 >> $outfile
echo Medusa - 32 >> $outfile
echo N1 >> $outfile
echo N2 >> $outfile
echo N3 >> $outfile
echo N4 >> $outfile
echo Tracheal Pressure >> $outfile
echo Blood Pressure >> $outfile
echo CO2 >> $outfile
echo Stimulus >> $outfile
echo Gemini - 1 >> $outfile
echo Gemini - 2 >> $outfile
echo Gemini - 3 >> $outfile
echo Gemini - 4 >> $outfile
echo Gemini - 5 >> $outfile
echo Gemini - 6 >> $outfile
echo Gemini - 7 >> $outfile
echo Gemini - 8 >> $outfile
echo Gemini - 9 >> $outfile
echo Gemini - 10 >> $outfile
echo Gemini - 11 >> $outfile
echo Gemini - 12 >> $outfile
echo Gemini - 13 >> $outfile
echo Gemini - 14 >> $outfile
echo Gemini - 15 >> $outfile
echo Gemini - 16 >> $outfile

# TODO: uncomment these when the Neptue array exists
#echo Neptune - 1 >> $outfile
#echo Neptune - 2 >> $outfile
#echo Neptune - 3 >> $outfile
#echo Neptune - 4 >> $outfile
#echo Neptune - 5 >> $outfile
#echo Neptune - 6 >> $outfile
#echo Neptune - 7 >> $outfile
#echo Neptune - 8 >> $outfile
#echo Neptune - 9 >> $outfile
#echo Neptune - 10 >> $outfile
#echo Neptune - 11 >> $outfile
#echo Neptune - 12 >> $outfile
#echo Neptune - 13 >> $outfile
#echo Neptune - 14 >> $outfile
#echo Neptune - 15 >> $outfile
#echo Neptune - 16 >> $outfile
#echo Neptune - 17 >> $outfile
#echo Neptune - 18 >> $outfile
#echo Neptune - 19 >> $outfile
#echo Neptune - 20 >> $outfile
#echo Neptune - 21 >> $outfile
#echo Neptune - 22 >> $outfile
#echo Neptune - 23 >> $outfile
#echo Neptune - 24 >> $outfile
#echo Neptune - 25 >> $outfile
#echo Neptune - 26 >> $outfile
#echo Neptune - 27 >> $outfile
#echo Neptune - 28 >> $outfile
#echo Neptune - 29 >> $outfile
#echo Neptune - 30 >> $outfile
#echo Neptune - 31 >> $outfile
#echo Neptune - 32 >> $outfile




echo "You may want to edit this to exclude some channels"
echo


