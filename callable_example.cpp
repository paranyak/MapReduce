#include <iostream>

using namespace std;


template <typename T, typename U, typename  IT, typename I , typename M, typename N, typename O>
void func_tmpl ( T fn1, U fn2, M m, I int1, IT int2, N result, O result2) {
    result = fn1(m, int1, int2); //приймає vector<string>words, int startE, int endE
    result2= fn2(result);
}


void *callE(vector<string>words, int startE, int endE){
    std::cout << "Hello, World!" << std::endl;
    std::map<string, int> a;
    a.insert(pair<string, int>("a", 2));
    func_tmpl(counting_words_worker, result, words, startE, endE, a, a);
}



