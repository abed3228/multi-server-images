 
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



/* Client structure */
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
} client_t;



int main(int argc, char **argv){
	int option = 1;
	int listenfd = 0;
	int connfd = 0;
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
		int leave_flag = 0;
		FILE *fp;
		char filename[BUFFER_SZ];
		char buffer[SIZE];
		char data[SIZE] = {0};

		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
		/* Client settings */
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;

		srand(time(NULL));   // Initialization, should only be called once.
		sprintf(filename, "%d", rand());
		strcat(filename,".png");
		printf("%s\n",filename);

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
			} 
			else{
				break;
			}
		}

		//res to server
		strcpy(data,"Image saved successfully");
		if (send(cli->sockfd, data, sizeof(data), 0) == -1) {
      return EXIT_FAILURE;
    }
		printf("%s\n",data);


		//Set in Image Loc
		strcpy(file_loc,"./Image/");
		strcat(file_loc,filename);
		printf("%s\n",file_loc);
		rename(filename,file_loc);


		//free memory
		free(cli);
	}

	return EXIT_SUCCESS;
}
