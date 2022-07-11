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

#define SIZE 1024



int main(int argc, char **argv){


  FILE *read;
  int status;
  char data[SIZE] = {0};
  FILE *write;


  if(argc <2){
    printf("Error - Please use the following method\n");
    printf("./client.exe FilePath\n");
    return EXIT_FAILURE;
  }

  //file exists
  if( access( argv[1], F_OK ) != 0 ) {
    // file not exists
    printf("File: %s\n",argv[1]);
    printf("File not exists\n");
    return EXIT_FAILURE;
  } 

  
  write = fopen("1.text", "wb");
  if (!write) {
    perror("Error in reading file.");
    return EXIT_FAILURE;
  }

  read = fopen(argv[1], "rb");
  if (!read) {
    perror("Error in reading file.");
    return EXIT_FAILURE;
  }
  



  printf("Start send file\n\r");
  while(fgets(data, SIZE, read) != NULL) {
    fprintf(write, "%s", data);
    bzero(data, SIZE);
  }

 
  fclose(read);
  fclose(write);

	return EXIT_SUCCESS;
}




