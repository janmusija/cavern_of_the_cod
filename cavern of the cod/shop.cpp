//
//  shop.cpp
//  cavern of the cod
//
//  Created by jan musija on 9/21/25.
//

#include "shop.h"

void calc_hp(const player_data & pd, int_large & hp_amount, int_large & hp_cost){
    int_large hupgr_mult = 1;
    hupgr_mult += (pd.base_hp / 50)-2;
    if (hupgr_mult < 1) {hupgr_mult = 1;}
    hp_amount = 10 * hupgr_mult;
    hp_cost = hupgr_mult * trunc_root(pd.base_hp,3) * 10;
}

std::string display_warez(const player_data & pd, const world_data & wd, shop_cache & sc){
    std::string o;
    int_large hp_amt, hp_cost;
    calc_hp(pd,hp_amt, hp_cost);
    o+= "Health upgrade: " + hp_amt.get_str(10) + " hp: cost: " + hp_cost.get_str(10) + "\n";
    o+= "Buy shop reroll: cost: " + std::to_string(SHOP_REROLL_COST) + "\n";
    if (pd.xp.get_str(4).length() > pd.inventory_slots + (2 - BASE_INV_SLOTS)){
        o+= "Buy inventory slot: cost: " + pow_ui(4, pd.inventory_slots - (BASE_INV_SLOTS - 3)).get_str(10) + "\n";
    }
    if (pd.xp.get_str(7).length() > pd.equip_slots + (2 - BASE_EQUIP_SLOTS)){
        o+= "Buy equip slot: cost: " + pow_ui(7, pd.equip_slots - (BASE_EQUIP_SLOTS - 3)).get_str(10) + "\n";
    }
    if (sc.init == 0){
        sc.refresh();
    }
    if (pd.inventory_slots > pd.dice.owned.size()){
        o+= "Buy " + sc.shopdie.description + ": cost: " + sc.shopdie_cost.get_str(10) + "\n";
    }
    return o;
}

bool buy_hp_upgr(player_data & pd, int_large & amount_gained){ // bool is success
    int_large hp_amt, hp_cost;
    calc_hp(pd,hp_amt, hp_cost);
    amount_gained = hp_amt;
    if (hp_cost <= pd.coins){
        pd.coins -= hp_cost;
        pd.base_hp += hp_amt;
        return 1;
    }
    else{
        return 0;
    }
}

//deprecate eventually
die generate_shopdie_rudimentary(std::mt19937 & temp_rng, int_large & cost, int_large & cost_ceil){
    std::ifstream g;
    g.open("params_simple/dice_types_rudimentary.txt");
    std::vector<std::pair<int_large,die>> dice_weighted; // stands for "associated".
    dice_weighted.push_back(std::pair(0,die("a;1;1;1 ,2 ,3 ,4 ,5 ,6 ;attack die"))); // hardcoded
    std::string str;
    while (std::getline(g,str,'\n')){
        int_large val = int_large(str);
        std::getline(g,str,'\n');
        die d = die(str);
        if (d.guaranteed_safe()){
            val*=2;
        }
        if (d.type == "a"){
            val/=3;
        }
        if (d.type == "h"){
            val*=3;
            val/=2;
        }
        if (d.type == "m"){
            val*=((val/4)+1);
        }
        if (d.type == "hm"){
            val*=val;
        }
        if (val <= cost_ceil){
            dice_weighted.push_back(std::pair(val,d));
        }
    }
    std::vector<int> chosendice;
    for (int i = 0; i<dice_weighted.size(); i++){
        bool roll;
        if (dice_weighted[i].first == 0){
            roll = 1;
        } else {
            roll = (temp_rng()%(dice_weighted[i].first.get_str(7).length()+1) == 0);
        }
        if (roll){
            chosendice.push_back(i);
        }
    }
    int choice = chosendice[temp_rng()%chosendice.size()];
    cost = DICE_COST_MULT * (dice_weighted[choice].first+1);
    return dice_weighted[choice].second;
}

shop_cache::shop_cache(){
    init = 0;
}

void shop_cache::refresh(){
    std::mt19937 temp_rng = rng;
    int_large cost;
    int_large cost_ceil = 500;
    shopdie = generate_shopdie_rudimentary(temp_rng,cost,cost_ceil);
    init = 1;
    shopdie_cost = cost;
}
