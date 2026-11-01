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
    # ------------------------------------------------------------------ PASTORIA CITY
    'Sinnoh_PastoriaCityMart': {
        'Person2': "I'm buying POKé BALLS for the GREAT MARSH. Wait, you can't use your own there? Oh no...",
        'Person3': "My mom sent me for groceries. I'm buying MOéMON food first, though.",
    },
    'Sinnoh_PastoriaCityPokecenter1f': {
        'Person2': "I train in the marsh. The mud slows you down, so your legs get stronger!",
        'Person3': "CRASHER WAKE waved at me in the street once. He's so loud and nice!",
    },
    'Sinnoh_PastoriaCityPokecenter2f': {
        'Person4': "I'm painting the marsh from the window. It's a different color every hour.",
    },
    'Sinnoh_PastoriaCityObservatoryGate1f': {
        'Person3': "Go up to the observatory! You can see the whole GREAT MARSH from there.",
    },
    'Sinnoh_PastoriaCityObservatoryGate2f': {
        'Person1': "Look through the binoculars! Sometimes you can spot rare MOéMON in the reeds.",
        'Person2': "I saw a CROAGUNK sitting on a rock out there. It was just... grinning.",
        'Person3': "I've been watching the marsh all morning. It's better than TV.",
        'Person4': "The marsh looks so peaceful from up here. It's a lot muddier down there!",
    },
    'Sinnoh_PastoriaCitySouthwestHouse': {
        'Person1': "We grow BERRIES in the backyard. The marsh air makes them extra juicy.",
        'Person2': "I'm going to be a marsh ranger! I'll know every MOéMON out there by name.",
    },
    'Sinnoh_PastoriaCityMiddleHouse': {
        'Person1': "When I was a boy, the marsh came right up to the door. The town's grown a lot.",
        'Person2': "I caught a SKORUPI in the marsh! Okay, it caught me first. Then I caught it.",
    },
    'Sinnoh_PastoriaCityEastHouse': {
        'Person1': "I know every move a MOéMON could ever learn. Ask me about any of them! ...Later. I'm busy.",
        'Person2': "My uncle knows so many moves. He forgets where he put his glasses, though.",
    },
    'Sinnoh_PastoriaCityNorthHouse': {
        'Person1': "I study marsh MOéMON. They're tougher than they look, especially in the deep mud.",
        'Person2': "PSYDUCK: Psy psy...|It's sitting in a puddle, perfectly content.",
        'Person3': "I'm trading with a friend in VEILSTONE! The link cable reaches all the way!",
    },
    'Sinnoh_PastoriaCityNortheastHouse': {
        'Person1': "My sister and I both want to beat CRASHER WAKE. Whoever does it first gets dessert.",
        'Person2': "I'm going to win! My brother still can't get past WAKE's GYARADOS.",
        'Person3': "PACHIRISU: Chi!|It's cheering for someone.",
    },
    # ------------------------------------------------------------------ VEILSTONE CITY
    'Sinnoh_VeilstoneCityPokecenter1f': {
        'Person2': "The department store is the best thing in VEILSTONE. Well, after the MOéMON CENTER, darling.",
        'Person3': "MAYLENE trains every day at the GYM. I watch through the window sometimes!",
    },
    'Sinnoh_GameCorner': {
        'Person1': "Welcome to the GAME CORNER! The machines are being repaired today. Come back another time!",
        'Person3': "I used to play here every day. Now I just come for the music.",
        'Person4': "I came to town to sell vegetables and ended up here. Don't tell my wife.",
        'Person5': "The flashing lights give me ideas for songs. Ding ding ding! ...See?",
        'Person6': "After a long shift, I come here to relax. The machines don't mind if I just watch.",
        'Person7': "Someone dropped a POKé BALL near the machines once. I've been looking for more ever since.",
        'Person8': "My daughter runs the GYM. She trains so hard. I come here to... train too. My luck, that is.",
    },
    'Sinnoh_VeilstoneStore1f': {
        'Person1': "Welcome to the VEILSTONE DEPARTMENT STORE! Every floor has something for TRAINERS.",
        'Person2': "I'm here for a gift for my wife. Five floors and I still don't know what to get.",
        'Person3': "This store has everything. I came for one thing and forgot what it was.",
        'Person4': "Shopping is a sport, darling. And I'm a champion.",
    },
    'Sinnoh_VeilstoneStore2f': {
        'Person1': "This floor has all the medicine. I always buy more than I need. Just in case!",
        'Person2': "I'm buying POKé BALLS. I want to catch every bug in ETERNA FOREST!",
        'Person4': "Welcome to the second floor! For TRAINERS on the go.",
        'Person7': "My mom gave me money for a REPEL. I'm getting a NET BALL instead. Shh.",
    },
    'Sinnoh_VeilstoneStore3f': {
        'Person1': "Welcome to the third floor! TMs to teach your MOéMON new tricks.",
        'Person2': "I want every TM! My piggy bank says otherwise.",
        'Person3': "I'm looking for a TM that teaches SING. My MOéMON has the voice for it!",
        'Person6': "Teaching your MOéMON a new move is like giving it a new toy. They love it!",
    },
    'Sinnoh_VeilstoneStore4f': {
        'Person1': "I'm buying a toy for my MOéMON. It's a squeaky one. My mom's going to love it.",
        'Person2': "Dolls of MOéMON! I have to have the PIKACHU one!",
        'Person3': "Welcome to the fourth floor! Decorations and dolls for your room.",
        'Person6': "I'm here to look at the dolls. Just to look! ...Okay, maybe buy one.",
    },
    'Sinnoh_VeilstoneStore5f': {
        'Person1': "The view from the top floor is great! You can see the whole city.",
        'Person2': "I come up here to rest my feet after shopping. Five floors is a lot!",
        'Person3': "I've been to every floor twice. This store is a treasure hunt!",
        'Person4': "I collect the store's shopping bags. They change the design every season!",
    },
    'Sinnoh_VeilstoneCityPrizeExchange': {
        'Person2': "I won a prize once. It was a POTION. I'm still proud of it.",
        'Person3': "In my day, prizes were fancier. But I still come to look.",
    },
    'Sinnoh_VeilstoneCitySoutheastHouse': {
        'Person1': "I juggle POKé BALLS for fun! ...Empty ones. Mostly.",
        'Person2': "I train in the GYM every day. MAYLENE says I've got the right spirit!",
    },
    'Sinnoh_VeilstoneCityNorthwestHouse': {
        'Person1': "My grandma's BUNEARY likes me more than her. It's a little awkward.",
        'Person2': "BUNEARY has lived with me for years. It still hops like a baby.",
        'Person3': "BUNEARY: Bun!|It's hopping in place.",
    },
    'Sinnoh_VeilstoneCityNortheastHouse': {
        'Person1': "The rocks in the city fell from the sky long ago. I wonder what's inside them.",
        'Person2': "My husband talks about those rocks every day. Every single day.",
    },
    'Sinnoh_VeilstoneCitySouthwestHouse': {
        'Person1': "I love living by the cliffs. The wind sounds like music at night.",
        'Person2': "My PACHIRISU sleeps in my hood. It's warmer than a bed.",
        'Person3': "PACHIRISU: Chipa?|It peeks out from behind the chair.",
    },
    'Sinnoh_Route215GateToVeilstoneCity': {
        'Person1': "The old stones in VEILSTONE are covered in markings. I'm trying to read them.",
    },
    # ------------------------------------------------------------------ SUNYSHORE CITY
    'Sinnoh_SunyshoreCityPokecenter1f': {
        'Person2': "I've lived by the sea all my life. The waves here are the gentlest in SINNOH.",
        'Person3': "VOLKNER used to battle everyone who came. Now he just looks bored. Maybe you can help!",
        'Person4': "The solar walkways power the whole city. Even this MOéMON CENTER!",
    },
    'Sinnoh_SunyshoreCityMart': {
        'Person2': "I'm going to explore the ruins north of here. Bring ESCAPE ROPES, I say!",
        'Person3': "A sailor always stocks up before a long trip. Even if it's just to the next town!",
    },
    'Sinnoh_SunyshoreMarket': {
        'Person1': "The market's stalls change every day. Come back tomorrow for something new!",
        'Person2': "I found a shell shaped like a STARMIE here once. Best day of my life.",
        'Person3': "Sailors from all over sell their things here. You never know what you'll find.",
        'Person4': "Welcome! These seals go on your POKé BALLS. Pretty, aren't they?",
        'Person5': "I came for the fish, but I'm staying for the sunsets.",
    },
    'Sinnoh_SunyshoreCityNortheastHouse': {
        'Person1': "I'm JULIA! I travel around making my MOéMON look their absolute best.",
    },
    'Sinnoh_SunyshoreCityWestHouse': {
        'Person1': "We're all waiting for VOLKNER to battle us. He's been in a slump lately.",
        'Person2': "I make the kids laugh while they wait. Look, a POKé BALL behind your ear!",
        'Person3': "VOLKNER's the strongest LEADER in SINNOH. That's why he's last!",
        'Person4': "I bring snacks for the kids. Waiting is hungry work.",
        'Person5': "I'm going to be the first one to beat VOLKNER this year! ...After you, maybe.",
        'Person6': "VOLKNER waved at me once! I didn't wash that hand for a week.",
        'Person7': "Electric MOéMON are the coolest. Zap zap!",
    },
    'Sinnoh_SunyshoreCityNorthwestHouse': {
        'Person1': "Fifty years by the sea. I still watch the sunset every night.",
        'Person2': "He says that every evening. I watch with him anyway.",
    },
    'Sinnoh_SunyshoreCityEastHouse': {
        'Person1': "I study solar energy. SUNYSHORE gets more sun than any town in SINNOH.",
    },
    'Sinnoh_VistaLighthouse': {
        'Person1': "The lighthouse guides ships home. VOLKNER sometimes comes up here to think.",
        'Person2': "From the top you can see the MOéMON LEAGUE! It's so far away...",
    },
    # ------------------------------------------------------------------ SNOWPOINT CITY
    'Sinnoh_SnowpointCityMart': {
        'Person2': "I'm buying BURN HEALS. Not for burns. They're just warm to hold.",
        'Person3': "Bundle up and stock up! That's SNOWPOINT's motto.",
    },
    'Sinnoh_SnowpointCityPokecenter1f': {
        'Person2': "It's so warm in here! I could stay all day.",
        'Person3': "CANDICE trains outside in the snow. She says it builds focus.",
        'Person4': "PSYDUCK: Psy...|It's shivering a little, but happy to be inside.",
    },
    'Sinnoh_SnowpointCityWestHouse': {
        'Person1': "I came north to train. My MOéMON hates the cold. We're working on it.",
        'Person2': "I'm MINDY! I love trading MOéMON with TRAINERS from far away.",
    },
    'Sinnoh_SnowpointCityEastHouse': {
        'Person1': "The temple to the north is very old. They say a great MOéMON sleeps deep inside.",
        'Person2': "I climbed down from the mountains just for a hot drink. Worth it.",
    },
    # ------------------------------------------------------------------ MOéMON LEAGUE
    'Sinnoh_PokemonLeagueSouthPokecenter1f': {
        'Person2': "VICTORY ROAD is no joke. Rest up here before you go in.",
        'Person3': "I made it through VICTORY ROAD once. My MOéMON were so tired they slept for a day.",
    },
    'Sinnoh_PokemonLeagueNorthPokecenter1f': {
        'Person1': "Beyond this door are the ELITE FOUR. Once you go in, there's no coming back until it's over.|"
                   "Make sure your team is ready!",
        'Person5': "I've challenged the ELITE FOUR five times. I'm getting closer every time!",
        'Person7': "I play a song for every TRAINER who goes in. Good luck!",
    },
    # ------------------------------------------------------------------ BATTLE ZONE
    'Sinnoh_FightAreaPokecenter1f': {
        'Person2': "I sing to keep my MOéMON's spirits up. They battle better when they're happy!",
        'Person3': "Welcome! Everyone here is a TRAINER. Even the cooks!",
        'Person4': "The FRONTIER is the toughest place in SINNOH. I'm training here until I'm ready.",
    },
    'Sinnoh_FightAreaMart': {
        'Person1': "Even clowns need POTIONS out here. Battles everywhere!",
        'Person2': "I shop here before every challenge, darling. A lady must be prepared.",
    },
    'Sinnoh_Route225GateToFightArea': {
        'Person1': "The routes past here are wild. Only strong TRAINERS should go.",
        'Person2': "My father bought me a ticket to the BATTLE ZONE. I'm going to win everything!",
    },
    'Sinnoh_FightAreaMiddleHouse': {
        'Person1': "Welcome! TRAINERS from all over stay in the BATTLE ZONE to train.",
        'Person2': "I study the battles at the FRONTIER. I've filled ten notebooks!",
        'Person3': "I'm going to be a FRONTIER BRAIN someday!",
    },
    'Sinnoh_FightAreaSouthHouse': {
        'Person1': "My TORCHIC came all the way from HOENN with me. It loves the warm weather here.",
        'Person2': "TORCHIC: Torchic!|It's flapping its little wings.",
        'Person3': "He talks about HOENN all the time. I've never been, but it sounds lovely.",
        'Person4': "Items can turn a battle around. A good held item is half the battle!",
    },
    # ------------------------------------------------------------------ caves, gates and others
    'Sinnoh_OreburghMineB1f': {
        'Person1': "Careful down here! The carts don't stop for anyone.",
        'Person3': "My dad works here! I'm bringing him his lunch.",
    },
    'Sinnoh_OreburghMineB2f': {
        'Person8': "My MACHOP do the heavy lifting. I just tell them where to put it!",
        'Person9': "MACHOP: Ma-chop!|It's carrying a big rock.",
        'Person10': "MACHOP: Chop!|It's taking a break, sitting on a cart.",
        'Person11': "MACHOP: Machop!|It flexes proudly.",
    },
    'Sinnoh_ValleyWindworksBuilding': {
        'Person3': "The windmills make power for the towns nearby. The wind here never stops!",
    },
    'Sinnoh_EternaForest': {
        'Person2': "The forest is so big! Follow the light. That's how you find the way out.",
    },
    'Sinnoh_FuegoIronworksBuilding': {
        'Person5': "Welcome to FUEGO IRONWORKS! My MOéMON and I make iron here, the old way.",
    },
    'Sinnoh_SolaceonRuinsRoom2': {
        'Person1': "These ruins go on forever! I keep finding new rooms.",
    },
    'Sinnoh_AmitySquare': {
        'Person1': "Welcome to AMITY SQUARE! Enjoy a walk with your MOéMON.",
        'Person2': "Welcome to AMITY SQUARE! Enjoy a walk with your MOéMON.",
        'Person7': "My DRIFLOON follows me everywhere. Even into the bath!",
        'Person8': "DRIFLOON: Fuwa...|It's floating gently in the breeze.",
        'Person9': "HAPPINY: Happi!|It's carrying a little round stone.",
        'Person10': "My HAPPINY loves to collect round stones. It thinks they're eggs!",
        'Person11': "My PIKACHU and I come here every Sunday. It knows the way better than I do.",
        'Person12': "PIKACHU: Pika!|It's chasing a leaf.",
        'Person13': "My CLEFAIRY dances under the trees. Isn't it cute?",
        'Person14': "CLEFAIRY: Pi pi!|It's spinning in a circle.",
        'Person16': "Your MOéMON might find something on the paths. It has a good nose!",
    },
    'Sinnoh_FloaromaMeadow': {
        'Person3': "The honey trees here attract all sorts of MOéMON. Slather some on and wait!",
    },
    'Sinnoh_FloaromaMeadowHouse': {
        'Person1': "We sell honey from the meadow! It's the sweetest in SINNOH.",
        'Person2': "I helped collect the honey today! I only got stung twice.",
    },
    'Sinnoh_OreburghGate1f': {
        'Person11': "This cave connects JUBILIFE and OREBURGH. Watch out for ZUBAT!",
    },
    'Sinnoh_OreburghGateB1f': {
        'Person16': "The lower floor has rocks you'll need to smash. Come back when you can!",
    },
    'Sinnoh_RuinManiacCaveShort': {
        'Person2': "I'm digging a tunnel to find old ruins! It's slow work.",
    },
    'Sinnoh_Restaurant': {
        'Person19': "Welcome to the SEVEN STARS! Every dinner comes with a view of the lake.",
    },
    'Sinnoh_GrandLakeValorLakefrontWestHouse': {
        'Person1': "I make games for a living. My best ideas come while watching MOéMON play.",
    },
    'Sinnoh_Route205House': {
        'Person1': "Welcome! Rest your feet. The forest path is long.",
        'Person2': "I caught a WURMPLE today! It's sleeping in my hat.",
    },
    'Sinnoh_Route206CyclingRoadSouthGate': {
        'Person3': "The cycling road is downhill all the way south! Wheee!",
    },
    'Sinnoh_Route208House': {
        'Person1': "I've grown BERRIES my whole life. Each one has its own taste, and its own story.",
    },
    'Sinnoh_BattleTower': {
        'Person1': "Welcome to the BATTLE TOWER. Challenges open soon. Please come back!",
        'Person2': "Welcome to the BATTLE TOWER. Challenges with a partner open soon!",
        'Person5': "I'm sketching the TRAINERS who come here. The tower inspires them.",
        'Person6': "I run here every morning to get ready for my challenge!",
        'Person7': "I'm here to cheer on my favorite TRAINERS! Go go go!",
        'Person8': "I'll be the TOWER's champion! As soon as it opens!",
        'Person9': "PIKACHU: Pika!|It's sitting on its TRAINER's shoulder.",
        'Person10': "PIKACHU: Chu!|It's looking around excitedly.",
        'Person11': "I'm keeping notes on every battle. I'm going to beat the tower someday!",
        'Person12': "The tower is so tall. I wonder how far you have to win to get to the top.",
        'Person15': "I can tell how your MOéMON's stats grew. Come back once the tower is open!",
    },
    'Sinnoh_GalacticHq1f': {
        'Person1': "What are you doing here? Visitors aren't allowed! ...Well, look around, but don't touch.",
        'Person2': "I do research here. It's all very serious science. Very serious.",
        'Person8': "Our uniforms are space themed. Aren't they cool? Be honest.",
        'Person9': "I just started working here. I'm not really sure what we do yet.",
    },
}

