#!/usr/bin/env python

import os
import re

class tocken:
	def __init__(self,name,param):
		self.name=name
		self.val=param


def interleave(*args):
	for idx in range(0, max(len(arg) for arg in args)):
		for arg in args:
			try:
				yield arg[idx]
			except IndexError:
				continue

class callback:
	def __init__(self,pattern,call):
		self.pattern=pattern
		self.call=call

def template(name):
	print "###### template %s\nexport %s" % name % name

def inline_content(content):
	print "inline '%s'" % content[1:5]

def error_exit(x):
	print "Error %s" % x
#	exit(1)

def if_true(name):
	print "\tdef\t%s" %name

def prepare(calls):
	list =[ [ r'^<%\s*template\s+([a-zA-Z]\w*)\s*%>$' , template ], \
		[ r'^<%\s*if\s+([a-zA-Z]\w*)\s*%>$' , if_true ],\
		[ r'^<%(.*)%>$',(lambda s: error_exit("Unkown command '%s'"%s)) ], \
		[ r'^(.*)$',inline_content] ]
	for a in list:
		calls.append(callback(re.compile(a[0]),a[1]))

def main():
	for file in os.sys.argv[1:]:
		f=open(file,'r')
		content=f.read()
		f.close()
		texts=re.split(r'<\%[^\%]*\%\>',content)
		commands=re.findall(r'<\%[^\%]*\%\>',content)
		for x in interleave(texts,commands):
			if x=='' : continue
			for call in calls:
				m = call.pattern.match(x)
				if m :
					call.call(m.group(1))

		if(len(stack)!=0):
			sys.stderr.write("Unexpected end of file %s\n" % file)
			exit(1)

stack=[]
calls=[]
prepare(calls);
main()

