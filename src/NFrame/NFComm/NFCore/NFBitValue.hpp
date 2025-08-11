// -------------------------------------------------------------------------
//    @FileName         :    NFBitValue.hpp
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFPluginModule
//
// -------------------------------------------------------------------------

/**
 * @file NFBitValue.hpp
 * @brief 位值操作模板类
 * 
 * 此文件提供了对整数类型进行位级操作的工具类。支持位的设置、清除、
 * 查询等操作，可以用于标志位管理、权限控制、状态记录等场景。
 */

#pragma once

/**
 * @brief 位值操作模板类
 * 
 * NFBitValue提供了对任意内置整数类型的位级操作功能。
 * 可以将一个整数的每一位看作独立的布尔标志，支持对单个位的
 * 设置、清除、查询等操作。
 * 
 * 主要特性：
 * - 泛型设计：支持所有内置整数类型（int, uint16_t, uint64_t等）
 * - 位操作：提供完整的位设置、清除、查询功能
 * - 静态方法：支持对任意值进行位操作而不需要实例
 * - 高效实现：基于位运算，性能优异
 * - 边界检查：防止越界访问
 * 
 * 适用场景：
 * - 权限管理：每个位表示一种权限
 * - 状态标记：多个布尔状态的紧凑存储
 * - 配置选项：功能开关的集合
 * - 数据压缩：多个布尔值的压缩表示
 * 
 * 使用方法：
 * @code
 * // 使用32位整数进行位操作
 * NFBitValue<uint32_t> flags(0);
 * 
 * // 设置第0位和第5位
 * flags.SetBitValue(0);
 * flags.SetBitValue(5);
 * 
 * // 检查位是否设置
 * if (flags.HaveBitValue(0)) {
 *     std::cout << "第0位已设置" << std::endl;
 * }
 * 
 * // 清除第0位
 * flags.ClearBitValue(0);
 * 
 * // 查找第一个未设置的位
 * int firstEmpty = flags.GetFirstNoValueIndex();
 * 
 * // 静态方法使用
 * uint32_t value = 0;
 * NFBitValue<uint32_t>::SetBitValue(value, 3);
 * bool hasBit = NFBitValue<uint32_t>::HaveBitValue(value, 3);
 * @endcode
 * 
 * @tparam T 基础数据类型，必须是内置整数类型（如int, uint16_t, uint64_t等）
 * 
 * @note 位索引从0开始，0表示最低位
 * @note 支持的最大位数取决于类型T的大小
 */
template <class T>
class NFBitValue
{
public:
	/**
	 * @brief 位值状态枚举
	 * 
	 * 定义位操作的返回状态和特殊值。
	 */
	enum NFBitValueDefine
	{
		/** @brief 错误值，表示操作失败或参数无效 */
		ErrorValue = -1,
		/** @brief 无值，表示位未设置（为0） */
		NoneValue = 0,
		/** @brief 有值，表示位已设置（为1） */
		HaveValue = 1,
	};

	/**
	 * @brief 默认构造函数
	 * 
	 * 创建一个位值为0的对象，所有位都未设置。
	 */
	NFBitValue() : m_nValue(0)
	{
	}

	/**
	 * @brief 带初值的构造函数
	 * 
	 * 使用指定的值初始化位值对象。
	 * 
	 * @param nValue 初始值
	 */
	NFBitValue(T nValue) : m_nValue(nValue)
	{
	}

	/**
	 * @brief 设置整体值
	 * 
	 * 设置整个位值对象的数值，会影响所有位。
	 * 
	 * @param nValue 要设置的新值
	 */
	void SetValue(const T& nValue)
	{
		m_nValue = nValue;
	}

	/**
	 * @brief 获取整体值
	 * 
	 * 获取当前位值对象的完整数值。
	 * 
	 * @return T 当前的完整数值
	 */
	T GetValue() const
	{
		return m_nValue;
	}

	/**
	 * @brief 获取位长度
	 * 
	 * 返回类型T能表示的位数（即字节数乘以8）。
	 * 
	 * @return int 位长度
	 * 
	 * @note 这是一个静态方法，可以直接调用
	 */
	static int GetBitLength()
	{
		return sizeof(T) * 8;
	}

	/**
	 * @brief 获取第一个未设置位的索引
	 * 
	 * 从低位开始查找第一个值为0的位，返回其索引。
	 * 常用于分配空闲位置。
	 * 
	 * @return int 第一个未设置位的索引，如果所有位都已设置则返回-1
	 */
	int GetFirstNoValueIndex()
	{
		for (int i = 0; i < GetBitLength(); ++i)
		{
			if ((m_nValue & (T(1) << i)) == NoneValue)
			{
				return i;
			}
		}

		return -1;
	}

	/**
	 * @brief 获取指定位的值
	 * 
	 * 获取指定索引位置的位值（0或非0）。
	 * 
	 * @param nIndex 位索引（0开始）
	 * @return T 位值，ErrorValue表示索引无效，NoneValue表示未设置，其他表示已设置
	 */
	T GetBitValue(const int nIndex)
	{
		if (nIndex < 0 || nIndex >= GetBitLength())
		{
			return ErrorValue;
		}

		return (m_nValue & (T(1) << nIndex));
	}

	/**
	 * @brief 设置指定位
	 * 
	 * 将指定索引的位设置为1，不影响其他位。
	 * 
	 * @param nIndex 位索引（0开始）
	 * 
	 * @note 如果索引超出范围，操作无效果
	 */
	void SetBitValue(const int nIndex)
	{
		if (nIndex < 0 || nIndex >= GetBitLength())
		{
			return;
		}

		m_nValue |= (T(1) << nIndex);
	}

	/**
	 * @brief 清除指定位
	 * 
	 * 将指定索引的位设置为0，不影响其他位。
	 * 
	 * @param nIndex 位索引（0开始）
	 * 
	 * @note 如果索引超出范围，操作无效果
	 */
	void ClearBitValue(const int nIndex)
	{
		if (nIndex < 0 || nIndex >= GetBitLength())
		{
			return;
		}

		m_nValue &= ~(T(1) << nIndex);
	}

	/**
	 * @brief 检查指定位是否已设置
	 * 
	 * 检查指定索引的位是否为1。
	 * 
	 * @param nIndex 位索引（0开始）
	 * @return bool 位已设置返回true，否则返回false
	 */
	bool HaveBitValue(const int nIndex)
	{
		return GetBitValue(nIndex) != NoneValue;
	}

	/**
	 * @name 静态方法
	 * @brief 不需要实例的静态位操作方法
	 * 
	 * 这些静态方法允许直接对任意类型T的值进行位操作，
	 * 无需创建NFBitValue实例。
	 * @{
	 */
	
	/**
	 * @brief 获取指定值中第一个未设置位的索引（静态方法）
	 * 
	 * @param nValue 要检查的值
	 * @return int 第一个未设置位的索引，如果所有位都已设置则返回-1
	 */
	static int GetFirstNoValueIndex(const T& nValue)
	{
		for (int i = 0; i < GetBitLength(); ++i)
		{
			if ((nValue & (T(1) << i)) == NoneValue)
			{
				return i;
			}
		}

		return -1;
	}

	/**
	 * @brief 获取指定值中指定位的值（静态方法）
	 * 
	 * @param nValue 要检查的值
	 * @param nIndex 位索引（0开始）
	 * @return T 位值，ErrorValue表示索引无效
	 */
	static T GetBitValue(const T& nValue, const int nIndex)
	{
		if (nIndex < 0 || nIndex >= GetBitLength())
		{
			return ErrorValue;
		}

		return (nValue & (T(1) << nIndex));
	}

	/**
	 * @brief 设置指定值中的指定位（静态方法）
	 * 
	 * @param nValue [in,out] 要修改的值的引用
	 * @param nIndex 位索引（0开始）
	 */
	static void SetBitValue(T& nValue, const int nIndex)
	{
		if (nIndex < 0 || nIndex >= GetBitLength())
		{
			return;
		}

		nValue |= (T(1) << nIndex);
	}

	/**
	 * @brief 清除指定值中的指定位（静态方法）
	 * 
	 * @param nValue [in,out] 要修改的值的引用
	 * @param nIndex 位索引（0开始）
	 */
	static void ClearBitValue(T& nValue, const int nIndex)
	{
		if (nIndex < 0 || nIndex >= GetBitLength())
		{
			return;
		}

		nValue &= ~(T(1) << nIndex);
	}

	/**
	 * @brief 检查指定值中的指定位是否已设置（静态方法）
	 * 
	 * @param nValue 要检查的值
	 * @param nIndex 位索引（0开始）
	 * @return bool 位已设置返回true，否则返回false
	 */
	static bool HaveBitValue(const T& nValue, const int nIndex)
	{
		return GetBitValue(nValue, nIndex) != NoneValue;
	}

	/** @} */

private:
	/** @brief 存储位值的内部变量 */
	T m_nValue;
};

