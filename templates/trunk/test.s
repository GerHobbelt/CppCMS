extern main
	inline	'Hello World\x0a'
	show	title
#	ret
	inline	'\x0a'
	seqf	list,1,empty
next:
	inline	'Author: '
	show	author(1)
	inline	'content: '
	show	content(1)
	seqn	1,end
	jmp	u,next
empty:
	inline	'Nothing'
	jmp	u,end
end:	
	ret

