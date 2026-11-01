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


INDOOR = {
    # ------------------------------------------------------------------ JUBILIFE CITY
    'Sinnoh_JubilifeCityMart': {
        'Person1': "I buy new guitar strings here. The clerk always asks if I want a POTION too. I always say yes.",
        'Person2': "JUBILIFE's MART is the busiest in SINNOH. Come early or wait in line!",
        'Person6': "I'm stocking up before I head west. You never know what the road will throw at you.",
    },
    'Sinnoh_JubilifeCityPokecenter1f': {
        'Person1': "The nurse here remembers every TRAINER's name. How does she do it?",
        'Person2': "I came here to watch TRAINERS heal up. It's how I learn which MOéMON are strong!",
        'Person3': "My MOéMON got tired from playing tag. Now it's resting, and I'm waiting!",
    },
    'Sinnoh_PoketchCo1f': {
        'Person1': "We design watches for TRAINERS here. Each one has little apps that help on a journey.",
        'Person5': "I'm here for a tour. The newest models are on the floors above!",
    },
    'Sinnoh_PoketchCo2f': {
        'Person1': "I've worn the same watch for ten years. I just keep adding apps.",
        'Person2': "In my day, a watch told you the time and that was that. Now it tells you everything!",
        'Person3': "CLEFAIRY: Pippi pi!|It's tapping on a watch face, fascinated.",
        'Person4': "PACHIRISU: Chipa!|It's chewing on a watch strap.",
    },
    'Sinnoh_PoketchCo3f': {
        'Person1': "I test the apps. My favorite one counts my steps. I'm up to a million!",
        'Person2': "I design the faces of the watches. Every one has a tiny MOéMON hidden somewhere on it.",
        'Person3': "The president of the company? He's always out walking. Says it's where he gets ideas.",
    },
    'Sinnoh_JubilifeTv1f': {
        'Person1': "I'm here for an audition! My MOéMON is going to be a star.",
        'Person2': "Welcome to JUBILIFE TV! Feel free to look around. The upper floors have our studios.",
        'Person3': "I've watched every battle show they've ever aired. I know all the commentators by name.",
        'Person4': "I came down from the north just to see the station. It's so warm down here!",
        'Person5': "They say TRAINERS who win big battles get interviewed upstairs. Someday that'll be me.",
    },
    'Sinnoh_JubilifeTv2f': {
        'Person1': "I'm going to sing on TV with my MOéMON! We've practiced our harmony for weeks.",
        'Person2': "This floor has our gallery. Viewers send us photos of their MOéMON!",
        'Person4': "I came to complain about a show. Then I watched it again and liked it. Now I'm just visiting.",
    },
    'Sinnoh_JubilifeTv3f': {
        'Person1': "The ranking rooms show who's done the most in SINNOH. My name's on the list! Somewhere.",
        'Person2': "We film the weather report from this floor. The forecast? Sunny with a chance of MOéMON.",
        'Person3': "Quiet, please! We're about to go live.",
        'Person4': "The rankings change every day. TRAINERS all over SINNOH are working hard.",
    },
    'Sinnoh_JubilifeTv4f': {
        'Person1': "I'm the producer. My job is to make sure every show is fun. And on time. Mostly fun.",
        'Person3': "I'm here to talk about raising MOéMON on a cooking show. My HAPPINY loves POFFINS.",
    },
    'Sinnoh_JubilifeTv2fGallery': {
        'Person1': "I sent in a photo of my MOéMON at the top of MT. CORONET. I hope they hang it up!",
        'Person2': "Every photo here tells a story. That one with the sleeping SNORLAX is my favorite.",
        'Person3': "Look! That's my picture! The one with the flowers!",
    },
    'Sinnoh_JubilifeTv3fGlobalRankingRoom': {
        'Person1': "I'm in the rankings for the biggest MAGIKARP. It's not much, but it's mine.",
        'Person2': "These machines show TRAINERS from all over. One day they'll show the whole world.",
        'Person3': "I check the rankings every week. I like to see who's catching up.",
    },
    'Sinnoh_JubilifeTv3fGroupRankingRoom': {
        'Person1': "My friends and I made a group. We're going to beat everyone's records together!",
        'Person2': "Groups train together and climb together. That's how you get to the top!",
    },
    'Sinnoh_JubilifeCitySouthHouse1f': {
        'Person1': "My sister and I trade MOéMON so they can evolve. It's like a little exchange program.",
        'Person2': "I want a PACHIRISU of my own. This one belongs to my brother, but it likes me more.",
        'Person3': "PACHIRISU: Chipa chipa!|It's running in circles.",
    },
    'Sinnoh_JubilifeCitySouthHouse2f': {
        'Person1': "I'm doing my homework. It's about MOéMON types. I already know all of them!",
        'Person2': "My son spends every day at the TRAINERS' SCHOOL. I'm so proud.",
        'Person3': "We have a little garden on the balcony. My MOéMON water it for me.",
    },
    'Sinnoh_JubilifeCityCondominiums1f': {
        'Person1': "Everyone here has a MOéMON. The hallways are full of them in the evening!",
        'Person2': "PACHIRISU: Chipa?|It's staring at you curiously.",
        'Person3': "PIKACHU: Pika pika!|Its cheeks crackle happily.",
        'Person4': "I just moved in. My neighbors are so friendly, and so are their MOéMON.",
    },
    'Sinnoh_JubilifeCityCondominiums2f': {
        'Person1': "I keep my bug collection in my room. My mom says it's very... educational.",
        'Person2': "I raise MOéMON for TRAINERS who travel a lot. It's like a little day care!",
    },
    'Sinnoh_TrainersSchool': {
        'Person1': "Welcome to the TRAINERS' SCHOOL! Feel free to look around and read the boards.",
        'Person3': "We learned about status today. A sleeping MOéMON can't move! Unless it snores, I think.",
        'Person4': "The teacher says a good TRAINER watches the battle, not just their own MOéMON.",
        'Person5': "I'm the best in my class at throwing POKé BALLS. Well, the best at hitting the wall.",
        'Person7': "My notes are full of type charts. I drew little MOéMON in the margins too.",
        'Person8': "I want to be a gym leader when I grow up! I just have to pick a type.",
    },
    'Sinnoh_JubilifeCitySouthwestHouse1f': {
        'Person1': "My PIKACHU loves the TV station's music. It dances every time the news comes on.",
        'Person2': "PIKACHU: Pika!|It's wagging its tail to music only it can hear.",
        'Person3': "I worked at the TV station for thirty years. Now I just watch it.",
    },
    # ------------------------------------------------------------------ CANALAVE CITY
    'Sinnoh_CanalaveCityMart': {
        'Person1': "Sailors buy rope here, I buy POTIONS. We both leave happy.",
        'Person2': "The prices are a bit higher in CANALAVE. Everything comes by boat, you see.",
    },
    'Sinnoh_CanalaveCityPokecenter1f': {
        'Person1': "Travelers from the boats always stop here first. The nurse has seen it all.",
        'Person3': "My MOéMON got seasick on the ferry. A rest here fixed it right up!",
        'Person6': "I'm going to IRON ISLAND to catch STEEL MOéMON. Wish me luck!",
        'Person7': "I study the old books in the library. The ones on the third floor are the oldest.",
    },
    'Sinnoh_CanalaveLibrary1f': {
        'Person2': "I come here every day after school. It's so quiet... you can hear the pages turn.",
    },
    'Sinnoh_CanalaveLibrary2f': {
        'Person1': "I'm reading every book about MOéMON in this library. I'm on shelf two. Of fifty.",
    },
    'Sinnoh_CanalaveLibrary3f': {
        'Person1': "The oldest stories in SINNOH say three great MOéMON live in the three lakes.|"
                   "Some say they gave people knowledge, feelings and willpower. I think it's true.",
        'Person2': "I'm looking for a book about the strongest MOéMON ever. All the books say different things!",
    },
    'Sinnoh_CanalaveCitySoutheastHouse': {
        'Person1': "My dad's a sailor. He brings me a shell from every place he goes.",
        'Person2': "I train on the docks at sunrise. The ships' horns are my alarm clock.",
    },
    'Sinnoh_CanalaveCityEastHouse': {
        'Person1': ('script', 'LilycoveCity_MoveDeletersHouse_EventScript_MoveDeleter'),
    },
    'Sinnoh_CanalaveCitySailorEldritchHouse': {
        'Person1': "My dad's boat goes to IRON ISLAND! And sometimes even farther.",
        'Person2': "My husband loves the sea more than land. He'd sleep on his boat if I let him.",
    },
    # ------------------------------------------------------------------ OREBURGH CITY
    'Sinnoh_OreburghCityMart': {
        'Person1': "Miners buy lots of ESCAPE ROPES. Nobody wants to get lost underground.",
        'Person2': "I'm buying REPELS. I want to collect fossils in the mine, not battle ZUBAT!",
    },
    'Sinnoh_OreburghCityPokecenter1f': {
        'Person1': "I can sense... that your MOéMON are tired. That's why you're here, right?",
        'Person2': "ROARK uses ROCK MOéMON. My GRASS MOéMON had a great time in that battle!",
        'Person3': "I'm waiting for my friend to finish in the mine. He's been down there all day.",
        'Person5': "I've visited this MOéMON CENTER for forty years. The chairs have never been comfier.",
        'Person8': "Are you here to trade? I've got a GEODUDE, and I'm not afraid to use it!",
        'Person9': "We're linking up to battle! Our MOéMON are fighting in a tiny screen world.",
    },
    'Sinnoh_OreburghCityNorthwestHouse1f': {
        'Person1': "My MOéMON and I do the dishes together. It splashes, I dry.",
        'Person2': "My dad says I can go into the mine when I'm older. I'm already old enough!",
        'Person3': "Coal dust gets everywhere in this town. I've stopped fighting it.",
    },
    'Sinnoh_OreburghCityNorthwestHouse2f': {
        'Person1': "After a long day in the mine, there's nothing like a nap with my PSYDUCK.",
        'Person2': "I'm drawing a map of the mine. It's mostly squiggly lines so far.",
        'Person3': "Did you see the GYM? It's carved right into the rock. I think it's so cool.",
        'Person4': "PSYDUCK: Psy...|It's taking a nap. Or thinking very hard. Hard to tell.",
    },
    'Sinnoh_OreburghCityNorthHouse1f': {
        'Person1': "My husband works in the mine. I keep his dinner warm until he gets home.",
        'Person2': "When I grow up I'm going to find a fossil and bring it back to life!",
        'Person3': "I raise ONIX. They're huge, but they're gentle as anything.",
    },
    'Sinnoh_OreburghCityNorthHouse2f': {
        'Person1': "I'm writing a song about OREBURGH. It goes \"Dig, dig, dig, coal, coal, coal.\" Needs work.",
        'Person2': "This is my quiet place. Downstairs gets so busy when the miners come home.",
        'Person3': "I collect rocks. These ones are from the mine. That one's from the beach. I like them all.",
    },
    'Sinnoh_OreburghCityMiddleHouse': {
        'Person1': "My grandpa says the mine used to be a forest a long, long time ago.",
        'Person2': "I've dug in that mine for thirty years. Still find something new every week.",
    },
    'Sinnoh_MiningMuseum': {
        'Person1': "Before machines, MOéMON helped carry the coal. MACHOP were the best workers.",
        'Person2': "Look at the old helmets! They had little candles on them!",
        'Person3': "I'm doing a school report on the mine. I've written three pages about rocks!",
        'Person4': "I study fossils from the mine. Bring me any you find, and I'll tell you all about them.",
        'Person5': "The museum tells OREBURGH's story so well. I learned so much!",
        'Person6': "My MOéMON loves the coal display. It keeps sniffing it.",
        'Person7': "I came for the fossil corner. It's smaller than I hoped, but it's very good.",
        'Person8': "I worked in the mine when I was young. Seeing these old tools brings it all back.",
    },
    'Sinnoh_OreburghCityWestHouse': {
        'Person1': "My PIKACHU and I are going to beat ROARK next week. We've been practicing!",
        'Person2': "My son thinks his PIKACHU can beat ROCK MOéMON. I don't have the heart to tell him.",
        'Person3': "PIKACHU: Pika pi!|It seems very determined.",
    },
    'Sinnoh_OreburghCityEastHouse1f': {
        'Person1': "Welcome! My house is your house. Well, not really, but come in anyway!",
        'Person2': "I'm back from the mine. Time to wash up. And then wash up again.",
        'Person3': "I'm heading to ETERNA next. I heard the forest there is full of BUG MOéMON.",
    },
    'Sinnoh_OreburghCityEastHouse2f': {
        'Person1': "Did you know coal comes from plants? I didn't believe it either!",
        'Person2': "I've traveled all over SINNOH. OREBURGH's people are the hardest working of all.",
        'Person3': "I'm researching how to burn coal more cleanly. It's slow work, but it matters.",
    },
    'Sinnoh_OreburghCitySouthHouse': {
        'Person1': "My shift starts in an hour. I'm getting a little nap in first. Shh.",
    },
    # ------------------------------------------------------------------ ETERNA CITY
    'Sinnoh_EternaCityMart': {
        'Person1': "I write songs about the forest. The MART sells everything except inspiration.",
        'Person2': "Stock up on ANTIDOTES before ETERNA FOREST. The BUG MOéMON there love POISON.",
    },
    'Sinnoh_EternaCityGym': {
        'Person3': "GARDENIA's GYM is like a garden inside a clock. Isn't that pretty?",
        'Person4': "I tend the flowers in this GYM. They're almost as strong as GARDENIA's MOéMON.",
        'Person5': "Breathe in! Smell that? GRASS MOéMON give off the nicest scents.",
    },
    'Sinnoh_EternaCityPokecenter1f': {
        'Person1': "The statue in town is of a MOéMON from a long-ago legend. Nobody knows which one for sure.",
        'Person2': "I'm going to challenge GARDENIA! My FIGHTING MOéMON... wait, is that a good idea?",
        'Person5': "ETERNA's flowers are lovely, but the MOéMON CENTER's flowers are my favorite.",
    },
    'Sinnoh_EternaCityPokecenter2f': {
        'Person4': "I'm waiting for a friend to trade with. He's always late!",
    },
    'Sinnoh_CycleShop': {
        'Person2': "I want a bike so badly! I've been saving up for... a while. A long while.",
    },
    'Sinnoh_EternaCityCondominiums1f': {
        'Person1': "There's an old man upstairs who knows everything about ETERNA's legends.",
        'Person2': "I've lived in this building for thirty years. The views of the forest never get old.",
        'Person3': "Every family in this building has a MOéMON. Some have five!",
    },
    'Sinnoh_EternaCityCondominiums2f': {
        'Person1': "My grandson reads me the old stories every night. I know them all by heart now.",
        'Person2': "I'm playing hide and seek with my MOéMON! Shh! Don't tell it where I am!",
        'Person3': "They say the statue in town was put up when ETERNA was founded. That's a long time.",
    },
    'Sinnoh_EternaCityCondominiums3f': {
        'Person1': "My MOéMON knows the way home from anywhere in ETERNA. Better than I do!",
        'Person2': "I'm studying for a test about SINNOH history. Did you know ETERNA is really old?",
        'Person3': "I found a SILCOON in the forest! It's not moving, but I think it's happy.",
        'Person4': "I'm practicing sneaking. You didn't hear me come in, did you? ...Did you?",
    },
    'Sinnoh_EternaCityHerbShop': {
        'Person1': "Our herbs are bitter, but they work wonders. MOéMON just don't like the taste.",
        'Person3': "I grow all the herbs myself. The trick is to talk to them nicely.",
    },
    'Sinnoh_EternaCityEastHouse': {
        'Person1': "My wife and I walk through the forest every Sunday. We've never seen the same thing twice.",
        'Person2': "The forest is full of stories. Some are true. Some are even better.",
        'Person3': "I raise MOéMON that like the shade. ETERNA's the perfect town for that.",
    },
    # ------------------------------------------------------------------ HEARTHOME CITY
    'Sinnoh_HearthomeCityMart': {
        'Person2': "I came down from the mountains to shop. The city's prices make my head spin!",
        'Person3': "I'm buying gifts for my MOéMON. It's our anniversary. One year together!",
    },
    'Sinnoh_HearthomeCityPokecenter1f': {
        'Person2': "HEARTHOME is the friendliest city in SINNOH. Even the nurse says so!",
        'Person3': "I'm here to show off my MOéMON at the CONTEST HALL. Darling, isn't it beautiful?",
        'Person4': "I'm going to play a song for the FAN CLUB. They love it when I play!",
    },
    'Sinnoh_HearthomeCitySoutheastHouse1f': {
        'Person1': "My SHROOMISH has been with me since I was a boy. It's slow, but so am I now.",
        'Person2': "We moved to HEARTHOME for the CONTEST HALL. My daughter wants to be a star!",
        'Person3': "SHROOMISH: Shroo...|It's sleeping on the warm floor.",
        'Person4': "I'm going to be in a CONTEST someday! I'm practicing my wave.",
    },
    'Sinnoh_HearthomeCitySoutheastHouse2f': {
        'Person1': "My CLEFAIRY dances every night under the moonlight. It's magical!",
        'Person2': "CLEFAIRY: Pi pi pippi!|It's twirling around happily.",
    },
    'Sinnoh_HearthomeCityPokemonFanClub': {
        'Person1': "Welcome to the MOéMON FAN CLUB! Tell me all about your MOéMON. I'll listen all day!",
        'Person2': "Every member here loves MOéMON more than anything. We have weekly meetings about it.",
        'Person3': "PSYDUCK: Psy?|It's tilting its head, very confused.",
        'Person4': "BUNEARY: Bun bun!|It's bouncing on its toes.",
        'Person5': "CLEFAIRY: Pippi!|It waves its little hand at you.",
    },
    'Sinnoh_HearthomeCityWestGateToAmitySquare': {
        'Person2': "AMITY SQUARE is lovely! Walk with your MOéMON and see what it finds.",
        'Person3': "PIKACHU: Pika!|It's eager to go into the park.",
    },
    'Sinnoh_HearthomeCityEastGateToAmitySquare': {
        'Person2': "I take my MOéMON to the square every morning. It's our time to relax together.",
    },
    'Sinnoh_Route208GateToHearthomeCity': {
        'Person1': "HEARTHOME's just through here. Smell the POFFINS? That's the city calling.",
        'Person2': "I'm visiting my daughter in HEARTHOME. She runs a little flower stand.",
        'Person3': "CLEFAIRY: Pi!|It's bouncing up and down.",
    },
    'Sinnoh_Route209GateToHearthomeCity': {
        'Person1': "The road to SOLACEON is long but pretty. Watch for the old tower!",
    },
    'Sinnoh_Route212GateToHearthomeCity': {
        'Person1': "It's always raining on the road south. Bring an umbrella!",
        'Person2': "My MOéMON doesn't mind the rain. It actually loves it. Wish I did!",
    },
    'Sinnoh_HearthomeCityNortheastHouse1f': {
        'Person1': "Kids from all over town come here to play. It's never quiet!",
        'Person3': "PIKACHU: Pika chu!|It's playing tag with the children.",
        'Person4': "I'm teaching the kids to battle. Well, to play-battle. No real moves!",
        'Person5': "I'm hiding from my friends! This is the best spot in the house.",
        'Person6': "I'm going to be a MOéMON CONTEST champion! Watch my pose! ...Ta-da!",
        'Person7': "I'm reading a story to my MOéMON. It falls asleep every time.",
        'Person8': "I'm drawing everybody's MOéMON. Yours too, if you hold still!",
    },
    'Sinnoh_HearthomeCityNortheastHouse2f': {
        'Person1': "We've been married fifty years. Our MOéMON have been friends for just as long.",
        'Person2': "HEARTHOME has changed so much since we were young. But it's still home.",
        'Person3': "I tell my grandkids stories about the old days. They pretend to listen.",
        'Person4': "My husband snores like a SNORLAX. I love him anyway.",
    },
    'Sinnoh_HearthomeCityNorthwestHouse': {
        'Person1': "I'm the one who keeps the PC storage system running here in SINNOH.|"
                   "Every MOéMON you put in a box is safe with us. Treat them well!",
    },
    'Sinnoh_ForeignBuilding': {
        'Person1': "This building was built by people from far away. It still has their style.",
        'Person2': "I come here to pray for my old MOéMON. It's peaceful.",
        'Person3': "The stained glass is so pretty when the sun shines through!",
        'Person4': "The colored windows tell a story about MOéMON and people living together.",
        'Person5': "I come here to sit and think before big battles.",
        'Person6': "My grandmother was married here. So was my mother. Maybe someday, me!",
        'Person7': "Such a lovely building. The bells ring every evening.",
        'Person8': "I can feel... calm here. Very calm. Very... zzz.",
    },
}
