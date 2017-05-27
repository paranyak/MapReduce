#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include <assert.h>
#include <thread>
#include <mutex>
#include <deque>
#include <condition_variable>


using namespace std;
mutex myMutex;
mutex mx;
condition_variable cv;

deque <map<string, int>> dm;
map<string, int> mainMap;
//

void printMap(const map<string, int> &m) {
    for (auto &elem : m) {
        cout << elem.first << " " << elem.second << "\n";
    }
}
map<string, int> counting_words_worker( vector<string>::const_iterator beg, vector<string>::const_iterator  fin ){
    map<string, int> localm;
    for (auto i = beg; i != fin; ++i) {
        ++localm[*i];
    }
    {
    lock_guard<mutex> lg(myMutex);
    dm.push_back(localm);
    }
    cv.notify_one();
    cout<< "------" << endl;
    for(int i =0; i < dm.size(); i++){
    printMap(dm[i]);}
    cout << "-----" << dm.size()<<endl;
    cv.notify_all();
    return localm;
}
//
map<string, int> reduce_f( map<string, int> m, const map<string, int>& localm ) {

    for (auto it = localm.begin(); it != localm.end(); ++it)
        m[it->first] += it->second;
    return m;
}

void reducer(){
    unique_lock<mutex> ul(myMutex);
    while (!dm.empty()) {
        if (dm.size() != 1) {
            map<string, int> map1 = dm.front();
            dm.pop_front();
            map<string, int> map2 = dm.front();
            dm.pop_front();
            ul.unlock();
            map<string, int> map3 = reduce_f(map1, map2);
            ul.lock();
            dm.push_back(map3);
            cout << "Added map" << endl;
            printMap(map3);
        } else {
            break;
        }
    }
    if(dm.size() == 1) {
        mainMap = dm.front();
        cout << "MAin Map" << endl;
    }

}

template <class MF, class RF, class I, class N>
auto func_tmpl(I beg, I fin, MF fn1, RF fn2,  N num_of_threads) -> decltype( fn2(fn1(beg, fin), fn1(beg, fin)) ) {
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
    /////////////////////


    //std::vector<std::thread> threads; //потік
    thread myThreads [num_of_threads];
    for (auto i = segments.begin(); i < segments.end()-1; ++i) {
        //threads.push_back(std::thread(fn1,*i, *(i+1)));     //потік
        myThreads[i - segments.begin()] = thread(fn1,*i, *(i+1));
        //cv.notify_all();
    }

//    thread myThreads1 [num_of_threads];
//    for(int i = 0; i < num_of_threads; i++){
//        myThreads1[i] = thread(reducer);
//    }

    for (auto& th : myThreads) th.join();
    cout << "-*-*-*-" << endl;
    reducer();
    return res;
}






int main()
{
    vector<string> v = {"aaaa", "bbbbb", "ccccc", "ddddd", "aaaa", "eeeee", "wwwwwwwww", "ccccc", "ccccc", "ccccc"};
    cout<<"Words counter" << endl;
    for (auto i : func_tmpl(v.begin(), v.end(), counting_words_worker, reduce_f, 3)) {
        cout << i.first << " - " << i.second << endl;
    }
    cout << "****" << endl;
    printMap(mainMap);
}


