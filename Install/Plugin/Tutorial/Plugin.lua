require "Common"

LoadPlugin =
{
	TutorialAllServer = {
		ServerPlugins = {
			-------------------------
			-----基础框架引擎-------------
			"NFKernelPlugin",
			--"NFZDBPlugin",
			"NFTutorialPlugin",
			"NFShmPlugin",
			--"NFLuaScriptPlugin",
            --"NFTestPlugin",
		};
		ServerType = NF_ST_NONE;
	},
}
