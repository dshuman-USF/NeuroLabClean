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
   Take one or two .daq files and create a .bin file that can be imported into
   the CED Spike2 program.
   
   The format is to interleave the data from all of the chan files into sample
   blocks of up to 128 word signed 16-bit ints, like so:

       chan0 sample 1
       chan1 sample 1
          ...
       chan127 sample 1
       chan0 sample 2
       chan1 sample 2
           ...
   For subsets of channels, there will be fewer words in each sample block.
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
#define CHANS_PER_SAMP 64
#define MARKER_LEN 2          // 2 leading 0000 0000 words per sample in daq file
#define WORDS_PER_SAMP (CHANS_PER_SAMP+MARKER_LEN)
#define BYTES_PER_SAMP (WORDS_PER_SAMP*2)

bool DoOne = true;
bool DoTwo = true;
bool OverWrite = false;
bool HaveRaw = false;
char OutTag[2048] = "spike2";
char UsrTag[2048];
bool Debug = false;
bool SelList[MAX_CHANS] = {true};
int  SelChans = MAX_CHANS;
char RecNo[128];
char Daq0[PATH_MAX];
char Daq1[PATH_MAX];
char Bin[PATH_MAX];

#define BIN_EXT ".bin"
#define DAQ_EXT ".daq"

static void usage(char *name)
{
   printf (
"\nUsage: %s -r recording_num [-f] [-t tag_text] [subset of chans, e.g. 2 3 7-19 119 127]\n"\
"\n"\
"Scan one or two .daq files and create an interleaved .bin file that can be imported \n"\
"by the CED spike2 program.\n"\
"\n"\
"This must be run from the directory containing the .daq files.\n"\
"Assumes daq file names have one of these formats: YYYY-MM-DD_REC_1-64.daq\n"\
"                                                  YYYY-MM-DD_REC_64-128.daq.\n"\
"                                                  YYYY-MM-DD_REC.daq.\n"\
"Older files without the channel numbers are assumed to be 1-64 files.\n"\
"It defaults to combining the 1-64 and 65-128 files into one .bin file.\n"\
"The output file names will have the same base name as the .daq files, \n"\
"with \"_spike2-num_of_chans.bin\" attached to the name.\n"\
"\n"\
"OPTIONS\n"\
"-f to force over-writing existing output file.\n"\
"-t tag_text to add tag_text to the filename.\n"\
"List of chan numbers in the range of 1-128 to create a .bin with a subset of chans.\n"\
"Ranges of numbers are supported, e.g., 16-32.\n"\
"NOTE:  When you import the file in Spike2, you have to tell it the number of channels.\n"\
"\n"\
"Example use:   daq_to_bin -f 001 1-10 66-68 100 109 121\n",
name
);
}

static int parse_args(int argc, char *argv[])
{
   static struct option opts[] = { 
                                   {"r", required_argument, NULL, '1'},
                                   {"f", no_argument, NULL, '2'},
                                   {"d", no_argument, NULL, '3'},
                                   {"tag", required_argument, NULL, '4'},
                                   { 0,0,0,0} };
   int cmd;
   int ret = 0;
   opterr = 0;
   int sel_index;
   int sel_start = -1;
   int sel_end = -1;
   int rec = -1;
   char input[256];

   while ((cmd = getopt_long_only(argc, argv, "", opts, NULL )) != -1)
   {
      switch (cmd)
      {
         case '1':
               strncpy(RecNo,optarg,sizeof(RecNo)-1);
               sscanf(RecNo,"%d",&rec);
               if (rec > 0 && rec < 1000)  // 1-999
               {
                  sprintf(RecNo,"%03d",rec); // insure leading zeros
                  ret = true;
               }
               else
                  printf("%s is not a valid recording number\n",RecNo);
               break;

         case '2':
               OverWrite = true;
               break;

         case '3':
               Debug = true;
               break;

         case '4':
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

   if (!ret)
      usage(argv[0]); 

   return ret;
}


/* Find the daq files for the recording number in the current dir,
   then stick it/them in global name holder vars.
   Returns true of it finds one.
           false otherwise.
*/
static bool find_daq()
{
   DIR    *curr_dir;
   struct dirent *dir;
   bool   ret = false;
   
   curr_dir = opendir(".");
   if (curr_dir)
   {
           // find .daq file with current RecNo.  
           // (this can be fooled with sufficiently evil filename.)
       while ((dir = readdir(curr_dir)) != NULL)
       {
          if (strstr(dir->d_name,DAQ_EXT))
          {
             if (strstr(dir->d_name,RecNo))
             {
                if (strstr(dir->d_name,"1-64"))
                {
                   strncpy(Daq0,dir->d_name, sizeof(Daq0)-1);
                   ret = true;
                }
                else if (strstr(dir->d_name,"65-128"))
                {
                   strncpy(Daq1,dir->d_name, sizeof(Daq0)-1);
                   ret = true;
                }
                else   // assume old 1-64 daq file
                {
                   strncpy(Daq0,dir->d_name, sizeof(Daq0)-1);
                   ret = true;
                }
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
   Assumes Daq0 file exists.
*/

static bool create_bin()
{
   FILE *bin_fd;
   FILE *daq0_fd = NULL;
   FILE *daq1_fd = NULL;
   int chan;
   unsigned long long feedback = 0, count = 0;
   struct stat info;
   double percent;
   char input[256];
   unsigned short sample0[WORDS_PER_SAMP];
   unsigned short sample1[WORDS_PER_SAMP];
   size_t res;
   bool  done = false, dec_sel = false;;
   int  yr, mon, day;

   if (Daq0[0] != 0)
   {
      if ( (daq0_fd = fopen(Daq0,"r")) == NULL)
      {
         printf("Error opening %s, aborting. . .\n",Daq0);
         exit(1);
      }
   }

   if (Daq1[0] != 0)
   {
      if ( (daq1_fd = fopen(Daq1,"r")) == NULL)
      {
         printf("Error opening %s, aborting. . .\n",Daq1);
         exit(1);
      }
   }
   else  // if selected 65-128 chans but no .daq file, warn, but continue
   {
      for (chan = CHANS_PER_SAMP; chan < CHANS_PER_SAMP*2; chan++)
      {
         if (SelList[chan])   // 65-128
         {
            dec_sel = true;
            SelList[chan] = 0;
            --SelChans;
         }
      }
      if (dec_sel)
      {
         printf("Warning:  You selected channels from a 65-128 file\n");
         printf("          but there is no .daq file to read from.\n");
         printf("          Continuing for channels from 1-64 file.\n");
      }
   }

    // assume if here, at least a Daq0 file
   sscanf(Daq0,"%d-%d-%d", &yr,&mon,&day);
   if (UsrTag[0] !=0)
      sprintf(Bin,"%04d-%02d-%02d_%s_%s_%d_%s%s", yr,mon,day,RecNo,OutTag,SelChans,UsrTag,BIN_EXT);
   else
      sprintf(Bin,"%04d-%02d-%02d_%s_%s_%d%s", yr,mon,day,RecNo,OutTag,SelChans,BIN_EXT);

   printf("%d channels selected\n",SelChans);
   printf("Creating %s file for %s ",Bin, Daq0);
   if (Daq1[0])
      printf("\nand for %s",Daq1);
   printf("\n");


   fstat(fileno(daq0_fd),&info);
   percent = info.st_size/(BYTES_PER_SAMP);     // # samples in file

   if (percent == 0)
   {
      printf("Could not find any .daq files with data, aborting. . . \n");
      return false; 
   }

   if (!OverWrite && access(Bin,F_OK) == 0)
   {
      printf("WARNING:  The file %s already exists.\n  Okay to over-write (Y/N)?  ",Bin);
      fgets(input,sizeof(input),stdin);
      if (tolower(input[0]) != 'y')
      {
         printf("File is unchanged\n");
         goto error;
      }
   }

   bin_fd = fopen(Bin, "w");
   if (!bin_fd)
   {
      printf("Problem opening output file %s, aborting. . .\n",Bin);
      goto error;
   }

         // grab a sample. If the chan is selected in SelList
         // add it to the .bin file.
   while (!done)
   {
      res = fread(sample0,sizeof(sample0),1,daq0_fd);
      if (res == 0)
      {
         done = true;
         break;
      }
      if (daq1_fd)
      {
         res = fread(sample1,sizeof(sample1),1,daq1_fd);
         if (res == 0) // really shouldn't get here if files same size, but. . .
         {
            done = true;
            break;
         }
      }



            // 1-64
      for (chan = 0; chan < CHANS_PER_SAMP ; chan++)
      {
         if (SelList[chan])
         {
            short chanbuf = (int) sample0[chan+MARKER_LEN] - 32768;
            res = fwrite(&chanbuf,sizeof(chanbuf),1,bin_fd);
            if (res != 1)
            {
               printf("Error writing to output file %s\n",Bin);
               done = true;
               break;
            }
         }
      }
      // 65-128
      if (daq1_fd)
      {
         for (chan = 0; chan < CHANS_PER_SAMP ; chan++)
         {
            if (SelList[chan + CHANS_PER_SAMP])   // 64-127
            {
               short chanbuf = (int) sample1[chan+MARKER_LEN] - 32768;
               res = fwrite(&chanbuf,sizeof(chanbuf),1,bin_fd);
               if (res != 1)
               {
                  printf("Error writing to output file %s\n",Bin);
                  done = true;
                  break;
               }
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

   if (bin_fd)
   {
      printf("\r  %3.0f%%",(feedback/percent)*100.0);
      printf("\nCompleting write. . .\n"); // there can be a lot buffered data, 
      fflush(stdout);                      // closing can take many seconds, so reassure user
      fclose(bin_fd);
      printf("Creation of %s is complete.\n", Bin);
      fflush(stdout);
   }

error:
   if (daq0_fd)
      fclose(daq0_fd);
   if (daq1_fd)
      fclose(daq1_fd);

   return true;
}


int main (int argc, char **argv)
{
   int  complain;

   memset(SelList, true, sizeof(SelList)); // all chans by default

   if (!parse_args(argc, argv))
      exit(1);

   if (Debug)
   {
      printf("attach debugger, then press ENTER");
      getchar();
   }

   if (!find_daq())
   {
      printf("\nCould not find any .daq files for recording number %s, exiting. . .\n",RecNo);
      usage(argv[0]);
      exit(1);
   }

   complain = !create_bin();

   if (complain)
      usage(argv[0]);

  return 0;
}
