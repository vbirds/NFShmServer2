// -------------------------------------------------------------------------
//    @FileName         :    NFDeclareProperty.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFDeclareProperty.h
 * @brief 属性声明宏定义集合
 * 
 * 此文件提供了一套完整的属性声明宏，用于快速生成类的getter/setter方法。
 * 支持多种基本数据类型，包括布尔型、整型、浮点型和字符串型。
 * 这些宏简化了属性的声明和实现，确保代码的一致性和可维护性。
 */

#pragma once

/**
 * @brief 声明不可拷贝类
 * 
 * 通过声明私有的拷贝构造函数和赋值操作符来禁止类的拷贝操作。
 * 这是实现单例模式或管理资源类的常用技术。
 * 
 * @param Class 要禁止拷贝的类名
 * 
 * 使用示例：
 * @code
 * class MyClass {
 *     DECLARE_UNCOPYABLE(MyClass)
 * public:
 *     MyClass() = default;
 * };
 * @endcode
 */
#define DECLARE_UNCOPYABLE(Class) \
private: \
    Class(const Class&); \
    const Class& operator=(const Class&);

/**
 * @name 布尔型属性宏
 * @brief 用于声明布尔型属性的宏定义集合
 * @{
 */

/**
 * @brief 声明布尔型属性（自定义getter/setter名称）
 * 
 * 生成一个布尔型成员变量和对应的虚拟getter/setter方法。
 * 成员变量默认初始化为false。
 * 
 * @param name 属性名称，将生成m##name形式的成员变量
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 * 
 * 生成的代码结构：
 * - protected: bool m##name = false;
 * - public: virtual bool get_func() const;
 * - public: virtual void set_func(const bool value);
 */
#define DECLARE_PROPERTY_BOOL_VARIANT(name, get_func, set_func) \
    protected: \
    bool m##name = false; \
    public: \
    virtual bool get_func() const { return m##name; } \
    virtual void set_func(const bool value) \
    {\
        m##name = value;\
    }

/**
 * @brief 声明纯虚布尔型属性接口（自定义名称）
 * 
 * 仅声明纯虚的getter/setter方法，不生成成员变量。
 * 用于定义接口类或抽象基类。
 * 
 * @param name 属性名称（仅用于宏参数一致性）
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_BOOL_VARIANT_NULL(name, get_func, set_func) \
    public: \
    virtual bool get_func() const = 0; \
    virtual void set_func(const bool value) = 0;\

/**
 * @brief 声明布尔型属性（标准命名）
 * 
 * 使用标准的Get##name/Set##name命名方式声明布尔型属性。
 * 
 * @param name 属性名称，将生成Get##name()和Set##name()方法
 * 
 * 使用示例：
 * @code
 * class MyClass {
 *     DECLARE_PROPERTY_BOOL(Enable)  // 生成GetEnable()和SetEnable()
 * };
 * @endcode
 */
#define DECLARE_PROPERTY_BOOL(name) DECLARE_PROPERTY_BOOL_VARIANT(name, Get##name, Set##name)

/**
 * @brief 声明纯虚布尔型属性接口（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_BOOL_NULL(name) DECLARE_PROPERTY_BOOL_VARIANT_NULL(name, Get##name, Set##name)

/** @} */

/**
 * @name 32位有符号整型属性宏
 * @brief 用于声明int32_t类型属性的宏定义集合
 * @{
 */

/**
 * @brief 声明32位有符号整型属性（自定义getter/setter名称）
 * 
 * 生成一个int32_t型成员变量和对应的虚拟getter/setter方法。
 * 成员变量默认初始化为0。
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_INT32_VARIANT(name, get_func, set_func) \
    protected: \
    int32_t m##name = 0; \
    public: \
    virtual int32_t get_func() const { return m##name; } \
    virtual void set_func(const int32_t value) \
    {\
        m##name = value;\
    }

/**
 * @brief 声明纯虚32位有符号整型属性接口（自定义名称）
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_INT32_VARIANT_NULL(name, get_func, set_func) \
    public: \
    virtual int32_t get_func() const = 0; \
    virtual void set_func(const int32_t value) = 0;\

/**
 * @brief 声明32位有符号整型属性（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_INT32(name) DECLARE_PROPERTY_INT32_VARIANT(name, Get##name, Set##name)

/**
 * @brief 声明纯虚32位有符号整型属性接口（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_INT32_NULL(name) DECLARE_PROPERTY_UINT32_VARIANT_NULL(name, Get##name, Set##name)

/** @} */

/**
 * @name 32位无符号整型属性宏
 * @brief 用于声明uint32_t类型属性的宏定义集合
 * @{
 */

/**
 * @brief 声明32位无符号整型属性（自定义getter/setter名称）
 * 
 * 生成一个uint32_t型成员变量和对应的虚拟getter/setter方法。
 * 成员变量默认初始化为0。
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_UINT32_VARIANT(name, get_func, set_func) \
    protected: \
    uint32_t m##name = 0; \
    public: \
    virtual uint32_t get_func() const { return m##name; } \
    virtual void set_func(const uint32_t value) \
    {\
        m##name = value;\
    }

/**
 * @brief 声明纯虚32位无符号整型属性接口（自定义名称）
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_UINT32_VARIANT_NULL(name, get_func, set_func) \
    public: \
    virtual uint32_t get_func() const = 0; \
    virtual void set_func(const uint32_t value) = 0;\

/**
 * @brief 声明32位无符号整型属性（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_UINT32(name) DECLARE_PROPERTY_UINT32_VARIANT(name, Get##name, Set##name)

/**
 * @brief 声明纯虚32位无符号整型属性接口（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_UINT32_NULL(name) DECLARE_PROPERTY_UINT32_VARIANT_NULL(name, Get##name, Set##name)

/** @} */

///////////////////////////////////////////////////////////////////////////

/**
 * @name 64位有符号整型属性宏
 * @brief 用于声明int64_t类型属性的宏定义集合
 * @{
 */

/**
 * @brief 声明64位有符号整型属性（自定义getter/setter名称）
 * 
 * 生成一个int64_t型成员变量和对应的虚拟getter/setter方法。
 * 成员变量默认初始化为0。
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_INT64_VARIANT(name, get_func, set_func) \
    protected: \
    int64_t m##name = 0; \
    public: \
    virtual int64_t get_func() const { return m##name; } \
    virtual void set_func(const int64_t value) \
    {\
        m##name = value;\
    }

/**
 * @brief 声明纯虚64位有符号整型属性接口（自定义名称）
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_INT64_VARIANT_NULL(name, get_func, set_func) \
    public: \
    virtual int64_t get_func() const = 0; \
    virtual void set_func(const int64_t value) = 0;\

/**
 * @brief 声明64位有符号整型属性（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_INT64(name) DECLARE_PROPERTY_INT64_VARIANT(name, Get##name, Set##name)

/**
 * @brief 声明纯虚64位有符号整型属性接口（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_INT64_NULL(name) DECLARE_PROPERTY_INT64_VARIANT_NULL(name, Get##name, Set##name)
///////////////////////////////////////////////////////////////////////////

/**
 * @name 64位无符号整型属性宏
 * @brief 用于声明uint64_t类型属性的宏定义集合
 * @{
 */

/**
 * @brief 声明64位无符号整型属性（自定义getter/setter名称）
 * 
 * 生成一个uint64_t型成员变量和对应的虚拟getter/setter方法。
 * 成员变量默认初始化为0。
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_UINT64_VARIANT(name, get_func, set_func) \
    protected: \
    uint64_t m##name = 0; \
    public: \
    virtual uint64_t get_func() const { return m##name; } \
    virtual void set_func(const uint64_t value) \
    {\
        m##name = value;\
    }

/**
 * @brief 声明纯虚64位无符号整型属性接口（自定义名称）
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_UINT64_VARIANT_NULL(name, get_func, set_func) \
    public: \
    virtual uint64_t get_func() const = 0; \
    virtual void set_func(const uint64_t value) = 0;\

/**
 * @brief 声明64位无符号整型属性（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_UINT64(name) DECLARE_PROPERTY_UINT64_VARIANT(name, Get##name, Set##name)

/**
 * @brief 声明纯虚64位无符号整型属性接口（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_UINT64_NULL(name) DECLARE_PROPERTY_UINT64_VARIANT_NULL(name, Get##name, Set##name)
///////////////////////////////////////////////////////////////////////////

/**
 * @name 浮点型属性宏
 * @brief 用于声明浮点型属性的宏定义集合
 * @{
 */

/**
 * @brief 声明浮点型属性（自定义getter/setter名称）
 * 
 * 生成一个浮点型成员变量和对应的虚拟getter/setter方法。
 * 成员变量默认初始化为0。
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_FLOAT_VARIANT(name, get_func, set_func) \
    protected: \
    float m##name = 0; \
    public: \
    virtual float get_func() const { return m##name; } \
    virtual void set_func(const float value) \
    {\
        m##name = value;\
    }

/**
 * @brief 声明纯虚浮点型属性接口（自定义名称）
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_FLOAT_VARIANT_NULL(name, get_func, set_func) \
    public: \
    virtual float get_func() const = 0; \
    virtual void set_func(const float value) = 0; \

/**
 * @brief 声明浮点型属性（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_FLOAT(name) DECLARE_PROPERTY_FLOAT_VARIANT(name, Get##name, Set##name)

/**
 * @brief 声明纯虚浮点型属性接口（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_FLOAT_NULL(name) DECLARE_PROPERTY_FLOAT_VARIANT_NULL(name, Get##name, Set##name)

/** @} */

///////////////////////////////////////////////////////////////////////////

/**
 * @name 双精度浮点型属性宏
 * @brief 用于声明double类型属性的宏定义集合
 * @{
 */

/**
 * @brief 声明双精度浮点型属性（自定义getter/setter名称）
 * 
 * 生成一个双精度浮点型成员变量和对应的虚拟getter/setter方法。
 * 成员变量默认初始化为0。
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_DOUBLE_VARIANT(name, get_func, set_func) \
    protected: \
    double m##name = 0; \
    public: \
    virtual double get_func() const { return m##name; } \
    virtual void set_func(const double value) \
    {\
        m##name = value;\
    }

/**
 * @brief 声明纯虚双精度浮点型属性接口（自定义名称）
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_DOUBLE_VARIANT_NULL(name, get_func, set_func) \
    public: \
    virtual double get_func() const = 0;\
    virtual void set_func(const double value) = 0; \

/**
 * @brief 声明双精度浮点型属性（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_DOUBLE(name) DECLARE_PROPERTY_DOUBLE_VARIANT(name, Get##name, Set##name)

/**
 * @brief 声明纯虚双精度浮点型属性接口（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_DOUBLE_NULL(name) DECLARE_PROPERTY_DOUBLE_VARIANT_NULL(name, Get##name, Set##name)
///////////////////////////////////////////////////////////////////////////

/**
 * @name 字符串型属性宏
 * @brief 用于声明字符串型属性的宏定义集合
 * @{
 */

/**
 * @brief 声明字符串型属性（自定义getter/setter名称）
 * 
 * 生成一个字符串型成员变量和对应的虚拟getter/setter方法。
 * 成员变量默认初始化为空字符串。
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_STRING_VARIANT(name, get_func, set_func) \
    protected: \
    std::string m##name; \
    public: \
    virtual const std::string& get_func() const { return m##name; } \
    virtual void set_func(const std::string& value) \
    {\
        m##name = value;\
    }

/**
 * @brief 声明纯虚字符串型属性接口（自定义名称）
 * 
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_STRING_VARIANT_NULL(name, get_func, set_func) \
    public: \
    virtual const std::string& get_func() const = 0;\
    virtual void set_func(const std::string& value) = 0;\

/**
 * @brief 声明字符串型属性（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_STRING(name) DECLARE_PROPERTY_STRING_VARIANT(name, Get##name, Set##name)

/**
 * @brief 声明纯虚字符串型属性接口（标准命名）
 * 
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_STRING_NULL(name) DECLARE_PROPERTY_STRING_VARIANT_NULL(name, Get##name, Set##name)
///////////////////////////////////////////////////////////////////////////

/**
 * @name 对象型属性宏
 * @brief 用于声明对象型属性的宏定义集合
 * @{
 */

/**
 * @brief 声明对象型属性（指针类型）
 * 
 * 生成一个对象指针型成员变量和对应的虚拟getter/setter方法。
 * 成员变量默认初始化为nullptr。
 * 
 * @param object_type 对象类型
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_OBJECT_VARIANT_PTR(object_type, name, get_func, set_func) \
    protected: \
    object_type* m##name = nullptr; \
    public: \
    virtual const object_type* get_func() const { return m##name; } \
    virtual object_type* get_func() { return m##name; } \
    virtual void set_func(object_type* value) \
    {\
        m##name = value;\
    }

/**
 * @brief 声明纯虚对象型属性接口（指针类型）
 * 
 * @param object_type 对象类型
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_OBJECT_VARIANT_PTR_NULL(object_type, name, get_func, set_func) \
    public: \
    virtual const object_type* get_func() const = 0; \
    virtual object_type* get_func() = 0; \
    virtual void set_func(object_type* value) = 0; \

/**
 * @brief 声明对象型属性（指针类型）
 * 
 * @param object_type 对象类型
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_OBJECT_PTR(object_type, name) DECLARE_PROPERTY_OBJECT_VARIANT_PTR(object_type, name, Get##name, Set##name)

/**
 * @brief 声明纯虚对象型属性接口（指针类型）
 * 
 * @param object_type 对象类型
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_OBJECT_PTR_NULL(object_type, name) DECLARE_PROPERTY_OBJECT_VARIANT_PTR_NULL(object_type, name, Get##name, Set##name)
///////////////////////////////////////////////////////////////////////////

/**
 * @brief 声明对象型属性（值类型）
 * 
 * 生成一个对象值类型成员变量和对应的虚拟getter/setter方法。
 * 
 * @param object_type 对象类型
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_OBJECT_VARIANT(object_type, name, get_func, set_func) \
    protected: \
    object_type m##name; \
    public: \
    virtual const object_type& get_func() const { return m##name; } \
    virtual object_type& get_func() { return m##name; } \
    virtual void set_func(const object_type& value) \
    {\
        m##name = value;\
    }

/**
 * @brief 声明纯虚对象型属性接口（值类型）
 * 
 * @param object_type 对象类型
 * @param name 属性名称
 * @param get_func getter方法名称
 * @param set_func setter方法名称
 */
#define DECLARE_PROPERTY_OBJECT_VARIANT_NULL(object_type, name, get_func, set_func) \
    public: \
    virtual const object_type& get_func() const = 0; \
    virtual object_type& get_func() = 0; \
    virtual void set_func(const object_type& value) = 0; \

/**
 * @brief 声明对象型属性（值类型）
 * 
 * @param object_type 对象类型
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_OBJECT(object_type, name) DECLARE_PROPERTY_OBJECT_VARIANT(object_type, name, Get##name, Set##name)

/**
 * @brief 声明纯虚对象型属性接口（值类型）
 * 
 * @param object_type 对象类型
 * @param name 属性名称
 */
#define DECLARE_PROPERTY_OBJECT_NULL(object_type, name) DECLARE_PROPERTY_OBJECT_VARIANT_NULL(object_type, name, Get##name, Set##name)
///////////////////////////////////////////////////////////////////////////


