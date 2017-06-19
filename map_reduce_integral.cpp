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
#include "map_reduce_words.h"
//#include "timing.cpp"


using namespace std;
mutex myMutex;
condition_variable cv;



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
double thread_integration(double &r, const double &sum) {
    r += sum;
    return r;
}





template <class Dd, class RF>
Dd reducer_univ(int num_of_threads, deque <Dd> &dm, RF (*fn2)(Dd &, const Dd &), int num){
    unique_lock<mutex> uniqueLock(myMutex);
//    cout << dm.size() << " <- size of deque " << num << " <-- threads finished :) "<<endl;
//    std::thread::id this_id = std::this_thread::get_id();
//    std::cout << "thread in reduce " << this_id << "\n";
    while (dm.size() > 1) {
//        std::thread::id this_id_1 = std::this_thread::get_id();
//        std::cout << "thread in adding " << this_id_1 << "\n";
        Dd map1 = dm.front();
        dm.pop_front();
        Dd map2 = dm.front();
        dm.pop_front();
        uniqueLock.unlock();
        Dd map3 = (*fn2)(map1, map2);
        uniqueLock.lock();
        dm.push_back(map3);
//        cout << "adding state" << endl;
    }
    if (dm.size() == 1 && num == num_of_threads) {
        std::thread::id this_id_2 = std::this_thread::get_id();
//        std::cout << "thread in end " << this_id_2 << "\n";
//        cout << " end :) " << endl;
    } else {
//        std::thread::id this_id_3 = std::this_thread::get_id();
//        std::cout << "thread in waiting " << this_id_3 << "\n";
//        cout << "waiting state  " << endl;
        cv.wait(uniqueLock);
    }

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
        for(size_t i = 0; i<n; ++i)
            ++(*this);
        return *this;
    }
    Iter2Ddouble& operator-=(size_t n)
    {
        for(size_t i = 0; i<n; ++i)
            --(*this);
        return *this;
    }
    ptrdiff_t operator-=(Iter2Ddouble itr)
    {
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
    deque<double> d;
    int n;
    int num;
    func_wrapper(int m_, deque<double> d_, int n_, int num_): m(m_), d(d_), n(n_) , num(num_){};

    double operator()(const Iter2Ddouble& itr1, const Iter2Ddouble& itr2,  deque<double>& d, int n, int &num)
    {
        double r = 0;
        for(auto i = itr1; i <itr2; ++i) {
            r += func_calculation(m, (*i).first, (*i).second) * itr1.get_pr() * itr1.get_pr();
        }
        {lock_guard<mutex> lg(myMutex);
            d.push_back(r);
            num = num +1;
        }

//        cout << num << "-" << endl;
//        std::thread::id this_id = std::this_thread::get_id();
//        std::cout << "thread which finished " << this_id << "\n";
        cv.notify_one();
        cv.notify_all();
        reducer_univ(n, d, thread_integration, num);
        return r;
    }

};












template <class MF, class RF, class I, class N, class Dd>
Dd func_tmpl(I beg, I fin, MF fn1, deque<Dd> &d, RF fn2, N num_of_threads) {

    Dd res;
    size_t delta = (fin - beg) / num_of_threads;
    vector<I> segments;
    for (auto i = beg; i < fin - delta; i += delta) {
        segments.push_back(i);
    }
    segments.push_back(fin);

    thread myThreads [num_of_threads];
    int num = 0;
    for (auto i = segments.begin(); i < segments.end()-1; ++i) {
//        for (auto i = segments.begin(); i < segments.end()-1; ++i) {      НЕ ВИДАЛЯТИ, БО ДЕКОЛИ (ЗАЛЕЖНО ВІД ПОДІЛУ) ТРЕБА ТАК
        myThreads[i - segments.begin()] = thread(fn1, *i, *(i+1), ref(d), num_of_threads, ref(num));
//        cout << "i: "<<i - segments.begin() << endl;
    }

    for (auto& th : myThreads){
        if (th.joinable())
        {th.join(); }
    }
    res = d.front();
    return res;
}


int main()
{
    //auto stage1_start_time = get_current_time_fenced();


    vector<double> data = {0, 4, 0, 4, 5, 0.001};
    deque<double> d;
    Iter2Ddouble itr(data);
    int num = 0;
    int n;
    cout << "Integral" << endl;
    cout << func_tmpl(itr, itr.end(), func_wrapper(5, d, n, num), d, thread_integration, 4) << endl;
//    auto finish_time = get_current_time_fenced();
//    auto reading_time = finish_time - stage1_start_time;
//    fstream log;
//
//    log.open("./result.txt", fstream::app);
//    std::chrono::duration<double, std::milli> r_ms = reading_time;
//    log << r_ms.count() << "\n";
//    log.close();
}