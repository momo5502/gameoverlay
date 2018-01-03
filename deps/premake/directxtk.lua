directxtk = {
	source = "deps/directxtk"
}

function directxtk.import()
	links { "directxtk" }
	directxtk.includes()
end

function directxtk.includes()
	includedirs { path.join(directxtk.source, "Inc") }
	
	defines {
		"_WIN7_PLATFORM_UPDATE",
		"_LIB",
		"D3DXFX_LARGEADDRESS_HANDLE",
		"_CRT_STDIO_ARBITRARY_WIDE_SPECIFIERS",
	}
end

function directxtk.project()
	project "directxtk"
		language "C++"

		directxtk.includes()
		files
		{
			path.join(directxtk.source, "**.h"),
			path.join(directxtk.source, "**.inl"),
			path.join(directxtk.source, "**.cpp"),
		}
		removefiles
		{
			path.join(directxtk.source, "Audio/**.*"),
			path.join(directxtk.source, "XWBTool/**.*"),
			path.join(directxtk.source, "**/Xbox*.*"),
		}

		removelinks "*"
		warnings "Off"
		kind "StaticLib"
end