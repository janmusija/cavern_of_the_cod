//
//  dice.cpp
//  ominous dice torment game
//
//  Created by jan musija on 9/13/25.
//

#include "dice.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>

die::die(){ // creates a 6-sided attack die
    rolled = 0;
    face_count = 6;
    faces = {1,2,3,4,5,6};
    face_modifiers = {"","","","","",""};
    description = "attack die";
    type = "a";
    failure_threshold = 1; // fails on a 1
    multiplier = 1;
    lowest_face = 1;
}

bool die::rectify_stats(){
    face_count = (int)faces.size();
    if (face_count > 0){
        lowest_face = faces[0];
        for (int i = 1; i<face_count; i++){
            if (lowest_face > faces[i]){
                lowest_face = faces[i];
            }
        }
    } else {
        lowest_face = failure_threshold + 1;
    }
    if (face_modifiers.size() != face_count){ // modifier/face count mismatch. destroys extraneous modifiers/adds blank ones
        std::cout << "die-related error!! trying to fix modifiers.\n";
        face_modifiers.resize(face_count,"");
        return 0;
    }
    return 1;
}

bool die::guaranteed_safe() const{ // you should probably make sure that lowest_face IS the lowest, via rectify_stats or sth, first
    return (lowest_face > failure_threshold);
}

bool die::roll_const(std::mt19937& rng, int_large & val, std::string & output_mod) const{ // roll the die, return a value and the output type. also, bool is whether the roll succeeds.
    if (face_count == 0){
        rng.discard(multiplier); // yes, this IS weird. you're correct.
        val = 0;
        output_mod = "";
        return 1;
    }
    int index = rng() % face_count;
    val = faces[index];
    output_mod = face_modifiers[index];
    if (val <= failure_threshold) {return 0;}
    return 1;
}

bool die::roll(std::mt19937& rng, int_large & val, std::string & output_mod){
    rolled = 1;
    return roll_const(rng,val,output_mod);
}

const int MAX_REROLL_DEPTH = 3;
bool modify_roll(const int_large & roll, const int_large & mult, std::string mod, std::mt19937& rng, int_large & out, const die* rerolltarget, int rerolldepth){ // max reroll depth = 3
    out = mult * roll;
    if (mod == ""){
        return 1;
    }
    std::stringstream ss;
    ss << mod;
    std::string act;
    while(std::getline(ss,act,' ')){
        //std:: cout << act<< "\n";
        if (act == "+"){
            std::string amt_s;
            std::getline(ss,amt_s,' ');
            out += std::atoi(amt_s.c_str());
        }
        if (act == "x"){
            std::string amt_s;
            std::getline(ss,amt_s,' ');
            out *= std::atoi(amt_s.c_str());
        }
        if (act == "r" && rerolldepth < MAX_REROLL_DEPTH && rerolltarget != nullptr){ // this isn't op at all yes
            int_large val;
            std::string modif;
            bool s = (rerolltarget -> roll_const(rng, val, modif));
            if (!s) {return 0;} else {
                int_large val2;
                modify_roll(val,rerolltarget->multiplier,modif, rng, val2, rerolltarget, rerolldepth+1);
            }
        }
    }
    return 1;
} // example modifier : "+ 7 x 3 r x 3" -> add 7, multiply by 3, reroll [this die, implicitly], multiply by 3.
// the main utility of rerolling is to chain many rolls of a single safe die

std::string die::verbose_description(int flavor_lev) const{
    std::string o;
    if (flavor_lev >= 0){
        o += "This is a(n) " + description + " (fundamentally: ";
        {
            if (type == "a"){
                o+= "attack";
            } else if (type == "m"){
                o+= "multiplier";
            } else if (type == "h"){
                o+= "health";
            } else if (type == "hm"){
                o+= "health multiplier";
            } else {
                o += "UNKNOWN";
            }
        }
        o += ") with " + std::to_string(face_count) + " faces, which are:\n";
        int fails = 0;
        for (int i = 0; i<face_count; i++){
            if (faces[i] <= failure_threshold) {
                fails++;
            }
            if (face_modifiers[i] == "") {
                o += std::to_string(faces[i]);
                if (i+1 < face_count ) o += ", ";
            } else {
                o += std::to_string(faces[i]) + " " + face_modifiers[i];
                if (i+1 < face_count ) o += ", ";
            }
        }
        if (face_count != 0){
            float failchance = ((fails*1.0)/face_count);
            o += ".\nThe die has a failure threshold of " + std::to_string(failure_threshold) + ", failing on " + std::to_string(failchance*100) + "% of the faces.\n";
        } else {
            o += ".\nThis means instead of doing anything when rolled it steps the random number generator by its multiplier.\n";
        }
        if ((multiplier!= 1) || (face_count == 0)){
            o += "It has a multiplier of " + std::to_string(multiplier) + ".\n";
        }
        if (flavor_lev >= 1){
            o += flavor();
        }
        if (rolled){
            o+= "It has been rolled.\n";
        }
    } else {
        o += "This is a(n) " + description + ". It has " + std::to_string(face_count) + " faces";
        if (guaranteed_safe()){
            o += " and never fails";
        }
        if (rolled){
            o+= ". It has been rolled";
        }
        o += ".\n";
    }
    return o;
}

std::string die::flavor() const{
    if (face_count  == 0) {
        return "It quivers vacuously. ";
    }
    std::string o;
    if (guaranteed_safe()) {
        o += "It has a calming presence. ";
    }
    if (multiplier >= 3) {
        o += "It feels hot to the touch. ";
    }
    {
        if (type == "m") {
            o += "Your nearby attack dice seem to quiver. ";
        }
        if (type == "h") {
            o += "It smells of soap.";
        }
        if (type == "hm") {
            o += "You feel clean.";
        }
    }
    /* // deprecating for now
    int successcount = 0;
    int_large tally = 0;
    std::mt19937 rng((int)time(nullptr));
    const int FLAVOR_ROLLS = 7; // fewer is faster ig
    for(int i = 0; i<FLAVOR_ROLLS; i++){
        int_large t = 0;
        std::string s;
        if (roll_const(rng,t,s)){
            successcount++;
            modify_roll(t,s,rng,t,this,0);
            tally+=t;
        };
    }
    //std::cout << tally;
    if (successcount == 0) {
        return "You do not want to touch this die. ";
    } else if (successcount < FLAVOR_ROLLS/4) {
        o += "You feel stressed by the die. ";
    } else if (successcount < FLAVOR_ROLLS/2) {
        o += "The air around the die feels tense. ";
    }
    if (tally >= 1000000) {
        return "The gods fear this die. ";
    } else if (tally >= 1000) {
        o += "The die radiates hatred. ";
    } else if (tally >= 100 ){
        o += "The die looks sharp. ";
    }*/
    if (o.length() == 0){
        return "There is nothing exceptional about this die.";
    }
    return o;
}

std::string die::die_data() const {
    /* type;multiplier;threshold;faces separated by commas;description
     for example:
     a;1;1;1 ,2 ,3 ,4 ,5 ,6 ;attack die
     */
    std::string o;
    o+= type;
    o+= ";";
    o+= std::to_string(multiplier);
    o+= ";";
    o+= std::to_string(failure_threshold);
    o+= ";";
    for (int i = 0; i < face_count; i++){
        o+=std::to_string(faces[i]); o+=" "; o+=face_modifiers[i]; if (i+1<face_count) {o+= ",";}
    }
    o+= ";";
    o+= description;
    return o;
}

die::die(const std::string & data) {
    rolled = 0;
    faces = {};
    face_modifiers = {};
    face_count = 0;
    int i = 0;
    while (data[i] != ';'){
        i++;
    }
    type = data.substr(0,i);
    i++; // step past the ;
    std::string mstring = "";
    while (data[i] != ';'){
        mstring += data[i];
        i++;
    }
    multiplier = std::atoi(mstring.c_str());
    i++;// now at the beginning of the threshold
    std::string thstring = "";
    while (data[i] != ';'){
        thstring += data[i];
        i++;
    }
    failure_threshold = std::atoi(thstring.c_str());
    i++;// now at the beginning of the faces
    std::string fstring;
    bool val = 1; // value or modifier
    while (i< data.length()){
        if (data[i] == ',') {
            face_modifiers.push_back(fstring);
            fstring = "";
            i++;
            val = 1;
        } else if (val && data[i] == ' ') {
            face_count++;
            faces.push_back(std::atoi(fstring.c_str()));
            fstring = "";
            i++;
            val = 0;
        } else if (data[i] == ';'){
            face_modifiers.push_back(fstring);
            fstring = "";
            break;
        } else {
            fstring += data[i];
            i++;
        }
    }
    i++;
    description = data.substr(i, data.length()-i);
    if (face_count == 0){
        lowest_face = failure_threshold +1;
    } else {
        lowest_face = faces[0];
        for (int i = 1; i<face_count; i++){
            if (lowest_face > faces[i]){
                lowest_face = faces[i];
            }
        }
    }
}

dice_collection::dice_collection(const std::string & data){ // constructs collection from data string.
    /* format:
     equipped die pointers
     die
     die
     die
     die
     */
    owned = {};
    equipped = {};
    std::stringstream ss;
    ss << data;
    {
        std::string ptrvec;
        std::string nums;
        std::getline(ss,ptrvec,'\n');
        std::stringstream sss;
        sss << ptrvec;
        while (std::getline(sss,nums,' ')){
            equipped.push_back(std::atoi(nums.c_str()));
        }
    }
    std::string death;
    while(std::getline(ss,death,'\n')){
        owned.push_back(die(death));
    }
}
std::string dice_collection::collection_data() const{// returns info about collection in string form
    /* format:
     equipped die pointers
     die
     die
     die
     die
     */
    std::string o;
    for (int i = 0; i< equipped.size(); i++){
        o+= std::to_string(equipped[i]);
        if (i+1 < equipped.size()){
            o+=" ";
        }
    }
    o+= "\n";
    for (int i = 0; i< owned.size(); i++){
        o+= owned[i].die_data();
        if (i+1 < owned.size()){
            o+="\n";
        }
    }
    return o;
}

dice_collection::dice_collection(){ // constructs empty collection
    owned = {};
    equipped = {};
}

void dice_collection::push_new_die_from_string(std::string data, bool equip){
    die d = die(data);
    owned.push_back(d);
    if (equip){
        equipped.push_back((int)owned.size()-1);
    }
}

bool dice_collection::equips_valid(int skip){
    for (int i = 0; i<equipped.size(); i++){
        if (i == skip){
            continue;
        } else {
            if (!owned[equipped[i]].guaranteed_safe()){
                return 1; // risky die exists
            }
        }
    }
    return 0; // all dice safe
}

std::string dice_collection::describe_enemy_equipped(int flavor_lev) const{
    if (equipped.size() > 0){
        std::string o = "this enemy has the following dice:\n";
        for (int i = 0; i<equipped.size(); i++){
            o+= "slot " + std::to_string(i) + ": " + owned[equipped[i]].verbose_description(flavor_lev) + "\n";
        }
        return o;
    } else {
        return "this enemy has no dice.\n"; // should never happen.
    }
}

void die::success_chance(unsigned long long &num, unsigned long long &denom) const{
    denom = (int)faces.size();
    num = 0;
    if (denom > 0){
        for (int i = 1; i<denom; i++){
            if (faces[i] > failure_threshold){
                num++;
            }
        }
    } else {
        num = 1; denom = 1;
    }
}

void dice_collection::equip_success_chance(unsigned long long &num, unsigned long long &denom) const{
    num = 1; denom = 1;
    for (int i = 0; i<equipped.size(); i++){
        unsigned long long tempn, tempd;
        owned[equipped[i]].success_chance(tempn, tempd);
        num *= tempn; denom *= tempd;
    }
}

// deprecate below eventually
#include <fstream>
void dice_collection::select_rudimentary(std::mt19937 & rng, const int_large & target_dam, int_large&under_budg){ // create a collection from the placeholder params_simple/dice_types_rudimentary.txt
    std::ifstream g;
    g.open("params_simple/dice_types_rudimentary.txt");
    std::vector<int_large> dam_exp;
    std::vector<die> ass_dice; // stands for "associated".
    dam_exp.push_back(int_large(3));
    ass_dice.push_back(die("a;1;1;1 ,2 ,3 ,4 ,5 ,6 ;attack die")); // hardcoded
    std::string str;
    while (std::getline(g,str,'\n')){
        int_large il = int_large(str);
        std::getline(g,str,'\n');
        die d = die(str);
        if (d.type == "a" or d.type == "m"){ // for now no h or hm
            dam_exp.push_back(il);
            ass_dice.push_back(d);
        }
    }
    std::vector<int> risk_dice;
    for (int i = 0; i<ass_dice.size(); i++){
        if(!(ass_dice[i].guaranteed_safe())){
            risk_dice.push_back(i);
        }
    }
    std::vector<int> cand;
    cand.push_back(risk_dice[rng()%risk_dice.size()]);
    int_large damage = dam_exp[cand[0]];
    if (damage > target_dam){
        cand.pop_back();
        cand.push_back(0);
    }
    while(cand.size() < 7){
        damage = 0;
        cand.push_back(rng()%ass_dice.size());
        int_large mul = 1;
        int_large mul_denom = 1;
        for (int i = 0; i<cand.size(); i++){
            if (ass_dice[cand[i]].type == "a") {
                damage += dam_exp[cand[i]];
            } else if (ass_dice[cand[i]].type == "m") {
                mul *= dam_exp[cand[i]];
                mul_denom *= 2;
            }
        }
        damage *= mul;
        damage /= mul_denom;
        if (damage > target_dam){
            cand.pop_back();
            break;
        }
    }
    for (int i = 0; i<cand.size(); i++){
        owned.push_back(ass_dice[cand[i]]);
        equipped.push_back(i);
    }
    under_budg = target_dam - damage;
}

std::string dice_collection::view_all(int flavor_lev){
    std::string o;
    for (int i = 0; i<owned.size(); i++){
        o += "slot " + std::to_string(i) + ": ";
        o += owned[i].verbose_description(flavor_lev);
        //o += "\n";
    }
    return o;
}
std::string dice_collection::view_equips(int flavor_lev){
    std::string o;
    for (int i = 0; i<equipped.size(); i++){
        o += "slot " + std::to_string(equipped[i]) + " (equip slot " + std::to_string(i) + "): ";
        o += owned[equipped[i]].verbose_description(flavor_lev);
        //o += "\n";
    }
    return o;
}
std::string dice_collection::view_index(int slot, int flavor_lev, int islots){
    std::string o;
    if (slot < owned.size() && slot >= 0){
        o+= owned[slot].verbose_description(flavor_lev);
        o+= "\n";
    } else if (slot < islots && slot >=0) {
        o+= "You ponder an empty slot.\n";
    } else {
        o+= "This slot does not exist.\n";
    }
    return o;
}

void dice_collection::unroll(bool fullinv){
    if (fullinv){
        for (int i = 0; i<owned.size(); i++){
            owned[i].rolled = 0;
        }
    } else {
        for (int i = 0; i<equipped.size(); i++){
            owned[equipped[i]].rolled = 0;
        }
    }
}

bool dice_collection::remaining_unrolled(std::vector<int> & indices, bool indices_wrong_ok){
    indices.clear();
    bool b = 0;
    for (int i = 0; i<equipped.size(); i++){
        if (!owned[equipped[i]].rolled){
            b = 1;
            indices.push_back(i);
            if (indices_wrong_ok){
                break;
            }
        }
    }
    return b;
}

int dice_collection::roll_equipped(int equip_slot, std::mt19937& rng, int_large & val, std::string & output_mod){ // returns 0 on fail, 1 on success, 2 on die already rolled, 3 on out of bounds
    if (equip_slot < 0 || equip_slot >= equipped.size()){
        return 3;
    } else if (owned[equipped[equip_slot]].rolled) {
        return 2;
    } else {
        return owned[equipped[equip_slot]].roll(rng, val, output_mod);
    }
}
int dice_collection::modify_equipped(const int_large & roll, std::string mod, std::mt19937& rng, int_large & output, int equip_slot, int rerolldepth){ // returns 0 on fail, 1 on success, 3 on out of bounds
    if (equip_slot < 0 || equip_slot >= equipped.size()){
        return 3;
    } else {
        return modify_roll(roll, owned[equipped[equip_slot]].multiplier, mod, rng, output, & owned[equipped[equip_slot]]);
    }
}
int dice_collection::roll_modify_equipped(int equip_slot, std::mt19937& rng, int_large & output, int_large & val, std::string & output_mod){ // returns 0 on fail, 1 on success, 2 on die already rolled, 3 on out of bounds
    val = 0;
    int g = roll_equipped(equip_slot, rng, val, output_mod);
    if (g != 1){
        return g;
    } else {
        return modify_equipped(val, output_mod, rng, output, equip_slot);
    }
}

bool die::safe_check(const std::mt19937 &rng) const{ // if you wanted it would not be hard to adapt this to also return the total roll.
    std::mt19937 rngcopy = rng;
    if (rolled) { // easier to say already rolled dice are unsafe, and aligns better for purposes.
        return 0;
    } else {
        if (guaranteed_safe()){
            return 1;
        }
        int_large val;
        std::string output_mod;
        if (roll_const(rngcopy, val, output_mod)){
            int_large garb;
            return modify_roll(val, multiplier, output_mod, rngcopy, garb, this);
        } else {
            return 0;
        }
    }
}
