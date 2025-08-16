#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFLEN 4096
#define MAX_EVENTS 1024
#define SERV_PORT 8000

// static const int BUFLEN = 4096; // Why error?
// static const int MAX_EVENTS = 1024;
// static const int SERV_PORT = 8000;

void senddata(int fd, int events, void* arg);
void recvdata(int fd, int events, void* arg);

struct myevent_s {
  int fd;                                           // 监听的描述符
  int events;                                       // 监听的事件
  void* arg;                                        // 泛型参数
  void (*callback)(int fd, int events, void* arg);  // 回调函数
  int status;        // 是否在监听，1是，0否
  char buf[BUFLEN];  // 缓冲区
  int len;           // 缓冲区中的数据长度
  long last_active;  // 记录每次加入红黑树g_efd的事件
};

// 将结构体myevent_s的成员初始化
void eventset(struct myevent_s* ev, int fd, void (*callback)(int, int, void*),
              void* arg) {
  ev->fd = fd;
  ev->callback = callback;
  ev->arg = arg;
  ev->events = 0;
  ev->status = 0;
  ev->last_active = time(NULL);
}

// 向 epoll 红黑树添加一个文件描述符
void eventadd(int efd, int events, struct myevent_s* ev) {
  struct epoll_event epv = {0, {0}};
  int opt;
  epv.data.ptr = ev;
  epv.events = ev->events = events;

  if (ev->status == 1) {
    opt = EPOLL_CTL_MOD;
  } else {
    opt = EPOLL_CTL_ADD;
    ev->status = 1;
  }

  if (epoll_ctl(efd, opt, ev->fd, &epv) < 0)
    printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
  else
    printf("event add OK [fd=%d], opt=%d, events[%0X]\n", ev->fd, opt, events);
}

// 从 epoll 红黑树删除一个文件描述符
void eventdel(int efd, struct myevent_s* ev) {
  struct epoll_event epv = {0, {0}};

  if (ev->status != 1) return;

  ev->status = 0;
  epv.data.ptr = ev;

  // epv参数为了向下兼容，Linux 2.6.9之后，该参数可为NULL
  if (epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv) < 0)
    printf("event delete failed [fd=%d] err %s\n", ev->fd, strerror(errno));
  else
    printf("event delete OK [fd=%d]\n", ev->fd);
}

// 全局变量
static int g_efd;  // 红黑树
                   // 所有事件集合，+1 --> listenfd
static struct myevent_s g_events[MAX_EVENTS + 1];

// 接受一个连接
// arg 参数忽略
void acceptconn(int lfd, int events, void* arg) {
  struct sockaddr_in cli_addr;
  socklen_t len = sizeof(cli_addr);
  char str[INET_ADDRSTRLEN];
  int cfd, i;

  if ((cfd = accept(lfd, (struct sockaddr*)&cli_addr, &len)) < 0) {
    if (errno != EAGAIN && errno != EINTR) {
      /* 暂时不作处理 */
    }
    printf("%s : accept, %s\n", __func__, strerror(errno));
    return;
  }

  /* do...while(0) 只可能执行一次，这么做是为了避开 goto 语句的应用 */
  do {
    for (i = 0; i < MAX_EVENTS; i++)
      if (g_events[i].status == 0) break;

    if (i == MAX_EVENTS) {
      printf("%s: max connection limit[%d]\n", __func__, MAX_EVENTS);
      break;
    }

    // 设置 cfd 为非阻塞
    int flag;
    if ((flag = fcntl(cfd, F_GETFL, 0)) < 0) {
      printf("%s: fcntl F_GETFL failed, %s\n", __func__, strerror(errno));
      break;
    }
    if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0) {
      printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
      break;
    }

    // 给 cfd 设置一个 myevent_s 结构体，
    // 回调函数为 recvdata，参数为 myevent_s 结构体本身
    eventset(&g_events[i], cfd, recvdata, &g_events[i]);
    eventadd(g_efd, EPOLLIN, &g_events[i]);

  } while (0);

  // inet_ntop 替代函数 inet_ntoa
  printf("new connection [%s:%d][time:%ld], pos[%d]\n",
         inet_ntop(AF_INET, &cli_addr.sin_addr, str, sizeof(str)),
         ntohs(cli_addr.sin_port), g_events[i].last_active, i);

  return;
}

// TODO fd似乎和arg是重复的
void senddata(int fd, int events, void* arg) {
  struct myevent_s* ev = (struct myevent_s*)arg;
  ssize_t n;

  n = send(fd, ev->buf, ev->len, 0);  // 这是一个回射服务器

  if (n > 0) {
    printf("send[fd=%d], [%ld]%s\n", fd, n, ev->buf);
    eventdel(g_efd, ev);            /* 从红黑树上摘下 */
    eventset(ev, fd, recvdata, ev); /* 将该文件描述符的回调改为 recvdata */
    eventadd(g_efd, EPOLLIN, ev); /* 重新添加到红黑树上，设置为监听读事件 */
  } else {
    close(ev->fd);        // 关闭连接
    eventdel(g_efd, ev);  // 从红黑树上摘下
    printf("send[fd=%d] error %s\n", fd, strerror(errno));
  }
}

void recvdata(int fd, int events, void* arg) {
  struct myevent_s* ev = (struct myevent_s*)arg;
  int n;

  n = recv(fd, ev->buf, sizeof(ev->buf), 0);

  eventdel(g_efd, ev); /* 从红黑树上摘下 */

  if (n > 0) {
    ev->len = n;
    ev->buf[n] = '\0'; // 手动添加结束标记
    printf("recv[fd=%d], [%d]%s\n", fd, n, ev->buf);
    eventset(ev, fd, senddata, ev); /* 将该文件描述符的回调改为 senddata */
    eventadd(g_efd, EPOLLOUT, ev); /* 重新挂上红黑树，设置为监听写事件 */
  } else if (n == 0) {
    close(ev->fd);
    printf("[fd=%d] pos[%ld], closed\n", fd, ev - g_events);
  } else {
    close(fd);
    printf("recv[%d] error %s\n", fd, strerror(errno));
  }
}

void initlistensocket(int efd, short port) {
  int lfd = socket(AF_INET, SOCK_STREAM, 0);
  fcntl(lfd, F_SETFL, O_NONBLOCK);  // 设置 lfd 为非阻塞

  eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]);
  eventadd(g_efd, EPOLLIN, &g_events[MAX_EVENTS]);

  struct sockaddr_in serv_addr;

  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(SERV_PORT);

  bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  listen(lfd, 20);
}

int main(int argc, char** argv) {
  unsigned short port = SERV_PORT;

  if (argc == 2)
    port = atoi(argv[1]); /* 使用用户指定的端口。如未指定，使用默认端口 */

  g_efd = epoll_create(MAX_EVENTS + 1);  // 这个参数大于0即可
  if (g_efd <= 0) {
    printf("create efd in %s err %s\n", __func__, strerror(errno));
    return -1;
  }

  initlistensocket(g_efd, port);

  struct epoll_event events[MAX_EVENTS + 1];  // 保存已经就绪的文件描述符数组
  printf("server running: port[%d]\n", port);

  int checkpos = 0, i;

  while (1) {
    /* 超时验证，每次测试100个连接，不测试listenfd，当客户端60s没有和服务器通信，则关闭该连接
     */
    long now = time(NULL);
    for (i = 0; i < 100; i++, checkpos++) {
      if (checkpos == MAX_EVENTS) checkpos = 0;
      if (g_events[checkpos].status != 1) /* 不在红黑树上 */
        continue;

      long duration = now - g_events[checkpos].last_active;

      if (duration >= 60) {
        close(g_events[i].fd);
        printf("[fd=%d] timeout\n", g_events[checkpos].fd);
        eventdel(g_efd, &g_events[checkpos]);
      }
    }

    // 监听红黑树，将就绪文件描述符存入events，1s没有就绪事件，则返回0
    int nfd = epoll_wait(g_efd, events, MAX_EVENTS + 1, 1000);

    for (i = 0; i < nfd; i++) {
      struct myevent_s* ev = (struct myevent_s*)events[i].data.ptr;
      if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)) {
        ev->callback(ev->fd, ev->events, ev->arg);
      } else if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)) {
        ev->callback(ev->fd, ev->events, ev->arg);
      }
    }
  }

  /* 退出前释放所有资源 */
  return 0;
}