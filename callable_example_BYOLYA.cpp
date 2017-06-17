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


using namespace std;
mutex myMutex;
mutex mx;
condition_variable cv;


int num = 0;
//int num_of_threads = 0;
//

vector<string> reading(int &N){                     //відкривання файлу з даними, виділення шляхів до файлів і кількості потоків
    ifstream myfile;
    vector<string> words;
    string word;
    myfile.open("Read.txt");    //файл який має три шляхи для файлів і кількість потоків
    if (!myfile.is_open()) {
        cerr << "Error" << endl;
    }
    while (myfile >> word) {        //додаємо рядок без =
        words.push_back(word);
    }
    string fileO;
    for (int i = 0; i < words.size(); ++i) {     //прінт всіх
        cout << words[i] << ' ';
        if( words[i].find("infile=") != std::string::npos ){
            size_t pos = words[i].find("infile=");
             fileO = words[i].substr(pos+8);
            fileO = fileO.substr(0, fileO.length()-1);

        }
        else if( words[i].find("threads=") != std::string::npos){
            size_t pos = words[i].find("threads=");
            N = atoi(words[i].substr(pos+8).c_str());
        }
    }
    ifstream myfile1;
    vector<string> words_all;
    string word_all;
    myfile1.open(fileO);
    if (!myfile.is_open()) {
        cerr << "Error open data" << endl;
    }
    while (myfile1 >> word_all){
        words_all.push_back(word_all);
    }

    return words_all;
}

void printMap(const map<string, int> &m) {
    for (auto &elem : m) {
        cout << elem.first << " " << elem.second << "\n";
    }
}

//

map<string, int> reduce_f( map<string, int> m, const map<string, int>& localm ) {

    for (auto it = localm.begin(); it != localm.end(); ++it)
        m[it->first] += it->second;
    return m;
}

deque <map<string, int>>  reducer(int num_of_threads, deque <map<string, int>> &dm){
    unique_lock<mutex> uniqueLock(myMutex);
    while (dm.size() > 1) {
        map<string, int> map1 = dm.front();
        dm.pop_front();
        map<string, int> map2 = dm.front();
        dm.pop_front();
        uniqueLock.unlock();
        map<string, int> map3 = reduce_f(map1, map2);
        uniqueLock.lock();
        dm.push_back(map3);
       // cout << "1" << endl;
    }if (dm.size() == 1 && num == num_of_threads) {
        //cout << "2" << endl;
        printMap(dm.front());
    } else {
        //cout << "3" << endl;
        cv.wait(uniqueLock);
    }
    return dm;
}
map<string, int> counting_words_worker(const vector<string>::const_iterator &beg, const vector<string>::const_iterator &fin , deque <map<string, int>> & dm){
    map<string, int> localm;
    for (auto i = beg; i != fin; ++i) {
        ++localm[*i];
    }
    {
        lock_guard<mutex> lg(myMutex);
        dm.push_back(localm);
    }
    cv.notify_one();
    cv.notify_all();
    num += 1;
    //reducer(dm);
    return localm;
}



template <class MF, class RF, class I, class N, class D>
auto func_tmpl(I beg, I fin, MF fn1, D d,  RF fn2,  N num_of_threads) -> decltype( fn2(num_of_threads, ref(d)) ) {
    decltype(func_tmpl(beg, fin, fn1, d,  fn2, num_of_threads)) res;        // big map<string, int>

    size_t delta = (fin - beg) / num_of_threads;
    vector<I> segments;
    for (auto i = beg; i < fin - delta; i += delta) {
        segments.push_back(i);
    }
    segments.push_back(fin);

    /////////////////////


    thread myThreads [num_of_threads];
    for (auto i = segments.begin(); i < segments.end()-1; ++i) {
        myThreads[i - segments.begin()] = thread(fn1,*i, *(i+1), ref(d));
        fn2(num_of_threads, ref(d));
    }

    for (auto& th : myThreads) th.join();
    res = fn2(num_of_threads, ref(d));
    return res;
}






int main()
{
    int num_of_threads = 3;
    vector<string> v = reading(num_of_threads);//{"aaaa", "hhhhh", "bbbbb", "ccccc", "ddddd", "aaaa", "eeeee", "wwwwwwwww", "ccccc", "ccccc", "ccccc"};
    cout<<"Words counter" << endl;
    deque <map<string, int>> dm;
    func_tmpl(v.begin(), v.end(), counting_words_worker, dm, reducer, num_of_threads);
}


