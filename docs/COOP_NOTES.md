# Co-op feature notes

Moved out of CLAUDE.md: per-feature details (files, functions, debug commands, test recipes).
Read the part about a feature before changing it.

Emote / phrase pick (mp_menu.c MP_OpenQuickPick, the only emote screen; mp_battlechat.c in battle): LT/RT or Q/E held = held mode (release sends),
SOCIAL > EMOTES = sticky (A sends, B closes); on a pad emotes only come from the LT/RT hold menu (no right stick). Pages EMOTES, MINE 1-4 (16 phrases, MP_NUM_PHRASES,
ini phrase1-16; MINE n shows once page n-1 has one) + 4 stock groups (L/R); SELECT (pad west) swaps/pins
gMpConfig.quickStock (first MINE page only, ini quick_stock), START (pad north) edits MINE (INPUT_CONTEXT_QUICKPICK).
Test with `vpad xbox`, `vaxis rt 100`, `vbtn right 1`. Button icons (mp_buttons.c, hooked into text.c DrawKeypadIcon): CHAR_KEYPAD_ICON chars draw the last used
controller's buttons (gamepad.c Gamepad_IconFamily; art graphics/moemon/buttons/buttons.png -> tools/moemon_buttons/build.py
-> src/mp/mp_buttons_data.h). Co-op ASCII text writes them as {A} {B} {L} {R} {START} {SELECT} {LEFTRIGHT} {LT} {WEST}...
(MP_AsciiToGame); extra ids CHAR_MP_LT..CHAR_MP_LS; {COOP} = "the C key" or the pad's north button. Test with `vpad xbox|ps|nintendo`
(Nintendo pads: confirm is `vbtn east`; harness `press` doesn't switch icons back to the keyboard, real keys do).
Vanilla scenes patched to draw them (keyboard keeps the GBA art): summary A/B, bag SELECT tag, easy chat START/SELECT tags
+ side window (easy_chat.c MpLoadButtonWindow), naming screen side-button labels (naming_screen.c MpLoadButtonLabels,
which also shows the LOOK sprite, OBJ_EVENT_GFX_MP_LOCAL). Co-op help lines (mp_ui.c WrapText) keep an icon with its
label and redraw on a device switch (MpUi_RefreshIcons).
New game LOOK step (mp_intro.c CB2_MpIntroLook, main_menu.c Task_NewGameBirchSpeech_LetMeSeeYou): after the gender
pick the LOOK page opens in intro mode (mp_menu.c MP_OpenLookForIntro) on Birch's backdrop; harness condition `introlook`.
SKIP THE INTRO (main_menu.c Task_NewGameBirchSpeech_AskSkip -> CB2_ChooseStarter -> nickname -> Birch send-off;
mp_quickstart.c MP_QuickStart_Apply at the end of NewGameInitData): the end state of truck..Route 103 rival..running shoes
for both genders, starter at the level the two intro battles give (met Lv. 5 on ROUTE 101). It still starts in the truck:
FLAG_MP_SKIP_TRUCK makes InsideOfTruck's door lead out by your house and LittlerootTown hop you off it. Harness condition `naming`.
Welcome note (mp_setup.c HOST_NOTE, ini `welcome = a|b`, mp_main.c SendWelcome/ShowWelcome, MPR_WELCOME): the room
creator sends it on a new player's info; shown via MP_ShowNotice once on the map. Harness condition `notice`.
Emote icons (trainer_see.c MpIconLook): !, ?, heart and the music note (graphics/field_effects/pics/emotion_note.png,
MP_StartNoteIcon; MPR_SAY SAY_NOTE) pop as field-effect icons drawn on the co-op overlay palette (white 1, black 14,
pink 15 of mp_emote.c sBubblePal), not the player/NPC slots co-op looks recolor; a friend's name tag hides under one.
Pings (mp_ping.c, MPR_PING, INPUT_ACTION_PING: V / left stick click): bubble slot PING_SLOT in mp_emote.c, blinking inset
edge arrow via mp_compass.c MP_Compass_PointAt. Test: `vbtn lstick 1`, run2.sh WARP_B for a second start position.
Lend field moves (mp_lend.c, MPR_LEND): each client broadcasts the first party Moemon knowing CUT/SURF/STRENGTH/
ROCK SMASH/WATERFALL/DIVE; scrcmd.c checkpartymove falls back to the nearest friend within 4 tiles and returns MP_LEND_SLOT
(7), which bufferpartymonnick, FldEff_FieldMoveShowMonInit, FldEff_UseStrength and PartyHasMonWithSurf understand.
Test: Route 116 cut tree, WARP=0,31,23,7 (lender) WARP_B=0,31,21,7, `mp teach 0 15` on the lender, `mp givebadges`, `mp lend`.
Rescue (mp_rescue.c, MPR_RESCUE): battle_setup.c asks MP_Rescue_TryStart() before CB2_WhiteOut (not link/co-op/frontier);
the downed player returns to the field (script context cleared) with a DOWN! window, the nearest free friend within 10
tiles gets HELP? (Revive = lead at half HP / Max Revive = full HP / pay to heal the team: PatchUpPrice ($150 per mon + $4 per
missing HP) x (1 + badges/2), sent in the RS_ASK payload, RESCUE_NO_PATCH in MP_InDungeon / no). The downed player can use
their own Revive (A, RS_SELF). MP_InDungeon (mp_main.c: MAP_TYPE_UNDERGROUND + dungeon map sections) also blocks the shared PC.
Record mixing buffers (mp_linkx.c) are calloc'd, not gHeap: holding ~27 KB of gHeap crashed battles (move anim gfx). `mp party` prints HP. Harness conditions `down`, `helpask`; debug `mp hp SLOT V`, `mp item 24 2`.
TEAM PHOTO (mp_photo.c, SOCIAL > TEAM PHOTO, MPR_PHOTO): 2+ players on the same map, front pics via
mp_avatar.c MP_Avatar_PlayerFront, frame captured from Platform_GetFrame and saved 3x with mp_pack.c MP_WritePng to
photos/. Debug `mp photo`, harness condition `photo`. Co-op sprite palettes (friends 0x5390+, followers 0x1150+, overlay
MP_TAG_UI_PAL) go through sprite.c MP_AllocCoopPalette: NPC reflection slots 6-9/11 first, then free slots 12-15, which the
game needs for weather and field effects (tall grass, PC heal balls): LoadSpritePalette/AllocSpritePalette move or evict a
co-op palette (MP_MakeRoomForPalette) and PatchObjectPalette moves one out of a reflection slot (MP_VacatePaletteSlot).
Debug `mp pals` (slot owners), `mp palfill N`, `mp palvacate SLOT`, `mp palcoop TAG`. The overlay palette skips weather tinting.
Joined frames (mp_ui.c MP_FrameJoin_*): co-op windows that touch get joint tiles at BG0 tiles 0x2F0-0x2FE.
Piggyback (mp_piggy.c, A facing a friend, B hops off): the rider copies each step of the carrier's figure on its own screen
(same movement action, so the same speed, and over map connections), warps to the carrier's own map coords only when they
left the map or got 3+ tiles ahead, and sends the carrier's speed as its own; the carrier draws a rider on its back at its own
sprite position. MPF_RIDING is recomputed every state (it used to stick and leave riders drawn 12 px up). Test: run2.sh
WARP=0,16,10,3 WARP_B=0,16,11,3, B faces A (LEFT), A + mash A, A runs B+UP into Oldale, `mp objs` (also dumps followers).
Crash report (src/platform/crash_log.c, installed first thing in main): Windows SEH filter / Linux signals append to
crash_log.txt: exception, exe+offset of the faulting address, callbacks, map, and exe+offsets found on the stack (resolve with
x86_64-w64-mingw32-nm on the same exe; keep a copy of each shipped exe). Debug `mp crash` forces one.
Summary info page: "{SELECT} RENAME" prompt in the dark strip under the portrait (pokemon_summary_screen.c
MpDrawInfoPrompts, window baseBlock 700) when MpCanRename(). The font has no {PKMN} glyph pair: strings.c uses "MON".
POKé BALL page (5th summary page, PSS_PAGE_BALL, MpBall_* in pokemon_summary_screen.c; LOCK/SELECT_MOVE modes stop at page 4):
A edits TYPE (own ball + balls in the bag) / COLOR (BALL_POKE only, 11 colors) / DECOR (BALL_POKE only, 68 emblems:
shapes, the 17 types, letters, numbers; L/R jump between groups); another type costs one ball after a YES/NO.
Decorations (tools/moemon_balls/decorations.py -> src/mp/mp_balldeco_data.h; `--preview OUT` renders them on the real art)
are drawn into the game's own pixels: 5x5 on the item icon at (7,7) (palette slots 12-15, MP_BallStyle_DecorateIcon) and
3x4 on the ball's first frame at (6,3) (ball palette slots 9-15) via a per-decoration copy of the POKé BALL sheet
(TAG_MP_BALL_DECO + n, sprite->sheetTileStart moved to it; unused ones freed). Letters bend in at the top (the dome).
Debug `mp style SLOT HEX` (e.g. 120 = WATER). Style lives in BoxPokemon.unknown
(MON_DATA_ENCRYPT_SEPARATOR, mp_ballstyle.c: bits 0-3 color, 4-11 decoration), applied to the send-out ball (pokeball.c),
the summary ball and the item icon preview. Blocked in battle, links and on MAPSEC_BATTLE_FRONTIER (rentals).
Special balls (MoeGatcha BALL banner only; ITEM_LOVE/FRIEND/FAST/LEVEL/LURE/HEAVY/MOON/DREAM/HEAL/DUSK_BALL = old ITEM_035-03E,
ITEM_QUICK_BALL = ITEM_048; icons graphics/items/icons/moemon_*_ball.png from the user's art via tools/moemon_balls/convert.py IN_DIR . (the art isn't in the repo),
24x24 crop + 15 colors): a catch stores MON_DATA_POKEBALL = POKé BALL (other games / 3D games see a POKé BALL) and the
special index in style bits 12-15 (mp_ballstyle.c MP_Ball_SetCaught; FRIEND sets friendship 200, HEAL heals). Shown via
MP_BallStyle_DisplayItem (summary) and the POKé BALL sprite recolored with sSpecialRamps (thrown: battle_anim_throw.c,
send-out: pokeball.c). Catch bonuses: MP_SpecialBall_Multiplier from Cmd_handleballthrow's default case (x10 units;
HEAVY changes the catch rate by weight; LURE uses gMpFishingEncounter set in FishingWildEncounter, cleared in
CB2_EndWildBattle; DUSK = cave/underground or 20:00-5:59; QUICK = battleTurnCounter 0). MP_IsBallItem replaces the
FIRST_BALL..LAST_BALL checks; HandleAction_UseItem throws them with the POKé BALL script; catchAttempts only counts Hoenn
balls. Not customizable (MP_BallStyle_ItemCanColor/Decorate). `mp party` prints ball/style/friendship.
Battle starts: the intro slide (battle_intro.c) and the sprites sliding with it move INTRO_SLIDE_STEP px a frame (4 = double speed, battle.h; vanilla 2).
Co-op battles (link multi under the hood) look like single ones: normal transition (MP_GetWild/TrainerBattleTransition),
no VS screen, normal background and slide, no party bars for wild, wild mons drawn and slid in (no ball), one combined intro
line held 24 frames. Network link (gNetLinkActive): instead of the 100-frame start delay each game sends a READY (MPR_LINK_SYNC
kind 3, NetLink_SendReady) once BATTLE_TYPE_LINK_IN_BATTLE is set and the send task waits for the peer's (a block reaching a
game still in CB2_HandleStart* is wiped by its ResetBlockReceivedFlags). Queued blocks are sent/received per frame; the receive
task handles several messages a frame but stops after a B_COMM_TO_CONTROLLER one (a same-frame IS_DONE would clear the flag
before the local controller ran it). Queued blocks move into gBlockRecvBuffer in NetLink_GetBlockReceivedStatus, never in
ResetBlockReceivedFlag (the game reads the buffer right after clearing the flag). Own echo blocks queued (selfQueue).
GetMonData asked of all battlers at once, MP_FrameEnd flushes sends after each frame. These only broke over the internet:
test with tools/mp_server/lagproxy.py (TCP proxy adding delay + jitter: `lagproxy.py LISTEN TARGET 150 50`, clients use
MOEMON_LOBBY=127.0.0.1:LISTEN, FPS=60) and MOEMON_LINKDEBUG=1 (logs every block + link state each second). Verified at 150 ms.
The battle screen's opening window and grass overlay also move at INTRO_SLIDE_STEP.
Fast link sync (battle_controllers.c LinkSyncKind, MOEMON_LINK_STRICT=1 = vanilla): the master's engine waits on a controller
command only for its own screen (LINK_SYNC_FAST: messages, move anims, HP bars, status, SE, SETMONDATA...) or for its own +
the battler owner's (LINK_SYNC_OWNER: choices, EXP...); GETMONDATA, intro, ball throws, end = everyone (vanilla). Non-masters
play ahead-able commands one at a time (sLocalFastBusy) and free a battler when they send IS_DONE (their echo would queue
behind the next command); IS_DONE carries the finished command in data[1] so the master drops answers it didn't wait for.
PvP turn at 250 ms ping: 31 s -> 11 s. Tests: tools/test (README), `WORST=1 tools/test/regress.sh`.
Send-outs and the end fade run exactly 1.3x (battle_main.c MpBattleTurbo): on 3 of 10 frames BattleMainCB2 runs one more
AnimateSprites + UpdatePaletteFade + RunTasksExcept(IsBattleLinkTaskFunc) while pokeball.c MP_SendOutRunning (balls marked
in Task_DoPokeballSendOutAnim's SENDOUT cases, not catch throws) or gBattleOutcome && gPaletteFade.active. The link send /
receive tasks never run twice a frame (that would break the controller-command ordering); the battle engine and controllers
run once a frame as always. Wild intro -> action menu 916 -> 903 frames; co-op checked at 120-150 ms.
Passive effects between turns (B_ANIM_HELD_ITEM_EFFECT: Leftovers, berries, Shell Bell...; FOCUS_BAND, INGRAIN_HEAL,
WISH_HEAL, LEECH_SEED_DRAIN) run at 2x through the same extra step (battle_gfx_sfx_util.c MP_FastGeneralAnimRunning, rate
10 in MpBattleTurboRate), and General_HeldItemEffect wiggles once instead of three times: a Leftovers turn 852 -> 732 frames.
Debug `mp hold SLOT ITEM`.
Battle messages: holds after a message B_WAIT_TIME_LONG/MED/SHORT 40/30/20 (vanilla 64/48/32; data/*.s need a touch to
reassemble after changing them), A/B skips the rest of a hold after 8 frames outside link battles (Cmd_waitmessage),
single-battle text prints at twice the option speed (BattlePutTextOnWindow; link battles already use 1). A turn (attack +
enemy attack, MID text, no presses) 684 -> 476 frames.
Low HP beep (battle_gfx_sfx_util.c PlayLowHpBeep): SE_LOW_HEALTH loops every 36 frames; Task_StopLowHpBeep stops it after
100 (three "doo duh"s); it plays again when a Moemon newly drops into the red.
Co-op partner status (mp_coop.c MP_Coop_SetBattleStatus, MPR_COOP_BSTATUS): the player controller reports MP_BST_ACTION/
MOVE/BAG/PARTY/DONE; PrintLinkStandbyMsg shows "<partner> is selecting a move to use…" etc. (MP_Coop_ShowStandby) and
redraws on change until another B_WIN_MSG text replaces it (BattlePutTextOnWindow -> MP_Coop_OnBattleMessage). PvP keeps
"Link standby…" so the opponent can't see a switch coming.
Battle emotes (mp_battlechat.c): LT/RT or Q/E held in battle (gamepad.c/sdl2.c also fire in INPUT_CONTEXT_BATTLE;
input_context.c Platform_ContextAction -> MP_BattleChat_Open) opens a 128x64 sprite pick (2x 64x64, overlay palette) with
the quick pick's pages (mp_menu.c MP_Quick_*); main.c ReadKeys -> MP_BattleChat_FilterKeys hands it the buttons and the
battle gets none. Sending: mp_emote.c MP_Emote_BattleSay -> bubble over MP_BattleChat_LocalBattler's sprite + MPR_SAY kind
SAY_BATTLE {icon, text}; receivers show it over MP_BattleChat_BattlerOf(sender) (co-op partner / PvP opponent).
! and ? are text, heart / note are bubble pictures (MP_BUBBLE_ICON_*). No editing in battle.
Test: `vpad xbox`, `vaxis lt 100`, `vbtn up 1`, `vbtn up 0`, `vaxis lt 0` (bchat1/bchat2 style). Measured from encounter to action menu: solo wild 557 ->
497 frames, co-op wild double 947 -> 562. Harness prints "[test] action menu at frame N"; the `action` condition misses co-op.
Autosave (mp_autosave.c, MP_Autosave_Update from MP_FrameUpdate): every gMpConfig.autosaveMinutes (ini autosave, default 5)
of map time, 1 minute after a battle, once MP_IsLocalFree + standing still for 30 frames; skipped where the START menu has
no SAVE, during FLAG_MP_SKIP_TRUCK, and for a new game over an existing save (gDifferentSaveFile). Manual and MoeGatcha
saves call MP_Autosave_Saved. Log line "autosave: saved". Backups (sdl2.c BackUpSaveFile) rotate at most every 10 minutes.
Registered key items (item_menu.c): REGISTER opens a SELECT / L / R / extra chooser (ItemMenu_Register, ACTION_MP_REG_*);
L/R/extra live in the .coop file (reg:L, reg:R, reg:X, MP_REG_SLOTS), one button per item. The extra button is the pad's west
button / F key (INPUT_ACTION_ITEM_X -> gMpItemXPressed -> field_control_avatar.c). No auto-run any more; STICK RUN defaults off
and NewGameInitData turns it off. Exe icon: tools/app_icon/build.py writes DIB images up to 64 px + PNG 128/256, res.rc ID 1.
Follower Pokémon (SELECT in the party menu): local trailing, 64x64 sprites, talking to it,
and remote followers seen by the other client.
Phase 1 co-op: emotes/phrases (bubbles seen remotely), FRIENDS > GO TO, Town/Fly map heads, door
labels, shared Pokédex (+ CAUGHT BY), buddy bonus, co-op Roxanne win giving both players the badge
(test: MOEMON_DEBUG_WARP=11,3,5,3 / 11,3,5,4, MOEMON_DEBUG_PARTY=283:46, `mp badges`).
Phase 2: shared PC (put/take Pokémon and items, `MOEMON_STORE_DIR`), trade with money/items and a
trade evolution (Kadabra), daily gift + thank-you bonus. Battle Flag hides name tag and follower.
Dailies (mp_events.c): 3 goals a day from 3 pools (everyone G_TRAINERS..G_STRONG / co-op G_COOP..G_LEND / together
G_GIFTS..G_CARD; PickGoalsFull, solo fallbacks), hooks call MP_Events_Did(MP_DID_*) (goal + achievement counter). Pair goals
only count friends with 5+ minutes today ("h:NAME" minute counter). Pull caps: 5 daily pulls/week ("dailypulls:wN"),
weekly free +5 pulls (offline too), rematch pull once a day, tournament pulls once a day (3+ players, no forfeit), race needs
2+ racers, HoF pull once per friend. Achievements (mp_achieve.c, 70, personal, offline too: "av:ID", counters "acnt:NAME"),
DAILY > ACHIEVEMENTS. Achievement rewards: tier 0/1/2 = 3/10/20 pulls (mp_achieve.c AchPulls; saves from before get the
difference once, "ach:rewards2"). Offline, DAILY shows the day's 3 "everyone" goals (sSoloRows; PeakPlayers = 1), counted
on the save ("solo:DATE:S" via ContributeGoals/GoalNumbers), rewarded by CheckGoals ("sgoal:"/"sgoalall:" keys,
the same weekly pull cap); MP_Events_Update runs offline from MP_FrameUpdate. The outbreak runs offline too. RARE CANDY cap MP_RareCandyCap (party_menu.c). Save data (mp_savedata.c)
drops dated keys after 14 days and never evicts undated ones. Debug `mp goals` lists a week of goals.
Phase 3 (mp_events.c, mp_gatcha.c): goals/toasts/MoeGatcha/streak with `mp event ...`, `mp savedata`, `mp gatcharig N` (0 N .. 6 EX; 5 = legendary event),
`mp goals`, `mp pickup N` (0-18), harness `letters WORD`, MOEMON_DEBUG_DATE; `AllowSmallTweaks = 1` in the ini enables
the MoeGatcha typed codes (mp_gatcha.c CheckTypedCodes, off by default, only from the file) and typing "speed" anywhere
(5x until "slow"; text_input.c CheckTypedWords -> sdl2.c Platform_SetSpeedUp, harness `letters speed`); partner-by-partner co-op goal checked with three clients (run4-style).
Phase 4: co-op wild battles (mp_coop.c, `MP_Coop_TryWildTeamUp`): a land encounter pulls in the closest
free friend standing in grass within 8x6 tiles (no prompt), 2 players vs 1 wild Pokémon (the right
opponent slot starts absent). POKé BALLS work (only balls); the catch goes to the thrower via the
result message. Test: Route 101 grass, MOEMON_DEBUG_WARP=0,16,12,11 / 0,16,14,11, `mp wild 288:3`,
`mp item 1 2`, `waitfor action`, `mp party`. Verified: catch by the joining player, knock-out win
(EXP for both), friend out of range -> normal single battle. 2-on-2 (60%, gMpWildDoublePercent, debug `mp wilddouble N`): second mon from
wild_encounter.c MP_RollExtraLandMon on the right (enemy slot 3); a catch with the other still out runs
BattleScript_MpCoopBallCaughtGoOn (VARIOUS_MP_COOP_CAUGHT_ONE/REMOVE_CAUGHT). Only the link master runs battle scripts:
visuals for both screens must go through controller emits (sprite invisibility value 2 = caught, link partner ball throw). Also gym rematches and tournaments.
Difficulty: trainer/wild levels were vanilla Emerald; route/gym trainers got +2..+6 per story segment, gym aces 15/20/27/33/36/41/47/51,
E4 53-59, Wallace 62, land wild levels +3/+4 on under-leveled routes and dungeons, route trainers from badge 3 AI TRY_TO_FAINT
(scripted from the audit; see git history). Co-op team-ups (master only): MP_Coop_PrepareTrainerParty sets gMpEnemyLevelBonus
(+2, +3 for MP_Coop_IsBossTrainer) used by battle_main.c CreateNPCTrainerParty; MP_Coop_FinishTrainerParty pads the party
(route n + (n+1)/2, bosses 6) before the ace with PickFillerSpecies (shares a type with a party mon, base stats within
15% (25% fallback), no legendaries/babies/Shedinja/Ditto, not already in the party, seeded by trainer+slot); MP_Coop_TrainerBattleBoost re-enables trainer items and smarter AI flags
(battle_ai_script_commands.c); co-op wild mons +2 levels (BoostWild), gMpWildDoublePercent 60.

Wait for a partner (mp_coop.c Task_JoinCheck JOIN_WAIT_*): after a trainer's speech with nobody asking yet, a friend on
the same map and a 2+ Moemon trainer -> "Wait for another trainer to join you?"; YES = waiting (B = single battle), a
"Join me!" bubble (mp_emote.c MP_Emote_KeepBubbleFor) and MPR_COOP_WAIT {on, localId, mapNum, mapGroup} every 60 frames;
other games hold that NPC (MP_Coop_IsNpcHeld in trainer_see.c and field_control_avatar.c). A friend who walks up and faces
you (TryAsk) is accepted at once. Test: WARP=11,3,5,3 WARP_B=11,3,5,6, party 283:46, host UP A + mash A, joiner UP UP.

Co-op healing items: the bag allows ItemUseInBattle_Medicine items; party_menu.c MpCoopItemUseCB_Medicine only checks the
item on a copy, takes it from the bag and stores the target (gMpCoopItemSlot = GetPartyIdFromBattleSlot, any of the six:
the multi layout's gold slots are the partner's). CompleteWhenChoseItem packs it in bits 12-14 of the item; the master's
HandleAction_UseItem runs BattleScript_MpCoopUseItem (VARIOUS_MP_COOP_USE_ITEM works it out on a copy: active -> health bar
+ datahpupdate, benched -> SetMonData with monToCheck; VARIOUS_MP_COOP_ITEM_STATUS; result text from gMpCoopItemStringIds,
STRINGID_MPCOOP_ITEM_* 390-393). Test: heal2 style, `mp hp 1 3` on one, `mp item 13 3` on the other, `mp party`.

Co-op doubles retrofit (docs/TRIPLES_DECISIONS.md "Doubles first", protocol 21):
- CO-OP TEAM (mp_team.c; party_menu.c MENU_MP_TEAM, MpDrawTeamBadge): up to 3 marked 1-2-3 in the field party menu,
  stored as personalities ("team1".."team3" in the .coop file); mp_coop.c ArrangeLocalParty takes MP_Team_BattleSlots.
  Debug `mp team SLOT`.
- EXP per owner (battle_script_commands.c MpCoopSplitExp: slots 0-2 / 3-5 split separately) with each owner's own
  bonus (MP_Coop_ExpBonusFor; the partner's MP_ExpBonusPercent arrives in MPR_COOP_BONUS at battle start).
- RUN (co-op wild): battle_main.c SetActionsAndBattlersTurnOrder sets gMpCoopRunAgreed when every player battler chose
  RUN (extra runners -> NOTHING_FAINTED, RUN sorts with items/switches); battle_util.c HandleAction_Run: agreed ->
  MpCoopTryRun (fastest player vs fastest wild, Smoke Ball/Run Away) -> BattleScript_MpCoopGotAway (endlinkbattle,
  outcome RAN = 4), else BattleScript_MpCoopWantsToRun. Traps apply (IsRunningFromBattleImpossible no longer skips co-op
  wild). Co-op trainer RUN -> BattleScript_MpCoopCantRunFromTrainer selection script.
- Disconnect: MP_Coop_OnBattleAborted gives CAUGHT_MINE catches after LoadPlayerParty; the partner notes its own catches
  from the 3-shake throw anim (MP_Coop_NoteOwnCatch), replaced by the master's result list when it arrives.
- Ball target (battle_controller_player.c HandleInputChooseBallTarget, from the bag and L): item | MP_BALL_TARGET_FLAG
  (0x8000) | battler << 12; HandleAction_UseItem -> gMpCoopBallTarget; Cmd_handleballthrow calls a throw off if the
  target is gone (MP_Coop_RefundBall: local AddBagItem or MPR_COOP_REFUND, BattleScript_MpCoopBallNoTarget).
  IsPlayerPartyAndPokemonStorageFull checks the saved real party in a co-op battle.
- Ally intent: MP_Coop_SetIntent at each commit (move+target, ball, item+slot, switch, RUN) -> MPR_COOP_BSTATUS
  {DONE, kind, u16, target}; the action prompt (MpPrintActionPrompt) shows "BOB: EMBER" / "-> ZIGZAGOON" instead of the
  question, refreshed on arrival (MP_BattleRefreshActionPrompt). Logs "co-op: intent K value V target T".
Tests: coop_run, coop_ball_target, coop_abort_catch(2), coop_trainer (RUN refused).

Batch 3 (mp_linkx.c): record mixing over relay (record_mixing.c MP_RecordMix_*, mp_battle.c
NetLink_BeginRecordMix fakes a 2-4 player link), friend Secret Base visits (secret_base.c MP_SecretBase_*,
EventScript_MpVisitSecretBase), link Trainer Cards, Day Care co-op eggs (daycare.c MP_Daycare_TryCoopEgg,
`<save>.coopegg`), shared roamer/weather, Hall of Fame friend teams. Debug: `mp mkbase 181`, `mp daycare N`,
`mp coopegg`, `mp takeegg`, `mp hof`, `mp roamer`, `mp world`. All verified with two clients.
No UNION ROOM page any more: CO-OP > FRIENDS > a friend (mp_linkx.c MP_Linkx_OpenFriend) has GO TO, TRAINER CARD,
VISIT SECRET BASE; CO-OP > BERRY GAMES (MP_Linkx_OpenUnion) has the blender and crush. Mixing is automatic only.
Minigames (mp_minigame.c): BERRY BLENDER (berry_blender.c NPC mode with friends in the seats, leader syncs
arrow/speed/progress and final numbers) and BERRY CRUSH (a relay stand-in for the RFU calls via macros at the top
of berry_crush.c). Debug: `mp blender`, `mp crush`, `mp pokeblocks`, condition `crushask`. Link CONTESTS
(mp_grouplink.c): a 2-4 player link behind the NetLink_* calls (link.c) + matchmaking in TryContestEModeLinkup.
Test: MOEMON_DEBUG_WARP=13,4,15,4 on both, UP UP, mash A. All verified with two clients.

LOOK (mp_avatar.c, mp_menu.c): FRLG overworld sprites are catalog-only ids (MP_GFX_EXT_FIRST + n, mp_ext_gfx.h);
ID PIC / BK PIC choices (MpAppearance.frontSel/backSel, 0 = MATCH) load into the TRAINER_PIC_MP_* runtime slots.
Assets: tools/moemon_frlg/import.py (pokefirered checkout + Showdown download, not in the repo) writes graphics/moemon/ext.
Colors are hand-mapped per sprite in tools/moemon_frlg/colormaps.txt -> colormaps.py -> src/mp/mp_color_maps.h.
To map new sprites: `mp dumpcolors DIR`, colorsheets.py (highlight sheets), colorcheck.py (test looks).
LOOK > CREATE (mp_creator.c, mp_menu.c HandleCreateInput): CUSTOM 1-3 trainers from a parts kit. A look is a
14-byte recipe (struct MpCreated, MpAppearance.recipe, ini custom1-3 as hex, sprite = custom_N); every client
composes the 32x32 field frames (walk/run/Mach Bike), 64x64 front and 4-frame back and caches them per player
(MP_Created_Gfx). Pictures: graphics/moemon/creator/parts.bin (tools/moemon_creator/build.py KIT_DIR; the kit isn't in
the repo). The few Gen 4 walking parts build_ow.py borrows live in tools/moemon_creator/gen4_ow.bin/.json (not in the exe).
Field sprites (mp_creator.c ComposeField): Gen 3 style, baked by tools/moemon_creator/build_ow.py KIT3_DIR FRLG_DIR
(the Gen 3 Top Down Sprite Maker kit + a pokefirered checkout; neither in the repo) into graphics/moemon/creator/ow3.bin +
src/mp/mp_creator_ow.h: 27 frames per piece with color roles (skin shade / recolor group / fixed). Kit: body, head,
hair (30, recipe.fieldHair 0 = AUTO), hats (HAT_KIT), shirt/pants/shoes. Official FRLG donors (by body band, DONORS table):
skirts, shorts, coat, overalls, swimwear, bags. Small head accessories borrowed from the CREATE parts (HAT_BORROW).
Exact Gen 3 skin ramps (SKINS). `build_ow.py ... --preview out.png` renders looks with the same steps as the C code.
Test: ini `sprite = custom_1` + `custom1 = 0101020304000307010a0102060000`, `mp bike` / `mp bike 0`. Verified with two clients.
Extra field hairstyles (fieldHair 31-144): tools/moemon_creator/donor_hair.py cuts them from aveontrainer's FRLG-style
overworld sheets (2x art, `donor_hair.py --fetch DIR`; not in the repo) listed in hair_donors.txt; build_ow.py takes the
folder as a 3rd argument (Kit3.add_hair). `donor_hair.py IMG_DIR KIT3_DIR out.png [names]` renders review sheets.
Only clean cuts are listed; options per donor: long= (stop following long hair), noacc, eyes=, dx/dy, copy=N:S.
Placement (Donor.offset): every view moves by the front view's eye offset horizontally; side and back views' tops are
pinned to the front view's top on screen (kit3.head_offset places the head cell lower in side/back views); the sheets
have MARGIN spare rows above and below (kit3.head_frames handles taller sheets). Audit: all 114 tops level, none clipped.

Co-op Triple Battles (docs/TRIPLES_*.md, progress log in docs/TRIPLES_PROGRESS.md, protocol 23): MAX_BATTLERS_COUNT 6
(battlers 4/5 = B_POSITION_PLAYER/OPPONENT_CENTER, BATTLE_PARTNER(4/5) = 0/1, gIsTripleBattle / IS_TRIPLE_BATTLE() with
BATTLE_TYPE_DOUBLE set, BATTLER_SLOTS for "every battler" loops; recorded battles stay 4). Linked: link player i = seat i =
battler 2i (0 left, 2 right, 4 center) with party slots 2i-2i+1 (mp_coop.c SeatSize/SeatBase; doubles keep 3+3), the
master runs 1/3/5 (enemy slots 0-2 out, 3-5 a shared pool). mp_battle.c MP_Battle_OpenLinkGroup = 3 games (per-link
queues/sync/ready). Matchmaking: wild asks the two closest friends in grass (2 yes = triple, MP_RollExtraLandMon twice);
trainers with partySize >= 3 (single modes) wait for a third after the first joins (TripleEligible, B = double now).
Rules: target picker over six (battle_controller_player.c HandleInputChooseTargetTriple), Helping Hand on either ally,
spread 3/4 (pokemon.c SpreadDamage), MpNextSpreadTarget, battle_util.c MpTripleChooseTarget, replacements
(MP_TripleSwitchRange / MP_IsPartyIdOnSide). Cards: src/mp/mp_tricards.c over the frame (owner strip = seat color),
healthboxes parked below the screen. Party screen order MP_TripleFixPartyOrder (allies' slots 1,3,4,5 gold).
Second custom back pic slot TRAINER_BACK_PIC_MP_PARTNER2. Debug: `mp triple S:L S:L S:L [...]` (local, one player
commands all three), MOEMON_TRIPLEDEBUG=1 (logs; card visibility). Tests: tri_wild, tri_trainer, tri_catch, tri_heal,
tri_abort, tri_levelup, tri_faint (MODE=3, tools/test/runN.sh). Gotcha: anything that does `battler ^ BIT_FLANK` or
assumes positions 0-3 (reshow_battle_screen.c, shadows, Cmd_monbg) breaks for 4/5 (sprite 0 = battler 0's healthbox).
Cards: the HP bar follows MoveBattleBar's return value (battleBars[].currValue is fixed-point when max HP < 48);
same-named Pokémon on a side get A/B/C left to right (MP_TriCards_Letter, also in the target box). The cards are drawn
over the finished frame, so MP_TriCards_Composite paints the battle emote pick and bubbles again on top of them
(MP_Emote_BattleOverlaySprites / MP_BattleChat_OverlaySprites). Battle bubbles keep the tail on the speaker and slide
the box back on screen (Bubble.boxShift) instead of moving the whole bubble.
Intro: CONTROLLER_LOADMONSPRITE is LINK_SYNC_ALL (battle_controllers.c LinkSyncKind); as an owner command a joiner
replayed the wild Pokémon's slide-ins one after another.

Grouped stat moves (doubles and triples, any battle): a status move hitting several Pokémon (MOVE_TARGET_BOTH /
FOES_AND_ALLY: Growl, Tail Whip, Leer, String Shot, Sweet Scent) records each target in BattleScript_EffectStatDown
(`mpstatgroup 0` changed, `mpstatgroup 2` can't go lower; script command B_SCR_OP_MPSTATGROUP) instead of playing its
stat animation and message, and MOVEEND_NEXT_TARGET runs BattleScript_MpStatGroupShow after the last target: one
B_ANIM_STATS_CHANGE with animArg2 = 0x80 | battler mask (AnimTask_StatsChange -> gMpStatAnimMask: one OBJ-window
sprite copy per battler) and one line, "The party's" / "The foes'" picked per screen in BufferStringBattle
(STRINGID_MP_GROUP_STAT_*). One target alone gets the usual message. Misses, PROTECT, CLEAR BODY etc. still show per
target. Self-targeted buffs (Swords Dance) hit only the user, so they're unchanged. Test: coop_statgroup.

## Status over the name tag, renaming

"IN BATTLE" / "IN MENU" over a remote player's name (mp_overworld.c RemoteTagStatus). MPF_IN_MENU (state flag bit 7,
mp_main.c CollectLocalState): on the field while the START menu (start_menu.c MP_IsStartMenuOpen: StartMenuTask or
Task_ShowStartMenu), the CO-OP menu (MP_Menu_IsCoopMenuOpen) or any MpUi window, a script's multichoice or the shop's BUY/SELL menu is open; off the field on any screen that
isn't a battle or one of the field's own load/return callbacks (overworld.c MP_IsFieldLoadCallback), so warps don't
flash it. Renaming has no OT check: SELECT on the summary info page (MpCanRename) and the Slateport Name Rater.

## BAD EGGS from co-op catches

Up to protocol 23 a co-op wild catch by anyone but the host became a BAD EGG: the wild Pokémon is created in the
host's game with the host's OT ID, and GiveMonToPlayer overwrote boxMon->otId, which is half of the substructs' XOR
key. GiveMonToPlayer now uses MP_SetBoxMonOtIdKeepData (pokemon.c: XOR the substructs with old ^ new ID). The tests
missed it because every test client had the same Trainer ID (`mp giveforeign SPECIES` tests it now). After loading a
save (CB2_ContinueSavedGame), mp_badegg.c MP_CompensateBadEggs removes every BAD EGG (MP_IsBoxMonBadEgg: flag or
checksum) from the party and PC and pays 10 MoeGatcha pulls each, once per personality ("badegg:XXXXXXXX" in the
co-op data, in case the game isn't saved after). A party left empty takes the first healthy PC Pokémon. Debug:
`mp badegg SLOT`, `mp compensate`. Also: a caught wild Pokémon's HP is zeroed in every game's copy (the "caught"
SpriteInvisibility handlers), so the battle screen rebuilt after the BAG doesn't show it again.

## Heap size

The PC build's heap (include/malloc.h HEAP_SIZE) is 0x80000, not the GBA's 0x1C000: 64-bit MemBlock headers and
pointers plus six battlers' sprite buffers filled it in battle, and the summary screen opened to forget a move then
wrote through a NULL Alloc (crash on learning a fifth move, even in a single battle). Running out is now logged
("[heap] out of memory: ..."); `mp heap` prints the use. Test: solo_learn.

## Triple Battle joining rules

TripleEligible: the trainer's own partySize >= 3 and a one-trainer mode (SINGLE, CONTINUE_SCRIPT(_NO_MUSIC),
SINGLE_NO_INTRO_TEXT, REMATCH). A triple's enemy party is never padded (MP_Coop_FinishTrainerParty returns for
count 3; doubles still pad by half). Requests are sent automatically by TryAsk when a free player faces a challenged
one, so two friends next to the host ask at once: a second request while the host is still answering the first
(or before JOIN_WAIT_THIRD) is kept as requester2 (MPR_COOP_REQUEST handler) instead of DECLINE_BUSY, and only
DECLINE_NO / DECLINE_ONE_MON set the asker's declinedBy (busy / too late can retry). TryWaitForThird waits when a
third is queued or any other player is online and not battling (FriendAvailableExcept, any map). Not eligible but
someone else around: JOIN_NO_TRIPLE shows sText_NoTripleFew / sText_NoTripleKind to the host, a third who asks gets
DECLINE_NO_TRIPLE. Tests: tri_trainer_rush (failed on the old code), tri_no_triple (Tommy, 2 Moemon).
