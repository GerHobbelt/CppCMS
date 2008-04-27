extern main
	gt 'Hello'
	showf	v1
	showf	v2
	gt 'Hello [%2%] [%1%] [%3%]\x0a'
	ngt n1,'passed one day','passed %d days'
	ngt n2,'passed one day','passed %d days'
	ret 
