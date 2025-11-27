//
//  main.cpp
//  ominous dice torment game
//
//  Created by jan musija on 9/13/25.
//
// when you create another copy also remember to uhh
// if working with xcode
//
// project > search paths > header search paths > add /opt/homebrew/include
// targets > build phases > link binary with libraries, then cmd shift g /opt/homebrew/opt/gmp/lib and add libgmp.a and libgmpxx.a
//
// if you're using g++, normally,
// g++ *.cpp -pedantic -Wall -Wextra -lgmpxx -lgmp -o cotc


#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include <stack>
#include <string>

#include "int_large.h"
#include "base_info.h"
#include "enemy.h"
#include "world_gen.h"
#include "save_and_load.h"
#include "dice.h"
#include "combat.h"
#include "shop.h"
#include "stream_colors.h"

#define cheat 1 // enables cheats

//const int BASE_HP = 100;

/*
 TODO:
 - implement actual gameplay
 - implement world generation/exploration mechanic: create new sites: specify xp level (must be below your current xp, gets rounded a bit)
 - implement shop: should be able to purchase dice, modify dice, reroll (heh) shop options with coins.
 */

std::string input_loop(base_data & gamedata, player_data & playerdata, world_data & worlddata, temp_data & tempdata, shop_cache & shopcache, bool & ef){ // ef = enterflag
    std::string context = gamedata.current_context();
    if (tempdata.input_desired){
        int input_unknown = 1;
        if (ef){
            /*
             
             ***** MESSAGES ON ENTERING A SCREEN *****
             
             */
            if (context == "menu") {
                std::cout << "Welcome to the menu. Type \"" BBLK "help" BLK "\" for help.\n";
            }
            else if (context == "titlescreen") {
                std::cout << "Welcome to the title screen. The save title is: \"" << gamedata.savename << "\".\nType \"" BBLK "menu" BLK "\" for the menu. Type \"" BBLK "inv" BLK "\" to see your inventory. Type \"" BBLK "shop" BLK "\" to see the shop. Type \"" BBLK "map" BLK "\" or \"" BBLK "play" BLK "\" to view and enter sites. Type \"" BBLK "quit" BLK "\" to quit.\n"; // TK
            }
            else if (context == "shop") {
                std::cout << "Welcome to the evil domain of capitalism, known to some as the shop. Type \"" BBLK "return" BLK "\" to go back. Type \"" BBLK "inv" BLK "\" to view your inventory. "
                // << "Type \"" BBLK "sell" BLK "\" to sell a die. "
                << "Type \"" BBLK "stocks" BLK "\" to view the offerings. Type \"" BBLK "shopdie" BLK "\" to view the shop die. Type \"" BBLK "coins" BLK "\" to view how many coins and xp you have. Type \"" BBLK "viewdie" BLK "\" to view one of your dice. Type \"" BBLK "buy" BLK "\" to view the options for purchasing things.\n The shop has:\n"; // TK
                std::cout << display_warez(playerdata, worlddata, shopcache);
            }
            else if (context == "inventory") {
                std::cout << "You are in your inventory. Type \"" BBLK "return" BLK "\" to go back.\n Type \"" BBLK "info" BLK "\" to see information about your inventory and its size. Type \"" BBLK "view" BLK "\" to view a die in a specific inventory slot, \"" BBLK "viewall" BLK "\" to view all your owned dice, and \"" BBLK "viewequipped" BLK "\" to view your equipped dice (and their positions in your inventory).\nType \"" BBLK "equip" BLK "\" to equip a die you own to an empty slot. Type \"" BBLK "unequip" BLK "\" to unequip a die you currently have equipped. Type \"" BBLK "swap" BLK "\" to interchange the dice in two equip slots. \n"; // TK
            }
            else if (context == "map") {
                std::cout << "You are on the world map. Type \"" BBLK "sites" BLK "\" to view accessible sites at your xp level. " <<
                // "Type \"" BBLK "sitesall" BLK "\" to view all sites, including inaccessible ones. " <<
                "Type \"" BBLK "enter" BLK "\" to enter a site (by id). Type \"" BBLK "view" BLK "\" to view information about a site in more detail. Type \"" BBLK "new" BLK "\" to explore and find a new site at a given xp level (which must be no higher than your current xp level).\nType \"" BBLK "shop" BLK "\" to go to the shop, and \"" BBLK "inv" BLK "\" to go to your inventory.\n"; // TK
            }
            else if (context == "wiki") {
                std::cout << "You are in the in-game, auto-generated wiki. For a  to view a specific object's information. Type \"" BBLK "listenemy" BLK "\" to view all known enemy types (may be a long list!). Type \"" BBLK "viewenemy" BLK "\" to view an enemy type. Type \"" BBLK "enemydetail" BLK "\" to view an enemy type at a specific verbosity. Type \"" BBLK "enemypage" BLK "\" to view up to 100 enemy types. Type \"" BBLK "listsite" BLK "\" to view all sites. Type \"" BBLK "viewsite" BLK "\" to view a site.\n"; // TK
            }
            else if (context == "combat") {
                if (tempdata.site_ptr == NULL){
                    if (tempdata.selected_site < worlddata.sites.size()){
                        std::cout << "Error: no site pointer?! fixing...\n";
                        tempdata.site_ptr = & worlddata.sites[tempdata.selected_site];
                    } else {
                        std::cout << "Error: no site pointer?! Leaving combat\n";
                        gamedata.context.pop();
                        tempdata.reset();
                        playerdata.dice.unroll(1);
                    }
                }
                else{
                    int rt = tempdata.room_type();
                    if (tempdata.thisfloor_cleared() || tempdata.enemies.size() == 0){ // room with no enemies
                        tempdata.set_this_room_clear_in_map(); // if it isn't (because it generated empty)
                        std::cout << "You are on floor " << tempdata.current_floor.height;
                        if (!tempdata.current_floor.mainline()){
                            std::cout << " (path: " << tempdata.current_floor.curr_path() << ")";
                        }
                        std::cout << " of " << tempdata.site_ptr->name << ".\n";
                        if (rt == 2 && !tempdata.thisfloor_looted()){ // buff room / loot room
                            std::cout << "this is a buff room";
                            if (tempdata.thisfloor_looted()){
                                std::cout << ", but it has already been looted...\n";
                            } else {
                                tempdata.set_this_room_looted_in_map();
                                double roll = (gamedata.rng() * 1.0) / gamedata.rng.max();
                                std::string gained_consumable = "";
                                int gaincount = 0;
                                if (roll <= 0){
                                    gaincount = 0;
                                }
                                else if (roll < 0.125){
                                    gained_consumable = "first aid kit";
                                    gaincount = 1;
                                }
                                else if (roll < 0.25){
                                    gained_consumable = "extra life";
                                    gaincount = 1;
                                }
                                else if (roll < 0.3){
                                    gained_consumable = "summon crystal";
                                    gaincount = 2;
                                }
                                else if (roll < 0.6){
                                    gained_consumable = "insurance potion";
                                    gaincount = 3;
                                }
                                else if (roll < 1){
                                    gained_consumable = "retreat crystal";
                                    gaincount = 1;
                                }
                                else {
                                    gaincount = 0;
                                }
                                if (gaincount > 0) {
                                    std::cout << " and you found " + std::to_string(gaincount) + " " + gained_consumable + "!\n";
                                    tempdata.consumables[gained_consumable]+= gaincount;
                                    if ((gamedata.rng() * 1.0) / gamedata.rng.max() < 0.7) {
                                        std::cout << "You also found a cod charm!\n";
                                        tempdata.consumables["cod charm"]++;
                                    }
                                } else {
                                    if ((gamedata.rng() * 1.0) / gamedata.rng.max() < 0.7) {
                                        std::cout << ", and you found a cod charm!\n";
                                        tempdata.consumables["cod charm"]++;
                                    } else {
                                        std::cout << "... but it is empty.\n";
                                    }
                                }
                            }
                        }
                    if (tempdata.continue_here()) {
                        std::cout << "To go to the next floor, type \"" BBLK "up" BLK "\".\n";
                        if (tempdata.branch_here()){
                            std::cout << "To into a side path, type \"" BBLK "branch" BLK "\".\n";
                        }
                    }
                    if (tempdata.current_floor.height > 0){
                        std::cout << "To go to the previous floor, type \"" BBLK "down" BLK "\".\n";
                    }
                    std::cout << "To use a consumable item, type \"" BBLK "use" BLK "\". To view consumable items and effects, type \"" BBLK "items" BLK "\". To manage your inventory, type \"" BBLK "inv" BLK "\". To view your dice, type \"" BBLK "dice" BLK "\". To view a die, type \"" BBLK "viewdie" BLK "\". To view the current floor's xp level, type \"" BBLK "xplev" BLK "\". To view your current health, type \"" BBLK "hp" BLK "\".\n";
                }
                else { // enemies are to be had.
                    std::cout << "You are in combat, on floor " << tempdata.current_floor.height;
                    if (!tempdata.current_floor.mainline()){
                        std::cout << " (path: " << tempdata.current_floor.curr_path() << ")";
                    }
                    std::cout << " of " << tempdata.site_ptr->name << ".\n";
                    if (!tempdata.continue_here()){
                        std::cout << "This is a boss floor.\n";
                    }
                    if (tempdata.enemies.size() > 0){std:: cout << display_combatants(tempdata,worlddata);}
                    std::cout << "To view an enemy,  type \"" BBLK "view" BLK "\". To list all enemies, type \"" BBLK "list" BLK "\". To view your dice, type \"" BBLK "dice" BLK "\". To view a die, type \"" BBLK "viewdie" BLK "\". To roll a die you own, type \"" BBLK "roll" BLK "\". To roll all your dice, type \"" BBLK "rollall" BLK "\"-- you can also type a number after it to repeat this that many times or until a fail.\nTo use an inventory item, type \"" BBLK "use" BLK "\". To view consumable items and effects, type \"" BBLK "items" BLK "\". To attack an enemy and end your turn, type \"" BBLK "target" BLK "\". To view the current damage you will deal upon attacking, type \"" BBLK "damage" BLK "\". To view your own health, type \"" BBLK "hp" BLK "\". To view the equip slots of dice you have left to roll, type \"" BBLK "unrolled" BLK "\". To view a log of the enemies' previous turn, type \"" BBLK "log" BLK "\". To view the current floor's xp level, type \"" BBLK "xplev" BLK "\". \n";
                }
            }
            }
            else {
                std::cout << "WARNING! unknown context. Recommended to use the " BBLK "menu" BLK " to go to the " BBLK "title" BLK " screen and clear context.\n";
            }
        }
        ef = 0;
        while (input_unknown != 0){
            if (input_unknown > 5){ // something is wrong
                std::cin.clear();
                break;
            }
            int iu = 0;
            std::string user_input;
            std::cout << ">"; // to make it clear the user should input something.
            std::cin >> user_input;
            // general commands
            {
                if (user_input == "menu") {
                    gamedata.context.push("menu");
                    ef = 1;
                    break;
                }
                else if (user_input == "wiki") {
                    gamedata.context.push("wiki");
                    ef = 1;
                    break;
                }
                else if (user_input == "return") {
                    if (context == "combat"){ // exiting combat
                        std::cout << "are you sure (y/n)? " RED "You will lose all progress in this combat session! " BLK;
                        std::cin >> user_input;
                        if (user_input == "y") {
                            tempdata.reset();
                            playerdata.dice.unroll(1);
                            gamedata.context.pop();
                            ef = 1;
                        }
                        break;
                    }
                    if (gamedata.context.size() > 1){
                        gamedata.context.pop();
                        ef = 1;
                        break;
                    } else {
                        break;
                    }
                }
                else if (user_input == "whereami" or user_input == "where_am_i") {
                    ef = 1; break;
                }
#if cheat == 1
                else if (user_input == "cheat_help") {
                    std::cout << "Use \"" BBLK "cdbg" BLK "\" to open a breakpoint in the code.\n";
                    if (context == "combat"){
                        std::cout << "Use \"" BBLK "cheat_floor" BLK "\" + floor to go to a given floor.\n";
                        std::cout << "Use \"" BBLK "cheat_kill" BLK "\" + floor to instantly kill all enemies and void their drops. Use \"" BBLK "cheat_kill_drop" BLK "\" to include the drops. Use \"" BBLK "cheat_consumable" BLK "\" or \"" BBLK "cheat_item" BLK "\" to cheat in a consumable item\n";
                    }
                    else if (context == "inventory"){
                        std::cout << "Use \"" BBLK "cheat_xp" BLK "\" + xp to set your xp to a given quantity.\n";
                    }
                    else if (context == "shop"){
                        std::cout << "Use \"" BBLK "cheat_xp" BLK "\" + xp to set your xp to a given quantity.\n";
                        std::cout << "Use \"" BBLK "cheat_coins" BLK "\" + coins to set your coins to a given quantity.\n";
                    }
                    else {
                        std::cout << "No cheats on this screen.";
                    }
                    break;
                }
                else if (user_input == "cdbg"){
                    int a = 0;
                    break;
                }
#endif
            }
            if (context == "menu") {
                /*
                 ***** IN THE MENU *****
                 associated commands: quit help save (export) seed title
                 */
                if (user_input == "quit") {
                    std::cout << "are you sure? (y/n) Remember to save before quitting! ";
                    std::cin >> user_input;
                    if (user_input == "y") {
                        gamedata.gaming = 0;
                    }
                    break;
                }
                else if (user_input == "help") {
                    std::cout << "GENERAL HELP (always-accessible commands): type \"" BBLK "menu" BLK "\" to enter the menu. type \"" BBLK "return" BLK "\" to go to the previous screen/menu/etc. Type \"" BBLK "whereami" BLK "\" to display the entry message for the current area. Type \"" BBLK "wiki" BLK "\" to view the auto-generated wiki.\nMENU HELP: to quit, type \"" BBLK "quit" BLK "\". for this message, type \"" BBLK "help" BLK "\". to save, type \"" BBLK "save" BLK "\". to view the seed, type \"" BBLK "seed" BLK "\". to go to the title screen to the game, type \"" BBLK "title" BLK "\". To rename the save, type \"" BBLK "rename" BLK "\".\n";
                    std::cout << "GAMEPLAY HELP:\nIn combat, on your turn, you can " BBLK "roll" BLK "your dice as many times as you want, except that before rolling a die again you must roll all the others you have equipped too. However, if you fail a roll by rolling too low (typically, this is a 1), you immediately forfeit your turn.\n"; //TK
                    std::cout << "ITEM HELP:\n\"retreat crystal\": retreat to the previous floor while in combat.\n\"summon crystal\": summon enemies in empty room.\n\"extra life\": revives you and restores a little hp if you die.\n\"first aid kit\": restores your health to the maximum, in an empty room.\n\"insurance potion\": prevents you from failing two rolls.\n"; //TK
                    break;
                }
                else if (user_input == "save" or user_input == "export") {
                    std::cin.ignore(); // to get past newline
                    std::cout << "FYI: saving only saves data about the world and about the player's current collection of Things. Anyway, input name of target file:\n";
                    std::string title;
                    std::getline (std::cin,title);
                    export_data(title,gamedata,playerdata,worlddata);
                    std::cout << "saved!\n";
                }
                else if (user_input == "rename") {
                    std::cin.ignore(); // to get past newline
                    std::cout << "what should the save be titled?\n";
                    std::getline (std::cin,gamedata.savename);
                    std::cout << "renamed.\n";
                }
                else if (user_input == "seed") {
                    std::cout << "seed: " << gamedata.seed << "\n";
                    break;
                } else if (user_input == "title") {
                    std::cout << "Exiting to title screen...\n\n";
                    tempdata.reset();
                    playerdata.dice.unroll(1);
                    ef = 1;
                    std::stack<std::string> newcontext;
                    newcontext.push("titlescreen");
                    gamedata.context = newcontext;
                    break;
                } else {
                    std::cin.clear();
                    std::cout << "sorry, I'm not sure what you mean. Try again:\n";
                    iu = input_unknown +1;
                }
            }
            else if (context == "inventory") {
                /*
                 ***** IN THE INVENTORY *****
                 associated commands: info (xp coins) view (viewdie view_die) viewall (view_all) viewequipped (viewequip viewequips view_equipped view_equip view_equips) equip unequip swap
                 */
                if (user_input == "info" or user_input == "xp" or user_input == "coins" ) {
                    std::cout << "You have " << playerdata.dice.owned.size() << "/" << playerdata.inventory_slots << " inventory slots occupied, and " << playerdata.dice.equipped.size() << "/" << playerdata.equip_slots << " equip slots occupied. The currently equipped dice are at the following positions in your inventory: ";
                    for (int i = 0; i<playerdata.dice.equipped.size(); i++){
                        std::cout << playerdata.dice.equipped[i]; if(i+1 <playerdata.dice.equipped.size()) {std::cout << ", ";}
                    }
                    std::cout << ".\nYou have " << playerdata.xp << " experience points, and " << playerdata.coins << " coins.\n";
                    break;
                } else if (user_input == "view" or user_input == "view_die" or user_input == "viewdie") {
                    std::cout << "which inventory slot? ";
                    int slot;
                    std::cin >> slot;
                    std::cin.ignore();
                    std::cout << playerdata.dice.view_index(slot,1,playerdata.inventory_slots);
                    break;
                }
#if cheat == 1
                else if (user_input == "cheat_xp"){
                    int_large x;
                    std::cin >> x;
                    std::cout << "setting xp to " << x << "\n";
                    playerdata.xp = x;
                    break;
                }
#endif
                else if (user_input == "viewall"  or user_input == "view_all") {
                    std::cout << playerdata.dice.view_all(0);
                } else if (user_input == "viewequipped" or user_input == "viewequip" or user_input == "viewequips" or user_input == "view_equipped" or user_input == "view_equip" or user_input == "view_equips") {
                    std::cout << playerdata.dice.view_equips(0);
                } else if (user_input == "equip" ) {
                    // check if you have the space
                    if (playerdata.dice.equipped.size() >= playerdata.equip_slots){
                        std::cout << "No equip slots remaining!\n";
                        break;
                    } else {
                        std::cout << "which inventory slot? ";
                        int slot;
                        std::cin >> slot;
                        std::cin.ignore();
                        bool already = 0;
                        for (int i = 0; i<playerdata.dice.equipped.size(); i++){
                            if (playerdata.dice.equipped[i] == slot){
                                std::cout << "die " << slot << " is already equipped in equip slot " << i << ".\n";
                                already = 1;
                                break;
                            }
                        }
                        if (!already){
                            if (slot < playerdata.dice.owned.size() && slot >= 0){
                                playerdata.dice.equipped.push_back(slot);
                                std::cout << "equipped die " << slot << " in position " << playerdata.dice.equipped.size() -1  << ".\n";
                            } else {
                                std::cout << "you have no die in that inventory slot.\n";
                            }
                        }
                    }
                } else if (user_input == "unequip" ) {
                    // check if this would make all your dice safe (forbidden)
                    std::cout << "which equip (not inventory) slot? ";
                    int slot;
                    std::cin >> slot;
                    std::cin.ignore();
                    if (slot < playerdata.dice.equipped.size()){
                        if (playerdata.dice.equips_valid(slot)){
                            playerdata.dice.equipped.erase (playerdata.dice.equipped.begin()+slot);
                            std::cout << "Unequipped die from equip slot " << slot << ".\n";
                            break;
                        } else {
                            std::cout << "You need one die that isn't guaranteed safe.\n";
                            break;
                        }
                    } else {
                        std::cout << "There is no die to unequip here.\n";
                        break;
                    }
                } else if (user_input == "swap" ) {
                    std::cout << "which equip (not inventory) slots? ";
                    int slot1; int slot2;
                    std::cin >> slot1 >> slot2;
                    std::cin.ignore();
                    if (slot1 < playerdata.dice.equipped.size() && slot2 < playerdata.dice.equipped.size()){
                        int j = playerdata.dice.equipped[slot1];
                        playerdata.dice.equipped[slot1] = playerdata.dice.equipped[slot2];
                        playerdata.dice.equipped[slot2] = j;
                        std::cout << "swapped equipped dice positions successfully.\n";
                    } else {
                        std::cout << "positions invalid, did not swap dice.\n";
                    }
                } else if (user_input == "help"){
                    ef = 1;
                    break;
                } else {
                    std::cin.clear();
                    std::cout << "sorry, I'm not sure what you mean. Try again:\n";
                    iu = input_unknown +1;
                }
            }
            else if (context == "titlescreen") {
                /*
                 ***** IN THE TITLE SCREEN *****
                 associated commands: quit inv (inventory) shop play (map)
                 */
                if (user_input == "quit") {
                    std::cout << "are you sure? (y/n) Remember to save before quitting!\n";
                    std::cin >> user_input;
                    if (user_input == "y") {
                        gamedata.gaming = 0;
                    }
                    break;
                } else if (user_input == "play" or user_input == "map"){
                    gamedata.context.push("map");
                    ef = 1;
                    break;
                } else if (user_input == "inv" or user_input == "inventory"){
                    gamedata.context.push("inventory");
                    ef = 1;
                    break;
                } else if (user_input == "shop"){
                    gamedata.context.push("shop");
                    ef = 1;
                    break;
                } else {
                    std::cin.clear();
                    std::cout << "sorry, I'm not sure what you mean. Try again:\n";
                    iu = input_unknown +1;
                }
            }
            else if (context == "map") {
                /*
                 ***** ON THE WORLD MAP *****
                 associated commands: sites (listsite list_site) sitesall (sites_all) enter view (viewsite view_site) new shop inv (inventory)
                 */
                if (user_input == "sites" or user_input == "listsite" or user_input == "list_site") {
                    /*for (int i = 0; i<worlddata.sites.size(); i++){
                        if (worlddata.sites[i].xp_min <= playerdata.xp){
                            std::cout << worlddata.view_info(i,0);
                            std::cout << "\n";
                        }
                    }
                    break;
                    */ // currently irrelevant
                    for (int i = 0; i<worlddata.sites.size(); i++){
                        std::cout << worlddata.view_info(i,0);
                        std::cout << "\n";
                    }
                    break;
                } else if (user_input == "sitesall" or user_input == "sites_all"){
                    for (int i = 0; i<worlddata.sites.size(); i++){
                        std::cout << worlddata.view_info(i,0);
                        std::cout << "\n";
                    }
                    break;
                } else if (user_input == "enter"){
                    std::cout << "Which id? ";
                    int id;
                    std::cin >> id;
                    if (id < worlddata.sites.size()){
                        tempdata.reset();
                        playerdata.dice.unroll(1);
                        playerdata.hp = playerdata.base_hp; // health maxed
                        std::cout << "entering site " << id << ": " << worlddata.sites[id].name << ".\n";
                        tempdata.input_desired = 0; // though, player takes turn first!
                        tempdata.combat = 1;
                        //tempdata.player_turn = 1; // will be enabled in function
                        tempdata.current_floor.clear();
                        tempdata.site_ptr = &(worlddata.sites[id]);
                        tempdata.selected_site = id;
                        gamedata.context.push("combat");
                        tempdata.newfloor = 1; // load enemies for 0th floor in associated thing
                        { // TK: todo, more modifiable consumable inventory
                            std::map<std::string, unsigned int> con;
                            std::hash<std::string> h;
                            if (h(tempdata.room_hash()+"gamer salt... ughh")%2){
                                con["first aid kit"] = 1;
                            } else {
                                con["extra life"] = 1;
                            }
                            tempdata.initialize_consumables(con);
                        }
                        //ef = 0 still.
                        break;
                    } else {
                        std::cout << "No known site by that id!!\n";
                    }
                    break;
                } else if (user_input == "view" or user_input == "viewsite" or user_input == "view_site"){
                    std::cout << "Which id? ";
                    int id;
                    std::cin >> id;
                    std::cout << worlddata.view_info(id,1) << "\n";
                    break;
                } else if (user_input == "new"){
                    std::cout << "At what xp level? You can find sites at any amount of xp up to your current xp level (" << playerdata.xp << "); higher xp sites are generally more difficult. ";
                    int_large lvl;
                    std::cin >> lvl;
                    std::cin.ignore();
                    if (lvl <= playerdata.xp && lvl >= 0){
                        unsigned long id = worlddata.add_site(lvl);
                        std::cout << "Site discovered:\n" << worlddata.sites[id].name << " (site " << id << ")\n"; // print data
                    } else if (lvl < 0) {
                        std::cout << "Such a site would be for unborn babies. You don't look like an unborn baby to me.\n";
                    } else {
                        std::cout << "You do not have enough xp to find sites at this level!\n";
                    }
                    break;
                    break;
                }
                else if (user_input == "shop") {
                    gamedata.context.push("shop");
                    ef = 1;
                    break;
                }
                else if (user_input == "inv" or user_input == "inventory") {
                    gamedata.context.push("inventory");
                    ef = 1;
                    break;
                }
                else {
                    std::cin.clear();
                    std::cout << "sorry, I'm not sure what you mean. Try again:\n";
                    iu = input_unknown +1;
                }
            }
            else if (context == "wiki") {
                /*
                 ***** IN THE WIKI *****
                 associated commands: listenemy (list_enemy) viewenemy (view_enemy) enemypage (enemy_page) listsite (sites list_site) viewsite (view_site) enemydetail (enemy_detail) back
                 */
                if (user_input == "listsite" or user_input == "sites" or user_input == "list_site") {
                    for (int i = 0; i<worlddata.sites.size(); i++){
                        std::cout << worlddata.view_info(i,0);
                        std::cout << "\n";
                    }
                    break;
                } else if (user_input == "listenemy" or user_input == "list_enemy"){
                    for (int i = 0; i<worlddata.enemy_types.size(); i++){
                        std::cout << worlddata.view_enemy(i,0);
                        std::cout << "\n";
                    }
                    break;
                } else if (user_input == "viewenemy" or user_input == "view_enemy"){
                    std::cout << "Which id? ";
                    int id;
                    std::cin >> id;
                    std::cout << worlddata.view_enemy(id,2) << "\n";
                    break;
                } else if (user_input == "enemydetail" or user_input == "enemy_detail"){
                    std::cout << "Which id, and how verbose (0 = as verbose as viewenemy)? ";
                    int id, ver;
                    std::cin >> id >> ver;
                    std::cout << worlddata.view_enemy(id,ver+2) << "\n";
                    break;
                } else if (user_input == "enemypage" or user_input == "enemy_page"){
                    std::cout << "Which page? ";
                    int pg;
                    std::cin >> pg;
                    #define wikipagesize 100
                    pg*=wikipagesize;
                    if (pg < worlddata.enemy_types.size()){
                        for (int i = pg; i < pg+wikipagesize && i<worlddata.enemy_types.size(); i++){
                            std::cout << worlddata.view_enemy(i,0);
                            std::cout << "\n";
                        }
                    } else {
                        std::cout << "you have yet to find enough creatures to fill this page...\n";
                    }
                    break;
                } else if (user_input == "viewsite" or user_input == "view_site"){
                    std::cout << "Which id? ";
                    int id;
                    std::cin >> id;
                    std::cout << worlddata.view_info(id,1) << "\n";
                    break;
                } else {
                    std::cin.clear();
                    std::cout << "sorry, I'm not sure what you mean. Try again:\n";
                    iu = input_unknown +1;
                }
            }
            else if (context == "combat") {
                /*
                 ***** IN COMBAT *****
                 associated commands: view (viewenemy view_enemy) list (listenemy list_enemy) dice viewdie (view_die) roll target rollall (roll_all) unrolled damage log (showlog) hp (health my_hp myhp my_health myhealth) use items (item consumables)
                 next (proceed up) branch down (prev retreat) inv (inventory)
                 */
                if (user_input == "view" or user_input == "viewenemy" or user_input == "view_enemy") {
                    std::cout << "Which position? ";
                    int id;
                    std::cin >> id;
                    std::cout << view_combatant(tempdata, worlddata, id, 2) << "\n";
                    break;
                }
                else if (user_input == "list" or user_input == "listenemy" or user_input == "list_enemy"){
                    std::cout << display_combatants(tempdata,worlddata);
                    break;
                }
                else if (user_input == "dice"){
                    std::cout << playerdata.dice.view_equips(0);
                    break;
                }
#if cheat == 1
                else if (user_input == "cheat_floor"){
                    int f;
                    std::cin >> f;
                    std::cout << "going to floor " << f << "\n";
                    tempdata.current_floor.height = f-1;
                    tempdata.advance_floor(0);
                    break;
                }
                else if (user_input == "cheat_kill"){
                    std::cout << "slaughtering all enemies.\n";
                    tempdata.enemies.clear();
                    tempdata.reset_roll();
                    tempdata.end_player_turn(playerdata);
                }
                else if (user_input == "cheat_consumable" or user_input == "cheat_item"){
                    std::cout << "enter amount and type\n";
                    unsigned int amt;
                    std::cin >> amt;
                    if (std::cin.fail()){
                        std::cout << "not a number??\n";
                        std::cin.clear();
                    } else {
                        std::string item;
                        std::cin.ignore();
                        std::cin.clear();
                        std::getline (std::cin,item);
                        tempdata.consumables[item] = amt;
                        std::cout << "success\n";
                    }
                }
                else if (user_input == "cheat_kill_drop"){
                    std::cout << "slaughtering all enemies.\n";
                    std::vector<int> surv;
                    tempdata.enemies_surviving(surv);
                    for (int i = 0 ; i<surv.size(); i++){
                        bool dgf = 0;
                        bool ccf = 0;
                        loot_enemy(gamedata, surv[i], tempdata, playerdata, worlddata,dgf,ccf);
                        if (dgf){
                            int last_die_slot = (int) playerdata.dice.owned.size()-1;
                            std::cout << "new die attained! (added to inventory slot " << last_die_slot << "!)\n";
                            std::cout << playerdata.dice.view_index(last_die_slot,0,playerdata.inventory_slots);
                        }
                        if (ccf){
                            std::cout << "cod charm gained!\n";
                        }
                    }
                    tempdata.enemies.clear();
                    tempdata.reset_roll();
                    tempdata.end_player_turn(playerdata);
                }
#endif
                else if (user_input == "hp" or user_input == "myhp" or user_input == "my_hp" or user_input == "health" or user_input == "myhealth" or user_input == "my_health"){
                    std::cout << "You have " GRN << playerdata.hp << " / " << playerdata.base_hp << BLK " health.\n";
                    break;
                }
                else if (user_input == "viewdie" or user_input == "view_die"){
                    std::cout << "which equip slot? ";
                    int slot;
                    std::cin >> slot;
                    std::cin.ignore();
                    if (slot > playerdata.dice.equipped.size() || slot < 0){
                        if (slot < playerdata.equip_slots){
                            slot = playerdata.inventory_slots;
                        } else {
                            slot = playerdata.inventory_slots+1;
                        }
                    } else {
                        slot = playerdata.dice.equipped[slot];
                    }
                    std::cout << playerdata.dice.view_index(slot,1,playerdata.inventory_slots+1);
                    break;
                }
                else if (user_input == "unrolled"){
                    std::vector<int> ur;
                    bool b = playerdata.dice.remaining_unrolled(ur);
                    if (!b){
                        std::cout << "I did something wrong, there's no situation where this should be possible.\n";
                    }
                    std::cout << "equip slots with dice not yet rolled:\n";
                    for (int i = 0; i < ur.size(); i++){
                        std::cout << ur[i];
                        if (i+1 < ur.size()) {std::cout << ",  ";}
                    }
                    std::cout << "\n";
                    break;
                }
                else if (user_input == "showlog" or user_input == "log"){
                    if (tempdata.prev_attack_log.length()>0){
                        std::cout << tempdata.prev_attack_log << "\n";
                    } else {
                        std::cout << "No previous enemy turn this floor to show.\n";
                    }
                }
                else if (user_input == "items" or user_input == "consumables" or user_input == "item"){
                    std::cout << tempdata.all_consumables();
                }
                else if (user_input == "use"){
                    // list items probably. TK.
                    std::cout << "which item?\n";
                    std::string item;
                    std::cin.ignore();
                    std::cin.clear();
                    std::getline (std::cin,item);
                    if (tempdata.consumables.has(item)){
                        /* using items:
                         "first aid kit" -> not in combat
                         "insurance potion" -> not out of combat
                         "retreat crystal" -> not out of combat
                         "summon crystal" -> not in combat
                         "extra life" -> not usable
                         
                         "roll_insurance" -> not usable (not an item)
                         */
                        if (item == "first aid kit"){
                            if (tempdata.thisfloor_cleared() && playerdata.hp < playerdata.base_hp){
                                tempdata.consumables.use(item);
                                playerdata.hp = playerdata.base_hp; // full hp restored
                                std::cout << "Restored to full health! (" GRN + playerdata.hp.get_str(10) + BLK ")";
                            } else if (tempdata.thisfloor_cleared()) { // p & !(p & q) <-> p & !q
                                std::cout << "You have no wounds to tend to.\n";
                            } else {
                                std::cout << "You cannot tend to your wounds during combat.\n";
                            }
                        }
                        else if (item == "insurance potion"){
                            if (!tempdata.thisfloor_cleared()){
                                tempdata.consumables.use(item);
                                tempdata.consumables["roll_insurance"]+=2;
                            } else {
                                std::cout << "You cannot use this potion out of combat.\n";
                            }
                        }
                        else if (item == "retreat crystal"){
                            if (!tempdata.thisfloor_cleared()){
                                if (!tempdata.cleared_rooms.empty()){
                                    tempdata.retreat_floor();
                                    std::cout << "You retreat to the previous floor";
                                    if (tempdata.current_floor.height < 0 && tempdata.subterranean_floors_entered == 0){
                                        tempdata.subterranean_floors_entered = 1;
                                        std::cout << "... and you realize you're somewhere strange";
                                    }
                                    std::cout <<".\n";
                                } else {
                                    std::cout << "You cannot retreat if you have just entered the site!\n";
                                }
                            } else {
                                std::cout << "You are not in combat and cannot use this item.\n";
                            }
                        }
                        else if (item == "summon crystal"){
                            if (tempdata.thisfloor_cleared()){
                                tempdata.consumables.use(item);
                                tempdata.cleared_rooms.erase(tempdata.current_floor.curr_path());
                                get_enemies(gamedata.rng,tempdata,worlddata);
                                ef = 1;
                            } else {
                                std::cout << "You cannot use this in combat.\n";
                            }
                        }
                        else {
                            std::cout << "this cannot be " BBLK "use" BLK "d!!\n";
                        }
                    } else {
                        std::cout << "you do not have this item!!\n";
                    }
                }
                else if (user_input == "roll" && !tempdata.thisfloor_cleared()){
                    std::cout << "which equip slot? ";
                    int slot;
                    std::cin >> slot;
                    if (slot > playerdata.dice.equipped.size() || slot < 0){
                        if (slot < playerdata.equip_slots){
                            std::cout << "You have no die equipped there.\n";
                            break;
                        } else {
                            std::cout << "Slot does not exist.\n";
                            break;
                        }
                    }
                    else {
                        int_large roll;
                        int_large rollraw;
                        std::string mod;
                        int suc = playerdata.dice.roll_modify_equipped(slot, gamedata.rng, roll,rollraw, mod); // returns 0 on fail, 1 on success, 2 on die already rolled, 3 on out of bounds
                        if (suc == 0){
                            if (tempdata.consumables.use("roll_insurance")){
                                std::cout << "Failure prevented by insurance.\n";
                                if (playerdata.unroll_if_all_rolled()){
                                    tempdata.commit_roll();
                                }
                                break;
                            } else {
                                std::cout << RED "Roll failed!!\n\n" BLK;
                                tempdata.end_player_turn(playerdata);
                            }
                        } else if (suc == 1) {
                            die* roller = & playerdata.dice.owned[playerdata.dice.equipped[slot]];
                            int tup = tempdata.resolve_roll(*roller, roll, rollraw, mod);
                            // check if all dice have been rolled, in which case commit current roll and unroll:
                            if (playerdata.unroll_if_all_rolled()){
                                tempdata.commit_roll();
                            }
                            std::cout << roller->description << " rolled successfully (rolled a " << rollraw;
                            if (tup > 0){
                                std::cout << ", x" << tup+1 << "from tuples";
                            }
                            std::cout << ")\n";
                        } else if (suc == 2){
                            std::cout << "You cannot roll a die again until you have rolled all of your dice.\n";
                            break;
                        } else {
                            std::cout << "Slot out of bounds (you should never see this message, it means I messed up)\n"; break;
                        }
                        /*
                         die* roller = & playerdata.dice.owned[playerdata.dice.equipped[slot]];
                        if (roller->rolled){
                            std::cout << "You cannot roll a die again until you have rolled all of your dice.\n";
                        } else {
                            int_large roll;
                            int_large moddedroll;
                            std::string mod;
                            bool suc = (roller->roll(gamedata.rng, roll, mod));
                            if (suc) {
                                suc = modify_roll(roll, mod, gamedata.rng, moddedroll);
                            } else {
                                std::cout << "Roll failed!!\n\n";
                                tempdata.end_player_turn();
                                break;
                            }
                            if (suc){
                                if (roller->type == 'a'){
                                    tempdata.damage_thisroll += roll;
                                } else if (roller->type == 'm') {
                                    tempdata.this_mult += roll;
                                }
                            } else {
                                std::cout << "Roll failed!!\n\n";
                                tempdata.end_player_turn();
                                break;
                            }
                        }*/
                    }
                    break;
                }
                else if ((user_input == "rollall" or user_input == "roll_all") && !tempdata.thisfloor_cleared()){
                    std::string rollct_str;
                    std::getline (std::cin,rollct_str);
                    int num_rolls;
                    bool fail = 0;
                    if (rollct_str.length() > 1 && rollct_str[0] == ' '){
                        num_rolls = std::atoi(rollct_str.substr(1,rollct_str.length()-1).c_str()); // 0 on fail
                        if (num_rolls < 1){
                            num_rolls = 1;
                        } else if (num_rolls > MAX_ALLOWED_ROLLS){
                            std::cout << "too many rolls (most allowed at a time: " << MAX_ALLOWED_ROLLS << ")";
                            num_rolls = 1;
                        }
                    } else {
                        num_rolls = 1;
                    }
                    std::vector<int> ur;
                    bool b = playerdata.dice.remaining_unrolled(ur);
                    if (!b){
                        std::cout << "I did something wrong, there's no situation where this should be possible.\n";
                    }
                    std::cout << "Rolling all remaining dice";
                    if (num_rolls > 1){
                        std::cout << " " << num_rolls << " times";
                    }
                    std::cout << ".\n";
                    for (int rnum = 0; rnum < num_rolls; rnum++){
                        for (int i = 0; (!fail) && i < ur.size(); i++){
                            die* roller = & playerdata.dice.owned[playerdata.dice.equipped[ur[i]]];
                            std::cout << roller->description << " (slot " << ur[i] << "): ";
                            int_large roll;
                            int_large rollraw;
                            std::string mod;
                            int suc = playerdata.dice.roll_modify_equipped(ur[i], gamedata.rng, roll,rollraw,mod);
                            if (suc == 0){
                                fail = 1;
                                break;
                            } else if (suc == 1) {
                                int tup = tempdata.resolve_roll(*roller, roll, rollraw,mod);
                                std::cout << rollraw;
                                if (tup > 0){
                                    std::cout << " (x" << tup+1 << " from tuples!)";
                                }
                                std::cout << "\n";
                            }
                        }
                        if (fail){
                            if (tempdata.consumables.use("roll_insurance")){
                                std::cout << "Failure prevented by insurance.\n";
                                if (playerdata.unroll_if_all_rolled()){
                                    tempdata.commit_roll();
                                }
                                break;
                            } else {
                                std::cout << RED "Roll failed!!\n\n" BLK;
                                tempdata.end_player_turn(playerdata);
                                break;
                            }
                        } else { // all dice rolled
                            tempdata.commit_roll();
                            playerdata.dice.unroll();
                        }
                        std::cout << "\n";
                    }
                    break;
                }
                else if (user_input == "damage"){
                    int_large d;
                    int_large h;
                    tempdata.calc_rolls(d,h);
                    std::cout << "your current damage is " RED << d << BLK "\n";
                    if (h != 0){
                        std::cout << "you will heal by " GRN << h << BLK "\n";
                    }
                    break;
                }
                else if (user_input == "xplev"){
                    std::cout << "the xp level of the current floor is " << floor_xp_level(tempdata.site_ptr->xp_min, tempdata.site_ptr->xp_ramping, tempdata.current_floor.height) << "\n";
                    break;
                }
                else if (user_input == "target"){
                    if (tempdata.damage_thisroll == 0 && tempdata.damage_pastrolls == 0){
                        std::cout << "you won't deal any damage! do you still want to target an enemy? (y/n): ";
                        std::string confirm;
                        std::cin >> confirm;
                        if (confirm != "y"){
                            break;
                        }
                    }
                    std::cout << "which enemy? ";
                    int pos;
                    std::cin >> pos;
                    if (pos > tempdata.enemies.size()){
                        std::cout << "no such enemy exists!!\n";
                        break;
                    } else if (tempdata.enemies[pos].hp <= 0) {
                        std::cout << "cannot target dead enemy!!\n";
                        break;
                    } else { // this will probably have to be polished later when there are more things to do
                        tempdata.commit_roll(); // make sure pastrolls is current
                        if (tempdata.health_pastrolls != 0){
                            tempdata.apply_healing(playerdata);
                            std::cout << "Healed to " << playerdata.hp << "hp.\n";
                        }
                        std::cout << "Attacking " << tempdata.enemies[pos].name << " for " << tempdata.damage_pastrolls;
                        tempdata.apply_damage(tempdata.enemies[pos]);
                        if (!tempdata.enemies[pos].alive()){
                            std::cout << ", killing it.";
                            bool dgf = 0;
                            bool ccf = 0;
                            loot_enemy(gamedata, pos, tempdata, playerdata, worlddata,dgf,ccf);
                            if (dgf){
                                int last_die_slot = (int) playerdata.dice.owned.size()-1;
                                std::cout << "new die attained! (added to inventory slot " << last_die_slot << "!";
                                std::cout << playerdata.dice.view_index(last_die_slot,0,playerdata.inventory_slots);
                            }
                            if (ccf){
                                std::cout << "cod charm gained!\n";
                            }
                        }
                        std::cout << "\n";
                        tempdata.end_player_turn(playerdata);
                    }
                    break;
                }
                else if (user_input == "inv" or user_input == "inventory"){
                    if (tempdata.thisfloor_cleared()) {
                        gamedata.context.push("inventory");
                        ef = 1;
                        break;
                    } else {
                        std::cout << "You cannot safely manage your inventory here.\n";
                    }
                }
                else if (user_input == "proceed" or user_input == "up" or user_input == "next"){
                    if (tempdata.thisfloor_cleared()){
                        if (tempdata.continue_here()){
                            tempdata.advance_floor(0); // note that this sets tempdata.input_desired = 0
                            if (tempdata.current_floor.height > tempdata.site_ptr->highest_floor){
                                tempdata.site_ptr->highest_floor = tempdata.current_floor.height;
                            }
                        } else {
                            std::cout << "You are at the top of this branch.\n";
                        }
                    } else {
                        std::cout << "Your path is blocked by your enemies.\n";
                    }
                    break;
                }
                else if (user_input == "branch"){
                    if (tempdata.thisfloor_cleared()){
                        if (tempdata.branch_here()){
                            tempdata.advance_floor(1);
                            if (tempdata.current_floor.height > tempdata.site_ptr->highest_floor){
                                tempdata.site_ptr->highest_floor = tempdata.current_floor.height;
                            }
                        } else {
                            std::cout << "There are no branching paths from this room.\n";
                        }
                    } else {
                        std::cout << "Your path is blocked by your enemies\n";
                    }
                    break;
                }
                else if (user_input == "retreat" or user_input == "down" or user_input == "prev" or user_input == "previous" or user_input == "back"){
                    if (tempdata.thisfloor_cleared()){
                        if (tempdata.current_floor.height > 0 || tempdata.subterranean_floors_entered){
                            tempdata.retreat_floor();
                            ef = 1;
                        } else {
                            std::cout << "You are in the first floor of this site.\n";
                        }
                    } else {
                        std::cout << "You try to retreat but your enemies block your path.\n";
                    }
                    break;
                }
                else {
                    std::cin.clear();
                    std::cout << "sorry, I'm not sure what you mean. Try entering the command again:\n";
                    iu = input_unknown +1;
                }
            }
            else if (context == "shop") {
                /*
                 ***** IN THE SHOP *****
                 associated commands: inv (inventory) sell stocks (wares warez offerings) shopdie (shop_die view_shopdie view_shop_die) coins buy_hp buy buy_inv_slot buy_equip_slot
                 */
                if (user_input == "inv" or user_input == "inventory") {
                    gamedata.context.push("inventory");
                    ef = 1;
                    break;
                }
                else if (user_input == "sell"){
                    std::cout << "not yet implemented\n";
                    // instructions... TK
                    break;
                }
#if cheat == 1
                else if (user_input == "cheat_coins"){
                    int_large c;
                    std::cin >> c;
                    std::cout << "setting coins to " << c.get_str() << "\n";
                    playerdata.coins = c;
                    break;
                }
                else if (user_input == "cheat_xp"){
                    int_large x;
                    std::cin >> x;
                    std::cout << "setting xp to " << x << "\n";
                    playerdata.xp = x;
                    break;
                }
#endif
                else if (user_input == "stocks" or user_input == "wares" or user_input == "warez" or user_input == "offerings"){
                    std::cout << display_warez(playerdata, worlddata, shopcache);
                    break;
                }
                else if (user_input == "buy") {
                    std::cout << "To buy the hp upgrade, type \"" BBLK "buy_hp" BLK "\". To buy a shop reroll (and refresh offerings), type \"" BBLK "buy_reroll" BLK "\". To buy an inventory slot (if applicable), type \"" BBLK "buy_inv_slot" BLK "\". To buy an equip slot, type \"" BBLK "buy_equip_slot" BLK "\". To buy the shop die, type \"" BBLK "buy_shop_die" BLK "\".\n";
                    break;
                }
                else if (user_input == "buy_hp"){
                    int_large amt_gained;
                    if (buy_hp_upgr(playerdata,amt_gained)){
                        std::cout << "bought a " GRN << amt_gained.get_str(10) << BLK " hp upgrade (new base hp: " GRN << playerdata.base_hp << BLK ")\n";
                    } else {
                        std::cout << "you cannot afford that.\n";
                    }
                    // instructions... TK
                    break;
                }
                else if (user_input == "buy_reroll"){
                    if (playerdata.coins >= SHOP_REROLL_COST){
                        worlddata.shop_rng.discard(1);
                        shopcache.init = 0;
                        std::cout << "rerolled the shop! New offerings:\n" << display_warez(playerdata, worlddata,shopcache);
                    } else {
                        std::cout << "you cannot afford that.\n";
                    }
                    // instructions... TK
                    break;
                }
                /*
                 if (pd.xp.get_str(4).length() > pd.inventory_slots + (2 - BASE_INV_SLOTS)){
                     o+= "Buy inventory slot: cost: " + pow_ui(4, pd.inventory_slots - (BASE_INV_SLOTS - 3)).get_str(10) + "\n";
                 }
                 if (pd.xp.get_str(7).length() > pd.equip_slots + (2 - BASE_EQUIP_SLOTS)){
                     o+= "Buy equip slot: cost: " + pow_ui(7, pd.equip_slots - (BASE_EQUIP_SLOTS - 3)).get_str(10) + "\n";
                 }
                 */
                else if (user_input == "buy_inv_slot"){
                    if (playerdata.xp.get_str(4).length() > playerdata.inventory_slots + (2 - BASE_INV_SLOTS)) {
                        int_large cost = pow_ui(4, playerdata.inventory_slots - (BASE_INV_SLOTS - 3));
                        if (playerdata.coins >= cost){
                            playerdata.inventory_slots++;
                            playerdata.coins -= cost;
                            std::cout << "Slot purchased successfully!\n";
                        } else {
                            std::cout << "you cannot afford that.\n";
                        }
                    } else {
                        std::cout << "you are too low-level to access that.\n";
                    }
                }
                else if (user_input == "buy_equip_slot"){
                    if (playerdata.xp.get_str(7).length() > playerdata.equip_slots + (2 - BASE_EQUIP_SLOTS)) {
                        int_large cost = pow_ui(7, playerdata.equip_slots - (BASE_EQUIP_SLOTS - 3));
                        if (playerdata.coins >= cost){
                            playerdata.equip_slots++;
                            playerdata.coins -= cost;
                            std::cout << "Slot purchased successfully!\n";
                        } else {
                            std::cout << "you cannot afford that.\n";
                        }
                    } else {
                        std::cout << "you are too low-level to access that.\n";
                    }
                }
                else if (user_input == "buy_shopdie" or user_input == "buy_shop_die"){
                    if (playerdata.inventory_slots > playerdata.dice.owned.size()) {
                        if (playerdata.coins >= shopcache.shopdie_cost){
                            if (shopcache.init){
                                playerdata.dice.owned.push_back(shopcache.shopdie);
                                playerdata.coins -= shopcache.shopdie_cost;
                                worlddata.shop_rng.discard(1);
                                shopcache.init = 0;
                                std::cout << "Die purchased successfully!\n";
                            } else {
                                std::cout << "You don't even know what you're buying!!\n";
                            }
                        } else {
                            std::cout << "you cannot afford that.\n";
                        }
                    } else {
                        std::cout << "you are too low-level to access that.\n";
                    }
                }
                else if (user_input == "view" or user_input == "view_die" or user_input == "viewdie") {
                    std::cout << "which inventory slot? ";
                    int slot;
                    std::cin >> slot;
                    std::cin.ignore();
                    std::cout << playerdata.dice.view_index(slot,1,playerdata.inventory_slots);
                    break;
                }
                else if (user_input == "shopdie" or user_input == "shop_die" or user_input == "view_shopdie" or user_input == "view_shop_die") {
                    std::cout << shopcache.shopdie.verbose_description(0);
                    break;
                }
                else if (user_input == "coins"){
                    std::cout << "You have " << playerdata.coins << " coins, and " << playerdata.xp << " experience points.\n";
                    break;
                }
                else {
                    std::cin.clear();
                    std::cout << "sorry, I'm not sure what you mean. Try again:\n";
                    iu = input_unknown +1;
                }
            }
            else if (context == "context_template") {
                /*
                 ***** ON THE context_template *****
                 associated commands: [list of associated commands]
                 */
                if (user_input == "command1") {
                    std::cout << "not yet implemented\n";
                    // instructions... 
                    break;
                } else if (user_input == "command2"){
                    std::cout << "not yet implemented\n";
                    // instructions...
                    break;
                } else {
                    std::cin.clear();
                    std::cout << "sorry, I'm not sure what you mean. Try again:\n";
                    iu = input_unknown +1;
                }
            }
            else {
                ef = 1;
                iu = input_unknown +1;
            }
            std::cin.clear();
            input_unknown = iu;
        }
    } else { // tempdata.input_desired = 0
        if ((context == "combat") && tempdata.newfloor){
            int rt = tempdata.room_type();
            if (!tempdata.thisfloor_cleared()){ // rooms that generate enemies
                if (rt == 1){ // (mini)boss room
                    get_boss(gamedata.rng,tempdata,worlddata);
                }
                if (rt == 0){ // default: FOES
                    get_enemies(gamedata.rng,tempdata,worlddata);
                }
            }
            tempdata.newfloor = 0;
            tempdata.player_turn = 1;
            tempdata.input_desired = 1;
            ef = 1;
            //tempdata.input_desired = 1;
        }
        else if ((context == "combat") && ! tempdata.player_turn){
            //enemy turn
            std::vector<int> survivors;
            if (tempdata.enemies_surviving(survivors)){
                std::cout << "\nEnemy turns:\n";
                for (int i = 0; i< survivors.size(); i++){
                    std::string o = tempdata.enemies[survivors[i]].name + " (enemy " + std::to_string(survivors[i]) + ")'s turn:\n";
                    tempdata.prev_attack_log += o;
                    std::cout << o;
                    std::cout <<
                    take_turn(survivors[i],gamedata,playerdata,worlddata,tempdata);
                }
                // survivors attack ... though not for now
                if (playerdata.hp < 0 && tempdata.consumables["extra life"] == 0) { // on player death
                    std::cout << RED "You have been defeated!" BLK " View last turn attack log? (y/n) ";
                    std::cin.clear();
                    std::string confirm;
                    std::cin >> confirm;
                    if (confirm == "y"){
                        std::cout << tempdata.prev_attack_log << "\n";
                    }
                    // tk
                    tempdata.reset();
                    gamedata.context.pop();
                    std::cout << "Exiting to map...\n";
                } else if (playerdata.hp < 0) { // player is still alive due to extra life
                    tempdata.consumables["extra life"]--;
                    playerdata.hp = 25; // for now
                    tempdata.end_enemy_turn();
                } else {
                    tempdata.end_enemy_turn();
                }
            } else {
                std::cout << "All enemies defeated; floor cleared!\n";
                ef = 1;
                tempdata.end_enemy_turn();
                tempdata.set_this_room_clear_in_map();
            }
            //check if enemies survive. if none remain, new floor
        }
        else { // unknown countext
            std::cout << "Unknown context!! \n";
            tempdata.input_desired = 1;
        }
    }
    return "";
}

//#include "dictionary_data.h"
int hmain (){
    return 0;
}

int main(int argc, const char * argv[]) {
    // initialize game
    std::cout <<
    "+*   CAVERN OF THE   *+\n" <<
    "*+    **  ***  **    +*\n" <<
    "+*   *    * *  * *   *+\n" <<
    "*+   *    * *  * *   +*\n" <<
    "+*    **  ***  **    *+\n\n";
    //std::cout << "uhh, welcome to a random concept for a game I had I guess.\n"
    std::cout << "to import a new save, type \"" BBLK "import" BLK "\".\nto start a fresh, randomly seeded game, write \"" BBLK "new" BLK "\".\nto start a fresh, seeded game, type \"" BBLK "seeded" BLK "\". to quit the game aggressively, type \"" BBLK "quit" BLK "\".\n>";
    base_data gamedata;
    player_data playerdata;
    world_data worlddata;
    temp_data tempdata;
    shop_cache shopcache;
    {
        while (true){
            std::string selection;
            std::cin >> selection;
            if (selection == "new") {
                int seed1 = (int)time(nullptr);
                std::cin.ignore(); // to get past newline
                std::cout << "what should the save be titled?\n";
                std::string title;
                std::getline (std::cin,title);
                int seed2 = (int)time(nullptr);
                std::mt19937 a(seed1);
                std::mt19937 b(seed2);
                int seed = (a() + b())%(16777216); // yes, there are exactly 2^24 seeds that can be attained this way. the reason is because I think it is funny.
                // this also means that the variety of seeds that seeded games can have is much larger.
                base_data temp(seed,title);
                gamedata = temp;
                worlddata.shop_rng = gamedata.rng;
                worlddata.site_rng = gamedata.rng;
                worlddata.grammar_variation_rng = std::mt19937(seed);
                break;
            } else if (selection == "import" or selection == "load") {
                std::cin.ignore(); // to get past newline
                std::cout << "Input name of target file:\n";
                std::string title;
                std::getline (std::cin,title);
                while (!load_data(title,gamedata,playerdata,worlddata)){
                    std::cout << "Invalid name, try again:\n";
                    std::cin.clear();
                    std::getline (std::cin,title);
                };
                break;
            } else if (selection == "seeded" or selection == "seed") {
                std::cout << "input seed (integer): ";
                int seed;
                std::cin >> seed;
                while (std::cin.fail()){
                    std::cin.clear();
                    std::cin.ignore();
                    std::cout << "sorry, I don't understand. try again:\n";
                    std::cin >> seed;
                }
                std::cin.ignore(); // to get past newline
                std::cout << "what should the save be titled?\n";
                std::string title;
                std::getline (std::cin,title);
                base_data temp(seed,title);
                gamedata = temp;
                worlddata.shop_rng = gamedata.rng;
                worlddata.site_rng = gamedata.rng;
                worlddata.grammar_variation_rng = std::mt19937(seed);
                break;
            } else if (selection == "quit"){
                throw; // you do this with dice a lot
                break; // this isn't necessary but whatever
            } else {
                std::cin.ignore(); // to get past newline
                std::cin.clear(); // to clear
                std::cout << "sorry, I didn't get that. Try again.\n";
            }
        }
    }
    std::cout << "at any time, type \"" BBLK "menu" BLK "\" to view the menu, which has help.\n\n";
    bool ef = 1;
    tempdata.seed = gamedata.seed;
    while (gamedata.gaming){
        input_loop(gamedata,playerdata,worlddata,tempdata,shopcache,ef);
    }
    return 0;
}
