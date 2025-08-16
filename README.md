## ZTOFS

ZTOFS means zero to one filesystem

我希望能从0到1写一个文件系统服务，意图是提高知识的深度和广度。初步想法是基于brpc写分别写服务器和客户端, 短期内的规划如下。

step 1实现create, open, remove, read, write, setattr, getattr语义。 

step 2实现全部文件系统语义, 做简单性能测试

step 3基于fuse 实现posix语义客户端和挂载 

## 编译

1. 安装依赖
`sudo apt-get install -y git g++ make libssl-dev libgflags-dev libprotobuf-dev libprotoc-dev protobuf-compiler libleveldb-dev`

2. cmake编译

`mkdir build && cd build && cmake .. && make -j4`