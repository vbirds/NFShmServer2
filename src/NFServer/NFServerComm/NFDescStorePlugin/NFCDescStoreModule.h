// -------------------------------------------------------------------------
//    @FileName         :    NFCDescStoreModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCDescStoreModule
//    @Desc             :    NFShmXFrame描述存储模块接口定义
//                          提供配置表数据的存储、加载和管理功能
//                          支持数据库存储和文件存储两种方式，提供统一的访问接口
//
// -------------------------------------------------------------------------

#pragma once

#include "NFServerComm/NFServerCommon/NFIDescStoreModule.h"
#include "NFServerComm/NFServerCommon/NFIDescStore.h"
#include "NFComm/NFPluginModule/NFEventObj.h"
#include "NFComm/NFPluginModule/NFEventObj.h"
#include "NFServerComm/NFServerCommon/NFIDescStoreEx.h"
#include <unordered_map>

/**
 * @brief 描述存储模块实现类
 * 
 * NFCDescStoreModule是NFIDescStoreModule接口的具体实现，
 * 负责管理配置表数据的存储、加载和访问功能。
 * 
 * 该模块提供以下主要功能：
 * - 配置表数据的数据库存储和文件存储
 * - 支持动态加载和热更新配置表
 * - 提供统一的配置表访问接口
 * - 支持多种数据格式和存储方式
 * - 管理描述存储对象的生命周期
 * 
 * @author Gao.Yi
 * @date 2022-09-18
 */
class NFCDescStoreModule : public NFIDescStoreModule
{
public:
	/**
	 * @brief 构造函数
	 * 
	 * 初始化描述存储模块，设置插件管理器
	 * 
	 * @param p 插件管理器指针
	 */
	NFCDescStoreModule(NFIPluginManager* p);

	/**
	 * @brief 析构函数
	 *
	 * 清理描述存储模块资源
	 */
	~NFCDescStoreModule() override;

	/**
	 * @brief 加载配置
	 *
	 * 加载描述存储模块的配置文件
	 *
	 * @return 加载结果状态码
	 */
	virtual int LoadConfig() override;

	/**
	 * @brief 模块唤醒
	 *
	 * 在模块初始化完成后调用，进行必要的初始化工作
	 *
	 * @return 初始化结果状态码
	 */
	virtual int Awake() override;

	/**
	 * @brief 模块定时更新
	 *
	 * 每帧调用，处理模块的定时任务
	 *
	 * @return 处理结果状态码
	 */
	virtual int Tick() override;

	/**
	 * @brief 重新加载配置
	 *
	 * 当配置文件发生变化时，重新加载配置
	 *
	 * @return 重载结果状态码
	 */
	virtual int OnReloadConfig() override;

	/**
	 * @brief 检查停服条件
	 *
	 * 停服之前，检查服务器是否满足停服条件
	 *
	 * @return 检查结果状态码
	 */
	virtual int CheckStopServer() override;

	/**
	 * @brief 停服处理
	 *
	 * 停服之前，做一些操作，满足停服条件
	 *
	 * @return 处理结果状态码
	 */
	virtual int StopServer() override;

	/**
	 * @brief 所有连接完成后的处理
	 *
	 * 当所有服务器连接建立完成后调用
	 *
	 * @param serverType 服务器类型
	 * @return 处理结果状态码
	 */
	virtual int AfterAllConnectFinish(NF_SERVER_TYPE serverType) override;

public:
	/**
	 * @brief 注册描述存储模块（指定数据库）
	 *
	 * 注册描述存储模块，支持指定数据库名称
	 *
	 * @param strClassName 类名
	 * @param objType 对象类型
	 * @param dbName 数据库名称
	 */
	virtual void RegisterDescStore(const std::string& strClassName, int objType, const std::string& dbName) override;

	/**
	 * @brief 注册描述存储模块（默认数据库）
	 *
	 * 注册描述存储模块，使用默认数据库
	 *
	 * @param strClassName 类名
	 * @param objType 对象类型
	 */
	virtual void RegisterDescStore(const std::string& strClassName, int objType) override;

	/**
	 * @brief 注册扩展描述存储模块
	 *
	 * 注册扩展描述存储模块，提供额外的功能
	 *
	 * @param strClassName 类名
	 * @param objType 对象类型
	 */
	virtual void RegisterDescStoreEx(const std::string& strClassName, int objType) override;

	/**
	 * @brief 反注册描述存储模块
	 * @param strClassName 类名
	 * @note 用于动态卸载不再需要的描述存储模块，释放相关资源
	 */
	virtual void UnRegisterDescStore(const std::string& strClassName) override;

	/**
	 * @brief 反注册扩展描述存储模块
	 * @param strClassName 类名
	 * @note 用于动态卸载不再需要的扩展描述存储模块，释放相关资源
	 */
	virtual void UnRegisterDescStoreEx(const std::string& strClassName) override;

	/**
	 * @brief 查找描述存储对象
	 *
	 * 根据描述名称查找对应的描述存储对象
	 *
	 * @param strDescName 描述名称
	 * @return 描述存储对象指针，如果未找到返回nullptr
	 */
	virtual NFIDescStore* FindDescStore(const std::string& strDescName) override;

	/**
	 * @brief 根据文件名查找描述存储对象
	 *
	 * 根据文件名查找对应的描述存储对象
	 *
	 * @param strDescName 文件名
	 * @return 描述存储对象指针，如果未找到返回nullptr
	 */
	virtual NFIDescStore* FindDescStoreByFileName(const std::string& strDescName) override;

	/**
	 * @brief 初始化模块
	 *
	 * 初始化描述存储模块，包括资源加载和对象创建
	 *
	 * @return 初始化结果状态码
	 */
	virtual int Initialize();

	/**
	 * @brief 加载文件描述存储
	 *
	 * 从文件系统加载所有描述存储数据
	 *
	 * @return 加载结果状态码
	 */
	virtual int LoadFileDescStore();
	/**
	 * @brief 从数据库加载描述存储数据
	 * @return 加载结果状态码
	 * @note 当需要从数据库加载配置表时，应在各服务器的AfterAllConnectFinish回调中调用此接口
	 */
	int LoadDBDescStore() override;

	/**
	 * @brief 添加描述存储对象
	 *
	 * 向模块中添加描述存储对象
	 *
	 * @param strDescName 描述名称
	 * @param pDesc 描述存储对象指针
	 */
	virtual void AddDescStore(const std::string& strDescName, NFIDescStore* pDesc);

	/**
	 * @brief 添加扩展描述存储对象
	 *
	 * 向模块中添加扩展描述存储对象
	 *
	 * @param strDescName 描述名称
	 * @param pDescEx 扩展描述存储对象指针
	 */
	virtual void AddDescStoreEx(const std::string& strDescName, NFIDescStoreEx* pDescEx);

	/**
	 * @brief 移除描述存储对象
	 *
	 * 从模块中移除指定的描述存储对象
	 *
	 * @param strDescName 描述名称
	 */
	virtual void RemoveDescStore(const std::string& strDescName);

	/**
	 * @brief 根据文件名插入描述存储数据
	 *
	 * 向指定数据库的文件中插入描述存储数据
	 *
	 * @param dbName 数据库名称
	 * @param strDescName 描述名称
	 * @param pMessage 要插入的消息数据
	 * @return 操作结果状态码
	 */
	virtual int InsertDescStoreByFileName(const std::string& dbName, const std::string& strDescName, const google::protobuf::Message* pMessage) override;

	/**
	 * @brief 根据文件名删除描述存储数据
	 *
	 * 从指定数据库的文件中删除描述存储数据
	 *
	 * @param dbName 数据库名称
	 * @param strDescName 描述名称
	 * @param pMessage 要删除的消息数据
	 * @return 操作结果状态码
	 */
	virtual int DeleteDescStoreByFileName(const std::string& dbName, const std::string& strDescName, const google::protobuf::Message* pMessage) override;

	/**
	 * @brief 根据文件名保存描述存储数据
	 *
	 * 向指定数据库的文件中保存描述存储数据
	 *
	 * @param dbName 数据库名称
	 * @param strDescName 描述名称
	 * @param pMessage 要保存的消息数据
	 * @return 操作结果状态码
	 */
	virtual int SaveDescStoreByFileName(const std::string& dbName, const std::string& strDescName, const google::protobuf::Message* pMessage) override;

	/**
	 * @brief 初始化所有描述存储对象
	 *
	 * 初始化所有已注册的描述存储对象
	 */
	virtual void InitAllDescStore();

	/**
	 * @brief 初始化所有扩展描述存储对象
	 *
	 * 初始化所有已注册的扩展描述存储对象
	 */
	virtual void InitAllDescStoreEx();

	/**
	 * @brief 初始化单个描述存储对象
	 *
	 * 初始化指定的描述存储对象
	 *
	 * @param descClass 描述类名
	 * @param pDescStore 描述存储对象指针
	 * @return 初始化结果状态码
	 */
	virtual int InitDescStore(const std::string& descClass, NFIDescStore* pDescStore);

	/**
	 * @brief 初始化单个扩展描述存储对象
	 *
	 * 初始化指定的扩展描述存储对象
	 *
	 * @param descClass 描述类名
	 * @param pDescStore 扩展描述存储对象指针
	 * @return 初始化结果状态码
	 */
	virtual int InitDescStoreEx(const std::string& descClass, NFIDescStoreEx* pDescStore);

	/**
	 * @brief 检查所有描述存储是否已加载
	 *
	 * @return 如果所有描述存储都已加载返回true，否则返回false
	 */
	virtual bool IsAllDescStoreLoaded() override;

	/**
	 * @brief 检查所有数据库描述存储是否已加载
	 *
	 * @return 如果所有数据库描述存储都已加载返回true，否则返回false
	 */
	virtual bool IsAllDescStoreDBLoaded() override;

	/**
	 * @brief 加载所有文件描述存储
	 *
	 * 从文件系统加载所有描述存储数据
	 *
	 * @return 加载结果状态码
	 */
	virtual int LoadAllFileDescStore();

	/**
	 * @brief 加载数据库
	 *
	 * 从数据库加载描述存储数据
	 *
	 * @return 加载结果状态码
	 */
	virtual int LoadDB();

	/**
	 * @brief 重新加载
	 *
	 * 重新加载所有描述存储数据
	 *
	 * @return 重载结果状态码
	 */
	virtual int Reload();


	/**
	 * @brief 加载单个文件描述存储
	 *
	 * 加载指定的文件描述存储对象
	 *
	 * @param pDescStore 描述存储对象指针
	 * @return 加载结果状态码
	 */
	virtual int LoadFileDescStore(NFIDescStore* pDescStore);

	/**
	 * @brief 加载单个数据库描述存储
	 *
	 * 加载指定的数据库描述存储对象
	 *
	 * @param pDescStore 描述存储对象指针
	 * @return 加载结果状态码
	 */
	virtual int LoadDBDescStore(NFIDescStore* pDescStore);

	/**
	 * @brief 重新加载单个文件描述存储
	 *
	 * 重新加载指定的文件描述存储对象
	 *
	 * @param pDescStore 描述存储对象指针
	 * @return 重载结果状态码
	 */
	virtual int ReLoadFileDescStore(NFIDescStore* pDescStore);

	/**
	 * @brief 重新加载所有文件描述存储
	 *
	 * 重新加载所有文件描述存储对象
	 *
	 * @return 重载结果状态码
	 */
	virtual int ReLoadFileDescStore();

	/**
	 * @brief 重新加载所有数据库描述存储
	 *
	 * 重新加载所有数据库描述存储对象
	 *
	 * @return 重载结果状态码
	 */
	virtual int ReLoadDBDescStore();

	/**
	 * @brief 检查是否有数据库描述存储
	 *
	 * @return 如果有数据库描述存储返回true，否则返回false
	 */
	bool HasDBDescStore();

	/**
	 * @brief 检查所有数据加载完成后的处理
	 *
	 * 当所有数据加载完成后进行必要的处理
	 *
	 * @return 处理结果状态码
	 */
	virtual int CheckWhenAllDataLoaded();

	/**
	 * @brief 加载所有扩展描述存储
	 *
	 * 加载所有扩展描述存储对象
	 *
	 * @return 加载结果状态码
	 */
	virtual int LoadAllDescStoreEx();

	/**
	 * @brief 检查所有扩展数据加载完成后的处理
	 *
	 * 当所有扩展数据加载完成后进行必要的处理
	 *
	 * @return 处理结果状态码
	 */
	virtual int CheckExWhenAllDataLoaded();

	/**
	 * @brief 获取文件MD5值
	 *
	 * 计算指定文件的MD5值
	 *
	 * @param strFileName 文件名
	 * @param fileMd5 输出MD5值
	 * @return 操作结果状态码
	 */
	int GetFileContainMD5(const std::string& strFileName, std::string& fileMd5);

	/**
	 * @brief 从真实数据库创建资源数据库
	 *
	 * 基于真实数据库创建资源数据库对象
	 *
	 * @return 资源数据库对象指针
	 */
	NFResDb* CreateResDBFromRealDB();

	/**
	 * @brief 从文件创建资源数据库
	 *
	 * 基于指定目录的文件创建资源数据库对象
	 *
	 * @param dir 文件目录
	 * @return 资源数据库对象指针
	 */
	NFResDb* CreateResDBFromFiles(const std::string& dir);

	/**
	 * @brief 共享内存初始化后运行
	 *
	 * 在共享内存初始化完成后执行必要的操作
	 */
	virtual void runAfterShmInit();

public:
	/**
	 * @brief 通过RPC获取描述存储数据
	 *
	 * 通过RPC调用从指定服务器获取描述存储数据
	 *
	 * @param eType 服务器类型
	 * @param dbName 数据库名称
	 * @param table_name 表名
	 * @param pMessage 输出消息数据
	 * @return 操作结果状态码
	 */
	virtual int GetDescStoreByRpc(NF_SERVER_TYPE eType, const std::string& dbName, const std::string& table_name, google::protobuf::Message* pMessage) override;

private:
	std::unordered_map<std::string, NFIDescStore*> mDescStoreMap; ///< 描述存储对象映射表
	std::unordered_map<std::string, NFIDescStore*> mDescStoreFileMap; ///< 文件描述存储对象映射表
	std::unordered_map<std::string, int> mDescStoreRegister; ///< 描述存储注册表
	std::vector<std::string> mDescStoreRegisterList; ///< 记录注册顺序，根据顺序来加载
	std::unordered_map<std::string, std::string> mDescStoreDBNameMap; ///< 描述存储数据库名称映射表
	NFResDb* m_pResFileDB; ///< 文件资源数据库指针
	NFResDb* m_pResSqlDB; ///< SQL资源数据库指针
	bool m_bFinishLoaded; ///< 是否已完成加载
	bool m_bFinishReloaded; ///< 是否已完成重载
	bool m_bLoadingDB; ///< 是否正在加载数据库
private:
	std::unordered_map<std::string, NFIDescStoreEx*> mDescStoreExMap; ///< 扩展描述存储对象映射表
	std::unordered_map<std::string, int> mDescStoreExRegister; ///< 扩展描述存储注册表
	std::vector<std::string> mDescStoreExRegisterList; ///< 记录扩展注册顺序，根据顺序来加载
};
