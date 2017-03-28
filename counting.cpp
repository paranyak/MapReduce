#include <mutex>

std::mutex mtx;
using namespace std;
map<string, int> m;



void *counting_words_worker(vector<string>words, int startE, int endE){

    mtx.lock();
    for (int i = startE; i != endE; ++i) {
        if(m.find(words[i]) == m.end()){
            m[words[i]] = 1;
        }else{
            m[words[i]] +=1;
        }
    }
    mtx.unlock();
}