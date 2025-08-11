// -------------------------------------------------------------------------
//    @FileName         :    NFStoreProtoCommon.h
//    @Author           :    gaoyi
//    @Date             :    23-3-28
//    @Email			:    445267987@qq.com
//    @Module           :    NFStoreProtoCommon
//    @Desc             :    存储协议通用工具类，提供数据库操作的protobuf消息构建功能
//
// -------------------------------------------------------------------------

#pragma once


#include "NFComm/NFCore/NFPlatform.h"
#include "FrameSqlData.pb.h"

/**
 * @brief 存储协议通用工具类
 * 
 * 提供数据库操作的protobuf消息构建功能，支持增删改查等操作
 * 封装了与StoreServer通信的协议消息构建逻辑
 */
class NFStoreProtoCommon
{
public:
    /**
     * @brief 构建按条件查询的protobuf消息
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param fields 查询字段列表
     * @param vk_list 查询条件列表
     * @param additional_conds 附加查询条件
     * @param maxRecords 最大记录数
     * @param cls_name 类名
     * @param package_name 包名
     * @return 序列化后的protobuf消息字符串
     */
    static std::string storesvr_selectbycond(const std::string &dbname, const std::string &tbname,
                                             uint64_t mod_key, const std::vector<std::string> &fields,
                                             const std::vector<NFrame::storesvr_vk> &vk_list,
                                             const std::string &additional_conds = "", int maxRecords = 100, const std::string &cls_name = "",
                                             const std::string &package_name = "");

    /**
     * @brief 构建按条件查询的protobuf消息（填充到指定对象）
     * @param select 输出的查询消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param fields 查询字段列表
     * @param vk_list 查询条件列表
     * @param additional_conds 附加查询条件
     * @param maxRecords 最大记录数
     * @param cls_name 类名
     * @param package_name 包名
     */
    static void storesvr_selectbycond(NFrame::storesvr_sel &select, const std::string &dbname, const std::string &tbname,
                                      uint64_t mod_key, const std::vector<std::string> &fields,
                                      const std::vector<NFrame::storesvr_vk> &vk_list,
                                      const std::string &additional_conds = "", int maxRecords = 100, const std::string &cls_name = "",
                                      const std::string &package_name = "");

    /**
     * @brief 构建按主键查询的protobuf消息
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param fields 查询字段列表
     * @param privateKeys 主键列表
     * @param maxRecords 最大记录数
     * @param cls_name 类名
     * @param package_name 包名
     * @return 序列化后的protobuf消息字符串
     */
    static std::string storesvr_selectbycond(const std::string& dbname, const std::string& tbname,
                                             uint64_t mod_key, const std::vector<std::string>& fields, const std::vector<std::string>& privateKeys,
                                             int maxRecords = 100, const std::string& cls_name = "",
                                             const std::string& package_name = "");

    /**
     * @brief 构建按主键查询的protobuf消息（填充到指定对象）
     * @param select 输出的查询消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param fields 查询字段列表
     * @param privateKeys 主键列表
     * @param maxRecords 最大记录数
     * @param cls_name 类名
     * @param package_name 包名
     */
    static void storesvr_selectbycond(NFrame::storesvr_sel& select, const std::string& dbname, const std::string& tbname,
                                      uint64_t mod_key, const std::vector<std::string>& fields, const std::vector<std::string>& privateKeys,
                                      int maxRecords = 100, const std::string& cls_name = "",
                                      const std::string& package_name = "");

    /**
     * @brief 构建对象查询的protobuf消息（使用protobuf对象）
     * @param select 输出的查询消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 查询的protobuf对象
     * @param cls_name 类名
     * @param package_name 包名
     * @param vecFields 查询字段列表
     */
    static void storesvr_selectobj(NFrame::storesvr_selobj &select, const std::string &dbname, const std::string &tbname,
                                   uint64_t mod_key, const ::google::protobuf::Message &msg_obj, const std::string &cls_name = "",
                                   const std::string &package_name = "", const std::vector<std::string> &vecFields = std::vector<std::string>());

	/**
     * @brief 构建对象查询的protobuf消息（使用序列化字符串）
     * @param select 输出的查询消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msgObjStr 查询对象的序列化字符串
     * @param cls_name 类名
     * @param package_name 包名
     * @param vecFields 查询字段列表
     */
	static void storesvr_selectobj(NFrame::storesvr_selobj &select, const std::string &dbname, const std::string &tbname,
							   uint64_t mod_key, const std::string& msgObjStr, const std::string &cls_name = "",
							   const std::string &package_name = "", const std::vector<std::string> &vecFields = std::vector<std::string>());

    /**
     * @brief 构建对象查询的protobuf消息（返回序列化字符串）
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 查询的protobuf对象
     * @param cls_name 类名
     * @param package_name 包名
     * @param vecFields 查询字段列表
     * @return 序列化后的protobuf消息字符串
     */
    static std::string storesvr_selectobj(const std::string &dbname, const std::string &tbname,
                                          uint64_t mod_key, const ::google::protobuf::Message &msg_obj, const std::string &cls_name = "",
                                          const std::string &package_name = "",
                                          const std::vector<std::string> &vecFields = std::vector<std::string>());

    /**
     * @brief 构建对象插入的protobuf消息（返回序列化字符串）
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 要插入的protobuf对象
     * @param cls_name 类名
     * @param package_name 包名
     * @return 序列化后的protobuf消息字符串
     */
    static std::string storesvr_insertobj(const std::string &dbname, const std::string &tbname, uint64_t mod_key,
                                          const ::google::protobuf::Message &msg_obj, const std::string &cls_name = "",
                                          const std::string &package_name = "");

    /**
     * @brief 构建对象插入的protobuf消息（使用protobuf对象）
     * @param select 输出的插入消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 要插入的protobuf对象
     * @param cls_name 类名
     * @param package_name 包名
     */
    static void storesvr_insertobj(NFrame::storesvr_insertobj &select, const std::string &dbname, const std::string &tbname, uint64_t mod_key,
                       const ::google::protobuf::Message &msg_obj, const std::string &cls_name = "",
                       const std::string &package_name = "");

	/**
     * @brief 构建对象插入的protobuf消息（使用序列化字符串）
     * @param select 输出的插入消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msgObjStr 要插入对象的序列化字符串
     * @param cls_name 类名
     * @param package_name 包名
     */
	static void storesvr_insertobj(NFrame::storesvr_insertobj &select, const std::string &dbname, const std::string &tbname, uint64_t mod_key,
				   const std::string& msgObjStr, const std::string &cls_name = "",
				   const std::string &package_name = "");

    /**
     * @brief 构建按条件删除的protobuf消息（返回序列化字符串）
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param vk_list 删除条件列表
     * @param additional_conds 附加删除条件
     * @param cls_name 类名
     * @param package_name 包名
     * @return 序列化后的protobuf消息字符串
     */
    static std::string storesvr_deletebycond(const std::string &dbname, const std::string &tbname,
                                             uint64_t mod_key, const std::vector<NFrame::storesvr_vk> &vk_list,
                                             const std::string &additional_conds = "", const std::string &cls_name = "",
                                             const std::string &package_name = "");

    /**
     * @brief 构建按条件删除的protobuf消息（填充到指定对象）
     * @param select 输出的删除消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param vk_list 删除条件列表
     * @param additional_conds 附加删除条件
     * @param cls_name 类名
     * @param package_name 包名
     */
    static void storesvr_deletebycond(NFrame::storesvr_del &select, const std::string &dbname, const std::string &tbname,
                                      uint64_t mod_key, const std::vector<NFrame::storesvr_vk> &vk_list,
                                      const std::string &additional_conds = "", const std::string &cls_name = "",
                                      const std::string &package_name = "");

    /**
     * @brief 构建按对象删除的protobuf消息（返回序列化字符串）
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 要删除的protobuf对象
     * @param cls_name 类名
     * @param package_name 包名
     * @return 序列化后的protobuf消息字符串
     */
    static std::string storesvr_deleteobj(const std::string &dbname, const std::string &tbname,
                                          uint64_t mod_key, const ::google::protobuf::Message &msg_obj, const std::string &cls_name = "",
                                          const std::string &package_name = "");

    /**
     * @brief 构建按对象删除的protobuf消息（填充到指定对象）
     * @param select 输出的删除消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 要删除的protobuf对象
     * @param cls_name 类名
     * @param package_name 包名
     */
    static void storesvr_deleteobj(NFrame::storesvr_delobj &select, const std::string &dbname, const std::string &tbname,
                                   uint64_t mod_key, const ::google::protobuf::Message &msg_obj, const std::string &cls_name = "",
                                   const std::string &package_name = "");

    /**
     * @brief 构建按条件修改的protobuf消息（返回序列化字符串）
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 要修改的protobuf对象
     * @param vk_list 修改条件列表
     * @param additional_conds 附加修改条件
     * @param cls_name 类名
     * @param package_name 包名
     * @return 序列化后的protobuf消息字符串
     */
    static std::string storesvr_modifybycond(const std::string &dbname, const std::string &tbname,
                                             uint64_t mod_key, const ::google::protobuf::Message &msg_obj,
                                             const std::vector<NFrame::storesvr_vk> &vk_list,
                                             const std::string &additional_conds = "", const std::string &cls_name = "",
                                             const std::string &package_name = "");

    /**
     * @brief 构建按条件修改的protobuf消息（填充到指定对象）
     * @param select 输出的修改消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 要修改的protobuf对象
     * @param vk_list 修改条件列表
     * @param additional_conds 附加修改条件
     * @param cls_name 类名
     * @param package_name 包名
     */
    static void storesvr_modifybycond(NFrame::storesvr_mod &select, const std::string &dbname, const std::string &tbname,
                                      uint64_t mod_key, const ::google::protobuf::Message &msg_obj,
                                      const std::vector<NFrame::storesvr_vk> &vk_list,
                                      const std::string &additional_conds = "", const std::string &cls_name = "",
                                      const std::string &package_name = "");

    /**
     * @brief 构建按对象修改的protobuf消息（返回序列化字符串）
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 要修改的protobuf对象
     * @param cls_name 类名
     * @param package_name 包名
     * @return 序列化后的protobuf消息字符串
     */
    static std::string storesvr_modifyobj(const std::string &dbname, const std::string &tbname,
                                          uint64_t mod_key, const ::google::protobuf::Message &msg_obj, const std::string &cls_name = "",
                                          const std::string &package_name = "");

    /**
     * @brief 构建按对象修改的protobuf消息（填充到指定对象）
     * @param select 输出的修改消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 要修改的protobuf对象
     * @param cls_name 类名
     * @param package_name 包名
     */
    static void storesvr_modifyobj(NFrame::storesvr_modobj &select, const std::string &dbname, const std::string &tbname,
                                   uint64_t mod_key, const ::google::protobuf::Message &msg_obj, const std::string &cls_name = "",
                                   const std::string &package_name = "");

    /**
     * @brief 构建按条件更新的protobuf消息（返回序列化字符串）
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 要更新的protobuf对象
     * @param vk_list 更新条件列表
     * @param additional_conds 附加更新条件
     * @param cls_name 类名
     * @param package_name 包名
     * @return 序列化后的protobuf消息字符串
     */
    static std::string storesvr_updatebycond(const std::string &dbname, const std::string &tbname,
                                             uint64_t mod_key, const ::google::protobuf::Message &msg_obj,
                                             const std::vector<NFrame::storesvr_vk> &vk_list,
                                             const std::string &additional_conds = "", const std::string &cls_name = "",
                                             const std::string &package_name = "");

    /**
     * @brief 构建按条件更新的protobuf消息（填充到指定对象）
     * @param select 输出的更新消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 要更新的protobuf对象
     * @param vk_list 更新条件列表
     * @param additional_conds 附加更新条件
     * @param cls_name 类名
     * @param package_name 包名
     */
    static void storesvr_updatebycond(NFrame::storesvr_update &select, const std::string &dbname, const std::string &tbname,
                                      uint64_t mod_key, const ::google::protobuf::Message &msg_obj,
                                      const std::vector<NFrame::storesvr_vk> &vk_list,
                                      const std::string &additional_conds = "", const std::string &cls_name = "",
                                      const std::string &package_name = "");

    /**
     * @brief 构建按对象更新的protobuf消息（填充到指定对象）
     * @param select 输出的更新消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 要更新的protobuf对象
     * @param cls_name 类名
     * @param package_name 包名
     */
    static void storesvr_updateobj(NFrame::storesvr_updateobj &select, const std::string &dbname, const std::string &tbname,
                                   uint64_t mod_key, const ::google::protobuf::Message &msg_obj, const std::string &cls_name = "",
                                   const std::string &package_name = "");

    /**
     * @brief 构建按对象更新的protobuf消息（返回序列化字符串）
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg_obj 要更新的protobuf对象
     * @param cls_name 类名
     * @param package_name 包名
     * @return 序列化后的protobuf消息字符串
     */
    static std::string storesvr_updateobj(const std::string &dbname, const std::string &tbname,
                                          uint64_t mod_key, const ::google::protobuf::Message &msg_obj, const std::string &cls_name = "",
                                          const std::string &package_name = "");

    /**
     * @brief 构建执行SQL的protobuf消息（返回序列化字符串）
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg SQL语句
     * @param cls_name 类名
     * @param package_name 包名
     * @return 序列化后的protobuf消息字符串
     */
    static std::string storesvr_execute(const std::string &dbname, const std::string &tbname,
                                        uint64_t mod_key, const std::string &msg, const std::string &cls_name = "",
                                        const std::string &package_name = "");

    /**
     * @brief 构建执行SQL的protobuf消息（填充到指定对象）
     * @param select 输出的执行消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg SQL语句
     * @param cls_name 类名
     * @param package_name 包名
     */
    static void storesvr_execute(NFrame::storesvr_execute &select, const std::string &dbname, const std::string &tbname,
                                 uint64_t mod_key, const std::string &msg, const std::string &cls_name = "",
                                 const std::string &package_name = "");

    /**
     * @brief 构建执行多条SQL的protobuf消息（返回序列化字符串）
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg SQL语句
     * @param max_records 最大记录数
     * @param cls_name 类名
     * @param package_name 包名
     * @return 序列化后的protobuf消息字符串
     */
    static std::string storesvr_execute_more(const std::string &dbname, const std::string &tbname,
                                             uint64_t mod_key, const std::string &msg, int max_records, const std::string &cls_name,
                                             const std::string &package_name = "");

    /**
     * @brief 构建执行多条SQL的protobuf消息（填充到指定对象）
     * @param select 输出的执行消息对象
     * @param dbname 数据库名称
     * @param tbname 表名
     * @param mod_key 分片键
     * @param msg SQL语句
     * @param max_records 最大记录数
     * @param cls_name 类名
     * @param package_name 包名
     */
    static void storesvr_execute_more(NFrame::storesvr_execute_more &select, const std::string &dbname, const std::string &tbname,
                                             uint64_t mod_key, const std::string &msg, int max_records, const std::string &cls_name,
                                             const std::string &package_name = "");

public:
    /**
     * @brief 获取protobuf字段类型
     * @param fieldDesc protobuf字段描述符
     * @return 字段类型（数值类型或字符串类型）
     */
    static int get_proto_field_type(const google::protobuf::FieldDescriptor &fieldDesc);

    /**
     * @brief 从protobuf对象中提取查询条件列表
     * @param message protobuf消息对象
     * @param vk_list 输出的查询条件列表
     * @return 操作结果（0表示成功）
     */
    static int get_vk_list_from_proto(const google::protobuf::Message &message, std::vector<NFrame::storesvr_vk> &vk_list);
};