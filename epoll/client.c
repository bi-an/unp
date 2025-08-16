#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // STDOUT_FILENO

#include "wrap.h"

static const int MAX_LINE = 80;

int main(int argc, char **argv) {
  struct sockaddr_in serv_addr;
  int sockfd, n;
  char buf[MAX_LINE];

  if (argc != 3) perr_exit("./client IP PORT");

  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
  serv_addr.sin_port = htons(atoi(argv[2]));

  sockfd = Socket(AF_INET, SOCK_STREAM, 0);

  Connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  printf("--------------- connected --------------\n");

  while (fgets(buf, sizeof(buf), stdin) != NULL) {
    Write(sockfd, buf, strlen(buf));
    if ((n = Read(sockfd, buf, sizeof(buf))) == 0) {
      printf("The connection was closed by peer\n");
      break;
    } else if (n < 0) {
      perr_exit("Read error");
    } else {
      Write(STDOUT_FILENO, buf, n);
    }
  }

  Close(sockfd);

  return 0;
}