#include <arpa/inet.h>
#include <sys/select.h>
/* According to earlier standards */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "wrap.h"

static const int SERV_PORT = 6666;

int main() {
  int maxfd, maxi, i, j, n;
  int nready, opt;
  int clients[FD_SETSIZE];
  int connfd, listenfd, peerfd;
  char buf[BUFSIZ], str[INET_ADDRSTRLEN];
  // sockaddr_in  主要用于不同主机之间的socket编程；
  // sockaddr_un  主要用于同一个主机中的本地Local socket
  struct sockaddr_in serv_addr, peer_addr;
  socklen_t peer_addr_len;
  fd_set rset, allset;

  listenfd = Socket(AF_INET, SOCK_STREAM, 0);

  opt = 1;  // 1表示on，0表示off
  // man 7 socket
  // 设置允许地址重用
  // 先调用close的一端会进入TIME_WAIT状态，可能要等待2min，在此期间端口不得重复绑定使用
  // 这可能发生在服务器重启等情况
  // 服务器应该总是设置SO_REUSEADDR，以确保端口能够在socket关闭之后立即能够使用
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(SERV_PORT);

  Bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  Listen(listenfd, 128);

  maxi = -1;  // maxi is the maximum index of the clients
  // clients reserve the conneted socket
  for (i = 0; i < FD_SETSIZE; i++) clients[i] = -1;

  maxfd = listenfd;  // At beginning, the maximum fd is listen fd

  FD_ZERO(&allset);
  FD_SET(listenfd, &allset);

  while (1) {
    rset = allset;
    // wait forever, blocked
    nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
    if (nready < 0) perr_exit("select error");

    // accept socket
    // 同一时刻listenfd可能收到多个请求连接socket，这些请求将在内核中排队
    // 可以通过多次调用select获取
    if (FD_ISSET(listenfd, &rset)) {
      // accept第三个参数是value-return类型
      peer_addr_len = sizeof(peer_addr_len);
      connfd = Accept(listenfd, (struct sockaddr *)&peer_addr, &peer_addr_len);
      // inet_ntop将二进制形式IP转化成人类可读的字符串形式
      printf("received from %s at port %d\n",
             inet_ntop(AF_INET, &peer_addr.sin_addr, str, sizeof(str)),
             ntohs(peer_addr.sin_port));

      for (i = 0; i < FD_SETSIZE; i++)
        if (clients[i] < 0) {
          clients[i] = connfd;
          break;
        }

      if (i >= FD_SETSIZE) {
        fputs("too many clients\n", stderr);
        break;
      }

      if (connfd > maxfd) maxfd = connfd;  // parameter for select

      FD_SET(connfd, &allset);

      if (i > maxi) maxi = i;  // maxi指向最后的有效fd

      if (--nready <= 0)  // have processed one ready fd
        continue;
    }

    // process other type fds except from accepting requests
    // 避免轮询1024个fd
    for (i = 0; i <= maxi; i++) {
      if ((peerfd = clients[i]) < 0) continue;

      if (FD_ISSET(peerfd, &rset)) {
        if ((n = Read(peerfd, buf, sizeof(buf))) ==
            0) {  // socket was closed by the peer
          Close(peerfd);
          FD_CLR(peerfd, &allset);
          clients[i] = -1;
          printf("clients[%d] has been closed\n", i);
        } else if (n > 0) {
          for (j = 0; j < n; j++) buf[j] = toupper(buf[j]);
          Write(peerfd, buf, n);  // TODO 写阻塞怎么办？
          Write(STDOUT_FILENO, buf, n);
        } else {
          perr_exit("Read error");
        }
        if (--nready <= 0)  // have processed one ready fd
          break;            // jump out of for, but still in while
      }
    }
  }

  Close(listenfd);

  return 0;
}
