#include <boost/locale/boundary.hpp>
#include <boost/locale/generator.hpp>
#include <boost/locale/codepage.hpp>
#include "test_locale.hpp"

#include <list>

namespace lb = boost::locale::boundary;

template<typename Char,typename Iterator>
void test_word_container(Iterator begin,Iterator end,
    std::vector<int> const &ipos,
    std::vector<int> const &imasks,
    std::vector<std::basic_string<Char> > const &ichunks,
    std::locale l)
{
    {   // token iterator tests
        typedef lb::token_iterator<Iterator> iter_type;
        lb::mapping<iter_type> map(lb::word,begin,end,l);

        for(unsigned mask=0;mask<=15;mask++) {
            map.mask(mask);
            std::vector<std::basic_string<Char> > chunks;
            std::vector<int> masks,pos;
            /// mask==0 has special meaning
            if(mask == 0) {
                chunks=ichunks;
                masks=imasks;
                pos=ipos;
            }
            else {
                for(unsigned i=0;i<imasks.size();i++) {
                    if(imasks[i] & mask) {
                        masks.push_back(imasks[i]);
                        chunks.push_back(ichunks[i]);
                        pos.push_back(ipos[i]);
                    }
                }
            }
        
            unsigned i=0;
            iter_type p;
            for(p=map.begin();p!=map.end();++p,i++) {
                TEST(*p==chunks[i]);
                TEST(p.flag() == unsigned(masks[i]));
            }
            TEST(chunks.size() == i);
            
            for(;;) {
                if(p==map.begin()) {
                    TEST(i==0);
                    break;
                }
                else {
                    --p;
                    TEST(*p==chunks[--i]);
                    TEST(p.flag() == unsigned(masks[i]));
                }
            }

            unsigned chunk_ptr=0;
            i=0;
            for(Iterator optr=begin;optr!=end;optr++,i++) {
                p=optr;
                if(chunk_ptr < pos.size()) {
                    if(i>pos[chunk_ptr])
                        chunk_ptr++;
                }
                if(chunk_ptr>=pos.size()) {
                    TEST(p==map.end());
                }
                else {
                    std::cerr << "bi["<< *p << "] t[" << chunks[chunk_ptr] << "] <" << std::string(begin,optr)<<"|"<<std::string(optr,end)<<">" <<std::endl;
                    TEST(*p==chunks[chunk_ptr]);
                    TEST(p.flag()==masks[chunk_ptr]);
                }
            }

        } // for mask
    }

}

template<typename Char>
std::basic_string<Char> to_correct_string(std::string const &e,std::locale l)
{
    return boost::locale::conv::to_utf<Char>(e,l);
}


template<>
std::string to_correct_string(std::string const &e,std::locale l)
{
    return boost::locale::conv::from_utf(e,l);
}

template<typename Char>
void run_word(std::string *original,int *num,int *word,int *kana,int *ideo,std::locale l)
{
    std::vector<int> pos;
    std::vector<std::basic_string<Char> > chunks;
    std::vector<int> masks;
    std::basic_string<Char> test_string;
    for(int i=0;!original[i].empty();i++) {
        chunks.push_back(to_correct_string<Char>(original[i],l));
        pos.push_back(test_string.size());
        test_string+=chunks.back();
        pos.push_back(test_string.size());
        masks.push_back(num[i] | (word[i]<<1) | (kana[i] << 2) | (ideo[i]<<3));
    }

    std::list<Char> lst(test_string.begin(),test_string.end());
    test_word_container<Char>(lst.begin(),lst.end(),pos,masks,chunks,l);
    test_word_container<Char>(test_string.begin(),test_string.end(),pos,masks,chunks,l);
}

void word_boundary()
{
    std::cout << "Testing word boundary" << std::endl;
    boost::locale::generator g;
    
    std::string all1[]={"10"," ","Hello"," ","Windows7"," ","平仮名","ひらがな","ヒラガナ",""};
    int         num1[]={ 1,   0,      0,  0,         1,   0,      0 ,        0 ,         0};
    int        word1[]={ 0,   0,      1,  0,         1,   0,      0 ,        0 ,         0};
    int        kana1[]={ 0,   0,      0,  0,         0,   0,      0,         1 ,         1}; 
    int        ideo1[]={ 0,   0,      0,  0,         0,   0,      1,         0 ,         0}; 


    int zero[25]={0};
    std::string all2[]={""};

    std::string all3[]={" "," ","Hello",",","World","!"," ",""};
    int        word3[]={ 0,  0,      1,  0,      1,   0,  0, 0};

    std::cout << " char UTF-8" << std::endl;
    run_word<char>(all1,num1,word1,kana1,ideo1,g("ja_JP.UTF-8"));
    run_word<char>(all2,zero,zero,zero,zero,g("en_US.UTF-8"));
    run_word<char>(all3,zero,word3,zero,zero,g("en_US.UTF-8"));

    std::cout << " char Shift-JIS" << std::endl;
    run_word<char>(all1,num1,word1,kana1,ideo1,g("ja_JP.Shift-JIS"));
    run_word<char>(all2,zero,zero,zero,zero,g("ja_JP.Shift-JIS"));
    run_word<char>(all3,zero,word3,zero,zero,g("ja_JP.Shift-JIS"));

    #if 0 //#ifndef BOOST_NO_STD_WSTRING
    std::cout << " wchar_t"<<std::endl;
    run_word<wchar_t>(all1,num1,word1,kana1,ideo1,g("ja_JP.UTF-8"));
    run_word<wchar_t>(all2,zero,zero,zero,zero,g("en_US.UTF-8"));
    run_word<wchar_t>(all3,zero,word3,zero,zero,g("en_US.UTF-8"));
    #endif 

    #ifdef BOOST_HAS_CHAR16_T
    std::cout << " char16_t"<<std::endl;
    run_word<char16_t>(all1,num1,word1,kana1,ideo1,g("ja_JP.UTF-8"));
    run_word<char16_t>(all2,zero,zero,zero,zero,g("en_US.UTF-8"));
    run_word<char16_t>(all3,zero,word3,zero,zero,g("en_US.UTF-8"));
    #endif 

    #ifdef BOOST_HAS_CHAR32_T
    std::cout << " char32_t"<<std::endl;
    run_word<char32_t>(all1,num1,word1,kana1,ideo1,g("ja_JP.UTF-8"));
    run_word<char32_t>(all2,zero,zero,zero,zero,g("en_US.UTF-8"));
    run_word<char32_t>(all3,zero,word3,zero,zero,g("en_US.UTF-8"));
    #endif 
}



int main()
{
    try {
        word_boundary();
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Ok" << std::endl;
    return EXIT_SUCCESS;

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

