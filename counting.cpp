#include <mutex>

std::mutex mtx;
using namespace std;

map<string, int> m;
map<string, int> localm;



void *counting_words_worker(vector<string>words, int startE, int endE){
    map<string, int> localm;
    for (int i = startE; i != endE; ++i) {
        if(localm.find(words[i]) == localm.end()){
            localm[words[i]] = 1;
        }else{
            localm[words[i]] +=1;
        }
    }
    mtx.lock();
    for(auto it = localm.begin(); it != localm.end(); ++it) m[it->first] += it->second;
    mtx.unlock();
}


