#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "a1_lib.h"
#include "rpc.h"
#define BUFSIZE 1024

int main(int argc, char *argv[])
{
  int sockfd;
  char user_input[BUFSIZE] = {0};
  char server_msg[BUFSIZE] = {0};
  char reply[3*BUFSIZE] = {0};
  request_msg request;
  response_msg *response;

  if (argc <= 2 || argc > 3)
  {
    fprintf(stderr, "Wrong number of arguments to connect to server\n");
    return 0;
  }
  if (connect_to_server(argv[1], atoi(argv[2]), &sockfd) < 0)
  {
    fprintf(stderr, "oh no\n");
    return -1;
  }
  while (strcmp(user_input, "exit\n"))
  {
    memset(user_input, 0, sizeof(user_input));
    memset(server_msg, 0, sizeof(server_msg));
    printf(">>");

    // read user input from command line
    fgets(user_input, BUFSIZE, stdin);
    strcat(user_input, " \n"); 
    char *token = strtok(user_input, " ");
    strcpy(request.cmd, token);
    token = strtok(NULL, "");
    strcpy(request.params, token);

    //send message to backend for processing
    send_message(sockfd, (char*) &request, sizeof(request));
  
    if (strcmp(user_input, "quit\n") == 0 || strcmp(user_input, "shutdown\n") == 0)
    {
      send_message(sockfd, (char*) &request, sizeof(request));
      break;
    }

    // receive a msg from the server
    ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
    if (byte_count <= 0)
    {
      break;
    }
    
    response = (response_msg*) server_msg;
    strcpy(reply, response -> return_value);
    printf("%s\n", reply);
    fflush(stdout);
  }
  return 0;
}
