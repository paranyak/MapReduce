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
mutex myMutex;
mutex mx;
condition_variable cv;




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

map<string, int> reduce_f( map<string, int> &m, const map<string, int>& localm ) {

    for (auto it = localm.begin(); it != localm.end(); ++it)
        m[it->first] += it->second;
    return m;
}

template <class Dd, class RF>
void reducer(int num_of_threads, deque <Dd> &dm, RF (*fn2)(Dd &, const Dd &), int num){
    unique_lock<mutex> uniqueLock(myMutex);
    while (dm.size() > 1) {
        Dd map1 = dm.front();
        dm.pop_front();
        Dd map2 = dm.front();
        dm.pop_front();
        uniqueLock.unlock();
        Dd map3 = (*fn2)(map1, map2);
        uniqueLock.lock();
        dm.push_back(map3);
    }if (dm.size() != 1 && num != num_of_threads) {
//        cout << "here" << endl;
//        printMap(dm.front());
//    } else {
        cv.wait(uniqueLock);
    }else{
    printMap(dm.front());
    return;}

}
map<string, int> counting_words_worker(const vector<string>::const_iterator &beg, const vector<string>::const_iterator &fin , deque <map<string, int>> & dm, map<string, int> (*reduce_f)( map<string, int> &, const map<string, int> &),int num_of_threads,  int &num){
    map<string, int> localm;
    for (auto i = beg; i != fin; ++i) {
        ++localm[*i];
    }
    {
        lock_guard<mutex> lg(myMutex);
        dm.push_back(localm);
        num  +=1;
    }
    cv.notify_one();
    cv.notify_all();

    reducer(num_of_threads, dm, reduce_f, num);
    return localm;
}



template <class MF, class RF, class I, class N, class D, class NM>

auto func_tmpl(I beg, I fin, MF fn1, D d,  RF fn2,  N num_of_threads, NM nm) -> decltype( fn1(beg, fin, d, fn2, num_of_threads, ref(nm)))  {
    decltype(func_tmpl(beg, fin, fn1, d,  fn2, num_of_threads, nm)) res;        // big map<string, int>

    size_t delta = (fin - beg) / num_of_threads;
    vector<I> segments;
    for (auto i = beg; i < fin - delta; i += delta) {
        segments.push_back(i);
    }
    segments.push_back(fin);

    thread myThreads [num_of_threads];
    for (auto i = segments.begin(); i < segments.end()-1; ++i) {
        myThreads[i - segments.begin()] = thread(fn1,*i, *(i+1), ref(d), fn2, num_of_threads, ref(nm));
    }

    for (auto& th : myThreads) th.join();
    return res;
}






int main()
{
   // int num_of_threads;
//    auto stage1_start_time = get_current_time_fenced();
    int num_of_threads = 2;
    vector<string> v = reading(num_of_threads);//{"aaaa", "hhhhh", "bbbbb", "ccccc", "ddddd", "aaaa", "eeeee", "wwwwwwwww", "ccccc", "ccccc", "ccccc"};
    cout<<"Words counter" << endl;
    deque <map<string, int>> dm;
    int num = 0;
    func_tmpl(v.begin(), v.end(), counting_words_worker, dm, reduce_f, num_of_threads, ref(num));
//    auto finish_time = get_current_time_fenced();
//    auto reading_time = finish_time - stage1_start_time;
//    fstream log;
//
//    log.open("./result.txt", fstream::app);
//    std::chrono::duration<double, std::milli> r_ms = reading_time;
//    log << r_ms.count() << "\n";
//    log.close();
}


