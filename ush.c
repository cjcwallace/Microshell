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

  const int linelen = strlen(line);

  int argc = 0;
  int i = 0;
  int inquote = 0;
  int quotec = 0;

  // Count args & quotes
  while (line[i] != 0)
  {
    //printf("count = %d, i:%d, char:%c\n", argc, i, line[i]);
    // skip spaces
    while (line[i] == ' ')
      i++;
    // start arg
    if (line[i] != ' ' && line[i] != 0)
    {
      argc++;
      //i++;
      // find end of arg
      while (line[i] != ' ' && inquote == 0)
      {
        if (line[i] == 0)
          break;
        // find start quote
        if (line[i] == '\"')
        {
          quotec++;
          inquote = 1;
          i++;
          while (inquote == 1)
          {
            // found end quote
            if (line[i] == '\"')
            {
              quotec++;
              inquote = 0;
            }
            i++;
          }
        }
        if (line[i] == ' ')
          break;
        //printf("i:%d, char:%c\n", i, line[i]);
        i++;
      }
    }
  }
  if (argc == 0)
    return NULL;
  if (argc == 0 || quotec % 2 == 1)
    return NULL;

  // Allocate memory
  char **argarr = (char **)malloc((argc + 1) * sizeof(char *));

  i = 0;
  int ac = 0;
  int currQ = 0;
  // Assign pointers and 0s
  while (line[i] != 0)
  {
    // skip spaces
    while (line[i] == ' ')
      i++;
    // start arg
    if (line[i] != ' ' && line[i] != 0)
    {
      //argarr[ac++] = &line[i++];
      argarr[ac] = &line[i - currQ];
      ac++; i++;
      // find end of arg
      while (line[i] != ' ' && inquote == 0)
      {
        if (line[i] == 0)
          break;
        // find start quote
        if (line[i] == '\"')
        {
          inquote = 1;
          currQ++;
          i++;
          while (inquote == 1)
          {
            // found end quote
            if (line[i] == '\"')
            {
              inquote = 0;
              currQ++;
            }
            i++;
          }
        }
        i++;
      }
      line[i++] = 0;
      //i++;
    }
  }
  argarr[ac] = NULL;
  if (quotec > 0)
  {
    line = removeQuotes(line, linelen, quotec, argarr);
    //line = removeQuote(line, (linelen - quotec), argarr);
  }
  
  /* // Debug: print line[:] from given pointers
  int b = 0;
  while (argarr[b] != NULL || b == 0) {
    printf("argarr[%d] = %s\n", b, argarr[b]);
    b++;
  }
  printf("zing\n");
  */
  // Debug: print args in order
  
  for (int z = 0; z < argc; z++)
  {
    printf("arg[%d] = %c\n", z, *argarr[z]);
  }
  
  /* // Debug: print modified line
  printf("\nhello\n");
  for (int a = 0; a < strlen(line); a++) {
    printf("%c", line[a]);
  }
  printf("\nbye\n");
  */
  *argcptr = argc;
  return argarr;
}

char *removeQuotes(char *line, int n, int quotes, char **argarr)
{
  printf("n - quotes = %d\n", (n - quotes));
  int dest = 0; //write
  int src = 0;  //read
  //while (line[dest] != 0) {
  while (dest < n)//(n-quotes) - quotes)
  {
    if (line[src] != '\"')// && line[src + 1] != 0)
    {
      line[dest++] = line[src];
      for (int a = 0; a < n; a++) 
      {
        printf("%c", line[a]);
      }
      printf("\n");
    }
    if (src < n) src++;
  }
  return line;
}