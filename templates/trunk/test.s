extern main
	inline	'Hello World\x0a'
	show	title
	inline	'\x0a'
###
	seqf	list,1,empty
next:
	sto	author(1),0
	sto	content(1),1
	callr	proc
	seqn	1,end
	jmp	u,next
empty:
	inline	'Nothing'
	jmp	u,end
end:	
	sto	list,2
	call test

	def	someval
	jmp	f,l2
	true	someval
	jmp	t,x1
	inline	'false\x0a'
	jmp	u,x2

x1:	
	inline	'true\x0a'
x2:
	jmp	u,l1
l2:
	inline	'Nothing'
l1:
	inline '\x0a'
	ret
##############	
extern	test
	seqf	2,0,empty2
next2:
	sto	author(0),0
	sto	content(0),1
	call	xml
	seqn	0,end2
	jmp	u,next2
empty2:
	inline	'Nothing'
	jmp	u,end2
end2:	
	ret
#############	
extern	text
	inline	'Author: '
	show	0
	inline	' content: '
	show	1
	inline	'\x0a'
	ret
extern	xml
	inline	'<author>'
	show	0
	inline	'</author><content>'
	show	1
	inline	'</content>\x0a'
	ret

