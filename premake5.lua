

-- Option to allow copying the DLL file to a custom folder after build
newoption {
	trigger = "copy-to",
	description = "Optional, copy the DLL to a custom folder after build, define the path here if wanted.",
	value = "PATH"
}

workspace "gameoverlay"
	configurations { "Debug", "Release" }
	platforms { "Win32", "Win64" }
	
	project "*"
		includedirs {
			"interfaces/include",
			"deps/literally/include"
		}
	
	require "components/overlay/project"
	
	group "Renderers"
		require "components/renderers/d3d10_11/project"

workspace "*"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"
	buildlog "%{wks.location}/obj/%{cfg.architecture}/%{cfg.buildcfg}/%{prj.name}/%{prj.name}.log"
	
	buildoptions {
		"/std:c++latest"
	}

	filter "toolset:msc*"
		buildoptions { "/utf-8", "/Zm200" }
	
	filter "platforms:*32"
		architecture "x86"
	
	filter "platforms:*64"
		architecture "x86_64"
	
	filter "platforms:Win*"
		system "windows"
		defines { "_WINDOWS" }
	
	filter {}

	flags {
		"StaticRuntime",
		"NoIncrementalLink",
		"NoMinimalRebuild",
		"MultiProcessorCompile",
		"No64BitChecks",
		"UndefinedIdentifiers"
	}
	
	largeaddressaware "on"
	editandcontinue "Off"
	warnings "Extra"
	symbols "On"

	configuration "Release*"
		defines { "NDEBUG" }
		optimize "On"
		flags {
			"FatalCompileWarnings",
			"FatalLinkWarnings",
		}

	configuration "Debug*"
		defines { "DEBUG", "_DEBUG" }
		optimize "Debug"
