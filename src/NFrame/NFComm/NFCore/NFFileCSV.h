// -------------------------------------------------------------------------
//    @FileName         :    NFFileCSV.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFFileCSV.h
//
// -------------------------------------------------------------------------

/**
 * @file NFFileCSV.h
 * @brief CSV文件处理工具类
 * 
 * 此文件提供了CSV（逗号分隔值）文件的读取和写入功能。
 * 支持标准CSV格式的解析和生成，主要用于数据导入导出、
 * 配置文件处理等场景。
 */

#pragma once

#include <vector>
#include <string>
#include "NFFileUtility.h"
#include "NFStringUtility.h"
#include "NFCommon.h"

/**
 * @brief CSV文件处理工具类
 * 
 * NFFileCSV提供了简单易用的CSV文件读写功能。
 * 支持将CSV文件解析为二维字符串数组，也支持将二维数组写入CSV文件。
 * 
 * 主要特性：
 * - 标准CSV格式支持：遵循RFC 4180标准
 * - 简单接口：提供静态方法，无需实例化
 * - 灵活数据结构：使用std::vector<std::vector<std::string>>表示表格
 * - 错误处理：返回布尔值指示操作成功与否
 * - 依赖工具：集成框架内的文件和字符串工具
 * 
 * CSV格式说明：
 * - 字段间用逗号分隔
 * - 每行表示一条记录
 * - 字段值可以用双引号包围
 * - 双引号内的逗号和换行符被视为字段内容
 * 
 * 适用场景：
 * - 游戏配置数据的导入导出
 * - 数据报表的生成和解析
 * - 与Excel等工具的数据交换
 * - 简单数据库的文本存储格式
 * 
 * 使用方法：
 * @code
 * // 读取CSV文件
 * std::vector<std::vector<std::string>> table;
 * if (NFFileCSV::ReadCsvFile("data.csv", table)) {
 *     // 遍历表格数据
 *     for (size_t row = 0; row < table.size(); ++row) {
 *         for (size_t col = 0; col < table[row].size(); ++col) {
 *             std::cout << "单元格[" << row << "][" << col << "]: " 
 *                       << table[row][col] << std::endl;
 *         }
 *     }
 * }
 * 
 * // 写入CSV文件
 * std::vector<std::vector<std::string>> data = {
 *     {"姓名", "年龄", "城市"},
 *     {"张三", "25", "北京"},
 *     {"李四", "30", "上海"}
 * };
 * if (NFFileCSV::WriteCsvFile("output.csv", data)) {
 *     std::cout << "CSV文件写入成功" << std::endl;
 * }
 * @endcode
 * 
 * @note 所有方法都是静态方法，不需要创建实例
 * @note 文件编码建议使用UTF-8以支持中文等字符
 */
class NFFileCSV {
public:
    /**
     * @brief 读取CSV文件
     * 
     * 从指定的CSV文件中读取数据，解析为二维字符串数组。
     * 支持标准CSV格式，包括引号包围的字段和嵌入的逗号。
     * 
     * @param szFileName CSV文件的路径，支持相对路径和绝对路径
     * @param table [out] 输出参数，存储解析后的表格数据
     *                   外层vector表示行，内层vector表示列
     * @return bool 读取成功返回true，失败返回false
     * 
     * @note 如果文件不存在或格式错误，会返回false
     * @note 空行会被忽略，但空字段会被保留
     * @note 表格的行数和列数可能不一致（某些行的列数可能不同）
     */
    static bool ReadCsvFile(const std::string &szFileName, std::vector<std::vector<std::string>> &table);

    /**
     * @brief 写入CSV文件
     * 
     * 将二维字符串数组写入指定的CSV文件。
     * 自动处理字段中的特殊字符（如逗号、引号、换行符）。
     * 
     * @param szFileName 要写入的CSV文件路径，如果文件存在会被覆盖
     * @param table 要写入的表格数据，外层vector表示行，内层vector表示列
     * @return bool 写入成功返回true，失败返回false
     * 
     * @note 如果字段内容包含逗号、引号或换行符，会自动用双引号包围
     * @note 字段内的双引号会被转义为两个双引号
     * @note 如果目录不存在，写入会失败
     */
    static bool WriteCsvFile(const std::string &szFileName, const std::vector<std::vector<std::string>> &table);
};