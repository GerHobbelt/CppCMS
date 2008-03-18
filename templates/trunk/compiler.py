#!/usr/bin/env python

import os
import re
import sys

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


class template:
	pattern=r'^<%\s*template\s+([a-zA-Z]\w*)\s*%>$'
	def use(self,m):
		name=m.group(1)
		print "# template %s" % name
		print "export %s" % name

class inline_content:
	pattern=r'(.*)';
	def use(self,m):
		content=m.group(0)
		print "\tinline '%s'" % to_string(content)

def error_exit(x):
	global exit_flag
	print "Error %s" % x
	exit_flag=1

def new_label():
	global labels_counter
	s="LBL_%d" % labels_counter
	labels_counter+=1
	return s

def to_string(s):
	res=''
	for ch in s:
		if ch < ' ':
			res+=("\\x%02x" % ch)
		else:
			res+=ch
	return res


def sequence(seq_name):
	if tmpl_seq.has_key(seq_name):
		return tmpl_seq[seq_name]
	error_exit("Undefined sequence %s" % seq_name)
	# FIIIX MMMEEE
	return 0

def make_ident(val):
	m=re.match(r'^ARG_(\d)$',val)
	if m :
		return "%s" % (template_id*10+int(m.group(1)))
	m=re.match(r'^\w+$',val)
	if m:
		return val
	m=re.match(r'^(\w+)\.(\w+)$',val)
	seq_id=sequence(m.group(1))
	return "%s(%d)" % ( m.group(2),seq_id )

class foreach_block:
	pattern=r'^<%\s*foreach\s+in\s+([a-zA-Z]\w*(\.([a-zA-Z]\w*))?)\s*%>$'
	def use(self):
		if tmpl_seq.has_key(seq_name):
			error_exit("Nested sequences with same name")
		pass
	def on_end(self):
		pass

class ifop:
	pattern=r'^<%\s*(if|elif)\s+((not)\s+)?((def)\s+)?([a-zA-Z]\w*(\.([a-zA-Z]\w*))?)\s*%>$'
	type='if'
	def prepare(self):
		ident_str=make_ident(self.ident)
		self.label=new_label()
		if self.has_def:
			print "\tdef\t%s" % ident_str
		else:
			print "\ttrue\t%s" % ident_str
		if self.has_not:
			print "\tjmp\tt,%s" % self.label
		else:
			print "\tjmp\tf,%s" % self.label

	def on_end(self):
		print "%s:" % self.label

	def use(self,m):
		global stack
		self.type=m.group(1)
		self.has_not=m.group(3)
		self.has_def=m.group(5)
		self.ident=m.group(6)
		if self.type == 'if' :
			stack.append(self)
			self.prepare()
		else: # type == elif
			if len(stack)!=0 :
				prev=stack.pop()
				if prev.type!='if':
					error_exit("elif without if");
				prev.on_end()
				stack.append(self)
				self.prepare()
			else:
				error_exit("Unexpeced elif");
# END ifop				
			

class end_block:
	pattern=r'^<%\s*end\s*%>';
	def use(self,m):
		global stack
		if len(stack)==0:
			error_exit("Unexpeced 'end'");
		else:
			stack.pop().on_end()

class error_com:
	pattern=r'^<%(.*)%>$'
	def use(self,m):
		error_exit("unknown command %s" % m.group(1))

def main():
	global stack
	for file in os.sys.argv[1:]:
		f=open(file,'r')
		content=f.read()
		f.close()
		texts=re.split(r'<\%[^\%]*\%\>',content)
		commands=re.findall(r'<\%[^\%]*\%\>',content)
		for x in interleave(texts,commands):
			if x=='' : continue
			for c in [  ifop(), template(), end_block(), error_com() , inline_content()]:
				m = re.match(c.pattern,x)
				if m :
					c.use(m)
					break

		if(len(stack)!=0):
			sys.stderr.write("Unexpected end of file %s\n" % file)
			exit(1)

labels_counter=0
tmpl_seq={}
seq_no=0
stack=[]
exit_flag=0
main()
