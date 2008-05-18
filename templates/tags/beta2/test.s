extern main
	gt 'hello\x0a'
	showf	v1
	showf	v2
	gt 'Hello [%2%] [%1%] [%3%]\x0a'
	ngt n1,'passed one day\x0a','passed %d days\x0a'
	ngt n2,'passed one day\x0a','passed %d days\x0a'
	ngt n3,'passed one day\x0a','passed %d days\x0a'
	ngt n11,'passed one day\x0a','passed %d days\x0a'
	ret 
