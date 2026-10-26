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
 'FearowAgilityEffect':('BARRIER_COIN',0),'RaichuAgilityEffect':('BARRIER_COIN',0),'RapidashAgilityEffect':('BARRIER_COIN',0),'SeadraAgilityEffect':('BARRIER_COIN',0),
 'ArcanineQuickAttack_DamageBoostEffect':('BONUS_COIN',20),'EeveeQuickAttack_DamageBoostEffect':('BONUS_COIN',20),'ElectabuzzQuickAttack_DamageBoostEffect':('BONUS_COIN',20),'FlareonQuickAttack_DamageBoostEffect':('BONUS_COIN',20),'JolteonQuickAttack_DamageBoostEffect':('BONUS_COIN',20),'VaporeonQuickAttack_DamageBoostEffect':('BONUS_COIN',20),'TaurosStomp_DamageBoostEffect':('BONUS_COIN',10),'RapidashStomp_DamageBoostEffect':('BONUS_COIN',10),
 'CuboneRage_DamageBoostEffect':('RAGE',0),'DodrioRage_DamageBoostEffect':('RAGE',0),'FlareonRage_DamageBoostEffect':('RAGE',0),
 'MrMimeMeditate_DamageBoostEffect':('MEDITATE',0),'JynxMeditate_DamageBoostEffect':('MEDITATE',0),'PsywaveEffect':('PSYWAVE',0),'Psychic_DamageBoostEffect':('PSYCHIC',0),
 'KarateChop_DamageSubtractionEffect':('KARATE_CHOP',0),'SuperFang_HalfHPEffect':('SUPER_FANG',0),
 'FuryAttack_MultiplierEffect':('MULTI_COIN',3),'CometPunch_MultiplierEffect':('MULTI_COIN',4),'Twineedle_MultiplierEffect':('MULTI_COIN',2),'PinMissile_MultiplierEffect':('MULTI_COIN',4),'NidoranFFurySwipes_MultiplierEffect':('MULTI_COIN',3),'PrimeapeFurySwipes_MultiplierEffect':('MULTI_COIN',3),'PsyduckFurySwipes_MultiplierEffect':('MULTI_COIN',3),'SandslashFurySwipes_MultiplierEffect':('MULTI_COIN',3),'DragonairSlam_MultiplierEffect':('MULTI_COIN',2),'DragoniteLv41Slam_MultiplierEffect':('MULTI_COIN',2),'DragoniteLv45Slam_MultiplierEffect':('MULTI_COIN',2),'NidorinaDoubleKick_MultiplierEffect':('MULTI_COIN',2),'NidorinoDoubleKick_MultiplierEffect':('MULTI_COIN',2),'JolteonDoubleKick_MultiplierEffect':('MULTI_COIN',2),'JynxDoubleslap_MultiplierEffect':('MULTI_COIN',2),'PoliwhirlDoubleslap_MultiplierEffect':('MULTI_COIN',2),'Bonemerang_MultiplierEffect':('MULTI_COIN',2),'OmastarSpikeCannon_MultiplierEffect':('MULTI_COIN',2),'CloysterSpikeCannon_MultiplierEffect':('MULTI_COIN',2),
 'DoTheWaveEffect':('DO_THE_WAVE',0),'PoliwagWaterGunEffect':('WATER_EXTRA',1),'LaprasWaterGunEffect':('WATER_EXTRA',1),'OmanyteWaterGunEffect':('WATER_EXTRA',1),'OmastarWaterGunEffect':('WATER_EXTRA',2),'SeadraWaterGunEffect':('WATER_EXTRA',1),'VaporeonWaterGunEffect':('WATER_EXTRA',2),'PoliwrathWaterGunEffect':('WATER_EXTRA',2),'HydroPumpEffect':('WATER_EXTRA',3),
}
covered=[];pending=[]
for name,commands in x['effects'].items():
 meaningful=[p for p in commands if p['phase'] not in ['EFFECTCMDTYPE_AI','EFFECTCMDTYPE_AI_SELECTION','EFFECTCMDTYPE_AI_SWITCH_DEFENDING_PKMN']]
 if len(meaningful)==1 and meaningful[0]['function'] in functions:
  op,param=functions[meaningful[0]['function']];covered.append((name,op,param,meaningful[0]['phase']=='EFFECTCMDTYPE_AFTER_DAMAGE'))
 else:pending.append(name)
out=['/* Generated explicit original effect coverage. */']
for name,op,param,after in covered:out.append('{"'+name+'", FX_'+op+','+str(param)+','+str(int(after))+'},')
(ROOT/'src/tcg/tcg_effect_table.inc').write_text('\n'.join(out)+'\n')
(ROOT/'tools/tcg/effect_coverage.json').write_text(json.dumps({'implemented_command_lists':[c[0] for c in covered],'pending_command_lists':pending},indent=2)+'\n')
print(f'Explicit attack-effect coverage: {len(covered)} command lists; {len(pending)} still require porting (including trainers/powers).')
