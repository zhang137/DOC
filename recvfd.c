#include <stdio.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    int clifd, fd, ret = -1;
    socklen_t slen; 
    struct msghdr msg; 
    struct sockaddr_un saddr;
    struct sockaddr_un caddr;
    int msg_len = CMSG_SPACE(sizeof(1));
    char cmsgbuf[msg_len];
    char buf[10] = {0};
    struct iovec iov;

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd < 0) {
        fprintf(stdout, "sockfd error");
        return -1;
    }
    
    memset(&saddr, 0, sizeof(saddr));
    saddr.sun_family = AF_UNIX;
//    memcpy(saddr.sun_path, "sendfd.sock", 12);

    /*unlink("sendfd.sock");*/
    if(bind(sockfd, (struct sockaddr *)&saddr, sizeof(short)+1/*sizeof(saddr)*/) < 0) {
        fprintf(stdout, "bind error");
        goto csk;
    }

    if(listen(sockfd, 5) < 0) {
        fprintf(stdout, "listen error");
        goto csk;
    }

    clifd = accept(sockfd, (struct sockaddr*)&caddr, &slen);
    if(clifd < 0) {
        fprintf(stdout, "accept error");
        goto csk; 
    }

    iov.iov_base = buf;
    iov.iov_len = 10;

    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = CMSG_SPACE(sizeof(1));

    if(recvmsg(clifd, &msg, 0) <= 0) {
        fprintf(stdout, "recvmsg error\n");
        goto ccsk;
    }

    struct cmsghdr *crmsg = (struct cmsghdr *)msg.msg_control;
    if(crmsg->cmsg_level != SOCK_STREAM || 
                    crmsg->cmsg_type != SCM_RIGHTS) {
        fprintf(stdout, "crmsg type or level error\n");
        goto ccsk;
    }

    fd = *(int *)CMSG_DATA(crmsg);
    write(fd, "send fd test", 12);
    
    close(fd);
	ret = 0;
	
ccsk:
    close(clifd);
csk:
    close(sockfd);
    return ret;
}