require "deps/premake/fx11"
require "deps/premake/directxtk"
require "deps/premake/minhook"
require "deps/premake/dxsdk"

workspace "gameoverlay"
	configurations { "Debug", "Release" }
	platforms { "Win32", "Win64" }
	
	project "*"
		includedirs {
			"deps/literally/include"
		}
		
		fx11.import()
		directxtk.import()
		minhook.import()
	
	require "components/test/project"
	require "components/overlay/project"
	require "components/generic/project"
	
	group "Renderers"
		require "components/renderers/dxgi/project"
		require "components/renderers/d3d9/project"
		
	group "Dependencies"
		fx11.project()
		directxtk.project()
		minhook.project()

workspace "*"
	location "./build"
	objdir "%{wks.location}/obj"
	targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"
	buildlog "%{wks.location}/obj/%{cfg.platform}/%{cfg.buildcfg}/%{prj.name}/%{prj.name}.log"
	
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
