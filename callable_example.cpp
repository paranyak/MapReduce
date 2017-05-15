#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include <assert.h>
#include <thread>
#include <mutex>


using namespace std;
std::mutex mtx;
//map<string, int> counting_words_worker(const vector<string>& words){
//    map<string, int> localm;
//    for (size_t  i = 0; i != words.size(); ++i) {
//            ++localm[words[i]];
//    }
//    return localm;
//}
//
//map<string, int> reduce_f(map<string, int> m, const map<string, int>& localm) {
//    for (auto it = localm.begin(); it != localm.end(); ++it)
//        m[it->first] += it->second;
//
//        for (auto i : m){
//        cout << i.first << " - " << i.second << endl;
//    }
//
//    return m;
//}
//
double func_calculation(int m, double x1, double x2) {
    double sum1 = 0;
    double sum2 = 0;
    double g;
    for (int i = 1; i <= m; ++i)
    {
        sum1 += i * cos((i + 1) * x1 + 1);
        sum2 += i * cos((i + 1) * x2 + 1);
    }

    g = - sum1 * sum2;

    return g;
}

using point_t = pair<double, double>;


class Iter2Ddouble: public iterator<random_access_iterator_tag, point_t >
{
    double x0, x1, y0, y1, m, pr;
    double x, y;
public:
    double get_pr() const {return pr; }
    Iter2Ddouble(const vector<double>& data)
    {
        x0 = data[0];
        x1  = data[1];
        y0 = data[2];
        y1  = data[3];
        m = data[4];
        pr = data[5];

        x = x0;
        y = y0;
    }

    Iter2Ddouble end() const{
        Iter2Ddouble t(*this);
        t.x = x1;
        t.y = y1;
        return t;
    }

    point_t operator*() const
    {
        return make_pair(x + pr / 2.0, y + pr / 2.0);
    }

    Iter2Ddouble& operator++()
    {
        x += pr;
        if(x>x1)
        {
            x = x0;
            y += pr;
        }
        return *this;
    }

    Iter2Ddouble& operator--()
    {
        x -= pr;
        if(x<x0)
        {
            x = x1;
            y -= pr;
        }
        return *this;
    }

    Iter2Ddouble& operator+=(size_t n)
    {
        //! TODO: write without loop!
        for(size_t i = 0; i<n; ++i)
            ++(*this);
        return *this;
    }
    Iter2Ddouble& operator-=(size_t n)
    {
        //! TODO: write without loop!
        for(size_t i = 0; i<n; ++i)
            --(*this);
        return *this;
    }
    ptrdiff_t operator-=(Iter2Ddouble itr)
    {
        //! TODO: write without loop!
        ptrdiff_t diff = 0;
        if( *this < itr) {
            while (*this < itr) {
                --itr;
                --diff;
            }
        }else{
            while ( !(*this < itr) ) {
                ++itr;
                ++diff;
            }

        }

        return diff;
    }

    friend bool operator<(const Iter2Ddouble& itr1, const Iter2Ddouble& itr2);
    friend bool operator==(const Iter2Ddouble& itr1, const Iter2Ddouble& itr2);
};

Iter2Ddouble operator+(Iter2Ddouble itr, size_t n)
{
    return itr += n;
}
Iter2Ddouble operator-(Iter2Ddouble itr, size_t n)
{
    return itr -= n;
}

ptrdiff_t  operator-(Iter2Ddouble itr_l, const Iter2Ddouble& itr_r)
{
    return itr_l -= itr_r;
}

bool operator<(const Iter2Ddouble& itr1, const Iter2Ddouble& itr2)
{
    if( itr1.y == itr2.y ) //! Bad code -- use abs|x - y | < eps
    {
        return itr1.x < itr2.x;
    }
    return itr1.y < itr2.y;
}

bool operator==(const Iter2Ddouble& itr1, const Iter2Ddouble& itr2)
{
    return itr1.y == itr2.y && itr1.x == itr2.x;  //! Bad code -- use abs|x - y | < eps
}
struct func_wrapper
{
    int m;
    func_wrapper(int m_): m(m_){}
//    double operator()(const point_t& p)
//    {
//        return func_calculation(m, p.first, p.second);
//    }
    double operator()(const Iter2Ddouble& itr1, const Iter2Ddouble& itr2)
    {
        double r = 0;
        for(auto i = itr1; i<itr2; ++i) {
            r += func_calculation(m, (*i).first, (*i).second) * itr1.get_pr() * itr1.get_pr();
            // cout << (*i).first << " " <<  (*i).second << " " << r << endl;
        }
        return r;
    }

};


double integration(const vector<double>& data ) {
    double x0 = data[0];
    double x = data[1];
    double y0 = data[2];
    double y = data[3];
    double m = data[4];
    double pr = data[5];

    assert (m >= 5);
    double sum = 0;
    for (double i = x0; i <= x; i += pr) {
        for (double j = y0; j <= y; j += pr) {
            sum += func_calculation(m, i + pr / 2.0, j + pr / 2.0) * pr * pr;
        }
    }
    return sum;
}

double thread_integration(double r, const double &sum) {

   // auto result = integration(x0, x, y0, y, m, pr);
//    lock_guard<mutex> lg(mx);
    r += sum;
//    cout << r << endl;
    return r;
}

//template <class V, class MF, class RF>
//auto func_tmpl(const V& v, MF fn1, RF fn2) -> decltype( fn2(fn1(v[0]), fn1(v[0])) ) {
//    // V - масив блоків (векторів)
//    decltype(func_tmpl(v, fn1, fn2)) res;        // big map<string, int>
//    for (size_t i = 0; i < v.size(); ++i) {
//        auto x = fn1(v[i]);        //return small   map<string, int>
//        res = fn2(move(res), x);
//    }
//
//    return res;
//}
//
map<string, int> counting_words_worker( vector<string>::const_iterator beg, vector<string>::const_iterator  fin ){
    map<string, int> localm;
    for (auto i = beg; i != fin; ++i) {
        ++localm[*i];
    }
    return localm;
}
//
map<string, int> reduce_f( map<string, int> m, const map<string, int>& localm ) {
    for (auto it = localm.begin(); it != localm.end(); ++it)
        m[it->first] += it->second;
    return m;
}

template <class MF, class RF, class I, class N>
auto func_tmpl(I beg, I fin, MF fn1, RF fn2,  N num_of_threads) -> decltype( fn2(fn1(beg, fin), fn1(beg, fin)) ) {
    // v - вектор усіх слів
    // beg = v.begin()
    // fin = v.end()
    decltype(func_tmpl(beg, fin, fn1, fn2, num_of_threads)) res;        // big map<string, int>
    size_t delta = (fin - beg) / num_of_threads;
    //    auto i = begin, j = begin + delta;
    vector<I> segments;
    for (auto i = beg; i < fin - delta; i += delta) {
        segments.push_back(i);
    }
    segments.push_back(fin);
    for(auto x: segments)
    {
        cout << distance(beg, x) << "," ;
    }

    std::vector<std::thread> threads; //потік

    for (auto i = segments.begin(); i < segments.end()-1; ++i) {
        threads.push_back(std::thread(fn1,*i, *(i+1)));     //потік
        auto x = fn1(*i, *(i+1));
        mtx.lock();                 //поки так зливаємо
        res = fn2(move(res), x);
        mtx.unlock();
    }


    for (auto& th : threads) th.join();
    return res;
}




//void callD2(const vector<string> &words){
//    cout << words.size() << endl;
//    //cout << words.begin() << endl;
//    func_tmpl(words.begin(), words.end(), counting_words_worker, reduce_f, 4);
//}

int main()
{
    vector<string> v = {"aaaa", "bbbbb", "ccccc", "ddddd", "aaaa", "eeeee", "wwwwwwwww", "ccccc", "ccccc", "ccccc"};
    cout<<"Words counter" << endl;
    for (auto i : func_tmpl(v.begin(), v.end(), counting_words_worker, reduce_f, 3)){
       cout << i.first << " - " << i.second << endl;
    }
// func_tmpl(v.begin(), v.end(), counting_words_worker, reduce_f, 4);
    //cout << func_tmpl(v.begin(), v.end(), counting_words_worker, reduce_f, 4)<< endl;

    vector<double> data  ={0, 1, 0, 1, 5, 0.001};
    Iter2Ddouble itr(data);
    cout << "Integral" << endl;
    cout << func_tmpl(itr, itr.end(), func_wrapper(5), thread_integration, 4) << endl;



    //    map<string, int> mpp;
    //    auto s1 = counting_words_worker(v, 0, 5);
    //    cout << ".................." << endl;
    //    auto s2 = counting_words_worker(v, 5, 8);
    //    mpp = move(reduce_f(mpp, s1));
    //    cout << "last reduce: \t";
    //    mpp = move(reduce_f(mpp, s2));
    //    cout << "++++++++++++++++++" << endl;
    //    for (auto i : mpp)
    //        cout << i.first << "-- " << i.second << endl;



    //    vector<string> v1 = {"asdf", "wert", "aaa", "qwerty"};
    //    vector<string> v2 = {"asdf", "poiuy", "qwewrtdxfb", "qwerty"};
    //    vector<vector<string>> v3;
    //    v3.push_back(v1);
    //    v3.push_back(v2);
    //    callD(v3);
    //    vector<double> data = {0, 1, 0, 1, 5, 0.001};
    //    vector<vector<double>> v4;
    //    v4.push_back(data);
    //    callI(v4);

}

// template <class T>  T reduce_f(T m, const T& localm)
// func_tmpl(words, counting_words_worker, reduce_f<map<string, int>>);

//U map(T &t);
//V reduce(T &result, const U &intermediate)