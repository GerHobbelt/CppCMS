#include <boost/locale/boundary.hpp>
#include <boost/locale/generator.hpp>
#include "test_locale.hpp"
#include "test_locale_tools.hpp"
#include <list>

//#define BOOST_NO_STD_WSTRING

namespace lb = boost::locale::boundary;

template<typename Char,typename Iterator>
void test_word_container(Iterator begin,Iterator end,
    std::vector<int> const &ipos,
    std::vector<int> const &imasks,
    std::vector<std::basic_string<Char> > const &ichunks,
    std::locale l,
    lb::boundary_type bt=lb::word
    )
{
    {   // token iterator tests
        typedef lb::token_iterator<Iterator> iter_type;
        lb::mapping<iter_type> map(bt,begin,end,l);
        
        for(int sm=(bt == lb::word ? 31 : 3 ) ;sm>=0;sm--) {
            unsigned mask = 
                  ((sm & 1 ) != 0) * 07
                + ((sm & 2 ) != 0) * 070
                + ((sm & 4 ) != 0) * 0700
                + ((sm & 8 ) != 0) * 07000
                + ((sm & 16) != 0) * 070000;

            map.mask(mask);
            std::vector<std::basic_string<Char> > chunks;
            std::vector<std::basic_string<Char> > fchunks;

            std::vector<int> masks,pos;
            std::basic_string<Char> empty_chunk;
            for(unsigned i=0;i<imasks.size();i++) {
                if(imasks[i] & mask) {
                    masks.push_back(imasks[i]);
                    chunks.push_back(ichunks[i]);
                    fchunks.push_back(empty_chunk + ichunks[i]);
                    empty_chunk.clear();
                    pos.push_back(ipos[i]);
                }
                else {
                    empty_chunk+=ichunks[i];
                }
            }
        
            unsigned i=0;
            iter_type p;
            for(p=map.begin();p!=map.end();++p,i++) {
                p.full_select(false);
                TEST(*p==chunks[i]);
                p.full_select(true);
                TEST(*p==fchunks[i]);
                TEST(p.mark() == unsigned(masks[i]));
            }
            TEST(chunks.size() == i);
                
            
            for(;;) {
                if(p==map.begin()) {
                    TEST(i==0);
                    break;
                }
                else {
                    --p;
                    p.full_select(false);
                    TEST(*p==chunks[--i]);
                    p.full_select(true);
                    TEST(p.mark() == unsigned(masks[i]));
                }
            }
            
            unsigned chunk_ptr=0;
            i=0;
            for(Iterator optr=begin;optr!=end;optr++,i++) {
                p=optr;
                if(chunk_ptr < pos.size() && i>=unsigned(pos[chunk_ptr])){
                    chunk_ptr++;
                }
                if(chunk_ptr>=pos.size()) {
                    TEST(p==map.end());
                }
                else {
                    p.full_select(false);
                    TEST(*p==chunks[chunk_ptr]);
                    p.full_select(true);
                    TEST(*p==fchunks[chunk_ptr]);
                    TEST(p.mark()==unsigned(masks[chunk_ptr]));
                }
            }

        } // for mask
    }

}

template<typename Char>
void run_word(std::string *original,int *none,int *num,int *word,int *kana,int *ideo,std::locale l,lb::boundary_type b=lb::word)
{
    std::vector<int> pos;
    std::vector<std::basic_string<Char> > chunks;
    std::vector<int> masks;
    std::basic_string<Char> test_string;
    for(int i=0;!original[i].empty();i++) {
        chunks.push_back(to_correct_string<Char>(original[i],l));
        test_string+=chunks.back();
        pos.push_back(test_string.size());
        masks.push_back(
              ( none ? none[i]*7 : 0)
            | ( num  ? ((num[i]*7)  << 3) : 0) 
            | ( word ? ((word[i]*7) << 6) : 0) 
            | ( kana ? ((kana[i]*7) << 9) : 0)
            | ( ideo ? ((ideo[i]*7) << 12) : 0)
        );
    }

    std::list<Char> lst(test_string.begin(),test_string.end());
    test_word_container<Char>(lst.begin(),lst.end(),pos,masks,chunks,l,b);
    test_word_container<Char>(test_string.begin(),test_string.end(),pos,masks,chunks,l,b);
}

std::string character[]={"שָ","ל","וֹ","ם","!",""};
int         nones[]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

std::string sentence1[]={"To be\n","or not\n","to be?\n"," That is the question. ","Or maybe not",""};
int         sentence1a[]={      0,          0,        1,                         1,             0, 0};
int         sentence1b[]={      1,          1,        0,                         0,             1, 0};

std::string line1[]={"To ","be\n","or ","not\n","to ","be",""};
int         line1a[]={ 1,   0,     1 ,  0,       1,   1 , 0 };
int         line1b[]={ 0,   1,     0 ,  1,       0,   0 , 0 };


void test_boundaries(std::string *all,int *first,int *second,lb::boundary_type t)
{
    boost::locale::generator g;
    std::cout << " char UTF-8" << std::endl;
    run_word<char>(all,first,second,0,0,0,g("he_IL.UTF-8"),t);
    std::cout << " char CP1255" << std::endl;
    run_word<char>(all,first,second,0,0,0,g("he_IL.cp1255"),t);
    #ifndef BOOST_NO_STD_WSTRING
    std::cout << " wchar_t"<<std::endl;
    run_word<wchar_t>(all,first,second,0,0,0,g("he_IL.UTF-8"),t);
    #endif
    #ifdef BOOST_HAS_CHAR16_T
    std::cout << " char16_t"<<std::endl;
    run_word<char16_t>(all,first,second,0,0,0,g("he_IL.UTF-8"),t);
    #endif
    #ifdef BOOST_HAS_CHAR32_T
    std::cout << " char32_t"<<std::endl;
    run_word<char32_t>(all,first,second,0,0,0,g("he_IL.UTF-8"),t);
    #endif

}

void word_boundary()
{
    boost::locale::generator g;
    
    std::string all1[]={"10"," ","Hello"," ","Windows7"," ","平仮名","ひらがな","ヒラガナ",""};
    int        none1[]={ 0,   1,      0,  1,         0,   1,      0,         0,          0};
    int         num1[]={ 1,   0,      0,  0,         1,   0,      0 ,        0 ,         0};
    int        word1[]={ 0,   0,      1,  0,         1,   0,      0 ,        0 ,         0};
    int        kana1[]={ 0,   0,      0,  0,         0,   0,      0,         1 ,         1}; 
    int        ideo1[]={ 0,   0,      0,  0,         0,   0,      1,         0 ,         0}; 


    int zero[25]={0};
    std::string all2[]={""};

    std::string all3[]={" "," ","Hello",",","World","!"," ",""};
    int        none3[]={ 1,  1,      0,  1,      0,   1,  1, 0};
    int        word3[]={ 0,  0,      1,  0,      1,   0,  0, 0};

    std::cout << " char UTF-8" << std::endl;
    run_word<char>(all1,none1,num1,word1,kana1,ideo1,g("ja_JP.UTF-8"));
    run_word<char>(all2,zero,zero,zero,zero,zero,g("en_US.UTF-8"));
    run_word<char>(all3,none3,zero,word3,zero,zero,g("en_US.UTF-8"));

    std::cout << " char Shift-JIS" << std::endl;
    run_word<char>(all1,none1,num1,word1,kana1,ideo1,g("ja_JP.Shift-JIS"));
    run_word<char>(all2,zero,zero,zero,zero,zero,g("ja_JP.Shift-JIS"));
    run_word<char>(all3,none3,zero,word3,zero,zero,g("ja_JP.Shift-JIS"));

    #ifndef BOOST_NO_STD_WSTRING
    std::cout << " wchar_t"<<std::endl;
    run_word<wchar_t>(all1,none1,num1,word1,kana1,ideo1,g("ja_JP.UTF-8"));
    run_word<wchar_t>(all2,zero,zero,zero,zero,zero,g("en_US.UTF-8"));
    run_word<wchar_t>(all3,none3,zero,word3,zero,zero,g("en_US.UTF-8"));
    #endif 

    #ifdef BOOST_HAS_CHAR16_T
    std::cout << " char16_t"<<std::endl;
    run_word<char16_t>(all1,none1,num1,word1,kana1,ideo1,g("ja_JP.UTF-8"));
    run_word<char16_t>(all2,zero,zero,zero,zero,zero,g("en_US.UTF-8"));
    run_word<char16_t>(all3,none3,zero,word3,zero,zero,g("en_US.UTF-8"));
    #endif 

    #ifdef BOOST_HAS_CHAR32_T
    std::cout << " char32_t"<<std::endl;
    run_word<char32_t>(all1,none1,num1,word1,kana1,ideo1,g("ja_JP.UTF-8"));
    run_word<char32_t>(all2,zero,zero,zero,zero,zero,g("en_US.UTF-8"));
    run_word<char32_t>(all3,none3,zero,word3,zero,zero,g("en_US.UTF-8"));
    #endif 
}


int main()
{
    try {
        std::cout << "Testing word boundary" << std::endl;
        word_boundary();
        std::cout << "Testing character boundary" << std::endl;
        test_boundaries(character,nones,0,lb::character);
        std::cout << "Testing sentence boundary" << std::endl;
        test_boundaries(sentence1,sentence1a,sentence1b,lb::sentence);
        std::cout << "Testing line boundary" << std::endl;
        test_boundaries(line1,line1a,line1b,lb::line);
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Ok" << std::endl;
    return EXIT_SUCCESS;

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

