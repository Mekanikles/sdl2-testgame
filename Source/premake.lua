rootDir = "../"

workspace "sdl2-testgame"			
    location (rootDir .. "Local/Build")
	configurations { "Debug", "Release" }

project "Main"
	kind "ConsoleApp"
	language "C++"
	targetdir (rootDir .. "Local/Bin/%{cfg.buildcfg}")
	
	includedirs { rootDir .. "External/SDL2/include" }
	includedirs { rootDir .. "External/gsl" }
	libdirs { rootDir .. "External/SDL2/lib/x86/" }
    libdirs { rootDir .. "External/SDL2/lib/osx/" }
	links { "SDL2", "SDL2main", "OpenGL.framework" }
	
	debugdir (rootDir .. "Bin/x86")
	
	files { "**.h", "**.cpp" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

if not (os.isdir(rootDir .. "Bin"))	then
	os.mkdir(rootDir .. "Bin")
end

function copyToBin (sourcePath, file)
    success, errormsg = os.copyfile(path.translate(rootDir .. sourcePath .. file), path.translate(rootDir .. "Bin/" .. file))	
    if (success == nil) then
	   print("Cannot copy binaries: " .. errormsg)
    end
end    

copyToBin("External/SDL2/lib/x86/", "SDL2.dll");
copyToBin("External/SDL2/lib/osx/", "libSDL2.dylib");