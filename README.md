# clusterChatServer

这是一个基于 CMake 的 C++ 聊天系统项目，仓库远端是：

`https://github.com/ValarIMMORTAL/clusterChatServer.git`

当前仓库适合把 GitHub 作为代码主存储。为了让后续在任意新环境中快速恢复，建议把源码、恢复文档、依赖来源和自动化脚本都纳入 Git，而不要依赖旧 Ubuntu 机器上的手工安装痕迹。

## 当前仓库状态

- 已使用 Git 管理，无子模块。
- `build/`、`bin/`、`.cache/` 已被 `.gitignore` 忽略，属于可重新生成内容。
- 仓库内没有 `README` 以外的恢复说明，也没有数据库建表 SQL 文件。
- 数据库和 Redis 连接参数是硬编码的，不是从环境变量或配置文件读取。
- 已提供恢复脚本：[scripts/bootstrap_ubuntu.sh](/home/arseqs/Codes/clusterChatServer/scripts/bootstrap_ubuntu.sh) 和 [scripts/create_recovery_bundle.sh](/home/arseqs/Codes/clusterChatServer/scripts/create_recovery_bundle.sh)。

## 建议纳入 Git 的内容

建议长期保留在 GitHub 中的内容：

- 项目源码
- 构建脚本和恢复脚本
- 依赖来源链接和版本说明
- 数据库恢复说明

不建议提交到 Git 的内容：

- `build/`、`bin/` 这类编译产物
- 本机 `/usr/local` 导出的二进制库压缩包
- 一次性恢复包 `recovery_bundle_*`

原因很简单：这些文件虽然不大，但它们是生成物或机器快照，容易过时，也会让仓库逐步变成备份盘。更适合提交的是“如何稳定重建环境”的脚本和文档。

## 运行时假设

代码里写死了下面这些默认值：

- MySQL: `127.0.0.1:3306`
- MySQL 用户: `root`
- MySQL 数据库名: `clusterChat`
- Redis: `127.0.0.1:6379`

对应源码位置：

- [src/server/db/db.cpp](/home/arseqs/Codes/clusterChatServer/src/server/db/db.cpp)
- [src/server/redis/redis.cpp](/home/arseqs/Codes/clusterChatServer/src/server/redis/redis.cpp)

程序会直接访问这些表：

- `user`
- `friend`
- `allGroup`
- `groupUser`
- `offlineMessage`

只要你的数据库备份里包含数据库 `clusterChat` 和这些表，恢复时就足够。

## 构建依赖

本机已经验证可以重新从空构建目录编译，依赖包括：

- `cmake`
- `g++`
- `make`
- `mysqlclient`
- `hiredis`
- `muduo_net`
- `muduo_base`
- `pthread`

其中最需要注意的是：

- `muduo` 头文件来自 `/usr/local/include/muduo`
- `muduo` 静态库来自 `/usr/local/lib/libmuduo_*.a`
- `hiredis` 头文件来自 `/usr/local/include/hiredis`
- `hiredis` 库来自 `/usr/local/lib/libhiredis*`

这说明 `muduo` 和 `hiredis` 不是仓库自带文件，因此仓库里已经补充了可复现安装方案，而不是只依赖旧系统里的库文件。

## 依赖来源

当前项目直接或间接依赖的上游项目如下：

- `muduo`: https://github.com/chenshuo/muduo
- `hiredis`: https://github.com/redis/hiredis
- `nlohmann/json`: https://github.com/nlohmann/json

本仓库推荐的恢复基线：

- `muduo` 使用官方仓库发布的 `v2.0.2`
- `hiredis` 使用 `1.3.0`
- `nlohmann/json` 当前直接以内置头文件 `thirdparty/json.hpp` 使用

如果以后想进一步自动化，最稳的方式是继续固定依赖版本，而不是在部署时直接追最新提交。

## Ubuntu 一键恢复依赖

仓库内已经提供 Ubuntu 恢复脚本：

```bash
./scripts/bootstrap_ubuntu.sh
```

这个脚本会做几件事：

- 安装系统依赖包
- 从上游仓库拉取并安装 `muduo` 和 `hiredis`
- 重新编译当前项目

默认安装前缀是 `/usr/local`。如果你以后想把依赖装到别的位置，可以这样执行：

```bash
INSTALL_PREFIX=/opt/clusterChat ./scripts/bootstrap_ubuntu.sh
```

## 最快恢复流程

1. 克隆仓库

```bash
git clone https://github.com/ValarIMMORTAL/clusterChatServer.git
cd clusterChatServer
```

2. 准备依赖

- 直接运行：

```bash
./scripts/bootstrap_ubuntu.sh
```

- 或手工安装 C++ 编译环境、CMake、MySQL 开发库、Redis，并从上游仓库安装 `muduo` 和 `hiredis`。

3. 恢复数据库

- 导入你已经备份好的 MySQL 数据库。
- 确保数据库名仍为 `clusterChat`。
- 如果 MySQL 用户、密码、主机或端口变了，需要修改 [src/server/db/db.cpp](/home/arseqs/Codes/clusterChatServer/src/server/db/db.cpp)。

4. 启动服务依赖

```bash
redis-server
```

MySQL 只要已经启动并且能连接即可。

5. 重新编译

```bash
cmake -S . -B build
cmake --build build -j
```

6. 启动程序

```bash
./bin/clusterChatServer 127.0.0.1 8080
./bin/chatClient 127.0.0.1 8080
```

## 删除本地 Ubuntu 前的建议

建议先做这几件事：

1. 处理工作区里的未提交改动。
   当前只发现 [src/server/chatService.cpp](/home/arseqs/Codes/clusterChatServer/src/server/chatService.cpp) 有一处注释修改；如果你想保留，记得提交并推送。

2. 把恢复文档和代码一起推到远端。
   如果 GitHub 是长期主存储，至少要保证重要说明和你想保留的代码改动已经推送。

3. 额外导出一份恢复包。
   运行下面的脚本会导出 Git 历史、本地未提交改动、环境清单，以及 `/usr/local` 里的 `muduo`/`hiredis` 文件。

```bash
./scripts/create_recovery_bundle.sh
```

4. 把恢复包和数据库备份放到仓库之外的位置。
   例如另一块硬盘、NAS、网盘或新的 Linux 主机。

## 风险提醒

- 当前仓库里硬编码了 MySQL 密码；如果这个 GitHub 仓库是公开的，建议尽快更换数据库密码。
- 仓库里没有数据库建表 SQL，因此数据库备份现在非常重要。
- 如果以后想进一步降低恢复成本，最好把数据库和 Redis 配置改成配置文件或环境变量，而不是继续写死在源码里。
- 如果未来要做真正的一键部署，建议下一步把数据库连接配置改成环境变量，并补一个数据库 schema SQL 文件。
