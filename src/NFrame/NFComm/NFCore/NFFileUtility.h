// -------------------------------------------------------------------------
//    @FileName         :    NFFileUtility.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFFileUtility.h
 * @brief 文件和路径操作工具类
 * 
 * 此文件提供了各种文件和路径操作的工具函数，包括路径规范化、文件名分割、
 * 目录操作、文件读写等功能。支持跨平台的文件系统操作。
 */

#pragma once

#include "NFPlatform.h"

#if NF_PLATFORM == NF_PLATFORM_WIN
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

/** @brief Windows平台的最大路径长度定义 */
#define _MAX_PATH 260
#endif

#include <list>
#include <string>
#include <cstring>
#include <vector>

/**
 * @brief 文件访问工具类
 * 
 * 提供了许多帮助方法来简化文件操作，包括路径处理、文件读写、
 * 目录操作等功能。所有方法都是静态方法，可以直接调用。
 * 
 * 主要功能包括：
 * - 路径规范化和清理
 * - 文件名和路径分离
 * - 绝对路径和相对路径转换
 * - 目录遍历和操作
 * - 文件内容读写
 */
class _NFExport NFFileUtility
{
public:
	/**
	 * @brief 查询文件名是否为有效的目录名
	 * @param szName 要检查的目录名
	 * @return bool 如果是有效目录名返回true，否则返回false
	 * @note "."和".."被认为是无效的目录名
	 */
	static bool IsValidDirName(const std::string& szName);

	/**
	 * @brief 查询字符串是否为绝对路径
	 * @param path 要检查的路径字符串
	 * @return bool 如果是绝对路径返回true，否则返回false
	 */
	static bool IsAbsolutePath(const std::string& path);

	/**
	 * @brief 获取绝对路径名
	 * @param path 输入路径，如果已经是绝对路径则直接返回，否则与当前路径组合
	 * @return std::string 绝对路径字符串
	 */
	static std::string GetAbsolutePathName(const std::string& path);

	/**
	 * @brief 清理路径名，返回等效的最短路径名
	 * 
	 * 此方法从Golang项目复制而来，通过纯词法处理应用以下规则：
	 * 1. 用单个分隔符替换多个分隔符元素
	 * 2. 消除每个 . 路径名元素（当前目录）
	 * 3. 消除每个内部 .. 路径名元素（父目录）及其前面的非..元素
	 * 4. 消除以根路径开头的..元素
	 * 
	 * @param path 要清理的路径
	 * @return std::string 清理后的路径
	 * @note 如果处理结果为空字符串，则返回"."
	 */
	static std::string Clean(const std::string& path);

	/**
	 * @brief 将完全限定的文件名分割为文件名和基路径目录
	 * @param filepath 完整的文件路径
	 * @param file_name [out] 分离出的文件名
	 * @param base_dir [out] 分离出的基目录
	 * @param with_trailing_slash [in] 是否在目录末尾添加斜杠，默认为false
	 */
	static void SplitFileName(const std::wstring& filepath,
		std::wstring& file_name, std::wstring& base_dir, bool with_trailing_slash = false);
	
	/**
	 * @brief 将完全限定的文件名分割为文件名和基路径目录（string版本）
	 * @param filepath 完整的文件路径
	 * @param file_name [out] 分离出的文件名
	 * @param base_dir [out] 分离出的基目录
	 * @param with_trailing_slash [in] 是否在目录末尾添加斜杠，默认为false
	 */
	static void SplitFileName(const std::string& filepath,
		std::string& file_name, std::string& base_dir, bool with_trailing_slash = false);

	/**
	 * @brief 连接两个路径为一个路径
	 * @param prefix 前缀路径
	 * @param postfix 后缀路径
	 * @return std::string 连接后的路径
	 */
	static std::string Join(const std::string& prefix, const std::string& postfix);
	
	/**
	 * @brief 连接两个路径为一个路径（wstring版本）
	 * @param prefix 前缀路径
	 * @param postfix 后缀路径
	 * @return std::wstring 连接后的路径
	 */
	static std::wstring Join(const std::wstring& prefix, const std::wstring& postfix);

	/**
	 * @brief 标准化路径 - 仅使用正斜杠"/"
	 * @param filepath 要标准化的文件路径
	 * @param with_trailing_slash true表示字符串末尾带斜杠"/"，false表示末尾不带斜杠"/"
	 * @return std::string 标准化后的路径
	 */
	static std::string NormalizePath(const std::string& filepath, bool with_trailing_slash = true);
	
	/**
	 * @brief 标准化路径 - 仅使用正斜杠"/"（wstring版本）
	 * @param filepath 要标准化的文件路径
	 * @param with_trailing_slash true表示字符串末尾带斜杠"/"，false表示末尾不带斜杠"/"
	 * @return std::wstring 标准化后的路径
	 */
	static std::wstring NormalizePath(const std::wstring& filepath, bool with_trailing_slash = true);

	/**
	 * @brief 获取路径的父目录名
	 * @param filepath 文件路径或目录路径
	 * @param with_trailing_slash 是否在目录末尾添加斜杠，默认为true
	 * @return std::string 父目录路径
	 * @note 返回的父目录路径适用于<code>NormalizePath</code>，
	 *       这意味着它使用正斜杠"/"作为路径分隔符
	 * @note 它处理相对路径
	 * @param with_trailing_slash
	 *         true, End of the string With a SLASH "/".
	 *         false, without a SLASH "/" at the end of the string
	 * @return string,
	 */
	static std::string GetParentDir(const std::string& filepath, bool with_trailing_slash = true);

	// Gets file name without path..
	// e.g. "D:/test/aab.jpg" ==> "aab.jpg"
	static std::string GetFileName(const std::string& filepath);

	// Gets base file name. the file extension will be removed.
	// e.g. "D:/test/aab.jpg" ==> "aab"
	static std::string GetFileNameWithoutExt(const std::string& filepath);

	// Gets file name extension. If can not find the last '.', null string
	// will be returned. e.g. "D:/test/aab.jpg" ==> "jpg"
	static std::string GetFileNameExtension(const std::string& filepath);

	// Gets file path name without end '/'.
	// e.g. "D:/test/aab.jpg" ==> "D:/test"
	static std::string GetFileDirName(const std::string& filepath);

	static std::string GetExcludeFileExt(const std::string &sFullFileName);

	static bool CreateLink(const std::string& oldpath, const std::string& newpath);
	// Quickly remove file or directory from local file system.
	// @param strFileName if end with '/', it is a directory, all the children will be
	// remove, else is a file and the file will be removed.
	// @return bool True if successfully.
	static bool Unlink(const std::string& filepath);

	static bool Remove(const std::string& filepath)
	{
		return Unlink(filepath);
	}

	// @brief Remove the empty directory. If it is not empty or it is not directory, we do nothing and just return true.
	// @remark Empty directory definition: it doesn't has any normal file.
	// e.g. /home/a_dir/b_dir/c_dir
	//     Condition: c_dir has no files in it, and b_dir only has c_dir and a_dir only has b_dir
	//     If we call RemoveDirIfEmpty( "/home/a_dir/b_dir", false ), them b_dir will be deleted and its child c_dir deleted
	//     If we call RemoveDirIfEmpty( "/home/a_dir/b_dir", true ), them a_dir, b_dir, c_dir will all be deleted.
	// @param recursively_delete_empty_parent_dir True, we will delete its parent directory if its parent directory is also empty
	// @return bool True if successfully.
	static bool RemoveDirIfEmpty(const std::string& dir, bool recursively_delete_empty_parent_dir = true);

	// Make directory hierarchy in local file system.
	// Its behavior is similar with the command 'mkdir -p'
	// @param dir Absolute directory file name. e.g. '/home/weizili/test'
	// @return true if successfully
	static bool Mkdir(const std::string& dir);

	static bool Rmdir(const std::string& dir);
	static bool Rmdir(const char* dir);

	// Determines if a file exists and could be opened.
	// @note The file CAN be a directory
	// @param filename is the string identifying the file which should be tested for existence.
	// @return Returns true if file exists, and false if it does not exist or an error occurred.
	static bool IsFileExist(const std::string& filepath);
	static uint64_t GetFileSize(const std::string& filename);

	static bool IsReadable(const std::string& filepath);
	static bool IsWriteable(const std::string& filepath);
	static bool IsExecutable(const std::string& filepath);

	// @brief copy a file to another place. This function has the same feature like DOS Command 'copy' or Linux Command 'cp'
	// @warning If the destination file is exist, it will be replaced.
	// @warning dest_file MUST BE a file, NOT a directory
	// @param src_file The source file
	// @param src_file The target file
	// @param override True, this will override the old existent file.
	// @return bool True if successfully
	static bool CopyFile(const std::string& src_file, const std::string& dest_file, bool override = true, bool comp = false);

	// @brief Query whether the given path is a directory.
	static bool IsDir(const char* filepath);
	static bool IsDir(const std::string& filepath);

	// @brief Walk the directory and get a list of files, excluded directory.
	// @param dir The directory path
	// @param files[out] The list of files are stored here. The file name is with the full path name(include the directory name)
	// @param recursively Whether walk the subdirectories.
	// @param filter Pattern to match against; which can include simple '*' wildcards
	static void GetFiles(const std::string& dir, std::list<std::string>& files, bool recursively = true, const std::string& filter = "*");

	// @brief Walk the directory and get a list of files, not include directory.
	// @param dir The directory path
	// @param files[out] The list of files are stored here. The file name is with the full path name(include the directory name)
	// @param depth The depth to walk the subdirectories. 0 means only walk the top dir strDirName
	// @param filter Pattern to match against; which can include simple '*' wildcards
	static void GetFiles(const std::string& dir, std::list<std::string>& files, int depth, const std::string& filter = "*");

	// Extract strings from the a file line-by-line,
	// every line content as a list element will be inserted to a list.
	static bool ReadFile(const char* filepath, std::list<std::string>& lines);
    static bool ReadFile(const std::string& filepath, std::vector<std::string>& lines);

	static bool ReadFileContent(const std::string& strFileName, std::string& strContent);

	// @brief Write the data into a file.
	//  The file is created if it does not exist, otherwise it is truncated
	// @param const char* filepath - The file path where the data is stored
	// @param const void* content - The file content
	// @param const size_t len - The length of the content
	// @return true if successfully
	static bool WriteFile(const char* filepath, const void* content, const size_t len);
	static bool WriteFile(const std::string& filepath, const void* content, const size_t len);
    static bool WriteFile(const std::string& filepath, const std::string& content);

    static bool AWriteFile(const char* filepath, const void* content, const size_t len);
    static bool AWriteFile(const std::string& filepath, const void* content, const size_t len);
    static bool AWriteFile(const std::string& filepath, const std::string& content);

    static bool ChmodFile(const char* filepath, int mode);
    static bool ChmodFile(const std::string& filepath, int mode);

#if NF_PLATFORM == NF_PLATFORM_WIN
	static uint64_t GetWindowsToUnixBaseTimeOffset();
#endif
	static uint64_t GetFileModificationSecond(const std::string& filename);
	static uint64_t GetFileModificationMicroSec(const std::string& filename);

	static int GetFileContainMD5(const std::string& strFileName, std::string& fileMd5);
};

