//
//  base_info.cpp
//  ominous dice torment game
//
//  Created by jan musija on 9/13/25.
//

#include "base_info.h"
#include <string>
#include <sstream>
#include <iostream> //debug

bool isolate_matched_brackets(std::string const & str, unsigned long long & beginning, unsigned long long & length, unsigned long long const start_pos, bool truncate_newlines, char open, char closed){ // for save/export manipulation
    unsigned long long tally = 0;
    unsigned long long pos = start_pos;
    if (pos > 1000000){throw;}
    unsigned long long len = str.size();
    bool reached_begin = 0;
    while (pos < len && !reached_begin){
        if (str[pos] == open){reached_begin = 1;tally++;}
        pos++;
        if (pos == len){
            return 0; // reached end of string witout an opening
        }
    } // note that this takes you to the position AFTER the open, with a tally of 1.
    if (truncate_newlines){
        while (pos < len && str[pos] == '\n'){pos++;}
        if (pos == len){
            return 0; // reached end of string truncating newlines
        }
    } // and now to after all newlines after that, if applicable
    beginning = pos; // this is the beginning of this substring.
    while(tally > 0){ // until you find a close for the first open
        if(str[pos] == open){
            tally++;
        } else if(str[pos] == closed) {
            tally--;
        }
        if (tally > 0) {pos++;}
        else // you are currently on the close for the first open
            pos--; // backtracking
        if (pos == len){
            return 0; // reached end of string searching for close
        }
    }
    // now we are just before the first close
    if (truncate_newlines){
        while(str[pos] == '\n' && pos > beginning){
            pos--;
        }
    }
    // now we are just before the first newline after the close
    length = (pos - beginning)+1;
    //std::cout << "BLOCK:\n{\n" + str.substr(beginning,length) + "\n}\n"; // debug
    return 1;
}

base_data::base_data (int input_seed, std::string title) {
    seed = input_seed;
    std::mt19937 initrng(seed);
    rng = initrng;
    gaming = 1;
    context.push("titlescreen");
    savename = title;
}

base_data::base_data () { // bad
    seed = 0;
    std::mt19937 initrng(seed);
    rng = initrng;
    gaming = 0;
    context.push("titlescreen");
    savename = "uninitialized game";
}

std::string base_data::current_context() {
    return context.top();
}

entity_data::entity_data(int health){
    dice = dice_collection();
    // empty collection
    base_hp = health; // for now
}

entity_data::entity_data(){
    dice = dice_collection();
    // empty collection
    base_hp = 100; // for now
}

bool entity_data::unroll_if_all_rolled(){ // 1 if all dice were rolled
    std::vector<int> garb;
    bool b = dice.remaining_unrolled(garb,1);
    if (!b){
       dice.unroll();
        return 1;
    }
    else return 0;
}

player_data::player_data(){
    dice = dice_collection();
    dice.push_new_die_from_string("a;1;1;1 ,2 ,3 ,4 ,5 ,6 ;your first die",1); // equip one die.
    //dice.push_new_die_from_string("h;1;0;2 ,3 ,4 ,5 ,6 ;health >1 die",1); // test
    //dice.push_new_die_from_string("hm;1;0;2 ,3 ,4 ,5 ,6 ;health mult >1 die",1); // test
    xp = 0;
    base_hp = 100; // for now
    inventory_slots = BASE_INV_SLOTS;
    equip_slots = BASE_EQUIP_SLOTS; // will definitely be improvable
    coins = 0;
}

bool player_data::load(const std::string & datastring){
    /*
     {
      dice_collection
     }
     {
     xp
     base_hp
     inventory_slots
     equip_slots
     coins
     }
     */
    unsigned long long pos = 0;
    unsigned long long b; unsigned long long l;
    std::string str;
    isolate_matched_brackets(datastring, b, l, pos, 1, '{', '}');
    str = datastring.substr(b,l);
    dice = dice_collection(str);
    pos = b+l;
    isolate_matched_brackets(datastring, b, l, pos, 1, '{', '}');
    str = datastring.substr(b,l);
    std::stringstream ss;
    ss << str;
    std::getline(ss,str,'\n'); // xp
    xp = mpz_class(str.c_str()); // atoll is a funny name, given that it is a word.
    std::getline(ss,str,'\n'); // base hp
    base_hp = mpz_class(str.c_str());
    std::getline(ss,str,'\n'); // inv slots
    inventory_slots = std::atoi(str.c_str());
    std::getline(ss,str,'\n'); // equip slots
    equip_slots = std::atoi(str.c_str());
    std::getline(ss,str,'\n'); // coins
    coins = mpz_class(str.c_str());
    return 1;
}

void player_data::pd_exp(std::string &exp) const{
    exp = "{\n";
    exp += dice.collection_data();
    exp += "\n}\n{\n";
    exp += xp.get_str(10) + "\n";
    exp += base_hp.get_str(10) + "\n";
    exp += std::to_string(inventory_slots) + "\n";
    exp += std::to_string(equip_slots) + "\n";
    exp += coins.get_str(10) + "\n}";
}

/*
 format:
{
 rng state
}
{
 seed
 savename
}
 */
bool base_data::load(const std::string & datastring){
    unsigned long long pos = 0;
    unsigned long long b; unsigned long long l;
    isolate_matched_brackets(datastring, b, l, pos, 1, '{', '}');
    {
        std::stringstream ss;
        ss << datastring.substr(b,l); // twister state
        ss >> rng;
    }
    pos = b+l;
    isolate_matched_brackets(datastring, b, l, pos, 1, '{', '}');
    {
        std::stringstream ss;
        ss << datastring.substr(b,l);
        std::string str;
        std::getline(ss,str,'\n'); // seed
        seed = std::atoi(str.c_str());
        std::getline(ss,str,'\n'); // savename
        savename = str;
    }
    return 1;
}

void base_data::bd_exp(std::string &exp) const{
    exp = "{\n";
    std::stringstream twister;
    twister << rng;
    exp += twister.str() + "\n}\n{\n";
    exp += std::to_string(seed) + "\n";
    exp += savename + "\n}";
}

bool health_haver::alive(){
    return (hp > 0);
}

health_haver::health_haver(int health){
    dice = dice_collection();
    // empty collection
    base_hp = health; // for now
    hp = health;
}

health_haver::health_haver(){
    dice = dice_collection();
    // empty collection
    base_hp = 100; // for now
    hp = 100;
}

/*
 --- special inv items ---
 */

void special_items::empty(){
    contents.clear();
}
bool special_items::has(const std::string & item) const{
    std::map<std::string,unsigned int>::const_iterator it = contents.find(item);
    return (it != contents.end() && it->second > 0);
}

bool special_items::use(const std::string & item){ // return 1 if use successful; 0 if none remain.
    std::map<std::string,unsigned int>::iterator it = contents.find(item);
    if (it != contents.end() && it->second > 0){
        (it->second)--;
        return 1;
    } else {
        return 0;
    }
}

unsigned int& special_items::operator[] (const std::string& k){
    return contents[k];
}
