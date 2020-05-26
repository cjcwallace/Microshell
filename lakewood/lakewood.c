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
#include <math.h>

long global_data = 0;
pthread_mutex_t mutex1;

/* Globals */

int rate;
int sleepv;
int rnGen;

int groups;
int freeJackets = 10;
char *crafts[] = {"kayak", "canoe", "boat"};
int costs[] = {1, 2, 4};

struct queue mq;
int line = 0;

//int gsize = (int) log10 (groups);

/* Prototypes */
void getJackets();
void returnJackets();

void fatal (long n) {
  printf ("Fatal error, lock or unlock error, thread %ld.\n", n);
  exit(n);
}

struct group {
  long gnum;
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

void queue_insert (struct queue* queue, long value)
{
  struct group *tmp = malloc(sizeof(struct group));
  if (tmp == NULL)
    {
      fputs ("malloc failed\n", stderr);
      exit(1);
    }

  tmp->gnum = value;
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
      retval = tmp->gnum; //change this
      queue->head = tmp->next;
      free(tmp);
    }
  line--;
  return retval;
}

int queue_head (struct queue *queue)
{
  if (line >= 1)
    {
      return queue->head->gnum;
    }
  return -1;
}

void print_queue (struct queue *queue)
{
  struct group *tmp = malloc(sizeof(struct group));
  tmp = queue->head;
  printf("    Queue: [");
  while (tmp->next != NULL)
    {
      printf("%ld, ",tmp->gnum);
      tmp = tmp->next;
    }
  if (tmp->next == NULL)
    {
      printf("%ld", tmp->gnum);
    }
  printf("]\n");
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
  long groupn     = (long) arg;
  struct group *g = malloc(sizeof(struct group));
  g->gnum = groupn;
  long selection = random() % 3;
  char *craft    = crafts[selection];
  long jackets   = costs[selection];
  //int sleepv = random() % 8;
  int waiting = 0;

  printf("Group %ld requesting a %s with %ld lifevests\n", groupn, craft, jackets);
  if ( line > 5 ) /* ignore all other processes if line is too long */
    {
      printf("   Group %ld has grown impatient!\n", groupn);
      pthread_exit((void *)jackets);
    }
  if ( line <= 5 || waiting == 1 )
    {
      if ( freeJackets < jackets || (!queue_isEmpty(&mq) && g->gnum != queue_head(&mq))) /* wait */
	{
	  if (pthread_mutex_lock(&mutex1)) { fatal(groupn); }
	  queue_insert(&mq, groupn);
	  printf("   Group %ld waiting in line for %ld vests.\n", groupn, jackets);
	  waiting = 1;
	  print_queue(&mq);
	  if (pthread_mutex_unlock(&mutex1)) { fatal(groupn); }
	}
      while ((!queue_isEmpty(&mq) && g->gnum != queue_head(&mq)) || freeJackets < jackets)
	{
	  sleep(1);//sleepv;	  
	}
    }
  if (g->gnum == queue_head(&mq))
    {
      if (pthread_mutex_lock(&mutex1)) { fatal(groupn); }
      queue_remove(&mq);
      printf("   Waiting group %ld may now proceed.\n", groupn);
      if (pthread_mutex_unlock(&mutex1)) { fatal(groupn); }
    }

  /* Use the craft */
  if (pthread_mutex_lock(&mutex1)) { fatal(groupn); }
  freeJackets -= jackets;
  printf("Group %ld issued %ld lifevests, %d remaining\n", groupn, jackets, freeJackets);
  if (pthread_mutex_unlock(&mutex1)) { fatal(groupn); }
  sleep(rand() % 8);//8
  /* Finish using craft, release jackets back to natural habitat */
  if (pthread_mutex_lock(&mutex1)) { fatal(groupn); }
  freeJackets += jackets;
  printf("Group %ld returning %ld lifevests, now we have %d\n", groupn, jackets, freeJackets);
  if (pthread_mutex_unlock(&mutex1)) { fatal(groupn); }
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
  queue_init(&mq);
 
  if (!argv[1])
    {
      printf("usage: ./lakewood num_customers [wait_time [r]]\n");
      exit(1);
    }
  groups = atoi(argv[1]);
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
    sleep(1);//sleepv
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
