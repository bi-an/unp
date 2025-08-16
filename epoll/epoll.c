#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
 
#define MAXLINE  1024
#define OPEN_MAX  16 //一些系统会定义这些宏
#define SERV_PORT  10001
 
int main()
{
    int i , maxi ,listenfd , connfd , sockfd ,epfd, nfds;
    int n;
    char buf[MAXLINE];
    struct epoll_event ev, events[20];  
    socklen_t clilen;
    struct pollfd client[OPEN_MAX];
 
    struct sockaddr_in cliaddr , servaddr;
    listenfd = socket(AF_INET , SOCK_STREAM , 0);
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 
    bind(listenfd , (struct sockaddr *) & servaddr, sizeof(servaddr));
    listen(listenfd,10);
    
    epfd = epoll_create(256);
    ev.data.fd=listenfd; 
    ev.events=EPOLLIN|EPOLLET;
    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
    
    for(;;)
    {
        nfds=epoll_wait(epfd,events,20,500); 
        for(i=0; i<nfds; i++)
        {
            if (listenfd == events[i].data.fd)
            {
                clilen = sizeof(cliaddr);
                connfd = accept(listenfd , (struct sockaddr *)&cliaddr, &clilen);
                if(connfd < 0)  
                {  
                    perror("connfd < 0");  
                    exit(1);  
                }
                ev.data.fd=connfd; 
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);                
            }
            else if (events[i].events & EPOLLIN)
            {
                if ( (sockfd = events[i].data.fd) < 0)  
                    continue;  
                n = recv(sockfd,buf,MAXLINE,0);
                if (n <= 0)   
                {    
                    close(sockfd);  
                    events[i].data.fd = -1;  
                }
                else
                {
                    buf[n]='\0';
                    printf("Socket %d said : %s\n",sockfd,buf);
                    ev.data.fd=sockfd; 
                    ev.events=EPOLLOUT|EPOLLET;
                    epoll_ctl(epfd,EPOLL_CTL_MOD,connfd,&ev);
                }
            }
            else if( events[i].events&EPOLLOUT )
            {
                sockfd = events[i].data.fd;  
                send(sockfd, "Hello!", 7, 0);  
                  
                ev.data.fd=sockfd;  
                ev.events=EPOLLIN|EPOLLET;  
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev); 
            }
            else 
            {
                printf("This is not avaible!");
            }
        }
    }
    close(epfd);  
    return 0;
}