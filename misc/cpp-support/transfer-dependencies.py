#!/usr/bin/python3

import os
import argparse
import re
import sys
import functools

### Arguments
argParser = argparse.ArgumentParser('Change the includes of a source file based on those of anohter')
argParser.add_argument('fileToAdjust')
argParser.add_argument('infoFile')
args = argParser.parse_args()

### Check input
if not os.path.isfile(args.fileToAdjust):
	print('Supplied file to adjust does not exist: ' + args.fileToAdjust)
	quit()
	
if not os.path.isfile(args.infoFile):
	print('Supplied info file does not exist: ' + args.infoFile)
	quit()

### Global declarations
includeRegex = re.compile('^(#include ".*".*)$')
commentedIncludeRegex = re.compile('^//I #include ".*".*$')
possiblyCommentedIncludeRegex = re.compile('^(?://I )?#include "(.*)".*$')
forwardDeclarationRegex = re.compile('^(\s*(template<.*>\s*\n?\s*)?(class|struct)\s+\w+;\s*(//.*)?)$')
possiblyCommentedForwardDeclarationRegex = re.compile('^((?:\s*//FD )?\s*(template<.*>\s*\n?\s*)?(class|struct)\s+\w+;\s*(//.*)?)$')
templateBeginningRegex = re.compile('^\s*template\s*<.+>$')
namespaceRegex = re.compile('^namespace\s+(\w+)')
emptyLineRegex = re.compile('^\s*$')

currentNamespace = ''
prevTemplate = ''

### Read Info File
includesToMatch = []
forwardsToMatch = []

with open(args.infoFile, 'r') as info:
	for line in info:
		line = line.rstrip()
		
		m = namespaceRegex.match(line)
		if m:
			prevTemplate = ''
			currentNamespace = m.group(1)
			continue
		
		m = includeRegex.match(line)
		if m:
			prevTemplate = ''
			includesToMatch.append(line)
			continue
		
		m = forwardDeclarationRegex.match(line)
		if m:
			forwardsToMatch.append(currentNamespace + ':' + prevTemplate + (' ' if prevTemplate else '') + line.strip())
			prevTemplate = ''
			continue
		
		m = templateBeginningRegex.match(line)
		if m:
			prevTemplate = line.strip()
			continue
			

### Helper class

# A part is essentially a namespace and associated lines
# The global namespace is ''
# The headers are in their own part (to enable sorting)
class Part:
	def __init__(self, namespace):
		self.namespace = namespace
		self.lines = []
		self.forwardDeclarationInsertIndex = -1
		self.forwardDeclarationIndent = 'unknown'
	
	def append(self, line):
		self.lines.append(line)
		if self.forwardDeclarationIndent == 'unknown' and len(self.lines) > 1 and line.strip():
			# This is the first non-empty line after the initial one. Remember its indent
			if line.startswith('\t'):
				self.forwardDeclarationIndent = '\t'
			else:
				self.forwardDeclarationIndent = ''
		
	def addForwardDeclaration(self, line):
		if self.forwardDeclarationInsertIndex == -1:
			self.forwardDeclarationInsertIndex = self.findForwardDeclarationInsertIndex()
			
		if self.namespace:
			assert len(self.lines) > 0
		self.lines.insert(self.forwardDeclarationInsertIndex, self.forwardDeclarationIndent + line)
		self.forwardDeclarationInsertIndex += 1
	
	def findForwardDeclarationInsertIndex(self):
		for i in range(len(self.lines)-1,-1,-1):
			m = possiblyCommentedForwardDeclarationRegex.match(self.lines[i])
			if m:
				return i + 1

		# There are no other forward declarations
		if self.namespace:
			assert len(self.lines) > 0
			return 1
		else:
			return 0
		

### Change detection
fileHasBeenAdjusted = False

### Helper methods
def checkInclude(include):
	if include in includesToMatch:
		includesToMatch.remove(include)
		return include
	elif '.hpp"' in include:
		return include
	else:
		fileHasBeenAdjusted = True
		return '//I ' + include
	
def checkForwardDeclaration(namespace, forwardDeclaration):
	stripped = forwardDeclaration.strip()
	name = namespace + ':' + stripped
	if name in forwardsToMatch:
		forwardsToMatch.remove(name)
		return forwardDeclaration
	else:
		fileHasBeenAdjusted = True
		return '//FD ' + stripped
		
### Process input file
parts = [Part('')]
originalIncludes = []

with open(args.fileToAdjust, 'r') as fileToAdjust:
	for line in fileToAdjust:
		line = line.rstrip()
		if (len(parts) == 1):
			# We have not yet seen a header
			m = includeRegex.match(line)
			if m:
				parts.append(Part(''))
				# fallthrough
			else: 
				parts[-1].append(line)
				continue
			
		if (len(parts) == 2):
			# We are currently processing includes, commented includes and empty lines between them
			m = includeRegex.match(line)
			if m:
				originalIncludes.append(line)
				parts[-1].append(checkInclude(line))
				continue
			
			m = emptyLineRegex.match(line)
			if m:
				originalIncludes.append(line)
				continue
			
			m = commentedIncludeRegex.match(line)
			if m:
				originalIncludes.append(line)
				parts[-1].append(line)
				continue
			
			# We are done with headers
			m = namespaceRegex.match(line)
			if not m:
				parts.append(Part(''))
			# fallthrough
			
		# We are done with headers and are processing namespaces
		m = namespaceRegex.match(line)
		if m:
			parts.append(Part(m.group(1)))
			parts[-1].append(line)
			continue
		
		m = forwardDeclarationRegex.match(line)
		if m:
			parts[-1].append(checkForwardDeclaration(parts[-1].namespace, line))
		else:
			parts[-1].append(line)

### Append the remaining includes
fileHasBeenAdjusted = fileHasBeenAdjusted or len(includesToMatch) > 0
parts[1].lines.extend(includesToMatch)

### Append the remaining forward declarations
fileHasBeenAdjusted = fileHasBeenAdjusted or len(forwardsToMatch) > 0
for descriptor in forwardsToMatch:
	words = descriptor.split(':')
	assert len(words) == 2
	namespace = words[0]
	forwardDeclaration = words[1]
	
	namespaceFound = False
	for part in reversed(parts[2:]):
		if part.namespace == namespace:
			namespaceFound = True
			part.addForwardDeclaration(forwardDeclaration)
			break
		
	if not namespaceFound:
		newPart = Part(namespace)
		newPart.append('namespace ' + namespace + ' {')
		newPart.append(forwardDeclaration)
		newPart.append('}')
		newPart.append('')
		
		if parts[2].namespace == '':
			parts.insert(3, newPart)
		else:
			parts.insert(2, newPart)

### Sort the includes
matchingHeaderFile = os.path.basename( os.path.splitext(args.fileToAdjust)[0]) + '.h'
projectNamesRegex = re.compile('^(?:Core|Launcher|HelloWorld|APIDepTest|SelfTest|Logger|ModelBase|FilePersistence|VisualizationBase|InteractionBase|Comments|Export|OOModel|OOVisualization|OOInteraction|JavaExport|OODebug|CppExport|CppImport|PythonWrapperGenerator|InformationScripting|ContractsLibrary|Alloy|CustomMethodCall|ControlFlowVisualization)/.*')

def compareTwoIncludes(includeA, includeB):
	m = possiblyCommentedIncludeRegex.match(includeA)
	assert m
	a = m.group(1)

	m = possiblyCommentedIncludeRegex.match(includeB)
	assert m
	b = m.group(1)
	
	# API file
	apiA = a.endswith('_api.h')
	apiB = b.endswith('_api.h')
	if apiA != apiB:
		return -1 if apiA else 1
	
	# Matching header
	if a == matchingHeaderFile:
		return 0 if b == matchingHeaderFile else -1
	if b == matchingHeaderFile:
		return 1
	
	# External vs non-external
	externalA = projectNamesRegex.match(a)
	externalB = projectNamesRegex.match(b)
	if externalA != externalB:
		return 1 if externalA else -1
	
	# no dot before dot
	dotA = a.startswith('.')
	dotB = b.startswith('.')
	if dotA != dotB:
		return 1 if dotA else -1
	
	#lexical comparison
	return (a > b) - (a < b)

parts[1].lines.sort(key = functools.cmp_to_key(compareTwoIncludes))

### Space includes with empty lines

# empty line after first include
firstNonStandardInclude = 0
if parts[1].lines[0].endswith('_api.h"') or parts[1].lines[0] == '#include "' + matchingHeaderFile + '"':
	parts[1].lines.insert(1, '')
	firstNonStandardInclude = 2

# empty line after plug-in and before external includes
for i in range(firstNonStandardInclude, len(parts[1].lines)):
	if projectNamesRegex.match(parts[1].lines[i].replace('#include "','')):
		if i > firstNonStandardInclude:
			parts[1].lines.insert(i,'')

		break
	
# empty line after all includes if that's not already the case
if not parts[1].lines[-1] == '':
	parts[1].lines.append('')

fileHasBeenAdjusted = fileHasBeenAdjusted or originalIncludes != parts[1].lines

### Write the output file
if fileHasBeenAdjusted:
	print('Adjusting ' + args.fileToAdjust)
	with open(args.fileToAdjust, 'w') as outputFile:
		for part in parts:
			for line in part.lines:
				outputFile.write(line + '\n')
else:
	print('No need for adjustment ' + args.fileToAdjust)