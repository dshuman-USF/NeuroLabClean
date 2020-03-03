#!/usr/bin/octave-cli -qf

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

if (length (argv) != 3 && length(argv) != 4)
  printf ("usage: %s filename_prefix chanlist_filename chunk_number [--no_r]\n", program_name);
  exit (3)
endif

# Clean the current chunk of data in all of the channel files with filename_prefix
# as a base name and and which are the chanlist_filename file.  Chunks are 0,
# 1, 2, etc.  Each chunk contains 1 second of data, which is 5,000,000 bytes,
# or 2,500,000 samples of data.

# This expects a basename of the form:
#  YYYY-MM-DD_REC
#  where 
#  REC is the recording number 001, 002, 003, etc.

# It assumes the source dir name is named split.REC/ 
# and that the destination dir is named clean.REC/  where:
# REC is the recording number, such as split.001/, and clean.001/.   
# If the dest dir does not exist, it is created here.

# The output filename format is:
#  YYYY-MM-DD_REC_CHN.chan  where CHN is the channel number 01, 02, etc.  
# For backwards compatibility with existing tools, the format specifier is
# %02d, which means there is at most a single leading zero. The channel field
# will be 2 digits for anything less than 100, and 3 digits for larger channel
# numbers.

# to allow this to process older chan files that were split with older software,
# if the --no_r flag is passed in, this expects the raw files in the split* dirs
# to not have a _r_ field in them.

# This returns zero on success, and several non-zero numbers on errors.

addpath("/usr/local/bin");

prefix = argv(){1};
chanlist = load (argv(){2});
chunk = str2num (argv(){3});

no_r = 0;
if (length(argv) == 4 && argv(){4} == "--no_r")
   no_r = 1;
endif

starttime = time;

chancnt = length (chanlist) - 2;
fidlist = cell ();
outlist = cell ();

# filename and dirname stuff.  
[year,mon,day,recno,pieces] = sscanf(prefix,"%d-%d-%d_%d","C");
[srcdir]=sprintf("split.%03d",recno);
[destdir]=sprintf("clean.%03d",recno);

# make destdir if we need to, and give all users all permissions
[oldmask]=umask(0);
[status,msg,msgid]=mkdir(destdir);
umask(oldmask);
[ex] = isdir(destdir);
if (!ex)
   printf("Create directory %s failed, msg is %s ex, is %d\n",destdir,msg,ex);
   exit(2)
endif

# Create output filename, lose the _r_ field  so, e.g., base name of 2012-02-08_r_001_29 
# becomes 2012-02-08_001_29.chan for channel 29 data.
for n = 1:chancnt
  if (no_r == 1)
     filename = sprintf ("%s/%s_%02d.chan", srcdir,prefix, chanlist(n));
  else
     filename = sprintf ("%s/%s_r_%02d.chan", srcdir,prefix, chanlist(n));
  endif

  [fid, msg] = fopen (filename, "r", "native");
  if (fid > 0)
     [seek_res] = fseek (fid, chunk * 5000000, SEEK_SET);
     if (seek_res == -1)         # seek fails in ver 3.x, this is the terminating condition.
                                 # it did not fail in 2.x, the read below was the 
                                 # terminating condition.  This may be a kernel issue, 
                                 # recent kernels appear to have changed the behavior of lseek
            # make sure in and out file are same size
        ofilename = sprintf ("%s/%04d-%02d-%02d_%03d_%02d.chan", destdir,year,mon,day,recno,chanlist(n));
        [i_info,i_err,i_msg]=stat(filename);
        [o_info,o_err,o_msg]=stat(ofilename);
        if (i_info.size != o_info.size)
           printf("\n*** WARNING ***\n"); 
           printf("Input file is not the same size as output file\n\n");
           fflush (stdout);
        endif
        exit (2)
     endif
     else
       printf("File %s not found\n",filename);
       exit(2)
  endif
  fidlist = [fidlist, fid];
  if (fid == -1)
    printf ("can't open %s: %s\n", filename, msg);
    fflush (stdout);
    exit (2)
  endif
endfor

printf ("chunk %d\n", chunk);
fflush (stdout);

[src_fname] = filename;

for n = 1:chancnt+2
  filename = sprintf ("%s/%04d-%02d-%02d_%03d_%02d.chan", destdir,year,mon,day,recno,chanlist(n));
  if (chunk == 0) mode = "w"; else mode = "a"; endif
  [fid, msg] = fopen (filename, mode, "native");
  outlist = [outlist, fid];
  if (fid == -1)
    printf ("can't open %s: %s\n", filename, msg);
    fflush (stdout);
    exit (2)
  endif
endfor


for chan = 1:chancnt
  [rawdata(:,chan), count] = fread (fidlist{chan}, 2500000, "int16");
  if (count == 0)
    exit (1)
  endif
endfor

cleaned = CleanData (rawdata(1:count,:));
do_fortran_indexing = 1;
cleaned(find (cleaned(:,:) >  32767)) =  32767;
cleaned(find (cleaned(:,:) < -32768)) = -32768;
do_fortran_indexing = 0;
cleaned = floor (cleaned + .5);

for chan = 1:chancnt+2
  count = fwrite (outlist{chan}, cleaned(:, chan), "int16");
endfor

for n = 1:chancnt
  fclose (fidlist{n});
endfor
  
for n = 1:chancnt+2
  fclose (outlist{n});
endfor

printf ("time elapsed = %8.2f\n", (time - starttime));

[s, err, msg] = stat(src_fname);
if (err == 0)
  [perc] = (((chunk+1)*5000000)/s.size)*100;
  if (perc > 100)
     perc = 100;
  endif
  printf("%3.0f%% complete\n",perc);
else
   printf("%s\n",msg);
endif

fflush (stdout);

% octave 3.4.2 has a bug that causes to throw
% an un-catchable exception when run from a command line script.
% By default, if we get here, a 0 will be returned, so we don't really
% need to do this anyway.
% exit (0)
