// -------------------------------------------------------------------------
//    @FileName         :    NFServerTime.h
//    @Author           :    Gao.Yi
//    @Date             :   2022-09-18
//    @Email			:    445267987@qq.com
//    @Module           :    NFCore
//
// -------------------------------------------------------------------------

/**
 * @file NFServerTime.h
 * @brief 服务器时间管理类
 * 
 * 此文件定义了服务器时间管理的单例类，用于统一管理服务器的时间系统，
 * 包括帧计数、时间偏移、FPS控制等功能。
 */

#pragma once

#include <string>

#include "NFPlatform.h"
#include "NFSingleton.hpp"

/**
 * @brief 服务器时间管理单例类
 * 
 * 该类负责管理服务器的时间系统，提供精确的时间控制和帧率管理。
 * 主要功能包括：
 * - 维护服务器启动时间和当前时间
 * - 提供帧计数和时间差计算
 * - 支持GM设置时间功能
 * - 提供时区和每日时间计算
 * - 帧率控制和周期性检查
 * 
 * 使用方法：
 * @code
 * NFServerTime* timeManager = NFServerTime::Instance();
 * timeManager->Init(60); // 初始化为60FPS
 * timeManager->Update(currentTick); // 每帧更新
 * @endcode
 */
class _NFExport NFServerTime : public NFSingleton<NFServerTime>
{
public:
    /**
     * @brief 构造函数
     * 初始化时间管理器的各项参数
     */
    NFServerTime();

    /**
     * @brief 虚析构函数
     * 清理时间管理器资源
     */
    virtual ~NFServerTime();

public:
    /**
     * @brief 初始化时间管理器
     * @param fps 目标帧率，用于控制服务器更新频率
     * @return bool 初始化成功返回true，失败返回false
     */
    bool Init(int fps);

    /**
     * @brief 反初始化时间管理器
     * @return bool 反初始化成功返回true，失败返回false
     */
    bool UnInit();

    /**
     * @brief 更新时间管理器
     * 每帧都应该调用此方法来更新时间状态
     * @param tick 当前的时间戳（毫秒）
     * @return bool 更新成功返回true，失败返回false
     */
    bool Update(uint64_t tick);

    /**
     * @brief GM设置时间功能
     * 允许游戏管理员设置特定的时间
     * @param year 年份
     * @param month 月份
     * @param day 日期
     * @param hour 小时
     * @param min 分钟
     * @param sec 秒
     * @return int 设置结果，0表示成功
     */
    int GmSetTime(int year, int month, int day, int hour, int min, int sec);
    
    /**
     * @brief GM设置时间功能（Unix时间戳版本）
     * @param tTime Unix时间戳
     * @return int 设置结果，0表示成功
     */
    int GmSetTime(uint64_t tTime);
public:
    /**
     * @brief 获取服务器启动时的时间戳
     * @return uint64_t 启动时间戳（毫秒）
     */
    uint64_t StartTick() { return m_startTick; }
    
    /**
     * @brief 获取当前运行时长
     * @return uint32_t 从启动到现在的时间差（毫秒）
     */
    uint32_t CurElapse() { return (uint32_t) (m_curTick - m_startTick); }
    
    /**
     * @brief 获取当前时间戳
     * @return uint64_t 当前时间戳（毫秒）
     */
    uint64_t Tick() { return m_curTick; }
    
    /**
     * @brief 获取上一帧的时间戳
     * @return uint64_t 上一帧时间戳（毫秒）
     */
    uint64_t LastTick() { return m_lastTick; }
    
    /**
     * @brief 获取当前帧与上一帧的时间差
     * @return int 时间差（毫秒）
     */
    int DeltaTick() { return m_deltaTick; }
    
    /**
     * @brief 获取当前Unix时间戳（秒）
     * @return uint64_t Unix时间戳（秒）
     */
    uint64_t UnixSec() { return m_unixSec; }
    
    /**
     * @brief 获取当前Unix时间戳（毫秒）
     * @return uint64_t Unix时间戳（毫秒）
     */
    uint64_t UnixMSec() { return m_unixMSec; }
    
    /**
     * @brief 获取总帧数
     * @return uint64_t 从启动到现在的总帧数
     */
    uint64_t Frames() { return m_frames; }
    
    /**
     * @brief 获取总秒数
     * @return uint64_t 从启动到现在的总秒数
     */
    uint64_t Secs() { return m_secs; }
    
    /**
     * @brief 获取当前FPS设置
     * @return int 目标FPS值
     */
    int GetFPS() { return m_fps; }
    
    /**
     * @brief 获得当日已过秒数（首日有16小时要去除）
     * @return uint32_t 当日已过秒数
     */
    uint32_t CurDaySec() { return ((uint32_t) UnixSec() - 57600) % DayTotalSec(); };
    
    /**
     * @brief 获取每日总秒数
     * @return uint32_t 每日总秒数（86400秒）
     */
    uint32_t DayTotalSec() { return m_daySec; };
    
    /**
     * @brief 获取服务器当前所在的时区
     * @return int32_t 时区值
     */
    int32_t CurTimeZone() { return m_curzone; };

    /**
     * @brief 检查是否到了指定的帧间隔
     * @param perFrames 间隔帧数
     * @return bool 如果当前帧是指定间隔的倍数则返回true
     */
    bool CheckPerFrames(int perFrames) { return m_frames % perFrames == 0; }
    
    /**
     * @brief 检查是否到了指定的秒间隔
     * @param perSecs 间隔秒数
     * @return bool 如果当前是指定秒间隔且是该秒的第一帧则返回true
     */
    bool CheckPerSecs(int perSecs) { return m_secs % perSecs == 0 && m_perSecFirstFrame; }

    /**
     * @brief 获取秒偏移量
     * @return int 当前的秒偏移量
     */
    int GetSecOffSet() const { return m_iSecOffSet; }
    
    /**
     * @brief 设置秒偏移量
     * @param iSecOffSet 要设置的秒偏移量
     * @return int 设置后的秒偏移量
     */
    int SetSecOffSet(int iSecOffSet) { return m_iSecOffSet = iSecOffSet; }
protected:
    /** @brief 服务器启动时的时间戳（毫秒） */
    uint64_t m_startTick;
    /** @brief 当前更新的时间戳（毫秒），1 tick = 1 msec */
    uint64_t m_curTick;
    /** @brief 上一次更新的时间戳（毫秒），1 tick = 1 msec */
    uint64_t m_lastTick;
    /** @brief 当前帧与上一帧的时间差（毫秒），m_curTick - m_lastTick */
    int m_deltaTick;
    /** @brief 当前Unix时间戳（秒），当前帧内保持一致 */
    uint64_t m_unixSec;
    /** @brief 当前Unix时间戳（毫秒） */
    uint64_t m_unixMSec;
    /** @brief 从应用启动开始的帧计数 */
    uint64_t m_frames;
    /** @brief 从应用启动开始的秒计数 */
    uint64_t m_secs;
    /** @brief 每秒的第一帧标志 */
    bool m_perSecFirstFrame;
    /** @brief 目标FPS */
    int m_fps;
    /** @brief 每日总秒数常量（86400秒） */
    const uint32_t m_daySec = 86400;
private:
    /** @brief 上一次的Unix时间戳（秒） */
    uint64_t m_lastUnixSec;
    /** @brief 当前时区 */
    int32_t m_curzone;
private:
    /** @brief 秒偏移量 */
    int m_iSecOffSet;
};
