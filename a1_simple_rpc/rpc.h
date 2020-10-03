#ifndef RPC_H_
#define RPC_H_

#define CMD_LENGTH 256
#define ARGS_LENGTH 256
#define BUFSIZE 1024

typedef struct message_t{
    char cmd[CMD_LENGTH];
    char args[ARGS_LENGTH];
}message_t;

typedef struct request_msg{
    char cmd[BUFSIZE];
    char x[BUFSIZE];
    char y[BUFSIZE];
}request_msg;

typedef struct response_msg{
    char return_value[BUFSIZE];
  //add some error handling
}response_msg;

#endif //RPC_H

// response
// requesstruct
// declare the RPCServ methods
