// -------------------------------------------------------------------------
//    @FileName         :    NFIMysqlModule.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Module           :    NFIMysqlModule
//    @Description      :    MySQL数据库模块接口定义，提供MySQL数据库的连接管理和操作功能
//                           支持SQL查询、protobuf数据映射、事务处理等数据库操作
//
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIModule.h"
#include "google/protobuf/message.h"
#include "NFComm/NFKernelMessage/FrameSqlData.pb.h"
#include "NFIDynamicModule.h"
#include "NFProtobufCommon.h"

/// @brief 信息模式数据库名称常量，用于查询数据库元数据
#define INFORMATION_SCHEMA "information_schema"

/**
 * @brief MySQL数据库模块接口类
 * 
 * NFIMysqlModule 提供了完整的MySQL数据库操作功能：
 * 
 * 1. 连接管理：
 *    - 多数据库服务器连接支持
 *    - 连接池管理和自动重连
 *    - 连接状态监控和维护
 * 
 * 2. SQL查询操作：
 *    - 单条和批量查询执行
 *    - 参数化查询支持
 *    - 查询结果集处理
 * 
 * 3. protobuf集成：
 *    - 配置表数据自动映射
 *    - protobuf消息与数据库表的双向转换
 *    - 类型安全的数据操作
 * 
 * 4. 表结构管理：
 *    - 动态表创建和修改
 *    - 表结构信息查询
 *    - 字段信息获取和验证
 * 
 * 5. 高级数据操作：
 *    - 条件查询和对象查询
 *    - 批量插入、更新、删除
 *    - 事务处理支持
 * 
 * 数据库特性：
 * - 支持MySQL 5.7+版本
 * - 连接池和连接复用
 * - 自动重连和故障恢复
 * - SQL注入防护
 * - 性能监控和统计
 * 
 * 使用场景：
 * - 游戏数据持久化
 * - 配置表数据加载
 * - 用户数据存储
 * - 日志和统计数据记录
 */
class NFIMysqlModule
	: public NFIDynamicModule
{
public:
	/**
	 * @brief 构造函数
	 * @param p 插件管理器指针
	 */
	NFIMysqlModule(NFIPluginManager* p) :NFIDynamicModule(p)
	{

	}

	/**
	 * @brief 虚析构函数
	 * 
	 * 确保派生类能够正确析构，关闭所有数据库连接。
	 */
	virtual ~NFIMysqlModule()
	{

	}
public:
	/**
	 * @brief 添加MySQL数据库连接
	 * @param serverID 服务器唯一标识ID
	 * @param strIP MySQL服务器IP地址
	 * @param nPort MySQL服务器端口号
	 * @param strDBName 数据库名称
	 * @param strDBUser 数据库用户名
	 * @param strDBPwd 数据库密码
	 * @param nRconnectTime 重连间隔时间（秒），默认10秒
	 * @param nRconneCount 重连次数，-1表示无限重连，默认为-1
	 * @return 添加成功返回0，失败返回非0值
	 * 
	 * 向MySQL模块添加一个新的数据库连接配置。支持连接池管理，
	 * 当连接断开时会根据配置的参数自动重连。
	 * 
	 * 使用示例：
	 * @code
	 * // 添加主数据库连接
	 * int result = AddMysqlServer("main_db", "127.0.0.1", 3306, 
	 *                           "game_db", "root", "password", 10, -1);
	 * if (result != 0) {
	 *     // 处理连接失败
	 * }
	 * @endcode
	 */
    virtual int AddMysqlServer(const std::string& serverID, const std::string &strIP, int nPort, std::string strDBName,
                               std::string strDBUser, std::string strDBPwd, int nRconnectTime = 10,
                               int nRconneCount = -1) = 0;

    /**
     * @brief 关闭MySQL数据库连接
     * @param serverID 服务器标识ID
     * @return 关闭成功返回0，失败返回非0值
     * 
     * 关闭指定服务器ID对应的MySQL数据库连接，释放相关资源。
     */
    virtual int CloseMysql(const std::string& serverID) = 0;

    /**
     * @brief 执行SQL查询并获取单行结果
     * @param serverID 服务器标识ID
     * @param qstr 要执行的SQL语句
     * @param keyvalueMap 查询结果的键值对映射，字段名->字段值
     * @param errormsg 错误信息输出参数
     * @return 执行成功返回0，失败返回非0值
     * 
     * 执行SQL查询语句并返回第一行结果。适用于查询单条记录的场景，
     * 如根据主键查询特定记录。
     * 
     * 使用示例：
     * @code
     * std::map<std::string, std::string> result;
     * std::string error;
     * int ret = ExecuteOne("main_db", "SELECT id, name FROM users WHERE id=1", result, error);
     * if (ret == 0) {
     *     std::string userName = result["name"];
     * }
     * @endcode
     */
    virtual int ExecuteOne(const std::string& serverID, const std::string &qstr, std::map<std::string, std::string> &keyvalueMap,
                           std::string &errormsg) = 0;

    /**
     * @brief 执行SQL查询并获取多行结果
     * @param serverID 服务器标识ID
     * @param qstr 要执行的SQL语句
     * @param keyvalueMap 查询结果的向量，每个元素是一行记录的键值对映射
     * @param errormsg 错误信息输出参数
     * @return 执行成功返回0，失败返回非0值
     * 
     * 执行SQL查询语句并返回所有匹配的记录。适用于查询多条记录的场景，
     * 如获取某个用户的所有物品记录。
     * 
     * 使用示例：
     * @code
     * std::vector<std::map<std::string, std::string>> results;
     * std::string error;
     * int ret = ExecuteMore("main_db", "SELECT * FROM items WHERE user_id=123", results, error);
     * if (ret == 0) {
     *     for (const auto& row : results) {
     *         std::string itemId = row.at("item_id");
     *         std::string itemCount = row.at("count");
     *     }
     * }
     * @endcode
     */
    virtual int
    ExecuteMore(const std::string& serverID, const std::string &qstr, std::vector<std::map<std::string, std::string>> &keyvalueMap,
                std::string &errormsg) = 0;

    /**
     * @brief 查询配置表数据并转换为protobuf消息
     * @param serverID 服务器标识ID
     * @param table 配置表名称
     * @param pMessage protobuf消息指针的指针，用于返回查询结果
     * @return 执行成功返回0，失败返回非0值
     * 
     * 从数据库配置表中查询数据并自动转换为protobuf消息格式。
     * 这个方法特别适用于加载游戏配置数据，如房间配置、道具配置等。
     * 
     * protobuf消息结构示例：
     * @code
     * message Sheet_GameRoomDesc {
     *     repeated GameRoomDesc GameRoomDesc_List = 1 [(nanopb).max_count=100];
     * }
     * @endcode
     * 
     * 使用示例：
     * @code
     * google::protobuf::Message* pMessage = nullptr;
     * int ret = QueryDescStore("config_db", "GameRoomDesc", &pMessage);
     * if (ret == 0 && pMessage) {
     *     // 使用配置数据
     *     auto* roomConfig = dynamic_cast<Sheet_GameRoomDesc*>(pMessage);
     * }
     * @endcode
     */
    virtual int QueryDescStore(const std::string& serverID, const std::string &table, google::protobuf::Message **pMessage) = 0;

    /**
     * @brief 执行SQL语句, 把数据库配置表里的数据取出来
     *
     * @param  table 配置表表明
     * @param  sheet_fullname protobuf中代表一个表格的message
     * @param  pMessage sheet_fullname的protobuf的数据结构，携带返回数据
     *  比如 message Sheet_GameRoomDesc
     *		{
     *			repeated GameRoomDesc GameRoomDesc_List = 1  [(nanopb).max_count=100];
     *		}
     * 代表一个Excel表格GameRoomDesc, 同时数据库有一个表GameRoomDesc
     * 都用这个数据结构来表达，以及存取数据
     *
     *
     * @return bool 执行成功或失败
     */
    virtual int QueryDescStore(const std::string& serverID, const std::string &table, google::protobuf::Message *pMessage) = 0;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int SelectByCond(const std::string& serverID, const NFrame::storesvr_sel &select,
                             NFrame::storesvr_sel_res &select_res) = 0;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  message 表结构体
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int
    SelectObj(const std::string& serverID, const std::string &tbName, google::protobuf::Message *pMessage, std::string &errMsg) = 0;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  message 表结构体
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int SelectObj(const std::string& serverID, const NFrame::storesvr_selobj &select,
                          NFrame::storesvr_selobj_res &select_res) = 0;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int DeleteByCond(const std::string& serverID, const NFrame::storesvr_del &select,
                             NFrame::storesvr_del_res &select_res) = 0;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int DeleteObj(const std::string& serverID, const NFrame::storesvr_delobj &select,
                          NFrame::storesvr_delobj_res &select_res) = 0;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  message 表结构体
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int
    InsertObj(const std::string& serverID, const std::string &tbName, const google::protobuf::Message *pMessage, std::string &errMsg) = 0;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int InsertObj(const std::string& serverID, const NFrame::storesvr_insertobj &select,
                          NFrame::storesvr_insertobj_res &select_res) = 0;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  message 表结构体
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int
    ModifyObj(const std::string& serverID, const std::string &tbName, const google::protobuf::Message *pMessage, std::string &errMsg) = 0;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int ModifyObj(const std::string& serverID, const NFrame::storesvr_modobj &select,
                          NFrame::storesvr_modobj_res &select_res) = 0;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  message 表结构体
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int
    UpdateObj(const std::string& serverID, const std::string &tbName, const google::protobuf::Message *pMessage, std::string &errMsg) = 0;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int UpdateObj(const std::string& serverID, const NFrame::storesvr_updateobj &select,
                          NFrame::storesvr_updateobj_res &select_res) = 0;

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
    virtual int
    QueryOne(const std::string& serverID, const std::string &strTableName, const std::map<std::string, std::string> &keyMap,
             const std::vector<std::string> &fieldVec, std::map<std::string, std::string> &valueVec,
             std::string &errormsg) = 0;

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
    virtual int QueryMore(const std::string& serverID, const std::string &strTableName, const std::map<std::string, std::string> &keyMap,
                          const std::vector<std::string> &fieldVec,
                          std::vector<std::map<std::string, std::string>> &valueVec, std::string &errormsg) = 0;

    /**
     * @brief 删除数据
     *
     * @param  strTableName		表名
     * @param  strKeyColName	列名，将以这个列排序
     * @param  strKey			数据
     * @return bool				成功或失败
     */
    virtual int Delete(const std::string& serverID, const std::string &strTableName, const std::string &strKeyColName,
                       const std::string &strKey, std::string &errormsg) = 0;

    /**
     * @brief 查找数据
     *
     * @param  strTableName		表名
     * @param  strKeyColName	列名，将以这个列排序
     * @param  strKey			数据
     * @param  bExit			是否存在
     * @return bool				成功或失败
     */
    virtual int
    Exists(const std::string& serverID, const std::string &strTableName, const std::string &strKeyColName, const std::string &strKey,
           bool &bExit) = 0;

    /**
     * @brief 更新或插入数据
     *
     * @param  strTableName		表名
     * @param  strKeyColName	列名
     * @param  strKey			列值
     * @param  keyvalueMap		要取出的数据的列名
     * @return bool				成功或失败
     */
    virtual int UpdateOne(const std::string& serverID, const std::string &strTableName, std::map<std::string, std::string> &keyMap,
                          const std::map<std::string, std::string> &keyvalueMap, std::string &errormsg) = 0;

    /**
     * @brief 是否存在数据库
     * @param dbName
     * @return
     */
    virtual int ExistsDB(const std::string& serverID, const std::string& dbName, bool &bExit) = 0;

    /**
     * @brief 创建数据库
     * @param dbName
     * @return
     */
    virtual int CreateDB(const std::string& serverID, const std::string& dbName) = 0;

    /**
     * @brief 选择数据库
     * @param dbName
     * @return
     */
    virtual int SelectDB(const std::string& serverID, const std::string& dbName) = 0;

    /**
     * @brief 是否存在表格
     * @param dbName
     * @param tableName
     * @param bExit
     * @return
     */
    virtual int ExistTable(const std::string& serverID, const std::string& dbName, const std::string& tableName, bool &bExit) = 0;

    /**
     * @brief 获取表列信息
     * @param dbName
     * @param tableName
     * @param col
     * @return
     */
    virtual int GetTableColInfo(const std::string& serverID, const std::string& dbName, const std::string& tableName, std::map<std::string, DBTableColInfo>& col) = 0;

    /**
     * @brief 查询表格信息
     * @param tableName
     * @param pTableMessage
     * @param needCreateColumn
     * @return
     */
    virtual int QueryTableInfo(const std::string& serverID, const std::string& dbName, const std::string& tableName, bool &bExit, std::map<std::string, DBTableColInfo> &primaryKey, std::multimap<uint32_t, std::string>& needCreateColumn) = 0;

    /**
     * @brief 创建table
     * @param serverID
     * @param dbName
     * @param tableName
     * @param needCreateColumn
     * @return
     */
    virtual int CreateTable(const std::string& serverID, const std::string& tableName, const std::map<std::string, DBTableColInfo> &primaryKey, const std::multimap<uint32_t, std::string>& needCreateColumn) = 0;

    /**
     * @brief 比较老的表列，看是否需要增加新的列
     * @param tableName
     * @param needCreateColumn
     * @return
     */
    virtual int AddTableRow(const std::string& serverID, const std::string& tableName, const std::multimap<uint32_t, std::string>& needCreateColumn) = 0;
};
