#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "sut.h"
#include "queue.h"
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

threaddesc threadarr[MAX_THREADS];
// pthreads global 
pthread_t cexec_thread_handle;
pthread_t iexec_thread_handle;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

static ucontext_t parent;
struct queue ready_q;
struct queue waiting_q;
struct waitinfo *cur_wait;
int numthreads;
int curthread;
int waitqcount;
char x = '3';



//cexec thread to handle computation of tasks
void *C_EXEC(void *arg){
    pthread_mutex_t *lock = arg;
    struct queue_entry *ptr;

    while(true){
        pthread_mutex_lock(lock);

        if(numthreads > 0){
            ptr = queue_pop_head(&ready_q);
            curthread = *(int *)ptr ->data;
            usleep(1000);
            getcontext(&parent);//might not be necesary. I want to save the curret context here
            pthread_mutex_unlock(lock);
            swapcontext(&parent,&(threadarr[curthread].threadcontext));
            usleep(1000 * 1000);

        }else{
            pthread_mutex_unlock(lock);
            usleep(1000 * 1000);
        }
    }
}

//I_EXEC thread to handle io interruptions
void *I_EXEC(void *arg){
    pthread_mutex_t *lock = arg;

    // while(true){
    //     pthread_mutex_lock(lock);
    //     printf("2 Hello from \n");
    //     usleep(1000);
    //     printf("2 I_EXEC\n");
    //     pthread_mutex_unlock(lock);
    //     usleep(1000 * 1000);

    // }
    struct queue_entry *ptr;

    while(true){
        pthread_mutex_lock(lock);

        if(waitqcount > 0){
            ptr = queue_pop_head(&waiting_q);
            cur_wait = (waitinfo *)ptr ->data;
            usleep(1000);
            if(strcmp("open", cur_wait->cmd)){
                printf("Reached here");
                waitqcount--;
            }
            // getcontext(&parent);//might not be necesary. I want to save the curret context here
            // pthread_mutex_unlock(lock);
            // swapcontext(&parent,&(threadarr[curthread].threadcontext));
            usleep(1000 * 1000);

        }else{
            pthread_mutex_unlock(lock);
            usleep(1000 * 1000);
        }
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
    queue_insert_tail(&ready_q, node); //might need to place a lock on this

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

void sut_open(char *dest, int port){
    waitinfo wait_info = {.threadid = curthread, .cmd = "open",
                    .arg_pointer = dest, .arg = port};
    struct queue_entry *node = queue_new_node(&(wait_info));
    queue_insert_tail(&waiting_q, node);
    waitqcount++;
    swapcontext(&(threadarr[curthread].threadcontext),&parent);

}


int connect_to_server(const char *host, uint16_t port, int *sockfd) {
    // might need to double check this method for accuracy
  struct sockaddr_in server_address = { 0 };

  // create a new socket
  *sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (*sockfd < 0) {
    perror("Failed to create a new socket\n");
    return -1;
  }

  // connect to server
  server_address.sin_family = AF_INET;
  inet_pton(AF_INET, host, &(server_address.sin_addr.s_addr));
  server_address.sin_port = htons(port);
  if (connect(*sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
    perror("Failed to connect to server\n");
    return -1;
  }
  return 0;
}

void sut_write(char *buf, int size);
void sut_close();
char *sut_read();



int main(){
    sut_init();
    sut_open(&x, 3);
    sut_shutdown();
    return 0;
}