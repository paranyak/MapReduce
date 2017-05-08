#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include <assert.h>

//#include <mutex>


using namespace std;

map<string, int> counting_words_worker(const vector<string>& words){
    map<string, int> localm;
    for (size_t  i = 0; i != words.size(); ++i) {
            ++localm[words[i]];
    }
    return localm;
}

map<string, int> reduce_f(map<string, int> m, const map<string, int>& localm) {
    for (auto it = localm.begin(); it != localm.end(); ++it)
        m[it->first] += it->second;

        for (auto i : m){
        cout << i.first << " - " << i.second << endl;
    }

    return m;
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

    return g;
}

double integration(vector<double> data) {
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
    cout << r << endl;
    return r;
}

template <class V, class MF, class RF>
auto func_tmpl(const V& v, MF fn1, RF fn2) -> decltype( fn2(fn1(v[0]), fn1(v[0])) ) {
    // V - масив блоків (векторів)
    decltype(func_tmpl(v, fn1, fn2)) res;        // big map<string, int>
    for (size_t i = 0; i < v.size(); ++i) {
        auto x = fn1(v[i]);        //return small   map<string, int>
        res = fn2(move(res), x);
    }

    return res;
}

void callD(const vector<vector<string>> &words){
    func_tmpl(words, counting_words_worker, reduce_f);
}

void callI(const vector<vector<double>> &data){
    func_tmpl(data, integration, thread_integration);
}

int main()
{
    vector<string> v1 = {"asdf", "wert", "aaa", "qwerty"};
    vector<string> v2 = {"asdf", "poiuy", "qwewrtdxfb", "qwerty"};
    vector<vector<string>> v3;
    v3.push_back(v1);
    v3.push_back(v2);
    callD(v3);
    vector<double> data = {0, 1, 0, 1, 5, 0.001};
    vector<vector<double>> v4;
    v4.push_back(data);
    callI(v4);

}

// template <class T>  T reduce_f(T m, const T& localm)
// func_tmpl(words, counting_words_worker, reduce_f<map<string, int>>);
