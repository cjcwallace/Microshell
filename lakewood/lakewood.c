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

void fatal (long n) {
  printf ("Fatal error, lock or unlock error, thread %ld.\n", n);
  exit(n);
}

struct group {
  long gnum;
  char *craft;
  long jackets;
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
      fputs ("malloc failed\n", stderr);
      exit(1);
    }

  tmp->jackets = value;
  tmp->next = NULL;

  if (queue->head == NULL)
    {
      queue->head = tmp;
    } else
    {
      queue->tail->next = tmp;
    }
  line++;
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
  line--;
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
void * thread_body ( void *arg ) {//, int gnum, int sleepv, struct group group) {
  long threadn = (long) arg;
  struct group g;
  long selection = random() % 3;
  char *craft = crafts[selection];
  long jackets = costs[selection];
    
  if ( line > 5 )
    {
      printf("Group %ld has grown impatient!\n", threadn + 1);
      pthread_exit((void *)jackets);
    }
  while ( line < 5 )
    {
      if ( freeJackets > jackets )
	{
	    if (pthread_mutex_lock(&mutex1)) { fatal(threadn); }
	    printf("Group Number:%2ld, Craft:%6s, Needed Jackets:%2ld\n", threadn + 1, craft, jackets);
	    sleep(1);
	    printf("bye\n");
	    if (pthread_mutex_unlock(&mutex1)) { fatal(threadn); }

	}
      else if ( freeJackets < jackets )
	{
	  
	}
    }
  pthread_exit((void *)jackets);  
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
int main (int argc, char **argv) {
  struct queue mq;
  queue_init(&mq);
 
  if (!argv[1])
    {
      printf("usage: ./lakewood num opt opt\n");
      exit(1);
    }
  int groups = atoi(argv[1]);
  pthread_t ids[groups];
  int err;
  long i;

  if (argc == 2)
    {
      rate = 6;
      sleepv = 0;
      rnGen = 0;
      srandom(0);
    }
  else if (argc == 3)
    {
      rate = atoi(argv[2]);
      sleepv = rand() % atoi(argv[2]);
      rnGen = 0;
      srandom(sleepv);
    }
  /*
  else if (argc == 3)
    {
      rate = argv[2];
      sleepv = rand() % argv[2];
      rnGen = srandom(argv[3]);
      srandom(sleepv);
    }
  */
  pthread_mutex_init(&mutex1, NULL);
  
  for (i = 0; i < groups; i++) {
    err = pthread_create (&ids[i], NULL, thread_body, (void *)i);
    if (err) {
      fprintf (stderr, "Can't create thread %ld\n", i);
      exit (1);
    }
  }
  
  void *retval;

  for (i = 0; i < groups; i++) {
    pthread_join(ids[i], &retval);
  }
 
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
