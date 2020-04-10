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

void processline (char *line);
char ** arg_parse (char *line, int *argcptr);

/* Shell main */

int
main (void)
{
    char   buffer [LINELEN];
    int    len;

    while (1) {

        /* prompt and get line */
	fprintf (stderr, "%% ");
	if (fgets (buffer, LINELEN, stdin) != buffer)
	  break;

        /* Get rid of \n at end of buffer. */
	len = strlen(buffer);
	if (buffer[len-1] == '\n')
	    buffer[len-1] = 0;

	/* Run it ... */
	//processline (buffer);
	
	int *ptr;
	arg_parse(buffer, ptr);

    }

    if (!feof(stdin))
        perror ("read");

    return 0;		/* Also known as exit (0); */
}


void processline (char *line)
{
    pid_t  cpid;
    int    status;
    
    /* Start a new process to do the job. */
    cpid = fork();
    if (cpid < 0) {
      /* Fork wasn't successful */
      perror ("fork");
      return;
    }
    
    /* Check for who we are! */
    if (cpid == 0) {
      /* We are the child! */
      execlp (line, line, (char *)0);
      /* execlp reurned, wasn't successful */
      perror ("exec");
      fclose(stdin);  // avoid a linux stdio bug
      exit (127);
    }
    
    /* Have the parent wait for child to complete */
    if (wait (&status) < 0) {
      /* Wait wasn't successful */
      perror ("wait");
    }   
}

char ** arg_parse (char *line, int *argcptr)
{
  /* line points to character array containing command to be processed */
  /* argcptr points to int variable representing number of args in line */
  /* return value: pointer to malloced area that points into line parameter  */
  /* call malloc(3) only once */

  int argc = 0;
  int i = 0;
  
  // Count args
  while (line[i] != 0) {
    //printf("count = %d, i:%d, char:%c\n", argc, i, line[i]);
    // skip spaces
    while(line[i] == ' ') i++;
    // start arg
    if (line[i] != ' ' && line[i] != 0) {
	argc++;
	i++;
      // find end of arg
      while (line[i] != ' ') {
	if (line[i] == 0) break;
	//printf("i:%d, char:%c\n", argc, i, line[i]);
	i++;
      }
    }
  }

  printf("%d args\n", argc);
  printf("int size %d\nchar size %d\n",sizeof(int), sizeof(char));
  int mem = ((argc * sizeof(int)) + sizeof(char));
  printf("argcptr size %d\n", mem);
  
  // Allocate memory
  argcptr = (int*) malloc(mem);

  
  i = 0;
  int ac = 0;
  // Assign pointers and 0s
  while (line[i] != 0) {
    //printf("count = %d, i:%d, char:%c\n", argc, i, line[i]);
    // skip spaces
    while(line[i] == ' ') i++;
    // start arg
    if (line[i] != ' ' && line[i] != 0) {
	argcptr[ac] = i;
	ac++;
	i++;
      // find end of arg
      while (line[i] != ' ') {
	if (line[i] == 0) break;
	//printf("i:%d, char:%c\n", argc, i, line[i]);
	i++;
      } //line[i] = 0;
    }
  } argcptr[ac] = NULL;
  
  int b = 0;
  while (argcptr[b] != NULL) {
    printf("argcptr[%d] = %d\n", b, argcptr[b]);
    b++;
  }
  
//  for (int a = 0; a < strlen(line); a++) {
 //   printf("%c", line[a]);
 // }

  return NULL;
}



























