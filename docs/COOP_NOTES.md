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
and remote followers seen by the other client. Ground effects (mp_followfx.c, 1.18.3): a follower is a plain sprite, so
UpdateSlot hands its step (prev / cur tile, pixels into it, facing, size, owner elevation, hidden) to MP_FollowFx_Update,
which plays the game's own field effects from its tiles with GetAllGroundEffectFlags_*'s rules: tall / long grass
(FLDEFFOBJ templates with our callback, stopping like UpdateTallGrassFieldEffect), sand / deep sand footprints and
ripples (FieldEffectStart with arguments), puddle splash, flowing water, hot springs, sand heap, short grass (sprites
following the follower), and a reflection (CreateCopySpriteAt like SetUpReflection, priority 3; MP_IsReflectionSprite
knows it) in a reflection palette of its own (below). UpdateGrassFieldEffectSubpriority counts followers (MP_FollowFx_GrassSubpriority).
Friends' followers get it too. MOEMON_FOLLOWFXTRACE=1 logs each effect; tests follow_ground, follow_ground_remote.
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
2+ racers, HoF pull once per friend. Achievements (mp_achieve.c, 184, personal, offline too: "av:ID", counters "acnt:NAME"
shared by an id's tiers: "coop1"/"coop25"/"coop100" all count "coop"), DAILY > ACHIEVEMENTS in six tabs (C_*, L/R;
MP_Achieve_CountIn/IndexIn, `mp achmenu TAB`, `mp ach`). Kinds read the save where they can (flags and flag lists FL_*,
game stats, POKéDEX counts and species lists DL_*, types caught, party levels, party + PC count, friendship, money),
so they're retroactive: GrantEarnedSoFar grants everything already earned once per version that adds some ("ach:retro3";
bump it when adding more) with one toast. Counted from now on: "pull" (StartSpin), "triple" (CB2 end, sCoop.count 3),
"emote" (quick pick); feats sr/ssr/ur (best prize of a pull), eventmon, ballstyle (MpBall_Apply, or a styled party
ball), follower, newlook (LOOK differs from the default). Tests ach_retro, ach_hooks, tri_wild (TRIPLE THREAT).
Achievement rewards: tier 0/1/2 = 3/10/20 pulls (mp_achieve.c AchPulls; saves from before get the
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

e-Reader Berries: ITEM_PUMKIN_BERRY..ITEM_TOPO_BERRY are items 377-388 (ITEMS_COUNT was 389, now 399: see "Items from later games"), berry numbers 47-58
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
Level scaling (1.19.0, mp_levelscale.c; OPTIONS > GAME > LEVEL SCALING, gMpConfig.levelScaling / ini level_scaling,
on by default; online the host's, MPR_RULES byte 3, re-sent by MP_SendRoomRules when the host changes it): the team's
level = MP_LevelScale_Top3 (average of the 3 highest-level party Pokémon, eggs left out); in a co-op battle the average
of every seat's (MpState.top3, wire byte 39, protocol 46). Trainers (battle_main.c CreateNPCTrainerParty): the party's
strongest normal level (partyMax) is raised to the team (+BossMargin: Gym Leaders, team admins / leaders, rematches via
MP_Coop_IsBossTrainer and the rival +2..+5 with badges, Elite Four / Champion +5), everyone in the party by the same
amount, never down, the ace never above the badge cap (sCaps 18..65, 100 after FLAG_SYS_GAME_CLEAR) + the margin; then
gMpEnemyLevelBonus (co-op) on top. A raised Pokémon goes through MP_LevelScale_EvolveMon (level evolutions incl.
Wurmple / Tyrogue / Nincada rules, its name and level-up moves; custom-moveset trainers keep their moves, set after).
Wild (wild_encounter.c ChooseWildMonLevel -> MP_LevelScale_Wild: team - 3..6, capped, never below the table's level;
CreateWildMon -> MP_LevelScale_WildMade evolves and remembers personality -> normal level; CreateScriptedWildMon too).
Co-op wild (MP_Coop_SetWildEnemyParty -> MP_LevelScale_CoopWild): the master re-scales each foe for the team (spread
from its personality, no Random), then BoostWild +2. Not scaled: Battle Frontier / Pike / Pyramid, mass outbreaks
(MP_LevelScale_ForgetPending), roamers, Battle Tower etc. Logs "level scale: trainer N ace A -> B (team T, cap C)",
"level scale: wild S LvA -> LvB", "level scale: evolved"; debug `mp landwild` (an encounter from the map's grass table);
env MOEMON_LEVEL_SCALING=0/1. Tests levelscale_wild, levelscale_trainer, levelscale_off, coop_levelscale.
Settings (1.23.0, protocol 49): MP_SCALE_ON / WILD / TRAINERS / BOSS / CAPS / PAST8 (include/mp.h; gMpConfig.levelScaling
and gMpConfig.scale[], ini `level_scaling_rules = W T B C P`, defaults 2 2 0 1 0). WILD: OFF, LOW (a quarter of the way
from the area's level to the team's, minus 0-1), MEDIUM (halfway, minus 0-2: the default; the user found 3-6 below the
team far too strong), HIGH (the old 3-6 below the team). TRAINERS: OFF, BELOW (the ace 4 under the team), EVEN, ABOVE
(3 over). BOSS MARGIN: AUTO (the old +2..+5 by badges, the Elite Four +5) or +0..+8 for every boss. BADGE CAPS on / off.
PAST 8 BADGES: capped until the League then free (the old rule), free, always capped (sCaps[8], 80 after the League).
A wild Pokémon raised past its evolution level stays the area's own (the normal encounter); 1 in 10 (personality >> 8
% 10 == 0, so every game in a co-op battle agrees) is the evolved form, a rare encounter (RareEvolvedForm; log
"level scale: a rare evolved form"; env MOEMON_SCALE_RARE=1 always / 0 never, tests).
Who sets them: playing alone, OPTIONS > GAME > LEVEL SCALING (A opens its page: GAME > LEVEL SCALING header, the six
rows, BACK; L / R off on the page; log "level scaling rule N: V"); in a room, its host: HOST A GAME > GAME RULES
(mp_setup.c SCR_RULES, replacing the TEAM PADDING / BATTLE SPEED rows: those two plus the six; logs "setup: game
rules", "setup: rule N now V"). A friend's rows show the host's values, locked. MPR_RULES = [id, padding, anim,
scaling, the five]; MP_LevelScaleRule reads the host's while in someone else's room (sRoomScale), ours otherwise.
Env MOEMON_LEVEL_RULES="W T B C P" (tests). Tests levelscale_options, host_rules.

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
Mixing rounds (mp_linkx.c UpdateMix): once every record has arrived (applyPending) the round is never dropped: neither
the 30 s timeout nor a friend leaving cancels it; it waits until we're free on the field (test mix_leave: menu open
while the friend quits).
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
(MP_Created_Gfx). The back frames (87x98 in the kit) keep their size: one fixed 96x64 window of them, moved down so the
head is where Brendan's is (BuildBack: BACK_KIT_X / BACK_KIT_Y); every part fits across, so the throwing hand stays in
the picture (a 64x64 window used to cut it, 1.5.0). The players' back picture slots (TRAINER_BACK_PIC_MP_*) are all
96x64 (mp.h MP_BACK_PIC_*): a 64x64 and a 32x64 sub-sprite, set up in CreateSpriteAt by MP_Avatar_OnSpriteCreated
(affine off: AddSubspritesToOamBuffer reads flip bits from the matrix number). 64x64 pictures (Emerald's, sprite packs)
are placed MP_BACK_PIC_LEFT in, so they show exactly as before. Debug: `mp created HEX` wears a CREATE recipe. Pictures: graphics/moemon/creator/parts.bin (tools/moemon_creator/build.py KIT_DIR; the kit isn't in
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
(MP_TripleSwitchRange / MP_IsPartyIdOnSide). Cards: src/mp/mp_tricards.c over the frame (no owner strips since the new card art),
healthboxes parked below the screen. Party screen: PARTY_LAYOUT_TRIPLE (party_menu.c; gPartyMenu.layout is 3 bits now), drawn
from the user's mock-up (docs/user_images/25.png, tools/moemon_tricards/party_mock.png): the three actives in 10x5 panels on the
left (ours, then the allies'), their reserves in the wide boxes on the right in the same order. MpBlitTripleArt builds both
from the stock box art pixel by pixel (panel = stock rows 2-5, 15-36, 38-51; wide = one pixel higher), and a caller's "tile
row" to redraw stands for a text band (sTriplePanelBands / sTripleWideBands). Actives not selected get the orange inner edge
(color 6). Order MP_TripleFixPartyOrder (actives 0-2, reserves 3-5; allies' slots 1,2,4,5 gold). Cursor:
UpdatePartySelectionTripleLayout. Test tri_party checks the screen against the mock-up pixel for pixel (check_party.py).
Second custom back pic slot TRAINER_BACK_PIC_MP_PARTNER2. Debug: `mp triple S:L S:L S:L [...]` (local, one player
commands all three), MOEMON_TRIPLEDEBUG=1 (logs; card visibility). Tests: tri_wild, tri_trainer, tri_catch, tri_heal,
tri_abort, tri_levelup, tri_faint, tri_party (MODE=3, tools/test/runN.sh). Gotcha: anything that does `battler ^ BIT_FLANK` or
assumes positions 0-3 (reshow_battle_screen.c, shadows, Cmd_monbg) breaks for 4/5 (sprite 0 = battler 0's healthbox).
Card art: the user's pixel mock-up (tools/moemon_tricards/cards_mockup.png; gen.py -> src/mp/mp_tricard_gfx.h), a
74x22 body with a tail toward the type icons (foe: right, team: left; the team card's top-left corner is one pixel
rounder, as drawn). Frames are stored as color roles (TC_*) and colored per frame through MP_FadeFit against the
TAG_TRICARD_PAL palette, so there's no 16-color limit. Name at (4,3) from the body, gender 1 pixel after it (the
original healthbox's colors), level at (4,11), or the original game's 20x8 status tag over it (status.png colors);
the HP bar (hpbar.png with a smaller HP label, the original green/yellow/red/empty rows and thresholds on a 48-pixel
scale) runs from 1 pixel after the level or tag to the ring, so it is 2 pixels shorter with a tag. Fill blends at the
card's opacity, the frame at +40, the rest is solid. Placement: foe bodies at x 1, y 1/22/43; team bodies at x 165,
y 48/69/90. Debug: `mp bstatus B STATUS1 [HP]`, `mp double S:L S:L` (local wild double).
Cards: the HP bar follows MoveBattleBar's return value (battleBars[].currValue is fixed-point when max HP < 48);
same-named Pokémon on a side get A/B/C left to right (MP_TriCards_Letter, also in the target box). The cards are drawn
over the finished frame, so MP_TriCards_Composite paints the battle emote pick and bubbles again on top of them
(MP_Emote_BattleOverlaySprites / MP_BattleChat_OverlaySprites). Battle bubbles keep the tail on the speaker and slide
the box back on screen (Bubble.boxShift) instead of moving the whole bubble.
Shared pools (the trainer's side, a local debug triple's player side): Cmd_openpartyscreen (BS_FAINTED_LINK_MULTIPLE_1)
asks battlers that fainted together one after another, so it counts the replacements already promised per pool and a
battler only gets one while MP_TripleCountReplacements still has one left over (else it's marked absent). The AI picks
later, in each opponent controller; asking one too many made it pick slot 6, past gEnemyParty: a BAD EGG out of
WATTSON's party (OpponentHandleChoosePokemon now never returns past the party). Cmd_forcerandomswitch (ROAR /
WHIRLWIND) has its own triple branch: the target's MP_TripleSwitchRange, minus anything out on its side. The AI's own
switches (battle_ai_switch_items.c: ShouldSwitch, Wonder Guard / absorb / type helpers, AI_TrySwitchOrUseItem's fallback)
use MP_TripleSwitchRange and skip MP_IsPartyIdOnSide too (they took the halves of three, which hold another battler's
Pokemon), and the fallback never ends past the range. Tests:
tri_wattson (one SURF knocks out all three, two left), tri_explode (the reported case: VOLTORB's SELF-DESTRUCT with
MOEMON_DEBUG_FOE_HP=1 and MOEMON_DEBUG_FOE_MOVE=2), tri_roar (WHIRLWIND only); MOEMON_PARTYTRACE=1 logs both parties,
gBattlerPartyIndexes, monToSwitchIntoId and absent battlers on every change, tools/test/check_partytrace.py checks them.
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

## Catch EXP (H3)

A caught Pokemon gives EXP like a faint, scaled by the damage dealt: (maxHP - HP) * 100 / maxHP, at least 10%
(a full-HP catch) and 100% at most (data/battle_scripts_2.s BattleScript_MpCatchExp, called right after "Gotcha!" in
BattleScript_PrintCaughtMonInfo, BattleScript_MpCoopBallCaught and ...GoOn; GENESIS / EPOCH BALLS use the same
scripts). VARIOUS_MP_CATCH_EXP sets sMpCatchExpPercent (log "catch exp: hp H/M, P% of a faint"); Cmd_getexp scales
calculatedExp by it before the co-op split, so in co-op everyone gets a share as after a faint, and leaves the music
alone (MUS_CAUGHT is playing, no wild victory song); state 6 clears it. getexp's own rules still apply (no EXP in the
Safari Zone or link battles; EVs too). Level-ups from it evolve after the battle: FreeResetData_ReturnToOvOrDoEvolutions
and mp_coop.c OutcomeEvolves accept B_OUTCOME_CAUGHT as well as WON. Tests: catch_exp (solo, full HP = 10%, Torchic
evolves), coop_catch_exp (lag, 1 HP = 95%, both evolve).

## BATTLE FLAG phrases and prize (H4, protocol 40: mp_trainer.c, mp_battle.c, item_use.c)

Using the flag (ItemUseOnFieldCB_BattleFlag -> MP_Trainer_UseFlag) turns the item task into Task_FlagMenu: "Set phrases,
or plant?" PHRASES / PLANT / CANCEL. PLANT calls item_use.c MP_BattleFlag_Plant (the old flag message, then trainer
mode). PHRASES lists CHALLENGE / WIN / LOSE / DONE; a pick fades out and opens DoEasyChatScreen
(EASY_CHAT_TYPE_BATTLE_START / WON / LOST) on gSaveBlock1Ptr->easyChatBattleStart / Won / Lost (the save's own phrases,
also the Battle Tower's: defaults from InitEasyChatPhrases); its exit callback returns to the field with
MP_Trainer_FieldCB_Phrases, which reopens the list (sPhraseCursor kept). DoEasyChatScreen calls ResetTasks, so the
menu task dies and is created again on the way back.

Handshake: MPR_ARRIVED / MPR_BATTLE_GO carry money + 3 x 6 u16 words (start, won, lost; ReadPhrases fills missing
words with EC_EMPTY_WORD, bad ones print as ??? through CopyEasyChatWord). On ARRIVED the target answers GO and both
games show the trainer's CHALLENGE ("NAME: words", MP_ShowFieldMessageFitted wraps it; states SP_PHRASE /
TM_PHRASE). A box closes on A / B or 3 s after its text is out; then MPR_PHRASE_READY goes to the other side, and a
side starts the battle (MP_Battle_Start + MP_Battle_SetOppPhrases) once its box is closed and the other's READY came.
An empty CHALLENGE shows no box and sends READY at once. Cancels / leaving close the box (CloseChallengeBox); a side
that never gets READY gives up after 20 s. The result message (CB2_MpReturnFromBattle) adds a page with the other's
LOSE (we won) or WIN (we lost) phrase (AppendOppPhrase; no page for an empty one or a tournament match). The winner
still gets oppMoney / 10 (the loser's money when the battle started) and a ball; the loser no longer pays it.
Harness: mp useflag (field only), mp phraseclear N, mp phrasedefault, mp phrases, mp money (no number: print), mp
easychat TYPE (the scripts' way in). Tests: flag_phrases (check.py: the prize and the loser's money), pvp_battle
(no CHALLENGE: no box).

## OPTIONS in tabs (H8, option_menu.c; START > OPTIONS and the title screen's OPTIONS)

The game's own options screen (its frame, palette, fonts, red / green choices, the WIN0 highlight bar) rebuilt as a
row table: BuildRows fills sRows (opt, arg) for the tab (TAB_GAME / DISPLAY / CONTROLS / COOP) or a friend's page
(sFriendPage = 1 + player id); Choices / Current / Change / RowLabel per OPT_*. Rows with 2-3 short choices show
them all (the one on in red, like the originals); longer lists show the one on, centered (FONT_SMALL_NARROW if long);
a long label is drawn narrow and its choices start after it (sLabelEnd). Seven rows show (sTop scrolls, the scroll
arrows are made again per tab) and a hint line under them says what the row does. L / R switch tabs (checked before A,
so BUTTON MODE L=A doesn't also press A), B / START / CLOSE leave (BACK on a friend's page). Values apply live: the
save's options straight into gSaveBlock2Ptr (FRAME reloads its tiles), display / pad through the platform API, co-op
into gMpConfig; leaving saves moemon_display.ini, moemon_controls.ini and the co-op ini, calls MP_MonArt_Changed if
CREATURES changed, and on the map plays the right song if BIKE / SURF MUSIC changed. ShownKey (every row's value, the
pad state, a friend's shown size) redraws when anything changes outside (F11, a controller, a friend coming or
going). START no longer has SETTINGS (display_menu.c is gone) and CO-OP no longer has SPECTATE (both moved here).
Rows: GAME = the originals + TYPE ICONS / NAME CASE / CREATURES + BIKE MUSIC / SURF MUSIC; DISPLAY = FULLSCREEN, WINDOW
SCALE, SCALING, FILTER, VSYNC; CONTROLS = CONTROLLER (A: press a button on it), PAD BUTTONS, STICK RUN, RUMBLE, BUTTON
ICONS; CO-OP = FRIENDS' SCREENS, a row per friend, NEW FRIENDS, AUTOSAVE (0 / 5 / 10 / 15 / 20 / 30 / 45 / 60 min),
FLAG RANGE (1-8), MOENAV CALLS.
BIKE MUSIC / SURF MUSIC (gMpConfig.bikeMusic / surfMusic, ini bike_music / surf_music): bike.c GetOnOffBike and
field_effect.c FldEff_UseSurf skip the song change; overworld.c MpRideMusic turns MUS_CYCLING / MUS_SURF into the map's
own song in Overworld_PlaySpecialMapMusic and keeps the song playing for MUS_UNDERWATER; TryFadeOutOldMapMusic doesn't
fade out when diving (sMpSeaSong remembers the sea's song); TransitionMapMusic keeps the current song for MUS_UNDERWATER.
BUTTON ICONS (gamepad.c sIconSet, moemon_controls.ini button_icons): Gamepad_IconFamily returns the set picked, else
the last device. MOENAV CALLS (gMpConfig.moenavCalls, ini moenav_calls = all / battles): match_call.c
SelectMatchCallTrainer refuses a random call unless the trainer would ask for a battle (same route and eligible, or
ShouldTrainerRequestBattle, the day's fixed draw); scripted calls (StartMatchCallFromScript) never go through it.
Harness: `mp optionstab N`, `mp iconfamily`, `mp matchcalltest N`, `mp mountbike N` (the game's own GetOnOffBike),
`mp ridemusic BIKE SURF`, `mp music [play]`. Test clients each get their own controls ini (run1/run2/runN.sh
MOEMON_CONTROLS_CONFIG). Tests: options_tabs, music_options, spec_menu (the CO-OP tab with real keys).

## Ball entrance and recall effects

Per Moemon, next to the ball style: MON_DATA_MP_BALLFX (PokemonSubstruct0's spare `filler` u16: form 0-3, particles
4-7, energy color 8-11, recall 12-14), MON_DATA_MP_BALLFX_EXT (8 bits: the markings byte's top 4 bits and the spare
ribbon bits; bit 0 form bit 4, bit 1 particles bit 4, bits 2-3 color bits 4-5, bits 4-6 recall bits 3-5; 0 in Moemon
from before, so they keep their effect; MON_DATA_MARKINGS only touches the low 4 bits) and MON_DATA_MP_BALLFX_SOUND
(BoxPokemon's spare flag bits). MP_BallFx_Pack / Unpack: 24 bits (base | ext << 16); MP_BallFx_SetBoxMon writes all
three. All in the Pokémon's data, so trades, the shared PC and link battles carry them. src/mp/mp_ballfx.c (engine,
names, presets), src/mp/mp_ballfx_editor.c (editor), mp_ballunlock.c (which pieces a save may pick), art from
tools/moemon_ballfx/gen.py -> src/mp/mp_ballfx_gfx.h. Debug: `mp ballfx SLOT F P C R S` / `mp ballfx SLOT preset N`,
`mp fxedit SLOT`, `mp fxget SLOT`, `mp party` prints fx. Logs: "ballfx: entrance form F part P color C",
"ballfx: recall R color C", "ball fx: preset NAME" (editor).

Pieces: 27 forms (DEFAULT + 11 + 15 newer), 31 particle sets (DEFAULT, NONE, 14 + 15 newer), 37 recalls (DEFAULT + 6
+ 30 newer), 41 energy colors (the ball colors, mp_ballstyle.c). The ones from MPFX_FORM_FIRST_EARNED (12),
MPFX_PART_FIRST_EARNED (16), MPFX_RECALL_FIRST_EARNED (7) and MP_BALL_FIRST_EARNED (11) on are earned
(MP_BallUnlock_Has): half by an achievement (mp_ballunlock_ach.h: {kind, index, achievement id}; it counts as soon as
the achievement is done, so old saves have theirs, and the achievement's reward text names the piece), the other half
are MOEGATCHA prizes (MP_BallUnlock_Give, save key "fxu:<c|f|p|r>:<index>"). mp_gatcha.c prize kind K_FX (POKE BALLS
banner R / SR / SSR, STANDARD SR): the piece is picked at the pull among MP_BallUnlock_PrizeCount (45), one already
had is a MOEGATCHA TICKET ("gatcha: ball piece N already had: a ticket"). Pickers step only through pieces the save
has (MP_BallUnlock_Step); the editor's PRESET list skips a ready-made preset with a piece the save hasn't earned
(MP_BallFx_PresetUnlocked). Only choosing is limited: other players' and traded Moemon play whatever they carry.
`MOEMON_FX_UNLOCK_ALL=1` unlocks everything; `mp fxunlock KIND INDEX` / `mp fxunlock all` (all prizes) print what's
left; `mp gatchafx N` makes the next pull prize N. Tests: fx_storage, fx_preset_lock, fx_preset_all, gatcha_fx.

MY PRESET 1-6 live in the save's .coop file ("fxpreset1" = bit 30 used | bit 29 layout 2 | sound << 24 | the 24-bit
pack; saved before layout 2: bit 30 | sound << 16 | the 16-bit pack; MP_BallFx_GetUserPreset / SaveUserPreset /
WhichUserPreset, MP_BallFx_EffectName names a mix for the editor and the summary). Editor rows SAVE AS (ASK_REPLACE
when the slot holds another mix) and APPLY TO PARTY (ASK_PARTY; party Moemon that aren't eggs); the questions and
"saved" notes use the info box. Tests: fx_presets, fx_mypreset6.

Art: indexed pixels with one palette, sFxPalette[FX_PAL_COUNT]: 1-15 the base colors, 16/17/18 the energy color and
its light / dark shade (filled in per particle), 19 on the colors of the game's own battle animation art, which gen.py
imports from graphics/battle_anims/sprites (imp / frames_of: Attract's hearts, Sing's notes, Razor Leaf, Ember, Surf's
splash and water column, Pay Day's coins, Petal Dance, Focus Energy recolored to the energy color, Snore's Z, Ice
Beam's ice chunk, ...). The palette is faded once a frame (sPalNow). Particle animation: nFrames / frameTime from art,
framePhase, PF_ONCE (keep the last frame), PF_PINGPONG; PF_FLIPX mirrors; MOVE_BOUNCE (bounces on the ground line ty,
then rests), MOVE_SETTLE (eases to tx, ty by frame dRadius, then stays). Bodies can turn (BodyTurn: scale + rotation
in the OAM matrix). FieldScale shrinks the big art for followers. With the AUTO sound some newer recalls play their
own sound (RecallSong) after the game's ball sound.

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

Followers: MpState.followerFx (24 bits: the wire state's byte 23 carries bits 16-23) + followerFlags bits 1-3 sound,
4-5 sent-out count, 6 recall playing (MP_WIRESTATE_SIZE 24, protocol 31).
A count change while the follower is around: a mini ball (particle) is tossed from its owner to its
tile and it comes out with its effect (DEFAULT = MPFX_FORM_VANILLA). Called back: sLeavingState keeps it out until its
recall has played. Tests: fx_coop, fx_triple, fx_switch, fx_follow, and with the newer pieces (their extra bits)
fx_coop_new, fx_trade, fx_follow_new.
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

## PMD Collab followers (mp_pmd.c, mp_bigsprite.c; data from tools/moemon_pmd)
Followers (yours and friends') and Garden Pokémon are PMD Collab overworld sprites (github PMDCollab/SpriteCollab,
CC BY-NC 4.0; artists in dist_extras/PMD_CREDITS.txt, packaged). tools/moemon_pmd/import.py SPRITECOLLAB_DIR writes
graphics/moemon/pmd/pmd.bin (one raw-deflated chunk per Pokémon: national dex 1-386, variant 0 normal, 1 female, 2/3
shiny / female shiny when the shiny colors need pixels of their own), src/data/mp_pmd.h (the index) and the credits.
A chunk ('PMD2'): bpp 4 (8 for the few shiny sheets with 16+ colors: all colors kept), palette and shiny palette (one
set of pixels when the (normal, shiny) color pairs fit in 15), 29 animations (PMD_ANIM_*), each frames x 8 directions
(rows: down, down-right, right, up-right, up, up-left, left, down-left; 1 row when the sheet has one) with durations in
60ths; a frame = the picture cropped to its pixels with its top-left from the ground point (the sheet frame's center,
where PMD draws the shadow). pmd.bin goes in with __asm__ .incbin (mp_pmd_data.c; .rdata on Windows; json_data_rules.mk
makes the object depend on it): INCBIN would spell 13 MB out as C text. mp_pmd.c inflates a sheet (stbi_zlib) when it's
first shown, shares it, frees it 5 s after the last view lets go (CACHE_MAX 48).
Big sprites (PC only): struct Sprite has mpBig. AddSpriteToOamBuffer writes such a sprite's entry in its sorted place with
shape 3 (prohibited on the GBA) and tileNum = an index into a side list (mp_bigsprite.c: picture, exact position,
flips, an affine pivot); the list is double-buffered and swapped in LoadOam, so it always matches the OAM shown.
gba_easy_draw.c DrawBigSprite draws the whole picture on each line through the same PutObjPixel as other sprites
(priority, blending, windows, battle weather), so depth with the player, NPCs and grass is the game's own. Colors 1-15
are the sprite's palette slot (fades, weather, Stadium hue); 8-bit pictures' colors 16+ follow the slot's fade
(MP_FadeFit_FromColors, MP_Pmd_Tick). Affine: a reflection copy turns around its own point (mirrors at the feet:
followfx puts it at ground + 6, the bottom of the tile, as the player's); anything else around the picture's middle
(ball effects: mp_ballfx.c BigFrame / BodyHeight / MonX / MonY / MonPixel read the big frame).
Placement: the ground point is PMD_GROUND_DY (8) below the old 32x32 frames' center (the tile's middle). Depth: Attach
zeroes centerToCornerVec, so MP_Pmd_SetSubpriority reads the depth as the old frames had it (centerToCornerVecY -8 for
the call); before 1.22 a Pokémon right below its Trainer shared their depth band half the time and was drawn behind.
A big follower (MP_Pmd_Reach, half its picture's bigger side, over 18) is drawn up to 16 pixels farther from its Trainer
(pushX / pushY, away from their tile, 1 pixel a frame; the depth from where it stands). Facing: the 4 ways only (1.22:
no diagonal rows or turning steps; SPIN looks down, right, up, left). Hops (HOP, LEAPFORTH, ROTATE, POSE, TUMBLE): the
sheets' rise (made for a top-down dungeon) is cut to a third (MpBigSprite.lift, UpdateLift: the lowest frame's bottom
is the ground). Fast (running, biking): a walk whose whole cycle is over 30 frames gets an extra tick every other frame;
quicker ones keep their pace (1.21 doubled every walk: some ran far too fast).
OPTIONS > GAME > FOLLOWERS (gMpConfig.followerSprites, ini follower_sprites, MP_PmdWanted): DEFAULT the original Moemon
sprite for followers on the map and PMD for the Garden's Pokémon, PMD both, ORIGINAL both; switching rebuilds the
sprites at once (MP_Follower_Update / Draw compare pmdOn). Tests: MOEMON_FOLLOWER_SPRITES (the starting value),
`mp followsprites N`; follow_sprites, pmd_depth. Standing with nothing to do: a DEEPBREATH every 12 s,
SIT after 40 s. A sheet missing an animation plays the nearest it has (sFallbacks: SLEEP -> LAYING -> EVENTSLEEP -> SIT ->
IDLE, EAT -> NOD -> IDLE, ...; WALK last, every sheet has it). No PMD sheet: the Moemon frames as before.
Poses -> animations (LIFE_POSE_*, MP_Pmd_Animate; Moemon sprites show MP_LifePose_Base's nearest old pose):
NONE IDLE (DEEPBREATH, SIT) | SIT SIT | NAP SLEEP (facing down) | HOP HOP | SHAKE EAT (rummaging, digging, sniffing) |
SPIN IDLE turning round | LOOKUP LOOKUP | EAT EAT | SPAR ATTACK / SHOOT / STRIKE (dex % 3) | NOD NOD | JOY POSE |
TWIRL ROTATE | TRIP TRIP / TUMBLE | HURT HURT / CRINGE | CHARGE CHARGE / SHOCK (dex odd: the second) | FLOAT FLOAT |
LEAP LEAPFORTH | PULL PULL | BREATH DEEPBREATH | LAY LAYING.
Where they come from: foraging and munching foliage EAT; basking LAY; storms CHARGE; sparring SPAR; the new-body stumble
TRIP; a squabble HURT; greeting a friend it's fond of / the double take / greeting its Trainer in the Garden TWIRL;
leveling up (back on the map: mp_life_field.c lastLevel) TWIRL; talking NOD; petting (warm) and praise, keepsakes,
being loved, a proud evolution JOY; fed EAT (a flavor it dislikes HURT); after a nap BREATH; partway through a hint,
with its Trainer within 3 tiles, PULL (facing them); Ghost / Psychic types investigating (field) or looking about
(Garden) sometimes FLOAT by the water; Garden pursuits end in LEAP / JOY, teaching NODs, a milestone LEAPs.
Network (protocol 47): poses 7-19 travel as before (wire state lifePose; Garden actors: byte 11 = dir (2 bits) | pose
<< 2), shiny in followerFlags bit 7 and the Garden actor's byte 6 bit 1. Debug: `mp pmd [SLOT]`; MOEMON_DEBUG_PARTY
"280:15*" = shiny. Tests pmd_follower, pmd_remote (and every follow_*, fx_follow*, garden_*, life_* test runs with them).
A very tall follower trailing south of its Trainer covers them (the depth order is the game's: it's in front).

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
untinted). Reflections of sprites in co-op palettes (16+, or a borrowed slot holding a co-op tag: friends, followers)
use mp_followfx.c MP_ReflectionPalette (field_effect_helpers.c ReflectionPaletteFor, SpriteCB_Reflection): a co-op slot
tagged 0x5B00 + the source slot, holding the source's colors washed 42% toward white (MP_ReflectionTint, the tint of
LoadPlayerObjectReflectionPalette for the player's own looks), rewritten each frame from the source (its fade fitted
with MP_FadeFit_FromColors). The game's palettes keep their hand-made reflection palettes. MP_ReflectionPalettes_Sweep
(MP_FrameUpdate) frees the ones no sprite shows; MP_MakeRoomForPalette gives them up first (the reflection then falls
back to the source's colors). Log "reflection palette: slot"; test follow_reflect_remote.
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
the triple cards, layered as in the user's triple mock-up: the secondary type behind (2 pixels out from the box's
outer ring, 2 below its top), the primary in front (6 out, 6 down); one type = the front icon only. Outer ring of a
healthbox: right of a foe's at box sprite x + 60, left of yours at x - 23, top y - 14 (measured, same in singles and
doubles); in triples the card body (MP_TriCards_Place). They follow the box's
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

gBagPockets point at mp_bag.c's arrays (MP_BAG_*_COUNT in constants/global.h: 224 items, 64 of the others; stacks
MP_BAG_ITEM_CAPACITY 999, BAG_ITEM_CAPACITY_DIGITS 3; the Battle Pyramid's u8 bag keeps MAX_BAG_ITEM_CAPACITY 99).
SaveBlock1's pockets are untouched in size: save.c copies the first slots there before every write
(MP_Bag_SyncToSaveBlock in WriteSaveSectorOrSlot / HandleReplaceSector), sdl2.c StoreSaveFile writes the rest to
"<save>.bag" with gSaveCounter and a checksum of SaveBlock1's pockets, and LoadGameSave takes them back only when both
match. sdl2.c BackUpSaveFile rotates "<save>.bakN.bag" with the backups, and loading tries "<save>.bag" then each
"<save>.bakN.bag", so a .bak renamed back to the save gets its own extra slots. load_save.c (frontier bag swap) and the Wally tutorial bag use gBagPockets. Debug `mp bag`.

## REJOIN and remembered invite codes (mp_setup.c)

OnRoomJoined saves the room (last_room, last_room_key: the room's key, not its password) and where it is:
last_room_address = "" (the online lobby), "home" (hosted on this PC: no REJOIN, its server ends with the session) or a
friend's own server's "host:port" (MP_Lobby_ServerAddress: the server we're on when it isn't the online lobby: an
invite code, a typed address, a home-network game). MAIN_REJOIN connects there and joins with the key, so a game joined
with an invite code (and a password) comes back without either. ENTER CODE OR IP starts with last_join_code (the code
that got us in, saved only once a join succeeds); a locked room that is the last one we were in is joined with its
key (no password), and a wrong key (the host changed the password) asks for the password. A friend's server that
doesn't answer on REJOIN says so and points at ENTER CODE OR IP (its address may have changed). Test:
tools/test/rejoin_smoke.sh (b1 address + password, b2 REJOIN, b3 the remembered code).

## Item descriptions from the Gen 4 games (mp_itemdesc.c)

src/data/text/item_descriptions.h holds the Gen 4 games' texts (HEARTGOLD / SOULSILVER, else PLATINUM, else DIAMOND /
PEARL; a TM's or HM's is its move's Gen 4 text): tools/moemon_items/gen_desc.py from PokeAPI's csv tables, in the
game's style (MOéMON, POKé, names in capitals). Items those games don't have keep their texts. The texts have no line
breaks: MP_ItemDesc_Print wraps one to its window's width (the normal font if it fits the window's lines, else the
narrow one) and, where it still doesn't fit (the narrow boxes of shops, the PC's item storage and the BATTLE PYRAMID
bag), shows a page of lines at a time (MP_ItemDesc_Update from MP_FrameUpdate, every 2.5 s, the first page 3.5 s).
The pager stops as soon as the window holds anything it didn't draw (a checksum of the window's pixels), the screen
changes or another description is printed. Callers: item_menu.c, shop.c, player_pc.c, battle_pyramid_bag.c,
pokemon_storage_system.c. Debug `mp itemdesc`. Test item_desc.

## The BAG's eight pockets (HGSS-style screen)

The BAG shows eight pockets in this order: ITEMS, MEDICINE, POKé BALLS, TMs & HMs, BERRIES, BATTLE ITEMS, KEY ITEMS,
FAVORITES (MP_BAGVIEW_*, include/mp.h). They're views of the save's five pockets, which stay as they are:
MP_Bag_ViewOfItem puts an ITEMS-pocket item in MEDICINE (battle usage MEDICINE, or the field functions Medicine / PPUp /
PPRecovery / RareCandy / SacredAsh), BATTLE ITEMS (battle usage OTHER: X items, GUARD SPEC., POKé DOLL...) or ITEMS
(the rest); the other pockets are one view each. FAVORITES holds every item the player marked (MP_Bag_IsFavorite:
MP_SaveData "bag:fav%d", 31 items a key), from any pocket, in pocket order. MP_BAG_ITEMS_COUNT is 224: room for every
ITEMS-pocket item (201) at once.

item_menu.c: gBagPosition.pocket, cursorPosition[] and scrollPosition[] are the VIEW's (BAG_VIEWS 8). sViewSlots[v]
holds each view's stacks as pointers into the save pockets; UpdatePocketItemList (whatever pocket it's given) compacts and
sorts all five pockets, then rebuilds every view. ViewItem / ViewQuantity read the view under the cursor; anything that
depends on the kind of item uses GetItemPocket(item) (the context menu, TM data, berries, holding rules), never the
pocket shown. GetItemListPosition (item.c RemoveBagItem's hint) maps the cursor back to its save slot (SlotPlace).
SELECT moves an item within its view: it goes before the item it's dropped on in the save pocket (after the view's last
one at the end), the others keep their places, and that save pocket's sort order becomes MANUAL; START sorts the save
pocket (ITEMS, MEDICINE and BATTLE ITEMS share one order). FAVORITES has neither. Every pocket's item menu has FAVORITE /
UNFAVORITE (ACTION_FAVORITE, its label set by OpenContextMenu; 2x2 menus fill their blank, ITEMS gets a 2x3 one); the
item menus sit over the list (15 tiles wide for UNFAVORITE). Wally's tutorial opens on MEDICINE (the POTION) and goes
right to POKé BALLS. berry_tag_screen.c follows the BERRIES view with BAG_LIST_ROWS rows.

The screen: tools/moemon_bag/gen_bag.py fits docs/user_images/31.png (256x192) to 240x160 and writes
src/mp/mp_bag_gfx.h (a full-color background, its colors, the eight pocket icons). mp_bagui.c MP_BagUI_Composite (sdl2.c,
after MP_Candy_Composite) draws it under the game's layers wherever nothing else drew (CB2_BagMenuRun only), plus the
pocket shown picked out (red icon, red frame) and a red frame on the list's cursor row (gray on an item being moved);
the fade comes from the backdrop color (white). BG2, the old background, is hidden; palette 1's colors 1-3 are the
design's text colors (dark / white / shadow; 7 is black). Windows: the list x14 y1 15x12 (5 rows, BAG_LIST_FIRST_Y 9,
no cursor arrow), the description x5 y14 24x6 (white text, COLORID_DESC), the pocket name on the plate x1 y11, the sort
hint left of the bag. Emerald's BRENDAN / MAY bag (per gender, its pocket frame = the view's save pocket, FAVORITES =
ITEMS) at 74,41; the item icon in the description box. No rotating ball and no switch arrows: L / R / LEFT / RIGHT go
straight to the next pocket, round and round. Test bag_pockets (`mp bagview V`).

## Player stand-ins wear the player's look
Scenes that show the player as another object take Brendan / May by gender: the Battle Tent and Frontier battle
rooms' walk-ins, the multi battle rooms, the Contest Hall, Route 111's fall, Southern Island, and the interview word
screen (easy_chat.c TryAddInterviewObjectEvents). With a look of their own (MP_LocalUsesCustomAvatar) they take
OBJ_EVENT_GFX_MP_LOCAL: MP_PlayerStandInGfxId() in C, `special MP_SetPlayerStandInGfx` at the end of each script's
player-gfx label (it swaps every VAR_OBJ_GFX_ID_n holding the player's own Brendan / May graphics). Actions with no
frames in the look are drawn on Brendan's / May's body (see "Action sprites for every look" below). Debug:
`mp easychat TYPE PERSON` (10 0 = Gabby and Ty's screen), `mp avatarstate N` (0-7).

## Action sprites for every look (mp_hybrid.c)
NPC looks, RS BRENDAN / RS MAY and CREATE trainers have no bike, surf, field move, fishing, watering or decorating
frames. mp_avatar.c ActionKind picks how a look shows each state: ACT_HYBRID builds them at run time from Brendan's
(May's for a girl: ActionGender, the catalog's gender / the recipe's) frames for that state with the look's head
(the rows above its neck: 23 in Kai's 16x32 sprites, 9 in 16x16 ones, the kit's band for CREATE; Kai sprites hold
their arms beside the head, so skin there is left out) and the body's clothes recolored into the look's palette
(BodyLut: skin 1-4 to the look's skin, Brendan's color groups to the look's body colors by size, the rest to the
nearest color). The result uses the look's own walking palette (tag MP_LOCAL), so no palette merging. Where
Brendan's head sits in each action frame and which pixels are his head is tools/moemon_hybrid/build.py ->
src/mp/mp_hybrid_data.h (template match of the walking head, plus a taller top row and the hat outline; fishing
frames keep the rod). CREATE keeps its own Mach Bike (ACT_KIT), Pokemon NPC looks ride / surf in their own frames
(ACT_OWN) and stand for the rest (ACT_STANDIN), drawn sprite packs are unchanged, underwater is the shared dark
shape. A BRENDAN / MAY look on the other gender's save gets that body's sprites with their own palette tag
(MP_BODY 0x1133). Remote players build the same for bikes / surf (BuildStateInfo). Debug: `mp sprite NAME` (ini
name), `mp actiondump FILE` (every state's frames + palette), `mp bike 2` (Acro Bike). Test: look_actions.
Fixed with it: the player's sprite keeps the tile block it was made with when it changes state, so every local
graphics info is sized for 32x32 frames (LOCAL_TILE_BYTES; a 16-wide look's bike frames overwrote other sprites);
MP_SpriteRefreshImages takes old and new block sizes (the object's info size, which event_object_movement.c
allocates and frees). The surf blob draws with colors 5, 6, 15 of the surfer's palette: a look whose palette isn't
Brendan's / May's gets a blob palette of its own (MP_Avatar_SurfBlobPalette, tags 0x53C1/2). The player's water
reflection takes the look's colors, washed out (LoadPlayerObjectReflectionPalette for the MP tags).
1.17.2: a friend's blob hides with their sprite (UpdateSurfBlobFieldEffect, MP objects only), and so does an emote
icon over them (trainer_see.c SpriteCB_TrainerIcons) and a bubble over a door tile (mp_emote.c, from its screen
place). Off the screen the game hides object sprites, but a plain sprite's place wraps (OAM x every 512, y every
256): a friend surfing about 28 tiles away had their blob show, riderless, on the other side of the view. The
remote's blob is checked to still be theirs (BlobIsOurs: the blob callback, following their object, never the
player's) before it is reused, recolored or destroyed, and destroyed before a respawn. Test surf_remote.

## Characters from Moemon's other branch (F1, protocol 39: mp_chars_gfx.h, tools/moemon_chars)
The user's sprite zip (113 characters, 284 trainer pictures, 115 back pictures) is imported by
tools/moemon_chars/import.py into graphics/moemon/chars/*.bin + src/mp/mp_chars_gfx.h. 000 Brendan and 001 May are
pixel for pixel the game's own, so they're skipped (the player's default is unchanged); front_000-092 are Emerald's
pictures and 64 more are the FireRed/LeafGreen ones already in graphics/moemon/ext (skipped); 127 new fronts (20 of
them nobody's: HeartGold/SoulSilver characters named in FRONT_NAMES) and 107 new backs (5-frame ones keep 0, 1, 3, 4
like Red's). Per character (sMpChars): walking + running 16x32, Mach / Acro Bike frames (32x32; 64x64 for PROF.
BIRCH, bikeSize), 9 diving 32x32 (one per picture-table entry, as the game's sheets), a field palette (walk, run,
bikes: <= 15 colors each) and a diving palette; flags MACH_RIDE / ACRO_RIDE (the sheet is a ridden Pokemon or a
vehicle: shape overlap with Brendan's bicycle < 0.78, minus the drawn bicycles in BICYCLE_MACH / _ACRO) and GENERIC.
The pack left the Acro front-wheel frames (11-12, 15-16, 19-20) empty for 64 characters and put Brendan's there for
6: the importer copies the back-wheel ones (9-10...) over them.
Looks: graphics ids MP_GFX_CHAR_FIRST (0x9000) + n, ini names kai_<name>; NpcInfo is the walking sheet. mp_avatar.c
ActionKind: ACT_CHAR (own frames, BuildCharInfo) for diving and the bikes, unless the bike sheet is a ride and the
look's `bike` (LOOK > BIKE, appearance byte 27: bytes 13-26 are the CREATE recipe, FIELD HAIR last; ini `bike = ride|bike`) asks for a bicycle -> ACT_HYBRID; the rest
ACT_HYBRID (Brendan's / May's body by the character's gender, skin shades from the color map's S colors: CharSkin).
They run in their own running frames. Diving uses its own palette (local tag MP_PACK_DIVE, StatePalette ST_UNDERWATER;
a map named kai_<name>_dive would recolor it). PROF. BIRCH's 64x64 bikes: every local state info is
LocalTileBytes = 2048 for him (the player's tile block never changes size), OAM / subsprites from RayquazaStill;
remote players resize per mode as before. The replaced looks (old STEVEN, RED, LEAF, WALLY, WALLACE, ARCHIE, MAXIE,
LUCY, PROF. BIRCH, MAGMA GRUNT F, FRLG PROF. OAK, GIOVANNI, BLUE, ROCKET GRUNTS) stay in the catalog but aren't
offered; sMpCharAliases turns their ini names into the characters' (MP_Avatar_ParseSpriteName).
LOOK > GROUP: the SPRITE choices are a built list (BuildChoices) in three groups: NAMED NPC (DEFAULT, BRENDAN, MAY,
the named characters in pack order, sNamedCatalog, RS BRENDAN / MAY), GENERICS (GENERIC characters, the rest of the
catalog), CUSTOM (CUSTOM 1-3, sprite packs; rebuilt with the pack list). SPRITE steps inside the group; GROUP goes
to the sprite last worn in the next group. BIKE only shows for riders: the LOOK cursor counts the rows shown
(LookRowAt / LookCursorOf). ID PIC / BK PIC lists end with the new pictures (CHAR_FRONT_FIRST / CHAR_BACK_FIRST).
Recolors: 345 maps in colormaps.txt (kai_*), first guesses by tools/moemon_chars/suggest_maps.py (skin from the face
middle, hair from the top of the head, outfit / accent from torso / legs), every one checked in the three test looks
(review_maps.py check), the wrong ones mapped by hand from per-color strips (review_maps.py strips); backs take their
front's roles by color (most share the palette). The sprite editor export writes walk, run, 32x32 bikes and diving.
Debug: `mp sprite kai_serena`, `mp bikemode 0|1`, `mp underwater`, `mp actiondump FILE`, `mp export` (writes into
the working folder's sprites/). Test: look_chars (NA-JP lag: BIRCH's 64x64 bikes, SERENA's ride vs bicycle, diving,
an old ini name).

## HMs (Moemon Kai): no restrictions, held HMs (mp_heldhm.c)
HM moves can be forgotten anywhere (summary CanReplaceMove, evolution / battle / co-op level-up learning), the MOVE
DELETER's last-SURF check and the PC's restricted release moves are gone. A Pokemon can hold an HM
(MP_ItemCanBeHeld in the bag's GIVE paths) and use its field move as if it knew it: ScrCmd_checkpartymove,
PartyHasMonWithSurf, the party menu's field moves (MENU_MP_TEAM drops out when 4 field moves + a held one fill the
9-entry list) and LEND. Badges are still needed. A held HM never leaves the player: MP_ReturnHeldHm on release
(ReleaseMon), link / in-game trades (TradeMons, both sides), co-op trades (ApplyTrade; the copy sent is stripped)
and the shared PC (PutInMon); MP_GiveMonKeepOT strips one that arrives. Thief / Covet / Trick never take a held HM.
Protocol 32. Debug: `mp monmoves SLOT`, `mp bagqty ITEM`, `mp pc`. Tests: hm_forget, hm_tm, hm_held, hm_surf,
hm_release, hm_trade.

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

Weather on screen (1.18.4, mp_battleweather.c): the "continues" scripts (RainContinuesOrEnds, DamagingWeatherContinues,
SunlightContinues) no longer print or animate (mpweathermsg 0 is unused now), OverworldWeatherStarts keeps its line
but not its animation. MP_BattleWeather_Composite (sdl2.c, after the Candy Jar) draws the particles over the scene
(y < 112, the message box below) from the game's art, decompressed once with malloc: Rain Dance's drops (AnimRainDrop
motion, one a frame, two in a downpour), Hail's stones and large ice crystal (AnimHailBegin), Sandstorm's 64x16 sand
crescents. The renderer hook gMpScenePixel (gba_easy_draw.c) runs every pixel of BG1-3 and of the battlers' sprite
palettes (gMpSceneObjPal) through ScenePixel: Sandstorm's background (gBattleAnimBgImage_Sandstorm, 256x256, scrolled
6 left / 1 up a frame) blended 5/16 like its BG1, and the tint: rain 4/16 black (BG and Pokémon, as General_Rain),
hail 4/16 black (BG, as Move_HAIL), sun 4/16 warm white on the Pokémon and 2/16 on the BG (Move_SUNNY_DAY's blend).
Healthboxes and the message box are never touched. Colors fade with the scene (MP_FadeFit on BG palette 0); tints are
off during palette fades. Own RNG (never the battle's). Debug `mp bweather rain|downpour|sun|sand|hail|none`; log
"battle weather on screen: ..."; tests battle_weather (shots of each, a turn in the rain), battle_weather_map.
From the first frame (1.22): until the battle's own weather is known (gBattleWeather set, or MP_BattleWeather_TurnStart
from battle_main.c where a turn's action selection begins), Composite draws the weather the field will give it
(FieldWeather: ABILITYEFFECT_SWITCH_IN_WEATHER's mapping; not in recorded battles), so the rain falls during the intro;
"It's raining." still comes after the send-out. `mp bwshown`; test battle_weather_intro.
The others' screens (1.25, protocol 50): in a link battle only the master's engine runs (the others' gBattleMainFunc
is BeginBattleIntroDummy), so their gBattleWeather never changed and their screens showed their own FIELD weather the
whole battle: a friend still in the rain of a route, or in a dry room while the master's field rained, saw a sky that
wasn't the battle's, and a RAIN DANCE never showed. Now the master's PrepareBufferDataTransferLink puts a
B_COMM_MP_WEATHER block (MP_BattleWeather_LinkState: gBattleWeather, the master's field weather, settled) in the
stream ahead of the next command whenever that changes (and first thing after the link tasks start, so a joiner gets
it). The others take it in its place (CopyReceivedLinkBuffersData: after their screen finished what it shows, at most
90 frames) and set gBattleWeather, their FieldWeather and settled from it. Before the first block, a co-op battle
already knows the master's field weather: MPR_COOP_ACCEPT (2 bytes after the second lose text) and MPR_COOP_WILD_GO
(bytes 6-7) carry MP_BattleWeather_FieldFlags, kept by MP_BattleWeather_SetMasterField until NewBattle
(SetUpBattleVarsAndBirchZigzagoon, which also clears gBattleWeather on every game). A joiner's battle is settled from
the snapshot. Debug `mp fieldweather N` (this game's field weather now); tests coop_fieldweather (a dry master and a
rainy friend, then RAIN DANCE), coop_fieldweather2 (the other way round), tri_rain (a triple at NA-JP lag).
Routes 119 / 123 (mp_linkx.c UpdateWorld): when a friend's weather cycle stage is taken over, MP_Weather_CycleStageChanged
brings the new stage's weather in at once on a route following the cycle (SetNextWeather), not on the next visit.

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

## SPECTATE panel (mp_spectate.c, display.c)

OPTIONS > CO-OP (option_menu.c, since 1.13; CO-OP > SPECTATE before): FRIENDS' SCREENS, a row per friend online (A:
their page SHOW / PLACE = BELOW / ABOVE / LEFT / RIGHT / OWN WINDOW / SIZE, through mp_spectate.c MP_Spectate_Friend /
MP_Spectate_ListFriends / MP_Spectate_ShownSize), NEW FRIENDS (gMpConfig.spectateScale, the size for friends without
settings). Per friend: gMpConfig.specFriends[16] (struct MpSpecFriend: ASCII name, side, size, popout,
show; SpecFriend(id, create) finds them by name, the oldest dropped when full; ini `spectate_friend = SIDE SIZE POPOUT
SHOW NAME`). Panel_Update only asks friends with show on; MP_Spectate_PanelSync skips any just hidden (a closed window).
display.c (1.12): each card has side / scale / popout / id. PanelLayout(f) puts the window's cards in groups: rows
above and below the game (the whole width), columns left and right (between the rows), each card at min(scale, f);
PanelFitScale = the biggest f whose layout leaves the game 1x; FitWindowToPanel tries f then the game's whole scale k
(PanelWindowSize). A pop-out (struct Popout) is its own SDL window + renderer (no vsync) + textures, opened beside the
game's window, drawn at the biggest whole size that fits it (PresentPopouts in Display_Present); its CLOSE event is
queued for Display_TakeClosedPopout (mp_spectate.c turns that friend's SHOW off) and the window is destroyed at once.
Closing the game's window while pop-outs are open pushes SDL_QUIT (SDL only quits when every window is closed).
Display_GetCardScale(i) = the size card i (the order given) really has. `display info` adds `| card ID SIDE Nx at X,Y
WxH` per card and `| popout ID window WxH`; harness `popshot NAME` saves each pop-out (NAME_ID.bmp), `display
closepopout ID` clicks its close button, `mp panelfriend PLAYER SIDE SIZE POPOUT SHOW`. Test spec_sides.
The rest as before: Panel_Update subscribes to up to 3 present friends with
MPR_SPEC_PANEL_JOIN (LEAVE when off or they leave; asks again after 9 s of silence); the sender streams whenever anyone
watches (sStream.viewers | panel), 15 updates a second (20 while a battle watcher is on), a full frame every 4 s.
Frame message (protocol 33): u8 type, u16 seq, u8 flags (KEY 1, SHIFT 2, LAST 4), u16 song, u8 tile count, s8 shiftX,
s8 shiftY, then tiles (mode 0 raw, 1 RLE, 2 indexed 1/2/4 bpp). FindShift detects the map scrolling (camera hint +
pixel samples), ShiftFrame moves the picture on both ends and only the uncovered and changed tiles go. One message per
update goes to 0xFF when every friend watches (the server copies it); a send backlog over 24 KB skips pictures
(MPNet_ClientQueued; a picture is a diff, so nothing is lost). ApplyUpdate decodes for the battle watcher and the
panel alike; Panel_DrawLabel draws the card label (dot, name, location / red IN BATTLE pill) with the small font.
MP_Spectate_PanelSync (sdl2.c VDraw, and SpectateStep while watching a battle) hands the cards to Display_SetPanel.
display.c: PanelSize = 2 pads + n cards (240s + 2s outline) + gaps, 160s + label 12s; RenderToBackbuffer reserves the
bottom, PanelFitScale shrinks the cards until they fit with the game at 1x or more; FitWindowToPanel (window only) keeps
the game's whole scale and shrinks it, then the cards, until the window fits the monitor's usable area, and restores the
window when the panel goes (also after leaving fullscreen with it up). Display_GetPanelScale = the size that fits now
(the menu's "(2X FITS)"). Around the game the window is the panel's slate. Bandwidth: ~1.2 KB per update.
Tests: spec_panel (4 players, DANA on a 1920x1080 monitor: sizes, fullscreen, off/on), spec_sides (left / right / above,
mixed sizes, a pop-out and closing it), spec_menu (real keys: BOB's page, ABOVE 2X saved by name, a friend leaving), spec_watch (panel while watching a battle), wine_smoke part 4 (Linux host + Windows friend).
MOEMON_SPECTRACE=1 logs every picture's FNV sum on both ends; tools/test/check_spectrace.py compares them.
Debug: `mp panel ON SIZE`, `display info` (window, game rect, cards), MOEMON_TEST_MONITOR=WxH.

## Tasks that destroy the next one (task.c NextLiveTask)

DestroyTask unlinks a task but leaves its own `next`; RunTasks followed it after the task's function returned. A task
that destroys itself and then (directly or through cleanup) the task after it made RunTasks run the dead one: MoeGatcha's
opener (Task_EnterGatcha) calls CleanupOverworldWindowsAndTilemaps -> ClearMirageTowerPulseBlendEffect, which destroys the
MIRAGE TOWER shimmer (priority 0xFF, often right after it) and frees its data: a crash on ROUTE 111 (UpdatePulseBlend
reading NULL+4). RunTasks / RunTasksExcept now skip dead tasks (NextLiveTask), the shimmer task destroys itself without
its data, and Task_EnterGatcha cleans up before destroying itself. Test gatcha_mirage (flags 334 + 157 = tower shown).

## Auto-updater and releases (src/platform/updater.c, tools/release)
Runs in main() (sdl2.c) right after the window opens, before the save is used. update.txt comes from
`https://github.com/<MOEMON_UPDATE_REPO>/releases/latest/download/update.txt` (no API, no rate limit): `version`,
`tag`, and per platform `<windows|linux> <zip> <bytes> <sha256> <exe>`. The zip is fetched from the tagged release
(`releases/download/<tag>/<zip>`) so a release published meanwhile can't mix in. Download in an SDL thread into
`update_download.zip` next to the exe (size capped at the manifest's), SHA-256 checked while streaming and again
after reading it back; entries checked against the zip's CRC-32 as they're unpacked (stb_image's deflate,
`stbi_zlib_decode_noheader_malloc`). The top folder of the zip is dropped; names with `..`, `:`, `\` or a leading
`/` are refused. Existing `*.ini` and `*.sav*` are kept. Install: every file to `<name>.upd_new`, then each existing
file renamed to `<name>.upd_old` (works on a running exe and loaded SDL2.dll on Windows) and the new one renamed in;
a failure renames everything back. `update_cleanup.txt` lists the `.upd_old` files; the next start (the restarted
game) deletes them, or keeps the ones still locked for the start after. The new exe starts with
MOEMON_JUST_UPDATED=1 (CreateProcessW on Windows, execv on Linux), which skips the check once. HTTPS: WinHTTP
(automatic proxy, then the system's) on Windows, libcurl via dlopen on Linux (honors https_proxy; CURL_CA_BUNDLE /
SSL_CERT_FILE set CAINFO). Screen: a 240x160 frame with a 5x7 font of its own (the game's text engine isn't up yet).
Off: AutoUpdate = 0 in the ini, --no-update, MOEMON_NO_UPDATE=1, no moemon_release.txt, or a headless test without
MOEMON_UPDATE_URL. Test: tools/test/update_test.sh (WIN=1 under Wine): offline, up to date, damaged, ESC, rollback
(MOEMON_UPDATE_TEST_FAIL_SWAP), update, cleanup, not a release folder. MOEMON_UPDATE_FETCH=url fetches one URL and
prints its SHA-256 (checked against curl on a 7.6 MB GitHub release asset, with redirects).
Releases: .github/workflows/release.yml runs on a push to the default branch that changes include/mp_version.h (or
by hand), skips a version that already has a release, builds both platforms, packages, runs the updater test and
publishes with `gh release create --target <commit>` (which makes the tag); notes from tools/release/notes/v<ver>.md.

## PATCH NOTES (mp_setup.c, mp_patchnotes.c; data from tools/release/notes)
The CO-OP setup screen (after the title screen; not when it opens from the field: BuildMainItems checks fromField)
has PATCH NOTES as its last row. tools/release/gen_patchnotes.py turns every `tools/release/notes/v*.md` into
src/data/mp_patchnotes.h (generated, gitignored; json_data_rules.mk, AUTO_GEN_TARGETS, so `make generated` builds it
before scaninc): `{version, title, summary, body}` newest first, title and summary from the first line ('Moemon Emerald
Kai Co-op <ver> "<Catchy Title!>": <summary>', required; release.yml names the GitHub release with the title too), the body one item per line with a kind letter (H heading, B item, S sub-item, P
paragraph), `**` / backticks dropped, `## Installing` and the "installs this update by itself" line left out, only é
outside ASCII and no braces (button tokens). SCR_PATCHES lists version lines (major.minor, BuildPatchLines) with the
line's x.y.0 title cut to fit (FitAscii); the reader opens each update with "<ver>  <title>" and its summary; A builds the reader rows (BuildPatchText: oldest update first, word-wrapped in
FONT_SMALL, 12 px rows, malloc'd PatchRow array) into WIN_FULL (28x16 at baseBlock 512, the last tiles before the
tilemap), shown by clearing rows 2-19 and framing it (ShowPatchReader; CopyBgTilemapBufferToVram: this screen runs no
scheduled copies). UP/DOWN scroll, LEFT/RIGHT/L/R/A page, B back. v0.1.0-v0.4.0 are the days before 1.0 (Oct 18-21),
written from the commit log. Test patch_notes (MOEMON_SKIP_COOP_SETUP=0 shows the screen in a test; waitfor
patchlist / patchtext; log "patch notes: line X, N notes").

## Full 7-letter names (1.4.1)
StringCopyN / StringAppendN copy exactly N bytes and never end the string. A name that fills all 7 letters has no end
mark inside those 7 bytes, so a label built with `StringAppendN(buf, name, PLAYER_NAME_LENGTH)` ran on into whatever
followed: junk on screen, or a crash in GetStringWidth on a 0xF7 (the "insert a stored string" code). The three such
labels (the CO-OP menu's players line, MP_Menu_BuildPlayersLine; the shared PC's "from NAME", MP_SharedPc_FromLabel;
the POKéDEX's "CAUGHT BY NAME", MP_Dex_CaughtByText) now write the end mark after the copy. `mp namelines` builds
each into a buffer of 0xF7s and prints it ("|" = where the text ends). Test names7 (4 players named SATOSHI,
TAKESHI, MITSURU, KENJIRO; regress.sh NAME_A..NAME_D set the names); the old code crashes in its CO-OP menu.

## Joining a battle in progress (mp_join.c, protocol 34)
Walking up to a battling friend: SPECTATE / BET / JOIN / CANCEL (mp_spectate.c Task_WatchPrompt, a window beside the
message, the yes/no box's tiles). JOIN (MP_Join_Ask) sends JOINM_ASK; a co-op partner who isn't the master answers
REDIRECT and the friend asks the master. The master queues the friend (WAIT) and asks its player at the next turn's
start: BattleTurnPassed / TryDoEventsBeforeFirstTurn call MP_Join_OnTurnStart, HandleTurnActionSelectionState starts
with MP_Join_TurnHook (TRUE holds the turn; states MJS_*). NO: ANS_NO (and the friend is told no at once for the rest
of the battle: sJoin.declined). YES: ANS_YES (Pokémon each, players, wild flag, trainer id) -> the friend sends
JOINM_PARTY (their picks: CO-OP TEAM first, MP_Team_BattleSlots) and starts the battle transition; a friend asking
while others are being let in waits for the next turn (JoiningCount).
Conversion on the master (mp_coop.c MP_Coop_JoinConvertMaster): SavePlayerParty for a solo battle (its party is the
real one), each old seat keeps its usable Pokémon in battle party-menu order (the one out first; 3 in a double, 2 in a
triple), the left-out ones go back to the real party at once (a partner's through CONVERT, as the master had them),
with their level-ups (sCoop.preEvo, evolve after a win) and moves to learn (learnOut); every party-slot state is
remapped (gBattlerPartyIndexes, gSentPokesToOpponent, knockedOffMons, wishMonId, gLeveledUpInBattle, givenExpMons,
learn-later slots); an absent battler points at its own seat. The trainer's party is laid out for co-op (double:
halves interleaved, triple: 0-2 then a pool) with TEAM PADDING once (solo battles only, into an empty slot: never
over a fainted foe, whose "gave EXP" bit would pass on). New battlers start absent and "entering" (the screen rebuild,
reshow_battle_screen.c, skips their sprites, boxes and shadows). A solo Double Battle: battler 2 first goes back to
its ball (BattleScript_MpJoinRecall) and becomes the friend's; battlers taking over a spot get clean sprite state.
Link: MP_Battle_JoinLink (a solo battle opens its link here; the others add link ids; everyone counts as ready; sync
barriers continue from the master's seq) and MP_StartBattleLinkTasks (past the cable's start-up wait).
The master sends JOINM_STATE (struct MpJoinSnapshot: plan, battle mons, sprite info, forms, side statuses, weather,
lose text, both parties) to joiners and JOINM_CONVERT (+ their old seat) to a partner; everyone fades and rebuilds
(ReshowBattleScreenAfterMenu; MP_TriCards_Reload starts a triple's cards), a partner only once its screen caught up
(exec flags 0, link drained). A joiner: CB2_JoinWaitState -> CB2_MpJoinBattle (battle_main.c: resources, registers as
the intro's slide leaves them, controllers, link, then the rebuild) -> READY. With every seat READY the master runs
BattleScript_MpJoinEntrance (Cmd_mpjoin: next friend -> trainer slides in, "X has come to help!", switch-in data,
IntroTrainerBallThrow, switchineffects; then each new foe spot's next Pokémon), then the turn starts (communication
state cleared). The end is the usual co-op one (CB2_MpCoopEndBattle; afterBattle = the solo battle's own callback).
Fades (1.8.1): MJS_FADE / PJS_FADE start the fade to black only when BeginNormalPaletteFade accepts it (a solo double's
recall leaves a fade-in running; the refused fade let ReshowBattleScreenAfterMenu, which resets the fade and clears VRAM,
run in plain view: white flash, bare background, empty boxes). Log "join: rebuilding the screen behind black (fade 16)",
expected by the join_* tests. Found with the harness's `shotburst PREFIX FRAMES [STEP]` (every frame saved; the first
fade-in frames only look bare when brightened: light colors all round to one value at that BLDY).
The joiner (1.11.1): CB2_MpJoinBattle builds a screen that was never there, and some palettes reach palette RAM
directly while ReshowBattleScreenAfterMenu runs (bufferTransferDisabled doesn't hold them), so ~30 frames of half-built
battle (green stripes, bits of sprites, an empty box) showed between the transition and the fade-in. gMpReshowDark
(reshow_battle_screen.c, set by CB2_MpJoinBattle) re-applies BLDCNT darken / BLDY 16 after every rebuild step (step 0
resets the blend registers); the closing BeginHardwarePaletteFade(0xFF, 0, 16, 0) fades in from that same darkness.
Joinable: solo wild (savedCallback = CB2_EndWildBattle: not scripted ones like KECLEON) or one trainer, single or
double (CB2_EndTrainerBattle, modes SINGLE / CONTINUE_SCRIPT / REMATCH and the double ones; not bosses, RIVAL,
TEAM AQUA / MAGMA), or a co-op double of that kind. Not: triples, two trainers, PvP, frontier, safari, legendaries.
Tests (MODE 2/3, WORST): join_no, join_wild, join_trainer, join_double, join_triple (double -> triple + redirect),
join_two (two at once), join_cancel, join_leave; `mp partyids` + join_wild/check.py check every party's identities.
Harness conditions: watchmenu, joinprompt, joinpendingN, friendbattleN.

## Items from later games (D1, protocol 35)
Items 389-397 (ITEMS_COUNT 398; the CANDY JAR made it 399): ITEM_QUICK_POWDER (HOLD_EFFECT_QUICK_POWDER: GetWhoStrikesFirst doubles DITTO's
Speed unless STATUS2_TRANSFORMED), ITEM_EVIOLITE (HOLD_EFFECT_EVIOLITE: CalculateBaseDamage, DEFENSE and SP. DEF x1.5
when SpeciesCanEvolve), and seven treats FIRST_TREAT_ITEM..LAST_TREAT_ITEM with gItemEffect_LavaCookie (all status +
confusion; ITEM_HAS_EFFECT includes them). Names fit ITEM_NAME_LENGTH (13 letters): LUMIOSEGALETE, PEWTERCRUNCHY,
RAGECANDYBAR (the user's pick). gItemEffectTable now ends at [ITEMS_COUNT - ITEM_POTION] so every item id is inside
it (it was read past its end for NINIKU / TOPO); GetItemEffectType returns NONE for an entry without effect. Icons:
tools/moemon_items/import_items.py SRC_DIR (the user's items.zip; not in the repo): GBA colors, wide art loses the
columns most like their neighbors, >15 colors median cut. MoeGatcha STANDARD: treats N (2 each), EVIOLITE / QUICK
POWDER R. Test items_d1 (mp holdtest runs the battle math on made-up battlers, in a battle; mp pstatus, mp bstatus2).

## PC TAG mode (pokemon_storage_system.c, end of the file)
MOVE POKeMON only. The west button / F key arrive as INPUT_ACTION_PC_TAG in INPUT_CONTEXT_PC_STORAGE
(input_context.c: MP_PcStorage_IsOpen) and set gMpPcTagPressed; HandleInput takes it for that frame (sTagPressNow;
CB2_PokeStorage clears it). sMpTag: one area (a box or the party), a position mask, gold badge sprites (36,
SpriteCB_TagMark follows the icons, hidden while the box scrolls: SetUpScrollToBox / SetCurrentBox). With tags, A =
Task_TagMenu (MOVE / RELEASE / CANCEL), B = drop them. MOVE reuses the SELECT group move: sMultiMove holds the tagged
Moemon over their bounding rectangle (holes for the rest; a party group is one row), Task_TagMove draws them on bg 0,
inBoxMovingMode MOVE_MODE_MULTIPLE_MOVING moves them with the cursor. A: MultiMove_CanPlaceSelection -> the stock
placement, else Tag_TryPlace's assorted targets (empty slots from the cursor on) -> MULTIMOVE_TAG_PLACE_ASSORTED, else
a buzz (no message: bg 0 is the 256-color group layer). R, or DOWN past the group to the buttons (Tag_InputOnButtons,
bg 0 hidden) and A on PARTY POKeMON: Tag_GroupToParty, then Task_TagShowParty slides the party in (its icons are made
from the party; don't touch party sprites or slot tilemaps while it's hidden: TilemapUtil crashed). RELEASE: no EGG,
no MAIL, a battler left in the party, two Moemon left in all; held HMs back (MP_ReturnHeldHm). Renderer fix
(gba_easy_draw.c, the renderer the game uses): a 256-color text BG's color 0 is transparent (the stock SELECT group
move blacked out the screen). Tests pc_tags, pc_tags_last (mp pctag, pctags, boxput, boxmons; gpuregs, bgshow,
bgpeek for display debugging).

## Shared PC boxes (D4, protocol 36: mp_store.c, mp_sharedbox.c, mp_sharedpc.c, pokemon_storage_system.c end)
Server (mp_store.c, file MOEST2: 14x30 slots of MP_STORE_MON_BYTES (struct Pokemon + who put it in), box names and
wallpapers, a hand per seat; MOEST1 files (the old list of 60) load into SHARED1-2 in order; whatever was in a hand
goes back into the first free slots on load and when its player leaves the room: MPStore_PlayerLeft from LeaveRoom).
MP_ST_BOX_GET -> BOX_DATA per box; MP_ST_TXN (seq + MP_TXN_* commands, applied one by one, each on its own): HOLD
(slot -> hand, only if the same personality/OT ID is there), PLACE (hand -> slot, or the next free), IN (from the
player's game -> slot, next free, refused if full), OUT / RELEASE (out of the hand), SET (new data, same Pokemon),
BOXINFO; the changed boxes go to the room as BOX_DATA, then TXN_RESULT to the sender (refused INs, lost HOLDs, the
hand). MP_ST_CURSOR is relayed to the others with the sender's seat. Client (mp_sharedbox.c): sMirror is a
struct PokemonStorage; while the box screen shows the shared boxes gPokemonStoragePtr points at it and
MP_PersonalStorage() is the player's own (save.c's sector table and load_save.c's save block mover use that, so a
save or a save block move never takes the mirror). The box screen changes the mirror like any boxes; MP_SharedBox_Update
(every frame, only when MP_PcStorage_Steady: the main task waiting for input) diffs it against sBase (the server's
last word + what we sent) and sends one transaction at a time: a slot whose Pokemon left -> HOLD; a slot with a
Pokemon the server doesn't have there -> PLACE (from our hand) or IN; hand Pokemon no longer held (MP_PcStorage_Held:
the moving Pokemon, sMultiMove's group while it's carried) and not in the mirror -> OUT (found in the party / our boxes; POKeDEX set) or
RELEASE. BOX_DATA and the result are kept until steady; ApplyBox rebases slots both sides changed (theirs wins; a
Pokemon we put there moves to a free slot; one we took that's now someone else's is "lost": TakeBackLost removes our
copies wherever they are, MP_PcStorage_DropHeld lets go of a held one). Lost link: what we hold of the server's goes
back (it returns it), the box screen closes (MpForceCloseNow puts our own held Pokemon in our boxes). Box screen
(MP_PcStorage_OpenShared, MOVE POKeMON mode): the stock PARTY POKeMON / CLOSE BOX tiles are blanked and three sprites
(graphics/moemon/sharedpc/btn_*.png from tools/moemon_sharedpc/make_buttons.py, cut from the stock buttons) take
their place: cursor positions 0 PARTY, 2 MY PC, 1 CLOSE (box columns 0-1 / 2-3 / 4-5 go down to them). The buttons
have subpriority 20, under the hand (6); on them the hand points down from y 6 (2 holding a Pokemon) and is never
flipped: the stock cursorFlipTimer flip is skipped and MpUpdateButtons clears vFlip each frame (a flip under way when
MY PC rebuilt the screen used to leave it upside down; test shared_pc_cursor, mp pccursor logs it). MY PC swaps
the pointer and rebuilds the screen (Task_MpRebuild: like coming back from naming a box; a held Pokemon stays, a held
group can't switch). MENU_MP_SEND_PC / SHARED (MpSendOne / MpSendTagged: party Pokemon always to the shared boxes).
Release: "Release LUNA's PIKACHU?" (MpReleaseQuestion), no field move check for a shared one. A friend's change to
the shown box's name or wallpaper (MP_SharedBox_TakeInfoChanges) is drawn in place by Task_MpBoxLook: the title
redrawn over its sprite tiles, the wallpaper faded over as the WALLPAPER menu does. Friends' cursors:
MP_ST_CURSOR (view, box, x, y, held species / count, item row); the box screen draws a hand in the seat's color, a
name plate and the held icon. Items (mp_sharedpc.c): MpUi list, SHARED / MY PC (gSaveBlock1Ptr->pcItems), one server
job at a time (sPc.queue), TAG via INPUT_CONTEXT_PC_STORAGE (MP_SharedPc_ItemsOpen). Tests: shared_pc_cursor, shared_pc (with
check.py: every Pokemon in one place, from the server's file and each player's monids), shared_pc_race,
shared_pc_drop, shared_pc_tags, shared_pc_old (a MOEST1 fixture), shared_items; tests run the server with
MOEMON_STORE_DIR = the test's folder (regress.sh copies a scenario's *.dat there). Harness: mp sharedpc / sharedin /
sharedbox / sharedname / monids / ownbox / itemcounts; conditions sharedsteady, sharedclosed, pcstorage,
slot:BOX:POS:SPECIES, boxname:BOX:NAME.

## Stadium hues (D5, mp_hue.c, src/data/mp_hues.h from tools/moemon_hues; protocol 37)
A nicknamed Pokemon (nickname != gSpeciesNames, never an Egg) gets Pokemon Stadium 2's hue, as the bluemoonfalls
"Stadium hues" page works it out (tools/moemon_hues/reference.js holds the page's math): NV = (OT name + nickname
codes + the Trainer ID's two bytes) & 0xFF (StadiumCode: ASCII / Latin-1 like the page's table), hue =
(NV - 255/range*min) * range/255 rounded like the page's step-1 slider (halves up, clamped to -min..max), then each
color goes RGB -> HSL (whole degrees, cut percents) -> +hue -> RGB (GBA 5-bit <-> 8-bit c<<3|c>>2, back to the
nearest). The hue_math test compares 10k values with the page's JavaScript (node). Limits: Stadium's for 1-251
(stadium_hues.json), Gen 3 = median of the Stadium species with the same first type + body color (make_hues.py).
Kai's pictures keep their skin: sMpHueSkin[species] = palette slots of skin (skin.py rules + skin_overrides.json,
114 species checked by eye; the same slots for shiny and back pictures). OPTIONS > CREATURES = POKeMON, icons and
followers turn whole (creature art). Hooks: pokemon.c's four palette getters (MP_Hue_Lz builds a literal-only LZ77
copy in a ring of 16; the struct getters keep the species tag - the game uses it as the species - and refresh a
palette already loaded under it), MP_Hue_Find (otId + personality: parties, boxes incl. the shared PC mirror, DAY
CARE), icons (pokemon_icon.c CreateMonIcon, the PC's CreateMonIconSprite: a copy of the icon palette under
0x6F00-0x6F7F, MP_Hue_FreeUnusedIconPalettes lets unused ones go), followers (FollowerSlot.hue; MpState.followerHue,
wire bytes 24-25). Not turned: the PC's group-move BG layer, a friend's held icon in the shared PC, minigame mons of
other players (no Pokemon data). Debug: mp nick SLOT NAME (- = species name), huemon SLOT, huecheck. Tests: hue_math
(check_hues.py), hue_look (screenshots), hue_follow (BOB sees ALICE's follower's hue).
CASTFORM's palette holds its four forms (128 bytes, every other one 32): MakeLz / MP_Hue_LzStruct read the size from
the LZ header (LzPaletteSize, up to LZ_MAX_COLORS 64), turn each 16 colors and pack them all (1.25.1: a one-palette
buffer overflowed, and the PC box crashed on Windows as the cursor reached a nicknamed CASTFORM). Test hue_castform.

## Two trainers at once, grouping on one trainer (D6, protocol 38: mp_coop.c, trainer_see.c, trainer_battle.inc)
Two trainers spotting a player (gNoOfApproachingTrainers 2): MP_Coop_OnTrainerConfigured keeps the challenge across
both configures (sCoop.twoTrainers, trainerIdB; ordinary modes only). Task_JoinCheck skips after the first speech
(gApproachingTrainerId 0: no state touched) and runs the usual steps after the second; no triples (TripleEligible),
CoopTrainerTeamSize = both teams (3 each at most) + padding. MPR_COOP_ACCEPT: ... u16 trainer B, lose text A, EOS,
lose text B, EOS (MP_Coop_LoseTextB for GetTrainerBLoseText). The co-op battle adds BATTLE_TYPE_TWO_OPPONENTS:
the master builds A at 0-2, B at 3-5 (CreateNPCTrainerParty(.., FALSE)), MP_Coop_FinishTwoTrainerParty pads the
smaller team once, no interleave (MP_Coop_ArrangeEnemyParty). Names/pictures/send-outs per side
(MP_Coop_TwoTrainers, MP_Coop_TrainerPicFor; battle_message.c), BattleScript_MpCoopTwoTrainersWon (both lose texts,
both prizes). The intro asked the right trainer's picture twice (MULTI and TWO_OPPONENTS blocks): over the link
the second request never completes, so it's skipped in co-op. Joining in progress refuses it (JOIN_CANT_KIND).
MPR_COOP_WAIT: + u8 localId2 (second held NPC, 0 = none), u16 waitAge.
Held trainer spots a free friend (trainer_see.c first pass, MP_Coop_CanJoinHeldTrainer / MP_Coop_NoteHeldSpotter):
only that trainer approaches; EventScript_TrainerApproach calls MP_Coop_TryJoinHeldTrainer (REQUEST arg 1,
"You team up with X!", Task_JoinHeld; no UpdateAsking / asker messages: joiningFromScript). Not let in within 12 s
or declined: the trainer's own speech and battle. Both waiting on the same NPC (WaitingOnSameTrainer): the lower
player number asks the other's waitAge (MPR_COOP_AGE_ASK / MPR_COOP_AGE, + half the round trip) and decides:
MPR_COOP_MERGE (they join us) or mergeTarget (Task_JoinCheck JOIN_WAITING -> JOIN_JOINING, StartJoiningFriend;
not let in: waiting again). A joiner from its own trainer script: the battle starts with the script stopped;
CB2_MpCoopFinish returns with CB2_ReturnToFieldContinueScriptPlayMapMusic and, on a win, MP_EndJoinedTrainerBattle
(battle_setup.c: flags, match call, ret-script state as after its own battle); MP_Coop_TakeJoinedOther 1 ->
EventScript_MpCoopAfterTrainerBattle (the post-dotrainerbattle steps), 2 -> releaseall. Without this the trainer
(unflagged, CONTINUE_SCRIPT partners never get the flag) spotted the joiner again at once.
Tests: coop_two_trainers (ROUTE 116 JANICE + JERRY, tile 28,6), coop_two_alone, coop_held_spot, coop_wait_merge
(check.py: one led, one joined, the leader waited longer). Debug: `mp trainerflag ID...`.

A trainer already battling a friend spots us (1.25): whoever leads a battle against a trainer from the field (solo,
or a co-op battle's master) sends MPR_COOP_BATTLING (39, routed to mp_coop.c) every second: map, the trainer's
object (MP_TrainerBattleLocalId: battle_setup.c's object id, or gSpecialVar_LastTalked as the stock trainer scripts
leave that 0, if it's a trainer object on the map), trainer id and whether the battle can take a friend now
(MP_Join_CanBeJoined = mp_join.c's JoinableReason); "off" when it ends. When that trainer walks up to another player
on that map (one trainer: gNoOfApproachingTrainers 1), MP_Coop_TryJoinHeldTrainer (TryOfferBattleInProgress) asks
"X is battling this trainer! Join their battle, or battle alone?" with JOIN BATTLE / BATTLE ALONE beside the message
(yes / no box tiles). JOIN BATTLE: MP_Join_AskFromScript (the join-in-progress request, sJoin.fromScript: no field lock,
the script waits); let in, MP_Coop_JoinedFromTrainerScript sets joinedOther when the snapshot comes, so the battle ends
as the held-trainer join does (MP_EndJoinedTrainerBattle, the script goes on: MP_Coop_TakeJoinedOther 1). Not let in
(NO, can't, no answer, B, called off before it started): the script goes on to our own battle with the trainer.
BATTLE ALONE: the usual speech and battle. Tests coop_spot_join, coop_spot_alone (ROUTE 116 JERRY).

Pokémon battle emotes in link battles (1.25): Emote() sends MPR_BATTLE_MONEMOTE (battler, icon, happy) to the battle
link's other games (MP_Battle_SendToPeers); MP_LifeBattle_OnPeerEmote shows it over that battler (battler ids are the
same on every game) with its sound. After a won trainer battle only the Pokémon that knocked out the last foe bubbles
for it in co-op (it was "the first of ours still out", so every game had its own Pokémon celebrate a friend's
knockout). Test coop_life_emote (check.sh: each game's emotes are what the other saw, on the same battler).

Quicker screen fades (1.25, palette.c): a BeginNormalPaletteFade of (nearly) every palette, 28 of 32 or more, between 0 and 16 (a
screen opening or closing: the party menu, the BAG, the POKéDEX pages, warps...) runs UpdateNormalPaletteFade an extra time on 3 of every 7 frames
(sQuickFade, sQuickFadeCredit): 30% shorter, its delays included. Fades of some palettes or to other levels (battle
animations, flashes) keep their speed.

## CANDY JAR (D3, mp_candy.c; ITEM_CANDY_JAR 398, ITEMS_COUNT 399)
gMpCandies[type]: ReleaseMon (stock release) and Tag_ReleaseAll call MP_Candy_OnRelease(species): +1 of the first
type (MYSTERY counts as NORMAL; max 9999). Saved in "<save>.bag" after the item marks ("CNDY", count, u16 each;
MP_Bag_LoadFromSave resets them first, ClearBag too). MP_Candy_Update (MP_FrameUpdate): candies and no jar, local
player free on the map -> Task_BirchCandyJar: Birch (OBJ_EVENT_GFX_PROF_BIRCH, local id 0xDA) spawns up to 4 tiles
away on the freest side (S, W, E, N), walks up, talks (MP_ShowFieldMessageFitted, "@" = the player's name), gives
the jar, walks back and is removed; nowhere to stand: the jar comes with a note. The jar's use function is
ItemUseOutOfBattle_CandyJar (item_use.c, like the POKeBLOCK CASE: from the BAG back to the BAG, registered back to
the map) -> MP_Candy_OpenJar(back). The menu (CB2_OpenJar): BG1 the jar picture (graphics/moemon/candies/jar.png,
from tools/moemon_items/draw_candies.py with the 17 candy sprites; the lid rows 0-17 are cut off into three 32x32
sprites), BG0 the list (small candies on LIST_MATRIX) and help. Physics (position based): balls of one radius
(BallRadius: shrinks with the count, 7.5..3), gravity, 4 substeps x 4 passes of ball pushes (the upper ball moves
more) and glass pushes; the glass is read from the picture (color 1 next to colors 6-8 = wall points, sInside = where
a center may be; above the neck the opening's sides are walls). Speed = how far a ball got; a push can't throw a ball
up or sideways faster than it was going (no popping out of a pile); resting balls keep their pixel (no flicker).
Rotation: ROT_STEPS shared affine matrices at the current scale (sprites made non-affine, then affineMode NORMAL with
gDummySpriteAffineAnimTable + affineAnimPaused so no affine animation rewrites the shared matrices). Up to 96 balls;
past that BallTypes gives each type its share (every type present at least one). Phases: POUR (lid flies off, one or
two balls a frame from above the neck), SETTLE (lid back on once resting), CONVERT (lid off, JarSyncBalls marks the
extra balls LEAVING: they steer up through the neck to GATHER, the RARE CANDY icon grows with each, pops and
vanishes; then any new balls pour). SELECT: shake (BG1 jiggles, walls move with it, balls get kicked). Tests
candy_jar, candy_jar_full (check.py reads mp jarstate: nothing outside the glass, no deep overlaps, all still),
candy_flow (release -> Birch -> BAG / registered jar -> save reload). mp candy TYPE N (-1 = all), candyjar,
jarstate, candyreload; harness conditions jar, jaridle.
1.18 rework, drawn from the user's mock-up (tools/moemon_candyjar: jar_mockup.png -> gen.py -> src/mp/mp_candyjar_gfx.h).
The framed jar panel, the list window (shortened: its rows from 115 are the mock-up's 14 lower, so it ends above the
game's message box) and the jar's glass are full-color pictures (one table of the mock-up's colors) drawn UNDER the
game's layers: MP_Candy_Composite (sdl2.c VDraw right after DrawFrame) fills every pixel nothing was drawn on (the
renderer leaves bit 15 clear on the backdrop) with them, the glass shifted by sJar.shownShakeX (the shake latched in
VBlankCB_Jar when OAM loads: the frame is drawn before the new OAM, so the current shake put the glass a frame ahead of
the lid and candies); elsewhere (sCJ_Bg 0) a 128x128 tile of 64-pixel diamonds (gen.py draws it: a checker turned 45
degrees, its edges dithered) drifting down-right a pixel every other VBlank (sJar.scroll). The backdrop is white, so its faded value gives the fade step (inverted
with palette.c BlendPalette's math) and the pictures fade with the palettes. gen.py paints the mock-up's three sample
candies out of the glass (rows above the floor from clear glass higher up, the floor's rows from its middle), takes the
lid off (rows 25-40: a 64x32 sprite, sCJ_LidTiles; it flies to LID_UP_Y above the screen), draws a glass lip on the
mouth (rows 36-40, all under the lid: only seen with the lid off), and fills the jar's place in the background with the
panel's own speckle. The physics read sCJ_JarRole (1 outline, 2 glass; the edge = the first and
last pixel that isn't panel green, so a light outline pixel still closes the wall); box CJ_JAR_X/Y 16,24, 80x96, neck
CJ_NECK_L/R, CJ_LID_ROWS 17; candies at most 6 px in radius (the mock-up's 12-pixel ones), sized from CJ_GLASS_AREA.
BG0 only (text): WIN_LIST (see-through, 16x15 from 112,8) and WIN_HELP in the game's own message box (rows 16-19:
LoadMessageBoxGfx / DrawDialogFrameWithCustomTileAndPalette; the user preferred it to the mock-up's, which ran off the
screen), palette 15 = the standard menu palette (message, YES / NO box, button glyphs) with the mock-up's inks in 10-15
(sJarInks). The list as the mock-up: names (dark) and counts (blue-gray; blue = can make a RARE CANDY) in FONT_SMALL at
sListRowY (11 pixels apart), 10-pixel candies at LIST_ICON_X, the mock-up's 4x6 gray arrow drawn into the window
(JarDrawCursor), ALL in the right column's empty last row. The message in FONT_NORMAL, the buttons small at its right.
JarIdleHelp says what the kind under the cursor needs ("N more FIRE for a RARE CANDY.", "2 RARE CANDY from WATER." with
{A}MAKE). A on a kind with 10+ (or ALL with anything) asks YES / NO (CreateYesNoMenu over the list's lower right; the
list's candies under it hide: JarHideUnderYesNo); YES converts that kind (MP_Candy_ConvertType) or all
(MP_Candy_Convert), and JarSyncBalls sends just those candies flying. Moving the cursor makes that kind's candies hop
(JarHopKind, idle only). New candies pour in kind by kind (no shuffle): they settle in layers of color. Tests candy_jar
(WATER alone, then ALL), candy_jar_full (96 candies at radius 3.0, none outside the glass or overlapping).

## Reusable TMs (1.18.1: party_menu.c, shop.c, mp_tmrefund.c; data from tools/moemon_tms)
As in later games a TM never wears out: party_menu.c Task_LearnedMove no longer removes it. shop.c: a TM already in
the BAG or the PC isn't sold again ("You already have that TM."), one sells at a time (maxQuantity 1; maxQuantity is a
u16 now, a u8 wrapped the 999 cap round to 231). Saves from before: MP_TmRefund_Update (every frame, once per save:
MP_SaveData "tm:refund") gives back every TM the save got and has in neither the BAG nor the PC. "Got": an event flag
from mp_tm_sources.h (tools/moemon_tms/gen_tm_sources.py reads the maps: item balls = map.json object events whose
script is in item_ball_scripts.inc, hidden items = bg_events, gifts = a script with `giveitem ITEM_TM*` and a `setflag`;
daily / weekly flags and variables prove nothing: Pacifidlog's RETURN / FRUSTRATION are left out), and the TRICK HOUSE's
puzzle 5 TAUNT by its variables (VAR_TRICK_HOUSE_LEVEL > 5, or 5 with VAR_TRICK_HOUSE_PRIZE_PICKUP 0). Shop, Game Corner
and Battle Frontier TMs leave no trace. With anything to give, the LILYCOVE LOTTERY CORNER clerk (OBJ_EVENT_GFX_BEAUTY,
LOCALID_CLERK 0xDB) walks up as PROF. BIRCH does with the CANDY JAR (Task_LotteryClerk: the side with the longest free
line, up to 4 tiles; nowhere to stand: a delivery note), calls them "lottery winnings", hands them over (MUS_OBTAIN_TMHM,
"@ received TM03, TM08 and TM27!") and leaves; with nothing to give (a new game too) the key is set quietly. Debug
`mp tmrefund [reset]`, `mp mart ITEM...` (a POKé MART). Test tm_reuse.

## POKéDEX entries from four games (H9, pokedex.c; data from tools/moemon_dex)
tools/moemon_dex/import_dex.py reads pret pokeruby `src/data/pokedex_entries_en.h` (`#ifdef SAPPHIRE` = Sapphire,
`#else` = Ruby, the _1/_2 page halves joined) and pokefirered `src/data/pokemon/pokedex_text_fr.h` (REVISION 1 text),
flows each into one line (a line ending in a word's hyphen joins without a space), turns POKéMON into MOéMON like
Kai's own entries (POKé BALL stays) and writes src/data/pokemon/mp_dex_more.h: per species Ruby / Sapphire / FireRed
strings and whether Emerald (gPokedexEntries[].description, Kai's text) is shown; a text that repeats an earlier one
word for word is NULL (183 Sapphire = Ruby, 3 Emerald = Ruby), MP_DEX_MORE_MAX_LEN = the longest combined text.
Species are matched by order and checked by name against Kai's pokedex_entries.h. Order: Ruby, Sapphire, Emerald,
FireRed, joined with spaces, no headings (the user's call: "as if it were one, long, PokeDex entry").
pokedex.c (after the pokedex_entries.h include): MpDexBuild (newlines -> spaces) and MpDexWrap (greedy, FONT_NORMAL,
MP_DEX_TEXT_WIDTH 212 px, the block centered by its widest line), MpDexDraw: 4 lines from sMpDexTop at y 95 (the
stock entry's box), DOWN_ARROW at the box's right while more follows, UP_ARROW while reading and not at the top.
PrintMonInfo draws it for owned Pokémon on the info page; the page after a catch (newEntry) keeps the stock entry.
The top bar gets a TEXT tab: TEXT_SCREEN is the first screen (the cursor starts there), sMpDexBarSlots gives the five
buttons' columns and widths (6 6 5 6 7 tiles), LoadScreenSelectBarMain loads tools/moemon_dex/gen_bar.py's 60 tiles
(mp_dex_bar.h, built from the stock bar's pixels; T and X drawn in its letter style) at tile 256 of BG1's char block
(after the 0x2000 bytes of menu tiles) and HighlightScreenSelectBarItem colors by slot. A on TEXT: owned and longer
than the box -> sMpDexReading (else SE_FAILURE); MpDexReadInput takes UP/DOWN (repeat) and B (back to the top);
TryDoInfoScreenScroll does nothing while reading. Logs "dex text: reading #N, L lines", "top T of M", "closed".
Debug: `mp dexcheck [N]` builds all 386 (most lines 19, #303). Test dex_text.

## MOéMON GARDEN (A-Life; the spec is docs/POKEMON_GARDEN.md)
Principles from the design: needs create behavior, experiences create emotions, relationships create attachment;
ordinary absence is never neglect (background needs only become preferences, nothing ever suffers or decays while the
player is away). Emerald's Friendship stays authoritative: only Emerald's own causes lower it; the Garden raises it
only through a moment (below). No settings.

G1 core (mp_life.c, include/mp_life.h). One MpLifeRec per Pokémon, keyed MP_Life_MonKey(personality, otId) (top bit
clear; Trainers are hash(otId, name) | 0x80000000). Records hold temperament (6 axes: BOLD WARM LIVELY CURIOUS
SOCIAL PROUD, base + current, current drifts +-1-2 within base +-40), needs (hunger rest play explore company quiet +
two type needs, dual types weighted by temperament: typeLean), 8 emotions (joy fear frustration sadness surprise
affection pride calm) over a slow mood, tastes (from the Nature's flavors), boundaries (proud -> PET, timid ->
PICKUP, aloof -> GROOM), 12 traits, 32 memories (16 bytes: kind, feeling, strength, tier, place, param, who), 24
places, known[] (journal facts revealed), parents, keepsake. RollTemperament seeds from personality ^ otId, nudged by
Nature, the species' instincts (tools/moemon_life/gen_instincts.py -> flags such as NIGHT SING DIG SHY FIERCE PACK
SEA SKY), egg groups and speed. Bonds (MpLifeBond: familiarity trust affection respect friction, flags
TRAINER/RIVAL/FAMILY/MET) go from a record to a Trainer or another Pokémon; the owner bond starts from Friendship
(familiarity 20 + f/3, trust/affection + f/5). Nothing erodes with absence.
MP_Life_Remember(rec, kind, strength, tier, who, param): tiers PART / WITNESS (x2/3) / HEARD (x1/2); under 80 only
stirs feelings; the same kind within 30 minutes strengthens the old memory; full -> replaces the weakest
unprotected one. The moment rule: a positive moment kind, strength >= 96, not heard, with the owner (who = owner, or
nobody while it's in the party) -> Friendship + 5 / 3 / 2 (Emerald's step sizes by Friendship band), once per memory
("life: moment (memory K) friendship +N"). Time is play time (MP_Life_PlayMinutes); MinuteTick (party only): needs
(capped at 120 minutes a step), places (level per minute there; level 200 + good mood -> TRAIT_LOVES_PLACE), mood,
drift, evolution (MEM_EVOLVED), reunion (a badge or 3+ hours of story progress apart). EmotionStep every 120 frames.
Sidecar "<save>.life" (header "LIFE" v1, saveCounter, counts, story/badge masks, news[16]; then records, bonds,
names), written after StoreSaveFile with the save's backups (.bakN.life), loaded after the save (exact counter first,
else the plain file); records of another size are copied field by field. Limits 640 records, 4096 bonds, 256 names.
JOURNAL summary page (pokemon_summary_screen.c PSS_PAGE_JOURNAL after the POKé BALL page; mp_life_journal.c):
sentences written as the Trainer's observations ({STR_VAR_1} Trainer, {STR_VAR_2} place, {STR_VAR_3} other), facts
revealed with familiarity or by WATCH (unknown ones are one "??? (not known yet)" line), on a lined sheet (INFO rows
4-7), FONT_NARROW, 7 rows a page, A turns pages. The "Ⓐ MORE" prompt (its own promptWindow at 22,0, baseBlock 762) shows
only when there is more than one page; A on a one-page journal does nothing (MpJournal_HandleInput still takes it, so it
never reaches the INFO page's move selection). LEFT on the first summary page goes to the last and RIGHT on the last to
the first; test summary_wrap. INFO is the bottom layer (BG3: its page in the tilemap's left half, the EGG layout in the
right half), the other pages sit in the right half of BG1 / BG2 (bgDisplayOrder 1: BG1 on top), so the wraps at INFO
can't use the stock scrolls (1.23.0): RIGHT round to INFO (PssWrapToInfo) slides the page on top away to the left with
the layer under it at its blank half, then order 0 as when the screen opens; LEFT round from INFO (PssWrapToLast)
slides the last page in over INFO from the left on BG1 (x 0 down to -256), then order 1 with BG2 holding the page before
it at 0x10000, ready for the next LEFT (before, the last page's layer sat at its blank half and the POKé BALL page's
background showed under the JOURNAL). A wrap from a page on a layer (picking a move) stages the last page under it, in
view. JOURNAL stripes: MpJournal_PaintStripes repaints its sheet (rows 4-19 of [JOURNAL][1]) a sentence at a time
(rowEntry; INFO rows 4-5 green, 6-7 white), so a wrapped sentence keeps one color. Test summary_wrap_shots.
`mp summary` hides the map name popup first, as the START menu does: its task kept running and erased its corner of
the summary's BG0 (the dex number's top row, the SKILLS title) when it timed out.

G2 field life. Emotes (mp_life_emote.c): 16 round pink bubbles drawn in code (NOTE HEART ZZZ EXCLAIM QUESTION SWEAT
ANGRY TEAR SPARKLE FOOD DOTS CALM SUN WATER BALL LEAF), never the Trainers' square emotes; one co-op palette
(TAG 0x5720); slots: local follower 0, friends' followers 1 + id, Garden 1 + MP_MAX_PLAYERS + i; 90 frames with a
pop and bob. The follower on its own (mp_follower.c auto*): while its Trainer stands still (same tile, no held
movement, MP_IsLocalFree) for 90 + (255 - lively) / 4 frames, mp_life_field.c picks a weighted activity: LOOK,
WANDER, INVESTIGATE (by an object, grass, water), FORAGE (hungry), NAP (tired), PLAY (inviting: the BALL bubble),
COMPANY (sits by its Trainer: social, timid or warm), QUIET (apart: unsocial), TYPE (its type need: soak, bask,
dig...), HINT (rare: curious or PICKUP, 1 in 8, sniffs at a hidden item's tile: BG_EVENT_HIDDEN_ITEM not yet found).
It walks greedily (AutoCanStand: no collision, objects, water, ledges, doors, warps, ice or deep sand, same
elevation, within 6x4 tiles of its Trainer), goes, does (a pose: SIT NAP HOP SHAKE SPIN LOOKUP), and the need it
served drops every 30 frames. When its Trainer walks: within 2 tiles it hurries into the old tile at double speed,
farther it's placed back (PlaceAtOwner clears the autonomy; the brain drops a stale activity). Cues: needs show as
bubbles shaped by temperament (a proud one hides tiredness, a dramatic one flops). Surroundings every 120 frames:
weather and darkness (first snow / ash / sandstorm memories, thunder fear or love, a fright memory and TRAIT_AFRAID
that new good experiences soften). Places: map sections; a new town or route is MEM_NEW_PLACE.
Interaction (mp_life_interact.c): A in front of the follower = the smart verb (asleep: let it sleep; upset: comfort;
inviting: play; proud: praise; else talk); A held 20 frames = the verb wheel (mp_menu.c, the quick-pick's window:
pages TOGETHER PET PLAY FEED TALK / HEART PRAISE COMFORT GROOM SIT WITH / QUIET WATCH KEEPSAKE; let go of A to pick).
Each verb's answer depends on fit and boundaries: a crossed boundary gets a sidestep, insisting a brief huff, and the
bond is never marked; repeats within 60 s mean little. FEED picks a BERRY by taste (a full one declines: no berry
used); a hungry or foraging one remembers it (MEM_FED, a moment). PLAY while inviting, PRAISE when proud, COMFORT when
upset (softens fears) leave memories; WATCH reveals a journal fact; KEEPSAKE makes the held item its keepsake.
Story (StoryTick, every 120 frames): sStory flags (rival battles, Wally, Magma / Aqua beats, the legends, the
Elite Four, the CHAMPION) and badges -> PartyLives: the follower and the lead were part of it, the rest witnessed;
news for word spreading (MP_Life_News); a save from before the Garden doesn't relive the past. Asking to come out
(MP_Life_WantsOut: curious + an explore need or a loved place, social + company, lively + play; asks in about one
10-minute spell of three): its party icon wiggles (party_menu.c SpriteCB_MpWantsOutIcon) and the hold-SELECT picker
says WANTS TO COME. Debug: `mp life show|remember|minutes|trait|need|feel|temper|verb|emote|act|field|story|wants|
count|save|load|reveal|journal`. Tests life_core, life_field, life_story.
G3 battles (mp_life_battle.c, called every frame from MP_FrameUpdate). Presentation only. It reads what every client
keeps current in a link battle: party HP (the controllers' SETMONDATA), gBattlerPartyIndexes, gAbsentBattlerFlags
and gBattlerAttacker (sent with each command), so host and guests see the same moments. Slots are party indexes
(ally = gPlayerParty, foe = gEnemyParty); ours = player side and, in co-op, MP_Coop_BattlerPlayer(b) == gMpLocalId
(STEVEN's half excluded). Kinds: wild, trainer (+25 strength), boss (+70: LEADER, ELITE_FOUR, CHAMPION, RIVAL,
Magma / Aqua leaders and admins), legend (+70: legendary / roamer flags), PvP (+50); recorded, Wally's tutorial and
Safari are skipped. "Won" = every gEnemyParty Pokémon at 0 HP (a guest's gBattleOutcome comes late). At the end:
MEM_FAINTED (85 + importance; 85 wild), MEM_NEAR_FAINT (survived at <= 1/8 HP and won: 90 + importance, +25 at 1 HP),
MEM_CLUTCH (the last knockout: boss / PvP always, trainers at <= 25% HP, wild at <= 12%), MEM_COMEBACK (won after our
side was down to one able Pokémon against two or more: 150 + importance), MEM_PARTNER for a save (a knockout while a
partner on the field is at <= 1/5 HP: the saved one 130 + importance/2, the saver 110 + TRAIT_PROTECTIVE). Side by
side: trust and respect +1-2 a battle. Foes: only notable ones (PvP, boss classes, legends) or one that knocked it
out get a bond (trainer Pokémon are keyed by trainer name + evolution family, so May's TORCHIC and COMBUSKEN are one
rival; names "MAY's MARSHTOMP" in the name table, 23 letters): respect +2..8, friction only for competitive
temperaments (proud >= 170, or bold >= 180 and warm < 110); first meeting across a battle +10 respect; notable foe
met 3+ times with respect >= 40 -> LIFE_BOND_RIVAL + MEM_RIVAL. Routine wins: TRAIT_CONFIDENT +1 (wild) / +2, pride.
Pokémon battle emotes: LIFE_EMOTE_SLOT_BATTLER(b) (= the Garden slots, unused in battle), at the sprite's top
corner, OAM priority 0 in battle, with mp_life_sound.c's bloop (up: happy, down: worried); at most 6 a battle and one
per Pokémon per 150 frames: low HP (sweat; proud: sparkle; fierce: angry), a clutch knockout (sparkle), saved
(heart) / saver (sparkle), a warm partner fainting (tear), a rival met again (exclaim / angry), a feared legend
(sweat), boss / PvP winners (note / sparkle). Rare lines: LN_SAVED / SAVED_BY (best friend per MP_Life_Closest
saved at <= 1/8 HP), FIRST_GYM (a LEADER with no badges at the start), CHAMPION, COMEBACK (boss / PvP, <= 25% HP),
RIVAL (a rival bond shared >= 5); each in four temperament flavors; at most one per 300 play minutes for the save
(header lastLineMin; `mp life battle lineforce 1` for tests). Shown by MP_LifeBattle_TextHold, which every battle
controller's print-done function calls (player, partner, opponent, link opponent, link partner, recorded opponent):
when the moment's message (TARGETFAINTED / ATTACKERFAINTED, or PLAYERDEFEATEDTRAINER1 / TWOENEMIESDEFEATED /
BATTLEEND) finished, it lets it be read 24 frames (A/B after 8 outside link battles), prints the line through
BattlePutTextOnWindow with gMpBattleTextPink (fg = BG color 8, unused in the box art and every battle text, set to
pink; shadow = the box's rose 14) at the battle's text speed, plays the chime (synthesized: three rising bells), and
completes once printed; the script's own hold follows. Local only, so link battles stay in step (a guest falls a
line behind and catches up). A line whose message never came is dropped and not spent. Word spreads (mp_life.c
SpreadWord in the minute tick, 1 in 3): news (badges, story, legends, and boss / PvP clutch and comeback wins with
their teller) is told by a party member who lived it to one who wasn't there: MEM_HEARD (who = teller, param =
kind << 8 | param, tier HEARD halves it), a proud teller makes it bigger (+pride), a timid one makes legends scarier
(+fear); a rival's boasting irritates (+friction), anyone else's makes it proud by association. One telling at a
time, one hearing per 30 minutes per listener. Debug `mp life battle state`, `mp life bond A B AFF TRUST`, `mp life
forget SLOT KIND`, `mp life spread`. Tests life_battle_gym, life_battle_save, life_word.
G4 among friends (mp_life_social.c; protocol 41). Wire state 36 bytes: followerKey (u32), lifeAct, lifePose, lifeDx /
lifeDy (its own tile from its Trainer's while on its own, MP_LIFE_NO_AUTO = following), lifeEmote + lifeEmoteSeq (a
change shows the bubble in slot LIFE_EMOTE_SLOT_REMOTE(id)). mp_follower.c DriveRemoteAuto walks a friend's follower
to that tile itself (smooth at any lag), shows the pose on arrival, and walks it back (FinishReturn, shared with the
local return) when their game says it follows again; only slot 0 tells mp_life_field.c about hurrying back. Being
near a friend (8 tiles, every 600 frames): MP_Life_BondNudge (no shared-experience count) familiarity with their
Trainer (follower up to 120, the party up to 60) and their follower (80); names noted ("ALICE", "ALICE's SKITTY").
Meeting (mp_life_field.c LIFE_ACT_MINGLE, chosen like any activity: 40 + social/4 + company/4, +60 to greet a
Trainer it's fond of (affection >= 120: it runs to them)): needs the friend standing still 90+ frames within 6
tiles; it goes beside their follower; the bubble from the bond (first meeting: shy sweat / warm heart / curious ?;
friction: a squabble; lively: tag) and MP_LifeSocial_Met: BondShare (+3 familiar, trust / affection by temperament,
respect / friction for competitive ones), MEM_FRIEND once a friend (affection 150, trust 120); then 2 minutes
before meeting that friend again. Friends' verbs (A / held A in front of a friend's follower:
MP_LifeSocial_TryFriendFollower, before our own follower's): MPR_LIFE_VERB {verb, item} -> its Trainer's game
answers (Answer: asleep, boundary = sidestep with no mark, shy with strangers, FEED only when hungry (the berry
leaves the friend's BAG only when eaten), PLAY for lively ones, PRAISE / COMFORT when it fits; a favorite Trainer
(MP_Life_Closest favoriteKey) is greeted "overjoyed"; the same friend again within a minute only adds familiarity;
MEM_FRIEND when affection reaches 120) with a reaction (MP_LifeField_Reaction, synced) and MPR_LIFE_REPLY {verb,
answer}; the friend sees "ALICE's TORCHIC steps aside. Not that, thanks." etc. Never Friendship. A new LOOK
(appearanceVersion changed) of a friend our follower knows (familiarity 20+): EXCLAIM, a sniff (QUESTION + shake;
shy ones wait longer), HEART; after 3 changes in a while just a QUESTION (the count fades one per 5 minutes).
Records travel: MPR_LIFE_RECORD kind 2 (a trade: mp_trade.c ApplyTrade sends the outgoing record, its bonds and
their names: the newest word, replaces ours; the bond with its first Trainer is marked TRAINER; MEM_MET traded),
kind 0 (ask: a new record asks the room, each key once a session) / 1 (answer: fills in only a fresh record of the
same species: <= 1 memory and under 30 minutes old). MP_Life_PackRecord / UnpackRecord (sizes in the message,
known[] reset for the new Trainer, a bond with the new Trainer made). Tests: MOEMON_DEBUG_PID_SALT=N gives a
client's debug party its own personalities (test clients otherwise get identical ones). Debug `mp life social
bonds|mingle|verb PLAYER V|trainerbond PLAYER N`. Tests life_social, trade.
G5 the Garden (mp_garden.c, mp_garden_life.c, mp_garden_decor.c, mp_garden_watch.c, mp_garden_net.c). The map:
MAP_MOEMON_GARDEN (group 0 num 57, appended so no map id moved; LAYOUT_MOEMON_GARDEN appended), 34x20, General +
Petalburg tilesets, made by tools/moemon_garden/gen_map.py from Littleroot's and Petalburg's own metatiles (their
collision / elevation bits kept; Petalburg's pond twice, its stair pieces 250/251 made shore) and its facts in
src/mp/mp_garden_map.h. It's connected west of LITTLEROOT (both map.jsons: left / right, offset 0; Littleroot's
west edge rows 4-16 were already open grass), region map section LITTLEROOT, no name popup of its own; the
GARDEN BOARD is a sign (bg event) running special MP_Garden_OpenBoard (waitstate). fieldmap.c InitMap /
InitMapFromSavedGame call MP_Garden_OnMapLoad: growth marks stamped with MapGridSetMetatileIdAt from the group's
badges (the most among Trainers on the map: 1 tall grass 00d, 2 the 3x3 puddle set c8-da, 4 sand 11c, 5 the storm
stone (a PUSHABLE_BOULDER sprite), 6 two grove trees (tops 1ce/1cf, trunks 1d6/1d7 with collision)), and the hedge
(cols 12-13, forest pattern) opened to grass when a friend is on the map (closed again once they've gone and nobody
stands on it). First visit: MEM_GARDEN for the party, a welcome note, MP_GARDEN_VISITED -> the PC's MOéMON GARDEN
row (player_pc.c: option orders with MENU_GARDEN, the window as tall as its rows, which PC told by sMpBedroomPC;
a Pokémon Center PC ends with EventScript_TurnOffPC so it doesn't go back to its own menu) -> the warp once
MP_IsLocalFree, MP_GARDEN_SHORTCUT (cleared on walking out of the Garden) -> the board's GO BACK to the saved spot.
Save data: struct MpGardenSave in the .life header (flags, the return spot, 16 decorations).
Garden Pokémon (actors, up to 24): in the Garden followers are hidden (LocalHidden / RemoteHidden ask
MP_Garden_HidesFollowers; mp_life_field.c pauses) and the whole party lives there. Each actor has its own sprite
(mp_follower.c MP_Follower_CreateMonSprite, palette TAG_GARDEN_PAL + index), positioned from the player's sprite
like followers are (UpdateAnchor: the player's position in 1/256 tiles from its sprite's movement), a tile people
can't walk through (field_player_avatar.c CheckForObjectEventCollision asks MP_Garden_ActorAt, which also covers
decorations), greedy steps around obstacles (CanStand: the Garden's open area, no collision, water, objects,
decorations or other actors). Their minds (ours only; friends' games run theirs): weighted choices from needs and
temperament: WANDER, LOOK, NAP (a cushion first, else a cozy spot), FORAGE (tall grass, flowers), TYPE (the type
need where the Garden has it: pond edge, flowers in the sun, sand, tall grass, the grove, the storm stone, puddles),
PLAY / CHASE with the warmest friend nearby (it's invited and joins; tag: it runs and the other follows), SQUABBLE
with a rival (friction 60+ or LIFE_BOND_RIVAL), COMPANY beside the Trainer, QUIET in the emptiest corner, DECOR
(dolls and big dolls: play). Shared play: BondShare at the end (MEM_FRIEND when it becomes a friendship, MEM_PLAYED
now and then; not about the Trainer, so no Friendship); squabbles: respect, friction for competitive ones. Emotes
in LIFE_EMOTE_SLOT_GARDEN(i). A in front of one: mp_life_interact.c's tap / hold-A with a target (sTarget: React /
TargetActivity route to the actor), a friend's: MP_LifeSocial_TryFriendPokemon (MPR_LIFE_VERB carries its key; their
game answers through its bond with us and reacts on that actor). Decorations: DECORPERM_SPRITE ones (dolls, big
dolls, cushions) drawn with CreateObjectGraphicsSprite(gDecorations[d].tiles[0]); DECORATE on the board is the
SECRET BASE's decorating (1.18.0, below); decoration.c marks Garden ones in use (sMpGardenItemsIndicesBuffer: blue
ball, can't be placed twice, placed or tossed elsewhere). A favorite grows
from repeated use (favDecor = type | count << 8; 5 = a favorite: +40 to choosing it, a journal line). WATCH (board):
starts once the board's script ended; SpawnCameraObject, the camera walks (slow) toward the most interesting actor
(play / tag / squabble 50+, decorations 40, nap piles, type needs...; changes after 8 s), A describes it in plain
words, B walks the view back and RemoveCameraObject; each new focus reveals a temperament fact (and a favorite
decoration). With friends (mp_garden_net.c, MPR_GARDEN_FIRST 250/251): every 10 frames when changed (heartbeat 2 s)
each game sends its actors (key, dex, female, hue, target tile, dir | pose << 4, act, emote + seq: 15 bytes each)
and every 3 s (at once when someone arrives) its decorations; receivers walk friends' actors to their tiles
(MP_GardenLife_SetRemote, a jump of 5+ tiles snaps), show their bubbles and decorations, and drop them when that
friend leaves. Debug `mp life garden state|shortcut|actors|force N ACT|talk N|decor give D|decor place D|decor list|
watch|board|pc`. Tests garden_map, garden_life, garden_shortcut, garden_together.

G6 long-term life (protocol 42: the Garden activities below go out in the actor relay; GA_* now lives in
include/mp_garden.h for mp_garden_life.c and mp_garden_watch.c).
Pursuits (MpLifeRec.pursuit / pursuitLevel; GA_PURSUIT): MP_Life_PursuitFor picks the one that fits the current
species' instincts and type needs and its temperament (CLIMB: INST_TREE / SKY or TNEED_HEIGHT / SKY; SWIM (splashing at
the pond's edge: actors never enter water): INST_SEA or TNEED_SOAK; SING: INST_SING; COLLECT: INST_COLLECT, TNEED_SHINE / STONE; DIG: INST_DIG, TNEED_DIG; TEACH: gentle or
warm 170+ and social 110+), several fitting: one from its personality. It takes it up the first time it goes for it
(a low weight until then). Where: the grove (or, before it grows, a tree at the Garden's edge: T_ByEdgeTree, never the
board or the hedge), the pond edge, flowers / open ground, the storm stone or sand, sand, beside a younger (5+ levels
lower) party Pokémon who sits and watches (learner: trust and respect for its teacher). Each session +2-5 (+1 lively);
milestones at 60 / 150 / 250 are MEM_PURSUIT (param pursuit << 8 | 0-2, a moment for the party), LIFE_FACT_PURSUIT and
a sparkle hop. The look changes with the level band (pose / bubble from sPursuitLook: a beginner's sweat or dots, then
notes, then sparkles), sessions get longer (7 / 9 / 11 s). Journal: "Loves climbing the GARDEN's trees." and the
milestone lines ("Reached the top branch!").
Rare moments (GA_RARE, LIFE_RARE_*): tried first in Choose, 1 in 12 when it all lines up, each kind once a visit
(sL.rareDone), only between our own Pokémon within 10 tiles. STARGAZE: both ways affection 180+ and trust 140+, a
clear night, one curious 120+ or warm 160+ (open ground, side by side, looking up). PADDLE: a warm (130+) Water type
and a Fire type that's timid (bold < 120) or afraid of water, trusting it 110+ (pond edge; the learner's sweat turns to
splashing halfway; its water fear softens by 40). RIVAL_NAP: a rival bond, 8+ shared, respect 120+, both tired, 12-17 h.
FAMILY: a family bond, 17-21 h, a warm one in a good mood grooms the other. The leader keeps the books at the end:
MEM_RARE for both (param kind | role << 8: role 1 = the other one led), bonds, hearts. The journal keeps it only if the
Trainer saw it (Seen: within 7x5 tiles of the player, or of the WATCH view, at any 30-frame check), otherwise its
memory's known bit is cleared; rareSeen collects the kinds seen. WATCH: interest 95, its own sentence.
Eggs and family: daycare.c _GiveEggFromDaycare calls MP_Life_NoteEgg with the save DAY CARE's guests: a record made now
(LIFE_REC_EGG, parents[] = their keys, RollTemperament then a quarter of the way toward the known parents' born
temperaments; a third of eggs, from the personality: a family food (a parent's comfort food or loved flavor, now a
TRAIT_COMFORT_FOOD), TRAIT_FAMILY_SONG or TRAIT_FAMILY_GREETING). Party EGGs count as owned (FindOwnedBox) so the
record isn't dropped. Hatching keeps personality and OT, so the next MP_Life_Get finds it: FinishEgg (owner bond, the
MET memory "Hatched in ...", name, family bonds with parents we have records of). FamilyCheck (minute tick): parents,
children and siblings (a shared parent) in the party meet once (LIFE_BOND_MET on the family bond): bonds, MEM_FAMILY
(param 0 its little one, 1 its parent, 2 its sibling). The family song: GA_SONG with family nearby, and the follower
hums now and then; the greeting: a twirl and a heart when it comes to keep the Trainer company.
Evolution: MP_Life_Get notices it at once for a party Pokémon (EvolutionCheck, only when the new species is in the old
one's gEvolutionTable: SHEDINJA shares NINCADA's personality). Instincts and type needs read rec->species, so they
switch; temperament, memories and bonds stay. MP_Life_NewBody: MEM_EVOLVED in the last 15 minutes. In the Garden it
looks at its reflection by the pond or stumbles (up to 3 times a visit); on the map the follower stumbles while walking
(SWEAT + SHAKE) or looks itself over when standing (EXCLAIM + SPIN), up to 4 times. Friends react the first time they
see it (MP_Life_EvolutionReaction, once per pair, needs familiarity 20+): JEALOUS (rival or friction 90+: frustration,
respect and friction), PROUD (warmth 250+ or family: pride, joy, affection), else AWED; Garden actors walk over
(GA_REACT), the follower shows it on the map. A Pokémon that evolves in the Garden gets its new sprite there.
Debug `mp life egg A B|hatchsoon|evolve SLOT SPECIES|evoreact W E|familiar A B V`, `mp life garden rare N KIND|
pursuit N LEVEL`. Tests garden_longterm, life_family.

1.15.1 (protocol 43):
- The follower's way back (mp_follower.c StartReturn / PathStep / FinishReturn). When its Trainer walks on while it's
  off on its own, it doesn't pop over: it heads for the tile they just left (trail, updated each step), on a shortest
  path within AutoCanStand's area (breadth-first, 13x9 tiles around the Trainer), at twice their pace (2 / 4 / 6
  pixels a frame for walking / running / cycling), and falls in behind them once it's on that tile, taking up their
  step where it is (no jump). Beside a Trainer who stands still it follows from there; boxed in for 20 frames: within
  3 tiles it follows from where it is, farther it's put back behind them (the last resort, logged). Only a warp
  (a jump of 4+ tiles) puts it straight back. Tests life_return (MOEMON_FOLLOWTRACE: tools/test/check_followtrace.py,
  no step over 12 px in a frame).
- Arrival bubbles (mp_life_field.c Arrival) only when the map section changes (a town, a route: not every door), and
  a new place only in its first minute; nothing on loading. Test life_arrival (check_arrival.py).
- Watching (mp_life_journal.c MP_Life_Observe): WATCH QUIETLY and the Garden's WATCH teach only what can be seen
  (temperament, the type need, the pursuit; never memories), one thing per Pokémon per 30 play minutes
  (MpLifeRec.watchMark, the old pad byte). Test life_watch (`mp life playtime M`, check_watch.py).
- Movement parity. Players were already replayed step by step (mp_overworld.c's queue of positions). A follower on
  its own now sends the tile it's stepping onto and the one it stepped from (lifeDx / lifeDy and lifePx / lifePy, from
  its Trainer's x / y in the same state) with its facing and pace (lifeFace); friends' games queue those tiles
  (only when the tile is new: a resent state queues nothing) and replay them in order (FollowerSlot.remoteDriven),
  then follow from where it stopped. Garden Pokémon the same (18-byte actors: cur, from, speed; every 4 frames when
  changed; SetRemote's queue). Tests move_parity and garden_parity at NA-JP lag (MOEMON_MOVETRACE logs every
  player's, follower's and Garden Pokémon's tile and facing; tools/test/check_movetrace.py compares the two games'
  lists: the same, in order, the viewer allowed to trail by up to 3 steps when the games quit).
- DEOXYS's icon (pokemon_icon.c GetMonIconTiles): Kai's icon has one form, so the PC box and party menu no longer
  read Emerald's Speed Forme 0x400 bytes on (past the end: garbage).
- Crash fix, co-op battles (1.15.0 crash reports: switching after a paralysis, using a REVIVE). Opening the party
  menu in battle puts gPlayerParty in battle order (party_menu.c UpdatePartyToBattleOrder) while
  gBattlerPartyIndexes keep the field order, so mp_life_battle.c's Poll saw "new" Pokémon in the slots. On the local
  player's battler holding a friend's Pokémon it noted the name as owned by MP_Coop_BattlerPlayer = gMpLocalId, whose
  gMpPlayers entry is never filled in: all zeros, and 0x00 is a space in the game's text, so StringCopy ran through
  the roster past a 40-byte stack buffer. Now: Poll and the battle emotes only run while gMain.callback2 is
  BattleMainCB2 (menus over the battle wait), names are copied with a limit (CopyName, MP_CopyPlayerName), our own
  name comes from the save, and a cleared roster entry gets an end marker (ClearPlayer, WELCOME, a state before the
  hello). Test coop_switch_twice (MOEMON_DEBUG_PID_SALT so the two players' Pokémon differ; it crashed before).
- A+B+START+SELECT (main.c) went through the wireless adapter (rfu_REQ_stopMode with gSTWIStatus never set up:
  1.8.0 crash report). On PC it now goes straight to DoSoftReset, which closes the game like CTRL+R. Test soft_reset.
- Menu text written with é in the source (UTF-8) keeps it (MP_AsciiToGame): the GARDEN BOARD read MOMON GARDEN.

1.16.0 (protocol 44): the MOéMON GARDEN is every POKéMON CENTER's second floor.
- The stairs: field_control_avatar.c SetupWarp calls MP_Garden_RedirectWarp after setting a warp's destination.
  A warp to one of the 17 CENTERs' 2F maps (mp_garden.c sCenters: 16 CENTERs + the POKéMON LEAGUE) goes to the
  Garden's warp 0 instead; the Garden's warp 0 goes to the last CENTER visited, at its escalator (the 1F warp that
  went up, found in the map's own data: EscalatorOf). The 2F maps, the CABLE CLUB, UNION ROOM and the rest stay in
  the game, unreached. Its own data names OLDALE's CENTER (never used: the redirect always decides).
- The escalator: the Garden's tileset (gTileset_MoemonGarden, tools/moemon_garden/gen_tileset.py) is Petalburg's
  plus the CENTER's 2F escalator at its own metatile ids (0x298/0x299, 0x2A0-0x2AD: fldeff_escalator.c animates by
  id) and the CENTER's floor and west wall at 0x2B0-0x2B2, recolored into palette slot 7 (14 colors). gen_map.py
  copies the escalator's neighbourhood from the 2F with its bits (the floor tile east of the escalator has
  elevation 0, the step between the escalator's 4 and the floor's 3; without it nobody can step on) and writes the
  warp and the GARDEN BOARD into map.json. Riding up plays the CENTER's own escalator animation (FieldCallback_
  EscalatorWarpIn: MB_DOWN_ESCALATOR at the destination) and ends on the floor east of it (GARDEN_ENTRY).
- The last CENTER: MpGardenSave.lastPc* (the old reserved bytes; MP_GARDEN_LASTPC), noted on every map load in a
  CENTER (NoteCenter). A save from before 1.16 has none: ReturnCenter matches the last heal location against the
  CENTERs' heal locations, and with nothing at all the way down goes to the last heal location itself (always real).
- Quick travel: START > GARDEN (start_menu.c, after OPTION, once the Garden was found, not inside it) and the PC's
  MOéMON GARDEN row call MP_Garden_QuickTravel; GO TO FRIEND to a friend in the Garden too (mp_linkx.c). Refused
  with a message (shown once the field is free): not before FLAG_SYS_POKEMON_GET, not in the SAFARI ZONE, BATTLE
  PYRAMID / PIKE, TRAINER HILL, a co-op battle or riding piggyback; from START, only where Fly would work (towns,
  cities, routes, the sea) and not the dungeon-like outdoor maps (sNoTravelMaps: PETALBURG WOODS, JAGGED PASS,
  MT. CHIMNEY, MT. PYRE, SKY PILLAR, SOUTHERN ISLAND, SAFARI ZONE), or in a CENTER; and only once a CENTER is known.
  You arrive beside the escalator (a fade), and it leads down to the last CENTER.
- One open meadow (no hedge: it read as a fence, and closed on a player left in the extra meadow when friends left).
  Garden Pokémon and decorations stay off the escalator's landing (GARDEN_PAD_*). Safety nets for older saves:
  standing where nobody could (StuckHere: a tree, outside the meadow) warps back to the landing; decorations whose
  place isn't open ground any more go back to the collection (MP_GardenDecor_CheckPlaces, also after growth).
- LITTLEROOT has no map connection to the Garden any more. The map name popup and friends' places say MOéMON GARDEN.
  Tests garden_map, garden_centers, garden_travel, garden_stuck (and the other garden_* at new spots).
- 1.16.1: the Garden's lone trees (and the grove growth mark, sGardenTree) are Rustboro's standalone tree (crown
  1CE/1CF, middle 1D6/1D7, trunk on grass 1E6/1E7), not a forest column's 1DC/1DD bottom half that joins the next tree.
- 1.16.2: the Garden's forest edge. Outside a map the game draws border.bin by map-coordinate parity (top halves
  1D4/1D5 on even rows), so the map is 26 rows (even): the bottom row of crowns (odd) sits over the border's top
  halves, and the corner columns (1DC/1DD on the top and bottom rows, no edge piece) run into it. The west column
  ends above the escalator's landing with 1E4/1E7 (a last tree over open ground, odd row) and starts again below it
  with a crown 1CE/1CF (odd row) and 1D4/1D7, as LITTLEROOT's edges do. The landing is 7 rows (one more wall).

1.17.0 (protocol 45): Garden habitats. A place for every type, and every badge adds one.
- The map is generated by tools/moemon_garden/gen_garden.py (garden_lib.py: layout / tileset loaders, metatile
  pictures, made-up metatiles, the tileset packer), which replaced gen_map.py and gen_tileset.py. 56x40 (even rows,
  as 1.16.2), only the original game's pieces put together the way its maps do:
  - Trees on a 2x2 grid (tops on even rows), joined by the rules LITTLEROOT / ROUTE 101-102 follow: top half 1D4/1D5
    when joined on that side or a tree above, else 1D6/1D7; bottom 1DC/1DD with a tree below, else 1E4/1E5 joined,
    1E6/1E7 at an end; a crown above a tree with none above it: 1CE/1CF on grass, 1C6/1C7 on tall grass, MAUVILLE
    278/279 on a path's bottom edge. Outside the map counts as trees.
  - Sand paths with the 3x3 set 118-12A as ROUTE 117 joins it (an inside corner is plain 121); General's pale grass
    1D0-1E2 and FORTREE's marsh 288-29A the same way; ponds as ROUTE 117's (rim 0B0/0B1/0B2/0B8/0BA, steps 0C2 / 0CC,
    the shore lip 002 below the last row); ROUTE 120's long grass 015 with its south edge (FORTREE 208).
  - Copied whole with their bits: ROUTE 117's flower bed (gray fence, MAUVILLE flowers) and rock outcrop (every cell
    made solid: ROUTE 117's bushes kept people off its top), LAVARIDGE's hot spring (walkable water, its steps kept),
    FALLARBOR's mossy boulders, FORTREE's marsh ponds (puddles) and bushes, ROUTE 117's dark hollow 14B-14D / 113-115,
    ROUTE 101's ledge 0D5 / 087 / 0D6, General's 3x3 puddle set.
  - Changed to fit (garden_lib CUSTOM metatiles, both layers under the sprites: METATILE_LAYER_TYPE_COVERED): cave
    crystals lifted off the cave floor onto grass (their floor's colours dropped), General's boulder in a pond
    0C5 without its water and foam (a gray stone), the outcrop's tip without ROUTE 117's bushes, the hot spring's
    rim with grass where LAVARIDGE's cliff was.
  - Bits of General pieces: what the original maps give each most often; of secondary pieces: what that tileset's
    maps give them (sec_bits).
  - The tileset: every secondary piece of every stage, the escalator's frames at their own ids (0x298/0x299,
    0x2A0-0x2AD); tiles copied (deduplicated) and their colours packed greedily into slots 6-12 (6 used). The tile
    count is written into graphics.h.
- Stages: the base map and one per badge; the GROWTH table (mp_garden_map.h sGardenGrowth, GARDEN_WANT_GROWTH) is
  every cell that changes from one stage to the next, in badge order. mp_garden.c ApplyGrowth stamps the entries
  above sG.growth up to the group's badges (the most among Trainers here) and logs `garden: growth N` per badge;
  live (drawn) when a friend with more badges comes in. Badges: 1 long grass field, 2 marsh, 3 boulder field, 4 dig
  field, 5 more shiny stones, 6 perch grove (two lone trees), 7 second flower bed, 8 stone circle. The storm stone
  (ELECTRIC) stands from the start (GARDEN_STONE_X/Y; MP_Garden_StoneShown).
- Zones (sGardenZones, GARDEN_WANT_ZONES): per TNEED_* the rectangles of its places with the badges that open them
  and what to face there. mp_garden_life.c TypePlace (GA_TYPE) tries FindZone first: a random standable tile in one
  of that need's open zones that the actor can reach and that passes the need's test (T_ByWater for SOAK / COOL,
  T_ByRock for STONE / SHINE (beside a rock, a crystal, a boulder, never a trunk or the ledge), T_Lookout for
  HEIGHT, T_Open for SKY / SPAR, T_Tall (tall or long grass) for FOLIAGE, T_ByStone for STORM); the old searches
  around the actor are the fallback. Pursuits SWIM / SING / COLLECT / DIG look in the SOAK / SUN / SHINE / DIG zones
  first. T_ByTree is "below a trunk" (1E4-1E7), any tree.
- The way there: Step follows a breadth-first count of steps back from the target over open ground (Flood /
  PathStep: in the Garden, no collision, no water, no decoration; cached per target per frame); Pokémon and people
  in the way only make it wait (40 frames, then it does it where it is); with no way at all it walks straight as
  before. A walk lasts up to 420 + 32 frames a tile of distance (walkLimit; it was 420 frames: far places now are).
  Friends' Pokémon still replay their own game's steps.
- Old saves: decorations placed in the old Garden go back to the collection once (MP_GARDEN_HABITATS), their
  places meaning nothing in the new layout. Debug `mp life garden place ACTOR NEED`; actors print their target.
- Friends' Garden Pokémon: turns replay in order with the steps (qDir: a turn entry on the queue after the steps
  before it, QueueTurn / PopStep, one turn a frame). Before, a facing that arrived while the copy was still
  finishing its last step was lost (a turn on arrival raced the step's end: garden_parity failed now and then);
  catching up counts steps only (QueuedSteps).
- 1.17.1: Garden Pokémon and decorations off the screen are hidden (Draw / DrawOne: sp->invisible from the screen
  position with gSpriteCoordOffset): OAM x wraps at 512 and y at 256, so in the 56x40 Garden far ones showed up
  again in the middle and at the top of the view (the old 40x26 one rarely put them that far). StuckHere ignores the
  ledge's tile (MB_JUMP_SOUTH) and only acts after 30 frames standing still (T_NOT_MOVING, no unfinished held
  movement): a jump down the ledge passed over its tile and sent the player back to the landing. DECORATE > PLACE
  ONE carries the decoration (sCarry: Task_CarryNote shows how, with field controls locked; then the decoration
  blinks on the tile ahead, slowly where it can go, quickly where it can't; field_control_avatar.c hands A / B to
  MP_GardenDecor_CarryInput: A puts it down there, B back): before, it was placed on the tile in front, which from
  the GARDEN BOARD is the board. Test garden_decorate.
- 1.18.0: the carrying locked the player for good after putting one down (ProcessPlayerFieldInput returning TRUE
  makes CB1_Overworld lock the field controls for a script that never came). Replaced by the SECRET BASE's
  decorating, as the user asked: decoration.c's MOéMON GARDEN mode (sDecorationContext.isMpGarden). The board's
  script: OpenBoard waits, VAR_RESULT 1 (DECORATE) goes on to `special MP_Garden_ShowDecorationMenu` and ends still
  locked, like the Secret Base PC's; CANCEL runs MoemonGarden_EventScript_DecorateDone (releaseall). The bedroom's
  rules (dolls and cushions only; the Garden's own message), the context's items = sMpGardenItems (the Garden save's
  list: pos[] can't hold a 56x40 map), CanPlaceDecoration -> MP_GardenDecor_CanPlaceAt (open ground, a big doll's
  other tiles clear, nobody, no decoration: ours, friends', the storm stone), PlaceDecoration -> MP_GardenDecor_PlaceAt
  (marks the copy in use), PUT AWAY's cursor -> MP_GardenDecor_OursAt / MP_GardenDecor_PutAway; no Secret Base
  sprites or scripts on the way back. Both end, as the Secret Base's do, with a reload of the map where the player
  stood: MP_GardenLife_KeepOnReload keeps the Garden Pokémon (their sprites come back by themselves), and while the
  player's sprite is the decorating cursor's they're hidden and still (a put-away one loses its sprite at once:
  MP_GardenDecor_Prune). The harness's yesno condition also sees menus' YES / NO (menu_helpers.c MP_MenuYesNoOpen).
  Test garden_decorate (placing, walking after it, the copy in use, PUT AWAY, walking again).
  Tests garden_places (each of the 17 types reaches a place of its own with 0 and 8 badges:
  check_garden_places.py), garden_together (a friend's badges grow your Garden live), the other garden_* at the new
  spots.

## BERRY farm (mp_farm.c, include/mp_farm.h; the map from tools/moemon_garden/gen_garden.py)
The MOéMON GARDEN's farm: two rows of 10 plots south of the main path by the landing (gen_garden.py FARM_ROWS:
soil x 7-16 on rows 23 and 25, the lanes 22 / 24 / 26 kept clear; mp_garden_map.h sGardenFarmRows). Plots open in
order, a row at a time: 4 + 2 per badge (MP_Badges, at most 20). MP_Farm_OnMapLoad (from MP_Garden_OnMapLoad, in
InitMap, before the objects spawn) stamps them framed as ROUTE 104 / 110 / 116 / 119 frame theirs: General's 14B / 14C
(soil, collision 1) / 14D on the row and the lip 113 / 114 / 115 below, round the open plots only (grass 0x001 past
them). Each plot is a BERRY tree object event of the game's own (map.json, generated: local ids 1-20, berry tree ids
100-119 = GARDEN_FARM_TREE_FIRST.., MOVEMENT_TYPE_BERRY_TREE_GROWTH, BerryTreeScript, hidden by FLAG_TEMP_1 + plot while
closed: set in OnMapLoad, so they never spawn); planting, watering, the stages and sprites, yields, regrowth, Emerald's
rules are berry.c's. Ours are the save's berryTrees[100..119] and grow with every other tree on our own clock (clock.c,
MP_ClockShift). When the open count changes while we're there (a host's farm heard, a room left), MP_Farm_FrameUpdate
restamps (drawn), sets the flags, removes or spawns the objects (spawn only in view, as TrySpawnObjectEvents would) and
runs MP_GardenDecor_CheckPlaces; every 30 frames CheckObjects puts a game loaded from a save (objects as saved) right.
Decorations stay off the farm's rows, lips and lanes, open or not (MP_Farm_IsFarmGround in GroundFree / CanPlaceAt).
Online (protocol 48), in a room someone else made (MP_RoomHostId: the MPR_RULES sender, while present), the plots show
the host's farm: berry.c GetBerryTreeInfo asks MP_Farm_Tree, which hands back the mirror for ids 100-119, and
GetBerryTypeByBerryTreeId / GetStageByBerryTreeId / GetBerryCountByBerryTreeId / RemoveBerryTree go through it (they
read the save directly before). BerryTreeTimeUpdate still works on the save: our own farm keeps growing, unseen, and
the save never gets the host's farm. The host (MP_Lobby_IsCreator) sends MPR_FARM_STATE (252: open plots, 7 bytes a
tree packed field by field: berry, stage | stopGrowth << 7, minutes (2), yield, regrowth | watered1-4 << 4, mpCare)
every 10 frames when changed, every 5 s, and at once to someone new. A friend asks before taking anything:
MPR_FARM_ASK (253: kind PLANT / PICK / WATER / TEND / NIBBLE, plot, berry, stage, seq). The BERRY tree script's
specialvar MP_Farm_AskPlant / MP_Farm_AskPick (constants/berry.h MP_FARM_*) answer GO at once on our own farm;
on the host's they return WAIT and the script waits (BerryTree_EventScript_FarmWait: waitstate) for MPR_FARM_ANSWER
(254: seq, ok, plot, berry, count) or 8 s (NO_ANSWER: "The farm's host didn't answer."). The host plants / removes on
its own tree only if the plot is still empty / still has those BERRIES ("Someone else planted a BERRY here just now." /
"Someone else picked these BERRIES just now."); a pick's answer carries the count really handed over, which
ObjectEventInteractionPickBerryTree adds (MP_Farm_PickedCount) and the message shows. A friend checks the BAG's room
first (no room: nothing asked, the game's "BERRIES POCKET is full"). Planted from the BAG (item_use.c removes the BERRY
first): a refusal gives it back (additem). Watering is sent with the stage seen (a late message never waters the next
stage); the change shows here at once and the host's next state settles it.
Garden Pokémon (mp_garden_life.c GA_FARM, ours only; friends' games run theirs on the same farm through the host): by
nature (FarmWork in Choose) a Grass type helps a plant grow (FARM_WORK_TEND: a tenth of the stage off, once a stage:
mpCare & FARM_CARE_TENDED = the stage), a Water type or a warm one (150+) waters dry soil (the stage's watered bit, as
the WAILMER PAIL), a hungry (110+) or mischievous (lively 160+, warm under 110) one nibbles one BERRY (yield - 1, at
least 1 left, once a crop: FARM_CARE_NIBBLED) of a plot ripe past half its time (minutesUntilNextStage <= 2 stages).
mpCare is a byte in struct BerryTree's padding (still 8 bytes: STATIC_ASSERT); BerryTreeGrow clears it when the BERRIES
regrow. One of our Pokémon to a plot; it walks beside it (FindNear), faces it (NOD + water bubble, CHARGE + leaf,
EAT + food) and does it at the end if the plot still needs it (MP_Farm_DoWork; a note or heart bubble). WATCH: interest
42, "is watering the BERRY plants." / "is helping a BERRY plant grow." / "is sneaking a bite of a BERRY!" (a friend's:
"is looking after the BERRY plants.").
Debug `mp farm state|plant PLOT BERRY [STAGE]|grow MINUTES|ripe PLOT`, `mp life garden force ACTOR 20` (GA_FARM: the
work its nature picks). Tests farm_solo, farm_together (NA-JP lag).
Watering shows the water on the soil (FLDEFF_RIPPLE on the plot every 32 frames, mp_garden_life.c DrawDoing); a
watering or tending that took ends with FLDEFF_SPARKLE on the plant.

## What Garden Pokémon do shows (mp_garden_life.c DrawDoing)
Every activity has to be seen, not imagined (the user's rule). Poses LIFE_POSE_SWIM / CLIMB / DIG (mp_life_field.h;
MP_LifePose_Base: NONE / NONE / SHAKE for the old sprites, mp_pmd.c: WALK paddling / WALK facing up / EAT):
- SWIM (TypePlace SOAK, the SWIM pursuit, the rare PADDLE's leader, and its follower halfway): on arrival EnterWater
  steps it onto the water beside it (the tile it faces first; nobody there), keeping the shore tile; at the end
  LeaveWater steps back (the shore, or any ground beside; no room: 30 frames more in the water). An activity cut
  short (act NONE while inWater) gets out first. In the water Actor.sink = a quarter of its height + 2, a dive right
  under every 300 frames (visT 200-249: height + 4, going down 3 a frame), FLDEFF_RIPPLE every 48 frames and at the
  dive's start and end. Friends' Pokémon: inWater = their tile is water (their game moved them there).
- CLIMB (the CLIMB pursuit, bands 1-2): Actor.lift up to 20 (a pixel every 2 frames), half of it toward the tree it
  faces, all of it up, drawn at OAM priority 1 (in front of the tree's top layer, which hid it).
- DIG (TypePlace DIG, the DIG pursuit): sink 3/5 of its height, FLDEFF_DUST every 24 frames.
The sink is drawn with sprite.h mpClipOn / mpClipY: nothing of the sprite's OAM entries from that screen line down
(the feet line: a PMD picture's ground point, an old sprite's bottom). AddSpriteToOamBuffer notes the clip for the
entries it wrote (mp_bigsprite.c MP_BigSprite_NoteClip; the side list is swapped in by MP_BigSprite_CommitOam with
the big sprites'), gba_easy_draw.c DrawSprites skips the lines. Any sprite can use it.
Garden Pokémon also get followers' ground effects (mp_followfx.c slots MP_FOLLOWFX_GARDEN + actor, 24 of them: grass,
sand, puddles, reflections; none while sunk, lifted or in the water). WATCH says "is swimming in the pond." / "is up
in a tree!" / "is digging in the dirt." (MP_GardenLife_Doing) except for pursuits and rare moments, which keep
their own lines (the SWIM pursuit: "is swimming laps in the pond."). Logs "garden: N swims in the pond at", "out of
the pond", "climbs a tree at", "digs at"; `mp life garden actors` prints sink / up / water; `mp life garden pursuit
ACTOR LEVEL [P]` picks the pursuit. Test garden_doing.

## 1.30.0: friends' arrows, items, battles, fixes
- WHERE IS EVERYONE (mp_compass.c, rewritten): drawn over the frame by MP_Compass_Composite (VDraw), no sprites or
  palettes. A world grid of Hoenn's outdoor maps (BuildWorld: breadth first over the map connections from Littleroot,
  each map's (0, 0) in world tiles, the first one found kept); an indoor map resolves to its door out (WorldPos: the
  fewest warps to a placed map, that warp's destination tile; cached). The same / a connected map still uses
  MP_GetRemoteMapPos; nowhere on the grid (underwater, islands) falls back to the Town Map areas. Arrows at the exact
  angle (a triangle by signed edge distances, soft outline), the label (small font, name + Manhattan steps) beside
  it on the inner side; a door in view = no arrow (its friend tag shows). Pings (MP_Compass_PointAt) are the same
  arrows, smaller, blinking, inset. Log "compass: player N at dx,dy (S steps)". Test compass.
- Town Map heads (region_map.c, mp_avatar.c MP_Avatar_HeadIcon): the look's standing front frame, the top 16x16 of
  what's drawn, centered, outlined (an unused palette index) in the player's color (MP_Compass_PlayerColor), white
  for yours; Brendan's / May's head only if it can't be built.
- RARE CANDY by the handful (party_menu.c sMpCandy, ItemUseCB_RareCandy): a count window (sMpCandyCountWindowTemplate)
  up to the candies held; Task_MpCandyNext applies one candy per level, a level that teaches a move shows its line and
  goes through the usual move tasks (PartyMenuTryEvolution comes back to Task_MpCandyNext while active), then one
  stats window from the first level to the last and evolution. Unused candies never leave the BAG ("N RARE CANDIES
  went back in the BAG."). Test candy_many.
- Smart targeting (mp_smartcursor.c MP_SmartItemSlot): status cures (item effect byte 3) and revives (byte 4
  ITEM4_REVIVE); in battle the battler choosing (gBattlerInMenuId), then other actives, then the rest; faints noted by
  MP_NoteFaint (Cmd_tryfaintmon, field poison, `mp hp SLOT 0`). Log "smart cursor: item I -> slot S". Test smart_cursor.
- PP beside moves (ShowMoveSelectWindow, the window 14 tiles wide). Test pp_shown. Held HMs back to the BAG on
  SetPlacedMonData into a box (test hm_deposit).
- Co-op levels: MPR_COOP_RESOLVE boxes carry the Pokemon's moves and PP (25 bytes): the moves learned without
  forgetting one (state 14) reach the owner's own copy in the battle (they were only in the master's). Tests
  coop_learn_fit, coop_learn_use, tri_learn_use, double_learn (`mp bmons`).
- Followers ride in their ball on a bike (mp_follower.c BikeBall: the recall as their owner gets on, the toss and
  entrance as they get off; friends' too, from their avatarMode). Test bike_follower.
- Fixes: the card screens tick the fanfare and finish it on close (sound.c MP_Sound_TickFanfare / EndFanfare: a
  CARD COUNTER prize left the BGM paused and a door's fade waiting for ever; test_ui.sh fanfare); contests get a
  stand-in DisableStruct for FURY CUTTER / ROLLOUT (battle_anim.c; test contest_furycutter); the Safari Zone's back
  picture is the look's (battle_controller_safari.c; test safari_look); a friend's DEOXYS shows this game's form in
  co-op and PvP (ShouldIgnoreDeoxysForm, BattleLoadOpponentMonSpriteGfx; test coop_deoxys); map people whose look is
  a variable (0-B) never take the player's own sprite (event_data.c VarGetObjectEventGraphicsId: a variable at 0
  made a copy of the player that took their palette), and the Lilycove Lady's POKeMON always has a POKeMON's look.

## The FRIEND FINDER (1.31.1)
- mp_compass.c over WHERE IS EVERYONE: hidden until INPUT_ACTION_FRIEND_FINDER (right stick click in gamepad.c's
  overworld context, G in sdl2.c) calls MP_Compass_Toggle (SE_PC_ON / SE_PC_OFF, log "friend finder: shown /
  hidden", saved at once: ini friend_finder_shown). OPTIONS > CO-OP > FRIEND FINDER = gMpConfig.finderStyle
  MP_FINDER_BOTH / ARROWS / RADAR (ini friend_finder = both / arrows / radar; `mp finder N`).
- sFade (0-16, 2 a frame) fades the whole finder in and out, also around scripts; sWalk (up to WALK_DIM) takes it
  down while tileTransitionState moves and for 20 frames after. Arrows: scale 11, alpha QUIET_ALPHA, still; a
  friend who goes away (off screen and NEAR_STEPS = 15 or more steps; m->away turning on) or the toggle gives
  FLASH_TIME frames at full size, alpha and bob. Pings (sPings) ignore the toggle, the fade and the dimming.
- The radar: RADAR_R 20 at the top right, a dot per friend with an offset (m->dot, also near or on screen), at
  (R-4) * d / (d + 24) from you (d in tiles), at the rim when only the area is known (m->exact off). EdgePoint only
  runs for away friends (0, 0 divided by zero). Test compass: a_hidden, a_pc_door_flash, a_radar, a_arrows,
  a_walking, a_hidden_again.

## The GAME CENTER and full-color map pictures (1.32.0)
- mp_mappic.c: pictures pinned to maps, drawn by the renderer past the GBA's tiles and palettes. gba_easy_draw.c calls
  gMpMapPicLine(line, layers) after the BG lines render (mode 0); MapPicLine puts the picture in BG3 and clears BG1/BG2
  where it's drawn (sprites on top), and its "over" picture in BG1 (over the sprites). A screen pixel's map pixel:
  (pos - 7) * 16 + ((x + BG3HOFS - tileOffset * 8) & 255) (GetFieldCameraTileOffset, field_camera.c). A room picture
  fills the rest of the screen (fill: black, the rooftop's sky); a picture on a connected map shows across the edge
  (FindShown: the connections). Fades: per frame, per channel c -> a + b * c from PLTT against gPlttBufferUnfaded
  (BG palettes 0-12). MP_MapPic_Update runs each overworld frame (OverworldBasic).
- tools/moemon_gamecenter/gen_gamecenter.py: graphics/moemon/gamecenter/*.png (the user's pictures, docs/user_images
  38-42) to RGB555 .bin (bit 15 drawn) and *_over.bin (furniture rising into the walkable cell north of it, scanned
  up from that cell's bottom until its floor colors; OVER rects for the clerks' rows; NO_OVER), the floors' layouts
  from the FLOORS grids on Building + Shop ('.' floor 0x001, '#' blocked, 'W' exit mat 0x006, 'S' stairs 0x250 with
  floor below for the arrival step, 'C' counter 0x232), and the region map icon (src/data/mp_gamecenter_icon.h).
- Maps RustboroCity_GameCenter_1F/2F/3F/Rooftop (gMapGroup_IndoorRustboro 17-20, appended). The tower in RUSTBORO at
  pixel (407, -47), doors (28, 7) / (32, 7) = Rustboro warps 12 / 13 on metatile 0x35E (a copy of the plaza's 0x2BB
  with MB_NON_ANIMATED_DOOR, appended to the Rustboro tileset); the trees and fence around it reworked (grass, plaza
  edges). Admission VAR_GAME_CENTER_PAID (0x404E): set by the receptionist (500), cleared arriving on the doormats
  (1F on-transition), coord events on 1F row 4 push back while 0; the elevator (MULTI_GAME_CENTER_FLOORS) too.
- 2F CARD SHOP: Tcg_GameCenterShop -> tcg_ui.c UI_GCSHOP (SINGLES: the binder in GC_BINDER, Tcg_ShopPrice 300 / 800 /
  4000 by rarity, holos never; DECKS: tcg_structure_decks.h from gen_structure_decks.py, the 1999 Base Set theme decks
  by their Base Set numbers, GC_DECK_PRICE 15000, into the first empty deck slot). The CARD CLUB's CP pick is commons
  and uncommons now (Tcg_PointCost). The table's players: Tcg_TalkChallenge (waitstate; FinishTalk resumes the
  script); on 2F townsfolk never decline (InGameCenterCardShop). The second seller only says he's unpacking.
- 3F: flavor on every console, TV and cabinet; the COIN counter sells 50 COINS for 1000. Rooftop: flavor people, the
  Lilycove rooftop's vending machine script.
- Region maps: region_map.c MpPlaceLandmarkIcon, a 16x16 tower beside RUSTBORO (Town Map, MOENAV, FLY).
- Tests gc_look (pictures, clerks behind counters, the Town Map), gc_visit (gate, admission, stairs, shop deck and
  single, a duel asked by talking, elevator, rooftop, exit). Debug: mp face DIR, mp textspeed N.

## The VS cabinet: MUGEN fights (1.31.0, 1.33.0)
Decisions and the as-built record: docs/FIGHT_DECISIONS.md. Where things are:
- `fight/`: the engine (mugen-unleashed, a C rebuild of MUGEN 1.1b1) with `compat/` (SDL 1.2 / SDL_mixer on
  the game's side, math, heap redirects), `host/` (mf_host.c: the engine thread and coroutine, sessions,
  frames, the 4:3 screens' sides, the state pages for desync recovery; mf_rollback.c; mf_mem.c: the heap
  region at a fixed address; mf_fit.c: the HUD fitting; mf_pack.c: the fight files' hashes) and the
  third-party code. API: include/mf_engine.h. Hooks in engine files are marked `Moemon Fight`.
- `src/platform/fight_platform.c`: the picture (bicubic), buttons and their bindings (saved by gamepad.c in
  moemon_controls.ini), the sound mix, the overlay (countdown, signal icon) and the CRT.
- `src/mp/mp_fight.c`: the cabinet (seats, bubbles, the specials in data/scripts/vs_cabinet.inc), the
  challenge (relay 25-31, protocol 57: the pack summary and the answering player's cabinet rules), the
  netcode (button packets over UDP or the relay, rollback, a hash every 30 frames, desync recovery), the
  results (wins / losses: `fightwin` / `fightloss` / `fightcpu` / `fightcpuloss` counters, achievements,
  the TRAINER CARD's VS FIGHTS line).
- Engine globals that are fight state go into one section (the Makefile renames the engine objects' data
  sections); anything allocated by the engine thread lands in the heap region. The BGM thread's files
  (`bgm*.c`) are `MF_SYSTEM_HEAP`: they use the C library's heap and aren't state.
- Desync recovery merges word by word: a word that is a pointer into this PC's own memory (outside the
  region) stays, so code and C-heap pointers survive; the globals only cross between the same build.
- Tests: tools/test/README.md (`fight_*`), and tools/fight/build_linux.sh's self test (`step` waits for a
  loading engine; `WIN=1` builds it for Wine, `SAN=1` with AddressSanitizer).

