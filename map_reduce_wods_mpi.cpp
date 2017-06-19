#include <mpi.h>

#include <iostream>
#include <map>
#include <vector>
#include <deque>
#include <condition_variable>
#include <fstream>
#include <iterator>
#include <numeric>


using namespace std;


vector<string>
reading(int &N) {                     //відкривання файлу з даними, виділення шляхів до файлів і кількості потоків
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
        if (words[i].find("infile=") != std::string::npos) {
            size_t pos = words[i].find("infile=");
            fileO = words[i].substr(pos + 8);
            fileO = fileO.substr(0, fileO.length() - 1);

        }
    }
    ifstream myfile1;
    vector<string> words_all;
    string word_all;
    myfile1.open(fileO);
    if (!myfile.is_open()) {
        cerr << "Error open data" << endl;
    }
    while (myfile1 >> word_all) {
        words_all.push_back(word_all);
        words_all.push_back(",,");
    }

    return words_all;
}


void printMap(const map<string, int> &m) {
    for (auto &elem : m) {
        cout << elem.first << " " << elem.second << "\n";
    }
}


map<string, int> reduce_f(map<string, int> &m, const map<string, int> &localm) {

    for (auto it = localm.begin(); it != localm.end(); ++it)
        m[it->first] += it->second;
    return m;
}

void reducer(int num_of_threads, deque<map<string, int>> dm, int num) {
    cout << "Node of rank: "<<num <<" is in reducer"<< endl;
    while (dm.size() > 1) {
        map<string, int> map1;
        dm.pop_front();
        map<string, int> map2;
        dm.pop_front();
        map<string, int> map3 = reduce_f(map1, map2);
        dm.push_back(map3);
        cout << "adding by node number " << num <<endl;
    }
    if (dm.size() == 1 && num == 0) {
        return;
    }

}

map<string, int> counting_words_worker(string beg, string bS, string eS) {

    deque<map<string, int>> dm;
    map<string, int> localm;
    string delimiter = ",,";

    size_t pos = 0;
    std::string token;
    while ((pos = beg.find(delimiter)) != std::string::npos) {
        token = beg.substr(0, pos);
        ++localm[token];
        beg.erase(0, pos + delimiter.length());
    }

    return localm;
}




int main(int argc, char *argv[]) {
    int commsize, rank, len;

    int num_of_threads = commsize;
    vector<string> v = reading(
            num_of_threads);//{"aaaa", "hhhhh", "bbbbb", "ccccc", "ddddd", "aaaa", "eeeee", "wwwwwwwww", "ccccc", "ccccc", "ccccc"};
    cout << "Words counter" << endl;
    deque<map<string, int>> dm;
    int num = 0;

    char procname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(procname, &len);

    size_t delta = v.size() / commsize;

    map<string, int> deque1;
    deque<map<string, int>> dn;
    if (rank == 0) {

        double start_time = MPI_Wtime();
        int j = 0;
        string from_to[] ={};
        int i;
        map<string, int> deque2;
        for ( i = delta + 1; i < v.size() - delta+1; i += delta+1) {
            string str ;//(v.begin() + i, v.begin() - 1 + i + delta);
            str = accumulate(v.begin() + i, v.begin() +1 + i + delta, str);
            from_to[0] = (str).c_str();
            from_to[1] = to_string(i);
            from_to[2] = to_string(i + delta+1);
            MPI_Send(from_to, 3, MPI_CHAR, j+1, 0, MPI_COMM_WORLD);
            j += 1;

        }
        if(i < v.size()){
            string str ;//(v.begin() + i, v.begin() - 1 + i + delta);
            str = accumulate(v.begin() + i, v.end() - 1, str);
            from_to[0] = (str).c_str();
            from_to[1] = to_string(i);
            from_to[2] = to_string(v.size()-1);
            MPI_Send(from_to, 3, MPI_CHAR, j, 0, MPI_COMM_WORLD);

        }

        deque2 = counting_words_worker(from_to[0], from_to[1], from_to[2]);

        for (int i = 1; i < commsize; ++i) {
            MPI_Recv(&deque2, 1, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            dn.push_back(deque2);
            reducer(commsize, dn, i - 1);
        }
        cout << "Total time: " << MPI_Wtime() - start_time << " it was node ->" <<rank<< endl;
    } else {
        double start_time = MPI_Wtime();
        string str = " ";
        str = accumulate(v.begin(), v.begin() - 1 + delta, str);
        string from_to[] = {str.c_str(), to_string(0), to_string(delta)};
        deque1 = counting_words_worker(from_to[0], from_to[1], from_to[2]);
        MPI_Send(&deque1, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        dn.push_back(deque1);
        reducer(commsize, dn, rank);
        cout << "Process " << rank << "/" << commsize << " execution time: " << MPI_Wtime() - start_time << endl;
    }

    MPI_Finalize();
    //printMap(dn.front());
    return 0;

}


