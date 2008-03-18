%baseclass-preinclude boost/any.hpp
%baseclass-preinclude iostream
%baseclass-preinclude string


%stype boost::any

%token TEMPLATE 
%token IDENT
%token INLINE
%token IF
%token END
%token ELSE
%token ELSEIF
%token FOREACH
%token IN
%token ITEM
%token EMPTY
%token INCLUDE
%token NOT
%token USING
%token DEF

%% 

/* Grammar rules and actions follow.  */
input:  template 
        | input template 
;

template:
	TEMPLATE IDENT  empty_body  { 
		std::cout<<"extern " << (boost::any_cast<std::string>($2)) <<std::endl;}	;

empty_body: /*empty*/ | body ;
body:	text_line | body text_line ;

text_line : INLINE  { std::cout<<$1<<std::endl; }
	| ifblock
	| foreach
	| show
	| include ;

ifblock:  IF expr  empty_body  END 
	|  IF expr empty_body other_case  END ;
other_case:
	ELSE  empty_body
	| ELSEIF expr empty_body ;

foreach:
	foreach_head foreach_body END
	| foreach_head foreach_body EMPTY body END;
foreach_head: FOREACH IDENT IN IDENT ;
foreach_body: empty_body ITEM empty_body END empty_body;

include:
	INCLUDE name
	| INCLUDE name using
	| INCLUDE '"' IDENT '"' 
	| INCLUDE '"' IDENT '"' using ;
using:
	USING params;

params: name | params ',' name ;

show:	name ;
expr:	DEF name | NOT DEF name | name | NOT name;
name:	IDENT | IDENT '.' IDENT;

%%
