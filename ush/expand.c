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
      printf("orig[%d]:%c\n",i,orig[i]);
      if ( orig[i] == '$' )
	{
	  int envIndex = i;
	  int j = i + 1;
	  if ( orig[j] == '{' ) /* Start of environment name */ 
	    {
	      while ( orig[j] != '}' ) /* get variable name */
		{
		  if ( orig[j] == 0)
		    {
		      fprintf(stderr, "No closing } found.\n");
		      return -1;
		    }
		  j++;
		}
	    }
	  if (orig[j] == '$' ) /* ppid */
	    {
	      rv = getppid();
	    }
	  orig[j] = 0; /* Temp replace } with 0 */
	  rv = getenv( &orig[j] );  /* return env value */
	  if ( rv == 0 ) /* check if rv is NULL */
	    {
	      break;
	    }
	  if ( (strlen(rv) + strlen(orig) ) > newsize )
	    {
	      fprintf(stderr, "Out of bounds error.\n");
	      return -1;
	    }
	  printf("rv:%s\n", rv);
	  orig[j] = '}'; /* Revert line to original */
	  int a = 0;
	  j = envIndex;
	  while ( rv[a] != 0 ) /* copy value to new string */
	    {
	      new[j++] = rv[a++];
	    }
	}
      new[j++] = orig[i++];
    }
  printf("orig:%s\nnew:%s\n",orig,new);
  return 1;
}
