#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "wrap.h"

static const int SERV_PORT = 8888;
static const int OPEN_MAX = 5000;
static const int MAX_LINE = 80;

int main() {
  struct sockaddr_in serv_addr, cli_addr;
  struct epoll_event events[OPEN_MAX], ev;
  int efd, sockfd, listenfd, connfd, i, j, maxi, opt, nready;
  ssize_t n;
  socklen_t cli_len;
  char buf[MAX_LINE], str[INET_ADDRSTRLEN];

  listenfd = Socket(AF_INET, SOCK_STREAM, 0);

  opt = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt,
             sizeof(opt)); /* 端口复用 */

  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(SERV_PORT);

  Bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  Listen(listenfd, 128);

  efd = epoll_create(OPEN_MAX);  // 参数已经废弃了
  if (efd < 0) perr_exit("epoll_create error");

  ev.events = EPOLLIN;
  ev.data.fd = listenfd;
  if (epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &ev) < 0)
    perr_exit("cannot add listenfd, epoll_ctl error");

  while (1) {
    nready = epoll_wait(efd, events, OPEN_MAX, -1);
    if (nready == 0)
      continue;
    else if (nready < 0)
      perr_exit("epoll_wait error");

    for (i = 0; i < nready; i++) {
      if (!(events[i].events & EPOLLIN)) /* 如果不是读事件，忽略 */
        continue;

      if (events[i].data.fd == listenfd) {
        cli_len = sizeof(cli_addr);
        connfd = Accept(listenfd, (struct sockaddr*)&cli_addr, &cli_len);
        printf("Accept from %s at PORT %d\n",
               inet_ntop(AF_INET, &cli_addr.sin_addr, str, sizeof(str)),
               ntohs(cli_addr.sin_port));
        ev.events = EPOLLIN;
        ev.data.fd = connfd;
        epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &ev);
      } else {
        sockfd = events[i].data.fd;
        if ((n = Read(sockfd, buf, sizeof(buf))) < 0) {
          if (errno == ECONNRESET) {
            printf("Connection was reset by peer\n");
            // TODO 这两个语句顺序需要注意吗？
            epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
            Close(sockfd);
            continue;
          } else {
            perr_exit("read error");
          }
        } else if (n == 0) {
          printf("Connection was closed by peer\n");
          epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
          Close(sockfd);
          continue;
        } else {
          for (j = 0; j < n; j++) buf[j] = toupper(buf[j]);
          Writen(sockfd, buf, n);
          Writen(STDOUT_FILENO, buf, n);
        }
      }
    }
  }

  Close(listenfd);
  Close(efd);

  return 0;
}