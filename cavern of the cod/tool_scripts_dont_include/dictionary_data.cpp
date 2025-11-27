//
//  dictionary_data.cpp
//  cavern of the cod
//
//  Created by jan musija on 9/25/25.
//

#include "dictionary_data.h"

void print_dictionary_issues(){ // indicates poorly formatted lines and redundancies
    std::ifstream is;
    is.open(DICT_LOCATION);
    std::unordered_set<std::string> entries;
    std::string str;
    int line = 0;
    while (std::getline(is,str,'\n')){
        line++;
        unsigned long p = str.find('@');
        if (p != std::string::npos){
            if (p == 0 && str[0] != '/'){
                std::cout << "bad " << line << "\n";
            } else {
                if (str[p-1] != ' '){
                    std::cout << "bad " << line << "\n";
                } else {
                    p = p-1;
                }
                str = str.substr(0,p);
                if (entries.contains(str)){
                    std::cout << str << "\n";
                } else {
                    entries.insert(str);
                }
            }
        }
    }
}
