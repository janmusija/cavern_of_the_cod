//
//  enemy.h
//  ominous dice torment game
//
//  Created by jan musija on 9/13/25.
//

#ifndef enemy_h
#define enemy_h

#include <stdio.h>
#include "base_info.h"
#include <random>

/* // old deprecated loot system
 class loot_table {
public:
    int_large coin_quantity;
    int_large xp_quantity;
    int_large coin_stdev;
    int_large xp_stdev;
    std::vector<std::pair<double,die>> dice; // doubles are odds out of 1 of getting one
    void drop_loot(std::mt19937& rng,int_large & new_xp, int_large & new_coins, std::vector<die> & new_dice);
    loot_table(int_large cq, int_large xp, int_large c_s, int_large x_s);
    void add_die(double odds, die d);
    loot_table(); // empty table
    loot_table(std::mt19937& rng, int_large quality); // generate loot table at given xp minumum level
};
 */

class enemy_type : public entity_data {
public:
    /* // inherited from entity_data:
     dice_collection dice;
     int_large base_hp;
     */
    std::string name; // name of enemy type
    int ai_type; // default ai type
    float greed; // default greed
    int_large base_xp_drop; // base xp drop
    int_large xp_minimum; // intended xp level at which this enemy is approachable
    
    
    // loot_table loot; // associated loot drops // deprecated
    //int uuid; // id of this enemy type. in particular, used for sites to identify their contents, and when saving/loading worldgen // deprecated
    enemy_type(std::mt19937 & rand, int_large xp_lev, std::string new_name); // constructor tk
    enemy_type(); // blank
    enemy_type(const std::string &); // import
    std::string type_exp() const; // export
    unsigned long long dice_success_num; // for ai type 2
    unsigned long long dice_success_denom;
};

class enemy : public health_haver {
public:
    std::string name; // name of enemy
    int ai_type; // 0 : push luck random amount of times determined by greed. 1: push luck maximal amount of times by using Magic. 2 : calculate chance p of success rolling all dice; roll floor(-1/ln(p)) times and then roll random dice per greed
    float greed; // see above
    // loot_table loot; // associated loot drops // deprecated
    int type_uuid; // type's uuid.
    enemy(); // create totally blank enemy
    void set_type(const enemy_type & typ, int uuid); // load from type
    enemy(const enemy_type & typ, int uuid); // just the previous two
    bool checkgreed(std::mt19937 & rng) const; // has a greed chance of returning 1.
};

#endif /* enemy_h */
