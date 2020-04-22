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
#include "defn.h"

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

void bi_envset( char **args, int *argc )
{
  if ( *argc != 3 )
    {
      fprintf(stderr, "usage: envset name value\n");
      //return -1;
    }
  if ( setenv(args[1], args[2], 1) != 0 )
    {
      perror("envset");
      return;
    }
  if ( setenv(args[1], args[2], 1) == 0 )
    {
      setenv(args[1], args[2], 1);
    }
}

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
  if ( unsetenv(args[1]) == 0 )
    {
      unsetenv(args[1]);
    }
}

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
      chdir(getenv("HOME"));
    }
  if ( *argc == 2 )
    {
      if ( chdir(args[1]) != 0 )
	{
	  perror("cd");
	  return;
	}
      chdir(args[1]);
    }
}

typedef void (*bicommands) ();
bicommands cmd[] = { &bi_exit, &bi_envset, &bi_envunset, &bi_cd };

int builtIn (char **args, int *argc)
{
  const int size = 4;
  const char *commands[] = { "exit", "envset", "envunset", "cd" };
    
  for ( int i = 0; i < size; i++ )
    {
      if ( strcmp(args[0], commands[i]) == 0 )
	{
	  //printf("command %s found\n", commands[i]);
	  cmd[i]( args, argc);
	  return 0;
	}
    }
  return -1;
}
