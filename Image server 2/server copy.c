 
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <netinet/in.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>


#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define PORT 1337 
#define SIZE 1024
#define BUFFER_SIZE 2048

//Image server 
#define IP_ADDRESS_SERVER "127.0.0.1" 
#define PORT_SERVER 3228 



/* Client structure */
typedef struct{
	int sockfd;
} client_t;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg);                    	// Handle all communication with the client
void send_Image(char *filename,int client);

int main(int argc, char **argv){
	int listenfd = 0;
	int connfd = 0;

	struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  pthread_t tid;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	/* Socket settings */
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);

	/* Bind */
  if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR: Socket binding failed\n");
    return EXIT_FAILURE;
  }

  /* Listen */
  if (listen(listenfd, MAX_CLIENTS) < 0) {
    perror("ERROR: Socket listening failed\n");
    return EXIT_FAILURE;
	}

	printf("=== server active  ===\n");


	while (1){
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
		/* Client settings */
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->sockfd = connfd;

		/* Add client to the queue and fork thread */
	
		pthread_create(&tid, NULL, &handle_client, (void*)cli);
	}
	
	return EXIT_SUCCESS;
}

void *handle_client(void *arg){
	int leave_flag = 0;
  FILE *fp;
  char filename[BUFFER_SZ];
  char buffer[SIZE];
	char* respone_message[BUFFER_SIZE] = {};

	client_t *cli = (client_t *)arg;

	//file temp name
	sprintf(filename, "%d", cli->sockfd);
	strcat(filename,".png");
	printf("%s\n",filename);


	//create file
	fp = fopen(filename, "w");


	// Receiving the file
	while(1){
		if (recv(cli->sockfd, buffer, SIZE, 0) > 0){
			//EOF
			if (strcmp(buffer,"File finish") == 0 ){
				break;
			}
			fwrite(buffer, sizeof(char), SIZE, fp);
    	bzero(buffer, SIZE);
			//send the file finish
  		send(cli->sockfd, "c", sizeof("c"), 0);
		} 
		else{
			break;
		}
	}

	// File close
	fclose(fp);
	
	// send the file to Image Server
	send_Image(filename,cli->sockfd);
	
	//delete the temp file from disk
	if (remove(filename) == 0)
		printf("Deleted successfully\n");

	//close the soket and free memory and thread
	close(cli->sockfd);
  free(cli);
  pthread_detach(pthread_self());

	return NULL;
}


void send_Image(char *filename,int client){
	pthread_mutex_lock(&mutex);

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
    return;
  }
	

	/* Image Socket settings */
  printf("Image Server socket created successfully.\n");
	bzero(&server_addr, sizeof(server_addr)); 			        // Clearing the struct
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
  server_addr.sin_port = htons(PORT_SERVER);

	// Connect to Server
  err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
		printf("ERROR: connect\n");
		return;
	}
  printf("Connected to Image Server.\n");


	 // Open the file
  fp = fopen(filename, "rb");
  if (fp == NULL) {
    perror("Error in reading file.");
    return;
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

	//send the file finish
  send(sockfd, "File finish", sizeof("File finish"), 0);

  //Waiting for a response from the server
  while (1) {
    int receive = recv(sockfd, respone_message, BUFFER_SIZE, 0);

    if (receive > 0) {
      printf("\n\tThe server response\n");
      printf("------------------------------------\n");
      printf("  %s\n", respone_message);
      printf("------------------------------------\n\n");
			break;
    } 
    else {
      break;
    }
    memset(respone_message, 0, sizeof(respone_message));
  }

	//send to client respone message from image server
	send(client, respone_message, sizeof(respone_message), 0);

	//close socket and file close

	fclose(fp);
	close(sockfd);
	printf("Closing the connection.\n");
	pthread_mutex_unlock(&mutex);
}