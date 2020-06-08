/* Cameron Wallace 
 * June 1, 2020 
 * CSCI 347 Spring 2020
 * Assignment 6
 *
 * Simple matrix multiply program provided by
 * Phil Nelson, March 5, 2019
 *
 */

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

/* idx macro calculates the correct 2-d based 1-d index
 * of a location (x,y) in an array that has col columns.
 * This is calculated in row major order. 
 */

#define idx(x,y,col)  ((x)*(col) + (y))

struct group {
  double *A;
  double *B;
  double *C;
  int x;
  int y;
  int z;
  int n; /* number of comps */
  int start;
  int threadn;
  int times;
  double clock;
  pthread_barrier_t *bar;
};

/* Returns list containing number of computations each thread computes
 * x <- (x * z), tcount <- number of threads used */
int getSplit(int x, int tcount, int *tsplit)
{
  int v = x / tcount;
  for( int i = 0; i < tcount; i++ )
    {
      tsplit[i] = v;
    }
  int excess = x % tcount;
  if ( excess > 0 )
    {
      for( int i = 0; i < excess; i++ )
	{
	  tsplit[i] += 1;
	} 
    }
  for (int i = 1; i < tcount; i++ )
    {
      tsplit[i] += tsplit[i-1];
    }
  return 0;
}

double wall_time() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL)) return 0;
  return (double) tv.tv_sec + (double) tv.tv_usec * 0.000001;
}

double cpu_time() {
  return (double) clock() / CLOCKS_PER_SEC;
}


/* idx = (x)(col) + y */
void * mul_thread (void *arg)
{
  struct group *g = (struct group*)arg;
  double *A = g->A;
  double *B = g->B;
  double *C = g->C;
  int x = g->x, y = g->y, z = g->z;
  /* start and end indices of completed matrix */
  int start = g->start;
  int end = start + g->n;
  int times = 0 + g->times;
  pthread_barrier_t *bar = g->bar;
  //printf("t:%d, s: %d, e: %d\n",g->threadn, start, end);
  int count = 0;
  double time1 = wall_time();
  while (times > 0)
    {
      count++;
      /* Goal: compute cells start -> end of solution matrix */
      for (int i = start; i < end; i++)
	{
	  /* row of matA and col of matB */
	  int row = i/z;
	  int col = (i % z);
	  /* multiply row A by col B */
	  double tval = 0;
	  for (int j = 0; j < y; j++)
	    { 
	      tval += A[idx(row,j,y)] * B[idx(j,col,z)];
	    } 
	  C[i] = tval;
	}
      times--;
      if (times > 0)
	{
	  pthread_barrier_wait(bar);
	  A = C;
	  C = B;
	  B = A;
	}
    }
  double time2 = wall_time();
  printf("thread:%d, count:%d\n", g->threadn, count);
  pthread_exit(NULL);
  
}

/* Matrix Multiply:
 *  C (x by z)  =  A ( x by y ) times B (y by z)
 *  This is the slow n^3 algorithm
 *  A and B are not be modified
 */
void MatMul (double *A, double *B, double *C, int x, int y, int z, int tcount)
{
  double *time;
  pthread_t ids[tcount];
  int tsplit[tcount];
  getSplit((x * z), tcount, tsplit);
  for (int i = 0; i < tcount; i++) {
    struct group *g = malloc(sizeof(struct group));
    g->threadn = i;
    g->A = A;
    g->B = B;
    g->C = C;
    g->x = x;
    g->y = y;
    g->z = z;
    g->clock = &time;
    if ( i == 0 ) g->n = tsplit[i];
    else g->n = tsplit[i] - tsplit[i - 1];
    g->start = tsplit[i] - g->n;
    g->times = 1;
    int err = pthread_create (&ids[i], NULL, mul_thread, (void *) g);
    if (err) {
      fprintf(stderr, "Can't create thread %d\n", i);
      exit(1);
    }
  }
  void *retval;
  for (int i = 0; i < tcount; i++)
    {
      pthread_join(ids[i], &retval);
    }
}

/* Matrix Square: 
 *  B = A ^ 2*times
 *    A are not be modified.
 */

void MatSquare (double *A, double *B, int x, int times, int tcount)
{
  pthread_t ids[tcount];
  int tsplit[tcount];
  pthread_barrier_t bar;
  pthread_barrier_init(&bar, NULL, tcount);
  getSplit((x * x), tcount, tsplit);
  
  memcpy(B, A, sizeof(double) * (x*x));
  double *T = (double *)malloc(sizeof(double)*x*x);
  for (int i = 0; i < tcount; i++) {
    struct group *g = malloc(sizeof(struct group));
    g->threadn = i;
    g->bar = &bar;
    g->A = g->B = B; g->C = T;
    g->x = g->y = g->z = x;
    if ( i == 0 ) g->n = tsplit[i];
    else g->n = tsplit[i] - tsplit[i - 1];
    g->start = tsplit[i] - g->n;
    g->times = times;
    int err = pthread_create (&ids[i], NULL, mul_thread, (void *)g);  
    if (err) {
      fprintf(stderr, "Can't create thread %d\n", i);
      exit(1);
    }
  }  
  void *retval;
  for (int i = 0; i < tcount; i++)
    {
      pthread_join(ids[i], &retval);
    }
  if (times%2)
    {
      memcpy(B, T, sizeof(double) * (x*x));
    }
}

/* Print a matrix: */
void MatPrint (double *A, int x, int y)
{
  int ix, iy;

  for (ix = 0; ix < x; ix++) {
    printf ("Row %d: ", ix);
    for (iy = 0; iy < y; iy++)
      printf (" %10.5G", A[idx(ix,iy,y)]);
    printf ("\n");
  }
}

/* Generate data for a matrix: */
void MatGen (double *A, int x, int y, int rand)
{
  int ix, iy;

  for (ix = 0; ix < x ; ix++) {
    for (iy = 0; iy < y ; iy++) {
      A[idx(ix,iy,y)] = ( rand ?
			  ((double)(random() % 200000000))/2000000000.0 :
			  (1.0 + (((double)ix)/100.0) + (((double)iy/1000.0))));
    }
  }	
}
  
/* Print a help message on how to run the program */
void usage(char *prog)
{
  fprintf (stderr, "%s: [-dr] -x val -y val -z val -n num_threads\n", prog);
  fprintf (stderr, "%s: [-dr] -s num -x val -n num_threads\n", prog);
  exit(1);
}

/* Main function
 *
 *  args:  -d   -- debug and print results
 *         -r   -- use random data between 0 and 1 
 *         -s t -- square the matrix t times 
 *         -x   -- rows of the first matrix, r & c for squaring
 *         -y   -- cols of A, rows of B
 *         -z   -- cols of B
 *         -n   -- num threads
 *         -t   -- calc time
 *  
 */

int main (int argc, char ** argv)
{
  extern char *optarg;   /* defined by getopt(3) */
  int ch;                /* for use with getopt(3) */

  /* option data */
  int x = 0, y = 0, z = 0;
  int debug = 0;
  int square = 0;
  int useRand = 0;
  int sTimes = 0;
  int tcount = 8;
  int t = 0;
  
  while ((ch = getopt(argc, argv, "drTs:x:y:z:n:")) != -1) {
    switch (ch) {
    case 'd':  /* debug */
      debug = 1;
      break;
    case 'n': /* num threads */
      tcount = atoi(optarg);
      break;
    case 'r':  /* debug */
      useRand = 1;
      srandom(time(NULL));
      break;      
    case 's':  /* s times */
      sTimes = atoi(optarg);
      square = 1;
      break;
    case 'T':
      t = 1;
      break;
    case 'x':  /* x size */
      x = atoi(optarg);
      break;
    case 'y':  /* y size */
      y = atoi(optarg);
      break;
    case 'z':  /* z size */
      z = atoi(optarg);
      break;
    case '?': /* help */
    default:
      usage(argv[0]);
    }
  }

  /* verify options are correct. */
  if (square) {
    if (y != 0 || z != 0 || x <= 0 || sTimes < 1) {
      fprintf (stderr, "Inconsistent options\n");
      usage(argv[0]);
    }
  } else if (x <= 0 || y <= 0 || z <= 0) {
    fprintf (stderr, "-x, -y, and -z all need to be specified or -s and -x.\n");
    usage(argv[0]);
  } 

  /* Matrix storage */
  double *A;
  double *B;
  double *C;
  
  struct timeval tv;
  time_t curtime;

  if (square) {
    A = (double *) malloc (sizeof(double) * x * x);
    B = (double *) malloc (sizeof(double) * x * x);
    MatGen(A,x,x,useRand);
    if (t) {
      
    }
    MatSquare(A, B, x, sTimes, tcount);
    if (debug) {
      printf ("-------------- orignal matrix ------------------\n");
      MatPrint(A,x,x);
      printf ("--------------  result matrix ------------------\n");
      MatPrint(B,x,x);
      if (t) {
	printf("Elapsed time: %d\nCPU time:%d\n");
      }
    }
  } else {
    A = (double *) malloc (sizeof(double) * x * y);
    B = (double *) malloc (sizeof(double) * y * z);
    C = (double *) malloc (sizeof(double) * x * z);
    MatGen(A,x,y,useRand);
    MatGen(B,y,z,useRand);
    if (t) {
      double time1 = wall_time(); 
    }
    MatMul(A, B, C, x, y, z, tcount);
    if (debug) {
      printf ("-------------- orignal A matrix ------------------\n");
      MatPrint(A,x,y);
      printf ("-------------- orignal B matrix ------------------\n");
      MatPrint(B,y,z);
      printf ("--------------  result C matrix ------------------\n");
      MatPrint(C,x,z);
      if (t) {
	double time2 = wall_time();
	printf("Elapsed time: %d\nCPU time:%d\n", (time2-time1), );
      }
    }
  }
  return 0;
}
