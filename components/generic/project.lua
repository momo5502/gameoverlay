project "*"
	includedirs {
		"./include"
	}
	links { "generic" }


project "generic"
	kind "StaticLib"
	language "C++"

	files {
		"./**.hpp",
		"./**.cpp",
	}

	removelinks "*"