#!/usr/bin/env python3
"""data/battle_scripts_gen58.inc: the Gen 5-9 effects' battle scripts.

    python3 tools/moemon_gen58/gen_effect_scripts.py

Each effect's label (BattleScript_Effect<Name>, from data/battle_effect_ptrs_gen58.inc) goes to one of the generic
Gen 5-9 scripts written below (they act on src/battle_gen59.c's per-move data and per-effect code: G59_* modes of
the gen4 command), or to a Kai script that already does the same (RECHARGE, RAMPAGE...). Effects not listed in
SCRIPT are plain hits (damaging) or status moves through G59StatusMove.
"""
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))

# effect (CamelCase, as in the label) -> script
SCRIPT = {
    # Kai's own scripts
    'PrismaticLaser': 'BattleScript_EffectRecharge', 'Eternabeam': 'BattleScript_EffectRecharge',
    'MeteorAssault': 'BattleScript_EffectRecharge', 'RagingFury': 'BattleScript_EffectRampage',
    'AxeKick': 'BattleScript_EffectRecoilIfMiss', 'SupercellSlam': 'BattleScript_EffectRecoilIfMiss',
    'MortalSpin': 'BattleScript_EffectRapidSpin', 'PhantomForce': 'BattleScript_EffectShadowForce',
    'ReflectDamage': 'BattleScript_EffectMetalBurst', 'SpeciesPowerOverride': 'BattleScript_EffectMultiHit',
    'PopulationBomb': 'BattleScript_EffectMultiHit', 'ScaleShot': 'BattleScript_EffectMultiHit',
    'SurgingStrikes': 'BattleScript_EffectMultiHit',
    # generic Gen 5-9 ones
    'MaxHp50Recoil': 'BattleScript_G59HitHalfRecoil', 'Chloroblast': 'BattleScript_G59HitHalfRecoil',
    'HitSwitchTarget': 'BattleScript_G59HitDrag', 'FinalGambit': 'BattleScript_G59FinalGambit',
    'HealPulse': 'BattleScript_G59HealTarget', 'ShoreUp': 'BattleScript_G59HealUser',
    'StrengthSap': 'BattleScript_G59HealUser', 'Purify': 'BattleScript_G59HealUser',
    'LifeDew': 'BattleScript_G59HealUser', 'JungleHealing': 'BattleScript_G59HealUser',
    'ClangorousSoul': 'BattleScript_G59PayHpStats', 'StatChangeHalfHp': 'BattleScript_G59PayHpStats',
    'MeteorBeam': 'BattleScript_G59TwoTurns', 'ElectroShot': 'BattleScript_G59TwoTurns',
    'Geomancy': 'BattleScript_G59TwoTurns', 'FreezeShock': 'BattleScript_G59TwoTurns',
    'IceBurn': 'BattleScript_G59TwoTurns',
}

GENERIC = r'''
@ ------------------------------------------------------------------------------------------------------------------
@ Moemon Kai: the generic Gen 5-9 scripts (src/battle_gen59.c). Everything above is generated: edit the generator.

@ a status move: its own effect (G59_STATUS_MOVE: fails, or picks a message), then the stat changes in its data
BattleScript_G59StatusMove::
	attackcanceler
	attackstring
	ppreduce
	gen4 G59_STATUS_MOVE, BattleScript_ButItFailed
	attackanimation
	waitanimation
	call BattleScript_G59SayIfAny
	call BattleScript_G59AllStats
	goto BattleScript_MoveEnd

@ a hit: may fail first (G59_BEFORE_HIT), its secondary status, its own after-hit effect, then its stat changes
BattleScript_G59Hit::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	gen4 G59_BEFORE_HIT, BattleScript_ButItFailed
	call BattleScript_G59DoHit
	goto BattleScript_MoveEnd

BattleScript_G59DoHit::
	critcalc
	damagecalc
	typecalc
	adjustnormaldamage
	attackanimation
	waitanimation
	effectivenesssound
	hitanimation BS_TARGET
	waitstate
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	critmessage
	waitmessage B_WAIT_TIME_LONG
	resultmessage
	waitmessage B_WAIT_TIME_LONG
	gen4 G59_ROLL_STATUS
	seteffectwithchance
	tryfaintmon BS_TARGET
	jumpifmovehadnoeffect BattleScript_G59Return
	gen4 G59_AFTER_HIT, BattleScript_G59DoHitStats
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_G59DoHitStats::
	call BattleScript_G59AllStats
BattleScript_G59Return::
	return

@ MIND BLOWN, STEEL BEAM, CHLOROBLAST: the user loses half its max HP (even when it misses: not here)
BattleScript_G59HitHalfRecoil::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	call BattleScript_G59DoHit
	gen4 G59_HALF_RECOIL, BattleScript_MoveEnd
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_HP_UPDATE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	tryfaintmon BS_ATTACKER
	goto BattleScript_MoveEnd

@ CIRCLE THROW / DRAGON TAIL: the target is dragged out after the hit
BattleScript_G59HitDrag::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	call BattleScript_G59DoHit
	gen4 G59_CAN_DRAG, BattleScript_MoveEnd
	forcerandomswitch BattleScript_MoveEnd
	goto BattleScript_MoveEnd

@ FINAL GAMBIT: as much damage as the user's HP, and the user faints
BattleScript_G59FinalGambit::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	typecalc
	bicbyte gMoveResultFlags, MOVE_RESULT_SUPER_EFFECTIVE | MOVE_RESULT_NOT_VERY_EFFECTIVE
	gen4 G59_GAMBIT, BattleScript_ButItFailed
	adjustnormaldamage
	attackanimation
	waitanimation
	effectivenesssound
	hitanimation BS_TARGET
	waitstate
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	resultmessage
	waitmessage B_WAIT_TIME_LONG
	gen4 G59_GAMBIT_SELF
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_HP_UPDATE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	tryfaintmon BS_TARGET
	tryfaintmon BS_ATTACKER
	goto BattleScript_MoveEnd

@ two-turn moves: a charging turn (with METEOR BEAM's / ELECTRO SHOT's SP. ATK), then the hit (GEOMANCY: its stats)
BattleScript_G59TwoTurns::
	jumpifstatus2 BS_ATTACKER, STATUS2_MULTIPLETURNS, BattleScript_G59TwoTurnsSecond
	jumpifword CMP_COMMON_BITS, gHitMarker, HITMARKER_NO_ATTACKSTRING, BattleScript_G59TwoTurnsSecond
	gen4 G59_ONE_TURN_NOW, BattleScript_G59Hit
	setbyte sTWOTURN_STRINGID, B_MSG_TURN1_SKY_ATTACK
	call BattleScriptFirstChargingTurn
	gen4 G59_STATS_SELF
	call BattleScript_G59SelfLoop
	goto BattleScript_MoveEnd
BattleScript_G59TwoTurnsSecond::
	attackcanceler
	setmoveeffect MOVE_EFFECT_CHARGING
	setbyte sB_ANIM_TURN, 1
	clearstatusfromeffect BS_ATTACKER
	orword gHitMarker, HITMARKER_NO_PPDEDUCT
	gen4 G59_IS_STATUS, BattleScript_G59TwoTurnsStatus
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	call BattleScript_G59DoHit
	goto BattleScript_MoveEnd
BattleScript_G59TwoTurnsStatus::
	attackstring
	ppreduce
	attackanimation
	waitanimation
	call BattleScript_G59AllStats
	goto BattleScript_MoveEnd

@ healing moves: the target's HP (HEAL PULSE) or the user's (SHORE UP, STRENGTH SAP, PURIFY, LIFE DEW...)
BattleScript_G59HealTarget::
	attackcanceler
	attackstring
	ppreduce
	gen4 G59_STATUS_MOVE, BattleScript_ButItFailed
	attackanimation
	waitanimation
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_TARGET
	datahpupdate BS_TARGET
	call BattleScript_G59SayIfAny
	call BattleScript_G59AllStats
	goto BattleScript_MoveEnd

BattleScript_G59HealUser::
	attackcanceler
	attackstring
	ppreduce
	gen4 G59_STATUS_MOVE, BattleScript_ButItFailed
	attackanimation
	waitanimation
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	call BattleScript_G59SayIfAny
	call BattleScript_G59AllStats
	goto BattleScript_MoveEnd

@ CLANGOROUS SOUL, FILLET AWAY: HP for stats
BattleScript_G59PayHpStats::
	attackcanceler
	attackstring
	ppreduce
	gen4 G59_PAY_HP, BattleScript_ButItFailed
	attackanimation
	waitanimation
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_HP_UPDATE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	call BattleScript_G59AllStats
	goto BattleScript_MoveEnd

BattleScript_G59SayIfAny::
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, 0xFF, BattleScript_G59Return
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	return

@ the move's stat changes: the user's, then the target's (each with its chance on a hit)
BattleScript_G59AllStats::
	gen4 G59_STATS_SELF
	call BattleScript_G59SelfLoop
	gen4 G59_STATS_TARGET
	call BattleScript_G59TargetLoop
	return

BattleScript_G59SelfLoop::
	gen4 G59_STATS_NEXT, BattleScript_G59Return
	statbuffchange MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_ALLOW_PTR, BattleScript_G59SelfLoop
	jumpifbyte CMP_GREATER_THAN, cMULTISTRING_CHOOSER, B_MSG_DEFENDER_STAT_ROSE, BattleScript_G59SelfLoop
	setgraphicalstatchangevalues
	playanimation BS_ATTACKER, B_ANIM_STATS_CHANGE, sB_ANIM_ARG1
	gen4 G59_STAT_WAS_DOWN, BattleScript_G59SelfDown
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_G59SelfLoop
BattleScript_G59SelfDown::
	printfromtable gStatDownStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_G59SelfLoop

BattleScript_G59TargetLoop::
	gen4 G59_STATS_NEXT, BattleScript_G59Return
	statbuffchange STAT_CHANGE_ALLOW_PTR, BattleScript_G59TargetLoop
	jumpifbyte CMP_GREATER_THAN, cMULTISTRING_CHOOSER, B_MSG_DEFENDER_STAT_ROSE, BattleScript_G59TargetLoop
	setgraphicalstatchangevalues
	playanimation BS_TARGET, B_ANIM_STATS_CHANGE, sB_ANIM_ARG1
	gen4 G59_STAT_WAS_DOWN, BattleScript_G59TargetDown
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_G59TargetLoop
BattleScript_G59TargetDown::
	printfromtable gStatDownStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_G59TargetLoop

@ end of turn (MP_Gen59_FieldEndTurn)
BattleScript_G59EndTurnSay::
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	end2

BattleScript_G59EndTurnHeal::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_HP_UPDATE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	end2

BattleScript_G59EndTurnHurt::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_HP_UPDATE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	tryfaintmon BS_ATTACKER
	end2

@ STICKY WEB on arrival (called by the switch-in effects)
BattleScript_G59StickyWeb::
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	return

@ Gen 5-9 abilities (MP_Gen59_SwitchInAbility, MP_Gen59_MoveEnd)
BattleScript_G59AbilitySay::
	pause B_WAIT_TIME_SHORT
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	end3

BattleScript_G59AbilityStat::
	pause B_WAIT_TIME_SHORT
	playanimation BS_SCRIPTING, B_ANIM_STATS_CHANGE, sB_ANIM_ARG1
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	end3

BattleScript_G59AbilityStatMoveEnd::
	playanimation BS_SCRIPTING, B_ANIM_STATS_CHANGE, sB_ANIM_ARG1
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_G59ItemStatMoveEnd::
	playanimation BS_SCRIPTING, B_ANIM_HELD_ITEM_EFFECT
	waitanimation
	playanimation BS_SCRIPTING, B_ANIM_STATS_CHANGE, sB_ANIM_ARG1
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_G59RockyHelmet::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_HP_UPDATE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	tryfaintmon BS_ATTACKER
	return

BattleScript_G59AbilitySayMoveEnd::
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_G59StatusMoveEnd::
	statusanimation BS_SCRIPTING
	updatestatusicon BS_SCRIPTING
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_G59WeatherMoveEnd::
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	playanimation BS_ATTACKER, B_ANIM_SANDSTORM_CONTINUES
	return

BattleScript_G59HealMoveEnd::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_HP_UPDATE
	healthbarupdate BS_SCRIPTING
	datahpupdate BS_SCRIPTING
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_G59HealEnd3::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_HP_UPDATE
	healthbarupdate BS_SCRIPTING
	datahpupdate BS_SCRIPTING
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	end3

BattleScript_G59StatusEnd3::
	updatestatusicon BS_SCRIPTING
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	end3

BattleScript_G59AbsorbStat::
	attackstring
	pause B_WAIT_TIME_SHORT
	orbyte gMoveResultFlags, MOVE_RESULT_DOESNT_AFFECT_FOE
	statbuffchange STAT_CHANGE_ALLOW_PTR | MOVE_EFFECT_CERTAIN, BattleScript_G59AbsorbStatUseless
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_STAT_WONT_INCREASE, BattleScript_G59AbsorbStatUseless
	setgraphicalstatchangevalues
	playanimation BS_TARGET, B_ANIM_STATS_CHANGE, sB_ANIM_ARG1
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
BattleScript_G59AbsorbStatUseless::
	printstring STRINGID_PKMNSXMADEYUSELESS
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_G59AbsorbStat_PPLoss::
	ppreduce
	goto BattleScript_G59AbsorbStat

@ EMERGENCY EXIT / WIMP OUT: the target switches out at the move's end
BattleScript_G59EmergencyExit::
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	jumpifcantswitch SWITCH_IGNORE_ESCAPE_PREVENTION | BS_TARGET, BattleScript_G59EmergencyExitEnd
	openpartyscreen BS_TARGET, BattleScript_G59EmergencyExitEnd
	switchoutabilities BS_TARGET
	waitstate
	switchhandleorder BS_TARGET, 2
	returntoball BS_TARGET
	getswitchedmondata BS_TARGET
	switchindataupdate BS_TARGET
	hpthresholds BS_TARGET
	printstring STRINGID_SWITCHINMON
	switchinanim BS_TARGET, TRUE
	waitstate
	switchineffects BS_TARGET
BattleScript_G59EmergencyExitEnd::
	return

@ a wild Pokemon's EMERGENCY EXIT: it leaves (the battle ends)
BattleScript_G59Flee::
	printfromtable gGen59StringIds
	waitmessage B_WAIT_TIME_LONG
	returntoball BS_TARGET
	waitstate
	setoutcomeonteleport BS_TARGET
	finishaction
'''


def main():
    import json
    labels = re.findall(r'ptrvalue BattleScript_Effect(\w+)', open(os.path.join(REPO, 'data/battle_effect_ptrs_gen58.inc')).read())
    moves = json.load(open(os.path.join(HERE, 'moves.json')))
    src = open(os.path.join(REPO, 'src/data/battle_moves_gen58.h')).read()
    eff = dict(re.findall(r'\[(MOVE_\w+)\] =\s*\{\s*\.effect = (EFFECT_\w+)', src))
    status = set()
    for m in moves:
        e = eff.get(m['const'], '')
        if m['category'] == 'DAMAGE_CATEGORY_STATUS':
            status.add(''.join(w.capitalize() for w in e[len('EFFECT_'):].split('_')))
    out = ['@ Generated by tools/moemon_gen58/gen_effect_scripts.py. Do not edit.',
           '@ Moemon Kai: the Gen 5-9 move effects\' battle scripts (BattleScript_Effect<Name>).', '']
    for l in labels:
        target = SCRIPT.get(l) or ('BattleScript_G59StatusMove' if l in status else 'BattleScript_G59Hit')
        out.append('BattleScript_Effect%s::\n\tgoto %s\n' % (l, target))
    open(os.path.join(REPO, 'data/battle_scripts_gen58.inc'), 'w').write('\n'.join(out) + GENERIC)
    print('%d effect scripts' % len(labels))


if __name__ == '__main__':
    main()
