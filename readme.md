# 1.概述
本仓库通过c++语言，使用了20种不同的并发模型实现了回显服务，并设计实现了简单的应用层协议。

# 2.目录结构
本仓库的目录结构如下所示。
```
.
├── BenchMark
├── Epoll
├── EpollReactorProcessPoolCoroutine
├── EpollReactorProcessPoolMS
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
├── PollReactorSingleProcess
├── ProcessPool1
├── ProcessPool2
├── Select
├── SelectReactorSingleProcess
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
├── readme.md
└── test
```
相关的文件和目录的说明如下。
- 在仓库根目录下的.h和.hpp文件属于公共代码。
- test目录为单元测试代码的目录。
- BenchMark为基准性能压测工具的代码目录。
- 其他剩余的20个目录对应着20种不同并发模型的代码目录。
