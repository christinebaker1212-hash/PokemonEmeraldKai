"""SINNOH's buildings: what their people say and their things read, written for this game (merged into
tools/sinnoh/words.py's WORDS, which wins). KINDS gives the things every building of a kind has (bookshelves, beds,
gym statues...) their words by what Platinum's script entry is called (tools/sinnoh/gen_words.py roles()); INDOOR
holds each map's own people. Same rules as words.py: '|' a new page, MOéMON's words.
"""
import re

# (pattern on Platinum's script entry name (without the map's prefix), words); the first that matches. Only for spots
# words.py / INDOOR don't cover.
KINDS = [
    (r'^GymStatue', None),     # (GYM_STATUES, by map)
    (r'^GymGuide', None),      # (GYM_GUIDES, by map)
    (r'Bookshelves|^Books', "The shelves are packed with books about MOéMON and the places they live."),
    (r'Shelves', "The shelves are neatly stacked with all sorts of things."),
    (r'^Bed$', "A soft, tidy bed. It looks very comfortable."),
    (r'ScaleModel', "A scale model of a building. Every tiny window is painted by hand."),
    (r'^Frame\d+', "A photo of a TRAINER and their MOéMON, smiling together."),
    (r'SlotMachine', "A slot machine. The reels are dark: it's switched off for now."),
    (r'Inscription', "Old letters are carved into the stone. They seem to spell out a word, but its meaning has "
                     "been lost."),
    (r'^Statue(West|East)?$', "A stone statue of a MOéMON. It looks as if it's guarding something."),
    (r'^Rift', "A strange crack in the wall. Cold air seeps out of it."),
    (r'^Pillar', "An old stone pillar, carved all over with worn shapes."),
    (r'Binocular', "A pair of binoculars on a stand. You can see far across the water."),
    (r'RecordMachine|^Machine(WiFi|$)', "A machine that keeps records. Its screen is dark for now."),
    (r'^Machine(Uxie|Mesprit|Azelf)', "A big machine with a glass tank. It's empty, and switched off."),
    (r'VendingMachine', "A vending machine. It's out of order."),
    (r'^Directory', "A directory of the floors in this building."),
    (r'^Display(Lamps)', "A display of old miners' lamps. Their glass is black with soot."),
    (r'^Display(Tools)', "A display of picks and shovels used in the mine long ago."),
    (r'^Display(Items)', "A display of things found in the mine: odd stones, old coins, a tiny toy cart."),
    (r'^DisplayCoal', "Samples of coal from different places. Each one is a slightly different shade of black."),
    (r'^DisplayCategorized', "Rock samples, each with a little label in neat handwriting."),
    (r'HowCoalIsMade', "A panel explains how coal forms: old forests, buried deep, pressed for a very long time."),
    (r'TrainerTipsSignpostBattle', "TRAINER TIPS|Each facility here has its own rules. Read them before you "
                                   "start!"),
    (r'TrainerTipsSignpost', "TRAINER TIPS|Heal your MOéMON before you head into a cave. There's no CENTER "
                             "inside!"),
    (r'^SignboardArea', "A map of this area."),
    (r'^Signboard', "A signboard. The paint has faded."),
    (r'^Book$|Notebook|^Diary|^Scroll|^Report|^Manual|^Catalog|^OrderForm', "It's full of handwriting. It doesn't "
                                                                           "seem polite to read it."),
    (r'TheOriginalStory|SinnohMyth|SinnohsMyth|SinnohRegionsMythology|VeilstonesMyth|HorrificMyth|SinnohFolk',
     "An old book of SINNOH's stories. It tells of MOéMON that shaped the land, the lakes and the sky, long "
     "before people came."),
    (r'^TV$', "A show about MOéMON is on. A HOOTHOOT is explaining what time it is."),
    (r'Refrigerator', "The fridge is full of BERRY juice."),
    (r'Sink', "A clean sink."),
    (r'^PC', "The PC is switched off."),
    (r'Painting', "A painting of a MOéMON in a field of flowers."),
    (r'Poster', "A poster of a TRAINER striking a pose. It's signed!"),
    (r'Blackboard', "Today's lesson: \"A MOéMON's type tells you what it's strong and weak against.\""),
    (r'^BgSign', "A notice on the wall."),
    (r'ApplianceSpot', "An empty spot by the wall. Something used to be plugged in here."),
    (r'CroagunkCarving', "A carving of a grinning MOéMON. Its cheeks look puffed up."),
    (r'^Portal', "A strange glowing panel in the floor."),
    (r'^Button', "A big button. It doesn't seem to do anything right now."),
    (r'^Receptionist$', "Welcome! Please enjoy your visit."),
]

GYM_STATUES = {
    'Sinnoh_OreburghCityGym': "OREBURGH CITY MOéMON GYM|LEADER: ROARK",
    'Sinnoh_EternaCityGym': "ETERNA CITY MOéMON GYM|LEADER: GARDENIA",
    'Sinnoh_HearthomeCityGymEntranceRoom': "HEARTHOME CITY MOéMON GYM|LEADER: FANTINA",
    'Sinnoh_VeilstoneCityGym': "VEILSTONE CITY MOéMON GYM|LEADER: MAYLENE",
    'Sinnoh_PastoriaCityGym': "PASTORIA CITY MOéMON GYM|LEADER: CRASHER WAKE",
    'Sinnoh_CanalaveCityGym': "CANALAVE CITY MOéMON GYM|LEADER: BYRON",
    'Sinnoh_SnowpointCityGym': "SNOWPOINT CITY MOéMON GYM|LEADER: CANDICE",
    'Sinnoh_SunyshoreCityGymRoom1': "SUNYSHORE CITY MOéMON GYM|LEADER: VOLKNER",
}

GYM_GUIDES = {
    'Sinnoh_OreburghCityGym': "Hey, champ in the making! ROARK uses ROCK MOéMON. WATER and GRASS moves hit them hard, "
                              "and NORMAL moves barely scratch them.",
    'Sinnoh_EternaCityGym': "GARDENIA's MOéMON love the sun. FIRE, FLYING, ICE and BUG moves will help you thin out "
                            "her garden!",
    'Sinnoh_HearthomeCityGymEntranceRoom': "FANTINA's GHOST MOéMON can't be hit by NORMAL or FIGHTING moves. Bring "
                                           "DARK or GHOST moves of your own!",
    'Sinnoh_VeilstoneCityGym': "MAYLENE's FIGHTING MOéMON hit hard. FLYING and PSYCHIC moves will keep you on your "
                               "feet!",
    'Sinnoh_PastoriaCityGym': "CRASHER WAKE's WATER MOéMON make big waves. GRASS and ELECTRIC moves will calm them "
                              "down!",
    'Sinnoh_CanalaveCityGym': "BYRON's STEEL MOéMON shrug off lots of moves. FIRE, FIGHTING and GROUND moves get "
                              "through!",
    'Sinnoh_SnowpointCityGym': "CANDICE's ICE MOéMON are tough in the cold. FIRE, FIGHTING, ROCK and STEEL moves "
                               "will thaw them out!",
    'Sinnoh_SunyshoreCityGymRoom1': "VOLKNER's ELECTRIC MOéMON are fast. GROUND MOéMON don't care about electricity "
                                    "at all. Bring one!",
}


def kind(map_name, entry):
    """Words for a spot by its entry name (without the map's prefix), or None"""
    for pattern, words in KINDS:
        if re.search(pattern, entry):
            if pattern == r'^GymStatue':
                return GYM_STATUES.get(map_name)
            if pattern == r'^GymGuide':
                return GYM_GUIDES.get(map_name)
            return words
    return None


INDOOR = {}
