rootDir = "../"

workspace "sdl2-testgame"			
    location (rootDir .. "Local/Build")
	configurations { "Debug", "Release" }

project "Main"
	kind "ConsoleApp"
	language "C++"
	targetname ("SDL2Test")

	files { "**.h", "**.cpp" }
	flags { "C++14", "MultiProcessorCompile" }

	buildoptions ("-std=c++14", "-ffast-math")
	linkoptions ("-stdlib=libc++")	

	filter "system:Windows"
        defines { "__WINDOWS__" }
		--includedirs { rootDir .. "External/SDL2/include/" }
		includedirs { rootDir .. "External/gsl/" }
		includedirs { rootDir .. "External/glm/" }       
        includedirs { rootDir .. "External/glew/include" }
		libdirs { rootDir .. "External/SDL2/lib/x86/" }
		libdirs { rootDir .. "External/glew/lib/Release/Win32/" }
		links { "opengl32", "glu32", "glew32s"}
		links { "SDL2", "SDL2main" }
		--copylocal { "SDL2" }	maybe ?

	filter "system:MacOSX"
        defines { "__OSX__" }
		includedirs { rootDir .. "External/gsl/MacOS/include" }
		includedirs { rootDir .. "External/glm/MacOS/include" }
		includedirs { rootDir .. "External/nuklear/MacOS" }

		buildoptions ( os.outputof(rootDir .. "External/SDL2/MacOS/bin/sdl2-config --cflags") )
		links { "OpenGL.framework" }
		linkoptions ( os.outputof(rootDir .. "External/SDL2/MacOS/bin/sdl2-config --libs") )

	filter "configurations:Debug"
		defines { "DEBUG" }
		binDir = rootDir .. "Local/Bin/Debug/"
		debugdir (binDir)
		targetdir (binDir)
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		binDir = rootDir .. "Local/Bin/Release/"
		debugdir (binDir)
		targetdir (binDir)	
		optimize "On"

	filter {} 

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

