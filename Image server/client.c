#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>


#define IP_ADDRESS "127.0.0.1" 
#define PORT 1337 
#define SIZE 1024
#define BUFFER_SIZE 2048

// Global variables


int main(int argc, char **argv){
  //variables
	struct sockaddr_in server_addr;
  FILE *fp;
  char file_send[SIZE] = {};
  char respone_message[BUFFER_SIZE] = {};
  int size =0;
  int err;
  int sockfd = 0;


	/* Socket settings */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) {
    perror("Error in socket");
    return EXIT_FAILURE;
  }
  printf("Server socket created successfully.\n");
	bzero(&server_addr, sizeof(server_addr)); 			        // Clearing the struct
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
  server_addr.sin_port = htons(PORT);
	
  //chack ./client.exe filePath
  if(argc <2){
    printf("Error - Please use the following method\n");
    printf("./client.exe FilePath\n");
    return EXIT_FAILURE;
  }

  // Chack file exists
  if( access( argv[1], F_OK ) != 0 ) {
    // file not exists
    printf("File: %s\n",argv[1]);
    printf("File not exists\n");
    return EXIT_FAILURE;
  } 

  // Connect to Server
  err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}
  printf("Connected to Server.\n");

  // Open the file
  fp = fopen(argv[1], "rb");
  if (fp == NULL) {
    perror("Error in reading file.");
    return EXIT_FAILURE;
  }

  //Send the file to server
  printf("Start send file\n\r");
  while(1){
    int read = fread(file_send, sizeof(unsigned char), SIZE, fp);

    if (read <= 0){
      printf("File size = %.2f MB\n",(double)size/1000000);
      break;
    }
    size += read;
    if (send(sockfd, file_send, sizeof(file_send), 0) == -1) {
      break;
    }
    bzero(file_send, SIZE);
  }

  // File close
  fclose(fp);


  //send the file finish
  send(sockfd, "File finish", sizeof("File finish"), 0);

  //Waiting for a response from the server
  while (1) {
    int receive = recv(sockfd, respone_message, BUFFER_SIZE, 0);

    if (receive > 0) {
      printf("\n\t\tThe server response\n");
      printf("------------------------------------------------------\n");
      printf("\t%s", respone_message);
      printf("------------------------------------------------------\n\n");
      break;
    } 
    else {
      break;
    }
    memset(respone_message, 0, sizeof(respone_message));
  }

  //close socket
	close(sockfd);
  printf("Closing the connection.\n");
	return EXIT_SUCCESS;
}


