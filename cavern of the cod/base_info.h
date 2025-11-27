//
//  base_info.h
//  ominous dice torment game
//
//  Created by jan musija on 9/13/25.
//

#ifndef base_info_h
#define base_info_h
#define BASE_INV_SLOTS 8
#define BASE_EQUIP_SLOTS 3


#include <random>
#include <stack>
#include <fstream>
#include "dice.h"
#include <string>
#include "int_large.h"
#include <map>

bool isolate_matched_brackets(std::string const & str, unsigned long long & beginning, unsigned long long & length, unsigned long long const start_pos = 0, bool truncate_newlines = 1, char open = '{', char closed = '}'); // for save/export manipulation. bool result is if match exists at all

class base_data {
public:
    int seed;
    std::mt19937 rng;
    bool gaming;
    std::stack<std::string> context;
    std::string savename;
    base_data(int input_seed, std::string title); // creating fresh game
    base_data(); // initialize trash game to not be used
    std::string current_context();
    bool load(const std::string & datastring); // load from string. bool is success
    void bd_exp(std::string &exp) const;
};

class entity_data {
public: dice_collection dice;
    entity_data(int health); // initialize a blank entity with some amount of health
    entity_data(); // initialize a blank entity with 100 health
    int_large base_hp;
    bool unroll_if_all_rolled(); // return 1 if all dice were rolled
};

class health_haver : public entity_data {
public: dice_collection dice;
    health_haver(int health); // initialize a blank entity with some amount of health
    health_haver(); // initialize a blank entity with 100 health
    int_large hp;
    bool alive();
};

class special_items {
public:
    std::map<std::string,unsigned int> contents;
    void empty();
    bool use(const std::string & item); // return 1 if use successful; 0 if none remain.
    bool has(const std::string & item) const;
    unsigned int& operator[] (const std::string& k);
};

class player_data : public health_haver {
public:
    int_large xp; // I don't plan for this to be really used for much other than maybe unlocking areas and Flexing.
    player_data(); // initialize blank player
    int inventory_slots;
    int equip_slots;
    int_large coins; // for use in the SHOP.
    bool load(const std::string & datastring); // load from string. bool is success
    void pd_exp(std::string &exp) const;
    special_items inv_items;
};



#endif /* base_info_hpp */
