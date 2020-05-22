#ifndef _CHATROOM_CLIENT_H_
#define _CHATROOM_CLIENT_H_

#include <string>

#include "Common.h"

using namespace std;

class CLient{
public:
  //无参数构造
  Client();

  //连接服务器
  void Connect();

  //断开连接
  void Close();

  //启动客户端
  void Start();

private:

  //当前连接服务器端创建的socket
  int sock;

  //当前进程ID
  int pid;

  //epoll_create创建后的返回值(也就是epoll的文件描述符)
  int epfd;

  //创建管道
  int pipe_fd[2];//pipe_fd[0]用于父进程读，pipe_fd[1]用于子进程写

  //表示客户端是否active
  bool isClientAlive;

  //缓冲区
  char message[BUF_SIZE];

  //socket中的参数
  struct sockaddr_in serverAddr;
};

#endif
