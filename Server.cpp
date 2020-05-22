#include <iostream>

#include "Server.h"

using namespace std;

//构造函数，初始化各个参数
Server::Server():
  listener(0),
  epfd(0){
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
}

void Server::Init(){
  //1. 创建socket socket（）
  //2. 绑定地址 bind（）
  //3. 监听连接 listen（）
  //4. 创建epoll事件表 epoll_create()
  //5. 添加监听epoll事件
  cout<<"Init Server..."<<endl;
  listener = socket(AF_INET,SOCK_STREAM,0);
  if(listener<0){
    perror("socket create error");
    exit(-1);
  }
  printf("listen socket created \n");

  if(bind(listener,(struct sockaddr *)&serverAddr,sizeof(serverAddr))<0){
    perror("bind error");
    exit(-1);
  }

  int ret = listen(listener,5);
  if(ret < 0){
    perror("listen error");
    exit(-1);
  }

  cout<<"Start to listen: "<<SERVER_IP<<": "<<SERVER_PORT<<endl;

  epfd = epoll_create(EPOLL_SIZE);

  if(epfd<0){
    perror("epoll creat error");
    exit(-1);
  }

  addfd(epfd,listener,true);
}

void Server::Close(){
  //关闭socket
  close(listener);
  //关闭epoll
  close(epfd);
}

int Server::SendBroadcastMessage(int clientfd){
  //1. 接受新消息 recv（）
  //2. 判断是否客户端中止连接
  //3. 判断是否chatroom中还有其他用户
  //4. 格式化发送的消息内容
  //5. 遍历客户端列表依次发送消息
  char buf[BUF_SIZE], message[BUF_SIZE];
  bzero(buf,BUF_SIZE);
  bzero(message,BUF_SIZE);

  cout<<"read from client (ClientId ="<<clientfd<<")"<<endl;
  int len = recv(clientfd,buf,BUF_SIZE,0);

  if(len==0){
    //如果客户端关闭了连接
    close(clientfd);

    //在clients_lists中删除该客户端
    clients_list.remove(clientfd);
    cout<<"ClientID = "<<clientfd<<" closed.\n Now there are "
    <<clients_list.size()<<" clients in the chatroom"<<endl;
  }else{
    //客户端没有中止连接
    if(clients_list.size()==1){
      //chatroom中只有这一个用户
      send(clientfd,CAUTION,strlen(CAUTION),0);
      return len;
    }
    //否则需要向所有其他的用户发送消息
    //格式化消息
    sprintf(message,SERVER_MESSAGE,clientfd,buf);

    //遍历client列表
    for(auto it=clients_list.begin();it!=clients_list.end();it++){
      if(*it!=clientfd){
        if(send(*it,message,BUF_SIZE,0)<0){
          cout<<"Message send to client "<<*it<<" error"<<endl;
          return -1;
        }
      }
    }
  }
  return len;
}

void Server::Start(){
  //1.初始化服务器
  //2. 进入主循环
  //3. 获取就绪事件 epoll_wait()
  //4. 循环处理所有就绪事件
  //4.1 如果是新的用户，则接受连接并添加epoll fd accept(),addfd()
  //4.2 如果是新消息，则广播给其他客户端

  Init();
  struct epoll_event events[EPOLL_SIZE]; //epoll事件队列
  while(1){
    int events_count = epoll_wait(epfd,events,EPOLL_SIZE,-1);
    if(events_count<0){
      perror("epoll failure");
      break;
    }
    cout<<"epoll events count = "<<events_count<<endl;
    for(int i=0;i<events_count;i++){
      int sockfd = events[i].data.fd;

      if(sockfd == listener){
        //新用户连接
        struct sockaddr_in clientAddr;
        socklen_t clientAddr_size = sizeof(clientAddr);
        int clientfd = accept(listener,(struct sockaddr* )&clientAddr,&clientAddr_size);

        cout<<"client connection from: "<<inet_ntoa(clientAddr.sin_addr)<<":"<<ntohs(clientAddr.sin_port)
        <<", clientfd = "<<clientfd<<endl;

        addfd(epfd,clientfd,true);

        //加入clients_lists
        clients_list.push_back(clientfd);
        cout<<"Add new clientfd = "<<clientfd<<" to epoll"<<endl;
        cout<<"Now there are "<<clients_list.size()<<" clients in the chat room"<<endl;

        //服务器发送欢迎信息
        cout<<"Welcome message"<<endl;
        char message[BUF_SIZE];
        bzero(message,BUF_SIZE);
        sprintf(message,WELCOME,clientfd);
        int ret = send(clientfd,message,BUF_SIZE,0);
        if(ret<0){
          perror("send error");
          Close();
          exit(-1);
        }
      }else{
        //有新消息
        int ret = SendBroadcastMessage(sockfd);
        if(ret<0){
          perror("message send error");
          Close();
          exit(-1);
        }
      }
    }
  }
  Close();
}
