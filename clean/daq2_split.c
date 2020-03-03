#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

/*
 Copyright 2005-2020 Kendall F. Morris
 
  This file is part of the USF Neural Recording Cleaning suite.
 
     The USF Neural Recording Cleaning Simulator suite is free software: you
     can redistribute it and/or modify it under the terms of the GNU General
     Public License as published by the Free Software Foundation, either
     version 3 of the License, or (at your option) any later version.
 
     The suite is distributed in the hope that it will be useful, but WITHOUT
     ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
     FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
     more details.
 
     You should have received a copy of the GNU General Public License along
     with the suite.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <stdbool.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define CHANS_PER_FILE 64

int
main (int argc, char **argv)
{
  if (argc == 1 || strncmp (argv[1], "-h", 2) == 0 || strncmp (argv[1], "--h", 3) == 0) {
    printf ("Usage: %s DAQFILE [CHANNEL]...\n"
            "Extracts channels from DAQFILE.daq into separate .chan files.\n\n"
            "If one or more CHANNEL's are specified, only those channels\n"
            "will be extracted, otherwise they all will be.\n"
            "CHANNEL must be in the range 1-64 for 1-64 daq files,\n"
            "65-128 for 65-128 daq files, or it wll be ignored.\n\n"
            "Leave off the .daq extension when specifying DAQFILE (but we'll remove it if you include it).\n"
            "Existing .chan files will be overwritten.\n\n"
            "This is backwards compatible, so if an older file without 1-64\n"
            "or 65-128 in the file name is given, it will assume a 1-64 channel file.\n",
            argv[0]);
    return 0;
  }

  bool include[CHANS_PER_FILE];
  int offset = 0;
  int yr, mon, day, recno;
  char chans[128];
  unsigned long long feedback = 0, count = 0;


  // Sometimes there is an extension, which breaks all kinds of
  // things, so remove it if it is there
  char *ext = strstr(argv[1], ".daq");
  if (ext)
     *ext = '\0';

  char *dirname;
  int match;

  printf("Splitting %s, ",argv[1]);
  if ((match = sscanf(argv[1],"%d-%d-%d_%d_%s", &yr,&mon,&day,&recno,chans)) == 5)
  {
     if (strcmp(chans,"1-64") == 0)
        printf("a 1-64 file.\n");
     else if (strcmp(chans,"65-128") == 0)
     {
        printf("a 65-128 file.\n");
        offset = CHANS_PER_FILE;
     }
     else
        printf("a legacy file\n");
  }
  else
     printf("a legacy file\n");

  if (argc > 2) 
  {
    memset (include, false, sizeof include);
    for (int i = 2; i < argc; i++) 
    {
      int chan = atoi (argv[i]) - offset;
      if (chan >= 1 && chan <= CHANS_PER_FILE)
        include[chan - 1] = true;
    }
  }
  else 
     memset (include, true, sizeof include);

  // Need to make a dir?
  asprintf(&dirname,"%s%03d","split.",recno);
  if (access(dirname,F_OK) != 0)
  {
     mode_t old_mask = umask(0);
     if (mkdir(dirname,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH) == -1)
        error (1, errno, "Error creating directory %s", dirname);
     umask(old_mask);
  }

  char *filename;
  asprintf (&filename, "%s.daq", argv[1]);

  FILE *fin = fopen (filename, "rb");
  if (!fin)
    error (1, errno, "Error opening %s for read", filename);
  free (filename);
  struct stat info;
  double percent;
  fstat(fileno(fin),&info);
  percent = info.st_size;

  FILE *f[CHANS_PER_FILE];
  for (int cidx = 0; cidx < CHANS_PER_FILE; cidx++)
    if (include[cidx]) {
          // strip off the 1-64 or 65-128 because we are adding the
          // explicit chan num to the filename.  Also add in a _r_ to
          // indicate a raw file.
      asprintf (&filename, "%s/%04d-%02d-%02d_%03d_r_%02d.chan", dirname,yr,mon,day,recno, cidx + 1 + offset);
      if ((f[cidx] = fopen (filename, "wb")) == NULL)
        error (1, errno, "Error opening %s for write", filename);
      free (filename);
    }
  unsigned short daqbuf;

  while (fread (&daqbuf, sizeof daqbuf, 1, fin) == 1)
    if (daqbuf == 0)
      break;
  int cidx = 0;
  if (fread (&daqbuf, sizeof daqbuf, 1, fin) == 1) 
  {
    if (daqbuf == 0)
      cidx = 0;
    else 
    {
      if (include[0]) 
      {
        short chanbuf = (int)daqbuf - 32768;
        fwrite (&chanbuf, sizeof daqbuf, 1, f[0]);
      }
      cidx = 1;
    }
  }
  bool sawzero = false;
  while (fread (&daqbuf, sizeof daqbuf, 1, fin) == 1)
  {
    if (cidx == CHANS_PER_FILE && daqbuf != 0)
       error(1, 0, " The file appears to be corrupted or it is not a recording file\n");
    ++feedback;
    ++count;

    if (daqbuf == 0) 
    {
      if (cidx != (sawzero ? 0 : CHANS_PER_FILE))
        error(1, 0, "bad data\n");
      sawzero = true;
      cidx = 0;
      continue;
    }
    sawzero = false;
    if (include[cidx]) 
    {
      short chanbuf = (int)daqbuf - 32768;
      fwrite (&chanbuf, sizeof daqbuf, 1, f[cidx]);
    }
    cidx++;
    if (count >= 1024*1024)
    {
      printf("\r  %3.0f%%",((feedback*2.0)/percent)*100.0);
      fflush(stdout);
      count = 0;
    }
  }

  free(dirname);
  printf("\r  100%%   \n");
  return 0;
}
