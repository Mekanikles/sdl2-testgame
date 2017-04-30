import os
import errno
import tarfile
import urllib
import subprocess
import exceptions

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

def requireDownloadedFile(url):
	downloadDir = "./Downloads"
	downloadTarget = os.path.join(downloadDir, os.path.basename(url))

	if not os.path.exists(downloadTarget):
		if not os.path.exists(downloadDir):
			os.mkdir(downloadDir)

		file = urllib.URLopener()
		print "Dowloading file: '" + url + "' To '" + downloadTarget + "'"
		file.retrieve(url, downloadTarget)

	return downloadTarget

def unpackArchive(archive, targetDir):
	print "Unpacking '" + archive + "' To '" + targetDir + "'"
	tar = tarfile.open(archive)
	try:
		tar.extractall(targetDir)
	except:
		raise
	finally:	
		tar.close()

def findDir(searchDir, searchFiles):
	for root, subFolders, files in os.walk(searchDir):
		containsFile = set(searchFiles).intersection(files)
		if containsFile:
			print "Found source at '" + root + "'"
			return root

def callProcess(args):
	p = subprocess.Popen(args)
	p.wait()
	if (p.returncode != 0):
		raise RuntimeError("Process failed")

def configureInstallSource(configureDir, buildDir, params):
	makeDirRecursive(buildDir)
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






