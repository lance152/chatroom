#ifndef _CHATROOM_COMMON_H_
#define _CHATROOM_COMMON_H_

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//默认服务器端IP地址
#define SERVER_IP "127.0.0.1"

//端口号
#define SERVER_PORT 8888

//epoll支持的最大句柄数
#define EPOLL_SIZE 5000

//缓冲区大小(65536)
#define BUF_SIZE 0xFFFF

//新用户登录欢迎信息
#define WELCOME "Welcome you join to the chat room! Your char ID is: Client #%d" //这里#表示给%d加上双引号

//其他用户收到的消息前缀
#define SERVER_MESSAGE "ClientID %d say >> %s"

//退出系统
#define EXIT "EXIT"

//提醒你是聊天室中的唯一用户
#define CAUTION "There is only one person in the chat room"

//定义一个函数用来向epoll中添加新的监听事件
//这里用一个参数来设置是否开启ET模式（默认开启）
//ET（edge trigger）表示epoll只会提示内核一次该文件描述符上有可读写事件
//LT （Level trigger） 表示epoll会一直提示内核知道该文件描述符上没有可读写事件
static void addfd(int epollfd,int fd,bool enable_et){
  struct epoll_event ev;
  ev.data.fd = fd;
  ev.events = EPOLLIN; //提示内核该文件描述符可以读
  if(enable_et){
    ev.events = EPOLLIN | EPOLLET;
  }
  epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);

  //设置fd为非阻塞模式
  fcntl(fd,F_SETFL,fcntl(fd,F_GETFD,0) | O_NONBLOCK);
  printf("fd added to epoll!\n\n");
}
//这里定义这个函数是因为client和server都会用到

#endif
