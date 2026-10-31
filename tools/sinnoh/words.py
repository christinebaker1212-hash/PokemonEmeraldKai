"""SINNOH's words: what its people say and its signs read, written for this game (docs/SINNOH_HANDOFF.md step 4; nothing
is taken from Platinum's text). tools/sinnoh/gen_words.py turns them into each map's scripts.inc, wrapped to the message
box, and gen_maps.py gives each person / sign its script. tools/sinnoh/who.py lists who is where on a map and the
number each one goes by.

WORDS[map][key]: key Person<n> / Sign<n> / Bg<n> (the numbers who.py prints), value:
  - a text: a person talks (faces the player), a sign is read. '|' starts a new page; lines wrap by themselves.
  - ('nurse',): the POKéMON CENTER's nurse heals the party.
  - ('mart',): a clerk sells what the region's badges allow (pokemart 0, as JOHTO's and KANTO's clerks).
MOéMON's words like HOENN's (MOéMON, MOéDEX, POKé BALL kept): OPTIONS > CREATURES swaps them back.
"""

WORDS = {
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
        'Person1': "Oh! A traveler? Welcome to TWINLEAF.|My kid left on a MOéMON journey last spring. "
                   "The house got so quiet...|Stop by any time. I always cook too much anyway.",
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
        'Person2': "I'm handing out samples, but I've run out!|The POKé MART in SANDGEM has the real thing. "
                   "It's just east of here.",
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
        'Person2': "My big sister knows the name of every MOéMON!|"
                   "Well, almost every one. She says there are always more.",
    },
    'Sinnoh_SandgemTownCounterpartHouse2f': {
        'Bg1': "The PC is open to a page of notes on how MOéMON grow.|"
               "\"Evolution: not just getting bigger. Ask the MAGIKARP.\"",
    },
    'Sinnoh_SandgemTownPokemonResearchLab': {
        'Person2': "We study how MOéMON evolve.|Some evolve with a stone, some with a friend, "
                   "and some just when they're good and ready.",
        'Person3': "The professor is out in the field again.|"
                   "He says you can't study MOéMON from behind a desk. He's right, but someone has to file "
                   "all this!",
        'Bg1': "A thick book: \"A TRAINER'S FIRST RULE: Take care of your MOéMON, and they'll take care "
               "of you.\"",
        'Bg2': "A thick book: \"A TRAINER'S SECOND RULE: Never stop being curious.\"",
        'Bg3': "The shelves are packed with books about MOéMON from every region.",
        'Bg4': "A binder full of sketches of SINNOH's MOéMON, with notes in tiny handwriting.",
        'Bg5': "The PC shows a chart of how fast different MOéMON grow.|The lines look like mountains.",
        'Bg6': "A fridge for samples. One shelf is just labeled \"LUNCH. DO NOT STUDY.\"",
    },
}
