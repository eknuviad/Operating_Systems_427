#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "a1_lib.h"
#include "rpc.h"

#define BUFSIZE   1024

int addInts(int a, int b){
  return a + b;
}

int multiplyInts(int a, int b){
  return a * b;
}

float divideFloats(float a, float b){
  return a/b;
}

//return the factorial x
uint16_t factorial(int x){
  int factorial = 1;
  for (int i = 1; i<=x; i++){
      factorial *= i;
  }
  return factorial;
}


int main(int argc, char *argv[]) {
  int sockfd, clientfd;
  char msg[4*BUFSIZE] = {0};
  char user_input[BUFSIZE] = {0};
  char cmd[4*BUFSIZE] = {0};
  char args[4*BUFSIZE] = {0};
  int end_signal = 0;
  pid_t childpid;

  if (argc <= 2 || argc >3){
    fprintf(stderr,"Wrong number of arguments to setup server\n");
    return 0;
  }

  if (create_server(argv[1], atoi(argv[2]), &sockfd) < 0) {
    fprintf(stderr, "oh no\n");
    return -1;
  }else{
    fprintf(stderr,"[+]Server created successfully\n");
  }

  while (1){
    if (accept_connection(sockfd, &clientfd) < 0) {
      fprintf(stderr, "oh no\n");
      return -1;
    }else{
      fprintf(stderr,"[+]Connection accepted from client\n");
    }

    //child process to handle messages
    if((childpid = fork())==0){
    
      while (strcmp(msg, "quit\n")) {
        memset(msg, 0, sizeof(msg));
        char result[BUFSIZE];
        char x[BUFSIZE]={0};
        char y[BUFSIZE]={0};
        int i = 1;

        //get message from frontend
        ssize_t byte_count = recv_message(clientfd, msg, sizeof(msg));
        if (byte_count <= 0) {
          break;
        }
     
        //make local copy of comand and parameters
        request_msg *request = (request_msg*) msg;
        strcpy(cmd, request -> cmd);
        strcpy(args, request->params);

        char *token = strtok(args, " ");

        if (strcmp(cmd, "add")==0){ 
          //store inputs in char arrays
            while(token!=0){
               if (i == 1){
                strcpy(x, token);
              }
              else if(i == 2){
              strcpy(y,token);
              }else{
                fflush(stdout);
              }
              token = strtok(0," ");
              i++;
          }
          sprintf(result, "%d", addInts(atoi(x),atoi(y)));

        }else if (strcmp(cmd, "multiply")==0){
          //store inputs in char arrays
            while(token!=0){
              if (i == 1){
                strcpy(x, token);
              }
              else if(i == 2){
              strcpy(y,token);
              }else{
               fflush(stdout);
              }
              token = strtok(0," ");
              i++;
          }
          sprintf(result, "%d", multiplyInts(atoi(x),atoi(y)));

        }else if (strcmp(cmd,"divide")==0){
              while(token!=0){
              if (i == 1){
                strcpy(x, token);
              }
              else if(i == 2){
              strcpy(y,token);
              }else{
                fflush(stdout);
              }
              token = strtok(0," ");
              i++;
          }

          //check division by zero error
          if(strcmp(y,"0\n") == 0){
            sprintf(result, "Error: Division by zero");
          }else{
            sprintf(result, "%f", divideFloats((float) atoi(x),(float) atoi(y)));
          }

        }else if (strcmp(cmd,"factorial")==0){
              while(token!=0){
              if (i == 1){
                strcpy(x, token);
              }
              else{
                fflush(stdout);
              }
              token = strtok(0," ");
              i++;
          }
          sprintf(result, "%d", factorial(atoi(x)));


        }else if (strcmp(cmd,"sleep")==0){
            fprintf(stderr, "Sleeping...");
            //make the calculator sleep for x seconds
            sleep(atoi(token)); 
            sprintf(result, "Done sleeping for %s seconds", token );

        }
        else if (strcmp(cmd, "quit")==0 || strcmp(cmd, "shutdown")==0){
          exit(0);
        }
        else{
          sprintf(result, "Error: Command %s not found", cmd);  
        }
        send_message(clientfd, result, BUFSIZE);
      }
    }
  }
  close(clientfd);
  return 0;
}