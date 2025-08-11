# NFShmXFrame 分布式启动文档

## 📋 概述

NFShmXFrame支持分布式部署，可以在一台或多台物理机上启动多个服务器实例。为了简化分布式启动的管理，框架提供了专门的服务器控制工具：

- **Windows平台**：使用QT制作的图形化工具 `NFServerControllerQt.exe`
- **Linux平台**：使用命令行工具 `NFServerController`

## 🖥️ Windows平台分布式启动

### 工具介绍

Windows平台使用基于QT开发的图形化服务器控制工具 `NFServerControllerQt.exe`，提供直观的界面操作。

### 工具位置
```
tools/NFServerControllerQt.exe
```

### 启动方法

1. **打开控制工具**
   - 双击运行 `tools/NFServerControllerQt.exe`
   - 工具界面将显示所有可管理的服务器

2. **配置服务器列表**
   - 工具会自动读取 `tools/` 目录下的配置文件
   - 支持多种配置文件：`win_servers.conf`、`lieren_win_servers.conf` 等

3. **一键启动所有服务器**
   - 点击界面上的 **"Start All Server"** 按钮
   - 工具将自动启动所有配置的服务器实例

### 界面操作

<div align="center">
  <img src="docs/NFServerControllQt.png" alt="NFServerControllerQt界面" width="800" height="600">
  <br>
  <strong>NFServerControllerQt图形化控制界面</strong>
</div>

### 主要功能

- **服务器管理**：启动、停止、重启单个或所有服务器
- **状态监控**：实时显示服务器运行状态
- **日志查看**：集成日志查看功能
- **配置管理**：管理服务器启动配置
- **批量操作**：支持批量启动和停止
- **共享内存清理**：安全清理服务器共享内存

## 🐧 Linux平台分布式启动

### 工具介绍

Linux平台使用命令行工具 `NFServerController`，提供灵活的命令行操作和脚本化支持。

### 工具位置
```
tools/NFServerController
```

**注意**：工具已经预编译好，无需重新编译，直接使用即可。

### 通配符模式匹配

NFServerController支持通配符模式匹配，服务器ID格式为：`WorldID.RegionID.ServerType.Index`

#### 服务器类型对照表

| 类型 | 服务器名称 |
|------|------------|
| 1 | MasterServer |
| 3 | RouteAgentServer |
| 5 | ProxyServer |
| 6 | StoreServer |
| 7 | LoginServer |
| 8 | WorldServer |
| 9 | LogicServer |
| 10 | GameServer |

#### 通配符示例

- `*.*.*.*` - 所有服务器
- `*.*.10.*` - 所有游戏服务器（ServerType=10）
- `1.13.*.*` - World 1, Region 13中的所有服务器
- `1.13.1.1` - 特定服务器

### 基本使用

NFServerController使用通配符模式匹配和批量操作，命令格式为：
```bash
NFServerController "命令 目标"
```

#### 1. 启动所有服务器

```bash
# 启动所有服务器
./NFServerController "start *.*.*.*"

# 启动特定区域的服务器
./NFServerController "start 1.13.*.*"
```

#### 2. 停止所有服务器

```bash
# 停止所有服务器
./NFServerController "stop *.*.*.*"

# 停止特定类型的服务器
./NFServerController "stop *.*.10.*"  # 停止所有游戏服务器
```

#### 3. 重启所有服务器

```bash
# 重启所有服务器（使用--restart参数）
./NFServerController "restart *.*.*.*"

# 重启特定类型的服务器
./NFServerController "restart *.*.10.*"  # 重启所有游戏服务器
```

#### 4. 重载配置

```bash
# 重载所有服务器配置（热更新）
./NFServerController "reload *.*.*.*"

# 重载特定服务器配置
./NFServerController "reload 1.13.10.1"
```

### 高级功能

#### 1. 查看服务器状态

```bash
# 查看所有服务器状态
./NFServerController "check"

# 查看特定区域服务器状态
./NFServerController "check 1.13.*.*"
```

#### 2. 清理共享内存

```bash
# 清理所有服务器共享内存
./NFServerController "clear"

# 清理特定服务器共享内存
./NFServerController "clear 1.13.10.1"
```

#### 3. 查看日志

```bash
# 查看所有服务器日志
./NFServerController "log all"

# 查看特定服务器日志
./NFServerController "log GameServer"
```

## 🔧 NFServerController详细使用

### 命令格式

```bash
NFServerController [选项] "命令 目标"
```

### 命令列表

| 命令 | 描述 | 示例 |
|------|------|------|
| `start <目标>` | 启动指定服务器 | `"start *.*.*.*"` |
| `stop <目标>` | 停止指定服务器 | `"stop *.*.10.*"` |
| `restart <目标>` | 重启指定服务器（使用--restart参数） | `"restart *.*.*.*"` |
| `reload <目标>` | 重载指定服务器配置（热更新） | `"reload *.*.10.*"` |
| `check [目标]` | 显示服务器状态 | `"check"` 或 `"check 1.13.*.*"` |
| `clear [目标]` | 清理共享内存 | `"clear"` 或 `"clear 1.13.10.1"` |
| `log [目标]` | 查看服务器日志 | `"log all"` 或 `"log GameServer"` |

### 选项参数

| 选项 | 描述 | 示例 |
|------|------|------|
| `-c, --config <文件>` | 指定配置文件路径 | `-c servers.conf` |
| `-v, --verbose` | 详细输出 | `-v "start *.*.*.*"` |
| `-q, --quiet` | 静默模式 | `-q "stop *.*.*.*"` |
| `-h, --help` | 显示帮助信息 | `--help` |

### 配置文件

NFServerController使用配置文件来管理服务器列表和启动参数。

#### 配置文件位置
```
tools/linux_servers.conf          # Linux平台配置
tools/win_servers.conf            # Windows平台配置
tools/lieren_linux_servers.conf   # LieRen项目Linux配置
tools/lieren_win_servers.conf     # LieRen项目Windows配置
```

#### 配置文件格式

配置文件格式（每行一个服务器）：
```
ServerName|ServerID|ConfigPath|PluginPath|LuaScriptPath|LogPath|GameName|ExecutablePath|WorkingDir
```

#### 配置文件示例

```
# MMO服务器配置示例
MasterServer|1.13.1.1|../Install/Config|../Install/MMOPlugin|../Install/LuaScript|../Install/logs|MMO|../Install/Bin/NFServerStatic|../Install
ProxyServer|1.13.4.1|../Install/Config|../Install/MMOPlugin|../Install/LuaScript|../Install/logs|MMO|../Install/Bin/NFServerStatic|../Install
GameServer|1.13.10.1|../Install/Config|../Install/MMOPlugin|../Install/LuaScript|../Install/logs|MMO|../Install/Bin/NFServerStatic|../Install
LogicServer|1.13.9.1|../Install/Config|../Install/MMOPlugin|../Install/LuaScript|../Install/logs|MMO|../Install/Bin/NFServerStatic|../Install
StoreServer|1.13.6.1|../Install/Config|../Install/MMOPlugin|../Install/LuaScript|../Install/logs|MMO|../Install/Bin/NFServerStatic|../Install
WorldServer|1.13.8.1|../Install/Config|../Install/MMOPlugin|../Install/LuaScript|../Install/logs|MMO|../Install/Bin/NFServerStatic|../Install
```

### 脚本化部署

#### 1. 创建启动脚本

```bash
#!/bin/bash
# start_all_servers.sh

echo "Starting NFShmXFrame distributed servers..."

# 检查工具是否存在
if [ ! -f "./NFServerController" ]; then
    echo "Error: NFServerController not found!"
    exit 1
fi

# 启动所有服务器
./NFServerController "start *.*.*.*"

# 检查启动状态
sleep 5
./NFServerController "check"

echo "All servers started successfully!"
```

#### 2. 创建停止脚本

```bash
#!/bin/bash
# stop_all_servers.sh

echo "Stopping NFShmXFrame distributed servers..."

# 停止所有服务器
./NFServerController "stop *.*.*.*"

echo "All servers stopped successfully!"
```

#### 3. 创建重启脚本

```bash
#!/bin/bash
# restart_all_servers.sh

echo "Restarting NFShmXFrame distributed servers..."

# 重启所有服务器（使用--restart参数）
./NFServerController "restart *.*.*.*"

echo "All servers restarted successfully!"
```

#### 4. 创建监控脚本

```bash
#!/bin/bash
# monitor_servers.sh

while true; do
    echo "=== Server Status Check ==="
    echo "$(date)"
    ./NFServerController "check"
    echo "=========================="
    sleep 30
done
```

## 🏗️ 分布式部署架构

### 单机分布式部署

在同一台物理机上启动多个服务器实例：

```
物理机A
├── MasterServer (1.13.1.1)
├── ProxyServer (1.13.4.1)
├── GameServer (1.13.10.1)
├── LogicServer (1.13.9.1)
├── StoreServer (1.13.6.1)
└── WorldServer (1.13.8.1)
```

### 多机分布式部署

在多台物理机上分布部署服务器：

```
物理机A (主控)
├── MasterServer (1.13.1.1)
└── ProxyServer (1.13.4.1)

物理机B (游戏逻辑)
├── GameServer (1.13.10.1)
└── LogicServer (1.13.9.1)

物理机C (数据存储)
├── StoreServer (1.13.6.1)
└── WorldServer (1.13.8.1)
```

### 高可用部署

```
物理机A (主控)
├── MasterServer (1.13.1.1)
└── ProxyServer (1.13.4.1)

物理机B (游戏服务器1)
├── GameServer (1.13.10.1)
└── LogicServer (1.13.9.1)

物理机C (游戏服务器2)
├── GameServer (1.13.10.2)
└── LogicServer (1.13.9.2)

物理机D (存储服务器)
├── StoreServer (1.13.6.1)
└── WorldServer (1.13.8.1)
```

## 📊 监控和管理

### 服务器状态监控

```bash
# 实时监控所有服务器状态
watch -n 5 './NFServerController "check"'

# 监控特定区域服务器
./NFServerController "check 1.13.*.*"
```

### 日志监控

```bash
# 监控所有服务器日志
./NFServerController "log all"

# 监控特定服务器日志
./NFServerController "log GameServer"
```

### 性能监控

```bash
# 查看服务器资源使用情况
ps aux | grep NFPluginLoader

# 查看网络连接
netstat -tulpn | grep NFPluginLoader

# 查看共享内存使用
ipcs -m
```

## 🔧 故障排除

### 常见问题

#### 1. 服务器启动失败

```bash
# 检查服务器状态
./NFServerController "check 1.13.10.1"

# 查看详细日志
./NFServerController "log GameServer"

# 使用详细模式查看启动信息
./NFServerController -v "start 1.13.10.1"
```

#### 2. 服务器连接失败

```bash
# 检查网络连接
ping 127.0.0.1

# 检查端口占用
netstat -tulpn | grep 6601
```

#### 3. 共享内存问题

```bash
# 清理共享内存
./NFServerController "clear"

# 重新启动服务器
./NFServerController "restart *.*.*.*"
```

### 调试技巧

#### 1. 启用调试模式

```bash
# 使用详细模式查看启动信息
./NFServerController -v "start *.*.*.*"

# 查看详细输出
./NFServerController -v "check"
```

#### 2. 单步调试

```bash
# 逐个启动服务器进行调试
./NFServerController "start 1.13.1.1"    # 启动MasterServer
./NFServerController "start 1.13.4.1"    # 启动ProxyServer
./NFServerController "start 1.13.6.1"    # 启动StoreServer
# ... 依次启动其他服务器
```

## 📚 最佳实践

### 1. 部署前准备

- 确保所有依赖库已安装
- 检查网络配置和防火墙设置
- 准备足够的系统资源
- 备份重要配置文件

### 2. 启动顺序

建议按以下顺序启动服务器：

1. **MasterServer** - 主控服务器
2. **StoreServer** - 存储服务器
3. **WorldServer** - 世界服务器
4. **LogicServer** - 逻辑服务器
5. **GameServer** - 游戏服务器
6. **ProxyServer** - 代理服务器

### 3. 监控和维护

- 定期检查服务器状态
- 监控系统资源使用
- 及时处理错误日志
- 定期备份数据

### 4. 安全考虑

- 使用防火墙保护服务器
- 定期更新系统和依赖
- 监控异常访问
- 实施访问控制

## 🔗 相关文档

- [NFShmXFrame服务器启动参数与执行流程详解](NFShmXFrame服务器启动参数与执行流程详解.md)
- [NFShmXFrame服务器热更重启详解](NFShmXFrame服务器热更重启详解.md)
- [NFShmXFrame服务器示例文档](NFShmXFrame服务器example.md)

---

通过NFServerController工具，您可以轻松管理NFShmXFrame的分布式部署，无论是单机多实例还是多机分布式部署，都能实现统一的管理和监控。 