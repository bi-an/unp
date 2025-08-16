#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "wrap.h"

static const int SERV_PORT = 7777;
static const int OPEN_MAX = 1024;
static const int MAX_LINE = 80;

// FIXME 这个程序有一个很奇怪的现象：
// 2个客户连入，第1个客户发送数据之后，第2个客户发送数据之时，poll会返回，但是clients[i].revents却始终为0
// 导致无法处理poll事件，陷入无限poll返回
int main() {
  struct sockaddr_in serv_addr, cli_addr;
  socklen_t addr_len;
  int listenfd, connfd, sockfd, opt;
  int maxi, i, j, nready;
  ssize_t n;
  char buf[MAX_LINE], str[INET_ADDRSTRLEN];
  struct pollfd clients[OPEN_MAX];

  listenfd = Socket(AF_INET, SOCK_STREAM, 0);

  // set SO_REUSEADDR = on
  opt = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(SERV_PORT);

  Bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  Listen(listenfd, 128);

  clients[0].fd = listenfd; /* 第一个监听文件描述符设置为listenfd */
  clients[0].events = POLLIN; /* 监听listenfd的普通读事件 */

  for (i = 1; i < OPEN_MAX; i++) clients[i].fd = -1;
  maxi = 0;

  while (1) {
    nready = poll(clients, maxi + 1, -1);  // 阻塞等待

    if (clients[0].revents & POLLIN) {
      addr_len = sizeof(cli_addr);
      connfd = Accept(listenfd, (struct sockaddr *)&cli_addr, &addr_len);
      printf("Accept from %s at PORT %d\n",
             inet_ntop(AF_INET, &cli_addr, str, sizeof(str)),
             ntohs(cli_addr.sin_port));
      for (i = 1; i < OPEN_MAX; i++)
        if (clients[i].fd < 0) {
          clients[i].fd = connfd;
          break;
        }
      if (i >= OPEN_MAX) perr_exit("too many connections");
      clients[i].events = POLLIN;  // 监听读事件

      if (i > maxi) maxi = i;
      if (--nready <= 0) continue;
    }

    for (i = 1; i <= maxi; i++) {
      if ((sockfd = clients[i].fd) < 0) continue;

      if (clients[i].revents & POLLIN) {
        n = Read(sockfd, buf, sizeof(buf));
        if (n == 0) {
          printf("clients[%d] closed the connection\n", i);
          Close(sockfd);
          clients[i].fd = -1;
        } else if (n < 0) {
          if (errno == ECONNRESET) {
            printf("clients[%d] aborted connetion\n", i);
            Close(sockfd);
            clients[i].fd = -1;
          } else {
            perr_exit("Read error");
          }
        } else {
          for (j = 0; j < n; j++) buf[j] = toupper(buf[j]);
          Writen(sockfd, buf, n);
        }
        // 处理一个POLLIN事件（处理方式是read），nready--
        if (--nready <= 0) break; /* 跳出for，还在while中 */
      }
    }
  }

  Close(listenfd);

  return 0;
}
