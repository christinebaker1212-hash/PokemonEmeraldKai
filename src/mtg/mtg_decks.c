// Moemon Kai: the Magic starter decks and the duelists' decks, built from the cards the engine plays (every card
// here is checked by tools/mtg/test.sh: none may be missing or unsupported). 40 cards each: 17 lands, 23 spells.
#include <stddef.h>
#include "mtg/collection.h"

static const char *const sFire[] = {
    "17 Mountain", "2 Raging Goblin", "2 Mogg Fanatic", "2 Canyon Wildcat", "2 Shock", "1 Lightning Bolt",
    "2 Volcanic Hammer", "2 Goblin Hero", "2 Gray Ogre", "2 Hurloon Minotaur", "2 Hill Giant", "2 Lowland Giant",
    "2 Fire Elemental", NULL};
static const char *const sTide[] = {
    "17 Island", "2 Storm Crow", "2 Coral Merfolk", "2 Bay Falcon", "2 Wind Drake", "2 Horned Turtle",
    "2 Prodigal Sorcerer", "2 Unsummon", "1 Counterspell", "2 Remove Soul", "2 Giant Octopus", "2 Azure Drake",
    "1 Air Elemental", "1 Water Elemental", NULL};
static const char *const sShadow[] = {
    "17 Swamp", "2 Bog Imp", "2 Black Knight", "2 Feral Shadow", "2 Scathe Zombies", "2 Terror", "2 Vicious Hunger",
    "2 Python", "2 Moor Fiend", "2 Bog Wraith", "2 Gluttonous Zombie", "1 Dark Banishing", "1 Specter's Wail",
    "1 Bog Raiders", NULL};
static const char *const sGrove[] = {
    "17 Forest", "2 Llanowar Elves", "2 Grizzly Bears", "2 Pygmy Razorback", "2 Giant Growth", "2 Trained Armodon",
    "2 Gorilla Warrior", "2 War Mammoth", "2 Giant Spider", "2 Bull Hippo", "2 Spined Wurm", "1 Craw Wurm",
    "1 Stream of Life", "1 Monstrous Growth", NULL};
static const char *const sDawn[] = {
    "17 Plains", "2 Suntail Hawk", "2 Youthful Knight", "2 Benalish Knight", "2 Armored Pegasus", "2 Pearled Unicorn",
    "2 Pegasus Charger", "2 Razorfoot Griffin", "2 Ekundu Griffin", "2 Regal Unicorn", "1 Swords to Plowshares",
    "1 Serra Angel", "1 Border Patrol", "1 Iron Tusk Elephant", "1 Angelic Page", NULL};

const MtgRecipe gMtgStarters[MTG_STARTERS] = {
    {"EMBER", "Red: fast creatures and burn spells that hit hard.", MTG_COLOR_BIT(MTG_R), sFire},
    {"TIDEWATER", "Blue: fliers, tricks, and spells that stop spells.", MTG_COLOR_BIT(MTG_U), sTide},
    {"NIGHTFALL", "Black: creatures that sneak past, and removal.", MTG_COLOR_BIT(MTG_B), sShadow},
    {"WILDWOOD", "Green: big creatures, and the mana to cast them.", MTG_COLOR_BIT(MTG_G), sGrove},
    {"DAYBREAK", "White: knights and fliers that strike first.", MTG_COLOR_BIT(MTG_W), sDawn},
};

static const char *const sAugust[] = {
    "8 Plains", "8 Island", "2 Suntail Hawk", "2 Storm Crow", "2 Armored Pegasus", "2 Wind Drake",
    "2 Pegasus Charger", "2 Razorfoot Griffin", "2 Azure Drake", "2 Serra Angel", "1 Air Elemental",
    "2 Counterspell", "2 Swords to Plowshares", "2 Unsummon", "1 Prodigal Sorcerer", NULL};
static const char *const sPip[] = {
    "17 Forest", "3 Llanowar Elves", "2 Fyndhorn Elves", "3 Grizzly Bears", "2 Wild Elephant", "2 War Mammoth",
    "3 Giant Growth", "2 Argothian Swine", "2 Giant Spider", "2 Durkwood Boars", "2 Yavimaya Wurm", NULL};
static const char *const sRhea[] = {
    "8 Mountain", "8 Forest", "2 Raging Goblin", "2 Llanowar Elves", "2 Mogg Fanatic", "2 Pygmy Razorback",
    "2 Shock", "2 Lightning Bolt", "2 Giant Growth", "2 Gorilla Warrior", "2 Hill Giant", "2 Trained Armodon",
    "2 Spined Wurm", "2 Fire Elemental", NULL};
static const char *const sMarlow[] = {
    "8 Swamp", "8 Mountain", "2 Bog Imp", "2 Black Knight", "2 Canyon Wildcat", "2 Terror", "2 Shock",
    "2 Volcanic Hammer", "2 Vicious Hunger", "2 Scathe Zombies", "2 Hurloon Minotaur", "2 Moor Fiend",
    "2 Lowland Giant", "1 Gluttonous Zombie", "1 Lightning Blast", NULL};
static const char *const sTess[] = {
    "8 Island", "8 Swamp", "2 Coral Merfolk", "2 Bog Imp", "2 Wind Drake", "2 Feral Shadow", "2 Prodigal Sorcerer",
    "2 Terror", "2 Remove Soul", "2 Counterspell", "2 Python", "2 Giant Octopus", "2 Dark Banishing",
    "1 Air Elemental", "1 Bog Wraith", NULL};
static const char *const sNolan[] = {
    "17 Plains", "2 Eager Cadet", "3 Youthful Knight", "2 Benalish Knight", "2 Pearled Unicorn", "2 Regal Unicorn",
    "2 Pegasus Charger", "2 Ekundu Griffin", "2 Swords to Plowshares", "2 Iron Tusk Elephant", "2 Serra Angel",
    "2 Border Patrol", NULL};

// MOSSDEEP's CARD SHOP: AUGUST and PIP upstairs, the others in the room below
const MtgDuelist gMtgDuelists[MTG_DUELISTS] = {
    {"AUGUST", "the shop's oldest regular", sAugust,
     "You've a deck at last! Sit down, young one. I've been playing this game since before you were born.",
     "Patience wins in the end. Come back and try me again.", "Ha! Splendid. You learn quickly.", 2},
    {"PIP", "a young regular", sPip,
     "You got a deck? Cool! Wanna play? My elves are really fast!", "My elves did it!",
     "Aw... next time my big wurms will get you!", 0},
    {"RHEA", "a regular who hits hard", sRhea,
     "New face! Let's see what that deck can do. I don't hold back.", "Too slow! Build something faster.",
     "Hah! Good game. You earned that one.", 1},
    {"MARLOW", "a regular who plays dark cards", sMarlow,
     "Every creature you play is one I get to destroy. Shall we?", "Nothing on your side survived. As planned.",
     "Hmph. Your creatures were tougher than they looked.", 2},
    {"TESS", "a regular who counters everything", sTess,
     "Go ahead, cast your spells. I'll be waiting with an answer.", "Countered, and countered again.",
     "You got the spells I couldn't answer. Nice.", 3},
    {"NOLAN", "a regular with a white knight deck", sNolan,
     "My knights are ready for anyone. Draw your cards!", "First strike wins again!",
     "My knights fell... but they'll be back.", 1},
};
