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
  char msg[BUFSIZE] = {0};
  char user_input[BUFSIZE] = {0};
  int running = 1;
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
        char error[BUFSIZE];
        char cmd[BUFSIZE] = {0};
        char x[BUFSIZE]={0};
        char y[BUFSIZE]={0};
        int i = 1;
        //get message from frontend
        ssize_t byte_count = recv_message(clientfd, msg, sizeof(msg));
        if (byte_count <= 0) {
          break;
        }
        char *token = strtok(msg, " ");

        if (strcmp(token, "add")==0){ 
          //store inputs in char arrays
            while(token!=0){
              if (i == 1){
                strcpy(cmd,token);
              }else if (i == 2){
                strcpy(x, token);
              }
              else if(i == 3){
              strcpy(y,token);
              }else{
                printf("Illegal arguments");
                fflush(stdout);
              }
              token = strtok(0," ");
              i++;
          }
          sprintf(result, "%d", addInts(atoi(x),atoi(y)));


        }else if (strcmp(token, "multiply")==0){
          //store inputs in char arrays
            while(token!=0){
              if (i == 1){
                strcpy(cmd,token);
              }else if (i == 2){
                strcpy(x, token);
              }
              else if(i == 3){
              strcpy(y,token);
              }else{
                printf("Illegal arguments");
              }
              token = strtok(0," ");
              i++;
          }
          sprintf(result, "%d", multiplyInts(atoi(x),atoi(y)));

        }else if (strcmp(token,"divide")==0){
              while(token!=0){
              if (i == 1){
                strcpy(cmd,token);
              }else if (i == 2){
                strcpy(x, token);
              }
              else if(i == 3){
              strcpy(y,token);
              }else{
                printf("Illegal arguments");
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

        }else if (strcmp(token,"factorial")==0){
              while(token!=0){
              if (i == 1){
                strcpy(cmd,token);
              }else if (i == 2){
                strcpy(x, token);
              }
            else{
                printf("Illegal arguments");
                fflush(stdout);
              }
              token = strtok(0," ");
              i++;
          }
          sprintf(result, "%d", factorial(atoi(x)));


        }else if (strcmp(token,"sleep")==0){
            token = strtok(0, " ");
            //make the calculator sleep for x seconds
            sleep(atoi(token)); 
            sprintf(result, "Done sleeping for %s seconds", token );

        }
        else if (strcmp(token, "quit")==0 || strcmp(token, "shutdown")==0){
          sprintf(result, "Goodbye! from Edem's server"); 
          send_message(clientfd, result, BUFSIZE);
          end_signal = 1; 
          exit(0);
        }
        else{
          sprintf(result, "Error: Command %s not found", token);  
        }
        send_message(clientfd, result, BUFSIZE);
      }
    }

    if(end_signal == 1){
      break;
    }
  }

  return 0;
}