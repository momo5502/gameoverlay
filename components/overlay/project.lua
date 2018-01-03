	project "overlay"
		kind "SharedLib"
		language "C++"

		files {
			"./src/**.hpp",
			"./src/**.cpp",
		}

		postbuildcommands {
			"if not exist \"%{wks.location}runtime\" mkdir \"%{wks.location}runtime\"",
			"copy /y \"$(TargetPath)\" \"%{wks.location}runtime\"",
		}

		pchheader "std_include.hpp"
		pchsource "src/std_include.cpp"