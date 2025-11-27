//
//  combat.cpp
//  ominous dice torment game
//
//  Created by jan musija on 9/15/25.
//

#include "combat.h"
#include "stream_colors.h"
#include <math.h>

void get_enemies(std::mt19937 & rng, temp_data & td, world_data & wd){ // initialize enemies of a floor
    td.enemies.clear(); // should be already, but just to be safe.
    int floor = td.current_floor.calc_equiv_floor(); // see documentation there for details
    //int_large xplev = floor_xp_level(td.site_ptr->xp_min,td.site_ptr->xp_ramping, floor);
    wd.fill_floor(floor, td.selected_site,0);
    std::vector<int> out_uuids;
    int local_pop_variants = wd.floor_enemies(floor, td.selected_site, out_uuids);
    int desired_enemies = (rng()%(std::to_string((floor*floor)/2).length()))+1; // can be tweaked
    for (int i = 0; i<desired_enemies; i++){
        int uid = out_uuids[rng()%local_pop_variants];
        td.enemies.push_back(enemy());
        td.enemies[td.enemies.size()-1].set_type(wd.enemy_types[uid], uid); // I think doing this this way prevents and extraneous copying
        // [per-enemy variance?]
    }
    // flags
    td.newfloor = 0;
    td.player_turn = 1;
    td.input_desired = 1;
}

std::string display_combatants(const temp_data & td, const world_data & wd){
    std::string o;
    for (int i = 0; i<td.enemies.size(); i++){
        o+= view_combatant(td,wd,i,0) + "\n";
    }
    return o;
}

std::string view_combatant(const temp_data & td, const world_data & wd, int num, int verbosity){
    if (num < td.enemies.size()){
        const enemy* en = &(td.enemies[num]);
        int uid = en->type_uuid;
        std::string o = "";
        o += "enemy at position " + std::to_string(num) + ": " + en->name + " (type id: " + std::to_string(uid) + "):\n";
        o += "hp: " GRN + en->hp.get_str(10) + " / " + en->base_hp.get_str(10) + BLK " hp.";
        if (en->hp < 0){
            o+= " (dead)";
        }
        o+="\n";
        //o += "xp level: " + enemy_types[uid].xp_minimum.get_str(10) + "\n";
        if (verbosity > 0){
            //o += "base xp drop: " + enemy_types[uid].base_xp_drop.get_str(10) + " xp.\n";
            if (en->ai_type == 1){
                o += "warning: can see the future.\n";
            } else if (en -> ai_type == 2){
                o += "warning: has taken a quarter or two of calculus.\n";
            }
            if (verbosity > 1){
                o += en->dice.describe_enemy_equipped(verbosity-3);
                // dice collection
            }
        }
        return o;
    } else {
        return "No enemy by this id.\n";
    }
}

void loot_enemy(base_data & gd,int pos,temp_data & td, player_data & pd, const world_data & wd, bool & dice_gain_flag, bool & cod_gain_flag){ // for now
    bool boss = !td.continue_here();
    const enemy* en_ptr = &(td.enemies[pos]);
    const enemy_type* typ_ptr = &(wd.enemy_types[en_ptr->type_uuid]);
    pd.xp += typ_ptr->base_xp_drop;
    int_large drop_tier = (typ_ptr->xp_minimum + floor_xp_level(td.site_ptr->xp_min, td.site_ptr->xp_ramping, td.current_floor.height));
    if (!boss) {drop_tier /= 2;}
    int_large held_coins = random_il_below(gd.rng, trunc_root(drop_tier,2)+5);
    pd.coins += held_coins;
    if (pd.dice.owned.size()<pd.inventory_slots && (boss || gd.rng()%DICE_CHANCE_RECIPROCAL == 0)){ // 1/DCR chance of die dropping; bosses guarantee a drop
        int select_die = gd.rng()%(en_ptr->dice.owned.size());
        pd.dice.owned.push_back(en_ptr->dice.owned[select_die]);
        pd.dice.owned[pd.dice.owned.size()-1].rolled = 0;
        dice_gain_flag = 1;
    } else {
        dice_gain_flag = 0;
    }
    if (typ_ptr->base_xp_drop > 40){
        double prob;
        if (typ_ptr->base_xp_drop >= DBL_MAX){
            prob = 0.3;
        } else {
            prob = 0.3/(1+5.0/sqrt(typ_ptr->base_xp_drop.get_d()-39));
        }
        if ((gd.rng() * 1.0) / gd.rng.max() < prob){
            td.consumables["cod charm"]++;
            cod_gain_flag = 1;
        }
    }
}

std::string take_turn(int a_id,base_data & gamedata,health_haver & targ,world_data & wd,temp_data & td){ // output is report of the attack
    td.reset_roll(); // should be already
    enemy* attacker = & (td.enemies[a_id]);
    std::string o;
    //o_primary += attacker->name + " (enemy " + std::to_string(a_id) + ")'s turn:";
    //td.prev_attack_log += attacker->name + " (enemy " + std::to_string(a_id) + ")'s turn:";
    enemy_type* a_typ = & (wd.enemy_types[attacker->type_uuid]);
    bool fail = 0;
    if (a_typ->dice_success_num == a_typ->dice_success_denom){
        o = BBLK "Error: dice cannot fail?!\n" BLK;
        td.prev_attack_log += o;
        return o;
    }
    if (attacker->ai_type == 0){ // 0 : push luck random amount of times determined by greed.
        bool goon = 1;
        int it = 0;
        while(goon && !fail && (it <MAX_ALLOWED_ROLLS)){
            std::vector<int> unr;
            if (!attacker->dice.remaining_unrolled(unr,0)){attacker->dice.unroll();} // condition shouldn't execute but w/e
            int_large roll, rollraw;
            fail = (enemy_roll_one(attacker, unr[gamedata.rng()%unr.size()], gamedata,td)==0);
            goon = attacker->checkgreed(gamedata.rng);
            it++;
        }
        if (!fail){
            td.commit_roll();
            td.apply_damage(targ);
            td.apply_healing(*attacker);
            o = "Dealt " RED + td.damage_pastrolls.get_str(10) + BLK " damage";
            if (td.health_thisroll != 0){
                o+= ", healed by " GRN + td.health_pastrolls.get_str(10) + BLK "health";
            }
            o += ".\n";
        } else {
            o = RED "Failed.\n" BLK;
        }
    }
    else if (attacker->ai_type == 1){ // 1: push luck maximal amount of times by using Magic.
        int it = 0;
        bool fail = 0;
        while (it < MAX_ALLOWED_ROLLS){
            int slot = 0;
            bool exists_g_safe = 0; // really this and the following while loop don't *need* to exist, and the "if !exists g_safe" thing can be taken out of its conditional. it's just that then these can (depending on the order of their dice) pull some horrific rng manipulation stunts. Yes, it's easy to program enemies manipulating rng *too* much.
            while (slot<attacker->dice.equipped.size()){
                if (attacker->dice.owned[attacker->dice.equipped[slot]].guaranteed_safe() && !attacker->dice.owned[attacker->dice.equipped[slot]].rolled) {exists_g_safe = 1; break;}
                else {slot++;}
            }
            if (!exists_g_safe){
                slot = 0;
                while (slot<attacker->dice.equipped.size()){
                    if (attacker->dice.owned[attacker->dice.equipped[slot]].safe_check(gamedata.rng)) {break;}
                    else {slot++;}
                }
            }
            if (slot >= attacker->dice.equipped.size()){break;} else {
                if (enemy_roll_one(attacker, slot, gamedata,td)!= 1){
                    fail = 1;
                }
            }
        }
        if (!fail){
            td.commit_roll();
            td.apply_damage(targ);
            td.apply_healing(*attacker);
            o = "Dealt " RED + td.damage_pastrolls.get_str(10) + BLK " damage";
            if (td.health_thisroll != 0){
                o+= ", healed by " GRN + td.health_pastrolls.get_str(10) + BLK "health";
            }
            o += ".\n";
        } else {
            o = BBLK "Failed; something is wrong.\n" BLK;
        }
        gamedata.rng.discard(1); // otherwise whoever goes next instantly fails under certain circumstances
    }
    else if (attacker->ai_type == 2){ // 2 : calculate chance p of success rolling all dice; roll floor(-1/ln(p)) times and then roll random dice per greed
        double p = (a_typ->dice_success_num*1.0)/(a_typ->dice_success_denom);
        double q = (-1.0)/log(p);
        if (q > MAX_ALLOWED_ROLLS-1){
            q = MAX_ALLOWED_ROLLS-1;
        }
        int roll_num = (int)q;
        for (int i = 0; i < roll_num; i++){
            fail = (enemy_roll_all(attacker, gamedata,td)==0);
            if (fail){
                break;
            }
        }
        if (!fail){
            for (int i = 0; i<attacker->dice.equipped.size(); i++){
                if (attacker->checkgreed(gamedata.rng)){
                    int_large roll, rollraw;
                    fail = (enemy_roll_one(attacker, i, gamedata,td)==0);
                    if (fail){
                        break;
                    }
                }
            }
        }
        if (!fail){
            td.commit_roll();
            td.apply_damage(targ);
            td.apply_healing(*attacker);
            o = "Dealt " RED + td.damage_pastrolls.get_str(10) + BLK " damage";
            if (td.health_thisroll != 0){
                o+= ", healed by " GRN + td.health_pastrolls.get_str(10) + BLK "health";
            }
            o += ".\n";
        } else {
            o = RED "Failed.\n" BLK;
        }
    }
    else {
        o = RED "Unknown AI type. Sat in a corner and cried instead of doing anything.\n" BLK;
        td.prev_attack_log += o;
        return o;
    }
    td.prev_attack_log += o;
    return o;
}

int enemy_roll_one(enemy* attacker,int slot, base_data & gamedata,temp_data & td){ // 0 is fail, 1 is suc, 2 is already rolled, 3 is oob.
    int_large roll, rollraw;
    std::string mod;
    int suc = attacker->dice.roll_modify_equipped(slot, gamedata.rng, roll,rollraw,mod); // returns 0 on fail, 1 on success, 2 on die already rolled, 3 on out of bounds
    die* roller = & attacker->dice.owned[attacker->dice.equipped[slot]];
    td.prev_attack_log += roller->description + ": rolled a " + rollraw.get_str(10);
    if (suc == 0){
        td.prev_attack_log += ", " RED "failing.\n" BLK;
        td.reset_roll();
    } else if (suc == 1) {
        int tup = td.resolve_roll(*roller, roll, rollraw, mod);
        if (tup > 0){
            td.prev_attack_log += " (x" + std::to_string(tup+1) + " from tuples)";
        }
        td.prev_attack_log += ".\n";
        // check if all dice have been rolled, in which case commit current roll and unroll:
        std::vector<int> garb;
        bool b = attacker->dice.remaining_unrolled(garb,1);
        if (!b){
            td.commit_roll();
            attacker->dice.unroll();
        }
    }
    return suc;
}

bool enemy_roll_all(enemy* attacker,base_data & gamedata,temp_data & td){ // relevant funcs are safe enough that if the die is already rolled, nothing happens, so it's entirely safe to use this even if some dice have already been rolled
    bool fail = 0;
    for (int i = 0; (!fail) && i < attacker->dice.equipped.size(); i++){
        std::string mod;
        die* roller = & attacker->dice.owned[attacker->dice.equipped[i]];
        int_large roll;
        int_large rollraw;
        td.prev_attack_log += roller->description + ": rolled a " + rollraw.get_str(10);
        int suc = attacker->dice.roll_modify_equipped(i, gamedata.rng, roll,rollraw, mod);
        if (suc == 0){
            td.prev_attack_log += ", " RED "failing.\n" BLK;
            fail = 1;
            break;
        } else if (suc == 1) {
            int tup = td.resolve_roll(*roller, roll, rollraw, mod);
            if (tup > 0){
                td.prev_attack_log += " (x" + std::to_string(tup+1) + " from tuples)";
            }
            td.prev_attack_log += ".\n";
        }
    }
    if (fail){
        td.reset_roll();
        return 0;
    } else { // all dice rolled
        td.commit_roll();
        attacker->dice.unroll();
        return 1;
    }
}

void get_boss(std::mt19937 & rng, temp_data & td, world_data & wd){ // initialize enemies of a floor
    td.enemies.clear(); // should be already, but just to be safe.
    int floor = td.current_floor.calc_equiv_floor();
    int_large xplev = floor_xp_level(td.site_ptr->xp_min,td.site_ptr->xp_ramping, floor+3);
    std::string path = td.current_floor.curr_path();
    if (td.site_ptr->boss_uuids.find(path) == td.site_ptr->boss_uuids.end()) {
            // no boss currently -> create new boss
        td.site_ptr->boss_uuids[path] = wd.generate_boss(xplev, td.selected_site);
    }
    int uid = td.site_ptr->boss_uuids.at(path);
    td.enemies.push_back(enemy());
    td.enemies[td.enemies.size()-1].set_type(wd.enemy_types[uid], uid);
    
    // flags
    td.newfloor = 0;
    td.player_turn = 1;
    td.input_desired = 1;
}
