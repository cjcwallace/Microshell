/*  Cameron Wallace
 *  April 14, 2020
 *  CSCI 347 Spring 2020
 *  Assignment 2
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "defn.h"
#include "globals.h"

   /* newsize default 1024 */
   /* orig: input string, scan only once */
   /* new: output string */
   /* return: expand un/sucessful */
   /* expand should not change original array */

int expand (char *orig, char *new, int newsize)
{
  int i = 0; /* orig pointer */
  int j = 0; /* new pointer */
  char *rv;  /* holds variables from ${name} */
  while ( orig[i] != 0 )
    {
      if ( orig[i] == '$' )
	{
	  i++;
	  if ( orig[i] == '{' ) /* Start of environment name */ 
	    {
	      int envIndex = i + 1;
	      while ( orig[i] != '}' ) /* get variable name */
		{
		  if ( orig[i] == 0)
		    {
		      fprintf(stderr, "ush: no matching }\n");
		      return -1;
		    }
		  i++;
		}
	      orig[i] = 0; /* Temp replace } with 0 */
	      rv = getenv( &orig[envIndex] );  /* return env value */
	      orig[i++] = '}'; /* Revert line to original */
	      if ( rv != 0 )
		{
		  if ( (strlen(rv) + strlen(orig) ) >= newsize )
		    {
		      fprintf(stderr, "out of bounds error\n");
		      return -1;
		    }
		  if ( writeNew( new, rv, &j, newsize ) == -1 )
		    {
		      return -1; /* Write failed, presumably from overflow */
		    }
		}
	      if ( orig[i] == 0 )
		{
		  new[j] = 0;
		  break;
		}
	      if ( orig[i] == '$') continue; /* 2 args next to eachother */
	    }
	  else if ( orig[i] == '$' ) /* ppid */
	    {
	      char pidstring[21];
	      if ( snprintf(pidstring, 21, "%d", getpid()) < 1 )
		{
		  fprintf(stderr, "pid not found\n");
		  return -1;
		}
	      if ( writeNew( new, pidstring, &j, newsize ) == -1 )
		{
		  return -1; /* Write failed */
		}
	      if ( orig[++i] == 0 )
		{
		  new[j] = 0;
		  break;
		}
	      if ( orig[i] == '$' ) continue; /* 2 args next to eachother */
	    }
	  else if ( orig[i] == '#' )
	    {
	      char numArgs[21];
	      if ( sprintf(numArgs, "%d", (gargc - gshift)) < 1)
		{
		  fprintf(stderr, "error finding number of args");
		  return -1;
		}
	      if ( writeNew( new, numArgs, &j, newsize ) == -1 )
		{
		  return -1;
		}
	      if ( orig[++i] == 0 )
		{
		  new[j] = 0;
		  break;
		}
	      if ( orig[i] == '$' ) continue;
	    }
	  else if ( isdigit(orig[i]) ) /* get arg  */
	    {
	      /*
	      if ( (gshift + orig[i]) > gargc || (gshift + orig[i]) < 0 )
		{
		  if ( writeNew( new, " ", &j, newsize ) == -1 )
		    {
		      return -1;
		    }
		}
	      */
	      int start = i;
	      while ( isdigit(orig[i]) ) 
		{
		  printf("orig[i] = %c\n", orig[i]);
		  i++;
		}
	      char tmp = orig[i];
	      orig[i] = 0;
	      printf("orig[start]: %d\n", orig[start]);
	      int num = atoi(orig[start]);
	      orig[i] = tmp;
	      printf("gshift = %d, num = %d\n", gshift, num);
	      rv = gargv[gshift + num];
	      printf("i = %d, gargv[i] = %s\n", i, gargv[num]);
	      if ( rv != NULL )
		{
		  if ( writeNew( new, rv, &j, newsize ) == -1 )
		    {
		      return -1;
		    }
		}
	      if ( orig[++i] == 0 )
		{
		  new[j] = 0;
		  break;
		}
	      if ( orig[i] == '$' ) continue;
	    }
	  else
	    {
	      i--;
	    }
	}
      if ( j >= newsize ) /* Check to ensure we still have space */
	{
	  fprintf(stderr, "buffer overflow\n");
	  return -1;
	}
      new[j++] = orig[i++];
    }
  return 0;
}

int writeNew (char *new, char *rv, int *j, int newsize)
{
  int a = 0;
  while ( rv[a] != 0 ) /* copy value to new string */
    {
      if ( *j > newsize )
	{
	  fprintf(stderr, "buffer overflow\n");
	  return -1;
	}
      new[*j] = rv[a++];
      *j = *j + 1;
    }
  return 0;
}
