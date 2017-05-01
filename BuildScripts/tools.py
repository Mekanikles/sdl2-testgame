import os
import errno
import tarfile
import urllib2
import subprocess
import exceptions
import zipfile
import shutil

def makeDirRecursive(path):
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

def checkExternalFiles(fileList, externalDir):
	hasPrerequisites = True
	for f in fileList:
		p = os.path.join(externalDir, f)
		if not os.path.exists(p):
			print "Required file '" + p + "' does not exist"
			hasPrerequisites = False
	return hasPrerequisites


def downloadFile(url, target):
    try:
        file = urllib2.urlopen(url)
        print "Dowloading file: '" + url + "' To '" + target + "'"
        with open(target, "wb") as targetFile:
            targetFile.write(file.read())
    except urllib2.HTTPError, e:
        print "HTTP Error:", e.code, url
    except urllib2.URLError, e:
        print "URL Error:", e.reason, url

def requireDownloadedFile(url):
	downloadDir = "./Downloads"
	downloadTarget = os.path.join(downloadDir, os.path.basename(url))

	if not os.path.exists(downloadTarget):
		if not os.path.exists(downloadDir):
			os.mkdir(downloadDir)
		downloadFile(url, downloadTarget)
	return downloadTarget

def unpackTarArchive(archive, targetDir):
	tar = tarfile.open(archive)
	try:
		tar.extractall(targetDir)
	except:
		raise
	finally:	
		tar.close()

def unpackZipArchive(archive, targetDir):
	zip = zipfile.ZipFile(archive, 'r')
	try:
		zip.extractall(targetDir)
	except:
		raise
	finally:	
		zip.close()

def unpackArchive(archive, targetDir):
	print "Unpacking '" + archive + "' To '" + targetDir + "'"
	filename, fileExtension = os.path.splitext(archive)
	if fileExtension == ".gz" or fileExtension == ".tar":
		unpackTarArchive(archive, targetDir)
	elif fileExtension == ".zip":
		unpackZipArchive(archive, targetDir)	
	else:
		raise RuntimeError("Unknown archive format")
			
def findRootDir(searchDir, searchFile):
	for root, subFolders, files in os.walk(searchDir):
		if searchFile in files:
			print "Found file " + searchFile + " at '" + root + "'"
			return root
		if searchFile in subFolders:
			print "Found dir " + searchFile + " at '" + root + "'"
			return root	

	raise RuntimeError("Could not find source directory")

def findDir(searchDir, searchFile):
	root = findRootDir(searchDir, searchFile)
	return os.path.join(root, searchFile)

def copyDir(sourceDir, destDir):
	dirName = os.path.basename(sourceDir)
	shutil.copytree(sourceDir, os.path.join(destDir, dirName), ignore = shutil.ignore_patterns('*.DS_Store', '*.Trashes'))

def callProcess(args):
	p = subprocess.Popen(args)
	p.wait()
	if (p.returncode != 0):
		raise RuntimeError("Process failed")

def configureInstallSource(configureDir, buildDir, params):
	configurePath = os.path.join(os.path.abspath(configureDir), "configure")
	curDir = os.getcwd()
	os.chdir(buildDir)
	try:
		callParams = [ configurePath ] + params
		print "Configuring build in directory '" + buildDir + "' from source '" + configureDir + "' using params '" + " ".join(params) + "'..."
		callProcess(callParams)
		print "Installing build..."
		callProcess(["make", "install"])
	except:
		raise
	finally:
		os.chdir(curDir)

def cmakeInstallSource(sourceDir, buildDir, params):
	sourcePath = os.path.abspath(sourceDir)
	curDir = os.getcwd()
	os.chdir(buildDir)
	try:
		callParams = [ "cmake" ] + params + [ sourcePath ]
		print "Configuring build in directory '" + buildDir + "' from source '" + sourcePath + "' using params '" + " ".join(params) + "'..."
		callProcess(callParams)
		print "Installing build..."
		callProcess(["make", "install"])
	except:
		raise
	finally:
		os.chdir(curDir)







