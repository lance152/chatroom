#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(void){
  int listener = socket(AF_INET,SOCK_STREAM,0);//1.创建socket
  if(listener<0){
    perror("socket create error");
    exit(-1);
  }
  printf("listen socket create \n");

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(1234);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");//将socket和ip，端口绑定

  if(bind(listener,(struct sockaddr *)&serverAddr,sizeof(serverAddr))<0){//2. bind()
    perror("bind error");
    exit(-1);
  }
  printf("bind successfully");

  int ret = listen(listener,5);//3. 监听端口

  //接受客户端连接
  struct sockaddr_in clinetAddr;
  socklen_t clientAddr_size = sizeof(clientAddr);
  int clientSock = accept(listener,(struct sockaddr*)&clientAddr,&clientAddr_size);

  char str[] = "HELLO WORLD!";
  write(clientSock,str,sizeof(str));

  close(clientSock);
  close(listener);
  return 0;
}
