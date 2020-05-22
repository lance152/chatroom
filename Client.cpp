#include <iostream>

#include "Client.h"

using namespace std;

Client::Client():
  sock(0),
  pid(0),
  epfd(0){
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
}

void Client::Connect(){
  //1. 创建socket
  //2. 连接服务端
  //3. 创建pipe，fd[0]父进程，fd[1]子进程
  //4. 创建epoll
  //5. 将sock及pipe读端加入epoll监听
  sock = socket(AF_INET,SOCK_STREAM,0);
  if(sock<0){
    perror("socket error");
    exit(-1);
  }

  if(connect(sock,(struct sockaddr *)&serverAddr,sizeof(serverAddr))<0){
    perror("connect error");
    exit(-1);
  }

  if(pipe(pipe_fd)<0){
    perror("pipe error");
    exit(-1);
  }

  epfd = epoll_create(EPOLL_SIZE);

  if(epfd<0){
    perror("epoll error");
    exit(-1);
  }

  addfd(epfd,sock,true);
  addfd(epfd,pipe_fd[0],true);
}

void Client::Close(){
  if(pid){
    //父进程
    close(pipe_fd[0]);
    close(sock);
  }else{
    //子进程
    close(pipe_fd[1]);
  }
}

void Client::Start(){
  //1. 连接服务器 connect()
  //2. 创建子进程 fork()
  //2.1 子进程负责收集用户输入的消息并写入管道
  //2.2 父进程负责读取管道数据及epoll事件
  //2.2.1 获取就绪事件 epoll_wait()
  //处理就绪事件

  //epoll events 队列
  struct epoll_events events[2]; //只有两个事件

  Connect();

  pid = fork();

  if(pid<0){
    //子进程创建失败
    perror("fork error");
    close(sock);
    exit(-1);
  }else if(pid==0){
    //子进程
    //因为pipe在这之前就创建了，子进程中有两个文件描述符分别对应pipe的读端和写端
    //因为子进程只负责向管道写入数据，所以要关闭pipe的读端
    close(pipe_fd[0]);

    cout<<"Enter 'EXIT' to quit chatroom"<<endl;

    while(isClientAlive){
      bzero(message,BUF_SIZE);
      fgets(message,BUF_SIZE,stdin);

      if(strncmp(message,EXIT,strlen(EXIT)==0)){
        //用户选择退出chatroom
        isClientAlive = 0;
      }else{
        //用户输入消息，写入pipe
        if（write(pipe_fd[1],message,strlen(message)-1)<0){
          perror("fork error");
          exit(-1);
        }
      }
    }
  }else{
    //父进程
    //父子进程中有两个文件描述符分别对应pipe的读端和写端
    //因为父进程只负责向管道读取数据，所以要关闭pipe的写端
    close(pipe_fd[1]);

    while(isClientAlive){
      int events_count = epoll_wait(epfd,events,2,-1);

      for(int i=0;i<events_count;i++){
        bzero(message,BUF_SIZE);

        if(events[i].data.fd==sock){
          //服务器发来的消息
          int ret = recv(sock,message,BUF_SIZE,0);

          if(ret==0){
            cout<<"Server closed connection: "<<sock<<endl;
            close(sock);
            isClientAlive = 0;
          }else{
            cout<<message<<endl;
          }
        }else{
          //从pipe来的消息
          int ret = read(events[i].data.fd,message,BUF_SIZE);

          if(ret==0){
            isClientAlive = 0;
          }else{
            send(sock,message,BUF_SIZE,0);
          }
        }
      }
    }
  }

  Close();
}
