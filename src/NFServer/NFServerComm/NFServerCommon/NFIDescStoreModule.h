// -------------------------------------------------------------------------
//    @FileName         :    NFIDescStoreModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFIDescStoreModule.h
//    @Desc             :    描述存储模块接口头文件，提供配置数据存储模块的管理功能。
//                          该文件定义了描述存储模块接口类，包括描述存储注册接口、
//                          描述存储查找接口、数据库操作接口、RPC服务接口。
//                          主要功能包括提供配置数据存储模块的管理、支持描述存储的注册和注销、
//                          支持描述存储的查找和访问、提供数据库操作接口。
//                          描述存储模块接口是NFShmXFrame框架的配置管理核心组件，负责：
//                          - 配置数据存储模块的管理
//                          - 描述存储的注册和注销
//                          - 描述存储的查找和访问
//                          - 数据库操作接口
//                          - RPC服务接口
//                          - 配置数据的加载和保存
//                          - 跨服务器配置数据同步
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIModule.h"
#include "google/protobuf/message.h"
#include "NFComm/NFPluginModule/NFIAsyDBModule.h"
#include "NFComm/NFPluginModule/NFIDynamicModule.h"

class NFIDescStore;

/**
 * @brief 描述存储模块接口类
 * 
 * 该类是配置数据存储模块的管理接口，提供了：
 * - 描述存储的注册和注销功能
 * - 描述存储的查找和访问功能
 * - 数据库操作接口
 * - RPC服务接口
 * - 配置数据的加载和保存
 * 
 * 主要功能：
 * - 管理所有描述存储对象
 * - 提供统一的描述存储访问接口
 * - 支持数据库操作和RPC服务
 * - 支持配置数据的加载和保存
 * - 支持跨服务器配置数据同步
 * 
 * 使用方式：
 * @code
 * // 注册描述存储
 * pDescStoreModule->RegisterDescStore("MyDescStore", MyDescStore::GetStaticClassType());
 * 
 * // 查找描述存储
 * MyDescStore* pDescStore = pDescStoreModule->FindDescStore<MyDescStore>();
 * 
 * // 检查是否所有描述存储已加载
 * bool isLoaded = pDescStoreModule->IsAllDescStoreLoaded();
 * @endcode
 */
class NFIDescStoreModule : public NFIDynamicModule
{
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针
     */
    NFIDescStoreModule(NFIPluginManager* p):NFIDynamicModule(p)
    {

    }

    /**
     * @brief 析构函数
     */
    virtual ~NFIDescStoreModule()
    {

    }

    /**
     * @brief 注册描述存储（带数据库名）
     * 
     * 注册描述存储到模块中，指定数据库名称
     * 
     * @param strClassName 类名字符串
     * @param objType 对象类型
     * @param dbName 数据库名称
     */
	virtual void RegisterDescStore(const std::string& strClassName, int objType, const std::string& dbName) = 0;

    /**
     * @brief 注册描述存储
     * 
     * 注册描述存储到模块中
     * 
     * @param strClassName 类名字符串
     * @param objType 对象类型
     */
    virtual void RegisterDescStore(const std::string& strClassName, int objType) = 0;

    /**
     * @brief 注册扩展描述存储
     * 
     * 注册扩展描述存储到模块中
     * 
     * @param strClassName 类名字符串
     * @param objType 对象类型
     */
    virtual void RegisterDescStoreEx(const std::string& strClassName, int objType) = 0;

    /**
     * @brief 注销描述存储
     * 
     * 从模块中注销描述存储
     * 
     * @param strClassName 类名字符串
     */
	virtual void UnRegisterDescStore(const std::string& strClassName) = 0;

    /**
     * @brief 注销扩展描述存储
     * 
     * 从模块中注销扩展描述存储
     * 
     * @param strClassName 类名字符串
     */
	virtual void UnRegisterDescStoreEx(const std::string& strClassName) = 0;

    /**
     * @brief 查找描述存储
     * 
     * 根据类名查找描述存储对象
     * 
     * @param strDescName 描述存储名称
     * @return 描述存储对象指针，如果未找到返回nullptr
     */
	virtual NFIDescStore* FindDescStore(const std::string& strDescName) = 0;

    /**
     * @brief 根据文件名查找描述存储
     * 
     * 根据文件名查找描述存储对象
     * 
     * @param strDescName 描述存储名称
     * @return 描述存储对象指针，如果未找到返回nullptr
     */
	virtual NFIDescStore* FindDescStoreByFileName(const std::string& strDescName) = 0;

    /**
     * @brief 检查是否所有描述存储已加载
     * 
     * 检查所有描述存储是否已经加载完成
     * 
     * @return 是否所有描述存储已加载
     */
	virtual bool IsAllDescStoreLoaded() = 0;

    /**
     * @brief 检查是否所有描述存储数据库已加载
     * 
     * 检查所有描述存储的数据库是否已经加载完成
     * 
     * @return 是否所有描述存储数据库已加载
     */
	virtual bool IsAllDescStoreDBLoaded() = 0;

    /**
     * @brief 加载数据库描述存储
     * 
     * 当有从DB数据库加载表格的需求时，请根据不同服务器，
     * 在使用服务器的bool AfterAllConnectFinish(NF_SERVER_TYPE serverType) override接口中，调用此接口。
     * 通用的很难搞定。因为服务器之间可能有依赖,会导致有依赖关系的服务器相互等待的情况发生
     * 
     * @return 加载结果
     */
    virtual int LoadDBDescStore() = 0;

    /**
     * @brief 查找描述存储（模板版本）
     * 
     * 根据类型查找描述存储对象，提供类型安全的访问
     * 
     * @tparam T 描述存储类型
     * @return 描述存储对象指针，如果未找到或类型不匹配返回nullptr
     */
	template <typename T>
	T* FindDescStore()
	{
		NFIDescStore* pDescStore = FindDescStore(typeid(T).name());
		if (pDescStore)
		{
			if (!TIsDerived<T, NFIDescStore>::Result)
			{
				return nullptr;
			}
			//TODO OSX上dynamic_cast返回了NULL
#if NF_PLATFORM == NF_PLATFORM_APPLE
			T* pT = (T*)pDescStore;
#else
			T* pT = dynamic_cast<T*>(pDescStore);
#endif
			return pT;
		}
		return nullptr;
	}

    /**
     * @brief 根据文件名保存描述存储
     * 
     * 根据文件名保存描述存储到数据库
     * 
     * @param dbName 数据库名称
     * @param strDescName 描述存储名称
     * @param pMessage 消息对象
     * @return 保存结果
     */
    virtual int SaveDescStoreByFileName(const std::string& dbName, const std::string& strDescName, const google::protobuf::Message *pMessage) = 0;

    /**
     * @brief 根据文件名插入描述存储
     * 
     * 根据文件名插入描述存储到数据库
     * 
     * @param dbName 数据库名称
     * @param strDescName 描述存储名称
     * @param pMessage 消息对象
     * @return 插入结果
     */
    virtual int InsertDescStoreByFileName(const std::string& dbName, const std::string& strDescName, const google::protobuf::Message *pMessage) = 0;

    /**
     * @brief 根据文件名删除描述存储
     * 
     * 根据文件名从数据库删除描述存储
     * 
     * @param dbName 数据库名称
     * @param strDescName 描述存储名称
     * @param pMessage 消息对象
     * @return 删除结果
     */
    virtual int DeleteDescStoreByFileName(const std::string& dbName, const std::string& strDescName, const google::protobuf::Message *pMessage) = 0;

public:
    /**
     * @brief 通过RPC获取描述存储
     * 
     * 通过RPC服务从远程服务器获取描述存储数据
     * 
     * @param eType 服务器类型
     * @param dbName 数据库名称
     * @param table_name 表名
     * @param pMessage 消息对象
     * @return 获取结果
     */
    virtual int GetDescStoreByRpc(NF_SERVER_TYPE eType, const std::string& dbName, const std::string &table_name, google::protobuf::Message *pMessage) = 0;
};