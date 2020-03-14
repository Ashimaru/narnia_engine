#!/usr/bin/python3

import sys
import os
from subprocess import call
	
shaderSource = sys.argv[1]
shaderDest = sys.argv[2]

def compileShader(filename, registry):
	if filename.endswith(".py"):
		return
	filename = os.path.basename(file)
	name = os.path.splitext(filename)[0]
	outputFileName = name + ".spv"
	outputPath =  shaderDest + outputFileName
	inputPath = shaderSource + "\\" + filename
	command = shaderCompiler + " -V " + inputPath + " -o " + outputPath
	print("Calling command: " + command)
	call(command)
	registry.write(name + " " + outputFileName + "\n")


vulkanPath = os.environ['VULKAN_SDK']
shaderCompiler = vulkanPath + "\\Bin\\glslangValidator.exe"

print("glslValidator path: " + shaderCompiler)

shaderSrc = os.listdir(shaderSource)
regFile = shaderDest + "\\index.lst"

if os.path.isfile(regFile):
	os.remove(regFile)

if not os.path.exists(shaderDest):
	os.makedirs(shaderDest)

with open(regFile, 'a') as registry:
	for file in shaderSrc:
		compileShader(file, registry)


	


	

	

	


