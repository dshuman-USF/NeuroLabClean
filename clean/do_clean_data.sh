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


# Tell do_clean_data2.m to clean the current channels with a filename_prefix of
# the form:
#   YYYY-MM-DD-REC
# Where YYYY year, MM is month, DD is day
# REC is the recording number, 001, 002, 003, etc.

# The channels to be cleaned are contained in the chanlist_filename file.  This
# is a list of channel numbers, such as 1 2 3 4 5 6.  These are used to name
# the output files of the cleaning operation, and take the form of:
# YYYY-MM-DD-REC_CHN.chan where CHN is the channel number contained in the
# chanlist_filename.

# The cleaning software expects two extra numbers in chanlist_filename, such as
# 198 and 199.  These files hold the noise data that was subtracted from the
# data to clean the data.  The first one contains the first cleaning pass data,
# and the second one contains the second pass cleaning data.  The convention is
# that the first chanlist file uses 198, 199.  The convention is that the
# second chanlist file contains 298 299, and so on.

# The cleaning software wants to process data in one second segments, so
# do_clean_data.m is invoked with a chunk parameter which indicates the next
# segment of the file to be cleaned.  The iteration continues as long as
# do_clean_data.m returns a status of zero.  The interation stops when it
# reaches end of file or there is an error condition.

# The optional --no_r flag tells do_clean_data2.m to not expect a _r_ in the 
# raw filenames.  This allows older datamax records that were split with
# earlier software to be processed by the daq2 scripts.


if [ $# -ne 2 ] && [ $# -ne 3 ] ; then
	echo usage: $0 filename_prefix chanlist_filename [--no_r]
	exit
fi

if [ $# -eq 3 ] && [ $3 != "--no_r" ] ; then
	echo The flag $3 is not --no_r
	exit
fi

prefix=$1
chanlist_filename=$2
opt_arg=$3

echo $prefix $chanlist_filename $opt_arg

((n = 0))
while do_clean_data2.m $prefix $chanlist_filename $n $opt_arg; do
    ((n++))
done

echo $0 done
