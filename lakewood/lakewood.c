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

/* Globals */

pthread_mutex_t mutex1;

int rate;
int sleepv;
int rnGen;

int groups;
int freeJackets = 10;
char *crafts[] = {"kayak", "canoe", "boat"};
int costs[] = {1, 2, 4};

struct queue mq;
struct queue cq;
int line = 0;

pthread_cond_t done;

void fatal (long n) {
  printf ("Fatal error, lock or unlock error, thread %ld.\n", n);
  exit(n);
}

struct group {
  long gnum;
  int jackets;
  struct group *next;
};

/* Prototypes */
int getJackets(struct group *g);
void putJackets(struct group *g);

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
  long selection = random() % 3;
  char *craft    = crafts[selection];
  long jackets   = costs[selection];
  g->gnum = groupn;
  g->jackets = jackets;

  printf("Group %ld requesting a %s with %ld lifevests\n", groupn, craft, jackets);
  int success = getJackets(g);
  if (!success) {
    pthread_exit(NULL);
  }
  sleep(rand() % 8);
  putJackets(g);
  pthread_exit((void *)jackets);
}

/* return: 0 did not get, 1 got */
int getJackets(struct group *g)
{
  int waiting = 0;
  if ( line > 5 ) /* ignore all other processes if line is too long */
    {
      printf("   Group %ld has grown impatient!\n", g->gnum);
      return 0;
    }
  if ( line <= 5 || waiting == 1 )
    {
      if ( freeJackets < g->jackets || (!queue_isEmpty(&mq) && g->gnum != queue_head(&mq)))
	{
	  if (pthread_mutex_lock(&mutex1)) { fatal(g->gnum); }
	  queue_insert(&mq, g->gnum);
	  printf("   Group %ld waiting in line for %d vests.\n", g->gnum, g->jackets);
	  waiting = 1;
	  print_queue(&mq);
	  if (pthread_mutex_unlock(&mutex1)) { fatal(g->gnum); }
	}
      while ((!queue_isEmpty(&mq) && g->gnum != queue_head(&mq)) || freeJackets < g->jackets)
	{
	  if (pthread_cond_wait(&done, &mutex1)) { perror("wait"); exit(g->gnum); }
	}
    }
  if (g->gnum == queue_head(&mq))
    {
      if (pthread_mutex_lock(&mutex1)) { fatal(g->gnum); }
      queue_remove(&mq);
      printf("   Waiting group %ld may now proceed.\n", g->gnum);
      if (pthread_mutex_unlock(&mutex1)) { fatal(g->gnum); }
    }
  return 1;
}

/* Use the craft and return jackets to available pool */
void putJackets(struct group *g)
{
  if (pthread_mutex_lock(&mutex1)) { fatal(g->gnum); }
  freeJackets -= g->jackets;
  printf("Group %ld issued %d lifevests, %d remaining\n", g->gnum, g->jackets, freeJackets);
  if (pthread_mutex_unlock(&mutex1)) { fatal(g->gnum); }
  sleep(rand() % 8);
  /* Finish using craft, release jackets back to natural habitat */
  if (pthread_mutex_lock(&mutex1)) { fatal(g->gnum); }
  freeJackets += g->jackets;
  pthread_cond_signal(&done);
  printf("Group %ld returning %d lifevests, now we have %d\n", g->gnum, g->jackets, freeJackets);
  if (pthread_mutex_unlock(&mutex1)) { fatal(g->gnum); }
  //pthread_exit((void *)jackets);
}


/*
  loop that creates new threads should join finished threads
  main should join all remaining threads after creation
*/
int main (int argc, char **argv) {
  queue_init(&mq); /* queue used to wait */
  queue_init(&cq); /* completed queue */
  
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
      rate = 7;
      sleepv = 0;
      srandom(0);
    }
  else if (argc == 3)
    {
      rate = atoi(argv[2]) + 1;
      sleepv = rand() % atoi(argv[2]);
      srandom(0);
    }
  else if (argc == 3)
    {
      rate = atoi(argv[2]);
      sleepv = rand() % atoi(argv[2]);
      srandom(atoi(argv[3]));
    }

  pthread_mutex_init(&mutex1, NULL);
  
  for (i = 0; i < groups; i++) {
    err = pthread_create (&ids[i], NULL, thread_body, (void *)i);
    if (err) {
      fprintf (stderr, "Can't create thread %ld\n", i);
      exit(1);
    }
    sleep(rand() % rate);
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
