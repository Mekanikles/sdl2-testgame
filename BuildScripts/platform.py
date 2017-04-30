import os
import sys
import tools

class Platform:
	def __init__(self):
		sysName = sys.platform
		name = None
		if sysName == "linux" or sysName == "linux2":
			name = "Linux"
		elif sysName == "darwin":
			name = "MacOS"
		elif sysName == "win32":
			name = "Win32"
		self.m_sysName = sysName
		self.m_name = name

	def name(self):
		if not self.m_name:
			return self.m_sysName
		return self.m_name

	def verifyExternalDir(self, externalName):
		path = os.path.join("External", externalName, self.name())
		tools.makeDirRecursive(path)
		return path

	def verifyExternalSourceDir(self, externalName):
		path = os.path.join("ExternalSource", externalName)
		tools.makeDirRecursive(path)
		return path

	def verifyExternalBuildDir(self, externalName):
		path = os.path.join("Local/ExternalBuild", externalName, self.name())
		tools.makeDirRecursive(path)
		return path

	def isMacOS(self):
		return (self.m_name == "MacOS")
