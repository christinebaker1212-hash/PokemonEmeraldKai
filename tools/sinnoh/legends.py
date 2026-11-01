#!/usr/bin/env python3
"""SINNOH's legendaries standing on the map, as Platinum stands them (its events' objects), in Platinum's order: UXIE,
AZELF and MESPRIT in their caverns and GIRATINA in TURNBACK CAVE once Platinum's story would have given them (the 7th
BADGE: after SPEAR PILLAR), the others after the LEAGUE (Platinum's postgame and its event islands).

    python3 -I tools/sinnoh/legends.py POKEPLATINUM_DIR      (before gen_maps.py, which gives the objects these scripts)

Writes data/scripts/sinnoh_legends.inc (each one's battle: Emerald's static encounter), src/data/sinnoh_legends.h
(where each stands, its object's flag: Platinum's hide flag's SINNOH number, and its "done" flag 0x3D00 + n: caught or
beaten) and each map's on-load hook (special Sinnoh_LegendsRefresh, src/sinnoh/s3d_legends.c: shown or hidden).
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ids  # noqa: E402

DONE_FLAGS = 0x3D00   # (free: the trainers' flags end at 0x391F, the hidden items' start at 0x3E00)
LEAGUE = 0xFF
FRIDAYS = 0xFE        # every Friday from the 1st BADGE (TEAM GALACTIC gone from the VALLEY WINDWORKS), once a Friday
# "done" flags past the first 12 (0x3D0C-0x3D0F are the REGIS' and ROTOM's, the map scripts'): a row's own
OWN_DONE = {'DRIFLOON': 0x3D32}
# (map, its Platinum object number, species, level, the SINNOH badges it waits for, LEAGUE: SINNOH cleared)
LEGENDS = [
    ('Sinnoh_AcuityCavern', 1, 'UXIE', 50, 7),
    ('Sinnoh_ValorCavern', 1, 'AZELF', 50, 7),
    ('Sinnoh_VerityCavern', 1, 'MESPRIT', 50, 7),
    ('Sinnoh_TurnbackCaveGiratinaRoom', 1, 'GIRATINA', 47, 7),
    ('Sinnoh_StarkMountainRoom3', 2, 'HEATRAN', 50, LEAGUE),
    ('Sinnoh_SnowpointTempleB5f', 1, 'REGIGIGAS', 1, LEAGUE),
    ('Sinnoh_FullmoonIslandForest', 1, 'CRESSELIA', 50, LEAGUE),
    ('Sinnoh_NewmoonIslandForest', 1, 'DARKRAI', 40, LEAGUE),
    ('Sinnoh_FlowerParadise', 1, 'SHAYMIN', 30, LEAGUE),
    ('Sinnoh_HallOfOrigin', 1, 'ARCEUS', 80, LEAGUE),
    # Platinum brings these by script (MT. CORONET's top leading to SPEAR PILLAR's other versions): here they stand at
    # the top of SPEAR PILLAR after the LEAGUE, beside where CYRUS waits in the story (('at', x, z): Platinum's tiles)
    ('Sinnoh_SpearPillar', ('at', 29, 28), 'DIALGA', 70, LEAGUE),
    ('Sinnoh_SpearPillar', ('at', 33, 28), 'PALKIA', 70, LEAGUE),
    # not a legendary: Platinum's Friday DRIFLOON at the VALLEY WINDWORKS' door (s3d_legends.c FRIDAYS)
    ('Sinnoh_ValleyWindworksOutside', 5, 'DRIFLOON', 15, FRIDAYS),
]
EXTRA_FLAGS = 0x3D80  # the hide flags of the ones this game adds (('at', x, z)), one each

# The story's battles (BARRY, TEAM GALACTIC), standing where Platinum's story puts them, each once, in Platinum's
# order: (map, its Platinum object number, the trainers (Platinum's names: RIVAL_<PLACE> in its three versions by
# the player's Trainer ID; two: one after the other), the SINNOH badges it waits for, LEAGUE: after it, and its words
# (meeting, losing, after). Their "done" flags: STORY_FLAGS + n.
STORY_FLAGS = 0x3D10
STORY = [
    ('Sinnoh_Route201', 3, ['RIVAL_ROUTE_201'], 0, (
        "BARRY: There you are! A new TRAINER\nin SINNOH? Let's see what you've got!",
        "Whoa! You're really good!", "I'm gonna get stronger! Just you wait!")),
    ('Sinnoh_Route203', 6, ['RIVAL_ROUTE_203'], 0, (
        "BARRY: Hey! I've been training! Round\ntwo, right now!", "Again?! How?!", "OREBURGH's GYM is next. Race you!")),
    ('Sinnoh_ValleyWindworksBuilding', 2, ['COMMANDER_MARS_VALLEY_WINDWORKS'], 1, (
        "MARS: TEAM GALACTIC needs this power\nplant's energy. Move aside, kid!",
        "Ugh! You're tougher than you look.", "This isn't over. TEAM GALACTIC never\ngives up!")),
    ('Sinnoh_TeamGalacticEternaBuilding4f', 1, ['COMMANDER_JUPITER_TEAM_GALACTIC_ETERNA_BUILDING'], 2, (
        "JUPITER: Breaking into our building?\nBold. I'll teach you some manners.",
        "Hmph. Not bad at all.", "Keep meddling and you'll meet our boss.")),
    ('Sinnoh_Route209GateToHearthomeCity', 2, ['RIVAL_ROUTE_209'], 2, (
        "BARRY: Two BADGES already? Me too!\nLet's battle, here and now!", "No way! Not again!",
        "HEARTHOME's GYM is tough. Good luck!")),
    ('Sinnoh_PastoriaCity', 27, ['RIVAL_PASTORIA_CITY'], 3, (
        "BARRY: Found you! I'm about to take on\nWAKE. Warm-up battle!", "Argh! Fine, fine!",
        "WAKE's next! For both of us!")),
    ('Sinnoh_CanalaveCity', 12, ['RIVAL_CANALAVE_CITY'], 5, (
        "BARRY: Stop right there! Before BYRON,\nyou face me!", "You're unbelievable!",
        "I'll catch up to you. I swear!")),
    ('Sinnoh_LakeVerity', 8, ['COMMANDER_MARS_LAKE_VERITY'], 6, (
        "MARS: You again! The lake's POKéMON\nis ours. Out of my way!", "Not again!",
        "MESPRIT's power will be ours anyway!")),
    ('Sinnoh_ValorCavern', 2, ['COMMANDER_SATURN_VALOR_CAVERN'], 6, (
        "SATURN: So you're the TRAINER MARS\nkeeps complaining about.", "Interesting. Very interesting.",
        "Our boss's plan won't wait for you.")),
    ('Sinnoh_GalacticHq1f', 10, ['COMMANDER_SATURN_GALACTIC_HQ'], 7, (
        "SATURN: You came all the way here?\nThen let's settle this.", "I see now why the boss is wary of you.",
        "Go on. CYRUS is waiting above.")),
    ('Sinnoh_GalacticHq4f', 1, ['GALACTIC_BOSS_CYRUS_GALACTIC_HQ'], 7, (
        "CYRUS: Emotion is a flaw. Let me show\nyou the strength of a cold heart.",
        "You fight with your heart... and win.", "The new world will come all the same.")),
    ('Sinnoh_SpearPillar', 3, ['COMMANDER_MARS_SPEAR_PILLAR', 'COMMANDER_JUPITER_SPEAR_PILLAR'], 7, (
        "MARS: You won't reach the top of\nSPEAR PILLAR! JUPITER, now!", "We... lost? Both of us?",
        "Go, then. See what CYRUS has done.")),
    ('Sinnoh_PokemonLeagueNorthPokecenter1f', 6, ['RIVAL_POKEMON_LEAGUE'], 8, (
        "BARRY: Eight BADGES! Before the LEAGUE,\none last battle with me!", "You're ready. Go be CHAMPION!",
        "I'll be cheering for you!")),
    ('Sinnoh_SurvivalArea', 12, ['RIVAL_SURVIVAL_AREA_1'], LEAGUE, (
        "BARRY: Hey, CHAMPION! The BATTLE\nFRONTIER made me tougher. Battle!", "Still unbeatable...",
        "My dad runs the TOWER, you know!")),
    ('Sinnoh_FightArea', 8, ['RIVAL_FIGHT_AREA'], LEAGUE, (
        "BARRY: Let's team up... after one more\nbattle!", "Fine! You win again!", "See you at the FRONTIER!")),
    ('Sinnoh_StarkMountainRoom1', 1, ['COMMANDER_MARS_STARK_MOUNTAIN', 'COMMANDER_JUPITER_STARK_MOUNTAIN'], LEAGUE, (
        "MARS: CHARON wants HEATRAN's power.\nJUPITER and I will stop you!", "Even together...",
        "TEAM GALACTIC is finished. Maybe that's\nfor the best.")),
    ('Sinnoh_CelesticTownCave', 2, ['GALACTIC_BOSS_CYRUS_CELESTIC_TOWN_RUINS'], LEAGUE, (
        "CYRUS: I came back to see this place\nagain. Battle me, CHAMPION.", "As expected of you.",
        "Perhaps a world with spirit isn't\nso flawed.")),
]


def story_people_header(root):
    """src/data/sinnoh_story_people.h: each story person's hide flag and when it shows (tools/sinnoh/story_people.py)"""
    import story_people as sp
    assert len(sp.DONE) == len(STORY), 'story_people.DONE must name every STORY battle'
    done = {n: STORY_FLAGS + k for k, n in enumerate(sp.DONE)}
    rows = []
    for name, (lo, hi, need, unless, time) in sorted(sp.SHOW.items()):
        if sp.SHOW[name] == sp.NEVER:
            lo, hi = 99, 0
        flag = ids.flag(root, 'FLAG_HIDE_' + name)
        if not flag:
            continue
        rows.append('    {0x%X, 0x%X, 0x%X, %d, %d, %d},  // %s\n' % (
            flag, done[need] if need else 0, done[unless] if unless else 0, lo, min(hi, 99),
            {None: 0, 'day': 1, 'night': 2}[time], name))
    open('src/data/sinnoh_story_people.h', 'w').write(
        '// Generated by tools/sinnoh/legends.py from tools/sinnoh/story_people.py. Do not edit.\n'
        '// {hide flag, story battle that must be beaten, story battle that must not, from badges, before badges (9:\n'
        '// SINNOH cleared), 0 any time / 1 day / 2 night}\n'
        'struct SinnohStoryPerson { u16 flag, need, unless; u8 from, before, time; };\n'
        'static const struct SinnohStoryPerson sSinnohStoryPeople[] = {\n' + ''.join(rows) + '};\n')
    return len(rows)


def story_label(k):
    return 'Sinnoh_EventScript_Story%d' % k


def label(species):
    return 'Sinnoh_EventScript_Legend%s' % species.capitalize()


def scripts():
    """{(map, object number): script label} for gen_maps.py"""
    out = {(m, n): label(s) for m, n, s, _, _ in LEGENDS if not isinstance(n, tuple)}
    out.update({(m, n): story_label(k) for k, (m, n, _, _, _) in enumerate(STORY)})
    return out


def added(name):
    """The objects this game adds to place NAME: [(x, z in Platinum's tiles, species, script label, hide flag)]"""
    out = []
    for k, (m, n, s, _, _) in enumerate(LEGENDS):
        if m == name and isinstance(n, tuple):
            out.append((n[1], n[2], s, label(s), EXTRA_FLAGS + k))
    return out


def object_flag(root, place_events, n):
    if isinstance(n, tuple):
        return EXTRA_FLAGS + [k for k, e in enumerate(LEGENDS) if e[1] is n][0]
    hide = place_events['object_events'][n - 1].get('hidden_flag')
    return ids.flag(root, hide) if isinstance(hide, str) and hide.startswith('FLAG_') else None


def hook(m):
    """Map M's on-load hook: special Sinnoh_LegendsRefresh (above its generated words): a script of its own, or a
    line at the start of the one it already has"""
    import re
    path = 'data/maps/%s/scripts.inc' % m
    text = open(path).read()
    if 'Sinnoh_LegendsRefresh' in text:
        return
    head = '%s_MapScripts::\n' % m
    trans = re.search(r'map_script MAP_SCRIPT_ON_TRANSITION, (\w+)', text)
    if trans:
        lab = trans.group(1) + ':'
        i = text.index('\n' + lab) + 1
        j = text.index('\n', i) + 1
        text = text[:j] + '\tspecial Sinnoh_LegendsRefresh\n' + text[j:]
    else:
        i = text.index(head) + len(head)
        text = text[:i] + '\tmap_script MAP_SCRIPT_ON_TRANSITION, %s_OnTransition\n' % m + text[i:]
        k = text.index('\t.byte 0\n', i) + len('\t.byte 0\n')
        text = text[:k] + '\n%s_OnTransition:\n\tspecial Sinnoh_LegendsRefresh\n\tend\n' % m + text[k:]
    open(path, 'w').write(text)


def main():
    root = sys.argv[1]
    from gen_maps import places
    from source import open_source
    from places import events
    by_name = {p['name']: p for p in places(open_source(root), root)}
    head = '// Generated by tools/sinnoh/legends.py. Do not edit.\n'
    rows, inc = [], [head.replace('//', '@')]
    for k, (m, n, species, level, badges) in enumerate(LEGENDS):
        p = by_name[m]
        obj = object_flag(root, events(root, p['place']), n)
        assert obj, '%s: no hide flag for its object %d' % (m, n)
        done = OWN_DONE.get(species, DONE_FLAGS + k)
        assert done < 0x3D0C or species in OWN_DONE, species
        map_const = json.load(open('data/maps/%s/map.json' % m))['id']
        rows.append('    {MAP_GROUP(%s), MAP_NUM(%s), SPECIES_%s, 0x%X, 0x%X, %d},\n'
                    % (map_const, map_const, species, obj, done, badges))
        lab = label(species)
        inc.append('%s::\n\tlock\n\tfaceplayer\n\twaitse\n\tplaymoncry SPECIES_%s, CRY_MODE_ENCOUNTER\n\tdelay 40\n'
                   '\twaitmoncry\n\tsetwildbattle SPECIES_%s, %d\n\tsetflag FLAG_SYS_CTRL_OBJ_DELETE\n'
                   '\tspecial BattleSetup_StartLegendaryBattle\n\twaitstate\n\tclearflag FLAG_SYS_CTRL_OBJ_DELETE\n'
                   '\tspecialvar VAR_RESULT, GetBattleOutcome\n'
                   '\tgoto_if_eq VAR_RESULT, B_OUTCOME_WON, %s_Gone\n'
                   '\tgoto_if_eq VAR_RESULT, B_OUTCOME_RAN, %s_Ran\n'
                   '\tgoto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, %s_Ran\n'
                   '\tsetflag 0x%X\n\trelease\n\tend\n\n'
                   '%s_Gone:\n\tsetflag 0x%X\n\tgoto Common_EventScript_RemoveStaticPokemon\n\tend\n\n'
                   '%s_Ran:\n\tsetvar VAR_0x8004, SPECIES_%s\n\tgoto Common_EventScript_LegendaryFlewAway\n\tend\n\n'
                   % (lab, species, species, level, lab, lab, lab, done, lab, done, lab, species))
        if badges == FRIDAYS:   # (the day it was met: s3d_legends.c clears its flag on a later day)
            inc[-1] = inc[-1].replace('\tsetflag 0x%X\n' % done, '\tsetflag 0x%X\n\tspecial Sinnoh_FridayMet\n' % done)
        # the map's on-load hook: shown or hidden (above the generated words, kept by gen_words.py)
        hook(m)
    for k, (m, n, trainers, badges, words) in enumerate(STORY):
        p = by_name[m]
        obj = object_flag(root, events(root, p['place']), n)
        assert obj, '%s: no hide flag for its object %d' % (m, n)
        done = STORY_FLAGS + k
        map_const = json.load(open('data/maps/%s/map.json' % m))['id']
        rows.append('    {MAP_GROUP(%s), MAP_NUM(%s), SPECIES_NONE, 0x%X, 0x%X, %d},\n'
                    % (map_const, map_const, obj, done, badges))
        lab = story_label(k)
        intro, defeat, after = words
        out = '%s::\n\tlock\n\tfaceplayer\n' % lab
        for i, t in enumerate(trainers):
            last = i == len(trainers) - 1
            nxt = '%s_Won' % lab if last else '%s_Next%d' % (lab, i + 1)
            if t.startswith('RIVAL_'):
                # (BARRY's team: Platinum's three versions, by the player's Trainer ID)
                out += '\tspecialvar VAR_RESULT, Sinnoh_RivalVersion\n'
                for v, starter in enumerate(('CHIMCHAR', 'PIPLUP', 'TURTWIG')):
                    out += '\tgoto_if_eq VAR_RESULT, %d, %s_V%d_%d\n' % (v, lab, i, v)
                for v, starter in enumerate(('CHIMCHAR', 'PIPLUP', 'TURTWIG')):
                    out += '%s_V%d_%d:\n\ttrainerbattle_single TRAINER_SINNOH_%s_%s, %s_Intro, %s_Defeat, %s\n\tend\n' \
                           % (lab, i, v, t, starter, lab, lab, nxt)
            else:
                out += '\ttrainerbattle_single TRAINER_SINNOH_%s, %s_Intro, %s_Defeat, %s\n\tend\n' % (t, lab, lab, nxt)
            if not last:
                out += '%s:\n' % nxt
        out += ('%s_Won::\n\tmsgbox %s_After, MSGBOX_DEFAULT\n\tclosemessage\n\tsetflag 0x%X\n'
                '\tfadescreen FADE_TO_BLACK\n\tremoveobject VAR_LAST_TALKED\n\tfadescreen FADE_FROM_BLACK\n\trelease\n\tend\n\n'
                % (lab, lab, done))
        for suffix, text in (('Intro', intro), ('Defeat', defeat), ('After', after)):
            out += '%s_%s:\n\t.string "%s$"\n\n' % (lab, suffix, text.replace('\n', '\\n"\n\t.string "'))
        inc.append(out)
        hook(m)
    open('src/data/sinnoh_legends.h', 'w').write(
        head + "// SINNOH's legendaries on the map (src/sinnoh/s3d_legends.c): where, which, the object's flag, its "
        "\"done\" flag and the\n// SINNOH badges it waits for (0xFF: the LEAGUE, 0xFE: Fridays from the 1st)\n"
        'static const struct { u8 group, num; u16 species; u16 objFlag, doneFlag; u8 badges; } sSinnohLegends[] = {\n'
        + ''.join(rows) + '};\n')
    open('data/scripts/sinnoh_legends.inc', 'w').write(''.join(inc))
    n = story_people_header(root)
    print('%d legendaries, %d story battles, %d story people\'s flags' % (len(LEGENDS), len(STORY), n))


if __name__ == '__main__':
    main()
