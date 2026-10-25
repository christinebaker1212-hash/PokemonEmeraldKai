# Moemon Quetzal: specials

`gSpecials` is at ROM 0x082BEE04 and has **837** entries (0-836); `gStdScripts` follows at 0x082BFB18.
Machine-readable names: `tools/quetzal_rip/qz_names.json` (`specials` for every Quetzal special the rip uses,
`stock_index_names` for raw indices 0-539).

**How the rip writes them** (applied after this file was first written): a raw index that holds a stock special Kai
still has is written by Kai's name (`special ShouldTryRematchBattle`); every other one as `Qz<Name>_<raw index>`
(`special QzQuestLogNop_737`), defined in `quetzal_rip/include/quetzal_macros.inc`. Where this file says
`QZ_SPECIAL_<n>`, the rip now has `Qz<Name>_<n>` with the name from the table below.

## Important: raw index != pokeemerald index below 540

Quetzal **inserted 13 specials in the middle of the stock table**, so a raw ROM index is not pokeemerald's index.
This was established by matching every entry's code against the vanilla Emerald ROM (Kai's base ROM,
gSpecials at 0x081DBA64, 527 entries) with a bytewise similarity score plus an order-preserving alignment, then
checking the low-scoring entries by hand (e.g. raw 62 is `ShouldTryRematchBattle` after every `trainerbattle`,
raw 193 is `GetBattleOutcome` after `dowildbattle`, raw 494 is `CreateEnemyEventMon` reading VAR_0x8004-0x8006).

| Raw index | = pokeemerald index | Notes |
|---|---|---|
| 0 | new | `HealPlayerPartyEx` (copy of HealPlayerParty with an extra follower/flag step; scripts call it after heals) |
| 1-40 | raw - 1 | 1 = HealPlayerParty ... 40 = SpawnLinkPartnerObjectEvent |
| 41, 42 | raw - 1 | SavePlayerParty, LoadPlayerParty (LoadPlayerParty also re-checks the region) |
| 43 | new | backs up the party to 0x0202383C when option sb2+0x2E7 bits 1-2 are set (GUESS: battle-mode party backup) |
| 44-133 | raw - 2 | |
| 134, 135 | raw - 2 | stubs (`bx lr`): GetDewfordHallPaintingNameIndex, SwapRegisteredBike |
| 136 | 134 | CalculatePlayerPartyCount |
| 137-143 | new | option getters: 137 party size limit (6 - sb2[0x2D2]>>5), 138 sb2[0x2D3] bits 2-3, 139/140 Nuzlocke sub-options, 141 sb2[0x2D3] bit 7 (item randomizer), 142/143 randomize item in VAR_0x8000 / VAR_0x8005 (0x08149CA0) |
| 144 | 135 | CountPartyNonEggMons |
| 145-167 | raw - 9 | |
| 168 | 159 | ChooseStarter (Hoenn; `0x082B2C10(0)`) |
| 169 | new | calls 0x08105B70 (not used by maps) |
| 170-489 | raw - 10 | |
| 490 | 480 | DoDeoxysRockInteraction |
| 491 | new | second rock-interaction task (0x081C41E9) |
| 492 | new | Deoxys-rock-style palette/position from var 0x5071 |
| 493 | 481 | SetDeoxysRockPalette (var 0x4035, or 0x5101 on map 0x25.0x35 Birth Island) |
| 494 | 482 | CreateEnemyEventMon (Quetzal version: species/level/item from VAR_0x8004/0x8005/0x8006, randomizer-aware) |
| 495 | new | enemy-mon fixup (species 0x1ED special case, option sb2+0x2D1 bit 0) |
| 496-539 | raw - 13 | 528 = PlayerNotAtTrainerHillEntrance, 538 = CloseDeptStoreElevatorWindow, 539 = last stock |
| 540-836 | new | Quetzal's own |

Many stock entries are stubbed to `bx lr` or `RESULT = 0` (TV, contests, Rock Smash encounters...).

**Consequence for the rip:** the first rip named stock specials `pokeemerald[raw-1]` (its list included the macro line);
that happens to be right only for raw 1-42. Naming them `pokeemerald[raw]` is wrong for *every* raw index >= 1.
Use `stock_index_names` from qz_names.json. The 75 stock specials the maps use, corrected:

| Raw | Uses | Old rip name | pokeemerald[raw] (naive) | Real (Quetzal) |
|---|---|---|---|---|
| 0 | 15 | ptr | HealPlayerParty | `HealPlayerPartyEx` **≠** |
| 33 | 1 | CloseLink | ColosseumPlayerSpotTriggered | `CloseLink` **≠** |
| 41 | 3 | SavePlayerParty | LoadPlayerParty | `SavePlayerParty` **≠** |
| 42 | 3 | LoadPlayerParty | ChooseHalfPartyForBattle | `LoadPlayerParty` **≠** |
| 43 | 1 | ChooseHalfPartyForBattle | Script_ShowLinkTrainerCard | `BackupPartyForOptionBattle` **≠** |
| 44 | 2 | Script_ShowLinkTrainerCard | ObjectEventInteractionGetBerryTreeData | `ChooseHalfPartyForBattle` **≠** |
| 46 | 1 | ObjectEventInteractionGetBerryName | ObjectEventInteractionGetBerryCountString | `ObjectEventInteractionGetBerryTreeData` **≠** |
| 48 | 1 | Bag_ChooseBerry | ObjectEventInteractionPlantBerryTree | `ObjectEventInteractionGetBerryCountString` **≠** |
| 49 | 1 | ObjectEventInteractionPlantBerryTree | ObjectEventInteractionPickBerryTree | `Bag_ChooseBerry` **≠** |
| 51 | 1 | ObjectEventInteractionRemoveBerryTree | ObjectEventInteractionWaterBerryTree | `ObjectEventInteractionPickBerryTree` **≠** |
| 52 | 1 | ObjectEventInteractionWaterBerryTree | PlayerHasBerries | `ObjectEventInteractionRemoveBerryTree` **≠** |
| 54 | 3 | IsEnigmaBerryValid | GetTrainerBattleMode | `PlayerHasBerries` **≠** |
| 62 | 234 | IsTrainerReadyForRematch | BattleSetup_StartRematchBattle | `ShouldTryRematchBattle` **≠** |
| 139 | 3 | SaveMuseumContestPainting | DoesContestCategoryHaveMuseumPainting | `IsNuzlockeOption2D4Bit6` **≠** |
| 141 | 1 | CountPlayerMuseumPaintings | ShowContestPainting | `IsItemRandomizerOn` **≠** |
| 142 | 1 | ShowContestPainting | MauvilleGymSetDefaultBarriers | `RandomizeItemInVar8000` **≠** |
| 154 | 79 | SetHiddenItemFlag | CableCarWarp | `DrawWholeMapView` **≠** |
| 160 | 12 | ChooseStarter | StartWallyTutorialBattle | `GetPlayerBigGuyGirlString` **≠** |
| 162 | 1 | ChangePokemonNickname | ChoosePartyMon | `SetHiddenItemFlag` **≠** |
| 166 | 1 | PlayRoulette | IsFanClubMemberFanOfPlayer | `StartWallClock` **≠** |
| 167 | 1 | IsFanClubMemberFanOfPlayer | GetNumFansOfPlayerInTrainerFanClub | `Special_ViewWallClock` **≠** |
| 171 | 3 | TryLoseFansFromPlayTime | SetPlayerGotFirstFans | `ChangePokemonNickname` **≠** |
| 172 | 13 | SetPlayerGotFirstFans | UpdateTrainerFanClubGameClear | `ChoosePartyMon` **≠** |
| 174 | 1 | Script_TryGainNewFanFromCounter | RockSmashWildEncounter | `DoBerryBlending` **≠** |
| 175 | 1 | RockSmashWildEncounter | GabbyAndTyGetBattleNum | `PlayRoulette` **≠** |
| 183 | 1 | GetGabbyAndTyLocalIds | GetBattleOutcome | `Script_TryGainNewFanFromCounter` **≠** |
| 184 | 2 | GetBattleOutcome | GetDaycareMonNicknames | `RockSmashWildEncounter` **≠** |
| 193 | 32 | ShowDaycareLevelMenu | GetNumLevelsGainedFromDaycare | `GetBattleOutcome` **≠** |
| 209 | 1 | EnterSafariMode | ExitSafariMode | `ShowLinkBattleRecords` **≠** |
| 213 | 1 | IsMirageIslandPresent | UpdateShoalTideFlag | `SetCB2WhiteOut` **≠** |
| 219 | 1 | DoPCTurnOffEffect | SetDeptStoreFloor | `ExitSafariMode` **≠** |
| 244 | 3 | CallBattlePyramidFunction | StopMapMusic | `CallFrontierUtilFunc` **≠** |
| 249 | 3 | ChoosePartyForBattleFrontier | ValidateEReaderTrainer | `DoSpecialTrainerBattle` **≠** |
| 261 | 3 | RetrieveLotteryNumber | PickLotteryCornerTicket | `ReducePlayerPartyToSelectedMons` **≠** |
| 265 | 6 | SetTrickHouseNuggetFlag | ScriptMenu_CreatePCMultichoice | `GetInGameTradeSpeciesInfo` **≠** |
| 266 | 6 | ScriptMenu_CreatePCMultichoice | AccessHallOfFamePC | `CreateInGameTradePokemon` **≠** |
| 267 | 6 | AccessHallOfFamePC | Special_ShowDiploma | `DoInGameTradeScene` **≠** |
| 268 | 6 | Special_ShowDiploma | CheckLeadMonCool | `GetTradeSpecies` **≠** |
| 270 | 1 | CheckLeadMonBeauty | CheckLeadMonCute | `RetrieveLotteryNumber` **≠** |
| 271 | 1 | CheckLeadMonCute | CheckLeadMonSmart | `PickLotteryCornerTicket` **≠** |
| 277 | 1 | MoveElevator | ShowGlassWorkshopMenu | `Special_ShowDiploma` **≠** |
| 278 | 1 | ShowGlassWorkshopMenu | SpawnCameraObject | `CheckLeadMonCool` **≠** |
| 285 | 3 | DoOrbEffect | FadeOutOrbEffect | `GameClear` **≠** |
| 286 | 4 | FadeOutOrbEffect | WaitWeather | `MoveElevator` **≠** |
| 288 | 8 | BufferEReaderTrainerName | GetSlotMachineId | `SpawnCameraObject` **≠** |
| 289 | 8 | GetSlotMachineId | GetPlayerFacingDirection | `RemoveCameraObject` **≠** |
| 298 | 12 | Script_FacePlayer | Script_ClearHeldMovement | `GetSlotMachineId` **≠** |
| 299 | 1 | Script_ClearHeldMovement | InitRoamer | `GetPlayerFacingDirection` **≠** |
| 307 | 2 | ScriptCheckFreePokemonStorageSpace | DoSealedChamberShakingEffect_Long | `Script_FacePlayer` **≠** |
| 308 | 16 | DoSealedChamberShakingEffect_Long | ShowDeptStoreElevatorFloorSelect | `Script_ClearHeldMovement` **≠** |
| 309 | 1 | ShowDeptStoreElevatorFloorSelect | InteractWithShieldOrTVDecoration | `InitRoamer` **≠** |
| 310 | 1 | InteractWithShieldOrTVDecoration | IsPokerusInParty | `TryUpdateRusturfTunnelState` **≠** |
| 318 | 4 | DoSealedChamberShakingEffect_Short | FoundBlackGlasses | `ShowDeptStoreElevatorFloorSelect` **≠** |
| 320 | 1 | StartDroughtWeatherBlend | DoDiveWarp | `IsPokerusInParty` **≠** |
| 322 | 28 | DoFallWarp | ShowContestEntryMonPic | `ShakeCamera` **≠** |
| 324 | 16 | HideContestEntryMonPic | SetEReaderTrainerGfxId | `BattleSetup_StartLegendaryBattle` **≠** |
| 349 | 1 | ShowGlassWorkshopMenu | PutFanClubSpecialOnTheAir | `BufferLottoTicketNumber` **≠** |
| 355 | 2 | DeclinedSecretBaseBattle | DrewSecretBaseBattle | `TryPutLotteryWinnerReportOnAir` **≠** |
| 361 | 1 | CheckInteractedWithFriendsFurnitureBottom | CheckInteractedWithFriendsFurnitureMiddle | `IncrementDailyPickedBerries` **≠** |
| 430 | 1 | ShowPokemonJumpRecords | IsDodrioInParty | `InitUnionRoom` **≠** |
| 431 | 2 | IsDodrioInParty | ShowDodrioBerryPickingRecords | `BufferUnionRoomPlayerName` **≠** |
| 471 | 1 | Script_DoRayquazaScene | OpenPokenavForTutorial | `ShowFrontierExchangeCornerItemIconWindow` **≠** |
| 483 | 1 | CreateEnemyEventMon | StartMirageTowerDisintegration | `CountPlayerTrainerStars` **≠** |
| 490 | 1 | SetMatchCallRegisteredFlag | DoDomeConfetti | `DoDeoxysRockInteraction` **≠** |
| 491 | 1 | DoDomeConfetti | CreateAbnormalWeatherEvent | `DoDeoxysRockInteractionAlt` **≠** |
| 492 | 1 | CreateAbnormalWeatherEvent | GetAbnormalWeatherMapNameAndType | `SetDeoxysRockPaletteVar5071` **≠** |
| 493 | 1 | GetAbnormalWeatherMapNameAndType | GetMartEmployeeObjectEventId | `SetDeoxysRockPalette` **≠** |
| 494 | 13 | GetMartEmployeeObjectEventId | SaveForBattleTowerLink | `CreateEnemyEventMon` **≠** |
| 495 | 1 | SaveForBattleTowerLink | Unused_SetWeatherSunny | `QzEnemyMonEventFixup` **≠** |
| 499 | 3 | ShouldDoBrailleRegicePuzzle | EnableNationalPokedex | `ChangeBoxPokemonNickname` **≠** |
| 500 | 4 | EnableNationalPokedex | ScriptMenu_CreateLilycoveSSTidalMultichoice | `GetPCBoxToSendMon` **≠** |
| 501 | 4 | ScriptMenu_CreateLilycoveSSTidalMultichoice | GetLilycoveSSTidalSelection | `ShouldShowBoxWasFullMessage` **≠** |
| 509 | 1 | Script_DoRayquazaScene | LoopWingFlapSE | `SetUnlockedPokedexFlags` **≠** |
| 512 | 6 | ShowBerryCrushRankings | TryBufferWaldaPhrase | `EnableNationalPokedex` **≠** |
| 522 | 6 | IsLastMonThatKnowsSurf | CountPartyAliveNonEggMons | `LoopWingFlapSE` **≠** |

## New specials (raw 528+) used by the rip

Columns: proposed name (CamelCase, prefix-free), what it does (ROM address in qz_names.json order is gSpecials[n]),
special vars in/out, use count with example maps (J: = Johto_, K: = Kanto_), and the Kai equivalent.
"needs porting" = no Kai equivalent. GUESS marks an unverified reading. Common helpers: 0x08291728 =
"Nuzlocke blocks gifts in this region" (sb2+0x2D1 bit 6 = Nuzlocke on), 0x082A0BB4 = current region,
0x082A300C = flag 0x1318 (Giovanni mode), 0x082AF6DC = flag 0x1425 (dream event), 0x082A42EC = flag 0x14AA
(time travel), 0x082ADE58 = flag 0x292B (SS Anne dream event), 0x08165470 / 0x0809FA60 = give mon,
gBattleOutcome = 0x020233A4, the wild/enemy mon written by setwildbattle-style code is at 0x02023AAC (GUESS gEnemyParty; 0x0202383C, right after gPlayerParty 0x020235CC, is another 6-mon array). struct Pokemon is 0x68 bytes in Quetzal (stock 0x64).
Species numbers below are Quetzal's (national-dex order).

### Stock entries with ROM index >= 528

| # | Name | What it does | Vars | Used (count: examples) | Kai |
|---|---|---|---|---|---|
| 528 | `PlayerNotAtTrainerHillEntrance` | **Stock** (raw 528 = pokeemerald #515): location != Trainer Hill Entrance (map 26.60) | out RESULT | 2: scripts/kanto_shared.inc, scripts/johto_shared.inc | stock `PlayerNotAtTrainerHillEntrance` |
| 538 | `CloseDeptStoreElevatorWindow` | **Stock** (pokeemerald #525): close elevator floor window | - | 9: K:TrainerTower_Elevator, K:RocketHideout_Elevator, K:SilphCo_Elevator | stock `CloseDeptStoreElevatorWindow` |

### Gifts, starters, fossils, eggs

| # | Name | What it does | Vars | Used (count: examples) | Kai |
|---|---|---|---|---|---|
| 540 | `SetEnemyMonOverrides` | Writes per-slot override bytes for an enemy party mon: entry 0x02023594+8*VAR_0x8000, values from VAR_0x8001..0x8007, bitmask + bit7 "active". Set before legendary battles (all = 6). GUESS: fixed IV/perfect-IV overrides | in 0x8000-0x8007 | 2: J:RuinsOfAlph_Cave5, scripts/kanto_shared.inc | needs porting |
| 567 | `GiveRevivedFossilMon` | Gives the revived fossil mon VAR_0x800A at Lv20 (Kanto) / Lv10 (Johto), capped by 0x08072088 (GUESS: level cap); species may be re-rolled by the randomizer (0x08263DFC/0x08262888) | in VAR_0x800A; out RESULT | 6: K:CinnabarIsland_PokemonLab_ExperimentRoom, J:RuinsOfAlph_PokemonCenter_1F1 | `givemon` |
| 561 | `BagHasFossil` | RESULT=1 if the bag holds any fossil (items 0xA5-0xB3) | out RESULT | 2: K:CinnabarIsland_PokemonLab_ExperimentRoom, J:RuinsOfAlph_PokemonCenter_1F1 | `checkitem` loop |
| 562 | `ChooseItemFromBagForFossil` | Opens the bag to choose an item (0x08228CE8(4,0,CB2_ReturnToFieldContinueScript)); result in VAR_ITEM_ID; waitstate | out VAR_ITEM_ID | 4: K:CinnabarIsland_PokemonLab_ExperimentRoom, J:RuinsOfAlph_PokemonCenter_1F1 | `ChooseItem`-style bag (Kai: `special ChooseBerry`-like, needs a fossil pocket filter) |
| 563 | `IsSelectedItemFossil` | RESULT = VAR_ITEM_ID is a fossil (0xA5-0xB3: Helix..Fossilized Dino) | in VAR_ITEM_ID, out RESULT | 4: K:CinnabarIsland_PokemonLab_ExperimentRoom, J:RuinsOfAlph_PokemonCenter_1F1 | needs porting |
| 564 | `IsSelectedItemFossilHalf` | If VAR_ITEM_ID is a Gen 8 fossil half (0xB0-0xB3): copy to VAR_0x800B, RESULT=1 | in VAR_ITEM_ID; out VAR_0x800B, RESULT | 2: K:CinnabarIsland_PokemonLab_ExperimentRoom, J:RuinsOfAlph_PokemonCenter_1F1 | needs porting |
| 565 | `AreFossilHalvesCompatible` | Swaps VAR_ITEM_ID/VAR_0x800B and RESULT=1 if one is Bird/Fish and the other Drake/Dino | in/out VAR_ITEM_ID, VAR_0x800B; out RESULT | 2: K:CinnabarIsland_PokemonLab_ExperimentRoom, J:RuinsOfAlph_PokemonCenter_1F1 | needs porting |
| 673 | `PartyHasTogepiLineOrTogekiss` | RESULT=1 if a non-egg party mon is species 0xAF/0xB0 (Togepi/Togetic) or 0x1D4 (Togekiss) | out RESULT | 1: J:ElmsLab | `checkpartymon`-like |
| 689 | `ChooseStarterJohto` | Johto starter choice UI (0x082B2C10(1)); RESULT=species or 0; waitstate. Stock raw 168 is the Hoenn one (arg 0) | out RESULT | 1: J:ElmsLab | stock `ChooseStarter` + region tables |
| 690 | `GiveStarterJohto` | Give starter VAR 0x4002 at Lv5 (0x08165470; a 1/3 roll passes 3 as an extra argument, GUESS: hidden ability/IV perk) | out RESULT | 1: J:ElmsLab | `givemon` |
| 691 | `ChooseStarterKanto` | Kanto starter choice (0x082B2C10(2)) | out RESULT | 1: K:PalletTown_ProfessorOaksLab | as 689 |
| 692 | `GiveStarterKanto` | Give Kanto starter (as 690) | out RESULT | 1: K:PalletTown_ProfessorOaksLab | `givemon` |
| 831 | `BufferLastReceivedMonForNickname` | After a gift: last received mon (party slot, or PC when VAR_0x8004 >= 6) species -> STR_VAR_1; RESULT=1 can nickname | in 0x8004; out RESULT | 3: J:ElmsLab, K:PalletTown_ProfessorOaksLab, scripts/shared.inc | stock `ChangePokemonNickname`/`GetPCBoxToSendMon` |
| 832 | `NicknameLastReceivedMon` | Nickname the last received mon (party: ChangePokemonNickname, PC: naming screen); waitstate | in 0x8004 | 1: scripts/shared.inc | stock `ChangePokemonNickname` / `ChangeBoxPokemonNickname` |

### Marts

| # | Name | What it does | Vars | Used (count: examples) | Kai |
|---|---|---|---|---|---|
| 569 | `ShowMegaStoneShop` | Mart with the Mega Stone list (60 items, 0x08154440 = CreateMart) | - | 2: K:VermilionCity, J:GoldenrodDeptStore5F | `pokemart` list |
| 570 | `ShowZCrystalShop` | Mart with the Z-Crystal list (35 items) | - | 2: K:VermilionCity, J:GoldenrodDeptStore5F | `pokemart` list |
| 571 | `ShowLegendaryItemShop` | Mart with legendary items (Soul Dew, orbs, Rusted Sword...) | - | 2: K:VermilionCity, J:GoldenrodDeptStore4F | `pokemart` list |

### Static / legendary encounters

| # | Name | What it does | Vars | Used (count: examples) | Kai |
|---|---|---|---|---|---|
| 704 | `JohtoStaticEncountersRefresh` | Johto static encounters ON_TRANSITION: for 106 entries of table 0x095BAA00 (32 bytes: map group/num, species @4, level @6, item @8, show flag @0xA, flag2 @0xC, kind @0x1F) sets/clears each object flag; extra flags 0x1313-0x1315 | - | 5: scripts/johto_shared.inc, J:WhirlIslands_Cave7, J:TinTower_Area1 | needs porting (Kai: per-map object flags) |
| 705 | `JohtoStaticEncounterSetup` | Static encounter for the talked object: finds entry, var 0x506F = index+1, VAR_0x8004 species, 0x8005 level, 0x8006 item, plays cry; RESULT=1. Some kinds require all-Lv100 party matching 0x095BB740 | out 0x8004-0x8006, RESULT, var 0x506F | 3: J:DarkCaveBlackthornEntrance, J:MtSilver_Cave1, scripts/johto_shared.inc | `seteventmon` |
| 706 | `JohtoStaticEncounterSetupTower` | Same for Embedded Tower (0x082AA5A8 entry) | out 0x8004-0x8006, RESULT | 1: J:EmbeddedTower_Cave1 | needs porting |
| 707 | `SetupUnownStaticEncounter` | Fixed: VAR_0x8004=0x544, 0x8005=70, 0x8006=item 0x88, plays cry | out 0x8004-0x8006 | 1: J:RuinsOfAlphOutside | `seteventmon` |
| 708 | `JohtoStaticEncounterAfterBattle` | After static battle: gBattleOutcome (0x020233A4) 7 caught -> record + clear flag, 1 won -> clear both flags; also patches the saved object template flag (sb1+0x1790 table); RESULT=1 handled | out RESULT | 4: J:DarkCaveBlackthornEntrance, J:RuinsOfAlph_Cave5, J:EmbeddedTower_Cave1 | `specialvar GetBattleOutcome` + flags |
| 709 | `JohtoStaticEncounterSetup2` | Wrapper of 705 | out 0x8004-0x8006, RESULT | 1: J:RuinsOfAlph_Cave5 | needs porting |
| 711 | `ClearStaticFlagAfterWildBattle` | After wild battle: outcome won/caught -> FlagClear 0x1313, RESULT=1 | out RESULT | 1: J:RuinsOfAlphOutside | needs porting |
| 755 | `KantoStaticEncounterLookupBySpecies` | Kanto static encounter lookup by species VAR_0x8004 on this map (table 0x095BB74C, 123 x 16 bytes) -> var 0x5148, RESULT | in 0x8004; out RESULT, var 0x5148 | 7: K:NavelRock_Summit, K:NavelRock_Base, K:PowerPlant | `seteventmon` |
| 756 | `KantoStaticEncounterAfterBattle` | Kanto static after battle (outcome 1/7): clear flags, RESULT=1, VAR_0x8004 = entry local id | out RESULT, 0x8004 | 9: scripts/kanto_shared.inc, K:NavelRock_Summit, K:NavelRock_Base | needs porting |
| 757 | `KantoStaticEncountersRefresh` | Kanto static encounters ON_LOAD/TRANSITION: refresh visibility flags (also 0x2E3D/0x2E3E) | - | 4: scripts/kanto_shared.inc, K:SevenIsland_SevaultCanyon_House, K:SSAnne_Exterior | needs porting |
| 758 | `KantoStaticEncounterSetupByLocalId` | Kanto static encounter for VAR_LAST_TALKED: 0x8004 species, 0x8005 level, 0x8006 0, cry, RESULT | out 0x8004-0x8006, RESULT | 5: scripts/kanto_shared.inc, K:SSAnne_1F_Room6, K:SSAnne_Exterior | needs porting |
| 769 | `KantoStaticEncounterPrepareMon` | Kanto static: CreateEnemyEventMon from vars (raw 494) then set EXP for its level | - | 1: scripts/kanto_shared.inc | needs porting |
| 771 | `SetupIcePathLegendary` | Ice Path legendary: enemy IVs all 31 (data 0x42 = 0x3FFFFFFF), species 0x1D9 gets fixed moves unless move randomizer | - | 1: J:IcePath_Cave4 | needs porting |
| 753 | `StartMarowakGhostBattle` | Pokémon Tower Marowak ghost: if enemy is 0x3C6 sets its moves (unless move randomizer), RESULT=0, starts battle; waitstate | out RESULT | 1: K:PokemonTower_6F | needs porting |

### Bug-Catching Contest

| # | Name | What it does | Vars | Used (count: examples) | Kai |
|---|---|---|---|---|---|
| 680 | `BugContestStart` | Bug-Catching Contest start: Nuzlocke -> RESULT 4; saves party into contest state 0x0203EE8C (0x4BC bytes), keeps only the lead, gives 20 Safari Balls (item 0x17) (RESULT 3 if bag full), RESULT 0 ok / 2 | in 0x8004; out RESULT | 2: J:Route35NationalParkGate | needs porting |
| 681 | `BugContestEnd` | Contest end: restore saved party, clear state. RESULT 1/0 | out RESULT | 1: J:NationalParkBugContest | needs porting |
| 682 | `BugContestGetCaughtCount` | RESULT = number of contest catches (state[6]) | out RESULT | 2: J:NationalParkBugContest | needs porting |
| 683 | `BugContestJudge` | Judge: score each catch (0x082B09E4), rank player + NPCs (tables at 0x0203F10C...) | out RESULT | 1: J:NationalParkBugContest | needs porting |
| 684 | `BugContestBufferPlaceWinner` | Buffer contestant name / species / score for place VAR_0x8004 (1-4) into STR_VAR_1..3 ("SHINY RADIANT ") | in 0x8004; out RESULT | 4: J:NationalParkBugContest | needs porting |
| 685 | `BugContestGetPlayerPlace` | RESULT = player place (state[8]) | out RESULT | 1: J:NationalParkBugContest | needs porting |
| 686 | `BugContestGetFlag3` | RESULT = state[3] (contest variant flag set from VAR_0x8004) | out RESULT | 1: J:NationalParkBugContest | needs porting |
| 687 | `BugContestGetFlag2` | RESULT = state[2] | out RESULT | 1: J:NationalParkBugContest | needs porting |
| 688 | `BugContestReleaseCaught` | Nuzlocke "exhibition run": releases the caught contest mons | out RESULT | 1: J:NationalParkBugContest | needs porting |
| 788 | `BugContestClearFlag1` | Bug contest: clear state[1] | - | 4: J:NationalParkBugContest | needs porting |
| 789 | `BugContestCheckState` | Bug contest: RESULT=1 if not active or 0x082B07FC | out RESULT | 1: J:NationalParkBugContest | needs porting |
| 790 | `BugContestChooseFromBag` | Bug contest: choose item from bag (0x08228CE8(0,0,...)); waitstate | out VAR_ITEM_ID | 3: J:NationalParkBugContest | needs porting |

### Region, badges, dex, travel, League

| # | Name | What it does | Vars | Used (count: examples) | Kai |
|---|---|---|---|---|---|
| 670 | `UseAltGymLeaderTeam` | RESULT=1 to use the alternate (rematch/hard) gym leader team: MPlayer session check (0x082625CC/0x08263D30) or (sb2[0x2EF]&0x18)==8. GUESS: difficulty option | out RESULT | 26: J:CianwoodGym, J:BlackthornGym1F, J:WillsRoom | Kai difficulty (COOP_NOTES "difficulty") |
| 671 | `HasBadgeForFieldMove` | RESULT = player has the badge for field move VAR_0x8004 in the current region (per-region table 0x082A0FC0; Kanto id 0xF needs flag 0x214E) | in 0x8004; out RESULT | 8: scripts/johto_shared.inc, scripts/kanto_shared.inc, K:SixIsland_RuinValley | Kai has HMs without restrictions (mp_heldhm.c) |
| 718 | `IsRegionalDexComplete` | RESULT = all species flagged in the current region's dex (0x082A135C(0xFE): species 1..0x40A vs per-region caught bits). GUESS: regional dex complete (Celadon game designer diploma) | out RESULT | 2: K:CeladonCity_Condominiums_3F, K:PalletTown_ProfessorOaksLab | needs porting |
| 719 | `IsNationalPokedexEnabled` | Stock fn IsNationalPokedexEnabled (0x080E4310: var 0x4046==0x302 && flag 0xAAE) | out RESULT | 4: K:PalletTown_ProfessorOaksLab, K:PokemonLeague_HallOfFame, K:PalletTown | `IsNationalPokedexEnabled()` |
| 731 | `GetPokedexCount` | FRLG `GetPokedexCount`: 0x8004 0=Kanto else National -> 0x8005 seen, 0x8006 caught; RESULT=IsNationalPokedexEnabled | in 0x8004; out 0x8005, 0x8006, RESULT | 8: K:PalletTown_ProfessorOaksLab, K:Route2_EastBuilding, K:Route15_WestEntrance_2F | needs porting |
| 751 | `SetSeenMon` | FRLG `SetSeenMon`: dex seen for species VAR_0x8004 | in 0x8004 | 13: K:FuchsiaCity, K:Route25_SeaCottage, K:Route15_WestEntrance_2F | `setseenmon`/GetSetPokedexFlag |
| 772 | `VictoryRoadGateCheck` | Victory Road gate: RESULT=1 if region 1 and 2 cleared and no special story active | out RESULT | 2: J:VictoryRoadGate, K:Route22_NorthEntrance | needs porting |
| 779 | `IsNuzlockeActiveInRegion` | RESULT = Nuzlocke active in this region (0x08291728: sb2+0x2D1 bit6, region flag, region not cleared) | out RESULT | 7: J:NationalParkBugContest, J:Route35NationalParkGate, J:VioletPokecenter1F | needs porting |
| 782 | `GetReachableRegionsMask` | RESULT = bitmask of regions the player can sail to from this port (table 0x095C1C88; region cleared = sb1+0x3762 bit) | out RESULT | 2: K:VermilionCity, scripts/shared.inc | needs porting |
| 783 | `TravelToRegion` | Travel to region VAR_0x8004 (0 Hoenn, 1 Johto, 2 Kanto) if allowed (0x082B5A14); waitstate | in 0x8004; out RESULT | 1: scripts/shared.inc | needs porting |
| 791 | `GetRegionBadgeCount` | RESULT = badges in the current region (8 flags per region, 0x082A0D38) | out RESULT | 1: J:MountMortarB1F | needs porting |
| 712 | `SaveLeagueVictoryRecord` | League victory record + save; RESULT=1 ok (else "The League could not be recorded"); handles region change vs sb1+0x3761 | out RESULT | 1: scripts/shared.inc | Kai `GameClear`/HoF save |
| 723 | `DoCredits` | Credits (SetMainCallback2 0x081F9B64); waitstate | - | 1: K:IndigoPlateau_Exterior | Kai credits (`DoCredits`-like FRLG) |
| 787 | `AfterCreditsReset` | After credits: heal, reset state, soft reset or 0x081FD318(0); waitstate | - | 1: scripts/johto_shared.inc | needs porting |

### FRLG ports (Kanto mechanics)

| # | Name | What it does | Vars | Used (count: examples) | Kai |
|---|---|---|---|---|---|
| 672 | `DoPokemonLeagueLightingEffect` | FRLG `DoPokemonLeagueLightingEffect` (E4 rooms; palette task, flag 3 / map check 0x0F23, 0x4F24) | in 0x8004 | 6: J:WillsRoom, J:KarensRoom, J:BrunosRoom | port from pokefirered |
| 724 | `DoSSAnneDepartureCutscene` | SS Anne departure cutscene (task 0x082A2614); waitstate | - | 1: K:SSAnne_Exterior | port FRLG `DoSSAnneDepartureCutscene` |
| 725 | `DoSeagallopFerryScene` | FRLG `DoSeagallopFerryScene`: warp by VAR_0x8006 (1-10) to Vermilion/One..Seven Island/Navel/Birth Island (groups 0x25-0x27) | in 0x8006 | 1: scripts/kanto_shared.inc | needs porting |
| 726 | `PartyHasSpecies` | FRLG `DoesPlayerPartyContainSpecies`: RESULT=1 if party has species VAR_0x8004 | in 0x8004; out RESULT | 2: K:Route12_FishingHouse, K:SixIsland_WaterPath_House1 | `checkpartymon` |
| 727 | `ShowSeagallopDestinationMenu` | FRLG Seagallop destination ListMenu (table 0x095BA8FC; vars 0x5117, 0x510A; flag 0x2108); waitstate | in 0x8004; out RESULT | 1: scripts/kanto_shared.inc | needs porting |
| 728 | `ForcePlayerOntoBike` | FRLG `ForcePlayerOntoBike` (Mach/Acro by sb2+0x2C7 bit2) + cycling music 0x193 | - | 2: K:Route16, K:Route18 | needs porting |
| 729 | `ForcePlayerToStartSurfing` | FRLG `ForcePlayerToStartSurfing` | - | 1: K:SeafoamIslands_B4F | needs porting |
| 730 | `GetElevatorFloor` | FRLG `GetElevatorFloor`: var 0x5104 = floor from dynamic warp (sb1+0x484) map | out var 0x5104 | 1: scripts/kanto_shared.inc | needs porting |
| 733 | `GetSeagallopNumber` | FRLG `GetSeagallopNumber` from VAR_0x8004/0x8006 | in 0x8004, 0x8006; out RESULT | 1: scripts/kanto_shared.inc | needs porting |
| 734 | `GetSeagallopMenuSelection` | RESULT (menu pick) -> destination, 127 if cancelled or same as 0x8004 | in RESULT, 0x8004 | 1: scripts/kanto_shared.inc | needs porting |
| 735 | `GetRivalStarterSpecies` | RESULT = rival starter species: var 0x5146 if set, else by var 0x5143 (1 -> 7, 2 -> 4, else 1) | out RESULT | 1: K:PokemonLeague_ChampionsRoom | needs porting |
| 737 | `QuestLogNop` | No-op (bx lr). Scripts set VAR_0x8004/0x8005 first: leftover FRLG Quest Log hooks | - | 132: K:PalletTown_ProfessorOaksLab, K:VermilionCity_Gym, K:CeladonCity_Gym | drop |
| 738 | `GetElevatorFloorCursor` | Elevator: RESULT = cursor index for the current floor (from dynamic warp) | in 0x8004; out RESULT | 4: K:RocketHideout_Elevator, K:TrainerTower_Elevator, K:SilphCo_Elevator | needs porting |
| 739 | `IsPlayerLeftOfVermilionSailor` | FRLG `IsPlayerLeftOfVermilionSailor` (map 0x25.0x3E, x <= 23) | out RESULT | 1: scripts/kanto_shared.inc | needs porting |
| 742 | `ListMenu` | FRLG `ListMenu`: VAR_0x8004 list (0, 1 elevator floors, 5) tables 0x095BA774/0x095BA7BC/0x095BA81C; waitstate | in 0x8004; out RESULT | 2: K:SilphCo_Elevator, K:CeruleanCity_House1 | needs porting |
| 747 | `ReturnToListMenu` | FRLG `ReturnToListMenu` | - | 1: K:CeruleanCity_House1 | needs porting |
| 748 | `SampleResortGorgeousMonAndReward` | FRLG `SampleResortGorgeousMonAndReward`: var 0x5141 species (random seen species), 0x5142 reward item, flag 0x2EB3; buffers species name | out RESULT | 2: K:FiveIsland_ResortGorgeous_House | needs porting |
| 749 | `SeafoamCurrentEffect` | 0x080CE5EC(2): Seafoam B4F current. GUESS: FRLG `SeafoamIslandsB4F_CurrentDumpsPlayerOnLand` | - | 1: K:SeafoamIslands_B4F | needs porting |
| 750 | `ClearSeagallopRestrictions` | FlagClear 0x2109, 0x2108 (Seagallop menu gates) | - | 1: K:OneIsland_PokemonCenter_1F | flags |
| 752 | `SetVermilionTrashCans` | FRLG `SetVermilionTrashCans`: random cans -> VAR_0x8004, 0x8005 | out 0x8004, 0x8005 | 2: K:VermilionCity_Gym | needs porting |
| 754 | `SetIcefallCaveCrackedIceMetatiles` | FRLG `SetIcefallCaveCrackedIceMetatiles` (var 0x4001, table 0x0911453C) | - | 1: K:FourIsland_IcefallCave_1F | needs porting |
| 774 | `UpdateLoreleiDollCollection` | FRLG `UpdateLoreleiDollCollection` (table 0x095BA87C) | - | 1: K:FourIsland_LoreleisHouse | needs porting |
| 775 | `OpenMuseumFossilPicKabutops` | FRLG `OpenMuseumFossilPic` Kabutops -> VAR_0x8007 | out 0x8007 (via specialvar) | 1: K:PewterCity_Museum_1F | needs porting |
| 776 | `OpenMuseumFossilPicAerodactyl` | Same, Aerodactyl | out 0x8007 | 1: K:PewterCity_Museum_1F | needs porting |
| 777 | `CloseMuseumFossilPic` | FRLG `CloseMuseumFossilPic` (sprite VAR_0x8007) | in 0x8007 | 2: K:PewterCity_Museum_1F | needs porting |
| 784 | `AnimateTeleporterHousing` | FRLG `AnimateTeleporterHousing` (Bill) | in 0x8004 | 2: K:Route25_SeaCottage | needs porting |
| 785 | `AnimateTeleporterCable` | FRLG `AnimateTeleporterCable` | - | 1: K:Route25_SeaCottage | needs porting |
| 786 | `PartyHasRequestedResortMon` | RESULT=1 if party has the Resort Gorgeous requested species (var 0x5141) | out RESULT | 1: K:FiveIsland_ResortGorgeous_House | needs porting |

### Ruins of Alph

| # | Name | What it does | Vars | Used (count: examples) | Kai |
|---|---|---|---|---|---|
| 675 | `IsLeadMonHoOh` | RESULT = lead mon (non-egg) is Ho-Oh (national dex check). Ruins of Alph chamber walls | out RESULT | 1: J:RuinsOfAlphKabutoChamber | needs porting |
| 676 | `IsLeadMonAerodactyl` | RESULT = lead mon (non-egg) is Aerodactyl (national dex check). Ruins of Alph chamber walls | out RESULT | 1: J:RuinsOfAlphKabutoChamber | needs porting |
| 677 | `IsLeadMonKabuto` | RESULT = lead mon (non-egg) is Kabuto (national dex check). Ruins of Alph chamber walls | out RESULT | 1: J:RuinsOfAlphKabutoChamber | needs porting |
| 678 | `IsLeadMonOmanyte` | RESULT = lead mon (non-egg) is Omanyte (national dex check). Ruins of Alph chamber walls | out RESULT | 1: J:RuinsOfAlphKabutoChamber | needs porting |
| 679 | `StartRuinsOfAlphPuzzle` | Ruins of Alph sliding panel puzzle UI (SetMainCallback2 0x082AF964, returns to script); puzzle id VAR_0x8004; RESULT=1 solved; waitstate | in 0x8004, 0x8005; out RESULT | 8: J:RuinsOfAlphKabutoChamber | needs porting (FRLG/HGSS puzzle) |

### Giovanni / Celebi alternate past

| # | Name | What it does | Vars | Used (count: examples) | Kai |
|---|---|---|---|---|---|
| 693 | `GiovanniStoryUpdateFlags` | Giovanni story ON_TRANSITION (Tohjo Falls): sets/clears flags 0x10D4, 0x131E, 0x14A9, 0x131D, 0x1321 from flag 0x1318/0x1319 and var 0x506C | - | 1: J:TohjoFalls_Cave2 | needs porting |
| 694 | `GiovanniStoryCanStart` | Giovanni story: RESULT = can start (0x082A2EC4) | out RESULT | 1: J:TohjoFalls_Cave2 | needs porting |
| 695 | `GiovanniStoryStart` | Start Giovanni story: stores gender (var 0x506D), sb2+0x2C7 bit6 (0x506E), resets followers, var 0x506C=4/5, sets sb2 gender byte from var 0x5075 (0x12 = Giovanni) | - | 2: J:TohjoFalls_Cave2 | needs porting |
| 697 | `GiovanniStoryNewBarkHouseFlags` | New Bark House 1 flags 0x14A1/0x14A2, var 0x5076 | - | 1: J:NewBarkTown_PokemonCenter_1F1 | needs porting |
| 698 | `GiovanniStoryEnd` | End Giovanni story: clear 0x131C/0x1319, var 0x506C=4, 699, var 0x5076=3 | - | 1: J:RadioTower5F | needs porting |
| 699 | `GiovanniStoryEnterPast` | Enter the past: restore player (0x082A2F28) if in Giovanni mode, FlagSet 0x1318, var 0x5076=0 | - | 2: J:RadioTower5F, scripts/johto_shared.inc | needs porting |
| 700 | `GiovanniStoryItemAvailable` | Ruins Cave5 item available: Giovanni mode, flag 0x131A clear, 0x082A2FC0 | out RESULT | 1: J:RuinsOfAlph_Cave5 | needs porting |
| 701 | `GiovanniStoryItemTaken` | FlagClear 0x131A, 0x131D | - | 1: J:RuinsOfAlph_Cave5 | needs porting |
| 702 | `GiovanniStoryElmCheck` | Elm lab check: flag 0x131A set, 0x131B clear, not Giovanni mode, cheat bit clear | out RESULT | 1: J:ElmsLab | needs porting |
| 703 | `GiovanniStoryElmDone` | FlagClear 0x131B, 0x131F | - | 1: J:ElmsLab | needs porting |
| 792 | `GiovanniStoryTohjoSignCheck` | Tohjo Falls sign: RESULT=1 if Giovanni story reachable | out RESULT | 2: J:TohjoFalls_Cave2 | needs porting |
| 793 | `ClearRadioTowerFlag14A3` | FlagClear 0x14A3 | - | 1: J:RadioTower1F | flag |
| 794 | `SetGiovanniModeFlag14A4` | Flag 0x14A4 = Giovanni mode | - | 4: J:GoldenrodCity, J:Route35, J:CherrygroveCity | flag |

### Celebi time travel (Bill's family house)

| # | Name | What it does | Vars | Used (count: examples) | Kai |
|---|---|---|---|---|---|
| 798 | `TimeTravelEventEligibility` | Bill's family house Celebi time-travel event eligibility (0x082A432C) | out RESULT | 4: J:BillsFamilysHouse | needs porting |
| 799 | `TimeTravelEventStart` | Start time travel: var 0x5077, flags 0x14AB/0x14B8, save party; RESULT=1 | out RESULT | 1: J:BillsFamilysHouse | needs porting |
| 802 | `TimeTravelSetFlag14AB` | FlagSet 0x14AB if time-travel active | - | 1: scripts/johto_shared.inc | needs porting |
| 806 | `TimeTravelMapMetatiles` | Time-travel map: rewrites metatiles by var 0x5077; RESULT | out RESULT | 1: scripts/johto_shared.inc | needs porting |
| 807 | `TimeTravelHouseFrameCheck` | Bill's house ON_FRAME (var 0x5077 == 6): sets field callback for cutscenes | - | 2: J:BillsFamilysHouse, scripts/johto_shared.inc | needs porting |
| 824 | `TimeTravelGoldenrodCheck` | Goldenrod: VAR 0x4002 = 1 if time-travel ready (flags 0x14B2/0x14B3, var 0x5077 == 0) | - | 2: J:GoldenrodCity | needs porting |
| 825 | `TimeTravelHouseProgress` | Bill's house: var 0x5077 0 -> 1, flag 0x14B4 | - | 1: J:BillsFamilysHouse | needs porting |
| 826 | `TimeTravelHouseStatus` | Bill's house status RESULT 0..5 | out RESULT | 3: J:BillsFamilysHouse | needs porting |
| 827 | `TimeTravelChoosePartyMon` | Pick party mon for the event (0x082A47C0); RESULT, VAR_0x8004 slot | out RESULT, 0x8004 | 1: J:BillsFamilysHouse | needs porting |
| 828 | `TimeTravelHandOverPartyMon` | Hand over party mon VAR_0x8004 (held item to bag, delete, follower update, flags 0x14B2-0x14B4) | in 0x8004; out RESULT | 1: J:BillsFamilysHouse | needs porting |

### Dream (Darkrai/Cresselia) and other postgame events

| # | Name | What it does | Vars | Used (count: examples) | Kai |
|---|---|---|---|---|---|
| 585 | `DreamEventEligibility` | New Bark House 2 bed: eligibility for the Darkrai/Cresselia dream event (map 0x23.0x02, flag 0x1318 off, owns Darkrai 0x1EB -> 2, Cresselia 0x1E8 -> 1) | out RESULT | 1: J:NewBarkTown_PokemonCenter_1F2 | needs porting |
| 586 | `StartDreamEvent` | Start dream event: save party/followers (0x08275770, 0x0826F898, 572), HealPlayerPartyEx, 0x0810B3EC(1), FlagClear 0x1425 | - | 2: J:NewBarkTown_PokemonCenter_1F2 | needs porting |
| 587 | `EndDreamEvent` | End dream event: FlagSet 0x1425, 0x0810B3EC(0), FlagClear 0x1321 | - | 1: J:DarkCaveBlackthornEntrance | needs porting |
| 759 | `SevaultHouseEventEligibility` | Sevault Canyon house event eligibility (map 0x21.0x27, flag 0x2924) | out RESULT | 1: K:SevenIsland_SevaultCanyon_House | needs porting |
| 760 | `SSAnneDreamEventEligibility` | SS Anne 1F room 6 Darkrai/Cresselia dream event eligibility | out RESULT | 2: K:SSAnne_1F_Room6 | needs porting |
| 761 | `SSAnneDreamEventStart` | Start it: flags 0x292C/0x292B, save party, heal | - | 1: K:SSAnne_1F_Room6 | needs porting |
| 762 | `SSAnneDreamEventEnd` | End it: flags 0x292B/0x2928/0x292D/0x292C | - | 1: K:SSAnne_1F_Room6 | needs porting |
| 763 | `SSAnneRoom6OnResume` | Room 6 ON_RESUME: flag 0x292D, refresh 757, field callback | - | 1: K:SSAnne_1F_Room6 | needs porting |
| 764 | `SSAnneExteriorEventCheck` | SS Anne exterior event check (0x082AE108) | out RESULT | 1: K:SSAnne_Exterior | needs porting |
| 765 | `SSAnneExteriorEventInit` | SS Anne exterior event init | - | 1: K:SSAnne_Exterior | needs porting |
| 766 | `SSAnneExteriorEventStep` | SS Anne exterior event step VAR_0x8004 (1-2) | in 0x8004 | 2: K:SSAnne_Exterior | needs porting |
| 768 | `PalletPostgameEventEligibility` | Pallet house 2F postgame event eligibility (all party Lv100, map 0x25.0x78, flag 0x2E13) | out RESULT | 1: K:PalletTown_PlayersHouse_2F | needs porting |
| 770 | `JohtoTravelingEventCheck` | Johto city ON_TRANSITION "traveling event here?": vars 0x5072 (1-12), 0x5073 (0x3E0-0x3E8), map in table 0x09245724 -> flag 0x1490; RESULT | out RESULT | 15: scripts/johto_shared.inc, J:CianwoodCity, J:BlackthornCity | needs porting |
| 773 | `KantoTravelingEventCheck` | Kanto version of 770 (vars 0x5149/0x514A, table 0x0924573C, flag 0x2EAB) | out RESULT | 20: scripts/kanto_shared.inc, K:CeladonCity, K:VermilionCity | needs porting |
| 781 | `BufferLabQuestChecklist` | Cinnabar lab checklist of Quetzal events (table 0x095BBDEC, " : Completed"); 0x8004 page; 0x8006 all done | in/out 0x8004, out 0x8006, RESULT | 2: K:CinnabarIsland_PokemonLab_Entrance | needs porting |

### Misc

| # | Name | What it does | Vars | Used (count: examples) | Kai |
|---|---|---|---|---|---|
| 552 | `IsCurrentPcBoxFull` | RESULT=1 if all 30 slots of the box returned by 0x08135934 (current box) are filled (GUESS: PC box full) | out RESULT | 3: J:VioletPokecenter1F, J:Route34_Mart1 | `ScriptCheckFreePokemonStorageSpace`-like |
| 557 | `ReloadMapAtPlayerPos` | Re-warps to the current map at the player position (SetWarpDestination(cur map,-1,x,y), DoWarp), FlagClear 0xB78. With waitstate | - | 2: scripts/kanto_shared.inc, scripts/johto_shared.inc | `warp` to self |
| 572 | `ResetFollowerObjects` | For party slots 0-5 refreshes follower/object state (0x082655E0, 0x080D001C), resets 0x02033CA1[i]=0x18. GUESS: reset followers before a cutscene | - | 9: J:RadioTower5F, K:CinnabarIsland_Extra1, K:PalletTown_PlayersHouse_2F | Kai followers (src/mp/mp_follower.c) |
| 584 | `HoennPostHealEventCheck` | After the heal fanfare, Hoenn only: if flags 0x29D/0x29C and not 0x2A2/0x2A3 and party condition (5+ mons, statuses...) runs a script. GUESS | - | 1: scripts/johto_shared.inc | needs porting |
| 732 | `ReturnZero` | Stub: returns 0 (scripts test RESULT==2 -> refuse). Leftover link check | out RESULT | 60: scripts/kanto_shared.inc, K:PowerPlant, K:CeladonCity_GameCorner_PrizeRoom | drop |
| 778 | `IncrementChallengeCounterVar` | Increments a counter var (id from 0x08291854) when a challenge option is on | - | 1: scripts/shared.inc | needs porting |
| 796 | `PickNpcFlavourLine` | NPC flavour line picker: RESULT=(localId+x+y)&7, +8..11 by metatile behaviour, +12/+24 by story state; 0x8004 = entry of table 0x095BA93C; 0x8005/0x8006 | out RESULT, 0x8004-0x8006 | 1: scripts/johto_shared.inc | needs porting |
| 797 | `ObjectFaceObject` | Make object VAR_0x8004 face object VAR_0x8005 (0xFF = player) and vice versa | in 0x8004, 0x8005 | 21: J:RadioTower5F, J:NewBarkTown_PokemonCenter_1F1, J:ElmsLab | `applymovement` face_* |
| 829 | `IsSaveFlag2F4Bit4Set` | RESULT = sb2[0x2F4] bit 4. GUESS: "cheat mode" save setting | out RESULT | 3: K:CinnabarIsland_PokemonLab_Entrance, J:EmbeddedTower_Cave1, scripts/johto_shared.inc | needs porting |
