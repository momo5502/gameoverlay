cef = {
	source = "deps/cef"
}

function cef.import()
	links { "cef", "cef_sandbox", "libcef" }
	linkoptions { "/DELAYLOAD:libcef.dll" }
	cef.includes()
end

function cef.includes()
	includedirs { cef.source }
	defines {
		"WRAPPING_CEF_SHARED",
		"NOMINMAX",
		"USING_CEF_SHARED",
	}
	
	filter { "Release", "platforms:*32" }
		libdirs { path.join(cef.source, "Release/Win32") }
	filter { "Release", "platforms:*64" }
		libdirs { path.join(cef.source, "Release/Win64") }
	filter { "Debug", "platforms:*32" }
		libdirs { path.join(cef.source, "Debug/Win32") }
	filter { "Debug", "platforms:*64" }
		libdirs { path.join(cef.source, "Debug/Win64") }

	filter {}
end

function cef.project()
	project "cef"
		language "C++"

		cef.includes()
		files
		{
			path.join(cef.source, "libcef_dll/**.h"),
			path.join(cef.source, "libcef_dll/**.cc"),
		}

		postbuildcommands {
			"mkdir \"%{wks.location}runtime/%{cfg.platform}/%{cfg.buildcfg}/cef/\" 2> nul",
			"mkdir \"%{wks.location}runtime/%{cfg.platform}/%{cfg.buildcfg}/cef/locales/\" 2> nul",
			"copy /y \"%{wks.location}..\\deps\\cef\\%{cfg.buildcfg}\\%{cfg.platform}\\*.dll\" \"%{wks.location}runtime\\%{cfg.platform}\\%{cfg.buildcfg}\\cef\\\"",
			"copy /y \"%{wks.location}..\\deps\\cef\\Resources\\*.pak\" \"%{wks.location}runtime\\%{cfg.platform}\\%{cfg.buildcfg}\\cef\\\"",
			"copy /y \"%{wks.location}..\\deps\\cef\\Resources\\*.dat\" \"%{wks.location}runtime\\%{cfg.platform}\\%{cfg.buildcfg}\\cef\\\"",
			"copy /y \"%{wks.location}..\\deps\\cef\\Resources\\locales\\*.pak\" \"%{wks.location}runtime\\%{cfg.platform}\\%{cfg.buildcfg}\\cef\\locales\\\"",
		}

		linkoptions { "-IGNORE:4221" }
		removelinks "*"
		warnings "Off"
		kind "StaticLib"
end