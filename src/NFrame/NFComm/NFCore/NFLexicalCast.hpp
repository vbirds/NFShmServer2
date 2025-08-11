/**
 * @file NFLexicalCast.hpp
 * @brief 类型安全的词法转换工具
 * 
 * 此文件提供了高效、类型安全的词法转换功能，支持字符串和各种数值类型之间的转换。
 * 基于C++11模板技术实现，提供了比标准库更简洁易用的类型转换接口。
 */

#ifndef CPP_11_LEXICAL_CAST_HPP
#define CPP_11_LEXICAL_CAST_HPP

#include <type_traits>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <cctype>
#include <cstring>
using namespace std;

/**
 * @namespace detail
 * @brief 词法转换的内部实现细节
 * 
 * 此命名空间包含了词法转换的具体实现，包括各种类型转换器的特化版本。
 * 用户一般不需要直接使用此命名空间中的内容。
 */
namespace detail
{
    /** @brief 布尔值true的字符串表示 */
    static const char* strue = "true";
    /** @brief 布尔值false的字符串表示 */
    static const char* sfalse = "false";

    /**
     * @brief 通用类型转换器模板
     * 
     * 所有具体的类型转换都通过特化此模板来实现。
     * 
     * @tparam To 目标类型
     * @tparam From 源类型
     */
    template <typename To, typename From>
    struct Converter
    {
    };

    /**
     * @name 数值类型转换器
     * @brief 各种数值类型的转换器特化
     * @{
     */
    
    /**
     * @brief 转换为int类型的转换器（C字符串版本）
     * 
     * 使用标准库的atoi函数进行转换。
     * 
     * @tparam From 源类型（C字符串或兼容类型）
     */
    template <typename From>
    struct Converter<int, From>
    {
        /**
         * @brief 执行转换
         * @param from 源值
         * @return int 转换后的整数值
         */
        static int convert(const From& from)
        {
            return std::atoi(from);
        }
    };

    /**
     * @brief 转换为int类型的转换器（std::string特化版本）
     * 
     * 专门针对std::string类型的优化版本。
     */
    template<>
    struct Converter<int, std::string>
    {
        /**
         * @brief 执行转换
         * @param from std::string源值
         * @return int 转换后的整数值
         */
        static int convert(const std::string& from)
        {
            return std::atoi(from.c_str());
        }
    };

    /**
     * @brief 转换为long类型的转换器（C字符串版本）
     * 
     * 使用标准库的atol函数进行转换。
     * 
     * @tparam From 源类型（C字符串或兼容类型）
     */
    template <typename From>
    struct Converter<long, From>
    {
        /**
         * @brief 执行转换
         * @param from 源值
         * @return long 转换后的长整数值
         */
        static long convert(const From& from)
        {
            return std::atol(from);
        }
    };

    /**
     * @brief 转换为long类型的转换器（std::string特化版本）
     */
    template <>
    struct Converter<long, std::string>
    {
        /**
         * @brief 执行转换
         * @param from std::string源值
         * @return long 转换后的长整数值
         */
        static long convert(const std::string& from)
        {
            return std::atol(from.c_str());
        }
    };

    /**
     * @brief 转换为long long类型的转换器（C字符串版本）
     * 
     * 根据编译器版本选择合适的转换函数。
     * VS2012及以下版本使用_atoi64，其他使用atoll。
     * 
     * @tparam From 源类型（C字符串或兼容类型）
     */
    template <typename From>
    struct Converter<long long, From>
    {
        /**
         * @brief 执行转换
         * @param from 源值
         * @return long long 转换后的64位整数值
         */
        static long long convert(const From& from)
        {
#if defined(_MSC_VER) && _MSC_VER <= 1700 // vs2012
            return _atoi64(from);
#else
            return std::atoll(from);
#endif
        }
    };

    /**
     * @brief 转换为long long类型的转换器（std::string特化版本）
     */
    template <>
    struct Converter<long long, std::string>
    {
        /**
         * @brief 执行转换
         * @param from std::string源值
         * @return long long 转换后的64位整数值
         */
        static long long convert(const std::string& from)
        {
#if defined(_MSC_VER) && _MSC_VER <= 1700 // vs2012
            return _atoi64(from.c_str());
#else
            return std::atoll(from.c_str());
#endif
        }
    };

    /**
     * @brief 转换为double类型的转换器（C字符串版本）
     * 
     * 使用标准库的atof函数进行转换。
     * 
     * @tparam From 源类型（C字符串或兼容类型）
     */
    template <typename From>
    struct Converter<double, From>
    {
        /**
         * @brief 执行转换
         * @param from 源值
         * @return double 转换后的双精度浮点数值
         */
        static double convert(const From& from)
        {
            return std::atof(from);
        }
    };

    /**
     * @brief 转换为double类型的转换器（std::string特化版本）
     */
    template <>
    struct Converter<double, std::string>
    {
        /**
         * @brief 执行转换
         * @param from std::string源值
         * @return double 转换后的双精度浮点数值
         */
        static double convert(const std::string& from)
        {
            return std::atof(from.c_str());
        }
    };

    /**
     * @brief 转换为float类型的转换器（C字符串版本）
     * 
     * 专门针对float类型的优化版本。
     * 
     * @tparam From 源类型（C字符串或兼容类型）
     */
    template <typename From>
    struct Converter<float, From>
    {
        /**
         * @brief 执行转换
         * @param from 源值
         * @return float 转换后的单精度浮点数值
         */
        static float convert(const From& from)
        {
            return (float)std::atof(from);
        }
    };

    /**
     * @brief 转换为float类型的转换器（std::string特化版本）
     */
    template <>
    struct Converter<float, std::string>
    {
        /**
         * @brief 执行转换
         * @param from std::string源值
         * @return float 转换后的单精度浮点数值
         */
        static float convert(const std::string& from)
        {
            return float(std::atof(from.c_str()));
        }
    };

    //to bool
    template <typename From>
    struct Converter<bool, From>
    {
        static typename std::enable_if<std::is_integral<From>::value, bool>::type convert(From from)
        {
            return !!from;
        }
    };

    static bool checkbool(const char* from, const size_t len, const char* s)
    {
        for (size_t i = 0; i < len; i++)
        {
            if (from[i] != s[i])
            {
                return false;
            }
        }

        return true;
    }

    static bool convert(const char* from)
    {
        size_t len = strlen(from);
        //if (len != 4 && len != 5)
        //    throw std::invalid_argument("argument is invalid");

        bool r = true;
        if (len == 4)
        {// "true"
            r = checkbool(from, len, strue);

            if (r)
                return true;
        }
        else if (len == 5)
        {//"false"
            r = checkbool(from, len, sfalse);

            if (r)
                return false;
        }
        else
        {
            int value = Converter<int, const char*>::convert(from);
            return (value > 0);
        }

        throw std::invalid_argument("argument is invalid");
    }

    template <>
    struct Converter<bool, string>
    {
        static bool convert(const string& from)
        {
            return detail::convert(from.c_str());
        }
    };

    template <>
    struct Converter<bool, const char*>
    {
        static bool convert(const char* from)
        {
            return detail::convert(from);
        }
    };

    template <>
    struct Converter<bool, char*>
    {
        static bool convert(char* from)
        {
            return detail::convert(from);
        }
    };

    template <unsigned N>
    struct Converter<bool, const char[N]>
    {
        static bool convert(const char(&from)[N])
        {
            return detail::convert(from);
        }
    };

    template <unsigned N>
    struct Converter<bool, char[N]>
    {
        static bool convert(const char(&from)[N])
        {
            return detail::convert(from);
        }
    };

    //to string
    template <typename From>
    struct Converter<string, From>
    {
        static string convert(const From& from)
        {
            return std::to_string(from);
        }
    };
}

template <typename To, typename From>
typename std::enable_if<!std::is_same<To, From>::value, To>::type lexical_cast(const From& from)
{
    return detail::Converter<To, From>::convert(from);
}

template <typename To, typename From>
typename std::enable_if<std::is_same<To, From>::value, To>::type lexical_cast(const From& from)
{
    return from;
}

template<typename DTYPE>
bool NFStrTo(const std::string& value, DTYPE& nValue)
{
	try
	{
		nValue = lexical_cast<DTYPE>(value);
		return true;
	}
	catch (...)
	{
		return false;
	}

	return false;
}

//test code
//void test()
//{
//    cout<<lexical_cast<int>(1)<<endl;
//    cout << lexical_cast<int>("1") << endl;
//    cout << lexical_cast<long>("1") << endl;
//    cout << lexical_cast<string>(1) << endl;
//    cout << lexical_cast<bool>(1) << endl;
//    cout << lexical_cast<double>("1.2") << endl;
//    cout << lexical_cast<float>("1.2") << endl;
//    string s = "true";
//    cout << lexical_cast<bool>(s) << endl;
//    char* p = "false";
//    cout << lexical_cast<bool>(p) << endl;
//    const char* q = "false";
//    cout << lexical_cast<bool>(q) << endl;
//    cout << lexical_cast<bool>("false") << endl;
//    cout << lexical_cast<bool>("test") << endl;
//}
//
//int main()
//{
//    try
//    {
//        test();
//    }
//    catch (const std::exception& e)
//    {
//        cout << e.what() << endl;
//    }
//
//    return 0;
//}

#endif // !CPP_11_LEXICAL_CAST_HPP
