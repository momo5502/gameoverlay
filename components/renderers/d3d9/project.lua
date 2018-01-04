	project "renderer_d3d9"
		kind "SharedLib"
		language "C++"

		files {
			"./src/**.hpp",
			"./src/**.cpp",
		}

		postbuildcommands {
			"if not exist \"%{wks.location}runtime\" mkdir \"%{wks.location}runtime\"",
			"if not exist \"%{wks.location}runtime\\renderers\" mkdir \"%{wks.location}runtime\\renderers\"",
			"copy /y \"$(TargetPath)\" \"%{wks.location}runtime\\renderers\"",
		}
		
		pchheader "std_include.hpp"
		pchsource "src/std_include.cpp"