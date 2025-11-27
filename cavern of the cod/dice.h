//
//  dice.h
//  ominous dice torment game
//
//  Created by jan musija on 9/13/25.
//

#ifndef dice_h
#define dice_h

#include <stdio.h>
#include <vector>
#include <string>
#include <random>
#include "int_large.h"

class die {
public:
    bool rolled; // has the die been rolled yet this turn?
    int face_count; // should always be faces.size();
    std::vector<int> faces; // the values on the die
    std::vector<std::string> face_modifiers; // the modifiers associated to each of these faces. These
    std::string description; // describes the type of the die
    std::string type; // die's output type. valid types so far: "a" (attack), "m" (attack multiplier)
    std::string die_data() const; // returns info about the die in string form
    bool roll_const(std::mt19937& rng, int_large & val, std::string & output_mod) const; // roll the die, return a value and the output modifier. also, bool is whether the roll succeeds.
    die(const std::string & data); // constructs die from data string
    int failure_threshold; // highest rolled value that fails
    int lowest_face; // to check if the die is safe
    int multiplier; // value multipliers
    die(); // construct default d6 attack die
    std::string verbose_description(int flavor_lev = 0) const; // describe the die in more detail. flavor levels: -1: brief; 0: default; 1: with flavor text
    std::string flavor() const;
    bool rectify_stats(); // make sure face_count, faces, and lowest_face are in accord with each other. returns 0 and destructively changes die if an unfixable problem is present, otherwise 1.
    bool guaranteed_safe() const; // at least one die you have equipped must not be guaranteed safe.
    void success_chance(unsigned long long &num, unsigned long long &denom) const; // determine risk
    bool roll(std::mt19937& rng, int_large & val, std::string & output_mod); // not const -- changes rolled()
    bool safe_check(const std::mt19937 &rng) const; // for Prophesy
};

bool modify_roll(const int_large & roll, const int_large & mult, std::string mod, std::mt19937& rng, int_large & ou, const die* rerolltarget, int rerolldepth = 3); // input value, modifier, rng, output value, target for rerolls die (ie this die probably), current depth of reroll. // pass nullptr if you don't need any rerolling

class dice_collection {
public:
    std::vector<die> owned;
    std::vector<int> equipped; // positions of equipped dice in owned tuple
    dice_collection(const std::string & data); // constructs collection from data string.
    std::string collection_data() const; // returns info about collection in string form
    dice_collection(); // initialize empty collection
    void push_new_die_from_string(std::string data, bool equip = 0); // adds a new die, which may be immediately equipped
    bool equips_valid(int skip = -1); // checks if there is  die which fails in this assortment, skipping die at position skip
    std::string describe_enemy_equipped(int flavor_lev = -1) const; // all that matters for enemies
    void equip_success_chance(unsigned long long &num, unsigned long long &denom) const; //chance of all equipped dice being rolled without a failure
    void select_rudimentary(std::mt19937 & rng, const int_large & target_dam, int_large & under_budg); // create a collection from the placeholder params_simple/dice_types_rudimentary.txt
    std::string view_all(int flavor_lev = 0);
    std::string view_equips(int flavor_lev = 0);
    std::string view_index(int slot, int flavor_lev = 1, int inv_size = 0); // inv size determines number of blank slots
    void unroll(bool fullinv = 0); // un-roll all equipped dice
    bool remaining_unrolled(std::vector<int> & indices, bool indices_wrong_ok = 0); // determine if any die are unrolled; 1 if yes. vector lists equip slots; indices_wrong_ok is whether it's okay to just immediately quit after determining the boolean
    int roll_equipped(int equip_slot,  std::mt19937& rng, int_large & val, std::string & output_mod); // returns 0 on fail, 1 on success, 2 on die already rolled, 3 on out of bounds
    int modify_equipped(const int_large & roll, std::string mod, std::mt19937& rng, int_large & output, int equip_slot, int rerolldepth = 3); // returns 0 on fail, 1 on success, 3 on out of bounds
    int roll_modify_equipped(int equip_slot, std::mt19937& rng, int_large & output, int_large & rollraw, std::string & output_mod); // returns 0 on fail, 1 on success, 2 on die already rolled, 3 on out of bounds
};



#endif /* dice_h */
