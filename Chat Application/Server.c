// Server.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#define MAXREQ 30
#define MAXQUEUE 5
#define MAXGRPSIZE 10


struct user{
  char u_ID[20];
  int socketfd;
  struct user *next;
};

//Linked List of all the users
struct user *Users;

struct group{
  char grp_ID[20];
  int socketfd[MAXGRPSIZE];
  int size;
  struct group * next;
};

//Linked List of all the groups
struct group *Groups;


int getlen(char* buf){

  int len=0;
  for(int i=12;i<16;i++){
    if(buf[i]=='Y')
      break;
    len=len*10+(buf[i]-'0');
  }
  return len;
}
char *getmsgtype(char* buf){

  int count=0;
  for(int i=0;i<12;i++){
    if(buf[i]=='Y')
      break;
    count++;
  }
  char *type=malloc(count);
  for(int i=0;i<count;i++){
    type[i]=buf[i];
  }
  return type;
}
char *getmsg(char* buf,int len){

  char *msg=malloc(len);
  for(int i=16;i<16+len;i++){
    msg[i-16]=buf[i];
  }
  return msg;
}
void addUser(char* userId,int sockfd){

  if(Users==NULL){
    Users= (struct user*)malloc(sizeof(struct user));
    for(int i=0;i<strlen(userId);i++){
      Users->u_ID[i]=userId[i];
    }
    Users->socketfd=sockfd;
    Users->next=NULL;
  }
  else{
    struct user *temp=Users;
    while(temp->next!=NULL){
      temp=temp->next;
    }
    temp->next=(struct user*)malloc(sizeof(struct user));
    temp=temp->next;
    for(int i=0;i<strlen(userId);i++){
      temp->u_ID[i]=userId[i];
    }
    temp->socketfd=sockfd;
    temp->next=NULL;
  }
}
void addGrp(char* GrpId,int sockfd){
  if(Groups==NULL){
    Groups= (struct group*)malloc(sizeof(struct group));
    for(int i=0;i<strlen(GrpId);i++){
      Groups->grp_ID[i]=GrpId[i];
    }
    Groups->socketfd[0]=sockfd;
    Groups->size=1;
    Groups->next=NULL;
  }
  else{
    struct group *temp=Groups;
    while(temp->next!=NULL){
      temp=temp->next;
    }
    temp->next=(struct group*)malloc(sizeof(struct group));
    temp=temp->next;
    for(int i=0;i<strlen(GrpId);i++){
      temp->grp_ID[i]=GrpId[i];
    }
    temp->socketfd[0]=sockfd;
    temp->size=1;
    temp->next=NULL;
  }
}

struct group *findgrp(char * Grp_ID){
  struct group *temp= Groups;
  while(temp!=NULL){
    if(!strcmp(temp->grp_ID,Grp_ID))
      return temp;
    temp=temp->next;
  }
  return temp;
}
struct user *finduser(char* userId){
  struct user *temp= Users;
  while(temp!=NULL){
    if(!strcmp(temp->u_ID,userId))
      return temp;
    temp=temp->next;
  }
  return temp;
}
struct user *finduserfrmsock(int socketfd){
  struct user *temp= Users;
  while(temp!=NULL){
    if(temp->socketfd==socketfd)
      return temp;
    temp=temp->next;
  }
  return temp;
}
int addUsrToGrp(char* msg){
  char group_id[10],user_id[10];
  int count=0;
  while(msg[count]!=','){
    user_id[count]=msg[count];
    count++;
  }
  user_id[count]='\0';
  struct user *S=finduser(user_id);
  for(int i = count+1; i < strlen(msg); i++) {
    group_id[i-count-1]=msg[i];
  }
  group_id[strlen(msg)-count-1]='\0';
  struct group *G=findgrp(group_id);
  if(G==NULL)
    return 2;
  if(G->size==MAXGRPSIZE)
    return 0;
  if(S==NULL)
    return 3;
  G->socketfd[G->size]=S->socketfd;
  G->size=G->size+1;
  printf("%d\n",G->size);
  printf("-------%s added to the group %s-------\n",user_id,group_id);
  return 1;
}
void* server(void *fd) {
  int *d=fd;
  int consockfd=*d;
  char reqbuf[MAXREQ];
  int n;
  while (1) {                   
    memset(reqbuf,0, MAXREQ);
  
    n = read(consockfd,reqbuf,MAXREQ-1); /* Recv */


    int lenOfPayload= getlen(reqbuf);
    char* msgtype=getmsgtype(reqbuf);
    char* msg=getmsg(reqbuf,lenOfPayload);
    if(!strcmp(msgtype,"myId")){
      char Prompt[]="----------Connects to the server , bootstrap Done----------\n";
      addUser(msg,consockfd);
      write(consockfd,Prompt,strlen(Prompt));
      printf("-------%s connected-------\n",msg);
    }
    else if(!strcmp(msgtype,"addGrp")){
      char Prompt[]="----------Group succesfully created----------\n";
      addGrp(msg,consockfd);
      write(consockfd,Prompt,strlen(Prompt));
      printf("-------%s group created-------\n",msg);
    }
    else if(!strcmp(msgtype,"addUsrToGrp")){
      char Prompt[]="----------User successfully added----------\n";
      int check=addUsrToGrp(msg);
      if(check==1){
        write(consockfd,Prompt,strlen(Prompt));
      }
      if(check==0){
        char M[]="----------Error:Group is full----------\n";
        write(consockfd,M,strlen(M));
        printf("%s",M);
      }
      if(check==2){
        char M[]="----------Error:Group Not Found----------\n";
        write(consockfd,M,strlen(M));
        printf("%s",M);
      }
      if(check==3){
        char M[]="----------Error:User not found----------\n";
        write(consockfd,M,strlen(M));
        printf("%s",M);
      }
    }
    else{
    
      char Prompt[]="----------Message Send----------\n";
      struct group *Grp=findgrp(msgtype);
      if(Grp!=NULL){
        printf("Group Chat\n");
        printf("%s\n",msg);
        struct user *S=finduserfrmsock(consockfd);
        char message[]="Message on the Group: ";
        strcat(message,Grp->grp_ID);
        strcat(message," by ");
        strcat(message,S->u_ID);
        strcat(message,"\n");
        strcat(message,msg);
        strcat(message,"\n");
        for(int i=0;i<Grp->size;i++){
          write(Grp->socketfd[i],message,strlen(message));
        }
        write(consockfd,Prompt,strlen(Prompt));
      }
      else{
        struct user *R=finduser(msgtype);
        if(R!=NULL){
          printf("Personal Chat\n");
          struct user *S=finduserfrmsock(consockfd);
          char message[]="Message from the User: ";
          strcat(message,S->u_ID);
          strcat(message,"\n");
          strcat(message,msg);
          strcat(message,"\n");
          write(R->socketfd,message,strlen(message));
          write(consockfd,Prompt,strlen(Prompt));
        }
      }
    }
  }
  free(Users);
  free(Groups);
}

int main() {

int lstnsockfd, consockfd, clilen, portno = 5033;
struct sockaddr_in serv_addr, cli_addr;

 memset((char *) &serv_addr,0, sizeof(serv_addr));
 serv_addr.sin_family      = AF_INET;
 serv_addr.sin_addr.s_addr = INADDR_ANY;
 serv_addr.sin_port        = htons(portno);

// Server protocol
/* Create Socket to receive requests*/
lstnsockfd = socket(AF_INET, SOCK_STREAM, 0);
Groups=NULL;
Users=NULL;
/* Bind socket to port */
bind(lstnsockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr));
printf("Bounded to port\n");
pthread_t thread_id[20];
int threads=0;
while (1) {
   printf("Listening for incoming connections\n");

/* Listen for incoming connections */
   listen(lstnsockfd, MAXQUEUE); 


/* Accept incoming connection, obtaining a new socket for it */
   consockfd = accept(lstnsockfd, (struct sockaddr *) &cli_addr,&clilen);
   printf("Accepted connection %d\n",consockfd);
   printf("Thread created\n");
   pthread_create(&thread_id[threads], NULL, server, &consockfd);
   threads++;
  }
close(lstnsockfd);
}
