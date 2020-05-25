/* Cameron Wallace
 * May 22, 2020
 * CSCI 347 Spring 2020
 * Assignment 5
 *
 * Base code provided by Phil Nelson
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

long global_data = 0;
pthread_mutex_t mutex1;

/* Globals */

int rate;
int sleepv;
int rnGen;

int freeJackets = 10;
char *crafts[] = {"kayak", "canoe", "boat"};
int costs[] = {1, 2, 4};

int line = 0;

/* Prototypes */
void getJackets();
void returnJackets();

void printids (char *name) {
  pid_t      pid = getpid();
  pthread_t  tid = pthread_self();

  printf ("%s: pid %u tid %lu\n", name, (unsigned) pid, (unsigned long)tid);
}

void fatal (long n) {
  printf ("Fatal error, lock or unlock error, thread %ld.\n", n);
  exit(n);
}

struct group {
  int gnum;
  int selection = random() % 3;
  char *craft = crafts[selection];
  int jackets = costs[selection];
  struct group *next;
};

struct queue {
  struct group *head;
  struct group *tail;
};

void queue_init (struct queue *queue)
{
  queue->head = NULL;
  queue->tail = NULL;
}

bool queue_isEmpty (struct queue *queue)
{
  return queue->head == NULL;
}

void queue_insert (struct queue* queue, int value)
{
  struct group *tmp = malloc(sizeof(struct group));
  if (tmp == NULL)
    {
      fputs ("malloc gailed\n", stderr);
      exit(1);
    }

  tmp->data = value;
  tmp->next = NULL;

  if (queue->head == NULL)
    {
      queue->head = tmp;
    } else
    {
      queue->tail->next = tmp;
    }
  queue->tail = tmp;
}

int queue_remove (struct queue *queue)
{
  int retval = 0;
  struct group *tmp;

  if (!queue_isEmpty(queue))
    {
      tmp = queue->head;
      retval = tmp->jackets; //change this
      queue->head = tmp->next;
      free(tmp);
    }
  return retval;
}

/*
  threads:
    Print out group #, craft requested, # jackets needed
    Check to see if enough jackets available
    If no jackets available:
      wait or
      if >5 groups waiting, exit
    When jackets available report group # and left jackets
    Call sleep with "use time"
    Report return of jackets and available jackets
    Unblock as many from queue as possible
 */
void * thread_body ( void *arg, int gnum,  int sleepv) {
  /*
  long threadn = (long) arg;
  long local_data = random() % 100000;
  long ix;
  
  printf ("Starting thread %ld, local_data is %ld\n", threadn, local_data);
  for (ix = 0; ix < local_data; ix++) {
    if (pthread_mutex_lock(&mutex1)) { fatal(threadn); }
    // CRITICAL
    
    if (pthread_mutex_unlock(&mutex1)) { fatal(threadn); }
  }
  pthread_exit((void *)local_data);
  

  printf("Group Number:%d, Craft:%s, Needed Jackets:%d\n", gnum, craft, jackets);
  if ( line > 5 )
    {
      printf("Group %d has grown impatient!\n", gnum);
      pthread_exit();
    }
  while ( line < 5 )
    {
      if ( freeJackets > jackets )
	{
	  
	}
      else if ( freeJackets < jackets )
	{
	  
	}
    }
  */
  if (pthread_mutex_lock(&mutex1)) { fatal(threadn); }
  printf("Group Number:%d, Craft:%s, Needed Jackets:%d\n", gnum, craft, jackets);
  sleep(sleepv);
  printf("bye\n");
  if (pthread_mutex_unlock(&mutex1)) { fatal(threadn); }
  pthread_exit((*void);
}

/*
  argv[1]: num groups to generate
  argv[2]: (optional) rate for new groups to arrive
           default=6, use rand number between 0 and
	   value for sleep
  argv[3]: (optional/requires argv[2]) Initialize rand
           num with time(NULL), default = 0
  loop that creates new threads should join finished threads
  main should join all remaining threads after creation
*/
int main (int argc, int *argv) {
  struct queue mq;
  queue init(&myqueue);
  
  if (argv[1])
    {
      pthread_t ids[argv[1]];
    }
  int err;
  long i;

  if (argc == 1)
    {
      
      rate = 6;
      sleepv = 0;
      rnGen = 0;
      srandom(0);
    }
  else if (argc == 2)
    {
      rate = argv[2];
      sleepv = rand() % argv[2];
      rnGen = 0;
      srandom(sleepv);
    }
  else if (argc == 3)
    {
      rate = argv[2];
      sleepv = rand() % argv[2];
      rnGen = srandom(argv[3]);
      srandom(sleepv);
    }

  pthread_mutex_init(&mutex1, NULL);
  
  for (i = 0; i < N; i++) {
    err = pthread_create (&ids[i], NULL, thread_body, (void *)i);
    if (err) {
      fprintf (stderr, "Can't create thread %ld\n", i);
      exit (1);
    }
  }
  /*
  printids("main");

  void *retval;

  for (i=0; i < N; i++) {
    pthread_join(ids[i], &retval);
    final_data += (long)retval;
  }

  printf ("global_data is %ld,  final_data is %ld\n", global_data, final_data);
  */
  pthread_mutex_destroy(&mutex1);  // Not needed, but here for completeness
  return 0;
}

/*
  globals:
    availableJackets
    activeKayak, activeCanoe, activeBoat
    waitList
  ideas:
    first come first serve: a kayak will not be served before
    a canoe/boat even if there are available lifejackets
    maximum of 5 waiter
    main spawns a thread for each renter
      each thread represents a group of renters
      each thread randomly selects craft
      each thread selects between 0-7 time to use craft
 */
