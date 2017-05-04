#include <iostream>

using namespace std;


template <typename X, typename Y, typename Z>
auto func_tmpl(const X& m, Y fn1, Z fn2) -> decltype(fn2(fn1(m))) {
    // x - vector
    // y - map
    // z - map
    auto result = fn1(m); //приймає vector<string>words
    auto result2 = fn2(result);
    return result2; // fn2(result)
}


void *callE(const vector<string>& words){
    std::map<string, int> a;
    //a.insert(pair<string, int>("a", 2));
    a = func_tmpl(words, counting_words_worker, result);
}



