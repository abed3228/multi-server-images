 
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


#define MAX_CLIENTS 1
#define BUFFER_SZ 2048
#define PORT 3228 
#define SIZE 1024
#define BUFFER_SIZE 2048






int main(int argc, char **argv){
	int listenfd = 0;
	int sockfd = 0;
	char file_loc[BUFFER_SIZE];

	struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  pthread_t tid;
	
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

	printf("=== server image active  ===\n");


	while (1){
		FILE *fp;
	 	char filename[SIZE];
		char buffer[SIZE];
		char respone_message[SIZE];
		char file_path[BUFFER_SIZE];

		socklen_t clilen = sizeof(cli_addr);
		sockfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
	


		// Receiving the file
		while(1){
			
			bzero(filename, SIZE);
			if (recv(sockfd, filename, SIZE, 0) <= 0){
				break;
			}
			else{
				bzero(buffer, SIZE);
				if (recv(sockfd, buffer, SIZE, 0) > 0){
					printf("%s\n",filename);
					if (strcmp(buffer,"File finish") == 0 ){
							strcpy(respone_message,"Image saved successfully");
							send(sockfd, respone_message, sizeof(respone_message), 0);
							memset(respone_message, 0, sizeof(respone_message));
					}
					else{
						strcpy(file_path,"./Image/");
						strcat(file_path,filename);
						if( access( file_path, F_OK ) == 0 ) {
							// file exists
							fp = fopen(file_path,"a");
							fwrite(buffer, sizeof(char), SIZE, fp);
							fclose(fp);
						} 
						else{
							fp = fopen(file_path,"w");
							fwrite(buffer, sizeof(char), SIZE, fp);
							fclose(fp);
						}
						send(sockfd, "c", sizeof("c"), 0);
					}
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
