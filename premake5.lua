dependencies = {
	basePath = "./deps"
}

function dependencies.load()
	dir = path.join(dependencies.basePath, "premake/*.lua")
	deps = os.matchfiles(dir)

	for i, dep in pairs(deps) do
		dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.import()
		end
	end
end

function dependencies.projects()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.project()
		end
	end
end

dependencies.load()

workspace "gameoverlay"
startproject "gameoverlay"
location "./build"
objdir "%{wks.location}/obj"
targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"

configurations {"Debug", "Release"}

if os.istarget("darwin") then
	platforms { "x64", "arm" }
else
	platforms { "x86", "x64" }
end

filter "platforms:x86"
architecture "x32"

filter "platforms:x64"
architecture "x64"

filter "platforms:arm"
architecture "ARM"
buildoptions { "-arch arm64" }
linkoptions { "-arch arm64" }

filter { "language:C++", "toolset:not msc*" }
	buildoptions {
		"-std=c++20"
	}
filter "toolset:msc*"
	buildoptions {
		"/std:c++latest"
	}
filter {}

systemversion "latest"
symbols "On"
staticruntime "On"
editandcontinue "Off"
warnings "Extra"
characterset "ASCII"

if os.istarget("linux") or os.istarget("darwin") then
	buildoptions { "-pthread" }
	linkoptions { "-pthread" }
end

if _OPTIONS["dev-build"] then
	defines {"DEV_BUILD"}
end

if os.getenv("CI") then
	defines {"CI"}
end

flags {"NoIncrementalLink", "NoMinimalRebuild", "MultiProcessorCompile", "No64BitChecks"}

configuration "Release"
optimize "Speed"

defines {"NDEBUG"}

flags {"FatalCompileWarnings"}

configuration "Debug"
optimize "Debug"

defines {"DEBUG", "_DEBUG"}

configuration {}

project "gameoverlay"
kind "SharedLib"
language "C++"

pchheader "std_include.hpp"
pchsource "src/std_include.cpp"

files {"./src/**.rc", "./src/**.hpp", "./src/**.cpp"}

includedirs {"./src", "%{prj.location}/src"}

filter "system:windows"
	files {
		"./src/**.rc",
	}
filter { "system:windows", "toolset:not msc*" }
	resincludedirs {
		"%{_MAIN_SCRIPT_DIR}/src"
	}
filter { "system:windows", "toolset:msc*" }
	linkoptions {"/IGNORE:4254", "/SAFESEH:NO", "/LARGEADDRESSAWARE", "/PDBCompress"}
	resincludedirs {
		"$(ProjectDir)src" -- fix for VS IDE
	}
filter {}

dependencies.imports()


group "Dependencies"
dependencies.projects()
