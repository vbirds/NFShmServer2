// -------------------------------------------------------------------------
//    @FileName         :    NFPebbleSha1.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPebbleSha1.h
//
// -------------------------------------------------------------------------

/**
 * @file NFPebbleSha1.h
 * @brief Pebble设备SHA1算法实现
 * 
 * 此文件提供了专门为Pebble智能手表设备优化的SHA1算法实现。
 * 提供了类封装的SHA1计算接口，支持增量数据处理和多种输出格式。
 */

#pragma once

/**
 * @brief Pebble设备SHA1算法实现类
 * 
 * NFPebbleSha1提供了专门为Pebble智能手表平台优化的SHA1算法实现。
 * 这个实现考虑了嵌入式设备的资源限制，提供了高效的哈希计算功能。
 * 
 * 主要特性：
 * - 嵌入式优化：专门为资源受限的设备优化
 * - 多种输出格式：支持十六进制和ASCII输出
 * - 状态管理：维护内部状态，支持增量更新
 * - 标准算法：符合SHA1标准规范
 * - 内存友好：高效的内存使用和缓冲区管理
 * 
 * SHA1算法特点：
 * - 输出长度：160位（20字节）
 * - 安全强度：曾经广泛使用，现在建议用于非安全敏感场景
 * - 计算效率：相比SHA256更快，适合资源受限环境
 * - 标准化：NIST FIPS 180-4标准
 * 
 * 适用场景：
 * - Pebble应用开发
 * - 嵌入式设备哈希计算
 * - 非安全敏感的数据指纹
 * - 资源受限环境下的哈希需求
 * - 兼容性要求SHA1的场景
 * 
 * 注意事项：
 * - SHA1已被发现存在碰撞攻击漏洞
 * - 不建议用于密码学安全应用
 * - 新项目建议使用SHA256或更强的算法
 * - 适合用于数据校验和兼容性需求
 * 
 * 使用方法：
 * @code
 * NFPebbleSha1 sha1;
 * 
 * // 十六进制输出
 * char hexOutput[41];  // 40字符+终止符
 * if (sha1.Encode2Hex("Hello World", hexOutput)) {
 *     std::cout << "SHA1 (Hex): " << hexOutput << std::endl;
 * }
 * 
 * // ASCII输出
 * char asciiOutput[21]; // 20字节+终止符
 * NFPebbleSha1 sha1_2;
 * if (sha1_2.Encode2Ascii("Hello World", asciiOutput)) {
 *     std::cout << "SHA1计算完成" << std::endl;
 *     // 处理二进制输出...
 * }
 * 
 * // 对于每次计算需要创建新实例
 * NFPebbleSha1 sha1_3;
 * sha1_3.Encode2Hex("Another message", hexOutput);
 * @endcode
 * 
 * @note 每个实例只能进行一次哈希计算
 * @note 多次计算需要创建新的实例
 * @note 输出缓冲区必须有足够的空间
 */
class NFPebbleSha1
{
public:
    /**
     * @brief 构造函数
     * 
     * 初始化SHA1计算器，设置初始状态和参数。
     */
    NFPebbleSha1();
    
    /**
     * @brief 虚析构函数
     * 
     * 清理资源，确保派生类正确析构。
     */
    virtual ~NFPebbleSha1();

    /**
     * @brief 计算SHA1并输出十六进制字符串
     * 
     * 对输入数据计算SHA1哈希值，并将结果转换为十六进制字符串输出。
     * 
     * @param lpData_Input 输入数据指针，以null结尾的C风格字符串
     * @param lpSHACode_Output [out] 输出缓冲区指针，必须至少41字节（40字符+终止符）
     * @return bool 计算成功返回true，失败返回false
     * 
     * @note 输出为40个字符的十六进制字符串（小写）加上null终止符
     * @note 输出缓冲区必须预先分配足够空间
     * @note 每个实例只能调用一次，多次调用行为未定义
     * @note 输入数据作为C风格字符串处理，遇到'\0'停止
     */
    bool Encode2Hex(const char *lpData_Input, char *lpSHACode_Output);

    /**
     * @brief 计算SHA1并输出二进制数据
     * 
     * 对输入数据计算SHA1哈希值，并将结果以二进制形式输出。
     * 
     * @param lpData_Input 输入数据指针，以null结尾的C风格字符串  
     * @param lpSHACode_Output [out] 输出缓冲区指针，必须至少20字节
     * @return bool 计算成功返回true，失败返回false
     * 
     * @note 输出为20字节的二进制SHA1哈希值
     * @note 输出缓冲区必须预先分配足够空间
     * @note 每个实例只能调用一次，多次调用行为未定义
     * @note 输入数据作为C风格字符串处理，遇到'\0'停止
     * @note 二进制输出不包含null终止符
     */
    bool Encode2Ascii(const char *lpData_Input, char *lpSHACode_Output);
    
private:
    /** @brief SHA1算法的5个32位消息摘要缓冲区 */
    unsigned int H[5];
    /** @brief 消息长度低32位（以位为单位） */
    unsigned int Length_Low;
    /** @brief 消息长度高32位（以位为单位） */
    unsigned int Length_High;
    /** @brief 512位消息块缓冲区 */
    unsigned char Message_Block[64];
    /** @brief 消息块数组的当前索引 */
    int Message_Block_Index;
    
private:
    /**
     * @brief 初始化SHA1状态
     * 
     * 设置SHA1算法的初始状态值和参数。
     * 
     * @note 这是内部方法，在构造时自动调用
     */
    void SHAInit();

    /**
     * @brief 添加数据长度
     * 
     * 更新已处理数据的长度计数器。
     * 
     * @param nDealDataLen 新处理的数据长度（字节数）
     * 
     * @note 这是内部方法，用于维护消息长度统计
     * @note 支持处理超过32位长度的数据
     */
    void AddDataLen(int nDealDataLen);

    /**
     * @brief 处理下一个512位消息块
     * 
     * 对当前缓冲区中的512位数据执行SHA1变换操作。
     * 
     * @note 这是SHA1算法的核心处理函数
     * @note 只有当缓冲区满（64字节）时才调用
     */
    void ProcessMessageBlock();

    /**
     * @brief 填充消息到512位边界
     * 
     * 在消息末尾添加填充位和长度信息，使总长度为512的倍数。
     * 
     * @note 按照SHA1标准进行填充：先添加1位1，然后添加0位，最后添加64位长度
     * @note 这是算法完成前的必要步骤
     */
    void PadMessage();

    /**
     * @brief 执行循环左移操作
     * 
     * 对32位字进行指定位数的循环左移，这是SHA1算法的基本操作。
     * 
     * @param bits 左移的位数（0-31）
     * @param word 要移位的32位字
     * @return unsigned int 移位后的结果
     * 
     * @note 内联函数，为了性能优化
     * @note 循环左移是SHA1算法中的核心运算之一
     */
    inline unsigned CircularShift(int bits, unsigned word);
};