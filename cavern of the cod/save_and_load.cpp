//
//  save_and_load.cpp
//  ominous dice torment game
//
//  Created by jan musija on 9/13/25.
//

#include "save_and_load.h"
#include "base_info.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "world_gen.h"
#include "enemy.h"

/* format:
{
base data
}
{
player data
}
{
world data
}
 */
bool load_data(const std::string filename, base_data & gamedata, player_data & playerdata, world_data & worlddata){
    std::ifstream is;
    is.open("save/" + filename);
    if (is.fail()){
        return 0;
    }
    std::stringstream data_ss;
    data_ss << is.rdbuf();
    std::string data_str = data_ss.str();
    unsigned long long pos = 0;
    unsigned long long b; unsigned long long l;
    std::string str;
    isolate_matched_brackets(data_str, b, l, pos, 1, '{', '}');
    str = data_str.substr(b,l);
    gamedata.load(str);
    pos = b+l;
    isolate_matched_brackets(data_str, b, l, pos, 1, '{', '}');
    str = data_str.substr(b,l);
    playerdata.load(str);
    pos = b+l;
    isolate_matched_brackets(data_str, b, l, pos, 1, '{', '}');
    std::string world_string;
    str = data_str.substr(b,l);
    worlddata.load(str,gamedata.seed);
    pos = b+l;
    is.close();
    gamedata.gaming = true; // we're so gaming.
    return 1;
}

bool export_data(const std::string filename, base_data & gamedata, player_data & playerdata, world_data & worlddata){
    std::ofstream os;
    os.open("save/" + filename);
    if (os.fail()){return 0;}
    std::string o;
    o+="base game data:\n{\n";
    std::string str;
    gamedata.bd_exp(str);
    o+= str + "\n}\nplayer:\n{\n";
    playerdata.pd_exp(str);
    o+= str + "\n}\nworld:\n{\n";
    worlddata.wd_exp(str);
    o+= str + "\n}";
    os << o;
    os.close();
    return 1;
}
