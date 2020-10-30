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
#define BUFSIZE 1024

threaddesc threadarr[MAX_THREADS];
int sockarr[MAX_THREADS];
// pthreads global 
pthread_t cexec_thread_handle;
pthread_t iexec_thread_handle;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//schedule context
ucontext_t parent;

struct queue ready_q;
struct queue waiting_q;
struct waitinfo *cur_wait;
int numthreads;
int numsockts;
int curthread;
int waitqcount;
bool shut_down;


char read_buf[BUFSIZE] = {0};


ssize_t send_message(int sockfd, const char *buf, size_t len) {
  return send(sockfd, buf, len, 0);
}
ssize_t recv_message(int sockfd, char *buf, size_t len) {
  return recv(sockfd, buf, len, 0);
}


int connect_to_server(const char *host, uint16_t port, int *sockfd) {
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

//cexec thread to handle computation of tasks
void *C_EXEC(void *arg){
    struct queue_entry *ptr;
    while(true){
        pthread_mutex_lock(&lock);
        if(queue_peek_front(&ready_q) != NULL){
            ptr = queue_pop_head(&ready_q);
            curthread = *(int *)ptr ->data;
            pthread_mutex_unlock(&lock);
            usleep(100);
            swapcontext(&parent,&(threadarr[curthread].threadcontext));
            usleep(100);
        }else if(numthreads == 0 && shut_down == true){
            pthread_mutex_unlock(&lock);
            break;
        }else{
            pthread_mutex_unlock(&lock);
            usleep(100);
        }
    }
}

//I_EXEC thread to handle io interruptions
void *I_EXEC(void *arg){
    struct queue_entry *ptr;
    // int num =0;

    while(true){
        pthread_mutex_lock(&lock);
        // printf("checkwait %d\n", queue_peek_front(&waiting_q) !=NULL);
        if(waitqcount > 0){
            ptr = queue_pop_head(&waiting_q);
            waitinfo *task = ptr->data;
            pthread_mutex_unlock(&lock);

            if(strcmp("open", task->cmd)==0){
                //io thread performs connection to server
                pthread_mutex_lock(&lock);
                connect_to_server(task->arg_pointer, task->arg, &(sockarr[task->threadid]));
                struct queue_entry *node = queue_new_node(&(threadarr[task->threadid].threadid));
                queue_insert_tail(&ready_q, node);
                pthread_mutex_unlock(&lock);
              
            }else if (strcmp("read", task->cmd)==0){
                usleep(1000);
                recv_message(sockarr[task->threadid], read_buf, sizeof(read_buf));
               

                pthread_mutex_lock(&lock);
                struct queue_entry *node1 = queue_new_node(&(threadarr[task->threadid].threadid));
                queue_insert_tail(&ready_q, node1);
                pthread_mutex_unlock(&lock);
                // dumcount++;
                // printf("dumcout %d\n", dumcount);

            }else if (strcmp("write", task->cmd)==0){
                pthread_mutex_lock(&lock);
                usleep(1000);
                send_message(sockarr[task->threadid], task->arg_pointer, task->arg);
                pthread_mutex_unlock(&lock);
                
              
            }else{//instruction was close
                shutdown(sockarr[task->threadid],SHUT_RDWR);
            }
            waitqcount--;
        }else if(numthreads == 0 && shut_down == true){
            pthread_mutex_unlock(&lock);
            break;
        }else{
            pthread_mutex_unlock(&lock);
        }
    }
}

//Method to initialise C_EXEC and IEXEC threads and queues
void sut_init(){
    pthread_create(&cexec_thread_handle,NULL, C_EXEC, &lock);
    pthread_create(&iexec_thread_handle,NULL, I_EXEC, &lock);
    numthreads = 0;
    numsockts = 0;
    shut_down = false;
    //initialise ready and wait queue
    ready_q = queue_create();
    queue_init(&ready_q);
    waiting_q =  queue_create();
    queue_init(&waiting_q); 
}



//Method to add function to a new context to be executed
bool sut_create(sut_task_f fn){
    pthread_mutex_lock(&lock);
    threaddesc *tdescptr;
    int *sockdescptr;
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

    sockdescptr = &(sockarr[numthreads]);
    *sockdescptr = -1; //initialise socket for every thread created
    numthreads++;
    pthread_mutex_unlock(&lock);
    return true;
}

void sut_shutdown(){
    pthread_mutex_lock(&lock);
    shut_down = true;
    pthread_mutex_unlock(&lock);
    pthread_join(cexec_thread_handle, NULL);
    pthread_join(iexec_thread_handle, NULL);
}

void sut_yield(){
    pthread_mutex_lock(&lock);
    struct queue_entry *node = queue_new_node(&(threadarr[curthread].threadid));
    queue_insert_tail(&ready_q, node);
    pthread_mutex_unlock(&lock);
    swapcontext(&(threadarr[curthread].threadcontext),&parent);
}

void sut_exit(){
   
    pthread_mutex_lock(&lock);
    numthreads--;
    pthread_mutex_unlock(&lock);
   
    swapcontext(&(threadarr[curthread].threadcontext),&parent); //schedule next task in queue of ready tasks
}

void sut_open(char *dest, int port){
    waitinfo wait_info = {.threadid = curthread, .cmd = "open",
                    .arg_pointer = dest, .arg = port};

    pthread_mutex_lock(&lock);
    struct queue_entry *node = queue_new_node(&(wait_info));
    queue_insert_tail(&waiting_q, node);
    waitqcount++;
    pthread_mutex_unlock(&lock);

    swapcontext(&(threadarr[curthread].threadcontext),&parent);

}



void sut_write(char *buf, int size){
    

    waitinfo wait_info = {.threadid = curthread, .cmd = "write",
                    .arg_pointer = buf, .arg = size};
    pthread_mutex_lock(&lock);
    struct queue_entry *node1 = queue_new_node(&(wait_info));
    queue_insert_tail(&waiting_q, node1);   
    waitqcount++;
    pthread_mutex_unlock(&lock);
    
    //get current state of task
    getcontext(&(threadarr[curthread].threadcontext));

    pthread_mutex_lock(&lock);
    //add to head of ready queue to immediately continue current thread fxn.
    struct queue_entry *node = queue_new_node(&(threadarr[curthread].threadid));
    queue_insert_head(&ready_q, node);
    pthread_mutex_unlock(&lock);

    swapcontext(&(threadarr[curthread].threadcontext),
                &parent);

}


void sut_close(){
    waitinfo wait_info = {.threadid = curthread, .cmd = "close",
                    .arg_pointer = 0, .arg = -1};
    pthread_mutex_lock(&lock);
    struct queue_entry *node1 = queue_new_node(&(wait_info));
    queue_insert_tail(&waiting_q, node1);   
    waitqcount++;
    pthread_mutex_unlock(&lock);
    
}

char *sut_read(){
    //adds task to wait queue
    char* buf_copy = (char*) malloc (sizeof(read_buf));
    waitinfo wait_info = {.threadid = curthread, .cmd = "read",
                    .arg_pointer = read_buf, .arg = strlen(read_buf)};
    pthread_mutex_lock(&lock);
    struct queue_entry *node = queue_new_node(&(wait_info));
    queue_insert_tail(&waiting_q, node);
    waitqcount++;
    pthread_mutex_unlock(&lock);
    getcontext(&(threadarr[curthread].threadcontext));

    //save state and dont resume until readbuf has a value
    swapcontext(&(threadarr[curthread].threadcontext), &parent);

    //Fxn swaps back here after io is complete to get read_buf value
    strcpy(buf_copy, read_buf);

    return buf_copy;
}

