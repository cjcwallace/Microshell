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
      exit(0);
    }
  if ( *argc > 1 )
    {
      exit(atoi(args[1]));
    }
}

/* Set environment variables */
void bi_envset( char **args, int *argc )
{
  if ( *argc != 3 )
    {
      fprintf(stderr, "usage: envset name value\n");
      return;
    }
  if ( setenv(args[1], args[2], 1) != 0 )
    {
      perror("envset");
      return;
    }
}

/* Unset environment variables */
void bi_envunset( char **args, int *argc )
{
  if ( *argc != 2 )
    {
      fprintf(stderr, "usage: envunset name\n");
    }
  if ( unsetenv(args[1]) != 0 )
    {
      perror("unsetenv");
      return;
    }
}

/* cd Functionality */
void bi_cd(char **args, int *argc )
{
  if ( *argc > 2 )
    {
      fprintf(stderr, "useage: cd [directory]\n");
    }
  if ( *argc == 1 )
    {
      if ( chdir(getenv("HOME")) != 0)
	{	 
	  perror("cd");
	  return;
	}
    }
  if ( *argc == 2 )
    {
      if ( chdir(args[1]) != 0 )
	{
	  perror("cd");
	  return;
	}
    }
}

/* shift Functionality */
void bi_shift( char **args, int *argc )
{
  if ( *argc < 2 )
    {
      if ( gshift + 1 <= gargc )
	{
	  gshift += 1;
	}
    }
  if ( *argc == 2 )
    {
      int shiftval = atoi(args[1]);
      if ( shiftval > (gargc + gshift) )
	{
	  fprintf(stderr, "err: shift out of range\n");
	  return;
	}
      //int shiftval = atoi(args[1]);
      gshift = gshift + shiftval;
    }
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
      //printf("unshiftval:%d, gshift:%d, gargc:%d \n", unshiftval, gshift, gargc);
      if ( unshiftval > gshift )
	{
	  fprintf(stderr, "err: unshift out of range\n");
	  return;
	};
      gshift = gshift - unshiftval;
    }
}

/* sstat Functionality */
void bi_sstat( char **args, int *argc )
{
  if ( *argc == 1 )
    {
      fprintf(stderr, "stat: no files to stat\n");
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
	      off_t fSize = (int) st.st_size;
	      nlink_t fLinks = (int) st.st_nlink;
	      char *fTime = asctime(localtime(&st.st_ctime));
	      fprintf(stdout,"%s %s %s %s %ld %ld %s",
		     fName, uName, gName, fPermissions, fSize, fLinks, fTime);
	      fflush(stdout);
	      i++;
	    }
	}
    }
}

/* sstat helpers */
char * getUser( struct stat st )
{
  struct passwd *pw = getpwuid(st.st_uid);
  if (pw != 0 )
    {
      return pw->pw_name;
    }
  return NULL;
}

char * getGroup( struct stat st )
{
  struct group *gr = getgrgid(st.st_gid);
  if ( gr != 0 )
    {
      return gr->gr_name;
    }
  return NULL;
}

typedef void (*bicommands) ();
/* Store functions for built in commands */
bicommands cmd[] =
  { &bi_exit, &bi_envset, &bi_envunset, &bi_cd, &bi_shift, &bi_unshift, &bi_sstat };

int builtIn (char **args, int *argc)
{
  const int size = 7;
  const char *commands[] =
    { "exit", "envset", "envunset", "cd", "shift", "unshift", "sstat" };
    
  for ( int i = 0; i < size; i++ )
    {
      if ( strcmp(args[0], commands[i]) == 0 )
	{
	  cmd[i]( args, argc);
	  return 0;
	}
    }
  return -1;
}
