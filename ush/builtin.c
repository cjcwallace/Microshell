
/*  Cameron Wallace
 *  April 14, 2020
 *  CSCI 347 Spring
 *  Assignment 2
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <time.h>
#include "defn.h"
#include "globals.h"

/* helper protos */
char * getUser( struct stat st );
char * getGroup( struct stat st );

/* Exit */
void bi_exit( char **args, int *argc )
{
  if ( *argc == 1 )
    {
      exitv = 0;
      exit(0);
    }
  if ( *argc > 1 )
    {
      exitv = atoi(args[1]);
      exit(atoi(args[1]));
    }
}

/* Set environment variables */
void bi_envset( char **args, int *argc )
{
  if ( *argc != 3 )
    {
      fprintf(stderr, "usage: envset name value\n");
      exitv = 1;
      return;
    }
  if ( setenv(args[1], args[2], 1) != 0 )
    {
      perror("envset");
      exitv = 1;
      return;
    }
  exitv = 0;
}

/* Unset environment variables */
void bi_envunset( char **args, int *argc )
{
  if ( *argc != 2 )
    {
      fprintf(stderr, "usage: envunset name\n");
      exitv = 1;
      return;
    }
  if ( unsetenv(args[1]) != 0 )
    {
      perror("unsetenv");
      exitv = 1;
      return;
    }
  exitv = 0;
}

/* cd Functionality */
void bi_cd(char **args, int *argc )
{
  if ( *argc > 2 )
    {
      fprintf(stderr, "useage: cd [directory]\n");
      exitv = 1;
      return;
    }
  if ( *argc == 1 )
    {
      if ( chdir(getenv("HOME")) != 0)
	{	 
	  perror("cd");
	  exitv = 1;
	  return;
	}
    }
  if ( *argc == 2 )
    {
      if ( chdir(args[1]) != 0 )
	{
	  perror("cd");
	  exitv = 1;
	  return;
	}
    }
  exitv = 0;
}

/* shift Functionality */
void bi_shift( char **args, int *argc )
{
  if ( *argc < 2 )
    {
      if ( (gshift + 1)  >= gargc )
	{
	  fprintf(stderr, "err: shift out of range\n");
	  exitv = 1;
	  return;
	}
      gshift += 1;
    }
  if ( *argc == 2 )
    {
      int shiftval = atoi(args[1]);
      if ( (shiftval + gshift)  >= gargc )
	{
	  fprintf(stderr, "err: shift out of range\n");
	  exitv = 1;
	  return;
	}
      gshift = gshift + shiftval;
    }
  exitv = 0;
}

/* unshift Functionality */
void bi_unshift( char **args, int *argc )
{
  if ( *argc < 2 )
    {
      gshift = 1;
    }
  if ( *argc == 2 )
    {
      int unshiftval = atoi(args[1]);
      if ( unshiftval >= gshift )
	{
	  fprintf(stderr, "err: unshift out of range\n");
	  exitv = 1;
	  return;
	};
      gshift = gshift - unshiftval;
    }
  exitv = 0;
}

/* sstat Functionality */
void bi_sstat( char **args, int *argc, int outfd )
{
  if ( *argc == 1 )
    {
      fprintf(stderr, "stat: no files to stat\n");
      exitv = 1;
      return;
    }
  if ( *argc >= 2 )
    {
      int i = 1;
      while ( i < *argc )
	{
	  char *fName = args[i];
	  struct stat st;
	  if ( stat(fName, &st) != 0 )
	    {
	      printf("%s: No such file or directory\n", fName);
	      exitv = 1;
	      i++;
	      continue;
	    }
	  else /* stat given file */
	    {
	      char *uName = getUser(st);
	      char *gName = getGroup(st);
	      mode_t fMode = st.st_mode;
	      char fPermissions[11];
	      strmode(fMode, fPermissions);
	      nlink_t fLinks = (int) st.st_nlink;
	      off_t fSize = (int) st.st_size;
	      char *fTime = asctime(localtime(&st.st_mtime));
	      dprintf(outfd,"%s %s %s %s%ld %ld %s",
		      fName, uName, gName, fPermissions, fLinks, fSize, fTime);
	      //fflush(stdout);
	      i++;
	    }
	}
    }
  if ( exitv != 1 ) exitv = 0;
}

/* sstat helpers */
char * getUser( struct stat st )
{
  struct passwd *pw = getpwuid(st.st_uid);
  if (pw != 0 )
    {
      return pw->pw_name;
    }
  char *fuid = malloc(11 * sizeof(char));
  sprintf(fuid, "%d", st.st_uid);
  return fuid;
}

char * getGroup( struct stat st )
{
  struct group *gr = getgrgid(st.st_gid);
  if ( gr != 0 )
    {
      return gr->gr_name;
    }
  char *fgid = malloc(11 * sizeof(char));
  sprintf(fgid, "%d", st.st_gid);
  return fgid;
}

typedef void (*bicommands) ();
/* Store functions for built in commands */
bicommands cmd[] =
  { &bi_exit, &bi_envset, &bi_envunset, &bi_cd,
    &bi_shift, &bi_unshift, &bi_sstat };

int builtIn (char **args, int *argc, int outfd)
{
  const int size = 7;
  const char *commands[] =
    { "exit", "envset", "envunset", "cd", "shift", "unshift", "sstat" };

  if ( hadsigint == 1 )
    {
      exitv = EINTR;
      return exitv;
    }
  
  for ( int i = 0; i < size; i++ )
    {
      if ( strcmp(args[0], commands[i]) == 0 )
	{
	  if ( strcmp(args[0], "sstat") == 0 )
	    {
	      cmd[i]( args, argc, outfd );
	      return 0;
	    }
	  cmd[i]( args, argc);
	  return 0;
	}
    }
  return -1;
}
