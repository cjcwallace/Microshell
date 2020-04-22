/*   Cameron Wallace
 *   April 1, 2020
 *   CSCI 347 Spring 2020
 *   Assignment 2
 *
 *   CS 352 -- Micro Shell!  
 *
 *   Sept 21, 2000,  Phil Nelson
 *   Modified April 8, 2001 
 *   Modified January 6, 2003
 *   Modified January 8, 2017
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "defn.h"

/* Constants */

#define LINELEN 1024

/* Prototypes */

void processline(char *line);
char **arg_parse(char *line, int *argcptr);
char *removeQuotes(char *line, int n, int quotes, char **argarr);

/* Shell main */

int main(void)
{
  char buffer[LINELEN];
  int len;

  while (1)
  {

    /* prompt and get line */
    fprintf(stderr, "%% ");
    if (fgets(buffer, LINELEN, stdin) != buffer)
      break;

    /* Get rid of \n at end of buffer. */
    len = strlen(buffer);
    if (buffer[len - 1] == '\n')
      buffer[len - 1] = 0;

    /* Run it ... */
    processline(buffer);
  }

  if (!feof(stdin))
    perror("read");

  return 0; /* Also known as exit (0); */
}

void processline(char *line)
{
  pid_t cpid;
  int status;
  
  char newLine[LINELEN];
  //memset(newLine, 0, LINELEN);
  int success = expand(line, newLine, LINELEN);
  if( success == -1 )
    {
      memset(newLine, 0, LINELEN);
      return;
    }
  
  int argc;
  char **args = arg_parse(newLine, &argc);

  if (args == NULL) return;

  int bi = builtIn(args, &argc);
  if ( bi != 0 )
    {
      /* Start a new process to do the job. */
      cpid = fork();
      if (cpid < 0 )
	{
	  /* Fork wasn't successful */
	  perror("fork");
	  return;
	}
      
      /* Check for who we are! */
      if (cpid == 0)
	{
	  /* We are the child! */
	  execvp(*args, args);
	   /* execlp reurned, wasn't successful */
	  perror("exec");
	  fclose(stdin); // avoid a linux stdio bug
	  exit(127);
	}
      /* Have the parent wait for child to complete */
      if (wait(&status) < 0)
	{
	  free(args);
	  /* Wait wasn't successful */
	  perror("wait");
	}
    }
  free(args);
  //newLine[0] = '0';
  //printf("newline:%s\n", newLine);
  memset(newLine, 0, LINELEN);
}

char **arg_parse(char *line, int *argcptr)
{
  /* line points to character array containing command to be processed */
  /* argcptr points to int variable representing number of args in line */
  /* return value: pointer to malloced area that points into line parameter  */
  /* call malloc(3) only once */

  int argc = 0;
  int i = 0;
  int inquote = 0;
  int quotec = 0;

  // Count args & quotes
  while (line[i] != 0)
  {
    while (line[i] == ' ') // skip spaces
      i++;
    if (line[i] != ' ' && line[i] != 0) // start arg
    {
      argc++;
      while (line[i] != ' ' && inquote == 0) // find end of arg
      {
        if (line[i] == 0)
          break;
        if (line[i] == '\"') // find start quote
        {
          quotec++;
          inquote = 1;
          i++;
          while (inquote == 1)
          {
            if (line[i] == 0)
            {
              fprintf(stderr,"Odd number of quotes.\n");
              return NULL; 
            }
            if (line[i] == '\"') // found end quote
            {
              quotec++;
              inquote = 0;
            }
            i++;
          }
        }
        if (line[i] == ' ')
          break;
        i++;
      }
    }
  }
  // Allocate memory
  char **argarr = (char **)malloc((argc + 1) * sizeof(char *));

  if (argc == 0)
    return NULL;
  if (quotec % 2 == 1)
  {
    fprintf(stderr,"Odd number of quotes.\n");
    return NULL;
  }

  i = 0; // src
  int dest = 0; // dest
  int ac = 0;

  const int len = strlen(line);

  while (line[i] == ' ') i++; // skip lead spaces
  // Assign pointers and 0s
  while (line[i] != 0 && i < len)
  {
    if (line[i] != ' ')  // start arg
    {
      argarr[ac++] = &line[dest]; // assign pointer to start of arg
      while (line[i] != ' ') // loop until we hit a space
      {
        if (line[i] == '\"') // find start quote
        {
          i++; // get i off quote
          while (line[i] != 0 && line[i] != '\"') // loop until eos or quote
          {
            if (line[i] != '\"') // replace characters that aren't "
            {
              line[dest++] = line[i];
            }
            i++;
          }
          line[dest] = line[i++];
          continue;
        }
        line[dest++] = line[i++];
      }
      line[dest++] = 0; // assign pointer to end of arg
    }
    i++; // i is a space, increment
  }
  argarr[ac] = NULL;
  
  *argcptr = argc;
  return argarr;
}
