"""Platinum's overworld sprite and movement names -> the game's own (tools/sinnoh/gen_maps.py, SINNOH's people). The game
has no Platinum sprites yet: each person gets the closest one it has, by kind (a trainer class, an item ball, a rock);
signs, mailboxes and books are read where they stand (sign()).
"""
import os
import re

# Platinum name (without OBJ_EVENT_GFX_) -> the game's (without OBJ_EVENT_GFX_); a name the game has itself (YOUNGSTER,
# LASS, HIKER, TWIN, TEALA...) needs no line: it is used as it is (OWN, from include/constants/event_objects.h)
SPRITES = {
    'ROCK_SMASH': 'BREAKABLE_ROCK', 'STRENGTH_BOULDER': 'PUSHABLE_BOULDER', 'CUT_TREE': 'CUTTABLE_TREE',
    'POKEBALL': 'ITEM_BALL', 'MAGIKARP': 'ITEM_BALL', 'BRIEFCASE': 'BIRCHS_BAG', 'KID_WITH_NDS': 'GAMEBOY_KID',
    # trainers and townsfolk
    'GRUNT_M': 'MAGMA_MEMBER_M', 'GRUNT_F': 'MAGMA_MEMBER_F', 'GRUNTS_GROUP_OF_3': 'MAGMA_MEMBER_M',
    'GRUNTS_GROUP_OF_4': 'MAGMA_MEMBER_M', 'ACE_TRAINER_M': 'MAN_3', 'ACE_TRAINER_F': 'WOMAN_5',
    'ACE_TRAINER_SNOW_M': 'MAN_3', 'ACE_TRAINER_SNOW_F': 'WOMAN_5', 'COLLECTOR': 'FAT_MAN',
    'POKEMON_BREEDER_F': 'WOMAN_2', 'POKEMON_BREEDER_M': 'MAN_2', 'WORKER': 'HIKER', 'RUIN_MANIAC': 'HIKER',
    'CASHIER_F': 'MART_EMPLOYEE', 'CASHIER_M': 'MART_EMPLOYEE', 'BATTLE_GIRL': 'WOMAN_5', 'PSYCHIC': 'PSYCHIC_M',
    'GUITARIST': 'MAN_5', 'RECEPTIONIST': 'LINK_RECEPTIONIST', 'SCIENTIST_M': 'SCIENTIST_1', 'SCIENTIST_F': 'SCIENTIST_2',
    'GYM_GUIDE': 'MAN_1', 'POLICEMAN': 'MAN_4', 'POKECENTER_NURSE': 'NURSE', 'PROF_ROWAN': 'PROF_BIRCH',
    'PROF_OAK': 'GENTLEMAN', 'SCHOOL_KID_F': 'LASS', 'MIDDLE_AGED_MAN': 'MAN_2', 'MIDDLE_AGED_WOMAN': 'WOMAN_2',
    'LOOKER': 'GENTLEMAN', 'REPORTER': 'REPORTER_F', 'LADY': 'BEAUTY', 'IDOL': 'BEAUTY', 'SOCIALITE': 'BEAUTY',
    'PARASOL_LADY': 'WOMAN_2', 'COWGIRL': 'GIRL_1', 'RANCHER': 'MAN_2', 'MAID': 'WOMAN_4', 'WAITRESS': 'WOMAN_4',
    'WAITER': 'COOK', 'CLOWN': 'MANIAC', 'BIKER': 'CYCLING_TRIATHLETE_M', 'ROUGHNECK': 'MAN_3', 'JOGGER':
    'RUNNING_TRIATHLETE_M', 'CYCLIST_M': 'CYCLING_TRIATHLETE_M', 'CYCLIST_F': 'CYCLING_TRIATHLETE_F', 'SKIER_M': 'MAN_1',
    'SKIER_F': 'WOMAN_1', 'SNOWPOINT_NPC_M': 'MAN_1', 'SNOWPOINT_NPC_F': 'WOMAN_1', 'GAME_DIRECTOR': 'MAN_2',
    'FRONTIER_BOOTH_ATTENDANT': 'LINK_RECEPTIONIST', 'FRONTIER_MULTI_ATTENDANT': 'LINK_RECEPTIONIST',
    'FRONTIER_SINGLE_ATTENDANT': 'LINK_RECEPTIONIST', 'MYSTERY_GIFT_DELIVERYMAN': 'MYSTERY_GIFT_MAN',
    # the story's people (shown only once their scripts are written: gen_maps.py)
    'BARRY': 'RIVAL_BRENDAN_NORMAL', 'DAWN': 'MAY_NORMAL', 'LUCAS': 'BRENDAN_NORMAL', 'COUNTERPART': 'MAY_NORMAL',
    'VAR_0': 'MAY_NORMAL', 'CYNTHIA': 'WOMAN_3', 'MARS': 'MAGMA_MEMBER_F', 'JUPITER': 'MAGMA_MEMBER_F',
    'SATURN': 'MAGMA_MEMBER_M', 'CYRUS': 'MAXIE', 'CHARON': 'SCIENTIST_1', 'AARON': 'BUG_CATCHER',
    'BERTHA': 'OLD_WOMAN', 'FLINT': 'MAN_3', 'LUCIAN': 'PSYCHIC_M', 'ROARK': 'HIKER', 'GARDENIA': 'PICNICKER',
    'MAYLENE': 'WOMAN_5', 'CRASHER_WAKE': 'MAN_3', 'FANTINA': 'BEAUTY', 'BYRON': 'HIKER', 'CANDICE': 'LASS',
    'VOLKNER': 'MAN_5', 'BUCK': 'BOY_3', 'CHERYL': 'WOMAN_2', 'MIRA': 'LITTLE_GIRL', 'RILEY': 'MAN_4',
    'MARLEY': 'GIRL_3', 'PALMER': 'GENTLEMAN', 'JASMINE': 'WOMAN_3',
    # POKeMON standing in the world: the closest the game has
    'CLEFAIRY': 'AZURILL', 'HAPPINY': 'AZURILL', 'BUNEARY': 'AZURILL', 'PACHIRISU': 'PIKACHU', 'PSYDUCK': 'ZIGZAGOON_1',
    'MACHOP': 'ZIGZAGOON_1', 'CROAGUNK': 'ZIGZAGOON_1', 'STARLY': 'WINGULL', 'DRIFLOON': 'WINGULL',
    'SHROOMISH': 'ZIGZAGOON_1', 'TORCHIC': 'ZIGZAGOON_1',
}
# kinds of thing that are no person to see (Platinum draws them, or they mean nothing here yet); SIGNS are read where they
# stand (gen_maps.py turns them into the map's signs)
SIGNS = re.compile(r'SIGNPOST|SIGNBOARD|MAILBOX|BOOK$')
HIDDEN = re.compile(r'SIGN|VENT|BOLLARD|SNOWBALL|WIFI|INVISIBLE|BERRY_SOIL|MAILBOX|BOOK$|DOOR|CAVE_PAINTING|_ROCK$|'
                    r'ROTOM|WALL_|BABY_IN_PRAM|^VAR_[1-9A-F]$|ARCEUS|AZELF|MESPRIT|UXIE|DARKRAI|CRESSELIA|SHAYMIN|'
                    r'GIRATINA|HEATRAN|REGIGIGAS')

_own = None


def own():
    """The game's own sprite names"""
    global _own
    if _own is None:
        here = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
        text = open(os.path.join(here, 'include/constants/event_objects.h')).read()
        _own = set(re.findall(r'#define OBJ_EVENT_GFX_([A-Z0-9_]+)\s', text)) - {'VARS'}
    return _own


MOVEMENT = {
    'MOVEMENT_TYPE_NONE': 'MOVEMENT_TYPE_FACE_DOWN', 'MOVEMENT_TYPE_WANDER_AROUND': 'MOVEMENT_TYPE_WANDER_AROUND',
    'MOVEMENT_TYPE_LOOK_SOUTH': 'MOVEMENT_TYPE_FACE_DOWN', 'MOVEMENT_TYPE_LOOK_NORTH': 'MOVEMENT_TYPE_FACE_UP',
    'MOVEMENT_TYPE_LOOK_WEST': 'MOVEMENT_TYPE_FACE_LEFT', 'MOVEMENT_TYPE_LOOK_EAST': 'MOVEMENT_TYPE_FACE_RIGHT',
    'MOVEMENT_TYPE_LOOK_AROUND': 'MOVEMENT_TYPE_LOOK_AROUND',
    'MOVEMENT_TYPE_WANDER_WEST_AND_EAST': 'MOVEMENT_TYPE_WANDER_LEFT_AND_RIGHT',
    'MOVEMENT_TYPE_WANDER_NORTH_AND_SOUTH': 'MOVEMENT_TYPE_WANDER_UP_AND_DOWN',
    'MOVEMENT_TYPE_LOOK_SOUTH_AND_WEST': 'MOVEMENT_TYPE_FACE_DOWN_AND_LEFT',
    'MOVEMENT_TYPE_LOOK_SOUTH_AND_EAST': 'MOVEMENT_TYPE_FACE_DOWN_AND_RIGHT',
    'MOVEMENT_TYPE_LOOK_NORTH_AND_WEST': 'MOVEMENT_TYPE_FACE_UP_AND_LEFT',
    'MOVEMENT_TYPE_LOOK_NORTH_AND_EAST': 'MOVEMENT_TYPE_FACE_UP_AND_RIGHT',
    'MOVEMENT_TYPE_LOOK_NORTH_AND_SOUTH': 'MOVEMENT_TYPE_FACE_DOWN_AND_UP',
    'MOVEMENT_TYPE_WALK_BACK_AND_FORTH': 'MOVEMENT_TYPE_WALK_UP_AND_DOWN',
}


def sprite(platinum):
    """The game's OBJ_EVENT_GFX_ name for a Platinum sprite, or None: not shown as a person"""
    name = platinum[len('OBJ_EVENT_GFX_'):] if platinum.startswith('OBJ_EVENT_GFX_') else platinum
    if HIDDEN.search(name):
        return None
    if name in SPRITES:
        return 'OBJ_EVENT_GFX_' + SPRITES[name]
    if name in own() and not name.startswith('VAR_'):
        return 'OBJ_EVENT_GFX_' + name
    return 'OBJ_EVENT_GFX_MAN_1'


def sign(platinum):
    """A sign, mailbox or book: read where it stands"""
    name = platinum[len('OBJ_EVENT_GFX_'):] if platinum.startswith('OBJ_EVENT_GFX_') else platinum
    return bool(SIGNS.search(name))


def movement(platinum):
    return MOVEMENT.get(platinum, 'MOVEMENT_TYPE_FACE_DOWN')
