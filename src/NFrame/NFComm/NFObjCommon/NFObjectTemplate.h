// -------------------------------------------------------------------------
//    @FileName         :    NFObjectTemplate.h
//    @Author           :    gaoyi
//    @Date             :    23-10-26
//    @Email			:    445267987@qq.com
//    @Module           :    NFObjectTemplate
//
// -------------------------------------------------------------------------

/**
 * @file NFObjectTemplate.h
 * @brief 对象模板类定义文件
 * @details 这个文件定义了NFrame框架中的核心对象模板类，提供了对象的统一管理、迭代器支持、
 *          共享内存管理、定时器功能、事件系统、以及对象的创建、销毁、查找等核心功能。
 *          该模板类是整个对象系统的基础，所有业务对象都应该继承自这个模板类。
 * 
 * @author gaoyi
 * @date 23-10-26
 * @email 445267987@qq.com
 * 
 * @note 该文件包含了：
 *       - NFObjectTemplate: 核心对象模板类
 *       - NFObjectTemplateNoParent: 无父类的对象模板特化
 *       - 对象迭代器相关定义
 *       - 对象管理相关宏定义
 *       - 共享内存对象的创建和管理功能
 * 
 * @warning 使用时需要注意：
 *          - 必须在对象构造时正确初始化共享内存模式
 *          - 对象销毁时需要清理相关资源
 *          - 继承类需要实现CreateInit()和ResumeInit()方法
 */

#pragma once

#include <NFComm/NFPluginModule/NFMemTracker.h>

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFComm/NFObjCommon/NFObjectIterator.h"
#include "NFComm/NFPluginModule/NFIMemMngModule.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFComm/NFPluginModule/NFIMessageModule.h"

/**
 * @class NFObjectTemplate
 * @brief 核心对象模板类
 * @details 提供了完整的对象生命周期管理、内存管理、迭代器支持等功能。
 *          这是NFrame框架中所有业务对象的基础模板类，支持共享内存环境下的
 *          对象创建、销毁、查找、迭代等操作。
 * 
 * @tparam ClassName 具体的类名，必须继承自该模板类
 * @tparam ClassType 类型标识符，用于在内存管理器中区分不同类型的对象
 * @tparam ParentClassName 父类类型，不能与ClassName相同
 * 
 * @note 该模板类提供了：
 *       - STL风格的迭代器接口
 *       - 对象的创建和销毁管理
 *       - 通过ID或Hash查找对象
 *       - 对象统计信息查询
 *       - 事务对象支持
 * 
 * @warning 使用时必须确保ClassType在系统中唯一，且ParentClassName不能与ClassName相同
 */
template <typename ClassName, int ClassType, typename ParentClassName>
class NFObjectTemplate : public ParentClassName
{
	static_assert(!std::is_same<ParentClassName, ClassName>::value, "parentClassName is equal to the className");
public:
	typedef NFObjectIterator<ClassName, ClassName&, ClassName*, NFIMemMngModule> iterator;           ///< 普通迭代器类型
	typedef NFObjectIterator<ClassName, const ClassName&, const ClassName*, NFIMemMngModule> const_iterator; ///< 常量迭代器类型
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;                          ///< 常量反向迭代器类型
	typedef std::reverse_iterator<iterator> reverse_iterator;                                       ///< 反向迭代器类型
public:
	/**
	 * @brief 构造函数
	 * @details 根据共享内存管理器的创建模式自动选择初始化方式
	 */
	NFObjectTemplate()
	{
		if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
		{
			CreateInit();
		}
		else
		{
			ResumeInit();
		}
	}

	/**
	 * @brief 创建时初始化
	 * @details 第一次创建对象时调用的初始化函数，派生类可以重写此方法
	 * @return int 初始化结果，0表示成功
	 */
	int CreateInit()
	{
		return 0;
	}

	/**
	 * @brief 恢复时初始化
	 * @details 从共享内存恢复对象时调用的初始化函数，派生类可以重写此方法
	 * @return int 初始化结果，0表示成功
	 */
	int ResumeInit()
	{
		return 0;
	}

	/**
	 * @brief 虚析构函数
	 */
	virtual ~NFObjectTemplate()
	{
	}

	/**
	 * @brief 获取当前类型的对象总数
	 * @return int 对象总数（包括已使用和空闲的）
	 */
	static int GetStaticItemCount()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetItemCount(ClassType);
	}

	/**
	 * @brief 获取当前类型已使用的对象数量
	 * @return int 已使用的对象数量
	 */
	static int GetStaticUsedCount()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetUsedCount(ClassType);
	}

	/**
	 * @brief 获取当前类型空闲的对象数量
	 * @return int 空闲的对象数量
	 */
	static int GetStaticFreeCount()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetFreeCount(ClassType);
	}

	/**
	 * @brief 获取类名字符串
	 * @return std::string 类名的字符串表示
	 */
	static std::string GetStaticClassName()
	{
		return typeid(ClassName).name();
	}

	/**
	 * @brief 检查指定索引是否为结束位置
	 * @param iIndex 要检查的索引
	 * @return bool true表示已到达结束位置
	 */
	static bool IsEnd(int iIndex)
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->IsEnd(ClassType, iIndex);
	}

	/**
	 * @brief 获取静态类型标识符
	 * @return int 类型标识符
	 */
	static int GetStaticClassType()
	{
		return ClassType;
	}

	/**
	 * @brief 获取迭代器起始位置
	 * @return iterator 指向第一个对象的迭代器
	 */
	static iterator Begin()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->IterBegin(ClassType);
	}

	/**
	 * @brief 获取迭代器结束位置
	 * @return iterator 指向最后一个对象之后位置的迭代器
	 */
	static iterator End()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->IterEnd(ClassType);
	}

	/**
	 * @brief 获取反向迭代器起始位置
	 * @return reverse_iterator 指向最后一个对象的反向迭代器
	 */
	static reverse_iterator RBegin()
	{
		return reverse_iterator(End());
	}

	/**
	 * @brief 获取反向迭代器结束位置
	 * @return reverse_iterator 指向第一个对象之前位置的反向迭代器
	 */
	static reverse_iterator REnd()
	{
		return reverse_iterator(Begin());
	}

	/**
	 * @brief 删除迭代器指向的对象
	 * @param iter 指向要删除对象的迭代器
	 * @return iterator 指向下一个有效对象的迭代器
	 */
	static iterator Erase(iterator iter)
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->Erase(iter);
	}

	/**
	 * @brief 检查迭代器是否有效
	 * @param iter 要检查的迭代器
	 * @return bool true表示迭代器有效
	 */
	static bool IsValid(iterator iter)
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->IsValid(iter);
	}

	/**
	 * @brief 创建新对象
	 * @return ClassName* 新创建的对象指针，失败返回nullptr
	 */
	static ClassName* CreateObj()
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->CreateObj(ClassType));
	}

	/**
	 * @brief 根据Hash键创建对象
	 * @param hashKey Hash键值
	 * @return ClassName* 创建的对象指针，失败返回nullptr
	 */
	static ClassName* CreateObjByHashKey(NFObjectHashKey hashKey)
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->CreateObjByHashKey(ClassType, hashKey));
	}

	/**
	 * @brief 创建事务对象
	 * @details 只有继承自NFTransBase的类才能调用此方法
	 * @return ClassName* 创建的事务对象指针，失败返回nullptr
	 */
	static ClassName* CreateTrans()
	{
		static_assert(std::is_base_of<NFTransBase, ClassName>::value, "className is not inherit to NFTransBase");
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->CreateTrans(ClassType));
	}

	/**
	 * @brief 根据事务ID获取事务对象
	 * @param ullTransId 事务ID
	 * @return ClassName* 事务对象指针，失败返回nullptr
	 */
	static ClassName* GetTrans(uint64_t ullTransId)
	{
		static_assert(std::is_base_of<NFTransBase, ClassName>::value, "className is not inherit to NFTransBase");
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetTrans(ullTransId));
	}

	/**
	 * @brief 销毁对象
	 * @param pObj 要销毁的对象指针
	 */
	static void DestroyObj(ClassName* pObj)
	{
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->DestroyObj(pObj);
	}

	/**
	 * @brief 清除所有对象
	 * @details 销毁当前类型的所有对象实例
	 */
	static void ClearAllObj()
	{
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->ClearAllObj(ClassType);
	}

	/**
	 * @brief 根据对象ID获取对象
	 * @param iObjId 对象ID
	 * @return ClassName* 对象指针，失败返回nullptr
	 */
	static ClassName* GetObjByObjId(int iObjId)
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetObjByObjId(ClassType, iObjId));
	}

	/**
	 * @brief 根据全局ID获取对象
	 * @param iGlobalId 全局ID
	 * @param withChildrenType 是否包含子类型，默认为false
	 * @return ClassName* 对象指针，失败返回nullptr
	 */
	static ClassName* GetObjByGlobalId(int iGlobalId, bool withChildrenType = false)
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetObjByGlobalId(ClassType, iGlobalId, withChildrenType));
	}

	/**
	 * @brief 根据Hash键获取对象
	 * @param hashKey Hash键值
	 * @return ClassName* 对象指针，失败返回nullptr
	 */
	static ClassName* GetObjByHashKey(NFObjectHashKey hashKey)
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetObjByHashKey(ClassType, hashKey));
	}

	/**
	 * @brief 自动删除对象
	 * @details 根据指定条件自动删除一定数量的对象
	 * @param maxNum 最大删除数量，默认为INVALID_ID（无限制）
	 * @param func 删除条件判断函数，默认为nullptr
	 * @return int 实际删除的对象数量
	 */
	static int DestroyObjAutoErase(int maxNum = INVALID_ID, const DESTROY_OBJECT_AUTO_ERASE_FUNCTION& func = nullptr)
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->DestroyObjAutoErase(ClassType, maxNum, func);
	}

	/**
	 * @brief 注册类到对象段的映射
	 * @param bType 对象类型ID
	 * @param siObjSize 对象大小
	 * @param iObjCount 对象数量
	 * @param strClassName 类名字符串
	 * @param useHash 是否使用Hash索引
	 * @param singleton 是否为单例，默认为false
	 * @return int 注册结果，0表示成功
	 */
	static int RegisterClassToObjSeg(int bType, size_t siObjSize, int iObjCount, const std::string& strClassName, bool useHash, bool singleton = false)
	{
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->RegisterClassToObjSeg(bType, siObjSize, iObjCount, ClassName::ResumeObjectRegisterFunction,
		                                                                                                           ClassName::CreateObjectRegisterFunction, ClassName::DestroyObjectRegisterFunction, ParentClassName::GetStaticClassType(),
		                                                                                                           strClassName, useHash, singleton);
		return 0;
	}

	/**
	 * @brief 反注册对象类型到内存段的映射
	 * @param bType 对象类型ID
	 * @return int 总是返回0
	 * @note 用于从内存管理器中移除对象类型的注册信息
	 */
	static int UnRegisterClassToObjSeg(int bType)
	{
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->UnRegisterClassToObjSeg(bType);
		return 0;
	}

	/**
	 * @brief 重载new操作符，用于在指定内存位置构造对象
	 * @param size 对象大小（未使用）
	 * @param pBuffer 内存缓冲区指针
	 * @return void* 返回传入的缓冲区指针
	 */
	void* operator new(size_t, void* pBuffer) throw()
	{
		return pBuffer;
	}

	/**
	 * @brief 对象创建注册函数
	 * @details 内存管理器用于创建新对象的回调函数
	 * @return NFObject* 创建的对象指针，失败返回nullptr
	 */
	static NFObject* CreateObjectRegisterFunction()
	{
		ClassName* pTmp = nullptr;
		void* pVoid = NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->AllocMemForObject(ClassType);
		if (!pVoid)
		{
			NFLogError(NF_LOG_DEFAULT, 0, "ERROR: class:{}, Item:{}, Used:{}, Free:{}", GetStaticClassName(), GetStaticItemCount(), GetStaticUsedCount(), GetStaticFreeCount());
			return nullptr;
		}
		pTmp = new(pVoid) ClassName();
		return pTmp;
	}

	/**
	 * @brief 对象恢复注册函数
	 * @details 内存管理器用于从共享内存恢复对象的回调函数
	 * @param pVoid 对象内存地址
	 * @return NFObject* 恢复的对象指针
	 */
	static NFObject* ResumeObjectRegisterFunction(void* pVoid)
	{
		ClassName* pTmp = nullptr;
		pTmp = new(pVoid) ClassName();
		return pTmp;
	}

	/**
	 * @brief 对象销毁注册函数
	 * @details 内存管理器用于销毁对象的回调函数
	 * @param pObj 要销毁的对象指针
	 */
	static void DestroyObjectRegisterFunction(NFObject* pObj)
	{
		ClassName* pTmp = nullptr;
		pTmp = dynamic_cast<ClassName*>(pObj);
		(*pTmp).~ClassName();
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->FreeMemForObject(ClassType, pTmp);
	}

public:
	/**
	 * @brief 获取对象的类型标识符
	 * @return int 类型标识符
	 */
	virtual int GetClassType() const
	{
		return ClassType;
	}
};

/**
 * @class NFObjectTemplateNoParent
 * @brief 无父类的对象模板类
 * @details 与NFObjectTemplate类似，但不继承任何父类，适用于作为继承层次根节点的对象。
 *          提供了完整的对象生命周期管理和迭代器支持功能。
 * 
 * @tparam ClassName 具体的类名，必须继承自该模板类
 * @tparam ClassType 类型标识符，用于在内存管理器中区分不同类型的对象
 * 
 * @note 该模板类适用于不需要继承其他类的根对象
 */
template <typename ClassName, int ClassType>
class NFObjectTemplateNoParent
{
public:
	typedef NFObjectIterator<ClassName, ClassName&, ClassName*, NFIMemMngModule> iterator;           ///< 普通迭代器类型
	typedef NFObjectIterator<ClassName, const ClassName&, const ClassName*, NFIMemMngModule> const_iterator; ///< 常量迭代器类型
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;                          ///< 常量反向迭代器类型
	typedef std::reverse_iterator<iterator> reverse_iterator;                                       ///< 反向迭代器类型
public:
	/**
	 * @brief 构造函数
	 * @details 根据共享内存管理器的创建模式自动选择初始化方式
	 */
	NFObjectTemplateNoParent()
	{
		if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
		{
			CreateInit();
		}
		else
		{
			ResumeInit();
		}
	}

	/**
	 * @brief 创建时初始化
	 * @return int 初始化结果，0表示成功
	 */
	int CreateInit()
	{
		return 0;
	}

	/**
	 * @brief 恢复时初始化
	 * @return int 初始化结果，0表示成功
	 */
	int ResumeInit()
	{
		return 0;
	}

	/**
	 * @brief 虚析构函数
	 */
	virtual ~NFObjectTemplateNoParent()
	{
	}

public:
	/**
	 * @brief 获取当前类型的对象总数
	 * @return int 对象总数
	 */
	static int GetStaticItemCount()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetItemCount(ClassType);
	}

	/**
	 * @brief 获取当前类型已使用的对象数量
	 * @return int 已使用的对象数量
	 */
	static int GetStaticUsedCount()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetUsedCount(ClassType);
	}

	/**
	 * @brief 获取当前类型空闲的对象数量
	 * @return int 空闲的对象数量
	 */
	static int GetStaticFreeCount()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetFreeCount(ClassType);
	}

	/**
	 * @brief 检查指定索引是否为结束位置
	 * @param iIndex 要检查的索引
	 * @return bool true表示已到达结束位置
	 */
	static bool IsEnd(int iIndex)
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->IsEnd(ClassType, iIndex);
	}

	/**
	 * @brief 获取类名字符串
	 * @return std::string 类名的字符串表示
	 */
	static std::string GetStaticClassName()
	{
		return typeid(ClassName).name();
	}

	/**
	 * @brief 获取静态类型标识符
	 * @return int 类型标识符
	 */
	static int GetStaticClassType()
	{
		return ClassType;
	}

	/**
	 * @brief 获取迭代器起始位置
	 * @return iterator 指向第一个对象的迭代器
	 */
	static iterator Begin()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->IterBegin(ClassType);
	}

	/**
	 * @brief 获取迭代器结束位置
	 * @return iterator 指向最后一个对象之后位置的迭代器
	 */
	static iterator End()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->IterEnd(ClassType);
	}

	/**
	 * @brief 获取反向迭代器起始位置
	 * @return reverse_iterator 指向最后一个对象的反向迭代器
	 */
	static reverse_iterator RBegin()
	{
		return reverse_iterator(End());
	}

	/**
	 * @brief 获取反向迭代器结束位置
	 * @return reverse_iterator 指向第一个对象之前位置的反向迭代器
	 */
	static reverse_iterator REnd()
	{
		return reverse_iterator(Begin());
	}

	/**
	 * @brief 删除迭代器指向的对象
	 * @param iter 指向要删除对象的迭代器
	 * @return iterator 指向下一个有效对象的迭代器
	 */
	static iterator Erase(iterator iter)
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->Erase(iter);
	}

	/**
	 * @brief 检查迭代器是否有效
	 * @param iter 要检查的迭代器
	 * @return bool true表示迭代器有效
	 */
	static bool IsValid(iterator iter)
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->IsValid(iter);
	}

	/**
	 * @brief 创建新对象
	 * @return ClassName* 新创建的对象指针，失败返回nullptr
	 */
	static ClassName* CreateObj()
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->CreateObj(ClassType));
	}

	/**
	 * @brief 根据Hash键创建对象
	 * @param hashKey Hash键值
	 * @return ClassName* 创建的对象指针，失败返回nullptr
	 */
	static ClassName* CreateObjByHashKey(NFObjectHashKey hashKey)
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->CreateObjByHashKey(ClassType, hashKey));
	}

	/**
	 * @brief 销毁对象
	 * @param pObj 要销毁的对象指针
	 */
	static void DestroyObj(ClassName* pObj)
	{
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->DestroyObj(pObj);
	}

	/**
	 * @brief 清除所有对象
	 * @details 销毁当前类型的所有对象实例
	 */
	static void ClearAllObj()
	{
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->ClearAllObj(ClassType);
	}

	/**
	 * @brief 根据对象ID获取对象
	 * @param iObjId 对象ID
	 * @return ClassName* 对象指针，失败返回nullptr
	 */
	static ClassName* GetObjByObjId(int iObjId)
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetObjByObjId(ClassType, iObjId));
	}

	/**
	 * @brief 根据全局ID获取对象
	 * @param iGlobalId 全局ID
	 * @param withChildrenType 是否包含子类型，默认为false
	 * @return ClassName* 对象指针，失败返回nullptr
	 */
	static ClassName* GetObjByGlobalId(int iGlobalId, bool withChildrenType = false)
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetObjByGlobalId(ClassType, iGlobalId, withChildrenType));
	}

	/**
	 * @brief 根据Hash键获取对象
	 * @param hashKey Hash键值
	 * @return ClassName* 对象指针，失败返回nullptr
	 */
	static ClassName* GetObjByHashKey(NFObjectHashKey hashKey)
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetObjByHashKey(ClassType, hashKey));
	}

	/**
	 * @brief 创建事务对象
	 * @details 只有继承自NFTransBase的类才能调用此方法
	 * @return ClassName* 创建的事务对象指针，失败返回nullptr
	 */
	static ClassName* CreateTrans()
	{
		static_assert(std::is_base_of<NFTransBase, ClassName>::value, "className is not inherit to NFTransBase");
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->CreateTrans(ClassType));
	}

	/**
	 * @brief 根据事务ID获取事务对象
	 * @param ullTransId 事务ID
	 * @return ClassName* 事务对象指针，失败返回nullptr
	 */
	static ClassName* GetTrans(uint64_t ullTransId)
	{
		static_assert(std::is_base_of<NFTransBase, ClassName>::value, "className is not inherit to NFTransBase");
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetTrans(ullTransId));
	}

	/**
	 * @brief 自动删除对象
	 * @param maxNum 最大删除数量，默认为INVALID_ID（无限制）
	 * @param func 删除条件判断函数，默认为nullptr
	 * @return int 实际删除的对象数量
	 */
	static int DestroyObjAutoErase(int maxNum = INVALID_ID, const DESTROY_OBJECT_AUTO_ERASE_FUNCTION& func = nullptr)
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->DestroyObjAutoErase(ClassType, maxNum, func);
	}

	/**
	 * @brief 注册类到对象段的映射
	 * @param bType 对象类型ID
	 * @param siObjSize 对象大小
	 * @param iObjCount 对象数量
	 * @param strClassName 类名字符串
	 * @param useHash 是否使用Hash索引
	 * @param singleton 是否为单例，默认为false
	 * @return int 注册结果，0表示成功
	 */
	static int RegisterClassToObjSeg(int bType, size_t siObjSize, int iObjCount, const std::string& strClassName, bool useHash, bool singleton = false)
	{
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->RegisterClassToObjSeg(bType, siObjSize, iObjCount, ClassName::ResumeObjectRegisterFunction,
		                                                                                                           ClassName::CreateObjectRegisterFunction, ClassName::DestroyObjectRegisterFunction, -1,
		                                                                                                           strClassName, useHash, singleton);
		return 0;
	}

	/**
	 * @brief 反注册对象类型到内存段的映射
	 * @param bType 对象类型ID
	 * @return int 总是返回0
	 * @note 用于从内存管理器中移除对象类型的注册信息
	 */
	static int UnRegisterClassToObjSeg(int bType)
	{
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->UnRegisterClassToObjSeg(bType);
		return 0;
	}

	/**
	 * @brief 重载new操作符，用于在指定内存位置构造对象
	 * @param size 对象大小（未使用）
	 * @param pBuffer 内存缓冲区指针
	 * @return void* 返回传入的缓冲区指针
	 */
	void* operator new(size_t, void* pBuffer) throw()
	{
		return pBuffer;
	}

	/**
	 * @brief 对象创建注册函数
	 * @details 内存管理器用于创建新对象的回调函数
	 * @return NFObject* 创建的对象指针，失败返回nullptr
	 */
	static NFObject* CreateObjectRegisterFunction()
	{
		ClassName* pTmp  = nullptr;
		void* pVoid = NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->AllocMemForObject(ClassType);
		if (!pVoid)
		{
			NFLogError(NF_LOG_DEFAULT, 0, "ERROR: class:{}, Item:{}, Used:{}, Free:{}", GetStaticClassName(), GetStaticItemCount(), GetStaticUsedCount(), GetStaticFreeCount());
			return nullptr;
		}
		pTmp = new(pVoid) ClassName();
		return pTmp;
	}

	/**
	 * @brief 对象恢复注册函数
	 * @details 内存管理器用于从共享内存恢复对象的回调函数
	 * @param pVoid 对象内存地址
	 * @return NFObject* 恢复的对象指针
	 */
	static NFObject* ResumeObjectRegisterFunction(void* pVoid)
	{
		ClassName* pTmp = nullptr;
		pTmp = new(pVoid) ClassName();
		return pTmp;
	}

	/**
	 * @brief 对象销毁注册函数
	 * @details 内存管理器用于销毁对象的回调函数
	 * @param pObj 要销毁的对象指针
	 */
	static void DestroyObjectRegisterFunction(NFObject* pObj)
	{
		ClassName* pTmp = nullptr;
		pTmp = dynamic_cast<ClassName*>(pObj);
		(*pTmp).~ClassName();
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->FreeMemForObject(ClassType, pTmp);
	}

public:
	/**
	 * @brief 获取对象的类型标识符
	 * @return int 类型标识符
	 */
	virtual int GetClassType() const
	{
		return ClassType;
	}
};

/**
 * @class NFObjectGlobalTemplate
 * @brief 全局对象模板类
 * @details 继承自NFObjectTemplate，专门用于全局单例对象的管理。
 *          提供了消息处理、RPC调用等网络通信相关的功能。
 *          适用于服务器中需要处理网络消息的全局管理器对象。
 * 
 * @tparam ClassName 具体的类名，必须继承自该模板类
 * @tparam ClassType 类型标识符，用于在内存管理器中区分不同类型的对象
 * @tparam ParentClassName 父类类型，不能与ClassName相同
 * 
 * @note 该模板类特别适用于：
 *       - 游戏服务器的管理器对象
 *       - 需要处理客户端和服务器消息的单例对象
 *       - 需要支持RPC调用的全局对象
 */
template <typename ClassName, int ClassType, typename ParentClassName>
class NFObjectGlobalTemplate : public ParentClassName
{
public:
	typedef NFObjectIterator<ClassName, ClassName &, ClassName*, NFIMemMngModule> iterator;         ///< 普通迭代器类型
	typedef NFObjectIterator<ClassName, const ClassName&, const ClassName*, NFIMemMngModule> const_iterator; ///< 常量迭代器类型
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;                          ///< 常量反向迭代器类型
	typedef std::reverse_iterator<iterator> reverse_iterator;                                       ///< 反向迭代器类型
public:
	/**
	 * @brief 构造函数
	 * @details 根据共享内存管理器的创建模式自动选择初始化方式
	 */
	NFObjectGlobalTemplate()
	{
		if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode())
		{
			CreateInit();
		}
		else
		{
			ResumeInit();
		}
	}

	/**
	 * @brief 创建时初始化
	 * @return int 初始化结果，0表示成功
	 */
	int CreateInit()
	{
		return 0;
	}

	/**
	 * @brief 恢复时初始化
	 * @return int 初始化结果，0表示成功
	 */
	int ResumeInit()
	{
		return 0;
	}

	/**
	 * @brief 虚析构函数
	 */
	virtual ~NFObjectGlobalTemplate()
	{
	}

public:
	/**
	 * @brief 获取当前类型的对象总数
	 * @return int 对象总数
	 */
	static int GetStaticItemCount()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetItemCount(ClassType);
	}

	/**
	 * @brief 获取当前类型已使用的对象数量
	 * @return int 已使用的对象数量
	 */
	static int GetStaticUsedCount()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetUsedCount(ClassType);
	}

	/**
	 * @brief 获取当前类型空闲的对象数量
	 * @return int 空闲的对象数量
	 */
	static int GetStaticFreeCount()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetFreeCount(ClassType);
	}

	/**
	 * @brief 检查指定索引是否为结束位置
	 * @param iIndex 要检查的索引
	 * @return bool true表示已到达结束位置
	 */
	static bool IsEnd(int iIndex)
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->IsEnd(ClassType,iIndex);
	}

	/**
	 * @brief 获取类名字符串
	 * @return std::string 类名的字符串表示
	 */
	static std::string GetStaticClassName()
	{
		return typeid(ClassName).name();
	}

	/**
	 * @brief 获取静态类型标识符
	 * @return int 类型标识符
	 */
	static int GetStaticClassType()
	{
		return ClassType;
	}

	/**
	 * @brief 获取迭代器起始位置
	 * @return iterator 指向第一个对象的迭代器
	 */
	static iterator Begin()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->IterBegin(ClassType);
	}

	/**
	 * @brief 获取迭代器结束位置
	 * @return iterator 指向最后一个对象之后位置的迭代器
	 */
	static iterator End()
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->IterEnd(ClassType);
	}

	/**
	 * @brief 获取反向迭代器起始位置
	 * @return reverse_iterator 指向最后一个对象的反向迭代器
	 */
	static reverse_iterator RBegin()
	{
		return reverse_iterator(End());
	}

	/**
	 * @brief 获取反向迭代器结束位置
	 * @return reverse_iterator 指向第一个对象之前位置的反向迭代器
	 */
	static reverse_iterator REnd()
	{
		return reverse_iterator(Begin());
	}

	/**
	 * @brief 删除迭代器指向的对象
	 * @param iter 指向要删除对象的迭代器
	 * @return iterator 指向下一个有效对象的迭代器
	 */
	static iterator Erase(iterator iter)
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->Erase(iter);
	}

	/**
	 * @brief 检查迭代器是否有效
	 * @param iter 要检查的迭代器
	 * @return bool true表示迭代器有效
	 */
	static bool IsValid(iterator iter)
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->IsValid(iter);
	}

	/**
	 * @brief 创建新对象
	 * @return ClassName* 新创建的对象指针，失败返回nullptr
	 */
	static ClassName* CreateObj()
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->CreateObj(ClassType));
	}

	/**
	 * @brief 根据Hash键创建对象
	 * @param hashKey Hash键值
	 * @return ClassName* 创建的对象指针，失败返回nullptr
	 */
	static ClassName* CreateObjByHashKey(NFObjectHashKey hashKey)
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->CreateObjByHashKey(ClassType, hashKey));
	}

	/**
	 * @brief 创建事务对象
	 * @details 只有继承自NFTransBase的类才能调用此方法
	 * @return ClassName* 创建的事务对象指针，失败返回nullptr
	 */
	static ClassName* CreateTrans()
	{
		static_assert(std::is_base_of<NFTransBase, ClassName>::value, "className is not inherit to NFTransBase");
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->CreateTrans(ClassType));
	}

	/**
	 * @brief 根据事务ID获取事务对象
	 * @param ullTransId 事务ID
	 * @return ClassName* 事务对象指针，失败返回nullptr
	 */
	static ClassName* GetTrans(uint64_t ullTransId)
	{
		static_assert(std::is_base_of<NFTransBase, ClassName>::value, "className is not inherit to NFTransBase");
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetTrans(ullTransId));
	}

	/**
	 * @brief 销毁对象
	 * @param pObj 要销毁的对象指针
	 */
	static void DestroyObj(ClassName* pObj)
	{
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->DestroyObj(pObj);
	}

	/**
	 * @brief 清除所有对象
	 * @details 销毁当前类型的所有对象实例
	 */
	static void ClearAllObj()
	{
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->ClearAllObj(ClassType);
	}

	/**
	 * @brief 根据对象ID获取对象
	 * @param iObjId 对象ID
	 * @return ClassName* 对象指针，失败返回nullptr
	 */
	static ClassName* GetObjByObjId(int iObjId)
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetObjByObjId(ClassType, iObjId));
	}

	/**
	 * @brief 根据全局ID获取对象
	 * @param iGlobalId 全局ID
	 * @param withChildrenType 是否包含子类型，默认为false
	 * @return ClassName* 对象指针，失败返回nullptr
	 */
	static ClassName* GetObjByGlobalId(int iGlobalId, bool withChildrenType = false)
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetObjByGlobalId(ClassType, iGlobalId, withChildrenType));
	}

	/**
	 * @brief 根据Hash键获取对象
	 * @param hashKey Hash键值
	 * @return ClassName* 对象指针，失败返回nullptr
	 */
	static ClassName* GetObjByHashKey(NFObjectHashKey hashKey)
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetObjByHashKey(ClassType, hashKey));
	}

	/**
	 * @brief 自动删除对象
	 * @param maxNum 最大删除数量，默认为INVALID_ID（无限制）
	 * @param func 删除条件判断函数，默认为nullptr
	 * @return int 实际删除的对象数量
	 */
	static int DestroyObjAutoErase(int maxNum = INVALID_ID, const DESTROY_OBJECT_AUTO_ERASE_FUNCTION& func = nullptr)
	{
		return NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->DestroyObjAutoErase(ClassType, maxNum, func);
	}

	/**
	 * @brief 获取单例实例（方法1）
	 * @return ClassName* 单例对象指针
	 */
	static ClassName* Instance()
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetHeadObj(ClassType));
	}

	/**
	 * @brief 获取单例实例（方法2）
	 * @return ClassName* 单例对象指针
	 */
	static ClassName* GetInstance()
	{
		return dynamic_cast<ClassName*>(NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->GetHeadObj(ClassType));
	}

	/**
	 * @brief 注册类到对象段的映射
	 * @param bType 对象类型ID
	 * @param siObjSize 对象大小
	 * @param iObjCount 对象数量
	 * @param strClassName 类名字符串
	 * @param useHash 是否使用Hash索引
	 * @param singleton 是否为单例，默认为false
	 * @return int 注册结果，0表示成功
	 */
	static int RegisterClassToObjSeg(int bType, size_t siObjSize, int iObjCount, const std::string& strClassName, bool useHash, bool singleton = false)
	{
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->RegisterClassToObjSeg(bType, siObjSize, iObjCount, ClassName::ResumeObjectRegisterFunction,
		                                                                                                           ClassName::CreateObjectRegisterFunction, ClassName::DestroyObjectRegisterFunction, ParentClassName::GetStaticClassType(),
		                                                                                                           strClassName, useHash, singleton);
		return 0;
	}

	/**
	 * @brief 反注册对象类型到内存段的映射
	 * @param bType 对象类型ID
	 * @return int 总是返回0
	 * @note 用于从内存管理器中移除对象类型的注册信息
	 */
	static int UnRegisterClassToObjSeg(int bType)
	{
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->UnRegisterClassToObjSeg(bType);
		return 0;
	}

	/**
	 * @brief 重载new操作符，用于在指定内存位置构造对象
	 * @param size 对象大小（未使用）
	 * @param pBuffer 内存缓冲区指针
	 * @return void* 返回传入的缓冲区指针
	 */
	void* operator new(size_t, void* pBuffer) throw()
	{
		return pBuffer;
	}

	/**
	 * @brief 对象创建注册函数
	 * @details 内存管理器用于创建新对象的回调函数
	 * @return NFObject* 创建的对象指针，失败返回nullptr
	 */
	static NFObject* CreateObjectRegisterFunction()
	{
		ClassName* pTmp = nullptr;
		void* pVoid = NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->AllocMemForObject(ClassType);
		if (!pVoid)
		{
			NFLogError(NF_LOG_DEFAULT, 0, "ERROR: class:{}, Item:{}, Used:{}, Free:{}", GetStaticClassName(), GetStaticItemCount(), GetStaticUsedCount(), GetStaticFreeCount());
			return nullptr;
		}
		pTmp = new(pVoid) ClassName();
		return pTmp;
	}

	/**
	 * @brief 对象恢复注册函数
	 * @details 内存管理器用于从共享内存恢复对象的回调函数
	 * @param pVoid 对象内存地址
	 * @return NFObject* 恢复的对象指针
	 */
	static NFObject* ResumeObjectRegisterFunction(void* pVoid)
	{
		ClassName* pTmp = nullptr;
		pTmp = new(pVoid) ClassName();
		return pTmp;
	}

	/**
	 * @brief 对象销毁注册函数
	 * @details 内存管理器用于销毁对象的回调函数
	 * @param pObj 要销毁的对象指针
	 */
	static void DestroyObjectRegisterFunction(NFObject* pObj)
	{
		ClassName* pTmp = nullptr;
		pTmp = dynamic_cast<ClassName*>(pObj);
		(*pTmp).~ClassName();
		NFGlobalSystem::Instance()->GetGlobalPluginManager()->FindModule<NFIMemMngModule>()->FreeMemForObject(ClassType, pTmp);
	}
	
public:
	/**
	 * @brief 添加RPC服务
	 * @tparam msgId 消息ID
	 * @tparam RequestType 请求消息类型
	 * @tparam ResponeType 响应消息类型
	 * @param serverType 服务器类型
	 * @param createCo 是否创建协程，默认为false
	 * @return bool true表示注册成功
	 */
	template<size_t msgId, typename RequestType, typename ResponeType>
	bool AddRpcService(NF_SERVER_TYPE serverType, bool createCo = false)
	{
		return FindModule<NFIMessageModule>()->AddRpcService<msgId, RequestType, ResponeType>(serverType, &ClassName::StaticOnHandleRpcMessage, createCo);
	}

	/**
	 * @brief 注册客户端消息处理函数
	 * @param eType 服务器类型
	 * @param nMsgID 消息ID
	 * @param createCo 是否创建协程，默认为false
	 * @return bool true表示注册成功
	 */
	static bool RegisterClientMessage(NF_SERVER_TYPE eType, uint32_t nMsgID, bool createCo = false)
	{
		NET_RECEIVE_FUNCTOR functor = std::bind((int(*)(uint64_t, NFDataPackage&))(&ClassName::StaticOnHandleClientMessage), std::placeholders::_1, std::placeholders::_2);
		return FindModule<NFIMessageModule>()->AddMessageCallBack(eType, NF_MODULE_CLIENT, nMsgID, functor, createCo);
	}

	/**
	 * @brief 注册客户端消息处理函数（带模块ID）
	 * @param eType 服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param createCo 是否创建协程，默认为false
	 * @return bool true表示注册成功
	 */
	static bool RegisterClientMessageWithModule(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgID, bool createCo = false)
	{
		NET_RECEIVE_FUNCTOR functor = std::bind((int(*)(uint64_t, NFDataPackage&))(&ClassName::StaticOnHandleClientMessage), std::placeholders::_1, std::placeholders::_2);
		return FindModule<NFIMessageModule>()->AddMessageCallBack(eType, nModuleId, nMsgID, functor, createCo);
	}

	/**
	 * @brief 注册服务器消息处理函数
	 * @param eType 服务器类型
	 * @param nMsgID 消息ID
	 * @param createCo 是否创建协程，默认为false
	 * @return bool true表示注册成功
	 */
	static bool RegisterServerMessage(NF_SERVER_TYPE eType, uint32_t nMsgID, bool createCo = false)
	{
		NET_RECEIVE_FUNCTOR functor = std::bind((int(*)(uint64_t, NFDataPackage&))(&ClassName::StaticOnHandleServerMessage), std::placeholders::_1, std::placeholders::_2);
		return FindModule<NFIMessageModule>()->AddMessageCallBack(eType, NF_MODULE_SERVER, nMsgID, functor, createCo);
	}

	/**
	 * @brief 注册服务器消息处理函数（带模块ID）
	 * @param eType 服务器类型
	 * @param nModuleId 模块ID
	 * @param nMsgID 消息ID
	 * @param createCo 是否创建协程，默认为false
	 * @return bool true表示注册成功
	 */
	virtual bool RegisterServerMessageWithModule(NF_SERVER_TYPE eType, uint32_t nModuleId, uint32_t nMsgID, bool createCo = false)
	{
		NET_RECEIVE_FUNCTOR functor = std::bind((int(*)(uint64_t, NFDataPackage&))(&ClassName::StaticOnHandleServerMessage), std::placeholders::_1, std::placeholders::_2);
		return FindModule<NFIMessageModule>()->AddMessageCallBack(eType, nModuleId, nMsgID, functor, createCo);
	}

	/**
	 * @brief 静态客户端消息处理函数
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return int 处理结果
	 */
	static int StaticOnHandleClientMessage(uint64_t unLinkId, NFDataPackage& packet)
	{
		return ClassName::Instance()->OnHandleClientMessage(unLinkId, packet);
	}

	/**
	 * @brief 静态客户端消息处理函数（重载版本）
	 * @param msgId 消息ID
	 * @param packet 数据包
	 * @param param1 参数1
	 * @param param2 参数2
	 * @return int 处理结果
	 */
	static int StaticOnHandleClientMessage(uint32_t msgId, NFDataPackage& packet, uint64_t param1, uint64_t param2)
	{
		return ClassName::Instance()->OnHandleClientMessage(msgId, packet, param1, param2);
	}

	/**
	 * @brief 静态服务器消息处理函数
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return int 处理结果
	 */
	static int StaticOnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet)
	{
		return ClassName::Instance()->OnHandleServerMessage(unLinkId, packet);
	}

	/**
	 * @brief 静态服务器消息处理函数（重载版本）
	 * @param msgId 消息ID
	 * @param packet 数据包
	 * @param param1 参数1
	 * @param param2 参数2
	 * @return int 处理结果
	 */
	static int StaticOnHandleServerMessage(uint32_t msgId, NFDataPackage& packet, uint64_t param1, uint64_t param2)
	{
		return ClassName::Instance()->OnHandleServerMessage(msgId, packet, param1, param2);
	}

	/**
	 * @brief 静态RPC消息处理函数
	 * @details 处理服务器之间的RPC调用，负责转发玩家part的RPC
	 * @param msgId 消息ID
	 * @param request 请求消息
	 * @param respone 响应消息
	 * @param param1 参数1
	 * @param param2 参数2
	 * @return int 处理结果
	 */
	static int StaticOnHandleRpcMessage(uint32_t msgId, google::protobuf::Message& request, google::protobuf::Message& respone, uint64_t param1, uint64_t param2)
	{
		return ClassName::Instance()->OnHandleRpcMessage(msgId, request, respone, param1, param2);
	}
	
public:
	/**
	 * @brief 处理客户端消息
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return int 处理结果
	 */
	virtual int OnHandleClientMessage(uint64_t unLinkId, NFDataPackage& packet)
	{
		return OnHandleClientMessage(packet.nMsgId, packet, packet.nParam1, packet.nParam2);
	}

	/**
	 * @brief 处理客户端消息（重载版本）
	 * @param msgId 消息ID
	 * @param packet 数据包
	 * @param param1 参数1
	 * @param param2 参数2
	 * @return int 处理结果
	 */
	virtual int OnHandleClientMessage(uint32_t msgId, NFDataPackage& packet, uint64_t param1, uint64_t param2)
	{
		NFLogError(NF_LOG_DEFAULT, 0, "client msg:({}) not handle", packet.ToString());
		return 0;
	}

	/**
	 * @brief 处理来自服务器的消息
	 * @param unLinkId 连接ID
	 * @param packet 数据包
	 * @return int 处理结果
	 */
	virtual int OnHandleServerMessage(uint64_t unLinkId, NFDataPackage& packet)
	{
		return OnHandleServerMessage(packet.nMsgId, packet, packet.nParam1, packet.nParam2);
	}

	/**
	 * @brief 处理来自服务器的消息（重载版本）
	 * @param msgId 消息ID
	 * @param packet 数据包
	 * @param param1 参数1
	 * @param param2 参数2
	 * @return int 处理结果
	 */
	virtual int OnHandleServerMessage(uint32_t msgId, NFDataPackage& packet, uint64_t param1, uint64_t param2)
	{
		NFLogError(NF_LOG_DEFAULT, 0, "server msg:({}) not handle", packet.ToString());
		return 0;
	}

	/**
	 * @brief 处理RPC消息
	 * @details 处理服务器之间的RPC调用，负责转发玩家part的RPC
	 * @param msgId 消息ID
	 * @param request 请求消息
	 * @param respone 响应消息
	 * @param param1 参数1
	 * @param param2 参数2
	 * @return int 处理结果
	 */
	virtual int OnHandleRpcMessage(uint32_t msgId, google::protobuf::Message& request, google::protobuf::Message& respone, uint64_t param1, uint64_t param2)
	{
		NFLogError(NF_LOG_DEFAULT, 0, "server rpc msgId:{} param1:{} param2:{} not handle", msgId, param1, param2);
		return 0;
	}
	
public:
	/**
	 * @brief 获取对象的类型标识符
	 * @return int 类型标识符
	 */
	virtual int GetClassType() const
	{
		return ClassType;
	}
};
