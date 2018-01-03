fx11 = {
	source = "deps/fx11"
}

function fx11.import()
	links { "directxtk" }
	fx11.includes()
end

function fx11.includes()
	includedirs { path.join(fx11.source, "Inc") }
	includedirs { path.join(fx11.source, "Binary") }

	defines {
		"_WIN7_PLATFORM_UPDATE",
		"_LIB",
		"D3DXFX_LARGEADDRESS_HANDLE",
		"_CRT_STDIO_ARBITRARY_WIDE_SPECIFIERS",
	}
end

function fx11.project()
	project "fx11"
		language "C++"

		fx11.includes()
		files
		{
			path.join(fx11.source, "**.h"),
			path.join(fx11.source, "**.inl"),
			path.join(fx11.source, "**.cpp"),
		}

		removelinks "*"
		warnings "Off"
		kind "StaticLib"
end