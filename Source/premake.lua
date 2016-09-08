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
	includedirs { rootDir .. "External/glm" }
	includedirs { rootDir .. "External/glew/include" }
	
	links { "SDL2", "SDL2main" }
	debugdir (rootDir .. "Bin")
	
	files { "**.h", "**.cpp" }
		
	filter "system:Windows"
		libdirs { rootDir .. "External/SDL2/lib/x86/" }
		libdirs { rootDir .. "External/glew/lib/Release/Win32/" }
		links { "opengl32", "glu32", "glew32s"}
		
	filter "system:MacOSX"
		libdirs { rootDir .. "External/SDL2/lib/osx/" }
		links { "OpenGL.framework" }
	
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