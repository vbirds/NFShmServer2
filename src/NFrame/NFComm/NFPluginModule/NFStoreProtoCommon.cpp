// -------------------------------------------------------------------------
//    @FileName         :    NFStoreProtoCommon.cpp
//    @Author           :    gaoyi
//    @Date             :    23-3-28
//    @Email			:    445267987@qq.com
//    @Module           :    NFStoreProtoCommon
//    @Desc             :    存储协议通用工具类实现，提供数据库操作的protobuf消息构建功能
//
// -------------------------------------------------------------------------

#include "NFStoreProtoCommon.h"
#include "NFProtobufCommon.h"
#include "NFCheck.h"

/**
 * @brief 构建按条件查询的protobuf消息（返回序列化字符串）
 * 
 * 构建一个完整的查询消息，包含数据库名、表名、查询条件等信息
 * 返回序列化后的字符串，可直接用于网络传输
 * 
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键，用于数据分片
 * @param fields 查询字段列表
 * @param vk_list 查询条件列表，包含字段名、值、比较操作符等
 * @param additional_conds 附加查询条件，可包含复杂的SQL条件
 * @param maxRecords 最大返回记录数，防止查询结果过大
 * @param cls_name 类名，用于反序列化
 * @param package_name 包名，用于反序列化
 * @return 序列化后的protobuf消息字符串
 */
std::string NFStoreProtoCommon::storesvr_selectbycond(const std::string& dbname, const std::string& tbname,
                                                      uint64_t mod_key, const std::vector<std::string>& fields,
                                                      const std::vector<NFrame::storesvr_vk>& vk_list,
                                                      const std::string& additional_conds/* = ""*/, int maxRecords/* = 100*/,
                                                      const std::string& cls_name/* = ""*/, const std::string& package_name/* = ""*/)
{
    NFrame::storesvr_sel select;
    storesvr_selectbycond(select, dbname, tbname, mod_key, fields, vk_list, additional_conds, maxRecords, cls_name, package_name);
    return select.SerializePartialAsString();
}

/**
 * @brief 构建按条件查询的protobuf消息（填充到指定对象）
 * 
 * 将查询参数填充到指定的protobuf消息对象中
 * 
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
void NFStoreProtoCommon::storesvr_selectbycond(NFrame::storesvr_sel& select, const std::string& dbname, const std::string& tbname,
                                               uint64_t mod_key, const std::vector<std::string>& fields,
                                               const std::vector<NFrame::storesvr_vk>& vk_list,
                                               const std::string& additional_conds/* = ""*/, int maxRecords/* = 100*/,
                                               const std::string& cls_name/* = ""*/, const std::string& package_name/* = ""*/)
{
    // 设置基本信息
    select.mutable_baseinfo()->set_dbname(dbname);
    select.mutable_baseinfo()->set_tbname(tbname);
    if (cls_name.empty())
    {
        select.mutable_baseinfo()->set_clname(tbname);
    }
    else
    {
        select.mutable_baseinfo()->set_clname(cls_name);
    }
    select.mutable_baseinfo()->set_package_name(package_name);
    
    // 添加查询字段
    for (int i = 0; i < (int)fields.size(); i++)
    {
        select.mutable_baseinfo()->add_sel_fields(fields[i]);
    }
    select.mutable_baseinfo()->set_max_records(maxRecords);

    // 设置查询条件
    select.mutable_cond()->set_mod_key(mod_key);
    select.mutable_cond()->set_where_additional_conds(additional_conds);
    
    // 添加查询条件列表
    for (size_t i = 0; i < vk_list.size(); i++)
    {
        ::NFrame::storesvr_vk* pvk = select.mutable_cond()->add_where_conds();
        *pvk = vk_list[i];
    }
}

/**
 * @brief 构建按主键查询的protobuf消息（返回序列化字符串）
 * 
 * 使用主键列表进行查询，适用于精确匹配的场景
 * 
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
std::string NFStoreProtoCommon::storesvr_selectbycond(const std::string& dbname, const std::string& tbname, uint64_t mod_key, const std::vector<std::string>& fields, const std::vector<std::string>& privateKeys, int maxRecords, const std::string& cls_name, const std::string& package_name)
{
    NFrame::storesvr_sel select;
    storesvr_selectbycond(select, dbname, tbname, mod_key, fields, privateKeys, maxRecords, cls_name, package_name);
    return select.SerializePartialAsString();
}

/**
 * @brief 构建按主键查询的protobuf消息（填充到指定对象）
 * 
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
void NFStoreProtoCommon::storesvr_selectbycond(NFrame::storesvr_sel& select, const std::string& dbname, const std::string& tbname, uint64_t mod_key, const std::vector<std::string>& fields, const std::vector<std::string>& privateKeys, int maxRecords, const std::string& cls_name,
                                               const std::string& package_name)
{
    // 设置基本信息
    select.mutable_baseinfo()->set_dbname(dbname);
    select.mutable_baseinfo()->set_tbname(tbname);
    if (cls_name.empty())
    {
        select.mutable_baseinfo()->set_clname(tbname);
    }
    else
    {
        select.mutable_baseinfo()->set_clname(cls_name);
    }
    select.mutable_baseinfo()->set_package_name(package_name);
    
    // 添加查询字段
    for (int i = 0; i < (int)fields.size(); i++)
    {
        select.mutable_baseinfo()->add_sel_fields(fields[i]);
    }
    select.mutable_baseinfo()->set_max_records(maxRecords);

    // 设置分片键和主键列表
    select.mutable_cond()->set_mod_key(mod_key);
    for (size_t i = 0; i < privateKeys.size(); i++)
    {
        select.mutable_cond()->add_private_keys(privateKeys[i]);
    }
}

/**
 * @brief 构建对象查询的protobuf消息（使用protobuf对象）
 * 
 * 使用protobuf对象作为查询条件，适用于复杂查询场景
 * 
 * @param select 输出的查询消息对象
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg_obj 查询的protobuf对象
 * @param cls_name 类名
 * @param package_name 包名
 * @param vecFields 查询字段列表
 */
void NFStoreProtoCommon::storesvr_selectobj(NFrame::storesvr_selobj& select, const std::string& dbname, const std::string& tbname,
                                            uint64_t mod_key, const ::google::protobuf::Message& msg_obj, const std::string& cls_name/* = ""*/,
                                            const std::string& package_name/* = ""*/,
                                            const std::vector<std::string>& vecFields/* = std::vector<std::string>()*/)
{
    storesvr_selectobj(select, dbname, tbname, mod_key, msg_obj.SerializePartialAsString(), cls_name, package_name, vecFields);
}

/**
 * @brief 构建对象查询的protobuf消息（使用序列化字符串）
 * 
 * @param select 输出的查询消息对象
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msgObjStr 查询对象的序列化字符串
 * @param cls_name 类名
 * @param package_name 包名
 * @param vecFields 查询字段列表
 */
void NFStoreProtoCommon::storesvr_selectobj(NFrame::storesvr_selobj& select, const std::string& dbname, const std::string& tbname,
                                            uint64_t mod_key, const std::string& msgObjStr, const std::string& cls_name/* = ""*/,
                                            const std::string& package_name/* = ""*/,
                                            const std::vector<std::string>& vecFields/* = std::vector<std::string>()*/)
{
    // 设置基本信息
    select.mutable_baseinfo()->set_dbname(dbname);
    select.mutable_baseinfo()->set_package_name(package_name);
    if (cls_name.empty())
    {
        select.mutable_baseinfo()->set_clname(tbname);
    }
    else
    {
        select.mutable_baseinfo()->set_clname(cls_name);
    }
    if (tbname.empty())
    {
        select.mutable_baseinfo()->set_tbname(cls_name);
    }
    else
    {
        select.mutable_baseinfo()->set_tbname(tbname);
    }
    
    // 设置查询参数
    select.set_mod_key(mod_key);
    select.set_record(msgObjStr);
    
    // 添加查询字段
    if (vecFields.size() > 0)
    {
        for (int i = 0; i < (int)vecFields.size(); i++)
        {
            select.mutable_baseinfo()->add_sel_fields(vecFields[i]);
        }
    }
}

/**
 * @brief 构建对象查询的protobuf消息（返回序列化字符串）
 * 
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg_obj 查询的protobuf对象
 * @param cls_name 类名
 * @param package_name 包名
 * @param vecFields 查询字段列表
 * @return 序列化后的protobuf消息字符串
 */
std::string NFStoreProtoCommon::storesvr_selectobj(const std::string& dbname, const std::string& tbname,
                                                   uint64_t mod_key, const ::google::protobuf::Message& msg_obj, const std::string& cls_name/* = ""*/,
                                                   const std::string& package_name/* = ""*/,
                                                   const std::vector<std::string>& vecFields/* = std::vector<std::string>()*/)
{
    NFrame::storesvr_selobj select;
    storesvr_selectobj(select, dbname, tbname, mod_key, msg_obj, cls_name, package_name, vecFields);
    return select.SerializePartialAsString();
}

/**
 * @brief 构建对象插入的protobuf消息（使用protobuf对象）
 * 
 * @param select 输出的插入消息对象
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg_obj 要插入的protobuf对象
 * @param cls_name 类名
 * @param package_name 包名
 */
void NFStoreProtoCommon::storesvr_insertobj(NFrame::storesvr_insertobj& select, const std::string& dbname, const std::string& tbname,
                                            uint64_t mod_key,
                                            const ::google::protobuf::Message& msg_obj, const std::string& cls_name/* = ""*/,
                                            const std::string& package_name/* = ""*/)
{
    storesvr_insertobj(select, dbname, tbname, mod_key, msg_obj.SerializePartialAsString(), cls_name, package_name);
}

/**
 * @brief 构建对象插入的protobuf消息（使用序列化字符串）
 * 
 * @param select 输出的插入消息对象
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msgObjStr 要插入对象的序列化字符串
 * @param cls_name 类名
 * @param package_name 包名
 */
void NFStoreProtoCommon::storesvr_insertobj(NFrame::storesvr_insertobj& select, const std::string& dbname, const std::string& tbname,
                                            uint64_t mod_key,
                                            const std::string& msgObjStr, const std::string& cls_name/* = ""*/,
                                            const std::string& package_name/* = ""*/)
{
    // 设置基本信息
    select.mutable_baseinfo()->set_dbname(dbname);
    select.mutable_baseinfo()->set_tbname(tbname);
    select.mutable_baseinfo()->set_package_name(package_name);
    if (cls_name.empty())
    {
        select.mutable_baseinfo()->set_clname(tbname);
    }
    else
    {
        select.mutable_baseinfo()->set_clname(cls_name);
    }
    
    // 设置插入数据
    select.set_mod_key(mod_key);
    select.set_record(msgObjStr);
}

/**
 * @brief 构建对象插入的protobuf消息（返回序列化字符串）
 * 
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg_obj 要插入的protobuf对象
 * @param cls_name 类名
 * @param package_name 包名
 * @return 序列化后的protobuf消息字符串
 */
std::string NFStoreProtoCommon::storesvr_insertobj(const std::string& dbname, const std::string& tbname,
                                                   uint64_t mod_key, const ::google::protobuf::Message& msg_obj, const std::string& cls_name/* = ""*/,
                                                   const std::string& package_name/* = ""*/)
{
    NFrame::storesvr_insertobj select;
    storesvr_insertobj(select, dbname, tbname, mod_key, msg_obj, cls_name, package_name);
    return select.SerializePartialAsString();
}

/**
 * @brief 构建按条件删除的protobuf消息（返回序列化字符串）
 * 
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param vk_list 删除条件列表
 * @param additional_conds 附加删除条件
 * @param cls_name 类名
 * @param package_name 包名
 * @return 序列化后的protobuf消息字符串
 */
std::string NFStoreProtoCommon::storesvr_deletebycond(const std::string& dbname, const std::string& tbname,
                                                      uint64_t mod_key, const std::vector<NFrame::storesvr_vk>& vk_list,
                                                      const std::string& additional_conds /*= ""*/, const std::string& cls_name/* = ""*/,
                                                      const std::string& package_name/* = ""*/)
{
    NFrame::storesvr_del select;
    storesvr_deletebycond(select, dbname, tbname, mod_key, vk_list, additional_conds, cls_name, package_name);
    return select.SerializePartialAsString();
}

/**
 * @brief 构建按条件删除的protobuf消息（填充到指定对象）
 * 
 * @param select 输出的删除消息对象
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param vk_list 删除条件列表
 * @param additional_conds 附加删除条件
 * @param cls_name 类名
 * @param package_name 包名
 */
void NFStoreProtoCommon::storesvr_deletebycond(NFrame::storesvr_del& select, const std::string& dbname, const std::string& tbname,
                                               uint64_t mod_key, const std::vector<NFrame::storesvr_vk>& vk_list,
                                               const std::string& additional_conds /*= ""*/, const std::string& cls_name/* = ""*/,
                                               const std::string& package_name/* = ""*/)
{
    // 设置基本信息
    select.mutable_baseinfo()->set_dbname(dbname);
    select.mutable_baseinfo()->set_tbname(tbname);
    select.mutable_baseinfo()->set_package_name(package_name);
    if (cls_name.empty())
    {
        select.mutable_baseinfo()->set_clname(tbname);
    }
    else
    {
        select.mutable_baseinfo()->set_clname(cls_name);
    }
    
    // 设置删除条件
    select.mutable_cond()->set_mod_key(mod_key);
    select.mutable_cond()->set_where_additional_conds(additional_conds);
    
    // 添加删除条件列表
    for (size_t i = 0; i < vk_list.size(); i++)
    {
        ::NFrame::storesvr_vk* pvk = select.mutable_cond()->add_where_conds();
        *pvk = vk_list[i];
    }
}

/**
 * @brief 构建按对象删除的protobuf消息（返回序列化字符串）
 * 
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg_obj 要删除的protobuf对象
 * @param cls_name 类名
 * @param package_name 包名
 * @return 序列化后的protobuf消息字符串
 */
std::string NFStoreProtoCommon::storesvr_deleteobj(const std::string& dbname, const std::string& tbname,
                                                   uint64_t mod_key, const ::google::protobuf::Message& msg_obj, const std::string& cls_name/* = ""*/,
                                                   const std::string& package_name/* = ""*/)
{
    NFrame::storesvr_delobj select;
    storesvr_deleteobj(select, dbname, tbname, mod_key, msg_obj, cls_name, package_name);
    return select.SerializePartialAsString();
}

/**
 * @brief 构建按对象删除的protobuf消息（填充到指定对象）
 * 
 * @param select 输出的删除消息对象
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg_obj 要删除的protobuf对象
 * @param cls_name 类名
 * @param package_name 包名
 */
void NFStoreProtoCommon::storesvr_deleteobj(NFrame::storesvr_delobj& select, const std::string& dbname, const std::string& tbname,
                                            uint64_t mod_key, const ::google::protobuf::Message& msg_obj, const std::string& cls_name/* = ""*/,
                                            const std::string& package_name/* = ""*/)
{
    // 设置基本信息
    select.mutable_baseinfo()->set_dbname(dbname);
    select.mutable_baseinfo()->set_tbname(tbname);
    select.mutable_baseinfo()->set_package_name(package_name);
    if (cls_name.empty())
    {
        select.mutable_baseinfo()->set_clname(tbname);
    }
    else
    {
        select.mutable_baseinfo()->set_clname(cls_name);
    }
    
    // 设置删除对象
    select.set_mod_key(mod_key);
    select.set_record(msg_obj.SerializePartialAsString());
}

/**
 * @brief 构建按条件修改的protobuf消息（返回序列化字符串）
 * 
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
std::string NFStoreProtoCommon::storesvr_modifybycond(const std::string& dbname, const std::string& tbname,
                                                      uint64_t mod_key, const ::google::protobuf::Message& msg_obj,
                                                      const std::vector<NFrame::storesvr_vk>& vk_list,
                                                      const std::string& additional_conds/* = ""*/, const std::string& cls_name/* = ""*/,
                                                      const std::string& package_name/* = ""*/)
{
    NFrame::storesvr_mod select;
    storesvr_modifybycond(select, dbname, tbname, mod_key, msg_obj, vk_list, additional_conds, cls_name, package_name);
    return select.SerializePartialAsString();
}

/**
 * @brief 构建按条件修改的protobuf消息（填充到指定对象）
 * 
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
void NFStoreProtoCommon::storesvr_modifybycond(NFrame::storesvr_mod& select, const std::string& dbname, const std::string& tbname,
                                               uint64_t mod_key, const ::google::protobuf::Message& msg_obj,
                                               const std::vector<NFrame::storesvr_vk>& vk_list,
                                               const std::string& additional_conds/* = ""*/, const std::string& cls_name/* = ""*/,
                                               const std::string& package_name/* = ""*/)
{
    // 设置基本信息
    select.mutable_baseinfo()->set_dbname(dbname);
    select.mutable_baseinfo()->set_tbname(tbname);
    select.mutable_baseinfo()->set_package_name(package_name);
    if (cls_name.empty())
    {
        select.mutable_baseinfo()->set_clname(tbname);
    }
    else
    {
        select.mutable_baseinfo()->set_clname(cls_name);
    }

    // 设置修改条件
    select.mutable_cond()->set_mod_key(mod_key);
    select.mutable_cond()->set_where_additional_conds(additional_conds);
    
    // 添加修改条件列表
    for (size_t i = 0; i < vk_list.size(); i++)
    {
        ::NFrame::storesvr_vk* pvk = select.mutable_cond()->add_where_conds();
        *pvk = vk_list[i];
    }

    // 设置修改数据
    select.set_record(msg_obj.SerializePartialAsString());
}

/**
 * @brief 构建按对象修改的protobuf消息（返回序列化字符串）
 * 
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg_obj 要修改的protobuf对象
 * @param cls_name 类名
 * @param package_name 包名
 * @return 序列化后的protobuf消息字符串
 */
std::string NFStoreProtoCommon::storesvr_modifyobj(const std::string& dbname, const std::string& tbname,
                                                   uint64_t mod_key, const ::google::protobuf::Message& msg_obj, const std::string& cls_name/* = ""*/,
                                                   const std::string& package_name/* = ""*/)
{
    NFrame::storesvr_modobj select;
    storesvr_modifyobj(select, dbname, tbname, mod_key, msg_obj, cls_name, package_name);
    return select.SerializePartialAsString();
}

/**
 * @brief 构建按对象修改的protobuf消息（填充到指定对象）
 * 
 * @param select 输出的修改消息对象
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg_obj 要修改的protobuf对象
 * @param cls_name 类名
 * @param package_name 包名
 */
void NFStoreProtoCommon::storesvr_modifyobj(NFrame::storesvr_modobj& select, const std::string& dbname, const std::string& tbname,
                                            uint64_t mod_key, const ::google::protobuf::Message& msg_obj, const std::string& cls_name/* = ""*/,
                                            const std::string& package_name/* = ""*/)
{
    // 设置基本信息
    select.mutable_baseinfo()->set_dbname(dbname);
    select.mutable_baseinfo()->set_tbname(tbname);
    select.mutable_baseinfo()->set_package_name(package_name);
    if (cls_name.empty())
    {
        select.mutable_baseinfo()->set_clname(tbname);
    }
    else
    {
        select.mutable_baseinfo()->set_clname(cls_name);
    }
    
    // 设置修改数据
    select.set_mod_key(mod_key);
    select.set_record(msg_obj.SerializePartialAsString());
}

/**
 * @brief 构建按条件更新的protobuf消息（返回序列化字符串）
 * 
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
std::string NFStoreProtoCommon::storesvr_updatebycond(const std::string& dbname, const std::string& tbname,
                                                      uint64_t mod_key, const ::google::protobuf::Message& msg_obj,
                                                      const std::vector<NFrame::storesvr_vk>& vk_list,
                                                      const std::string& additional_conds/* = ""*/, const std::string& cls_name/* = ""*/,
                                                      const std::string& package_name/* = ""*/)
{
    NFrame::storesvr_update select;
    storesvr_updatebycond(select, dbname, tbname, mod_key, msg_obj, vk_list, additional_conds, cls_name, package_name);
    return select.SerializePartialAsString();
}

/**
 * @brief 构建按条件更新的protobuf消息（填充到指定对象）
 * 
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
void NFStoreProtoCommon::storesvr_updatebycond(NFrame::storesvr_update& select, const std::string& dbname, const std::string& tbname,
                                               uint64_t mod_key, const ::google::protobuf::Message& msg_obj,
                                               const std::vector<NFrame::storesvr_vk>& vk_list,
                                               const std::string& additional_conds/* = ""*/, const std::string& cls_name/* = ""*/,
                                               const std::string& package_name/* = ""*/)
{
    // 设置基本信息
    select.mutable_baseinfo()->set_dbname(dbname);
    select.mutable_baseinfo()->set_tbname(tbname);
    select.mutable_baseinfo()->set_package_name(package_name);
    if (cls_name.empty())
    {
        select.mutable_baseinfo()->set_clname(tbname);
    }
    else
    {
        select.mutable_baseinfo()->set_clname(cls_name);
    }

    // 设置更新条件
    select.mutable_cond()->set_mod_key(mod_key);
    select.mutable_cond()->set_where_additional_conds(additional_conds);
    
    // 添加更新条件列表
    for (size_t i = 0; i < vk_list.size(); i++)
    {
        ::NFrame::storesvr_vk* pvk = select.mutable_cond()->add_where_conds();
        *pvk = vk_list[i];
    }

    // 设置更新数据
    select.set_record(msg_obj.SerializePartialAsString());
}

/**
 * @brief 构建按对象更新的protobuf消息（返回序列化字符串）
 * 
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg_obj 要更新的protobuf对象
 * @param cls_name 类名
 * @param package_name 包名
 * @return 序列化后的protobuf消息字符串
 */
std::string NFStoreProtoCommon::storesvr_updateobj(const std::string& dbname, const std::string& tbname,
                                                   uint64_t mod_key, const ::google::protobuf::Message& msg_obj, const std::string& cls_name/* = ""*/,
                                                   const std::string& package_name/* = ""*/)
{
    NFrame::storesvr_updateobj select;
    storesvr_updateobj(select, dbname, tbname, mod_key, msg_obj, cls_name, package_name);
    return select.SerializePartialAsString();
}

/**
 * @brief 构建按对象更新的protobuf消息（填充到指定对象）
 * 
 * @param select 输出的更新消息对象
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg_obj 要更新的protobuf对象
 * @param cls_name 类名
 * @param package_name 包名
 */
void NFStoreProtoCommon::storesvr_updateobj(NFrame::storesvr_updateobj& select, const std::string& dbname, const std::string& tbname,
                                            uint64_t mod_key, const ::google::protobuf::Message& msg_obj, const std::string& cls_name/* = ""*/,
                                            const std::string& package_name/* = ""*/)
{
    // 设置基本信息
    select.mutable_baseinfo()->set_dbname(dbname);
    select.mutable_baseinfo()->set_tbname(tbname);
    select.mutable_baseinfo()->set_package_name(package_name);
    if (cls_name.empty())
    {
        select.mutable_baseinfo()->set_clname(tbname);
    }
    else
    {
        select.mutable_baseinfo()->set_clname(cls_name);
    }
    
    // 设置更新数据
    select.set_mod_key(mod_key);
    select.set_record(msg_obj.SerializePartialAsString());
}

/**
 * @brief 构建执行SQL的protobuf消息（返回序列化字符串）
 * 
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg SQL语句
 * @param cls_name 类名
 * @param package_name 包名
 * @return 序列化后的protobuf消息字符串
 */
std::string NFStoreProtoCommon::storesvr_execute(const std::string& dbname, const std::string& tbname,
                                                 uint64_t mod_key, const std::string& msg, const std::string& cls_name/* = ""*/,
                                                 const std::string& package_name/* = ""*/)
{
    NFrame::storesvr_execute select;
    storesvr_execute(select, dbname, tbname, mod_key, msg, cls_name, package_name);
    return select.SerializePartialAsString();
}

/**
 * @brief 构建执行SQL的protobuf消息（填充到指定对象）
 * 
 * @param select 输出的执行消息对象
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg SQL语句
 * @param cls_name 类名
 * @param package_name 包名
 */
void NFStoreProtoCommon::storesvr_execute(NFrame::storesvr_execute& select, const std::string& dbname, const std::string& tbname,
                                          uint64_t mod_key, const std::string& msg, const std::string& cls_name/* = ""*/,
                                          const std::string& package_name/* = ""*/)
{
    // 设置基本信息
    select.mutable_baseinfo()->set_dbname(dbname);
    select.mutable_baseinfo()->set_tbname(tbname);
    select.mutable_baseinfo()->set_package_name(package_name);
    if (cls_name.empty())
    {
        select.mutable_baseinfo()->set_clname(tbname);
    }
    else
    {
        select.mutable_baseinfo()->set_clname(cls_name);
    }
    
    // 设置SQL语句
    select.set_mod_key(mod_key);
    select.set_record(msg + ";");
}

/**
 * @brief 构建执行多条SQL的protobuf消息（返回序列化字符串）
 * 
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg SQL语句
 * @param max_records 最大记录数
 * @param cls_name 类名
 * @param package_name 包名
 * @return 序列化后的protobuf消息字符串
 */
std::string NFStoreProtoCommon::storesvr_execute_more(const std::string& dbname, const std::string& tbname,
                                                      uint64_t mod_key, const std::string& msg, int max_records, const std::string& cls_name, const std::string& package_name)
{
    NFrame::storesvr_execute_more select;
    storesvr_execute_more(select, dbname, tbname, mod_key, msg, max_records, cls_name, package_name);
    return select.SerializePartialAsString();
}

/**
 * @brief 构建执行多条SQL的protobuf消息（填充到指定对象）
 * 
 * @param select 输出的执行消息对象
 * @param dbname 数据库名称
 * @param tbname 表名
 * @param mod_key 分片键
 * @param msg SQL语句
 * @param max_records 最大记录数
 * @param cls_name 类名
 * @param package_name 包名
 */
void NFStoreProtoCommon::storesvr_execute_more(NFrame::storesvr_execute_more& select, const std::string& dbname, const std::string& tbname,
                                               uint64_t mod_key, const std::string& msg, int max_records, const std::string& cls_name,
                                               const std::string& package_name)
{
    // 设置基本信息
    select.mutable_baseinfo()->set_dbname(dbname);
    select.mutable_baseinfo()->set_tbname(tbname);
    select.mutable_baseinfo()->set_package_name(package_name);
    if (cls_name.empty())
    {
        select.mutable_baseinfo()->set_clname(tbname);
    }
    else
    {
        select.mutable_baseinfo()->set_clname(cls_name);
    }
    
    // 设置执行参数
    select.mutable_baseinfo()->set_max_records(max_records);
    select.set_mod_key(mod_key);
    select.set_record(msg + ";");
}

/**
 * @brief 获取protobuf字段类型
 * 
 * 根据protobuf字段的C++类型，返回对应的数据库字段类型
 * 
 * @param fieldDesc protobuf字段描述符
 * @return 字段类型（E_COLUMNTYPE_NUM表示数值类型，E_COLUMNTYPE_STRING表示字符串类型）
 */
int NFStoreProtoCommon::get_proto_field_type(const google::protobuf::FieldDescriptor& fieldDesc)
{
    switch (fieldDesc.cpp_type())
    {
        case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
        case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
        case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
        case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
        case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
        {
            return NFrame::E_COLUMNTYPE_NUM;
        }
        break;
        case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
        case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
        {
            return NFrame::E_COLUMNTYPE_STRING;
        }
        break;
        default:
            break;
    }
    return NFrame::E_COLUMNTYPE_NUM;
}

/**
 * @brief 从protobuf对象中提取查询条件列表
 * 
 * 遍历protobuf对象的所有字段，将非空字段转换为查询条件
 * 
 * @param message protobuf消息对象
 * @param vk_list 输出的查询条件列表
 * @return 操作结果（0表示成功）
 */
int NFStoreProtoCommon::get_vk_list_from_proto(const google::protobuf::Message& message, std::vector<NFrame::storesvr_vk>& vk_list)
{
    std::map<std::string, std::pair<int, std::string>> keyMap;

    const google::protobuf::Descriptor* pDesc = message.GetDescriptor();
    CHECK_NULL(0, pDesc);

    const google::protobuf::Reflection* pReflect = message.GetReflection();
    CHECK_NULL(0, pReflect);

    // 遍历所有字段，提取非空字段作为查询条件
    for (int i = 0; i < pDesc->field_count(); i++)
    {
        const google::protobuf::FieldDescriptor* pFieldDesc = pDesc->field(i);
        if (pFieldDesc == NULL) continue;
        if (pFieldDesc->options().GetExtension(nanopb).db_no_field()) continue;
        if (!pFieldDesc->is_repeated() && pReflect->HasField(message, pFieldDesc) == false) continue;
        if (pFieldDesc->is_repeated() && pReflect->FieldSize(message, pFieldDesc) == 0) continue;

        if (pFieldDesc->is_repeated() == false)
        {
            keyMap.emplace(pFieldDesc->name(),
                           std::make_pair((int)get_proto_field_type(*pFieldDesc), NFProtobufCommon::GetFieldsString(message, pFieldDesc)));
        }
    }

    // 将字段信息转换为查询条件列表
    for (auto iter = keyMap.begin(); iter != keyMap.end(); ++iter)
    {
        NFrame::storesvr_vk cmd1;
        cmd1.set_column_name(iter->first);
        cmd1.set_column_value(iter->second.second);
        cmd1.set_column_type((NFrame::storesvr_column_type)iter->second.first);
        cmd1.set_cmp_operator(NFrame::E_CMPOP_EQUAL);

        auto temp_iter = iter;
        temp_iter++;
        if (temp_iter != keyMap.end())
        {
            cmd1.set_logic_operator(NFrame::E_LOGICOP_AND);
        }

        vk_list.push_back(cmd1);
    }
    return 0;
}
