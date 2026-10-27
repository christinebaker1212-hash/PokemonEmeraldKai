#!/usr/bin/env python3
"""Explicit rule coverage; unknown functions are never guessed from card names."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[2]
x=json.loads((ROOT/'tools/tcg/catalog.json').read_text())
# Each mapping is a reviewed original effect function, not a text/keyword guess.
functions={
 'Paralysis50PercentEffect':('PARALYZE_COIN',0),'PoisonEffect':('POISON',10),'Poison50PercentEffect':('POISON_COIN',10),'SpitPoison_Poison50PercentEffect':('POISON_COIN',10),
 'Confusion50PercentEffect':('CONFUSE_COIN',0),'SleepEffect':('SLEEP',0),'SingEffect':('SLEEP_COIN',0),'SleepingGasEffect':('SLEEP_COIN',0),
 'ZubatSupersonicEffect':('CONFUSE_COIN',0),'ShellderSupersonicEffect':('CONFUSE_COIN',0),'NidorinaSupersonicEffect':('CONFUSE_COIN',0),'TentacruelSupersonicEffect':('CONFUSE_COIN',0),'LickitungSupersonicEffect':('CONFUSE_COIN',0),
 'Toxic_DoublePoisonEffect':('POISON',20),'BulbasaurLeechSeedEffect':('DRAIN',10),'ParasectLeechLifeEffect':('DRAIN',20),'ZubatLeechLifeEffect':('DRAIN',10),'GolbatLeechLifeEffect':('DRAIN',20),
 'KakunaStiffenEffect':('BARRIER_COIN',0),'MetapodStiffenEffect':('BARRIER_COIN',0),'SquirtleWithdrawEffect':('BARRIER_COIN',0),'WartortleWithdrawEffect':('BARRIER_COIN',0),'ScrunchEffect':('BARRIER_COIN',0),
 'FearowAgilityEffect':('BARRIER_COIN',2),'RaichuAgilityEffect':('BARRIER_COIN',2),'RapidashAgilityEffect':('BARRIER_COIN',2),'SeadraAgilityEffect':('BARRIER_COIN',2),
 'HideInShellEffect':('BARRIER_COIN',0),'ExeggcuteLeechSeedEffect':('DRAIN',10),
 'ArcanineQuickAttack_DamageBoostEffect':('BONUS_COIN',20),'EeveeQuickAttack_DamageBoostEffect':('BONUS_COIN',20),'ElectabuzzQuickAttack_DamageBoostEffect':('BONUS_COIN',20),'FlareonQuickAttack_DamageBoostEffect':('BONUS_COIN',20),'JolteonQuickAttack_DamageBoostEffect':('BONUS_COIN',20),'VaporeonQuickAttack_DamageBoostEffect':('BONUS_COIN',20),'TaurosStomp_DamageBoostEffect':('BONUS_COIN',10),'RapidashStomp_DamageBoostEffect':('BONUS_COIN',10),
 'CuboneRage_DamageBoostEffect':('RAGE',0),'DodrioRage_DamageBoostEffect':('RAGE',0),'FlareonRage_DamageBoostEffect':('RAGE',0),
 'MrMimeMeditate_DamageBoostEffect':('MEDITATE',0),'JynxMeditate_DamageBoostEffect':('MEDITATE',0),'PsywaveEffect':('PSYWAVE',0),'Psychic_DamageBoostEffect':('PSYCHIC',0),
 'KarateChop_DamageSubtractionEffect':('KARATE_CHOP',0),'SuperFang_HalfHPEffect':('SUPER_FANG',0),
 'FuryAttack_MultiplierEffect':('MULTI_COIN',2),'CometPunch_MultiplierEffect':('MULTI_COIN',4),'Twineedle_MultiplierEffect':('MULTI_COIN',2),'PinMissile_MultiplierEffect':('MULTI_COIN',4),'NidoranFFurySwipes_MultiplierEffect':('MULTI_COIN',3),'PrimeapeFurySwipes_MultiplierEffect':('MULTI_COIN',3),'PsyduckFurySwipes_MultiplierEffect':('MULTI_COIN',3),'SandslashFurySwipes_MultiplierEffect':('MULTI_COIN',3),'DragonairSlam_MultiplierEffect':('MULTI_COIN',2),'DragoniteLv41Slam_MultiplierEffect':('MULTI_COIN',2),'DragoniteLv45Slam_MultiplierEffect':('MULTI_COIN',2),'NidorinaDoubleKick_MultiplierEffect':('MULTI_COIN',2),'NidorinoDoubleKick_MultiplierEffect':('MULTI_COIN',2),'JolteonDoubleKick_MultiplierEffect':('MULTI_COIN',2),'JynxDoubleslap_MultiplierEffect':('MULTI_COIN',2),'PoliwhirlDoubleslap_MultiplierEffect':('MULTI_COIN',2),'Bonemerang_MultiplierEffect':('MULTI_COIN',2),'OmastarSpikeCannon_MultiplierEffect':('MULTI_COIN',2),'CloysterSpikeCannon_MultiplierEffect':('MULTI_COIN',2),
 'DoTheWaveEffect':('DO_THE_WAVE',0),'PoliwagWaterGunEffect':('WATER_EXTRA',1),'LaprasWaterGunEffect':('WATER_EXTRA',1),'OmanyteWaterGunEffect':('WATER_EXTRA',1),'OmastarWaterGunEffect':('WATER_EXTRA',2),'SeadraWaterGunEffect':('WATER_EXTRA',1),'VaporeonWaterGunEffect':('WATER_EXTRA',2),'PoliwrathWaterGunEffect':('WATER_EXTRA',2),'HydroPumpEffect':('WATER_EXTRA',3),
}
# Whole command lists ported by hand from effect_functions.asm (checked
# against each list's functions, phases and the original damage order).
# (op, amount, extra); the handler reads both damage phases itself.
FIRE,GRASS,LIGHTNING,WATER,FIGHTING,PSYCHIC,ANY=0,1,2,3,4,5,7
lists={
 'CharmanderEmberEffectCommands':('DISCARD_TYPED',1,FIRE),'CharmeleonFlamethrowerEffectCommands':('DISCARD_TYPED',1,FIRE),
 'ArcanineFlamethrowerEffectCommands':('DISCARD_TYPED',1,FIRE),'MagmarFlamethrowerEffectCommands':('DISCARD_TYPED',1,FIRE),
 'FlareonFlamethrowerEffectCommands':('DISCARD_TYPED',1,FIRE),'NinetalesFireBlastEffectCommands':('DISCARD_TYPED',1,FIRE),
 'CharizardFireSpinEffectCommands':('DISCARD_TYPED',2,ANY),'ZapdosThunderboltEffectCommands':('DISCARD_TYPED',0,ANY),
 'ArcanineFlamesOfRageEffectCommands':('FLAMES_OF_RAGE',2,FIRE),
 'StarmieRecoverEffectCommands':('RECOVER',1,WATER),'KadabraRecoverEffectCommands':('RECOVER',1,PSYCHIC),
 'MewtwoBarrierEffectCommands':('BARRIER_ALL',1,PSYCHIC),
 'ArcanineTakeDownEffectCommands':('RECOIL',30,0),'MachokeSubmissionEffectCommands':('RECOIL',20,0),
 'JigglypuffDoubleEdgeEffectCommands':('RECOIL',20,0),'ChanseyDoubleEdgeEffectCommands':('RECOIL',80,0),
 'PikachuThunderJoltEffectCommands':('RECOIL_COIN',10,0),'RaichuThunderEffectCommands':('RECOIL_COIN',30,0),
 'ZapdosThunderEffectCommands':('RECOIL_COIN',30,0),
 'NidokingThrashEffectCommands':('THRASH',10,0),'ElectabuzzThunderpunchEffectCommands':('THRASH',10,0),
 'NidoranMHornHazardEffectCommands':('FAIL_COIN',0,0),'MoltresLv35DiveBombEffectCommands':('FAIL_COIN',0,0),
 'MoltresLv37DiveBombEffectCommands':('FAIL_COIN',0,0),'FarfetchdLeekSlapEffectCommands':('FAIL_COIN',0,1),
 'CloysterClampEffectCommands':('CLAMP',0,0),'FlyingPikachuFlyEffectCommands':('FLY',0,0),
 'KangaskhanFetchEffectCommands':('DRAW',1,0),'MeowthPayDayEffectCommands':('DRAW',1,1),
 'JigglypuffFirstAidEffectCommands':('HEAL_SELF',10,0),'SlowpokeSpacingOutEffectCommands':('HEAL_SELF',10,1),
 'VenusaurMegaDrainEffectCommands':('DRAIN_DONE',2,0),'ButterfreeMegaDrainEffectCommands':('DRAIN_DONE',2,0),
 'KabutopsAbsorbEffectCommands':('DRAIN_DONE',2,0),'VenonatLeechLifeEffectCommands':('DRAIN_DONE',1,0),
 # Leech Life heals the damage done (after Weakness and Resistance), not a fixed amount
 'ZubatLeechLifeEffectCommands':('DRAIN_DONE',1,0),'GolbatLeechLifeEffectCommands':('DRAIN_DONE',1,0),

 'GloomFoulOdorEffectCommands':('CONFUSE_BOTH',0,0),'VileplumePetalDanceEffectCommands':('PETAL_DANCE',3,40),
 'KoffingFoulGasEffectCommands':('FOUL_GAS',0,0),'VenomothVenomPowderEffectCommands':('VENOM_POWDER',0,0),
 'PrimeapeTantrumEffectCommands':('TANTRUM',0,0),'TaurosRampageEffectCommands':('RAMPAGE',0,0),
 'KinglerFlailEffectCommands':('FLAIL',0,0),'MagikarpFlailEffectCommands':('FLAIL',0,0),
 'ExeggutorBigEggsplosionEffectCommands':('EGGSPLOSION',20,0),'GeodudeStoneBarrageEffectCommands':('UNTIL_TAILS',10,0),
 'NidoqueenBoyfriendsEffectCommands':('BOYFRIENDS',20,0),'NinetalesDancingEmbersEffectCommands':('MULTI_COIN',8,0),
 'ElectrodeSonicboomEffectCommands':('NO_WR',0,0),'MagnetonSonicboomEffectCommands':('NO_WR',0,0),
 'HaunterDreamEaterEffectCommands':('DREAM_EATER',0,0),
 'WeezingSelfdestructEffectCommands':('SELFDESTRUCT',10,60),'MagnemiteSelfdestructEffectCommands':('SELFDESTRUCT',10,40),
 'MagnetonLv28SelfdestructEffectCommands':('SELFDESTRUCT',20,80),'MagnetonLv35SelfdestructEffectCommands':('SELFDESTRUCT',20,100),
 'GolemSelfdestructEffectCommands':('SELFDESTRUCT',20,100),'DugtrioEarthquakeEffectCommands':('EARTHQUAKE',10,0),
 'ArticunoBlizzardEffectCommands':('BLIZZARD',10,0),'ZapdosThunderstormEffectCommands':('THUNDERSTORM',20,10),
 'ElectrodeChainLightningEffectCommands':('CHAIN_LIGHTNING',10,0),
 'MeowthCatPunchEffectCommands':('RANDOM_OPP',20,0),'PidgeotSlicingWindEffectCommands':('RANDOM_OPP',30,0),
 'ArticunoIceBreathEffectCommands':('RANDOM_OPP',40,0),'ZapdosBigThunderEffectCommands':('BIG_THUNDER',70,0),
 'PikachuSparkEffectCommands':('CHOSEN_BENCH',10,0),'GengarDarkMindEffectCommands':('CHOSEN_BENCH',10,0),
 'HypnoDarkMindEffectCommands':('CHOSEN_BENCH',10,0),'HitmonleeStretchKickEffectCommands':('CHOSEN_BENCH',20,1),
 'RaichuGigashockEffectCommands':('GIGASHOCK',10,3),
 'NinetalesLureEffectCommands':('LURE',0,0),'VictreebelLureEffectCommands':('LURE',0,0),
 'ExeggutorTeleportEffectCommands':('TELEPORT',0,0),
 'PidgeyWhirlwindEffectCommands':('OPP_SWITCH',0,0),'PidgeottoWhirlwindEffectCommands':('OPP_SWITCH',0,0),
 'ButterfreeWhirlwindEffectCommands':('OPP_SWITCH',0,0),'ArbokTerrorStrikeEffectCommands':('OPP_SWITCH',0,1),
 'RhydonRamEffectCommands':('OPP_SWITCH',20,0),
 'GrimerMinimizeEffectCommands':('GUARD',2,0),'ClefableMinimizeEffectCommands':('GUARD',2,0),
 'GravelerHardenEffectCommands':('GUARD',3,0),'OnixHardenEffectCommands':('GUARD',3,0),
 'ElectabuzzLightScreenEffectCommands':('GUARD',4,0),'JigglypuffExpandEffectCommands':('GUARD',1,0),
 'PikachuLv16GrowlEffectCommands':('WEAKEN',1,0),'PikachuAltLv16GrowlEffectCommands':('WEAKEN',1,0),
 'PersianPounceEffectCommands':('WEAKEN',1,0),'CuboneSnivelEffectCommands':('WEAKEN',2,0),
 'HorseaSmokescreenEffectCommands':('WEAKEN',3,0),'MagmarSmokescreenEffectCommands':('WEAKEN',3,0),
 'SandshrewSandAttackEffectCommands':('WEAKEN',3,0),
 # (extra 1: it can't attack the Pokemon that did it: "benching or evolving either Pokemon ends this effect")
 'RhyhornLeerEffectCommands':('NO_ATTACK_COIN',0,1),'EeveeTailWagEffectCommands':('NO_ATTACK_COIN',0,1),
 'MarowakBoneAttackEffectCommands':('NO_ATTACK_COIN',0,0),'VictreebelAcidEffectCommands':('NO_RETREAT_COIN',0,0),
 'PsyduckHeadacheEffectCommands':('HEADACHE',0,0),
 'ScytherSwordsDanceEffectCommands':('CHARGE',0,0),'VaporeonFocusEnergyEffectCommands':('CHARGE',0,0),
 'NidoranFCallForFamilyEffectCommands':('CALL_FAMILY',0,0),'BellsproutCallForFamilyEffectCommands':('CALL_FAMILY',0,0),
 'KrabbyCallForFamilyEffectCommands':('CALL_FAMILY',0,0),'OddishSproutEffectCommands':('CALL_FAMILY',0,0),
 'MarowakCallforFriendEffectCommands':('CALL_FAMILY',0,1),'JigglypuffFriendshipSongEffectCommands':('FRIENDSHIP_SONG',0,0),
 'GastlyEnergyConversionEffectCommands':('ENERGY_CONVERSION',2,10),
 'MewtwoEnergyAbsorptionEffectCommands':('ENERGY_ABSORPTION',2,0),'MewtwoAltEnergyAbsorptionEffectCommands':('ENERGY_ABSORPTION',2,0),
 'ElectrodeEnergySpikeEffectCommands':('ENERGY_SPIKE',0,0),
 'DragonairHyperBeamEffectCommands':('HYPER_BEAM',0,0),'GolduckHyperBeamEffectCommands':('HYPER_BEAM',0,0),
 'PoliwrathWhirlpoolEffectCommands':('HYPER_BEAM',0,0),
}
covered=[];pending=[]
for name,commands in x['effects'].items():
 meaningful=[p for p in commands if p['phase'] not in ['EFFECTCMDTYPE_AI','EFFECTCMDTYPE_AI_SELECTION','EFFECTCMDTYPE_AI_SWITCH_DEFENDING_PKMN']]
 if name in lists:   # (a hand-ported list also corrects a single function's mapping)
  op,param,extra=lists[name];covered.append((name,op,param,2,extra))
 elif len(meaningful)==1 and meaningful[0]['function'] in functions:
  op,param=functions[meaningful[0]['function']];covered.append((name,op,param,int(meaningful[0]['phase']=='EFFECTCMDTYPE_AFTER_DAMAGE'),0))
 else:pending.append(name)
out=['/* Generated explicit original effect coverage. */']
for name,op,param,after,extra in covered:out.append('{"'+name+'", FX_'+op+','+str(param)+','+str(after)+','+str(extra)+'},')
(ROOT/'src/tcg/tcg_effect_table.inc').write_text('\n'.join(out)+'\n')
(ROOT/'tools/tcg/effect_coverage.json').write_text(json.dumps({'implemented_command_lists':[c[0] for c in covered],'pending_command_lists':pending},indent=2)+'\n')
print(f'Explicit attack-effect coverage: {len(covered)} command lists; {len(pending)} still require porting (including trainers/powers).')
