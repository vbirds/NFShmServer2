// -------------------------------------------------------------------------
//    @FileName         :    NFCMysqlModule.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFCMysqlModule
//    @Desc             :    MySQL数据库模块头文件，提供MySQL数据库操作接口。
//                          该文件定义了NFShmXFrame框架的MySQL数据库模块，提供MySQL数据库的各种操作功能，
//                          包括多个MySQL服务器连接管理、基本的CRUD操作、Protobuf与数据库映射、
//                          数据库表结构管理、事务和复杂查询支持。
//                          主要功能包括数据库连接管理、数据查询和修改、表结构管理、
//                          事务处理、错误处理和日志记录
//    @Description      :    MySQL数据库模块头文件，提供MySQL数据库操作接口
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIMysqlModule.h"
#include "NFCMysqlDriverManager.h"
#include "NFComm/NFPluginModule/NFTimerObj.h"

/**
 * @class NFCMysqlModule
 * @brief MySQL数据库模块实现类
 * 
 * 继承自NFIMysqlModule接口，提供MySQL数据库的各种操作功能。
 * 支持多个MySQL服务器的连接管理，提供同步的数据库操作。
 * 
 * 主要功能：
 * - 管理多个MySQL服务器连接
 * - 提供基本的CRUD操作（增删改查）
 * - 支持Protobuf与数据库的映射
 * - 提供数据库表结构管理
 * - 支持事务和复杂查询
 */
class NFCMysqlModule
        : public NFIMysqlModule {
public:
    /**
     * @brief 构造函数
     * @param p 插件管理器指针，用于管理模块的生命周期和依赖关系
     */
    NFCMysqlModule(NFIPluginManager *p);

    /**
     * @brief 析构函数
     * 释放模块占用的资源，关闭所有数据库连接
     */
    virtual ~NFCMysqlModule();

    /**
     * @brief 初始化模块
     * @return 返回初始化结果，0表示成功，非0表示失败
     */
    virtual int Init() override;

    /**
     * @brief 关闭模块
     * @return 返回关闭结果，0表示成功，非0表示失败
     */
    virtual int Shut() override;

    /**
     * @brief 模块主循环处理
     * @return 返回处理结果，0表示成功，非0表示失败
     * 
     * 定期检查连接状态，处理连接维护等操作
     */
    virtual int Tick() override;

    /**
     * @brief 定时器回调处理
     * @param nTimerID 定时器ID
     * @return 返回处理结果，0表示成功，非0表示失败
     */
    virtual int OnTimer(uint32_t nTimerID) override;

    //////////////////////////////////////////////////////////////////////////
    /**
     * @brief 查询数据
     *
     * @param serverID 服务器ID标识
     * @param qstr 执行sql语句
     * @param keyvalueMap 数据结果
     * @param errormsg 错误信息输出
     * @return int 0表示成功，非0表示失败
     */
    virtual int
    ExecuteMore(const std::string& serverID, const std::string &qstr, std::vector<std::map<std::string, std::string>> &keyvalueMap,
                std::string &errormsg) override;

    /**
     * @brief 执行sql语句, 把数据库配置表里的数据取出来
     *
     * @param serverID 服务器ID标识
     * @param table 配置表表名
     * @param pMessage sheet_fullname的protobuf的数据结构，携带返回数据
     *  比如 message Sheet_GameRoomDesc
     *		{
     *			repeated GameRoomDesc GameRoomDesc_List = 1  [(nanopb).max_count=100];
     *		}
     * 代表一个Excel表格GameRoomDesc, 同时数据库有一个表GameRoomDesc
     * 都用这个数据结构来表达，以及存取数据
     *
     * @return int 0表示成功，非0表示失败
     */
    virtual int QueryDescStore(const std::string& serverID, const std::string &table, google::protobuf::Message **pMessage) override;

    /**
     * @brief 执行sql语句, 把数据库配置表里的数据取出来
     *
     * @param serverID 服务器ID标识
     * @param table 配置表表名
     * @param pMessage sheet_fullname的protobuf的数据结构，携带返回数据
     *  比如 message Sheet_GameRoomDesc
     *		{
     *			repeated GameRoomDesc GameRoomDesc_List = 1  [(nanopb).max_count=100];
     *		}
     * 代表一个Excel表格GameRoomDesc, 同时数据库有一个表GameRoomDesc
     * 都用这个数据结构来表达，以及存取数据
     *
     * @return int 0表示成功，非0表示失败
     */
    virtual int QueryDescStore(const std::string& serverID, const std::string &table, google::protobuf::Message *pMessage) override;

    /**
     * @brief 通过select结构体，从数据库获取数据，并把结果放到select_res
     *
     * @param serverID 服务器ID标识
     * @param select 查询语句结构
     * @param select_res 查询结果
     * @return int 0表示成功，非0表示失败
     */
    virtual int SelectByCond(const std::string& serverID, const NFrame::storesvr_sel &select,
                             NFrame::storesvr_sel_res &select_res) override;

    /**
     * @brief 通过select结构体，从数据库获取数据，并把结果放到select_res
     *
     * @param serverID 服务器ID标识
     * @param tbName 表名
     * @param pMessage 表结构体对应的protobuf消息
     * @param errMsg 错误信息输出
     * @return int 0表示成功，非0表示失败
     */
    virtual int SelectObj(const std::string& serverID, const std::string& tbName, google::protobuf::Message *pMessage, std::string& errMsg);

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int SelectObj(const std::string& serverID, const NFrame::storesvr_selobj &select,
                          NFrame::storesvr_selobj_res &select_res) override;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int DeleteByCond(const std::string& serverID, const NFrame::storesvr_del &select,
                             NFrame::storesvr_del_res &select_res) override;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int DeleteObj(const std::string& serverID, const NFrame::storesvr_delobj &select,
                          NFrame::storesvr_delobj_res &select_res) override;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  message 表结构体
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int InsertObj(const std::string& serverID, const std::string& tbName, const google::protobuf::Message *pMessage, std::string& errMsg);

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int InsertObj(const std::string& serverID, const NFrame::storesvr_insertobj &select,
                          NFrame::storesvr_insertobj_res &select_res) override;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  message 表结构体
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int ModifyObj(const std::string& serverID, const std::string& tbName, const google::protobuf::Message *pMessage, std::string& errMsg);

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int ModifyObj(const std::string& serverID, const NFrame::storesvr_modobj &select,
                          NFrame::storesvr_modobj_res &select_res) override;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  message 表结构体
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int UpdateObj(const std::string& serverID, const std::string& tbName, const google::protobuf::Message *pMessage, std::string& errMsg);

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int UpdateObj(const std::string& serverID, const NFrame::storesvr_updateobj &select,
                          NFrame::storesvr_updateobj_res &select_res) override;

    /**
     * @brief 查询数据
     *
     * @param  qstr			执行sql语句
     * @param keyvalueMap   数据结果
     * @return bool			成功或失败
     */
    int ExecuteOne(const std::string& serverID, const std::string &qstr, std::map<std::string, std::string> &keyvalueMap,
                   std::string &errormsg) override;

    /**
     * @brief 更新或插入数据
     *
     * @param  strTableName		表名
     * @param  strKeyColName	列名
     * @param  strKey			列值
     * @param  keyvalueMap		要取出的数据的列名
     * @return bool				成功或失败
     */
    int UpdateOne(const std::string& serverID, const std::string &strTableName, std::map<std::string, std::string> &keyMap,
                  const std::map<std::string, std::string> &keyvalueMap, std::string &errormsg) override;

    /**
     * @brief 查询数据
     *
     * @param  strTableName		表名
     * @param  strKeyColName	列名
     * @param  strKey			列值
     * @param  fieldVec			要取出的数据的列名
     * @param  valueVec			数据
     * @return bool				成功或失败
     */
    int QueryOne(const std::string& serverID, const std::string &strTableName, const std::map<std::string, std::string> &keyMap,
                 const std::vector<std::string> &fieldVec, std::map<std::string, std::string> &valueVec,
                 std::string &errormsg) override;

    /**
     * @brief 查询数据
     *
     * @param  strTableName		表名
     * @param  strKeyColName	列名
     * @param  strKey			列值
     * @param  fieldVec			要取出的数据的列名
     * @param  valueVec			数据
     * @return bool				成功或失败
     */
    int QueryMore(const std::string& serverID, const std::string &strTableName, const std::map<std::string, std::string> &keyMap,
                  const std::vector<std::string> &fieldVec,
                  std::vector<std::map<std::string, std::string>> &valueVec, std::string &errormsg) override;

    /**
     * @brief 删除数据
     *
     * @param  strTableName		表名
     * @param  strKeyColName	列名，将以这个列排序
     * @param  strKey			数据
     * @return bool				成功或失败
     */
    int Delete(const std::string& serverID, const std::string &strTableName, const std::string &strKeyColName,
               const std::string &strKey, std::string &errormsg) override;

    /**
     * @brief 查找数据
     *
     * @param  strTableName		表名
     * @param  strKeyColName	列名，将以这个列排序
     * @param  strKey			数据
     * @param  bExit			是否存在
     * @return bool				成功或失败
     */
    int
    Exists(const std::string& serverID, const std::string &strTableName, const std::string &strKeyColName, const std::string &strKey,
           bool &bExit) override;

    /**
     * @brief 添加Mysql链接
     *
     * @param  nServerID			ID
     * @param  strIP				IP地址
     * @param  nPort				端口
     * @param  strDBName			数据库名字
     * @param  strDBUser			数据库用户名
     * @param  strDBPwd				数据库密码
     * @param  nRconnectTime		重连间隔
     * @param  nRconneCount			重连次数
     * @return bool					成功或失败
     */
    int AddMysqlServer(const std::string& serverID, const std::string &strIP, int nPort, std::string strDBName,
                       std::string strDBUser, std::string strDBPwd, int nRconnectTime = 10,
                       int nRconneCount = -1) override;

    /**
     * @brief 关闭MySQL连接
     * @param serverID 服务器ID标识
     * @return int 0表示成功，非0表示失败
     */
    int CloseMysql(const std::string& serverID) override;

    /**
     * @brief 检查指定数据库是否存在
     * @param serverID 服务器ID标识
     * @param dbName 数据库名称
     * @param bExit 返回是否存在的结果
     * @return int 0表示成功，非0表示失败
     */
    int ExistsDB(const std::string& serverID, const std::string& dbName, bool &bExit) override;

    /**
     * @brief 创建数据库
     * @param serverID 服务器ID标识
     * @param dbName 要创建的数据库名称
     * @return int 0表示成功，非0表示失败
     */
    int CreateDB(const std::string& serverID, const std::string& dbName) override;

    /**
     * @brief 选择要使用的数据库
     * @param serverID 服务器ID标识
     * @param dbName 要选择的数据库名称
     * @return int 0表示成功，非0表示失败
     */
    int SelectDB(const std::string& serverID, const std::string& dbName) override;

    /**
     * @brief 检查指定表是否存在
     * @param serverID 服务器ID标识
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @param bExit 返回是否存在的结果
     * @return int 0表示成功，非0表示失败
     */
    int ExistTable(const std::string& serverID, const std::string& dbName, const std::string& tableName, bool &bExit) override;

    /**
     * @brief 获取指定表的列信息
     * @param serverID 服务器ID标识
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @param col 返回的列信息映射表
     * @return int 0表示成功，非0表示失败
     */
    int GetTableColInfo(const std::string& serverID, const std::string& dbName, const std::string& tableName, std::map<std::string, DBTableColInfo>& col) override;

    /**
     * @brief 查询表格的详细信息
     * @param serverID 服务器ID标识
     * @param dbName 数据库名称
     * @param tableName 表名称
     * @param bExit 返回表是否存在
     * @param primaryKey 返回主键信息
     * @param needCreateColumn 返回需要创建的列信息
     * @return int 0表示成功，非0表示失败
     */
    int QueryTableInfo(const std::string& serverID, const std::string& dbName, const std::string& tableName, bool &bExit, std::map<std::string, DBTableColInfo> &primaryKey, std::multimap<uint32_t, std::string>& needCreateColumn) override;

    /**
     * @brief 创建新的数据表
     * @param serverID 服务器ID标识
     * @param tableName 要创建的表名称
     * @param primaryKey 主键列信息
     * @param needCreateColumn 需要创建的列信息
     * @return int 0表示成功，非0表示失败
     */
    int CreateTable(const std::string& serverID, const std::string& tableName, const std::map<std::string, DBTableColInfo> &primaryKey, const std::multimap<uint32_t, std::string>& needCreateColumn) override;

    /**
     * @brief 为现有表添加新的列
     * @param serverID 服务器ID标识
     * @param tableName 表名称
     * @param needCreateColumn 需要添加的列信息
     * @return int 0表示成功，非0表示失败
     * 
     * 比较现有表结构与所需结构，添加缺少的列
     */
    int AddTableRow(const std::string& serverID, const std::string& tableName, const std::multimap<uint32_t, std::string>& needCreateColumn) override;

private:
    /**
     * @brief MySQL驱动管理器
     * 负责管理所有MySQL数据库连接和驱动实例
     */
    NFCMysqlDriverManager *m_pMysqlDriverManager;

    /**
     * @brief 最后一次检查时间
     * 用于定期检查数据库连接状态
     */
    uint64_t mnLastCheckTime;
};
