//
//  world_gen.h
//  ominous dice torment game
//
//  Created by jan musija on 9/13/25.
//

#ifndef world_gen_h
#define world_gen_h

#include <stdio.h>
#include <fstream>
#include "enemy.h"
#include <random>
#include <vector>
#include <unordered_set>
#include "nomenclature.h"
#define MIN_ENEMIES_EACH_FLOOR 3
#define HASH_MAX SIZE_MAX

class site_room {
public:
    int height;
    std::vector<int> bpath; // branch path
    
    std::string curr_path() const;
    bool mainline() const; // is mainline?
    /*
     nonmainline -> can generate minibosses
     */
    site_room(); // initialize
    void increment(); // increment room
    void branch(); // increment room onto a branching path
    void clear();
    void decrement(); // decrement room
    int calc_equiv_floor();
};

class site{ // consideration: make sites nonlinear-- have floors with multiple pieces to them
public:
    std::string name;
    int_large xp_min; // "minimum level" of site
    int_large xp_ramping; // "rate of ramping" at site
    int highest_floor; // highest floor player has reached, initially 0
    
    std::vector<int> enemy_uuids; // uuids of local enemies
    std::map<std::string,int> boss_uuids; // current_room.curr_path() |-> uuid of unique[caveat] boss enemy
    std::mt19937 enemy_rng;
    
    // for generating enemies at this site -- this allows the same enemies to appear when a new enemy type is created, so long as it's created within the right xp range
    site(const int_large & xp_lvl, std::mt19937 & wg_rng, const grammar_bank & base_gram, std::mt19937 & grammar_variation_rng); // create new site. presumably, player can choose its difficulty level
    
    site_gb loc_grammar; // for generating names
    site(); // blank
    site(const std::string & str); // import
    std::string site_exp() const; //export
};

class world_data{
public:
    // permanent content:
    std::vector<site> sites;
    std::vector<enemy_type> enemy_types;
    grammar_bank base_gram;
    
    
    // rngs:
    std::mt19937 site_rng;
    std::mt19937 shop_rng; // for cycling the current shop contents after purchases/refreshes.
    std::mt19937 grammar_variation_rng; /* does not need to be exported. it is initialized to the seed.
                                         the reason for this existing is that it ensures that you don't have to store separate grammars for each site when exporting, because the variations are determined just by doing this in sequence for each site
                                         */
    
    // functions
    world_data(const std::mt19937 & rng); // create new world with this rng.
    void wd_exp(std::string &exp); // export. is actually const but can't be said to be for Reasons
    bool load(const std::string & datastring, int seed); // load from string. bool is success.
    world_data(); // blank
    unsigned long add_site(int_large xp_min); // result is the new site's position in the site vector
    //void normalize_uuids(); // resets uuids of enemies to vector indices, and associated site indices //deprecated
    
    // stuff migrated from site due to access reasons:
    int generate_enemy(int_large xp_lvl, int s_ind); // generates a new local enemy with a given minimal difficulty xp (associated to ramping). int output is associated uuid
    int generate_boss(int_large xp_lvl, int s_ind); // generates a new local boss with a given minimal difficulty xp (associated to ramping). int output is associated uuid
    int floor_enemies(int floor,  int s_ind, std::vector<int> & out_uuids, bool sortbool = 1); // number of local enemy types at floor. vector is the associated uuids
    std::string view_info(int s_ind, int verbosity = 0) const; // xp min, highest floor reached, known local enemy names
    std::string view_enemy(int e_ind, int verbosity = 0) const; // xp min, highest floor reached, known local enemy names
    void fill_floor(int floor, int s_ind, int extra = 0); // fill out remaining enemies at this floor level
};

class temp_data{
    /*
     mainly contains information pertinent to battles, and the flag for player input. consequently:
     __ do not put anything here that is relevant outside the context of a battle!!! (other than the input_desired flag I guess) __
     */
public:
    int seed;
    temp_data(); //constructor -- will actually be used, mostly. sets values to the same as:
    void reset(); // quitting to title, etc.
    
    // various flags:
    bool combat; // if in combat
    bool player_turn; // if it is the player's turn [if not, it is the enemies']
    bool input_desired; //whether or not the player's input is needed
    bool newfloor; // enable this and disable input_desired to generate enemies for this floor (and clear existing enemies)
    
    // site and floor information
    int selected_site;
    site_room current_floor;
    std::vector<enemy> enemies;
    //std::vector<bool> deaths_looted; // positions of dead enemies whose loot has been allocated -- multiattack not implemented yet, so not necessary
    site* site_ptr; // usually nullptr
    bool branch_here() const;
    bool continue_here() const;
    std::string room_hash() const;
    std::unordered_set<std::string> cleared_rooms; // rooms that have been cleared-- rooms which have already been cleared do not generate enemies.
    void set_this_room_clear_in_map(); // adds to cleared_rooms, duh.
    bool thisfloor_cleared() const; // used to be a toggleable flag. now checks if this floor has been set clear in the map.
    int room_type() const; // determine room type from the current room. Possible types:
    /*
     0 -> enemies spawn here
     1 -> boss
     2 -> small buff room (contains low-tier consumable)
     */
    bool subterranean_floors_entered; // this totally doesn't Secrets.
    std::unordered_set<std::string> looted_rooms; // rooms that have been looted.
    void set_this_room_looted_in_map(); // adds to cleared_rooms, duh.
    bool thisfloor_looted() const; // used to be a toggleable flag. now checks if this floor has been set clear in the map.
    
    // roll info
    int_large damage_pastrolls; // does not include this roll. add to get total damage
    int_large damage_thisroll; //for applying multiplication
    int_large this_mult; //for applying multiplication
    int_large health_pastrolls;
    int_large health_thisroll;
    int_large this_health_mult;
    std::map<std::pair<std::string,std::string>,int> rollcounts; // type x rollraw_concat_mods -> number times already rolled
    
    // consumables contents:
    /*
     "first aid kit" -- full heal, when not in combat
     "extra life" -- restores you to 10 hp if you're killed by enemies
     "insurance potion" -- imbues roll_insurance until the end of turn
     "retreat crystal" -- immediately retreat from current floor
     "cod charm" -- used for transport to the !!cavern of the cod!!
     
     "roll_insurance" -- (status effect) -- prevents a roll failure from ending your turn or voiding the rest of the roll.
     */
    special_items consumables; // includes both items and temporary status effects (e.g. fail protection). as well as permanent things that (despite the name
    void initialize_consumables(); // initializes consumables to default
    void initialize_consumables(std::map<std::string,unsigned int> con); // initializes consumables to states in con
    void end_turn_consumables(); // ends turn-based status effects
    std::string usable_consumables(); // list of consumables usable by "use"
    std::string all_consumables(); //list all consumables and status effects.
    
    // roll manipulation:
    int resolve_roll(die & roller, const int_large & roll, const int_large & rollraw, const std::string & mod); // add a roll to thisroll. returns number of times this specific roll has already happened, for doubles.
    void commit_roll(); // commit thisroll to pastrolls.
    void calc_rolls(int_large &dam, int_large &health) const; // calculate current damage
    void reset_roll(); //set roll to 0
    
    // setting flags in game phases
    void end_player_turn(player_data & pd);
    void end_enemy_turn();
    void advance_floor(bool br);
    void retreat_floor();
    void cod_ritual();
    
    // misc
    bool enemies_surviving(std::vector<int>& positions); // put remaining enemy indices in vector. bool is if there are any
    
    
    //dealing damage, etc. commit_roll() before using these!!
    void apply_damage(health_haver & targ) const;
    void apply_healing(health_haver & targ) const;
    
    // log of previous attack phase
    std::string prev_attack_log;
};

int_large floor_xp_level(const int_large & min_xp, const int_large & xp_ramping, int floor);

void xp_interval(const int_large & xp_tier, const int_large & next_xp_tier, int_large & xp_floor, int_large & xp_ceiling);

#endif /* world_gen_h */
