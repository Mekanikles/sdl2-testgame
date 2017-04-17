rootDir = "../"

workspace "sdl2-testgame"			
    location (rootDir .. "Local/Build")
	configurations { "Debug", "Release" }

project "Main"
	kind "ConsoleApp"
	language "C++"
	targetname ("SDL2Test")
	
	includedirs { rootDir .. "External/SDL2/include/" }
	includedirs { rootDir .. "External/gsl/" }
	includedirs { rootDir .. "External/glm/" }

	files { "**.h", "**.cpp" }
	flagList = { "C++14", "MultiProcessorCompile" }
	buildoptions ("-std=c++14")
	linkoptions ("-stdlib=libc++")	

	filter "system:Windows"
        defines { "__WINDOWS__" }
        includedirs { rootDir .. "External/glew/include" }
		libdirs { rootDir .. "External/SDL2/lib/x86/" }
		libdirs { rootDir .. "External/glew/lib/Release/Win32/" }
		links { "opengl32", "glu32", "glew32s"}
		links { "SDL2", "SDL2main" }
		--copylocal { "SDL2" }	maybe ?

	filter "system:MacOSX"
        defines { "__OSX__" }
		frameworkdirs { "/Library/Frameworks" }
		links { "OpenGL.framework" }
		links { "SDL2.framework" }


	filter "configurations:Debug"
		defines { "DEBUG" }
		binDir = rootDir .. "Local/Bin/Debug/"
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		binDir = rootDir .. "Local/Bin/Release/"
		optimize "On"

	filter {} 
	flags (flagList)
	debugdir (binDir)
	targetdir (binDir)

if not (os.isdir(binDir))	then
	os.mkdir(binDir)
end

function copyToBin (sourcePath, file)
	source = path.translate(rootDir .. sourcePath .. file)
	dest = path.translate(binDir .. file)
    success, errormsg = os.copyfile(source, dest)	
    if (success == nil) then
		print("Cannot copy binaries: " .. errormsg)
	else
		print("Copied " .. dest)
    end
end    

--copyToBin("External/SDL2/lib/x86/", "SDL2.dll");
--copyToBin("External/SDL2/lib/osx/", "libSDL2.dylib");
