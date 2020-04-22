/*  Cameron Wallace
 *  April 14, 2020
 *  CSCI 347 Spring 2020
 *  Assignment 2
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "defn.h"

   /* newsize default 1024 */
   /* orig: input string, scan only once */
   /* new: output string */
   /* return: expand un/sucessful */
   /* expand should not change original array */

int expand (char *orig, char *new, int newsize)
{
  int i = 0;
  int j = 0;
  char *rv;
  while ( orig[i] != 0 )
    {
      if ( orig[i] == '$' )
	{
	  //int replaceIndex = j;
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
		  //orig[i++] = '}'; /* Revert line to original */
		  //j = replaceIndex;
		  if ( writeNew( new, rv, &j, newsize ) == -1 )
		    {
		      return -1; // Write failed, presumably from overflow
		    }
		}
	      if ( orig[i] == 0 )
		{
		  new[j] = 0;
		  break;
		}
	      if ( orig[i] == '$') continue; /* 2 args next to eachother */
	    }
	  else if (orig[i] == '$' ) /* ppid */
	    {
	      char pidstring[21];
	      if ( snprintf(pidstring, 21, "%d", getpid()) < 1 )
		{
		  fprintf(stderr, "pid not found\n");
		  return -1;
		}
	      //printf("pid:%d, rv: %s\n",pid, rv);
	      //j = replaceIndex;
	      if ( writeNew( new, pidstring, &j, newsize ) == -1 )
		{
		  return -1;
		}
	      if ( orig[++i] == 0 )
		{
		  new[j] = 0;
		  break;
		}
	      if ( orig[i] == '$' ) continue; /* 2 args next to eachother */
	    }
	  else
	    {
	      i--;
	    }
	  //printf("j:%d, i:%d, new:%s\n",j,i,new);
	}
      //printf("j:%d, i:%d, new:%s\n",j,i,new);
      //printf("j: %d\n", j);
      if ( j >= newsize )
	{
	  fprintf(stderr, "buffer overflow\n");
	  return -1;
	}
      new[j++] = orig[i++];
    }
  //printf("orig:%s, new:%s, i:%d, j:%d\n", orig,new,i,j);
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
