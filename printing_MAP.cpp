
#include <stdlib.h>
#include <iostream>

#include <map>


using namespace std;

void print_Map(map<string, int> myMap){
    for(auto elem : myMap)
    {
        cout << elem.first << " " << elem.second <<  "\n";
    }
}