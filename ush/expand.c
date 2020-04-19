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
	  int replaceIndex = i;
	  j = i + 1;
	  int a = 0;
	  if ( orig[j] == '{' ) /* Start of environment name */ 
	    {
	      int envIndex = j + 1;
	      while ( orig[j] != '}' ) /* get variable name */
		{
		  if ( orig[j] == 0)
		    {
		      fprintf(stderr, "No closing } found.\n");
		      return -1;
		    }
		  j++;
		}
	      i = j + 1;
	      orig[j] = 0; /* Temp replace } with 0 */
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
	      //printf("rv:%s\n", rv);
	      orig[j] = '}'; /* Revert line to original */
	      j = replaceIndex;
	      while ( rv[a] != 0 ) /* copy value to new string */
		{
		  new[j++] = rv[a++];
		  //printf("j:%d, writing rv new:%s\n", j, new);
		}
	      if ( orig[i+1] == 0 ) break;
    	    }
	  if (orig[j] == '$' ) /* ppid */
	    {
	      i = j + 1;
	      int pid = getppid();
	      //rv = malloc(6);
	      sprintf(rv, "%d", pid);
	      //printf("pid:%d, rv: %s\n",pid, rv);
	      j = replaceIndex;
	      while ( rv[a] != 0 )
		{
		  new[j++] = rv[a++];
		  //printf("j:%d writing rv new:%s\n", j, new);
		}
	    }
	}
      //printf("j: %d\n", j);
      new[j++] = orig[i++];
    }
  //printf("orig:%s\nnew:%s\n",orig,new);
  return 1;
}
