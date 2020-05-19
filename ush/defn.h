/*  Cameron Wallace
 *  April 15, 2020
 *  CSCI 347 Spring
 *  Assignment 2
 *  Header file for A2
 */

/* Prototype */

int builtIn (char **args, int *argc, int outfd);
int expand (char *orig, char *new, int newsize);
int processline(char *line, int infd, int outfd, int flags);
int writeNew  (char *new, char *rv, int *j, int newsize);
int zombie ();

//void processline(char *line);
void got_int(int sig);
void strmode (mode_t mode, char *p);
void sighelper(int status);
