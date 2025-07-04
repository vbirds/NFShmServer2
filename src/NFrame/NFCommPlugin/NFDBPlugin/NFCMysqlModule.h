// -------------------------------------------------------------------------
//    @FileName         :    NFCMysqlModule.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFCMysqlModule
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFPluginModule/NFIMysqlModule.h"
#include "NFCMysqlDriverManager.h"
#include "NFComm/NFPluginModule/NFTimerObj.h"

class NFCMysqlModule
        : public NFIMysqlModule {
public:
    NFCMysqlModule(NFIPluginManager *p);

    virtual ~NFCMysqlModule();


    virtual bool Init() override;

    virtual bool Shut() override;

    virtual bool Execute() override;

    virtual int OnTimer(uint32_t nTimerID) override;

    //////////////////////////////////////////////////////////////////////////
    /**
     * @brief 查询数据
     *
     * @param  qstr			执行sql语句
     * @param keyvalueMap   数据结果
     * @return bool			成功或失败
     */
    virtual int
    ExecuteMore(const std::string& serverID, const std::string &qstr, std::vector<std::map<std::string, std::string>> &keyvalueMap,
                std::string &errormsg) override;

    /**
     * @brief 执行sql语句, 把数据库配置表里的数据取出来
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
    virtual int QueryDescStore(const std::string& serverID, const std::string &table, google::protobuf::Message **pMessage) override;

    /**
     * @brief 执行sql语句, 把数据库配置表里的数据取出来
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
    virtual int QueryDescStore(const std::string& serverID, const std::string &table, google::protobuf::Message *pMessage) override;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int SelectByCond(const std::string& serverID, const NFrame::storesvr_sel &select,
                             NFrame::storesvr_sel_res &select_res) override;

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  message 表结构体
     * @param  select_res 查询结果
     * @return int =0执行成功, != 0失败
     */
    virtual int SelectObj(const std::string& serverID, const std::string& tbName, google::protobuf::Message *pMessage, std::string& errMsg);

    /**
     * @brief 通过select结构体， 从数据库获取数据，并把结果放到selelct_res
     *
     * @param  select 查询语句
     * @param  message 表结构体
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
     * @brief
     * @param serverID
     * @return
     */
    int CloseMysql(const std::string& serverID) override;

    /**
     * @brief 是否存在数据库
     * @param dbName
     * @return
     */
    int ExistsDB(const std::string& serverID, const std::string& dbName, bool &bExit) override;

    /**
     * @brief 创建数据库
     * @param dbName
     * @return
     */
    int CreateDB(const std::string& serverID, const std::string& dbName) override;

    /**
     * @brief 选择数据库
     * @param dbName
     * @return
     */
    int SelectDB(const std::string& serverID, const std::string& dbName) override;

    /**
     * @brief 是否存在表格
     * @param dbName
     * @param tableName
     * @param bExit
     * @return
     */
    int ExistTable(const std::string& serverID, const std::string& dbName, const std::string& tableName, bool &bExit) override;

    /**
     * @brief 获取表列信息
     * @param dbName
     * @param tableName
     * @param col
     * @return
     */
    int GetTableColInfo(const std::string& serverID, const std::string& dbName, const std::string& tableName, std::map<std::string, DBTableColInfo>& col) override;

    /**
     * @brief 查询表格信息
     * @param tableName
     * @param pTableMessage
     * @param needCreateColumn
     * @return
     */
    int QueryTableInfo(const std::string& serverID, const std::string& dbName, const std::string& tableName, bool &bExit, std::map<std::string, DBTableColInfo> &primaryKey, std::multimap<uint32_t, std::string>& needCreateColumn) override;

    /**
     * @brief 创建table
     * @param serverID
     * @param dbName
     * @param tableName
     * @param needCreateColumn
     * @return
     */
    int CreateTable(const std::string& serverID, const std::string& tableName, const std::map<std::string, DBTableColInfo> &primaryKey, const std::multimap<uint32_t, std::string>& needCreateColumn) override;

    /**
     * @brief 比较老的表列，看是否需要增加新的列
     * @param tableName
     * @param needCreateColumn
     * @return
     */
    int AddTableRow(const std::string& serverID, const std::string& tableName, const std::multimap<uint32_t, std::string>& needCreateColumn) override;
private:
    NFCMysqlDriverManager *m_pMysqlDriverManager;

    uint64_t mnLastCheckTime;
};
