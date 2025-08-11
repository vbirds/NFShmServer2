// -------------------------------------------------------------------------
//    @FileName         :    NFPlatformMacros.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// ------------------------------------------------------------------------

/**
 * @file NFPlatformMacros.h
 * @brief 平台相关宏定义
 * 
 * 此文件定义了跨平台开发所需的各种宏，包括：
 * - 平台检测宏（Windows/Linux）
 * - 编译器检测宏（MSVC/GCC）
 * - 架构检测宏（32位/64位）
 * - 字节序检测宏（大端/小端）
 * - 导出符号宏定义
 * 
 * 通过这些宏定义，可以在编译时自动适配不同的平台和编译器环境。
 */

#pragma once

/**
 * @name 平台标识宏
 * @brief 用于标识不同的操作系统平台
 * @{
 */
/** @brief Windows平台标识 */
#define NF_PLATFORM_WIN 1
/** @brief Linux平台标识 */
#define NF_PLATFORM_LINUX 2
/** @} */

/**
 * @name 编译器标识宏
 * @brief 用于标识不同的编译器
 * @{
 */
/** @brief Microsoft Visual C++编译器标识 */
#define NF_COMPILER_MSVC 1
/** @brief GNU C++编译器标识 */
#define NF_COMPILER_GNUC 2
/** @} */

/**
 * @name 字节序标识宏
 * @brief 用于标识处理器的字节序
 * @{
 */
/** @brief 小端序标识 */
#define NF_ENDIAN_LITTLE 1
/** @brief 大端序标识 */
#define NF_ENDIAN_BIG 2
/** @} */

/**
 * @name 架构标识宏
 * @brief 用于标识处理器架构
 * @{
 */
/** @brief 32位架构标识 */
#define NF_ARCHITECTURE_32 1
/** @brief 64位架构标识 */
#define NF_ARCHITECTURE_64 2
/** @} */

/**
 * @name 编译器检测和版本定义
 * @brief 自动检测编译器类型和版本
 * @{
 */
#if defined( _MSC_VER )
/** @brief 当前编译器类型（MSVC） */
#   define NF_COMPILER NF_COMPILER_MSVC
/** @brief 编译器版本号 */
#   define NF_COMP_VER _MSC_VER
#elif defined( __GNUC__ )
/** @brief 当前编译器类型（GCC） */
#   define NF_COMPILER NF_COMPILER_GNUC
/** @brief 编译器版本号（GCC格式：主版本*100 + 次版本*10 + 补丁版本） */
#   define NF_COMP_VER (((__GNUC__)*100) + \
                        (__GNUC_MINOR__*10) + \
                        __GNUC_PATCHLEVEL__)
#else
#   pragma error "No known compiler. Abort! Abort!"
#endif
/** @} */

/**
 * @name 平台检测
 * @brief 自动检测当前编译的目标平台
 * @{
 */
#if defined( __WIN32__ ) || defined( _WIN32 ) || defined(_WINDOWS) || defined(WIN) || defined(_WIN64) || defined( __WIN64__ )
/** @brief 当前平台为Windows */
#   define NF_PLATFORM NF_PLATFORM_WIN
#else
/** @brief 当前平台为Linux */
#   define NF_PLATFORM NF_PLATFORM_LINUX
#endif
/** @} */

/**
 * @name 架构类型检测
 * @brief 根据平台自动检测处理器架构
 * @{
 */
#if NF_PLATFORM == NF_PLATFORM_WIN

#if defined(_WIN64) || defined(__WIN64__)
/** @brief Windows 64位架构 */
#   define NF_ARCH_TYPE NF_ARCHITECTURE_64
#else
/** @brief Windows 32位架构 */
#   define NF_ARCH_TYPE NF_ARCHITECTURE_32
#endif

#elif NF_PLATFORM == NF_PLATFORM_LINUX

#if defined(__x86_64__)
/** @brief Linux 64位架构 */
#   define NF_ARCH_TYPE NF_ARCHITECTURE_64
#else
/** @brief Linux 32位架构 */
#   define NF_ARCH_TYPE NF_ARCHITECTURE_32
#endif

#endif
/** @} */

/**
 * @name Windows平台设置
 * @brief Windows平台特定的配置和宏定义
 * @{
 */
#if NF_PLATFORM == NF_PLATFORM_WIN

#ifndef WIN32_LEAN_AND_MEAN
/** @brief 从Windows头文件中排除很少使用的内容 */
#define WIN32_LEAN_AND_MEAN
#endif

/** @brief 禁用MSVC编译器警告4091 */
#pragma warning(disable:4091)
#include <Windows.h>
/** @brief Windows平台的C导出宏 */
#define NF_EXPORT extern "C"  __declspec(dllexport)

#include <Dbghelp.h>

/**
 * @brief 调试模式检测
 * Win32编译器使用_DEBUG来指定调试构建
 * 对于MinGW，我们设置DEBUG
 */
#   if defined(_DEBUG) || defined(DEBUG)
#       define NF_DEBUG_MODE 1
#   endif

//#define NF_STATIC_PLUGIN 1

#ifndef NF_STATIC_PLUGIN
/** @brief 启用动态插件支持 */
#define NF_DYNAMIC_PLUGIN 1
#endif

/**
 * @name 符号导入导出宏
 * @brief Windows平台的DLL符号导入导出宏定义
 * @{
 */
#   if defined( NF_STATIC_PLUGIN )
/** @brief 静态插件模式下的导出宏（空定义） */
#       define _NFExport
/** @brief 静态插件模式下的私有宏（空定义） */
#       define _NFPrivate
#   else
#       if defined( NF_NONCLIENT_BUILD )
/** @brief 非客户端构建时的导出宏 */
#           define _NFExport __declspec( dllexport )
#       else
#           if defined( __MINGW32__ )
/** @brief MinGW编译器下的导出宏 */
#               define _NFExport
#           else
/** @brief 其他编译器下的导出宏 */
#               define _NFExport __declspec( dllimport )
#           endif
#       endif
/** @brief 静态插件模式下的私有宏 */
#       define _NFPrivate
#   endif
/** @} */

// Disable unicode support on MingW for GCC 3, poorly supported in stdlibc++
// STLPORT fixes this though so allow if found
// MinGW C++ Toolkit supports unicode and sets the define __MINGW32_TOOLBOX_UNICODE__ in _mingw.h
// GCC 4 is also fine
#if defined(__MINGW32__)
# if NF_COMP_VER < 400
#  if !defined(_STLPORT_VERSION)
#   include<_mingw.h>
#   if defined(__MINGW32_TOOLBOX_UNICODE__) || NF_COMP_VER > 345
#    define NF_UNICODE_SUPPORT 1
#   else
#    define NF_UNICODE_SUPPORT 0
#   endif
#  else
#   define NF_UNICODE_SUPPORT 1
#  endif
# else
#  define NF_UNICODE_SUPPORT 1
# endif
#else
#  define NF_UNICODE_SUPPORT 1
#endif

#ifndef __attribute__
#define __attribute__(x)
#endif

#endif // NF_PLATFORM == NF_PLATFORM_WIN


//----------------------------------------------------------------------------
// Linux/Apple/iOs/Android/Symbian/Tegra2/NaCl Settings
#if NF_PLATFORM == NF_PLATFORM_LINUX

#if !defined(NF_GCC)
#if defined(__GNUC__)
#define NF_GCC 1
#else
#define NF_GCC 0
#endif
#endif // THERON_GCC

//#include <syswait.h>

//#define NF_STATIC_PLUGIN 1

#ifndef NF_STATIC_PLUGIN
#define NF_DYNAMIC_PLUGIN 1
#endif

// Enable GCC symbol visibility
#   if defined( NF_GCC_VISIBILITY )
#       define _NFExport  __attribute__ ((visibility("default")))
#       define _NFPrivate __attribute__ ((visibility("hidden")))
#   else
#       define _NFExport
#       define _NFPrivate
#   endif

// A quick define to overcome different names for the same function
#   define stricmp strcasecmp

// Unlike the Win32 compilers, Linux compilers seem to use DEBUG for when
// specifying a debug build.
// (??? this is wrong, on Linux debug builds aren't marked in any way unless
// you mark it yourself any way you like it -- zap ???)
#   if defined(_DEBUG) || defined(DEBUG)
#       define NF_DEBUG_MODE 1
#   endif

// Always enable unicode support for the moment
// Perhaps disable in old versions of gcc if necessary
#define NF_UNICODE_SUPPORT 1
#define MAX_PATH 255

#define NF_EXPORT extern "C" __attribute ((visibility("default")))

#endif

/* See if we can use __forceinline or if we need to use __inline instead */

#if !defined(NF_FORCEINLINE)
#if NF_DEBUG_MODE
#define NF_FORCEINLINE inline
#else // NF_DEBUG_MODE
#if NF_COMPILER == NF_COMPILER_MSVC
#   if NF_COMP_VER >= 1200
#       define NF_FORCEINLINE __forceinline
#   endif
#elif defined(__MINGW32__)
#   if !defined(NF_FORCEINLINE)
#       define NF_FORCEINLINE __inline
#   endif
#elif NF_GCC
#define NF_FORCEINLINE inline __attribute__((always_inline))
#else
#define NF_FORCEINLINE inline
#endif
#endif // THERON_DEBUG
#endif // THERON_FORCEINLINE

//----------------------------------------------------------------------------
// Endian Settings
// check for BIG_ENDIAN config flag, set NF_ENDIAN correctly
#ifdef NF_CONFIG_BIG_ENDIAN
#    define NF_ENDIAN NF_ENDIAN_BIG
#else
#    define NF_ENDIAN NF_ENDIAN_LITTLE
#endif


#if defined(__GNUC__) || defined(__clang__)
#define PRINTF_FORMAT_ATTR __attribute__((format(printf, 1, 2)))
#else
    #define PRINTF_FORMAT_ATTR
#endif
