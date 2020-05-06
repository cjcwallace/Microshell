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
#include "defn.h"
#include "globals.h"

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
      gshift += 1;
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
      if ( unshiftval > (gargc - gshift) )
	{
	  fprintf(stderr, "err: unshift out of range\n");
	  return;
	}
      //int unshiftval = atoi(args[1]);
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
      while ( i < argc )
	{
	  struct stat st;
	  char *fName = args[i];
	  if ( stat(fName, &st) != 0 && fName != '*')
	    {
	      printf("%s: No such file or directory\n");
	      continue;
	    }
	  if ( args[i] == '*' ) /* stat all files in dir */
	    {
	      
	    }
	  else /* stat given file */
	    {
	      char *uName = getUser(&st, uName);
	      char *gName = getGroup(&st, gName);
	      off_t fSize = (int) st.st_size;
	      nlink_t fLinks = (int) st.st_nlink;
	      printf("%s %s %d %d", uName, gName, fSize, fLinks);
	    }
	}
    }
}

void getUser( struct stat st, char *uName )
{
  if ( struct passwd *pw = getpwuid(&st.st_uid) != 0 )
    {
      &uName = pw->pw_name;
    }
}

void getGroup( struct stat st, char *gName )
{
  if ( struct group *gr = getgrid(&st.st_gid) != 0 )
    {
      &gName = gr->gr_name;
    }
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
