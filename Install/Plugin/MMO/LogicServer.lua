require "Common"		--

--LogicServer 游戏服务器, 一般一个小区只有一个， 负责游戏非战斗的业务， 连接世界服务器
--ServerId = "15.100.4.1" 15是世界服务ID， 范围1-15
-- 100是区服务ID， 范围1-4095
-- 4是服务器类型ID， 必须跟serverType一样 范围1-255
-- 1服务器索引，范围1-255
LogicServer = {
   LogicServer_1_1 = {
      ServerName = "LogicServer_1_1",
      ServerType = NF_ST_LOGIC_SERVER,
      ServerId = NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_LOGIC_SERVER..".1",			--对每一个服务器来说都是唯一的， 应用程序需要通过这个ServerId才能知道需要加载的配置是他
      LinkMode = NF_LINK_MODE,
      BusLength = NF_COMMON_BUS_LENGTH,     --20M共享内存
      IdleSleepUS = 1000,
      ServerIp = NF_INTER_SERVER_IP,
      ServerPort = NF_INTER_SERVER_PORT+NF_ST_LOGIC_SERVER*10+1,
      MaxConnectNum = NF_INTER_MAX_CONNECT,
      NetThreadNum = 1,
      WorkThreadNum = 1,
      Security = false,
      WebSocket = false,
      MaxOnlinePlayerNum = NF_MAX_ONLINE_PLAYER_COUNT,
      DefaultDBName = NF_MYSQL_DB_NAME,
      HandleMsgNumPerFrame = NF_NORMAL_SERVER_HANDLE_MSG_COUNT,
      RouteConfig = {
         RouteAgent = NF_ROUTE_AGENT_ID,
         MasterIp = NF_MASTER_IP,
         MasterPort = NF_MASTER_PORT,
      },
      IsNoToken = false,
      ExternalData = {
            m_dwLogOutFailRetryTime = 1,
            m_iRoleDetailSaveTimeGap = 2,
            m_iTickSaveRoleIndexGap = 3,
            m_iTickSaveRoleNum = 4,
            m_iTickTransIndexGap = 5,
            m_iTickTransNum = 6,
            m_iServerStopKickNumPerLoop = 7,
            m_iInTeamKeepAliveTime = 8,
            m_uClientRecvPacketFreqThresholdValue = 9,
            m_uClientRecvWarningThresholdValue = 10,
            m_uClentRecvSameCmdIntervalTime = 11,
            m_uClentRecvFreqCycleTime = 12,
            m_bWhiteListLogin = false,
            m_iPlatID = 1,
            m_bNoToken = false,
            m_bNotCheckZone = false,
            m_bIsCheckClientVersion = false,
            m_iIndulgeTime = 18,
            operationConfig = {
                m_bForbidRegist = false,
                m_bIsRegistLimit = false,
                m_iRegistLimit = 100;
                m_iMaxOnline = 100;
                m_sessionPerSecond = 100, --每1秒进入游戏的人数，超出进行排队
                m_bIsWhiteLogin = false,
                m_tServerOpenTime = 1723713843;
                --渠道号白名单
                m_bIsRegChannelOpen = false;
                m_regChannelWhiteList = {"gaoyi", "37wan"},
                m_dwSessionHeartbeatTimeOut = 300;
            }
      }
   };

   LogicServer_2_1 = {
      ServerName = "LogicServer_2_1",
      ServerType = NF_ST_LOGIC_SERVER,
      ServerId = NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_LOGIC_SERVER..".1",			--对每一个服务器来说都是唯一的， 应用程序需要通过这个ServerId才能知道需要加载的配置是他
      LinkMode = NF_LINK_MODE,
      BusLength = NF_COMMON_BUS_LENGTH,     --20M共享内存
      IdleSleepUS = 1000,
      ServerIp = NF_INTER_SERVER_IP,
      ServerPort = NF_INTER_SERVER_PORT2+NF_ST_LOGIC_SERVER*10+1,
      MaxConnectNum = NF_INTER_MAX_CONNECT,
      NetThreadNum = 1,
      WorkThreadNum = 1,
      Security = false,
      WebSocket = false,
      MaxOnlinePlayerNum = NF_MAX_ONLINE_PLAYER_COUNT,
      DefaultDBName = NF_MYSQL_DB_NAME2,
      HandleMsgNumPerFrame = NF_NORMAL_SERVER_HANDLE_MSG_COUNT,
      RouteConfig = {
         RouteAgent = NF_ROUTE_AGENT_ID2,
         MasterIp = NF_MASTER_IP,
         MasterPort = NF_MASTER_PORT,
      },
      IsNoToken = false,
      ExternalData = {
            m_dwLogOutFailRetryTime = 1,
            m_iRoleDetailSaveTimeGap = 2,
            m_iTickSaveRoleIndexGap = 3,
            m_iTickSaveRoleNum = 4,
            m_iTickTransIndexGap = 5,
            m_iTickTransNum = 6,
            m_iServerStopKickNumPerLoop = 7,
            m_iInTeamKeepAliveTime = 8,
            m_uClientRecvPacketFreqThresholdValue = 9,
            m_uClientRecvWarningThresholdValue = 10,
            m_uClentRecvSameCmdIntervalTime = 11,
            m_uClentRecvFreqCycleTime = 12,
            m_bWhiteListLogin = false,
            m_iPlatID = 1,
            m_bNoToken = false,
            m_bNotCheckZone = false,
            m_bIsCheckClientVersion = false,
            m_iIndulgeTime = 18,
            operationConfig = {
                m_bForbidRegist = false,
                m_bIsRegistLimit = false,
                m_iRegistLimit = 100;
                m_iMaxOnline = 100;
                m_sessionPerSecond = 100, --每1秒进入游戏的人数，超出进行排队
                m_bIsWhiteLogin = false,
                m_tServerOpenTime = 1723713843;
                --渠道号白名单
                m_bIsRegChannelOpen = false;
                m_regChannelWhiteList = {"gaoyi", "37wan"},
                m_dwSessionHeartbeatTimeOut = 300;
            }
      }
   };

   CrossLogicServer_1 = {
      ServerName = "CrossLogicServer_1",
      ServerType = NF_ST_LOGIC_SERVER,
      ServerId = NF_ST_WORLD_ID.."."..NF_ST_CROSS_ZONE_ID.."."..NF_ST_LOGIC_SERVER..".1",			--对每一个服务器来说都是唯一的， 应用程序需要通过这个ServerId才能知道需要加载的配置是他
      LinkMode = NF_LINK_MODE,
      BusLength = NF_COMMON_BUS_LENGTH,     --20M共享内存
      IdleSleepUS = 1000,
      ServerIp = NF_INTER_SERVER_IP,
      ServerPort = NF_INTER_SERVER_PORT+NF_ST_LOGIC_SERVER*10+20,
      MaxConnectNum = NF_INTER_MAX_CONNECT,
      NetThreadNum = 1,
      WorkThreadNum = 1,
      Security = false,
      WebSocket = false,
      CrossServer = true,
      MaxOnlinePlayerNum = NF_MAX_ONLINE_PLAYER_COUNT,
      DefaultDBName = NF_MYSQL_CROSS_DB_NAME,
      HandleMsgNumPerFrame = NF_NORMAL_SERVER_HANDLE_MSG_COUNT,
      RouteConfig = {
         RouteAgent = NF_CROSS_ROUTE_AGENT_ID1,
         MasterIp = NF_MASTER_IP,
         MasterPort = NF_MASTER_PORT,
      },
      IsNoToken = false,
      ExternalData = {
            m_dwLogOutFailRetryTime = 1,
            m_iRoleDetailSaveTimeGap = 2,
            m_iTickSaveRoleIndexGap = 3,
            m_iTickSaveRoleNum = 4,
            m_iTickTransIndexGap = 5,
            m_iTickTransNum = 6,
            m_iServerStopKickNumPerLoop = 7,
            m_iInTeamKeepAliveTime = 8,
            m_uClientRecvPacketFreqThresholdValue = 9,
            m_uClientRecvWarningThresholdValue = 10,
            m_uClentRecvSameCmdIntervalTime = 11,
            m_uClentRecvFreqCycleTime = 12,
            m_bWhiteListLogin = false,
            m_iPlatID = 1,
            m_bNoToken = false,
            m_bNotCheckZone = false,
            m_bIsCheckClientVersion = false,
            m_iIndulgeTime = 18,
            operationConfig = {
                m_bForbidRegist = false,
                m_bIsRegistLimit = false,
                m_iRegistLimit = 100;
                m_iMaxOnline = 100;
                m_sessionPerSecond = 100, --每1秒进入游戏的人数，超出进行排队
                m_bIsWhiteLogin = false,
                m_tServerOpenTime = 1723713843;
                --渠道号白名单
                m_bIsRegChannelOpen = false;
                m_regChannelWhiteList = {"gaoyi", "37wan"},
                m_dwSessionHeartbeatTimeOut = 300;
            }
      }
   };
};