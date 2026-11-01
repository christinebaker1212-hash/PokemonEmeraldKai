"""SINNOH's words: what its people say and its signs read, written for this game (docs/SINNOH_HANDOFF.md step 4; nothing
is taken from Platinum's text). tools/sinnoh/gen_words.py turns them into each map's scripts.inc, wrapped to the message
box, and gen_maps.py gives each person / sign its script. tools/sinnoh/who.py lists who is where on a map and the
number each one goes by.

WORDS[map][key]: key Person<n> / Sign<n> / Bg<n> (the numbers who.py prints), value:
  - a text: a person talks (faces the player), a sign is read. '|' starts a new page; lines wrap by themselves.
  - ('nurse',): the POKéMON CENTER's nurse heals the party.
  - ('mart',): a clerk sells what the region's badges allow (pokemart 0, as JOHTO's and KANTO's clerks).
  - ('gym', n): the n-th gym leader (GYMS[n]): battles, gives the badge (flag 0x3320 + n) and a TM (flag 0x3770 + n).
  - ('e4', n): the n-th of the ELITE FOUR (ELITE[n]); ('champion',): the CHAMPION (CHAMPION), once all four are
    beaten; winning makes SINNOH cleared and plays the HALL OF FAME and the credits.
  - ('boat', [maps]): a sailor who sails to each of those maps (asked in turn), landing beside its own sailor.
  - ('script', LABEL): a person who runs a script of the game's own (LABEL), e.g. EventScript_SinnohRegionBoat.
MOéMON's words like HOENN's (MOéMON, MOéDEX, POKé BALL kept): OPTIONS > CREATURES swaps them back.
"""

WORDS = {
    'Sinnoh_FightArea': {'Person6': ('boat', ['Sinnoh_SnowpointCity']), 'Person19': ('boat', ['Sinnoh_SnowpointCity'])},
    'Sinnoh_SnowpointCity': {'Person6': ('boat', ['Sinnoh_FightArea'])},
    'Sinnoh_NewmoonIsland': {'Person1': ('boat', ['Sinnoh_CanalaveCity'])},
    'Sinnoh_FullmoonIsland': {'Person1': ('boat', ['Sinnoh_CanalaveCity'])},
    'Sinnoh_IronIsland': {'Person1': ('boat', ['Sinnoh_CanalaveCity'])},
    'Sinnoh_CanalaveCity': {'Person15': ('script', 'EventScript_SinnohRegionBoat'), 'Person17': ('boat', ['Sinnoh_IronIsland', 'Sinnoh_FullmoonIsland', 'Sinnoh_NewmoonIsland'])},
    'Sinnoh_PokemonLeagueChampionRoom': {'Person1': ('champion',)},
    'Sinnoh_PokemonLeagueLucianRoom': {'Person1': ('e4', 3)},
    'Sinnoh_PokemonLeagueFlintRoom': {'Person1': ('e4', 2)},
    'Sinnoh_PokemonLeagueBerthaRoom': {'Person1': ('e4', 1)},
    'Sinnoh_PokemonLeagueAaronRoom': {'Person1': ('e4', 0)},
    'Sinnoh_SunyshoreCityGymRoom3': {'Person1': ('gym', 7)},
    'Sinnoh_SnowpointCityGym': {'Person1': ('gym', 6)},
    'Sinnoh_CanalaveCityGym': {'Person5': ('gym', 5)},
    'Sinnoh_PastoriaCityGym': {'Person1': ('gym', 3)},
    'Sinnoh_VeilstoneCityGym': {'Person1': ('gym', 2)},
    'Sinnoh_HearthomeCityGymLeaderRoom': {'Person1': ('gym', 4)},
    'Sinnoh_EternaCityGym': {'Person2': ('gym', 1)},
    'Sinnoh_OreburghCityGym': {'Person1': ('gym', 0)},
    # ------------------------------------------------------------------ TWINLEAF TOWN
    'Sinnoh_TwinleafTown': {
        'Person1': "The wind off the lake smells like rain today.|My MOéMON curl up by the fire when it does. "
                   "I think I'll join them.",
        'Person2': "I've lived in TWINLEAF my whole life and I still find new rocks by the pond.|"
                   "This one's shaped like a STARLY! Well... if you squint.",
        'Person4': "North of here is ROUTE 201. The grass gets tall fast, so wild MOéMON jump out "
                   "before you know it.|If you came all the way to SINNOH, you can handle that, right?",
        'Sign5': "TWINLEAF TOWN|Where the wind off the lake starts its journey.",
        'Sign6': "It's a mailbox.|There's a postcard sticking out. \"Back for dinner. Promise!\"",
        'Sign7': "It's a mailbox.|It's stuffed with flyers for running shoes.",
    },
    'Sinnoh_TwinleafTownPlayerHouse1f': {
        'Person1': "Oh! You're the TRAINER from overseas! PROF. ROWAN told me you were coming.|"
                   "The room upstairs is yours while you're in SINNOH. My kid left on a MOéMON journey last spring, "
                   "so the house got so quiet...|PROF. ROWAN's lab is in SANDGEM TOWN, past ROUTE 201. "
                   "Come home whenever you're tired. I always cook too much anyway.",
        'Bg1': "A cooking show is on.|\"...and that's how you make a POFFIN that even a MUNCHLAX would share!\"",
        'Bg2': "A cooking show is on.|\"...and that's how you make a POFFIN that even a MUNCHLAX would share!\"",
        'Bg3': "The fridge is full of BERRY juice and leftovers labeled \"FOR WHEN YOU VISIT.\"",
        'Bg4': "The sink is spotless. Someone here takes pride in it.",
        'Bg5': "The sink is spotless. Someone here takes pride in it.",
        'Bg6': "There's a recipe card on the counter.|\"ORAN soup: three ORAN BERRIES, a pinch of salt, "
               "and a lot of patience.\"",
        'Bg7': "There's a recipe card on the counter.|\"ORAN soup: three ORAN BERRIES, a pinch of salt, "
               "and a lot of patience.\"",
        'Bg8': "There's a recipe card on the counter.|\"ORAN soup: three ORAN BERRIES, a pinch of salt, "
               "and a lot of patience.\"",
    },
    'Sinnoh_TwinleafTownPlayerHouse2f': {
        'Bg1': "A game console. A save file is paused in the middle of a boss fight.|Better not touch it.",
        'Bg2': "The PC is off. A sticky note says \"Charge the STARLY lamp.\"",
        'Bg4': "A weather channel is on.|\"Clear skies over LAKE VERITY, with a light breeze from the north.\"",
        'Bg5': "A weather channel is on.|\"Clear skies over LAKE VERITY, with a light breeze from the north.\"",
    },
    'Sinnoh_TwinleafTownRivalHouse1f': {
        'Person1': "My son? He's off somewhere faster than a QUICK CLAW.|"
                   "He never could sit still. Not even as a baby!|If you see a blur running past, "
                   "tell him to call his mother.",
    },
    'Sinnoh_TwinleafTownRivalHouse2f': {
        'Bg1': "A game console, with a sticky note: \"HIGH SCORE. DON'T YOU DARE.\"",
        'Bg2': "The PC is on. It's showing a list titled \"FASTEST WAYS TO THE MOéMON LEAGUE.\"",
    },
    'Sinnoh_TwinleafTownNortheastHouse': {
        'Person1': "I'm writing a song about LAKE VERITY.|So far I have \"lake\" and \"VERITY.\" "
                   "The middle part is giving me trouble.",
    },
    'Sinnoh_TwinleafTownSouthwestHouse': {
        'Person1': "My grandkid wants to be a TRAINER.|I told her a TRAINER needs three things: "
                   "a good friend, good shoes and a good lunch.",
        'Person2': "When I grow up I'm going to catch a MOéMON as big as a house!|"
                   "Then it can carry me to school.",
    },
    # ------------------------------------------------------------------ ROUTE 201
    'Sinnoh_Route201': {
        'Person1': "Wild MOéMON hide in the tall grass. Stay on the path if your team is tired.|"
                   "Or don't! Running into them is half the fun.",
        'Person2': "I'm handing out samples from the POKé MART in SANDGEM! It's just east of here.",
        'Sign4': "ROUTE 201|West: TWINLEAF TOWN",
        'Sign5': "ROUTE 201|East: SANDGEM TOWN",
        'Person10': "I'm practicing my throws. POKé BALLS fly better when your MOéMON has worn the wild one "
                    "down a little first.",
        'Sign11': "TRAINER TIPS|Talk to people everywhere you go. Most of them have something to say, "
                  "and some of them have something to give.",
        'Person12': "STARLY travel in flocks around here.|When one gets tired, the others slow down and "
                    "wait. Isn't that sweet?",
    },
    # ------------------------------------------------------------------ SANDGEM TOWN
    'Sinnoh_SandgemTown': {
        'Person1': "The sea breeze in SANDGEM makes everyone sleepy around noon.|"
                   "That's not laziness. It's tradition!",
        'Person2': "The MOéMON lab here studies how MOéMON change as they grow.|"
                   "My SHINX became a LUXIO last week, and the lab took a hundred pictures!",
        'Person3': "Head south and you'll reach the beach of ROUTE 219.|"
                   "You'll need a MOéMON that can SURF to get much farther, though.",
        'Sign6': "MOéMON RESEARCH LAB",
        'Sign7': "A family home. A wind chime shaped like a CHINGLING tinkles by the door.",
        'Sign8': "SANDGEM TOWN|The town where the sand shines like gems.",
        'Sign13': "MOéMON CENTER|Rest and heal your MOéMON.",
        'Sign14': "POKé MART|Everything a TRAINER needs.",
    },
    'Sinnoh_SandgemTownHouse': {
        'Person1': "We came to SANDGEM for the beach and stayed for the people.|"
                   "Well, mostly for the beach.",
        'Person2': "Our BUNEARY runs off to the shore every morning.|"
                   "It comes back with its ears full of sand, every single time.",
    },
    'Sinnoh_SandgemTownMart': {
        'Person1': ('mart',),
        'Person2': "I spent all my allowance on POKé BALLS.|Now I just need to get good at throwing them.",
        'Person3': "A POTION for the trip, an ANTIDOTE for the grass... and a snack for me.|"
                   "Shopping for a journey is the best part!",
    },
    'Sinnoh_SandgemTownPokecenter1f': {
        'Person1': "A MOéMON CENTER heals your team for free.|"
                   "Come back whenever your MOéMON look tired. They'd do the same for you.",
        'Person2': "I've traveled every region I can reach by boat.|"
                   "SINNOH has the best sunsets. Don't tell HOENN I said that.",
        'Person3': "The PC here keeps your MOéMON safe in boxes.|"
                   "I keep my favorites at the front so they don't feel lonely.",
        'Person4': ('nurse',),
    },
    'Sinnoh_SandgemTownPokecenter2f': {
        'Person1': "The link rooms upstairs are still being set up.|"
                   "Your friends in CO-OP can find you anywhere, though!",
        'Person2': "The link rooms upstairs are still being set up.|"
                   "Your friends in CO-OP can find you anywhere, though!",
        'Person3': "The link rooms upstairs are still being set up.|"
                   "Your friends in CO-OP can find you anywhere, though!",
        'Person4': "I'm waiting for my friend so we can trade.|"
                   "They said they'd be here an hour ago... They're probably catching something.",
    },
    'Sinnoh_SandgemTownCounterpartHouse1f': {
        'Person1': "My grandchild helps out at the MOéMON lab.|"
                   "Always running errands for the professor. I'm proud as can be.",
        'Person2': ('script', 'Sinnoh_EventScript_SwarmTwin'),   # (her words: data/scripts/sinnoh_wild.inc)
    },
    'Sinnoh_SandgemTownCounterpartHouse2f': {
        'Bg1': "The PC is open to a page of notes on how MOéMON grow.|"
               "\"Evolution: not just getting bigger. Ask the MAGIKARP.\"",
    },
    'Sinnoh_SandgemTownPokemonResearchLab': {
        'Person2': "We study how MOéMON evolve.|Some evolve with a stone, some with a friend, "
                   "and some just when they're good and ready.",
        'Person3': ('script', 'Sinnoh_EventScript_RadarAide'),   # (the POKé RADAR: data/scripts/sinnoh_wild.inc)
        'Bg1': "A thick book: \"A TRAINER'S FIRST RULE: Take care of your MOéMON, and they'll take care "
               "of you.\"",
        'Bg2': "A thick book: \"A TRAINER'S SECOND RULE: Never stop being curious.\"",
        'Bg3': "The shelves are packed with books about MOéMON from every region.",
        'Bg4': "A binder full of sketches of SINNOH's MOéMON, with notes in tiny handwriting.",
        'Bg5': "The PC shows a chart of how fast different MOéMON grow.|The lines look like mountains.",
        'Bg6': "A fridge for samples. One shelf is just labeled \"LUNCH. DO NOT STUDY.\"",
    },
}


# The gym leaders, in badge order: (trainer, badge, TM, intro, defeat, badge info, TM words, after)
GYMS = [  # in Platinum's order: the n-th is the (n + 1)-th badge (CANALAVE's BYRON the 6th)
    ('TRAINER_SINNOH_LEADER_ROARK', 'COAL BADGE', 'ITEM_TM76',
     "Welcome to the OREBURGH GYM! I'm ROARK, the LEADER here.|I spend my mornings in the mine and my afternoons in "
     "here. Both make you tough. Let's see how tough you are!",
     "Crushed... like gravel!",
     "That's the COAL BADGE. It's made from OREBURGH coal, pressed hard. Like you pressed me!",
     "Take this too. STEALTH ROCK scatters sharp stones that hurt every MOéMON your foe sends out.",
     "The mine runs right under the city. Every rock down there has a story. Your battle's one of them now."),
    ('TRAINER_SINNOH_LEADER_GARDENIA', 'FOREST BADGE', 'ITEM_TM86',
     "Hi there! I'm GARDENIA. I grow MOéMON the way I grow flowers: with sunshine and a little stubbornness!|"
     "Ready to see what's blooming?",
     "Wilted... but in a nice way.",
     "The FOREST BADGE! It's shaped like a leaf from the oldest tree in ETERNA FOREST.",
     "And this is GRASS KNOT. The heavier your foe, the harder it trips. Big MOéMON hate it!",
     "Have you walked through ETERNA FOREST? It's spooky, but I love it. The trees there are older than the city."),
    ('TRAINER_SINNOH_LEADER_MAYLENE', 'COBBLE BADGE', 'ITEM_TM60',
     "I'm MAYLENE. I've trained here since I was small.|I won't hold anything back. If you're ready, so am I!",
     "I'm not strong enough yet...",
     "This is the COBBLE BADGE. Every LEADER of this GYM has handed it out with pride. Now I have too.",
     "Please take this. DRAIN PUNCH takes some of the foe's strength for your own.",
     "Thank you for that battle. I'm going to train even harder. You should too!"),
    ('TRAINER_SINNOH_LEADER_WAKE', 'FEN BADGE', 'ITEM_TM55',
     "I am CRASHER WAKE! The wave that crashes over every challenger!|Can you ride it? Let's find out! WAHAHA!",
     "The wave crashed over me!",
     "The FEN BADGE! Shaped like a raindrop! It's slippery, so don't drop it in the marsh! WAHAHA!",
     "And this! BRINE: it hits hardest when your foe is already half worn out. Like me right now!",
     "Wahaha! What a match! I'm going to the GREAT MARSH to cool off. Come splash around some time!"),
    ('TRAINER_SINNOH_LEADER_FANTINA', 'RELIC BADGE', 'ITEM_TM65',
     "Bonjour! I am FANTINA. My GHOST MOéMON and I, we dance in the dark!|Tonight you will dance with us. Allez!",
     "Ah... the music stops.",
     "The RELIC BADGE is yours. It is very old, and very lovely. Like my GHOSTS.",
     "Take this as well. SHADOW CLAW: a quick slash from the shadows that finds the weak spot.",
     "You dance well in battle. Come to the CONTEST HALL some time, yes? We dance there too."),
    ('TRAINER_SINNOH_LEADER_BYRON', 'MINE BADGE', 'ITEM_TM91',
     "So you're the one who's been climbing through SINNOH! I'm BYRON. My STEEL MOéMON are as hard as the mountains "
     "I dig in.|Show me what you're made of!",
     "Cracked my shell, you did!",
     "The MINE BADGE. Forged from iron dug out of the hills behind CANALAVE.",
     "And FLASH CANNON: a burst of light from a STEEL MOéMON's body. It may lower the foe's SP. DEF.",
     "My son runs the gym in OREBURGH. Tell him his old man got beaten fair and square. He'll laugh about it."),
    ('TRAINER_SINNOH_LEADER_CANDICE', 'ICICLE BADGE', 'ITEM_TM72',
     "I'm CANDICE! In SNOWPOINT we say it's all about focus. Diamond-hard focus!|"
     "And right now I'm focused on beating you!",
     "Brr... I lost my focus!",
     "Here's the ICICLE BADGE! Don't worry, it never melts.",
     "AVALANCHE is yours too. Get hit first, then hit back twice as hard. Cool, right?",
     "Your focus is amazing! Come back when it's snowing. That's when SNOWPOINT is prettiest."),
    ('TRAINER_SINNOH_LEADER_VOLKNER', 'BEACON BADGE', 'ITEM_TM57',
     "I'm VOLKNER. I've been bored a long time. Nobody gives me a real battle any more.|"
     "Maybe you're different. Light me up.",
     "...Now that was a battle.",
     "The BEACON BADGE. It's the last badge in SINNOH. Shine bright with it.",
     "Take CHARGE BEAM too. It may raise your SP. ATK as it hits.",
     "Thanks. I remember why I liked battling. The MOéMON LEAGUE is past VICTORY ROAD. Go show them that spark."),
]


# The ELITE FOUR, in the order their rooms come: (trainer, intro, defeat, after)
ELITE = [
    ('TRAINER_SINNOH_ELITE_FOUR_AARON',
     "I'm AARON of the ELITE FOUR. BUG MOéMON are small, and people think that makes them weak.|"
     "They're wrong. Let me show you how wrong!",
     "My bugs fought beautifully...",
     "Go on to the next room. BERTHA is waiting. She's tougher than she looks."),
    ('TRAINER_SINNOH_ELITE_FOUR_BERTHA',
     "Well, well. Another young one made it past AARON. I'm BERTHA.|"
     "My GROUND MOéMON have been with me a long, long time. Let's see if you can move a mountain!",
     "Oh my. The mountain moved.",
     "You remind me of myself, long ago. Go on, dear. FLINT will keep you warm."),
    ('TRAINER_SINNOH_ELITE_FOUR_FLINT',
     "I'm FLINT! My FIRE MOéMON burn hot, and so do I!|Are you ready to get scorched?",
     "Burned out...",
     "That was hot! LUCIAN's next. He reads a lot. Don't let him bore you before the battle starts!"),
    ('TRAINER_SINNOH_ELITE_FOUR_LUCIAN',
     "I am LUCIAN. I was reading, but a battle with you will be a better story.|"
     "Let's see how this chapter ends.",
     "An unexpected ending.",
     "The CHAMPION is beyond this room. She is the strongest TRAINER I know. Go and write your own ending."),
]

# The CHAMPION: (trainer, intro, defeat, the words after the win, before the HALL OF FAME)
CHAMPION = ('TRAINER_SINNOH_CHAMPION_CYNTHIA',
            "Welcome. I'm CYNTHIA, the CHAMPION of SINNOH.|You beat all four of the ELITE FOUR. I can tell your "
            "MOéMON trust you completely.|I won't hold anything back. Neither should you!",
            "Brilliant... truly brilliant.",
            "You are SINNOH's new CHAMPION. Your MOéMON fought with everything they had, and so did you.|"
            "Come. The HALL OF FAME is waiting for your team.")


# The towns' and routes' words (tools/sinnoh/words_outdoor.py): a map's entries above win
def _merge():
    import os
    import sys
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from words_outdoor import OUTDOOR
    from words_indoor import INDOOR
    from story_people import STORY_WORDS
    for more in (OUTDOOR, INDOOR, STORY_WORDS):
        for name, entries in more.items():
            mine = WORDS.setdefault(name, {})
            for key, value in entries.items():
                mine.setdefault(key, value)


_merge()
