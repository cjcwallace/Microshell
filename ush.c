/* CS 352 -- Micro Shell!  
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

  int argc;
  char **args = arg_parse(line, &argc);

  /* Start a new process to do the job. */
  cpid = fork();
  if (cpid < 0)
  {
    /* Fork wasn't successful */
    //free(args); breaks
    perror("fork");
    return;
  }

  /* Check for who we are! */
  if (cpid == 0)
  {
    /* We are the child! */
    //execlp (line, line, (char *)0);
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
  if (argc == 0)
    return NULL;
  if (argc == 0 || quotec % 2 == 1)
  {
    return NULL;
  }
    

  printf("args=%d\n", argc);
  // Allocate memory
  char **argarr = (char **)malloc((argc + 1) * sizeof(char *));

  i = 0; // src
  int dest = 0; // dest
  int ac = 0;

  while (line[i] == ' ') i++; // skip lead spaces

  // Assign pointers and 0s
  while (line[i] != 0)
  {
    if (line[i] != ' ')  // start arg
    {
      argarr[ac++] = &line[i]; // assign pointer to nonspace character
      dest = i;
      while (line[i] != ' ') // loop until we hit a space
      {
        printf("arg:%s\n", argarr[ac - 1]);
        if (line[i] != '\"') // find start quote
        {
          line[dest++] = line[i];
        }
        i++;
      } // end while
      line[i] = 0; // assign pointer to end of arg
    }
    i++; // i is a space, increment
  }
  argarr[ac] = NULL;
  // Debug: print args in order
  
  for (int z = 0; z < argc; z++)
  {
    printf("arg[%d] = %c\n", z, *argarr[z]);
  }
  
  *argcptr = argc;
  return argarr;
}

char *removeQuotes(char *line, int n, int quotes, char **argarr)
{
  printf("n - quotes = %d\n", (n - quotes));
  int dest = 0; //write
  int src = 0;  //read
  while (dest < n)
  {
    if (line[src] != '\"')
    {
      line[dest++] = line[src];
    }
    if (src < n) src++;
  }
  /*
  for (int a = 0; a < n; a++) 
  {
    printf("%c", line[a]);
  }
  */
  printf("\n");
  return line;
}