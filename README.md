#### dpdk-nginx
--------------
dpdk-nginx fork from official nginx-1.9.5, and run on the dpdk user space TCP/IP stack(ANS). For detail function, please refer to nginx official website(http://nginx.org/).

#### Build and install
--------------
*  Download latest dpdk version from [dpdk website](http://dpdk.org/)
```
$ make config T=x86_64-native-linuxapp-gcc
$ make install T=x86_64-native-linuxapp-gcc
$ export RTE_SDK=/home/mytest/dpdk
$ export RTE_TARGET=x86_64-native-linuxapp-gcc
```
*  Build dpdk and ANS following the [ANS wiki](https://github.com/ansyun/dpdk-ans/wiki/Compile-APP-with-netdp) 
```
$ git clone https://github.com/ansyun/dpdk-ans.git
$ export RTE_ANS=/home/mytest/dpdk-ans
$ ./install_deps.sh
$ cd ans
$ make
$ sudo ./build/ans -c 0x2 -n 1  -- -p 0x1 --config="(0,0,1)"
EAL: Detected lcore 0 as core 0 on socket 0
EAL: Detected lcore 1 as core 1 on socket 0
EAL: Support maximum 128 logical core(s) by configuration.
EAL: Detected 2 lcore(s)
EAL: VFIO modules not all loaded, skip VFIO support...
EAL: Setting up physically contiguous memory...
EAL: Ask a virtual area of 0x400000 bytes
EAL: Virtual area found at 0x7fdf90c00000 (size = 0x400000)
EAL: Ask a virtual area of 0x15400000 bytes
```
*  Download dpdk-nginx, build dpdk-nginx

```
$ git clone https://github.com/ansyun/dpdk-nginx.git
$ ./configure  --with-http_dav_module
$ make
$ make install   # default install dir is /usr/local/nginx
```
#### Testing
--------------
*  Setup DPDK Environment

Refer to [Getting Started Guide for Linux](http://dpdk.org/doc/guides/linux_gsg/quick_start.html)

*  Startup ANS TCP/IP stack
```
$ sudo ./build/ans -c 0x2 -n 1  -- -p 0x1 --config="(0,0,1)"
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
CPU:Intel(R) Xeon(R) CPU E5-2609 v4 @ 1.70GHz.
NIC:Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01) 
ANS run on a lcore.

# ab -n 30000 -c 500 10.0.0.2:80/
This is ApacheBench, Version 2.3 <$Revision: 1796539 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 10.0.0.2 (be patient)
Completed 3000 requests
Completed 6000 requests
Completed 9000 requests
Completed 12000 requests
Completed 15000 requests
Completed 18000 requests
Completed 21000 requests
Completed 24000 requests
Completed 27000 requests
Completed 30000 requests
Finished 30000 requests


Server Software:        nginx/1.12.2
Server Hostname:        10.0.0.2
Server Port:            80

Document Path:          /
Document Length:        612 bytes

Concurrency Level:      500
Time taken for tests:   0.851 seconds
Complete requests:      30000
Failed requests:        0
Total transferred:      25350000 bytes
HTML transferred:       18360000 bytes
Requests per second:    35262.84 [#/sec] (mean)
Time per request:       14.179 [ms] (mean)
Time per request:       0.028 [ms] (mean, across all concurrent requests)
Transfer rate:          29098.73 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        3    6   0.9      6      10
Processing:     2    8   1.5      8      17
Waiting:        2    6   1.5      7      15
Total:          8   14   1.8     14      24

Percentage of the requests served within a certain time (ms)
  50%     14
  66%     14
  75%     15
  80%     15
  90%     16
  95%     17
  98%     18
  99%     19
 100%     24 (longest request)

```
*  Test file download by wget tool
```
CPU:Intel(R) Xeon(R) CPU E5-2430 0 @ 2.20GHz.
NIC:Intel Corporation 82576 Gigabit Network Connection (rev 01) 
ANS run on a lcore.

root@h163:~# wget http://2.2.2.2/nginx_big_data
--2016-01-02 20:58:24--  http://2.2.2.2/nginx_big_data
Connecting to 2.2.2.2:80... connected.
HTTP request sent, awaiting response... 200 OK
Length: 44046102 (42M) [application/octet-stream]
Saving to: ‘nginx_big_data.1’

100%[=====================================>] 44,046,102   111MB/s   in 0.4s

2016-01-02 20:58:24 (111 MB/s) - ‘nginx_big_data.1’ saved [44046102/44046102]

root@h163:~#
```
*  dpdk-nginx QPS performance
```
CPU:Intel(R) Xeon(R) CPU E5-2670 0 @ 2.60GHz.
NIC:82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01) 
ANS run on a lcore.
6 dpdk-nginx run on ANS.

./wrk -c 5k -d30s -t16  http://10.0.0.2/
Running 30s test @ http://10.0.0.2/
  16 threads and 5000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    41.93ms  119.14ms   1.73s    92.61%
    Req/Sec    18.16k     1.73k   26.34k    76.00%
  8700983 requests in 30.11s, 6.88GB read
Requests/sec: 288956.57
Transfer/sec:    233.95MB

```

#### Notes
* Shall use the same gcc version to compile your application.
* ANS tcp stack support reuseport, so can enable nginx reuseport feature, multi nginx can listen on same port.
* proxy_pass is supported.
* In order to improve ANS performance, you shall isolate ANS'lcore from kernel by isolcpus and isolcate interrupt from ANS's lcore by update /proc/irq/default_smp_affinity file.
* You shall include dpdk libs as below way because mempool lib has __attribute__((constructor, used)) in dpdk-16.07 version, otherwise your application would coredump.
```
   $(RTE_ANS)/librte_anssock/librte_anssock.a \
  -L$(RTE_SDK)/$(RTE_TARGET)/lib \
  -Wl,--whole-archive -Wl,-lrte_mbuf -Wl,-lrte_mempool -Wl,-lrte_ring -Wl,-lrte_eal -Wl,--no-whole-archive -Wl,-export-dynamic \

```

#### Support
-------
For free support, please use ANS team mail list at anssupport@163.com, or QQ Group:86883521, or https://dpdk-ans.slack.com.
