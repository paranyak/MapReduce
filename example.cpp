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
int N=0;                            //кількість потоків(потім змінюється відповідно до вказаного)
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
        words.push_back(word);
    }

    for (int i = 0; i < words.size(); ++i) {     //прінт всіх
        cout << words[i] << ' ';
        if( words[i].find("infile=") != std::string::npos ){
            size_t pos = words[i].find("infile=");
            fileO = words[i].substr(pos+8);
            fileO = fileO.substr(0, fileO.length()-1);
        }
        else if( words[i].find("out_by_a=") != std::string::npos){
            size_t pos = words[i].find("out_by_a=");
            fileWA = words[i].substr(pos+10);
            fileWA = fileWA.substr(0, fileWA.length()-1);
        }
        else if( words[i].find("out_by_n=") != std::string::npos){
            size_t pos = words[i].find("out_by_n=");
            fileWN = words[i].substr(pos+10);
            fileWN = fileWN.substr(0, fileWN.length()-1);
        }
        else if( words[i].find("threads=") != std::string::npos){
            size_t pos = words[i].find("threads=");
            N = atoi(words[i].substr(pos+8).c_str());
        }

    }
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
    write_file(fileWA, m);
    write_vector_file(fileWN, mapcopy);
    auto stage5_start_time = get_current_time_fenced(); //кінець запису
    auto writing_time = stage5_start_time - stage4_start_time;

    auto all_time = writing_time + counting_time + reading_time;


    //можливо знадобиться для одного процесу
    cout << "ONLY reading time: " << to_us(reading_time) << endl;
    cout << "ONLY counting time: " << to_us(counting_time) << endl;
    cout << "ONLY writing time: " << to_us(writing_time) << endl;
    cout << "ALL time: " << to_us(all_time) << endl;

    // записуємо всі значення обрахування часу в файл, ВОНИ НЕ ВИДАЛЯЮТЬСЯ кожного разу
    fstream log;
    log.open("/home/natasha/CLionProjects/threads_project/result.txt", fstream::app);
    std::chrono::duration<double, std::milli> r_ms = reading_time;
    std::chrono::duration<double, std::milli> w_ms = writing_time;
    std::chrono::duration<double, std::milli> c_ms = counting_time;
    std::chrono::duration<double, std::milli> all_ms = all_time;
    log<< r_ms.count();
    log<< "  ";
    log<< w_ms.count();
    log<< "  ";
    log<< c_ms.count();
    log<< "  ";
    log<< all_ms.count();
    log<< " \n";
    log.close();
    pthread_exit(NULL);

}