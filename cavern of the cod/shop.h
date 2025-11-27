//
//  shop.h
//  cavern of the cod
//
//  Created by jan musija on 9/21/25.
//

#ifndef shop_h
#define shop_h
#define SHOP_REROLL_COST 100
#define DICE_COST_MULT 100

#include <stdio.h>
#include <random>
#include "dice.h"
#include "base_info.h"
#include "world_gen.h"

class shop_cache { // not saved!!
public:
    std::mt19937 rng;
    bool init;
    die shopdie; // me shouting this screaming and crying
    int_large shopdie_cost;
    shop_cache();
    void refresh();
};

std::string display_warez(const player_data & pd, const world_data & wd, shop_cache & sc);

void calc_hp(const player_data & pd, int_large & hp_amount, int_large & hp_cost);

bool buy_hp_upgr(player_data & pd, int_large & amount_gained);

//deprecate eventually
die generate_shopdie_rudimentary(std::mt19937 & temp_rng, int_large & cost, int_large & cost_ceil);

#endif /* shop_h */
