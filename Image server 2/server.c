 
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
void send_Image(char file_name[],char buf[]);
int image_sockfd = -1;

int main(int argc, char **argv){
	int listenfd = 0;
	int connfd = 0;
	struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;

  pthread_t tid;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	//image server
	struct sockaddr_in image_addr;
	int err;

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

	/* Socket settings */
	image_sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(image_sockfd < 0) {
    perror("Error in socket");
    return EXIT_FAILURE;
  }
	/* Image Socket settings */
  printf("Image Server socket created successfully.\n");
	bzero(&image_addr, sizeof(image_addr)); 			        // Clearing the struct
  image_addr.sin_family = AF_INET;
  image_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
  image_addr.sin_port = htons(PORT_SERVER);

	// Connect to Server
  err = connect(image_sockfd, (struct sockaddr *)&image_addr, sizeof(image_addr));
  if (err == -1) {
		printf("ERROR: connect\n");
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


	close(image_sockfd);
	printf("Closing the connection.\n");
	return EXIT_SUCCESS;
}

void *handle_client(void *arg){
  char filename[SIZE];
  char buffer[SIZE];
	char respone_message[BUFFER_SIZE];

	client_t *cli = (client_t *)arg;

	printf("sokect - %d\n",cli->sockfd);
	
	// Receiving the file
	while(1){
		int exit = 0;
		bzero(filename, SIZE);
		bzero(buffer, SIZE);
		if (recv(cli->sockfd, filename, SIZE, 0) > 0){
			if (recv(cli->sockfd, buffer, SIZE, 0) > 0){
				if (strcmp(buffer,"File finish") == 0 ){
					exit = 1;
				}
				// send the file to Image Server
				send_Image(filename,buffer);
				//send the file finish
				bzero(buffer, SIZE);
				while (1){
					if (recv(image_sockfd, buffer, SIZE, 0) > 0){
						send(cli->sockfd, buffer, sizeof(buffer), 0);
						break;
					}
				}
			}
		}
		if (exit){
			break;
		}
		
	}	

	//close the soket and free memory and thread
	close(cli->sockfd);
	printf("sokect %d close\n",cli->sockfd);
  free(cli);
  pthread_detach(pthread_self());
	return NULL;
}


void send_Image(char file_name[],char buf[]){
	pthread_mutex_lock(&mutex);
	if (send(image_sockfd, file_name, SIZE, 0) == -1) {
		printf("error2\n");
		return;
	}
 	if (send(image_sockfd, buf, SIZE, 0) == -1) {
		 printf("error3\n");
		 return;
	}
	pthread_mutex_unlock(&mutex);
}