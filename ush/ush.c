/*   Cameron Wallace
 *   April 1, 2020
 *   CSCI 347 Spring 2020
 *   Assignment 4
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
#include <signal.h>
#include "defn.h"
#define DEFINE_GLOBALS
#include "globals.h"
#define WAIT 1
#define EXPAND 2
#define NOWAIT 0
#define NOEXPAND 0

/* Constants */

#define LINELEN 1024
#define MAXLEN 200000

/* Prototypes */

char **arg_parse(char *line, int *argcptr);
void signals();

/* Globals */

pid_t cpid;
struct sigaction sa;
FILE* infile;

/* Shell main */
int main(int mainargc, char **mainargv)
{
  char buffer[MAXLEN];
  int len;
  
  gargc = mainargc;
  gargv = mainargv;
  gshift = 0;
  
  int open = 0;
  
  signals();
  
  while (1)
    {
      hadsigint = 0;
      if ( mainargc == 1 )
	{
	  infile = stdin;
	  fprintf(stderr,"%% ");
	}
      if ( mainargc > 1 && open == 0 )
	{
	  gshift = 1;
	  open = 1;
	  infile = fopen(mainargv[1], "r");
	  if ( infile == NULL)
	    {
	      fprintf(stderr, "Error reading script.\n");
	      exit(127);
	    }
	}
      /* prompt and get line */
      if (fgets(buffer, MAXLEN, infile) != buffer)
	break;
      /* Get rid of \n at end of buffer. */
      len = strlen(buffer);
      int i = 0;
      while ( i  < len )
	{
	  if ( buffer[i] == '#' && i == 0 )
	    {
	      buffer[i] = 0;
	      break;
	    }
	  else if ( i > 0 && buffer[i] == '#' && buffer[i - 1] != '$' )
	    {
	      buffer[i] = 0;
	      break;
	    }
	  else if ( buffer[i]  == '\n' )
	    {
	      buffer[i] = 0;
	      break;
	    }
	  i++;
	}
      /* Run it ... */
      processline(buffer, 0, 1, WAIT | EXPAND);
    }
  if (!feof(infile))
    perror("read");
  
  return 0; /* Also known as exit (0); */
}

/* using sigaction(2) */
void signals()
{
  sa.sa_handler = got_int;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if ( sigaction(SIGINT, &sa, NULL) < 0)
    {
      exitv = 1;
      exit(1);
    }
}

/* handle interruption */
void got_int(int sig)
{
  if (sig == SIGINT)
    {
      hadsigint = 1;
      return;
    }
}

int processline(char *line, int infd, int outfd, int flag)
{
  zombie();
  int status;
  int rv = 0;

  int fd[2];
  
  char newLine[MAXLEN];
  memset(newLine, 0, MAXLEN);

  /* the initial call is expanded, and subsequent calls are simply
     copied to avoid repeat expansion. */
  if ( flag&EXPAND )
    {
      int success = expand(line, newLine, MAXLEN);
      if( success == -1 )
	{
	  exitv = -1;
	  return -1;
	}
    }
  else {
    strncpy(newLine, line, MAXLEN);
  }
  
  /* start pipes */
  char *cmd = newLine;
  char *loc = strchr(cmd, '|');
  int nextIn;

  /* the main while will send individual pipes recursively to processline
     until there are no more | symbols. when this condition is met
     the final pipe is sent to process line using outfd and all open
     file descriptors are closed. */
  if ( loc != NULL )
    {
      *loc = 0;
      if ( pipe(fd) < 0 ) perror("pipe");
      processline(cmd, infd, fd[1], NOWAIT | NOEXPAND );
      close(fd[1]);
      
      nextIn = fd[0];
 
      cmd = loc + 1;
      if ( (loc = strchr(cmd, '|')) != NULL )
	{
	  *loc = 0;
	}
      /* loop until there are no remaining | symbols */
      while ( loc != NULL )
	{
	  if ( pipe(fd) < 0 ) perror("pipe");
	  processline(cmd, nextIn, fd[1], NOWAIT | NOEXPAND );
	  close(fd[1]);
	  close(nextIn);
	  
	  nextIn = fd[0];

	  cmd = loc + 1;
	  if ( (loc = strchr(cmd, '|')) != NULL )
	    {
	      *loc = 0;
	    }
	}
      /* last piece of the pipe is sent to initial outfd */
      processline(cmd, nextIn, outfd, NOEXPAND | WAIT);
      close(fd[0]);
      close(fd[1]);
      return rv;
    }
  
  int argc;
  char **args = arg_parse(newLine, &argc);
  
  if (args == NULL) return -1;

  int bi = builtIn(args, &argc, outfd);
  if ( bi != 0 )
    {
      /* Start a new process to do the job. */
      cpid = fork();
      rv = cpid;
      if (cpid < 0)
	{
	  /* Fork wasn't successful */
	  perror("fork");
	  return -1;
	}
      
      /* Check for who we are! */
      if (cpid == 0)
	{
	  if ( hadsigint == 1 )
	    {
	      kill(cpid, SIGINT);
	    }
	  /* We are the child! */
	  if ( dup2(infd, 0) < 0 )
	    {
	      perror("dup");
	      return -1;
	    }
	  if ( dup2(outfd, 1) < 0 )
	    {
	      perror("dup");
	      return -1;
	    }
	  execvp(*args, args);
	  printf("args:%s\n", *args);
	  /* execlp reurned, wasn't successful */
	  perror("exec");
	  fclose(infile); // avoid a linux stdio bug
	  exit(127);
	}
      /* Have the parent wait for child to complete */
      if ( flag&WAIT )
	{
	  if ( waitpid(cpid, &status, 0) < 0 )
	    {
	      free(args);
	      /* Wait wasn't successful */
	      perror("wait");
	    }
	  sighelper(status);
	}
      if ( !flag&WAIT )
	{
	  rv = cpid;
	}
      zombie();
    }
  free(args);
  memset(newLine, 0, MAXLEN);
  return rv;
}

/* used to kill zombie processes 
   return: int value to be passed to signal helper */
int zombie()
{
  int status;
  int pid = waitpid(-1, &status, WNOHANG);
  while (pid > 0)
    {
      pid = waitpid(-1, &status, WNOHANG);
    }
  return status;
}

/* checks the wait status and assigns appropriate exit 
   values, along with printing status messages */
void sighelper(int status)
{
  if ( WIFEXITED(status) )
    {
      exitv = WEXITSTATUS(status);
    }
  else if ( WIFSIGNALED(status) )
    {
      int sigret = WTERMSIG(status);
      exitv = sigret + 128;
      if ( status != 2 )
	{
	  const char *retstr = sys_siglist[sigret];
	  if ( WCOREDUMP(status) )
	    {
	      printf("%s (core dumped)\n", retstr);
	    }
	  else
	    {
	      printf("%s\n", retstr);
	    }
	}
    }
  fflush(stdout);
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
	if (line[i] == 0)
	  {
	    break;
	  }
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
