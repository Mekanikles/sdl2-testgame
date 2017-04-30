#!/usr/bin/env python2.7
from BuildScripts.platform import *
from BuildScripts.tools import *
import os

externalDir = "./External"

platform = Platform()

if (platform.isMacOS()):
	print "Building for MacOS."

	sdl2Dir = platform.verifyExternalDir("SDL2")
	sdl2Files = [ "bin", "include", "lib" ]
	if not checkExternalFiles(sdl2Files, sdl2Dir):
		archive = requireDownloadedFile("https://www.libsdl.org/release/SDL2-2.0.5.tar.gz")
		sourceDir = platform.verifyExternalSourceDir("SDL2")
		unpackArchive(archive, sourceDir)
		sourceDir = findDir(sourceDir, ["configure"])
		print sourceDir
		configureInstallSource(sourceDir, platform.verifyExternalBuildDir("SDL2"), 
			["CC=sh " + os.path.abspath(os.path.join(sourceDir, "build-scripts/gcc-fat.sh")),
			"--prefix=" + os.path.abspath(sdl2Dir)])

	print "Generating projects..."
	subprocess.call(["premake5", "--file=./Source/premake.lua", "xcode4"])
	print "Compiling..."
	subprocess.call(["xcodebuild", "-project", "Local/Build/Main.xcodeproj", "-configuration", "Debug"])

else:
	print "Platform '" + platform.name() + "' not supported"
