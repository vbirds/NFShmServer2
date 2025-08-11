// -------------------------------------------------------------------------
//    @FileName         :    NFStackTrace.cpp
//    @Author           :    gaoyi
//    @Date             :    24-8-23
//    @Email            :    445267987@qq.com
//    @Module           :    NFStackTrace
//    @Desc             :    堆栈跟踪工具类实现，提供程序运行时堆栈信息获取功能
//
// -------------------------------------------------------------------------

#include "NFStackTrace.h"

#include <NFComm/NFCore/NFStringUtility.h>

#if NF_PLATFORM == NF_PLATFORM_WIN
#include "windows.h"
#include <psapi.h>
#include <iostream>
#include <sstream>
#include <cstddef>
#include <dbghelp.h>
#pragma comment( lib, "dbghelp" )
#pragma comment(lib, "iphlpapi.lib")
#pragma warning(disable:4100)

static HANDLE g_hProcess = 0;
#else
#include <cxxabi.h>

#endif

/**
 * @brief 构造函数
 * 
 * 初始化堆栈跟踪工具，根据平台进行不同的初始化
 */
NFStackTrace::NFStackTrace()
{
#if NF_PLATFORM == NF_PLATFORM_WIN
    // Windows平台初始化
    m_hProcess = GetCurrentProcess();
    g_hProcess = m_hProcess;
    DWORD dwOpts = SymGetOptions();
    dwOpts |= SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS | SYMOPT_DEBUG;
    SymSetOptions(dwOpts);
    ::SymInitialize(m_hProcess, 0, true);
#else
    // Linux平台初始化
    m_iExecCnt = 0;
    m_tLastReportTime = 0;
#endif
}

/**
 * @brief 析构函数
 * 
 * 清理资源，释放系统句柄
 */
NFStackTrace::~NFStackTrace()
{
#if NF_PLATFORM == NF_PLATFORM_WIN
    ::SymCleanup(m_hProcess);
#else
#endif
}

/**
 * @brief 获取堆栈跟踪信息
 * 
 * 根据平台调用相应的堆栈跟踪实现
 * 
 * @param bNeedExec 是否执行tracestack，为false直接返回
 * @param bFullParse 是否完全解析堆栈信息
 * @return 堆栈跟踪信息字符串
 */
const char *NFStackTrace::TraceStack(bool bNeedExec/* = false*/, bool bFullParse/* = false*/)
{
#if NF_PLATFORM == NF_PLATFORM_WIN
    return TraceWindowsStack(bNeedExec, bFullParse);
#else
    return TraceLinuxStack(bNeedExec, bFullParse);
#endif
}

#if NF_PLATFORM == NF_PLATFORM_WIN
/////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 基本数据类型枚举
 * 
 * 用于Windows平台符号解析
 */
enum BasicType
{
    btNoType = 0,
    btVoid = 1,
    btChar = 2,
    btWChar = 3,
    btInt = 6,
    btUInt = 7,
    btFloat = 8,
    btBCD = 9,
    btBool = 10,
    btLong = 13,
    btULong = 14,
    btCurrency = 25,
    btDate = 26,
    btVariant = 27,
    btComplex = 28,
    btBit = 29,
    btBSTR = 30,
    btHresult = 31
};

/**
 * @brief 将地址转换为字符串
 * 
 * 将堆栈地址转换为可读的字符串，包含函数名和行号信息
 * 
 * @param pVoid 地址指针
 * @return 格式化的地址字符串
 */
std::string NFStackTrace::addressToString(PVOID pVoid)
{
    std::ostringstream oss;
    STACKFRAME *pStackFrame = (STACKFRAME *) pVoid;
    DWORD dwAddress = pStackFrame->AddrPC.Offset;
    oss << "0x" << pVoid;
    
    // 获取符号信息
    struct tagSymInfo
    {
        IMAGEHLP_SYMBOL symInfo;
        char nameBuffer[4 * 256];
    } SymInfo = {{sizeof(IMAGEHLP_SYMBOL)}};
    IMAGEHLP_SYMBOL *pSym = &SymInfo.symInfo;
    pSym->MaxNameLength = sizeof(SymInfo) - offsetof(tagSymInfo, symInfo.Name);
    DWORD dwDisplacement;
    
#ifdef _MSC_VER
    if (SymGetSymFromAddr(m_hProcess, dwAddress, (PDWORD64) &dwDisplacement, pSym))
#else
    if (SymGetSymFromAddr(m_hProcess, dwAddress, &dwDisplacement, pSym))
#endif // _MSC_VER
    {
        oss << " " << pSym->Name;
        //if ( dwDisplacement != 0 )
        //	oss << "+0x" << std::hex << dwDisplacement << std::dec;
    }

    // 获取文件和行号信息
    IMAGEHLP_LINE lineInfo = {sizeof(IMAGEHLP_LINE)};
    if (SymGetLineFromAddr(m_hProcess, dwAddress, &dwDisplacement, &lineInfo))
    {
        char const *pDelim = strrchr(lineInfo.FileName, '\\');
        oss << " in " << (pDelim ? pDelim + 1 : lineInfo.FileName) << "(" << lineInfo.LineNumber << ")";
    }
    return oss.str();
}

/**
 * @brief Windows平台堆栈跟踪
 * 
 * 使用Windows API获取当前线程的堆栈信息
 * 
 * @param bNeedExec 是否执行
 * @param bFullParse 是否完全解析
 * @return 堆栈信息字符串
 */
const char *NFStackTrace::TraceWindowsStack(bool bNeedExec, bool bFullParse)
{
    m_szStackInfoString[0] = 0;
    char *szOut = m_szStackInfoString;
    int iOutLen = 0;
#ifndef _WIN64
    // 获取当前线程上下文
    CONTEXT context = {CONTEXT_FULL};
    ::GetThreadContext( GetCurrentThread(), &context );
    _asm call $+5
    _asm pop eax
    _asm mov context.Eip, eax
    _asm mov eax, esp
    _asm mov context.Esp, eax
    _asm mov context.Ebp, ebp

PCONTEXT pContext = &context;
    iOutLen += _snprintf(szOut, sizeof(m_szStackInfoString), "%s", "TrackStack begin \n");
    
    // 初始化堆栈帧
    STACKFRAME stackFrame = {0};
    stackFrame.AddrPC.Offset = pContext->Eip;
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = pContext->Ebp;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = pContext->Esp;
    stackFrame.AddrStack.Mode = AddrModeFlat;
    
    int i=0;
    // 遍历堆栈帧
    while ( ::StackWalk(IMAGE_FILE_MACHINE_I386,m_hProcess,	GetCurrentThread(),
        &stackFrame,pContext,NULL,::SymFunctionTableAccess,::SymGetModuleBase,NULL))
    {
        i++;
        if(i > 3)
        {
            iOutLen += _snprintf(szOut+iOutLen, sizeof(m_szStackInfoString) - iOutLen, "%s", addressToString( (PVOID)&stackFrame).c_str());
            iOutLen += _snprintf(szOut+iOutLen, sizeof(m_szStackInfoString) - iOutLen, "%s", "\n");
        }
        if(i > MAX_BACK_TRACE_SIZE)
        {
            break;
        }
    }
    iOutLen += _snprintf(szOut+iOutLen, sizeof(m_szStackInfoString) - iOutLen, "%s", "TrackStack end \n");
#endif
    return m_szStackInfoString;
}
#else
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <signal.h>
#include <ucontext.h>
#include <dlfcn.h>
#include <execinfo.h>

#define MAX_BACK_TRACE_SIZE (11)//以前是11,减少到6,看看性能开销

/**
 * @brief 获取执行次数
 * @return 执行次数
 */
int NFStackTrace::GetExecCnt()
{
    return m_iExecCnt;
}

/**
 * @brief 获取最后报告时间
 * @return 最后报告时间
 */
time_t NFStackTrace::GetLastReportTime()
{
    return m_tLastReportTime;
}

/**
 * @brief 设置执行次数
 * @param iExecCnt 执行次数
 */
void NFStackTrace::SetExecCnt(int iExecCnt)
{
    m_iExecCnt = iExecCnt;
}

/**
 * @brief 更新报告时间
 * @param tCur 当前时间
 */
void NFStackTrace::UpdateReportTime(time_t tCur)
{
    m_tLastReportTime = tCur;
}

/**
 * @brief Linux平台堆栈跟踪
 * 
 * 使用backtrace库获取当前线程的堆栈信息
 * 
 * @param bNeedExec 是否执行
 * @param bFullParse 是否完全解析
 * @return 堆栈信息字符串
 */
const char* NFStackTrace::TraceLinuxStack(bool bNeedExec, bool bFullParse)
{
    if (false == bNeedExec)
    {
        return " ";
    }

    // 记录开始时间
    struct timeval stBegin;
    gettimeofday( &stBegin, NULL );

    m_iExecCnt++;

    m_szStackInfoString[0] = 0;
    char* pCurBuff = m_szStackInfoString;
    void *bt[MAX_BACK_TRACE_SIZE];
    int iOff = 0;
    iOff += snprintf(pCurBuff, sizeof(m_szStackInfoString), "%s", "TrackStack begin \n");
    
    // 获取堆栈信息
    size_t sz = backtrace(bt, MAX_BACK_TRACE_SIZE);
    char **strings = backtrace_symbols(bt, sz);
    if (NULL == strings)
    {
        return " ";
    }

    // 根据参数选择解析方式
    if (bFullParse)
    {
        TraceLinuxStackFullParse(strings, pCurBuff, iOff, sz);
    }
    else
    {
        TraceLinuxStackHalfParse(strings, pCurBuff, iOff, sz);
    }

    free(strings);

    iOff += snprintf(pCurBuff+iOff, sizeof(m_szStackInfoString)-iOff, "%s", "TrackStack end\n");

    // 计算耗时
    struct timeval stEnd;
    gettimeofday( &stEnd, NULL );
    int64_t llCost = (int64_t)( stEnd.tv_sec - stBegin.tv_sec )*1000000 + stEnd.tv_usec - stBegin.tv_usec;

    iOff += snprintf(pCurBuff+iOff, sizeof(m_szStackInfoString)-iOff, "(bFullParse %d)TrackStack cost time %ld ms  %ld us",  (bFullParse ? 1 : 0), llCost/1000, llCost % 1000);

    return m_szStackInfoString;
}

/**
 * @brief Linux平台完整解析堆栈
 * 
 * 会把符号demangle，解析成可读性强的形式,但是性能太差，外网勿用
 * 
 * @param strings 符号字符串数组
 * @param pCurBuff 输出缓冲区
 * @param iOff 偏移量
 * @param sz 大小
 */
void NFStackTrace::TraceLinuxStackFullParse(char **strings, char* pCurBuff, int & iOff, size_t sz)
{
    for(size_t i = 3; i < sz; ++i)
    {
        const char * _xxs_str = ReplaceDumpSymname(strings[i]);
        if (_xxs_str && strlen(_xxs_str) > 0)
        {
            iOff += snprintf(pCurBuff+iOff, sizeof(m_szStackInfoString)-iOff, "%s \n", _xxs_str);
        }
        else if (_xxs_str)
        {
            iOff += snprintf(pCurBuff+iOff, sizeof(m_szStackInfoString)-iOff, "%s \n",strings[i]);
        }
        else
        {
            iOff += snprintf(pCurBuff+iOff, sizeof(m_szStackInfoString)-iOff, "\r\n");
        }
    }
}

/**
 * @brief Linux平台半解析堆栈
 * 
 * 不执行demangle, 查看日志后，需要手动使用c++filt来解析成可读性强的格式，看看对比下性能如何
 * 
 * @param strings 符号字符串数组
 * @param pCurBuff 输出缓冲区
 * @param iOff 偏移量
 * @param sz 大小
 */
void NFStackTrace::TraceLinuxStackHalfParse(char **strings, char* pCurBuff, int & iOff, size_t sz)
{
    for(size_t i = 3; i < sz; ++i)
    {
        iOff += snprintf(pCurBuff+iOff, sizeof(m_szStackInfoString)-iOff, "%s \n",strings[i]);
    }
}

#define MAX_DEMANGLED_NAME_LEN	(2048)

/**
 * @brief 替换符号名称
 * 
 * 将C++符号名称转换为可读的函数名
 * 
 * @param dump_sym_str 符号字符串
 * @return 处理后的符号字符串
 */
char* NFStackTrace::ReplaceDumpSymname(char* dump_sym_str)
{
    static char mangled_str[MAX_DEMANGLED_NAME_LEN];
    static char demangled_str[MAX_DEMANGLED_NAME_LEN];
    static char bin_str[MAX_DEMANGLED_NAME_LEN];
    static char offset_str[32];
    static int	dump_stack_status=0;
    static char* dump_demangled=0;

    static char dump_demangled_str[MAX_DEMANGLED_NAME_LEN*2];

    if(sizeof(dump_demangled_str)<strlen(dump_sym_str)+1)
    {
        return NULL;
    }
    //fmt=%[^(](%s)
    std::vector<std::string> vecString;
    NFStringUtility::SplitString(dump_sym_str, "(", vecString);

    // /data/home/user00/projectx/svr/gamesvr/bin/gamesvr(_ZN5MAYEX10CServiceGM12ProcessGMReqEPNS_8CSessionEPKN2CS8CSMsgReqE+0x38b2)
    //自己解析,代替sscanf,sscanf被codeccc告警,可能会缓冲区溢出
    if (2 ==vecString.size())
    {
        //vecString[0]: bin_str
        //vecString[1]: func + offstring
        snprintf(bin_str, sizeof(bin_str), "%s", vecString[0].c_str());
        std::vector<std::string> vecString2;
        NFStringUtility::SplitString(vecString[1], "+", vecString2);
        if (2 == vecString2.size())
        {
            snprintf(mangled_str, sizeof(mangled_str), "%s", vecString2[0].c_str());
            snprintf(offset_str, sizeof(offset_str), "%s", vecString2[1].c_str());

            size_t _length = sizeof(demangled_str);
            dump_demangled = abi::__cxa_demangle(mangled_str,
                                demangled_str,
                                &_length,
                                &dump_stack_status);
            if( 0 == dump_stack_status && dump_demangled)
            {
                snprintf(dump_demangled_str,sizeof(dump_demangled_str),"%s(%s+%s)",bin_str,demangled_str,offset_str);
            }
            else
            {
                return NULL;
            }
        }
    }

    return dump_demangled_str;
}
#endif
