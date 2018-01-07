	project "renderer_dxgi"
		kind "SharedLib"
		language "C++"

		files {
			"./src/**.hpp",
			"./src/**.cpp",
		}

		postbuildcommands {
			"if not exist \"%{wks.location}runtime\" mkdir \"%{wks.location}runtime\"",
			"if not exist \"%{wks.location}runtime\\%{cfg.platform}\" mkdir \"%{wks.location}runtime\\%{cfg.platform}\"",
			"if not exist \"%{wks.location}runtime\\%{cfg.platform}\\%{cfg.buildcfg}\" mkdir \"%{wks.location}runtime\\%{cfg.platform}\\%{cfg.buildcfg}\"",
			"if not exist \"%{wks.location}runtime\\%{cfg.platform}\\%{cfg.buildcfg}\\renderers\" mkdir \"%{wks.location}runtime\\%{cfg.platform}\\%{cfg.buildcfg}\\renderers\"",
			"copy /y \"$(TargetPath)\" \"%{wks.location}runtime\\%{cfg.platform}\\%{cfg.buildcfg}\\renderers\"",
		}
		
		pchheader "std_include.hpp"
		pchsource "src/std_include.cpp"