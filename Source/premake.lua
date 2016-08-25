
rootDir = "../"

workspace "sdl2-testgame"			
    location (rootDir .. "Local/Build")
	configurations { "Debug", "Release" }

project "Main"
	kind "ConsoleApp"
	language "C++"
	targetdir (rootDir .. "Local/Bin/%{cfg.buildcfg}")
	
	includedirs { rootDir .. "External/SDL2/include" }
	libdirs { rootDir .. "External/SDL2/lib/x86/" }
	links { "SDL2", "SDL2main" }
	
	debugdir (rootDir .. "Bin/x86")
	
	files { "**.h", "**.cpp" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

if not (os.isdir(rootDir .. "Bin/x86/"))	then
	os.mkdir(rootDir .. "Bin/x86/")
end
success, errormsg = os.copyfile(path.translate(rootDir .. "External/SDL2/lib/x86/SDL2.dll"), path.translate(rootDir .. "Bin/x86/SDL2.dll"))	
if (success == nil) then
	print("Cannot copy binaries: " .. errormsg)
end