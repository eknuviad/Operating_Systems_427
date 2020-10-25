#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include "sut.h"
#include "queue.h"
#include <pthread.h>

threaddesc threadarr[MAX_THREADS];
// pthreads global 
pthread_t cexec_thread_handle;
pthread_t iexec_thread_handle;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

ucontext_t parent;
// struct queue ready_q;
// struct queue waiting_q;
int numthreads;

//cexec thread to handle computation of tasks
void *C_EXEC(void *arg){
    pthread_mutex_t *lock = arg;
    while(true){
        pthread_mutex_lock(lock);
        printf(" 1 Hello from\n");
        usleep(1000);
        printf(" 1  C_EXEC\n");
        pthread_mutex_unlock(lock);
        usleep(1000 * 1000);
    }
    //critical section is the queue so place a lock before accessing that
    //and release the lock for iexec to use when done
}

//I_EXEC thread to handle io interruptions
void *I_EXEC(void *arg){
    pthread_mutex_t *lock = arg;
    while(true){
        pthread_mutex_lock(lock);
        printf("2 Hello from \n");
        usleep(1000);
        printf("2 I_EXEC\n");
        pthread_mutex_unlock(lock);
        usleep(1000 * 1000);

    }
}

//Method to initialise C_EXEC and IEXEC threads and queues
void sut_init(){

    pthread_create(&cexec_thread_handle,NULL, C_EXEC, &m);
    pthread_create(&iexec_thread_handle,NULL, I_EXEC, &m);

    numthreads = 0;

    // struct queue *rq = &ready_q;
    // rq = queue_create();
    // queue_init(rq);

    // struct queue *wq =&waiting_q;
    // wq =  queue_create();
    // queue_init(wq);
    sut_shutdown();
}



//Method to add function to a new context to be executed
bool sut_create(sut_task_f fn){

    threaddesc *tdescptr;

    if (numthreads >= 32) {
		printf("FATAL: Maximum thread limit reached... creation failed! \n");
		return false;
	}

    tdescptr = &(threadarr[numthreads]);
	getcontext(&(tdescptr->threadcontext));
	tdescptr->threadid = numthreads;
	tdescptr->threadstack = (char *)malloc(THREAD_STACK_SIZE);
	tdescptr->threadcontext.uc_stack.ss_sp = tdescptr->threadstack;
	tdescptr->threadcontext.uc_stack.ss_size = THREAD_STACK_SIZE;
	tdescptr->threadcontext.uc_link = 0;
	tdescptr->threadcontext.uc_stack.ss_flags = 0;
	tdescptr->threadfunc = fn;

	makecontext(&(tdescptr->threadcontext), fn, 1, tdescptr);
    
    // struct queue_entry *node = queue_new_node(tdescptr);
    // queue_insert_tail(&ready_q, node);

    numthreads++;
    
    return true;
}

void sut_shutdown(){

    pthread_join(cexec_thread_handle, NULL);
    pthread_join(iexec_thread_handle, NULL);

}

void sut_yield();

void sut_exit();
void sut_open(char *dest, int port);
void sut_write(char *but, int size);
void sut_close();
char *sut_read();


int main(){
    sut_init();
    return 0;
}