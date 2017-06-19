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

#include <stdio.h>
#include <stdbool.h>
#include <mpi.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <assert.h>
#include <map>
#include <string>
#include <sstream>
#include <unistd.h>
//#include "timing.cpp"


using namespace std;
mutex myMutex;
condition_variable cv;

int commsize = 0, rankk = 0, len = 0;
char procname[MPI_MAX_PROCESSOR_NAME];

double func_calculation(int m, double x1, double x2) {
    double sum1 = 0;
    double sum2 = 0;
    double g;
    for (int i = 1; i <= m; ++i) {
        sum1 += i * cos((i + 1) * x1 + 1);
        sum2 += i * cos((i + 1) * x2 + 1);
    }
    g = -sum1 * sum2;
    return g;
}

double thread_integration(double &r, const double &sum) {
    r += sum;
    return r;
}


template<class Dd, class RF>
Dd reducer_univ(int num_of_threads, deque<Dd> &dm, RF (*fn2)(Dd &, const Dd &), int num) {
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
        //   std::thread::id this_id_2 = std::this_thread::get_id();
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


class Iter2Ddouble : public iterator<random_access_iterator_tag, point_t> {
    double x0, x1, y0, y1, m, pr;
    double x, y;
public:
    double get_pr() const { return pr; }

    Iter2Ddouble(const vector<double> &data) {
        x0 = data[0];
        x1 = data[1];
        y0 = data[2];
        y1 = data[3];
        m = data[4];
        pr = data[5];

        x = x0;
        y = y0;
    }

    Iter2Ddouble end() const {
        Iter2Ddouble t(*this);
        t.x = x1;
        t.y = y1;
        return t;
    }

    point_t operator*() const {
        return make_pair(x + pr / 2.0, y + pr / 2.0);
    }

    Iter2Ddouble &operator++() {
        x += pr;
        if (x > x1) {
            x = x0;
            y += pr;
        }
        return *this;
    }

    Iter2Ddouble &operator--() {
        x -= pr;
        if (x < x0) {
            x = x1;
            y -= pr;
        }
        return *this;
    }

    Iter2Ddouble &operator+=(size_t n) {
        for (size_t i = 0; i < n; ++i)
            ++(*this);
        return *this;
    }

    Iter2Ddouble &operator-=(size_t n) {
        for (size_t i = 0; i < n; ++i)
            --(*this);
        return *this;
    }

    ptrdiff_t operator-=(Iter2Ddouble itr) {
        ptrdiff_t diff = 0;
        if (*this < itr) {
            while (*this < itr) {
                --itr;
                --diff;
            }
        } else {
            while (!(*this < itr)) {
                ++itr;
                ++diff;
            }

        }

        return diff;
    }

    friend bool operator<(const Iter2Ddouble &itr1, const Iter2Ddouble &itr2);

    friend bool operator==(const Iter2Ddouble &itr1, const Iter2Ddouble &itr2);
};

Iter2Ddouble operator+(Iter2Ddouble itr, size_t n) {
    return itr += n;
}

Iter2Ddouble operator-(Iter2Ddouble itr, size_t n) {
    return itr -= n;
}

ptrdiff_t operator-(Iter2Ddouble itr_l, const Iter2Ddouble &itr_r) {
    return itr_l -= itr_r;
}

bool operator<(const Iter2Ddouble &itr1, const Iter2Ddouble &itr2) {
    if (itr1.y == itr2.y) //! Bad code -- use abs|x - y | < eps
    {
        return itr1.x < itr2.x;
    }
    return itr1.y < itr2.y;
}

bool operator==(const Iter2Ddouble &itr1, const Iter2Ddouble &itr2) {
    return itr1.y == itr2.y && itr1.x == itr2.x;  //! Bad code -- use abs|x - y | < eps
}

struct func_wrapper {
    int m;
    int n;
    int num;

    func_wrapper(int m_, int n_, int num_) : m(m_), n(n_), num(num_) {};

    double operator()(const Iter2Ddouble &itr1, const Iter2Ddouble &itr2, int n, int &num) {
        double r = 0;
        for (auto i = itr1; i < itr2; ++i) {
            r += func_calculation(m, (*i).first, (*i).second) * itr1.get_pr() * itr1.get_pr();
        }
        {
            lock_guard<mutex> lg(myMutex);

            num = num + 1;
        }

        cv.notify_one();
        cv.notify_all();
//        reducer_univ(n, d, thread_integration, num);
        return r;
    }

};


template<class MF, class RF, class I, class N, class Dd>
Dd func_tmpl(I beg, I fin, MF fn1, deque<Dd> &d, RF fn2, N num_of_threads) {

    Dd res;
    size_t delta = (fin - beg) / num_of_threads;

    vector<I> segments;
    for (auto i = beg; i < fin - delta; i += delta) {
        segments.push_back(i);
    }
    segments.push_back(fin);

    thread myThreads[num_of_threads];
    int num = 0;
    double result = 0;
    if (rankk == 0) {
        int from_to[] = {0, 1, commsize, num};
        for (int i = 1; i < commsize - 1; ++i) {
            MPI_Send(from_to, 4, MPI_INTEGER, i, 0, MPI_COMM_WORLD);

            from_to[0] = i;
            from_to[1] = i;
        }
        int k = 0;
        for (auto i = segments.begin(); i < segments.end() - 1; ++i) {
            if (k != from_to[0])continue;
            result = fn1(*i, *(i + 1), num_of_threads, ref(num));
            k++;
        }

//        result = fn1(*segments.begin(), *segments.end(), num_of_threads, ref(num));       //DEBUG
//        cout << result << endl;
//        result = fn1(*segments.at(from_to[0]), *(segments.at(from_to[1]) + 1), from_to[2], from_to[3]);

        for (int i = 1; i < commsize; ++i) {
            double recieved[1];
            MPI_Recv(&recieved, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            result += recieved[0];

        }
        cout << "Result: " << result << endl;
    } else {
        int from_to[4];
        int k = 0;
        MPI_Recv(&from_to, 4, MPI_INTEGER, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (auto i = segments.begin(); i < segments.end() - 1; ++i) {
            if (k != from_to[0])continue;
            result = fn1(*i, *(i + 1), num_of_threads, ref(num));
            k++;
        }

//        result = fn1(*segments[from_to[0]], *(segments[from_to[1]] + 1), from_to[2], ref(from_to[3]));
        double res_to_send[]{result};
        MPI_Send(res_to_send, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

    }
//    for (auto i = segments.begin(); i < segments.end() - 1; ++i) {
//        myThreads[i - segments.begin()] = thread(fn1, *i, *(i + 1), num_of_threads, ref(num));
//
//    }

//    for (auto &th : myThreads) {
//        if (th.joinable()) { th.join(); }
//    }
//    res = d.front();
//    return res;
    MPI_Finalize();
    return 0;

}


int main(int argc, char *argv[]) {
    //auto stage1_start_time = get_current_time_fenced();

    int commsize1, rank1, len1;
    char procname[MPI_MAX_PROCESSOR_NAME];
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize1);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank1);
    MPI_Get_processor_name(procname, &len1);
    rankk = rank1;
    len = len1;
    commsize = commsize1;

    vector<double> data = {0, 4, 0, 4, 5, 0.001};
    double data_to_send[] = {0, 4, 0, 4, 5, 0.001};
    deque<double> d;
    Iter2Ddouble itr(data);
    int num = 0;
    int n;
//    cout << "Integral" << endl;
//    cout << func_tmpl(itr, itr.end(), func_wrapper(5, n, num), d, thread_integration, commsize) << endl;
    if (rankk == 0) {
        for (int i = 1; i < commsize1; ++i) {
            MPI_Send(data_to_send, 6, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }
        func_tmpl(itr, itr.end(), func_wrapper(5, n, num), d, thread_integration, commsize);
    } else {
        vector<double> data_to_recv;
        MPI_Recv(&data_to_recv, 6, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        deque<double> d;
        Iter2Ddouble itr(data_to_recv);
        int num = 0;
        int n;
        func_tmpl(itr, itr.end(), func_wrapper(5, n, num), d, thread_integration, commsize);
    }
}