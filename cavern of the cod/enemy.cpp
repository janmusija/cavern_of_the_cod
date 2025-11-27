//
//  enemy.cpp
//  ominous dice torment game
//
//  Created by jan musija on 9/13/25.
//

#include "enemy.h"
#include "base_info.h"
#include "int_large.h"
#include <algorithm>

// blank constructors

/* // old deprecated overambitious loot system
 loot_table::loot_table(){
    coin_quantity =0;
    xp_quantity =0;
    coin_stdev =0;
    xp_stdev =0;
    dice = {};
}
 */

enemy_type::enemy_type(){
    name = "UNINITIALIZED FOE";
    base_hp = 1;
    dice = dice_collection();
    dice.push_new_die_from_string("a;1;1;1 ,2 ,3 ,4 ,5 ,6 ;attack die",1);
    ai_type = 0;
    greed = 0.5;
    //loot = loot_table();
    base_xp_drop = 0;
    xp_minimum = 0;
    //uuid = -1; -- deprecated
    dice.equip_success_chance(dice_success_num, dice_success_denom);
}

enemy::enemy(){
    enemy_type blank;
    name = blank.name;
    ai_type = blank.ai_type;
    greed = blank.greed;
    base_hp = blank.base_hp;
    hp = blank.base_hp;
    dice = blank.dice;
    type_uuid = -1; // this entails BAD!!
    //loot = blank.loot;
}


/*
 ***** LOOT *****
 */
/* // deprecated system
loot_table::loot_table(int_large cq, int_large xp, int_large c_s, int_large x_s){
    coin_quantity = cq;
    xp_quantity = xp;
    coin_stdev = c_s;
    xp_stdev = x_s;
    dice = {};
}

void loot_table::drop_loot(std::mt19937& rng,int_large & new_coins, int_large & new_xp, std::vector<die> & new_dice){
    if (coin_stdev == 0 ){
        new_coins = coin_quantity;
    } else {
        double sd = (coin_stdev*1.0)/(coin_quantity);
        std::normal_distribution<double> distribution(1.0,sd);
        double mult = distribution(rng);
        new_coins = std::llround(coin_quantity*mult);
    }
    if (xp_stdev == 0 ){
        new_coins = xp_quantity;
    } else {
        double sd = (xp_stdev*1.0)/(xp_quantity);
        std::normal_distribution<double> distribution(1.0,sd);
        double mult = distribution(rng);
        new_xp = std::llround(xp_quantity*mult);
    }
    new_dice = {};
    for (int i = 0; i< dice.size(); i++){
        if (rng()/(rng.max()*1.0 - rng.min()*1.0) < dice[i].first){
            new_dice.push_back(dice[i].second);
        }
    }
    if (new_coins < 0) {
        new_coins = 0;
    }
    if (new_xp < 0) {
        new_xp = 0;
    }
}
 */

enemy_type::enemy_type(std::mt19937 & rng, int_large xp_lev, std::string new_name){
    dice = {};
    name = new_name;
    xp_minimum = xp_lev;
    int_large budget = trunc_root(xp_lev,2)*10 + 1;// can be tweaked
    if (budget < 0){
        budget = 0;
    }
    
    base_hp = trunc_root(xp_lev,3)+10; // can also be tweaked
    /*if (base_hp < 0){
        base_hp = LONG_LONG_MAX; // probably irrelevant once int_large is fully implemented. more long long business.
    }*/
    
    // budget goes to:
    // ai type
    // health
    // dice
    
    // ai type:
    // 0 : push luck random amount of times determined by greed. 1: push luck maximal amount of times by using Magic. 2 : calculate chance p of success rolling all dice; roll floor(-1/ln(p)) times and then roll random dice per greed
    // weights at lvl 0: 0 is 58/64ths of enemies -- a bit over 90%
    // 1 is 1/64th of enemies
    // 2 is 5/64ths of enemies
    // at higher levels, proportions shift
    
    /* relevant bit of math for calculating value of weight 1:
     suppose the chance of dealing (wlog 1) damage without failing is a chance of 0<=p<1.
     now an oracle (i.e. ai type 1) correctly guesses 100% of the time, so deals 1 damage p of the time, another 1 damage p^2 of the time, etc-- sum n = 1 to inf p^n = p/(1-p)
     meanwhile, you, a mortal, can only choose to roll an average of x times per round, dealing x * p^x damage.
     d/dx x * p^x = p^x + ln(p) * x * p^x, this equals zero when ln(p) * x = -1, which is when x = -1/ln(p) (negative because p<1 means ln(p) is negative). Thus, the best YOU can realistically deal per turn is -1/ln(p) * p^(-1/ln(p)) = -1/ln(p) * e^(-ln(p)/ln(p)) = -1/(e * ln(p)) damage per turn.
     -- and yes this means if your chance of success each turn is p, you should go for -1/ln(p) turns. I solved my own game, before I finished devving it --
     there's some funny business with this estimate when the risk is very high because of the fact that x * p^x is a bit wrong, I think, when x is small.
     but anyway, how much better is an oracle?
     e * p ln(p)/(p-1)
     and the limit as p tends to 1?, well, that's
     e * lim {p -> 1} p ln p / (p - 1) = e * lim {p -> 1} (ln p + p/p) / 1 = e. In other words, an oracle can't do more than e better than a non-oracular player, basically.
     [roll your dice -1/ln(5/6) = 5.4848 times! okay actually you should pick the integer maximizing x * p^x (either floor(-1/lnp) or ceil(-1/ln(p)), depending.)-- incidentally if p = n/(n+1) there are two solutions which are n and n+1. so roll your dice 5 times, or 6 times, it's the same. graph of how much better an oracle is: https://www.desmos.com/calculator/rgkvrgjmhn . tldr "an oracle is no better than e times better"]
    */
    
    int ai_roll = rng()%64;
    int shift = (((int)xp_lev.get_str(10).length())-3)/2; if (shift < 0){shift = 0;}
    if (ai_roll < 58 - 2 * shift){
        ai_type = 0;
        // determine greed. for now, 0.5 + 0.5*random in [0,1)
        greed = 0.5;
        greed += (rng()*0.5)/rng.max();
    } else if (ai_roll <63 - shift){
        ai_type = 2;
        // determine greed. for now, random in [0,1)
        greed += (rng()*1.0)/rng.max();
        ai_type = 1;
        greed = 0;
    } else {
        ai_type = 1;
        greed = 0;
    }
    
    
    // allocate budget
    int_large hp_budg, dice_budg;
    // you could binary search but that would be insane
    hp_budg = random_il_below(rng,budget);
    dice_budg = (budget - hp_budg);
    
    /*{
    std::vector<int_large> cutoffs;
        for (int i = 0; i<2; i++){ // 2 = number of things to distribute between, -1
            cutoffs.push_back(random_il(rng)%(budget+1));
        }
        std::sort (cutoffs.begin(), cutoffs.end());
        hp_budg = cutoffs[0]; dice_budg = cutoffs[1]-cutoffs[0];
    }*/
    
    // hp allocation
    base_hp += hp_budg;
    
    int_large desired_perroll_damage; // at budget 0, about 4. Generally, want to deal player_hp/25 or so, I guess? Approximate: player hp is about hp is about dice budg...
    
    desired_perroll_damage = (dice_budg/25)+4;
    if (ai_type == 2){
        if (desired_perroll_damage < 6){ // too low
            ai_type = 0;
        }
        desired_perroll_damage = (desired_perroll_damage*2)/3;
    }
    if (ai_type == 1){ // too low
        if (desired_perroll_damage < 16){
            ai_type = 0;
        } else {
            desired_perroll_damage = desired_perroll_damage/3;
        }
    }
    
    int_large under_budg = 0;
    
    // TK : dice generation -- weighted based on ai type
    dice = dice_collection();
    if (desired_perroll_damage <= 4){
        dice.push_new_die_from_string("a;1;1;1 ,2 ,3 ,4 ,5 ,6 ;attack die",1); // expected damage is 4
    } else {
        dice.select_rudimentary(rng, desired_perroll_damage, under_budg); // for now. later make a non-stupid version
        //dice.push_new_die_from_string("a;1;1;1 ,2 ,3 ,4 ,5 ,6 ;attack die",1); // TK
    }
    if (under_budg > 0){
        base_hp += under_budg;
    }
    
    // enemy specific loot deprecated, don't plan on readding
    // loot = loot_table();
    
    base_xp_drop = trunc_root(xp_lev,2);
    /*if (base_xp_drop < 0){ // irrel
        base_xp_drop = LONG_LONG_MAX;
    }*/
    if (base_xp_drop < 5){
        base_xp_drop = 5; // minimum of 5 xp dropped.
    }
    
    dice.equip_success_chance(dice_success_num, dice_success_denom);
}

void enemy::set_type(const enemy_type & typ, int uuid){
    name = typ.name;
    ai_type = typ.ai_type;
    greed = typ.greed;
    base_hp = typ.base_hp;
    hp = base_hp;
    dice = typ.dice;
    type_uuid = uuid;
}

enemy::enemy(const enemy_type & typ, int uuid){
    set_type(typ,uuid);
}

bool enemy::checkgreed(std::mt19937 & rng) const{
    float res = (rng()*1.0)/rng.max();
    return (greed > res);
}

#include <sstream>
enemy_type::enemy_type(const std::string & data_str){ // import
    /* format:
     name
     xp_minimum
     base_hp
     base_xp_drop
     ai_type
     greed
     {
     dice (collection_data)
     }
     */
    std::stringstream ss;
    ss << data_str;
    std::getline(ss,name,'\n'); // name
    std::string s;
    std::getline(ss,s,'\n'); // xp_minimum
    xp_minimum = int_large(s);
    std::getline(ss,s,'\n'); // base_hp
    base_hp = int_large(s);
    std::getline(ss,s,'\n'); // base_xp_drop
    base_xp_drop = int_large(s);
    std::getline(ss,s,'\n'); // ai_type
    ai_type = std::atoi(s.c_str());
    ss << std::hexfloat;
    ss >> greed; // greed?
    std::string str = ss.str(); // dice
    unsigned long long b, l, pos; pos = 0;
    isolate_matched_brackets(str, b, l, pos, 1, '{', '}');
    dice = dice_collection(str.substr(b,l));
}
std::string enemy_type::type_exp() const{ // export
    std::string o;
    o+=name + "\n";
    o+= xp_minimum.get_str(10) + "\n";
    o+= base_hp.get_str(10) + "\n";
    o+= base_xp_drop.get_str(10) + "\n";
    o+= std::to_string(ai_type) + "\n";
    std::stringstream jank; jank << std::hexfloat << greed;
    o+= jank.str() + "\n{\n";
    o+= dice.collection_data();
    o+= "\n}";
    return o;
}
