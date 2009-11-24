#include <boost/locale.hpp>

struct cp1255_tag {};
struct utf8_tag {};

template<typename CharType,typename tag_type=utf8_tag>
struct S {
	static CharType const *ch_boundary; 
	static CharType const *ch_boundary_o[4];
	static CharType const *word_boundary;
	static CharType const *word_boundary_o[4];
	static int const flags[8];
	static CharType const *line_boundary;
	static CharType const *line_boundary_o[3];
	static CharType const *sentence_boundary;
	static CharType const *sentence_boundary_o[2];
};

template<>
struct S<char,utf8_tag> {
	static char const *ch_boundary = "\xd7\xa9\xd6\xb8\xd7\x9c\xd7\x95\xd6\xb9\xd7\x9d"; /*Sha l o m */
	static char const *ch_boundary_o[4] = { "\xd7\xa9\xd6\xb8", "\xd7\x9c", "\xd7\x95\xd6\xb9", "\xd7\x9d" };
	static char const *word_boundary = 	"10 Hello Windows7 " 
						"\xe5\xb9\xb3\xe4\xbb\xae\xe5\x90\x8d"
						"\xe3\x81\xb2\xe3\x82\x89\xe3\x81\x8c\xe3\x81\xaa" 
						"\xe3\x83\x92\xe3\x83\xa9\xe3\x82\xac\xe3\x83\x8a";
	static char const *word_boundary_locale = "ja_JP.UTF-8"; 
	static char const *word_boundary_o[8] = { "10" , " ", "Hello", "Windows7", " ",
						"\xe5\xb9\xb3\xe4\xbb\xae\xe5\x90\x8d",
						"\xe3\x81\xb2\xe3\x82\x89\xe3\x81\x8c\xe3\x81\xaa",
						"\xe3\x83\x92\xe3\x83\xa9\xe3\x82\xac\xe3\x83\x8a"
	
	}; 
	static int const flags = { 1, 0, 2, 3, 0, 8, 4, 4 }; 
	static char const *line_boundary = "This is the-test"; 
	static char const *line_boundary_o[4] = { "This ", "is ", "the-", "test" }; 
	static char const *sentence_boundary = "Are? You"; 
	static char const *sentence_boundary_o[2] = { "Are? ", "You" }; 
}

template<>
struct S<char,cp1255_tag> {
	static char const *ch_boundary = "\xd7\xa9\xd6\xb8\xd7\x9c\xd7\x95\xd6\xb9\xd7\x9d"; /*Sha l o m */
	static char const *ch_boundary_o[4] = { "\xd7\xa9\xd6\xb8", "\xd7\x9c", "\xd7\x95\xd6\xb9", "\xd7\x9d" };
	static char const *word_boundary = 	"10 Hello Windows7 " 
						"\xd7\xa9\xd6\xb8\xd7\x9c\xd7\x95\xd6\xb9\xd7\x9d"
	static char const *word_boundary_locale = "ja_JP.CP1255"; 
	static char const *word_boundary_o[6] = { "10" , " ", "Hello", "Windows7", " ",
						"\xd7\xa9\xd6\xb8\xd7\x9c\xd7\x95\xd6\xb9\xd7\x9d"
	
	}; 
	static int const flags = { 1, 0, 2, 3, 0, 2 }; 
	static char const *line_boundary = "This is the-test"; 
	static char const *line_boundary_o[4] = { "This ", "is ", "the-", "test" }; 
	static char const *sentence_boundary = "Are? You"; 
	static char const *sentence_boundary_o[2] = { "Are? ", "You" }; 
}

#define TEST_BOUND_STRS(CHAR,L) 							\
	static CHAR const *ch_boundary = L##"\u05e9\u05b8\u05dc\u05d5\u05b9\u05dd"; /*Shalom */ \
	static CHAR const *ch_boundary_o[4] = { L##"\u05e9\u05b8", L##"\u05dc", L##"\u05d5\u05b9", L##"\u05dd" }; \
	static CHAR const *word_boundary = L##"10 Hello Windows7 \u5e73\u4eee\u540d\u3072\u3089\u304c\u306a\u30d2\u30e9\u30a9\u30ca"; \
	static char const *word_boundary_locale = "ja_JP.UTF-8"; \
	static CHAR const *word_boundary_o[8] = { L##"10" , L##" ", L##"Hello", L##"Windows7", L##" ", L##"\u5e73\u4eee\u540d" , L##"\u3072\u3089\u304c\u306a", L##"\u30d2\u30e9\u30a9\u30ca" }; \
	static int const flags = { 1, 0, 2, 3, 0, 8, 4, 4 }; \
	static CHAR const *line_boundary = L##"This is the-test"; \
	static CHAR const *line_boundary_o[4] = { L##"This ", L##"is ", L##"the-", L##"test" }; \
	static CHAR const *sentence_boundary = L##"Are? You"; \
	static CHAR const *sentence_boundary_o[2] = { L##"Are? ", L##"You" }; 

#ifndef BOOST_NO_STD_WSTRING

template<typename tag_type>
struct S<wchar_t> {
	TEST_BOUND_STRS(wchar_t,L)
};

#endif

#ifndef BOOST_HAS_CHAR16_T

template<typename tag_type>
struct S<char16_t> {
	TEST_BOUND_STRS(char16_t,u)
};

#endif

#ifndef BOOST_HAS_CHAR32_T

template<typename tag_type>
struct S<char16_t> {
	TEST_BOUND_STRS(char32_t,U)
};

#endif



template<typename CharType,typename TagType>
test()
{
	typedef S<CharType,TagType> X;
	using namespace boost::locale;
	generator gen;
	std::locale loc=gen(X::word_boundary_locale);
	boundary::index_map indx=boundary::map(boundary::word,X::word_boundary,loc);
	for(unsigned i=0;i<indx.size()-1;i++) {
		CHECK(indx[i].next==indx[i+1].prev);
	}

}


int main()
{
	test<char,utf8_tag>();
	test<char,cp1255_tag>();
	#ifndef BOOST_NO_STD_WSTRING
	test<wchar_t,utf8_tag>();
	#endif
	#ifdef BOOST_HAS_CHAR16_T
	test<char16_t,utf8_tag>();
	#endif
	#ifdef BOOST_HAS_CHAR32_T
	test<char32_t,utf8_tag>();
	#endif

}



#endif
