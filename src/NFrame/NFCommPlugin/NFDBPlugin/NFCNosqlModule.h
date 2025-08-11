// -------------------------------------------------------------------------
//    @FileName         :    NFCNosqlModule.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFCNosqlModule
//    @Desc             :    NoSQL数据库模块头文件，提供Redis等NoSQL数据库操作接口。
//                          该文件定义了NFShmXFrame框架的NoSQL数据库模块，继承自NFINosqlModule接口，
//                          提供NoSQL数据库（主要是Redis）的操作功能，支持多个NoSQL服务器的连接管理，
//                          提供对象的增删改查操作。
//                          主要功能包括管理多个NoSQL服务器连接、提供对象的选择保存删除操作、
//                          支持异步操作和连接池管理、提供数据库连接状态检查、支持服务器配置管理
//    @Description      :    NoSQL数据库模块头文件，提供Redis等NoSQL数据库操作接口
//
// -------------------------------------------------------------------------

#pragma once

#include "NFRedisDriver.h"
#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFCore/NFCommMapEx.hpp"
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFINosqlModule.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFCNosqlDriverManager.h"

/**
 * @class NFCNosqlModule
 * @brief NoSQL数据库模块实现类
 * 
 * 继承自NFINosqlModule接口，提供NoSQL数据库（主要是Redis）的操作功能。
 * 支持多个NoSQL服务器的连接管理，提供对象的增删改查操作。
 * 
 * 主要功能：
 * - 管理多个NoSQL服务器连接
 * - 提供对象的选择、保存、删除操作
 * - 支持异步操作和连接池管理
 * - 提供数据库连接状态检查
 */
class NFCNosqlModule
	: public NFINosqlModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针，用于管理模块的生命周期和依赖关系
	 */
	NFCNosqlModule(NFIPluginManager* p);
	
	/**
	 * @brief 析构函数
	 * 释放模块占用的资源，关闭数据库连接
	 */
	virtual ~NFCNosqlModule();

public:
	/**
	 * @brief 初始化模块
	 * @return 返回初始化结果，0表示成功，非0表示失败
	 */
	virtual int Init();
	
	/**
	 * @brief 关闭模块
	 * @return 返回关闭结果，0表示成功，非0表示失败
	 */
	virtual int Shut();
	
	/**
	 * @brief 模块主循环处理
	 * @return 返回处理结果，0表示成功，非0表示失败
	 * 
	 * 定期检查连接状态，处理异步操作队列等
	 */
	virtual int Tick();

public:
	/**
	 * @brief 添加NoSQL服务器（仅指定服务器ID和IP）
	 * @param strID 服务器唯一标识符
	 * @param strIP 服务器IP地址
	 * @return 返回添加结果，0表示成功，非0表示失败
	 */
	virtual int AddNosqlServer(const std::string& strID, const std::string& strIP);
	
	/**
	 * @brief 添加NoSQL服务器（指定服务器ID、IP和端口）
	 * @param strID 服务器唯一标识符
	 * @param strIP 服务器IP地址
	 * @param nPort 服务器端口号
	 * @return 返回添加结果，0表示成功，非0表示失败
	 */
	virtual int AddNosqlServer(const std::string& strID, const std::string& strIP, const int nPort);
	
	/**
	 * @brief 添加NoSQL服务器（指定服务器ID、IP、端口和密码）
	 * @param strID 服务器唯一标识符
	 * @param strIP 服务器IP地址
	 * @param nPort 服务器端口号
	 * @param strPass 服务器连接密码
	 * @return 返回添加结果，0表示成功，非0表示失败
	 */
	virtual int AddNosqlServer(const std::string& strID, const std::string& strIP, const int nPort, const std::string& strPass);

public:
	/**
	 * @brief 选择对象数据
	 * @param strID 服务器ID
	 * @param select 选择条件
	 * @param select_res 选择结果
	 * @return 返回操作结果，0表示成功，非0表示失败
	 */
    virtual int SelectObj(const std::string& strID, const NFrame::storesvr_selobj &select, NFrame::storesvr_selobj_res &select_res);
    
    /**
     * @brief 保存对象数据（使用选择结构）
     * @param strID 服务器ID
     * @param select 保存的数据结构
     * @param select_res 保存结果
     * @return 返回操作结果，0表示成功，非0表示失败
     */
    virtual int SaveObj(const std::string& strID, const NFrame::storesvr_selobj &select,
                        NFrame::storesvr_selobj_res &select_res);
                        
    /**
     * @brief 保存对象数据（使用插入结构）
     * @param strID 服务器ID
     * @param select 插入的数据结构
     * @param select_res 插入结果
     * @return 返回操作结果，0表示成功，非0表示失败
     */
    virtual int SaveObj(const std::string& strID, const NFrame::storesvr_insertobj &select, NFrame::storesvr_insertobj_res &select_res);
    
    /**
     * @brief 保存对象数据（使用修改结构）
     * @param strID 服务器ID
     * @param select 修改的数据结构
     * @param select_res 修改结果
     * @return 返回操作结果，0表示成功，非0表示失败
     */
    virtual int SaveObj(const std::string& strID, const NFrame::storesvr_modobj &select, NFrame::storesvr_modobj_res &select_res);
    
    /**
     * @brief 保存对象数据（使用更新结构）
     * @param strID 服务器ID
     * @param select 更新的数据结构
     * @param select_res 更新结果
     * @return 返回操作结果，0表示成功，非0表示失败
     */
    virtual int SaveObj(const std::string& strID, const NFrame::storesvr_updateobj &select, NFrame::storesvr_updateobj_res &select_res);

    /**
     * @brief 删除对象数据（使用删除结构）
     * @param strID 服务器ID
     * @param select 删除条件
     * @return 返回操作结果，0表示成功，非0表示失败
     */
    virtual int DeleteObj(const std::string& strID, const NFrame::storesvr_delobj &select);
    
    /**
     * @brief 删除对象数据（使用插入结构作为删除条件）
     * @param strID 服务器ID
     * @param select 删除条件（复用插入结构）
     * @return 返回操作结果，0表示成功，非0表示失败
     */
    virtual int DeleteObj(const std::string& strID, const NFrame::storesvr_insertobj &select);
    
    /**
     * @brief 删除对象数据（使用修改结构作为删除条件）
     * @param strID 服务器ID
     * @param select 删除条件（复用修改结构）
     * @return 返回操作结果，0表示成功，非0表示失败
     */
    virtual int DeleteObj(const std::string& strID, const NFrame::storesvr_modobj &select);

public:
    /**
     * @brief 连接到NoSQL数据库
     * @param strIP 服务器IP地址
     * @param nPort 服务器端口号
     * @param strPass 连接密码
     * @return 总是返回false，此方法未实现具体功能
     * @note 此方法为接口方法，实际连接通过AddNosqlServer方法实现
     */
    virtual bool Connect(const std::string& strIP, const int nPort, const std::string& strPass) { return false; };
    
    /**
     * @brief 检查数据库连接是否可用
     * @return 返回连接状态，true表示可用，false表示不可用
     */
    virtual bool Enable();
    
    /**
     * @brief 检查数据库是否繁忙
     * @return 返回繁忙状态，true表示繁忙，false表示空闲
     */
    virtual bool Busy();
    
    /**
     * @brief 保持数据库连接活跃
     * @return 返回保活操作结果，true表示成功，false表示失败
     */
    virtual bool KeepLive();

public:
	/**
	 * @brief 获取指定服务器的NoSQL驱动
	 * @param strID 服务器唯一标识符
	 * @return 返回对应的NoSQL驱动指针，如果不存在则返回nullptr
	 */
	virtual NFINosqlDriver* GetNosqlDriver(const std::string& strID);

protected:
    /**
     * @brief NoSQL驱动管理器
     * 负责管理所有NoSQL数据库连接和驱动实例
     */
    NFCNosqlDriverManager* m_pNoSqlDriverManager;
};
