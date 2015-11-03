####dpdk-nginx
--------------
dpdk-nginx fork from official nginx-1.9.5, and run on the dpdk user space TCP/IP stack(NETDP). For detail function, please refer to nginx official website(http://nginx.org/).

####build and install
--------------
*  Downlaod latest dpdk version from [dpdk website](http://dpdk.org/)
*  git clone https://github.com/opendp/dpdk-odp.git
*  Build dpdk and opendp following the [opendp wiki](https://github.com/opendp/dpdk-odp/wiki/Compile-APP-with-netdp) 
*  git clone https://github.com/opendp/dpdk-nginx.git
*  ./configure
*  make
*  make install   # default install dir is /usr/local/nginx

####debug
--------------
*  make
*  ./objs/nginx
