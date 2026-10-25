@ Moemon Kai: battle scripts of the Gen 4 move effects (BattleScript_Effect<Name>) and the Gen 4
@ helpers they use. The "gen4 MODE, ptr" command does the work (src/battle_gen4.c MP_Gen4_Command);
@ the rest is the stock engine's commands, laid out like the Gen 3 moves these effects resemble.

@ ------------------------------------------------------------------------------------------------ plain hits
@ The power or type comes from src/battle_gen4.c (MP_Gen4_MovePower / MP_Gen4_MoveType).
BattleScript_EffectGyroBall::
BattleScript_EffectBrine::
BattleScript_EffectPayback::
BattleScript_EffectAssurance::
BattleScript_EffectTrumpCard::
BattleScript_EffectWringOut::
BattleScript_EffectPunishment::
BattleScript_EffectJudgment::
	goto BattleScript_EffectHit

BattleScript_EffectHammerArm::
	setmoveeffect MOVE_EFFECT_SPD_MINUS_1 | MOVE_EFFECT_AFFECTS_USER | MOVE_EFFECT_CERTAIN
	goto BattleScript_EffectHit

BattleScript_EffectCloseCombat::
	setmoveeffect MOVE_EFFECT_DEF_SPDEF_DOWN | MOVE_EFFECT_AFFECTS_USER | MOVE_EFFECT_CERTAIN
	goto BattleScript_EffectHit

BattleScript_EffectHeadSmash::
	setmoveeffect MOVE_EFFECT_RECOIL_50 | MOVE_EFFECT_AFFECTS_USER | MOVE_EFFECT_CERTAIN
	goto BattleScript_EffectHit

BattleScript_EffectSpAttackUpHit::
	setmoveeffect MOVE_EFFECT_SP_ATK_PLUS_1 | MOVE_EFFECT_AFFECTS_USER
	goto BattleScript_EffectHit

BattleScript_EffectSpDefenseDown2Hit::
	setmoveeffect MOVE_EFFECT_SP_DEF_MINUS_2
	goto BattleScript_EffectHit

BattleScript_G4DefSpDefDown::
	setbyte sSTAT_ANIM_PLAYED, FALSE
	playstatchangeanimation BS_ATTACKER, BIT_DEF | BIT_SPDEF, STAT_CHANGE_CANT_PREVENT | STAT_CHANGE_NEGATIVE | STAT_CHANGE_MULTIPLE_STATS
	playstatchangeanimation BS_ATTACKER, BIT_DEF, STAT_CHANGE_CANT_PREVENT | STAT_CHANGE_NEGATIVE
	setstatchanger STAT_DEF, 1, TRUE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | MOVE_EFFECT_CERTAIN | STAT_CHANGE_ALLOW_PTR, BattleScript_G4DefSpDefDown_TrySpDef
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_STAT_WONT_DECREASE, BattleScript_G4DefSpDefDown_TrySpDef
	printfromtable gStatDownStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_G4DefSpDefDown_TrySpDef::
	playstatchangeanimation BS_ATTACKER, BIT_SPDEF, STAT_CHANGE_CANT_PREVENT | STAT_CHANGE_NEGATIVE
	setstatchanger STAT_SPDEF, 1, TRUE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | MOVE_EFFECT_CERTAIN | STAT_CHANGE_ALLOW_PTR, BattleScript_G4DefSpDefDown_End
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_STAT_WONT_DECREASE, BattleScript_G4DefSpDefDown_End
	printfromtable gStatDownStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_G4DefSpDefDown_End::
	return

@ a hit up to the damage, its messages and the chance effect; the caller adds what comes after
BattleScript_G4HitNoEnd::
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
	return

@ THUNDER / ICE / FIRE FANG: 10% flinch (the move's chance) and 10% status
BattleScript_EffectThunderFang::
BattleScript_EffectIceFang::
BattleScript_EffectFireFang::
	setmoveeffect MOVE_EFFECT_FLINCH
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	call BattleScript_G4HitNoEnd
	seteffectwithchance
	gen4 G4_FANG_STATUS, BattleScript_G4HitEnd
	seteffectsecondary
BattleScript_G4HitEnd::
	tryfaintmon BS_TARGET
	goto BattleScript_MoveEnd

@ FLARE BLITZ: thaws its user, 10% burn, 1/3 recoil
BattleScript_EffectFlareBlitz::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	call BattleScript_G4HitNoEnd
	gen4 G4_FANG_STATUS, BattleScript_G4FlareBlitzRecoil
	seteffectsecondary
BattleScript_G4FlareBlitzRecoil::
	setmoveeffect MOVE_EFFECT_RECOIL_33 | MOVE_EFFECT_AFFECTS_USER | MOVE_EFFECT_CERTAIN
	seteffectwithchance
	goto BattleScript_G4HitEnd

BattleScript_EffectWakeUpSlap::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	call BattleScript_G4HitNoEnd
	seteffectwithchance
	gen4 G4_WAKE_TARGET, BattleScript_G4TargetWoke
	goto BattleScript_G4HitEnd
BattleScript_G4TargetWoke::
	printstring STRINGID_G4_TARGET_WOKE
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_TARGET
	goto BattleScript_G4HitEnd

BattleScript_EffectBugBite::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	call BattleScript_G4HitNoEnd
	seteffectwithchance
	tryfaintmon BS_TARGET
	gen4 G4_BUG_BITE, BattleScript_G4BugBiteAte
	goto BattleScript_MoveEnd
BattleScript_G4BugBiteAte::
	printstring STRINGID_G4_BUG_BITE
	waitmessage B_WAIT_TIME_LONG
	jumpifword CMP_EQUAL, gBattleMoveDamage, 0, BattleScript_G4BugBiteStatus
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
BattleScript_G4BugBiteStatus::
	updatestatusicon BS_ATTACKER
	goto BattleScript_MoveEnd

@ ------------------------------------------------------------------------------------------- hits that can fail
BattleScript_EffectFeint::
	attackcanceler
	gen4 G4_FEINT, BattleScript_FailedFromAtkString
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	call BattleScript_G4HitNoEnd
	printstring STRINGID_G4_FEINT
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_G4HitEnd

BattleScript_EffectSuckerPunch::
	attackcanceler
	gen4 G4_SUCKER_PUNCH, BattleScript_FailedFromAtkString
	goto BattleScript_HitFromAccCheck

BattleScript_EffectLastResort::
	attackcanceler
	gen4 G4_LAST_RESORT, BattleScript_FailedFromAtkString
	goto BattleScript_HitFromAccCheck

BattleScript_EffectNaturalGift::
	attackcanceler
	gen4 G4_NATURAL_GIFT, BattleScript_FailedFromAtkString
	gen4 G4_CONSUME_ITEM
	goto BattleScript_HitFromAccCheck

BattleScript_EffectFling::
	attackcanceler
	gen4 G4_FLING, BattleScript_FailedFromAtkString
	attackstring
	ppreduce
	printstring STRINGID_G4_FLING
	waitmessage B_WAIT_TIME_LONG
	gen4 G4_CONSUME_ITEM
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	goto BattleScript_HitFromCritCalc

BattleScript_EffectMetalBurst::
	attackcanceler
	gen4 G4_METAL_BURST, BattleScript_FailedFromAtkString
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	typecalc2
	adjustsetdamage
	goto BattleScript_HitFromAtkAnimation

@ U-TURN: hits, then the user switches out (if it can)
BattleScript_EffectUTurn::
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	call BattleScript_G4HitNoEnd
	seteffectwithchance
	tryfaintmon BS_TARGET
	jumpifhasnohp BS_ATTACKER, BattleScript_MoveEnd
	jumpifbattletype BATTLE_TYPE_ARENA, BattleScript_MoveEnd
	jumpifcantswitch SWITCH_IGNORE_ESCAPE_PREVENTION | BS_ATTACKER, BattleScript_MoveEnd
	openpartyscreen BS_ATTACKER, BattleScript_MoveEnd
	switchoutabilities BS_ATTACKER
	waitstate
	switchhandleorder BS_ATTACKER, 2
	returntoball BS_ATTACKER
	getswitchedmondata BS_ATTACKER
	switchindataupdate BS_ATTACKER
	hpthresholds BS_ATTACKER
	printstring STRINGID_SWITCHINMON
	switchinanim BS_ATTACKER, TRUE
	waitstate
	switchineffects BS_ATTACKER
	goto BattleScript_MoveEnd

@ SHADOW FORCE: vanishes, then strikes through PROTECT
BattleScript_EffectShadowForce::
	jumpifstatus2 BS_ATTACKER, STATUS2_MULTIPLETURNS, BattleScript_G4ShadowForceStrike
	jumpifword CMP_COMMON_BITS, gHitMarker, HITMARKER_NO_ATTACKSTRING, BattleScript_G4ShadowForceStrike
	setbyte sTWOTURN_STRINGID, B_MSG_TURN1_SHADOW_FORCE
	call BattleScriptFirstChargingTurn
	setsemiinvulnerablebit
	goto BattleScript_MoveEnd
BattleScript_G4ShadowForceStrike::
	attackcanceler
	setmoveeffect MOVE_EFFECT_CHARGING
	setbyte sB_ANIM_TURN, 1
	clearstatusfromeffect BS_ATTACKER
	orword gHitMarker, HITMARKER_NO_PPDEDUCT
	gen4 G4_LIFT_PROTECT
	goto BattleScript_SemiInvulnerableTryHit

@ ---------------------------------------------------------------------------------------------- status moves
BattleScript_EffectRoost::
	attackcanceler
	attackstring
	ppreduce
	tryhealhalfhealth BattleScript_AlreadyAtFullHp, BS_ATTACKER
	gen4 G4_ROOST
	attackanimation
	waitanimation
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printstring STRINGID_PKMNREGAINEDHEALTH
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

@ a status move that only sets something up: gen4 MODE (jumps on failure), the animation, one message
.macro g4simple mode:req, string:req
	attackcanceler
	attackstring
	ppreduce
	gen4 \mode, BattleScript_ButItFailed
	attackanimation
	waitanimation
	printstring \string
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
.endm

@ the same against a target, after an accuracy check
.macro g4target mode:req, string:req
	attackcanceler
	attackstring
	ppreduce
	accuracycheck BattleScript_ButItFailed, ACC_CURR_MOVE
	gen4 \mode, BattleScript_ButItFailed
	attackanimation
	waitanimation
	printstring \string
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
.endm

BattleScript_EffectGravity::
	g4simple G4_GRAVITY, STRINGID_G4_GRAVITY_START

BattleScript_EffectTailwind::
	g4simple G4_TAILWIND, STRINGID_G4_TAILWIND

BattleScript_EffectLuckyChant::
	g4simple G4_LUCKY_CHANT, STRINGID_G4_LUCKY_CHANT

BattleScript_EffectPowerTrick::
	g4simple G4_POWER_TRICK, STRINGID_G4_POWER_TRICK

BattleScript_EffectAquaRing::
	g4simple G4_AQUA_RING, STRINGID_G4_AQUA_RING

BattleScript_EffectMagnetRise::
	g4simple G4_MAGNET_RISE, STRINGID_G4_MAGNET_RISE

BattleScript_EffectPowerSwap::
	g4simple G4_POWER_SWAP, STRINGID_G4_POWER_SWAP

BattleScript_EffectGuardSwap::
	g4simple G4_GUARD_SWAP, STRINGID_G4_GUARD_SWAP

BattleScript_EffectHeartSwap::
	g4simple G4_HEART_SWAP, STRINGID_G4_HEART_SWAP

BattleScript_EffectMiracleEye::
	g4target G4_MIRACLE_EYE, STRINGID_G4_MIRACLE_EYE

BattleScript_EffectEmbargo::
	g4target G4_EMBARGO, STRINGID_G4_EMBARGO

BattleScript_EffectHealBlock::
	g4target G4_HEAL_BLOCK, STRINGID_G4_HEAL_BLOCK

BattleScript_EffectGastroAcid::
	g4target G4_GASTRO_ACID, STRINGID_G4_GASTRO_ACID

BattleScript_EffectWorrySeed::
	g4target G4_WORRY_SEED, STRINGID_G4_WORRY_SEED

BattleScript_EffectToxicSpikes::
	attackcanceler
	gen4 G4_TOXIC_SPIKES, BattleScript_FailedFromAtkString
	attackstring
	ppreduce
	attackanimation
	waitanimation
	printstring STRINGID_G4_TOXIC_SPIKES
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_EffectStealthRock::
	attackcanceler
	gen4 G4_STEALTH_ROCK, BattleScript_FailedFromAtkString
	attackstring
	ppreduce
	attackanimation
	waitanimation
	printstring STRINGID_G4_STEALTH_ROCK
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_EffectTrickRoom::
	attackcanceler
	attackstring
	ppreduce
	attackanimation
	waitanimation
	gen4 G4_TRICK_ROOM, BattleScript_G4TrickRoomOff
	printstring STRINGID_G4_TRICK_ROOM
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
BattleScript_G4TrickRoomOff::
	printstring STRINGID_G4_TRICK_ROOM_END
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_EffectAcupressure::
	attackcanceler
	gen4 G4_ACUPRESSURE, BattleScript_FailedFromAtkString
	goto BattleScript_EffectStatUpAfterAtkCanceler

BattleScript_EffectCaptivate::
	setstatchanger STAT_SPATK, 2, TRUE
	attackcanceler
	jumpifstatus2 BS_TARGET, STATUS2_SUBSTITUTE, BattleScript_FailedFromAtkString
	gen4 G4_CAPTIVATE, BattleScript_FailedFromAtkString
	goto BattleScript_EffectStatDownAfterAtkCanceler

BattleScript_EffectDefog::
	setstatchanger STAT_EVASION, 1, TRUE
	attackcanceler
	accuracycheck BattleScript_PrintMoveMissed, ACC_CURR_MOVE
	attackstring
	ppreduce
	attackanimation
	waitanimation
	jumpifstatus2 BS_TARGET, STATUS2_SUBSTITUTE, BattleScript_G4DefogClear
	statbuffchange STAT_CHANGE_ALLOW_PTR, BattleScript_G4DefogClear
	jumpifbyte CMP_GREATER_THAN, cMULTISTRING_CHOOSER, B_MSG_DEFENDER_STAT_FELL, BattleScript_G4DefogClear
	setgraphicalstatchangevalues
	playanimation BS_TARGET, B_ANIM_STATS_CHANGE, sB_ANIM_ARG1
	printfromtable gStatDownStringIds
	waitmessage B_WAIT_TIME_LONG
BattleScript_G4DefogClear::
	gen4 G4_DEFOG, BattleScript_MoveEnd
	printstring STRINGID_G4_DEFOG
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

BattleScript_EffectPsychoShift::
	attackcanceler
	attackstring
	ppreduce
	accuracycheck BattleScript_ButItFailed, ACC_CURR_MOVE
	gen4 G4_PSYCHO_SHIFT, BattleScript_ButItFailed
	attackanimation
	waitanimation
	seteffectprimary
	gen4 G4_PSYCHO_SHIFT_CURE
	updatestatusicon BS_ATTACKER
	goto BattleScript_MoveEnd

@ HEALING WISH / LUNAR DANCE: the user faints; the Pokémon that replaces it is healed on arrival
BattleScript_EffectHealingWish::
	attackcanceler
	attackstring
	ppreduce
	jumpifcantswitch SWITCH_IGNORE_ESCAPE_PREVENTION | BS_ATTACKER, BattleScript_ButItFailed
	attackanimation
	waitanimation
	gen4 G4_HEALING_WISH
	goto BattleScript_G4WishFaint
BattleScript_EffectLunarDance::
	attackcanceler
	attackstring
	ppreduce
	jumpifcantswitch SWITCH_IGNORE_ESCAPE_PREVENTION | BS_ATTACKER, BattleScript_ButItFailed
	attackanimation
	waitanimation
	gen4 G4_LUNAR_DANCE
BattleScript_G4WishFaint::
	setatkhptozero
	tryfaintmon BS_ATTACKER
	goto BattleScript_MoveEnd

@ ME FIRST / COPYCAT run another move (gen4 moves the script there)
BattleScript_EffectMeFirst::
	attackcanceler
	attackstring
	pause B_WAIT_TIME_SHORT
	setbyte sB_ANIM_TURN, 0
	setbyte sB_ANIM_TARGETS_HIT, 0
	gen4 G4_ME_FIRST, BattleScript_FailedFromPPReduce
	goto BattleScript_FailedFromPPReduce

BattleScript_EffectCopycat::
	attackcanceler
	attackstring
	pause B_WAIT_TIME_SHORT
	attackanimation
	waitanimation
	setbyte sB_ANIM_TURN, 0
	setbyte sB_ANIM_TARGETS_HIT, 0
	gen4 G4_COPYCAT, BattleScript_FailedFromPPReduce
	goto BattleScript_FailedFromPPReduce

@ ------------------------------------------------------------------------------------- arriving (switchineffects)
BattleScript_G4HealingWishArrive::
	printstring STRINGID_G4_HEALING_WISH
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_G4WishArriveHeal
BattleScript_G4LunarDanceArrive::
	printstring STRINGID_G4_LUNAR_DANCE
	waitmessage B_WAIT_TIME_LONG
BattleScript_G4WishArriveHeal::
	playanimation BS_SCRIPTING, B_ANIM_WISH_HEAL
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_HP_UPDATE
	healthbarupdate BS_SCRIPTING
	datahpupdate BS_SCRIPTING
BattleScript_G4WishArriveSync::
	waitstate
	gen4 G4_ARRIVE_SYNC, BattleScript_G4WishArriveSync
	waitstate
	updatestatusicon BS_SCRIPTING
	waitstate
	return

BattleScript_G4RockOnAttacker::
	call BattleScript_G4RockHurt
	tryfaintmon BS_ATTACKER
	tryfaintmon_spikes BS_ATTACKER, BattleScript_SpikesOnAttackerFainted
	return
BattleScript_G4RockOnTarget::
	call BattleScript_G4RockHurt
	tryfaintmon BS_TARGET
	tryfaintmon_spikes BS_TARGET, BattleScript_SpikesOnTargetFainted
	return
BattleScript_G4RockOnFainted::
	call BattleScript_G4RockHurt
	tryfaintmon BS_FAINTED
	tryfaintmon_spikes BS_FAINTED, BattleScript_SpikesOnFaintedBattlerFainted
	return
BattleScript_G4RockHurt::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_HP_UPDATE
	healthbarupdate BS_SCRIPTING
	datahpupdate BS_SCRIPTING
	printstring STRINGID_G4_STEALTH_ROCK_HIT
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_G4ToxicSpikesGone::
	printstring STRINGID_G4_TOXIC_SPIKES_GONE
	waitmessage B_WAIT_TIME_LONG
	return
BattleScript_G4ToxicSpikesPoisoned::
	statusanimation BS_EFFECT_BATTLER
	printstring STRINGID_PKMNWASPOISONED
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_UpdateEffectStatusIconRet
BattleScript_G4ToxicSpikesBadlyPoisoned::
	statusanimation BS_EFFECT_BATTLER
	printstring STRINGID_PKMNBADLYPOISONED
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_UpdateEffectStatusIconRet

@ ---------------------------------------------------------------------------------------- end of turn
@ the message comes from gGen4TurnStringIds (src/battle_gen4.c sets cMULTISTRING_CHOOSER)
BattleScript_G4TurnHeal::
	printfromtable gGen4TurnStringIds
	waitmessage B_WAIT_TIME_LONG
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	end2

BattleScript_G4TurnHurt::
	printfromtable gGen4TurnStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_DoTurnDmg

BattleScript_G4TurnMsg::
	printfromtable gGen4TurnStringIds
	waitmessage B_WAIT_TIME_LONG
	end2

BattleScript_G4TurnStatusAnim::
	statusanimation BS_ATTACKER
BattleScript_G4TurnStatus::
	printfromtable gGen4TurnStringIds
	waitmessage B_WAIT_TIME_LONG
	updatestatusicon BS_ATTACKER
	waitstate
	end2

@ ------------------------------------------------------------------------------- arrival abilities (end3)
BattleScript_G4AbilityMsg::
	pause B_WAIT_TIME_SHORT
	printfromtable gGen4TurnStringIds
	waitmessage B_WAIT_TIME_LONG
	end3

BattleScript_G4AbilityStat::
	pause B_WAIT_TIME_SHORT
	playanimation BS_SCRIPTING, B_ANIM_STATS_CHANGE, sB_ANIM_ARG1
	printfromtable gGen4TurnStringIds
	waitmessage B_WAIT_TIME_LONG
	end3

BattleScript_G4SnowWarning::
	pause B_WAIT_TIME_SHORT
	printfromtable gGen4TurnStringIds
	waitstate
	playanimation BS_BATTLER_0, B_ANIM_HAIL_CONTINUES
	call BattleScript_WeatherFormChanges
	end3

@ GRAVITY / HEAL BLOCK stop the move before it starts
BattleScript_G4MoveBanned::
	printfromtable gGen4TurnStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

@ MOTOR DRIVE takes the ELECTRIC move and raises its SPEED
BattleScript_G4MotorDrive_PPLoss::
	ppreduce
BattleScript_G4MotorDrive::
	attackstring
	pause B_WAIT_TIME_SHORT
	orbyte gMoveResultFlags, MOVE_RESULT_DOESNT_AFFECT_FOE
	setstatchanger STAT_SPEED, 1, FALSE
	statbuffchange STAT_CHANGE_ALLOW_PTR | MOVE_EFFECT_CERTAIN, BattleScript_G4MotorDriveUseless
	jumpifbyte CMP_EQUAL, cMULTISTRING_CHOOSER, B_MSG_STAT_WONT_INCREASE, BattleScript_G4MotorDriveUseless
	setgraphicalstatchangevalues
	playanimation BS_TARGET, B_ANIM_STATS_CHANGE, sB_ANIM_ARG1
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
BattleScript_G4MotorDriveUseless::
	printstring STRINGID_PKMNSXMADEYUSELESS
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd

@ ------------------------------------------------------------------------------------- move end (return)
BattleScript_G4RemoveTargetItem::
	removeitem BS_TARGET
	return

BattleScript_G4AngerPoint::
	playanimation BS_TARGET, B_ANIM_STATS_CHANGE, sB_ANIM_ARG1
	printstring STRINGID_G4_ANGER_POINT
	waitmessage B_WAIT_TIME_LONG
	return

BattleScript_G4AttackerHurt::
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_HP_UPDATE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printfromtable gGen4TurnStringIds
	waitmessage B_WAIT_TIME_LONG
	tryfaintmon BS_ATTACKER
	return

@ POWER HERB: the charging turn is skipped (the move's script now takes its second-turn path)
BattleScript_G4PowerHerb::
	printstring STRINGID_G4_POWER_HERB
	waitmessage B_WAIT_TIME_LONG
	gen4 G4_RERUN_MOVE

@ ---------------------------------------------------------------------------------------- Gen 4 BERRIES
@ a type-resist BERRY / MICLE BERRY: its message, then it's eaten (gBattleScripting.battler = the target for MICLE)
BattleScript_G4TargetBerryMsg::
	playanimation BS_TARGET, B_ANIM_HELD_ITEM_EFFECT
	printfromtable gGen4TurnStringIds
	waitmessage B_WAIT_TIME_LONG
	removeitem BS_TARGET
	return

@ JABOCA / ROWAP BERRY
BattleScript_G4BerryHurtsAttacker::
	playanimation BS_TARGET, B_ANIM_HELD_ITEM_EFFECT
	removeitem BS_TARGET
	orword gHitMarker, HITMARKER_IGNORE_SUBSTITUTE | HITMARKER_PASSIVE_HP_UPDATE
	healthbarupdate BS_ATTACKER
	datahpupdate BS_ATTACKER
	printfromtable gGen4TurnStringIds
	waitmessage B_WAIT_TIME_LONG
	tryfaintmon BS_ATTACKER
	return

@ CUSTAP BERRY: before the move (HandleAction_UseMove pushed the move's script)
BattleScript_G4Custap::
	playanimation BS_ATTACKER, B_ANIM_HELD_ITEM_EFFECT
	printfromtable gGen4TurnStringIds
	waitmessage B_WAIT_TIME_LONG
	removeitem BS_ATTACKER
	return

@ STEADFAST: flinching raises its SPEED
BattleScript_G4Steadfast::
	printstring STRINGID_PKMNFLINCHED
	waitmessage B_WAIT_TIME_LONG
	setstatchanger STAT_SPEED, 1, FALSE
	statbuffchange MOVE_EFFECT_AFFECTS_USER | STAT_CHANGE_ALLOW_PTR, BattleScript_MoveEnd
	setgraphicalstatchangevalues
	playanimation BS_ATTACKER, B_ANIM_STATS_CHANGE, sB_ANIM_ARG1
	printfromtable gStatUpStringIds
	waitmessage B_WAIT_TIME_LONG
	goto BattleScript_MoveEnd
