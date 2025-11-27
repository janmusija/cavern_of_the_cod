//
//  world_gen.cpp
//  ominous dice torment game
//
//  Created by jan musija on 9/13/25.
//

#include "base_info.h"
#include "world_gen.h"
#include <fstream>
#include "enemy.h"
#include <algorithm>
#include <gmp.h>
#include <gmpxx.h>
#include <sstream>
#include <functional> // hash
#include "stream_colors.h"

world_data::world_data(){
    sites = {};
    enemy_types = {};
    std::mt19937 defaul(0);
    site_rng = defaul;
    shop_rng = defaul;
    base_gram = grammar_bank();
    //base_gram.open("params_simple/words","params_simple/enemy.txt","params_simple/site.txt");
    //base_gram.open("params_simple/words","params_simple/subs.txt");
    base_gram.open("params_simple/dict.txt","params_simple/subs.txt");
}

temp_data::temp_data(){ // still needs replacing, to set the seed to the seed
    seed = 0;
    reset();
}

unsigned long world_data::add_site(int_large xp_min){// output is the new site's position in the site vector
    site_rng.discard((xp_min.get_si())%137); // so that sites of different xp levels generate differentously.
    site newsite (xp_min,site_rng,base_gram,grammar_variation_rng);
    site_rng.discard(3); //skip forward a bit because of the way this works to avoid duplicating the same
    sites.push_back(newsite);
    unsigned long s_ind = sites.size()-1; // this site's index
    fill_floor(0, (int)s_ind, 1);
    return s_ind;
}

#define SITE_CHARACTER 8
site::site(const int_large & xp_lvl, std::mt19937 & wg_rng, const grammar_bank & base_gram, std::mt19937 & grammar_variation_rng){
    std::mt19937 new_rng(wg_rng());
    new_rng.discard(wg_rng()%256); // just to get to  random spot
    enemy_rng = new_rng;
    loc_grammar = base_gram;
    loc_grammar.randomly_bias(grammar_variation_rng);
    unsigned long such = (xp_lvl.get_str(10).length())/2;
    if (such > MAX_NAME_BUDGET){such = MAX_NAME_BUDGET;}
    name = loc_grammar.get_site_name(enemy_rng,(int)such);
    highest_floor = 0;
    xp_min = xp_lvl;
    enemy_uuids = {};
    xp_ramping =  (xp_min/10)+1; // idk, I'll tweak this. keeping it like this for now
}

#include <cmath>
long long floor_xp_level_ll(long long min_xp, long long xp_ramping, int floor){ // currently written under the assumption int_large = long long
    double out = min_xp + floor;
    out+= pow((floor+2)/3.0,2.5) * 0.25;
    if (floor >=31){
        out*= pow(floor/28.0,floor-31);
    }
    if (out >= LONG_LONG_MAX){ // I think if your long long max is 2^63-1 this happens at floor 70ish. I don't... ANTICIPATE players being able to construct a build that does this, but I think it isn't too hard to cheat in a die that could deal enough damage to dispatch foes up to this floor. Speaking of which, I might have to implement bigints and change xp and health values to use bigints
        return LONG_LONG_MAX; // if you have quintillions of xp somehow, screaming.
    }
    long long o = out;
    if (o<0){o=0;}
    return o;
}

int_large floor_xp_level(const int_large & min_xp, const int_large & xp_ramping, int floor){
    mpz_class out(floor*floor);
    mpz_class mult(floor+10);
    mpz_mul(out.get_mpz_t(),out.get_mpz_t(),mult.get_mpz_t());
    if (floor > 30){
        int_large exp = pow_ui(2, floor-26);
        mpz_add(out.get_mpz_t(),out.get_mpz_t(),exp.get_mpz_t());
    }
    out *= xp_ramping;
    out /= 15;
    out += min_xp + (floor + 1);
    if (out<0){out=0;}
    return out;
}//

void xp_interval(const int_large & xp_tier, const int_large & next_xp_tier, int_large & xp_floor, int_large & xp_ceiling){ // heristic for determining valid xp interval
    xp_floor = trunc_root(xp_tier,2)-1;
    xp_ceiling = trunc_root(next_xp_tier,2)+1;
    if (xp_floor < 0) xp_floor = 0;
    xp_floor = (xp_floor * xp_floor) - 10;
    if (xp_floor < 0) xp_floor = 0;
    xp_ceiling = (xp_ceiling * xp_ceiling) + 10;
    //if (xp_ceiling < 0) xp_ceiling = LONG_LONG_MAX; //note long long usage
}
    
int world_data::floor_enemies(int floor, int s_ind, std::vector<int> & out_uuids, bool sortbool){ // number of local enemy types at floor // out_uuids is sorted if sortbool is enabled.
    
    out_uuids = {}; // it should already be empty, but w/e
    
    int_large xp_tier = floor_xp_level(sites[s_ind].xp_min,sites[s_ind].xp_ramping,floor);
    int_large next_xp_tier = floor_xp_level(sites[s_ind].xp_min,sites[s_ind].xp_ramping,floor+1);
    int_large xp_floor;
    int_large xp_ceiling;
    xp_interval(xp_tier,next_xp_tier,xp_floor,xp_ceiling);
    /* // if you need to see these for debug reasons
    std::string xpt = xp_tier.get_str(10);
    std::string nxpt = next_xp_tier.get_str(10);
    std::string xpf = xp_floor.get_str(10);
    std::string xpc = xp_ceiling.get_str(10);
     */
    
    for (int i = 0; i<sites[s_ind].enemy_uuids.size();i++){
        int uuid = sites[s_ind].enemy_uuids[i];
        int_large cand_xp = enemy_types[uuid].xp_minimum;
        if (xp_floor <= cand_xp && cand_xp < xp_ceiling){
            out_uuids.push_back(uuid);
        }
    }
    
    if (sortbool){
        std::sort (out_uuids.begin(), out_uuids.end());
    }
    
    return (int)out_uuids.size();
}
std::string world_data::view_info(int s_ind, int verbosity) const{ // xp min, highest floor reached, known local enemy names and uuids
    if (s_ind < sites.size()){
        std::string o = "SITE: " + sites[s_ind].name + " (id: " + std::to_string(s_ind) + "):\n";
        o += "Recommended xp level: " + (sites[s_ind].xp_min).get_str(10) + "\n";
        o += "Highest floor reached: " + std::to_string(sites[s_ind].highest_floor) + "\n";
        if (verbosity > 0){
            unsigned long num_en = sites[s_ind].enemy_uuids.size();
            o += "There are " + std::to_string(num_en) + " known local enemies";
            if (num_en == 0){
                o += ".\n";
            } else {
                o += ":\n";
                for (int i = 0; i< num_en; i++){
                    int uuid = sites[s_ind].enemy_uuids[i];
                    o+= enemy_types[uuid].name + " (id: " + std::to_string(uuid) + ")\n";
                }
            }
        }
        return o;
    } else {
        return "This is not a known site's id.\n";
    }
}

void world_data::fill_floor(int floor,int s_ind,int extra){ // fill out remaining enemies at this floor level
    std::vector<int> loc_en;
    int missing = MIN_ENEMIES_EACH_FLOOR + extra - floor_enemies(floor,s_ind,loc_en,1);
    if (missing <= 0) {
        return;
    } else {
        std::mt19937 * local_rng = & sites[s_ind].enemy_rng;
        
        int_large xp_tier = floor_xp_level(sites[s_ind].xp_min,sites[s_ind].xp_ramping,floor);
        int_large next_xp_tier = floor_xp_level(sites[s_ind].xp_min,sites[s_ind].xp_ramping,floor+1);
        int_large xp_floor;
        int_large xp_ceiling;
        xp_interval(xp_tier,next_xp_tier,xp_floor,xp_ceiling);
        
        std::vector<int> candidate_uuids;
        
        int j = 0; // loc_en is sorted so that this can be done in O(nlogn) instead of O(n^2). I'm so algorithmic
        for (int i = 0; i<enemy_types.size();i++){
            if (j < loc_en.size() && i == loc_en[j]){ // already present, skip.
                j++;
                continue;
            } else {
                int_large cand_xp = enemy_types[i].xp_minimum;
                if (xp_floor <= cand_xp && cand_xp < xp_ceiling){
                    candidate_uuids.push_back(i);
                }
            }
        }
        
        int target_olds = ((*local_rng)())%(missing+1);
        { // when there are many, many candidates, limit amount you spam out:
            if (std::round(sqrt(candidate_uuids.size())-1.4) > target_olds){
                target_olds = std::round(sqrt(candidate_uuids.size())-1.4);
                if (target_olds > missing){
                    // small chance to set to missing-1 instead:
                    double value = ((*local_rng)()*1.0)/(local_rng->max()); // random number between 0 and 1
                    if (value < 1.0/(1+pow(1.05,candidate_uuids.size()))){
                        target_olds = missing-1;
                    } else {
                        target_olds = missing;
                    }
                }
            }
        }
        
        int olds = 0;
        while (candidate_uuids.size() > 0 && target_olds > 0){
            olds++;
            target_olds--;
            unsigned long pos = ((*local_rng)())%(candidate_uuids.size());
            // random pos in vector
            sites[s_ind].enemy_uuids.push_back(candidate_uuids[pos]);
            candidate_uuids.erase(candidate_uuids.begin() + pos);
        } // add new olds
        missing -= olds;
        // create remaining ones for missing many
        for (int i = 0; i<missing; i++){
            int_large xp_diff = next_xp_tier -xp_tier;
            if (xp_diff<1) {xp_diff = 1;}
            int_large new_enemy_xp_lev = xp_tier+random_il_below(*local_rng,xp_diff);
            std::string xpt = xp_tier.get_str(10);
            std::string nxpl = new_enemy_xp_lev.get_str(10);
            std::string nxpt = next_xp_tier.get_str(10);
            generate_enemy(new_enemy_xp_lev, s_ind);
            std::vector<int> loc_en2;
            floor_enemies(floor,s_ind,loc_en2,1);
        }
    }
}


int world_data::generate_enemy(int_large xp_lvl, int s_ind){
    site* locale = & (sites[s_ind]);
    unsigned long such = xp_lvl.get_str(10).length()-1;
    if (such > MAX_NAME_BUDGET){such = MAX_NAME_BUDGET;}
    std::string name = locale->loc_grammar.get_enemy_name(locale->enemy_rng,(int)such);
    enemy_type new_enemy(locale->enemy_rng, xp_lvl, name);
    enemy_types.push_back(new_enemy);
    int id = (int)(enemy_types.size()-1);
    locale->enemy_uuids.push_back(id);
    return id;
}

int world_data::generate_boss(int_large xp_lvl, int s_ind){
    site* locale = & (sites[s_ind]);
    unsigned long such = xp_lvl.get_str(10).length()+1;
    if (such > MAX_NAME_BUDGET){such = MAX_NAME_BUDGET + 2;} // "max"
    std::string name = locale->loc_grammar.get_boss_name(locale->enemy_rng,(int)such);
    enemy_type new_enemy(locale->enemy_rng, xp_lvl, name); // for now-- TK changes?
    enemy_types.push_back(new_enemy);
    int id = (int)(enemy_types.size()-1);
    locale->enemy_uuids.push_back(id);
    return id;
}

std::string world_data::view_enemy(int e_ind, int verbosity) const{
    if (e_ind < enemy_types.size()){
        std::string o = "";
        o += "ENEMY: " + enemy_types[e_ind].name + " (id: " + std::to_string(e_ind) + "):\n";
        o += "xp level: " + enemy_types[e_ind].xp_minimum.get_str(10) + "\n";
        if (verbosity > 0){
            o += "base hp: " + enemy_types[e_ind].base_hp.get_str(10) + " hp.\n";
            o += "base xp drop: " + enemy_types[e_ind].base_xp_drop.get_str(10) + " xp.\n";
            if (enemy_types[e_ind].ai_type == 1){
                o += "warning: can see the future, flawlessly.\n";
            } else if (enemy_types[e_ind].ai_type == 2){
                o += "warning: has taken a quarter or two of calculus.\n";
            }
            if (verbosity > 1){
                o += enemy_types[e_ind].dice.describe_enemy_equipped(verbosity-3);
                // dice collection
            }
        }
        return o;
    } else {
        return "This is not a known enemy's id.\n";
    }
}

void temp_data::reset(){
    selected_site = 0;
    current_floor.clear();
    consumables.empty();
    initialize_consumables();
    combat = 0;
    player_turn = 1;
    enemies.clear();
    input_desired = 1;
    site_ptr = nullptr;
    newfloor = 1;
    cleared_rooms.clear();
    looted_rooms.clear();
    reset_roll();
    prev_attack_log = "";
    subterranean_floors_entered = 0;
}

bool temp_data::enemies_surviving(std::vector<int>& positions){ // put remaining enemy indices in vector. bool is if there are any
    positions.clear();
    bool b = 0;
    for (int i = 0; i<enemies.size(); i++){
        if (enemies[i].alive()){
            b = 1; positions.push_back(i);
        }
    }
    return b;
}

void temp_data::end_player_turn(player_data & pd){
    reset_roll();
    pd.dice.unroll();
    player_turn=0;
    input_desired = 0;
    prev_attack_log = "";
    end_turn_consumables();
}
void temp_data::end_enemy_turn(){
    reset_roll();
    for (int i = 0; i<enemies.size(); i++){
        if (enemies[i].alive()){
            enemies[i].dice.unroll();
        }
    }
    player_turn = 1;
    input_desired = 1;
}

void temp_data::advance_floor(bool br){
    if (br){
        current_floor.branch();
    } else {
        current_floor.increment();
    }
    end_turn_consumables();
    reset_roll();
    enemies.clear();
    player_turn = 1;
    newfloor = 1;
    input_desired = 0; //0 is not a mistake!! needs to be 0 to get_enemies(...) properly.
}

void temp_data::retreat_floor(){
    end_turn_consumables();
    current_floor.decrement();
    reset_roll();
    enemies.clear();
    input_desired = 1;
    player_turn = 1;
    newfloor = 1;
    input_desired = 0;
}


void temp_data::commit_roll(){// commit thisroll to pastrolls.
    rollcounts.clear();
    damage_pastrolls += damage_thisroll*this_mult;damage_thisroll = 0;this_mult = 1;
    health_pastrolls += health_thisroll*this_health_mult; health_thisroll = 0; this_health_mult = 1;
}
void temp_data::calc_rolls(int_large &dam, int_large &health) const{// calculate current damage
    dam = damage_pastrolls + (damage_thisroll*this_mult);
}
void temp_data::reset_roll(){ //set roll to 0
    rollcounts.clear();
    damage_pastrolls = 0; damage_thisroll = 0; this_mult = 1;
    health_pastrolls = 0; health_thisroll = 0; this_health_mult = 1;
}

void temp_data::apply_damage(health_haver & targ) const{
    targ.hp -= damage_pastrolls;
}
void temp_data::apply_healing(health_haver & targ) const{
    targ.hp += health_pastrolls;
    if (targ.hp > targ.base_hp){ // cap
        targ.hp = targ.base_hp;
    }
}

void world_data::wd_exp(std::string &exp){
    /*
     FORMAT:
     {
     {site1}
     {site2}
     ...
     }
     {
     {enemy1}
     {enemy2}
     ...
     }
     {
     site rng
     }
     {
     shop rng
     }
     {
     grammar
     }
     */
    exp = "";
    // sites
    exp+= "sites:\n{\n";
    {
        for (int i = 0; i<sites.size(); i++){
            exp+= "{\n";
            exp+= sites[i].site_exp();
            exp+= "\n}\n";
        }
    }
    exp+= "}\n";
    // enemy_types
    exp+= "enemies:\n{\n";
    {
        for (int i = 0; i<enemy_types.size(); i++){
            exp+= "{\n";
            exp+= enemy_types[i].type_exp();
            exp+= "\n}\n";
        }
    }
    exp+= "}\n";
    // site rng
    exp+= "site_rng:\n{\n";
    {
        std::stringstream twister;
        twister << site_rng;
        exp += twister.str() + "\n}\n";
    }
    // shop rng
    exp+= "shop_rng:\n{\n";
    {
        std::stringstream twister;
        twister << site_rng;
        exp += twister.str() + "\n}\n";
    }
    // grammar
    exp+= "grammar:\n{\n";
    std::string str;
    base_gram.gb_exp(str);
    exp+= str;
    exp+= "\n}";
}
bool world_data::load(const std::string & data_str, int seed){
    sites = {};
    enemy_types = {};
    unsigned long long pos = 0;
    unsigned long long b = 0; unsigned long long l = 0;
    isolate_matched_brackets(data_str, b, l, pos, 1, '{', '}'); // sites
    {
        unsigned long long pos1, b1, l1;
        std::string dstr2 = data_str.substr(b,l);
        pos1 = 0;
        while(isolate_matched_brackets(dstr2, b1, l1, pos1, 1, '{', '}')){
            std::string str;
            if (b1 > l){ break;}
            str = dstr2.substr(b1,l1);
            pos1 = b1+l1;
            site s(str);
            sites.push_back(s);
        }
    }
    pos = b+l;
    isolate_matched_brackets(data_str, b, l, pos, 1, '{', '}'); // enemies
    {
        unsigned long long pos1, b1, l1;
        std::string dstr2 = data_str.substr(b,l);
        pos1 = 0;
        while(isolate_matched_brackets(dstr2, b1, l1, pos1, 1, '{', '}')){
            std::string str;
            if (b1 > l){ break;}
            str = dstr2.substr(b1,l1);
            pos1 = b1+l1;
            enemy_type en(str);
            enemy_types.push_back(en);
        }
    }
    pos = b+l;
    isolate_matched_brackets(data_str, b, l, pos, 1, '{', '}');
    // site rng
    {
        std::stringstream ss;
        ss << data_str.substr(b,l); // twister state
        ss >> site_rng;
    }
    pos = b+l;
    isolate_matched_brackets(data_str, b, l, pos, 1, '{', '}');
    // shop rng
    {
        std::stringstream ss;
        ss << data_str.substr(b,l); // twister state
        ss >> shop_rng;
    }
    pos = b+l;
    isolate_matched_brackets(data_str, b, l, pos, 1, '{', '}'); // grammar
    grammar_variation_rng = std::mt19937(seed);
    base_gram.load(data_str.substr(b,l));
    for (int i = 0; i<sites.size(); i++){
        sites[i].loc_grammar = base_gram;
        sites[i].loc_grammar.randomly_bias(grammar_variation_rng);
    }
    return 0;
}

site::site(const std::string & str){ // does not initialize grammar!!
    /*
     format:
     name
     xp_min
     xp_ramping
     highest floor reached
     {enemy uuids}
     {boss_uuids, formatted thusly:
     uuid coordinate string
     }
     {enemy rng}
     */
    std::stringstream ss;
    ss << str;
    std::getline(ss,name,'\n'); // name
    std::string s;
    std::getline(ss,s,'\n'); // xp_min
    xp_min = int_large(s);
    std::getline(ss,s,'\n'); // xp_ramping
    xp_ramping = int_large(s);
    std::getline(ss,s,'\n'); // highest_floor
    highest_floor = std::atoi(s.c_str());
    std::string str2 = ss.str();
    
    // enemy uuids
    enemy_uuids.clear();
    unsigned long long b, l, pos; pos = 0;
    isolate_matched_brackets(str2, b, l, pos, 1, '{', '}');
    {
        std::stringstream ss2;
        ss2 << str2.substr(b,l);
        while (std::getline(ss2,s,' ')){
            enemy_uuids.push_back(std::atoi(s.c_str()));
        }
    }
    pos = b+l;
    // boss ids
    isolate_matched_brackets(str2, b, l, pos, 1, '{', '}');
    {
        std::stringstream ss2;
        ss2 << str2.substr(b,l);
        while (std::getline(ss2,s,' ')){
            int uid = std::atoi(s.c_str());
            std::getline(ss2,s,'\n');
            boss_uuids[s] = uid;
        }
    }
    pos = b+l;
    // twister
    isolate_matched_brackets(str2, b, l, pos, 1, '{', '}');
    {
        std::stringstream ss2;
        ss2 << str2.substr(b,l); // twister state
        ss2 >> enemy_rng;
    }
}
std::string site::site_exp() const{
    /*
     format:
     name
     xp_min
     xp_ramping
     highest floor reached
     {enemy uuids}
     {boss_uuids, formatted thusly:
     uuid coordinate string
     }
     {enemy rng}
     */
    std::string o;
    o+= name + "\n";
    o+= xp_min.get_str(10) + "\n";
    o+= xp_ramping.get_str(10) + "\n";
    o+= std::to_string(highest_floor) + "\n{\n";
    for (int i = 0; i<enemy_uuids.size(); i++){
        o+= std::to_string(enemy_uuids[i]); if (i+1 < enemy_uuids.size()) {o+= " ";}
    }
    o+= "\n}\n{\n";
    for (std::map<std::string, int>::const_iterator it = boss_uuids.begin(); it != boss_uuids.end(); it++)
    {
        o+= std::to_string(it->second) + " " + it->first + "\n";
    }
    o+= "\n}\n{\n";
    std::stringstream twister;
    twister << enemy_rng;
    o += twister.str() + "\n}";
    return o;
}

int temp_data::resolve_roll(die & roller, const int_large & roll, const int_large & rollraw, const std::string & mod){
    int ct = 0;
    if (roller.type == "a" or roller.type == "h") { // types eligible for doublesing
        std::string desc = roller.description;
        if (desc == "your first die"){desc = "attack die";}
        std::pair<std::string,std::string> rolled = std::pair(desc,rollraw.get_str(10) + mod);
        if (rollcounts.find(rolled) == rollcounts.end()) {
            rollcounts[rolled]=1;
        } else {
            ct = rollcounts[rolled];
            rollcounts[rolled]++;
        }
    }
    int doublesmult = (ct * 2) + 1;
   if (roller.type == "a"){
       damage_thisroll += roll * doublesmult;
   } else if (roller.type == "m") {
       this_mult += roll;
   } else if (roller.type == "h") {
       health_thisroll += roll * doublesmult;
   } else if (roller.type == "hm") {
       this_health_mult += roll;
   }
    return ct;
}

site_room::site_room(){
    height = 0;
    bpath.clear();
}

void site_room::clear(){
    height = 0;
    bpath.clear();
}

std::string site_room::curr_path() const{
    std::string o;
    for (int i = 0; i<bpath.size(); i++){
        o += std::to_string(bpath[i]) + ":";
    }
    o+=std::to_string(height);
    return o;
}
bool site_room::mainline() const{ // is mainline?
    return (bpath.size() == 0);
}
void site_room::increment(){ // increment room
    height++;
}
void site_room::branch(){ // add new branch
    bpath.push_back(height);
    height++;
}
void site_room::decrement(){ // decrement room
    height--;
    unsigned long bs = bpath.size();
    if (bs>0 && bpath[bs-1] == height){
        bpath.pop_back();
    }
}

std::string temp_data::room_hash() const { // hash for room things
    return std::to_string(selected_site) + std::to_string(site_ptr->xp_min.get_ui()) + std::to_string(seed) + current_floor.curr_path();
}

bool temp_data::branch_here() const{ // determine if the room branches
    std::hash<std::string> hashfun;
    std::string dat = room_hash();
    std::size_t hashed = hashfun(dat);
    float margin = (abs(current_floor.height)*1.0)/(3*abs(current_floor.height)+6);
    return ((hashed*1.0) / HASH_MAX < margin);
}

bool temp_data::continue_here() const{ // determine if the room continues
    if (current_floor.height <= 0) {return 1;}
    if (current_floor.mainline()){return 1;} else {
        if (current_floor.bpath[current_floor.bpath.size()-1] < 0){return 1;}
        std::hash<std::string> hashfun;
        std::string dat = room_hash();
        std::size_t hashed = hashfun(dat);
        float margin = 7.0/10.0; // yes, eventually (circa floor 18) expected branch size becomes unbounded. have fun with your dfs suffering.
        return ((hashed*1.0) / HASH_MAX < margin);
    }
}

int site_room::calc_equiv_floor(){ // progression after n branchings counts n additional times.
    int res;
    if (bpath.size() == 0){
        res= abs(height);
    } else {
        res = abs(bpath[0]);
        for (int i = 1; i<bpath.size(); i++){
            res+= (bpath[i]-bpath[i-1])*(i+1);
        }
        res+= (height - bpath[bpath.size()-1])*(bpath.size()+1);
    }
    if (height < 0 || (bpath.size() > 0 && bpath[0] < 0)){
        res *=2;
        res += 13;
    }
    return res;
}

/*
 --- CONSUMABLES section!! ---
 */

/*
 "first aid kit" -- full heal, when not in combat
 "extra life" -- restores you to 10 hp if you're killed by enemies
 "insurance potion" -- imbues roll_insurance until the end of the round
 "retreat crystal" -- imbues roll_insurance until the end of the round
 "summon crystal" -- imbues roll_insurance until the end of the round
 
 "roll_insurance" -- (status effect) -- prevents a roll failure from ending your turn or voiding the rest of the roll.
 */
void temp_data::initialize_consumables(){ // initializes consumables to default
    end_turn_consumables(); // wipe status effects
    consumables["first aid kit"] = 0;
    consumables["extra life"] = 0;
    consumables["insurance potion"] = 0;
    consumables["retreat crystal"] = 0;
    consumables["summon crystal"] = 0;
    consumables["cod charm"] = 0;
}
void temp_data::initialize_consumables(std::map<std::string,unsigned int> con){ // initializes consumables to states in con
    initialize_consumables();
    std::map<std::string, unsigned int>::iterator it;
    for (it = con.begin(); it != con.end(); it++)
    {
        consumables[it->first] = it->second;
    }
}
void temp_data::end_turn_consumables(){ // ends turn-based status effects
    consumables["roll_insurance"] = 0;
}

std::string temp_data::usable_consumables(){
    std::string o;
    o += "first aid kit: " + std::to_string(consumables["first aid kit"]) + "\n";
    o += "insurance potion: " +  std::to_string(consumables["insurance potion"]) + "\n";
    o += "retreat crystal: " +  std::to_string(consumables["retreat crystal"]) + "\n";
    o += "summon crystal: " +  std::to_string(consumables["summon crystal"]) + "\n";
    if (consumables["cod charm"] > 0){
        o += "cod charm: " +  std::to_string(consumables["cod charm"]) + "\n";
    }
    return o;
}

std::string temp_data::all_consumables(){
    std::string o;
    o += BBLK "USE" BLK "ABLE CONSUMABLES:\n";
    o += usable_consumables();
    o += "\nPASSIVE CONSUMABLES:\n";
    o += "extra life: " + std::to_string(consumables["extra life"]) + "\n";
    o += "\nEFFECTS:\n";
    o += "roll insurance: " + std::to_string(consumables["roll_insurance"]) + "\n";
    return o;
}

void temp_data::set_this_room_clear_in_map(){ // adds to cleared_rooms, duh.
    cleared_rooms.insert(current_floor.curr_path());
}
bool temp_data::thisfloor_cleared() const{ // used to be a toggleable flag. now checks if this floor has been set clear in the map.
    return cleared_rooms.contains(current_floor.curr_path());
}

void temp_data::set_this_room_looted_in_map(){ // analogously
    looted_rooms.insert(current_floor.curr_path());
}
bool temp_data::thisfloor_looted() const{ // analogously
    return looted_rooms.contains(current_floor.curr_path());
}

int temp_data::room_type() const{ // determine room type from the current room. Possibles types [see world_gen.h]
    std::hash<std::string> hashfun;
    std::string dat = room_hash() + "typesalt";
    if (!continue_here()){ // boss room
        return 1;
    }
    else if (current_floor.height <= 0){ // floor 0 is enemies. secret subterranean rooms are also enemies.
        return 0;
    }
    else if (current_floor.height >= 5){
        if (current_floor.curr_path() == "5"){
            return 2;
        }
        std::size_t typ_hash = hashfun(dat);
        double typ_float = (typ_hash * 1.0) / HASH_MAX;
        if (typ_float < 0.2){
            return 2;
        } else {
            return 0;
        }
    }
    return 0;
}

void temp_data::cod_ritual() { // transport to the CAVERN OF THE COD
    // TK
}
