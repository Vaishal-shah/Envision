#!/usr/bin/python2

import os
import argparse
import re
import sys

# Argument parsers
argParser = argparse.ArgumentParser('Sort the declarations in a source file')
argParser.add_argument('inputFile')
argParser.add_argument('outputFile')

args = argParser.parse_args()

# Read entire file in a single string
with open(args.inputFile, 'r') as inputFile:
	sourceText = inputFile.read()
	if not sourceText.endswith('\n'):
		sourceText.append('\n')


# Block class used to store and various fragments of the file
class Block:
	methodRegex = re.compile(r'.*\)(\s|const|override)*\s*(=\s*\w+)?(\s|\n)*{(\s|\n)*$', re.DOTALL)
	
	def __init__(self):
		self.prefix = ""
		self.suffix = ""
		self.children = []
		self.finalized = False
	
	def add(self, char):
		if self.finalized:
			self.suffix += char
		else:
			self.prefix += char
			
	def removeLast(self, char):
		if self.finalized:
			assert self.suffix.endswith(char)
			self.prefix = self.prefix[:-1]
		else:
			assert self.prefix.endswith(char)
			self.prefix = self.prefix[:-1]
	
	def deepen(self):
		assert not self.finalized
		self.children.append(Block())
		return self.children[-1]
	
	def finalize(self):
		assert not self.finalized
		self.finalized = True
		
	def debugPrint(self):
		if not self.prefix and not self.suffix and not self.children:
			return
		sys.stdout.write('>>>')
		sys.stdout.write(self.prefix)
		match = self.methodRegex.match(self.prefix)
		if match:
			sys.stdout.write('METHOD BODY')
		else:
			for c in self.children:
				assert c != self
				c.debugPrint()
			
		sys.stdout.write(self.suffix)
		sys.stdout.write('<<<')
		
	def sort(self):
		match = self.methodRegex.match(self.prefix)
		if not match:
			self.children.sort(key = lambda x : x.sortingString())
			for c in self.children:
				c.sort()
			
	def sortingString(self):
		return self.prefix
			
	def text(self):
		return self.prefix + (''.join(c.text() for c in self.children))  + self.suffix

# Store current nesting level in a stack
top = Block() # Here we will collect all blocks
stack = [top, top.deepen()]

inString = False
escaped = False
quote = "" #Either " or '

inComment = False
lineComment = False

prev = '' # previous character
current = '' # current character
prevPeer = None # previous peer node

# Loop over all the characters and build the tree of Block structures
for char in sourceText:
	prev = current
	current = char
	stack[-1].add(char)
	
	if inString:
		assert char != '\n'
		if escaped:
			escaped = False
			continue
		if char == quote:
			inString = False
			continue
		if char == '\\':
			escaped = True
			continue
		continue
	
	if inComment:
		if lineComment and char == '\n':
			lineComment = False
			inComment = False
			continue
		if not lineComment and prev == '*' and char == '/':
			inComment = False
			continue
		continue
	
	assert not inString and not escaped and not inComment and not lineComment
	
	if char == '/' and prev == '/':
		inComment = True
		lineComment = True
		continue
	
	if char == '*' and prev == '/':
		inComment = True
		continue
	
	if char == '"' or char =='\'':
		inString = True
		quote = char
		continue
	
	if char == '{':
		stack.append( stack[-1].deepen() )
		continue
	
	if char == '}' or ( char ==';' and not prev == '}') or (char == '\n' and prev == ':'):
		if char == '}':
			stack[-1].removeLast(char)
			stack[-2].finalize()
			stack[-2].add(char)
			stack = stack[:-1]
		else:
			stack[-1].finalize()
			
		prevPeer = stack[-1] # we need this to handle ; and :
		stack = stack[:-1]
		stack.append( stack[-1].deepen())
		continue
	
	if prev == '}' and (char == ';' or char == '\n'):
		stack[-1].removeLast(char)
		prevPeer.add(char)
		continue
	
	if prev == ';' and char == '\n':
		stack[-1].removeLast(char)
		prevPeer.add(char)
		continue
	
	if prev == '{' and char == '\n':
		stack[-1].removeLast(char)
		stack[-2].add(char)
		continue

#top.debugPrint() # Use this to debug things

top.sort()
with open(args.outputFile, 'w') as outputFile:
	outputFile.write(top.text())