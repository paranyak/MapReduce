#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include <assert.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>


using namespace std;
mutex myMutex;
mutex mx;
condition_variable cv;
int num = 0;
int num_of_threads;
deque <double> dm;

double reduce_f(double & m, const double &localm ) {
    m += localm;
    return m;
}

void reducer(deque <double> &dm){
    unique_lock<mutex> uniqueLock(myMutex);
    while (dm.size() > 1) {
        double map1 = dm.front();
        dm.pop_front();
        double map2 = dm.front();
        dm.pop_front();
        uniqueLock.unlock();
        double map3 = reduce_f(map1, map2);
        uniqueLock.lock();
        dm.push_back(map3);
        cout << "1A"  << " n " << num<<endl;
    }if (dm.size() == 1 && num == num_of_threads) {
       cout << dm.front() << endl;
        cout << "2A" << endl;
    } else {
        cv.wait(uniqueLock);
        cout << "3A" << endl;
    }

}

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
    //cout << g  << " -g-- "<< endl;

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

    double operator()(const Iter2Ddouble& itr1, const Iter2Ddouble& itr2)
    {
        double r = 0;
        for(auto i = itr1; i<itr2; ++i) {
            r += func_calculation(m, (*i).first, (*i).second) * itr1.get_pr() * itr1.get_pr();
            {lock_guard<mutex> lg(myMutex);
            dm.push_back(r); }
        }

        cv.notify_one();
        num += 1;
        cv.notify_all();
        reducer(dm);

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
    //cout << sum << " " << endl;
    return sum;
}

double thread_integration(double r, const double &sum) {
    r += sum;
    return r;
}



template <class MF, class RF, class I, class N, class D >
auto func_tmpl(I beg, I fin, MF fn1, D d, RF fn2,  N num_of_threads) -> decltype( fn2(fn1(beg, fin), fn1(beg, fin)) ) {

    decltype(func_tmpl(beg, fin, fn1, d, fn2, num_of_threads)) res;        // big map<string, int>
    size_t delta = (fin - beg) / num_of_threads;
    vector<I> segments;
    for (auto i = beg; i < fin - delta; i += delta) {
        segments.push_back(i);
    }
    segments.push_back(fin);
//    for(auto x: segments)
//    {
//        cout << distance(beg, x) << "," ;
//    }

    vector<thread> threads; //потік

    for (auto i = segments.begin(); i < segments.end()-1; ++i) {
        threads.push_back(std::thread(fn1,*i, *(i+1)));
        auto x = fn1(*i, *(i+1));
        cout << x << endl;
    }


    for (auto& th : threads) th.join();
    return res;
}

int main() {
    vector<double> data = {0, 1, 0, 1, 5, 0.001};
    Iter2Ddouble itr(data);
    cout << "Integral" << endl;
    num_of_threads = 4;
    deque <double> dm;
    cout << func_tmpl(itr, itr.end(), func_wrapper(5), dm,  thread_integration, num_of_threads) << endl;


}