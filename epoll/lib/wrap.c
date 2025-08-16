#include <errno.h>
#include <stdio.h>
// #include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

void perr_exit(const char *s)
{
  perror(s);
  exit(-1);
}

int Socket(int domain, int type, int protocol)
{
  int n;

  if ((n = socket(domain, type, protocol)) < 0)
    perr_exit("socket error");

  return n;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
  int n;

again:
  if ((n = accept(sockfd, addr, addrlen)) < 0)
    if (errno == EINTR || errno == ECONNABORTED)
      goto again;
    else
      perr_exit("accept error");
  return n;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  int n;

  if ((n = bind(sockfd, addr, addrlen)) < 0)
    perr_exit("bind error");

  return n;
}

int Listen(int sockfd, int backlog)
{
  int n;

  if ((n = listen(sockfd, backlog)) < 0)
    perr_exit("listen error");

  return n;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  int n;

  if ((n = connect(sockfd, addr, addrlen)) < 0)
    perr_exit("connect error");

  return n;
}

int Close(int fd)
{
  int n;

  if ((n = close(fd)))
    perr_exit("close error");

  return n;
}

ssize_t Read(int fd, void *buf, size_t count)
{
  int n;

again:
  if ((n = read(fd, buf, count)) < 0)
    if (errno == EINTR)
      goto again;
    else
      return -1;

  return n;
}

ssize_t Write(int fd, const void *buf, size_t count)
{
  int n;

again:
  if ((n = write(fd, buf, count)) < 0)
    if (errno == EINTR)
      goto again;
    else
      return -1;

  return n;
}

ssize_t Readn(int fd, void *buf, size_t count)
{
  ssize_t nleft;
  ssize_t nread = 0;
  char *ptr;

  nleft = count;
  ptr = (char *)buf;

  while (nleft > 0)
  {
    if ((nread = read(fd, ptr, nleft)) < 0)
      if (errno == EINTR)
        nread = 0;
      else
      {
        perror("Error occurs when calling Readn");
        return -1;
      }
    else if (nread == 0)
      break;

    nleft -= nread;
    ptr += nread;
  }

  return count - nleft;
}

ssize_t Writen(int fd, const void *buf, size_t count)
{
  ssize_t nleft;
  ssize_t nwritten;
  const char *ptr;

  nleft = count;
  ptr = (const char *)buf;

  while (nleft > 0)
  {
    if ((nwritten = write(fd, buf, nleft)) < 0)
      if (errno == EINTR)
        nwritten = 0;
      else
      {
        perror("Error occurs when calling Writen");
        return -1;
      }

    nleft -= nwritten;
    ptr += nwritten;
  }

  return count - nwritten;
}

// static函数，该函数只在本文件使用，不对外提供接口
// 这是 ReadLine 函数的内置函数
// @return 读取到ptr中的字节数：0, 1, -1, 其中 -1 表示出错
static ssize_t my_read(int fd, char *ptr)
{
  static ssize_t read_cnt; // global变量，默认初始化为0
  static char read_buf[100];
  static char *read_ptr;

  // 优先读取缓冲区
  if (read_cnt <= 0)
  {
  again:
    if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0)
      if (errno == EINTR)
        goto again;
      else
      {
        perror("Error occurs when calling my_read");
        return -1;
      }
    else if (read_cnt == 0)
      return 0;
    read_ptr = read_buf; // 内置缓冲区读指针放回头部
  }
  // 读取1个字节到外部缓冲区ptr
  read_cnt--;
  *ptr = *read_ptr++;

  return 1;
}

// 从套接字中读取一行
// 库函数提供的 fgets, gets, readline 都只能从标准缓冲区中读取，不能从 fd 中读取
// @return 读取到的字节数
ssize_t ReadLine(int fd, void *buf, int maxlen)
{
  ssize_t rc, n; // returned count
  char c, *ptr;

  ptr = buf;

  // 最多读取 maxlen-1 个字符
  for (n = 1; n < maxlen; n++)
  {
    if ((rc = my_read(fd, &c)) == 1)
    {
      *ptr++ = c;
      if (c == '\n')
        break;
    }
    else if (rc == 0)
    {
      *ptr = 0; // 结束标记 '\0'
      return n - 1; // 本次循环开始时n++，但是本次没有读取到数据，故还原n
    }
    else
    {
      perror("Error occurs when calling ReadLine");
      return -1;
    }
  }
  *ptr = 0; // 结束标记 '\0'

  return n;
}
