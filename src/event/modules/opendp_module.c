#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <netinet/in.h>
#include <termios.h>
#include <sys/epoll.h>
#include <assert.h>
 #include <unistd.h>
 
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifndef __linux__
#ifdef __FreeBSD__
#include <sys/socket.h>
#else
#include <net/socket.h>
#endif
#endif

#include <sys/time.h>
#include "netdpsock_intf.h"
#include "netdp_errno.h"

#define _GNU_SOURCE
#define __USE_GNU

#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <dlfcn.h>


/* 
 *  opendp socket fd large than linux "ulimit -n " value
 *  
*/
#define ANS_FD_BASE 2000

/* 1: redis socket will go through opendp stack, 0: go through linux stack */
int ans_sock_enable = 1; 

int ans_debug_flag = 0;

#define ANS_FD_DEBUG( fmt, args...)  \
  do {                                                           \
    if(ans_debug_flag == 1)   \
        printf(fmt ,  ## args);  \
  } while (0)
  
static int inited = 0;

static int (*real_close)(int);
static int (*real_socket)(int, int, int);
static int (*real_bind)(int, const struct sockaddr*, socklen_t);
static int (*real_connect)(int, const struct sockaddr*, socklen_t);
static int (*real_listen)(int, int);
static int (*real_accept)(int, struct sockaddr *, socklen_t *);
static int (*real_accept4)(int, struct sockaddr *, socklen_t *, int);
static ssize_t (*real_recv)(int, void *, size_t, int);
static ssize_t (*real_send)(int, const void *, size_t, int);
static int (*real_shutdown)(int, int);

static ssize_t (*real_writev)(int, const struct iovec *, int);
static ssize_t (*real_write)(int, const void *, size_t );
static ssize_t (*real_read)(int, void *, size_t );
static ssize_t (*real_readv)(int, const struct iovec *, int);

static int (*real_setsockopt)(int, int, int, const void *, socklen_t);

static int (*real_ioctl)(int, int, void *);

static int (*real_epoll_create)(int);
static int (*real_epoll_ctl)(int, int, int, struct epoll_event *);
static int (*real_epoll_wait)(int, struct epoll_event *, int, int);

/**
 * @param 
 *
 * @return  
 *
 */
void opendp_init()
{
    int rc;
       
#define INIT_FUNCTION(func) \
        real_##func = dlsym(RTLD_NEXT, #func); \
        assert(real_##func)

    INIT_FUNCTION(socket);
    INIT_FUNCTION(bind);
    INIT_FUNCTION(connect);
    INIT_FUNCTION(close);
    INIT_FUNCTION(listen);
    INIT_FUNCTION(accept);
    INIT_FUNCTION(accept4);
    INIT_FUNCTION(recv);
    INIT_FUNCTION(send);
    INIT_FUNCTION(shutdown);
    
    INIT_FUNCTION(writev);
    INIT_FUNCTION(write);
    INIT_FUNCTION(read);
    INIT_FUNCTION(readv);

    INIT_FUNCTION(setsockopt);

    INIT_FUNCTION(ioctl);

    INIT_FUNCTION(epoll_create);
    INIT_FUNCTION(epoll_ctl);
    INIT_FUNCTION(epoll_wait);
   /*    
    INIT_FUNCTION();
    INIT_FUNCTION();
    INIT_FUNCTION();
    INIT_FUNCTION();
    INIT_FUNCTION();
    INIT_FUNCTION();
    INIT_FUNCTION();
    */

#undef INIT_FUNCTION

    if(ans_sock_enable != 1)
    {
        printf("ans socket is disable \n");
        return;
    }

    rc = netdpsock_init(NULL);
    assert(0 == rc);

    inited = 1;
}

/**
 * @param 
 *
 * @return  
 *
 */
int socket(int domain, int type, int protocol)
{
    int rc;

     ANS_FD_DEBUG("socket create start , domain %d, type %d \n", domain, type);    
   
    if ((inited == 0) ||  (AF_INET != domain) || (SOCK_STREAM != type && SOCK_DGRAM != type))
    {
        rc = real_socket(domain, type, protocol);
        ANS_FD_DEBUG("linux socket fd %d \n", rc);    

        return rc;
    }

    assert(inited);
    rc = netdpsock_socket(domain, type, protocol);
    
    if(rc > 0)
        rc += ANS_FD_BASE;
    
    ANS_FD_DEBUG("netdp socket fd %d \n", rc);    
    return rc;
}

/**
 * @param 
 *
 * @return  
 *
 */
int socketpair (__attribute__((unused)) int __domain, __attribute__((unused)) int __type,__attribute__((unused))  int __protocol, __attribute__((unused)) int __fds[2])
{
    return -1;
}

/**
 * @param 
 *
 * @return  
 *
 */
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    struct sockaddr_in *in_addr; 
    in_addr = (struct sockaddr_in *)addr;

    ANS_FD_DEBUG("bind ip: %x , port %d, family:%d \n", in_addr->sin_addr.s_addr, ntohs(in_addr->sin_port), in_addr->sin_family);

    if (sockfd > ANS_FD_BASE) 
    {
        sockfd -= ANS_FD_BASE;
        return netdpsock_bind(sockfd, addr, addrlen);
    } 
    else 
    {
        return real_bind(sockfd, addr, addrlen);
    }
}

/**
 * @param 
 *
 * @return  
 *
 */
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{

    ANS_FD_DEBUG("fd(%d) start to connect \n", sockfd);

    if (sockfd > ANS_FD_BASE) 
    {
        sockfd -= ANS_FD_BASE;
        return netdpsock_connect(sockfd, addr, addrlen);
    } 
    else 
    {
        return real_connect(sockfd, addr, addrlen);
    }
    
}

/**
 * @param 
 *
 * @return  
 *
 */
int getsockname (__attribute__((unused))int __fd, __attribute__((unused))__SOCKADDR_ARG __addr, __attribute__((unused)) socklen_t *__restrict __len)
{
    return -1;
}

/**
 * @param 
 *
 * @return  
 *
 */
int getpeername (__attribute__((unused))int __fd, __attribute__((unused)) __SOCKADDR_ARG __addr, __attribute__((unused)) socklen_t *__restrict __len)
{
    return -1;
}

/**
 * @param 
 *
 * @return  
 *
 */
ssize_t send (int sockfd, const void *buf, size_t len, int flags)
{
    ssize_t n;
    int nwrite, data_size;
    char *data_buf;
    
    ANS_FD_DEBUG("send data fd %d , len %lu \n", sockfd, len);

    if (sockfd > ANS_FD_BASE) 
    {
        sockfd -= ANS_FD_BASE;
        ANS_FD_DEBUG("netdp send data fd %d , len %lu \n", sockfd, len);

        data_size = len;
        n = len;
        data_buf = (char *)buf;
        while (n > 0) 
        {
            nwrite = netdpsock_send(sockfd, data_buf + data_size - n, n, 0);  

            if(nwrite<=0) 
            {   
                if(errno==NETDP_EAGAIN)  
                {  
                    usleep(100);  /* no space in netdp stack */
                    continue;  
                }  
                else 
                {  
                    printf("write error: errno = %d, strerror = %s \n" , errno, strerror(errno));  
                    return(nwrite);  
                }  
            }  

            if (nwrite < n) 
            {
                usleep(200);/* no space in netdp stack */
            }
            n -= nwrite;
            
        }

        return len;

    }
    else 
    {
        ANS_FD_DEBUG("linux send data fd %d , len %lu \n", sockfd, len);

        return real_send(sockfd, buf, len, flags);
    }
}

/**
 * @param 
 *
 * @return  
 *
 */
ssize_t write(int fd, const void *buf, size_t count)
{
    ssize_t n;
    int nwrite, data_size;
    char *data;

//    ANS_FD_DEBUG("write data fd %d , len %lu \n", fd, count);

    if (fd > ANS_FD_BASE) 
    {
        fd -= ANS_FD_BASE;

        ANS_FD_DEBUG("netdp write data fd %d , len %lu \n", fd, count);

        data_size = count;
        n = count;
        data = (char *)buf;
        while (n > 0) 
        {
            nwrite = netdpsock_write(fd, data + data_size - n, n);  

            if(nwrite<=0) 
            {   
                if(errno==NETDP_EAGAIN)  
                {  
             //       usleep(200);  /* no space in netdp stack */
                    continue;  
                }  
                else 
                {  
                    printf("write error: errno = %d, strerror = %s \n" , errno, strerror(errno));  
                    return(nwrite);  
                }  
            }  

            if (nwrite < n) 
            {
         //       usleep(200);/* no space in netdp stack */
            }
            n -= nwrite;
            
        }

        return count;

    }
    else 
    {

        n = real_write(fd, buf, count);
        
     //  ANS_FD_DEBUG("linux write data fd %d , len %ld \n", fd, count);
     
        return n;
    }
}

/**
 * @param 
 *
 * @return  
 *
 */
ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
    ssize_t rc;
    if (sockfd > ANS_FD_BASE) 
    {
        sockfd -= ANS_FD_BASE;

        rc = netdpsock_recv(sockfd, buf, len, flags);
        if (-1 == rc && NETDP_EAGAIN == errno)
        {
            errno = EAGAIN;
        }

        ANS_FD_DEBUG("netdp fd %d recv data len %ld \n", sockfd, rc);

        return rc;
    } 
    else
    {
        rc = real_recv(sockfd, buf, len, flags);

        ANS_FD_DEBUG("linux fd %d recv data len %ld \n", sockfd, rc);
        
        return rc;
    }
}

/**
 * @param 
 *
 * @return  
 *
 */
ssize_t read(int fd, void *buf, size_t count)
{
    ssize_t rc;
    if (fd > ANS_FD_BASE) 
    {
        fd -= ANS_FD_BASE;

        rc = netdpsock_read(fd, buf, count);
        if (-1 == rc && NETDP_EAGAIN == errno)
        {
            errno = EAGAIN;
        }
        ANS_FD_DEBUG("netdp fd %d read data len %ld \n", fd, rc);
        
        return rc;
    } 
    else
    {
        rc =real_read(fd, buf, count);
   //     ANS_FD_DEBUG("linux fd %d read data len %ld  \n", fd, rc);

        return rc;
    }
}

/**
 * @param 
 *
 * @return  
 *
 */
ssize_t sendto(__attribute__((unused)) int sockfd, __attribute__((unused))const void *buf, __attribute__((unused))size_t len, __attribute__((unused))int flags,
                      __attribute__((unused))const struct sockaddr *dest_addr, __attribute__((unused))socklen_t addrlen)
{
    return -1;
}

/**
 * @param 
 *
 * @return  
 *
 */

 ssize_t recvfrom(__attribute__((unused))int sockfd, __attribute__((unused))void *buf, __attribute__((unused))size_t len, __attribute__((unused))int flags,
                        __attribute__((unused))struct sockaddr *src_addr, __attribute__((unused))socklen_t *addrlen)
{
    return -1;
}

 int getsockopt(__attribute__((unused))int sockfd, __attribute__((unused))int level, __attribute__((unused))int optname, 
        __attribute__((unused))void *optval, __attribute__((unused))socklen_t *optlen)
{
    return -1;
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    if (sockfd > ANS_FD_BASE) 
    {
        sockfd -= ANS_FD_BASE;

        return netdpsock_setsockopt(sockfd, level, optname, optval, optlen);
    } 
    else 
    {
        return real_setsockopt(sockfd, level, optname, optval, optlen);
    }
}

/**
 * @param 
 *
 * @return  
 *
 */
int listen(int sockfd, int backlog)
{
    if (sockfd > ANS_FD_BASE) 
    {
        sockfd -= ANS_FD_BASE;
        
        ANS_FD_DEBUG("netdp listen fd %d, pid %d \n", sockfd, getpid());
        
        return netdpsock_listen(sockfd, backlog);
    }
    else
    {
        ANS_FD_DEBUG("linux listen fd %d , pid %d \n", sockfd, getpid());
  
        return real_listen(sockfd, backlog);
    }
}

/**
 * @param 
 *
 * @return  
 *
 */
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int rc;

    if (sockfd > ANS_FD_BASE) 
    {
        sockfd -= ANS_FD_BASE;

        rc = netdpsock_accept(sockfd, addr, addrlen);
        addr->sa_family = AF_INET;

        ANS_FD_DEBUG("netdp accept fd %d \n", rc);
        if(rc > 0 )
            rc += ANS_FD_BASE;
        
        if (-1 == rc && NETDP_EAGAIN == errno) 
        {
            errno = EAGAIN;
        }
    } 
    else 
    {
        rc = real_accept(sockfd, addr, addrlen);
        ANS_FD_DEBUG("linux accept fd %d \n", rc);

    }
    return rc;
}

/**
 * @param 
 *
 * @return  
 *
 */
int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags)
{
    int rc;

    if (sockfd > ANS_FD_BASE) 
    {
        sockfd -= ANS_FD_BASE;

        rc = netdpsock_accept(sockfd, addr, addrlen);
        addr->sa_family = AF_INET;
        
        ANS_FD_DEBUG("netdp accep4t fd %d, errno %d \n", rc, errno);
        
        if(rc > 0 )
            rc += ANS_FD_BASE;

        if (-1 == rc && NETDP_EAGAIN == errno)
        {
            errno = EAGAIN;
        }
    } 
    else 
    {
        rc = real_accept4(sockfd, addr, addrlen, flags);
        ANS_FD_DEBUG("linux accept4 fd %d, errno %d \n", rc, errno);
    }
    return rc;
}

/**
 * @param 
 *
 * @return  
 *
 */
int shutdown (int fd, int how)
{
    ANS_FD_DEBUG("netdp shutdown fd %d, how %d,  pid %d \n", fd, how, getpid());

    if (fd > ANS_FD_BASE) 
    {
        fd -= ANS_FD_BASE;

        return netdpsock_shutdown(fd, how);;
    } 
    else
    {
        return real_shutdown(fd, how);
    }
}

/**
 * @param 
 *
 * @return  
 *
 */
int close(int fd)
{
    if (fd > ANS_FD_BASE) 
    {
        fd -= ANS_FD_BASE;

       ANS_FD_DEBUG("netdp close fd %d, pid %d \n", fd, getpid());

        return netdpsock_close(fd);
    }
    else
    {
     //   ANS_FD_DEBUG("linux close fd %d \n", fd);
     
        return real_close(fd);
    }
}

/**
 * @param 
 *
 * @return  
 *
 */
int epoll_create (int size)
{
    int rc;

    ANS_FD_DEBUG("epoll create start \n");

    if (inited == 1) 
    {
        rc = netdpsock_epoll_create(size);
        if(rc > 0)
            rc += ANS_FD_BASE;
        
         ANS_FD_DEBUG("netdp epoll fd %d \n", rc);
      
    } 
    else 
    {
        rc = real_epoll_create(size);
        ANS_FD_DEBUG("linux epoll fd %d \n", rc);
    }
    return rc;
}

/**
 * @param 
 *
 * @return  
 *
 */
int epoll_create1 (__attribute__((unused))int __flags)
{
    return -1;
}

/**
 * @param 
 *
 * @return  
 *
 */
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
    int rc;
    ANS_FD_DEBUG("epoll ctl  start, epfd %d ,op %d, fd %d, event:0x%x \n", epfd, op, fd, event->events);

    if (epfd > ANS_FD_BASE) 
    {
        if(fd <= ANS_FD_BASE)
        {
            printf("skip linux fd %d \n", fd);
            return 0;
        }
        epfd -= ANS_FD_BASE;
        fd -= ANS_FD_BASE;

        rc = netdpsock_epoll_ctl(epfd, op, fd, event);
    }
    else 
    {
        if(fd > ANS_FD_BASE)
        {
            printf("skip netdp fd %d \n", fd);
            return 0;
        }

        rc = real_epoll_ctl(epfd, op, fd, event);
    }
    return rc;
}

 int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    int rc;

    if (epfd > ANS_FD_BASE) 
    {
        epfd -= ANS_FD_BASE;
        rc = netdpsock_epoll_wait(epfd, events, maxevents, timeout);
    }
    else
    {
        rc = real_epoll_wait(epfd, events, maxevents, timeout);
    }
    return rc;
}

/**
 * @param 
 *
 * @return  
 *
 */
int epoll_pwait (__attribute__((unused))int __epfd, __attribute__((unused))struct epoll_event *__events, __attribute__((unused))int __maxevents,
    __attribute__((unused))int __timeout, __attribute__((unused))const __sigset_t *__ss)
{
    return -1;
}


/**
 * @param 
 *
 * @return  
 *
 */
int ioctl(int fd, int request, void *p)
{
    if (fd > ANS_FD_BASE) 
    {
        fd -= ANS_FD_BASE;

        //return netdpsock_ioctl(fd, request, p);
        return 0;
    } 
    else
    {
        return real_ioctl(fd, request, p);
    }
}


/**
 * @param 
 *
 * @return  
 *
 */
ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    ssize_t rc;
    int i, n;
    int nwrite = 0, data_size;
    char *buf;

    if (fd > ANS_FD_BASE) 
    {
        fd -= ANS_FD_BASE;
        
        ANS_FD_DEBUG("netdp writev data fd %d , iovcnt %d \n", fd, iovcnt);

        rc = 0;
        for (i = 0; i < iovcnt; ++i) 
        {
            data_size = iov[i].iov_len;
            buf = iov[i].iov_base;
            n = data_size;
            while (n > 0) 
            {
                nwrite = netdpsock_send(fd, buf + data_size - n, n, 0);  

                if(nwrite<=0) 
                {   
                    if(errno==NETDP_EAGAIN)  
                    {  
                        usleep(200);  /* no space in netdp stack */
                        continue;  
                    }  
                    else 
                    {  
                        printf("write error: errno = %d, strerror = %s \n" , errno, strerror(errno));  
                        return(nwrite);  
                    }  
                }  

                if (nwrite < n) 
                {
                    usleep(200);/* no space in netdp stack */
                }
                n -= nwrite;
                
            }

            if (nwrite <= 0) 
                return nwrite;
            
            rc += data_size;
        }
    }
    else 
    {
     //   ANS_FD_DEBUG("linux writev data fd %d , len %d \n", fd, iovcnt);

        rc = real_writev(fd, iov, iovcnt);
    }
    return rc;
}

/**
 * @param 
 *
 * @return  
 *
 */
 ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
    int i;
    int nread = 0, buf_len;
    ssize_t rc;
    char *buf;

    if (fd > ANS_FD_BASE) 
    {
        fd -= ANS_FD_BASE;

        ANS_FD_DEBUG("netdp fd %d readv with iovcnt %d \n", fd, iovcnt);

        rc = 0;
        for (i = 0; i < iovcnt; ++i) 
        {
            buf_len = iov[i].iov_len;
            buf = iov[i].iov_base;
            
            nread = netdpsock_read(fd, buf, buf_len);
            if(nread <= 0) 
            {   
                if(errno == NETDP_EAGAIN)  
                {  
                    errno = EAGAIN;
                }  
                else 
                {  
                    if(nread < 0)
                        printf("readv error: rc=%ld, nread=%d,  errno=%d, strerror=%s \n", rc, nread, errno, strerror(errno));  
                }  
                return ((rc > 0)? rc : nread);  
            }  

            ANS_FD_DEBUG("netdp fd %d readv data len %d iov index %d \n", fd, nread, i);
      
            rc += nread;
            
        }

        ANS_FD_DEBUG("netdp fd %d readv data len %ld \n", fd, rc);
        
        return rc;
    } 
    else
    {
        rc =real_readv(fd, iov, iovcnt);

        return rc;
    }


}

    

