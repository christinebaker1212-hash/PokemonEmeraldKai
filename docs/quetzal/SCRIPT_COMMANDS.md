# Moemon Quetzal: custom script commands, std scripts, trainerbattle 13

Source: Moemon Quetzal English Alpha 9 ROM (addresses are ROM addresses; file offset = address - 0x08000000).
`gScriptCmdTable` is at 0x082BE9D0 (246 entries). 0x00-0xE2 are stock Emerald; 0xE3-0xF5 are Quetzal's.
Machine-readable names: `tools/quetzal_rip/qz_names.json` (`commands`).

Abbreviations: sb1/sb2 = gSaveBlock1Ptr / gSaveBlock2Ptr. "Giovanni mode" = the Celebi alternate-past story
in which the player plays Giovanni: flag 0x1318 set, sb2+0x2C7 bit 6 set (see SPECIALS.md 693-703).
A handler that returns TRUE makes the script wait a frame (same rule as stock).

| Op | Handler | Proposed macro | Args (bytes) | Used in quetzal_rip |
|----|---------|----------------|--------------|---------------------|
| E3 | 0x080E141C | `bufferfollowerspecies` | b strVar | 0 |
| E4 | 0x080E0DAC | `getplayergender` | - | 0 |
| E5 | 0x080E1478 | `questmenu` | b op, b var | 0 |
| E6 | 0x080E1540 | `waitframe_vars` | b (skipped), h, h | 0 |
| E7 | 0x080E1570 | `setfollowergfxvar` | b mode | 0 |
| E8 | 0x080E0BB8 | `dostaticwildbattle` | - | 0 |
| E9 | 0x082A0580 | `givestorymon` | h id | 4 |
| EA | 0x082A071C | `takebackstorymon` | h id | 1 |
| EB | 0x082A04CC | `keeponlyleadmon` | - | 0 |
| EC | 0x082A053C | `setstaticwildmon` | h species, b level, h item | 1 |
| ED | 0x082A0818 | `takepartymonofspecies` | h species | 2 |
| EE | 0x082A08F0 | `checkpartymoninlist` | b list | 4 |
| EF | 0x082A0A20 | `giveoddegg` | h var | 1 |
| F0 | 0x082A0B0C | `addbattlepoints` | h var/value | 0 |
| F1-F4 | 0x080DE5F4 | `nop_f1`..`nop_f4` | - | 0 (same handler as `nop`) |
| F5 | 0x082A0B78 | `checkmultiplayerorflag` | - | 0 |

## Details

**E3 bufferfollowerspecies b** — `slot = FollowerPartySlot(Giovanni mode ? 2 : 1)` (0x08264E7C via 0x082678A0),
species = that party mon's species (0 if no follower); plays its cry (0x080F36AC), copies its species name into the
string buffer table at 0x09114334[b] (gStringVar1..). Returns FALSE. Quetzal has overworld followers, like Kai.

**E4 getplayergender** — `VAR_RESULT = Giovanni mode ? 0xFF : sb2->playerGender (sb2+8)`.

**E5 questmenu op, var** — a quest log (the "questmenu" feature; quest records are 8-byte entries at sb2+0x2F8,
names/descriptions at 0x09594768, all still placeholders "Generic quest 1", "Side Quest 7"...). `value = VarGet(var)`.
op 0: open the quest menu UI (0x0827A718, returns to the field with CB2_ReturnToFieldContinueScript), waits.
op 1: nothing. op 2: set bit 6 of sb2[0x2FA+8*v]. op 3: VAR_RESULT = sb2[0x2F8+8*v] != 0 (unlocked).
op 4: VAR_RESULT = bit 6 of sb2[0x2FA+8*v]. op 5: StringCopy(gStringVar1, quest name v). Not used by any map. Kai: no quest menu; skip.

**E6** — skips one byte, reads two halfwords and VarGets both, discards them, returns TRUE (one-frame wait).
Effectively a no-op. Not used.

**E7 setfollowergfxvar mode** — `gfx = FollowerObjectGfxId()` (0x082677DC: species-based object graphics id;
+5000 shiny, +20000 female form... ; alternate path if flag 0xB7B). mode 0: VarSet(0x4011 = VAR_OBJ_GFX_ID_1),
1: 0x401F (VAR_OBJ_GFX_ID_F), 2: 0x4011 and 0x4010, 3: VarSet(0x4013, gfx of the party mon at 0x0203BA78)
(0x08267A50; 0x1EB fallback). Lets cutscenes show the follower as a normal object. Not used in the ripped maps.

**E8 dostaticwildbattle** — calls 0x08104600: if a co-op/MPlayer battle is pending (0x082B04AC) it starts the
normal scripted wild battle; else if 0x081687FC it calls 0x08103E44; else a legendary-style wild battle
(own transition, gMain.savedCallback 0x08104AAC, music 0x1D). Then stops the script (waitstate not needed).
GUESS on the exact distinctions. Kai equivalent: `special BattleSetup_StartLegendaryBattle` / `dowildbattle`.

**E9 givestorymon id** — GSC story gifts, all Lv20 unless noted, OT = player, also marks dex seen+caught:
1 Spearow nicknamed KENYA (Route 35 gate), 2 Shuckle SHUCKIE holding Berry Juice (Mania's house),
3 Rotom (Bill's family house), 4 Dratini Lv35 (Dragon Shrine; nature fixed via 0x082A049C, extra
fields set via 0x082A0454; holds Everstone, or a randomizer-chosen item if sb2+0x2D1 bit 0).
VAR_RESULT = GiveMonToPlayer result (0 party, 1 PC, 2 no room); 2 also when Nuzlocke blocks gifts (0x08291728).
Scripts: `setvar VAR_TEMP_1, species; bufferspeciesname 0, SPECIES_X; qz_cmd_e9 n; call Quetzal_EventScript_840181E`.
Kai: `givemon` + `setmonmetlocation`/nickname script code; port as a small special or inline script.

**EA takebackstorymon id** — Looks for the gift by nickname (1 "KENYA", 2 "SHUCKIE") in the party (needs 2+ mons).
Not found: VAR_RESULT = 2. id 2 and friendship > 200: VAR_RESULT = 3 (Mania lets you keep it).
Else removes that mon (follower slot fix, zero, CompactPartySlots) and VAR_RESULT = 0. Route 31 (Kenya mail).

**EB keeponlyleadmon** — lead fainted or lead species 0x5F9 (egg): VAR_RESULT = 2. Otherwise deletes party slots 1-5
(scripts save the party first with SavePlayerParty). VAR_RESULT = 0.

**EC setstaticwildmon species, level, item** — CreateScriptedWildMon (0x081658E4) into gEnemyParty[0], then 0x082A0454
(sets two Quetzal-only mon data fields 0x59/0x5A and personality bit 8; GUESS: marks it as a static/event mon), recalcs stats.
Like stock `setwildbattle`. Use: Lake of Rage red Gyarados (`qz_cmd_ec SPECIES_GYARADOS, 30, ITEM_NONE` then `dowildbattle`).

**ED takepartymonofspecies species** — mon = party[VAR_0x8004]. VAR_RESULT: 0 slot invalid / wrong species / egg;
3 it is the last usable mon (0x081256A8); 4 it holds an item; else the mon is removed and VAR_RESULT = 1 if it was
Magikarp (0x81) at Lv100, otherwise 2. Use: Lake of Rage Magikarp house buys your Magikarp (`qz_cmd_ed 0x81` after
ChoosePartyMon; then showmoneybox).

**EE checkpartymoninlist list** — VAR_RESULT = 1 if party[VAR_0x8004] (non-egg) is in a hard-coded list (national dex numbers):
1 Cacnea, Lotad, Makuhita, Lombre, Trapinch, Beldum, Vibrava; 2 Tropius, Chimecho, Absol, Castform;
3 Barboach, Whiscash, Meditite, Numel, Baltoy, Absol, Medicham, Camerupt; 4 Whismur, Nosepass, Bagon, Relicanth, Feebas.
Use: Johto Safari Zone gate house (buys Hoenn Pokémon, lists 1-4).

**EF giveoddegg var** — index = VarGet(var) 1..7 (script: `random 7; addvar VAR_RESULT, 1`) into species table
0x095A4C14; species re-rolled if egg randomizer (sb2+0x2D9 bit 5); CreateEgg, hatch counter 10, first move Dizzy Punch
(or a random move if sb2+0x2D1 bit 0). VAR_RESULT = give result, 2 if Nuzlocke blocks. Use: Route 34 Day-Care Odd Egg.

**F0 addbattlepoints v** — sb2+0x244 (frontier BP) += v capped 9999; sb2+0x246 (lifetime BP) += v capped 65535;
ConvertIntToDecimalString(gStringVar1, v, 3 digits); 0x0815FD60 (record/TV hook). Kai: `special`-less; port as a
small helper using `gSaveBlock2Ptr->frontier.battlePoints`.

**F5 checkmultiplayerorflag** — VAR_RESULT = (0x08263DFC() == 1 [MPlayer link session]) || (sb2+0x2D0 bit 1). GUESS on meaning.

## New std scripts (gStdScripts 0x082BFB18)

| Id | Address | Body | Use |
|----|---------|------|-----|
| 11 | 0x0835F079 | `message; waitmessage; yesnobox 20, 8; nop1; return` | identical to MSGBOX_YESNO (std 5). Not used. |
| 12 | 0x0835F22D | `message; waitmessage; delay 20; release; return` | auto-closing message (no button). Not used. |

`gotostd 254` in Johto_IcePath1F is a ripper mis-decode of data, not a std call.

## trainerbattle type 13

BattleSetup_ConfigureTrainerBattle = 0x08105EA0, jump table 0x08105EE0 (modes 1-13). Mode 13 runs the same path as
mode 3 (SINGLE_NO_INTRO_TEXT, returns EventScript at 0x0835F14E) but with parameter table 0x091818B8:

| # | Field | Mode 3 (0x09181870) | Mode 13 |
|---|-------|---------------------|---------|
| 1 | mode u8 | load | load |
| 2 | opponent A u16 (0x0203A6D4) | load | load |
| 3 | u16 | object local id (0x0203A6DA) | **new u16 at 0x0203A6D2** |
| 4 | intro text | clear | clear |
| 5 | defeat text | load | load |
| 6 | victory text (0x0203A6EC) | clear | **load** |
| 7-9 | cannot-battle text, before-battle script, return addr | clear/clear/ret | same |

Byte layout: `5C 0D, h trainer, h flags, w defeatText, w victoryText`. This matches pokefirered's
`TRAINER_BATTLE_EARLY_RIVAL` (`trainerbattle_earlyrival trainer, rivalFlags, defeat, victory`: the battle can be lost
without blacking out, `RIVAL_BATTLE_HEAL_AFTER` 1 / `RIVAL_BATTLE_TUTORIAL` 2). Uses: Kanto Oak's Lab rival (flags 3)
and Route 22 rival (flags 0). Where the flags are read was not traced (GUESS that semantics match FRLG).
Kai: no such mode; port as a `trainerbattle_no_intro` plus a "lose without whiteout" option (Emerald has
`B_FLAG_NO_WHITEOUT`-style code only in expansion; Kai needs a small battle_setup change), or reuse Wally's tutorial path.
