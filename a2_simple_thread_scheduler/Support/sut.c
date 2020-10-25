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

static ucontext_t parent;
struct queue ready_q;
struct queue waiting_q;
int numthreads;
int curthread;

// int x =1;
// int y =2;
// int z =3;
// int w =4;

//cexec thread to handle computation of tasks
void *C_EXEC(void *arg){
    pthread_mutex_t *lock = arg;

    struct queue_entry *ptr;
    //need to figure out how to read out and swap to context
    // int curthread;
    while(true){
        pthread_mutex_lock(lock);
        if(numthreads > 0){
            ptr = queue_pop_head(&ready_q);
            curthread = *(int *)ptr ->data;
            usleep(1000);
            // ptr = queue_pop_head(&ready_q);
            // printf(" 1  C_EXEC\n");
            getcontext(&parent);//might not be necesary. I want to save the curret context here
            pthread_mutex_unlock(lock);
            swapcontext(&parent,&(threadarr[curthread].threadcontext));
            usleep(1000 * 1000);
        }else{
            pthread_mutex_unlock(lock);
            usleep(1000 * 1000);
        }
        // printf("popped %d\n", *(int*)ptr->data);
        // printf(" 1 Hello from\n");
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

//initialise ready and wait queue
    ready_q = queue_create();
    queue_init(&ready_q);

    waiting_q =  queue_create();
    queue_init(&waiting_q);
    
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
    
    struct queue_entry *node = queue_new_node(&(tdescptr->threadid));
    queue_insert_tail(&ready_q, node);

    numthreads++;
    
    return true;
}

void sut_shutdown(){

    pthread_join(cexec_thread_handle, NULL);
    pthread_join(iexec_thread_handle, NULL);

}

void sut_yield(){
    
    struct queue_entry *node = queue_new_node(&(threadarr[curthread].threadid));
    queue_insert_tail(&ready_q, node);
    swapcontext(&(threadarr[curthread].threadcontext),&parent);
}

void sut_exit(){
    numthreads--;
    swapcontext(&(threadarr[curthread].threadcontext),&parent);
}
void sut_open(char *dest, int port);
void sut_write(char *but, int size);
void sut_close();
char *sut_read();


// int main(){
//     sut_init();
//     test_sut_create(&x);
//     test_sut_create(&y);
//     test_sut_create(&z);
//     test_sut_create(&w);
//     sut_shutdown();
//     return 0;
// }



//useful to add stuff to queue
//  struct queue_entry *node1 = queue_new_node(&x);
//     queue_insert_tail(&ready_q, node1);
    
//     struct queue_entry *node2 = queue_new_node(&y);
//     queue_insert_tail(&ready_q, node2);

//     struct queue_entry *ptr = queue_pop_head(&ready_q);
//     while(ptr) {
//         printf("popped %d\n", *(int*)ptr->data);
        
//         queue_insert_tail(&ready_q, ptr);
//         usleep(1000 * 1000);
        
//         ptr = queue_pop_head(&ready_q);
//     }