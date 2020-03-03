#!/usr/bin/octave3.2 -qf

if (length (argv) != 3)
  printf ("usage: %s filename_prefix chanlist_filename chunk_number\n", program_name);
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


# This returns zero on success, and several non-zero numbers on errors.

addpath("/usr/local/bin");

prefix = nth(argv, 1);
chanlist = load (nth(argv, 2));
chunk = str2num (nth(argv, 3));

starttime = time;

chancnt = length (chanlist) - 2;
fidlist = cell ();
outlist = cell ();

# filename and dirname stuff.  
[year,mon,day,recno,pieces] = sscanf(prefix,"%d-%d-%d_%d","C");
[srcdir]=sprintf("split.%03d",recno);
[destdir]=sprintf("clean.%03d",recno);

# make destdir if we need to, and give all users all permissions
[ex]= isdir(destdir);
if (!ex)
   [oldmask]=umask(0);
   [status,msg,msgid]=mkdir(destdir);
   umask(oldmask);
   if (status == 0)
      printf("Create directory %s failed, msg is %s",destdir,msg);
      exit(2)
   endif
endif

# Create output filename, lose the _r_ field  so, e.g., base name of 2012-02-08_r_001 becomes
# input file name of 2012-02-08_001_029.chan for channel 29 data.
for n = 1:chancnt
  filename = sprintf ("%s/%s_r_%02d.chan", srcdir,prefix, chanlist(n));
  [fid, msg] = fopen (filename, "r", "native");
  if (fid > 0)
     [seek_res] = fseek (fid, chunk * 5000000, SEEK_SET);
     if (seek_res == -1)         # seek fails in ver 3.x, this is the terminating condition
       exit (2)                  # it apparently did not fail in 2.x, the read below
     endif                       # was the terminating condition
     else 
       printf("File %s not found\n",filename);
       exit(2)
  endif
  fidlist = append (fidlist, fid);
  if (fid == -1)
    printf ("can't open %s: %s\n", filename, msg);
    fflush (stdout);
    exit (2)
  endif
endfor

[src_fname] = filename;

printf ("chunk %d\n", chunk);
fflush (stdout);

for n = 1:chancnt+2
  filename = sprintf ("%s/%04d-%02d-%02d_%03d_%02d.chan", destdir,year,mon,day,recno,chanlist(n));
  if (chunk == 0) mode = "w"; else mode = "a"; endif
  [fid, msg] = fopen (filename, mode, "native");
  outlist = append (outlist, fid);
  if (fid == -1)
    printf ("can't open %s: %s\n", filename, msg);
    fflush (stdout);
    exit (2)
  endif
endfor


for chan = 1:chancnt
  [rawdata(:,chan), count] = fread (nth (fidlist, chan), 2500000, "int16");
  if (count == 0)
    exit (1)
  endif
endfor

cleaned = CleanData (rawdata);

do_fortran_indexing = 1;
cleaned(find (cleaned(:,:) >  32767)) =  32767;
cleaned(find (cleaned(:,:) < -32768)) = -32768;
do_fortran_indexing = 0;
cleaned = floor (cleaned + .5);

for chan = 1:chancnt+2
  count = fwrite (nth (outlist, chan), cleaned(:, chan), "int16");
endfor
  
for n = 1:chancnt
  fclose (nth (fidlist,n));
endfor
  
for n = 1:chancnt+2
  fclose (nth (outlist,n));
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

exit (0)
