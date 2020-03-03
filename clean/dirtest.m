#!/usr/bin/octave -qf

destdir="mkdirtest";

# make destdir if we need to, and give all users all permissions
[ex]= isdir(destdir)
if (!ex)
sleep(1);
   [oldmask]=umask(0);
   [status,msg,msgid]=mkdir(destdir)
   printf("\n");
   fflush(stdout);
   umask(oldmask);
   [ex2]= isdir(destdir)
   if (status == 0)
      printf("\nCreate directory %s failed, status is %d msg is %s",destdir,status,msg);
      fflush(stdout);
   endif
endif

printf("Done\n");
fflush(stdout);

