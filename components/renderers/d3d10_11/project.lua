	project "renderer_d3d10_11"
		kind "SharedLib"
		language "C++"

		files {
			"./src/**.hpp",
			"./src/**.cpp",
		}

		postbuildcommands {
			"mkdir \"%{wks.location}runtime\\renderers\"",
			"copy /y \"$(TargetPath)\" \"%{wks.location}runtime\\renderers\"",
		}
		
		pchheader "std_include.hpp"
		pchsource "src/std_include.cpp"