//
//  nomenclature.h
//  ominous dice torment game
//
//  Created by jan musija on 9/14/25.
//

#ifndef nomenclature_h
#define nomenclature_h

#include <stdio.h>
#include <string>
#include "base_info.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

#define MAX_NAME_BUDGET 50 // maximal number of subcalls for names. longer names are possibly obrusive, but probably not really relevant to regular gameplay.

// you'll see why I need the following imminently:
void add_string(std::string const & str, std::unordered_map<std::string,std::vector<std::pair<std::string,std::unordered_set<std::string>>>> & target_dict, std::unordered_set<std::string> & tag_ref);
void add_string(std::string const & input_string, std::vector<std::pair<std::string,std::unordered_set<std::string>>> & target_list, std::unordered_set<std::string> & tag_ref); // for adjs and such
void add_string(std::string const & input_string, std::vector<std::vector<std::string>> & target_list); // for e_nouns and other format type vecs
void add_string(std::string const & input_string, std::map<std::string,std::vector<std::vector<std::string>>> & target_map); // for subs

class grammar_bank { // this used to be a "simple version" that "generates names in less involved way".
public:
    grammar_bank(); // blank
    void open(std::string dict_file, std::string sub_file); // load words and grammars
    
    // these are going to be removed soon after I change how stuff is stored; they're not used for the base_gram and I'm going to make site_gb not store an entire grammar_bank and just have information that can be connected to the worlddata's grammar, to improve storage efficiency and stuff ig.
    void remove_tagged(const std::string & tag); // remove tagged words
    void remove_tagged(const std::unordered_set<std::string> & tagset); // remove tagged words within set
    
    
    bool load(const std::string datastring); // load from string. bool is success
    /*
     format is, basically:
     {subs.txt}
     {
     {a.txt}
     {av.txt}
     {e.txt}
     {l.txt}
     {n.txt} [full version; and so the import is slightly different]
     {p.txt}
     {s.txt}
     {v.txt}
     }
     */
    void gb_exp(std::string &exp) const; // export
    
    /* deprecated
    std::string global_construct_from_string(const int seed, const std::vector<std::string> struc, unsigned int budget) const;
private:
    std::string global_cfs_pass_mt(std::mt19937 & rng, const std::vector<std::string> struc, unsigned int budget, bool capall, bool respect) const; // don't use this. construct_from_string instead.
     */
public:
    //contents
    std::unordered_set<std::string> all_tags;
    
    std::unordered_map<std::string,std::vector<std::pair<std::string,std::unordered_set<std::string>>>> dict; // my code is perfect. I will hear no complaints
    /*
     a -> adjs
     av -> enemy nouns
     l -> locations
     mb -> miniboss
     n -> all nouns
     p -> people
     s -> s_nouns
     v -> verbs (present participles
     */
    /*
    std::vector<std::pair<std::string,std::unordered_set<std::string>>> adjs; // a
    std::vector<std::pair<std::string,std::unordered_set<std::string>>> adverbs; // av
    std::vector<std::pair<std::string,std::unordered_set<std::string>>> e_nouns; // e
    std::vector<std::pair<std::string,std::unordered_set<std::string>>> loc; // l
    std::vector<std::pair<std::string,std::unordered_set<std::string>>> miniboss; //mb
    std::vector<std::pair<std::string,std::unordered_set<std::string>>> all_nouns; // *should* contain all of e and s, or Jank. can be invoked as "n". there is also n.txt
    std::vector<std::pair<std::string,std::unordered_set<std::string>>> ppl; // p
    std::vector<std::pair<std::string,std::unordered_set<std::string>>> s_nouns; // s
    std::vector<std::pair<std::string,std::unordered_set<std::string>>> verbs; // v
     */
    
    //rec determines if recursive (subs) or not (base_subs)
    std::vector<std::vector<std::string>>* e_struc(bool rec); // enemy structure. currently, string is "ep".
    std::vector<std::vector<std::string>>* s_struc(bool rec); // site structure. currently, string is "sp".
    std::vector<std::vector<std::string>>* mb_struc(bool rec); // site structure. currently, string is "bnp".
    
    std::map<std::string,std::vector<std::vector<std::string>>> subs; // substitutions
    std::map<std::string,std::vector<std::vector<std::string>>> base_subs; // substitutions with no further substitutions ($)
};

std::string capitalize(std::string i, bool b, bool resp); // bool resp: override and return i as it was inputted. bool b: if 1, capitalize; if 0, lowercasize.

// misc aux
void remove_tagged_new(const std::vector<std::pair<std::string,std::unordered_set<std::string>>> & wordbank, const std::unordered_set<std::string> & tagset, std::vector<std::pair<std::string,std::unordered_set<std::string>>> & wordbank_new);


class site_gb : public grammar_bank {
public:
    std::unordered_map<std::string,std::vector<int>> pref_indices; // preferred indices
    std::string local_type_pref; // local type
    std::string local_type_dispref; // type weakness
    
    site_gb(const grammar_bank &); // constructor
    site_gb(); // "default constructor"
    
    // funcs
    void randomly_bias(std::mt19937& rng);
    void update_pref_indices(const std::unordered_set<std::string> &);
    void update_pref_indices(const std::string &);
    std::string site_construct_from_string(const int seed, const std::vector<std::string> struc, unsigned int budget) const;
    std::string get_site_name(const std::mt19937& rng, int budget); // in practice these are const, but The.
    std::string get_enemy_name(const std::mt19937& rng, int budget);
    std::string get_boss_name(const std::mt19937& rng, int budget);
private:
    std::string site_cfs_pass_mt(std::mt19937 & rng, const std::vector<std::string> struc, unsigned int budget, bool capall, bool respect) const; // don't use this. construct_from_string instead.
};
 

#endif /* nomenclature_h */
