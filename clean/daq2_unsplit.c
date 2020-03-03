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



/*
   Take a bunch of chan files and put them all back together in the .daq file
   format.  Typically, this is used to create a play-able file of cleaned
   channels using the play_daq2 utility.  Can also be used for old Datamax chan files.
*/


#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdbool.h>
#include <error.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/limits.h>
#include <getopt.h>
#include <errno.h>
#include <dirent.h> 

#define CHANS_PER_FILE 64

bool DoOne = true;
bool DoTwo = true;
bool OverWrite = false;
bool HaveRaw = false;
char OutTag[2048] = "clean";
bool Debug = false;

#define RAW_TAG "_r_"
#define DAQ_EXT ".daq"
#define CHAN_EXT ".chan"

static void usage(char *name)
{
   printf (
"\nUsage: %s [-1] [-2] [-t tag] [-f]\n"\
"\n"\
"Combine a set of 1-64 and 65-128 chan files from split recordings\n"\
"into two .daq format files.\n"\
"\n"\
"This must be run from the directory containing the chan files.\n"\
"Assumes all of the chan files are from the same recording.\n"\
"Assumes chan file names have these formats: YYYY-MM-DD_REC_CH.chan\n"\
"                                            YYYY-MM-DD_REC_r_CH.chan.\n"\
"It defaults to combining both 1-64 and 65-128 chan files into two .daq files.\n"\
"Datamax files will have 1-64 in one file and 65-88 in the second one.\n"\
"The output file names will have the same base name as the chan files, \n"\
"with \"_clean\" attached to the name.\n"\
"\n"\
"-1 means combine only the 1-64 chan files.\n"\
"-2 means combine only the 65-128 chan files.\n"\
"-t tag adds \"tag\" to the output name instead of \"_clean\".\n"\
"-f to force over-writing existing output files.\n"\
"\n"\
"It is not an error for chan files to be missing.  A default value of zero\n"\
"will be written to the .daq file for those channels.\n",
name
);
}

static int parse_args(int argc, char *argv[])
{
   static struct option opts[] = { {"1", no_argument, NULL, '1'},
                                   {"2", no_argument, NULL, '2'},
                                   {"f", no_argument, NULL, '3'},
                                   {"tag", required_argument, NULL, '4'},
                                   {"d", no_argument, NULL, '5'},
                                   { 0,0,0,0} };
   int cmd;
   bool have_1 = false, have_2 = false;
   int ret = 1;
   opterr = 0;

   while ((cmd = getopt_long_only(argc, argv, "", opts, NULL )) != -1)
   {
      switch (cmd)
      {
         case '1':
               have_1 = true;
               break;
 
         case '2':
               have_2 = true;
               break;

         case '3':
               OverWrite = true;
               break;

         case '4':
               strncpy(OutTag,optarg, sizeof(OutTag)-1);
               break;

         case '5':
               Debug = true;
               break;

         case '?':
         default:
            ret = 0;
           break;
      }
   }

   if (have_1 && !have_2)
   {
      DoOne = true;
      DoTwo = false;
   }
   else if (!have_1 && have_2)
   {
      DoOne = false;
      DoTwo = true;
   }
     // otherwise always look for both sets of chans

   if (!ret)
      usage(argv[0]); 

   return ret;
}


/* Find a chan file in the current dir if there is one,
   then return the basename minus the .chan extenstion.
   Also note if it is a raw file for later.
   Returns true of it finds one.
           false otherwise.
*/
static bool find_chan(char *basename)
{
   DIR    *curr_dir;
   struct dirent *dir;
   bool   ret = false;
   int    match, yr, mon, day, recno;
   
   curr_dir = opendir(".");
   if (curr_dir)
   {
       while ((dir = readdir(curr_dir)) != NULL)
       {
          if (strstr(dir->d_name,CHAN_EXT))
          {
             if (strstr(dir->d_name,RAW_TAG))
                HaveRaw = true;

             match = sscanf(dir->d_name,"%d-%d-%d_%d", &yr,&mon,&day,&recno);
             if (match == 4)
             {
                sprintf(basename,"%04d-%02d-%02d_%03d", yr,mon,day,recno);
                ret = true;
                break;
             }
             else
             {
                printf("The chan file %s is not in the correct filename format\n",dir->d_name);
                break;
             }
          }
      }
      closedir(curr_dir);
   }
   return ret;
}
      
      
/* What we are here for.  Read all of the chan files for the current
   section, 1-64 or 65-128 and combine them all back into a .daq file that
   looks like a recording.  
   The format is a set of blocks.  The first two words in the block are 0000 0000.
   Then, in order, the value of the channels from 1-64 or 65-128.
*/
static bool create_daq(char *outname, char* basename, int chan_start)
{
   FILE  *out_fd;
   FILE *in_fd[CHANS_PER_FILE];
   int chan, chan_files = 0;
   char channame[PATH_MAX];
   unsigned long long feedback = 0, count = 0;
   struct stat info;
   double percent;
   char input[256];
   unsigned short marker[1] = {0};
   unsigned short word_to_write[1];
   size_t res;
   bool  done = false;
   bool need_marker = true;

        // any chan files?
   for (chan = 0; chan < CHANS_PER_FILE ; chan++)
   {
      if (HaveRaw)
         sprintf(channame,"%s%s%02d%s",basename,RAW_TAG,chan_start+chan,CHAN_EXT);
      else
         sprintf(channame,"%s_%02d%s",basename,chan_start+chan,CHAN_EXT);

      if ((in_fd[chan] = fopen(channame,"r")))
      {
         if (!chan_files)  // get 1st chan file size, assumes all are same size
         {
            fstat(fileno(in_fd[chan]),&info);
            percent = info.st_size/2;  // # words in file
         }
         ++chan_files;
      }
   }

   if (chan_files == 0)
   {
      printf("Could not find any chan files.\n");
      return false; 
   }

   if (percent == 0)
   {
      printf("Could not find any chan files with data, skipping %s\n",outname);
      return false; 
   }

   if (!OverWrite && access(outname,F_OK) == 0)
   {
      printf("WARNING:  The file %s already exists.\n  Okay to over-write (Y/N)?  ",outname);
      fgets(input,sizeof(input),stdin);
      if (tolower(input[0]) != 'y')
      {
         printf("File is unchanged, channels %d-%d skipped\n",chan_start,chan_start+63);
         goto error;
      }
   }

   out_fd = fopen(outname, "w");
   if (!out_fd)
   {
      printf("Problem opening output file %s, skipping. . .\n",outname);
      goto error;
   }

   while (!done)
   {
      for (chan = 0; chan < CHANS_PER_FILE ; chan++)
      {
         if (in_fd[chan])
         {
            res = fread(word_to_write,sizeof(word_to_write),1,in_fd[chan]);
            if (res == 0)
            {
               done = true;
               break;
            }
            *word_to_write += 0x8000;   // convert to daq board's data format
         }
         else
            *word_to_write = 0x8000;   // daq file's zero value

        if (need_marker)
        {
           fwrite(marker,sizeof(marker),1,out_fd);    // block header of 0000 0000 
           fwrite(marker,sizeof(marker),1,out_fd);
           need_marker = false;
           ++feedback;
           ++count;
        }
         errno = 0;
         res = fwrite(word_to_write,sizeof(word_to_write),1,out_fd);
         if (res != 1)
         {
            printf("Error writing to output file %s\n",outname);
            printf("   errno is %d   res is %d \n",errno,res);
            char *errstr = strerror(errno);
            printf("   %s\n",errstr);
            printf("   Note that %s will be incomplete.\n",outname);
            done = true;
            break;
         }
      }
      need_marker = true;   // start new block
 
      if (count > 1024)
      {
         printf("\r  %3.0f%%",(feedback/percent)*100.0);
         fflush(stdout);
         count = 0;
      }
   }

   if (out_fd)
   {
      printf("\r  %3.0f%%",(feedback/percent)*100.0);
      printf("\nCompleting write. . .\n"); // there can be a lot buffered data, 
      fflush(stdout);                      // closing can take many seconds, so reassure user
      fclose(out_fd);
      printf("Creation of %s is complete.\n", outname);
      fflush(stdout);
   }

error:
   for (chan = 0; chan < CHANS_PER_FILE ; chan++)
   {
      if (in_fd[chan])
         fclose(in_fd[chan]);
   }

   return true;
}


int main (int argc, char **argv)
{
   char basename[PATH_MAX];
   char daq1name[PATH_MAX];
   char daq65name[PATH_MAX];
   int  complain;

   if (!parse_args(argc, argv))
      exit(1);

   if (Debug)
   {
      printf("attach debugger, then press ENTER");
      getchar();
   }

   if (!find_chan(basename))
   {
      printf("\nCould not find any chan files, exiting. . .\n");
      usage(argv[0]);
      exit(1);
   }

   if (DoOne)
   {
      sprintf(daq1name,"%s_%s%s%s",basename,OutTag,"_1-64",DAQ_EXT);
      complain = !create_daq(daq1name, basename, 1);
   }

   if (DoTwo)
   {
      sprintf(daq65name,"%s_%s%s%s",basename,OutTag,"_65-128",DAQ_EXT);
      complain += !create_daq(daq65name, basename, 65);
   }

   if (complain)
      usage(argv[0]);

  return 0;
}
