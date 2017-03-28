
int write_file(string path, map <string, int> a){
    ofstream myfile;
    myfile.open (path);
    for(auto elem : a){
        myfile << elem.first + "   " + to_string(elem.second) + "\n";
    }
    myfile.close();
    return 0;
}


int write_vector_file(string path, vector<pair<string, int>> v ){
    ofstream myfile;
    myfile.open (path);
    for(const auto& p: v){
        myfile << p.first+ "   " + to_string(p.second) + "\n";
    }
    myfile.close();
    return 0;
}
