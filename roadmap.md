# roadmap
basically, todos.

## HIGH PRIORITY
- player special items export
- rollall repetition argument
- rework enemy generation, because currently it's very silly.
- buff enemies' hp if they are underleveled for this floor
- dice pool non-test implementation
- check over tags again
- site variation should be stored more efficiently: the grammar is a very large and memory intensive thing to keep copying. 

## LOW PRIORITY
- cod talismans, the cavern of the cod and final boss implementation (https://discord.com/channels/869096504063823912/1417036358186434641/1431545268435816468): The Cod(father|mother|parent) (random chosen name among the three based on seed)
 - associated noitastyle orbs that scale hp?
- capitalize multi word morphemes properly
- drops
 - rework maybe? idk. fine tuning.
- shop item tiers?
- optimize saving and loading
- change maps to unordered maps

## SEMI COMPLETE
- inventory items & strategy
 (- ways to get these -> not implemented. add some more generic ones to shop, e.g.)
 (- more general initialization upon "enter"ing site -> not implemented)
 - full heal, when not in combat (hence you must determine in advance when you need a hit to maximize value) -> implemented
 - consumables that heal/deal damage/save from fail/etc
  (- save from fail -> implemented)
  - extra life that restores you to nonzero hp if you die -> implemented
  - first aid kit -> implemented
 - permanent buff items/baubles -> not implemented
 - perks? -> not implemented
 - skip to next floor, with risk of failure and instantly ending turn? -> not implemented
- rooms with non-enemy content -> buff rooms exist, but are kind of minimal
- saving and loading (unless Things Bad)
- shop dice
 - rng produces new shop items, only advances after purchasing from the shop, idk.

## COMPLETE
- handle imports of nonexistent files etc
- dice export format: change the char to a string, and dice type to a string too.
- more complex grammars
- ingame wiki
 - implement enemy info -> done
- nonlinear site progression format?


## FUTURE ROADMAP IDEAS?
- typed damage and element system
 - enemy weakness and resistance to specific types
 - could also interplay with baubles
 - word tags (geoduck #earth #water for example) to associate enemies with specific types of area etc
- more dice types:
 - risk-mitigating (if successful, prevents failure this turn)
 - coin and/or xp farming-oriented
- hardcore/permadeath mode? adjust difficulty to make this possible
