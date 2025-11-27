//
//  nomenclature.cpp
//  ominous dice torment game
//
//  Created by jan musija on 9/14/25.
//

#include "nomenclature.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

// """simple"" version ... except, over time, I literally turned this into what I envisioned for the complex one...

grammar_bank::grammar_bank(){
    // I think all the things just have default constructors that initialize as blank
}

void grammar_bank::open(std::string dict_file, std::string sub_file){
    dict.clear();
    subs.clear();
    std::ifstream is;
    std::string str;
    is.open(sub_file);
    while (std::getline(is,str,'\n')){
        if (str[0] == '$'){ // forcing into the subs map
            if (str[1] == '_'){ //forcing into the base_subs map
                add_string(str.substr(2,str.length()-2), subs);
                add_string(str.substr(2,str.length()-2), base_subs);
            } else {
                add_string(str.substr(1,str.length()-1), subs);
            }
        } else if (str[0] == '_') { //forcing into the base_subs map
            add_string(str.substr(1,str.length()-1), base_subs);
        }
        else {
            if (str.find('$') != str.npos){
                add_string(str, subs);
            } else {
                add_string(str, base_subs);
            }
        }
    }
    is.close();
    is.open(dict_file);
    while (std::getline(is,str,'\n')){
        add_string(str, dict, all_tags);
    }
    is.close();
}

void add_string(std::string const & str, std::unordered_map<std::string,std::vector<std::pair<std::string,std::unordered_set<std::string>>>> & target_dict, std::unordered_set<std::string> & tag_ref){
    // all dicts MUST precede tags
    std::string str2;
    unsigned long dictpos=str.find('@');
    unsigned long tagpos=str.find('#');
    std::vector<std::string> target_dicts;
    std::unordered_set<std::string> tags;
    if (dictpos== std::string::npos){
        return; // not being added to any dicts-- skill issue?!
    }
    if (tagpos == std::string::npos){ // if no tags exist
        tagpos = str.size();
    }
    // dictpos exists
    if (dictpos-1 >= 0 && str[dictpos-1] == ' '){ // stuff @dict
        str2 = str.substr(0,dictpos-1);
    }
    else { // no space
        str2 = str.substr(0,dictpos);
    } //note: don't stuff  @dict with two spaces!!
    
    { // dict handling
        unsigned long pos = 0;
        while (dictpos<tagpos){
            pos = dictpos+1; if (pos > str.size()){break;}
            dictpos=str.find('@',pos);
            if (dictpos == std::string::npos){dictpos = tagpos;}
            unsigned long l = dictpos-pos;
            std::string dict;
            if (str[dictpos-1] == ' ' && l > 0){
                target_dicts.push_back(str.substr(pos,l-1));
            } else {
                target_dicts.push_back(str.substr(pos,l));
            }
        }
    }
    // tags
    if (tagpos!=str.size()){
        unsigned long pos = 0;
        while (tagpos<str.size()){
            pos = tagpos+1; if (pos > str.size()){break;}
            tagpos=str.find('#',pos);
            if (tagpos == std::string::npos){tagpos = str.size();}
            unsigned long l = tagpos-pos;
            std::string tag;
            if (str[tagpos-1] == ' ' && l > 0){
                std::string tg = str.substr(pos,l-1);
                tags.insert(tg);
                if (!('A' <= tg[0] && tg[0] <= 'Z')){
                    tag_ref.insert(tg);
                }
            } else {
                std::string tg = str.substr(pos,l);
                tags.insert(tg);
                if (!('A' <= tg[0] && tg[0] <= 'Z')){
                    tag_ref.insert(tg);
                }
            }
        }
    }
    if (str2.length() == 0 || (str[0] == '/' && str[1] == '/')){
        return; // skip commented and empty lines
    } else {
        for (int i = 0; i<target_dicts.size(); i++){
            target_dict[target_dicts[i]].push_back(std::pair(str2,tags));
        }
    }
}

void add_string(std::string const & str, std::vector<std::pair<std::string,std::unordered_set<std::string>>> & target_list, std::unordered_set<std::string> & tag_ref){ // deprecated
    std::string str2;
    unsigned long tagpos=str.find('#');
    std::unordered_set<std::string> tags;
    if (tagpos!=std::string::npos){
        if (tagpos-1 >= 0 && str[tagpos-1] == ' '){ // stuff #tag
            str2 = str.substr(0,tagpos-1);
        }
        else { // stuff#tag
            str2 = str.substr(0,tagpos);
        } //note: don't stuff  #tag with two spaces!!
        unsigned long pos = 0;
        while (tagpos<str.size()){
            pos = tagpos+1; if (pos > str.size()){break;}
            tagpos=str.find('#',pos);
            if (tagpos == std::string::npos){tagpos = str.size();}
            unsigned long l = tagpos-pos;
            std::string tag;
            if (str[tagpos-1] == ' ' && l > 0){
                tags.insert(str.substr(pos,l-1));
                tag_ref.insert(str.substr(pos,l-1));
            } else {
                tags.insert(str.substr(pos,l));
                tag_ref.insert(str.substr(pos,l));
            }
        }
    } else {
        str2 = str;
    }
    if (str2.length() == 0 || (str[0] == '/' && str[1] == '/')){
        return; // skip commented and empty lines
    } else {
        target_list.push_back(std::pair(str2,tags));
    }
}

void add_string(std::string const & str, std::vector<std::vector<std::string>> & target_list){
    if (str.length() == 0 || (str[0] == '/' && str[1] == '/')){
        return; // skip commented and empty lines
    } else {
        std::stringstream ss;
        std::string s;
        ss << str;
        std::vector<std::string> sv;
        while(std::getline(ss,s,' '))
        {
            sv.push_back(s);
        }
        if (sv.size() > 0){
            target_list.push_back(sv);
        }
    }
}

void add_string(std::string const & str, std::map<std::string,std::vector<std::vector<std::string>>> & target_map){
    if (str.length() == 0 || (str[0] == '/' && str[1] == '/')){
        return; // skip commented and empty lines
    } else {
        std::stringstream ss;
        std::string S;
        ss << str;
        std::getline(ss,S,' ');
        std::vector<std::vector<std::string>>* tl = &(target_map[S]);
        std::string s;
        std::vector<std::string> sv;
        while(std::getline(ss,s,' '))
        {
            sv.push_back(s);
        }
        if (sv.size() > 0){
            tl->push_back(sv);
        }
    }
}

// construct_from_string algorithm is deprecated for grammar_banks. may bring it back if I actually need it for something unbiased, though even then probably easier to just construct_from_string with an unbiased site_gb.
/*
std::string grammar_bank::global_cfs_pass_mt(std::mt19937 & rng, const std::vector<std::string> struc, unsigned int budget, bool capall = 0, bool respect = 1) const { // don't use this except in the recursive use of construct_from_string!!
    std::string o = "";
    std::vector<unsigned int> subbudg;
    int subbudgets = 0;
    for (int i = 0; i<struc.size(); i++){
        if (struc[i][0] == '$' && struc[i].length() > 1){
            subbudg.push_back(budget);
            subbudgets++;
        } else {
            subbudg.push_back(0);
        }
    }
    if (subbudgets > 0 && budget > 0){
        std::vector<unsigned int> subbudg_nums;
        subbudg_nums.push_back(0);
        for (int i = 1; i<subbudgets; i++){
            subbudg_nums.push_back(rng()%budget);
        }
        subbudg_nums.push_back(budget);
        std::sort(subbudg_nums.begin(), subbudg_nums.end());
        for (int i = 0; i<subbudgets; i++){
            subbudg_nums[i] = subbudg_nums[i+1]-subbudg_nums[i];
        }
        subbudg_nums.pop_back();
        int i = 0;
        for (int j = 0; j<struc.size();j++){
            if (subbudg[j] > 0){
                subbudg[j] = subbudg_nums[i]; i++;
            }
        }
        // this code will work flawlessly the first time, watch.
        // holy shit it did?! I really wasn't expecting that
    }
    bool cap = 0;
    bool resp = 0;
    bool refresh = 1;
    for (int i = 0; i<struc.size(); i++){
        if (refresh && capall) {
            cap = 1;
        }
        if (refresh && respect) {
            resp = 1;
        }
        if (struc[i][0] == '^'){
            refresh = 0;
            if (struc[i][1] == 'c') {
                cap = 1; resp = 0;
            } else if (struc[i][1] == 'l') {
                cap = 0; resp = 0;
            } else if (struc[i][1] == 'r') {
                resp = 1;
            } else if (struc[i][1] == '+') {
                int g = std::atoi((struc[i].substr(2,struc[i].length()-2).c_str()));
                subbudg[i]+= g;
            }
            
        }
        else {
            refresh = 1;
            if (struc[i][0] == ':'){
                o += struc[i].substr(1,struc[i].length()-1);
            }
            else if (struc[i][0] == '$'){
                if (struc[i].length() > 1){ // otherwise this is just a "put this in the subs rather than base subs" flag
                    std::string s = struc[i].substr(1,struc[i].length()-1);
                    if (subbudg[i] > 0 && subs.find(s) != subs.end()){
                        std::vector<std::string> v = subs.at(s)[rng()%(subs.at(s).size())];
                        o += global_cfs_pass_mt(rng,v,subbudg[i] - 1,cap && ! resp, resp);
                    } else if (base_subs.find(s) != base_subs.end()){
                        std::vector<std::string> v = base_subs.at(s)[rng()%(base_subs.at(s).size())];
                        o += global_cfs_pass_mt(rng,v,0,cap && ! resp, resp);
                    } // otherwise, you don't have a base sub for this string, bad.
                }
            } else if (struc[i] == "_"){
                o += " ";
            } else if (dict.find(struc[i]) != dict.end()) {
                o += capitalize(dict.at(struc[i])[rng() % dict.at(struc[i]).size()].first,cap,resp);
            }
            cap = 0;
        }
    }
    return o;
}
 


 std::string grammar_bank::global_construct_from_string(const int seed, const std::vector<std::string> struc, unsigned int budget) const{
    std::mt19937 rng(seed);
    return global_cfs_pass_mt(rng,struc,budget,0,1);
}
*/


#define e_struc_name "ep" // change to determine which substitution string is used
#define s_struc_name "sp"
#define mb_struc_name "mbp"
std::vector<std::vector<std::string>>* grammar_bank::e_struc(bool rec){
    if (rec && subs[e_struc_name].size() > 0){
        return (& (subs[e_struc_name]));
    } else {
        return (& (base_subs[e_struc_name]));
    }
}
std::vector<std::vector<std::string>>* grammar_bank::s_struc(bool rec){
    if (rec && subs[e_struc_name].size() > 0){
        return (& (subs[s_struc_name]));
    } else {
        return (& (base_subs[s_struc_name]));
    }
}
std::vector<std::vector<std::string>>* grammar_bank::mb_struc(bool rec){
    if (rec && subs[mb_struc_name].size() > 0){
        return (& (subs[mb_struc_name]));
    } else {
        return (& (base_subs[mb_struc_name]));
    }
}

/*
 format is, basically:
 {subs.txt}
 {dict.txt}
 */
bool grammar_bank::load(const std::string datastring){ // load from string. bool is success
    dict.clear();
    subs.clear();
    unsigned long long pos = 0;
    unsigned long long b; unsigned long long l;
    { // subs
        std::stringstream ss;
        isolate_matched_brackets(datastring, b, l, pos, 1, '{', '}');
        pos = b+l;
        std::string str;
        ss << datastring.substr(b,l);
        while (std::getline(ss,str,'\n')){
            if (str.find('$') != str.npos){
                add_string(str, subs);
            } else {
                add_string(str, base_subs);
            }
        }
    }
    isolate_matched_brackets(datastring, b, l, pos, 1, '{', '}'); // kind of jank
    pos = b;
    std::stringstream ss;
    pos = b+l;
    std::string str;
    ss << datastring.substr(b,l);
    while (std::getline(ss,str,'\n')){
        add_string(str, dict, all_tags);
    }
    return 1;
}
void grammar_bank::gb_exp(std::string &exp) const{ // export
    exp = "";
    exp+= "{\n";
    // subs
    std::map<std::string,std::vector<std::vector<std::string>>>::const_iterator it;
    for (it=subs.begin(); it!= subs.end(); it++){
        for (int i = 0; i<it->second.size(); i++){
            exp+= it->first + " ";
            for (int j = 0; j<it->second[i].size(); j++){
                exp+= it->second[i][j];
                if (j+1<it->second[i].size()){exp+= " ";}
            }
            exp+= "\n";
        }
    }
    // base subs
    for (it=base_subs.begin(); it!= base_subs.end(); it++){
        for (int i = 0; i<it->second.size(); i++){
            exp+= it->first + " ";
            for (int j = 0; j<it->second[i].size(); j++){
                exp+= it->second[i][j];
                if (j+1<it->second[i].size()){exp+= " ";}
            }
            exp+= "\n";
        }
    }
    exp += "}\n{\n";
    {
        std::unordered_map<std::string,std::vector<std::pair<std::string,std::unordered_set<std::string>>>>::const_iterator it; // the raw power of the substring ">>>>::const_iterator"
        for (it = dict.begin(); it != dict.end();){
            for (int i = 0; i< it->second.size(); i++){
                exp += it->second[i].first;
                exp += " @" + it->first;
                std::unordered_set<std::string>::const_iterator jt = it->second[i].second.begin();
                std::string tags = "";
                while (jt != it->second[i].second.end()){
                    tags+= "#" + *jt;
                    jt++;
                }
                exp += tags;
                exp += "\n";
            }
            it++;
        }
    }
    exp += "}";
}

std::string capitalize(std::string i, bool b, bool resp){
    if (resp) {
        return i;
    }
    else {
        if (i.size() > 0){
            if (i[0] <= 'Z' && i[0] >= 'A' && !b){
                i[0] += 'a' - 'A';
            }
            if (b && i[0] >= 'a' && i[0] <= 'z'){
                i[0] -= 'a' - 'A';
            }
            unsigned long p = 0;
            while(i.find(" ",p+1)!=std::string::npos){
                p=i.find(" ",p+1);
                if (p < i.length()){
                    if (i[p+1] <= 'Z' && i[p+1] >= 'A' && !b){
                        i[p+1] += 'a' - 'A';
                    }
                    if (b && i[p+1] >= 'a' && i[p+1] <= 'z'){
                        i[p+1] -= 'a' - 'A';
                    }
                }
            }
        }
    }
    return i;
}

void grammar_bank::remove_tagged(const std::string & tag){ // remove tagged words
    std::unordered_set<std::string> tags = {tag};
    remove_tagged(tags);
}

void remove_tagged_new(const std::vector<std::pair<std::string,std::unordered_set<std::string>>> & wordbank, const std::unordered_set<std::string> & tagset, std::vector<std::pair<std::string,std::unordered_set<std::string>>> & wordbank_new){ // erasing from a vector is slow, and I want to avoid something whose running time is like, O(len wordbank^2 * len tagset), so the better solution is To Make A New One. Then you can swap and murder the other one, and it's at least a little more efficient.
    wordbank_new.clear();
    std::vector<std::pair<std::string,std::unordered_set<std::string>>>::const_iterator it;
    for (it = wordbank.begin(); it!=wordbank.end(); it++){
        std::unordered_set<std::string>::iterator jt = tagset.begin(); // jterator... so true...
        while (jt != tagset.end()){
            if (it->second.contains(*jt)){
                break;
            } else {
                jt++;
            }
            if (jt == tagset.end()){
                wordbank_new.push_back(*it);
            }
        }
    }
}

void grammar_bank::remove_tagged(const std::unordered_set<std::string> & tagset){ // remove tagged words within set
    std::vector<std::pair<std::string,std::unordered_set<std::string>>> temp;
    for (auto it = dict.begin(); it!= dict.end(); it++){
        remove_tagged_new(it->second, tagset, temp);
        it->second.swap(temp);
    }
}

void site_gb::randomly_bias(std::mt19937& rng){
    { // choosing these randomly. getting a random element of an unordered set is a bit of a pain and honestly it would probably be Better to define lexicographic order, use an ordered set, and binary search a random position, but given that there is a limited quantity of tags, that would be mega overkill
        std::unordered_set<std::string>::iterator it;
        it = all_tags.begin();
        int adv_num1 = rng()%all_tags.size();
        int adv_num2 = rng()%(all_tags.size()-1);
        if (adv_num2 == adv_num1){adv_num2++;}
        for (int i = 0; i< adv_num1; i++){
            it++;
        }
        local_type_pref = *it; // local type
        it = all_tags.begin();
        for (int i = 0; i< adv_num2; i++){
            it++;
        }
        local_type_dispref = *it; // type weakness
    }
    remove_tagged(local_type_dispref);
    
    { // limit to one location type
        int i = ((int) rng()) % dict["l"].size();
        dict["l"][0] = dict["l"][i];
        dict["l"].resize(1);
    }
    { // limit to two to four people (mental image I'm imagining: God and two mathematicians cohabitating in a crypt)
        int num = ((int) (rng() % 3))+2;
        for (int i = 0; i<num; i++){
            int j = ((int) rng()) % dict["p"].size();
            dict["p"][i] = dict["p"][j];
        }
        dict["p"].resize(num);
    }
    { // limit to one site type
        int i = ((int) rng()) % dict["s"].size();
        dict["s"][0] = dict["s"][i];
        dict["s"].resize(1);
    }
    update_pref_indices(local_type_pref);
}

void site_gb::update_pref_indices(const std::unordered_set<std::string> & preferences){
    std::unordered_map<std::string,std::vector<std::pair<std::string,std::unordered_set<std::string>>>>::iterator it;
    pref_indices.clear();
    for (it = dict.begin(); it != dict.end(); it++){
        pref_indices[it->first]; // create
        for (int j = 0; j<it->second.size(); j++){
            for (std::unordered_set<std::string>::const_iterator kt = it->second[j].second.begin(); kt != it->second[j].second.end(); kt++){
                if (preferences.contains(*kt)){pref_indices[it->first].push_back(j);break;}
            }
        }
    }
}

void site_gb::update_pref_indices(const std::string & str){
    std::unordered_set<std::string> uset = {str};
    update_pref_indices(uset);
}

std::string site_gb::site_construct_from_string(const int seed, const std::vector<std::string> struc, unsigned int budget) const{
    std::mt19937 rng(seed);
    return site_cfs_pass_mt(rng,struc,budget,0,1);
}

std::string site_gb::site_cfs_pass_mt(std::mt19937 & rng, const std::vector<std::string> struc, unsigned int budget, bool capall, bool respect) const{ // don't use this except in the recursive use of construct_from_string!!
    std::string o = "";
    std::vector<unsigned int> subbudg;
    int subbudgets = 0;
    for (int i = 0; i<struc.size(); i++){
        if (struc[i][0] == '$' && struc[i].length() > 1){
            subbudg.push_back(budget);
            subbudgets++;
        } else {
            subbudg.push_back(0);
        }
    }
    if (subbudgets > 0 && budget > 0){
        std::vector<unsigned int> subbudg_nums;
        subbudg_nums.push_back(0);
        for (int i = 1; i<subbudgets; i++){
            subbudg_nums.push_back(rng()%budget);
        }
        subbudg_nums.push_back(budget);
        std::sort(subbudg_nums.begin(), subbudg_nums.end());
        for (int i = 0; i<subbudgets; i++){
            subbudg_nums[i] = subbudg_nums[i+1]-subbudg_nums[i];
        }
        subbudg_nums.pop_back();
        int i = 0;
        for (int j = 0; j<struc.size();j++){
            if (subbudg[j] > 0){
                subbudg[j] = subbudg_nums[i]; i++;
            }
        }
        // this code will work flawlessly the first time, watch.
        // holy shit it did?! I really wasn't expecting that
    }
    bool cap = 0;
    bool resp = 0;
    bool refresh = 1;
    for (int i = 0; i<struc.size(); i++){
        if (refresh && capall) {
            cap = 1;
        }
        if (refresh && respect) {
            resp = 1;
        }
        if (struc[i][0] == '^'){
            refresh = 0;
            if (struc[i][1] == 'c') {
                cap = 1; resp = 0;
            } else if (struc[i][1] == 'l') {
                cap = 0; resp = 0;
            } else if (struc[i][1] == 'r') {
                resp = 1;
            } else if (struc[i][1] == '+') {
                int g = std::atoi((struc[i].substr(2,struc[i].length()-2).c_str()));
                subbudg[i]+= g;
            }
            
        }
        else {
            refresh = 1;
            if (struc[i][0] == ':'){
                o += struc[i].substr(1,struc[i].length()-1);
            }
            else if (struc[i][0] == '$'){
                if (struc[i].length() > 1){ // otherwise this is just a "put this in the subs rather than base subs" flag
                    std::string s = struc[i].substr(1,struc[i].length()-1);
                    if (subbudg[i] > 0 && subs.find(s) != subs.end()){
                        std::vector<std::string> v = subs.at(s)[rng()%(subs.at(s).size())];
                        o += site_cfs_pass_mt(rng,v,subbudg[i] - 1,cap && ! resp, resp);
                    } else if (base_subs.find(s) != base_subs.end()){
                        std::vector<std::string> v = base_subs.at(s)[rng()%(base_subs.at(s).size())];
                        o += site_cfs_pass_mt(rng,v,0,cap && ! resp, resp);
                    } // otherwise, you don't have a base sub for this string, bad.
                }
            } else if (struc[i] == "_"){
                o += " ";
            } else if (dict.find(struc[i]) != dict.end()) {
                unsigned long ind;
                float josephine = (rng() * 1.0 / rng.max());
                if (josephine < 0.5 && pref_indices.at(struc[i]).size() > 0){
                    ind = pref_indices.at(struc[i])[rng() % pref_indices.at(struc[i]).size()];
                }
                else {
                    ind = rng() % dict.at(struc[i]).size();
                }
                if (dict.at(struc[i])[ind].second.contains("RARE") && (rng() < rng.max()/2)){
                    josephine = (rng() * 1.0 / rng.max());
                    if (josephine < 0.5 && pref_indices.at(struc[i]).size() > 0){
                        ind = pref_indices.at(struc[i])[rng() % pref_indices.at(struc[i]).size()];
                    }
                    else {
                        ind = rng() % dict.at(struc[i]).size();
                    }
                }
                o+= capitalize(dict.at(struc[i])[ind].first,cap,resp);
            }
            cap = 0;
        }
    }
    return o;
}

site_gb::site_gb(const grammar_bank & gb){
    this->dict= gb.dict;
    this->pref_indices.clear();
    this->all_tags= gb.all_tags;
    this->base_subs = gb.base_subs;
    this->subs = gb.subs;
}
site_gb::site_gb(){
    grammar_bank gb;
    this->dict= gb.dict;
    this->pref_indices.clear();
    this->all_tags= gb.all_tags;
    this->base_subs = gb.base_subs;
    this->subs = gb.subs;
}


std::string site_gb::get_site_name(const std::mt19937& rng, int budget){
    bool b = (budget > 0);
    std::mt19937 namerng = rng;
    std::vector<std::vector<std::string>>* ptr = s_struc(b);
    /*
    if (b){
        if ((namerng() % (s_struc(0)->size() + s_struc(1)->size()))< s_struc(0)->size()){
            ptr = s_struc(0);
        }
    }
     */
    int i = namerng() % ptr->size();
    int d1 = budget-1; if (d1 < 0){d1 = 0;}
    int j = namerng();
    return site_construct_from_string(j, (*ptr)[i],d1);
}
std::string site_gb::get_enemy_name(const std::mt19937& rng, int budget){
    bool b = (budget > 0);
    std::mt19937 namerng = rng;
    std::vector<std::vector<std::string>>* ptr = e_struc(b);
    /*
    if (b){
        if ((namerng() % (e_struc(0)->size() + e_struc(1)->size()))< e_struc(0)->size()){
            ptr = e_struc(0);
        }
    }
     */
    int i = namerng() % ptr->size();
    int d1 = budget-1; if (d1 < 0){d1 = 0;}
    return site_construct_from_string(namerng(), (*ptr)[i],d1);
}

std::string site_gb::get_boss_name(const std::mt19937& rng, int budget){
    bool b = (budget > 0);
    std::mt19937 namerng = rng;
    std::vector<std::vector<std::string>>* ptr = mb_struc(b);
    /*
    if (b){
        if ((namerng() % (mb_struc(0)->size() + mb_struc(1)->size()))< mb_struc(0)->size()){
            ptr = mb_struc(0);
        }
    }
     */
    int i = namerng() % ptr->size();
    int d1 = budget-1; if (d1 < 0){d1 = 0;}
    return site_construct_from_string(namerng(), (*ptr)[i],d1);
}
