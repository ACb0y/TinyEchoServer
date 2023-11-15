# 1.概述
本仓库通过c++语言，使用了17种不同的并发模型实现了回显服务，并设计实现了简单的应用层协议。

# 2.目录结构
本仓库的目录结构如下所示。
```
.
├── BenchMark
├── Epoll
├── EpollReactorProcessPoolCoroutine
├── EpollReactorSingleProcess
├── EpollReactorSingleProcessCoroutine
├── EpollReactorSingleProcessET
├── EpollReactorThreadPool
├── EpollReactorThreadPoolHSHA
├── EpollReactorThreadPoolMS
├── LeaderAndFollower
├── MultiProcess
├── MultiThread
├── Poll
├── ProcessPool1
├── ProcessPool2
├── Select
├── SingleProcess
├── ThreadPool
├── cmdline.cpp
├── cmdline.h
├── codec.hpp
├── common.hpp
├── conn.hpp
├── coroutine.cpp
├── coroutine.h
├── epollctl.hpp
├── packet.hpp
├── percentile.hpp
├── readme.md
├── test
└── timer.h
```
相关的文件和目录的说明如下。
- 在仓库根目录下的.h和.hpp文件属于公共代码。
- test目录为单元测试代码。
- BenchMark目录为基准性能压测工具代码。
- 其他剩余的17个目录对应着17种不同并发模型的代码目录。
