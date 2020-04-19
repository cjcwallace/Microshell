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
      //printf("orig[%d]:%s\n",i,orig);
      //printf("new[%d]:%s\n",j,new);
      if ( orig[i] == '$' )
	{
	  int replaceIndex = j;
	  //printf("replacein:%d\n",replaceIndex);
	  i++;
	  int a = 0;
	  if ( orig[i] == '{' ) /* Start of environment name */ 
	    {
	      int envIndex = i + 1;
	      while ( orig[i] != '}' ) /* get variable name */
		{
		  if ( orig[i] == 0)
		    {
		      fprintf(stderr, "No closing } found.\n");
		      return -1;
		    }
		  i++;
		}
	      orig[i] = 0; /* Temp replace } with 0 */
	      rv = getenv( &orig[envIndex] );  /* return env value */
	      if ( rv == 0 ) /* check if rv is NULL */
                {
		  printf("rv is null.\n");
		  break;          
                }
	      if ( (strlen(rv) + strlen(orig) ) > newsize )
    	        {
		  fprintf(stderr, "Out of bounds error.\n");
		  return -1;
    	        }
	      orig[i++] = '}'; /* Revert line to original */
	      j = replaceIndex;
	      writeNew( new, rv, &j );
	      /*
	      while ( rv[a] != 0 ) /* copy value to new string 
		{
		  new[j++] = rv[a++];
		  //printf("j:%d, writing rv new:%s\n", j, new);
		}
	      */
	      if ( orig[i+1] == 0 ) break;
    	    }
	  if (orig[i] == '$' ) /* ppid */
	    {
	      int pid = getppid();
	      rv = (char*)  malloc(6);
	      sprintf(rv, "%d", pid);
	      //printf("pid:%d, rv: %s\n",pid, rv);
	      j = replaceIndex;
	      writeNew( new, rv, &j );
	      /*
	      while ( rv[a] != 0 )
		{
		  new[j++] = rv[a++];
		  //printf("j:%d writing rv new:%s\n", j, new);
		}
	      */
	      if ( orig[++i] == 0 ) break;
	    }
	  //printf("j:%d, i:%d, new:%s\n",j,i,new);
	}
      //printf("j: %d\n", j);
      new[j++] = orig[i++];
    }
  return 1;
}

int writeNew (char *new, char *rv, int *j)
{
  int a = 0;
  while ( rv[a] != 0 )
    {
      new[*j] = rv[a++];
      *j = *j + 1;
      printf("j:%d\n", *j);
    }
  return 1;
}
