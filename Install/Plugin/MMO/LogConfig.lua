
NF_LOG_LEVEL_TRACE = 0 --    trace = 0,
NF_LOG_LEVEL_DEBUG = 1 --    debug = 1,
NF_LOG_LEVEL_INFO = 2  --    info = 2,
NF_LOG_LEVEL_WARN = 3  --    warn = 3,
NF_LOG_LEVEL_ERROR = 4 --    err = 4,


NFLogId = {
	--0-100是基础框架层LOG
	NF_LOG_DEFAULT = 0,		--系统LOG
	NF_LOG_STATISTIC = 1,	--统计日志
	NF_LOG_BEHAVIOR = 2,	--行为日志
	NF_LOG_MAX_ID = 3, --最大LOGID
}

--	配置要打印的LOG
--	display 是否打印log
--	level 打印log最低等级
--  logname 打印时显示的名字
--  guid 只打印一个或几个玩家的LOG 打印几个玩家的LOG系统guid={guid1, guid2}
LogInfo = {
	--0-100是基础框架层LOG
	{logid = NFLogId.NF_LOG_DEFAULT, display=true, level=NF_LOG_LEVEL_DEBUG, logname = "default", desc = "默认LOG"},
	{logid = NFLogId.NF_LOG_STATISTIC, display=true, level=NF_LOG_LEVEL_TRACE, logname = "statistic", desc = "统计LOG"},
	{logid = NFLogId.NF_LOG_BEHAVIOR, display=true, level=NF_LOG_LEVEL_TRACE, logname = "behavior", desc = "行为LOG"},
}
