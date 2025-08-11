// -------------------------------------------------------------------------
//    @FileName         :    NFCmdLine.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFCmdLine.h
 * @brief 命令行参数解析器
 * 
 * 此文件提供了一个功能强大的命令行参数解析库。
 * 支持多种数据类型的参数解析，包括必选参数、可选参数、标志参数等。
 * 提供类型安全的参数转换和友好的错误处理机制。
 */

#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <cstring>
#include <algorithm>
#include <cstdlib>
#include "NFPlatform.h"
#include "NFStringUtility.h"

/**
 * @brief 命令行解析命名空间
 * 
 * NFCmdLine命名空间包含了命令行参数解析的所有功能。
 * 提供了类型安全的参数解析、验证和转换机制。
 * 
 * 主要功能：
 * - 支持多种参数类型：字符串、整数、浮点数、布尔值等
 * - 支持必选参数和可选参数
 * - 支持参数验证和约束
 * - 支持帮助信息自动生成
 * - 提供友好的错误处理
 * 
 * 使用方法：
 * @code
 * // 创建解析器
 * NFCmdLine::parser parser("MyProgram", "1.0");
 * 
 * // 添加参数定义
 * parser.add<std::string>("input", 'i', "input file", true);
 * parser.add<int>("port", 'p', "server port", false, 8080);
 * parser.add("verbose", 'v', "verbose mode");
 * 
 * // 解析命令行
 * parser.parse(argc, argv);
 * 
 * // 获取参数值
 * std::string input = parser.get<std::string>("input");
 * int port = parser.get<int>("port");
 * bool verbose = parser.exist("verbose");
 * @endcode
 */
namespace NFCmdLine
{
	/**
	 * @brief 内部实现命名空间
	 * 
	 * NFDetail包含了命令行解析器的内部实现细节，
	 * 包括类型转换、类型萃取等辅助功能。
	 */
	namespace NFDetail
	{
		/**
		 * @brief 词法转换模板类
		 * 
		 * 提供类型安全的字符串与其他类型之间的转换功能。
		 * 
		 * @tparam Target 目标类型
		 * @tparam Source 源类型
		 * @tparam Same 是否为相同类型
		 */
		template <typename Target, typename Source, bool Same>
		class lexical_cast_t
		{
		public:
			/**
			 * @brief 类型转换函数
			 * 
			 * 将源类型转换为目标类型。
			 * 
			 * @param arg 源值
			 * @return Target 转换后的目标值
			 * @throws std::bad_cast 转换失败时抛出异常
			 */
			static Target cast(const Source& arg)
			{
				Target ret;
				std::stringstream ss;
				if (!(ss << arg && ss >> ret && ss.eof()))
					throw std::bad_cast();

				return ret;
			}
		};

		/**
		 * @brief 相同类型的词法转换特化
		 * 
		 * 当源类型和目标类型相同时，直接返回原值。
		 * 
		 * @tparam Target 目标类型
		 * @tparam Source 源类型
		 */
		template <typename Target, typename Source>
		class lexical_cast_t<Target, Source, true>
		{
		public:
			/**
			 * @brief 相同类型转换
			 * 
			 * @param arg 源值
			 * @return Target 直接返回源值
			 */
			static Target cast(const Source& arg)
			{
				return arg;
			}
		};

		/**
		 * @brief 转换为字符串的特化
		 * 
		 * 将任意类型转换为字符串。
		 * 
		 * @tparam Source 源类型
		 */
		template <typename Source>
		class lexical_cast_t<std::string, Source, false>
		{
		public:
			/**
			 * @brief 转换为字符串
			 * 
			 * @param arg 源值
			 * @return std::string 转换后的字符串
			 */
			static std::string cast(const Source& arg)
			{
				std::ostringstream ss;
				ss << arg;
				return ss.str();
			}
		};

		/**
		 * @brief 从字符串转换的特化
		 * 
		 * 将字符串转换为指定类型。
		 * 
		 * @tparam Target 目标类型
		 */
		template <typename Target>
		class lexical_cast_t<Target, std::string, false>
		{
		public:
			/**
			 * @brief 从字符串转换
			 * 
			 * @param arg 源字符串
			 * @return Target 转换后的目标值
			 * @throws std::bad_cast 转换失败时抛出异常
			 */
			static Target cast(const std::string& arg)
			{
				Target ret;
				std::istringstream ss(arg);
				if (!(ss >> ret && ss.eof()))
					throw std::bad_cast();
				return ret;
			}
		};

		/**
		 * @brief 类型相同性检测模板
		 * 
		 * 用于在编译时检测两个类型是否相同。
		 * 
		 * @tparam T1 第一个类型
		 * @tparam T2 第二个类型
		 */
		template <typename T1, typename T2>
		struct is_same
		{
			/** @brief 类型不同时为false */
			static const bool value = false;
		};

		/**
		 * @brief 相同类型的特化
		 * 
		 * @tparam T 类型
		 */
		template <typename T>
		struct is_same<T, T>
		{
			/** @brief 相同类型时为true */
			static const bool value = true;
		};

		/**
		 * @brief 词法转换包装函数
		 * 
		 * 根据类型是否相同选择合适的转换策略。
		 * 
		 * @tparam Target 目标类型
		 * @tparam Source 源类型
		 * @param arg 源值
		 * @return Target 转换后的目标值
		 */
		template <typename Target, typename Source>
		Target lexical_cast_x(const Source& arg)
		{
			return lexical_cast_t<Target, Source, NFDetail::is_same<Target, Source>::value>::cast(arg);
		}

		/**
		 * @brief 类型名称解析函数
		 * 
		 * 获取类型的可读名称。
		 * 
		 * @param name 类型名称字符串
		 * @return std::string 解析后的类型名称
		 */
		static inline std::string demangle(const std::string& name)
		{
			return name;
		}

		template <class T>
		std::string readable_typename()
		{
			return demangle(typeid(T).name());
		}

		template <class T>
		std::string default_value(T def)
		{
			return NFDetail::lexical_cast_x<std::string>(def);
		}

		template <>
		inline std::string readable_typename<std::string>()
		{
			return "string";
		}
	} // detail

	//-----

	class NFCmdLine_Error : public std::exception
	{
	public:
		explicit NFCmdLine_Error(const std::string& msg) : msg(msg)
		{
		}

		virtual ~NFCmdLine_Error() throw()
		{
		}

		const char* what() const throw() override
		{
			return msg.c_str();
		}

	private:
		std::string msg;
	};

	template <class T>
	struct NFDefaultReader
	{
		T operator()(const std::string& str)
		{
			return NFDetail::lexical_cast_x<T>(str);
		}
	};

	template <class T>
	struct NFRangeReader
	{
		NFRangeReader(const T& low, const T& high) : low(low), high(high)
		{
		}

		T operator()(const std::string& s) const
		{
			T ret = NFDefaultReader<T>()(s);
			if (!(ret >= low && ret <= high)) throw NFCmdLine::NFCmdLine_Error("range_error");
			return ret;
		}

	private:
		T low, high;
	};

	template <class T>
	NFRangeReader<T> NFRange(const T& low, const T& high)
	{
		return NFRangeReader<T>(low, high);
	}

	template <class T>
	struct NFOneOfReader
	{
		T operator()(const std::string& s)
		{
			T ret = NFDefaultReader<T>()(s);
			if (std::find(alt.begin(), alt.end(), ret) == alt.end())
				throw NFCmdLine_Error("");
			return ret;
		}

		void Add(const T& v)
		{
			alt.push_back(v);
		}

	private:
		std::vector<T> alt;
	};

	template <class T>
	NFOneOfReader<T> OneOf(T a1)
	{
		NFOneOfReader<T> ret;
		ret.Add(a1);
		return ret;
	}

	template <class T>
	NFOneOfReader<T> OneOf(T a1, T a2)
	{
		NFOneOfReader<T> ret;
		ret.Add(a1);
		ret.Add(a2);
		return ret;
	}

	template <class T>
	NFOneOfReader<T> OneOf(T a1, T a2, T a3)
	{
		NFOneOfReader<T> ret;
		ret.Add(a1);
		ret.Add(a2);
		ret.Add(a3);
		return ret;
	}

	template <class T>
	NFOneOfReader<T> OneOf(T a1, T a2, T a3, T a4)
	{
		NFOneOfReader<T> ret;
		ret.Add(a1);
		ret.Add(a2);
		ret.Add(a3);
		ret.Add(a4);
		return ret;
	}

	template <class T>
	NFOneOfReader<T> OneOf(T a1, T a2, T a3, T a4, T a5)
	{
		NFOneOfReader<T> ret;
		ret.Add(a1);
		ret.Add(a2);
		ret.Add(a3);
		ret.Add(a4);
		ret.Add(a5);
		return ret;
	}

	template <class T>
	NFOneOfReader<T> OneOf(T a1, T a2, T a3, T a4, T a5, T a6)
	{
		NFOneOfReader<T> ret;
		ret.Add(a1);
		ret.Add(a2);
		ret.Add(a3);
		ret.Add(a4);
		ret.Add(a5);
		ret.Add(a6);
		return ret;
	}

	template <class T>
	NFOneOfReader<T> OneOf(T a1, T a2, T a3, T a4, T a5, T a6, T a7)
	{
		NFOneOfReader<T> ret;
		ret.Add(a1);
		ret.Add(a2);
		ret.Add(a3);
		ret.Add(a4);
		ret.Add(a5);
		ret.Add(a6);
		ret.Add(a7);
		return ret;
	}

	template <class T>
	NFOneOfReader<T> OneOf(T a1, T a2, T a3, T a4, T a5, T a6, T a7, T a8)
	{
		NFOneOfReader<T> ret;
		ret.Add(a1);
		ret.Add(a2);
		ret.Add(a3);
		ret.Add(a4);
		ret.Add(a5);
		ret.Add(a6);
		ret.Add(a7);
		ret.Add(a8);
		return ret;
	}

	template <class T>
	NFOneOfReader<T> OneOf(T a1, T a2, T a3, T a4, T a5, T a6, T a7, T a8, T a9)
	{
		NFOneOfReader<T> ret;
		ret.Add(a1);
		ret.Add(a2);
		ret.Add(a3);
		ret.Add(a4);
		ret.Add(a5);
		ret.Add(a6);
		ret.Add(a7);
		ret.Add(a8);
		ret.Add(a9);
		return ret;
	}

	template <class T>
	NFOneOfReader<T> OneOf(T a1, T a2, T a3, T a4, T a5, T a6, T a7, T a8, T a9, T a10)
	{
		NFOneOfReader<T> ret;
		ret.Add(a1);
		ret.Add(a2);
		ret.Add(a3);
		ret.Add(a4);
		ret.Add(a5);
		ret.Add(a6);
		ret.Add(a7);
		ret.Add(a8);
		ret.Add(a9);
		ret.Add(a10);
		return ret;
	}

	//-----
	class NFOptionBase;

	class NFParser
	{
	public:
		NFParser()
		{
		}

		~NFParser()
		{
			for (auto p = options.begin(); p != options.end(); ++p)
			{
				delete p->second;
			}
		}

		void Add(const std::string& name,
		         char short_name = 0,
		         const std::string& desc = "")
		{
			std::string lowerName = name;
			NFStringUtility::ToLower(lowerName);
			if (options.count(lowerName)) throw NFCmdLine_Error("multiple definition: " + lowerName);
			options[lowerName] = new NFOptionWithoutValue(lowerName, short_name, desc);
			ordered.push_back(options[lowerName]);
		}

		template <class T>
		void Add(const std::string& name,
		         char short_name = 0,
		         const std::string& desc = "",
		         bool need = true,
		         const T def = T())
		{
			std::string lowerName = name;
			NFStringUtility::ToLower(lowerName);
			Add(lowerName, short_name, desc, need, def, NFDefaultReader<T>());
		}

		template <class T, class F>
		void Add(const std::string& name,
		         char short_name = 0,
		         const std::string& desc = "",
		         bool need = true,
		         const T def = T(),
		         F reader = NFDefaultReader<T>())
		{
			std::string lowerName = name;
			NFStringUtility::ToLower(lowerName);
			if (options.count(lowerName)) throw NFCmdLine_Error("multiple definition: " + lowerName);
			options[lowerName] = new NFOptionWithValueWithReader<T, F>(lowerName, short_name, need, def, desc, reader);
			ordered.push_back(options[lowerName]);
		}

		void Footer(const std::string& f)
		{
			ftr = f;
		}

		void SetProgramName(const std::string& name)
		{
			prog_name = name;
		}

		std::string GetProgramName() const
        {
		    return prog_name;
        }

		bool Exist(const std::string& name) const
		{
			std::string lowerName = name;
			NFStringUtility::ToLower(lowerName);
			if (options.count(lowerName) == 0) throw NFCmdLine_Error("there is no flag: --" + lowerName);
			auto it = options.find(lowerName);
			if (it != options.end())
			{
				if (it->second)
				{
					return it->second->has_set();
				}
			}
			return false;
		}

		template <class T>
		const T& Get(const std::string& name) const
		{
			std::string lowerName = name;
			NFStringUtility::ToLower(lowerName);

			auto it = options.find(lowerName);
			if (it == options.end()) throw NFCmdLine_Error("there is no flag: --" + lowerName);
			if (it->second == nullptr) throw NFCmdLine_Error("there is no flag: --" + lowerName);

			const NFOptionWithValue<T>* p = dynamic_cast<const NFOptionWithValue<T>*>(it->second);
			if (p == nullptr) throw NFCmdLine_Error("type mismatch flag '" + lowerName + "'");

			return p->get();
		}

		const std::vector<std::string>& Rest() const
		{
			return others;
		}

		bool Parse(const std::string& arg)
		{
			std::vector<std::string> args;

			std::string buf;
			bool in_quote = false;
			for (std::string::size_type i = 0; i < arg.length(); i++)
			{
				if (arg[i] == '\"')
				{
					in_quote = !in_quote;
					continue;
				}

				if (arg[i] == ' ' && !in_quote)
				{
					args.push_back(buf);
					buf.clear();
					continue;
				}

				if (arg[i] == '\\')
				{
					i++;
					if (i >= arg.length())
					{
						errors.push_back("unexpected occurrence of '\\' at end of string");
						return false;
					}
				}

				buf += arg[i];
			}

			if (in_quote)
			{
				errors.push_back("quote is not closed");
				return false;
			}

			if (buf.length() > 0)
				args.push_back(buf);

			//for (size_t i = 0; i < args.size(); i++)
			//	std::cout << "\"" << args[i] << "\"" << std::endl;

			return Parse(args);
		}

		bool Parse(const std::vector<std::string>& args)
		{
			int argc = static_cast<int>(args.size());
			std::vector<const char*> argv(argc);

			for (int i = 0; i < argc; i++)
				argv[i] = args[i].c_str();

			return Parse(argc, &argv[0]);
		}

		bool Parse(int argc, const char* const argv[])
		{
			errors.clear();
			others.clear();

			if (argc < 1)
			{
				errors.push_back("argument number must be longer than 0");
				return false;
			}
			if (prog_name.empty())
				prog_name = argv[0];

			std::map<char, std::string> lookup;
			for (auto p = options.begin(); p != options.end(); ++p)
			{
				if (p->first.length() == 0) continue;
				char initial = p->second->short_name();
				if (initial)
				{
					if (lookup.count(initial) > 0)
					{
						lookup[initial].clear();
						errors.push_back(std::string("short option '") + initial + "' is ambiguous");
						return false;
					}
					else lookup[initial] = p->first;
				}
			}

			for (int i = 1; i < argc; i++)
			{
				if (strncmp(argv[i], "--", 2) == 0)
				{
					const char* p = strchr(argv[i] + 2, '=');
					if (p)
					{
						std::string name(argv[i] + 2, p);
						NFStringUtility::ToLower(name);
						std::string val(p + 1);
						SetOption(name, val);
					}
					else
					{
						std::string name(argv[i] + 2);
						NFStringUtility::ToLower(name);
						if (options.count(name) == 0)
						{
							errors.push_back("undefined option: --" + name);
							continue;
						}
						if (options[name]->has_value())
						{
							if (i + 1 >= argc)
							{
								errors.push_back("option needs value: --" + name);
								continue;
							}
							else
							{
								i++;
								SetOption(name, argv[i]);
							}
						}
						else
						{
							SetOption(name);
						}
					}
				}
				else if (strncmp(argv[i], "-", 1) == 0)
				{
					if (!argv[i][1]) continue;
					char last = argv[i][1];
					for (int j = 2; argv[i][j]; j++)
					{
						last = argv[i][j];
						if (lookup.count(argv[i][j - 1]) == 0)
						{
							errors.push_back(std::string("undefined short option: -") + argv[i][j - 1]);
							continue;
						}
						if (lookup[argv[i][j - 1]].empty())
						{
							errors.push_back(std::string("ambiguous short option: -") + argv[i][j - 1]);
							continue;
						}
						SetOption(lookup[argv[i][j - 1]]);
					}

					if (lookup.count(last) == 0)
					{
						errors.push_back(std::string("undefined short option: -") + last);
						continue;
					}
					if (lookup[last].empty())
					{
						errors.push_back(std::string("ambiguous short option: -") + last);
						continue;
					}

					if (i + 1 < argc && options[lookup[last]]->has_value())
					{
						SetOption(lookup[last], argv[i + 1]);
						i++;
					}
					else
					{
						SetOption(lookup[last]);
					}
				}
				else
				{
					others.push_back(argv[i]);
				}
			}

			for (auto p = options.begin(); p != options.end(); ++p)
			{
				if (!p->second->valid())
				{
					errors.push_back("need option: --" + std::string(p->first));
				}
			}

			return errors.size() == 0;
		}

		bool ParseConsoleCommand(const std::string& arg)
		{
			if (!options.count("help"))
				Add("help", '?', "print this message");
			
			return Parse(arg);
		}

		void ParseCheck(const std::string& arg)
		{
			if (!options.count("help"))
				Add("help", '?', "print this message");
			Check(0, Parse(arg));
		}

		void ParseCheck(const std::vector<std::string>& args)
		{
			if (!options.count("help"))
				Add("help", '?', "print this message");
			Check((int)args.size(), Parse(args));
		}

		void ParseCheck(int argc, char* argv[])
		{
			if (!options.count("help"))
				Add("help", '?', "print this message");
			Check(argc, Parse(argc, argv));
		}

		void ClearParse()
		{
			errors.clear();
			others.clear();
			for (auto p = options.begin(); p != options.end(); ++p)
			{
				p->second->clear();
			}
		}

		std::string Error() const
		{
			return errors.size() > 0 ? errors[0] : "";
		}

		std::string ErrorFull() const
		{
			std::ostringstream oss;
			for (size_t i = 0; i < errors.size(); i++)
				oss << errors[i] << std::endl;
			return oss.str();
		}

		std::string Usage() const
		{
			std::ostringstream oss;
			oss << "usage: " << prog_name << " ";
			for (size_t i = 0; i < ordered.size(); i++)
			{
				if (ordered[i]->must())
					oss << ordered[i]->short_description() << " ";
			}

			oss << "[options] ... " << ftr << std::endl;
			oss << "options:" << std::endl;

			size_t max_width = 0;
			for (size_t i = 0; i < ordered.size(); i++)
			{
				max_width = (std::max)(max_width, ordered[i]->name().length());
			}
			for (size_t i = 0; i < ordered.size(); i++)
			{
				if (ordered[i]->short_name())
				{
					oss << "  -" << ordered[i]->short_name() << ", ";
				}
				else
				{
					oss << "      ";
				}

				oss << "--" << ordered[i]->name();
				for (size_t j = ordered[i]->name().length(); j < max_width + 4; j++)
					oss << ' ';
				oss << ordered[i]->description() << std::endl;
			}
			return oss.str();
		}

	private:
		void Check(int argc, bool ok) const
		{
			if ((argc == 1 && !ok) || Exist("help"))
			{
				std::cerr << Usage();
				exit(0);
			}

			if (!ok)
			{
				std::cerr << Error() << std::endl << Usage();
				NFSLEEP(1000);
				exit(1);
			}
		}

		void SetOption(const std::string& name)
		{
			std::string lowerName = name;
			NFStringUtility::ToLower(lowerName);
			if (options.count(lowerName) == 0)
			{
				errors.push_back("undefined option: --" + lowerName);
				return;
			}
			if (!options[lowerName]->set())
			{
				errors.push_back("option needs value: --" + lowerName);
				return;
			}
		}

		void SetOption(const std::string& name, const std::string& value)
		{
			std::string lowerName = name;
			NFStringUtility::ToLower(lowerName);
			if (options.count(lowerName) == 0)
			{
				errors.push_back("undefined option: --" + lowerName);
				return;
			}
			if (!options[lowerName]->set(value))
			{
				errors.push_back("option value is invalid: --" + lowerName + "=" + value);
				return;
			}
		}

		class NFOptionBase
		{
		public:
			virtual ~NFOptionBase()
			{
			}

			virtual bool has_value() const = 0;
			virtual bool set() = 0;
			virtual bool set(const std::string& value) = 0;
			virtual bool has_set() const = 0;
			virtual bool valid() const = 0;
			virtual bool must() const = 0;
			virtual void clear() = 0;

			virtual const std::string& name() const = 0;
			virtual char short_name() const = 0;
			virtual const std::string& description() const = 0;
			virtual std::string short_description() const = 0;
		};

		class NFOptionWithoutValue : public NFOptionBase
		{
		public:
			NFOptionWithoutValue(const std::string& name,
			                     char short_name,
			                     const std::string& desc)
				: nam(name), snam(short_name), desc(desc), has(false)
			{
			}

			~NFOptionWithoutValue()
			{
			}

			virtual bool has_value() const override
			{
				return false;
			}

			virtual bool set() override
			{
				has = true;
				return true;
			}

			virtual bool set(const std::string&) override
			{
				return false;
			}

			virtual bool has_set() const override
			{
				return has;
			}

			virtual bool valid() const override
			{
				return true;
			}

			virtual bool must() const override
			{
				return false;
			}

			virtual void clear() override
			{
				has = false;
			}

			virtual const std::string& name() const override
			{
				return nam;
			}

			virtual char short_name() const override
			{
				return snam;
			}

			virtual const std::string& description() const override
			{
				return desc;
			}

			virtual std::string short_description() const override
			{
				return "--" + nam;
			}

		private:
			std::string nam;
			char snam;
			std::string desc;
			bool has;
		};

		template <class T>
		class NFOptionWithValue : public NFOptionBase
		{
		public:
			NFOptionWithValue(const std::string& name,
			                  char short_name,
			                  bool need,
			                  const T& def,
			                  const std::string& desc)
				: nam(name), snam(short_name), need(need), has(false)
				  , def(def), actual(def)
			{
				this->desc = full_description(desc);
			}

			~NFOptionWithValue()
			{
			}

			const T& get() const
			{
				return actual;
			}

			virtual bool has_value() const override
			{
				return true;
			}

			virtual bool set() override
			{
				return false;
			}

			virtual bool set(const std::string& value) override
			{
				try
				{
					actual = read(value);
					has = true;
				}
				catch (const std::exception& e)
				{
					std::cout << e.what() << std::endl;
					return false;
				}
				return true;
			}

			virtual bool has_set() const override
			{
				return has;
			}

			virtual bool valid() const override
			{
				if (need && !has) return false;
				return true;
			}

			virtual bool must() const override
			{
				return need;
			}

			virtual const std::string& name() const override
			{
				return nam;
			}

			virtual char short_name() const override
			{
				return snam;
			}

			virtual const std::string& description() const override
			{
				return desc;
			}

			virtual std::string short_description() const override
			{
				return "--" + nam + "=" + NFDetail::readable_typename<T>();
			}

			virtual void clear() override
			{
				has = false;
			}
		protected:
			std::string full_description(const std::string& xdesc)
			{
				return
					xdesc + " (" + NFDetail::readable_typename<T>() +
					(need ? "" : " [=" + NFDetail::default_value<T>(def) + "]")
					+ ")";
			}

			virtual T read(const std::string& s) = 0;

			std::string nam;
			char snam;
			bool need;
			std::string desc;

			bool has;
			T def;
			T actual;
		};

		template <class T, class F>
		class NFOptionWithValueWithReader : public NFOptionWithValue<T>
		{
		public:
			NFOptionWithValueWithReader(const std::string& name,
			                            char short_name,
			                            bool need,
			                            const T def,
			                            const std::string& desc,
			                            F reader)
				: NFOptionWithValue<T>(name, short_name, need, def, desc), reader(reader)
			{
			}

		private:
			virtual T read(const std::string& s) override
			{
				return reader(s);
			}

			F reader;
		};

		std::map<std::string, NFOptionBase*> options;
		std::vector<NFOptionBase*> ordered;
		std::string ftr;

		std::string prog_name;
		std::vector<std::string> others;

		std::vector<std::string> errors;
	};
} // cmdline


