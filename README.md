## ZTOFS

ZTOFS means zero to one filesystem

从0到1写一个文件系统服务，意图是提高知识的深度和广度。初步想法是基于brpc写分别写服务器和客户端, 规划如下。

### step 1
- [x] step 1: 实现create, open, remove, read, write, setattr, getattr语义。 以及代码框架和CI配置

### step 2
- [x] step 2: 实现全部文件系统语义, 做简单性能测试

版本1 分析和性能测试: [测试报告](doc/version_1_analysis.md)

优化项目

- [ ] 增加io 线程池负责文件系统调用, 相同inode操作被同一个线程执行。

预期收益 
1. 单机里文件只会被固定的线程打开, 保证数据写入原子性（例如数据A，B写入，结果是AB或者BA，不会出现ABA这种写入）
2. 隔离性, 对于某文件的操作，影响面可控。io操作异常不会影响rpc线程执行，io线程异常影响面就是应该线程处理的请求
3. 性能上，会增加切线程的性能开销；但把rpc和io线程分离, 可以适当降低rpc线程增加io线程数量, 以及应用iouring等功能，可以提高性能

- [ ] 支持异步fsync

预期收益，大幅提高写性能，当前写操作瓶颈主要是fsync

- [ ] 实现打开fd 缓存

预期收益，提高元数据性能，当前每个操作都要先open再执行文件系统调用

### step 3
- [ ] step 3: 基于fuse 实现posix语义客户端和挂载  

## 编译

1. 安装依赖
`sudo apt-get install -y git g++ make libssl-dev libgflags-dev libprotobuf-dev libprotoc-dev protobuf-compiler libleveldb-dev libgoogle-glog-dev libgtest-dev`

2. cmake编译

`mkdir build && cd build && cmake .. && make -j4`