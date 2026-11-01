"""SINNOH's story people: the ones Platinum shows and hides with its story (a hide flag), here shown while Platinum's
story would have them there, judged by what this game has instead of Platinum's scripts: the SINNOH badges, the story
battles beaten (tools/sinnoh/legends.py STORY, their "done" flags), day or night, and the LEAGUE.

SHOW[hide flag, without FLAG_HIDE_] = (from badges, before badges, need, unless, time):
  - badges: SINNOH's badge count, from (inclusive) .. before (exclusive); 9 = SINNOH cleared (the LEAGUE beaten).
  - need / unless: a story battle's name (DONE below) that must / must not be beaten (None: either).
  - time: None, 'day' (4:00-20:00) or 'night' (20:00-4:00: Platinum's night).
A hide flag not in SHOW keeps the old rule (gen_maps.py: hidden until it has a script). NEVER: Platinum's scenes this
game has no use for (cutscene stand-ins, blockades, Wi-Fi, the VILLA, the event islands' deliveries...).

STORY_WORDS[map][Person<n>]: what each one shown here says (original words), merged into words.py's WORDS. Trainers
need none (their battles are their scripts). gen_maps.py places them with their flag; s3d_legends.c Sinnoh_StoryRefresh
sets or clears each flag as a map loads (src/data/sinnoh_story_people.h, written by legends.py).
"""

# The story battles by name (legends.STORY's order)
DONE = ['RIVAL_201', 'RIVAL_203', 'MARS_WINDWORKS', 'JUPITER_ETERNA', 'RIVAL_209', 'RIVAL_PASTORIA', 'RIVAL_CANALAVE',
        'MARS_VERITY', 'SATURN_VALOR', 'SATURN_HQ', 'CYRUS_HQ', 'SPEAR_PILLAR', 'RIVAL_LEAGUE', 'RIVAL_SURVIVAL',
        'RIVAL_FIGHT', 'STARK', 'CYRUS_CELESTIC']

# ... and the story's own flags past those (the DISTORTION WORLD: data/scripts/sinnoh_places.inc)
EXTRA_DONE = {'DISTORTION': 0x3D35, 'NEW_GAME': 0x3D38, 'STARTER': 0x3D39}   # (a new game begun in SINNOH, its first POKéMON)

LEAGUE = 9
NEVER = (99, 99, None, None, None)
ALWAYS = (0, 99, None, None, None)


def _r(lo, hi=99, need=None, unless=None, time=None):
    return (lo, hi, need, unless, time)


SHOW = {
    # ------------------------------------------------------------------ the start (no badge yet)
    'TWINLEAF_TOWN_RIVAL': NEVER, 'TWINLEAF_TOWN_RIVAL_HOUSE_2F_RIVAL': NEVER,
    'TWINLEAF_TOWN_PLAYER_HOUSE_2F_RIVAL': NEVER, 'TWINLEAF_TOWN_PLAYER_HOUSE_1F_RIVAL_MOM': _r(0, 1),
    'ROUTE_201_PROF_ROWAN': NEVER, 'ROUTE_201_COUNTERPART': NEVER, 'ROUTE_201_BRIEFCASE': _r(0, need='NEW_GAME', unless='STARTER'),
    'LAKE_VERITY_PROF_ROWAN': _r(0, 1), 'LAKE_VERITY_COUNTERPART': _r(0, 1),
    'SANDGEM_TOWN_RIVAL': NEVER, 'SANDGEM_TOWN_PROF_ROWAN': NEVER, 'SANDGEM_TOWN_COUNTERPART': _r(0, 1),
    'SANDGEM_TOWN_LAB_PROF_ROWAN': ALWAYS, 'SANDGEM_TOWN_LAB_COUNTERPART': _r(1),
    'SANDGEM_TOWN_LAB_PROF_OAK': _r(LEAGUE), 'ROUTE_202_COUNTERPART': _r(0, 1),
    'JUBILIFE_ROWAN': NEVER, 'JUBILIFE_CITY_COUNTERPART': _r(0, 1), 'JUBILIFE_GALACTIC_GRUNTS': _r(0, 1),
    'JUBILIFE_CITY_LOOKER': _r(0, 2), 'JUBILIFE_CITY_CLOWNS_1_AND_2': _r(0, 2), 'JUBILIFE_CITY_CLOWN_3': _r(0, 2),
    'JUBILIFE_CITY_COLLECTOR': ALWAYS, 'JUBILIFE_CITY_POKETCH_CO_PRESIDENT': NEVER,
    'JUBILIFE_TV_2F_REPORTER': ALWAYS, 'JUBILIFE_TV_3F_GLOBAL_RANKING_ROOM_WORKER': ALWAYS,
    'JUBILIFE_TV_3F_GROUP_RANKING_ROOM_WORKER': ALWAYS, 'POKETCH_CO_1F_REPORTER': ALWAYS,
    'TRAINERS_SCHOOL_RIVAL': _r(0, 1), 'OREBURGH_CITY_RIVAL': _r(0, 1),
    'OREBURGH_MINE_B1F_HIKER': ALWAYS, 'OREBURGH_MINE_B2F_ROARK': _r(0, 1),
    'MT_CORONET_1F_SOUTH_CYRUS': _r(1, 2),
    # ------------------------------------------------------------------ the VALLEY WINDWORKS (1 BADGE: until MARS)
    'FLOAROMA_TOWN_GRUNTS': _r(1, unless='MARS_WINDWORKS'),
    'FLOAROMA_MEADOW_GRUNT_M_WEST': _r(1, unless='MARS_WINDWORKS'),
    'FLOAROMA_MEADOW_GRUNT_M_EAST': _r(1, unless='MARS_WINDWORKS'),
    'ROUTE_205_SOUTH_GRUNTS': _r(1, unless='MARS_WINDWORKS'),
    'ROUTE_205_SOUTH_LITTLE_GIRL': _r(1, unless='MARS_WINDWORKS'), 'ROUTE_205_SOUTH_YOUNGSTER': ALWAYS,
    'VALLEY_WINDWORKS_OUTSIDE_GRUNT_M': _r(1, unless='MARS_WINDWORKS'),
    'VALLEY_WINDWORKS_OUTSIDE_LOOKER': _r(1, unless='MARS_WINDWORKS'),
    'VALLEY_WINDWORKS_BUILDING_GALACTIC_GRUNT_1': _r(1, unless='MARS_WINDWORKS'),
    'VALLEY_WINDWORKS_BUILDING_TEAM_GALACTIC': _r(1, unless='MARS_WINDWORKS'),
    'VALLEY_WINDWORKS_BUILDING_LITTLE_GIRL': _r(1, need='MARS_WINDWORKS'),
    # ------------------------------------------------------------------ ETERNA (1-2 BADGES; its building until JUPITER)
    'ETERNA_FOREST_CHERYL': _r(1, 3), 'ETERNA_FOREST_GARDENIA': _r(1, 2),
    'ETERNA_CITY_GARDENIA': _r(1, 2), 'ETERNA_CITY_CYRUS': _r(1, 2), 'ETERNA_CITY_CYNTHIA': _r(1, 2),
    'ETERNA_CITY_RIVAL': _r(1, 2),
    'ETERNA_CITY_GALACTIC_GRUNTS': _r(2, unless='JUPITER_ETERNA'),
    'TEAM_GALACTIC_ETERNA_BUILDING_1F_LOOKER': _r(2, unless='JUPITER_ETERNA'),
    'TEAM_GALACTIC_ETERNA_BUILDING_1F_GRUNT_M_LOOKER': _r(2, unless='JUPITER_ETERNA'),
    'TEAM_GALACTIC_ETERNA_BUILDING_4F_HOSTAGES': _r(2, unless='JUPITER_ETERNA'),
    'CYCLE_SHOP_POKEFAN_M': _r(0, need='JUPITER_ETERNA'), 'CYCLE_SHOP_CLEFAIRY': _r(0, need='JUPITER_ETERNA'),
    'ETERNA_CITY_POKECENTER_1F_BUNEARY': ALWAYS, 'ETERNA_CITY_SOUTH_HOUSE_PROF_OAK': _r(LEAGUE),
    'ROUTE_207_COUNTERPART': _r(1, 2), 'ROUTE_206_CYCLING_ROAD_NORTH_GATE_SCIENTIST_M': ALWAYS,
    # ------------------------------------------------------------------ HEARTHOME, SOLACEON
    'HEARTHOME_CITY_KEIRA': _r(2, 4), 'HEARTHOME_CITY_BUNEARY': _r(2, 4), 'HEARTHOME_CITY_HIKER_3': ALWAYS,
    'HEARTHOME_CITY_GYM_GUIDE': NEVER, 'HEARTHOME_CITY_ROUTE_209_BLOCKADE': NEVER,
    'HEARTHOME_CITY_GATE_TO_AMITY_SQUARE_REPORTER': ALWAYS, 'HEARTHOME_CITY_POKEMON_FAN_CLUB_REPORTER': ALWAYS,
    'AMITY_SQUARE_FOLLOWER_MON': NEVER, 'SOLACEON_TOWN_RIVAL': _r(2, 4),
    # ------------------------------------------------------------------ VEILSTONE and PASTORIA (3-5 BADGES)
    'VEILSTONE_GALACTIC_GRUNTS': _r(3, 6, unless='SATURN_VALOR'),
    'VEILSTONE_CITY_GRUNT_M_STORAGE_KEY': _r(3, 6, unless='SATURN_VALOR'),
    'VEILSTONE_CITY_LOOKER': _r(3, 6), 'VEILSTONE_CITY_GALACTIC_WAREHOUSE_LOOKER': _r(3, 6),
    'VEILSTONE_COUNTERPART': _r(3, 5), 'VEILSTONE_CRASHER_WAKE': _r(4, 5), 'GAME_CORNER_LOOKER': _r(3, 6),
    'VEILSTONE_STORE_B1F_PROF_ROWAN': _r(3, 5),
    'PASTORIA_CITY_GRUNT_M': _r(3, 5), 'PASTORIA_CITY_CROAGUNK': _r(3, 5), 'PASTORIA_CITY_CRASHER_WAKE': _r(3, 4),
    'DAY_CARE_GYM_GUIDE': NEVER,
    'ROUTE_213_GRUNT_M': _r(4, 6), 'ROUTE_213_LOOKER': _r(4, 6), 'GRAND_LAKE_ROUTE_213_LOBBY_LOOKER': _r(4, 6),
    'ROUTE_210_SOUTH_CYNTHIA': NEVER, 'ROUTE_210_SOUTH_PSYDUCK': NEVER,
    'ROUTE_212_BLOCKADE': NEVER, 'ROUTE_214_GATE_TO_VEILSTONE_CITY_REPORTER': ALWAYS,
    # ------------------------------------------------------------------ CELESTIC (4-6 BADGES)
    'CELESTIC_TOWN_GRUNT_M': _r(4, 6), 'CELESTIC_TOWN_ELDER': _r(4, 6), 'CELESTIC_TOWN_CYNTHIA': _r(4, 6),
    'CELESTIC_TOWN_NORTH_HOUSE_ELDER': ALWAYS, 'CELESTIC_TOWN_CAVE_ELDER': _r(4, 6),
    'CELESTIC_TOWN_CAVE_CYNTHIA': _r(4, 6),
    # ------------------------------------------------------------------ CANALAVE, IRON ISLAND (5-7 BADGES)
    'CANALAVE_CITY_RIVAL_LIBRARY': _r(5, 7, unless='SATURN_VALOR'), 'CANALAVE_CITY_PROF_ROWAN': _r(5, 7),
    'CANALAVE_CITY_COUNTERPART': _r(5, 7),
    'CANALAVE_LIBRARY_ROWAN': _r(5, 7, unless='SATURN_VALOR'),
    'CANALAVE_LIBRARY_RIVAL': _r(5, 7, unless='SATURN_VALOR'),
    'CANALAVE_LIBRARY_COUNTERPART': _r(5, 7, unless='SATURN_VALOR'),
    'CANALAVE_LIBRARY_1F_LUCIAN': _r(LEAGUE), 'CANALAVE_LIBRARY_2F_HIKER': ALWAYS,
    'CANALAVE_CITY_HARBOR_INN_GYM_GUIDE': NEVER, 'CANALAVE_CITY_WEST_HOUSE_REPORTER': ALWAYS,
    'CANALAVE_CITY_SAILOR_ELDRITCH_HOUSE_SAILOR_ELDRITCH': _r(LEAGUE),
    'IRON_ISLAND_RILEY': _r(5, LEAGUE), 'IRON_ISLAND_B2F_LEFT_ROOM_RILEY': _r(5, LEAGUE),
    'IRON_ISLAND_B2F_LEFT_ROOM_GRUNTS': _r(5, LEAGUE), 'IRON_ISLAND_HOUSE_BYRON': _r(LEAGUE),
    'ROUTE_218_BLOCKADE': NEVER, 'ROUTE_218_GATE_TO_CANALAVE_CITY_SCIENTIST_M': ALWAYS,
    # ------------------------------------------------------------------ the lakes and TEAM GALACTIC's HQ (6-7 BADGES)
    'LAKE_VALOR_GALACTIC': _r(6, unless='SATURN_VALOR'), 'LAKE_VALOR_DRAINED_MAGIKARP': ALWAYS,
    'VALOR_LAKEFRONT_CYNTHIA': _r(6, unless='CYRUS_HQ'), 'VALOR_LAKEFRONT_RIVAL': _r(6, unless='CYRUS_HQ'),
    'VALOR_LAKEFRONT_GRUNT_M': _r(6, unless='SATURN_VALOR'), 'VALOR_LAKEFRONT_CAMERAMEN': _r(6, 8),
    'VALOR_LAKEFRONT_BEAUTY': _r(6, 8), 'VALOR_LAKEFRONT_COLLECTOR': _r(6, 8),
    'GRAND_LAKE_VALOR_LAKEFRONT_EAST_HOUSE_BEAUTY': ALWAYS,
    'LAKE_VERITY_TEAM_GALACTIC': _r(6, unless='MARS_VERITY'),
    'LAKE_VERITY_LOW_WATER_PROF_ROWAN': NEVER, 'LAKE_VERITY_LOW_WATER_COUNTERPART': NEVER,
    'LAKE_VERITY_LOW_WATER_CYRUS': NEVER, 'LAKE_VERITY_LOW_WATER_RIVAL': NEVER, 'LAKE_VERITY_LOW_WATER_STARLY': NEVER,
    'LAKE_ACUITY_JUPITER': _r(6, unless='CYRUS_HQ'), 'LAKE_ACUITY_RIVAL': _r(6, unless='CYRUS_HQ'),
    'LAKE_ACUITY_LOW_WATER_RIVAL': NEVER, 'ACUITY_LAKEFRONT_RIVAL': NEVER, 'VERITY_CAVERN_PROF_OAK': NEVER,
    'GALACTIC_HQ_TEAM_GALACTIC': _r(6, unless='CYRUS_HQ'),
    'GALACTIC_HQ_CONTROL_ROOM_CHARON': _r(6, unless='CYRUS_HQ'),
    'GALACTIC_HQ_CONTROL_ROOM_LAKE_GUARDIANS': _r(6, unless='CYRUS_HQ'),
    'GALACTIC_HQ_HALL_GRUNTS': _r(6, unless='SPEAR_PILLAR'), 'GALACTIC_HQ_HALL_CYRUS': _r(6, unless='CYRUS_HQ'),
    'GALACTIC_HQ_HALL_LOOKER': _r(6, unless='SPEAR_PILLAR'),
    # ------------------------------------------------------------------ MT. CORONET and SPEAR PILLAR (7 BADGES)
    'MT_CORONET_GALACTIC_GRUNTS': _r(7, unless='SPEAR_PILLAR'),
    'MT_CORONET_1F_NORTH_ROOM_1_GRUNTS_M': _r(7, unless='SPEAR_PILLAR'),
    'MT_CORONET_1F_NORTH_ROOM_1_GRUNT_F': _r(7, unless='SPEAR_PILLAR'),
    'MT_CORONET_2F_LOOKER': _r(7, unless='SPEAR_PILLAR'),
    'SPEAR_PILLAR_GRUNTS': _r(7, unless='SPEAR_PILLAR'), 'SPEAR_PILLAR_MARS_JUPITER': _r(7, unless='SPEAR_PILLAR'),
    'SPEAR_PILLAR_RIVAL': _r(7, unless='SPEAR_PILLAR'), 'SPEAR_PILLAR_CYRUS': _r(7, need='SPEAR_PILLAR', unless='DISTORTION'),
    'SPEAR_PILLAR_DISTORTED_CYNTHIA': NEVER, 'SPEAR_PILLAR_DISTORTED_MESPRIT': NEVER,
    'SPEAR_PILLAR_DISTORTED_TEAM_GALACTIC': NEVER,
    # ------------------------------------------------------------------ SNOWPOINT, SUNYSHORE (6-8 BADGES)
    'SNOWPOINT_CITY_CANDICE': _r(6, 7), 'SNOWPOINT_CITY_CYNTHIA': NEVER,
    'SNOWPOINT_CITY_POKECENTER_1F_MAYLENE': _r(6, 8), 'ROUTE_217_MAYLENE': _r(6, 8),
    'SNOWPOINT_CITY_SAILOR_SS_SPIRAL': NEVER, 'ROUTE_217_NORTHEAST_HOUSE_POKEMON_BREEDER_F': ALWAYS,
    'SUNYSHORE_CITY_FLINT': _r(7, 8), 'SUNYSHORE_CITY_RIVAL': _r(7, 8), 'VISTA_LIGHTHOUSE_VOLKNER': _r(7, LEAGUE),
    'ROUTE_221_WORKER_EAST': ALWAYS, 'ROUTE_221_WORKER_WEST': ALWAYS,
    'VICTORY_ROAD_1F_COLLECTOR': _r(8), 'VICTORY_ROAD_1F_ROOM_2_MARLEY': _r(LEAGUE),
    'POKEMON_LEAGUE_HALLWAY_TO_HALL_OF_FAME_CYNTHIA': NEVER,
    'POKEMON_LEAGUE_HALLWAY_TO_HALL_OF_FAME_PROF_ROWAN': NEVER,
    'WAYWARD_CAVE_1F_MIRA': _r(4),
    # ------------------------------------------------------------------ after the LEAGUE
    'FIGHT_AREA_BUCK': _r(LEAGUE), 'FIGHT_AREA_PALMER': _r(LEAGUE), 'FIGHT_AREA_BATTLERS_AND_SPECTATORS': _r(LEAGUE),
    'FIGHT_AREA_BLOCKADE': NEVER, 'SURVIVAL_AREA_BUCK': _r(LEAGUE), 'BATTLEGROUND_BUCK': _r(LEAGUE),
    'ROUTE_227_BUCK': _r(LEAGUE), 'ROUTE_227_CRASHER_WAKE': _r(LEAGUE), 'ROUTE_227_RIVAL': _r(LEAGUE),
    'STARK_MOUNTAIN_OUTSIDE_LOOKER': _r(LEAGUE, unless='STARK'), 'STARK_MOUNTAIN_OUTSIDE_BUCK': _r(LEAGUE, unless='STARK'),
    'STARK_MOUNTAIN_OUTSIDE_GRUNTS': _r(LEAGUE, unless='STARK'),
    'STARK_MOUNTAIN_ROOM_1_TEAM_GALACTIC': _r(LEAGUE, unless='STARK'),
    'STARK_MOUNTAIN_ROOM_2_BUCK': _r(LEAGUE, unless='STARK'), 'STARK_MOUNTAIN_ROOM_3_BUCK': _r(LEAGUE, need='STARK'),
    'STARK_MOUNTAIN_ROOM_3_LOOKER': _r(LEAGUE, need='STARK'), 'STARK_MOUNTAIN_ROOM_3_POLICEMEN': _r(LEAGUE, need='STARK'),
    'STARK_MOUNTAIN_ROOM_3_CROAGUNK': _r(LEAGUE, need='STARK'),
    'STARK_MOUNTAIN_ROOM_3_TEAM_GALACTIC': _r(LEAGUE, need='STARK'),
    'ROUTE_224_PROF_OAK': _r(LEAGUE), 'ROUTE_224_MARLEY': _r(LEAGUE), 'ROUTE_224_SHAYMIN': NEVER,
    'SENDOFF_SPRING_CYNTHIA': _r(LEAGUE), 'ROTOMS_ROOM_PROF_ROWAN': NEVER,
    'RESORT_AREA_PROF_ROWAN': _r(LEAGUE), 'RESORT_AREA_ROARK': _r(LEAGUE), 'RESORT_AREA_GARDENIA': _r(LEAGUE),
    'RESORT_AREA_BYRON': _r(LEAGUE), 'RESORT_AREA_MAYLENE': _r(LEAGUE), 'RESORT_AREA_SCHOOL_KID_M': _r(LEAGUE),
    'BATTLE_SALON_BUCK': _r(LEAGUE), 'BATTLE_SALON_CHERYL': _r(LEAGUE), 'BATTLE_SALON_MARLEY': _r(LEAGUE),
    'BATTLE_SALON_MIRA': _r(LEAGUE), 'BATTLE_SALON_RILEY': _r(LEAGUE),
    'BATTLE_TOWER_TEALA': NEVER, 'BATTLE_TOWER_REPORTER': ALWAYS, 'BATTLE_HALL_REPORTER': ALWAYS,
    'BATTLE_FRONTIER_REPORTER': ALWAYS,
    # ------------------------------------------------------------------ the rest
    'POKECENTER_BASEMENT_BLOCKADE': NEVER, 'MART_MYSTERY_GIFT_DELIVERYMAN': NEVER,
    'OLD_CHATEAU_DINING_AREA_OLD_MAN': _r(0, time='night'), 'OLD_CHATEAU_BACK_MIDDLE_EAST_ROOM_TWIN': _r(0, time='night'),
    'POKEMON_MANSION_FIVE_MAID_KNOCKOUT_BOSS': NEVER, 'POKEMON_MANSION_FIVE_MAID_KNOCKOUT_MAID': NEVER,
    'POKEMON_MANSION_OFFICE_POLICEMAN': ALWAYS,
}
# The VILLA (bought in Platinum: this game has no VILLA) and the RESTAURANT's paired diners (Platinum's double battles
# by day of the week: the RESTAURANT has its own TRAINERS here)
for _n in ('BYRON', 'CANDICE', 'CRASHER_WAKE', 'CYNTHIA', 'DELIVERY_GUY', 'FANTINA', 'FLINT', 'GARDENIA', 'MAYLENE',
           'MOM', 'PROF_ROWAN', 'RIVAL', 'ROARK', 'SCHOOL_KID_M', 'VOLKNER'):
    SHOW['VILLA_' + _n] = NEVER
for _n in ('DARRYL_VALERIE', 'EMANUEL_BLYTHE', 'EMILIO_KAYLEE', 'ESTEBAN_MEREDITH', 'EUGENE_ALISON', 'ISMAEL_HARLEY',
           'KENDRICK_GABRIELLA', 'LEONARDO_REBECCA', 'ROMAN_KYLIE'):
    SHOW['RESTAURANT_' + _n] = NEVER
# Platinum's day and night trainers: POLICEMEN battle by night and only talk by day; JOGGERS the other way round
for _n in ('ROUTE_212_NORTH_POLICEMAN_ALEX', 'ROUTE_212_NORTH_POLICEMAN_BOBBY', 'ROUTE_212_NORTH_POLICEMAN_CALEB',
           'ROUTE_212_NORTH_POLICEMAN_DYLAN', 'ROUTE_212_SOUTH_POLICEMAN_DANNY', 'ROUTE_222_POLICEMAN_THOMAS'):
    SHOW[_n] = _r(0, time='night')
    SHOW[_n + '_NO_BATTLE'] = _r(0, time='day')
for _n in ('ROUTE_209_JOGGER_RAUL', 'ROUTE_209_JOGGER_RICHARD', 'ROUTE_210_SOUTH_JOGGER_WYATT',
           'ROUTE_215_JOGGER_CRAIG', 'ROUTE_215_JOGGER_SCOTT'):
    SHOW[_n] = _r(0, time='day')
    SHOW[_n + '_NO_BATTLE'] = _r(0, time='night')

_GRUNT_LINES = [
    "We're TEAM GALACTIC! We'll build a brand-new world for everyone. Well... for the BOSS, mostly.",
    "These uniforms are hard to run in. Don't tell anyone I said that.",
    "The BOSS says feelings get in the way. I've been trying not to have any. It's harder than it sounds.",
    "Space! The final... uh, I forgot the rest of the speech. Go away!",
    "I joined for the free meals. Now I'm stuck guarding doors all day.",
    "You won't get past TEAM GALACTIC! ...You're not even trying to get past me? Oh.",
]
_HALL_LINES = [
    "The BOSS is about to speak. Stand up straight!",
    "A new universe... Do you think it'll have weekends?",
    "Shh! The speech is starting soon!",
    "I've been standing here for hours. My feet are a galaxy of pain.",
    "We're all ears for CYRUS. Every single one of us.",
    "When the new world comes, I'm getting a corner office.",
]


def _hall():
    out = {}
    for n in range(1, 37):
        if n in (3, 4):
            continue
        out['Person%d' % n] = _HALL_LINES[n % len(_HALL_LINES)]
    out['Person3'] = ("CYRUS: Look at them. All of them waiting for a new world.|"
                      "Spirit makes people weak. Soon there will be nothing left to make them weak.")
    out['Person4'] = ("LOOKER: Psst! It is I, LOOKER, in my finest disguise!|"
                      "The BOSS of TEAM GALACTIC is up there. I will keep watch down here. Go!")
    return out


STORY_WORDS = {
    'Sinnoh_TwinleafTownPlayerHouse1f': {
        'Person2': "Oh, hello! I live next door. My son BARRY ran off again before breakfast.|"
                   "If you see a blur of yellow and orange heading north, that's him. Tell him lunch is at noon!"},
    'Sinnoh_LakeVerity': {
        'Person6': "PROF. ROWAN: Hm. A TRAINER from another region, here at LAKE VERITY.|"
                   "They say a POKéMON lives in this lake that holds the heart of every person. "
                   "I have spent my life studying such stories. Some of them are even true.",
        'Person7': "I'm PROF. ROWAN's assistant! We came to check on the lake.|"
                   "Something felt different here this morning... like the water was holding its breath."},
    'Sinnoh_SandgemTown': {
        'Person5': "Hi! I help PROF. ROWAN with his research.|"
                   "The lab's right there. He'll want to hear all about the POKéMON where you come from!"},
    'Sinnoh_Route201': {'Person13': ('script', 'Sinnoh_EventScript_Briefcase')},
    'Sinnoh_SandgemTownPokemonResearchLab': {
        'Person1': ('script', 'Sinnoh_EventScript_RowanLab'),
        'Person4': "We've been following TEAM GALACTIC's movements for the PROFESSOR.|"
                   "Whatever they want with the lakes, it can't be good.",
        'Person5': "PROF. OAK: Oh ho! You must be the TRAINER who cleared SINNOH's LEAGUE.|"
                   "ROWAN and I go way back. He was always the serious one. I was always the one with the snacks."},
    'Sinnoh_Route202': {
        'Person4': "Wild POKéMON love this tall grass.|"
                   "If you're heading to JUBILIFE, take your time. The city's so big it never goes anywhere!"},
    'Sinnoh_JubilifeCity': {
        'Person8': "PROF. ROWAN sent me to JUBILIFE for supplies. This city is huge!|"
                   "I got lost twice on the way to the MART. Don't tell the PROFESSOR.",
        'Person18': "We're TEAM GALACTIC. We're... uh, conducting a survey.|"
                    "Question one: how much would you pay for a better universe?",
        'Person19': "Move along, kid. Nothing to see here. Just two totally normal people in matching space suits.",
        'Person25': "Hello hello! The PÓKETCH COMPANY is giving away a brand-new watch!|"
                    "The PRESIDENT himself is handing them out. Their building's right over there!",
        'Person26': "Answer this: what's round, beeps, and tells the time?|A PÓKETCH, of course! Ha ha!",
        'Person28': "I'm practicing my juggling for the PÓKETCH campaign. One ball... two balls... oops.",
        'Person31': "I collect TRAINER CARDS. Yours is from overseas? That's a rare one!",
        'Person32': "LOOKER: Ah! You have noticed me. I am LOOKER, of the INTERNATIONAL POLICE.|"
                    "I am investigating a certain group in strange outfits. If you see them, be careful."},
    'Sinnoh_JubilifeTv2f': {
        'Person3': "We're always looking for stories! A TRAINER who sailed all the way to SINNOH? That's a good one."},
    'Sinnoh_JubilifeTv3fGlobalRankingRoom': {
        'Person4': "This room keeps records from all over. Some of them are frankly unbelievable."},
    'Sinnoh_JubilifeTv3fGroupRankingRoom': {
        'Person3': "Groups of TRAINERS compete for the best records here. Teamwork wins, every time."},
    'Sinnoh_PoketchCo1f': {
        'Person2': "I'm here to interview the PRESIDENT about the newest PÓKETCH app. He keeps showing me the clock."},
    'Sinnoh_TrainersSchool': {
        'Person2': "BARRY: Oh! It's you! I was teaching these kids how to battle.|"
                   "Well, they were teaching me, kind of. Anyway! I'm off to OREBURGH! Last one there pays a fine!"},
    'Sinnoh_OreburghCity': {
        'Person4': "BARRY: The GYM LEADER's down in the mine. I'm going to beat him the second he's back!|"
                   "...After I stock up on POTIONS. And maybe a snack."},
    'Sinnoh_OreburghMineB1f': {
        'Person4': "The mine's tunnels go deeper every year. ROARK says the fossils down here are older than the hills."},
    'Sinnoh_OreburghMineB2f': {
        'Person1': "ROARK: A visitor in my mine? I'm OREBURGH's GYM LEADER.|"
                   "I dig here when I'm not battling. Meet me at the GYM when you're ready. I'll be waiting."},
    'Sinnoh_MtCoronet1fSouth': {
        'Person7': "CYRUS: This mountain is the center of SINNOH. Of everything, perhaps.|"
                   "Long ago, time and space were born here. Imagine if one could begin again."},
    'Sinnoh_FloaromaTown': {
        'Person8': "TEAM GALACTIC's taking the sweet HONEY from the MEADOW. It's for... science!",
        'Person9': "Don't look at me like that. Honey is very important to our plans. Very."},
    'Sinnoh_FloaromaMeadow': {
        'Person1': "This HONEY is ours now! TEAM GALACTIC needs it to lure rare POKéMON.",
        'Person2': "We've got the WINDWORKS' key too. Not that I'll tell YOU that. Oops."},
    'Sinnoh_Route205South': {
        'Person13': "TEAM GALACTIC is busy at the WINDWORKS. Nobody gets through!",
        'Person14': "We're guarding the road. From what? From you, I guess.",
        'Person19': "My daddy works at the WINDWORKS... Some scary people in space suits went in, and he hasn't come out.",
        'Person32': "The windmills here turn all day long. I like to count them. I always lose track at eleven."},
    'Sinnoh_ValleyWindworksOutside': {
        'Person1': "The WINDWORKS is closed! TEAM GALACTIC orders. Shoo!",
        'Person7': "LOOKER: TEAM GALACTIC has taken the WINDWORKS. Its energy could power something very large.|"
                   "Their leader inside is a woman called MARS. Be careful."},
    'Sinnoh_ValleyWindworksBuilding': {
        'Person1': "Turn back! COMMANDER MARS is busy and doesn't want visitors.",
        'Person6': "My daddy's back! Thank you for chasing those mean people away!",
        'Person7': "This place makes so much energy. Imagine what the BOSS could do with it.",
        'Person8': "CHARON: Heh heh. Energy is the key to everything, you see.|"
                   "A little old scientist like me needs a LOT of it."},
    'Sinnoh_EternaForest': {
        'Person1': "CHERYL: The forest is so dark... Would you mind walking with me? "
                   "My CHANSEY and I heal anyone who needs it.|I'll patch up your team whenever you ask.",
        'Person25': "GARDENIA: You're in ETERNA FOREST! I'm GARDENIA, ETERNA's GYM LEADER.|"
                    "I come here to listen to the trees. Visit my GYM soon!"},
    'Sinnoh_EternaCity': {
        'Person4': "This is TEAM GALACTIC's building. No visitors allowed!",
        'Person5': "The BOSS is very busy. Very, very busy. Go away!",
        'Person13': "CYNTHIA: That statue is said to show a POKéMON of time. I study old legends like it.|"
                    "Sometimes I wonder what the people who made it were afraid of.",
        'Person24': "Our ETERNA building is very important. Like a moon base. On the ground.",
        'Person32': "GARDENIA: The GYM's waiting for you! My grass POKéMON have been restless all day.",
        'Person33': "CYRUS: The statue shows time itself. Time keeps moving whether we wish it to or not.|"
                    "I would rather the world began again. Without the parts that hurt.",
        'Person34': "BARRY: ETERNA's got a statue that's like a million years old! "
                    "I raced here to see it. Now I'm racing to the GYM. Bye!"},
    'Sinnoh_TeamGalacticEternaBuilding1f': {
        'Person1': _GRUNT_LINES[0], 'Person2': _GRUNT_LINES[1],
        'Person5': "Hey! Who let you in? ...Oh, you're with the cleaning crew? Fine, go ahead.",
        'Person6': "LOOKER: It is I, LOOKER. I am undercover in this building.|"
                   "The POKéMON they took are upstairs. I shall cover your back. Mostly."},
    'Sinnoh_TeamGalacticEternaBuilding2f': {'Person1': _GRUNT_LINES[2], 'Person2': _GRUNT_LINES[3]},
    'Sinnoh_TeamGalacticEternaBuilding3f': {'Person1': _GRUNT_LINES[4]},
    'Sinnoh_TeamGalacticEternaBuilding4f': {
        'Person2': "Clefairy looks frightened. It huddles close to the old man.",
        'Person3': "They took my CLEFAIRY to study it! Please, help us get out of here!",
        'Person4': "BUNEARY is shivering in the corner."},
    'Sinnoh_CycleShop': {
        'Person1': "Thanks to you, my CLEFAIRY is home safe! Take a look around the shop. Everything rolls!",
        'Person3': "Clefairy twirls around happily."},
    'Sinnoh_Route207': {
        'Person19': "MT. CORONET is ahead. The PROFESSOR says it's the backbone of SINNOH.|"
                    "I'm going to map the cave entrance. Then I'm going to get lost in it, probably."},
    'Sinnoh_Route206CyclingRoadNorthGate': {
        'Person3': "I'm counting cyclists for a study. You'd be amazed how many just ride in circles."},
    'Sinnoh_EternaCityPokecenter1f': {'Person3': "BUNEARY bounces on the spot, ears flopping."},
    'Sinnoh_EternaCitySouthHouse': {
        'Person1': "PROF. OAK: I'm visiting SINNOH to see its POKéMON for myself. Marvelous variety!|"
                   "Your POKéDEX must be getting heavy by now."},
    'Sinnoh_HearthomeCity': {
        'Person22': "HEARTHOME is the friendliest city around. Even the HIKERS stop to say hello.",
        'Person32': "BUNEARY looks up at you curiously, then hops back to its trainer.",
        'Person33': "I'm KEIRA! I love walking my BUNEARY in AMITY SQUARE. It's the best place in the world!"},
    'Sinnoh_HearthomeCityWestGateToAmitySquare': {
        'Person4': "I'm writing a piece on AMITY SQUARE. Every POKéMON there seems to be smiling."},
    'Sinnoh_HearthomeCityEastGateToAmitySquare': {
        'Person3': "Have you been to AMITY SQUARE yet? My camera's full of happy POKéMON."},
    'Sinnoh_HearthomeCityPokemonFanClub': {
        'Person6': "The FAN CLUB is my favorite beat. Everyone here has a story about their POKéMON."},
    'Sinnoh_SolaceonTown': {
        'Person19': "BARRY: SOLACEON's ruins have UNOWN all over the walls! I tried to read them. They said 'GO AWAY'. "
                    "Probably."},
    'Sinnoh_VeilstoneCity': {
        'Person7': "TEAM GALACTIC's headquarters are somewhere in this city. Please be careful!",
        'Person8': "This warehouse is TEAM GALACTIC property. Hands off!",
        'Person21': "The warehouse is locked tight. Not that I lost the key. Nope.",
        'Person24': "I'm supposed to be guarding the STORAGE KEY. I'm doing great. Totally great.",
        'Person25': "Our HQ is the biggest building in VEILSTONE. Can't miss it. Please miss it.",
        'Person26': "WAKE: Hah! TEAM GALACTIC grabbed some POKéMON and ran into that warehouse!|"
                    "I'll be standing guard. Nobody gets past CRASHER WAKE!",
        'Person27': "LOOKER: TEAM GALACTIC's headquarters stands right here in VEILSTONE.|"
                    "I am gathering clues. Clue number one: they really like the color blue."},
    'Sinnoh_GameCorner': {
        'Person9': "LOOKER: Do not mind me. I am simply a normal visitor who enjoys... slots. Yes.|"
                   "TEAM GALACTIC's people come here to relax. I am listening."},
    'Sinnoh_VeilstoneCityGalacticWarehouse': {
        'Person5': "LOOKER: You found the warehouse too! Excellent work.|"
                   "The stolen POKéMON have been returned to their owners. TEAM GALACTIC is losing its grip."},
    'Sinnoh_VeilstoneStoreB1f': {
        'Person10': "PROF. ROWAN: Even a PROFESSOR needs to restock. These POTIONS are for the lab's POKéMON.|"
                    "Keep traveling. Your notes on SINNOH's POKéMON have been most useful."},
    'Sinnoh_PastoriaCity': {
        'Person22': "I'm totally not hiding from TEAM GALACTIC's bosses. I'm resting. In a bush.",
        'Person28': "WAKE: You're the TRAINER everyone's talking about! I'm CRASHER WAKE, PASTORIA's GYM LEADER!|"
                    "Come splash around in my GYM anytime!",
        'Person29': "CROAGUNK puffs out its cheeks and croaks."},
    'Sinnoh_Route213': {
        'Person31': "LAKE VALOR's going to be big news soon. Real big. Boom-big. Forget I said that.",
        'Person34': "LOOKER: TEAM GALACTIC is planning something at LAKE VALOR. I can feel it in my mustache."},
    'Sinnoh_GrandLakeRoute213Lobby': {
        'Person3': "LOOKER: This hotel has a fine view of LAKE VALOR. And of anyone sneaking toward it."},
    'Sinnoh_CelesticTown': {
        'Person3': "This shrine has an old painting of a legend. The BOSS wants it. Very badly.",
        'Person4': "Our town keeps the oldest stories of SINNOH. Some people want to use them for their own ends.",
        'Person5': "CYNTHIA: This is my grandmother's town. The ruins here tell how time and space began.|"
                   "If TEAM GALACTIC wants these stories, they want something terrible."},
    'Sinnoh_CelesticTownCave': {
        'Person1': "The painting in this cave shows the beginning of everything. Treat it gently.",
        'Person3': "CYNTHIA: Look at the wall. Two POKéMON, and one in the shadow between them.|"
                   "Every legend in SINNOH comes back to this one."},
    'Sinnoh_CelesticTownNorthHouse': {
        'Person2': "I'm the elder of CELESTIC. My granddaughter visits now and then. She's quite the TRAINER."},
    'Sinnoh_CanalaveCity': {
        'Person13': "The PROFESSOR wants us all at the library. Something about the lakes!",
        'Person14': "PROF. ROWAN: The library holds a book about SINNOH's three lakes. I must read it at once.",
        'Person19': "BARRY: The library! The PROFESSOR's calling a meeting. Hurry up, I'll fine you if you're late!"},
    'Sinnoh_CanalaveLibrary3f': {
        'Person3': "PROF. ROWAN: The book is clear. The three lakes hold three POKéMON: knowledge, emotion and "
                   "willpower.|If TEAM GALACTIC seizes them, they could seize far more. "
                   "We must split up and protect every lake.",
        'Person4': "I'll head to LAKE VERITY. It's the closest to home!",
        'Person5': "BARRY: I'm taking LAKE ACUITY! Up north where it's cold! Don't worry about me!"},
    'Sinnoh_CanalaveLibrary1f': {
        'Person3': "LUCIAN: Hm? Oh, I'm off duty. Even the ELITE FOUR need a good book now and then."},
    'Sinnoh_CanalaveLibrary2f': {'Person2': "I came to read about mountains. Now I'm too tired to climb one."},
    'Sinnoh_CanalaveCityWestHouse': {
        'Person1': "I'm on vacation from the TV station. Of course, I still have my notebook. Just in case."},
    'Sinnoh_CanalaveCitySailorEldritchHouse': {
        'Person3': "My son has nightmares every night. They say there's an island where nightmares come from...|"
                   "A crescent of light might chase them away."},
    'Sinnoh_IronIsland': {
        'Person2': "RILEY: I'm RILEY. I train on IRON ISLAND with my LUCARIO.|"
                   "Something's not right in the mine today. Wild POKéMON are fleeing the deep tunnels."},
    'Sinnoh_IronIslandB2fLeftRoom': {
        'Person5': "RILEY: TEAM GALACTIC is down here, hurting the POKéMON for their experiments.|"
                   "Let's push them out of the island together!",
        'Person6': "We're just here for the iron! And the POKéMON. And... okay, mostly the POKéMON.",
        'Person7': "RILEY's too strong! Fall back! FALL BAAACK!"},
    'Sinnoh_IronIslandHouse': {
        'Person1': "BYRON: Gahaha! My son runs the CANALAVE GYM now, so I can dig all day long!|"
                   "This island has iron in its bones. Like me!"},
    'Sinnoh_Route218GateToCanalaveCity': {
        'Person2': "I'm studying the sea breeze. It smells of salt and adventure. Mostly salt."},
    'Sinnoh_LakeValorDrained': {
        'Person1': "We drained the lake! The BOSS will be so proud!",
        'Person2': "The POKéMON of this lake is ours now. Too bad for the MAGIKARP.",
        'Person3': "COMMANDER SATURN went into the cavern. You'll never catch him!"},
    'Sinnoh_ValorLakefront': {
        'Person1': "CYNTHIA: TEAM GALACTIC set off a bomb at LAKE VALOR! The lake is... gone.|"
                   "Whatever they plan, it needs the three lake POKéMON. Go, I'll help the people here.",
        'Person2': "We came to film the lake. Now we're filming a crater.",
        'Person3': "The explosion shook the whole hotel. Everyone's okay, thank goodness.",
        'Person6': "Don't go near the lake! Uh, for your own safety. Definitely not because we're busy there.",
        'Person9': "My lakeside vacation is ruined. Someone owes me a refund. And a lake.",
        'Person10': "I came to buy a piece of LAKE VALOR's famous lakeshore. Now it's just shore.",
        'Person11': "BARRY: What happened here?! A bomb?! TEAM GALACTIC is going to pay! I'll fine them a million!"},
    'Sinnoh_GrandLakeValorLakefrontEastHouse': {
        'Person1': "My view of the lake was the best in town. I just want the water back."},
    'Sinnoh_LakeVerity': {
        'Person9': "COMMANDER MARS is busy catching the lake's POKéMON. Wait your turn!"},
    'Sinnoh_LakeAcuity': {
        'Person2': "JUPITER: The POKéMON of LAKE ACUITY is ours. Your little friend couldn't stop us.|"
                   "Go home. Our HQ is no place for children.",
        'Person3': "BARRY: I... I lost. They took the lake's POKéMON right in front of me.|"
                   "Their HQ is in VEILSTONE. You go! I'll catch up, I promise!"},
    'Sinnoh_GalacticHq3f': {'Person1': _GRUNT_LINES[5]},
    'Sinnoh_GalacticHqB1f': {'Person1': _GRUNT_LINES[0]},
    'Sinnoh_GalacticHqB2f': {
        'Person1': _GRUNT_LINES[2],
        'Person2': "The red chains are almost ready. With them, the BOSS can command the legendary POKéMON."},
    'Sinnoh_GalacticHqLaboratory': {
        'Person1': "Our machines pull the power out of POKéMON. It's very efficient. Don't look at me like that.",
        'Person2': "I just do the math. The math says this is a bad idea. Nobody listens to the math."},
    'Sinnoh_GalacticHqControlRoom': {
        'Person1': "SATURN: The lake POKéMON are in those tubes. Free them if you can. I won't stop you.|"
                   "I'm starting to think our BOSS doesn't care what happens to any of us.",
        'Person2': "AZELF floats behind the glass, eyes closed.",
        'Person3': "MESPRIT floats behind the glass, eyes closed.",
        'Person4': "UXIE floats behind the glass, eyes closed.",
        'Person5': "CHARON: Heh heh. The red chains are made. CYRUS has gone to SPEAR PILLAR.|"
                   "Whatever happens next, I'll be watching from a safe distance."},
    'Sinnoh_GalacticHqHall': _hall(),
    'Sinnoh_MtCoronet1fNorthRoom1': {
        'Person16': "The summit is off limits! TEAM GALACTIC business!",
        'Person17': "SPEAR PILLAR is at the top. The BOSS is already there. You're too late!",
        'Person18': "Wait for us at the top, BOSS! We'll... we'll take the stairs!"},
    'Sinnoh_MtCoronet2f': {
        'Person6': "LOOKER: The mountain is full of TEAM GALACTIC. I will hold them off here.|"
                   "Keep climbing! The fate of SINNOH is at the top!"},
    'Sinnoh_MtCoronet1fTunnelRoom': {'Person17': "No one passes! ...Unless you beat my friends. Then maybe."},
    'Sinnoh_SpearPillar': {
        'Person1': "SPEAR PILLAR is ours! The BOSS is calling the POKéMON of time and space!",
        'Person4': "Did you feel that? The ground's humming. Something big is coming.",
        'Person5': "JUPITER: MARS, together! This one's too much for either of us alone.",
        'Person6': "BARRY: I made it! Whoa, look at them all. Let's take them on together!",
        'Person2': ('script', 'Sinnoh_EventScript_DistortionEnter')},
    'Sinnoh_SnowpointCity': {
        'Person8': "CANDICE: A visitor in SNOWPOINT! I'm CANDICE, the GYM LEADER here.|"
                   "Snow's no reason to stay inside! Come show me what you've got at the GYM!"},
    'Sinnoh_SnowpointCityPokecenter1f': {
        'Person5': "MAYLENE: I came north to train in the snow. Cold is good for focus!|Achoo! ...Mostly good."},
    'Sinnoh_Route217': {
        'Person16': "MAYLENE: Running through snow is the best training! My legs are numb, but my spirit is warm!"},
    'Sinnoh_Route217NortheastHouse': {
        'Person1': "We've got hot soup here for anyone braving the snow. You look like you need some!"},
    'Sinnoh_SunyshoreCity': {
        'Person19': "FLINT: Hey! I'm FLINT, of the ELITE FOUR. VOLKNER's my friend. He's been bored lately.|"
                    "Beat him and wake him up, would you? He needs a battle that lights him up.",
        'Person21': "BARRY: SUNYSHORE has solar panels on the roads! I tried running on them. "
                    "Felt like I was going faster!"},
    'Sinnoh_VistaLighthouse': {
        'Person3': "VOLKNER: From up here you can see all the way to the LEAGUE.|"
                   "I've been waiting for a TRAINER who makes battles exciting again. Maybe it's you."},
    'Sinnoh_Route221': {
        'Person16': "We're building a park up north. A place where POKéMON from far away come to play.",
        'Person17': "Pass me that shovel! ...No, the other shovel. The shiny one."},
    'Sinnoh_VictoryRoad1f': {
        'Person12': "VICTORY ROAD is the last test before the LEAGUE. I'm collecting the courage to go on."},
    'Sinnoh_VictoryRoad1fRoom2': {
        'Person29': "MARLEY: ...Hi. I'm MARLEY. I train here, quietly.|...You can walk with me, if you want."},
    'Sinnoh_WaywardCave1f': {
        'Person5': "MIRA: Mira is lost... Mira is always lost in here.|Will you help Mira find the way out?"},
    'Sinnoh_FightArea': {
        'Person9': "BUCK: Yo! I'm BUCK. I love tough battles more than anything.|"
                   "STARK MOUNTAIN's where I'm headed next. Something's waking up inside it.",
        'Person25': "VOLKNER: I came to see if the BATTLE ZONE's TRAINERS are as strong as they say. "
                    "So far, so shocking.",
        'Person26': "FLINT: The FRONTIER's got TRAINERS who burn hotter than any LEAGUE. I love it here!",
        'Person27': "PALMER: I'm PALMER, the TOWER TYCOON! My son BARRY never stops talking about you.|"
                    "Come challenge the BATTLE TOWER. I'll be waiting at the top.",
        'Person28': "That battle was incredible! I can't stop cheering!",
        'Person29': "I'm studying every move these TRAINERS make. My notebook's on fire.",
        'Person30': "Watch closely. The best TRAINERS read the battle like a book.",
        'Person31': "I came all the way here just to watch. Best decision ever!",
        'Person32': "I sailed in from SNOWPOINT. My boat's still frozen, but my heart's warm!"},
    'Sinnoh_SurvivalArea': {
        'Person9': "BUCK: The FRONTIER's tough, but STARK MOUNTAIN's tougher. I'll see you up there!"},
    'Sinnoh_Battleground': {
        'Person2': "BUCK: Battleground's my favorite place to warm up. Wanna see me warm up? It's very warm."},
    'Sinnoh_Route227': {
        'Person4': "BARRY: STARK MOUNTAIN's up ahead! Its ash gets in your shoes. I'm gonna fine the volcano!",
        'Person5': "WAKE: The ashy road makes for great training! Hah! My cape is gray now.",
        'Person12': "BUCK: Up the road is STARK MOUNTAIN. TEAM GALACTIC's leftovers went in. Let's go!"},
    'Sinnoh_StarkMountainOutside': {
        'Person5': "LOOKER: CHARON of TEAM GALACTIC is inside. He wants the POKéMON that sleeps in the magma.|"
                   "I am going in. Well. After you.",
        'Person6': "BUCK: Charon's after the mountain's POKéMON! We stop him, right?",
        'Person7': "This mountain is TEAM GALACTIC territory now! Kind of. Sort of.",
        'Person8': "It's so hot in here. Space suits were a terrible idea."},
    'Sinnoh_StarkMountainRoom1': {
        'Person15': "CHARON: Heh heh. With the POKéMON in this mountain, TEAM GALACTIC will rise again. "
                    "Led by me, of course.",
        'Person16': "COMMANDER CHARON's in charge now. I think. Nobody told me.",
        'Person17': "I miss the HQ. It had air conditioning."},
    'Sinnoh_StarkMountainRoom2': {
        'Person15': "BUCK: Phew, it's hot! I'll keep the path behind us clear. You go on ahead!"},
    'Sinnoh_StarkMountainRoom3': {
        'Person1': "CROAGUNK croaks proudly at CHARON's feet.",
        'Person3': "BUCK: Phew. The mountain's quiet again. Thanks for the help, CHAMPION!",
        'Person5': "LOOKER: CHARON is under arrest! Excellent work. TEAM GALACTIC is truly finished.",
        'Person6': "CHARON: Arrested? Me? I'm just a harmless old scientist! ...Fine, a mostly harmless one.",
        'Person8': "We'll take it from here. Thank you for your help.",
        'Person9': "Everyone stay calm. The suspect is very slow. This won't take long.",
        'Person10': "We surrender! Can we leave the volcano now?",
        'Person11': "I'm going home to become a florist."},
    'Sinnoh_Route224': {
        'Person4': "PROF. OAK: This route leads to a stone where people once left their feelings.|"
                   "What a place to study! What a place for a picnic!",
        'Person22': "MARLEY: ...The sea here is quiet. I like quiet."},
    'Sinnoh_SendoffSpring': {
        'Person1': "CYNTHIA: This spring is where SINNOH's oldest stories say the world turns back on itself.|"
                   "There's a cave beyond it. Strange things live there. Be careful, CHAMPION."},
    'Sinnoh_ResortArea': {
        'Person14': "My dad brought me here for a vacation. I'm going to swim until I'm a prune!",
        'Person15': "PROF. ROWAN: Even I take a holiday now and then. Don't tell my assistants.",
        'Person16': "GARDENIA: The flowers here are amazing! I'm taking seeds home for my GYM.",
        'Person17': "BYRON: Gahaha! A fine place to rest these old bones.",
        'Person18': "ROARK: Dad's here too? Ugh. Can't even vacation in peace.",
        'Person19': "MAYLENE: Resting is training too! ...Is that true? I hope that's true."},
    'Sinnoh_BattleTowerBattleSalon': {
        'Person2': "CHERYL: The TOWER's TRAINERS are so strong. I'm here to heal everyone afterward!",
        'Person3': "MIRA: Mira likes the TOWER. It's the only building Mira doesn't get lost in.",
        'Person4': "RILEY: I come here to test my aura against the best. It's humbling.",
        'Person5': "MARLEY: ...It's loud in here. But the battles are good.",
        'Person6': "BUCK: Want a partner for the TOWER? Hah! I'd win every round!"},
    'Sinnoh_BattleTower': {
        'Person13': "I'm covering the BATTLE TOWER's records. Some of these streaks are legendary."},
    'Sinnoh_BattleHall': {'Person11': "The BATTLE HALL's crowd is the loudest in SINNOH!"},
    'Sinnoh_BattleFrontier': {
        'Person30': "The BATTLE FRONTIER is the place to see the toughest TRAINERS. Got a story for me?"},
    'Sinnoh_Route214GateToVeilstoneCity': {
        'Person2': "I'm writing about VEILSTONE's huge store. It sells everything except quiet."},
    'Sinnoh_OldChateauDiningArea': {'Person1': "......|The old man smiles, and then he isn't there."},
    'Sinnoh_OldChateauBackMiddleEastRoom': {'Person2': "......|The girl giggles, and the room grows cold."},
    'Sinnoh_PokemonMansionOffice': {
        'Person4': "I guard the owner's office. Mostly I guard it from boredom."},
    'Sinnoh_Route209': {
        'Person30': "Night jogging is peaceful. Too dark to battle, though. Come back by day!",
        'Person31': "Running at night keeps me cool. I'm too out of breath to battle anyway."},
    'Sinnoh_Route210South': {'Person31': "I only battle in daylight. At night I just run and think about soup."},
    'Sinnoh_Route212North': {
        'Person25': "Patrolling by day is quiet. At night, though, I check every TRAINER who walks by.",
        'Person26': "Daytime duty is easy. Come by at night and we'll see how tough you are.",
        'Person27': "Everything's calm on ROUTE 212. That's how I like it.",
        'Person28': "I'm on day patrol. My battle shift starts after sunset."},
    'Sinnoh_Route212South': {'Person41': "The rain keeps the troublemakers home. By night, though... watch out."},
    'Sinnoh_Route215': {
        'Person26': "I'm running laps till sunrise. Battles? Not till morning!",
        'Person27': "Night runs are my favorite. No battling though. Too sleepy."},
    'Sinnoh_Route222': {'Person30': "The beach is quiet by day. Night patrol's where the action is."},
}
