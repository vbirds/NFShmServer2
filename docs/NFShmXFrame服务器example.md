# NFShmXFrame 服务器示例文档

## 📋 概述

本文档介绍NFShmXFrame框架提供的两个主要示例项目：
- **Tutorial示例**：基础功能演示，适合初学者学习框架核心概念
- **MMO示例**：完整的MMO游戏服务器示例，展示实际项目开发

## 🎯 Tutorial示例项目

### 项目位置
```
src/NFTest/NFTutorialPlugin/
```

### 功能特点
- 基础模块开发示例
- 插件系统演示
- 共享内存对象使用
- 定时器功能展示
- 热更新支持

### 启动命令

#### Linux平台
```bash
# 启动Tutorial示例服务器
./NFPluginLoader --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --restart

# 停止服务器
./NFPluginLoader --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --Stop

# 重载配置
./NFPluginLoader --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --Reload

# 热重启
./NFPluginLoader --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --Restart --Daemon
```

#### Windows平台
```bash
# 启动Tutorial示例服务器
NFPluginLoader .exe --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --restart

# 停止服务器
NFPluginLoader .exe --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --Stop

# 重载配置
NFPluginLoader .exe --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --Reload

# 热重启
NFPluginLoader .exe --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --Restart --Daemon
```

### 核心代码示例

#### 1. 模块开发 (NFTutorialModule)

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

#### 2. 插件开发 (NFTutorialPlugin)

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

#### 3. 共享内存对象 (NFTutorialShmObj)

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

#### 4. 插件配置文件

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

## 🎮 MMO示例项目

### 项目位置
```
game/MMO/
```

### 功能特点
- 完整的MMO游戏服务器架构
- 多服务器分布式部署
- 玩家管理、场景管理
- 网络通信、数据存储
- 完整的游戏逻辑实现

### 启动命令

#### Linux平台
```bash
# 启动MMO示例服务器
./NFPluginLoader --Server=AllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --Game=MMO --restart

# 停止服务器
./NFPluginLoader --Server=AllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --Game=MMO --Stop

# 重载配置
./NFPluginLoader --Server=AllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --Game=MMO --Reload

# 热重启
./NFPluginLoader --Server=AllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --Game=MMO --Restart --Daemon
```

#### Windows平台
```bash
# 启动MMO示例服务器
NFServerStatic.exe --Server=AllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --Game=MMO --restart

# 停止服务器
NFServerStatic.exe --Server=AllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --Game=MMO --Stop

# 重载配置
NFServerStatic.exe --Server=AllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --Game=MMO --Reload

# 热重启
NFServerStatic.exe --Server=AllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --Game=MMO --Restart --Daemon
```

### MMO服务器架构

MMO示例采用完整的分布式架构，包含以下服务器类型：

#### 1. MasterServer (主控服务器)
- **功能**：服务器注册中心，管理所有服务器节点
- **端口**：6511
- **配置**：`Install/Plugin/MMO/MasterServer.lua`

#### 2. ProxyServer (代理服务器)
- **功能**：客户端连接代理，负载均衡
- **端口**：6601
- **配置**：`Install/Plugin/MMO/ProxyServer.lua`

#### 3. GameServer (游戏服务器)
- **功能**：游戏逻辑处理，场景管理
- **端口**：6602
- **配置**：`Install/Plugin/MMO/GameServer.lua`

#### 4. LogicServer (逻辑服务器)
- **功能**：业务逻辑处理，数据计算
- **端口**：6603
- **配置**：`Install/Plugin/MMO/LogicServer.lua`

#### 5. StoreServer (存储服务器)
- **功能**：数据存储，数据库操作
- **端口**：6604
- **配置**：`Install/Plugin/MMO/StoreServer.lua`

#### 6. WorldServer (世界服务器)
- **功能**：跨服功能，全局数据管理
- **端口**：6605
- **配置**：`Install/Plugin/MMO/WorldServer.lua`

### MMO插件配置

```lua
-- Install/Plugin/MMO/Plugin.lua
require "Common"

LoadPlugin = {
    AllServer = {
        -- 框架插件（底层引擎）
        FramePlugins = {
            "NFKernelPlugin",
            "NFNetPlugin", 
            "NFShmPlugin",      -- 共享内存插件
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
        },

        -- 业务工作插件
        WorkPlugins = {
            "NFMMOCommonPlugin",
            "NFMMOProxyPlayerPlugin",
            "NFMMOLogicPlayerPlugin",
            "NFMMOGamePlayerPlugin",
        },

        -- 服务器列表配置
        ServerType = NF_ST_NONE,
        ServerList = {
            {Server="MasterServer", ID="1.13.1.1", ServerType=NF_ST_MASTER_SERVER},
            {Server="ProxyServer", ID="1.13.4.1", ServerType=NF_ST_PROXY_SERVER},
            {Server="GameServer", ID="1.13.10.1", ServerType=NF_ST_GAME_SERVER},
            {Server="LogicServer", ID="1.13.9.1", ServerType=NF_ST_LOGIC_SERVER},
            {Server="StoreServer", ID="1.13.6.1", ServerType=NF_ST_STORE_SERVER},
            {Server="WorldServer", ID="1.13.8.1", ServerType=NF_ST_WORLD_SERVER},
        }
    },
}
```

### MMO核心功能模块

#### 1. 玩家管理模块
- 玩家登录/登出
- 玩家数据管理
- 玩家状态同步

#### 2. 场景管理模块
- 场景创建/销毁
- 场景数据管理
- 场景同步

#### 3. 网络通信模块
- 客户端连接管理
- 消息路由
- 数据包处理

#### 4. 数据存储模块
- 数据库操作
- 数据缓存
- 数据同步

## 🔧 启动参数说明

### 通用参数

| 参数 | 描述 | 示例 | 必需 |
|------|------|------|------|
| `--Server` | 服务器类型 | `TutorialAllServer`, `AllServer` | ✅ |
| `--ID` | 服务器唯一标识 | `1.13.1.1` | ✅ |
| `--Config` | 配置文件路径 | `../../Config` | ✅ |
| `--Plugin` | 插件配置路径 | `../../Plugin` | ✅ |
| `--game`/`--Game` | 游戏类型 | `Tutorial`, `MMO` | ✅ |
| `--Start` | 启动服务器 | 无需参数值 | ❌ |
| `--Stop` | 停止服务器 | 无需参数值 | ❌ |
| `--Reload` | 重载配置 | 无需参数值 | ❌ |
| `--Restart` | 热重启 | 无需参数值 | ❌ |
| `--Daemon` | 守护进程模式 | 无需参数值 | ❌ |

### 平台差异

#### Linux平台
- 使用 `./NFPluginLoader` 启动
- 支持完整的信号处理
- 共享内存热更新功能完整

#### Windows平台
- 使用 `NFServerStatic.exe` 启动
- 共享内存热更新仅限开发调试
- 生产环境建议使用传统停服更新

## 📊 示例对比

| 特性 | Tutorial示例 | MMO示例 |
|------|-------------|---------|
| **复杂度** | 简单，适合学习 | 复杂，适合生产 |
| **服务器数量** | 单服务器 | 多服务器分布式 |
| **功能完整性** | 基础功能演示 | 完整游戏功能 |
| **适用场景** | 学习框架 | 实际开发 |
| **启动模式** | TutorialAllServer | AllServer |
| **插件数量** | 3个核心插件 | 10+个业务插件 |

## 🚀 快速开始

### 1. 编译项目
```bash
mkdir build && cd build
cmake ..
make -j4
```

### 2. 运行Tutorial示例
```bash
# Linux
./NFPluginLoader --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --restart

# Windows
NFServerStatic.exe --Server=TutorialAllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --game=Tutorial --restart
```

### 3. 运行MMO示例
```bash
# Linux
./NFPluginLoader --Server=AllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --Game=MMO --restart

# Windows
NFServerStatic.exe --Server=AllServer --ID=1.13.1.1 --Config=../../Config --Plugin=../../Plugin --Game=MMO --restart
```

### 4. 查看日志
```bash
# 查看服务器日志
tail -f logs/NF*.log

# 查看特定服务器日志
tail -f logs/NFGameServer_*.log
```

## 📚 学习建议

### 初学者
1. 先运行Tutorial示例，理解基础概念
2. 阅读Tutorial代码，学习模块开发
3. 尝试修改Tutorial代码，熟悉框架API
4. 参考文档，深入理解架构设计

### 进阶开发者
1. 运行MMO示例，了解完整架构
2. 分析MMO代码，学习最佳实践
3. 基于MMO示例开发自己的游戏
4. 参与社区讨论，分享开发经验

## 🔗 相关文档

- [NFShmXFrame插件框架架构详解](NFShmXFrame插件框架架构详解.md)
- [NFShmXFrame服务器启动参数与执行流程详解](NFShmXFrame服务器启动参数与执行流程详解.md)
- [NFShmXFrame服务器热更重启详解](NFShmXFrame服务器热更重启详解.md)
- [NFShmXFrame编译指南](NFShmXFrame编译指南.md)

---

通过这两个示例项目，您可以全面了解NFShmXFrame框架的使用方法和最佳实践。建议从Tutorial示例开始学习，然后逐步深入MMO示例的复杂功能。 