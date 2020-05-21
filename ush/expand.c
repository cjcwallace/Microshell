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
#include <dirent.h>
#include <sys/wait.h>
#include "defn.h"
#include "globals.h"
#define WAIT 1

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
  char *numarg; /* holds value of arg */
  int plrv; /* holds value returned by process line */

  if (hadsigint == 1)
    {
      return 0;
    }
  while ( orig[i] != 0 && hadsigint == 0)
    {
      if ( orig[i] == '$' )
	{
	  i++;
	  if ( orig[i] == '{' ) /* Start of environment name */ 
	    {
	      int envIndex = i + 1;
	      while ( orig[i] != '}' ) /* get variable name */
		{
		  if ( orig[i] == 0 )
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
	  else if ( orig[i] == '(' ) /* command expansion */
	    {
	      int envIndex = i + 1;
	      int paren = 1;
	      /* fd[0]: read, fd[1]: write */
	      int fd[2];
	      i++;

	      while ( paren != 0 ) /* get content of parens */
		{
		  if ( orig[i] == 0 )
		    {
		      fprintf(stderr, "ush: expected closing )\n");
		      return -1;
		    }
		  if ( orig[i] == '(' ) paren++;
		  if ( orig[i] == ')' ) paren--;
		  i++;
		}
	      orig[i - 1] = 0; /* Temp replace closing ) with 0 */
	      if ( pipe(fd) < 0 )
		{
		  perror("pipe");
		  return -1;
		}
	      plrv = processline( &orig[envIndex], 0, fd[1], 2);
	      close(fd[1]);
	      if ( plrv < 0 )
		{
		  fprintf(stderr, "plrv:%d, processline error.\n", plrv);
		  return -1;
		}

	      /* Create a buffer that read() will fill, which is then passed
	       * to new. */
	      
	      char readbuf[newsize];
	      int readline = read(fd[0], readbuf, newsize);
	      int readcount = 0;
	      int nl;
	      while (readline > 0)
		{
		  readcount++;
		  nl = 0;
		  if ( readbuf[readline - 1] == '\n' )
		    {
		      readbuf[readline - 1] = ' ';
		      nl = 1;
		    }
		  if ( writeNew( new, readbuf, &j, newsize) == -1 )
		    {
		      return -1;
		    }
		  readline = read(fd[0], readbuf, newsize);
		  if ( readline < 0 ) 
		    {
		      perror("read");
		      return -1;
		    }
		  if ( readcount > 39 )
		    {
		      fprintf(stderr, "exceeded read limit\n");
		      return -1;
		    }
		  //fflush(stdout);
		}
	      close(fd[0]);
	      close(fd[1]);
	      /* wait child process from pl */
	      if ( plrv > 0 )
		{
		  zombie();
		  //sighelper();
		}
	      orig[i - 1] = ')';
	      if ( nl ) j--;
	      if ( orig[i] == 0 )
		{
		  new[j] = 0;
		  break;
		}
	      if ( orig[i] == '$' ) continue;
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
	  else if ( orig[i] == '?' )
	    {
	      char exitvalue[21];
	      if ( snprintf(exitvalue, 21, "%d", exitv) < 1 )
		{
		  fprintf(stderr, "writing exit value failed\n");
		  return -1;
		}
	      if ( writeNew( new, exitvalue, &j, newsize ) == -1 )
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
	  else if ( orig[i] == '#' ) /* get number of args */
	    {
	      char numArgs[21];
	      if ( sprintf(numArgs, "%d", (gargc - gshift)) < 1 )
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
	      char numc[8];
	      memset(&numc[0], 0, sizeof(numc));
	      while ( isdigit(orig[i]) ) 
		{
		  strncat(numc, &orig[i], 1);
		  i++;
		}
	      int num = atoi(numc);
	      if ( (gshift + num) <= gargc )
		{
		  numarg = gargv[gshift + num];
		}
	      if ( (gshift + num) > gargc )
		{
		  numarg = " ";
		}	      
	      if ( numarg != NULL )
		{
		  if ( writeNew( new, numarg, &j, newsize ) == -1 )
		    {
		      return -1;
		    }
		}
	      memset(&numc[0], 0, 8);
	      if ( orig[i] == 0 )
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
      if ( orig[i] == '\\' )
	{
	  if ( orig[i + 1] == '*' )
	    {
	      i++;
	    }
	}
      if ( orig[i] == '*' )
	{
	  DIR *d;
	  struct dirent *dir;
	  d = opendir(".");
	  if (!d)
	    {
	      fprintf(stderr, "err: can't open directory\n");
	      return -1;
	    }
	  /* print all files */
	  if ( (orig[i - 1] == ' ' || orig[i - 1] == '"')
	       && (orig[i + 1] == ' ' || orig[i +1] == '"' || orig[i + 1] == 0) )
	    {
	      while ( (dir = readdir(d)) != NULL )
		{
		  char *fname = dir->d_name;
		  if ( strcmp(fname, ".") != 0 && strcmp(fname, "..") != 0 )
		    {
		      if ( writeNew( new, fname, &j, newsize ) == -1
			   || writeNew( new, " ", &j, newsize ) == - 1 )
			{
			  fprintf(stderr, "error writing all files\n");
			  return -1;
			}
		    }
		}
	      j--;
	      i = i + 1;
	    }
	  /* context */
	  if ( (orig[i - 1] == ' ' || orig[i - 1] == '"')
	       && orig[i + 1] != ' ' && orig[i + 1] != '"')
	    {
	      int sufIndex = i + 1;
	      while ( orig[i] != ' ' && orig[i] != '"' && orig[i] != '\0' )
		{
		  i++;
		}
	      char tmp = orig[i];
	      orig[i] = 0; /* Temp replace with EOS */
	      char *suf = &orig[sufIndex];
	      if ( strstr(suf, "/") != NULL )
		{
		  fprintf(stderr, "context cannot contain \'/\'\n");
		  return -1;
		}
	      int suflen = strlen(suf);
	      int found = 0;
	      while ( (dir = readdir(d)) != NULL )
		{
		  char *fname = dir->d_name;
		  int in = strlen(fname) - suflen;
		  if ( strcmp(&fname[in], suf) == 0 )
		    {
		      found = 1;
		      if ( writeNew( new, fname, &j, newsize ) == -1
			   || writeNew( new, " ", &j, newsize ) == -1 )
			{
			  fprintf(stderr, "error with d_name\n");
			  return -1;
			}
		    }
		}
	      if ( found == 0 ) /* File was not found in dir */
		{
		  if ( writeNew( new, &orig[sufIndex -1], &j, newsize ) == -1
		       || writeNew( new, " ", &j, newsize ) == -1 )
		    {
		      fprintf(stderr, "error with writing\n");
		      return -1;
		    }
		}
	      orig[i] = tmp;
	    }
	  closedir(d);
	}
      if ( j > newsize ) /* Check to ensure we still have space */
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
	  fprintf(stderr, "2buffer overflow\n");
	  return -1;
	}
      /*
      if ( rv[a] == '\n' )
	{
	  rv[a] = ' ';
	}
      */
      new[*j] = rv[a++];
      *j = *j + 1;
    }
  return 0;
}
