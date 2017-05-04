#include <mutex>

std::mutex mtx;
using namespace std;

map<string, int> m;
map<string, int> localm;



map<string, int> counting_words_worker(vector<string> words){
    map<string, int> localm;
    for (int i = 0; i != words.size(); ++i) {
        if(localm.find(words[i]) == localm.end()){
            localm[words[i]] = 1;
        }else{
            localm[words[i]] +=1;
        }
    }
    return localm;
}

//map<string, int> result (map<string, int> localm) {
map<string, int> result (const map<string, int> &r) {
//переробити на попарне злиття і константна меп
//    mtx.lock();
    map<string, int> localm = r;
    for (auto it = *localm.begin(); it != *localm.end(); ++it) m[it->first] += it->second;
//    mtx.unlock();
    return m;
}

