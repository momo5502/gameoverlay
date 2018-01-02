	project "renderer_d3d10_11"
		kind "SharedLib"
		language "C++"

		files {
			"./src/**.hpp",
			"./src/**.cpp",
		}

		-- Pre-compiled header
		pchheader "std_include.hpp" -- must be exactly same as used in #include directives
		pchsource "src/std_include.cpp" -- real path