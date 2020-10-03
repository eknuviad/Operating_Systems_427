#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
  //dont forget to report divide by zero error
}

//make the calculator sleep for x seconds - this is blocking
// int sleep (int x){
//  usleep(x);
//   return 0;
// }

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
  // const char *greeting = "hello, world\n";
  char msg[BUFSIZE] = {0};
  char user_input[BUFSIZE] = {0};
  const char *greeting;
  int running = 1;
  pid_t childpid;

  if (argc <= 2 || argc >3){
    fprintf(stderr,"Wrong number of arguments to setup server\n");
    return 0;
  }
    // fgets(user_input, BUFSIZE, stdin);

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

    if((childpid = fork())==0){
    
      while (strcmp(msg, "quit\n")) {
        memset(msg, 0, sizeof(msg));
        // char result[BUFSIZE];
        char result[BUFSIZE];
        char error[BUFSIZE];
        char cmd[BUFSIZE] = {0};
        char x[BUFSIZE]={0};
        char y[BUFSIZE]={0};
        int i = 1;
        ssize_t byte_count = recv_message(clientfd, msg, sizeof(msg));
        if (byte_count <= 0) {
          break;
        }
        char *token = strtok(msg, " ");

        if (strcmp(token, "add")==0){
          //  printf("cmd is %s\n", token);
          //store inputs in char arrays
            while(token!=0){
              if (i == 1){
                strcpy(cmd,token);
                printf("cmd is %s\n", cmd);
              }else if (i == 2){
                strcpy(x, token);
                printf("x is %s\n", x);
              }
              else if(i == 3){
              strcpy(y,token);
                printf("y is %s\n", y);
              }else{
                printf("Illegal arguments");
              }
              token = strtok(0," ");
              i++;
          }
          sprintf(result, "%d", addInts(atoi(x),atoi(y)));
          printf("The final result is %s\n", result);

        // }
        // }else if (strcmp(token, "multiply\n")==0){

        // }else if (strcmp(token,"divide\n")==0){

        }else if (strcmp(token,"factorial")==0){
              while(token!=0){
              if (i == 1){
                strcpy(cmd,token);
                printf("cmd is %s\n", cmd);
              }else if (i == 2){
                strcpy(x, token);
                printf("x is %s\n", x);
              }
            else{
                printf("Illegal arguments"); // modify this to accurately capture error
              }
              token = strtok(0," ");
              i++;
          }
          sprintf(result, "%d", factorial(atoi(x)));
          printf("The factorial result is %s\n", result);

        // }else if (strcmp(token, "quit")==0){

        }else if (strcmp(token,"sleep")==0){
            token = strtok(0, " ");
            //I need to send the message, sleep, send message
            sleep(atoi(token)); //make the calculator sleep for x seconds
          sprintf(result, "Done sleeping for %s seconds", token );
          // printf("The factorial result is %s\n", result);

        } else{
          sprintf(result, "Error: Command %s not found", token);  
        }
        send_message(clientfd, result, BUFSIZE);
      }
    }
  }

  return 0;
}

//problem right now is that when i recieve the message from the client atm
//it prints it out 3 times and the x y args give garbage values
// a possible solutiopn to test out wil be to change the use of char
//in the struct request
//might need to find an efficient declaration of the struct with params istead