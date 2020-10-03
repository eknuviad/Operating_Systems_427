#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "a1_lib.h"
#include "rpc.h"
#define BUFSIZE   1024

// typedef struct request_msg{
//     char func_name[BUFSIZE];
//     char x[BUFSIZE];
//     char y[BUFSIZE];
// }request_msg;

int main(int argc, char *argv[]) {
  int sockfd;
  char user_input[BUFSIZE] = { 0 };
  char server_msg[BUFSIZE] = { 0 };

  if(argc <=2 || argc >3){
    fprintf(stderr, "Wrong number of arguments to connect to server\n");
    return 0;
  }
  if (connect_to_server(argv[1], atoi(argv[2]), &sockfd) < 0) {
    fprintf(stderr, "oh no\n");
    return -1;
  }
  while (strcmp(user_input, "quit\n")) {
    memset(user_input, 0, sizeof(user_input));
    memset(server_msg, 0, sizeof(server_msg));

    // read user input from command line
    fgets(user_input, BUFSIZE, stdin);
    // send the input to server
    // request_msg rmsg = {"add", "13","7"};
    // send_message(sockfd, (char*) &rmsg, sizeof(rmsg));
    send_message(sockfd, user_input, strlen(user_input));
    // receive a msg from the server
    ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
    if (byte_count <= 0) {
      break;
    }
    // response_msg *rpmsg = (response_msg*) server_msg; 
    // printf("Server: %s\n", rpmsg->return_value);
    printf("Server: %s\n", server_msg);
  }

  return 0;
}

