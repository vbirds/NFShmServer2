// -------------------------------------------------------------------------
//    @FileName         :    NFStackTrace.h
//    @Author           :    gaoyi
//    @Date             :    24-8-23
//    @Email            :    445267987@qq.com
//    @Module           :    NFStackTrace
//    @Desc             :    堆栈跟踪工具类，提供程序运行时堆栈信息获取功能
//
// -------------------------------------------------------------------------

#pragma once

#include <NFComm/NFCore/NFSingleton.hpp>

#include "NFComm/NFCore/NFPlatform.h"

/**
 * @brief 获取完整堆栈跟踪信息
 * 
 * 获取当前线程的完整堆栈信息，包括函数名、文件名、行号等
 * 性能开销较大，建议只在调试时使用
 */
#define TRACE_STACK() (NFStackTrace::Instance()->TraceStack(true, true))

#ifdef NF_DEBUG_MODE
    /**
     * @brief 调试模式下的堆栈跟踪
     * 
     * 在调试模式下获取完整堆栈信息
     */
    #define DEBUG_TRACE_STACK() (NFStackTrace::Instance()->TraceStack(true, true))
#else
   /**
    * @brief Release模式下的堆栈跟踪
    * 
    * Release版本直接不打印，因为此调用性能开销大
    */
    #define DEBUG_TRACE_STACK() (SingleStackTrace::instance()->TraceStack(false, false))
#endif

/**
 * @brief 获取堆栈跟踪执行次数
 */
#define GET_TRACE_STACK_EXEC_CNT() ((NFStackTrace::Instance()->GetExecCnt())
/**
 * @brief 设置堆栈跟踪执行次数
 */
#define SET_TRACE_STACK_EXEC_CNT(iCnt) (NFStackTrace::Instance()->SetExecCnt(iCnt))
/**
 * @brief 获取堆栈跟踪最后报告时间
 */
#define GET_TRACE_STACK_LAST_REPORT_TIME() ((NFStackTrace::Instance()->GetLastReportTime())
/**
 * @brief 更新堆栈跟踪报告时间
 */
#define UPDATE_TRACE_STACK_REPORT_TIME(tCur) ((NFStackTrace::Instance()->UpdateReportTime(tCur))

/**
 * @brief 堆栈跟踪工具类
 * 
 * 提供程序运行时堆栈信息获取功能，支持Windows和Linux平台
 * 使用单例模式，确保全局唯一实例
 * 
 * 性能说明：
 * - TraceStack一次执行耗时平均20ms左右，多的150ms左右
 * - 默认关闭，只在逻辑出错想打印堆栈的地方传true参数
 * - 建议在调试阶段使用，生产环境谨慎使用
 */
class NFStackTrace : public NFSingleton<NFStackTrace>
{
public:
    /**
     * @brief 获取堆栈跟踪信息
     * 
     * @param bNeedExec 是否执行tracestack，为false直接返回
     * @param bFullParse 是否完全解析堆栈信息
     * @return 堆栈跟踪信息字符串
     */
    const char* TraceStack(bool bNeedExec = false, bool bFullParse = false);

public:
    /**
     * @brief 构造函数
     */
    NFStackTrace();
    
    /**
     * @brief 析构函数
     */
    ~NFStackTrace();

private:
    char m_szStackInfoString[10240];  ///< 堆栈信息字符串缓冲区

#if NF_PLATFORM == NF_PLATFORM_WIN
    void* m_hProcess;  ///< Windows进程句柄
    
    /**
     * @brief 将地址转换为字符串
     * @param address 地址指针
     * @return 地址字符串
     */
    std::string addressToString(void* address);
    
    /**
     * @brief Windows平台堆栈跟踪
     * @param bNeedExec 是否执行
     * @param bFullParse 是否完全解析
     * @return 堆栈信息字符串
     */
    const char* TraceWindowsStack(bool bNeedExec, bool bFullParse);
#else
public:
    /**
     * @brief 获取执行次数
     * @return 执行次数
     */
    int GetExecCnt();
    
    /**
     * @brief 获取最后报告时间
     * @return 最后报告时间
     */
    time_t GetLastReportTime();
    
    /**
     * @brief 设置执行次数
     * @param iExecCnt 执行次数
     */
    void SetExecCnt(int iExecCnt);
    
    /**
     * @brief 更新报告时间
     * @param tCur 当前时间
     */
    void UpdateReportTime(time_t tCur);

private:
    /**
     * @brief Linux平台堆栈跟踪
     * @param bNeedExec 是否执行
     * @param bFullParse 是否完全解析
     * @return 堆栈信息字符串
     */
    const char* TraceLinuxStack(bool bNeedExec, bool bFullParse);
    
    /**
     * @brief 替换符号名称
     * @param dump_sym_str 符号字符串
     * @return 处理后的符号字符串
     */
    char* ReplaceDumpSymname(char* dump_sym_str);

private:
    /**
     * @brief Linux平台完整解析堆栈
     * @param strings 符号字符串数组
     * @param pCurBuff 输出缓冲区
     * @param iOff 偏移量
     * @param sz 大小
     */
    void TraceLinuxStackFullParse(char **strings, char* pCurBuff, int & iOff, size_t sz);
    
    /**
     * @brief Linux平台半解析堆栈
     * @param strings 符号字符串数组
     * @param pCurBuff 输出缓冲区
     * @param iOff 偏移量
     * @param sz 大小
     */
    void TraceLinuxStackHalfParse(char **strings, char* pCurBuff, int & iOff, size_t sz);

private:
    int m_iExecCnt;        ///< 执行次数
    time_t m_tLastReportTime;  ///< 最后报告时间
#endif
};