// -------------------------------------------------------------------------
//    @FileName         :    NFIDescTemplate.h
//    @Author           :    gaoyi
//    @Date             :    23-10-26
//    @Email			:    445267987@qq.com
//    @Module           :    NFIDescTemplate
//    @Desc             :    描述模板接口头文件，提供配置数据的模板化实现。
//                          该文件定义了描述模板接口类，包括模板化配置管理、
//                          数据加载和保存、索引管理、文件管理。
//                          主要功能包括提供配置数据的模板化实现、支持数据加载和保存、
//                          支持索引管理和查询、提供文件管理功能。
//                          描述模板接口是NFShmXFrame框架的配置管理模板组件，负责：
//                          - 配置数据的模板化实现
//                          - 数据加载和保存接口
//                          - 索引管理和查询功能
//                          - 文件管理和路径处理
//                          - 内存使用率计算
//                          - 跨平台文件路径支持
//
// -------------------------------------------------------------------------

#pragma once

#include "NFServerComm/NFServerCommon/NFIDescStore.h"
#include "NFComm/NFObjCommon/NFShmMgr.h"
#include "NFComm/NFShmStl/NFShmHashMap.h"
#include "NFComm/NFShmStl/NFShmHashMultiMap.h"
#include "NFComm/NFShmStl/NFShmHashMultiSet.h"
#include "NFComm/NFShmStl/NFShmHashSet.h"
#include "NFComm/NFShmStl/NFShmVector.h"
#include "NFComm/NFObjCommon/NFObjectTemplate.h"
#include "NFComm/NFCore/NFStringUtility.h"

/**
 * @brief 描述模板接口类
 * 
 * 该类是配置数据的模板化实现，提供了：
 * - 配置数据的模板化实现
 * - 数据加载和保存接口
 * - 索引管理和查询功能
 * - 文件管理和路径处理
 * 
 * @tparam className 类名
 * @tparam className_s 描述类名
 * @tparam classType 类类型
 * @tparam DescNum 描述数量
 * 
 * 主要功能：
 * - 提供配置数据的模板化实现
 * - 支持数据加载和保存接口
 * - 支持索引管理和查询功能
 * - 提供文件管理和路径处理
 * - 支持内存使用率计算
 * - 支持跨平台文件路径
 * 
 * 使用方式：
 * @code
 * class MyDescTemplate : public NFIDescTemplate<MyDescTemplate, MyDesc, EOT_MY_DESC, 1000> {
 * public:
 *     virtual int Load(NFResDb* pDB) override;
 *     virtual int SaveDescStore() override;
 * };
 * @endcode
 */
template <typename className, typename className_s, int classType, int DescNum>
class NFIDescTemplate : public NFObjectGlobalTemplate<className, classType, NFIDescStore>
{
public:
    /**
     * @brief 构造函数
     * 
     * 根据共享内存管理器的创建模式，选择调用CreateInit或ResumeInit
     */
    NFIDescTemplate()
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
     * @brief 创建初始化
     * 
     * 在对象创建时调用，初始化描述模板
     * 
     * @return 初始化结果
     */
    int CreateInit()
    {
        return 0;
    }

    /**
     * @brief 恢复初始化
     * 
     * 在对象恢复时调用，恢复描述模板状态
     * 
     * @return 初始化结果
     */
    int ResumeInit()
    {
        return 0;
    }

public:
    /**
     * @brief 获取资源数量
     * 
     * 获取描述向量的数量
     * 
     * @return 资源数量
     */
    virtual int GetResNum() const override { return m_astDescVec.size(); }

    /**
     * @brief 获取资源描述向量（非const版本）
     * 
     * 获取描述向量的引用，可以修改
     * 
     * @return 描述向量引用
     */
    NFShmVector<className_s, DescNum>& GetResDesc() { return m_astDescVec; }

    /**
     * @brief 获取资源描述向量（const版本）
     * 
     * 获取描述向量的常量引用，只读
     * 
     * @return 描述向量常量引用
     */
    const NFShmVector<className_s, DescNum>& GetResDesc() const { return m_astDescVec; }

    /**
     * @brief 根据索引获取描述（const版本）
     * 
     * 根据索引获取描述对象，只读
     * 
     * @param index 索引
     * @return 描述对象指针，如果索引无效返回nullptr
     */
    const className_s* GetDescByIndex(int index) const
    {
        CHECK_EXPR(index >= 0 && index < (int)m_astDescVec.size(), NULL, "index:{} size:{}", index, m_astDescVec.size());
        return &m_astDescVec[index];
    }

    /**
     * @brief 根据索引获取描述（非const版本）
     * 
     * 根据索引获取描述对象，可以修改
     * 
     * @param index 索引
     * @return 描述对象指针，如果索引无效返回nullptr
     */
    className_s* GetDescByIndex(int index)
    {
        return const_cast<className_s*>((static_cast<const className*>(this))->GetDescByIndex(index));
    }

    /**
     * @brief 根据ID获取描述（const版本）
     * 
     * 根据ID从映射表中查找并获取描述对象，只读
     * 
     * @param id 描述ID
     * @return 描述对象指针，如果未找到返回nullptr
     */
    const className_s* GetDesc(int64_t id) const
    {
        auto iter = m_astDescMap.find(id);
        if (iter != m_astDescMap.end())
        {
            return GetDescByIndex(iter->second);
        }
        return NULL;
    }

    /**
     * @brief 根据ID获取描述（非const版本）
     * 
     * 根据ID从映射表中查找并获取描述对象，可以修改
     * 
     * @param id 描述ID
     * @return 描述对象指针，如果未找到返回nullptr
     */
    className_s* GetDesc(int64_t id)
    {
        return const_cast<className_s*>((static_cast<const className*>(this))->GetDesc(id));
    }

public:
    /**
     * @brief 初始化
     * 
     * 初始化描述模板
     * 
     * @return 初始化结果
     */
    virtual int Initialize() override { return 0; }

    /**
     * @brief 加载数据
     * 
     * 从数据库加载描述数据
     * 
     * @param pDB 数据库对象
     * @return 加载结果
     */
    virtual int Load(NFResDb* pDB) override { return 0; }

    /**
     * @brief 加载数据库数据
     * 
     * 从数据库加载描述数据
     * 
     * @param pDB 数据库对象
     * @return 加载结果
     */
    virtual int LoadDB(NFResDb *pDB)  override { return 0; }

    /**
     * @brief 检查所有数据加载完成
     * 
     * 检查所有描述数据是否已经加载完成
     * 
     * @return 检查结果
     */
    virtual int CheckWhenAllDataLoaded() override { return 0; }

    /**
     * @brief 重新加载
     * 
     * 重新加载描述数据
     * 
     * @param pDB 数据库对象
     * @return 重新加载结果
     */
    virtual int Reload(NFResDb* pDB) override
    {
        this->PrepareReload();
        int iRetCode = this->Load(pDB);
        return iRetCode;
    }

    /**
     * @brief 获取文件名
     * 
     * 根据类名生成文件名
     * 
     * @return 文件名
     */
    virtual std::string GetFileName() override
    {
        std::string strSubModuleName = typeid(className).name();

#if NF_PLATFORM == NF_PLATFORM_WIN

        std::size_t position = strSubModuleName.find(' ');
        if (string::npos != position)
        {
            strSubModuleName = strSubModuleName.substr(position + 1, strSubModuleName.length());
        }
#else
        for (int i = 0; i < (int) strSubModuleName.length(); i++)
        {
            std::string s = strSubModuleName.substr(0, i + 1);
            int n = atof(s.c_str());
            if ((int) strSubModuleName.length() == i + 1 + n)
            {
                strSubModuleName = strSubModuleName.substr(i + 1, strSubModuleName.length());
                break;
            }
        }
#endif
        int pos = strSubModuleName.rfind("Desc");
        if (pos != std::string::npos)
        {
            strSubModuleName = strSubModuleName.substr(0, pos);
        }
        strSubModuleName = "E_" + strSubModuleName;
        return strSubModuleName;
    }

    virtual int CalcUseRatio() override
    {
        return m_astDescVec.size() * 100 / m_astDescVec.max_size();
    }

    virtual int SaveDescStore() override
    {
        return 0;
    }

    virtual int InsertDescStore(const className_s& desc)
    {
        return 0;
    }

    virtual int DeleteDescStore(const className_s& desc)
    {
        return 0;
    }

protected:
    NFShmVector<className_s, DescNum> m_astDescVec;
    NFShmHashMap<int64_t, int, DescNum> m_astDescMap;
};
