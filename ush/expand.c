/*  Cameron Wallace
 *  April 14, 2020
 *  CSCI 347 Spring 2020
 *  Assignment 2
 */

#include <stdio.h>
#include "defn.h"

   /* newsize default 1024 */
   /* orig: input string, scan only once */
   /* new: output string */
   /* return: expand un/sucessful */
   /* expand should not change original array */

int expand (char *orig, char *new, int newsize)
{
  int i = 1;
  char envName[256];
  while ( orig[i] != 0 )
    {
      if ( orig[i] == '$' )
	{
	  i+=2;   /* Skip $ and { */
	  int j = 0;
	  while ( orig[i] != '}' ) /* get variable name */
	    {
	      if ( orig[i] == 0)
		{
		  fprintf(stderr, "No closing } found.");
		  return NULL;
		}
	      envName[j++] = orig[i++];
	    }
	  j = 0;
	  char varVal = getenv( envName );  /* return env value */
	  if ( strlen(varVal) > newsize )
	    {
	      fprintf(stderr, "Out of bounds error.");
	      return NULL;
	    }
	  while ( envName[j] != 0 ) /* copy value to new string */
	    {
	      new[i++] = envName[j++];
	    }
	}
      new[i] = orig[i];
      i++;
    }
  return 1;
}
