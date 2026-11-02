#!/usr/bin/env python3
"""The SINNOH maps as the field engine's maps (docs/SINNOH_HANDOFF.md): every outdoor place on Platinum's map matrix 0
(its towns, cities, routes and lakefronts) as a map of group 40, with map.json, scripts.inc, a layout and their places
in map_groups.json / layouts.json, and the 3D engine's table of where each one lies on the matrix
(src/sinnoh/s3d_maps.h). Only the places' names, rectangles, music, type and weather are read from pret/pokeplatinum
(its map matrix and map header table); nothing of Platinum's art is written here. The layouts' blocks are placeholders
(every cell blocked): the game takes each cell's collision and behavior from the 3D pack when it loads the map
(src/sinnoh/s3d.c S3D_LayoutBlocks, from Platinum's terrain attributes by build_pack.py).

    python3 -I tools/sinnoh/gen_maps.py POKEPLATINUM_DIR      (run from the repository; safe to run again)

Buildings (each its own matrix, tools/sinnoh/places.py) go in groups 41 on, 120 a group (a warp's map number is an
s8), by name. Every place gets
its warps from its events file (warps into places left out are dropped). A place covering an L of matrix cells (ETERNA CITY, OREBURGH CITY, ROUTE 209, ROUTE 224) becomes the rectangle around
it; the corner left over is the matrix's filler (sea, mountains), blocked. Maps whose rectangles touch are connected.
Map numbers stay as they were (TWINLEAF TOWN 0, ROUTE 201 1); new places are added after the last.
"""
import json
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from source import open_source  # noqa: E402
from places import places as kept_places, events  # noqa: E402
import ids  # noqa: E402
import items  # noqa: E402
import gen_trainers  # noqa: E402
import sprites  # noqa: E402
import legends  # noqa: E402
import story_people  # noqa: E402
import gen_townmap  # noqa: E402

GROUP = 'gMapGroup_Sinnoh'                      # the outdoor places
INDOOR_GROUP = 'gMapGroup_SinnohIndoor%d'       # the buildings, PER_GROUP a group (1, 2, 3...)
PER_GROUP = 120                 # (a warp keeps its map number as an s8: at most 127 maps a group)
BLOCKED = 0x0001 | (1 << 10) | (3 << 12)   # metatile 1, collision, elevation 3
# the tilesets aren't used: the 3D engine draws the picture and each block's metatile id carries its behavior
# (tools/sinnoh/build_pack.py BEHAVIOR, fieldmap.c GetMetatileAttributesById)
SECONDARY = 'gTileset_Johto_GoldenrodUndergroundEntrances'

# Platinum's day songs -> SINNOH's song slots (tools/kaisong/sinnoh_songs.py; S3D_MapMusic plays <slot>_night by night)
MUSIC = {
    'SEQ_TOWN01_D_sseq': 'twinleaf', 'SEQ_TOWN02_D_sseq': 'sandgem', 'SEQ_TOWN03_D_sseq': 'floaroma',
    'SEQ_TOWN06_D_sseq': 'route225', 'SEQ_TOWN07_D_sseq': 'valor_lakefront',
    'SEQ_CITY01_D_sseq': 'jubilife', 'SEQ_CITY02_D_sseq': 'canalave', 'SEQ_CITY03_D_sseq': 'oreburgh',
    'SEQ_CITY04_D_sseq': 'eterna', 'SEQ_CITY05_D_sseq': 'hearthome', 'SEQ_CITY06_D_sseq': 'solaceon',
    'SEQ_CITY07_D_sseq': 'veilstone', 'SEQ_CITY08_D_sseq': 'sunyshore', 'SEQ_CITY09_D_sseq': 'snowpoint',
    'SEQ_CITY10_D_sseq': 'league', 'SEQ_CITY11_D_sseq': 'fight_area',
    'SEQ_ROAD_A_D_sseq': 'route201', 'SEQ_ROAD_B_D_sseq': 'route203', 'SEQ_ROAD_C_D_sseq': 'route205',
    'SEQ_ROAD_D_D_sseq': 'route206', 'SEQ_ROAD_E_D_sseq': 'route209', 'SEQ_ROAD_F_D_sseq': 'route210',
    'SEQ_ROAD_SNOW_D_sseq': 'route216', 'SEQ_ROAD_BZA_D_sseq': 'route228',
    'SEQ_D_LAKE_sseq': 'lake', 'SEQ_D_RYAYHY_sseq': 'cavern_on_the_lake',
    # buildings, caves and the rest
    'SEQ_PC_01_sseq': 'pokecenter', 'SEQ_FS_sseq': 'pokemart', 'SEQ_GYM_sseq': 'gym', 'SEQ_KENKYUJO_sseq': 'research_lab',
    'SEQ_D_01_sseq': 'victory_road', 'SEQ_D_02_sseq': 'eterna_forest', 'SEQ_D_03_sseq': 'cavern_on_the_lake',
    'SEQ_D_04_sseq': 'oreburgh_mine', 'SEQ_D_05_sseq': 'mt_coronet', 'SEQ_D_06_sseq': 'iron_mountain',
    'SEQ_D_MOUNT1_sseq': 'mt_coronet', 'SEQ_D_MOUNT2_sseq': 'spear_pillar', 'SEQ_D_GINLOBBY_sseq': 'galactic_building',
    'SEQ_D_AGITO_sseq': 'galactic_hideout', 'SEQ_THE_EVENT04_sseq': 'galactic_hideout_deep',
    'SEQ_D_LEAGUE_sseq': 'league_showdown', 'SEQ_BLD_DENDO_sseq': 'hall_of_fame', 'SEQ_D_SAFARI_sseq': 'great_marsh',
    'SEQ_D_KOUEN_sseq': 'amity_square', 'SEQ_BLD_GAME_sseq': 'game_corner', 'SEQ_BLD_TV_sseq': 'jubilife',
    'SEQ_BF_TOWWER_sseq': 'battle_tower', 'SEQ_PL_BF_FACTORY_sseq': 'battle_tower', 'SEQ_PL_BF_STAGE_sseq': 'battle_tower',
    'SEQ_PL_BF_CASTLE_sseq': 'battle_tower', 'SEQ_PL_BF_ROULETTE_sseq': 'battle_tower',
    'SEQ_PL_D_GIRATINA_sseq': 'distortion_world', 'SEQ_AUS_sseq': 'arceus', 'SEQ_SILENCE_FIELD_sseq': 'villa',
}
# Platinum's overworld weather -> Emerald's
WEATHER = {
    'OVERWORLD_WEATHER_RAINING': 'WEATHER_RAIN', 'OVERWORLD_WEATHER_ROUTE_212_SOUTH': 'WEATHER_RAIN',
    'OVERWORLD_WEATHER_FOG': 'WEATHER_FOG_HORIZONTAL', 'OVERWORLD_WEATHER_BLIZZARD': 'WEATHER_SNOW',
    'OVERWORLD_WEATHER_SNOWPOINT_CITY': 'WEATHER_SNOW', 'OVERWORLD_WEATHER_ACUITY_LAKEFRONT': 'WEATHER_SNOW',
    'OVERWORLD_WEATHER_ROUTE_216': 'WEATHER_SNOW', 'OVERWORLD_WEATHER_SANDSTORM': 'WEATHER_SANDSTORM',
    'OVERWORLD_WEATHER_SLOW_ASHFALL': 'WEATHER_VOLCANIC_ASH',
}


def ident(header):
    """MAP_HEADER_ROUTE_204_NORTH -> ('ROUTE204_NORTH', 'Route204North')"""
    name = header[len('MAP_HEADER_'):]
    const = re.sub(r'_(\d)', r'\1', name)
    camel = ''.join(p.capitalize() for p in re.split(r'_', name))
    return const, camel


def places(src, root):
    out = []
    for kp in kept_places(src, root):
        header, f, cs = kp['header'], kp['fields'], kp['cells']
        xs, zs = [c[0] for c in cs], [c[1] for c in cs]
        const, camel = ident(header)
        ptype = f.get('mapType')
        if ptype == 'MAP_TYPE_TOWN_CITY':
            mtype = 'MAP_TYPE_CITY' if 'CITY' in const else 'MAP_TYPE_TOWN'
        elif ptype == 'MAP_TYPE_CAVE':
            mtype = 'MAP_TYPE_UNDERGROUND'
        elif ptype == 'MAP_TYPE_INDOORS':
            mtype = 'MAP_TYPE_INDOOR'
        else:
            mtype = 'MAP_TYPE_ROUTE'
        out.append({'header': header, 'id': 'MAP_SINNOH_' + const, 'name': 'Sinnoh_' + camel,
                    'matrix': kp['matrix'], 'outdoor': kp['outdoor'], 'place': kp,
                    'x0': min(xs), 'z0': min(zs), 'x1': max(xs), 'z1': max(zs),
                    'music': 'MUS_SINNOH_' + MUSIC.get(f.get('dayMusicID'), 'twinleaf').upper(),
                    'type': mtype, 'weather': WEATHER.get(f.get('weather'), 'WEATHER_SUNNY'),
                    'bike': f.get('isBikeAllowed') == 'TRUE'})
    return out





def labels(name):
    """The scripts a map's own scripts.inc has (SINNOH's words are written for this game, not taken from Platinum's
    scripts: docs/SINNOH_HANDOFF.md step 4). A person, sign or trigger runs <map>_EventScript_Person<n> / _Sign<n> /
    _Bg<n> / _Trigger<n> when the file has it (n: its number in Platinum's lists, from 1)"""
    path = 'data/maps/%s/scripts.inc' % name
    if not os.path.exists(path):
        return set()
    return set(re.findall(r'^(\w+)::', open(path, encoding='utf-8').read(), re.M))


used_items = set()   # (item, count) of every item ball: data/scripts/sinnoh_items.inc


def inside(p, x, z):
    """A Platinum spot (whole-matrix tiles) inside the place's map"""
    return 0 <= x - p['x0'] * 32 < 32 * (p['x1'] - p['x0'] + 1) and 0 <= z - p['z0'] * 32 < 32 * (p['z1'] - p['z0'] + 1)


# People this game adds to a place, after Platinum's (their local ids follow): the GAME CENTER's attendant (as in
# GOLDENROD's and CELADON's GAME CORNERS: data/scripts/mp_region.inc EventScript_GameCenterGuide)
EXTRA_PEOPLE = {
    # the DISTORTION WORLD's arena (data/scripts/sinnoh_places.inc): CYRUS, GIRATINA and CYNTHIA on Platinum's rock
    'Sinnoh_DistortionWorldGiratinaRoom': [
        {'graphics_id': 'OBJ_EVENT_GFX_SINNOH_CYRUS', 'x': 15, 'y': 25, 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN',
         'script': 'Sinnoh_EventScript_DistortionCyrus', 'flag': '0x3D34'},
        {'graphics_id': 'OBJ_EVENT_GFX_SINNOH_GIRATINA_ALTERED', 'x': 15, 'y': 22, 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN',
         'script': 'Sinnoh_EventScript_DistortionGiratina', 'flag': '0x3D36'},
        {'graphics_id': 'OBJ_EVENT_GFX_SINNOH_CYNTHIA', 'x': 18, 'y': 26, 'movement_type': 'MOVEMENT_TYPE_FACE_LEFT',
         'script': 'Sinnoh_EventScript_DistortionCynthia'},
    ],
    'Sinnoh_VeilstoneCityPrizeExchange': [
        {'graphics_id': 'OBJ_EVENT_GFX_SINNOH_UNUSED_RECEPTIONIST', 'x': 7, 'y': 4, 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN',
         'script': 'EventScript_GameCenterGuide'},
    ],
}


# Platinum's people changed for this game's scripts: {map: {Platinum's object number: fields}}. The SINNOH BATTLE
# TOWER's room: the opponent is Emerald's tower opponent (VAR_OBJ_GFX_ID_0, its hide flag), the player's stand-in
# hidden (the player walks in). (BARRY's mom stays on the player's house's door mat, where her visit starts:
# EXTRA_TRIGGERS)
OVERRIDES = {
    # people Platinum stands on a door for a scene this game doesn't play (they shut players in or out):
    # CRASHER WAKE on PASTORIA's gym door (shown with 3 badges: the 4th is in there) and VEILSTONE's, the sailor on
    # his house's only door, the policemen on STARK MOUNTAIN's room's only way out
    'Sinnoh_PastoriaCity': {28: {'x': 12, 'y': 28, 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN'}},
    'Sinnoh_VeilstoneCity': {26: {'x': 11, 'y': 36, 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN'}},
    'Sinnoh_CanalaveCitySailorEldritchHouse': {3: {'x': 3, 'y': 7, 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN'}},
    'Sinnoh_StarkMountainRoom3': {8: {'x': 6, 'y': 16, 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN'},
                                  9: {'x': 8, 'y': 16, 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN'}},
    'Sinnoh_BattleTowerBattleRoom': {2: {'graphics_id': 'OBJ_EVENT_GFX_VAR_0', 'flag': 'FLAG_HIDE_BATTLE_TOWER_OPPONENT',
                                         'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'script': '0x0'},
                                     3: {'flag': '0x3FFF'}},
}


def talk_trainer(root, p, o):
    """The trainer (gen_trainers.py TALK) a person's Platinum script battles, or None"""
    import who
    import gifts
    s = o['script']
    if not isinstance(s, int):
        return None
    low = p['place']['header'][len('MAP_HEADER_'):].lower()
    names = who.entries(root, low)
    if not 1 <= s <= len(names):
        return None
    path = os.path.join(root, 'res/field/scripts/scripts_%s.s' % low)
    if not os.path.exists(path):
        return None
    body = gifts._body(open(path, encoding='utf-8', errors='replace').read(), names[s - 1], set())
    for t in re.findall(r'StartTrainerBattle (TRAINER_\w+)', body):
        if t in gen_trainers.TALK:
            return t
    return None


OBSTACLES = {'OBJ_EVENT_GFX_CUT_TREE': 'EventScript_CutTree', 'OBJ_EVENT_GFX_ROCK_SMASH': 'EventScript_RockSmash',
             'OBJ_EVENT_GFX_STRENGTH_BOULDER': 'EventScript_StrengthBoulder'}


def people(p, root, have):
    """The place's people: where they stand, the game's closest sprite (tools/sinnoh/sprites.py), how they move, their
    hide flag (SINNOH's number, tools/sinnoh/ids.py) and their script. Local ids follow Platinum's order; those with
    nothing to show stay, invisible (FLAG_SINNOH_HIDDEN, 0x3FFF, always set), to keep the order: signs (read as the
    map's signs), a neighbor's people Platinum loads early, and anyone the story shows or hides (a hide flag) until
    their script is written."""
    out = []
    for i, o in enumerate(events(root, p['place'])['object_events']):
        label = '%s_EventScript_Person%d' % (p['name'], i + 1)
        gfx = sprites.sprite(o['graphics_id'])
        hide = o.get('hidden_flag')
        flag = ids.flag(root, hide) if isinstance(hide, str) and hide.startswith('FLAG_') else None
        ghost = isinstance(hide, str) and hide.startswith('MAP_HEADER_')
        script = label if label in have else '0x0'
        trainer = isinstance(o['script'], str) and o['script'].startswith('TRAINER_')
        if trainer:
            # a trainer: its battle script (data/scripts/sinnoh_trainers.inc, tools/sinnoh/gen_trainers.py) and sight
            script = gen_trainers.label(o['script'])
        talk = talk_trainer(root, p, o)
        if talk:
            # (one who battles when talked to: gen_trainers.py TALK; no sight)
            script = gen_trainers.label(talk)
            hide = None if not isinstance(hide, str) or not hide.startswith('FLAG_') else hide
        obstacle = OBSTACLES.get(o['graphics_id'])
        if obstacle and gfx and inside(p, o['x'], o['z']):
            # a CUT tree, a ROCK SMASH rock or a STRENGTH boulder: the game's own script; no flag, so it's back each
            # time the map loads (as Platinum's and the game's own)
            out.append({'graphics_id': gfx, 'x': o['x'] - p['x0'] * 32, 'y': o['z'] - p['z0'] * 32, 'elevation': 3,
                        'movement_type': 'MOVEMENT_TYPE_NONE', 'movement_range_x': 0, 'movement_range_y': 0,
                        'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                        'script': obstacle, 'flag': '0'})
            continue
        legend = legends.scripts().get((p['name'], i + 1))
        if legend:
            # a legendary: its battle (tools/sinnoh/legends.py), shown by its hide flag (Sinnoh_LegendsRefresh)
            out.append({'graphics_id': gfx, 'x': o['x'] - p['x0'] * 32, 'y': o['z'] - p['z0'] * 32, 'elevation': 3,
                        'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0, 'movement_range_y': 0,
                        'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                        'script': legend, 'flag': '0x%X' % flag})
            continue
        # a story person (tools/sinnoh/story_people.py): shown by the story (Sinnoh_StoryRefresh sets its flag)
        story = story_people.SHOW.get(hide[len('FLAG_HIDE_'):]) if isinstance(hide, str) and hide.startswith('FLAG_HIDE_') \
            else None
        ball = items.ball(root, o['script']) if o['graphics_id'].endswith('POKEBALL') else None
        if ball and not inside(p, o['x'], o['z']):
            ball = None
        if ball:
            # an item ball: the shared script for its item (data/scripts/sinnoh_items.inc), its flag set once taken
            assert flag, '%s: item ball without a flag' % label
            script = items.script_label(*ball)
            used_items.add(ball)
            flag_s = '0x%X' % flag
        elif gfx is None or ghost or story == story_people.NEVER or o['graphics_id'].endswith('POKEBALL') \
                or (flag and not (story and (trainer or talk)) and label not in have) or not inside(p, o['x'], o['z']):
            flag_s = '0x3FFF'
        else:
            flag_s = ('0x%X' % flag) if flag else '0'
        out.append({'graphics_id': gfx or 'OBJ_EVENT_GFX_ITEM_BALL', 'x': o['x'] - p['x0'] * 32, 'y': o['z'] - p['z0'] * 32,
                    'elevation': 3, 'movement_type': sprites.movement(o['movement_type']),
                    'movement_range_x': o.get('movement_range_x', 0), 'movement_range_y': o.get('movement_range_z', 0),
                    'trainer_type': 'TRAINER_TYPE_NORMAL' if trainer else 'TRAINER_TYPE_NONE',
                    'trainer_sight_or_berry_tree_id': str((o.get('data') or [0])[0]) if trainer else '0',
                    'script': script, 'flag': flag_s})
    for x, z, species, script, flag in legends.added(p['name']):
        out.append({'graphics_id': 'OBJ_EVENT_GFX_SINNOH_' + species, 'x': x - p['x0'] * 32, 'y': z - p['z0'] * 32,
                    'elevation': 3, 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0,
                    'movement_range_y': 0, 'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                    'script': script, 'flag': '0x%X' % flag})
    for n, fields in OVERRIDES.get(p['name'], {}).items():
        out[n - 1].update(fields)
    for e in EXTRA_PEOPLE.get(p['name'], []):
        out.append({'graphics_id': e['graphics_id'], 'x': e['x'], 'y': e['y'], 'elevation': 3,
                    'movement_type': e['movement_type'], 'movement_range_x': 0, 'movement_range_y': 0,
                    'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                    'script': e['script'], 'flag': e.get('flag', '0')})
    return out


def signs(p, root, have):
    """Platinum's signs are people (a signpost, a mailbox, a book): each is a sign where it stands; then its own signs"""
    facing = {'BG_EVENT_DIR_NORTH': 'BG_EVENT_PLAYER_FACING_NORTH', 'BG_EVENT_DIR_SOUTH': 'BG_EVENT_PLAYER_FACING_SOUTH',
              'BG_EVENT_DIR_WEST': 'BG_EVENT_PLAYER_FACING_WEST', 'BG_EVENT_DIR_EAST': 'BG_EVENT_PLAYER_FACING_EAST'}
    out = []
    ev = events(root, p['place'])
    for i, o in enumerate(ev['object_events']):
        label = '%s_EventScript_Sign%d' % (p['name'], i + 1)
        if sprites.sign(o['graphics_id']) and label in have and inside(p, o['x'], o['z']):
            out.append({'type': 'sign', 'x': o['x'] - p['x0'] * 32, 'y': o['z'] - p['z0'] * 32, 'elevation': 0,
                        'player_facing_dir': 'BG_EVENT_PLAYER_FACING_ANY', 'script': label})
    for i, b in enumerate(ev['bg_events']):
        label = '%s_EventScript_Bg%d' % (p['name'], i + 1)
        if not inside(p, b['x'], b['z']):
            continue
        hid = items.buried(root, b['script']) if b.get('type') == 2 else None
        if hid:
            item, count, flag = hid    # (a count above 1 is given as 1: the game's hidden items hold one)
            out.append({'type': 'hidden_item', 'x': b['x'] - p['x0'] * 32, 'y': b['z'] - p['z0'] * 32, 'elevation': 0,
                        'item': item, 'flag': '0x%X' % flag})
            continue
        if b.get('type') != 0 or label not in have:
            continue
        out.append({'type': 'sign', 'x': b['x'] - p['x0'] * 32, 'y': b['z'] - p['z0'] * 32, 'elevation': 0,
                    'player_facing_dir': facing.get(b['player_facing_dir'], 'BG_EVENT_PLAYER_FACING_ANY'),
                    'script': label})
    return out


# Triggers of this game's own: BARRY's mom's visit in the player's house (data/scripts/sinnoh_places.inc
# Sinnoh_EventScript_RivalMomVisit, once: the three tiles next to the door mat, while VAR_SINNOH_ARRIVAL is 2 (in
# SINNOH); the visit sets it to 3. A trigger whose script only ends still holds the player on its tile here, so the
# var turns it off, not a check in the script)
EXTRA_TRIGGERS = {
    'Sinnoh_TwinleafTownPlayerHouse1f': [(6, 9), (5, 10), (7, 10)],
}
EXTRA_TRIGGER_SCRIPT = {
    'Sinnoh_TwinleafTownPlayerHouse1f': 'Sinnoh_EventScript_RivalMomVisit',
}


def triggers(p, root, have):
    """Platinum's coord events cover an area: the game's are one tile each. Only those with a script of this game's"""
    out = [{'type': 'trigger', 'x': x, 'y': y, 'elevation': 0, 'var': '0x52F0', 'var_value': '2',
            'script': EXTRA_TRIGGER_SCRIPT[p['name']]} for x, y in EXTRA_TRIGGERS.get(p['name'], [])]
    for i, c in enumerate(events(root, p['place'])['coord_events']):
        label = '%s_EventScript_Trigger%d' % (p['name'], i + 1)
        v = ids.var(root, c['var']) if isinstance(c.get('var'), str) else None
        if v is None or label not in have or not inside(p, c['x'], c['z']):
            continue
        for dz in range(max(1, c.get('length', 1))):
            for dx in range(max(1, c.get('width', 1))):
                out.append({'type': 'trigger', 'x': c['x'] - p['x0'] * 32 + dx, 'y': c['z'] - p['z0'] * 32 + dz,
                            'elevation': 0, 'var': v if isinstance(v, str) else '0x%X' % v,
                            'var_value': str(int(c['value'])) if isinstance(c['value'], (bool, int)) else str(c['value']),
                            'script': label})
    return out


# The GREAT MARSH: Platinum's six headers share one matrix (each a block of it, its own people, items and wild
# Pokemon). The game's player stays on the 6th's map (the gate's warp leads there) for the whole marsh: it holds the
# other five's people, items and signs too, and the wild Pokemon are the block's (src/sinnoh/s3d_legends.c
# Sinnoh_GreatMarshArea)
MARSH_MAP = 'Sinnoh_GreatMarsh6'


def marsh_merge(m, every, root):
    def spot(e):
        return (e['x'], e['y'])
    shown = {spot(o) for o in m['object_events'] if o['flag'] != '0x3FFF'}
    signed = {spot(b) for b in m['bg_events']}
    for q in every:
        if not q['name'].startswith('Sinnoh_GreatMarsh') or q['name'] == MARSH_MAP:
            continue
        have = labels(q['name'])
        for o in people(q, root, have):
            if o['flag'] != '0x3FFF' and spot(o) not in shown:
                m['object_events'].append(o)
                shown.add(spot(o))
        for b in signs(q, root, have):
            if spot(b) not in signed:
                m['bg_events'].append(b)
                signed.add(spot(b))


DYNAMIC = 'MAP_HEADER_DYNAMIC'   # an elevator's doors: the floor it stopped at (Emerald's MAP_DYNAMIC, set by the
                                 # floor menu: Sinnoh_EventScript_Elevator, src/sinnoh/s3d_elevator.c)


# Elevators: Platinum's run by script (a panel or an attendant, then the doors open on the floor picked); here the
# panel / attendant opens a floor menu (Sinnoh_EventScript_Elevator, data/scripts/sinnoh_elevators.inc) and its doors
# are a dynamic warp, as Emerald's LILYCOVE elevator. The floors are the places with a door into the elevator.
ELEVATOR_SCRIPT = 'Sinnoh_EventScript_Elevator'
PANEL = (1, 5)   # (where Platinum's elevator rooms have their panel: the VISTA LIGHTHOUSE's gets one there too)


def floor_rank(p):
    """Bottom to top: B1F -1, nF n, a place outside (the lighthouse's door in SUNYSHORE) 1, anything else the top"""
    m = re.search(r'B(\d)f$', p['name'], re.I)
    if m:
        return -int(m.group(1))
    m = re.search(r'(\d)f$', p['name'], re.I)
    if m:
        return int(m.group(1))
    return 1 if p['outdoor'] else 99


def floor_name(p):
    r = floor_rank(p)
    return 'B%dF' % -r if r < 0 else 'TOP' if r == 99 else '%dF' % r


def elevators(every, root):
    """{elevator place name: (place, [floor places, top first])}"""
    out = {}
    for e in every:
        if not any(w['dest_header_id'] == DYNAMIC for w in events(root, e['place'])['warp_events']):
            continue
        floors = [q for q in every
                  if any(w['dest_header_id'] == e['header'] for w in events(root, q['place'])['warp_events'])]
        if len(floors) >= 2 or (floors and floor_rank(floors[0]) != 99):
            out[e['name']] = (e, sorted(floors, key=floor_rank, reverse=True))
    return out


def elevator_panel(m):
    """The elevator's panel and attendant open the floor menu (a panel where Platinum has none)"""
    for o in m['object_events']:
        if o['graphics_id'].endswith('RECEPTIONIST') and o['flag'] != '0x3FFF':
            o['script'] = ELEVATOR_SCRIPT
    m['bg_events'] = [b for b in m['bg_events'] if (b['x'], b['y']) != PANEL]
    m['bg_events'].append({'type': 'sign', 'x': PANEL[0], 'y': PANEL[1], 'elevation': 0,
                           'player_facing_dir': 'BG_EVENT_PLAYER_FACING_ANY', 'script': ELEVATOR_SCRIPT})


# HONEY trees: Platinum's is a map prop (model honey_tree.nsbmd), two tiles square, faced from below; here the two
# tiles of its trunk are signs for the north-facing player (Sinnoh_EventScript_HoneyTree, data/scripts/sinnoh_wild.inc;
# src/sinnoh/s3d_wild.c knows the tree by the map)
HONEY_SCRIPT = 'Sinnoh_EventScript_HoneyTree'


def honey_tree(m, p, src):
    from platinum import Land
    model = src._props.index('honey_tree.nsbmd')
    mat = src.matrix(p['matrix'])
    for cx, cz in p['place']['cells']:
        for pr in Land(src.land(mat.cell(cx, cz)[0])).props:
            if pr.model != model:
                continue
            tx, tz = int(pr.pos[0] // 16 + 16), int(pr.pos[2] // 16 + 16)   # (the prop's point: its bottom-right tile)
            for dx in (-1, 0):
                x, y = (cx - p['x0']) * 32 + tx + dx, (cz - p['z0']) * 32 + tz
                m['bg_events'] = [b for b in m['bg_events'] if (b['x'], b['y']) != (x, y)]
                m['bg_events'].append({'type': 'sign', 'x': x, 'y': y, 'elevation': 0,
                                       'player_facing_dir': 'BG_EVENT_PLAYER_FACING_NORTH', 'script': HONEY_SCRIPT})


def write_elevators(lifts):
    rows, names = [], {}
    for name, (e, floors) in sorted(lifts.items()):
        assert len(floors) <= 8, name
        for f in floors:
            names.setdefault(floor_name(f), 'sText_Floor%d' % len(names))
        rows.append('    {%s, %d, {%s}, {%s}},\n' % (e['id'], len(floors), ', '.join(f['id'] for f in floors),
                                                   ', '.join(names[floor_name(f)] for f in floors)))
    open('src/data/sinnoh_elevators.h', 'w').write(
        '// Generated by tools/sinnoh/gen_maps.py: SINNOH\'s elevators (src/sinnoh/s3d_elevator.c) and their floors, top '
        'first\n' + ''.join('static const u8 %s[] = _("%s");\n' % (v, k) for k, v in names.items()) +
        'static const struct { u16 map; u8 count; u16 floors[8]; const u8 *names[8]; } sSinnohElevators[] = {\n'
        + ''.join(rows) + '};\n')


def kept_warps(p, ids, root):
    """[(Platinum's warp number, warp)] of the place's warps that lead to kept places with a way out of their own, and an
    elevator's doors (MAP_DYNAMIC: the floor it stopped at)"""
    return [(i, w) for i, w in enumerate(events(root, p['place'])['warp_events'])
            if w['dest_header_id'] == DYNAMIC or (w['dest_header_id'] in ids and w['dest_header_id'] in _exits(ids, root))]


_exit_cache = {}


def _exits(ids, root):
    """Headers of the kept places that have a warp to a kept place"""
    key = id(ids)
    if key not in _exit_cache:
        _exit_cache[key] = {h for h, p in _places_by_header.items()
                            if any(w['dest_header_id'] in ids or w['dest_header_id'] == DYNAMIC
                                   for w in events(root, p['place'])['warp_events'])}
    return _exit_cache[key]


_places_by_header = {}


def warps(p, ids, root, numbers):
    """The place's warps as the field engine's: local tiles, the kept place each leads to (warps to places left out
    are dropped) and the warp there, renumbered as that place's kept warps are (numbers: {map id: {Platinum's warp
    number: ours}}); a warp to one that was dropped goes to that place's first"""
    out = []
    for _, w in kept_warps(p, ids, root):
        if w['dest_header_id'] == DYNAMIC:
            out.append({'x': w['x'] - p['x0'] * 32, 'y': w['z'] - p['z0'] * 32, 'elevation': 0, 'dest_map': 'MAP_DYNAMIC',
                        'dest_warp_id': 'WARP_ID_DYNAMIC'})
            continue
        dest = ids[w['dest_header_id']]
        to = numbers.get(dest, {}).get(w['dest_warp_id'], 0)
        out.append({'x': w['x'] - p['x0'] * 32, 'y': w['z'] - p['z0'] * 32, 'elevation': 0, 'dest_map': dest,
                    'dest_warp_id': str(to)})
    return out


def connections(p, every):
    conns = []
    if not p['outdoor']:
        return conns
    for q in every:
        if q is p or not q['outdoor']:
            continue
        if q['z1'] + 1 == p['z0'] or q['z0'] == p['z1'] + 1:        # above / below, overlapping in x
            if q['x0'] <= p['x1'] and q['x1'] >= p['x0']:
                conns.append({'map': q['id'], 'offset': (q['x0'] - p['x0']) * 32,
                              'direction': 'up' if q['z1'] + 1 == p['z0'] else 'down'})
        elif q['x1'] + 1 == p['x0'] or q['x0'] == p['x1'] + 1:      # left / right, overlapping in z
            if q['z0'] <= p['z1'] and q['z1'] >= p['z0']:
                conns.append({'map': q['id'], 'offset': (q['z0'] - p['z0']) * 32,
                              'direction': 'left' if q['x1'] + 1 == p['x0'] else 'right'})
    return sorted(conns, key=lambda c: (c['direction'], c['offset'], c['map']))


def main():
    root = sys.argv[1]
    src = open_source(root)
    every = places(src, root)
    map_ids = {p['header']: p['id'] for p in every}
    _places_by_header.update({p['header']: p for p in every})
    groups = json.load(open('data/maps/map_groups.json'))
    layouts = json.load(open('data/layouts/layouts.json'))
    # the outdoor places in group 40 (numbers kept), the buildings after them in groups of PER_GROUP (their numbers
    # kept too: new ones go after the last)
    outdoor = [p for p in every if p['outdoor']]
    indoor = sorted((p for p in every if not p['outdoor']), key=lambda p: p['name'])
    if GROUP not in groups:
        groups['group_order'].append(GROUP)
        groups[GROUP] = []
    known = groups[GROUP]
    outdoor.sort(key=lambda p: (p['name'] not in known, known.index(p['name']) if p['name'] in known else 0, p['name']))
    for p in outdoor:
        if p['name'] not in known:
            known.append(p['name'])
    order = list(groups['group_order'])
    # (the buildings keep their numbers: those already in the groups first, in their order, new ones after the last)
    before = [n for g in order if g.startswith('gMapGroup_SinnohIndoor') for n in groups[g]]
    indoor.sort(key=lambda p: (p['name'] not in before, before.index(p['name']) if p['name'] in before else 0, p['name']))
    for g in [g for g in order if g.startswith('gMapGroup_SinnohIndoor')]:
        groups['group_order'].remove(g)
        del groups[g]
    nindoor = (len(indoor) + PER_GROUP - 1) // PER_GROUP
    at = groups['group_order'].index(GROUP) + 1
    for k in range(nindoor):
        name = INDOOR_GROUP % (k + 1)
        groups['group_order'].insert(at + k, name)
        groups[name] = [p['name'] for p in indoor[k * PER_GROUP:(k + 1) * PER_GROUP]]
    numbers = {p['id']: {pi: k for k, (pi, _) in enumerate(kept_warps(p, map_ids, root))} for p in every}
    lifts = elevators(every, root)
    town = gen_townmap.write(root, every)   # (each map's TOWN MAP section, and the TOWN MAP's tables)
    for p in outdoor + indoor:
        bw, bh = p['x1'] - p['x0'] + 1, p['z1'] - p['z0'] + 1
        w, h = 32 * bw, 32 * bh
        layout_id = 'LAYOUT_' + p['id'][4:]
        ldir = 'data/layouts/' + p['name']
        os.makedirs(ldir, exist_ok=True)
        open(ldir + '/map.bin', 'wb').write(struct.pack('<H', BLOCKED) * (w * h))
        open(ldir + '/border.bin', 'wb').write(struct.pack('<H', BLOCKED) * 4)
        entry = {'id': layout_id, 'name': p['name'] + '_Layout', 'width': w, 'height': h,
                 'primary_tileset': 'gTileset_General', 'secondary_tileset': SECONDARY,
                 'border_filepath': ldir + '/border.bin', 'blockdata_filepath': ldir + '/map.bin'}
        layouts['layouts'] = [l for l in layouts['layouts'] if l.get('id') != layout_id] + [entry]
        mdir = 'data/maps/' + p['name']
        os.makedirs(mdir, exist_ok=True)
        path = mdir + '/map.json'
        old = json.load(open(path)) if os.path.exists(path) else {}
        m = {'id': p['id'], 'name': p['name'], 'layout': layout_id, 'music': p['music'],
             'region_map_section': gen_townmap.mapsec_of(town, p['place']), 'requires_flash': False, 'weather': p['weather'],
             'map_type': p['type'], 'allow_cycling': p['bike'], 'allow_escaping': not p['outdoor'],
             'allow_running': True, 'show_map_name': False, 'battle_scene': 'MAP_BATTLE_SCENE_NORMAL',
             'connections': connections(p, every)}
        have = labels(p['name'])
        m['object_events'] = people(p, root, have)
        m['warp_events'] = warps(p, map_ids, root, numbers)
        m['coord_events'] = triggers(p, root, have)
        m['bg_events'] = signs(p, root, have)
        if p['name'] == MARSH_MAP:
            marsh_merge(m, every, root)
        if p['name'] in lifts:
            elevator_panel(m)
        honey_tree(m, p, src)
        json.dump(m, open(path, 'w'), indent=2)
        open(path, 'a').write('\n')
        if not os.path.exists(mdir + '/scripts.inc'):
            open(mdir + '/scripts.inc', 'w').write('%s_MapScripts::\n\t.byte 0\n' % p['name'])
    json.dump(groups, open('data/maps/map_groups.json', 'w'), indent=2)
    open('data/maps/map_groups.json', 'a').write('\n')
    json.dump(layouts, open('data/layouts/layouts.json', 'w'), indent=2)
    open('data/layouts/layouts.json', 'a').write('\n')
    # the maps' scripts, and the 3D engine's table (map, its matrix, its top-left cell there, its size in cells)
    names = known + [p['name'] for p in indoor]
    path = 'data/scripts/sinnoh_items.inc'
    open(path, 'w').write('@ Generated by tools/sinnoh/gen_maps.py: SINNOH\'s item balls, one script an item (tools/sinnoh/items.py)\n' +
                          ''.join('%s::\n\tfinditem %s, %d\n\tend\n\n' % (items.script_label(i, c), i, c)
                                  for i, c in sorted(used_items)))
    assert ids.FLAGS_START + ids.counts(root)[0] <= items.HIDDEN_FLAGS, 'SINNOH flags run into the hidden items\''
    path = 'data/event_scripts_sinnoh.s'
    text = open(path).read()
    head = text[:text.index('\t.include "data/maps/Sinnoh_')]
    if 'sinnoh_items.inc' not in head:
        head += '\t.include "data/scripts/sinnoh_items.inc"\n'
    if 'sinnoh_marts.inc' not in head:
        head += '\t.include "data/scripts/sinnoh_marts.inc"\n'
    if 'sinnoh_trainers.inc' not in head:
        head += '\t.include "data/scripts/sinnoh_trainers.inc"\n'
    if 'sinnoh_elevators.inc' not in head:
        head += '\t.include "data/scripts/sinnoh_elevators.inc"\n'
    if 'sinnoh_wild.inc' not in head:
        head += '\t.include "data/scripts/sinnoh_wild.inc"\n'
    if 'sinnoh_places.inc' not in head:
        head += '\t.include "data/scripts/sinnoh_places.inc"\n'
    if 'platinum_scripts.inc' not in head:
        # Platinum's own scripts, converted when the game is built (tools/sinnoh/gen_scripts.py; never committed)
        head += '\t.include "sinnoh/platinum_scripts.inc"\n'
    open(path, 'w').write(head + ''.join('\t.include "data/maps/%s/scripts.inc"\n' % n for n in names))
    rows = ''.join('    {%s, %d, %d, %d, %d, %d},\n' % (p['id'], p['matrix'], p['x0'], p['z0'], p['x1'] - p['x0'] + 1,
                                                       p['z1'] - p['z0'] + 1) for p in outdoor + indoor)
    open('src/sinnoh/s3d_maps.h', 'w').write(
        '// Generated by tools/sinnoh/gen_maps.py: each SINNOH map, its Platinum matrix (0 the region), the cell of its\n'
        '// top-left block there and its size in blocks (src/sinnoh/s3d.c)\n'
        '#define SINNOH_GROUPS %d   // group 40 and the buildings\' groups after it\n' % (1 + nindoor) +
        'static const struct { u16 map, mat; u8 cx, cz, bw, bh; } sMaps[] = {\n' + rows + '};\n')
    write_elevators(lifts)
    print('%d maps (%d outdoor, %d buildings in %d groups, %d elevators)' % (len(every), len(outdoor), len(indoor),
                                                                            nindoor, len(lifts)))


if __name__ == '__main__':
    main()
