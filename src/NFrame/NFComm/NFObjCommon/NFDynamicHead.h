/**
 * @file NFDynamicHead.h
 * @brief 动态对象头部定义文件
 * @details 定义了NFrame框架中动态对象的头部宏定义，提供了对象预分配、对象管理、
 *          迭代器支持等功能的标准接口声明。这些宏用于自动生成对象的静态管理方法，
 *          包括对象创建、销毁、查找、迭代等核心功能。
 * 
 * @author 系统生成
 * @date 未知
 * 
 * @note 该文件包含了：
 *       - _DECLARE_PREALLOCATED_: 预分配对象声明宏
 *       - 对象迭代器类型定义
 *       - 静态对象管理方法声明
 *       - 对象创建和销毁方法声明
 *       - 对象查找和遍历方法声明
 * 
 * @warning 使用这些宏时需要注意：
 *          - 不要直接调用CreateObjectRegisterFunction等系统函数
 *          - 这些是系统内部使用的注册函数，会导致崩溃
 *          - 应该使用提供的静态方法来管理对象
 *          - 确保在正确的共享内存环境中使用
 */

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"

class NFIMemMngModule;

/**
 * @brief 不要调用CreateObjectRegisterFunction，ResumeObjectRegisterFunction, DestroyObjectRegisterFunction 创建对象，会崩溃， 走系统创建函数
 */

#define _DECLARE_PREALLOCATED_(class_name)\
public:\
    typedef NFObjectIterator<class_name, class_name &, class_name *, NFIMemMngModule> iterator;\
    typedef NFObjectIterator<class_name, const class_name &, const class_name *, NFIMemMngModule> const_iterator;\
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;\
    typedef std::reverse_iterator<iterator> reverse_iterator;\
    static int   GetStaticItemCount(NFIPluginManager* pPluginManager);\
    static int   GetStaticUsedCount(NFIPluginManager* pPluginManager);\
    static int   GetStaticFreeCount(NFIPluginManager* pPluginManager);\
    static std::string GetStaticClassName() { return #class_name; }\
    static iterator Begin(NFIPluginManager* pPluginManager);\
    static iterator End(NFIPluginManager* pPluginManager);\
    static reverse_iterator RBegin(NFIPluginManager* pPluginManager);\
    static reverse_iterator REnd(NFIPluginManager* pPluginManager);\
    static iterator Erase(NFIPluginManager* pPluginManager, iterator iter);\
    static class_name* CreateObj(NFIPluginManager* pPluginManager);\
    static class_name* CreateObjByHashKey(NFIPluginManager* pPluginManager, NFObjectHashKey hashKey);\
    static void DestroyObj(NFIPluginManager* pPluginManager, class_name*);\
    static void ClearAllObj(NFIPluginManager* pPluginManager);\
    static class_name* GetObjByObjId(NFIPluginManager* pPluginManager, int iObjId);\
    static class_name *GetObjByGlobalId(NFIPluginManager* pPluginManager, int iGlobalID, bool withChildrenType = false); \
    static class_name *GetObjByHashKey(NFIPluginManager* pPluginManager, NFObjectHashKey hashKey);\
    static int DestroyObjAutoErase(NFIPluginManager* pPluginManager,int maxNum = INVALID_ID, const DESTROY_SHM_AUTO_ERASE_FUNCTION &func = NULL);\
	static class_name* Instance(NFIPluginManager* pPluginManager);\
	static class_name* GetInstance(NFIPluginManager* pPluginManager);\
    static int GetStaticClassType();      \
    virtual std::string GetClassName() const;\
    virtual int GetClassType() const;     \
    virtual int GetItemCount() const;\
    virtual int GetUsedCount() const;\
    virtual int GetFreeCount() const;\
    /* 下面的函数 共享内存类系统注册函数， 不要手动调用 */\
    static int  RegisterClassToObjSeg(NFIPluginManager* pPluginManager, int bType, size_t siObjSize,int iObjCount, const std::string& className, bool useHash, bool singleton = false);  \
    static void* operator new( size_t nSize,void *pBuffer) throw();\
    static NFShmObj * CreateObjectRegisterFunction(NFIPluginManager* pPluginManager);\
    static NFShmObj * ResumeObjectRegisterFunction(NFIPluginManager* pPluginManager,void *pVoid);\
    static void DestroyObjectRegisterFunction(NFIPluginManager* pPluginManager,NFShmObj *pObj);\



#define _IMPLEMENT_PREALLOCATED_(class_name, type) \
    class_name::iterator class_name::Begin(NFIPluginManager* pPluginManager) \
    {\
        return pPluginManager->FindModule<NFIMemMngModule>()->IterBegin(type); \
    }\
    class_name::iterator class_name::End(NFIPluginManager* pPluginManager)\
    {\
        return pPluginManager->FindModule<NFIMemMngModule>()->IterEnd(type);\
    }\
    class_name::reverse_iterator class_name::RBegin(NFIPluginManager* pPluginManager) \
    {\
        return reverse_iterator(End(pPluginManager)); \
    }\
    class_name::reverse_iterator class_name::REnd(NFIPluginManager* pPluginManager)\
    {\
        return reverse_iterator(Begin(pPluginManager));\
    }\
    class_name::iterator class_name::Erase(NFIPluginManager* pPluginManager, class_name::iterator iter) \
    {\
        return pPluginManager->FindModule<NFIMemMngModule>()->Erase(iter);\
    }\
    class_name* class_name::CreateObj(NFIPluginManager* pPluginManager)      \
    {\
        return dynamic_cast<class_name*>(pPluginManager->FindModule<NFIMemMngModule>()->CreateObj(type));\
    }\
    class_name* class_name::CreateObjByHashKey(NFIPluginManager* pPluginManager, NFObjectHashKey hashKey)\
    {\
        return dynamic_cast<class_name*>(pPluginManager->FindModule<NFIMemMngModule>()->CreateObjByHashKey(type, hashKey));\
    }\
    void class_name::DestroyObj(NFIPluginManager* pPluginManager, class_name* pObj)\
    {\
        pPluginManager->FindModule<NFIMemMngModule>()->DestroyObj(pObj);\
    }\
    void class_name::ClearAllObj(NFIPluginManager* pPluginManager)\
    {\
        pPluginManager->FindModule<NFIMemMngModule>()->ClearAllObj(type);\
    }\
	class_name* class_name::GetObjByObjId(NFIPluginManager* pPluginManager, int iID)\
	{\
        return dynamic_cast<class_name*>(pPluginManager->FindModule<NFIMemMngModule>()->GetObjByObjId(type, iID));\
	}\
    class_name *class_name::GetObjByGlobalId(NFIPluginManager* pPluginManager, int iGlobalID, bool withChildrenType)\
	{\
        return dynamic_cast<class_name*>(pPluginManager->FindModule<NFIMemMngModule>()->GetObjByGlobalId(type, iGlobalID, withChildrenType));\
	}\
    class_name *class_name::GetObjByHashKey(NFIPluginManager* pPluginManager, NFObjectHashKey hashKey)\
    {\
        return dynamic_cast<class_name*>(pPluginManager->FindModule<NFIMemMngModule>()->GetObjByHashKey(type, hashKey));\
    }\
	int class_name::GetStaticItemCount(NFIPluginManager* pPluginManager)\
	{\
	    return pPluginManager->FindModule<NFIMemMngModule>()->GetItemCount(type);\
	}\
	int class_name::GetStaticUsedCount(NFIPluginManager* pPluginManager)\
	{\
        return pPluginManager->FindModule<NFIMemMngModule>()->GetUsedCount(type);\
	}\
	int class_name::GetStaticFreeCount(NFIPluginManager* pPluginManager)\
	{\
        return pPluginManager->FindModule<NFIMemMngModule>()->GetFreeCount(type);\
	}\
	std::string class_name::GetClassName() const\
	{\
        return FindModule<NFIMemMngModule>()->GetClassName(GetClassType());\
	}                                                 \
    int class_name::GetStaticClassType()           \
    {\
        return type;\
    }\
	int class_name::GetClassType() const\
	{\
        return type;\
	}\
    int class_name::GetItemCount() const\
    {\
        return FindModule<NFIMemMngModule>()->GetItemCount(GetClassType());\
    }\
    int class_name::GetUsedCount() const\
    {\
        return FindModule<NFIMemMngModule>()->GetUsedCount(GetClassType());\
    }\
    int class_name::GetFreeCount() const\
    {\
        return FindModule<NFIMemMngModule>()->GetFreeCount(GetClassType());\
    }\
    int class_name::DestroyObjAutoErase(NFIPluginManager* pPluginManager, int maxNum, const DESTROY_SHM_AUTO_ERASE_FUNCTION &func)\
    {\
        return pPluginManager->FindModule<NFIMemMngModule>()->DestroyObjAutoErase(type, maxNum, func);\
    }\
	class_name* class_name::Instance(NFIPluginManager* pPluginManager)\
	{\
		return (class_name *)pPluginManager->FindModule<NFIMemMngModule>()->GetHeadObj(type);\
	}\
	class_name* class_name::GetInstance(NFIPluginManager* pPluginManager)\
	{\
		return (class_name *)pPluginManager->FindModule<NFIMemMngModule>()->GetHeadObj(type);\
	}\
    /* 下面的函数 共享内存类系统注册函数， 不要手动调用 */\
    void* class_name::operator new( size_t nSize,void *pBuffer ) throw()\
	{\
		return  pBuffer;\
	}\
	NFShmObj * class_name::CreateObjectRegisterFunction(NFIPluginManager* pPluginManager)\
	{\
		class_name *pTmp = NULL;\
		void* pVoid = pPluginManager->FindModule<NFIMemMngModule>()->AllocMemForObject(type);      \
		if(!pVoid)\
		{\
			NFLogError(NF_LOG_DEFAULT, 0, "ERROR: class:{}, Item:{}, Used:{}, Free:{}", GetStaticClassName(), GetStaticItemCount(pPluginManager), GetStaticUsedCount(pPluginManager), GetStaticFreeCount(pPluginManager)); \
            return NULL;\
		}\
        pTmp = new (pVoid) class_name();\
		return pTmp;\
	}\
	NFShmObj * class_name::ResumeObjectRegisterFunction(NFIPluginManager* pPluginManager, void * pVoid)\
	{\
		class_name *pTmp = NULL;\
		pTmp = new (pVoid) class_name();\
		return pTmp;\
	}\
	void  class_name::DestroyObjectRegisterFunction(NFIPluginManager* pPluginManager, NFShmObj *pObj)\
	{\
		class_name *pTmp = NULL;\
		pTmp = (class_name*)pObj;\
        (*pTmp).class_name::~class_name();\
        pPluginManager->FindModule<NFIMemMngModule>()->FreeMemForObject(type, pTmp);\
		return;\
	}\


#define DECLARE_IDCREATE(class_name)\
    _DECLARE_PREALLOCATED_(class_name)\

#define IMPLEMENT_IDCREATE_WITHTYPE(class_name, type, parent_class) \
	_IMPLEMENT_PREALLOCATED_(class_name, type)\
	int  class_name::RegisterClassToObjSeg(NFIPluginManager* pPluginManager, int bType, size_t siObjSize,int iObjCount,const std::string& className, bool useHash, bool singleton)\
	{\
		pPluginManager->FindModule<NFIMemMngModule>()->RegisterClassToObjSeg(bType, siObjSize,iObjCount, class_name::ResumeObjectRegisterFunction,\
													   class_name::CreateObjectRegisterFunction,class_name::DestroyObjectRegisterFunction, parent_class::GetStaticClassType(),\
													   className, useHash, singleton);\
		return 0;\
	}\




