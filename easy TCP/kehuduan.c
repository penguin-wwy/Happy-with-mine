#include "unp.h"
#include <string.h>

void str_cli(FILE *, int);
ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t readline(int fd, void *vptr, size_t maxlen);

int main(int argc, char **argv)
{
    int     sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    printf("wait for connect\n");
    connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
    printf("connect is good\n");


    str_cli(stdin, sockfd);

    exit(0);
}

void str_cli(FILE *fp, int sockfd)
{
    char sendline[MAXLINE], recvline[MAXLINE];

    while(fgets(sendline, MAXLINE, fp) != NULL){
        writen(sockfd, sendline, sizeof(sendline));
        if(0 == readline(sockfd, recvline, MAXLINE))
            printf("str_cli: server terminated prematurely");
        fputs(recvline, stdout);
    }
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while(nleft > 0){
        if((nwritten = write(fd, ptr, nleft)) <= 0){
            if(nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return 0;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char    c, *ptr;

    ptr = vptr;
    for(n = 1; n < maxlen; n++){
    again:
        if((rc = read(fd, &c, 1)) == 1){
            *ptr++ = c;
            if('\n' == c)
                break;
        }else if(0 == rc){
            *ptr = 0;
            return (n - 1);
        }else{
            if(EINTR == errno)
                goto again;
            return -1;
        }
    }
    *ptr = 0;
    return n;
}
