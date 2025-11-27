//
//  combat.h
//  ominous dice torment game
//
//  Created by jan musija on 9/15/25.
//

#ifndef combat_h
#define combat_h

#include <stdio.h>
#include "dice.h"
#include "int_large.h"
#include "base_info.h"
#include "enemy.h"
#include "world_gen.h"

void get_enemies(std::mt19937 & rng, temp_data & td, world_data & wd);
void get_boss(std::mt19937 & rng, temp_data & td, world_data & wd);

std::string display_combatants(const temp_data & td, const world_data & wd);

std::string view_combatant(const temp_data & td, const world_data & wd, int num, int detail = 0);

#define DICE_CHANCE_RECIPROCAL 20
void loot_enemy(base_data & gd,int pos,temp_data & td, player_data & pd, const world_data & wd, bool & dice_gain_flag, bool & codflag);

int enemy_roll_one(enemy* attacker,int slot, base_data & gamedata,temp_data & td);

bool enemy_roll_all(enemy* attacker,base_data & gamedata,temp_data & td);

#define MAX_ALLOWED_ROLLS 1000
// for performance reasons in Edge Cases. This refers to enemies.
std::string take_turn(int a_id,base_data & gamedata,health_haver & targ,world_data & wd,temp_data & td);

#endif /* combat_h */
