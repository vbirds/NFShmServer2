// -------------------------------------------------------------------------
//    @FileName         :    NFShm.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFShm.h
 * @brief 跨平台共享内存封装类
 * 
 * 此文件提供了跨平台的共享内存操作封装，支持Windows和Linux系统。
 * 包括共享内存的创建、连接、分离等操作，以及相关的异常处理。
 */

#pragma once

#include "NFPlatform.h"
#include "NFException.hpp"

/**
 * @name 平台相关头文件和类型定义
 * @brief 根据平台包含相应的共享内存头文件
 * @{
 */
#if NF_PLATFORM == NF_PLATFORM_WIN
#include <Windows.h>
/** @brief Windows平台的key_t类型定义 */
typedef long key_t;
#else
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
/** @} */

/**
 * @brief 共享内存异常类
 * 
 * 继承自NFException的专用异常类，用于共享内存操作中的错误处理。
 * 提供了共享内存操作失败时的详细错误信息。
 * 
 * 使用方法：
 * @code
 * try {
 *     NFShm shm(1024, 12345);
 * } catch (const NFShmException& e) {
 *     std::cout << "共享内存错误: " << e.what() << std::endl;
 * }
 * @endcode
 */
struct NFShmException : public NFException
{
	/**
	 * @brief 构造函数（仅错误消息）
	 * @param buffer 错误描述信息
	 */
	NFShmException(const std::string &buffer) : NFException(buffer) {};
	
	/**
	 * @brief 构造函数（错误消息+错误码）
	 * @param buffer 错误描述信息
	 * @param err 系统错误码
	 */
	NFShmException(const std::string &buffer, int err) : NFException(buffer, err) {};
	
	/**
	 * @brief 析构函数
	 */
	~NFShmException() throw() {};
};

/**
 * @brief 跨平台共享内存连接类
 * 
 * NFShm提供了完整的共享内存操作功能，支持Windows和Linux平台。
 * 主要功能包括共享内存的创建、连接、访问和管理。
 * 
 * 主要特性：
 * - 跨平台兼容：统一的API，支持Windows和Linux
 * - 自动管理：支持RAII模式的资源管理
 * - 权限控制：共享内存权限默认为0666
 * - 所有权管理：支持所有者和非所有者模式
 * - 异常安全：完整的异常处理机制
 * 
 * 所有权说明：
 * - bOwner = false: 析构时不分离（detach）共享内存
 * - bOwner = true: 析构时分离共享内存
 * 
 * 使用场景：
 * - 进程间数据共享
 * - 高性能数据交换
 * - 大数据缓存共享
 * - 多进程协同工作
 * 
 * 使用方法：
 * @code
 * try {
 *     // 创建或连接共享内存
 *     NFShm shm(1024 * 1024, 12345, true);  // 1MB，key=12345，拥有所有权
 *     
 *     // 获取共享内存指针
 *     void* ptr = shm.getPointer();
 *     
 *     // 检查是否是新创建的
 *     if (shm.iscreate()) {
 *         // 初始化共享内存
 *         memset(ptr, 0, shm.size());
 *     }
 *     
 *     // 使用共享内存
 *     char* data = static_cast<char*>(ptr);
 *     strcpy(data, "Hello Shared Memory");
 *     
 * } catch (const NFShmException& e) {
 *     std::cerr << "共享内存操作失败: " << e.what() << std::endl;
 * }
 * @endcode
 * 
 * @note 共享内存的权限默认为0666
 * @note 使用完毕后会根据所有权设置自动进行清理
 */
class _NFExport NFShm
{
public:

	/**
	 * @brief 默认构造函数
	 * 
	 * 创建一个未初始化的共享内存对象，需要后续调用init()方法进行初始化。
	 * 
	 * @param bOwner 是否拥有共享内存所有权，默认为false
	 *               - true: 析构时会分离共享内存
	 *               - false: 析构时不分离共享内存
	 */
#if NF_PLATFORM == NF_PLATFORM_WIN
	NFShm(bool bOwner = false) : _bOwner(bOwner), _shmSize(0), _shmKey(0), _bCreate(true), _pshm(nullptr), _shemID(nullptr), _fileID(nullptr) {}
#else
	NFShm(bool bOwner = false) : _bOwner(bOwner), _shmSize(0), _shmKey(0), _bCreate(true), _pshm(nullptr), _shemID(-1) {}
#endif

	/**
	 * @brief 带参数的构造函数
	 * 
	 * 创建并初始化共享内存对象，自动进行共享内存的创建或连接操作。
	 * 
	 * @param iShmSize 共享内存大小（字节）
	 * @param iKey 共享内存的键值，用于标识共享内存段
	 * @param bOwner 是否拥有共享内存所有权，默认为false
	 * @throws NFShmException 当共享内存操作失败时抛出异常
	 * 
	 * @note 如果指定key的共享内存已存在且大小匹配，则连接到现有的共享内存
	 * @note 如果共享内存不存在或大小不匹配，则创建新的共享内存
	 */
	NFShm(size_t iShmSize, key_t iKey, bool bOwner = false);

	/**
	 * @brief 析构函数
	 * 
	 * 根据所有权设置决定是否分离共享内存。
	 * 如果拥有所有权(bOwner=true)，会自动分离共享内存。
	 */
	~NFShm();

	/**
	 * @brief 初始化共享内存
	 * 
	 * 手动初始化共享内存对象，用于默认构造函数创建的对象。
	 * 执行共享内存的创建或连接操作。
	 * 
	 * @param iShmSize 共享内存大小（字节）
	 * @param iKey 共享内存的键值，用于标识共享内存段
	 * @param bOwner 是否拥有共享内存所有权，默认为false
	 * @throws NFShmException 当共享内存操作失败时抛出异常
	 * 
	 * @note 如果对象已经初始化过，会先清理原有的共享内存连接
	 */
	void init(size_t iShmSize, key_t iKey, bool bOwner = false);

	/**
	 * @brief 判断共享内存是否为新创建
	 * 
	 * 判断当前连接的共享内存是新创建的还是连接到已存在的。
	 * 新创建的共享内存通常需要进行初始化操作。
	 * 
	 * @return bool true表示新创建的共享内存，false表示连接到已存在的共享内存
	 * 
	 * @note 这个信息对于决定是否需要初始化共享内存内容很重要
	 */
	bool iscreate() { return _bCreate; }

	/**
	 * @brief 获取共享内存的指针
	 * 
	 * 返回指向共享内存起始地址的指针，可以直接用于读写操作。
	 * 
	 * @return void* 共享内存的起始地址指针
	 * 
	 * @warning 使用返回的指针时要确保不超出共享内存的大小范围
	 * @warning 多进程访问时需要自行处理同步问题
	 */
	void *getPointer() { return _pshm; }

	/**
	* @brief  获取共享内存Key.
	*
	* @return key_t* ,共享内存key
	*/
	key_t getkey() { return _shmKey; }

	/**
	* @brief  获取共享内存ID.
	*
	* @return int ,共享内存Id
	*/
#if NF_PLATFORM == NF_PLATFORM_WIN
	HANDLE getid() { return _shemID; }
#else
	int getid() { return _shemID; }
#endif

	/**
	*  @brief  获取共享内存大小.
	*
	* return size_t,共享内存大小
	*/
	size_t size() { return _shmSize; }

	/**
	*  @brief 解除共享内存，在当前进程中解除共享内存
	* 共享内存在当前进程中无效
	* @return int
	*/
	int detach();

	/**
	*  @brief 删除共享内存.
	*
	* 完全删除共享内存
	*/
	int del();

protected:

	/**
	* 是否拥有共享内存
	*/
	bool            _bOwner;

	/**
	* 共享内存大小
	*/
	size_t          _shmSize;

	/**
	* 共享内存key
	*/
	key_t           _shmKey;

	/**
	* 是否是生成的共享内存
	*/
	bool            _bCreate;

	/**
	* 共享内存
	*/
	void            *_pshm;

	/**
	* 共享内存id
	*/
#if NF_PLATFORM == NF_PLATFORM_WIN
	HANDLE			_shemID;
	HANDLE			_fileID;
#else
	int             _shemID;
#endif
};



