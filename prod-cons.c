/*
  A simple producer-consumer task, to get into parallel programming
*/

// Include the necessary libraries
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define QUEUESIZE 10
#define LOOP 50000

void *producer (void *args);
void *consumer (void *args);

// my addings
typedef struct workFunction{
  void * (*work)(void *);
  void * arg;
  clock_t start;
} workFunction;

void* whatIHaveToDo(){
	printf("Hi, this is a message from me, an ordinary thread! \n");
}

typedef struct {
  workFunction buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
} queue;

queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q, workFunction in);
void queueDel (queue *q, workFunction *out);






/*
 ***### The main function ###***
*/
int main (){
  clock_t starting = clock();
  queue *fifo;
  int p = 4;
  int q = 10;
  pthread_t pro[p], con[q];
  //FILE *fptr;
  
  //change the path to a suitable one!!!!!!!!!!!!!!!!!!!
  //fptr = fopen("C:\\Users\\Christos\\chris\\auth\\Parallel and distributed systems\\pds-codebase\\prod_con_times.dat", "w");

  fifo = queueInit ();
  if (fifo == NULL) {
    fprintf(stderr, "main: Queue Init failed.\n");
    exit(1);
  }
  for(int i = 0 ; i < p ; i++){
	  pthread_create (&pro[i], NULL, producer, (void*)fifo);
  }
  for(int i = 0 ; i < q ; i++){
	  pthread_create (&con[i], NULL, consumer, (void*)fifo);
  }
  //wait until all threads finish
  for(int t = 0 ; t < p ; t++) {
       pthread_join(pro[t], NULL);
	}
  for(int t = 0 ; t < q ; t++) {
	   pthread_join(con[t], NULL);
	}
  queueDelete(fifo);
  //fclose(fptr);
  clock_t ending = clock();
  double cpu_time_used;
  cpu_time_used = ((double) (ending - (starting))) / CLOCKS_PER_SEC;
  printf("The program's time was %f \n", cpu_time_used);
  return 0;
}








void *producer (void *q)
{
  queue *fifo;
  int i;

  fifo = (queue *)q;

  for (i = 0; i < LOOP; i++) {
    pthread_mutex_lock (fifo->mut);
    while (fifo->full) {
      printf ("producer: queue FULL.\n");
      pthread_cond_wait (fifo->notFull, fifo->mut);
    }
	workFunction wF;
	wF.work = whatIHaveToDo;
    queueAdd (fifo, wF);
    pthread_mutex_unlock (fifo->mut);
    pthread_cond_signal (fifo->notEmpty);
  }
  return (NULL);
}

void *consumer (void *q)
{
  queue *fifo;
  int i;
  workFunction d;
  fifo = (queue *)q;

  while(1){
    pthread_mutex_lock (fifo->mut);
    while (fifo->empty) {
      printf ("consumer: queue EMPTY.\n");
      pthread_cond_wait (fifo->notEmpty, fifo->mut);
    }
    queueDel (fifo, &d);
    pthread_mutex_unlock (fifo->mut);
    pthread_cond_signal (fifo->notFull);
    printf ("consumer: recieved %d.\n", d);
  }
  return (NULL);
}

/*
  typedef struct {
  int buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
  } queue;
*/

queue *queueInit (void)
{
  queue *q;

  q = (queue *)malloc (sizeof (queue));
  if (q == NULL) return (NULL);

  q->empty = 1;
  q->full = 0;
  q->head = 0;
  q->tail = 0;
  q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
  pthread_mutex_init (q->mut, NULL);
  q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notFull, NULL);
  q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notEmpty, NULL);
	
  return (q);
}

void queueDelete (queue *q)
{
  pthread_mutex_destroy (q->mut);
  free (q->mut);	
  pthread_cond_destroy (q->notFull);
  free (q->notFull);
  pthread_cond_destroy (q->notEmpty);
  free (q->notEmpty);
  free (q);
}

void queueAdd (queue *q, workFunction in)
{
  q->buf[q->tail] = in;
  q->buf[q->tail].start = clock();
  q->tail++;
  if (q->tail == QUEUESIZE)
    q->tail = 0;
  if (q->tail == q->head)
    q->full = 1;
  q->empty = 0;

  return;
}

void queueDel (queue *q, workFunction *out)
{
  *out = q->buf[q->head];
  //check the time
  clock_t end = clock();
  double cpu_time_used;
  cpu_time_used = ((double) (end - (out->start))) / CLOCKS_PER_SEC;
  printf("The measured time was %f \n", cpu_time_used);
  //save the time(maybe use mutex)
  //fprintf(fptr, "%f", cpu_time_used);
  out -> work(NULL);
  q->head++;
  if (q->head == QUEUESIZE)
    q->head = 0;
  if (q->head == q->tail)
    q->empty = 1;
  q->full = 0;

  return;
}