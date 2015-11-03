# dpdk-nginx
dpdk-nginx fork from official nginx-1.9.5, and run on the dpdk user space TCP/IP stack(NETDP). For detail function, please refer to nginx official website(http://nginx.org/).

## build and install
1. git clone https://github.com/opendp/dpdk-odp.git
2. build opendp following the wiki
3. git clone https://github.com/opendp/dpdk-nginx.git
3. ./configure
4. make
5. make install   # default install dir is /usr/local/nginx

## debug
1. make
2. ./objs/nginx
