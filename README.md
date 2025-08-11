# NFShmXFrame 分布式游戏服务器框架

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/yigao/NFShmXFrame)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)](https://github.com/yigao/NFShmXFrame)
[![Language](https://img.shields.io/badge/language-C%2B%2B17-orange.svg)](https://github.com/yigao/NFShmXFrame)

## 🎯 项目概述

**NFShmXFrame** = **NF** + **Shm** + **X** + **Frame**

- **NF**：现有框架从NoahFrame框架改进而来，提供稳定的插件化架构
- **Shm**：Shared Memory共享内存技术，提供双内存池选择方案
- **X**：eXtensible脚本扩展系统，支持多语言混合开发  
- **Frame**：Framework框架，完整的游戏服务器解决方案

### 🎯 两大核心特色

#### 1. Shm共享内存技术
- **技术选择**：提供NFShmPlugin和NFMemPlugin两种内存池方案
- **开发自由**：完全基于开发者个人爱好和公司技术方案选择
- **技术创新**：为共享内存开发提供现代化的STL容器库
- **告别传统**：不再需要手写复杂的裸C数组数据结构

#### 2. X脚本扩展系统  
- **多语言支持**：当前支持Lua，未来支持Python、JavaScript
- **混合开发**：C++底层框架 + 脚本业务逻辑的开发模式
- **热更新**：脚本代码可实时更新，无需重启服务器
- **开发效率**：快速迭代，降低开发和维护成本

### 🚀 框架核心亮点

**NFShmXFrame = NF + Shm + X + Frame**
- **NF**：基于插件化的分布式游戏服务器框架
- **Shm**：共享内存技术，实现数据持久化和代码热更新
- **X**：脚本扩展系统，支持Lua等多种脚本语言
- **Frame**：完整的服务器框架解决方案

目前已实现单线程Lua脚本插件，后续将支持多线程Actor Lua系统（类似Skynet架构）、Python、JavaScript等更多脚本语言。

### 🏗️ 核心特色

- **🔌 插件化架构**：所有功能以插件形式实现，支持动态加载/卸载
- **🧠 Shm共享内存技术**：支持数据持久化、崩溃恢复、代码热更新的革命性内存管理
- **📜 X脚本扩展系统**：支持Lua等多种脚本语言，C++底层+脚本业务逻辑混合开发
- **🔥 双模式热更新**：配置重载、Lua脚本热更、C++代码热重启三重保障
- **🌐 分布式设计**：支持多服务器、多物理机部署，类IP地址的服务器标识
- **⚡ 高性能驱动**：固定30帧/秒驱动模式，支持性能监控和调优
- **🛡️ 稳定性保障**：崩溃恢复、信号处理、异常捕获、分阶段初始化
- **🔧 运维友好**：完善的启动参数、监控工具、HTTP管理接口

## 📋 目录结构

```
NFShmXFrame/
├── src/                    # 核心源代码
│   ├── NFrame/            # 框架底层 - 核心框架层
│   │   ├── NFComm/        # 通信与工具模块
│   │   ├── NFPluginManager/ # 插件管理器
│   │   └── NFCommPlugin/  # 通用插件
│   ├── NFServer/          # 服务器架构层 - 各种服务器实现
│   │   ├── NFMasterServer/    # 主控服务器
│   │   ├── NFProxyServer/     # 代理服务器
│   │   ├── NFGameServer/      # 游戏服务器
│   │   ├── NFLogicServer/     # 逻辑服务器
│   │   ├── NFWorldServer/     # 世界服务器
│   │   └── NFStoreServer/     # 存储服务器
│   └── NFTools/           # 工具集
├── game/                  # 游戏逻辑层
│   ├── Other/            # 手游项目示例
│   └── NFGameCommon/     # 游戏公共模块
├── doc/                   # 详细文档
├── thirdparty/           # 第三方库
├── tools/                # 构建和管理工具
└── Install/              # 安装目录
```

## 🚀 快速开始

### 环境要求

- **操作系统**：Windows 10/11, Ubuntu 18.04+, CentOS 7+
- **编译器**：Visual Studio 2019+, GCC 8+, Clang 10+
- **依赖库**：CMake 3.16+, Git

### 编译和安装

```bash
# 1. 克隆项目
git clone https://github.com/yigao/NFShmXFrame.git
cd NFShmXFrame

# 2. 编译（支持Linux/Windows）
mkdir build && cd build
cmake ..
make -j4

# 3. 安装
make install
```

### 启动服务器

```bash
# 1. 单一服务器模式启动
./NFPluginLoader --Server=GameServer --ID=1.13.10.1 --Plugin=LieRenPlugin

# 2. 多服务器模式启动（每种服务器一个实例）
./NFPluginLoader --Server=AllServer --ID=1.13.1.1 --Plugin=LieRenPlugin

# 3. 调试模式启动（便于跨服务器调试，每种服务器可多个实例）
./NFPluginLoader --Server=AllMoreServer --ID=1.13.1.1 --Plugin=LieRenPlugin

# 4. 生产环境守护进程模式
./NFPluginLoader --Server=GameServer --ID=1.13.10.1 --Plugin=LieRenPlugin --Start --Daemon
```

### 常用启动参数

| 参数 | 描述 | 示例 | 必需 |
|------|------|------|------|
| `--Server` | 服务器类型 | `GameServer`, `AllServer`, `AllMoreServer` | ✅ |
| `--ID` | 服务器唯一标识 | `1.13.10.1` (世界.区服.类型.索引) | ✅ |
| `--Plugin` | 插件配置目录 | `LieRenPlugin`, `MMOPlugin` | ✅ |
| `--Start` | 启动服务器 | 无需参数值 | ❌ |
| `--Stop` | 停止服务器 | 无需参数值 | ❌ |
| `--Reload` | 重载配置 | 无需参数值 | ❌ |
| `--Restart` | 热重启 | 无需参数值 | ❌ |
| `--Daemon` | 守护进程模式 | 无需参数值 | ❌ |

### 三种启动模式对比

| 启动模式 | 参数示例 | 进程数 | 插件管理器数 | 服务器实例数 | 适用场景 |
|----------|----------|--------|--------------|--------------|----------|
| **单一服务器** | `--Server=GameServer` | 1 | 1 | 1 | 生产环境单服务器部署 |
| **AllServer** | `--Server=AllServer` | 1 | 1 | 多个（每种类型1个） | 开发调试，功能测试 |
| **AllMoreServer** | `--Server=AllMoreServer` | 1 | 多个 | 多个（每种类型可多个） | 开发调试，跨服务器调试优化 |

### 分布式启动

对于多服务器分布式部署，建议使用NFServerController工具进行统一管理：

- **Windows平台**：使用图形化工具 `tools/NFServerControllerQt.exe`
- **Linux平台**：使用命令行工具 `tools/NFServerController`

详细使用方法请参考：[🚀 分布式启动文档](docs/NFShmXFrame分布式启动.md)

## 🏛️ 架构设计

### 三层架构

```
游戏逻辑层 (Game)
    ↓
服务器架构层 (NFServer)  
    ↓
框架基础层 (NFrame)
```

### 插件系统三大核心组件

1. **NFPluginManager（插件管理器）**：系统的核心控制器，负责整个插件生态的管理
2. **NFPlugin（插件基类）**：功能模块的逻辑容器，组织和管理相关模块  
3. **NFIModule（模块接口）**：最小功能单元，实现具体的业务逻辑

### 初始化任务系统

NFShmXFrame采用基于任务组的分阶段初始化机制，确保服务器按正确顺序启动：

```cpp
enum APP_INIT_TASK_GROUP
{
    APP_INIT_TASK_GROUP_SERVER_CONNECT = 1,          // 服务器连接任务组
    APP_INIT_TASK_GROUP_SERVER_LOAD_DESC_STORE = 2,  // 配置数据加载任务组
    APP_INIT_TASK_GROUP_SERVER_LOAD_OBJ_FROM_DB = 3, // 数据库数据加载任务组
    APP_INIT_TASK_GROUP_SERVER_REGISTER = 4,         // 服务器注册任务组
};
```

**初始化流程**：
1. **服务器连接**：建立与其他服务器的连接
2. **配置加载**：加载Lua配置和描述存储
3. **数据库加载**：从数据库加载全局数据
4. **服务器注册**：完成服务器间注册和认证

### 完整生命周期管理

框架提供15个不同的生命周期阶段，确保组件按正确顺序初始化：

```cpp
// 启动初始化阶段
AfterLoadAllPlugin()    → 所有插件加载完成
AfterInitShmMem()       → 共享内存初始化完成
Awake()                 → 模块唤醒
Init()                  → 模块初始化
CheckConfig()           → 配置检查
ReadyExecute()          → 准备执行

// 运行执行阶段
Execute()               → 主循环（30FPS）

// 服务器特定初始化阶段
AfterAllConnectFinish()    → 所有服务器连接建立完成
AfterAllDescStoreLoaded()  → 所有配置数据加载完成
AfterObjFromDBLoaded()     → 数据库数据加载完成
AfterServerRegisterFinish() → 服务器注册完成
AfterAppInitFinish()       → 应用初始化完成

// 关闭清理阶段
BeforeShut() → Shut() → Finalize()
```

### 🧠 双内存池机制

**灵活的内存管理方案**：
- **NFShmPlugin**：共享内存池 + 共享内存STL容器
  - 适合腾讯系技术栈的团队
  - 支持进程崩溃后数据恢复
  - 提供现代化的STL容器接口，告别裸C数组时代
- **NFMemPlugin**：传统进程内存池
  - 适合传统服务器开发团队
  - 开发简单，性能优异
  - 可自由使用标准C++ STL容器

**技术创新点**：
- 提供完整的共享内存STL容器库（vector、map、set、string等）
- 解决传统共享内存开发中手写复杂数据结构的痛点
- API与标准STL高度兼容，降低学习成本

## 🎮 主要功能

### 🔧 服务器管理

- **启动模式**：支持单服务器、AllServer、AllMoreServer三种启动模式
- **热重启**：`--restart` 参数实现零停机代码更新
- **配置重载**：`--reload` 参数实现配置文件热重载
- **守护进程**：`--daemon` 参数支持后台运行
- **信号控制**：支持优雅停服、强制终止等信号操作

### 🔥 热更新系统

**三种热更新方式**：

#### 1. 配置重载（--reload）
- **重载Lua配置文件**：实时更新服务器配置
- **重载Lua脚本**：业务逻辑脚本热更新
- **重载游戏数据**：Excel配置数据热重载
- **支持HTTP接口和信号触发**

#### 2. 代码热重启（--restart）：零数据丢失的C++代码热更新

NFShmXFrame的`--restart`功能实现了**真正的零数据丢失热更新**，这是框架的核心竞争优势之一。

##### ✨ 核心优势
- **🔄 零数据丢失**：通过共享内存技术保持所有游戏数据
- **⚡ 秒级重启**：整个重启过程通常在2-5秒内完成
- **🛡️ 双向确认机制**：确保旧进程完全释放资源后再启动新进程
- **🌍 跨平台支持**：Windows和Linux平台都有完整实现
- **📊 详细日志**：完整记录整个重启过程，便于运维监控

##### 🔧 使用方法

```bash
# Linux平台热重启
./NFPluginLoader --Server=GameServer --ID=1.13.10.1 --Plugin=LieRenPlugin --Restart --Daemon

# Windows平台热重启
NFPluginLoader.exe --Server=GameServer --ID=1.13.10.1 --Plugin=LieRenPlugin --Restart --Daemon

# HTTP接口触发重启
curl "http://127.0.0.1:6011/restart?Server=GameServer&ID=1.13.10.1"
```

##### 🏗️ 实现原理

**Linux平台实现**：
- 使用Linux信号机制实现进程间通信
- 通过`SIGUNUSED`信号触发优雅退出
- PID文件管理确保进程唯一性
- 共享内存自动恢复数据状态

**Windows平台实现**：
- 使用Windows命名事件实现进程间通信
- 双向确认机制确保安全重启：
  - 程序A发送`NFServer_Kill_{ProcessID}`事件
  - 程序B收到信号后优雅释放资源
  - 程序B发送`NFServer_KillSuccess_{ProcessID}`确认信号
  - 程序A收到确认后启动新进程

##### 📋 重启流程详解

**Linux平台重启流程图**：

```
用户执行重启命令
        │
        ▼
   ┌─────────────┐
   │检查PID文件？│
   └─────┬───────┘
         │
    ┌────▼────┐         ┌──────────────┐
    │文件存在？│   否    │直接启动新进程│
    └────┬────┘  ────▶  └──────┬───────┘
         │是                    │
         ▼                      │
   ┌──────────────┐              │
   │读取旧进程PID │              │
   └──────┬───────┘              │
          │                      │
          ▼                      │
   ┌──────────────────┐          │
   │发送SIGUNUSED信号 │          │
   └──────┬───────────┘          │
          │                      │
          ▼                      │
   ┌──────────────┐              │
   │旧进程收到信号│              │
   └──────┬───────┘              │
          │                      │
          ▼                      │
   ┌──────────────┐              │
   │设置停服标志  │              │
   └──────┬───────┘              │
          │                      │
          ▼                      │
   ┌──────────────┐              │
   │保存数据到共享│              │
   │内存          │              │
   └──────┬───────┘              │
          │                      │
          ▼                      │
   ┌──────────────┐              │
   │旧进程优雅退出│              │
   └──────────────┘              │
          │                      │
          ▼                      │
   ┌──────────────┐    超时      │
   │新进程等待    │ ────────┐    │
   │最多10秒      │         │    │
   └──────┬───────┘         │    │
          │进程已退出        │    │
          ▼                 ▼    │
   ┌──────────────┐  ┌───────────┐│
   │创建新PID文件 │  │强制杀死   ││
   └──────┬───────┘  │旧进程     ││
          │          └───────┬───┘│
          │                  │    │
          └─────────┬────────┘    │
                    │             │
                    ▼             │
            ┌──────────────┐      │
            │从共享内存    │      │
            │恢复数据      │ ◀────┘
            └──────┬───────┘
                   │
                   ▼
            ┌──────────────┐
            │初始化并启动  │
            │服务          │
            └──────┬───────┘
                   │
                   ▼
            ┌──────────────┐
            │重启完成      │
            └──────────────┘
```

**Windows平台重启流程图（双向确认机制）**：

```
用户执行重启命令
        │
        ▼
   ┌─────────────┐
   │检查PID文件？│
   └─────┬───────┘
         │
    ┌────▼────┐         ┌──────────────┐
    │文件存在？│   否    │直接启动新进程│
    └────┬────┘  ────▶  └──────┬───────┘
         │是                    │
         ▼                      │
   ┌──────────────┐              │
   │读取旧进程PID │              │
   └──────┬───────┘              │
          │                      │
          ▼                      │
   ┌──────────────┐              │
   │检查旧进程    │   不运行      │
   │是否运行？    │ ──────────────┤
   └──────┬───────┘              │
          │运行                   │
          ▼                      │
                                 │
程序A端：                        │
   ┌──────────────────────┐      │
   │创建Kill事件          │      │
   │NFServer_Kill_{PID}   │      │
   └──────┬───────────────┘      │
          │                      │
          ▼                      │
   ┌──────────────────────┐      │
   │SetEvent发送Kill信号  │      │
   └──────┬───────────────┘      │
          │                      │
          ▼                      │
   ┌──────────────────────┐      │
   │Sleep(50ms)避免竞态   │      │
   └──────┬───────────────┘      │
          │                      │
          ▼                      │
   ┌──────────────────────┐      │
   │等待KillSuccess事件   │      │
   │最多15秒超时          │      │
   └──────┬───────────────┘      │
          │                      │
     ┌─────▼─────┐                │
     │收到确认？  │                │
     └─────┬─────┘                │
          │                      │
      ┌────▼────┐                 │
      │是       │                 │
      └────┬────┘                 │
          │                      │
          ▼                      │
   ┌──────────────────────┐      │
   │从共享内存恢复数据    │      │
   │状态（立即开始）      │      │
   └──────┬───────────────┘      │
          │                      │
          ▼                      │
   ┌──────────────────────┐      │
   │初始化并启动服务      │      │
   │（与程序B退出并行）   │      │
   └──────┬───────────────┘      │
          │                      │
          └──────────────────────┤
                                 │
程序B端：                        │
   ┌──────────────────────┐      │
   │事件处理线程检测到    │      │
   │Kill事件              │      │
   └──────┬───────────────┘      │
          │                      │
          ▼                      │
   ┌──────────────────────┐      │
   │设置停服和优雅退出    │      │
   │标志                  │      │
   └──────┬───────────────┘      │
          │                      │
          ▼                      │
   ┌──────────────────────┐      │
   │检查所有服务是否停止  │      │
   │最多10秒超时          │      │
   └──────┬───────────────┘      │
          │                      │
          ▼                      │
   ┌──────────────────────┐      │
   │确保数据保存到共享内存│      │
   └──────┬───────────────┘      │
          │                      │
          ▼                      │
   ┌──────────────────────┐      │
   │发送KillSuccess确认   │ ─────┼──┐
   │事件                  │      │  │
   └──────┬───────────────┘      │  │
          │                      │  │
          ▼                      │  │
   ┌──────────────────────┐      │  │
   │Sleep(50ms)确保信号   │      │  │
   │传递                  │      │  │
   └──────┬───────────────┘      │  │
          │                      │  │
          ▼                      │  │
   ┌──────────────────────┐      │  │ 程序A收到确认后
   │Sleep(10秒)等待       │      │  │ 立即开始恢复数据
   │程序A完成启动         │      │  │ 和启动服务
   └──────┬───────────────┘      │  │
          │                      │  │
          ▼                      │  │
   ┌──────────────────────┐      │  │
   │旧进程正常退出        │      │  │
   │exit(0)               │      │  │
   └──────────────────────┘      │  │
                                 │  │
                                 ▼  │
                          ┌──────────────┐
                          │重启完成      │ ◀┘
                          └──────────────┘
```

**流程对比说明**：

| 阶段 | Linux实现 | Windows实现 |
|------|-----------|-------------|
| **信号发送** | 发送SIGUNUSED信号 | 创建并触发Kill事件 |
| **确认机制** | 单向信号 + 进程状态检测 | 双向事件确认机制 |
| **超时处理** | 10秒后强制SIGKILL | 15秒等待 + 自动退出保护 |
| **竞态处理** | 信号队列机制 | 延迟50ms + 轮询检测 |
| **数据保护** | 系统级共享内存自动持久化 | 手动确认数据保存完成 |

##### ⚠️ Windows平台注意事项

由于Windows和Linux在共享内存生命周期管理上的差异，Windows平台有以下特殊考虑：

**🚨 重要风险警告**：
- **Windows平台热更重启仅供开发调试使用**
- **生产环境禁止使用Windows热更功能**
- **存在数据完全丢失的巨大风险**

**问题**：Windows系统在进程终止时会自动释放共享内存对象，导致数据丢失。

**风险详情**：
1. **数据丢失风险**：一旦出现异常，可能导致所有游戏数据完全丢失
2. **时序依赖风险**：依赖复杂的双向确认机制，任何环节失败都可能造成数据损失
3. **系统稳定性风险**：Windows共享内存机制不如Linux稳定，容易受系统负载影响
4. **恢复困难**：数据丢失后难以恢复，影响开发进度

**适用场景**：
- ✅ **开发环境**：代码调试、功能测试
- ✅ **测试环境**：集成测试、压力测试
- ❌ **生产环境**：绝对禁止使用
- ❌ **重要数据环境**：任何包含重要数据的环境都不应使用

**生产环境建议**：
- 使用传统的停服-更新-启服流程
- 实施完善的数据备份策略
- 考虑使用容器化部署方案
- 建议迁移到Linux平台获得更好的热更支持

**解决方案**：
1. **双向确认机制**：确保旧进程完全释放资源后再启动新进程
2. **超时保护**：避免死锁（程序A等待15秒，程序B自动退出10秒）
3. **详细日志记录**：完整跟踪重启过程
4. **数据备份策略**：重要数据定期备份到持久化存储

**开发调试最佳实践**：
```bash
# 1. 确保共享内存配置正确
BusLength = 20971520  # 20M共享内存

# 2. 监控重启日志
tail -f logs/NFGameServer_*.log | grep -E "(Kill|Restart|Graceful)"

# 3. 验证数据完整性
# 重启后检查关键数据是否正确恢复

# 4. 定期备份测试数据
# 避免因意外数据丢失影响开发进度
```

##### 📊 性能指标

- **重启时间**：通常2-5秒完成
- **数据丢失率**：0%（正常情况下）
- **服务中断时间**：< 1秒
- **内存使用**：重启过程中内存使用峰值约为平时的1.5倍
- **CPU占用**：重启期间CPU使用率短暂上升到50-80%

#### 3. Lua脚本热更新
- **实时更新业务逻辑**：无需重启服务器
- **支持单文件或批量更新**
- **自动依赖检测和重载**

### ⚙️ 初始化任务系统

**分阶段初始化保证**：
- **任务分组管理**：按功能将初始化任务分为4个主要组
- **依赖关系控制**：确保任务按正确的依赖顺序执行
- **状态实时跟踪**：可以实时查看初始化进度和状态
- **超时监控**：自动检测和报告超时任务
- **事件驱动**：基于事件机制实现模块间解耦

**任务组类型**：
```bash
1. 服务器连接任务组 → 建立服务器间连接
2. 配置数据加载任务组 → 加载Lua配置和描述存储  
3. 数据库数据加载任务组 → 从数据库加载全局数据
4. 服务器注册任务组 → 完成服务器间注册认证
```

### 🌐 分布式架构

- **服务器ID机制**：类似IP地址的服务器标识（如1.11.1.1）
- **路由代理**：RouteAgent处理同机通信，RouteServer处理跨机通信
- **服务发现**：Master服务器作为注册中心
- **负载均衡**：支持多实例负载分担

### 📜 X脚本扩展系统

NFShmXFrame的"X"核心特色：强大的多语言脚本扩展能力

**当前支持**：
- **🌙 Lua脚本引擎**：
  - ✅ 单线程Lua插件（已实现）
  - 🔥 Lua脚本实时热更新
  - 🔗 C++与Lua无缝互调
  - 📦 完整的Lua业务逻辑支持

**未来规划**：
- **🌟 多线程Actor Lua系统**（类似Skynet架构）
- **🐍 Python脚本插件**：支持Python业务逻辑
- **🌐 JavaScript脚本插件**：前后端统一语言

**X脚本系统优势**：
- 🎯 **混合开发模式**：C++负责性能关键部分，脚本处理业务逻辑
- ⚡ **快速迭代**：脚本修改无需重新编译，大幅提升开发效率
- 🔄 **热更新支持**：业务逻辑可以实时更新，无需停服
- 🛠️ **多语言选择**：根据团队技能栈选择最适合的脚本语言

### 🛠️ 开发工具

- **Excel工具**：Excel配置自动生成代码和数据库
- **Lua配置系统**：采用Lua作为配置语言，支持灵活的配置管理
- **协议生成**：基于Protobuf的协议自动生成
- **服务器控制器**：NFServerController统一管理工具
- **性能分析**：内置性能监控和分析系统

## 📚 详细文档

### 核心架构文档
- [📖 插件框架架构详解](docs/NFShmXFrame插件框架架构详解.md) - 深入理解框架设计原理
- [🔧 插件系统三大核心组件详解](docs/NFShmXFrame插件系统三大核心组件详解.md) - 掌握插件开发
- [🏗️ 服务器代码组织结构分析](docs/NFShmXFrame服务器代码组织结构分析.md) - 了解项目结构

### 启动和运行文档
- [🚀 服务器启动参数与执行流程详解](docs/NFShmXFrame服务器启动参数与执行流程详解.md) - 掌握启动配置
- [⚙️ 服务器初始化启动任务系统详解](docs/NFShmXFrame服务器初始化启动任务系统详解.md) - 理解启动流程
- [🔄 服务器运行流程详解](docs/NFShmXFrame服务器运行流程详解.md) - 了解运行机制

### 运维和管理文档
- [🔥 服务器热更重启详解](docs/NFShmXFrame服务器热更重启详解.md) - 实现零停机更新
- [🔄 服务器重载配置详解](docs/NFShmXFrame服务器重载配置详解.md) - 配置热重载技术
- [🔨 编译指南](docs/NFShmXFrame编译指南.md) - 详细编译说明
- [🚀 分布式启动文档](docs/NFShmXFrame分布式启动.md) - 分布式部署和管理

### 示例和教程文档
- [🎯 服务器示例文档](docs/NFShmXFrame服务器example.md) - Tutorial和MMO示例详解

## 💡 使用示例

### 🎯 Tutorial示例项目

NFShmXFrame提供了完整的Tutorial示例项目，位于`src/NFTest/NFTutorialPlugin/`目录下，展示了框架的核心功能。

#### 启动Tutorial示例

```bash
# 启动Tutorial示例服务器
./NFPluginLoader --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --restart

# 停止服务器
./NFPluginLoader --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --Stop

# 重载配置
./NFPluginLoader --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --Reload
```

#### 基础模块开发示例

```cpp
// NFTutorialModule.h - 模块头文件
class NFTutorialModule : public NFIDynamicModule
{
public:
    NFTutorialModule(NFIPluginManager* p);
    virtual ~NFTutorialModule();

    virtual int Awake();
    virtual int Init();
    virtual int Tick();
    virtual int Shut();
    virtual int Finalize();
    virtual int OnDynamicPlugin();
    virtual int OnTimer(uint32_t nTimerID) override;

private:
    uint32_t m_idCount;
};

// NFTutorialModule.cpp - 模块实现
NFTutorialModule::NFTutorialModule(NFIPluginManager* p): NFIDynamicModule(p)
{
    m_idCount = 0;
}

int NFTutorialModule::Awake()
{
    NFLogError(NF_LOG_DEFAULT, 0, "tutorial awake...........");
    SetTimer(TUTORIAL_TIMER_ID, 10000); // 设置10秒定时器
    return 0;
}

int NFTutorialModule::Init()
{
    NFLogError(NF_LOG_DEFAULT, 0, "tutorial init...........");
    return 0;
}

int NFTutorialModule::OnTimer(uint32_t nTimerID)
{
    if (nTimerID == TUTORIAL_TIMER_ID)
    {
        m_idCount++;
        NFLogError(NF_LOG_DEFAULT, 0, "id count:{}.......", m_idCount);
    }
    return 0;
}
```

#### 插件开发示例

```cpp
// NFTutorialPlugin.h - 插件头文件
class NFTutorialPlugin : public NFIPlugin
{
public:
    explicit NFTutorialPlugin(NFIPluginManager* p):NFIPlugin(p) {}

    virtual int GetPluginVersion();
    virtual std::string GetPluginName();
    virtual void Install();
    virtual void Uninstall();
    virtual bool InitShmObjectRegister() override;
};

// NFTutorialPlugin.cpp - 插件实现
int NFTutorialPlugin::GetPluginVersion()
{
    return 0;
}

std::string NFTutorialPlugin::GetPluginName()
{
    return GET_CLASS_NAME(NFTutorialPlugin);
}

void NFTutorialPlugin::Install()
{
    REGISTER_MODULE(m_pObjPluginManager, NFTutorialModule, NFTutorialModule);
}

void NFTutorialPlugin::Uninstall()
{
    UNREGISTER_MODULE(m_pObjPluginManager, NFTutorialModule, NFTutorialModule);
}

bool NFTutorialPlugin::InitShmObjectRegister()
{
    REGISTER_SINGLETON_SHM_OBJ(NFTutorialShmObj);
    return true;
}
```

#### 共享内存对象示例

```cpp
// NFTutorialShmObj.h - 共享内存对象头文件
class NFTutorialTestData
{
public:
    NFTutorialTestData()
    {
        if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
        {
            CreateInit();
        }
        else
        {
            ResumeInit();
        }
    }

    int CreateInit()
    {
        NFLogInfo(NF_LOG_DEFAULT, 0, "CreateInit");
        m_test = 0;
        m_intVec.push_back(1);
        m_intVec.push_back(2);
        m_intVec.push_back(3);
        m_intVec.push_back(4);
        return 0;
    }

    int ResumeInit()
    {
        m_test = 10;
        NFLogInfo(NF_LOG_DEFAULT, 0, "ResumeInit");
        for (int i = 0; i < (int)m_intVec.size(); i++)
        {
            std::cout << m_intVec[i] << std::endl;
        }
        return 0;
    }

    NFShmVector<int, 10> m_intVec;  // 共享内存STL容器
    int m_test;
};

class NFTutorialShmObj : public NFObjectTemplate<NFTutorialShmObj, 100, NFObject>
{
public:
    NFTutorialShmObj();
    virtual ~NFTutorialShmObj();
    
    int CreateInit();
    int ResumeInit();
    virtual int OnTimer(int timeId, int callcount);

private:
    uint32_t m_idCount;
    int m_timerId;
    NFTutorialTestData m_data;
    NFShmVector<NFTutorialTestData, 10> m_test;
};

// NFTutorialShmObj.cpp - 共享内存对象实现
NFTutorialShmObj::NFTutorialShmObj()
{
    if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
    {
        CreateInit();
    }
    else
    {
        ResumeInit();
    }
}

int NFTutorialShmObj::CreateInit()
{
    m_idCount = 0;
    m_timerId = SetTimer(10000, 0, 0, 0, 0, 0); // 10秒定时器
    m_test.push_back(NFTutorialTestData());
    NFLogError(NF_LOG_DEFAULT, 0, "CreateInit, m_idCount:{} m_timerId:{}", m_idCount, m_timerId);
    return 0;
}

int NFTutorialShmObj::ResumeInit()
{
    NFLogError(NF_LOG_DEFAULT, 0, "ResumeInit, m_idCount:{} m_timerId:{}", m_idCount, m_timerId);
    return 0;
}

int NFTutorialShmObj::OnTimer(int timeId, int callcount)
{
    if (timeId == m_timerId)
    {
        m_idCount++;
        NFLogError(NF_LOG_DEFAULT, 0, "OnTimer, m_idCount:{} m_timerId:{}", m_idCount, m_timerId);
    }
    return 0;
}
```

#### 插件配置文件

```lua
-- Install/Plugin/Tutorial/Plugin.lua
require "Common"

LoadPlugin = {
    TutorialAllServer = {
        ServerPlugins = {
            -- 基础框架引擎
            "NFKernelPlugin",
            "NFTutorialPlugin",  -- 我们的Tutorial插件
            "NFShmPlugin",       -- 共享内存插件
        };
        ServerType = NF_ST_NONE;
    },
}
```

### 初始化任务系统使用

```cpp
// 注册初始化任务
bool MyGameModule::Awake()
{
    // 注册连接Store服务器任务
    RegisterAppTask(NF_ST_LOGIC_SERVER, APP_INIT_CONNECT_STORE,
                   "Connect to Store Server", 
                   APP_INIT_TASK_GROUP_SERVER_CONNECT);
                   
    // 注册数据库加载任务
    RegisterAppTask(NF_ST_LOGIC_SERVER, APP_INIT_LOAD_GLOBAL_DATA_DB,
                   "Load Global Data From Store Server", 
                   APP_INIT_TASK_GROUP_SERVER_LOAD_OBJ_FROM_DB);
    
    // 订阅任务组完成事件
    Subscribe(NF_ST_LOGIC_SERVER, NFrame::NF_EVENT_SERVER_TASK_GROUP_FINISH, 
             NFrame::NF_EVENT_SERVER_TYPE, APP_INIT_TASK_GROUP_SERVER_CONNECT,
             __FUNCTION__);
    
    return true;
}

// 在适当时机标记任务完成
void MyGameModule::OnConnectSuccess()
{
    FinishAppTask(NF_ST_LOGIC_SERVER, APP_INIT_CONNECT_STORE,
                 APP_INIT_TASK_GROUP_SERVER_CONNECT);
}

// 处理任务组完成事件
int MyGameModule::OnExecute(uint32_t serverType, uint32_t nEventID, 
                           uint32_t bySrcType, uint64_t nSrcID, 
                           const google::protobuf::Message *pMessage)
{
    if (nEventID == NFrame::NF_EVENT_SERVER_TASK_GROUP_FINISH && 
        bySrcType == NFrame::NF_EVENT_SERVER_TYPE &&
        nSrcID == APP_INIT_TASK_GROUP_SERVER_CONNECT)
    {
        // 连接任务组完成，可以开始后续初始化
        StartLoadingData();
    }
    return 0;
}

### 服务器控制

```bash
# 启动Tutorial示例服务器
./NFPluginLoader --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --Start --Daemon

# 重载配置
./NFPluginLoader --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --Reload

# 热重启
./NFPluginLoader --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --Restart --Daemon

# 停止服务器
./NFPluginLoader --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --Stop
```

### HTTP管理接口

```bash
# 重启指定服务器
curl "http://127.0.0.1:6011/restart?Server=GameServer&ID=1.13.10.1"

# 重载所有服务器配置
curl "http://127.0.0.1:6011/reloadall"

# 查看服务器状态
curl "http://127.0.0.1:6011/status" | jq '.'
```

## 🔧 配置示例

### 插件配置（Plugin.lua）

```lua
-- Install/LieRenPlugin/Plugin.lua
-- 插件和服务器配置示例

require "Common"

LoadPlugin = {
    -- AllServer模式配置
    AllServer = {
        -- 框架插件（底层引擎）
        FramePlugins = {
            "NFKernelPlugin",
            "NFNetPlugin", 
            "NFShmPlugin",      -- 共享内存插件
            -- "NFMemPlugin",   -- 普通内存插件（二选一）
            "NFDBPlugin",
        },

        -- 服务器插件
        ServerPlugins = {
            "NFServerCommonPlugin",
            "NFDescStorePlugin",
            "NFMasterServerPlugin",
            "NFRouteServerPlugin",
            "NFRouteAgentServerPlugin", 
            "NFStoreServerPlugin",
            "NFProxyServerPlugin",
            "NFGameServerPlugin",
            "NFLogicServerPlugin",
            "NFWorldServerPlugin",
            -- 其他服务器插件...
        },

        -- 业务工作插件
        WorkPlugins = {
            "NFMMOCommonPlugin",
            "NFMMOProxyPlayerPlugin",
            "NFMMOLogicPlayerPlugin",
            "NFMMOGamePlayerPlugin",
            -- 其他业务插件...
        },

        -- 服务器列表配置
        ServerType = NF_ST_NONE,
        ServerList = {
            {Server="MasterServer", ID="1.13.1.1", ServerType=NF_ST_MASTER_SERVER},
            {Server="ProxyServer", ID="1.13.4.1", ServerType=NF_ST_PROXY_SERVER},
            {Server="GameServer", ID="1.13.10.1", ServerType=NF_ST_GAME_SERVER},
            {Server="LogicServer", ID="1.13.9.1", ServerType=NF_ST_LOGIC_SERVER},
            {Server="StoreServer", ID="1.13.6.1", ServerType=NF_ST_STORE_SERVER},
            -- 更多服务器配置...
        }
    },

    -- AllMoreServer模式配置
    AllMoreServer = {
        ServerList = {
            {Server="AllServer", ID="1.13.1.1"},
            {Server="CrossAllServer", ID="1.20.1.1"},
            -- 可以添加更多服务器实例用于调试
            -- {Server="LogicServer", ID="1.13.9.2"},
            -- {Server="GameServer", ID="1.13.10.2"},
        }
    }
}
```

### 单个服务器配置（GameServer.lua）

```lua
-- Install/LieRenPlugin/GameServer.lua
-- 单个服务器详细配置示例

require "Common"

GameServer = {
    GameServer_1 = {
        ServerName = "GameServer_1",
        ServerType = NF_ST_GAME_SERVER,
        ServerId = "1.13.10.1",                    -- 服务器唯一ID
        LinkMode = "bus",                          -- 通信模式：bus/tcp
        BusLength = 20971520,                      -- 20M共享内存
        IdleSleepUS = 1000,                        -- 空闲睡眠时间(微秒)
        ServerIp = "127.0.0.1",                    -- 服务器IP
        ServerPort = 6601,                         -- 服务器端口
        MaxConnectNum = 100,                       -- 最大连接数
        NetThreadNum = 1,                          -- 网络线程数
        WorkThreadNum = 1,                         -- 工作线程数
        MaxOnlinePlayerNum = 100,                  -- 最大在线玩家数
        DefaultDBName = "proto_ff_cgzone13",       -- 默认数据库名
        HandleMsgNumPerFrame = 2000,               -- 每帧处理消息数
        
        -- 路由配置
        RouteConfig = {
            RouteAgent = "1.13.3.1",              -- 路由代理ID
            MasterIp = "127.0.0.1",               -- Master服务器IP
            MasterPort = 6511,                     -- Master服务器端口
        }
    }
}
```

### 通用配置（Common.lua）

```lua
-- Install/LieRenPlugin/Common.lua
-- 通用配置和常量定义

-- 服务器类型定义
NF_ST_MASTER_SERVER = 1
NF_ST_ROUTE_SERVER = 2
NF_ST_ROUTE_AGENT_SERVER = 3
NF_ST_PROXY_SERVER = 4
NF_ST_STORE_SERVER = 6
NF_ST_LOGIN_SERVER = 7
NF_ST_WORLD_SERVER = 8
NF_ST_LOGIC_SERVER = 9
NF_ST_GAME_SERVER = 10
-- 更多服务器类型...

-- 服务器ID组成规则
-- ServerId = "世界ID.区服ID.服务器类型.服务器索引"
-- 例如: "1.13.10.1" = 世界1.区服13.GameServer.实例1
NF_ST_WORLD_ID = 1                    -- 世界服务ID(1-15)
NF_ST_ZONE_ID = 13                    -- 区服务ID(1-4095) 
NF_ST_CROSS_ZONE_ID = 20              -- 跨服区ID

-- 网络配置
NF_INTER_SERVER_IP = "127.0.0.1"     -- 内网服务器IP
NF_EXTER_SERVER_IP = "127.0.0.1"     -- 外网服务器IP
NF_LINK_MODE = "bus"                  -- 通信模式：bus/tcp
NF_COMMON_BUS_LENGTH = 20971520       -- 20M共享内存

-- 数据库配置
NF_MYSQL_IP = "127.0.0.1"
NF_MYSQL_PORT = 3306
NF_MYSQL_DB_NAME = "proto_ff_cgzone13"
NF_MYSQL_USER = "root"
NF_MYSQL_PASSWORD = "password"
```

### 配置文件组织结构

```
Install/LieRenPlugin/
├── Plugin.lua              # 主配置文件（插件加载和服务器列表）
├── Common.lua               # 通用配置和常量定义
├── AllMoreServer.lua        # AllMoreServer模式配置
├── MasterServer.lua         # Master服务器配置
├── ProxyServer.lua          # Proxy服务器配置
├── GameServer.lua           # Game服务器配置
├── LogicServer.lua          # Logic服务器配置
├── StoreServer.lua          # Store服务器配置
├── WorldServer.lua          # World服务器配置
├── LoginServer.lua          # Login服务器配置
├── RouteServer.lua          # Route服务器配置
├── RouteAgentServer.lua     # RouteAgent服务器配置
├── CenterServer.lua         # Center服务器配置
├── WebServer.lua            # Web服务器配置
├── SnsServer.lua            # SNS服务器配置
└── LogConfig.lua            # 日志配置
```

### 配置文件使用说明

1. **Plugin.lua**：主配置文件，定义不同启动模式下的插件加载和服务器列表
2. **Common.lua**：通用配置，包含服务器类型、网络配置、数据库配置等常量
3. **单个服务器配置文件**：每种服务器类型的详细配置，包含多个实例配置
4. **AllMoreServer.lua**：调试模式下的服务器组合配置

## 🎯 应用场景

### 游戏类型支持
- **MMO游戏**：大型多人在线游戏
- **手机游戏**：移动端游戏后端
- **实时对战**：MOBA、吃鸡类游戏
- **卡牌游戏**：回合制策略游戏
- **社交游戏**：休闲社交类游戏

### 部署模式
- **开发调试**：AllServer或AllMoreServer模式，快速开发测试和跨服务器调试
- **小规模生产**：同一台物理机部署完整分布式架构，节省硬件资源
- **大规模生产**：多台物理机分布式部署，高可用高性能
- **混合云部署**：支持公有云、私有云、混合云部署

**分布式部署管理**：使用NFServerController工具统一管理多服务器部署，支持Windows图形化界面和Linux命令行操作。详细使用方法请参考：[🚀 分布式启动文档](docs/NFShmXFrame分布式启动.md)

## 🤝 贡献指南

我们欢迎任何形式的贡献！

### 贡献方式
1. **提交Issue**：报告bug、提出功能需求
2. **提交PR**：修复bug、添加新功能
3. **完善文档**：改进文档、添加示例
4. **分享经验**：分享使用经验、最佳实践

### 开发流程
1. Fork项目到个人仓库
2. 创建功能分支：`git checkout -b feature/your-feature`
3. 提交更改：`git commit -am 'Add some feature'`
4. 推送分支：`git push origin feature/your-feature`
5. 创建Pull Request

## 📄 许可证

本项目采用 [Apache License 2.0](LICENSE) 开源许可证。

## 🙏 致谢

感谢所有为NFShmXFrame项目做出贡献的开发者和用户！

### 主要贡献者
- [gaoyi](https://github.com/yigao) - 项目创始人和主要维护者

### 第三方库
- [Google Protobuf](https://github.com/protocolbuffers/protobuf) - 序列化
- [spdlog](https://github.com/gabime/spdlog) - 日志系统
- [Lua](https://www.lua.org/) - 脚本引擎
- [libevent](https://libevent.org/) - 网络库
- [MySQL](https://www.mysql.com/) - 数据库

## 📞 联系我们

- **项目主页**：https://github.com/yigao/NFShmXFrame
- **问题反馈**：https://github.com/yigao/NFShmXFrame/issues
- **讨论交流**：https://github.com/yigao/NFShmXFrame/discussions

### 💬 QQ交流群

欢迎加入NFShmXFrame技术交流群，与开发者和其他用户交流经验、分享心得！

<div align="center">
  <img src="docs/qrcode_811848532.jpg" alt="NFShmXFrame QQ群二维码" width="200" height="200">
  <br>
  <strong>扫描二维码加入QQ群</strong>
</div>

**群号**：811848532

**群内服务**：
- 🆘 技术问题解答
- 💡 功能需求讨论  
- 🔧 开发经验分享
- 📚 学习资料交流
- 🚀 项目动态更新

---

⭐ 如果这个项目对你有帮助，请给我们一个Star！

🚀 开始你的游戏服务器开发之旅吧！

## 📖 技术背景说明

### 共享内存技术的行业使用情况

**腾讯系技术栈**：
- **使用范围**：腾讯以及从腾讯出来的员工创建的公司普遍采用共享内存技术
- **技术特点**：这些公司技术方案相对保守，常年使用裸C数组编写复杂业务逻辑
- **更新缓慢**：技术栈更新较慢，长期停留在传统的C语言开发模式

**传统开发痛点**：
- **手动内存管理**：需要手写复杂的数据结构管理代码
- **开发效率低**：大量时间花费在底层内存操作上
- **维护困难**：裸C数组代码可读性差，维护成本高
- **容错性差**：手动管理容易出现内存泄漏和越界问题

### NFShmXFrame的技术创新

**现代化共享内存开发**：
- **STL容器库**：提供完整的共享内存版本STL容器
- **API兼容性**：接口与标准STL高度一致，学习成本极低
- **开发体验**：像使用标准STL一样简单，告别手写数据结构
- **功能完整**：支持vector、map、set、string、list等常用容器

**技术选择的自由度**：
- **不强制使用**：开发者可以完全基于个人爱好和公司技术方案选择
- **双重方案**：NFShmPlugin（共享内存）和NFMemPlugin（普通内存）并存
- **平滑切换**：可以根据项目需求在两种模式间灵活切换
- **团队适配**：适合不同技术背景的开发团队
