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

####Testing
--------------
*  Startup netdp TCP/IP stack
```
$ sudo ./build/opendp -c 0x1 -n 1  -- -p 0x1 --config="(0,0,0)"
EAL: Detected lcore 0 as core 0 on socket 0
EAL: Detected lcore 1 as core 1 on socket 0
EAL: Support maximum 128 logical core(s) by configuration.
EAL: Detected 2 lcore(s)
EAL: VFIO modules not all loaded, skip VFIO support...
EAL: Setting up physically contiguous memory...
...
```
*  startup nginx
```
$ sudo ./objs/nginx
EAL: Detected lcore 0 as core 0 on socket 0
EAL: Detected lcore 1 as core 1 on socket 0
....
2015/11/09 19:04:37 [notice] 1812#0: OS: Linux 3.16.0-30-generic
2015/11/09 19:04:37 [notice] 1812#0: getrlimit(RLIMIT_NOFILE): 1024:4096
```
*  Test http connection by ab tool
```
$ sudo ab -n 100000 -c 800  -t 200 2.2.2.2:80/
Benchmarking 2.2.2.2 (be patient)
Completed 5000 requests
Completed 10000 requests
Completed 15000 requests
Completed 20000 requests
Completed 25000 requests
Completed 30000 requests
Completed 35000 requests
Completed 40000 requests
Completed 45000 requests
Completed 50000 requests
Finished 50000 requests
Server Software:        nginx/1.9.5
Server Hostname:        2.2.2.2
Server Port:            80
Document Path:          /
Document Length:        612 bytes
Concurrency Level:      800
Time taken for tests:   41.034 seconds
Complete requests:      50000
Failed requests:        0
Total transferred:      42200000 bytes
HTML transferred:       30600000 bytes
Requests per second:    1218.51 [#/sec] (mean)
Time per request:       656.540 [ms] (mean)
Time per request:       0.821 [ms] (mean, across all concurrent requests)
Transfer rate:          1004.32 [Kbytes/sec] received
Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0  537 411.7    211    1150
Processing:     0  110  93.3    164     330
Waiting:        0  110  93.3    164     330
Total:        110  648 322.1    422    1320
Percentage of the requests served within a certain time (ms)
  50%    422
  66%   1001
  75%   1003
  80%   1005
  90%   1011
  95%   1021
  98%   1039
  99%   1098
 100%   1320 (longest request)

```
*  Test file download by wget tool
```
$ wget http://2.2.2.2/nginx_test_file
--2015-11-09 19:17:04--  http://2.2.2.2/nginx_test_file
Connecting to 2.2.2.2:80... connected.
HTTP request sent, awaiting response... 200 OK
Length: 44046102 (42M) [application/octet-stream]
Saving to: ‘nginx_test_file.2’
100%[============================================>] 44,046,102  31.7MB/s   in 1.3s
2015-11-09 19:17:05 (31.7 MB/s) - ‘nginx_test_file.2’ saved [44046102/44046102]

```

####Support
-------
For free support, please use netdp team mail list at zimeiw@163.com. or QQ Group:86883521
