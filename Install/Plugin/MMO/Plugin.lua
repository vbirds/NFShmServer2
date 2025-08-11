require "Common"

LoadPlugin =
{
	TestAllServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",
			--"NFMemPlugin",
			"NFDBPlugin",
			--"NFTutorialPlugin"
			--"NFTestPlugin"
		};
		ServerType = NF_ST_NONE;
	},

	RobotAllServer = {
		ServerPlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
		};
		ServerType = NF_ST_NONE;
	},

	AllServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",
			--"NFMemPlugin",
			"NFDBPlugin",
			--"NFTutorialPlugin"
		};


		ServerPlugins = {
			"NFServerCommonPlugin",
			"NFDescStorePlugin",

			"NFMasterServerPlugin",
			"NFRouteServerPlugin",
			"NFRouteAgentServerPlugin",
			"NFStoreServerPlugin",
			"NFProxyServerPlugin",
			"NFProxyClientPlugin",
			"NFProxyAgentServerPlugin",
			"NFLoginServerPlugin",
			"NFGameServerPlugin",
			"NFWorldServerPlugin",
			"NFSnsServerPlugin",
			"NFLogicServerPlugin",
			"NFWebServerPlugin",
			"NFCenterServerPlugin",

			"NFCityServerPlugin",
			"NFMatchServerPlugin",
			"NFNavMeshServerPlugin",
			"NFOnlineServerPlugin",
		};

		WorkPlugins = {
		};

		ServerType = NF_ST_NONE;
		ServerList = {
			{Server="MasterServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_MASTER_SERVER..".1", ServerType=NF_ST_MASTER_SERVER},
			{Server="ProxyServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_PROXY_SERVER..".1", ServerType=NF_ST_PROXY_SERVER},
			{Server="ProxyAgentServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_PROXY_AGENT_SERVER..".1", ServerType=NF_ST_PROXY_AGENT_SERVER},
			{Server="RouteAgentServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_ROUTE_AGENT_SERVER..".1", ServerType=NF_ST_ROUTE_AGENT_SERVER},
			{Server="RouteServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_ROUTE_SERVER..".1", ServerType=NF_ST_ROUTE_SERVER},
			{Server="GameServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_GAME_SERVER..".1", ServerType=NF_ST_GAME_SERVER},
			{Server="StoreServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_STORE_SERVER..".1", ServerType=NF_ST_STORE_SERVER},
			{Server="WorldServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_WORLD_SERVER..".1", ServerType=NF_ST_WORLD_SERVER},
			{Server="LoginServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_LOGIN_SERVER..".1", ServerType=NF_ST_LOGIN_SERVER},
			{Server="LogicServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_LOGIC_SERVER..".1", ServerType=NF_ST_LOGIC_SERVER},
			{Server="SnsServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_SNS_SERVER..".1", ServerType=NF_ST_SNS_SERVER},
			{Server="WebServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_WEB_SERVER..".1", ServerType=NF_ST_WEB_SERVER},
			{Server="CenterServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_CENTER_SERVER..".1", ServerType=NF_ST_CENTER_SERVER},
			{Server="CityServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_CITY_SERVER..".1", ServerType=NF_ST_CITY_SERVER},
			{Server="MatchServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_MATCH_SERVER..".1", ServerType=NF_ST_MATCH_SERVER},
			{Server="NavMeshServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_NAVMESH_SERVER..".1", ServerType=NF_ST_NAVMESH_SERVER},
			{Server="OnlineServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_ONLINE_SERVER..".1", ServerType=NF_ST_ONLINE_SERVER},
		};
	},

	AllServer2 = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",
			"NFDBPlugin",
			--"NFTutorialPlugin"
		};


		ServerPlugins = {
			"NFServerCommonPlugin",
			"NFDescStorePlugin",

			--"NFMasterServerPlugin",
			"NFRouteServerPlugin",
			"NFRouteAgentServerPlugin",
			"NFStoreServerPlugin",
			"NFProxyServerPlugin",
			"NFProxyClientPlugin",
			"NFProxyAgentServerPlugin",
			"NFLoginServerPlugin",
			"NFGameServerPlugin",
			"NFWorldServerPlugin",
			"NFSnsServerPlugin",
			"NFLogicServerPlugin",
			"NFWebServerPlugin",
			"NFCenterServerPlugin",
		};

		WorkPlugins = {
		};

		ServerType = NF_ST_NONE;
		ServerList = {
			--{Server="MasterServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID.."."..NF_ST_MASTER_SERVER..".1", ServerType=NF_ST_MASTER_SERVER},
			{Server="ProxyServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_PROXY_SERVER..".1", ServerType=NF_ST_PROXY_SERVER},
			{Server="ProxyAgentServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_PROXY_AGENT_SERVER..".1", ServerType=NF_ST_PROXY_AGENT_SERVER},
			{Server="RouteAgentServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_ROUTE_AGENT_SERVER..".1", ServerType=NF_ST_ROUTE_AGENT_SERVER},
			{Server="RouteServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_ROUTE_SERVER..".1", ServerType=NF_ST_ROUTE_SERVER},
			{Server="GameServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_GAME_SERVER..".1", ServerType=NF_ST_GAME_SERVER},
			{Server="StoreServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_STORE_SERVER..".1", ServerType=NF_ST_STORE_SERVER},
			{Server="WorldServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_WORLD_SERVER..".1", ServerType=NF_ST_WORLD_SERVER},
			{Server="LoginServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_LOGIN_SERVER..".1", ServerType=NF_ST_LOGIN_SERVER},
			{Server="LogicServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_LOGIC_SERVER..".1", ServerType=NF_ST_LOGIC_SERVER},
			{Server="SnsServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_SNS_SERVER..".1", ServerType=NF_ST_SNS_SERVER},
			{Server="WebServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_WEB_SERVER..".1", ServerType=NF_ST_WEB_SERVER},
			{Server="CenterServer", ID=NF_ST_WORLD_ID.."."..NF_ST_ZONE_ID2.."."..NF_ST_CENTER_SERVER..".1", ServerType=NF_ST_CENTER_SERVER},
		};
	},

	CrossAllServer = {
        FramePlugins = {
            -------------------------
            -----基础框架引擎-------------
            "NFKernelPlugin",
            "NFNetPlugin",
            "NFShmPlugin",

            "NFServerCommonPlugin",
            "NFDescStorePlugin",
            "NFDBPlugin",
        };

        ServerPlugins = {
            "NFRouteServerPlugin",
            "NFRouteAgentServerPlugin",
            "NFStoreServerPlugin",
            "NFGameServerPlugin",
            "NFCenterServerPlugin",
			"NFLogicServerPlugin",
			"NFWorldServerPlugin",
        };

        WorkPlugins = {
        };

        ServerType = NF_ST_NONE;
        ServerList = {
            {Server="RouteAgentServer", ID=NF_ST_WORLD_ID.."."..NF_ST_CROSS_ZONE_ID.."."..NF_ST_ROUTE_AGENT_SERVER..".1", ServerType=NF_ST_ROUTE_AGENT_SERVER},
            {Server="RouteServer", ID=NF_ST_WORLD_ID.."."..NF_ST_CROSS_ZONE_ID.."."..NF_ST_ROUTE_SERVER..".1", ServerType=NF_ST_ROUTE_SERVER},
            {Server="GameServer", ID=NF_ST_WORLD_ID.."."..NF_ST_CROSS_ZONE_ID.."."..NF_ST_GAME_SERVER..".1", ServerType=NF_ST_GAME_SERVER},
			{Server="LogicServer", ID=NF_ST_WORLD_ID.."."..NF_ST_CROSS_ZONE_ID.."."..NF_ST_LOGIC_SERVER..".1", ServerType=NF_ST_LOGIC_SERVER},
            {Server="StoreServer", ID=NF_ST_WORLD_ID.."."..NF_ST_CROSS_ZONE_ID.."."..NF_ST_STORE_SERVER..".1", ServerType=NF_ST_STORE_SERVER},
            {Server="CenterServer", ID=NF_ST_WORLD_ID.."."..NF_ST_CROSS_ZONE_ID.."."..NF_ST_CENTER_SERVER..".1", ServerType=NF_ST_CENTER_SERVER},
			{Server="WorldServer", ID=NF_ST_WORLD_ID.."."..NF_ST_CROSS_ZONE_ID.."."..NF_ST_WORLD_SERVER..".1", ServerType=NF_ST_WORLD_SERVER},
        };
    },

	MasterServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFServerCommonPlugin",
		};


		ServerPlugins = {
			"NFMasterServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_MASTER_SERVER;
	},

	ProxyServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",

			"NFServerCommonPlugin",
		};


		ServerPlugins = {
			"NFProxyServerPlugin",
			"NFProxyClientPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_PROXY_SERVER;
	},

	ProxyAgentServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",

			"NFServerCommonPlugin",
		};


		ServerPlugins = {
			"NFProxyAgentServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_PROXY_AGENT_SERVER;
	},

	GameServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",

			"NFServerCommonPlugin",
			"NFDescStorePlugin",
		};

		ServerPlugins = {
			"NFGameServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_GAME_SERVER;
	},

	LoginServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",

			"NFServerCommonPlugin",
			"NFDescStorePlugin",
		};


		ServerPlugins = {
			"NFLoginServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_LOGIN_SERVER;
	},

	WorldServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",

			"NFServerCommonPlugin",
			"NFDescStorePlugin",
		};


		ServerPlugins = {
			"NFWorldServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_WORLD_SERVER;
	},

	StoreServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFDBPlugin",
			"NFServerCommonPlugin",
		};


		ServerPlugins = {
			"NFStoreServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_STORE_SERVER;
	},

	LogicServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",

			"NFServerCommonPlugin",
			"NFDescStorePlugin",
		};


		ServerPlugins = {
			"NFLogicServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_LOGIC_SERVER;
	},

	SnsServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",

			"NFServerCommonPlugin",
			"NFDescStorePlugin",
		};


		ServerPlugins = {
			"NFSnsServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_SNS_SERVER;
	},

	WebServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFServerCommonPlugin",
		};


		ServerPlugins = {
			"NFWebServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_WEB_SERVER;
	},

	RouteAgentServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFServerCommonPlugin",
		};


		ServerPlugins = {
			"NFRouteAgentServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_ROUTE_AGENT_SERVER;
	},

	RouteServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFServerCommonPlugin",
		};


		ServerPlugins = {
			"NFRouteServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_ROUTE_SERVER;
	},

	CenterServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",

			"NFServerCommonPlugin",
			"NFDescStorePlugin",
		};


		ServerPlugins = {
			"NFCenterServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_CENTER_SERVER;
	},

	CheckServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",

			"NFServerCommonPlugin",
			"NFDescStorePlugin",
		};


		ServerPlugins = {
			"NFCheckServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_CHECK_SERVER;
	},

	CityServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",

			"NFServerCommonPlugin",
			"NFDescStorePlugin",
		};


		ServerPlugins = {
			"NFCityServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_CITY_SERVER;
	},

	MatchServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",

			"NFServerCommonPlugin",
			"NFDescStorePlugin",
		};


		ServerPlugins = {
			"NFMatchServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_MATCH_SERVER;
	},

	NavMeshServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",

			"NFServerCommonPlugin",
			"NFDescStorePlugin",
		};


		ServerPlugins = {
			"NFNavMeshServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_NAVMESH_SERVER;
	},

	OnlineServer = {
		FramePlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			"NFNetPlugin",
			"NFShmPlugin",

			"NFServerCommonPlugin",
			"NFDescStorePlugin",
		};


		ServerPlugins = {
			"NFOnlineServerPlugin",
		};

		WorkPlugins = {
		};
		ServerType = NF_ST_ONLINE_SERVER;
	},
}
