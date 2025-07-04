// -------------------------------------------------------------------------
//    @FileName         :    NFCPluginManager.h
//    @Author           :    LvSheng.Huang
//    @Date             :   2022-09-18
//    @Module           :    NFCPluginManager
//
// -------------------------------------------------------------------------

#ifndef NFC_DYNLIB_H
#define NFC_DYNLIB_H

#include <stdio.h>
#include <iostream>
#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"

#if NF_PLATFORM == NF_PLATFORM_WIN
#    define DYNLIB_HANDLE hInstance
#    define DYNLIB_LOAD( a ) LoadLibraryExA( a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH )
#    define DYNLIB_GETSYM( a, b ) GetProcAddress( a, b )
#    define DYNLIB_UNLOAD( a ) FreeLibrary( a )

struct HINSTANCE__;
typedef HINSTANCE__* hInstance;

#elif NF_PLATFORM == NF_PLATFORM_LINUX || NF_PLATFORM == NF_PLATFORM_ANDROID
#include <dlfcn.h>
#define DYNLIB_HANDLE void*
#define DYNLIB_LOAD( a ) dlopen( a, RTLD_LAZY | RTLD_GLOBAL) //RTLD_GLOBAL是so热更出现问题
//#define DYNLIB_LOAD( a ) dlopen( a, RTLD_LAZY)
#define DYNLIB_GETSYM( a, b ) dlsym( a, b )
#define DYNLIB_UNLOAD( a ) dlclose( a )

#elif NF_PLATFORM == NF_PLATFORM_APPLE || NF_PLATFORM == NF_PLATFORM_APPLE_IOS
#include <dlfcn.h>
#define DYNLIB_HANDLE void*
#define DYNLIB_LOAD( a ) dlopen( a, RTLD_LOCAL|RTLD_LAZY)
#define DYNLIB_GETSYM( a, b ) dlsym( a, b )
#define DYNLIB_UNLOAD( a ) dlclose( a )

#endif

class _NFExport NFCDynLib
{

public:
    explicit NFCDynLib(const std::string& strName)
    {
        m_strName = strName;
		m_inst = nullptr;
#if NF_PLATFORM == NF_PLATFORM_WIN
        m_strName.append(".dll");
#elif NF_PLATFORM == NF_PLATFORM_LINUX || NF_PLATFORM == NF_PLATFORM_ANDROID
        m_strName = "lib" + m_strName + ".so";
#elif NF_PLATFORM == NF_PLATFORM_APPLE || NF_PLATFORM == NF_PLATFORM_APPLE_IOS
        m_strName = "lib" + m_strName + ".so";
#endif
    }

    ~NFCDynLib()
    {

    }

    bool Load();

    bool UnLoad();

    /// Get the name of the library
    const std::string& GetName() const
    {
        return m_strName;
    }

    void* GetSymbol(const char* szProcName);

protected:

    std::string m_strName;

    DYNLIB_HANDLE m_inst;
};

#endif
