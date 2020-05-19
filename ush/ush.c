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
#define DEFINE_GLOBALS
#include "globals.h"


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
int first = 1;


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
      if (fgets(buffer, LINELEN, infile) != buffer)
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
      processline(buffer, 0, 1, 1);
    }
  printf("left while\n");
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
      //fprintf(stderr, "Could not register SIGINT\n");
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
  int status;
  int rv = 0;

  int fd[2];
  //  int fd2[2];
  int nextIn;
  
  char newLine[MAXLEN];
  memset(newLine, 0, MAXLEN);
  if ( flag == 1 )
    {
      int success = expand(line, newLine, MAXLEN);
      //printf("exp\n");
      if( success == -1 )
	{
	  perror("expand");
	  return -1;
	}
    }

      /*
	pipe p1
	pl(a, infd, p1[1], noexpand nowait )
	close p1[1]
	pipe p1
	nextIn = p1[0]
	pl(b, nextIn, p1[1], noexpand nowait )
	close nextIn, p1[1]
	nextIn = p1[0]
	pipe p1
	pl(c, nextIn, p1[1], noexpand nowait )
	close ....
	-------------------
	pl(e, nextIn, outfd, noexpand wait if flags have it )
       */
  
  /* start pipes */
  char *loc = strchr(newLine, '|');
  char *nextP;
  if ( loc || ( flag != 1 && flag != 2) )//|| flag == 0 )
    {
      printf("found pipe\n");
      if ( pipe(fd) < 0 )  perror("pipe");
      *loc = 0;
      nextIn = fd[0];
      nextP = strchr(&loc[1], '|');
      if ( nextP )
	{
	  if ( pipe(fd) < 0 ) perror("pipe");
	  if ( first == 1 )
	    {
	      processline( &loc[1], infd, fd[1], 0 );
	      first = 0;
	      close(fd[1]);
	    }
	  else
	    {
	      processline( &loc[1], nextIn, fd[1], 0 );
	      close(fd[1]);
	      close(nextIn);
	    }
	}
      else if ( nextP == NULL )
	{
	  if ( pipe(fd) < 0 ) perror("pipe");
	  printf("null\nfd[0]:%d\n", fd[0]);
	  printf("fd[1]:%d\n", outfd);
	  if ( first == 1 )
	    {
	      processline( &loc[1], infd, outfd, 2 );
	      close(fd[1]);
	      close(nextIn);
	    }
	  else
	    {
	      processline( &loc[1], nextIn, outfd, 2 );
	      close(fd[1]);
	      close(nextIn);
	    }
	}
    }
  if ( flag == 2 )
    {
      zombie();
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
	  
	  if ( dup2(outfd, 1) < 0 )
	    {
	      perror("dup");
	      return -1;
	    }

	  execvp(*args, args);
	   /* execlp reurned, wasn't successful */
	  perror("exec");
	  fclose(infile); // avoid a linux stdio bug
	  exit(127);
	}
      /* Have the parent wait for child to complete */
      if (wait(&status) < 0)
	{
	  free(args);
	  /* Wait wasn't successful */
	  perror("wait");
	}
      sighelper(status);
    }
  free(args);
  memset(newLine, 0, MAXLEN);
  //printf("rv:%d\n", rv);
  return rv;
}

int zombie()
{
  int pid = waitpid(-1, &cpid, WNOHANG);
  while (pid > 0)
    {
      pid = waitpid(-1, &cpid, WNOHANG);
    }
  return pid;
}

void sighelper(int status)
{
  if ( WIFEXITED(status) == 1 )
    {
      exitv = WEXITSTATUS(status);
    }
  if ( WIFSIGNALED(status) == 1 )
    {
      int sigret = WTERMSIG(status);
      exitv = sigret + 128;
      if ( sigret != 2 )
	{
	  if ( WCOREDUMP(status) )
	    {
	      printf("%s (core dumped)\n", sys_siglist[sigret]);
	    }
	  else
	    {
	      printf("%s\n", sys_siglist[sigret]);
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
