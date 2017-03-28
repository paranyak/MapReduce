#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <algorithm>
#include "printing_MAP.cpp"
#include "timing.cpp"
#include "writing.cpp"
#include "counting.cpp"


using namespace std;
int N=2;                            //кількість потоків(потім змінюється відповідно до вказаного)
string fileO, fileWA, fileWN;       // файл з якого беруться слова і два файли для запису (WA - по афавіту,WN- за кількістю)


vector<string> open_data_file(string path) {    //відкривання файлу зі всіма словами
    ifstream myfile;
    vector<string> words;
    string word;
    myfile.open(path);
    if (!myfile.is_open()) {
        cerr << "Error open data" << endl;
    }
    while (myfile >> word){
        words.push_back(word);
    }
    return words;
}



void reading(){                     //відкривання файлу з даними, виділення шляхів до файлів і кількості потоків
    ifstream myfile;
    vector<string> words;
    string word;
    myfile.open("/home/natasha/CLionProjects/threads_project/Read.txt");    //файл який має три шляхи для файлів і кількість потоків
    if (!myfile.is_open()) {
        cerr << "Error" << endl;
    }
    while (myfile >> word) {        //додаємо рядок без =
        size_t pos = word.find("=");
        string str3 = word.substr(pos+1);
        words.push_back(str3);
    }
    for (int i = 0; i < words.size(); ++i) {     //прінт всіх
        cout << words[i] << ' ';}
    N = atoi( words[words.size()-1].c_str() );      // останній елемент кастимо в інт
    fileO = words[0].substr(1, words[0].length()-2);  //шлях без лапок
    fileWA = words[1].substr(1, words[1].length()-2);
    fileWN = words[2].substr(1, words[2].length()-2);
    //cout << fileO + "    " + fileWA + "     " + fileWN << endl;

}



int main()
{
    auto stage1_start_time = get_current_time_fenced();
    reading();
    vector<string> all_words = open_data_file(fileO);

    auto finish_time = get_current_time_fenced();
    auto reading_time = finish_time - stage1_start_time;


    thread my_thread[N];
    long id;
    long starting = 0;
    long step = all_words.size()/N;
    long ending = all_words.size()/N;

    auto stage2_start_time = get_current_time_fenced(); //початок підрахунку

    //потоки
   for(id = 0; id < N; id++){
        my_thread[id] =  thread(counting_words_worker, all_words, starting, ending);
        starting += step +1;
        if(ending + step > all_words.size()){
            ending = all_words.size();
        }else{
            ending = starting + step;
        }

    }
    for(id = 0; id < N; id++){
        my_thread[id].join();
    }

    auto stage3_start_time = get_current_time_fenced(); //кінець підрахунку
    auto counting_time = stage3_start_time - stage2_start_time;

    // потоки

    print_Map(m);

    //перетворення в вектор для сортування за 2 аргументом
    struct less_second {
        typedef pair<string, int> type;
        bool operator ()(type const& a, type const& b) const {
            return a.second < b.second;
        }
    };
    vector<pair<string, int> > mapcopy(m.begin(), m.end());
    sort(mapcopy.begin(), mapcopy.end(), less_second());

    auto stage4_start_time = get_current_time_fenced(); //початок запису
    //запис в файли
    //write_file(fileWA, m);
    //write_vector_file(fileWN, mapcopy);
    auto stage5_start_time = get_current_time_fenced(); //кінець запису
    auto writing_time = stage5_start_time - stage4_start_time;

    auto all_time = writing_time + counting_time + reading_time;



    cout << "ONLY reading time: " << to_us(reading_time) << endl;
    cout << "ONLY counting time: " << to_us(counting_time) << endl;
    cout << "ONLY writing time: " << to_us(writing_time) << endl;
    cout << "ALL time: " << to_us(all_time) << endl;

    fstream log;
    log.open("/home/natasha/CLionProjects/threads_project/result.txt", fstream::app);
    unsigned int dwDuration = reading_time.count();
    log<< dwDuration;
    log<< "  ";
    log.close();
    pthread_exit(NULL);

}