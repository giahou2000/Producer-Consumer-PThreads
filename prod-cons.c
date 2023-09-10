/*
 * A simple producer-consumer task, to get into parallel programming
*/

// Include the necessary libraries
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define QUEUESIZE 10
#define LOOP 50000

// The producer and the consumer
void *producer (void *args);
void *consumer (void *args);

// This struct exists because that's the only way to get a whole function through pthreads
typedef struct workFunction{
  void * (*work)(void *);
  void * arg;
  clock_t start;
} workFunction;

// This is what the thread will produce after the consumption
void* whatIHaveToDo(){
	printf("Hi, this is a message from me, an ordinary thread! \n");
}

// The queue is necessary for the producer-consumer program
// The producer stores "products" in the queue and the consumer drags the "products" to consume them
typedef struct {
  workFunction buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
} queue;

// Functions of the queue data structure
// Queue initialization
queue *queueInit (void);
// Queue deletion
void queueDelete (queue *q);
// Add "product" to queue
void queueAdd (queue *q, workFunction in);
// Subtract/delete "product" from queue
void queueDel (queue *q, workFunction *out);



/*
 ***### The main function. Here the program begins to run. ###***
*/
int main (){
	
	// Get time measurements for observation and statistics
	clock_t starting = clock();
	
	// Creation of a FIFO(First in - First out) queue
	queue *fifo;
	
	// Declare the number of producers(p) and consumers(q)
	int p = 4;
	int q = 10;
	
	// Declare the producers and the consumers
	pthread_t pro[p], con[q];
	
	// If you want to save the results to a file uncomment the followings
	//FILE *fptr;
	//change the path to a suitable one!!!!!!!!!!!!!!!!!!!
	//fptr = fopen("C:\\Users\\...\\prod_con_times.dat", "w");
	
	// Initialize the queue
	fifo = queueInit ();
	if (fifo == NULL) {
		fprintf(stderr, "main: Queue Init failed.\n");
		exit(1);
	}
	
	/*
	* Here the producers and the consumers start to work
	*/
	// Create the producers' threads
	for(int i = 0 ; i < p ; i++){
		pthread_create (&pro[i], NULL, producer, (void*)fifo);
	}
	// Create the consumers' threads
	for(int i = 0 ; i < q ; i++){
		pthread_create (&con[i], NULL, consumer, (void*)fifo);
	}
	
	// Wait until all producers finish their work
	for(int t = 0 ; t < p ; t++) {
		pthread_join(pro[t], NULL);
	}
	
	// Wait until all consumers finish their work
	for(int t = 0 ; t < q ; t++) {
		pthread_join(con[t], NULL);
	}
	
	// Destroy the queue so that it doesn't use memory without reason
	queueDelete(fifo);
	
	// If you want to save the results to a file uncomment the following
	//fclose(fptr);
	
	// Stop the stopwatch
	clock_t ending = clock();
	
	// Convert to an understandable format
	double cpu_time_used;
	cpu_time_used = ((double) (ending - (starting))) / CLOCKS_PER_SEC;
	printf("The program's time was %f \n", cpu_time_used);
	return 0;
}



/*
* Building the producer's structure
*/
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

/*
* Building the consumer's structure
*/
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