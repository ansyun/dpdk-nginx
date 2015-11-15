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
#ifdef __USE_GNU
/* Access macros for `cpu_set'.  */
#define CPU_SETSIZE __CPU_SETSIZE
#define CPU_SET(cpu, cpusetp)   __CPU_SET (cpu, cpusetp)
#define CPU_CLR(cpu, cpusetp)   __CPU_CLR (cpu, cpusetp)
#define CPU_ISSET(cpu, cpusetp) __CPU_ISSET (cpu, cpusetp)
#define CPU_ZERO(cpusetp)       __CPU_ZERO (cpusetp)
#endif
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <dlfcn.h>

#define ODP_FD_BITS 30


static int inited;

static int (*real_close)(int);
static int (*real_socket)(int, int, int);
static int (*real_bind)(int, __CONST_SOCKADDR_ARG, socklen_t);
static int (*real_listen)(int, int);
static int (*real_accept4)(int, struct sockaddr *, socklen_t *, int);
static ssize_t (*real_recv)(int, void *, size_t, int);
static ssize_t (*real_send)(int, const void *, size_t, int);
static int (*real_shutdown)(int, int);

static ssize_t (*real_writev)(int, const struct iovec *, int);

static int (*real_setsockopt)(int, int, int, const void *, socklen_t);

static int (*real_ioctl)(int, int, void *);

static int (*real_epoll_create)(int);
static int (*real_epoll_ctl)(int, int, int, struct epoll_event *);
static int (*real_epoll_wait)(int, struct epoll_event *, int, int);

void ngx_opendp_init()
{
    int rc;
        
#define INIT_FUNCTION(func) \
        real_##func = dlsym(RTLD_NEXT, #func); \
        assert(real_##func)

    INIT_FUNCTION(socket);
    INIT_FUNCTION(bind);
    INIT_FUNCTION(close);
    INIT_FUNCTION(listen);
    INIT_FUNCTION(accept4);
    INIT_FUNCTION(recv);
    INIT_FUNCTION(send);
    INIT_FUNCTION(shutdown);
    
    INIT_FUNCTION(writev);

    INIT_FUNCTION(setsockopt);

    INIT_FUNCTION(ioctl);

    INIT_FUNCTION(epoll_create);
    INIT_FUNCTION(epoll_ctl);
    INIT_FUNCTION(epoll_wait);
/*    INIT_FUNCTION();
    INIT_FUNCTION();
    INIT_FUNCTION();
    INIT_FUNCTION();
    INIT_FUNCTION();
    INIT_FUNCTION();
    INIT_FUNCTION();*/

#undef INIT_FUNCTION

    rc = netdpsock_init(NULL);
    assert(0 == rc);

    inited = 1;
}

int socket(int domain, int type, int protocol)
{
    int rc;
    
    if (AF_INET != domain || (SOCK_STREAM != type && SOCK_DGRAM != type)) {
        return real_socket(domain, type, protocol);
    }

    assert(inited);
    rc = netdpsock_socket(domain, type, protocol);
    rc |= 1 << ODP_FD_BITS;
        
    return rc;
}

int socketpair (int __domain, int __type, int __protocol,
               int __fds[2])
{
    return -1;
}

int bind (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len)
{
    if (__fd & (1 << ODP_FD_BITS)) {
        __fd &= ~(1 << ODP_FD_BITS);
        return netdpsock_bind(__fd, __addr, __len);
    } else {
        return real_bind(__fd, __addr, __len);
    }
}

int getsockname (int __fd, __SOCKADDR_ARG __addr,
            socklen_t *__restrict __len)
{
    return -1;
}

int connect (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len)
{
    return -1;
}

int getpeername (int __fd, __SOCKADDR_ARG __addr,
            socklen_t *__restrict __len)
{
    return -1;
}

ssize_t send (int __fd, const void *__buf, size_t __n, int __flags)
{
    ssize_t n;
    int nwrite, data_size;
    char *buf;

    if (__fd & (1 << ODP_FD_BITS))
    {
        __fd &= ~(1 << ODP_FD_BITS);

        data_size = __n;
        n = __n;
        buf = (char *)__buf;
        while (n > 0) 
        {
            nwrite = netdpsock_send(__fd, buf + data_size - n, n, 0);  

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

        return __n;

    }
    else 
    {
        return real_send(__fd, __buf, __n, __flags);
    }
}

ssize_t recv (int __fd, void *__buf, size_t __n, int __flags)
{
    ssize_t rc;
    if (__fd & (1 << ODP_FD_BITS)) {
        __fd &= ~(1 << ODP_FD_BITS);
        rc = netdpsock_recv(__fd, __buf, __n, __flags);
        if (-1 == rc && NETDP_EAGAIN == errno) {
            errno = EAGAIN;
        }
        return rc;
    } else {
        return real_recv(__fd, __buf, __n, __flags);
    }
}

ssize_t sendto (int __fd, const void *__buf, size_t __n,
               int __flags, __CONST_SOCKADDR_ARG __addr,
                              socklen_t __addr_len)
{
    return -1;
}

ssize_t recvfrom (int __fd, void *__restrict __buf, size_t __n,
             int __flags, __SOCKADDR_ARG __addr,
             socklen_t *__restrict __addr_len)
{
    return -1;
}

ssize_t sendmsg (int __fd, const struct msghdr *__message,
            int __flags)
{
    return -1;
}

ssize_t recvmsg (int __fd, struct msghdr *__message, int __flags)
{
    return -1;
}

int getsockopt (int __fd, int __level, int __optname,
               void *__restrict __optval,
               socklen_t *__restrict __optlen)
{
    return -1;
}

int setsockopt (int __fd, int __level, int __optname,
               const void *__optval, socklen_t __optlen)
{
    if (__fd & (1 << ODP_FD_BITS)) {
        __fd &= ~(1 << ODP_FD_BITS);
        return netdpsock_setsockopt(__fd, __level, __optname, __optval, __optlen);
    } else {
        return real_setsockopt(__fd, __level, __optname, __optval, __optlen);
    }
}

int listen (int __fd, int __n)
{
    if (__fd & (1 << ODP_FD_BITS)) {
        __fd &= ~(1 << ODP_FD_BITS);
        return netdpsock_listen(__fd, __n);
    } else {
        return real_listen(__fd, __n);
    }
}

int accept (int __fd, __SOCKADDR_ARG __addr,
               socklen_t *__restrict __addr_len)
{
    return -1;
}

int accept4(int sockfd, struct sockaddr *addr,
               socklen_t *addrlen, int flags)
{
    int rc;

    if (sockfd & (1 << ODP_FD_BITS)) {
        sockfd &= ~(1 << ODP_FD_BITS);
        rc = netdpsock_accept(sockfd, addr, addrlen);
        addr->sa_family = AF_INET;
        rc |= 1 << ODP_FD_BITS;
        if (-1 == rc && NETDP_EAGAIN == errno) {
            errno = EAGAIN;
        }
    } else {
        rc = real_accept4(sockfd, addr, addrlen, flags);
    }
    return rc;
}

int shutdown (int fd, int how)
{
    if (fd & (1 << ODP_FD_BITS)) {
        fd &= ~(1 << ODP_FD_BITS);
        //netdpsock_close(fd);
        return 0;
    } else {
        return real_shutdown(fd, how);
    }
}

int close(int fd)
{
    if (fd & (1 << ODP_FD_BITS)) {
        fd &= ~(1 << ODP_FD_BITS);
        return netdpsock_close(fd);
    } else {
        return real_close(fd);
    }
}

int epoll_create (int __size)
{
    int rc;

    if (__size > 1) {
        rc = netdpsock_epoll_create(__size);
        rc |= 1 << ODP_FD_BITS;
    } else {
        rc = real_epoll_create(__size);
    }
    return rc;
}

int epoll_create1 (int __flags)
{
    return -1;
}

int epoll_ctl (int __epfd, int __op, int __fd,
              struct epoll_event *__event)
{
    int rc;

    if (__epfd & (1 << ODP_FD_BITS)) {
        __epfd &= ~(1 << ODP_FD_BITS);
        assert(__fd & (1 << ODP_FD_BITS));
        __fd &= ~(1 << ODP_FD_BITS);
        rc = netdpsock_epoll_ctl(__epfd, __op, __fd, __event);
    } else {
        assert(!(__fd & (1 << ODP_FD_BITS)));
        rc = real_epoll_ctl(__epfd, __op, __fd, __event);
    }
    return rc;
}

int epoll_wait (int __epfd, struct epoll_event *__events,
               int __maxevents, int __timeout)
{
    int rc;

    if (__epfd & (1 << ODP_FD_BITS)) {
        __epfd &= ~(1 << ODP_FD_BITS);
        rc = netdpsock_epoll_wait(__epfd, __events, __maxevents, __timeout);
    } else {
        rc = real_epoll_wait(__epfd, __events, __maxevents, __timeout);
    }
    return rc;
}

int epoll_pwait (int __epfd, struct epoll_event *__events,
            int __maxevents, int __timeout,
            const __sigset_t *__ss)
{
    return -1;
}


// fcntl

int ioctl(int fd, int request, void *p)
{
    if (fd & (1 << ODP_FD_BITS)) {
        fd &= ~(1 << ODP_FD_BITS);
        //return netdpsock_ioctl(fd, request, p);
        return 0;
    } else {
        return real_ioctl(fd, request, p);
    }
}


// read
// write
// sendfile


ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    ssize_t rc, i, n;
    int nwrite, data_size;
    char *buf;

    if (fd & (1 << ODP_FD_BITS)) 
    {
        fd &= ~(1 << ODP_FD_BITS);
        rc = 0;
        for (i = 0; i != iovcnt; ++i) 
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
        rc = real_writev(fd, iov, iovcnt);
    }
    return rc;
}



