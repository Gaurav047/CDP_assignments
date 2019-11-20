// Client.c
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#define MAXIN 20
#define MAXOUT 100


char *getreq(char *inbuf, int len) {
  /* Get request char stream */
  printf("REQ: ");              /* prompt */
  memset(inbuf,0,len);          /* clear for good measure */
  return fgets(inbuf,len,stdin); /* read up to a EOL */
}

void* client(void* fd) {
  int *d=fd;
  int sockfd=*d;
  int n;
  char rcvbuf[MAXOUT];
  while (1) {
    //printf("Wrote message: %s\n",sndbuf);
    
    memset(rcvbuf,0,MAXOUT);               /* clear */
    n=read(sockfd, rcvbuf, MAXOUT-1);      /* receive */
    // printf("Received reply: %d",n);
    write(STDOUT_FILENO, rcvbuf, n);	      /* echo */
  }
}
char *make_header(char* msgtype,int lenOfPayload){
  char* header = malloc(20*sizeof(char));
  int count;
  // Append msgType to header
  for(count=0;count<strlen(msgtype);count++){
    header[count]=msgtype[count];
  }

  // Append Y if header length is Less than 12
  while(count<12){
    header[count]='Y';
    count++;
  }
  
  // convert length of payload to char array
  int tc=0;
  char temp[4];
  while(lenOfPayload!=0){
    char t = lenOfPayload%10 +'0';
    lenOfPayload=lenOfPayload/10;
    temp[tc]=t;
    tc++;
  }

  // Append the the length in char array form to header
  for(int i=tc-1;i>=0;i--){
    header[count]=temp[i];
    count++;
  }

  // If Message length is less than 16 than append 'Y'
  while(count<16){
    header[count]='Y';
    count++;
  }

  return header;
}

// Server address
struct hostent *buildServerAddr(struct sockaddr_in *serv_addr,
	char *serverIP, int portno) {
  /* Construct an address for remote server */
  memset((char *) serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr->sin_family = AF_INET;
  inet_aton(serverIP, &(serv_addr->sin_addr));
  serv_addr->sin_port = htons(portno);
 }


int main() {
	//Client protocol
	char *serverIP = "127.0.0.1";
	int sockfd, portno = 5033;
	struct sockaddr_in serv_addr;
	printf("server address is built\n");
	buildServerAddr(&serv_addr, serverIP, portno);

	/* Create a TCP socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
  printf("socketfd is %d\n",sockfd);
	/* Connect to server on port */
	connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	printf("Connected to %s:%d\n",serverIP, portno);
  int lenOfPayload;
	while(1){
    char msgtype[20],msg[30];
    scanf("%s %s",msgtype,msg);
    // printf("%s %s %ld\n",msgtype,msg,strlen(msg));

    int lenOfPayload=strlen(msg);
    char *header=make_header(msgtype,lenOfPayload);

    char *full_msg;
    if((full_msg=(char*)malloc(strlen(header)+strlen(msg)+1))!=NULL){
      full_msg[0]='\0';
      strcat(full_msg,header);
      strcat(full_msg,msg);
    }
    else{
      printf("Memory is full, malloc failed\n");
      return 0;
    }

    if(!strcmp(msgtype,"myId")){

      pthread_t thread_id;
      write(sockfd,full_msg,strlen(full_msg));
      /* Carry out Client-Server protocol */
      pthread_create(&thread_id, NULL, client, &sockfd);
    }
    else if(!strcmp(msgtype,"addGrp")){
      write(sockfd,full_msg,strlen(full_msg));
    }
    else if(!strcmp(msgtype,"addUsrToGrp")){
      write(sockfd,full_msg,strlen(full_msg));
    }
    else{
      write(sockfd,full_msg,strlen(full_msg));
    }
  }

	/* Clean up on termination */
	close(sockfd);
}