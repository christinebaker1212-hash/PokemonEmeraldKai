# Co-op feature notes

Moved out of CLAUDE.md: per-feature details (files, functions, debug commands, test recipes).
Read the part about a feature before changing it.

Emote / phrase pick (mp_menu.c MP_OpenQuickPick, the only emote screen; mp_battlechat.c in battle): LT/RT or Q/E held = held mode (release sends),
SOCIAL > EMOTES = sticky (A sends, B closes); on a pad emotes only come from the LT/RT hold menu (no right stick). Pages EMOTES/MOODS/REACT/HYPE (MP_EMOTE_PAGES, sQuickEmotes: the extra emotes MP_EMOTE_ANGRY..SKULL, art from
tools/moemon_emotes/gen.py -> src/mp/mp_emote_gfx.h, field icon trainer_see.c MP_StartExtraEmoteIcon on the overlay
palette, 2-frame ones flip every 8 frames, sent as MPR_SAY SAY_ICON {kind}, battle bubbles MP_BUBBLE_ICON_EXTRA + n;
MP_Emote_Send(kind) sends any; `mp emotekind K`, `mp battleicon K`, `mp quick PAGE`; test emote_pages), MINE 1-8 (32 phrases, MP_NUM_PHRASES,
ini phrase1-32; MINE n shows once page n-1 has one) + 4 stock groups (L/R); SELECT (pad west) swaps/pins
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
ANIM SPEED (host setting, HOST A GAME > ANIM SPEED: NORMAL / +25% / +30%, gMpConfig.animSpeed, ini battle_anim_speed,
default 30, MOEMON_ANIM_SPEED overrides ours for tests): MPR_RULES is {u8 padding, u8 animSpeed} (protocol 27) and
MP_BattleAnimSpeed() is the room's value. While gAnimScriptActive, MpBattleTurboRate is animSpeed/5 extra steps per 20
frames; a move animation's script steps in its battler's controller, so the extra step also calls gAnimScriptCallback
(not for general / special / status animations, whose script steps in a task: MP_BattleAnimInTask). Cmd_end waits for
the sound effects to finish, so the SE players (SE1-3) play at the same tempo while an animation runs (MpSetSoundTempo;
m4aMPlayTempoControl is a stub on PC). Scratch / Tackle: 41 -> 33 frames at +25%, 32-33 at +30%. Special animations
(ball throws, switch-outs, substitute: MP_SpecialAnimRunning) keep their speed: a sped-up co-op catch hung (coop_catch).
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
free pulls (mp_gatcha.c MP_Gatcha_GrantFree from MP_Events_Update and on opening: +1 free 10-pull
"gatcha:free10" per week (STANDARD only), +1 free pull per banner "gatcha:free1bN" per day played, stocked; menu
FREE x10 / FREE x1 / TICKET), MOEGATCHA TICKETS ("gatcha:tickets": data/scripts/obtain_item.inc Std_ObtainItem ->
special MP_GatchaTicketForItem for key items / HMs / gym leader TMs / EXP. SHARE, once per item "ticketitem:ID", item
from scrcmd.c gMpLastScriptItem since VAR_0x8000 changes during the message; Mauville Game Corner doll clerk sells them,
GATCHA_TICKET_COINS 2500, special MP_GatchaBuyTicket; test gatcha_free; saves from before tickets: MP_Gatcha_BackfillTickets
once per save ("gatcha:ticketbackfill", from MP_Gatcha_GrantFree; a toast, never a message box) gives "ticketitem:ID" for story items proven by their
event flag (handed over / used up: DEVON GOODS, gym TMs, HMs...) or held in the bag or PC (other key items, not the
item-ball keys), one bike; test gatcha_backfill, `mp ticketbackfill [reset]`, `mp setflag N`; `mp giveitemscript/additem/coins/gatchastock`), rematch pull once a day, tournament pulls once a day (3+ players, no forfeit), race needs
2+ racers, HoF pull once per friend. Achievements (mp_achieve.c, 70, personal, offline too: "av:ID", counters "acnt:NAME"),
DAILY > ACHIEVEMENTS. Achievement rewards: tier 0/1/2 = 3/10/20 pulls (mp_achieve.c AchPulls; saves from before get the
difference once, "ach:rewards2"). Offline, DAILY shows the day's 3 "everyone" goals (sSoloRows; PeakPlayers = 1), counted
on the save ("solo:DATE:S" via ContributeGoals/GoalNumbers), rewarded by CheckGoals ("sgoal:"/"sgoalall:" keys,
the same weekly pull cap); MP_Events_Update runs offline from MP_FrameUpdate. The outbreak runs offline too. RARE CANDY cap MP_RareCandyCap (party_menu.c). Save data (mp_savedata.c)
drops dated keys after 14 days and never evicts undated ones. Debug `mp goals` lists a week of goals.
MoeGatcha reel and banners (mp_gatcha.c): teaser cells never repeat an icon within TEASER_GAP (8) cells
(IconKey: item id, set's first item, 0x8000|species; PickVaried keeps the best of 24 tries for small pools; StartSpin
fixes the prize and its forced neighbours first, then fills from center+7 so cells with live icons stay). STANDARD's
R/SR/UR/EX pools are rebuilt on opening by SetupStandard from the day number (sDailyMons 4 a day in turn,
sDailyLegends a 7-day table with no species twice a day); the tagline takes turns with "TODAY'S UR / R". PICKUP
reroll vote: relays MPR_GATCHA_FIRST (230) GV_ASK/ANSWER/RESULT/STATE; the asker counts YES, ends once the majority is
certain (or 45 s), draws a random different PICKUP and sends it in GV_RESULT; "gatcha:votewk" (week+1) /
"gatcha:votepick" override the rotation (ThisWeeksPickupIndex: `mp pickup` override, then the vote); two askers at
once: the lower player number keeps theirs; friends who arrive later get GV_STATE. Questions show in the field
(MpUi, when MP_IsLocalFree) or on the gatcha screen (GS_VOTE_ASKED). Tests: gatcha_vote (3 clients at NA-JP lag),
`mp gatchareel` (icons in view + closest repeat), `mp gatchaday N`, `mp gatchagive EV [SHINY]` (logs "event mon").

Game clock (mp_main.c, sdl2.c UpdateInternalClock): the room creator sends MPR_CLOCK (s64 local time as if UTC,
MP_LocalClockAsUtc, field by field since Windows has no tm_gmtoff) with the room rules to each friend who joins and
every 5 minutes; others keep `hostLocal - time(NULL)` and the RTC uses gmtime(time + offset) while in the room
(IN_ROOM / RECONNECT_WAIT), their own localtime otherwise. MoeGatcha/daily keys stay on UTC days (MP_Today).
Berry trees count on the player's own clock: clock.c UpdatePerMinute subtracts MP_ClockShift() (host clock minus
ours) and lastBerryTreeUpdate is always in that frame, so a join/leave never grows or freezes them; VAR_DAYS follows
the shared clock (a day only ever runs once). The RTC itself was broken on PC: sdl2.c set the 24-hour status after
AgbMain > RtcInit, whose probe then "reset" the clock (siirtc.c PORTABLE reset left sLocked TRUE), so every read failed
and gLocalTime stayed day 0 00:00 (status 0001). Fixed both; the game clock is the PC's (localTimeOffset kept 0,
wallclock.c starts at the PC's time). clock.c FixStuckClockSave moves old saves (offset set or lastBerryTreeUpdate
before day 2000) to the PC clock without a catch-up flood (Berry timer = now, VAR_DAYS = yesterday), and
DoTimeBasedEvents turns time events on for SKIP THE INTRO saves (FLAG_SET_WALL_CLOCK without FLAG_SYS_CLOCK_SET).
Tests: clock_sync (TZ=Asia/Tokyo host, TZ=America/Los_Angeles friend), clock_berries, clock_oldsave; `mp clock`,
`mp berryinfo/berrytree/berrywait`, `mp stuckclock`, `mp goffline` / `mp gjoin`.

Bag sorting and new-item marks (mp_bag.c, item_menu.c): START opens a 2x3 grid (ITEMWIN_2x3) of MP_BAGSORT_* orders;
the order per pocket is "bag:sort%d" in the .coop data (0 = TM/Berries by ID, others MANUAL). UpdatePocketItemList
compacts and calls MP_Bag_Sort (stable insertion sort; TYPE = ItemCategory from battle usage / field use / hold
effect / mail). A SELECT move sets MANUAL; TM/Berry pockets allow moving only in MANUAL. AddBagItem calls
MP_Bag_NoteAdded when an item wasn't in the BAG (running number + "new" bit); ClearBag resets them. They're written to
"<save>.bag" after the pockets (MARKS_MAGIC); an older sidecar or none counts the current order as oldest-first. The
list shows "!" (GetItemNameFromPocketMarked); BagMenu_MoveCursorCallback clears it and the input task redraws the list.
Hint window WIN_SORT_HINT at baseBlock 0x280 (the context windows use up to 0x270). Test bag_sort, `mp bagorder P`.

e-Reader Berries: ITEM_PUMKIN_BERRY..ITEM_TOPO_BERRY are items 377-388 (ITEMS_COUNT 389), berry numbers 47-58
(FIRST_EREADER_BERRY_TYPE, after the three unused berry numbers 44-46). ITEM_TO_BERRY / BERRY_TO_ITEM / BERRY_SLOT /
IS_BERRY_ITEM / IsValidBerryType know both ranges; ITEM_TO_BERRY keeps the plain formula outside C (`#ifdef MODERN`)
because the scripts' assembler has no `?:` (scripts only name CHERI..ENIGMA). Tables indexed by berry number
(gBerries, gBerryCrush_BerryData, sBerryPicTable, the tree tables) have entries at BERRY_SLOT(item) and zeros for 44-46.
gItemEffectTable grew to the new items (PUMKIN/DRASH/YAGO/TOUGA reuse ASPEAR/PECHA/RAWST/PERSIM's effects);
ITEM_HAS_EFFECT includes them. Held effects are the vanilla ones (CURE_FRZ, CURE_PSN, CURE_ATTRACT, RESTORE_STATS,
CURE_BRN, CURE_CONFUSION). Data from Bulbapedia (flavors, firmness, effects; sizes where known); growth 72 h (CHILAN
4 h) and the tag text are ours. Art: tools/moemon_berries/draw_ereader_berries.py draws the icons and tag pictures;
trees reuse vanilla trees with matching colors (as ENIGMA uses DURIN's). Berry Crush sends berry numbers
("crush: berries N N" in the log); the Blender treats an e-Reader Berry like ENIGMA for NPC partners. Tests:
crush_ereader (NA-JP lag), `mp berrytree TREE BERRY STAGE`, `mp item ID QTY`.

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
(+2, +3 for MP_Coop_IsBossTrainer) used by battle_main.c CreateNPCTrainerParty; MP_Coop_FinishTrainerParty adds exactly one
Moemon when TEAM PADDING is on (doubles and triples, bosses too; MP_TrainerPadding) before the ace with PickFillerSpecies (shares a type with a party mon, base stats within
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
Dewford trend online (dewford_trend.c, mp_linkx.c LX_TREND): TrySetTrendyPhrase always takes the phrase while
connected (MP_Dewford_PutTrendOnTop: trendiness above every saved one, max 127, others stepped down to 126) and
sends words + rand to the room; receivers put it on top the same way. Record mixing had set FLAG_SYS_MIX_RECORD (no
first-phrase rule) and a mixed-in trend could keep the top spot. MP_Dewford_Sanitize replaces saved trends with
invalid words ("??? ???", MP_IsEasyChatWordInvalid) every ~10 s on the field, before sending a mix record and after
receiving one; ReceiveDewfordTrendData skips invalid ones. One broken save with a high trendiness used to spread
"??? ???" to every game at the first mix and block new phrases. Debug: `mp trend`, `mp trends`, `mp settrend SLOT W1
W2 T`. Tests: dewford_trend (online, the real screens), dewford_broken.
Harness states `yesno` (a script YES/NO box) and `easychat` (the word screen); `mp warp G,N,X,Y` warps on the field
(map constants are NUM | GROUP << 8: Mauville Game Corner is 10,3).
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
trainers with 3+ Moemon (CoopTrainerTeamSize: partySize + 1 with TEAM PADDING; single modes) wait for a third after the
first joins (TripleEligible, B = double now).
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

TripleEligible: CoopTrainerTeamSize >= 3 (the trainer's partySize, +1 with TEAM PADDING) and a one-trainer mode
(SINGLE, CONTINUE_SCRIPT(_NO_MUSIC), SINGLE_NO_INTRO_TEXT, REMATCH). Doubles need CoopTrainerTeamSize >= 2 (else
DECLINE_ONE_MON). Requests are sent automatically by TryAsk when a free player faces a challenged
one, so two friends next to the host ask at once: a second request while the host is still answering the first
(or before JOIN_WAIT_THIRD) is kept as requester2 (MPR_COOP_REQUEST handler) instead of DECLINE_BUSY, and only
DECLINE_NO / DECLINE_ONE_MON set the asker's declinedBy (busy / too late can retry). TryWaitForThird waits when a
third is queued or any other player is online (FriendAvailableExcept: any map, whatever they're doing - they may be
finishing a battle). It first asks "<friend> has joined! Wait for another?" (JOIN_MORE_*, YES first; TryWaitForThird
returns THIRD_ASK, already sends MPR_COOP_WAIT and keeps a third who asks meanwhile, which starts the triple at once;
NO = AcceptRequester, the double; THIRD_QUEUED = a third already asked, no question). YES = StartWaitingThird.
While waiting for a third, B only asks "Start a Double Battle ... without waiting?" (JOIN_START_DOUBLE_*,
NO first): a single B pressed through text used to start the Double at once and the third was told "can't team up"
(tri_wait_mash). A third asking once the battle is accepted gets DECLINE_TOO_LATE. Not eligible but
someone else around: JOIN_NO_TRIPLE shows sText_NoTripleFew / sText_NoTripleKind to the host, a third who asks gets
DECLINE_NO_TRIPLE. Tests: tri_trainer_rush (failed on the old code), tri_no_triple (Tommy, 2 Moemon), tri_more_no
(NO to "Wait for another?"), coop_ask_cancel. B while asking (UpdateAsking) is not a "no": it sets cancelledAsk, and
TryAsk skips that friend only while still facing them (turning away clears it; facing them again asks again). While the
host's "wants to join" question is up for that friend (promptedFor), their new request just takes the seat back.
Field messages in Task_JoinCheck go through MP_ReplaceFieldMessage: ShowFieldMessage ignores a new message while
the last one is still printing, which left "Waiting for a partner" over the "Wait for another?" YES / NO.

## Co-op EXP at a single battle's pace

Cmd_getexp in a co-op battle (gMpCoopBattle) leaves the per-slot loop after the calculation for states 10-15
(battle_script_commands.c MpCoopExpBegin / MpCoopExpStep): every slot's gain at once (same rules: own share,
EXP Share, Lucky Egg, trainer x1.5, traded, buddy bonus); one STRINGID_MP_EXP_ALL line (gains in gBattleTextBuff1,
boosted mask in buff 2, each screen names its own slots via MP_Coop_OwnBase, others' if none); then rounds: one
CONTROLLER_EXPUPDATE per player at once, to that player's battler (seat * 2; the owner's controller applies it,
the others mirror it, MP_Coop_MirrorExp); level-ups of the round get one STRINGID_MP_GREW_ALL line
(BattleScript_MpCoopGrew). State 14 gives the moves that fit (MonTryLearningNewMove) and queues the rest
(MP_Coop_ResolveAddLearn) plus every stat box (MP_Coop_ResolveAddBox, stats from before the EXP); state 15 runs
BattleScript_MpCoopResolve.

Resolve (mpcoopresolve + mp_coop.c MP_Coop_Resolve*): the master sends each player MPR_COOP_RESOLVE with their own
boxes and learns (with the Pokémon's moves as the master has them: the owner's copy can miss a move learned earlier
in the battle) and starts its own part locally. Every game runs Task_MpCoopResolve at once: Cmd_drawlvlupbox driven
from the task (gBattlescriptCurrInstr swapped in and back the same frame; the master's script is parked in
mpcoopresolve), then per learn the single battle prompts (trying to learn, can't learn more, delete?, summary
screen, HM, stop teaching?) printed locally, never with printstring (that would reach every screen). Our side's
healthboxes are hidden meanwhile (in triples they cover the stat box and YES/NO). Joiners apply the choice to their
own copy and answer MPR_COOP_RESOLVE_DONE (move slots, 0xFF = didn't learn); a player who leaves counts as done.
When everyone is done the master applies the answers to its party and battle mons (MOVE_IS_PERMANENT) and prints one
STRINGID_MP_LEARN_ALL line (fanfare) for every move learned this round. MP_Coop_RecordLearnLater (after the battle)
is only a fallback now. Logs: "co-op levels: ...". Tests: tri_levelup, coop_levelup (Torchic Lv16 forgets for Peck).
Battle emotes: MP_BattleOverlay_Composite repaints the pick and bubbles over the finished frame in every battle.

## Ball entrance and recall effects

Per Moemon, next to the ball style: MON_DATA_MP_BALLFX (PokemonSubstruct0's spare `filler` u16: form 0-3, particles
4-7, energy color 8-11, recall 12-14) and MON_DATA_MP_BALLFX_SOUND (BoxPokemon's 4 spare flag bits). Both are in the
Pokémon's data, so trades, the shared PC and link battles carry them. src/mp/mp_ballfx.c (engine, names, presets),
src/mp/mp_ballfx_editor.c (editor), art from tools/moemon_ballfx/gen.py -> src/mp/mp_ballfx_gfx.h (indexed pixels;
16/17/18 = the energy color and its light / dark shade). Debug: `mp ballfx SLOT F P C R S` / `mp ballfx SLOT preset N`,
`mp fxedit SLOT`, `mp party` prints fx. Logs: "ballfx: entrance form F part P", "ballfx: recall R".
MY PRESET 1-3 live in the save's .coop file ("fxpreset1" = bit 30 used | sound << 16 | MP_BallFx_Pack;
MP_BallFx_GetUserPreset / SaveUserPreset / WhichUserPreset, MP_BallFx_EffectName names a mix for the editor and the
summary). Editor rows SAVE AS (ASK_REPLACE when the slot holds another mix) and APPLY TO PARTY (ASK_PARTY; party
Moemon that aren't eggs); the questions and "saved" notes use the info box. Test: fx_presets.

Two halves:
- Bodies (the Moemon's own sprite, Task_FxBody): palette blends (silhouettes, glows), the OAM matrix (DOUBLE mode;
  BodyScale keeps the feet on the ground), mosaic, blending (BLDALPHA for the haunted fade / dissolve), flicker.
  Entrances last FORM_LEN = 16 frames (the game's send-out: emerge + palette fade), recalls RECALL_LEN = 22. BodyEnd
  puts the sprite back exactly. MPFX_FORM_VANILLA / MPFX_RECALL_VANILLA are the game's own as bodies (followers).
- Particles: not sprites. sParts (1024) are moved by Task_FxManager and drawn by MP_BallFx_Composite over the finished
  frame (platform VDraw, before the battle emotes): nearest-neighbor scale / rotation / stretch, 16-level alpha,
  BLEND_ADD (light) and BLEND_GLOW (adds on dark pixels, paints on bright ones), trails, fades, the game's palette
  fade applied too. PF_UNDER skips pixels the Moemon sprite covers (MonPixel reads its tiles from VRAM, through its
  matrix), so ground effects and the far side of orbits are behind it; MonSquares turns the sprite into 4x4 squares
  in its own colors (digital form / shutdown, dissolve). Spotlight (dim everything but a dithered lit cone) and flash
  are screen passes in the compositor. In battle nothing draws over the text box (y >= 112) unless PF_NOCLIP; the
  editor clips to its field (MP_BallFx_SetClip). Particles only draw on the screen that spawned them (sPartScreen).
  Several entrances at once (doubles / triples) scale the counts down (Budget).

Battle hooks: pokeball.c SpriteCB_ReleaseMonFromBall (DEFAULT parts stay the game's own; a custom form skips the
emerge affine anim and the ball fade task, HandleBallAnimEnd_MpFx waits for the body, then the NORMAL affine anim ends
so the cry and idle anim start as usual); battle_anim_throw.c AnimTask_SwitchOutBallEffect runs the custom recall
(Task_MpWaitRecallFx keeps the anim script waiting) and AnimTask_SwitchOutShrinkMon does nothing then. The send-out
turbo (MP_SendOutRunning) speeds the effect up with the rest. NPC trainers' and wild Moemon are always DEFAULT.

Editor: SELECT on the summary's POKé BALL page (Task_MpStartFxEditor, like the rename round trip; back on the ball page:
sMpStartOnBallPage + InitBGs sets the layers as if scrolled there). Battle background (TallGrass), the Moemon on the
player's platform (CreateMonPicSprite_Affine; SELECT front/back), the panel and a description box (wrapped by width).
The preview loops: ball drops in, opens, entrance, 56-frame pause, recall.

Followers: MpState.followerFx + followerFlags bits 1-3 sound, 4-5 sent-out count, 6 recall playing (MP_WIRESTATE_SIZE
22, protocol 25). A count change while the follower is around: a mini ball (particle) is tossed from its owner to its
tile and it comes out with its effect (DEFAULT = MPFX_FORM_VANILLA). Called back: sLeavingState keeps it out until its
recall has played. Tests: fx_coop, fx_triple, fx_switch, fx_follow.
Follower friendship (mp_follower.c): +5 when chosen (JoyOfBeingChosen, once a day per personality, save key
followjoy:DATE:PID) and +1 every 128 steps (MP_Follower_OnWalkSteps from field_control_avatar.c, on top of the party's
50% walking roll). Test: follow_joy.


## TEAM PADDING (host setting)

HOST A GAME > TEAM PADDING (mp_setup.c HOST_PADDING, gMpConfig.trainerPadding, ini trainer_padding, default on). The
room creator sends MPR_RULES {u8 padding, u8 animSpeed} to each friend who joins, right before the welcome note (mp_main.c
SendWelcome); MP_TrainerPadding() is the host's value when heard, else ours (MOEMON_TRAINER_PADDING=0/1 overrides
ours, for tests). The battle master decides everything with it: CoopTrainerTeamSize (Double needs 2, Triple 3) and
MP_Coop_FinishTrainerParty (exactly +1 when on, never more; logs "coop: trainer N party a -> b" or "party a, no
padding"). Protocol 26. Tests: tri_padded (Tommy 2 -> 3, Triple), coop_one_mon (Josh 1 -> 2, Double), tri_no_triple
(padding off: Tommy's own two, Double, "room rules ... off" heard by BOB).

## Graphics past the GBA limits (PC only)

include/gba/defines.h: OBJ_PAL_SLOTS 64 sprite palettes (the GBA's 16, then 16-63 right after them in PLTT and the
palette buffers, so OBJ_PLTT_ID(n) works for any slot), OBJ_TILE_COUNT_EXT 4096 sprite tiles (128 KB of OBJ VRAM),
OAM_ENTRY_COUNT 256, MAX_SPRITES 128. struct OamData is 12 bytes on PC (tileNum 12 bits, paletteNum 6 bits; field
order and names unchanged), OAM_SIZE follows (static assert in sprite.c). gOamLimit is u16 and defaults to
OAM_MANUAL_BASE (192): the last 64 entries stay for code that fills OAM by hand (digit_obj_util, confetti_util,
link_rfu_3 - their old fixed 64 / 125 now count from OAM_MANUAL_BASE / OAM_ENTRY_COUNT). Both renderers walk all
OAM_ENTRY_COUNT entries and read palettes as PLTT + 0x200 + paletteNum * 32. Matrices stay at 32.

Palettes: sprite.c MpFreePaletteSlot gives the GBA's 16 first, then 16+; MpPrefersExtraSlot tags (co-op palettes,
POKé BALLS 55000-55039 incl. their particles and the battle shadow, mp_ballstyle 0x56A0-0x56DF / 0x5710-0x575F) go
to 16+ first, so co-op never borrows or evicts the game's slots any more (MpFreeSlot tries 16+ first;
MP_MakeRoomForPalette only looks at 0-15). Palette masks have no bit for slots 16+: palette.c
MP_MaskTakesExtraPalettes includes them in fades / blends / tints whose mask has 8+ sprite bits (screen fades, "all
but the attacker"), not in ones aimed at a sprite or two. Use MP_BlendSpritePalette / MP_BeginSpritePaletteFade (by
slot) for one sprite's palette when it may be 16+ (the capture "ball darkens", battle_anim_throw.c). Weather tints
16+ like the game's dynamic sprite palettes (field_weather.c WEATHER_PAL_COUNT; the co-op overlay palette stays
untinted). Reflections of sprites in 16+ keep their palette (field_effect_helpers.c ReflectionPaletteFor).
Running out of sprites, palettes or tiles prints "[gfx] out of ..." (tests: UNEXPECT "[gfx] out of"); `mp pals`
lists every used slot. Test: gfx_pals_triple.
OAM indexes are u16 (sprite.c AddSpritesToOamBuffer / AddSpriteToOamBuffer / AddSubspritesToOamBuffer): the slot
machine sets gOamLimit = OAM_ENTRY_COUNT (256) and a u8 index never reached it (endless loop, black screen; test
slots_online). Emerald's object palette table ends with an empty entry while FindObjectEventPaletteIndexByTag looks
for OBJ_EVENT_PAL_TAG_NONE: on PC the end marker is there (BUGFIX branch kept under PORTABLE), so a tag that isn't in
it (a co-op avatar's, 0x1381+) no longer loops forever (the shop's map view with a friend in it; test shop_online);
the shop colors friends' avatars with MP_Overworld_MenuSpritePalette.

## Type icons (OPTIONS > TYPE ICONS)

gSaveBlock2Ptr->optionsTypeIcons (one of the 4 spare bits after regionMapZoom; old saves read OFF). option_menu.c has
eight rows 14 pixels apart (OPTION_ROW_H, 16 in the original). mp_typeicons.c draws 11x11 icons
(tools/moemon_typeicons/gen.py -> mp_typeicon_gfx.h, BGR555 with bit 15 = drawn) over the frame from sdl2.c VDraw after
the triple cards: on the name's line, right of a foe's healthbox (box sprite x + 60), left of yours (x - 23), top
y - 12 (measured, same in singles and doubles); in triples beside the cards (MP_TriCards_Place). They follow the box's
x2/y2 and invisible, fade with gPaletteFade and use gBattleMons types when its species matches the party Mon (link
battles: the games that don't run the engine have no gBattleMons types, so species types there). No Fairy in Kai.
Debug: `mp typeicons 1`, `mp options`; MOEMON_TYPEICON_DEBUG=1 logs box positions.
Battle animations' "battle background" palettes (GetBattlePalettesMask, AnimTask_BlendColorCycleExclude) are 2-3 on
PC, not 1-3: BG palette 1 in battle is the OPTIONS window frame (LoadBattleMenuWindowGfx), and co-op windows that
stay up during other battlers' moves (doubles/triples) flashed white with every backdrop flash (Double-Edge,
Take Down...). The backdrop itself is 2-4.

## GENESIS / EPOCH BALLS (Pokémon Red's and Crystal's POKé BALLS)

Items ITEM_GENESIS_BALL (ITEM_052) / ITEM_EPOCH_BALL (ITEM_057), special balls 12 / 13 (style `c000` / `d000`, kept
as POKé BALL + style like the other special balls); MoeGatcha BALL banner only.
- Catch (mp_legacy_catch.inc.c, plain C, shared with tools/test/legacy_catch_test.c which checks it against
  register-level transliterations of pokered af51989 ItemUseBall and pokecrystal 97187cd PokeBallEffect: run
  `tools/test/legacy_catch_test.sh`). Red: Rand1 - status (25 sleep/freeze, 12 others; underflow catches), `<=`
  catch rate, W = MaxHP*255/12 / max(HP/4,1) (> 255 catches), Rand2 <= X; failures show 0 (a miss: BALL_MP_MISS,
  caseId 6) to 3 shakes from Z. Crystal: A = rate*(3M-2H)/(3M), min 1, +10 / +5 (the +5 fixed), saturates at 255,
  caught when rand <= A; failures roll wobbles from the table. The 341+ max HP overflow is fixed by quartering
  then halving until 3*MaxHP fits. Catch rate comes from the encounter's species (gEnemyParty), so Transform
  doesn't change it.
- Resolved once in Cmd_handleballthrow by the game running the battle (mp_legacy_balls.c MP_LegacyBall_ResolveThrow),
  the result goes out in the usual BALLTHROWANIM caseId; messages per game via MP_LegacyBall_String (gLastUsedItem).
  Tests: `mp legacyrand B,B,...` rigs the next random bytes (Red draws 1-2, Crystal 1 + wobbles).
- Animation (mp_legacy_anim.c): tools/moemon_legacy/anim_bake.py runs both games' battle-animation engines offline
  from the pret checkouts (`python3 tools/moemon_legacy/anim_bake.py REFS` with pokered/ and pokecrystal/ in REFS)
  and writes mp_legacy_anim_data.h: per-frame OAM, events (sounds with Crystal's pan, Red's hide/show, Crystal's
  shrink steps) and tiles. Special_BallThrow jumps to Special_MpLegacyBallThrow for these balls. Drawn over the
  frame (VDraw, like mp_ballfx.c) below y 112 (the text box stays in front); touching tiles move as one object
  (each object's centre is mapped from the GB positions (38,96) / (124,56) onto the thrower / target, its tiles
  keep their 8 px spacing: mapping each tile on its own opened gaps through the ball).
- Send-out / recall: a Moemon in one of these balls with no ball effect of its own (MpBallFx defaults) comes out
  the original way (pokeball.c hides the Emerald ball; Red: poof, then 1 tile / 3x3 / 5x5 / whole; Crystal: poof +
  3x3 / 5x5 / whole every 4 frames) and goes back shrinking (battle_anim_throw.c AnimTask_SwitchOutBallEffect).
  Kai pics are 8x8 tiles: the 5x5 / 3x3 steps use tiles {0,1,4,6,7} / {0,4,7}.
- Art: tools/moemon_balls/legacy.py makes the 24x24 item icons and mp_legacy_ball_art.h (the supplied 12x12 ball),
  which mp_ballstyle.c draws into every frame of the ball sprite (summary, battle).
- Sounds (mp_legacy_sound.c): tools/moemon_legacy/sfx_render.py renders the GB sound effects from the disassemblies
  into mp_legacy_sfx.h; mixed into m4aSoundVSync's output (music_player.c), panned when the sound option is STEREO.
- Debug: `mp ballstyle SLOT HEX` sets a party Moemon's ball style. Tests: legacy_solo, legacy_coop.

## NAME CASE and CREATURES (OPTIONS, this game only)

SaveBlock2 spare option bits: optionsNameCase (2 bits: MP_NAMECASE_AS_TYPED / CAPS / MIXED) and optionsPokemonArt
(1 bit). OPTIONS shows eight rows and scrolls (option_menu.c sTop, ROW_Y). Debug: `mp namecase N`, `mp creatures N`.
The OPTION header (the game's own name, "Moémon Emerald Kai") is printed with the filter off.
- NAME CASE (mp_textopts.c): text.c AddTextPrinter hands every string to MP_TextOpts_Filter, which copies it into a
  per-window buffer (the source is never changed) and recases the names it recognizes: species and NPC trainer names
  (built once, bucketed by first letter, longest first), the player, co-op friends, the party, the foe's party and
  link players in battle, the PC's current box (looked at per string). Words must stand alone and match exactly as
  stored (MatchAt): the same letters in another case are ordinary words ("If I may", "I hope", "red" vs the trainers
  MAY, HOPE, RED). Move, ability and item names of two or more words are kept whole first (KeepAt: WAILMER PAIL,
  SERENE GRACE, LEAF STONE). Debug `mp namefilter TEXT` logs TEXT as shown (~ stands for é). Mixed Case
  starts a word after a space or hyphen ("Ho-Oh", "Mr. Mime", "Farfetch'd"). The naming screen is skipped. Name tags
  (mp_overworld.c BuildNametag) use MP_NameCase_Recase. gMpTextFilterOff shows a string as written.
- CREATURES: include/data.h turns gMonFrontPicTable, gMonBackPicTable, gMonPaletteTable, gMonShinyPaletteTable,
  gMonFrontPicCoords, gMonBackPicCoords, gEnemyMonElevation, gMonFrontAnimsPtrTable (and pokemon.c
  sMonFrontAnimIdsTable) into MP_MONART(...) picks between Kai's (_Moe) and the original game's (_Pokemon,
  src/mp/mp_monart_data.c, generated by `tools/moemon_monart/build.py POKEEMERALD_CHECKOUT` from pret/pokeemerald;
  the vanilla pictures are still in graphics/pokemon). The Pokédex menu and PokéNav header graphics switch the same
  way (graphics.h). The text filter swaps MOé -> POKé and MOEMON -> POKéMON. Followers keep Kai's overworld art (asked for).

## Bigger BAG (mp_bag.c)

gBagPockets point at mp_bag.c's arrays (MP_BAG_*_COUNT in constants/global.h: 200 items, 64 of the others; stacks
MP_BAG_ITEM_CAPACITY 999, BAG_ITEM_CAPACITY_DIGITS 3; the Battle Pyramid's u8 bag keeps MAX_BAG_ITEM_CAPACITY 99).
SaveBlock1's pockets are untouched in size: save.c copies the first slots there before every write
(MP_Bag_SyncToSaveBlock in WriteSaveSectorOrSlot / HandleReplaceSector), sdl2.c StoreSaveFile writes the rest to
"<save>.bag" with gSaveCounter and a checksum of SaveBlock1's pockets, and LoadGameSave takes them back only when both
match. sdl2.c BackUpSaveFile rotates "<save>.bakN.bag" with the backups, and loading tries "<save>.bag" then each
"<save>.bakN.bag", so a .bak renamed back to the save gets its own extra slots. load_save.c (frontier bag swap) and the Wally tutorial bag use gBagPockets. Debug `mp bag`.

## Player stand-ins wear the player's look
Scenes that show the player as another object take Brendan / May by gender: the Battle Tent and Frontier battle
rooms' walk-ins, the multi battle rooms, the Contest Hall, Route 111's fall, Southern Island, and the interview word
screen (easy_chat.c TryAddInterviewObjectEvents). With a look of their own (MP_LocalUsesCustomAvatar) they take
OBJ_EVENT_GFX_MP_LOCAL: MP_PlayerStandInGfxId() in C, `special MP_SetPlayerStandInGfx` at the end of each script's
player-gfx label (it swaps every VAR_OBJ_GFX_ID_n holding the player's own Brendan / May graphics). Actions with no
frames in the look (field moves, fishing, watering, decorating; CREATE builds walk / run / Mach Bike only) keep the
standing frames (mp_avatar.c StandInInfo). Debug: `mp easychat TYPE PERSON` (10 0 = Gabby and Ty's screen),
`mp avatarstate N` (0-7).

## Small adjustments (traded OT, tents, genders, target level)
- Traded Moemon act as if the player were their OT (MP_OtActsAsOwn, mp_gatcha.c): battle_util.c IsMonDisobedient
  treats them as obedient. They keep the traded 1.5x EXP (the owner's call). MoeGatcha prizes (GACHA_OT_ID) keep
  obeying by badges. Debug: `mp giveforeign SPECIES:LEVEL:lead`.
- Battle Tents: battle_tent.c Give*TentPrize calls MP_BattleTentWon (3 MOEGATCHA TICKETS) when the prize is handed
  over; the three lobby scripts `call EventScript_MpTentGatchaTickets` (obtain_item.inc) for the message. A full BAG
  keeps both for later. Debug: `mp challenge 3` then walk into a tent lobby.
- Gender ratios: src/data/pokemon/species_info.h has pret/pokeemerald's genderRatio again (313 species Kai had moved
  to 75% / 87.5% female). Protocol 30 so old and new games never battle together (genders are simulated on every
  game). Debug: `mp gendersample SPECIES` (1000 Pokemon, gender counts).
- Triple target window: the foe's Lv comes from the same Pokemon data as its card (it read gBattleMons), and a
  difference is logged ("target level mismatch", checked by tests/tri_target_level). The vanilla level-up refresh
  only updated battlers 0 and 2; in a Triple battler 4 kept its old level in battle (now MpCoopUpdateBattleMon).
  `mp bhp` lists every battler's battle and party level.

## Weather all at once, lines once
battle_scripts_1.s BattleScript_DamagingWeatherContinues: `mpweathergroup 0` works out every battler's damage
(MpWeatherDamageFor, the old weatherdamage); with two or more hit it prints gMpWeatherAllStringIds ("Everyone was
buffeted by the sandstorm!" / "pelted by HAIL!"), then mode 1 sends every hit battler's hit flash and mode 2 every HP
bar in one go (one waitstate each), then mode 3 walks the turn order for datahpupdate / tryfaintmon / checkteamslost
like the vanilla loop. One hit (or none): the vanilla named loop. `mpweathermsg 0` skips a weather's "continues" line
after its first time (the box is cleared instead), mode 2 the same for the "Everyone" line, mode 1 (weather ended)
forgets; MP_Battle_ResetWeatherLines at battle start. Logs: "battle: weather hits N at once / (named)", "... line
already shown". Only the master runs battle scripts, so co-op and PvP see the same thing. Test: coop_weather.

## Shared world and lights (mp_world.c)
Relay MPR_WORLD_FIRST (220): WS_CHANGE (live, to everyone) and WS_LIST (everything so far, to a friend who just
walked onto the map: MP_World_Update watches friends' state maps). Each game keeps the change list of the map it is
on (a new map starts a new list, so when everyone has left the map is as new, like the game's temp flags). Changes:
WC_OBJ_GONE (specials MP_World_CutTreeGone / RockGone after removeobject in field_move_scripts.inc; receivers set the
template's flag and play cut_tree / rock_smash_break when live), WC_TILE (fldeff_cut.c FldEff_CutGrass snapshots
11x11 metatiles before/after and sends the differences; field_tasks.c AshGrassPerStepCallback sends the ash tile,
receivers show the puff, the soot stays the walker's), WC_BOULDER (field_player_avatar.c TryPushBoulder: final x, y
and direction; receivers walk it if it's one step away, else move it). Applied while the player is free.
Lights: MpState.light = GetFlashLevel() (wire state byte 22). MP_World_PrepareLights (sdl2.c VDraw, before
DrawFrame) turns each friend's light on the same dark map (radius from their flash level, grown/shrunk 2 px a frame)
into per-scanline spans; gba_easy_draw.c / gba_fast_draw.c count a pixel in a span as inside window 0. In a dark
cave, gMpDarkUiPal (the MP_TAG_UI_PAL slot) lets toasts / bubbles / "!" through the darkness (easy draw: their
pixels set WINMASK_OBJ). Debug: `mp lights`, `mp cuttree ID`, `mp smashrock ID`, `mp cutgrass`, `mp worldobj ID`,
`mp tileat X,Y`, `mp ashcount`. Tests: coop_flash, coop_world, coop_world2.
