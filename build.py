#!/usr/bin/env python2.7
from BuildScripts.platform import *
from BuildScripts.tools import *
import os
import sys

externalDir = "./External"

platform = Platform()

if (platform.isMacOS()):
	print "Building for MacOS."

	# SDL2
	sdl2Dir = platform.verifyExternalDir("SDL2")
	sdl2Files = [ "bin", "include", "lib" ]
	if not checkExternalFiles(sdl2Files, sdl2Dir):
		archive = requireDownloadedFile("https://www.libsdl.org/release/SDL2-2.0.5.tar.gz")
		sourceDir = platform.verifyExternalSourceDir("SDL2")
		unpackArchive(archive, sourceDir)
		sourceDir = findRootDir(sourceDir, "configure")
		configureInstallSource(sourceDir, platform.verifyExternalBuildDir("SDL2"), 
			["CC=sh " + os.path.abspath(os.path.join(sourceDir, "build-scripts/gcc-fat.sh")),
			"--prefix=" + os.path.abspath(sdl2Dir)])

	# GLM
	glmDir = platform.verifyExternalDir("glm")
	glmFiles = ["include"]
	if not checkExternalFiles(glmFiles, glmDir):
		archive = requireDownloadedFile("https://github.com/g-truc/glm/archive/0.9.8.4.tar.gz")
		sourceDir = platform.verifyExternalSourceDir("glm")
		unpackArchive(archive, sourceDir)
		sourceDir = findRootDir(sourceDir, "CMakeLists.txt")
		cmakeInstallSource(sourceDir, platform.verifyExternalBuildDir("glm"),
			["-DCMAKE_INSTALL_PREFIX:PATH=" + os.path.abspath(glmDir)])

	# GSL
	gslDir = platform.verifyExternalDir("gsl")
	gslFiles = ["include"]
	if not checkExternalFiles(gslFiles, gslDir):
		archive = requireDownloadedFile("https://github.com/Microsoft/GSL/archive/master.zip")
		sourceDir = platform.verifyExternalSourceDir("gsl")
		unpackArchive(archive, sourceDir)
		sourceDir = findDir(sourceDir, "include")
		copyDir(sourceDir, gslDir)

	# Nuklear
	nuklearDir = platform.verifyExternalDir("nuklear")
	nuklearFiles = ["nuklear.h"]
	if not checkExternalFiles(gslFiles, gslDir):
		print "Cannot find nuklear.h!"
		raise -1

	print "Generating projects..."
	subprocess.call(["premake5", "--file=./Source/premake.lua", "xcode4"])
	print "Compiling..."
	retcode = subprocess.call(["xcodebuild", "-project", "Local/Build/Main.xcodeproj", "-configuration", "Debug"])
	sys.exit(retcode)

else:
	print "Platform '" + platform.name() + "' not supported"
