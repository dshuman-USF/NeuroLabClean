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
   Take a bunch of chan files and put them all back together in the binary file format
   that the spike2 program expects.  The format is to interleave the data from all of the chan files into blocks of 128 word signed 16-bit ints, like so:

       chan0 sample 1
       chan1 sample 1
          ...
       chan127 sample 1
       chan0 sample 2
           ...

   Can also be used for old Datamax chan files.
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

#define MAX_CHANS 128

bool DoOne = true;
bool DoTwo = true;
bool OverWrite = false;
bool HaveRaw = false;
char OutTag[2048] = "spike2";
char UsrTag[2048];
bool Debug = false;
bool SelList[MAX_CHANS];
int  SelChans = MAX_CHANS;

#define RAW_TAG "_r_"
#define BIN_EXT ".bin"
#define CHAN_EXT ".chan"

static void usage(char *name)
{
   printf (
"\nUsage: %s [-f] [-t tag_text] [subset of chans, e.g. 2 3 119 127]\n"\
"\n"\
"Combine a set of chan files from split recordings into a single \n"\
"interleaved .bin file that can be imported by the CED spike2 program.\n"\
"\n"\
"This must be run from the directory containing the chan files.\n"\
"Assumes all of the chan files are from the same recording.\n"\
"Assumes chan file names have these formats: YYYY-MM-DD_REC_CH.chan\n"\
"                                            YYYY-MM-DD_REC_r_CH.chan.\n"\
"It defaults to combining both 1-64 and 65-128 chan files into one .bin file.\n"\
"Datamax files will have 1-88 channels.  Channels 89-128 will be zeros.\n"\
"The output file names will have the same base name as the chan files, \n"\
"with \"_spike2-num_of_chans.bin\" attached to the name.\n"\
"\n"\
"OPTIONS\n"\
"-f to force over-writing existing output files.\n"\
"-t tag_text to add tag_text to the filename.\n"\
"List of chan numbers in the range of 1-128 to create a .bin with a subset.\n"\
"NOTE:  When you import the file in Spike2, you have to tell it the number of channels.\n"\
"\n"\
"It is not an error for chan files to be missing.  A default value of zero\n"\
"will be written to the .bin file for those channels.\n",
name
);
}

static int parse_args(int argc, char *argv[])
{
   static struct option opts[] = { 
                                   {"f", no_argument, NULL, '1'},
                                   {"d", no_argument, NULL, '2'},
                                   {"t", required_argument, NULL, '3'},
                                   { 0,0,0,0} };
   int cmd;
   int ret = 1;
   opterr = 0;
   int sel_index;
   int sel_start = -1;
   int sel_end = -1;
   char input[256];

   while ((cmd = getopt_long_only(argc, argv, "", opts, NULL )) != -1)
   {
      switch (cmd)
      {
         case '1':
               OverWrite = true;
               break;

         case '2':
               Debug = true;
               break;

         case '3':
               strncpy(UsrTag,optarg,sizeof(UsrTag)-1);
               if (UsrTag[0] == 0)
               {
                  printf("Tag text is missing, aborting. . .\n");
                  exit(1);
                  // note:  if there are chan nums following -t, the first one
                  // will be taken as the file name tag.
               }
               break;

         case '?':
         default:
            printf("Unknown argument, aborting. . .\n");
            ret = 0;
           break;
      }
   }


   if (ret)
   {
      if (optind < argc)
      {
         memset(SelList, false, sizeof(SelList)); // include only selected chans
         SelChans = 0;
         while (optind < argc)
         {
            if (sscanf(argv[optind],"%d-%d",&sel_start,&sel_end) == 2)
            {
               if (sel_start < 0 || sel_start > MAX_CHANS || sel_end < 0 || sel_start > MAX_CHANS)
               {
                  printf("Numbers are out of range, aborting. . .\n");
                  exit(1);
               }
               for (sel_index = sel_start; sel_index <= sel_end; ++sel_index)
               {
                  if (SelList[sel_index-1] == false)  // don't count duplicates
                  {
                     SelList[sel_index-1] = true;
                     ++SelChans;
                  }
               }
            }
            else if (sscanf(argv[optind],"%d",&sel_index) == 1)
            {
               if (sel_index < 1 || sel_index > MAX_CHANS)
               {
                  printf("Argument is out of range, aborting. . .\n");
                  ret = 0;
                  break;
               }
               else if (SelList[sel_index-1] == false)
               {
                  SelList[sel_index-1] = true;
                  ++SelChans;
               }
            }
            else
            {
               printf("Argument is not a number, aborting. . .\n");
               ret = 0;
               break;
            }
            ++optind;
         }
      }
      else
      {
         printf("You have selected all channels.\nThis can create a large file and take a long time.\nAre you sure you want to do this (Y/N)?");
         fgets(input,sizeof(input),stdin);
         if (tolower(input[0]) != 'y')
         {
            printf("Okay, aborting program. . .\n");
            exit(1);
         }
      }
   }

   printf("%d channels selected\n",SelChans);

   if (!ret)
      usage(argv[0]); 

   return ret;
}


/* Find a chan file in the current dir if there is one,
   then return the basename minus the recno.chan fields.
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
      
      
/* What we are here for.  Read all of the chan files selected in the SelList array
   combine them into a .bin file that CED Spike2 program can import.
   All 1-128 chans are selected by default, the user can specify a subset on
   the command line if they want to.
*/

static bool create_bin(char *outname, char* basename)
{
   FILE  *out_fd;
   FILE *in_fd[MAX_CHANS] = {NULL};
   int chan, chan_files = 0;
   char channame[PATH_MAX];
   unsigned long long feedback = 0, count = 0;
   struct stat info;
   double percent;
   char input[256];
   unsigned short word_to_write[1];
   size_t res;
   bool  done = false;

        // any chan files?
   for (chan = 0; chan < MAX_CHANS ; chan++)
   {
      if (HaveRaw)
         sprintf(channame,"%s%s%02d%s",basename,RAW_TAG,chan+1,CHAN_EXT);
      else
         sprintf(channame,"%s_%02d%s",basename,chan+1,CHAN_EXT);

      if (SelList[chan] && (in_fd[chan] = fopen(channame,"r")))
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
   else
      printf("Found %d chan files\n", chan_files);

   if (percent == 0)
   {
      printf("Could not find any chan files with data, aborting. . . \n");
      return false; 
   }

   if (!OverWrite && access(outname,F_OK) == 0)
   {
      printf("WARNING:  The file %s already exists.\n  Okay to over-write (Y/N)?  ",outname);
      fgets(input,sizeof(input),stdin);
      if (tolower(input[0]) != 'y')
      {
         printf("File is unchanged\n");
         goto error;
      }
   }

   out_fd = fopen(outname, "w");
   if (!out_fd)
   {
      printf("Problem opening output file %s, aborting. . .\n",outname);
      goto error;
   }

   while (!done)
   {
      for (chan = 0; chan < MAX_CHANS ; chan++)
      {
         if (SelList[chan])
         {
            if (in_fd[chan])
            {
               res = fread(word_to_write,sizeof(word_to_write),1,in_fd[chan]);
               if (res == 0)
               {
                  done = true;
                  break;
               }
            }
            else
              *word_to_write = 0;  // no chan file

            res = fwrite(word_to_write,sizeof(word_to_write),1,out_fd);
            if (res != 1)
            {
               printf("Error writing to output file %s\n",outname);
               done = true;
               break;
            }
         }
      }
      ++feedback;
      ++count;

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
   for (chan = 0; chan < MAX_CHANS ; chan++)
   {
      if (in_fd[chan])
         fclose(in_fd[chan]);
   }

   return true;
}


int main (int argc, char **argv)
{
   char basename[PATH_MAX];
   char binname[PATH_MAX];
   int  complain;

   memset(SelList, true, sizeof(SelList)); // all chans by default

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

   if (UsrTag[0] !=0)
      sprintf(binname,"%s_%s_%d_%s%s",basename,OutTag,SelChans,UsrTag,BIN_EXT);
   else
      sprintf(binname,"%s_%s_%d%s",basename,OutTag,SelChans,BIN_EXT);
   complain = !create_bin(binname , basename);

   if (complain)
      usage(argv[0]);

  return 0;
}
