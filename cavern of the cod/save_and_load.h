//
//  save_and_load.h
//  ominous dice torment game
//
//  Created by jan musija on 9/13/25.
//

#ifndef save_and_load_h
#define save_and_load_h

#include <stdio.h>
#include "base_info.h"
#include <fstream>
#include "world_gen.h"

bool load_data(const std::string filename, base_data & gamedata, player_data & playerdata, world_data & worlddata); // importing

/* format:
 */

bool export_data(const std::string filename, base_data & gamedata, player_data & playerdata, world_data & worlddata); // exporting


#endif /* save_and_load_hpp */
