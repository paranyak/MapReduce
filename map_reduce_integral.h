//
// Created by natasha on 19.06.17.
//

#ifndef THREADS_PROJECT_MAP_REDUCE_INTEGRAL_H
#define THREADS_PROJECT_MAP_REDUCE_INTEGRAL_H

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

double func_calculation(int m, double x1, double x2);
double thread_integration(double &r, const double &sum) ;
template <class Dd, class RF>
Dd reducer_univ(int num_of_threads, deque <Dd> &dm, RF (*fn2)(Dd &, const Dd &), int num);
//using point_t = pair<double, double>;
//
//class Iter2Ddouble: public iterator<random_access_iterator_tag, point_t > {
//public:
//    double get_pr() const;
//
//    Iter2Ddouble(const vector<double> &data);
//    Iter2Ddouble end() const;
//    point_t operator*() const;
//    Iter2Ddouble& operator++();
//    Iter2Ddouble& operator--();
//    Iter2Ddouble& operator+=(size_t n);
//    Iter2Ddouble& operator-=(size_t n);
//    ptrdiff_t operator-=(Iter2Ddouble itr);
//
//    friend bool operator<(const Iter2Ddouble& itr1, const Iter2Ddouble& itr2);
//    friend bool operator==(const Iter2Ddouble& itr1, const Iter2Ddouble& itr2);}
//
//
//Iter2Ddouble operator+(Iter2Ddouble itr, size_t n);
//Iter2Ddouble operator-(Iter2Ddouble itr, size_t n);
//ptrdiff_t  operator-(Iter2Ddouble itr_l, const Iter2Ddouble& itr_r);
//bool operator<(const Iter2Ddouble& itr1, const Iter2Ddouble& itr2);
//bool operator==(const Iter2Ddouble& itr1, const Iter2Ddouble& itr2);
//struct func_wrapper{
//    func_wrapper(int m_, deque<double> d_, int n_, int num_): m(m_), d(d_), n(n_) , num(num_);
//
//    double operator()(const Iter2Ddouble& itr1, const Iter2Ddouble& itr2,  deque<double>& d, int n, int &num);
//};


template <class MF, class RF, class I, class N, class Dd>
Dd func_tmpl(I beg, I fin, MF fn1, deque<Dd> &d, RF fn2, N num_of_threads);
int main();

#endif //THREADS_PROJECT_MAP_REDUCE_INTEGRAL_H
