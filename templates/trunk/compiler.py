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
	print "# template %s" % name
	print "export %s" % name

def inline_content(content):
	if content=='' :
		return
	print "inline '%s'" % content[1:5]

def error_exit(x):
	print "Error %s" % x
#	exit(1)

def ifop(param,name):
	print "\tif %s\t%s" % (param , name)

def prepare(calls):
	list =[ [ r'^<%\s*template\s+([a-zA-Z]\w*)\s*%>$' , (lambda x: template(x.group(1))) ], \
		[ r'^<%\s*(elif|if)(\s+(not|\s+)([a-zA-Z]\w*)\s*%>$' , (lambda x : ifop('true',x)) ],\
		[ r'^<%\s*(elif|if)\s+not\s+([a-zA-Z]\w*)\s*%>$' , (lambda x : ifop('false',x)) ],\
		[ r'^<%\s*(if\s+def\s+([a-zA-Z]\w*)\s*%>$' , (lambda x : ifop('def',x)) ],\
		[ r'^<%\s*if\s+not\s+def\s+([a-zA-Z]\w*)\s*%>$' , (lambda x : ifop('ndef',x)) ],\
		[ r'^<%\s*if\s+not\s+def\s+([a-zA-Z]\w*)\s*%>$' , (lambda x : ifop('ndef',x)) ],\
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
			for c in calls:
				m = c.pattern.match(x)
				if m :
					c.call(m.group(1))

		if(len(stack)!=0):
			sys.stderr.write("Unexpected end of file %s\n" % file)
			exit(1)

stack=[]
calls=[]
prepare(calls);
main()

