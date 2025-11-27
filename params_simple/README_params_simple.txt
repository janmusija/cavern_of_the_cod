put the params_simple/ folder in your working directory. you will also want to have a save/ folder there.

--- dice_types_rudimentary.txt ---
[this is really just for test purposes. an actual version will have things weighted so that less common dice are less common until high values, or sth]
dice types that appear in enemies.
format:
approx average damage / multiplier*2 / hp
type;multiplier;fail threshold;faces separated by commas;description

nothing is weighted because this is really supposed to be a test thing

--- dict.txt ---
each line is:
word @type1@type2@type3#tag1#tag2#tag3
(tags are optional)
type -> word classes. these can be seen by subs.txt. For example, a -> adjectives, so @a marks that a word is an adjective. You can add your own types if you want, because I've made this very general with the power of code everyone hates
tags -> associated concepts (if [it begins with] lowercase), used for determining local type etc (e.g. earth, fire), if [it begins with] uppercase instead it is for some other purpose (e.g. #RARE denotes rarity)


lines can also be
//comment (these only work if they're the full line! no comments in the same line as content, sorry.)
do not use {braces} in this file unless you like having a broken save
//things can act up if there is a file with no untyped entries

--- abbreviations ---
flags at head of subsitution:
$ -> force this to be a non-base substituion
_ -> force this to be a base substitution
$_ -> force this to be both
(otherwise, will be determined based on presence of $)
these do not have a space between them and the name of the substitution!! e.g. stylize it "$_np" to make a np forced to be base AND non-base

[in substitution body]
a -> adjective
av -> adverb
e -> enemy (noun)
s -> site (noun)
mb -> miniboss (noun)
n -> noun (generic, includes e and s)
p -> person
l -> location
v -> verb
ca -> cod-specific adjectives
ce -> cod specific enemy nouns

$[str] -> substitution in subs.txt in the line beginning with [str].
:[str] -> "[str]" itself
_ -> " " (space)
^c -> mark to be caps
^l -> mark to be lowercase
^r -> respect initial doc's case
^+n -> increase budget of following substitution by n. only happens if there is a substitution afterwards, otherwise can error badly.

--- subs.txt ---
each line is:
[str] series of abbreviations
which indicates that $[str] can be subbed for said abbreviations.
special case note: sp and ep (by default) are used to construct site and enemy names respectively. Which are used can be changed by changing grammar_bank::s_struc() and grammar_bank::e_struc() respectively in nomenclature.cpp

do not use {braces} in this file (or the other ones) unless you like having a broken save
subs.txt does NOT support #tags

--- old/.. ---
various deprecated files -- most of these would probably need a little bit of editing to fix up

--- for my own purposes ---
I've been using
https://www.textfixer.com/tools/remove-duplicate-lines-online.php
and
https://www.textfixer.com/tools/sort-lines-alphabetically-online.php
to organize the files