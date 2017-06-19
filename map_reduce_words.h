//
// Created by natasha on 19.06.17.
//

#ifndef THREADS_PROJECT_MAP_REDUCE_WORDS_H
#define THREADS_PROJECT_MAP_REDUCE_WORDS_H
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include <assert.h>
#include <thread>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <fstream>
#include "timing.cpp"


using namespace std;
vector<string> reading(int &N);//відкривання файлу з даними, виділення шляхів до файлів і кількості потоків
void printMap(const map<string, int> &m) ;
map<string, int> reduce_f( map<string, int> &m, const map<string, int>& localm ) ;
template <class Dd, class RF>
void reducer(int num_of_threads, deque <Dd> &dm, RF (*fn2)(Dd &, const Dd &), int num);
map<string, int> counting_words_worker(const vector<string>::const_iterator &beg, const vector<string>::const_iterator &fin , deque <map<string, int>> & dm, map<string, int> (*reduce_f)( map<string, int> &, const map<string, int> &),int num_of_threads,  int &num);
template <class MF, class RF, class I, class N, class D, class NM>

auto func_tmpl(I beg, I fin, MF fn1, D d,  RF fn2,  N num_of_threads, NM nm) -> decltype( fn1(beg, fin, d, fn2, num_of_threads, ref(nm)))  ;
int main();


#endif //THREADS_PROJECT_MAP_REDUCE_WORDS_H
