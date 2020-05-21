#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

int main(void){
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(1234);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");//将socket和ip，端口绑定

  int sock = socket(AF_INET,SOCK_STREAM,0);
  if(sock<0){
    perror("connect error");
    exit(-1);
  }
  connect(sock,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
  char buffer[40];
  read(sock,buffer,sizeof(buffer)-1);

  printf("Message from server:%s \n",buffer);
  close(sock);

  return 0;
}
