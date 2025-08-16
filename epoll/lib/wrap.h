#ifndef WRAP_H
#define WRAP_H

#include <sys/socket.h>
#include <sys/types.h> /* See NOTES */

void perr_exit(const char *s);
int Socket(int domain, int type, int protocol);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Close(int fd);
// 最多写入count字节
ssize_t Read(int fd, void *buf, size_t count);
// 成功返回不保证一定将数据写入到了磁盘，如需保证，需要在所有数据写完之后调用fsync()
// Man page(2) `fsync'
// According  to  the  standard  specification (e.g., POSIX.1-2001), sync()
// schedules the writes, but may return before the actual writing is done.
// However Linux waits for I/O completions, and
// thus sync() or syncfs() provide the same guarantees as fsync called on every
// file in the system or filesystem respectively.
ssize_t Write(int fd, const void *buf, size_t count);
// 除非发生错误，不然保证能够读取count字节
// 中断不属于错误
ssize_t Readn(int fd, void *buf, size_t count);
ssize_t Writen(int fd, const void *buf, size_t count);
ssize_t ReadLine(int fd, void *buf, int maxlen);

#endif