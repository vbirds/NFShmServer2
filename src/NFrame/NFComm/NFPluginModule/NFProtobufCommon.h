// -------------------------------------------------------------------------
//    @FileName         :    NFProtobufCommon.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFProtobufCommon
//    @Description      :    Protocol Buffers公共工具模块，提供消息序列化、反序列化
//                           数据库映射、动态消息创建等完整的Protobuf操作支持
//
// -------------------------------------------------------------------------
#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "google/protobuf/message.h"
#include "NFIHttpHandle.h"
#include "NFComm/NFPluginModule/NFILuaLoader.h"
#include "NFComm/NFCore/NFSingleton.hpp"
#include "google/protobuf/dynamic_message.h"
#include "NFComm/NFPluginModule/NFJson2PB/NFPbToJson.h"
#include "NFComm/NFKernelMessage/nanopb.pb.h"
#include <vector>
#include <map>
#include <unordered_map>

/// @brief 默认Protobuf包名定义
#define DEFINE_DEFAULT_PROTO_PACKAGE "proto_ff"
/// @brief 默认Protobuf包名前缀（带点号）
#define DEFINE_DEFAULT_PROTO_PACKAGE_ADD "proto_ff."

/**
 * @brief 数据库表列信息结构体
 *
 * 该结构体用于存储数据库表列的详细信息，
 * 支持从Protobuf消息自动生成数据库表结构：
 *
 * 列属性：
 * - 数据类型和长度限制
 * - 主键、联合键、索引键标识
 * - 自动递增和默认值设置
 * - 非空约束和注释信息
 *
 * 使用场景：
 * - 自动建表和表结构同步
 * - ORM框架的元数据管理
 * - 数据库迁移工具
 * - 表结构验证和比较
 */
struct DBTableColInfo
{
    /**
     * @brief 默认构造函数
     *
     * 初始化所有字段为默认值，确保结构体状态一致。
     */
    DBTableColInfo()
    {
        m_colType = 0;
        m_primaryKey = false;
        m_unionKey = false;
        m_indexKey = false;
        m_bufsize = 32;
        m_fieldIndex = 0;
        m_autoIncrement = false;
        m_autoIncrementValue = 0;
        m_defaultValue = 0;
        m_isDefaultValue = false;
        m_notNull = false;
    }

    uint32_t m_colType;               ///< 列数据类型（内部编码）
    bool m_primaryKey;                ///< 是否为主键
    bool m_unionKey;                  ///< 是否为联合键
    bool m_indexKey;                  ///< 是否为索引键
    bool m_autoIncrement;             ///< 是否自动递增
    bool m_notNull;                   ///< 是否为非空约束
    bool m_isDefaultValue;            ///< 是否设置了默认值
    uint32_t m_defaultValue;          ///< 默认值（数值类型）
    int64_t m_autoIncrementValue;     ///< 自动递增的起始值
    uint32_t m_bufsize;               ///< 缓冲区大小（字符串类型）
    uint32_t m_fieldIndex;            ///< 字段在消息中的索引
    std::string m_comment;            ///< 列注释信息
};

/**
 * @brief Protocol Buffers公共工具类
 *
 * NFProtobufCommon 提供了完整的Protobuf操作工具集：
 *
 * 1. 消息序列化和反序列化：
 *    - JSON ↔ Protobuf 双向转换
 *    - XML ↔ Protobuf 双向转换
 *    - Lua ↔ Protobuf 数据交换
 *    - HTTP参数到Protobuf映射
 *
 * 2. 数据库集成功能：
 *    - 自动提取数据库字段信息
 *    - Protobuf到数据库映射
 *    - 主键和索引信息提取
 *    - 表结构自动生成
 *
 * 3. 动态消息处理：
 *    - 运行时消息类型创建
 *    - .proto文件动态加载
 *    - 反射式字段访问
 *    - 类型安全的操作接口
 *
 * 4. 字段操作工具：
 *    - 字段值的读取和设置
 *    - 重复字段的批量处理
 *    - 字段类型和属性查询
 *    - 字段验证和转换
 *
 * 5. 枚举处理支持：
 *    - 枚举名称到数值映射
 *    - 宏定义到枚举转换
 *    - 枚举类型查找和创建
 *    - 动态枚举值解析
 *
 * 设计特点：
 * - 单例模式确保全局一致性
 * - 静态方法提供便捷接口
 * - 类型安全的反射操作
 * - 高性能的批量处理
 * - 完善的错误处理机制
 *
 * 使用场景：
 * - 网络协议处理
 * - 数据库ORM框架
 * - 配置文件解析
 * - API接口开发
 * - 跨语言数据交换
 *
 * 架构优势：
 * - 统一的数据格式处理
 * - 自动化的数据库集成
 * - 灵活的动态类型支持
 * - 高度可复用的工具集
 */
class _NFExport NFProtobufCommon : public NFSingleton<NFProtobufCommon>
{
public:
    /**
     * @brief 获取字段的字符串值
     * @param message Protobuf消息对象
     * @param pFieldDesc 字段描述符
     * @return 字段的字符串表示
     *
     * 从Protobuf消息中提取指定字段的值，并转换为字符串格式。
     * 支持所有基础数据类型的转换。
     */
    static std::string GetFieldsString(const google::protobuf::Message &message, const google::protobuf::FieldDescriptor *pFieldDesc);

    /**
     * @brief 获取重复字段指定索引的字符串值
     * @param message Protobuf消息对象
     * @param pFieldDesc 字段描述符
     * @param index 重复字段的索引
     * @return 指定索引位置的字段值字符串
     *
     * 专门处理重复字段（repeated）的值提取，
     * 通过索引访问数组中的特定元素。
     */
    static std::string GetRepeatedFieldsString(const google::protobuf::Message &message, const google::protobuf::FieldDescriptor *pFieldDesc, int index);

    /**
     * @brief 设置字段的字符串值
     * @param message Protobuf消息对象
     * @param pFieldDesc 字段描述符
     * @param strValue 要设置的字符串值
     * @return 设置成功返回true，失败返回false
     *
     * 将字符串值转换为字段的实际类型并设置到消息中。
     * 自动进行类型转换和验证。
     */
    static bool SetFieldsString(google::protobuf::Message& message, const google::protobuf::FieldDescriptor* pFieldDesc, const std::string& strValue);

    /**
     * @brief 向重复字段添加字符串值
     * @param message Protobuf消息对象
     * @param pFieldDesc 字段描述符
     * @param strValue 要添加的字符串值
     * @return 添加成功返回true，失败返回false
     *
     * 专门处理重复字段的值添加操作，
     * 将新值追加到字段数组的末尾。
     */
    static bool AddFieldsString(google::protobuf::Message& message, const google::protobuf::FieldDescriptor* pFieldDesc, const std::string& strValue);

    /**
     * @brief 根据消息类型名查找消息原型
     * @param full_name 完整的消息类型名（包含包名）
     * @return 找到的消息原型指针，未找到返回nullptr
     *
     * 在全局消息类型注册表中查找指定名称的消息类型，
     * 返回该类型的默认实例，可用于创建新的消息对象。
     */
    static const ::google::protobuf::Message *FindMessageTypeByName(const std::string &full_name);

    /**
     * @brief 根据消息类型名创建消息实例
     * @param full_name 完整的消息类型名（包含包名）
     * @return 创建的消息实例指针，创建失败返回nullptr
     *
     * 基于指定的消息类型名创建新的消息实例，
     * 调用者负责释放返回的指针。
     */
    static ::google::protobuf::Message *CreateMessageByName(const std::string &full_name);

    /**
     * @brief 从消息中提取数据库字段映射
     * @param message Protobuf消息对象
     * @param mapFields 输出的字段名到值的映射
     * @param lastFieldName 字段名前缀，用于嵌套消息
     *
     * 遍历消息的所有字段，提取字段名和值的映射关系，
     * 用于数据库操作和数据导出。支持嵌套消息的递归处理。
     */
    static void GetDBMapFieldsFromMessage(const google::protobuf::Message &message, std::map<std::string, std::string> &mapFields, const std::string& lastFieldName = "");

    /**
     * @brief 从消息描述符中提取字段名列表
     * @param pDesc 消息描述符
     * @param vecFields 输出的字段名列表
     * @param lastFieldName 字段名前缀，用于嵌套消息
     *
     * 基于消息描述符提取所有字段名，
     * 用于数据库表结构生成和字段验证。
     */
    static void GetDBFieldsFromDesc(const google::protobuf::Descriptor *pDesc, std::vector<std::string> &vecFields, const std::string& lastFieldName = "");

    /**
     * @brief 获取消息中的私有字段信息
     * @param message Protobuf消息对象
     * @param field 输出的字段名
     * @param fieldValue 输出的字段值
     * @return 操作结果，0表示成功
     *
     * 提取消息中标记为私有的特殊字段，
     * 通常用于内部标识和状态管理。
     */
    static int GetPrivateFieldsFromMessage(const google::protobuf::Message &message, std::string& field, std::string& fieldValue);

    /**
     * @brief 从消息描述符获取私有键字段
     * @param pDesc 消息描述符
     * @param field 输出的私有键字段名
     * @return 操作结果，0表示成功
     *
     * 查找消息类型中标记为私有键的字段，
     * 用于特殊的索引和查找操作。
     */
    static int GetPrivateKeyFromMessage(const google::protobuf::Descriptor *pDesc, std::string& field);

    /**
     * @brief 从消息描述符获取宏类型映射
     * @param pDesc 消息描述符
     * @param base 基础名称
     * @param fieldMap 输出的字段映射
     * @return 操作结果，0表示成功
     *
     * 提取消息中的宏类型定义，
     * 用于代码生成和类型转换。
     */
    static int GetMacroTypeFromMessage(const google::protobuf::Descriptor *pDesc, const std::string& base, std::unordered_map<std::string, std::string>& fieldMap);

    /**
     * @brief 从消息中提取映射类型的数据库字段
     * @param message Protobuf消息对象
     * @param keyMap 输出的键字段映射
     * @param kevValueMap 输出的键值字段映射
     * @param lastFieldName 字段名前缀
     *
     * 专门处理Protobuf的map类型字段，
     * 分别提取键和值的映射关系。
     */
    static void GetMapDBFieldsFromMessage(const google::protobuf::Message &message, std::map<std::string, std::string> &keyMap, std::map<std::string, std::string> &kevValueMap, const std::string& lastFieldName = "");

    /**
     * @brief 从消息中提取数据库字段列表
     * @param message Protobuf消息对象
     * @param kevValueList 输出的字段名值对列表
     * @param lastFieldName 字段名前缀
     *
     * 提取消息的所有字段为有序的名值对列表，
     * 保持字段在消息中的定义顺序。
     */
    static void GetDBFieldsFromMessage(const google::protobuf::Message &message, std::vector<std::pair<std::string, std::string>> &kevValueList, const std::string& lastFieldName = "");

    /**
     * @brief 从映射字段创建数据库消息
     * @param result 输入的字段映射
     * @param pMessageObject 目标消息对象
     * @param lastFieldName 字段名前缀
     * @param pbAllEmpty 输出参数，指示是否所有字段都为空
     *
     * 将键值映射转换为Protobuf消息，
     * 用于从数据库结果集重建消息对象。
     */
    static void GetDBMessageFromMapFields(const std::map<std::string, std::string> &result, google::protobuf::Message *pMessageObject, const std::string& lastFieldName = "", bool* pbAllEmpty = NULL);

    /**
     * @brief 从哈希映射字段创建消息
     * @param result 输入的字段哈希映射
     * @param pMessageObject 目标消息对象
     * @param lastFieldName 字段名前缀
     * @param pbAllEmpty 输出参数，指示是否所有字段都为空
     * @return 操作结果，0表示成功
     *
     * 高性能版本的映射到消息转换，
     * 使用哈希表提高查找效率。
     */
    static int GetMessageFromMapFields(const std::unordered_map<std::string, std::string>& result, google::protobuf::Message* pMessageObject, const std::string& lastFieldName = "", bool* pbAllEmpty = NULL);

    /**
     * @brief 将消息转换为日志字符串
     * @param msg 输出的日志字符串
     * @param message Protobuf消息对象
     * @param split 字段分隔符，默认为"|"
     * @param repeated_split 重复字段内分隔符，默认为";"
     * @param repeated_field_split 重复字段分隔符，默认为","
     * @param repeated 是否为重复字段
     * @return 操作结果，0表示成功
     *
     * 将Protobuf消息格式化为易读的日志字符串，
     * 支持自定义分隔符和重复字段处理。
     */
    static int MessageToLogStr(std::string &msg, const google::protobuf::Message &message, const std::string& split = "|", const std::string& repeated_split = ";", const std::string& repeated_field_split = ",", bool repeated = false);

    /**
     * @brief 将Protobuf消息转换为XML格式
     * @param message Protobuf消息对象
     * @param json 输出的XML字符串
     * @return 转换成功返回true，失败返回false
     *
     * 将Protobuf消息序列化为XML格式，
     * 用于配置文件和数据交换。
     */
    static bool ProtoMessageToXml(const google::protobuf::Message &message,
                                  std::string *json);

    /**
     * @brief 将XML格式转换为Protobuf消息
     * @param json 输入的XML字符串
     * @param message 目标Protobuf消息对象
     * @return 转换成功返回true，失败返回false
     *
     * 从XML字符串反序列化为Protobuf消息，
     * 用于配置文件解析和数据导入。
     */
    static bool XmlToProtoMessage(const std::string &json,
                                  google::protobuf::Message *message);

    /**
     * @brief 将Protobuf消息转换为JSON格式
     * @param message Protobuf消息对象
     * @param json 输出的JSON字符串
     * @param error 输出的错误信息
     * @return 转换成功返回true，失败返回false
     *
     * 将Protobuf消息序列化为JSON格式，
     * 是最常用的数据交换格式转换方法。
     */
    static bool ProtoMessageToJson(const google::protobuf::Message &message,
                                   std::string *json,
                                   std::string *error = NULL);

    /**
     * @brief 使用自定义选项将Protobuf消息转换为JSON
     * @param options JSON转换选项配置
     * @param message Protobuf消息对象
     * @param json 输出的JSON字符串
     * @param error 输出的错误信息
     * @return 转换成功返回true，失败返回false
     *
     * 提供更多控制选项的JSON转换方法，
     * 可以自定义字段命名、空值处理等。
     */
    static bool ProtoMessageToJson(const NFJson2PB::Pb2JsonOptions& options, const google::protobuf::Message &message,
                                   std::string *json,
                                   std::string *error = NULL);

    /**
     * @brief 将Protobuf消息转换为JSON流
     * @param message Protobuf消息对象
     * @param json 输出的JSON流
     * @param error 输出的错误信息
     * @return 转换成功返回true，失败返回false
     *
     * 流式JSON转换，适用于大型消息和内存受限环境。
     */
    static bool ProtoMessageToJson(const google::protobuf::Message &message,
                                   google::protobuf::io::ZeroCopyOutputStream *json,
                                   std::string *error = NULL);

    /**
     * @brief 将JSON格式转换为Protobuf消息
     * @param json 输入的JSON字符串
     * @param message 目标Protobuf消息对象
     * @param error 输出的错误信息
     * @return 转换成功返回true，失败返回false
     *
     * 从JSON字符串反序列化为Protobuf消息，
     * 广泛用于API接口和配置解析。
     */
    static bool JsonToProtoMessage(const std::string &json,
                                   google::protobuf::Message *message,
                                   std::string *error = NULL);

    /**
     * @brief 检查JSON到Protobuf消息转换的有效性
     * @param json_string 输入的JSON字符串
     * @param message 目标消息对象
     * @param repeated 是否为重复字段
     * @return 转换有效返回true，无效返回false
     *
     * 验证JSON格式是否与消息类型兼容，
     * 不执行实际转换，仅检查格式正确性。
     */
    static bool CheckJsonToProtoMessage(const std::string& json_string, google::protobuf::Message* message, bool repeated);

    /**
     * @brief 检查JSON值到Protobuf字段转换的有效性
     * @param json_string JSON值字符串
     * @param fieldName 字段名
     * @param fieldType 字段类型
     * @param isRepeated 是否为重复字段
     * @return 转换有效返回true，无效返回false
     *
     * 细粒度的字段级别验证，
     * 确保JSON值与字段类型匹配。
     */
    static bool CheckJsonValueToProtoField(const std::string& json_string, const std::string& fieldName, google::protobuf::FieldDescriptor::CppType fieldType, bool isRepeated);

    /**
     * @brief 将JSON转换为指定的Protobuf字段
     * @param json JSON字符串
     * @param pMessage 目标消息对象
     * @param pFiledDesc 字段描述符
     * @return 转换成功返回true，失败返回false
     *
     * 精确的字段级JSON转换，
     * 用于部分字段更新和增量数据处理。
     */
    static bool JsonToProtoField(const std::string& json, google::protobuf::Message* pMessage, const google::protobuf::FieldDescriptor* pFiledDesc);

    /**
     * @brief 从JSON流转换为Protobuf消息
     * @param stream 输入的JSON流
     * @param message 目标Protobuf消息对象
     * @param error 输出的错误信息
     * @return 转换成功返回true，失败返回false
     *
     * 流式JSON解析，适用于大型JSON文件和网络流。
     */
    static bool JsonToProtoMessage(google::protobuf::io::ZeroCopyInputStream *stream,
                                   google::protobuf::Message *message,
                                   std::string *error = NULL);

    /**
     * @brief 将Lua数据转换为Protobuf消息
     * @param luaRef Lua数据引用
     * @param pMessageObject 目标Protobuf消息对象
     * @return 转换成功返回true，失败返回false
     *
     * 支持Lua脚本与Protobuf的数据交换，
     * 用于脚本配置和动态数据处理。
     */
    static bool LuaToProtoMessage(NFLuaRef luaRef, google::protobuf::Message *pMessageObject);

    /**
     * @brief 按字段复制消息内容
     * @param pSrcMessage 源消息对象
     * @param pDescMessage 目标消息对象
     * @return 操作结果，0表示成功
     *
     * 字段级别的消息复制，只复制目标消息中存在的字段，
     * 用于消息的部分更新和类型转换。
     */
    static int CopyMessageByFields(google::protobuf::Message *pSrcMessage, const google::protobuf::Message *pDescMessage);

    /**
     * @brief 从HTTP GET请求获取消息数据
     * @param pSrcMessage 目标消息对象
     * @param req HTTP请求处理器
     * @return 操作结果，0表示成功
     *
     * 将HTTP GET请求的查询参数映射到Protobuf消息字段，
     * 用于RESTful API的参数解析。
     */
    static int GetMessageFromGetHttp(google::protobuf::Message *pSrcMessage, const NFIHttpHandle &req);

    /**
     * @brief 从消息描述符中提取数据库字段信息
     * @param pDesc 消息描述符
     * @param primaryKeyMap 输出的主键信息映射
     * @param mapFileds 输出的字段信息列表
     * @param lastFieldName 字段名前缀
     * @param lastComment 注释前缀
     * @return 操作结果，0表示成功
     *
     * 通过Protobuf反射机制分析消息结构，
     * 自动提取数据库表的列信息和约束。
     */
    static int GetDbFieldsInfoFromMessage(const google::protobuf::Descriptor *pDesc, std::map<std::string, DBTableColInfo> &primaryKeyMap, std::vector<std::pair<std::string, DBTableColInfo>> &mapFileds, const std::string& lastFieldName = "", const std::string& lastComment = "");

    /**
     * @brief 从数据库数据类型获取Protobuf数据类型
     * @param dbDataType 数据库数据类型名
     * @param strColumnType 列类型字符串
     * @return Protobuf数据类型编码
     *
     * 数据库类型到Protobuf类型的映射转换，
     * 用于ORM框架和代码生成工具。
     */
    static uint32_t GetPBDataTypeFromDBDataType(const std::string& dbDataType, const std::string& strColumnType);

    /**
     * @brief 从Protobuf数据类型获取数据库数据类型
     * @param pbDataType Protobuf数据类型编码
     * @param textMax 文本类型的最大长度
     * @return 数据库数据类型字符串
     *
     * Protobuf类型到数据库类型的映射转换，
     * 用于自动建表和表结构同步。
     */
    static std::string GetDBDataTypeFromPBDataType(uint32_t pbDataType, uint32_t textMax);

    /**
     * @brief 获取Protobuf消息的基础名称
     * @param message Protobuf消息对象
     * @return 消息的基础名称（不含包名）
     */
    static std::string GetProtoBaseName(const google::protobuf::Message& message);

    /**
     * @brief 获取Protobuf消息的包名
     * @param message Protobuf消息对象
     * @return 消息的包名
     */
    static std::string GetProtoPackageName(const google::protobuf::Message& message);

    /**
     * @brief 从消息名称获取基础名称
     * @param message 消息完整名称
     * @return 消息的基础名称（不含包名）
     */
    static std::string GetProtoBaseName(const std::string& message);

    /**
     * @brief 从消息名称获取包名
     * @param message 消息完整名称
     * @return 消息的包名
     */
    static std::string GetProtoPackageName(const std::string& message);

    /**
     * @brief 获取描述存储类名称
     * @param message Protobuf消息对象
     * @return 描述存储使用的类名
     *
     * 用于存储系统的类名生成，
     * 确保类名的一致性和唯一性。
     */
    static std::string GetDescStoreClsName(const google::protobuf::Message& message);

public:
    /**
     * @brief 从消息中获取主键值
     * @param pMessage 消息对象指针
     * @param result 输出的主键值字符串
     * @return 操作结果，0表示成功
     *
     * 提取消息中标记为主键的字段值，
     * 用于数据库查询和索引操作。
     */
    static int GetPrimarykeyFromMessage(const google::protobuf::Message* pMessage, std::string &result);

    /**
     * @brief 设置消息的主键值
     * @param pMessage 消息对象指针
     * @param data 主键值字符串
     * @return 操作结果，0表示成功
     *
     * 将字符串形式的主键值设置到消息的主键字段中，
     * 自动进行类型转换和验证。
     */
    static int SetPrimarykeyFromMessage(google::protobuf::Message* pMessage, const std::string& data);

public:
    ////////////////////////////////////////store pb//////////////////////////////////////////////////////////////////

public:
    /**
     * @brief 构造函数
     *
     * 初始化Protobuf公共工具类，设置描述符池和消息工厂。
     */
    NFProtobufCommon();

    /**
     * @brief 析构函数
     *
     * 清理描述符池、消息工厂和相关资源。
     */
    virtual ~NFProtobufCommon();

public:
    /**
     * @brief 加载Protobuf描述符文件
     * @param ds 描述符文件路径（通过protoc --descriptor_set_out生成）
     * @return 加载结果，0表示成功
     *
     * 动态加载.proto文件的编译结果，
     * 支持运行时添加新的消息类型。
     */
    int LoadProtoDsFile(const std::string &ds);

    /**
     * @brief 查找动态消息类型描述符
     * @param full_name 完整的消息类型名
     * @return 找到的描述符指针，未找到返回nullptr
     *
     * 在动态加载的描述符池中查找消息类型，
     * 支持运行时类型发现。
     */
    const google::protobuf::Descriptor *FindDynamicMessageTypeByName(const std::string &full_name);

    /**
     * @brief 创建动态消息实例
     * @param full_name 完整的消息类型名
     * @return 创建的消息实例指针，创建失败返回nullptr
     *
     * 基于动态加载的类型创建消息实例，
     * 支持运行时消息创建。
     */
    ::google::protobuf::Message *CreateDynamicMessageByName(const std::string &full_name);

    /**
     * @brief 获取字段解析类型
     * @param pFieldDesc 字段描述符
     * @return 字段的解析类型枚举
     *
     * 分析字段的解析方式和特性，
     * 用于自定义解析器和验证器。
     */
    static FieldParseType GetFieldPasreType(const google::protobuf::FieldDescriptor *pFieldDesc);

    /**
     * @brief 获取字段的数据库最大计数
     * @param pFieldDesc 字段描述符
     * @return 数据库中该字段的最大计数限制
     */
    int GetFieldsDBMaxCount(const google::protobuf::FieldDescriptor *pFieldDesc) const;

    /**
     * @brief 获取字段的数据库最大大小
     * @param pFieldDesc 字段描述符
     * @return 数据库中该字段的最大大小限制
     */
    int GetFieldsDBMaxSize(const google::protobuf::FieldDescriptor *pFieldDesc) const;

    /**
     * @brief 获取字段的最大计数
     * @param pFieldDesc 字段描述符
     * @return 字段的最大计数限制
     */
    int GetFieldsMaxCount(const google::protobuf::FieldDescriptor *pFieldDesc) const;

    /**
     * @brief 获取字段的最大大小
     * @param pFieldDesc 字段描述符
     * @return 字段的最大大小限制
     */
    int GetFieldsMaxSize(const google::protobuf::FieldDescriptor *pFieldDesc) const;

    /**
     * @brief 根据名称查找枚举值描述符
     * @param name 枚举值名称
     * @return 找到的枚举值描述符，未找到返回nullptr
     */
    const ::google::protobuf::EnumValueDescriptor* FindEnumValueByName(const string& name) const;

    /**
     * @brief 根据名称查找枚举类型描述符
     * @param name 枚举类型名称
     * @return 找到的枚举类型描述符，未找到返回nullptr
     */
    const ::google::protobuf::EnumDescriptor* FindEnumTypeByName(const string& name) const;

    /**
     * @brief 根据宏名称查找枚举数值
     * @param enumName 枚举类型名称
     * @param macroName 宏名称
     * @param value 输出的枚举值
     * @return 查找成功返回true，失败返回false
     *
     * 支持通过宏定义查找对应的枚举值，
     * 用于配置文件和脚本集成。
     */
    bool FindEnumNumberByMacroName(const string& enumName, const std::string& macroName, std::string& value);

public:
    std::string m_fileMd5;                                               ///< 描述符文件的MD5值
    google::protobuf::DescriptorPool *m_pDescriptorPool;                ///< 描述符池
    std::vector<google::protobuf::DescriptorPool *> m_pOldPoolVec;      ///< 旧描述符池列表
    google::protobuf::DynamicMessageFactory *m_pDynamicMessageFactory;  ///< 动态消息工厂

public:
	/**
	 * @brief 枚举宏数据结构
	 *
	 * 存储枚举类型的各种映射关系，
	 * 支持名称、数值和宏名之间的快速转换。
	 */
	struct EnumMacroData
	{
		std::unordered_map<std::string, int> m_enumNameToNumber;    ///< 枚举名到数值的映射
		std::unordered_map<std::string, int> m_enumToNumber;        ///< 枚举到数值的映射
		std::unordered_map<int, std::string> m_numberToEnumName;    ///< 数值到枚举名的映射
	};
    std::unordered_map<std::string, EnumMacroData> m_enumMacroData;     ///< 枚举宏数据映射
};

