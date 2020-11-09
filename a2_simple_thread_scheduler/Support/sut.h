#ifndef __SUT_H__
#define __SUT_H__
#include <stdbool.h>
#include <ucontext.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#define MAX_THREADS                        32
#define THREAD_STACK_SIZE                  1024*64
#define CMD_LENGTH 256

typedef struct __threaddesc
{
	int threadid;
	char *threadstack;
	void *threadfunc;
	ucontext_t threadcontext;
} threaddesc;

typedef struct waitinfo
{
    int threadid;
    char cmd[CMD_LENGTH];
    char *arg_pointer;
    int arg;
}waitinfo;

typedef void (*sut_task_f)();

/**
 * @brief Handles computation of tasks
 * 
 * @param arg 
 * @return void* 
 */
void *C_EXEC(void *arg);

/**
 * @brief Handles IO interruptions
 * 
 * @param arg 
 * @return void* 
 */
void *I_EXEC(void *arg);

/**
 * @brief Initialize the CEXEC and IEXEC threads using pthread
 * library. Initialise shared data structure, mutex and other variables.
 * 
 */
void sut_init();

/**
 * @brief Creates new contexts and places into the queue. If context is successfully
 * added the function would return true otherwise false due to exceeding maximum concurrent
 * thread count (50). Also initialises sockets for each context.
 * 
 * @param fn 
 * @return true 
 * @return false 
 */
bool sut_create(sut_task_f fn);

/**
 * @brief Save the current state of running task and reschedule
 * for later
 */
void sut_yield();

/**
 * @brief Destroy the state of the current running task by not adding
 * it back to ready queue
 * 
 */
void sut_exit();

/**
 * @brief Opens the destination socket 
 * Socket information stored in socket array.
 * 
 * @param dest 
 * @param port 
 */
void sut_open(char *dest, int port);
/**
 * @brief Write to the previously opened socket. This is non-blocking
 * and currently running task is alowed to continue
 * 
 * @param buf 
 * @param size 
 */
void sut_write(char *buf, int size);
/**
 * @brief Close the previously opened socket assigned to current task. 
 * 
 */
void sut_close();
/**
 * @brief Reads buffer from the socket. The task that calls this will only 
 * be continued after a successful read
 * 
 * @return char* 
 */
char *sut_read();

/**
 * @brief Shuts down the connection and wait until all task completes.
 *
 */
void sut_shutdown();

/**
 * @brief Helper function to hand over operation to IO
 *
 */
void transfer_to_io(waitinfo *io_task);

/**
 * @brief Helper function to hand add task to tail
 * of ready queue for execution
 *
 */
void transfer_to_task_q_tail(int id);

/**
 * @brief Helper function to hand add task to head
 * of ready queue for execution
 *
 */
void transfer_to_task_q_head(int id);

/**
 * @brief Helper function to send message in buffer to 
 * opened socket connection
 *
 */
ssize_t send_message(int sockfd, const char *buf, size_t len);

/**
 * @brief Helper function to read message in buffer from 
 * opened socket connection
 *
 */
ssize_t recv_message(int sockfd, char *buf, size_t len);

/**
 * @brief Helper function to connect to socket connection
 *
 */
int connect_to_server(const char *host, uint16_t port, int *sockfd);




#endif
