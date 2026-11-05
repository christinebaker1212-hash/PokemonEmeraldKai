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
Low HP beep (battle_gfx_sfx_util.c PlayLowHpBeep): SE_LOW_HEALTH loops every 36 ticks; Task_StopLowHpBeep stops it once
its player's clock reaches 100 ticks (three "doo duh"s), not by task steps, so the solo battle turbo can't cut it short,
and MpBattleTurbo leaves SE3's tempo alone while it plays (MP_LowHpBeepPlaying): the same in singles, doubles, triples
and link battles. It plays again when a Moemon newly drops into the red. MOEMON_SE_TRACE=90 logs it (m4a.c TraceSong);
tests lowhp_beep, lowhp_pvp.
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
"gatcha:free10" per week (STANDARD only), +1 free pull per banner "gatcha:free1bN" per day played, stocked; only for
a later day / week than the last given ("gatcha:free1day", "gatcha:freewk": a clock set back and forth gave them on
every launch); each pull stirs gRngValue with the PC's timer (StirRandom: the RNG starts at 0 on every launch and steps
once a frame, so the same inputs on the same frames pulled the same prizes); test tools/test/gatcha_fair.sh; menu
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
sDailyLegends a 7-day table with no species twice a day); the tagline takes turns with "TODAY'S UR / R".
Banners by GENERATION (1.61.4; GatchaGen = the region's cap, never below 3, the room host's online): GEN 3 is the
MoeGatcha as it always was. Every pull, the reel and the dump go through DefAllowed / DefWeight (an item past the
generation, GatchaItemGen: MEGA STONES 6, Z-CRYSTALS 7; a Moemon past it), so the pools may list later things. Each later
generation adds one daily Moemon (R, shiny at SR) and one daily legend (UR, EX) from sGenDailyMons / sGenDailyLegends
(EV_SPECIES* kinds carry the species in qty: Lv 20, 5 for starters, 50 fateful for legends), two PICKUPs (its legends,
its starters: appended after the random one, so GEN 3's weeks and vote indices stay the same; the rotation and the vote
use the allowed ones, AllowedPickup), held / evolution items (sRareItems), MEGA STONES / Z-CRYSTALS (sSSuper), POWER
items (TRAINING, GEN 4), and random Moemon up to MP_Gen_DexCount with the later legends (sLaterLegendDex). The later
items in the old pools (EVIOLITE, QUICK POWDER, the region treats) now wait for their GENERATION. `mp gatchadump`;
tests gatcha_gen3, gatcha_gen9 (tools/test/check_gatcha_gen.py). PICKUP
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
Tournaments (mp_tourney.c, CO-OP > TOURNAMENT): the organizer first picks the kind (TS_CHOOSE: BATTLES or CARD DUELS,
TK_*; MPR_T_INVITE carries it, protocol 58), then the round robin as before. Card matches: Tcg_TourneyDuel (tcg_online.c)
opens both games' deck pick with the round's session (0x40000000 | tid | round | ids), the lower id invites; the result
comes back once through MP_Tourney_OnCardResult (a finished duel; leaving it: a loss for whoever left). The ready check
is the CARD CASE and the deck in use (Tcg_TourneyNotReady). Debug `mp tourney`; tests tcg_tourney, tourney_battles.
Level scaling (1.19.0, mp_levelscale.c; OPTIONS > BATTLE > LEVEL SCALING, gMpConfig.levelScaling / ini level_scaling,
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
Who sets them: playing alone, OPTIONS > BATTLE > LEVEL SCALING (A opens its page: GAME > LEVEL SCALING header, the six
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
(mp_grouplink.c): a 2-4 player link behind the NetLink_* calls (link.c) + matchmaking in TryContestEModeLinkup. Contests
at the room host's ANIM SPEED (1.101.0): contest.c MP_ContestTurbo gives sprites, tasks and fades an extra step on 5 (25%)
or 6 (30%) frames in 20, the results screen too (contest_util.c); the move list's input and contest_link.c's exchange
tasks run once a frame (IsContestLinkTaskFunc). CONCEDE: START on the move list asks; solo, the contest ends there in
last place; in a link contest the choice goes out as move 0xFFFF (MP_CONTEST_CONCEDE) in the move exchange, every game
gives that contestant noMoreTurns and puts it last in DetermineFinalStandings. Super Contests: START on the ACTING move
list (C.conceded: straight to the results, last place); their words, acting and results take the extra steps, the
dance and the dressing keep the music's time. Tests contest_fast, contest_concede, contest_concede_link,
sinnoh_contest_concede.
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
Revived after having nothing out (1.61.4): a player whose last Pokemon fainted is marked absent; a friend's REVIVE on
one of theirs brings them back at the end of the turn (HandleFaintedMonActions case 0). MP_IsPartyIdOnSide skips an
absent battler and one whose Pokemon fainted (gBattleMons hp 0), so the last one out counts as a replacement once
revived (it used to stay "on the field": no prompt, while the host's screen already drew it); the send-out menu
(party_menu.c TrySwitchInPokemon) doesn't call the fainted battler's own Pokemon "already in battle". Tests
tri_revive_self (one player's two Pokemon faint one after the other, the last one out revived, then the other),
tri_revive_two (two friends with nothing out, revived one a turn by the host), tri_revive_bench (the first one to
faint revived), coop_revive (the doubles case). Harness: MOEMON_DEBUG_FOE_MOVEID=MOVE puts a move in every foe's first
slot (the master's engine), MOEMON_DEBUG_FOE_TARGET=B[,B...] = each foe's battler to attack (the foe's own entry, else
the first still standing), `mp teach SLOT MOVE [MOVESLOT]`, `mp unitem ID`, `mp battlerhp B HP`.

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
graphics/moemon/pmd/pmd.bin (one raw-deflated chunk per Pokémon: national dex 1-493; 387-493 added later with
`--append 387-493` from a newer collab checkout, the rest kept byte for byte, variant 0 normal, 1 female, 2/3
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
OPTIONS > DISPLAY > FOLLOWERS (gMpConfig.followerSprites, ini follower_sprites, MP_PmdWanted): DEFAULT the original Moemon
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

## HGSS POKé BALLS (1.26: battle and summary balls)
The thrown / sent-out / summary ball sprites are the HEARTGOLD / SOULSILVER sheet (docs/user_images/35.png, ripped by
Sunday-Doneko, credited in dist_extras/README.txt; tools/moemon_balls/hgss_balls.py writes graphics/balls/<ball>.png).
Each ball: eleven 16x32 frames (include/pokeball.h BALL_FRAME_*): 0 closed (the sheet's first row, the round ball, as in
menus: docs/user_images/36.png; the sheet's squashed row 8 isn't used), 1 open (23 px tall, its base on the closed
ball's), 2 empty, 3-10 spinning (the sheet's rows 0-7). The ball's middle is the sprite's (8, 16), so every caller's
coordinates are as with the old 16x16 balls. OAM 16x32 affine double; anim tile offsets are BALL_FRAME(n) = n * 8;
BALL_ANIM_SPIN (7) loops the spin frames (2 frames each). StartBallSpin (pokeball.c) starts it for a throw
(battle_anim_throw.c SpriteCB_Ball_Throw), the player's send-out flight (instead of affine anim 4) and the BALL EFFECTS
editor's drop; landing goes back to frame 0. GENESIS / EPOCH sheets have their 16x16 art in every frame
(MP_BallStyle_SpriteHasSpin FALSE): those still turn with affine anim 4. Palettes: POKé BALL's reds are slots 1-3 (dark
to light) and it uses no slot past 8: COLOR ramps go into 1-3 (ramp shades 1-3), decorations take 9-15. Decorations:
the closed frame at (6, 11), and on each spinning frame where 40+ red pixels face the screen, at their middle, over red
pixels only (DecorateSpinFrames). Special balls with art in the sheet (LOVE, FRIEND, FAST, LEVEL, LURE, HEAVY, MOON,
HEAL, DUSK, QUICK) load their own sheet (tag TAG_MP_BALL_HGSS 0x57B0 + special) and palette (TAG_MP_BALL_SPECIAL +
special); DREAM is the POKé BALL in its ramp. Test hgss_balls (shots by hand: send-out spin, MASTER BALL throw, blue
POKé BALL with a STAR, GENESIS, the summary's ball).

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

## The ASTRAL BALL (a sure catch, and shiny)

The user's design (2026-11-04; art docs/user_images/69.png battle sheet, 70.png BAG icon): ITEM_ASTRAL_BALL (ITEM_058,
88), special ball 14 (MP_SPECIAL_ASTRAL, style `e000`; 14 was the last free special index: 15 is a color bit), MoeGatcha
BALL banner ULTRA x1 and EX x2 (beside the MASTER BALL). Art: `python3 tools/moemon_balls/astral.py` (the sheet in
the HGSS layout -> graphics/balls/astral.png like hgss_balls.py; the icon's 24x24 middle -> moemon_astral_ball.png/.pal).
- Catch: Cmd_handleballthrow sets odds 255 (caught on the throw, three shakes like the MASTER BALL). The ghost battle's
  dodge (gMpUncatchableWild) and trainers' blocks still apply.
- Shiny: pokemon.c MP_Astral_MakeShiny, from MP_Ball_SetCaught and again in GiveMonToPlayer after the OT re-key (a
  co-op catch is made with the host's ID). A new personality: low half searched from the old one, high half picked
  for shininess, keeping nature, gender, UNOWN's letter and WURMPLE's class (only SPINDA's spots move); the four
  substructures are re-sorted for the new personality and the checksum redone. Logs `astral: species ... ->`.
- Not a re-housing ball (MP_BallStyle_CanRehouse): the summary's POKé BALL page and the phone (export "catchonly")
  never offer it, since only a catch makes an ASTRAL Moemon shiny.
- Looks (src/mp/mp_astralfx.c, no sprite or palette used, the art untouched; the user's design, 2026-11-04):
  MP_AstralFx_Prism (sdl2.c, right after DrawFrame and the BAG / CANDY JAR pictures, before weather and tints)
  finds the ball's pixels by its palette's colors inside its box and tints them with a diagonal rainbow that slides
  and breathes, under it a slower shimmer of lighter and darker patches (two triangle waves meeting); outline and
  shine kept. MP_AstralFx_Composite draws silver four-pointed sparkles around it, a trail while it moves, darker
  steel-silver over bright backgrounds, fading with its palette.
  Only an ASTRAL BALL that hasn't caught anything shows them: item icons (item_icon.c MP_AstralFx_Icon; a tile
  checksum drops reused slots) and thrown balls (battle_anim_throw.c MP_AstralFx_Thrown, on every player's screen).
  At the click (SpriteCB_Ball_Capture_Step, MP_AstralFx_Caught) the sheen fades to the ball's own grey over
  FADE_FRAMES and the sparkles, plus a ring of twelve, fly into it; the summary's ball and send-outs are plain.
- Test astral_ball (four full-HP catches incl. RAYQUAZA, check.py: shiny, nature, gender, UNOWN letter, WURMPLE;
  `mp monid SLOT`; shots: flight, shakes, the click, the summary's plain ball, the BAG).

## Fishing as Gen 1 had it (1.97.0)

The user (2026-11-04) found Emerald's fishing minigame no fun: field_player_avatar.c's state machine now goes cast ->
the one-second wait -> Fishing_CheckForBite (the game's own odds: half the casts, SUCTION CUPS / STICKY HOLD more; the
map's fishing table) -> "Oh! A bite!" (first line) held FISHING_BITE_PAUSE frames (A skips) -> the battle, or "Not even
a nibble!". Fishing_ShowDots / CheckMoreDots / MonOnHook / GotAway stay in the table but are never reached (InitDots
jumps to FISHING_CHECK_BITE, WaitForA to FISHING_START_ENCOUNTER), so no reaction timing, no extra rounds for the
GOOD / SUPER RODS, nothing gets away. Log `fishing: a bite (rod N)` / `not even a nibble`; tests rig the roll with
`mp fishbite 1|2|0` (gMpFishBiteRig). SINNOH's TV fishing show and the LURE BALL's flag are fed as before. Test
fishing_gen1.

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

## Save files: the saves folder and three save files (1.61.5)
- src/platform/save_slots.c, include/save_slots.h. The save and everything named after it ("<save>.*": sidecars,
  .bakN backups, .summary) live in saves/slot1-3; saves/slot.txt is the one in use (gPlatformSavePath set before the
  save is read). A pokeemerald.sav beside the game (an older version's) and its "pokeemerald.sav.*" files move into
  saves/slot1 by rename, unchanged, when saves/slot1 has no save yet. --save PATH / MOEMON_SAVE (the tests) use that
  path and turn the slots off (the title menu is as before); dev builds read the slot's save, never move or write.
- Title screen (main_menu.c MpDrawSlotItem...): SAVE FILE is the last item (window 3 in the no-save layout, window 5
  in the saved one): LEFT / RIGHT the file shown, from its "<save>.summary" (name, region, badges, time; written by
  StoreSaveFile) or else the save's own SaveBlock2 (name, time); A on another file writes slot.txt and starts the game
  again (SaveSlots_Switch: CreateProcessW / execv; under MOEMON_TEST it logs and exits); SELECT asks twice in window 7
  and erases the file's "pokeemerald.sav*" files (SaveSlots_Erase; the file in use restarts empty). Harness
  condition "mainmenu". Test tools/test/save_slots.sh (a scratch game folder: the move, the switch, the erase).

## KANTO's rival battles, regional music, the ARCEUS trials (1.61.6)
- Quetzal's trainerbattle 13 (qz_trainerbattle_13: BLUE in OAK's lab and on ROUTE 22) is FireRed's
  TRAINER_BATTLE_EARLY_RIVAL: trainer, flags (1 heal after), the text when the player wins, the text when the rival
  wins. battle_setup.c sQzEarlyRivalBattleParams / EventScript_QzEarlyRivalBattle: no intro, the script goes on
  after the battle; a loss shows the rival's text (battle_main.c -> BattleScript_LocalBattleLostPrintTrainersWinText,
  battle_message.c B_TXT_TRAINER1_WIN_TEXT) and doesn't white out (CB2_EndTrainerBattle). Tests kanto_rival_win /
  kanto_rival_lose.
- Music: pokemon.c GetBattleBGM in JOHTO / KANTO -> MpRegionBattleBGM (MUS_RG_VS_WILD / TRAINER / GYM_LEADER for
  leaders and the ELITE FOUR / CHAMPION); Quetzal trainers' FireRed encounter music ids (0-13) play FireRed's
  encounter themes (PlayTrainerEncounterMusic), 14-23 JOHTO's. BattleSetup_StartLegendaryBattle's default was
  GROUDON's battle: now MUS_RG_VS_LEGEND (MEWTWO MUS_RG_VS_MEWTWO, LUGIA / HO-OH JOHTO's own in JOHTO). Tests
  battle_music_hoenn / _johto.
- qz_specials.c ArceusTrialWaits: KANTO's 17 ARCEUS trial objects stay hidden until FLAG_QZ_0x2E13 (ARCEUS's visit
  to the PALLET bedroom), ALTERING CAVE's Normal-type one (0x2E15) until the 17 are done. Test arceus_trials.
- Legendaries wait for the LEAGUE: RefreshStatics shows a region's statics only once MP_Region_Cleared(region), save
  PreLeagueLegend's (what the region's own games give before the LEAGUE: JOHTO's RAIKOU / ENTEI / SUICUNE, KANTO's
  ARTICUNO / ZAPDOS / MOLTRES, not their GALARIAN forms). The dreams (DreamAllowed) and the CHANSEY Dance wait too.
  Test legend_gate; the event tests run `mp regionclear` first. Wild battles ignore MOEMON_TEST_FOE_HP1 (trainers'
  parties only): the dream tests set DARKRAI's HP with `mp battlerhp 1 1`.
- Achievements: jalph (FL_ALPH 0x1076-0x1079), jbugwin (qz_bugcontest.c, first place), jmeloetta / jdarkrai /
  kmeloetta / kdarkrai (the statics' done flags), karceus (FL_ARCEUS_TRIALS: the 17 done flags), karceusn (0x2E16).

## Battle intro diagnostics (1.61.5)
- src/mp/mp_battlediag.c, on with AllowSmallTweaks = 1 (or MOEMON_BATTLEDIAG=1). battle_main.c BattleMainCB1 names
  the intro step (MpIntroStage: GET MONS DATA ... EVENTS BEFORE FIRST TURN, NULL at the action menu); sound.c
  PlayCryInternal and pokemon_animation.c's front / back animation launchers report in. The log ("[diag]") gets the
  battlers (species, level, HP, cry number and whether it holds a sound, sprite), each step with its frame, each cry
  and animation, and at the action menu each battler's expected (shown, cry, animation) against what happened
  ("<- out of order"). A step that doesn't move on for 600 frames (1200 linked; not while the text box prints or
  waits for a button) writes "INTRO STUCK: <step>" with the foe, shown or not, cried or not in the text box.
  MOEMON_BATTLEDIAG_STUCK=frames for tests; scenario battle_diag.

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
pocket (ITEMS, MEDICINE and BATTLE ITEMS share one order). FAVORITES has its own order (1.63.14): sort "pocket"
POCKETS_COUNT (bag:sort5, MANUAL unless chosen) and each favorite's place (bag:fr<item>, given when marked, bag:frnext);
mp_bag.c SortFavorites orders the view, MP_Bag_FavoriteMove (SELECT) renumbers them as they show (`mp bagfav ITEM [0]`;
test bag_fav_sort). Every pocket's item menu has FAVORITE /
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
MP_Spectate_PanelSync (sdl2.c VDraw, and ScreenStep for the card screens, a VS fight or a battle we watch: VDraw
doesn't run then; the panel froze through card duels, test spec_tcg) hands the cards to Display_SetPanel.
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
Joinable (1.43.0): solo wild (savedCallback = CB2_EndWildBattle: not scripted ones like KECLEON), one trainer or two
who spotted the player at once (CB2_EndTrainerBattle, every trainerbattle mode 0-8), or a co-op battle of that kind:
any trainer but the Elite Four and the Champion (MP_Coop_JoinTrainerOk: gym leaders, rivals, admins, bosses, story
grunts too). The master's own script goes on after the battle as it would have alone; a joiner who beat a gym leader
gets the badge (GymLeaderEndScript: the leader's "you won" script, from the gym's map when the joiner was elsewhere,
checked before the rematch branch: a leader's first battle is in the rematch table too); route trainers in a
continue-script mode (MATCH CALL sign-ups) count as beaten for the joiner (OrdinaryTrainer); other story trainers keep
their own events. A story trainer has no trainer-type object, so MPR_COOP_BATTLING goes out with localId 0 (JOIN works,
nothing spots). Not: triples (full), PvP, frontier, safari, legendaries.
Two trainers (589afed3, reapplied in 1.43.0): a third player makes a Triple Battle against both (MpJoinPlan
trainerIdB, the snapshot's second lose text). TEAM PADDING on: a friend who asks to team up but can't have a seat when
the battle starts (two trainers, or a moment too late) gets DECLINE_JOIN_AFTER instead of a no, and their game asks to
JOIN by itself once the battle reports itself joinable (sCoop.joinAfter, UpdateJoinAfter; 30 s at most). A friend a
held trainer brought in waits up to a minute while the leader answers "wait for another?" (Task_JoinHeld).
Tests: join_gym (ROXANNE, the badge on both games), coop_two_join3, coop_two_solo_join, coop_two_wait3, coop_held_join3.
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
the second request never completes, so it's skipped in co-op. Joining in progress (1.26, protocol 51): a third player
makes it a Triple Battle against both (a solo battle against two trainers becomes a co-op double against both). The
conversion keeps BATTLE_TYPE_TWO_OPPONENTS and the enemy party where it is (slots 0-2 trainer A, 3-5 trainer B: a
slot says whose a Pokémon is); the new foe spot (battler 5) takes the next Pokémon of the trainer with more left
(JoinTwoTrainersNextFoe), and the triple's foe pool is shared from then on (MP_TripleSwitchRange). MpJoinPlan
trainerIdB and the snapshot's loseTextB carry the second trainer to the joiner (sCoop.twoTrainers / trainerIdB via
JoinSetSeats, gTrainerBattleOpponent_B in MP_Join_ApplySnapshot). "B sent out X" picks the trainer by the Pokémon's
slot in a triple (MP_Coop_SecondTrainersMon, from B_BUFF1's party index so every game agrees), by side in a double.
A joiner who came from a trainer script flags both trainers. MPR_COOP_BATTLING bytes 8-10: the second trainer's
object and id (MP_TrainerBattleLocalIds: both objects from gApproachingTrainers); TryOfferBattleInProgress matches
the trainer walking up against either, and two walking up against both. Test coop_two_join3 (mp joinask N: JOIN's
request from a script).
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

## Gen 4 battles (src/battle_gen4.c, include/battle_gen4.h, data/battle_scripts_gen4.inc)
The engine stays Gen 3's; battle_gen4.c adds what Gen 4 changed. State: gGen4Battlers (EMBARGO, HEAL BLOCK, MAGNET RISE,
SLOW START, AQUA RING, ROOST, choice lock, METRONOME count, LAST RESORT slots, arrival / end-turn steps), gGen4Sides
(TAILWIND, LUCKY CHANT, TOXIC SPIKES, STEALTH ROCK, HEALING WISH / LUNAR DANCE) and gGen4Field (GRAVITY, TRICK ROOM,
COPYCAT's last move, the move-end bits). All reset at battle start (MP_Gen4_BattleStart); a switch-in resets the battler
(BATON PASS keeps what Gen 4 passes).
- Physical / special: OPTIONS > BATTLE > MOVE KIND (GEN 3 by type, GEN 4 by move: gMoveCategories); online the host's
  (MPR_RULES byte 4, protocol 49). MP_IsPhysicalMove decides; Counter / Mirror Coat / Hustle follow it.
- Moves 355-467 have effects 214-266. struct BattleMove.effect is a u16 since then (a u8 wrapped 256+ around:
  SHADOW FORCE ran "ATTACK UP"). The scripts use one command, `gen4 MODE, ptr` (0xFE, constants/battle_gen4.h G4_*):
  each mode goes on, jumps to ptr (usually "but it failed"), or moves the script itself (ME FIRST, COPYCAT, POWER HERB).
  MOVE_EFFECT 55 / 56 = CLOSE COMBAT's DEF + SP. DEF drop and HEAD SMASH's 1/2 recoil. SHADOW FORCE vanishes with
  STATUS3_PHANTOM (part of STATUS3_SEMI_INVULNERABLE: nothing hits it) and lifts PROTECT on its second turn.
- Damage: MP_Gen4_MovePower (GYRO BALL, BRINE, PAYBACK, PUNISHMENT...), MP_Gen4_AttackStats, MP_Gen4_FinalDamage
  (LIFE ORB, EXPERT BELT, FILTER, TINTED LENS...), MP_Gen4_Stab (ADAPTABILITY), MP_Gen4_MoveType (JUDGMENT, NORMALIZE,
  NATURAL GIFT), MP_Gen4_TargetAbility (MOLD BREAKER), crit stages and SNIPER, accuracy (NO GUARD, GRAVITY, ZOOM LENS...),
  MP_Gen4_OrderKey (TRICK ROOM, TAILWIND, CHOICE SCARF, UNBURDEN, QUICK FEET, STALL, LAGGING TAIL, SLOW START).
- Arrival (Cmd_switchineffects, before SPIKES, one effect per pass): HEALING WISH / LUNAR DANCE (the status and PP go to
  the games by G4_ARRIVE_SYNC, one controller message at a time), STEALTH ROCK (1/8 x ROCK effectiveness), TOXIC SPIKES
  (grounded; a POISON type takes them away). Arrival abilities (ABILITYEFFECT_ON_SWITCHIN -> MP_Gen4_SwitchInAbility):
  DOWNLOAD, ANTICIPATION, FOREWARN, FRISK, SNOW WARNING (B_WEATHER_HAIL_PERMANENT), MOLD BREAKER, SLOW START.
- Before a move: GRAVITY and HEAL BLOCK stop the moves they ban (MP_Gen4_MoveBanned in HandleAction_UseMove).
  POWER HERB: BattleScriptFirstChargingTurn ends with G4_CHARGE_HERB; the move's script runs again and takes its
  second-turn path (STATUS2_MULTIPLETURNS is already set).
- Move end (MOVEEND_KINGSROCK_SHELLBELL -> MP_Gen4_MoveEnd, once a move each): FOCUS SASH used up, ANGER POINT,
  AFTERMATH, LIFE ORB recoil. FOCUS SASH holds in the four FOCUS BAND spots (MP_Gen4_FocusSash).
- End of turn: ENDTURN_GEN4 per battler (AQUA RING, POISON HEAL, DRY SKIN / SOLAR POWER / ICE BODY, HYDRATION, BLACK
  SLUDGE, BAD DREAMS, TOXIC / FLAME ORB, STICKY BARB, the EMBARGO / HEAL BLOCK / MAGNET RISE / SLOW START counters) and
  ENDTURN_GEN4_FIELD (GRAVITY, TRICK ROOM, TAILWIND, LUCKY CHANT). Messages come from gGen4TurnStringIds via
  cMULTISTRING_CHOOSER. MAGIC GUARD skips poison, burn, LEECH SEED, NIGHTMARE, CURSE, recoil and the Gen 4 hurts.
- Also: SIMPLE (ChangeStatBuffs), SKILL LINK, BIG ROOT, LIGHT CLAY, the weather rocks, CHOICE SCARF / SPECS lock
  (MP_IS_CHOICE_ITEM where CHOICE BAND was), UNBURDEN (Cmd_removeitem), MOTOR DRIVE and DRY SKIN's water absorb.
- Strings: STRINGID_G4_* from 420 (tools/moemon_gen4/gen_battle_strings.py).
- Gen 4 BERRIES (items 474-493, berry numbers 59-78 after the e-Reader ones: IS_GEN4_BERRY, FIRST_GEN4_BERRY_TYPE in
  ITEM_TO_BERRY / BERRY_TO_ITEM; tools/moemon_gen4/gen_berries.py writes src/data/berries_gen4.h, whose macros the
  berry-number tables include). Kai's own CHILAN BERRY (e-Reader, 381) only gained the NORMAL resist. Battle: the
  type-resist BERRIES halve the hit (MP_Gen4_FinalDamage) and are eaten when the move ends with "The X weakened the
  damage", JABOCA / ROWAP hurt a physical / special attacker by 1/8, MICLE gives the next move x1.2 accuracy,
  CUSTAP moves first at 1/4 HP (MP_Gen4_OrderKey, eaten in MP_Gen4_BeforeMove). Trees: a vanilla tree of the berry's
  color. They have no place to be found yet (debug `mp additem`); the other regions will sell or grow them.
- Followers: the default (HGSS-style) sprites of 387-493 come from Moemon Quetzal's overworld sprites
  (tools/moemon_gen4/gen_followers.py QUETZAL_ROM: appended to followers.bin, "sheet": "quetzal" in index.json);
  the PMD ones from the PMD Collab (tools/moemon_pmd/import.py --append 387-493).
- Animations: 47 Gen 4 moves have expansion's (tools/moemon_gen4/gen_anims.py EXPANSION_DIR: only scripts whose
  templates, tasks, tags, sounds and commands Kai has; their shared routines copied with a G4_ prefix; a
  loadspritegfx added for each template's tag, since expansion loads them by itself; invert_screen_color's mask
  turned into Kai's three flags). SHADOW FORCE's is hand-written. The other 65 still borrow a Gen 3 animation.
- STEADFAST (CANCELER_FLINCH -> BattleScript_G4Steadfast), LEAF GUARD (SetMoveEffect: no status in sunshine).
- Not yet: STORM DRAIN's redirect (doubles), STICKY BARB passing on contact, and animations of their own for the 65
  moves that borrow.
Debug `mp g4side SIDE ROCK TSPIKES WISH` lays hazards (0 = the player's side), `mp setmove SLOT IDX MOVE`, `mp hold SLOT ITEM`,
`mp foehold ITEM` (in a battle: the first foe holds ITEM).
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
  from the FLOORS grids on Building + Shop ('.' floor 0x001, '#' blocked, 'W' exit mat 0x006, 'S' stairs 0x250 (a
  non-animated door: arriving, Task_ExitNonAnimDoor steps the player off the way MP_MapPic_ArrivalDir says, sideways
  for the stairs and left out of the rooftop door, not down through the rail), 'C' counter 0x232).
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
- MOENAV map: "GAME CENTER" is a landmark of RUSTBORO's two cells (landmark.c Landmarks_RustboroCity), printed under
  the city's name when the cursor is on it as the routes' landmarks are; the city map moves down by its lines
  (pokenav_region_map.c sCityMapRowsDown). No icon on any region map.
- Pictures and the scroll: MapPicLine's scroll offset against the BG tile offset is signed (a step moves the tile
  offset a whole tile at once; wrapping at 256 drew the picture's far side down the screen's left edge for a step).
- Tests gc_look (pictures, clerks behind counters, the Town Map), gc_visit (gate, admission, stairs, shop deck and
  single, a duel asked by talking, elevator, rooftop, exit). Debug: mp face DIR, mp textspeed N.
- Achievements for JOHTO and KANTO (1.61.3, mp_achieve.c): their own tabs (C_JOHTO, C_KANTO) with the first
  POKéMON, each badge, all eight, the CHAMPION (AK_CLEARED: MP_Region_Cleared), the legendary beasts / birds, LUGIA,
  HO-OH, CELEBI, MEWTWO and MEW (the static encounters' "done" flags: caught or beaten), and in JOURNEY "WORLD
  TRAVELER" (AK_STARTED: all three regions visited) and "TRIPLE CROWN" (CHAMPION of all three). Quetzal's flags are
  numbers here (0x11B2...): they live in the region side file and FlagGet reads them in any region. Test ach_regions.
- Every card in every region (1.61.3), by HOENN's rules (the user: don't change how a card is had): POKeMON promo
  cards come from beating special trainers, so JOHTO and KANTO have theirs (tcg_field.c sQzSpecials): each stands in
  for one of HOENN's slots - its deck, its promo, its "beaten" / "promo given" bits (a promo is given once, in
  whichever region it's won first) - with its own name, lines, record key (0xFE000000 | row) and requirement (its
  region's badge flag 0x11B2+ / 0x2100+, an ELITE FOUR win, MP_Region_Cleared for LANCE). Paired by role first, so
  HOENN's rules read the same (a region's eight leaders fill the eight leader slots: "beat all eight GYM LEADERS" means
  its own; ELITE FOUR for ELITE FOUR, CHAMPION, rival), then by type within the role (FALKNER - WINONA's birds,
  JASMINE - WATTSON's MAGNEMITE, PRYCE - JUAN's water, KOGA - SIDNEY's GENGAR...). KANTO's BLUE is rival and CHAMPION
  in one sprite: the rival's slot (WALLACE's has no promo). RED / PROF. OAK stand in for STEVEN, PROF. ELM / BILL for
  WALLY. Tcg_LookGfx leaves these sprites as they are, so Yu-Gi-Oh! and Triple Triad don't take them for look-alike
  townsfolk. The GAME CENTER sells no promo and no 4- or 5-star Triple Triad card. Test cards_region_specials (the
  log's "challenge: NAME, special N, promo P"); gc_cards_everywhere; the card guide lists them (gen_guide.py reads
  sQzSpecials). Yu-Gi-Oh! needed nothing (every card is in a booster, a shop or a deck).
- JOHTO's and KANTO's GAME CENTER (1.61.3): HOENN's 2F and 3F, no 1F and no ROOFTOP (the user's rule: HOENN's tower
  keeps its four floors; a region without a tower of its own gets an attendant and 2F / 3F only). The attendants
  stand in GOLDENROD's and CELADON's GAME CORNERS at (12, 2) (map.json objects added after the import: put them back
  if import.py runs again; CELADON's stands clear of the row along the wall to the basement stairs at (15, 2)) with 2F's elevator doors pinned on the wall behind them (passage.png, cut from 2f.png by
  gen_gamecenter.py PASSAGE; sPics). EventScript_GameCenterGuide sets VAR_GAME_CENTER_PAID and warps to 2F's stairs.
  The floors are shared maps (mp_region.c MP_Region_IsSharedMap: with the MOéMON GARDEN): MP_Region_Current there is
  the region the player came from (no trip, no PC swap), MP_Region_NoteWarp (overworld.c ApplyCurrentWarp) keeps
  where they stepped in (RegionState.origin*, in the region sidecar), and MP_Region_SharedExit (SetupWarp) sends a
  warp out of a shared map, or 2F's stairs down to 1F, back there. Outside HOENN the elevator asks only for the other
  floor (MP_Region_GameCenterOtherFloor) and 3F's stairs up say the ROOFTOP is closed (MP_Region_WarpClosed in
  TryStartWarpEventScript). Tests gc_johto, gc_kanto, garden_johto.

## The GAME CENTER's arcade cabinets (1.62.0)

The five back-to-back pairs in the middle of the GAME CENTER 3F (rows 9-13 since 1.63.13, 8-12 before, bg events at x 3 facing east = the west
seat, 1P, and x 4 facing west = the east seat, 2P) are VS cabinets, one game each (data/scripts/vs_cabinet.inc,
mp_fight.c CABINET_*): 9 SUPER STREET FIGHTER II X, 10 VAMPIRE SAVIOR, 11 CYBERBOTS, 12 the MUGEN cabinet ("A weird
bootleg", moved here from MAUVILLE's GAME CORNER, whose (7,9)/(8,9) are slot machines again), 13 STREET FIGHTER ALPHA
3. Each cabinet names its game in a message first. The two Gamer NPCs stand beside the row, out of the chairs.

- The CAPCOM games are CPS-2 boards: arcade/cps2recomp (tmcleroy's cps2recomp 1.2.0, GPL-3.0: machine, video, QSound
  and Z80 sound, runtime; the 68000 always interpreted, nothing of the games translated or kept) through the C bridge
  arcade/arcade_cps2.cpp / include/arcade.h (Arcade_*). Built by the Makefile's ARCADE_* rule (C++20, -iquote so the
  game's include/strings.h doesn't shadow the system one). Credits: dist_extras/ARCADE_CREDITS.txt.
- **ROM sets are the player's**: never in the repository or the downloads (the user declined shipping them through the
  updater; nothing can). The bridge reads `<set>.zip` with its parent's zip beside it from the "arcade" folder next to
  the exe (Updater_ExeFile("arcade"); tests: MOEMON_ARCADE_DIR), files found by CRC (kGames: MAME's names and CRCs:
  vsavj + vsav, cybots, ssf2xj + ssf2t (graphics: three banks of four, 2 MB then 1 MB files), sfa3). Arcade_Check says
  why a set doesn't load; Arcade_HasSet is the same remembered until the zips' sizes / times change. The release ships
  an empty arcade folder with a note. The machine keeps a reference to its RomSet: the bridge owns it as long as the
  machine (a stack RomSet crashed on the first frame). Picture() is 0x00BBGGRR.
- include/fight_engine.h: mp_fight.c and fight_platform.c call FightEngine_*, which answer from the MUGEN engine or,
  for gFightKind >= FIGHT_KIND_ARCADE, from src/platform/arcade_fight.c (ArcFight_*: the board with the MUGEN engine's
  calls). So the CRT on / off, the 480x320 picture through display.c, the sound in place of the game's, spectating,
  1P play, online VS with rollback (a save state kept per frame once the 300-frame boot is over: until then frames run
  on known buttons only, so the input delay settles) and desync recovery (the save state as the "globals" region in 4
  KB pages, the cabinet's START handling in MfStateInfo.host) are all the MUGEN cabinet's code.
- The picture is shown 4:3 (384x224 to 427x320, bilinear, black sides). The board's 24 kHz sound is stretched over
  each game frame's samples (frame-locked: no drift). Buttons: X Y Z = LP MP HP, A B C = LK MK HK. (1.64.0, the user: on every
  arcade board) SELECT (fight button COIN, MF_BTN_COIN: Backslash / Back) puts in a coin from the COIN CASE (mp_fight.c
  LocalButtons takes it once a press, the bit on that frame only; NO COINS on screen when there are none; the board
  sees ARCADE_COIN 5 frames, arcade_fight.c); START is the board's START while held; held 120 frames leaves
  (MF_RESULT_CANCELLED, no result message); the arcade pack core is 0xA2C1DE00 + game since (older builds' differ); online the leave counts 12 frames later, past any guessed buttons (it rolls back with the state).
- Seats and online: the protocol's cabinet byte is the CABINET_* id (0 was MAUVILLE's), so two cabinets never mix. The
  pack check: a board's MfPack core is 0xA2C0DE00 + game when the set loads, 0 otherwise, so a friend without it gets
  "NAME doesn't have this game's ROM set" (FIGHT_RESULT_PACKS).
- SPECTATE (mp_spectate.c): frames carry the arcade game on the sender's screen + 1 in FRAMEF bits 3-5
  (MP_Fight_ArcadeShown). A viewer without that set (Arcade_HasSet) doesn't apply them: the card shows CONNECTING...
  until the game is off that screen, then asks for a whole picture (the user's rule).
- Input lag (1.62.3, the user: "moves come out late", controller, alone and online): the board answers on the next
  frame (`mp arcadelag GAME AT BUTTONS`), costs 2-3 ms a frame and replays a loaded state exactly (`mp arcadedet GAME
  AT`), so the lag was the PC's: with vsync a Direct3D driver queues up to ~3 frames. display.c Display_Present reads
  one pixel after presenting while Fight_LowLatency() (a fight is the picture; accelerated renderer, vsync on): the
  "hard GPU sync" emulators use. Online, mp_fight.c's input delay ignored the first 120 frames' round trips (the
  board's ROM load looked like a 9-frame ping and locked the delay at 3). Game over / a VS win don't end the
  session: holding START does (the user prefers it).
- Tests: `mp arcade GAME FRAMES` (harness: opens a board, runs it, prints sound, lit pixels and a RAM hash, then its
  frame / save / load times); the fight_*
  scenarios now sit at row 12 (the MUGEN cabinet). The CAPCOM games can only be tested with the sets (scratch only).

## The Astro City cabinets: PUZZLE BOBBLE and TETRIS THE GRAND MASTER 2 PLUS (1.64.0)
- The 3F's top right wall (bg events (10,5), (11,5), (12,5) facing north): (10,5) CABINET_PBOBBLE (6), (12,5)
  CABINET_TGM2 (7); the middle one keeps its sign (the ARCADE FAN stands in front of it). One seat each:
  data/scripts/vs_cabinet.inc EventScript_AstroCabinet (the game's name, then "Play?": the CPU path, MP_Fight_StartCpu;
  a friend sitting there is Taken / InUse as at the VS cabinets).
- The bridge (arcade/arcade_cps2.cpp) has a Board interface: Cps2Board, TaitoBBoard, Ps5Board; Arcade_PictureSize gives
  each picture (384x224, 320x224, 320x240) and arcade_fight.c scales it to the 427x320 4:3 (tables rebuilt per size).
  Non-CPS-2 sets are lists of RomFiles by CRC read with cps2recomp's ReadZipFiles (romset.cpp, also a list-only check).
- arcade/taitob: PUZZLE BOBBLE (MAME "pbobble", the Japanese set): cps2recomp's 68000 (12 MHz) and Z80 (4 MHz, an NMI
  input added for the TC0140SYT, saved only on boards that set hasNmi), the TC0180VCU (bg / fg 16x16 tilemaps with
  scroll blocks, 8x8 text, sprites drawn into a frame buffer at vblank, big / zoomed sprites), TC0640FIO inputs, a
  93C46 (cps2recomp's Eeprom), the YM2610B (ymfm, OPN_FIDELITY_MIN: 55.5 kHz; timers counted in YM clocks). 256 lines
  a frame; IRQ 3 at vblank, 5 eight lines on (held until acknowledged). ~2 ms a frame.
- arcade/psikyosh: TGM2 PLUS (tgm2p's program and EEPROM, the rest from tgm2): sh2.cpp an SH-2 interpreter (the
  SH7604's divide unit; on-chip RAM; autovectored IRL), ps5.cpp the PS5V2 (the PS6406B's 4 background layers with
  per-line effects, zoomed sprites with alpha and priorities, the line fill / blend layers; the YMF278B from ymfm at
  37.3 kHz, x4; a 93C56 x8 from the set's .nv). 262 lines at 61.68 Hz; IRL 4 at vblank (cleared by irqctrl), 12 the
  YMF278B. ~4 ms a frame.
- arcade/ymfm: ymfm vendored (BSD-3), credited in dist_extras/ARCADE_CREDITS.txt with the boards.
- Controls on every arcade board (the user): SELECT (fight button COIN) puts in a coin from the COIN CASE, START is
  START, holding START leaves (see "The GAME CENTER's arcade cabinets").
- Tests: gc_astro_noset (no set: the cabinet names pbobble.zip); with the sets (scratch only, never committed):
  `mp arcade 4|5 FRAMES FILE.bmp` and the cabinets played with `fpress COIN` / `fpress START`.

## The VS cabinet: MUGEN fights (1.31.0, 1.33.0)
Decisions and the as-built record: docs/FIGHT_DECISIONS.md. Where things are:
- `fight/`: the engine (mugen-unleashed, a C rebuild of MUGEN 1.1b1) with `compat/` (SDL 1.2 / SDL_mixer on
  the game's side, math, heap redirects), `host/` (mf_host.c: the engine thread and coroutine, sessions,
  frames, the 4:3 screens' sides, the state pages for desync recovery; mf_rollback.c; mf_mem.c: the heap
  region at a fixed address; mf_fit.c: the HUD fitting; mf_pack.c: the fight files' hashes) and the
  third-party code. API: include/mf_engine.h. Hooks in engine files are marked `Moemon Fight`.
- `src/platform/fight_platform.c`: the picture (shown by display.c as the game picture at 480x320: the game's
  scaling modes and filters; 1.50.0), buttons and their bindings (saved by gamepad.c in
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
- Sound (1.50.0): the shipped sounds are GBA sound (8-bit mono 13379 Hz clips, 13379 Hz Ogg songs) by
  tools/fight/gba_audio.py; a stage's song is kept 3 dB under the fighters' voices by the mixer
  (MfMix_MusicGain, levels from mf_bgm.c MfBgm_Measure and matchload.c Ml_VoiceLevel): FIGHT_DECISIONS
  "GBA sound, and music under the voices".
- Pixels (1.50.0): scaled, rotated and parallax sprites are area-resampled (fight/compat/mf_softscale.c, hooked
  in softrender.c / softblend.c / bg.c; MOEMON_FIGHT_NEAREST=1 for MUGEN's nearest), and the picture goes
  through display.c's own path (srcW x srcH 480x320): FIGHT_DECISIONS "Pixels". Tests fight_display,
  tools/fight/test_softscale.c.
- Character audit and balance audit (1.51.0): tools/fight/check_chars.py (static), tools/fight/audit_chars.sh (every
  pairing CPU against CPU plus a rollback check, via mf_selftest's `audit`; hooks MOEMON_FIGHT_PICK, MOEMON_FIGHT_WATCH,
  MOEMON_FIGHT_WARN), tools/fight/balance_audit.py (docs/FIGHT_BALANCE.md; baseline tools/fight/balance_baseline.json),
  all reading the pack through tools/fight/mugen_files.py. Patterns found: docs/FIGHT_AUDIT.md.
- Asset ingestion (1.52.0): tools/fight/ingest.py (Makefile fight-ingest runs it when mugen/ changed; record
  mugen/processed_assets.json, log mugen/ingestion_log.txt, index processed_assets.idx for the game's start check
  Fight_CheckIngestion); FIGHT_DECISIONS "Asset ingestion"; test tools/fight/test_ingest.sh.
- `MOEMON_FIGHT_SEED=N` (tests) fixes an online fight's seed, so its picks are the same every run
  (fight_online_lag pins one: with seed 1998331917, a Ryu mirror, the round can run past the test's wait, 1.49.3 too).

## Yu-Gi-Oh! card duels (src/ygo/, include/ygo/)

The CARD CASE's second side. Decisions (the user's 30 answers and the choices made) are in docs/YGO_DECISIONS.md,
the work left in docs/YGO_HANDOFF.md.

- Data: tools/ygo/import_sdd.py reads Stairway to the Destined Duel's card list, Forbidden/Limited list and pictures
  from the player's ROM (never committed) into tools/ygo/sdd.json and graphics/ygo/sdd_art.bin; build_catalog.py +
  gen_catalog.py add the English sets to the end of 2003 and write src/ygo/ygo_catalog.c, include/ygo/card_ids.h and
  src/ygo/ygo_fusions.c. fetch_art.py + build_printed.py: graphics/ygo/printed.bin (160x233 JPEGs, then the real
  back; `--keep-cards` rebuilds only the back).
- Engine: EDOPro's since 1.48.0 (see "Yu-Gi-Oh! on EDOPro" below; the game's own engine, ygo_duel.c and the
  ygo_fx_*.c effects, is in the history before that). One YgoQuery at a time, answered with a YgoAnswer; the whole duel
  follows from the seed and the answers (YgoDuel_Hash), so online only has to exchange answers.
- The table (ygo_board.c): plays the engine's events one by one, then asks. A chance to chain while a phase changes
  (YgoDuel_QuietChance: no chain, summon, attack or Damage Step) is passed for the player unless L is held; effects
  made for such a moment carry YF_PHASE and are still asked. This is the screen's choice, never the engine's.
- Screens (ygo_ui.c) share the Pokemon case's look (src/tcg/tcg_ui_kit.h); tcg_ui.c hands Step / Draw / IsActive to
  them while the YU-GI-OH! side is open. CARD ART and CARD BACKS (gMpConfig) apply to both games.
- World (ygo_field.c): who plays which game by trainer class / townsfolk sprite; three in five bring a deck from the
  GBA games' opponents by badge tier (GameDeck), the rest themed decks; special duelists with signature cards; tcg_field.c's Task_CardChallenge asks "MOéMON CARDS or YU-GI-OH!" for the CARD CLUB,
  the rival, WALLY and STEVEN. Both CARD CLUB NPCs stand in OLDALE's and MAUVILLE's POKéMON MARTS and LILYCOVE's
  DEPARTMENT STORE (data/scripts/ygo_club.inc, card_club.inc).
- Friends (1.36.0): src/ygo/ygo_net.c (include/ygo/net.h, struct YgoNet: the chain link's struct is YgoLink) on the
  card games' relay extension 255 (TCG_LINK_VERSION 6, message types 8-13; tcg_integration.c routes them). The
  invite / accept carry the decks (main + Fusion), then only answers go over, each with the duel's hash before and
  after (YgoNet_Hash: YgoDuel_Hash plus the answers inside the current step) and an ACK before the next; a
  concession crossing an answer rolls the answer back (as tcg_link.c). Who goes first comes from the session. The
  asking is tcg_online.c's with request kinds TCG_ASK_YGO_DUEL / TRADE (2-3); CO-OP > FRIENDS' CARD DUEL / TRADE pick
  the game with LEFT / RIGHT (mp_linkx.c), the CARD CASE button uses the case's last side. ygo_ui.c: Y_WAIT,
  Y_TRADE, the board at seat 1 for the friend who accepted, FriendRewards. Tests tools/ygo/test_net.c (two engines
  through a queue), scenarios ygo_online, ygo_online_nodeck, ygo_trade.
- No Yu-Gi-Oh! deck (ygo_ui.c ResolveNoDeck): a challenge, a friend's duel or trade opened for a player without a
  deck goes to the Starter Deck screen first: no Yu-Gi-Oh! cards = a Starter Deck to keep (RESOLVE_GIFT), cards but
  no deck ready = one to borrow for this duel (RESOLVE_LOAN, Y.loanMain; online onlineDeck -1). A CARD CASE is all
  a friend needs to be asked (tcg_online.c HasCaseFor).
- GAME CENTER 2F clerk (1.36.0): the counter with Y.gc set: SINGLES (GC_BINDER, Ygo_MoneyPrice, one copy each in
  gcSingles) and DECKS (the four Starter Decks, gcDecks) before TRADE IN and CARD SHOP; .ygo format 2.
- The CARD CLUB member stands behind the POKéMON MART counter (1,2) beside the shopkeeper (the Mart layout has room
  for one); in LILYCOVE's DEPARTMENT STORE both card NPCs are behind the counter (7,2) and (9,2).
- Save: "<save>.ygo" beside the save (Ygo collection, written with the Pokemon cards' sidecar, backed up with the
  save). CARD POINTS are shared with the Pokemon cards.
- Tests: tools/ygo/test.sh (EDOPro bridge, card scenarios, random duels, collection, deck codes, the link;
  sanitizers) and
  tools/ygo/test_ui.sh (the club, a challenge, the case's screens, a practice duel mashed with random keys).
  Debug: `mp ygo ...` (ygo_ui.c Ygo_Debug).

## Deck menus, recipes, ACE CARD, zones (1.38.0)
- Both card games' DECKS: A on a slot opens TcgUi_PopMenu (EDIT / RENAME / ACE CARD / DUEL WITH IT / COPY / DELETE;
  an empty slot BUILD A DECK / FROM A RECIPE); START / SELECT / R keep their shortcuts. Rows: the 8 slots, DECK
  RECIPES, BACK (DECK_ROWS).
- RENAME: TcgUi_Name* in tcg_ui.c (shared, tcg_ui_kit.h): a 13x4 keyboard + CASE / SPACE / DEL / OK, typing through
  Platform_SetTextInput; 16 letters and the width of a deck row. Tests type with the harness's `type` command.
- ACE CARD: TcgCollection.ace[] (cards format 7), YgoCollection.ace[] (.ygo format 3); Tcg_DeckAce / Ygo_DeckAce fall
  back to the old pick (Tcg_DeckMascot / Ygo_DeckMascot) when unset or no longer in the deck. COPY copies it, DELETE
  and a recipe clear it.
- DECK RECIPES: Tcg_DeckFromRecipe / Ygo_DeckFromRecipe build a recipe from owned cards (copies other decks use
  still count, as in the builder); the Pokemon recipes are the three starters and tcg_structure_decks.h, the
  Yu-Gi-Oh! ones the four Starter Decks (StarterDeckOf).
- Yu-Gi-Oh! zones: YgoAnswer.zone (seq + 1) on a Main Phase answer that puts a card down from the hand; Idle()
  keeps it in YgoState.zoneWish, TakeZone() uses it when the card lands (else the first empty zone), and the next
  Main / Battle question drops it. The board's M_ZONE mode (ZoneChoice: no Tribute Summons, no Field Spells) shows
  the free zones and the card faint in the chosen one. Sent online (ygo_net.c ANSWER_LEN 22, TCG_LINK_VERSION 7).
- Yu-Gi-Oh! turn menu (OpenPhaseMenu): ChoicesAfter plays the move on a copy of the duel, passing chain windows, to
  see whether an attack or a play from the hand follows. PreferredPick starts attack targets / Tributes on the
  best / weakest. Animations: FX_ATTACK (AttackReach: wind-up, dash, home; Attackers() draws it over the band),
  FX_IMPACT (+ Shake via the render viewport), FX_RETICLE (Brackets, MarchingLine), FX_SHARDS, FX_RISE.
- Pokemon table: Refocus() after our own action's state is shown (B.refocus, set in Submit).
- Text fitting: TcgUi_WrapLines / TcgUi_FitText; tcg_board.c Prompt() splits a long prompt in two lines.

## Triple Triad (src/tt/, include/tt/; 1.40.0)
The CARD CASE's third side. Decisions: docs/TT_DECISIONS.md.
- Data: tools/tt/fetch_cards.py reads every Final Fantasy XIV card from xivapi (TripleTriadCard + Resident: names,
  the four numbers, stars, type, sale value, XIV's order) into tools/tt/cards.json, and both pictures (088xxx 40x40
  icons, 087xxx 104x128 faces) plus the card frame (ui/uld/CardTripleTriad_hr1: the grey plate, the star, the type
  icons) into the gitignored tools/tt/metadata/. gen_catalog.py writes src/tt/tt_catalog.c and include/tt/card_ids.h
  (ids are XIV's and never move); build_art.py packs graphics/tt/cards.bin (each face at full size, at the board's
  40x49 by Lanczos, the icon; the back (graphics/tt/back.png), the plate at both sizes, star and type icons, the large
  numbers (graphics/tt/numbers.png + the A of graphics/tt/ff8_menu.png) at 10x14 and 7x10; .incbin'd by
  tt_art_data.c). gen_pools.py writes src/tt/tt_pools.c: each region's cards, each seller's shelf (checked under
  15%), each special player's five-star cards (every one belongs to someone).
- Rules (tt_duel.c): FF8's capture code as the decomp reads it (Same, Plus, Plain, the COMBO loop), XIV's rules
  beside them; deterministic from seed + moves (TtDuel_Hash). Events (TTE_PLACE / RULE / FLIP / SHIFT / ROUND /
  END) drive the table. AI tt_ai.c (negamax, alpha-beta, endgame search from six empty squares). Hands
  (tt_hands.c): a region's players rotate through its cards a hand a day, stars by badges; specials one of their
  five-star cards a day. Deck limits in tt_common.c (Tt_DeckFits).
- Collection and the regions (tt_collection.c): "<save>.tt" (tagged records "MTTR", unknown tags skipped), owned
  copies, seen, the shelves' sold singles, six decks, packs, each region's rules / trade rule / games, the rules
  carried and from where, the CARD QUEEN's region and day, records. Tt_MixRules (spread 30 / abolish 20, 50 / 40
  where the queen is), Tt_PlayedIn, Tt_QueenMoves. Written with the save (Tcg_WriteSidecar), backed up with it
  (sdl2.c sSidecars).
- Screens (tt_ui.c, the kit of tcg_ui_kit.h): HOME, DECKS (EDIT / RENAME / PLAY WITH IT / COPY / DELETE), the
  builder and the CARD LIST tabbed by stars (L / R), INSPECT, PACKS and the reveal, RULES OF HOENN, CHALLENGE
  (where the rules mix, announced), the table, SPOILS (the trade rule's picks), a seller's counter (SINGLES /
  PACKS / SELL), the CARD QUEEN's menu. tcg_ui.c routes Step / Draw / IsActive here like the Yu-Gi-Oh! side;
  Tcg_OpenCaseGame opens any side; L / R go round POKeMON, YU-GI-OH!, (MAGIC,) TRIPLE TRIAD, always last (TcgUi_CaseTabs: the chosen
  side's tab full width). The last side: TT_FLAG_CASE_SIDE.
- The table (tt_board.c): XIV's layout, hands stacked bottom-up so every card's numbers show; phases DEAL, RULES,
  SPIN (who starts), PLAY, EVENTS, END. Pictures and numbers: tt_gfx.c (the plate tinted per seat once; a 5x7 pixel
  font of our own for numbers and banners, Tt_Char / Tt_Text).
- Who plays (tt_field.c Tt_FieldPick, asked first by tcg_field.c Ask): TT_PLAYS_ONLY / TT_PLAYS_ALSO by trainer
  class or sprite; specials by sprite (and map for MR. STONE / MR. BRINEY); sellers and the queen by their script
  (EventScript_TtSeller / EventScript_TtQueen). tcg_field.c's which-game menu holds up to three games (sGames);
  Tt_WhichLine gives each kind of player its own "which?" line.
- Map side: data/scripts/tt_seller.inc (EventScript_TtSeller, EventScript_TtQueen, EventScript_TtPlayer,
  EventScript_TtMapSetup called from each seller's POKeMON CENTER's ON_TRANSITION); specials Tt_SellerState
  (gStringVar1-3: region, rules, trade rule), Tt_SellerStarter, Tt_SellerShop, Tt_MapSetup, Tt_QueenTalk /
  Here / FirstMeeting. FLAG_TT_QUEEN_AWAY (FLAG_UNUSED_0x022) hides the queen outside her region; the region is
  the map section's (Tt_CurrentRegion; buildings of no town keep the last region).
- Who starts (1.53.0): tt_board.c SpinPhase: the light runs side to side, an even count of runs ends on the left
  (you), odd on the right, so it stops where it lands (it used to jump at the end); the tick plays as it reaches each
  name and the pin when it stops. The deal and the spin can't be skipped.
- Music: every game plays a battle song (1.52.2): ordinary players and friends MUS_RG_VS_TRAINER, specials their own
  (MUS_VS_GYM_LEADER, MUS_VS_CHAMPION...), a win MUS_VICTORY_TRAINER; as the `tt` scene, `@tt` files replace them.
- Achievements: the TRIPLE TRIAD tab (counters ttwin, ttsame, ttplus, ttcombo, ttspread, ttabolish, ttspecial,
  ttcards, ttpack). LEGENDS OF THE TABLE counts Triple Triad specials too.
- Tests: tools/tt/test.sh (rules scenarios from real cards, 2000 random games replayed, the collection file, the
  spread odds, every card winnable), tools/tt/test_ui.sh (seller, case, practice, gamecenter, which, queen).
  Debug: `mp tt ...` (tt_ui.c Tt_Debug): starter, giveall, give ID N, packs P N, first N, auto, rules R HEX,
  trade R T, carry R, queen R, favor, practice, case, shop R, queenmenu, focus N, win, state, net, concede.
- With friends (1.41.0, protocol 62, relay extension 255 version 8): asked like the other card games
  (tcg_online.c request kinds TCG_ASK_TT_DUEL 4 / TCG_ASK_TT_TRADE 5; CO-OP > FRIENDS > CARD DUEL / TRADE with
  TRIAD picked, or the CARD CASE button in front of them while the case is on its TRIPLE TRIAD side). A game:
  tt_net.c (struct TtNet, type 14, the kind in byte 7) carries the asker's rules (their region's) and hand in the
  INVITE, the other's hand in the ACCEPT (RANDOM: each deals their own from their collection), then each move with
  the game's hash before and after; the next move waits for the ACK. Who starts comes from the session
  (TtNet_First). A concession crossing a move on the wire takes the move back (the table gets 254, then 255); two
  crossing: the asker's stands. tt_ui.c VS_FRIEND: the CHALLENGE screen picks the hand (the one asked sees "THEIR
  RULES" once the invite is in), T_WAIT, the table with me = our seat, their moves replayed into U.duel while the
  table is still (FriendStep). No cards change hands, no rules spread, no region played in; a win counts
  onlineWins, ttwin and TABLE TALK. Trades: T_TRADE, cards beyond the deck in use, by stars (L / R), through
  Tcg_SocialOffer / Confirm. Tests: tools/tt/test_net.c (in test.sh: 2000 linked games, the crossings, a tampered
  move, the timeout), scenarios tt_online (+ tools/test/check_tt_online.sh: the same game on both) and tt_trade.
- Not done: tournaments.

## The three card games together (1.45.0)
What the POKéMON, Yu-Gi-Oh! and Triple Triad screens share, so they feel like one game.
- INSPECT: PLATFORM_KEY_INSPECT (0x800, past the GBA's ten keys; main.c masks gMain keys to KEYS_MASK so the game
  never sees it). The right trigger sets it while a card screen is up (gamepad.c, Tcg_IsActive; the trigger's turbo
  is off then), D on the keyboard (sdl2.c), INSPECT in test scripts. The card files read it as KEY_INSPECT (2048);
  R and START stay what they were (pages, menus). Hints write {INSPECT}: the RT/R2/ZR icon on a controller, a drawn
  "D" keycap on the keyboard (tcg_gfx.c G_BUTTON_INSPECT).
- CONCEDE lives on your own deck in the POKéMON and Yu-Gi-Oh! duels (tcg_board.c OpenDeckMenu, ygo_board.c deckMenu;
  logs "deck menu: CONCEDE"); the B / START turn menu no longer has it. The deck's tooltip shows the counts and
  "{A} CONCEDE". Triple Triad keeps it in its turn menu.
- One cursor: G_Focus (tcg_gfx.c), the POKéMON table's corner brackets stepping out every 8 frames, white, yellow
  (G_FOCUS_TARGET) while choosing. The Yu-Gi-Oh! table and its card lists use it; picks carry the POKéMON table's
  numbered badge. Menus share TcgUi_Row and the red G_Frame card cursor; Triple Triad's game table keeps its own.
- Challenge pictures: tcg_field.c CaptureChallengePic: a trainer's TRAINER_PIC (from its trainerbattle script),
  else sPicOfGfx (the overworld graphics' own or look-alike picture), else the map sprite as before; a friend gets
  MP_Avatar_RemoteFrontPic. Stored 64x64 in the opponents' portrait (portraitW 64 = a battle picture) and drawn by
  G_PicCut: 1x, cut to the window, framed on the top of the figure. Logs "challenge picture: graphics G, trainer
  picture P". The CARD QUEEN's own screen keeps her map sprite.
- The deck card on the CARD CASE's first screen floats the same in all three (whole pixels over a still shadow).
- Effects shared by the tables: G_Circle; the POKéMON table got the Yu-Gi-Oh! table's FX_IMPACT (an attack's damage),
  FX_SHARDS (a Knock Out) and FX_RISE (a Basic put into play), run beside the existing waits, none longer.
  `mp cards fx 0/1/2` plays one on their Active.
- Victory song (1.52.3): a win plays MUS_VICTORY_TRAINER in all three (Yu-Gi-Oh! too now, on its YOU WIN! banner) and
  it plays on through the cards to take: the UI calls XBoard_EndHoldMusic instead of XBoard_End when a win goes on
  to the spoils screen; each UI's Screen() calls XBoard_ReleaseMusic (the map's song again) for any screen but the
  spoils and INSPECT, so it ends when the cards are taken (or the case is closed). Logs show the order with
  MOEMON_MUSIC_LOG=1.
- No buttons while the table shows what happened (1.52.3): the boards drop the frame's presses while their events
  play, so nothing can be picked or moved mid-animation and no press lands after it. POKéMON: TcgBoard_Busy (the
  deal, a state being presented, the queue, a drawn card's spotlight; not the sprites still gliding after a cursor
  move, which would make the cursor slow). Yu-Gi-Oh!: !YgoBoard_Idle (events left or the timer running). Triple
  Triad: a card flying or a banner up, and the deal and the spin can no longer be skipped (the rules page still can).

- HOW TO PLAY (1.53.0): SELECT on any game's first CARD CASE screen opens src/tcg/tcg_guide.c over it (Tcg_Step
  gives it the buttons first; the canvas draws it instead of the screen). Pages per game (sPokemon, sYugioh, sTriad):
  a title, a few words in the strip (two lines of the small font or three of the narrow one: opening it logs
  "page N's words don't fit" otherwise, and the tests refuse that), and a stage drawn from the page's own frame count
  with the games' own card faces. Triple Triad's examples use real cards whose numbers fit (FindCards: a plain capture,
  SAME, PLUS, a COMBO corner). LEFT / RIGHT / L / R / A turn pages, B or SELECT close it once let go (so the screen under
  it never sees the press). The narrow font has no "=": labels say "BEATS", "AND", "BOTH". Tests: each game's test_ui
  "guide".
- The case's menu and its Decks / Booster pages draw the row cursor 1 px lower (TcgUi_RowDown around those three
  screens' draws; the deck builder keeps its own), the binders' and Pokemon / Yu-Gi-Oh! booster pages' card frames too.

## SEARCH & SORT (1.54.0)
One menu for the BINDER and the deck builder of all three card games: src/tcg/tcg_search.c / .h.
- The key: PLATFORM_KEY_SEARCH (0x1000, past INSPECT's 0x800): the left trigger while a card screen is up (gamepad.c,
  Tcg_IsActive), F on the keyboard (sdl2.c), SEARCH in test scripts; KEY_SEARCH in each card file's key enum, {SEARCH}
  in hints (the LT/L2/ZL icon, a drawn "F" keycap: tcg_gfx.c G_BUTTON_SEARCH). R2 stays INSPECT, L / R the pages/tabs.
- Each game hands over a SearchGame: its filters (SF_CHOICE with names / values, SF_MIN / SF_MAX from lo by step;
  names on a MIN show Triple Triad's A), its sorts (key -1 by name, -2 by copies owned, else value()'s key:
  SEARCH_KEY_NUMBER, SEARCH_KEY_RARITY, then the game's own), name(), value() and owned(). SEARCH_NA: the card has no
  such value (a Trainer's HP): any filter on it fails and sorting puts it last either way; a choice whose value is
  SEARCH_ALL lets everything through. Ties keep the screen's own order.
- TcgSearch_Apply filters and sorts the ids the screen would show, after the screen's own rules (what's visible /
  collectible, the type or star tab), so it never shows anything new. tcg_ui.c / ygo_ui.c / tt_ui.c keep the result in
  a Shown() cache keyed on the screen, tab, owned counts (and Triple Triad's seen cards) and TcgSearch_Generation;
  ListCard / ListSize (Triple Triad: BinderList / BuilderList) read it, so A / SELECT / INSPECT / buying act on the card
  under the cursor as shown. The trade screens keep their own lists. Sorting never touches decks or counts.
- The cursor stays on its card when the list changes (SearchStep: ListIndexOf the card it was on; gone: the first).
- Menu: SEARCH (A: TcgUi_NameOpen, the keyboard or the screen keys; every letter updates the list, B puts it back),
  SORT BY, ORDER, OWNED ONLY, the filters, CLEAR ALL, DONE; LEFT / RIGHT change a row, START empties it, B or the key
  closes. The header counts the cards. Badge (TcgSearch_Badge) at the strip's bottom right: orange with what narrows
  the list and the count, blue "BY <sort>" when only sorted. An empty list says "No cards match." and how to clear.
- POKéMON: CARD, TYPE (not for Trainers), STAGE, MIN / MAX HP, WEAKNESS / RESISTANCE (NONE too), RETREAT COST; sorts
  NUMBER, NAME, RARITY, OWNED, HP, RETREAT COST. Yu-Gi-Oh!: CARD, MONSTER TYPE, ATTRIBUTE, LEVEL (the catalog ends in
  2003: no Ranks or Links), MIN / MAX ATK and DEF ("?" is none), LIST STATUS from Ygo_CopyLimit (the Forbidden &
  Limited list in use); sorts NUMBER, NAME, RARITY, OWNED, LEVEL, ATK, DEF. Triple Triad: STARS (BY TAB = the tabs;
  ALL or a star: the tabs show it, and L / R hand back to the tabs: ListTabStep), MIN TOP / RIGHT / BOTTOM / LEFT
  (A = 10), TYPE (PRIMAL, SCION, SOCIETY, GARLEAN, NO TYPE); sorts NUMBER, NAME, STARS, OWNED, each side, TOTAL. A card
  not met yet has no name to find and no sides or type to filter or sort by (it stays "???").
- Settings: "<save>.search", a tab-separated line per game ("pokemon", "yugioh", "triad"); a game's line is read when
  it's first registered, saving keeps the lines of games not opened since start. Saved when the menu closes.
- Tests: tools/test/card_search.sh (typing with the harness's `typetext`, take out / add, INSPECT and back, paging,
  nothing found then CLEAR ALL, sorts); `mp cards|ygo|tt search text X | sort N | desc | owned | filter I V | clear`
  and `mp cards|ygo|tt shown` (the list, the cursor's card and the deck's count).

## Every POKéMON card attack (1.46.0)
- The last 20 command lists from the original's effect_functions.asm (pret/poketcg), as FX_* kinds in
  tcg_effects.c (generate_effects.py "lists"): Mix-Up, Wildfire, Amnesia, Magnetic Storm, Wail, Scavenge, Destiny
  Bond, Prophecy, Mystery Attack, Devolution Beam, Mirror Move, Gale, Hurricane, Metronome, Morph, Conversion 1 / 2.
  Every attack of the 228 cards has an effect now (no "Effect not yet implemented").
- Choices: TcgAction.option (TcgDuel_AttackOptions: the menu's labels and which can be picked) and .copy (Metronome:
  the Defending Pokemon's attack + 1; tcg_duel.c uses that attack in Metronome's place, its own type kept), then
  TcgDuel_AttackAsk (Scavenge's Trainer, Prophecy's order, Devolution Beam's Pokemon). The table builds them as
  steps (tcg_board.c AttackSteps / AttackStepsAfterOption, ST_ATTACK_OPTION, SRC_TOP3_*), logs "attack menu: N
  choices", "metronome copies NAME", "attack option N". The AI: TcgDuel_AiAttackChoices.
- New per-Pokemon state (TcgFieldCard): disabled (Amnesia, until its owner's turn ends), destinyBond (until its
  owner's next turn), weakAs / resistAs (Conversion, until it leaves the Active spot; TcgDuel_Weakness /
  TcgDuel_Resistance), mirror* (what the opponent's last attack did to it, recorded after every attack, cleared at
  the end of its owner's turn). New events TCG_EV_DEVOLVE / TO_HAND / MORPH / DECK_CHANGED.
- TCG_LINK_VERSION 9: an action carries option and copy (bytes 25-26, selections from 27).
- When the hand has nothing left to play (tcg_board.c HandHasMove: HandPlayable plus TrainerHasUse for the plain
  cases), the cursor goes to the Active ("refocus: nothing left to play, your Active"; `mp cards checkdone`,
  `mp cards discardhand` stage it). Tests: tools/tcg/test_attacks.c, test_ui.sh "done" and "metronome".

## Yu-Gi-Oh! table: backgrounds, END TURN, menu widths (1.47.0)
- Backgrounds: YgoBoard_SetTheme (ygo_ui.c ThemeOf: the opponent's main deck's most common Attribute; online
  YGO_BG_ONLINE), built once per theme as a 240x160 texture (ygo_board.c BuildTheme). A face-up Field Spell on either
  side (FieldSpellCard) covers it with Ygo_ArtTexture of that card, cropped to the table and darkened, so any Field
  Spell, new ones too, gets its own; logs "field background: NAME". `mp ygo fieldbg ID` shows one for tests. (The
  board isn't redrawn every frame when nothing moves: nothing here is timed by draws.)
- The Battle Phase query also offers YGO_ACT_END_TURN: the turn frame's b = MAIN_END, TS_BATTLE_DONE skips Main
  Phase 2. The phase menu's cursor starts on MAIN PHASE 2; the AI still prefers Main Phase 2. TCG_LINK_VERSION 10.
- Menus: WindowBox grows to its longest row (RowsWidth) and stays on screen.

## The DANCE machine (src/rhythm/, mp_rhythm.c; protocol 61)
Decisions: docs/RHYTHM_DECISIONS.md. The GAME CENTER's 3F picture is the user's (docs/user_images 43); 44 marks what's
drawn over the player (blue: read by gen_gamecenter.py from graphics/moemon/gamecenter/3f_marks.png) and where to
walk (red: checked against the FLOORS grid; row 6 in front of the arcade cabinets is floor now).
- The game: rh_chart.c (StepMania .sm / .ssc: header, BPMs, stops, the dance-single charts; tools/rhythm/test_charts),
  rh_play.c (Beware's EXTREME build's rules: windows, rows, combo, freezes judged by row pairs, MAX2 score, dance
  points, LifeMeterBar; tools/rhythm/test_play runs every chart perfect / 50 ms late / unplayed), rh_field.c (a
  field), rh_screen.c (the credit, the screens, online sync RN_*), rh_gfx.c (a 24-bit 240x160 picture, the game's
  fonts through text.c MP_RenderFontGlyph), rh_audio.c (songs streamed and resampled into the game's 42048 Hz mix;
  the clock: samples mixed + what's queued + the device buffer, smoothed against the PC clock), rh_songs.c (the
  song list), rh_assets.c (graphics/rhythm/rhythm.bin from tools/rhythm/gen_assets.py: the theme's and note skins'
  art cut down, the announcer's 207 lines and the theme's sounds as Ogg files).
- In the game: sdl2.c ScreenStep draws Rhythm_Step into sFrameImage (the GBA picture: scaling, filters, SPECTATE and
  shots carry on), Platform_QueueAudio mixes Rhythm_MixAudio instead of the GBA's, ProcessEvents hands every event to
  Rhythm_OnEvent (steps timed by SDL's event time). Headless tests step on the harness's D-pad at the frame, or
  MOEMON_DANCE_AUTOPLAY=1 steps on every arrow.
- The map side (mp_rhythm.c): pads by where the player stands (row 7, x 0-2); MPR_RH_PAD (32) repeated each second
  (side, state, arrows held) for the bubbles and the sprites drawn up on the pads (x2 / y2, mp_overworld.c
  MP_Rhythm_AvatarPose); MPR_RH_ASK / ANSWER (33-34) to join a waiting friend; MPR_RH_SESSION (35) carries rh_screen's
  RN_* messages. Specials MP_Rhythm_StepUp / PlayAlone / Wait / Join / Result (data/scripts/dance_machine.inc).
- Hops (mp_rhythm.c struct PadHop, mine and each friend's): the player hops up onto the pad (an arc 10 px high,
  SE_LEDGE) when starting alone, waiting (WAIT FOR 2P) or accepting a friend; the credit starts once they're up
  (Task_StartWhenUp). On the pad: two or more arrows at once make a small double hop facing by the arrows (LEFT+RIGHT
  alternates west / east, LEFT combos west, RIGHT combos east, three or more north); single LEFT / RIGHT / UP turn
  that way, DOWN keeps the facing: never south. After the credit (or giving up waiting) the player hops down and the
  script waits until they're down (Task_EnableWhenDown). The rails in front of the pads are drawn over a player up
  on them (3f_marks.png blue, docs/user_images 50). MOEMON_DANCE_TRACE=1 logs each hop's facing.
- The screen turns on and off like an old TV (src/platform/crt_effect.c Crt_Apply, shared with the VS cabinet:
  24 frames each way; rh_screen.c ST_CLOSING holds the last picture while it collapses, then MP_Rhythm_Ended).
- The closing line (MP_Rhythm_Result into gStringVar4): from the stages' average grade (a failed stage counts as E),
  whether one failed and the hardest meter played (CreditMessage: "That was easy!" ... "My legs feel like jelly...").
- Records: the server's (mp_scores.c, store ops MP_ST_RH_* 40-42, rhythm_scores.dat in MOEMON_STORE_DIR or where it
  runs; top 10 a chart, one per name), cached in mp_rhythm.c; the player's bests, cleared songs and options in
  "<save>.dance" (bests made offline are sent when next at the machine online). Achievements (category DANCE)
  from MP_Rhythm_StageDone, counted when the RESULTS are left (the friend's final score is in by then).
- The songs: stepmania/Songs in the repository (Beware's 240, simfiles, .ogg, banners, title cards). Not in the
  game's zip: the release workflow makes MoemonDanceSongs.zip (tools/release/package_songs.sh) and an update.txt
  line "songs <tag> <zip> <bytes> <sha256> <id>", keeping the last release's line when the id (a hash of every file)
  is the same; updater.c Updater_Songs* downloads and unpacks it a file at a time into stepmania/ next to the exe
  (stepmania/songs_version.txt = the id). RhSongs_Root: MOEMON_STEPMANIA_DIR, else that folder, else ./stepmania.
- Tests: dance_solo, dance_versus, dance_hop (tools/test/README.md), tools/test/dance_songs_test.sh (the download against a
  local server), tools/rhythm/test.sh (every chart).

## BATTLE ANIMS > MODERN (mp_modernanim.c; 1.43.0)
OPTIONS > BATTLE > BATTLE ANIMS: CLASSIC / MODERN (gMpConfig.battleAnims, ini battle_anims; MOEMON_BATTLE_ANIMS for
tests). This game only: the Classic animation script always runs (same sprites, tasks, sounds, frame count), MODERN
draws over the finished picture (sdl2.c VDraw after the battle weather), so friends on either setting stay in step.
Every animation through LaunchBattleAnimation: moves, general (stat changes, weather, Leech Seed...), status; battles
and contests. Special ones (ball throws, switches) are left to mp_ballfx.c.
- Effect sprites (template tileTag in the ANIM_TAG range): sprite.c AddSpritesToOamBuffer flags their OAM entries
  (gMpOamFx), gba_easy_draw.c PutObjPixel writes their pixels into gMpFxLayer while MODERN draws; GlowAndTrails turns
  them into a blurred halo of their own color pulled toward the move type's, plus fading afterimages (mixed in, not
  added: halos show on the pale battle fields as well as in caves).
- Hits: battle_anim.c Cmd_createsprite (hit splat tags), Cmd_createvisualtask (ShakeMon / ShakeMon2 / ShakeMonInPlace /
  ShakeAndSinkMon / ShakeTargetBasedOnMovePowerOrDmg / ShakeTargetInPattern on the target; HorizontalShake = a quake at
  the target's feet), Cmd_playsewithpan (a sound panned to the target: a softer burst). A burst: core light, shockwave
  ring (white inner line), sparks, the type's particles (sTypeParticles: embers rise, drops and rocks fall, leaves
  sway, bolts, shards, ringlets, wisps, stars, feathers, bubbles), a 1-2 pixel shake for strong hits. Charging: motes
  and a ring closing on the user; stat changes: chevrons up (warm) or down (cool).
- Gentle on the eyes: no whole-screen flash, local light fading over 6-7 frames, shake at most 2 px.
- Nothing outlives the Classic animation (gAnimScriptActive false: cleared).
Tests: anim_classic then anim_modern (every move both ways with MOEMON_ANIM_FRAMES: tools/test/check_anim_frames.sh
checks all 716 lengths match; shots of TACKLE, EMBER, WATER GUN, THUNDERBOLT, EARTHQUAKE, ICE BEAM, SHADOW BALL, SWORDS
DANCE).

## The BATTLE SIMULATOR (mp_sim.c; 1.43.0, protocol 63)
A counter NPC (OBJ_EVENT_GFX_SCIENTIST_2 at 12,5) in each BATTLE TENT lobby, data/scripts/battle_sim.inc. Opponents
are digital copies of other players from the last server: their look (MP_Avatar_DigitalFrontPic: the opponent pic
slot, palette turned to hologram cyan by brightness) and their top 6 by battles taken part in (MpLifeRec.battles, added
at the end of the record so older .life files load; counted in mp_life_battle.c Finish for every ally sent out).
- The record (struct SimRecord: name, Trainer ID, packed look, 6 BattleTowerPokemon) goes to the server on connecting
  (MP_Sim_OnConnected after WELCOME), back on the field after a battle (MP_Sim_BattleOver -> MP_Sim_Update) and on
  closing (sdl2.c -> MP_Sim_OnClose, flushed). During a simulator battle the record is of the real party (sSim.saved).
- Server: mp_scores.c MP_ST_SIM_PUT (latest per name + Trainer ID, 256 players, "sim_players.dat") / GET -> ROSTER
  parts; a PUT is also passed to the room at once (part 0xFF: merge). Client cache "<save>.sim" (server name: the
  friend's server address or "online lobby", best streak, records), so it works offline; a save that never played
  online has none ("no battle data, play with others").
- The battle: BATTLE_TYPE_TRAINER | BATTLE_TYPE_EREADER_TRAINER (Frontier-level AI flags, no EXP, no whiteout, trainers
  switch as usual), gMpSimBattle for the picture (battle_controller_opponent.c), class "DIGITAL", name and lines
  (battle_message.c). Everyone at level 50 (SetLevel50: EXP of level 50, stats, healed); the party is saved before and
  put back after (CB2_EndSimBattle). Each opponent once before repeats; a loss ends the streak; prize money 500 + 250 a
  win (5000 at most); a new best at 3/5/7/10/15/20: RARE CANDY, PP UP, NUGGET, PP MAX, LUCKY EGG, MASTER BALL.
Tests: sim_tent (two players, a win at level 50, levels back), sim_nodata.

## Custom music: .kaisong songs (mp_kaisong.c; 1.44.0)

Moemon Music Studio's `.kaisong` exports (format and loading: docs/KAISONG_FORMAT.md) play in place of the game's songs.
- **Files.** `music/` next to the exe (`Updater_ExeFile("music")`, `MOEMON_MUSIC_DIR` for tests). `<song>.kaisong` =
  everywhere, `<song>@<place>.kaisong` = only on that map (place = `MAP_` id without `MAP_`, lower case). A place's file
  beats the everywhere one. Song names: `sound/song_table.inc` (`src/data/mp_kaisong_names.h`); only BGM-player songs.
  The repository's `music/` ships in every zip (package.sh), so a committed song reaches every player in the update.
- **Generated tables** (`python3 tools/kaisong/gen.py`, `--check` to verify; the release workflow runs it):
  `src/data/mp_kaisong_samples.h` (every DirectSoundWaveData_* / ProgrammableWaveData_*: referencing them also keeps
  them all in the binary), `src/data/mp_kaisong_names.h` (song names, map keys), `docs/SONG_TABLE.md` (every song and
  where it plays: map music from map.json, script playbgm / playfanfare / savebgm per map and shared scripts, trainer
  battles by class from GetBattleBGM and encounter music by trainer, on the maps those trainers stand on, legendary
  battles from battle_setup.c, C screens; plus the custom songs). The studio imports gen.py at build time
  (gen_soundfont.py: sf_places, sf_maps, sf_song_md) for its SONG > Replaces picker and writes the same page next to its
  exports (export.c export_song_table; its make test checks the two are byte for byte the same).
- **Scenes** (`<song>@<scene>.kaisong`). include/mp_kaisong_scene.h wraps every call that starts a song (sound.h,
  m4a.h, pokemon.h; files that define them say `MP_KAISONG_NO_SCENE`) to record the asking file (`MP_Kaisong_Request`,
  nested calls keep the outer one). gen.py: a scene is a source file that names songs (`src/mp/mp_x.c` = `x`, `src/x.c` =
  `x`, a folder `src/tcg` / `ygo` / `tt` / `rhythm` = one scene), unless it is the world's (WORLD: field, scripts,
  battles, co-op battle code: their songs are the map's places); its songs are the MUS_ constants it names (not ones it
  only compares with); the label is SCREENS / SUBDIR_SCENES or a `// MUSIC SCENE: Name` comment in the file. The game
  gets the scenes, their songs and files in mp_kaisong_names.h (sKaisongScenes / sKaisongSceneFiles, looked up by
  `__FILE__`). Rule: a song the asking scene names plays `@scene`, else everywhere, never a map's file; any other song
  (a scene handing back the map's song) is the map's. A scene using the game's battle code holds the music
  (`MP_MUSIC_SCENE_HOLD(MUS_...)` / `RELEASE`, mp_sim.c: the songs named are what the picker lists for it). Card boards
  hand the map's song back with `MP_MUSIC_TO_MAP(...)`. `MOEMON_MUSIC_LOG=1` logs each song's scene.
  tools/kaisong/test_scenes.py adds scene files and checks they are picked up.
- **Loudness** (1.52.1; automatic from 1.52.2). `python3 tools/kaisong/loudness.py [--write] [--passes N]` plays every
  file in `music/` and the song it replaces in headless games (`mp playkai FILE` / `mp playsong N`: a clean sound state;
  then `mp musiclock 1`: the test-only `gMpTestMusicLock` in m4a.c stops the overworld from changing the BGM while it is
  measured) and prints how loud each plays; `--write` puts the difference in the file's INFO `gain=` (dB). Files with
  the same music (a hash of every chunk but INFO, as the game's `KaiFile.music`) are one group: measured once, one gain
  (the mean of their songs' levels), so the music doesn't jump where the game plays it on from one to the other. Until
  1.62.0 the release and dev release workflows ran it (`--reference 0 --write --passes 2`) on the shipped copies; it
  never took effect there and then hung the 1.62.0 release, so (the user's call, 1.62.1) no workflow runs it: the
  gains in music/ ship as they are, and the tool is run by hand when wanted. The game applies
  the gain per voice in music_player.c ChnVolSetAsm (`sChnGain`, from MP_Kaisong_Gain of the player's song header;
  256 = unchanged, so the game's own songs and the bit-exact studio comparisons stay the same). kaisong_test.sh check 6.
- **Same music, no restart** (1.52.2). `MP_Kaisong_SameMusic(song, warp)`: the file that would play there holds the
  music already playing (the same file, or the same `music` hash: the GAME CENTER's 1F / 3F `mus_game_corner@` and 2F
  `mus_poke_mart@`). Overworld_PlaySpecialMapMusic and TransitionMapMusic then call sound.c `MP_KeepMapMusic` (the map's
  song number changes, the music plays on), TryFadeOutOldMapMusic doesn't fade, and MP_Kaisong_Differs is FALSE. Only
  custom songs: two of the game's own never count as the same. kaisong_test.sh check 7 (the clock in `mp bgm` goes on).
- **Hooks.** m4a.c `SONG_HEADER(n, song)` in every m4aSongNum* function; MPlayStart on the BGM player calls
  `MP_Kaisong_Started` (sample channels = the file's INFO voices, 5 for the game's songs; dropping channels clears the
  ones past the count). overworld.c: `MP_Kaisong_Differs` in Overworld_PlaySpecialMapMusic, TransitionMapMusic and
  TryFadeOutOldMapMusic (the same song number, another file on the next map: fade over like a new song).
  mp_spectate.c: the song number + the place variant in bits 10-15 (protocol 64).
- **Engine widening.** `gMPlayTrack_BGM[64]` / `NUM_TRACKS_BGM` 64 (PORTABLE; MPlayOpen's cap is 64 there too: until
  1.52.2 it was still MAX_MUSICPLAYER_TRACKS 16, so a song's tracks past 16 never played), MP2KPlayerMain's track bits are u64 and
  `status` keeps a nonzero 16-bit summary (title_screen.c's `status & 0xFFFF` still works), `TRACKS_ALL` reaches every
  track in m4aMPlay*Control, `MAX_DIRECTSOUND_CHANNELS` = `MAX_SAMPLE_CHANNELS` = 64 (the mixer only runs maxChans of
  them). ply_memacc's not-taken branch skips a whole pointer.
- **Tests.** tools/test/kaisong_test.sh (studio + game, audio sample for sample). Debug: `mp kaisong` (files),
  `mp bgm` (song, tracks, voices, the player's tracks and clock), `mp playsong N` (a clean sound state first: Game Boy channel phase, frame clock and
  frequency registers carry over from earlier sounds in the game, so only a clean start matches the studio's render).


## Yu-Gi-Oh! on EDOPro (1.48.0)

The rules engine is EDOPro's: every card with every effect, today's rules. The catalog (ygo_catalog.c) stays the cards
players can collect and see; anything else a duel brings in (tokens, a card an effect creates) gets an id past
YGO_CARD_COUNT made from EDOPro's database (Ygo_EdoCard).

- Code: edopro/ocgcore (edo9300's ygopro-core, commit in dist_extras/EDOPRO_CREDITS.txt) with its Lua 5.4 compiled as
  C++ (Makefile EDO_SRCS / EDO_CXXFLAGS; Lua's four global tables renamed edo_* so the fight engine's Lua 5.1 can
  link beside it; libstdc++ static on Windows; CI needs g++-mingw-w64). Duel flags: Master Rule 5 without the Extra
  Monster Zones (five zones a side, as the table draws).
- Data: graphics/ygo/edo.pak, built in (ygo_edo_pak.c .incbin), from tools/ygo/edo/build_pack.py: cards.bin (every
  card of BabelCdb's cards.cdb with name, text and effect strings), the root and official/ CardScripts, the LFLists,
  strings.conf. Files are grouped in ~256 KB raw-deflate chunks (stb's zlib decoder); a few chunks are cached.
  build_pack.py also writes src/ygo/ygo_edo_codes.c (each catalog card's code: its password, else by name).
- The bridge (src/ygo/ygo_edo.c): runs OCG_DuelProcess to a question; each message becomes events and moves objects in
  lists mirroring the engine's (Deck, hand, GY, banished, Extra Deck in order; zones by sequence; MSG_MOVE removes
  then inserts like the engine). When the engine stops, every location is read back (OCG_DuelQueryLocation): codes,
  positions, ATK/DEF, counters, equips; if the objects disagree with the engine it rebuilds them (gYgoEdoRebuilds,
  tests require 0). The engine's player 0 always goes first: GP() swaps when the game's player 1 starts.
- Questions: IDLECMD / BATTLECMD / CHAIN / EFFECTYN / YESNO / OPTION / CARD / TRIBUTE / UNSELECT_CARD / SUM /
  POSITION / PLACE / COUNTER / SORT / ANNOUNCE_* / ROCK_PAPER_SCISSORS map to YgoQuery types (YGO_Q_SUM, SORT,
  COUNTER, NAME are new; OPTION can take several picks). Answered here without asking, always from the duel and its
  answers: a chain window with nothing to chain, SORT_CHAIN (default order), the zone of a Main Phase answer
  (YgoAnswer.zone, also "first empty"), and "Attack directly?" turned into one attack-target pick (the opponent's
  monsters or their LP; the following SELECT_CARD answered with the pick). Labels: YgoDuel_QueryText / ItemText from
  the card's effect strings and the system strings.
- Copies: a YgoDuel owns its engine (d->edo); never assign one. YgoDuel_Copy and YgoDuel_Rewind rebuild from the seed and
  the answers (about 25 ms to start a duel); ygo_net.c rewinds when a concession crosses an answer; ygo_ui.c's online
  Y.duel is a view of the link's duel (ShowOnline, duelShown: never freed from there).
- AI (ygo_ai.c): rule-based on the questions (no lookahead: the engine can't be copied cheaply), never reading hidden
  cards (Seen()).
- Deck codes: Ygo_ReadYdke / WriteYdke / ReadYdk / WriteYdk (ygo_collection.c); a code's card the catalog hasn't got
  (YgoEdo_CatalogOf also matches other printings) refuses the deck with "This deck contains cards that are not
  currently implemented. Please try another code."; cards not owned stay in the deck, gray in the builder (the deck
  is ready once they're owned). FROM CODE takes a code from the clipboard as it's copied (and typing / Ctrl+V);
  DECK CODE copies one. .ydk files through src/platform/file_dialog.c (Windows' Open / Save As; not offered elsewhere).
- Lists: Ygo_BanlistCount / Name / UseBanlist: 0 the game's own (Stairway's), then each EDOPro list (whitelists like
  GOAT allow only what they name). The player's choice is YgoCollection.banlist (.ygo format 4); opponents' decks
  follow the game's own (Ygo_GameLimit).
- Opponents' decks: tools/ygo/scrape_npc_decks.py reads Yugipedia's pages for the opponents of Stairway to the
  Destined Duel, The Eternal Duelist Soul, World Championship 2004, Reshef of Destruction, The Sacred Cards, World
  Championship 2006 and GX Duel Academy (1.56.0; three page layouts; on pages with decks from several games only the
  game's own section, scrape_npc_decks.py section(); names matched through EDOPro's database too), keeps decks the catalog nearly covers (two a character),
  fills short ones to 40 with Normal Monsters of the deck's own Attribute/Type; tools/ygo/rate_npc_decks.sh rates
  them by AI-against-AI duels (Elo) into src/ygo/ygo_npc_decks.c, weakest first; ygo_field.c GameDeck picks from the
  quarter matching the badge tier.
- The table turns the other side's cards to face them (Home: +180 degrees; piles too).
- Counters (1.61.5): YgoObj.counterType keeps the kind (EDOPro's number: 0x1 Spell Counter...; MSG_ADD_COUNTER, the
  first kind in QUERY_COUNTERS), YGO_EV_COUNTER carries it in flags; the names are strings.conf's "!counter" lines
  (YgoEdo_Counter; kept with the system strings, top bit set). The table shows a badge with the count at a card's top
  right (YgoBoard_CounterBadge; color by kind: Spell violet, Venom green, A-Counter blue, the rest amber), glows and
  logs each change ("Breaker the Magical Warrior got a Spell Counter (1 now).": arrivals start at 0 shown, their
  events bring the counters), and the close-up says "1 Spell Counter on it" with the badge on the picture
  (YgoBoard_InspectCounters: the object the last INSPECT was at). The online hash leaves counterType out (older
  versions agree). `mp ygo decktop ID...` puts cards at the bottom of the active Deck list (drawn first after
  `mp ygo stack`). Tests: test_edo_cards "spell counters", test_ui.sh "counters".
- Tests: tools/ygo/test_edo.c (determinism, AI / random duels, Copy / Rewind, no rebuilds, no answer the engine
  refuses), test_edo_cards.c (Decks unshuffled with gYgoEdoNoShuffle: Summons, Tribute, Dark Hole, Monster Reborn,
  Polymerization, a Ritual's sum, Mirror Force, Change of Heart, Scapegoat's tokens, Time Wizard's coin, Prohibition's
  declared name), test_collection.c (deck codes, lists); test_ui.sh "codes" and "table" (`mp ygo mineuntil Q`,
  `mp ygo code TEXT`).

## Pokemon Card GB2's cards (1.55.0)

The second Game Boy game's 445 cards (pret/poketcg2, the Japanese game; its English text and scans from
Bulbapedia, tools/tcg/fetch_gb2_text.py): 230 are the first game's cards and keep their ids; the 214 new ones are
ids 229-442, kept stable by tools/tcg/gb2_ids.json (tools/tcg/import_poketcg2.py writes them into catalog.json with
the GB2 decks and boosters). TCG_CARD_COUNT 442, 11 booster packs (packs 4-10 = GB2's seven sets 6-12; a first-game
card in a GB2 booster has it in `set2`), collection format 8 (442 cards, 11 packs, milestonesHi, gcDecks bits 8-31),
TCG_LINK_VERSION 15.

- Attacks: tools/tcg/gb2_effects.py maps every GB2 effect command list to an FX_* kind (amount, extra; after 2 =
  the handler reads both phases); generate_effects.py appends them to tcg_effect_table.inc. The new kinds are in
  tcg_effects.c Gb2Effect. Where the Game Boy game plays a card unlike its print (Horrid Pollen at random, Fury
  Strikes' markers at random, Cat Punch at a random Pokemon...) the game is followed and gb2_effects.TEXT is the card's
  text in this game (generate_catalog.py). Choices the print leaves to the player that the engine fixes (as it did
  for Gigashock): damage "divided as you like" goes to one chosen Pokemon; Earth Wave takes the 2 with the least HP
  left; Energy Control moves Energy off the Defending Pokemon; Texture Magic changes only the Resistance.
- Targets: TcgDuel_AttackChoice 3 = any of theirs (0-5), 4 = any of yours; TcgDuel_AttackTargetPrompt the words;
  phase 3 of TcgDuel_AttackEffect switches before the attack (Fox Fire, Drag Off). Swift (g->swift) skips Weakness,
  Resistance, Defender, guards and powers on the Defending Pokemon. Turn-numbered effects (lockTurn, skipDrawTurn,
  strangeBeamUntil, poisonMistUntil, powerOffUntil, goopGasUntil) end by themselves.
- Powers (tcg_powers.c, TCG_POWER_*): always-on ones hook the rules (TcgPower_NoWeakness / NoResistance /
  RetreatBlock / AfterRetreat / AttackBonus / KnockedOut / NoTrainers, TcgDuel_RetreatCost); "when you play it from
  your hand" ones check g->fromHand; Puppet Master goes through TcgDuel_Attacker (the card whose attacks the Active
  uses: everything about attacks asks it, never Field(0, 0)).
- Trainers and special Energy: tcg_trainers.c; Rainbow Energy pays any one type (EnergyPaid's shortfall), Recycle
  Energy returns to the hand when it leaves play (TcgDuel_Apply), Computer Error ends the turn (g->turnOver), Here
  Comes Team Rocket! turns the Prizes face up on the board. Bill's PC can't be played in a duel (it's for trades).
- Decks: GB2's 113 decks ("GB2 " names) join card players' choices: Gb2Kind by Energy then Pokemon type, Gb2Tier by
  rating against the first game's tiers (tcg_field.c); about half of card players give a GB2 booster of their kind.
- The GAME CENTER sells 14 structure decks (tools/moemon_gamecenter/gen_structure_decks.py): the 1999 Base Set's four
  at 15000, and the later English theme decks every card of which the two games have (theme_decks.json, from
  pokemon-tcg-data decks/en) at 18000. The Gym and Neo theme decks need cards neither game has.
- Tests: tools/tcg/test_gb2.c (every attack 40 times then the AI carries on, every used power, every GB2 deck to the
  end against others).

## Yu-Gi-Oh! to World Championship 2006 (1.56.0)

- Pool: tools/ygo/build_catalog.py adds the English sets from Invasion of Chaos to Shadow of Infinity, the Starter
  Decks Kaiba / Yugi Evolution and 2006, Structure Decks 1-8, and every card of World Championship 2006 itself
  (tools/ygo/wc2006.json from tools/ygo/fetch_wc2006.py: the game's card pages by password, its March 2006 Forbidden
  & Limited list, the decks' copies from Yugipedia's Set Card Lists, its own card names). 2311 cards, 33 sets; old
  ids never move (tools/ygo/ids.json). North American printings: "LOB-001" codes up to 2004, then "SOD-EN001"; an
  Ultimate Rare is the second printing of a card the set also has.
- Limits: Stairway's for its cards; the 2003 pool keeps what it had (3 when Stairway lacks the card) so no saved deck
  changes; the new cards follow World Championship 2006's list, never below what a deck sold here holds
  (Blaze of Destruction's two Level Limit - Area B).
- Sets past 16: YgoCard.sets is 64 bits (YGO_SET_BIT); the YGO_SET_* enum is generated into card_ids.h; YgoSet.starter
  is YGO_DECK_STARTER / YGO_DECK_STRUCTURE (0 a booster). Ygo_SetShortName drops "Starter Deck: " / "Structure Deck: ".
- Save: .ygo format 6 stores the card and set counts it was saved with (Ygo_CollectionLoad reads 1-5 as 1509 cards and
  13 sets) and 32 bits of sold decks (gcDecks).
- Art: tools/ygo/build_printed.py keeps the pictures printed.bin already has when the catalog grows; fetch_art.py
  fetches the new ones. The engine's codes: tools/ygo/edo/build_pack.py --codes BABELCDB (no new pak needed: edo.pak
  has every card).
- GAME CENTER 2F: every Starter Deck at 10000, every Structure Deck at 12000 (GcDeckCount / GcDeckSet / GcDeckPrice;
  row k = bit k of gcDecks, the four 2002-2003 decks keep bits 0-3), a scrolling list.
- DECK RECIPES (both card games): a deck sold at the GAME CENTER only once bought (gcDecks; Yu-Gi-Oh!'s chosen
  Starter Deck too, the Pokemon side's three starters always), then each special duelist's deck once beaten
  (specialBeaten): ygo_field.c Ygo_SpecialRecipe builds it at their best (tier 3, a fixed seed, their signature card
  in), cached. Tests: `mp ygo recipes` (gcshop), `mp cards recipes` (counter).
- Opponents: 112 World Championship 2006 decks and 9 GX Duel Academy decks join the others (7 Trials to Glory's aren't
  on Yugipedia); BuildDeck's themed decks and the boosters handed out draw on the whole pool by themselves.
- 1.56.1: every turn and phase passing is the POKeMON table's sweep (ygo_board.c Sweep / FX_SWEEP: same band, length
  and sounds), so a chain window in the Draw and then the Standby Phase reads as two moments, not one question asked
  twice. A direct attack's target is the far middle edge of the defender's side (FX_ATTACK value 1: it goes all the
  way); the blow's sound plays as FX_IMPACT lands (Advance) and the attack event waits only until then, so the damage
  follows the slam; YgoBoard_Idle holds input while an attacker is still flying. The end of a duel is G_ResultScreen
  (tcg_gfx.c), shared with the POKeMON table: its window as tall as its text (G_TextHeight).
- 1.56.2 / 1.56.3: every phase still gets its sweep (1.56.2 skipped empty ones; 1.56.3 shows them all again). The
  middle line's words about the card fade out while a sweep plays and back in as soon as it ends (B.infoFade,
  G_TextAlpha in tcg_gfx.c: text drawn from then on at that alpha). G_ResultScreen's timing for both tables: the
  banner alone for a second, then the box with its words (G_RESULT_BOX), A leaves from G_RESULT_READY; test scripts
  wait 320 frames after `mp ygo win` / `mp cards win`.
- 1.57.1: the middle line's words fade out (as fast as a sweep's band opens) the moment any turn or phase sweep
  comes up and back in as soon as none is on screen, in every phase; what to pick when the player is asked always
  shows. Cards turning over
  (FX_FLIP: a Flip Summon, a Set card flipped by an attack or an effect, one turned face-down) go edge-on and show
  their other side (Turn, Animated). A Tribute (YGO_EV_TO_GRAVE with YGO_REASON_TRIBUTE) goes white and up as light
  toward the middle of its side's Monster Zones (FX_TRIBUTE: the event waits for all of it, Advance moves it to the
  Graveyard after, so the table never catches up mid-way); a Tribute Summon gathers the light on its zone and bursts
  (FX_GATHER), then its picture big as TRIBUTE SUMMON. No beams of light: a Summon is a ring.
- 1.59.0, the Frontier Brains out with their cards: each stands by their facility's door on BATTLE FRONTIER's outside
  maps (map.json objects, hidden by FLAG_FRONTIER_CARDS_* = FLAG_UNUSED_0x023-0x028) once the player holds that
  facility's silver symbol. tcg_field.c Tcg_FrontierSetup (both maps' ON_TRANSITION) puts (n + 1) / 2 of the n beaten
  Brains out each day, a window over them that moves one along per Tcg_Today() (online: the room host's day). Their
  talk lines are data/scripts/frontier_cards.inc; the CARD CASE finds their decks by graphics id as before (sSpecials,
  sDuelists). Test scenario frontier_cards.
- 1.58.0, every POKeMON card can be had: the 17 Game Boy 2 promos no trainer handed over are milestone prizes under
  PRIZES (tcg_ui.c sMilestonePromos: duels won, packs opened, special trainers beaten, the four ELITE FOUR decks,
  the CLEAN SWEEP / PURE ENERGY achievements, booster sets complete), counted from what the collection and the
  achievements already hold so older saves claim theirs at once; claimed bits in TcgCollection.rewards (collection
  format 9). `mp cards stats WINS PACKS SPECIAL_BITS` sets the counters for tests. The card guide (docs/CARD_GUIDE.html,
  tools/card_guide/build.sh: every card of the three games and where it comes from) lists them.
- 1.57.0, the AI's "wait, what?" moves (ygo_ai.c): it reads a card's purpose from its words (Says): an Equip Spell
  goes on its own monster unless it cripples (HarmfulEquip: Paralyzing Potion, Germ Infection, Flint, Snatch Steal...),
  a boost (Boost: Rush Recklessly, Reinforcements) on its own; Pointless keeps back Life Point gains above 6000 LP,
  a field wipe that takes more of its own, an Equip with nothing of the right side, battle tricks (ForBattle:
  Waboku, Negate Attack...) away from a battle, and a boost on its own turn unless in Main Phase 1 it turns a battle
  around. It never chains to its own link (one answer to an attack: Magic Cylinder and Mirror Force aren't both
  spent). A FLIP monster is Flip Summoned when its effect has something to work on. The "second best" slip only
  swaps two close choices on the same side. Scenarios in tools/ygo/test_edo_cards.c ("ai ...").
- 1.57.0, the POKeMON AI (tcg_ai.c): Energy goes only where it brings an attack closer, its own or an Evolution's in
  the hand (EnergyHelps / Unmet; Hydro Pump-style attacks take up to two more), else it stays in the hand; Potion /
  Super Potion only on a hurt Pokemon (Super Potion for 40 or more), Defender on the Active when the opponent has
  Energy, Full Heal on a condition, POKeMON CENTER only when 50+ is healed and two Energy or fewer are lost
  (tools/tcg/test_ai.c). Ratings rerun (tcg_ratings.c, tcg_levels.c, the Yu-Gi-Oh! decks and cards).

## Rare card foil (1.56.0)

- src/tcg/tcg_foil.c (tcg_foil.h), shared by the three card games: G_Foil(kind, card rect, art rect, seed, frame) and
  G_FoilHalo. Everything is added light (G_FillAdd, SDL_BLENDMODE_ADD), never a color laid over the picture: a soft
  diagonal sheen (three layered widths, eased, brightest midway, then a rest), a few four-point twinkles at hashed
  places, a glint along the name strip (Rares), a light round the edge (Ultra / Secret). Kinds: GLINT, SILVER, GOLD,
  PRISM (pastel hues), COSMOS (POKeMON holos: pale prism and more twinkles), STAR (TRIPLE TRIAD five stars).
- The art rectangle comes from the code that draws each face, in every CARD ART mode: G_CardArtRect (tcg_gfx.c: the
  frame's box, or the printed scan's measured rectangle) and Ygo_CardArtRect (ygo_gfx.c, the same, at the face's
  built size scaled to the drawn one). TRIPLE TRIAD: the card inside a small inset.
- Where: the BINDER grids (POKeMON, Yu-Gi-Oh!), the big card of all three, and the pack reveals (with G_FoilHalo instead of the old
  colored box). Checked by bursts of rendershots (the light's union covers the picture exactly).

## Card AI levels, power ratings and how a match looks (1.49.0)

All three card games now share one idea of difficulty. What the player sees is a word and six pips on the challenge
screen (VERY EASY .. FEARSOME), worked out for the deck under the cursor, with a line under it that takes turns
saying how their cards compare and how they play (`TcgUi_Match`, tcg_ui.c). Ratings are Elo-style numbers and are
never shown.

- **AI levels** (0 BEGINNER, 1 CASUAL, 2 SKILLED, 3 EXPERT). Not "the best move N% of the time": each choice is
  weighed by how much worse it looks than the best one, e^(-loss / T), with T smaller the better the level, so near
  misses are common and blunders rare. On top of that, per-game slips people make: a Trap's moment missed, the
  second-best target, an Evolution or Trainer held for later, the Energy on the second-best Pokémon, the weaker
  attack. The dice come only from what both players can see (never the hidden cards), so a duel replays alike.
  Triple Triad: tt_ai.c (`sTemper`, `TtAi_View` turns cards it can't see into stand-ins as strong as what it has
  seen). Yu-Gi-Oh!: ygo_ai.c `YgoDuel_AiAnswerLevel`. Pokémon: tcg_ai.c `TcgDuel_AiActionLevel` (level 3 is the old
  AI exactly, so tcg_ratings.c stays valid).
- **Who plays at which level**: special players 3; ordinary ones by badges (1 early, 2 from 4 badges), some a level
  looser; the CARD CLUB lessons 0. When CARD SCALING can't find a deck/hand strong enough (badge caps), an ordinary
  player plays a level better instead (SKILLED at most).
- **Fitting (CARD SCALING)**: the player counts as the SKILLED level plus their own record (skill, per save: TT tag
  TAG_SKILL, .ygo format 5, TCG's existing one). Target = player - favor - their level's give-away, favor +35 for
  ordinary players (about 55% for the player), -35 for special ones (45%), -70 for the CARD QUEEN (40%). TT:
  `Tt_FitOpponentHand` (tt_hands.c; other regions' cards when the home region's can't come close; TT favors since
  1.49.1 below), YGO:
  `Ygo_FitOpponent` (ygo_field.c; Game Boy decks up to the badge quarter, or the themed deck's closest tier), TCG:
  `Tcg_FitOpponent` (tcg_field.c). The challenge screen refits for the deck picked (TT keeps the hand under OPEN).
- **Triple Triad SAME / PLUS (1.49.1)**: the COMBO starts only from the cards SAME or PLUS took, and each wave only from
  the cards the last wave took (tt_duel.c; before, the placed card's own plain captures comboed too: one PLUS could
  flip most of the board). The AI has SAME and PLUS in mind for a move 75 / 55 / 35 / 15% of the time by level
  (tt_ai.c sSeesRules: otherwise it plans without them; the rules still apply to what it plays). Level give-aways are
  measured for four kinds of table (hidden / OPEN, with or without SAME / PLUS); under SAME / PLUS the player counts as
  CASUAL (people miss those sums far more than the AI levels do). Favors: special players -20, the CARD QUEEN -35. A
  special player whose gentlest hand with their five-star card is still well above the player leads with a four-star.
- **Triple Triad RANDOM**: both hands dealt fair: yours from your cards as close to their usual hand as 48 deals come,
  theirs from their region's cards as close to yours (`Tt_DealFair`, `Tt_DealRegion`).
- **Power ratings**, regenerated by tools (rerun after rules / AI / card changes):
  - TT `tools/tt/rate_cards.sh`: every card from 30000 AI games with random hands (a hand = its cards' average),
    pulled toward its stars; level give-aways hidden and OPEN apart (`gTtLevelRating`, `gTtLevelRatingOpen`);
    `rate_cards audit N` plays every region's rules at every badge tier, the CARD QUEEN and RANDOM, before and now:
    docs/audit_tt.txt.
  - YGO `tools/ygo/rate_cards.sh` (after rate_npc_decks.sh): every card's worth as three copies in the median and a
    strong Game Boy deck against the deck without them, blended with its Forbidden & Limited history (Stairway's 2003
    list and GOAT's 2005 explicit entries first, today's TCG/OCG/Traditional less); a deck's rating is read from its
    cards (`Ygo_DeckRating`, fitted to the Game Boy decks' measured ratings): docs/audit_ygo.txt.
  - TCG `tools/tcg/rate_cards.sh`: levels (src/tcg/tcg_levels.c) and every card's worth in the median Game Boy deck
    of its type (an Evolution with its stages; cards already there four times measured by taking them out), blended
    with the 1999-2000 tournament staples: docs/audit_tcg.txt. Decks themselves stay rated by AI duels
    (`TcgScale_RateDeck`, live for the hovered deck: about 30 ms, cached).
- **Shuffling**: every card game's duel seed is stirred with the clock (`Tcg_DuelSeed`; scripted tests keep theirs).
  EDOPro never shuffles a Deck itself: ygo_edo.c `Start` shuffles each from the seed (1.48.2).

## Triple Triad: HOENN's rules from the room host (1.49.2)

Online in a room someone else made, HOENN's Triple Triad rules are the host's: every region's rules and TRADE RULE,
where the CARD QUEEN is and since when (tt_integration.c, type 15 on the card games' relay, TCG_LINK_VERSION 14). The
host sends them to everyone whenever they change, every five seconds and at once to someone new (the collection is
loaded for it even if the case was never opened). A friend's collection holds the host's HOENN while in the room; its
own is kept aside and written to the save instead (`WorldForSave`), and comes back when the room or the host is gone.
What changes in a friend's game (a rule spread or abolished on a challenge, a CARD QUEEN favor, a paid journey) is
found by comparing with the host's last word and sent to the host, who takes it and passes it on. The CARD QUEEN only
moves by herself in the host's game (tt_field.c QueenDrift). Carried rules and GAMES PLAYED HERE stay each player's.
`mp tt world` prints HOENN as the game sees it; test tt_world.

## JOHTO and KANTO (1.60.0)
The plan is docs/REGIONS_PLAN.md; the content is Moemon Quetzal's (quetzal_rip/, capped to Gen 4: quetzal_rip/gen4),
brought in by tools/quetzal_port/ (import.py: maps, layouts, tilesets, scripts, constants; gen_*.py: items, sprites,
music, trainers, wild, statics, heal locations, town maps, menus, Gen 3 stand-ins, card look-alikes). Re-run
`python3 tools/quetzal_port/import.py --force` after changing the importer: its output is deterministic.
- Region of a map: its group (MP_Region_OfGroup: Johto 34-35, Kanto 36-39). src/mp/mp_region.c keeps flags
  0x1000-0x3FFF, vars 0x5000-0x52FF and struct RegionState in "<save>.region" (format 3: chunks for the other regions'
  PC boxes, the GLOBAL PC and the levels per region). The save itself is unchanged.
- Engine: FRLG-style 640-tile primaries (Tileset.isFrlg), 16-bit object graphics ids (OBJ_GFX_ID; Quetzal's sprites
  from 0x100 with palettes by tag, PALSLOT_QZ), 16-bit map sections (Johto's are MAPSEC_JOHTO_* 0x100+; a Pokémon met
  there has MET_GAME 14), Quetzal's script commands 0xE3-0xF5 (src/qz_port.c) and specials (src/qz_specials.c; the
  rest log and do nothing: src/qz_specials_stub.c), its own multichoice menus as MULTI_QZ_* (ids 117+).
- Travel: QzGetReachableRegionsMask / QzTravelToRegion (League beaten + S.S. TICKET; the MAGNET TRAIN with
  VAR_0x8007 = 0x7A), customs EventScript_RegionCustoms (data/scripts/mp_region.inc), MP_Region_Travel swaps the
  personal PC and fits levels (MP_Region_FitBoxMon: Lv 5 the first time); entering another region on foot does the
  same (MP_Region_OnMapLoaded). First trips go to MP_Region_GoToStartHouse.
- New game: PROF. BIRCH's region question (main_menu.c), MP_Region_StartNewGame; harness MOEMON_DEBUG_REGION=N.
- After a League (1.61.4): GameClear (post_battle_event_funcs.c) calls MP_Region_LeagueLevels: every Pokemon with a
  level record in that region takes its best EXP in every region it has been in (party, own boxes, GLOBAL PC, away
  PCs); a region it hasn't been to still starts it at Lv 5 (the user's rule). The credits end in credits.c
  Task_CreditsSoftReset: the save is loaded again (LoadGameSave, region sidecar too) and CB2_ContinueSavedGame, no
  title screen and no exit (co-op stays connected; only an unreadable save still resets). Tests region_chain_* from
  tools/test/gen_region_chain.py (checked by check_region_chain.py: badges, League, credits, customs keeping the Lv 70
  one, Lv 5 on arrival, the best level after the League, the GLOBAL PC linked only where cleared), region_hof*,
  hoenn_national_dex (HOENN's POKeDEX comes with the NATIONAL DEX; older saves get it on loading).
- Co-op with JOHTO's / KANTO's trainers (1.61.4): their ids are FIRST_QZ_TRAINER (864) on, past TRAINERS_COUNT, and
  every co-op check used to stop at TRAINERS_COUNT (no MPR_COOP_BATTLING: friends couldn't JOIN; no badge for a
  friend; trainer names / classes as TRAINER_NONE). IS_REAL_TRAINER (constants/trainers.h) / MP_IsRealTrainer take
  both ranges. Their classes are HOENN's numbers (LEADER 32, ELITE_FOUR 31, CHAMPION 38, RIVAL 50), so the joining
  rules (no E4 / Champion) and boss boosts hold; QZ_RIVAL / QZ_TEAM_ROCKET / QZ_ROCKET_ADMIN are story trainers too.
  Gym leaders: mp_coop.c sQzGymLeaders (both teams of each JOHTO leader, KANTO's eight: badge flag and the script that
  follows the first battle, for a friend who joined). gDisplayedStringBattle is 1024 (was 300: BROCK's 340-letter
  defeat speech overran it, harmless alone, a crash in co-op as it hit gBattleMsgDataPtr); the speech sent to friends
  (240 bytes, in the protocol) is cut at a whole paragraph by MP_TextFitLength. Tests join_gym_johto (FALKNER),
  join_gym_kanto (BROCK).
- Generations per region: HOENN Gen 1-3, JOHTO / KANTO Gen 1-2 (gMpGenSetting raises them; MP_Gen_Cap / Allowed /
  StandIn; Johto / Kanto tables swap to the Gen 2 cap: MP_Region_WildInfo / TrainerData, tools/quetzal_port/gen_gen2.py;
  evolutions into a later generation wait: GetEvolutionTargetSpecies).
- Town map / Fly: region_map.c sShownMap (QZMAP_*), data src/data/region_map/qz_region_maps.h.
- Card games: Tcg_LookGfx (Quetzal sprites -> Kai look-alikes), TT_REGION_JOHTO / KANTO, KAI_PEOPLE in import.py.
- Co-op: MPR_RULES' last byte is the host's region; a player elsewhere leaves (protocol 66).
- Items stay in their region (1.101.0, the user: "players cannot take items to other regions EXCEPT for items
  designated as being able to be brought, such as the Devices"): customs (qz_specials.c TravelNow, MP_RegionGateCross)
  calls mp_region.c MP_Region_SendLuggage: every BAG item outside the KEY ITEMS pocket goes to the GLOBAL PC's items
  (chunk "GITM", 512 slots of up to 999; what can't fit stays in the BAG), with EventScript_RegionLuggage's "luggage is
  far too heavy" line when there is any. Each region keeps its own item PC (pcItems swapped in MP_Region_Travel like
  the PC boxes, chunks "PI0 ".. "PI3 "). Opening the item PC (player_pc.c PlayerPC_ItemStorage) in a region whose
  LEAGUE is beaten hands the GLOBAL PC's items over, as many as fit (MP_Region_DeliverGlobalItems). Test
  region_luggage (harness `mp pcadd`, `mp pcqty`, `mp globalitems [deliver]`).
- Only the host leads the way (1.101.0, the user: "only the host can change regions; everyone is warped to the host,
  picks one Pokémon, and boards the ship"): qz_specials.c MP_RegionTripAllowed (FALSE for a friend in someone's room)
  stops the boats, the MAGNET TRAIN (EventScript_RegionCustoms) and the VICTORY ROAD gates (a step back, as when
  closed), and QzTravelToRegion / MP_RegionGateCross refuse too. When the host travels, each friend runs
  EventScript_RegionFollowHost (mp_main.c): where the host sails, the customs pick of the one POKéMON who comes (B: the
  first), then MP_RegionFollowNow: the same voyage to the same port. No room in the GLOBAL PC: they stay and leave the
  room (MP_Region_FollowFailed). The customs pick waits for the party menu now (`waitstate`; before, the region's
  number was taken as the party slot). Tests region_coop_follow (harness `mp tripok`, `waitfor partymenu`).
- Harness: mp gensetting N / regioninfo / regionclear R / regionreload / wildroll N / pcmenu / setvar ID V; waitfor multichoice.
- Tests: region_walk, region_fly, region_travel, region_globalpc, region_gens, region_starter, region_newgame,
  region_train, region_coop.
- Bug-Catching Contest (1.61.1): src/qz_bugcontest.c (the lead alone, 30 SAFARI BALLS, 8 minutes or
  MOEMON_BUGCONTEST_FRAMES, up to five catches, all kept; judged against four contestants; SAVE and autosave off while
  it runs; QzBugContest_FieldCheck in DoCB1_Overworld ends it). Test qz_bugcontest.
- RUINS OF ALPH puzzles (1.61.1): src/qz_alphpuzzle.c (6x6 board, 16 pieces cut from the species' front picture;
  MOEMON_ALPH_EASY). Test qz_alph_puzzle.
- LEAGUE credits (1.61.1): QzDoCredits / QzAfterCreditsReset; post_battle_event_funcs.c GameClear is region-aware
  (JOHTO / KANTO never set HOENN's FLAG_SYS_GAME_CLEAR; CONTINUE in NEW BARK / PALLET). Test region_hof.
- What may battle at GEN 3 in JOHTO / KANTO: never below Gen 3 (mp_region.c UseCap); the regions' Gen 2 cap is for
  their wild / trainer pools only.
- Later generations on the maps (1.61.2): the Gen 4 cap hid every Gen 5-9 overworld POKéMON with FLAG_QZ_GEN_CAPPED;
  tools/quetzal_port/gen_later_objects.py puts Quetzal's flags back and writes src/data/qz_later_objects.h, which
  MP_Gen_ObjectHidden (event_object_movement.c spawn checks) uses to hide each until its GENERATION. gen_static.py
  reads Kai's species list as it is now (216 statics). Rerun both after import.py. Test region_later_mons.
  Quetzal's KANTO statics each have their own availability condition (its KantoStaticEncountersRefresh switch);
  RefreshStatics only shows what isn't done, plus MELOETTA's (after the CHANSEY Dance, FLAG_QZ_0x2924).
- Graphics variables (1.61.2): Quetzal's scripts store its own graphics ids in VAR 0x4010 (VAR_QZ_0x4010), or 0x513E on
  VIRIDIAN / BIRTH ISLAND / FUCHSIA; event_data.c QzVarGraphicsId maps them through src/data/object_events/qz_gfx_ids.h
  (gen_objects.py). Sprites only scripts name come from tools/quetzal_rip/var_sprites.py ROM; they're imported last so
  older graphics ids never move.
- FireRed's `textcolor COLOR` (1.61.2) has its own handler that skips the color: as nop1 the color ran as a command
  (2 = end, 3 = return) and cut 41 KANTO scripts short (the S.S. ANNE captain, OAK's LAB...). check_scripts.py fails on
  any used command with arguments mapped to nop / nop1. Test qz_textcolor.
- MELOETTA's CHANSEY Dance (1.61.2): QzSevaultHouseEventEligibility (Quetzal's rule). Test qz_meloetta_dance.
- The DARKRAI dreams: CRESSELIA caught, DARKRAI not. JOHTO: the bed at home (QzDreamEventEligibility: 1 first time,
  2 when a beaten DARKRAI waits again; sleeping during the dream wakes you), FLAG_QZ_0x1425 = dreaming, DARKRAI in DARK
  CAVE's BLACKTHORN side, QzEndDreamEvent after its battle. KANTO (after its LEAGUE): the S.S. ANNE cabin's bed,
  FLAG_QZ_0x292B, DARKRAI in the room; QzSSAnneRoom6OnResume ends a dream whose DARKRAI is done. RefreshStatics shows
  DARKRAI only in its dream. Tests qz_dream, qz_dream_kanto.
- PALLET's bed: ARCEUS appears once to a full team at Lv 100 after KANTO's LEAGUE (QzPalletPostgameEventEligibility).
- The S.S. ANNE's truck: a Lv 100 NIDOKING leading with STRENGTH after KANTO's LEAGUE, MEW not caught
  (QzSSAnneExteriorEventCheck); ShiftTruck moves its metatiles (Quetzal's own: layout columns 55-57, rows 1-3);
  FLAG_QZ_0x2931 keeps it moved; MEW's static shows only then. Test qz_mew_truck.
- HEADBUTT and ROCK SMASH: Quetzal's 5-slot ROCK SMASH table (src/data/wild_headbutt_qz.h; its maps' own
  rockSmashMonsInfo are empty) feeds ROCK SMASH where a map has no rock table and HEADBUTT on trees: A facing a tree
  while a party POKeMON knows HEADBUTT (field_control_avatar.c -> EventScript_QzHeadbuttTree; wild_encounter.c
  MP_Headbutt_IsTree / MP_HeadbuttWildEncounter; slots 50/15/15/10/10, the table's rate as a percent, species the
  GENERATION shows). Trees: tools/quetzal_port/gen_headbutt_trees.py (per outdoor map, its unwalkable tree-colored
  metatiles; --show draws them). Test qz_headbutt.
- Badge marts: Quetzal's `pokemart 0` sells one of nine lists by the region's badge count (src/data/qz_badge_marts.h
  from quetzal_rip/data/badge_marts.json: tools/quetzal_rip/badge_marts.py ROM, tools/quetzal_port/gen_badge_marts.py;
  shop.c MP_ShopItems takes NULL as that). It crashed every JOHTO mart clerk before 1.62.
- The cuttable trees and breakable rocks get Emerald's obstacle animations (gen_objects.py OBSTACLE_ANIMS: JOHTO's
  82 / 86, KANTO's FireRed 16127 / 16190): with plain still animations CUT and ROCK SMASH crashed. FireRed's nurse
  (16170) keeps her 10th frame and Emerald's nurse animation: KANTO's nurses bow (nurse_joy_bow), and without the
  frame the heal script waited forever (test qz_nurse_kanto).
- In-game trades in JOHTO / KANTO are Quetzal's 17 (src/data/qz_ingame_trades.h from tools/quetzal_port/gen_trades.py,
  trade.c InGameTrade); Emerald's table of four was read past its end and crashed. Test qz_trade. The trade screen's
  VBlank no longer touches its freed data (every in-game trade could crash on a PC).
- A shop item without a price (KURT's Apricorn balls had none) no longer divides by zero; they cost 300 (DREAM BALL
  and METEORITE 1000).
- Borders: KANTO's seven FireRed layouts with 3x2 borders (VIRIDIAN FOREST, the SAFARI ZONE, BERRY FOREST, PATTERN
  BUSH) repeat them 3 wide (fieldmap.c GetBorderBlockAt); every other map's border is 2x2.
- Doors: KANTO's are FireRed's (src/data/frlg_doors.h). JOHTO's are Quetzal's own (1.83.0): its door table at ROM
  0x952148 (FireRed's format, 19 one-metatile doors, read next to Emerald's at 0x951EC0) plus FireRed's tall doors at
  0x952358, ripped by tools/quetzal_rip/rip_doors.py QUETZAL_ROM into quetzal_rip/doors/ and src/data/qz_doors.h;
  mp_frlgdoor.c Find looks a JOHTO door up there by metatile, and a door tile it doesn't list opens without an
  animation, as in Quetzal (the frames once made from the closed door, all rising, are gone). Test johto_doors.
- Triple Triad sellers in JOHTO / KANTO stock what MAUVILLE's / LILYCOVE's do (Tt_PoolOf, as their players' hands):
  gTtSellerStock has HOENN's eight shelves only, and the seller crashed on the missing ones.
- Quetzal's system flags (FireRed's, from 0xA50) that the engine reads are Emerald's own (constants/qz_port.h, from
  import.py QZ_SYS_FLAGS): 0xA78 POKéMON received, 0xA79 POKéDEX, 0xAA1 STRENGTH in use, 0xAD8 RUNNING SHOES. They
  sat in the region flags (0x2A05/06/08/0A), so a JOHTO / KANTO new game had no POKéMON / POKéDEX in the START menu
  and couldn't run; mp_region.c MoveQuetzalSystemFlags moves a save's old copies (test story_johto checks them).
- STRENGTH: JOHTO's / KANTO's boulders are Quetzal's sprites (OBJ_EVENT_GFX_QZ_87, QZ_16186); fldeff_rocksmash.c
  MP_ObstacleGfxIs counts them (and Quetzal's rocks and trees) as Emerald's for the push and the party menu's field
  moves. FireRed's boulder switches (step triggers waiting for 99: VICTORY ROAD, the TANOBY KEY) run when a pushed
  boulder stops on them (field_control_avatar.c TryRunBoulderSwitchScript, from FireRed's
  HandleBoulderActivateVictoryRoadSwitch). Test qz_strength (harness mp objat X Y, mp boulderswitch X Y).
- GOLDENROD's department store elevator was Quetzal's own code (its panel had no script, and each floor's door warp
  sat a row above the door): EventScript_GoldenrodElevator + qz_specials.c QzGoldenrodElevatorMenu / Go list B1F-6F
  and set the elevator's way out; the floors' warps are on the doors (9, 4), B1F got one at (10, 6). Quetzal's
  arrival scene (VAR 0x5015 = 1) isn't used. Test qz_goldenrod_elevator.
- Ripped scripts that call Quetzal's own code by ROM address (callnative / gotonative 0x08xxxxxx: ICE PATH's palette
  save, FIVE ISLAND's quest-log save, BILL's teleporter wait, GOLDENROD's elevator arrival) are skipped (scrcmd.c
  IsGbaRomAddress): they crashed the game. Quetzal's movement type 0x64 (the DANCE THEATER leader's spin) is
  ROTATE_COUNTERCLOCKWISE; any other type past Emerald's table stands (scrcmd.c, and object spawns guard it): the
  theater crashed on every visit after its scene. Test qz_dance_theater.
- Story triggers: tools/quetzal_port/story_vars.py (run by regress.sh) checks every JOHTO / KANTO step trigger and frame
  script waits for a value some script or the code sets (KNOWN lists the ones that rightly don't, checked against
  the ROM); tools/test/story_sweep.sh fires each trigger in the game (its variable set, the player stepping on it)
  and reports any that hold the player, don't start or don't move their variable on (harness mp sweepmark).
- JOHTO's TEAM ROCKET HIDEOUT B2F: LANCE fights beside the player against ARIANA and a grunt. Quetzal's special battle
  12 (battle_tower.c DoSpecialTrainerBattle: a multi battle with an in-game partner, TRAINER_QZ_PARTNER standing in
  for any Quetzal partner: its name, class and picture come from gTrainers[sQzPartnerTrainer]; the partner's team is
  built in party slots 3-5 by MP_CreateTrainerPartyAt). QzBackupPartyForOptionBattle is Emerald's
  ChooseHalfPartyForBattle (pick 1-3). Quetzal's script never calls LoadPlayerParty after the battle, so the battle's
  end (HandleSpecialTrainerBattleEnd case 12) puts the picked Pokemon back in their own slots and the rest of the party
  with them, as Emerald's STEVEN script does with frontier_saveparty + LoadPlayerParty. Test story_rocket_lance.
  (The test-only MOEMON_TEST_FOE_HP1 used to write six Pokemon from the second trainer's slot 3, past the enemy party
  into the player's: it now stops at the trainer's own team.)
  LANCE is shown from behind like STEVEN (MP_PartnerShowsBack / MP_PartnerBackPic in battle_tower.c: a partner
  listed in sQzPartnerLooks gets its LOOK's back picture, mp_avatar.c MP_Avatar_NamedBackPic, in the Triple Battle
  second ally's slot, free when no friend is in the battle); any other partner keeps the Frontier layout.
- (1.68.1: it never begins any more, the user's "Quetzal's fan additions aren't wanted": QzGiovanniStoryCanStart is
  always FALSE and GIOVANNI isn't at TOHJO FALLS; a save already in it, VAR 0x506C set and 0x1319 not, can still see
  it through by TOHJO's radio: GiovanniStoryBegun; test story_giovanni. What follows is how it works.)
- GIOVANNI's alternate past (JOHTO postgame; qz_specials.c, Quetzal's specials 693-703 and 792 read from its ROM -
  note Emerald's order FlagSet 0x80E4788, FlagToggle, FlagClear 0x80E47D8, FlagGet 0x80E4800; earlier notes had Set /
  Clear swapped): GIOVANNI at TOHJO FALLS asks a player in Team Rocket's colors (mp_avatar.c
  MP_Avatar_LocalIsTeamRocket: the GIOVANNI, ROCKET GRUNT, JESSIE and JAMES LOOKs, as Quetzal asks for its Rocket
  characters; the LOOK isn't changed) with CELEBI leading (its ILEX FOREST encounter done, 0x11FA), JOHTO's League beaten,
  no dream and no friends in the room. FLAG 0x1318 = in the past, 0x1319 = done, VAR 0x506C = the step (5, 1 after the
  RADIO TOWER battle, 2 after home, 3 after SILVER, 4 done), 0x5076 = the scene to play. Special battles 13 (beside
  GIOVANNI against ETHAN) and 14 (beside SILVER against GIOVANNI) are multi battles against ONE trainer: Emerald's
  two-opponent multi with the same trainer in both slots (battle_main.c MP_QzSoloMulti: the whole team, the back half in
  the second slot; one picture, one intro line, one defeat and prize); partners from the LOOKs' back pictures; the
  party comes back after (HandleSpecialTrainerBattleEnd). 693 refreshes who stands where on every JOHTO map load.
  The ARCEUS chamber's item (700) wants six level 100 legendaries and every other JOHTO / KANTO legendary caught or beaten
  at the GENERATION. Tests story_giovanni (the whole story), story_giovanni_no. Test-only MOEMON_TEST_FOE_WEAK: foes'
  Attack / Sp. Atk at 1 too.
- BILL's time travel (Quetzal's JOHTO postgame trip to a "possible future" playing as CELEBI, after giving BILL a
  MAGEARNA) is removed, the user's call: BILL's house keeps its people and BILL his ROTOM gift and chat (his
  time-travel menu, the passage object and its scenes are gone), GOLDENROD lost its phone call about it, and Quetzal's
  ten QzTimeTravel* specials and johto_shared.inc's time-travel scripts are gone (data/qz_specials.inc too). If
  tools/quetzal_port/import.py runs again they come back: remove them again. Test qz_bill_house.
- TEAM PADDING (FILL) and the GENERATION: the extra Moemon (mp_coop.c PickFillerSpecies, at a co-op battle's start or
  when a friend joins one) is never past the GENERATION or the region's cap (MP_Gen_SealedSpecies / MP_Gen_Allowed);
  a player saw REUNICLUS at GEN 3. Level scaling's trainer evolutions stop before a sealed species too
  (mp_levelscale.c MP_LevelScale_Evolve). Test coop_fill_gen3 (tools/test/check_fill_gen.py reads the padded team).
- A special that is done when it returns (a JOHTO / KANTO special with nothing to wait for, every stub) calls
  ScriptContext_Enable at once; Emerald's waitstate right after it then stopped the script for good (S.S. ANNE's
  departure froze the player on the dock). ScrCmd_special / specialvar note when the special enabled the script
  (script.c gMpScriptEnables) and a waitstate that is the very next command goes on (scrcmd.c). Stubs answer
  VAR_RESULT 0, the "not available" path of every caller. Test story_ssanne.
- KANTO's TRAINER TOWER records sign: FireRed's ShowLinkBattleRecords with VAR_0x8004 = 1 is its tower's records
  screen; Emerald's TRAINER HILL records stand in (battle_records.c; test qz_tower_records). FireRed's list kept open
  after a choice (CERULEAN's badge describer) took no input once drawn again: QzReturnToListMenu now also calls
  ScrollableMultichoice_TryReturnToList (test qz_list_return). The NPC sweep's menus that ignore B (CELADON's
  department store clerks, the condominium roof's notice) close with their EXIT row, as in Quetzal.
- SAFFRON's MAGNET TRAIN counter: an attendant by the POKeMON CENTER (data/maps/Kanto_SaffronCity/map.json, added
  after the import: put it back if import.py runs again) runs EventScript_MagnetTrainSaffron to GOLDENROD once KANTO's
  League is beaten (QzTravelToRegion's train lands at the other city's POKeMON CENTER). Test region_train_saffron.
- The host's trip takes the room along: MPR_RULES carry the host's region as the trip sets off (gRegionState.current;
  the warp lands later); a friend already in the room who hears a new region from the same host follows when free
  (mp_main.c sFollowRegion -> qz_specials.c MP_Region_FollowHost: customs keeps their first POKeMON, the port or the
  first town the first time); with no room in the GLOBAL PC they leave as before. Harness `mp travel R`. Test
  region_coop_follow.
- A region's first arrival (its new game or the first trip there) runs Quetzal's setup: the people and objects that
  start hidden and a few story variables, then its guard flag (JOHTO 0x1422, KANTO 0x2269), plus a flag on every
  arrival (mp_region.c MP_Region_Arrive; data src/data/qz_region_init.h from tools/quetzal_rip/region_init.py ROM ->
  quetzal_rip/data/region_init.json -> tools/quetzal_port/gen_region_init.py). Saves already under way aren't touched.
- QzGiovanniStoryUpdateFlags (Quetzal's special 693) runs on every JOHTO warp (MP_Region_OnMapLoaded), as Quetzal's
  map loading does: it keeps GIOVANNI's-story people and ELM's lab's ARCEUS hidden (they stood there from 1.60).
- Not ported yet: Quetzal's per-legendary unlock conditions (special 704's kind switch at 0x082AA38C, 45 kinds;
  KANTO's at 0x082AE79C): RefreshStatics shows every one not yet caught or beaten after the LEAGUE (save MELOETTA,
  DARKRAI, MEW's; 1.61.6: PreLeagueLegend's before it).
- Cross-region trips: tools/test/gen_region_trips.py writes region_trip_<from>_<to> (all six, there and back by the
  real clerks). story_johto plays JOHTO's opening (house, MOM, ELM, the starter); harness mp getvar.
- tools/test/npc_sweep.sh talks to every person and reads every sign on every map (harness `mp sweepnote`); it
  reports crashes, hangs, scripts still holding the player and stubs reached.
- Not done yet: FRLG door animations, 3x2 map
  borders, the sightseeing walk on a first arrival, everyone following the host on a trip,
  per-region co-op shared PCs. The plan for what's left: docs/HANDOFF_REGIONS_GENS.md.

## Gens 5-9 and GENERATION (1.61.0)
- Cries (1.61.5): PlayCryInternal reads any of the cry tables' 128-entry blocks (it stopped at block 3, so cry ids
  512 and up, most of Gens 5-9, played nothing; MP_CRY_COUNT from the table labels; `mp crycheck`, test cry_all).
  The Gen 4-9 cries are their newest (tools/moemon_cries/fetch_cries.py from github.com/PokeAPI/cries
  latest/<id>.ogg, mapped by the cry file names; gba_cries.py: silence trimmed, 80 Hz - 4.4 kHz, 10512 Hz like
  Gen 3, the loud part's RMS matched to the Gen 3 cries' median with soft peaks, 8-bit; the build packs them with
  wav2agb -c). Rerun both after adding a species' cry.
Pokémon 494-1025 (species 519-1050: VICTINI..PECHARUNT; the EGG is 1051 and Unown's letters follow), moves 468-847,
abilities 125-311, items 508-552 and the FAIRY type (18), from pokeemerald-expansion's data and Moemon Quetzal's art.
Old saves need nothing: eggs and Unown keep their real species in the save.
- Data generators: tools/moemon_gen58/ (moves: expansion_moves.py -> moves.json, gen_moves.py; per-move effect data
  gen_effect_data.py -> src/data/battle_moves_gen59_data.h; effect scripts gen_effect_scripts.py ->
  data/battle_scripts_gen58.inc) and tools/moemon_gen59/ (gen_species.py --expansion DIR [--rom QUETZAL]:
  species, learnsets, evolutions, dex, cries, graphics; gen_abilities.py, gen_items.py, gen_dex_orders.py).
  Gen 8-9 sprites have one frame in the expansion: gen_species.py stacks it twice for anim_front.png.
- Sizes past the old limits: abilities are u16 everywhere (BattlePokemon, SpeciesInfo, battle message buffers:
  PREPARE_ABILITY_BUFFER writes two bytes), level-up learnsets are u32 (LEVEL_UP_MOVE(lvl, move) = lvl << 16 | move).
  The HALL OF FAME's 9-bit species keep their high bits in the save's free space after the 50 teams (hall_of_fame.c
  HofSpecies; zero in old saves), and a co-op Hall of Fame team is 126 bytes.
- Battles (src/battle_gen59.c, include/battle_gen59.h): Gen 4's hooks call in (MP_Gen59_MovePower, AttackStats,
  FinalDamage, MoveType, IsPhysical, the field end of turn, arrival, move end). Most moves are data: gGen59Moves holds
  their stat changes on the user and target (with chances) and secondary status, worked through by the generic
  scripts (BattleScript_G59StatusMove / G59Hit / G59TwoTurns / G59Heal* / G59PayHpStats...) with `gen4 G59_*` modes.
  Per-effect code: power formulas (HEAVY SLAM, ELECTRO BALL, STORED POWER, HEX, ACROBATICS, RAGE FIST, LAST
  RESPECTS...), PSYSHOCK / FOUL PLAY / BODY PRESS stats, always-crit moves, terrains (5 turns; boosts, GRASSY healing,
  MISTY / ELECTRIC blocking status), WONDER / MAGIC ROOM, AURORA VEIL, STICKY WEB, type and ability moves, healing
  moves, SMACK DOWN, SALT CURE... Messages are gGen59StringIds (STRINGID_G59_* after Gen 4's, printfromtable, so
  co-op screens print the same).
- Abilities: the SURGES, INTREPID SWORD / DAUNTLESS SHIELD on arrival (MP_Gen59_SwitchInAbility), MOXIE / BEAST BOOST,
  PRANKSTER / GALE WINGS / TRIAGE priority (MP_Gen59_Priority in GetWhoStrikesFirst), SAND RUSH / SLUSH RUSH / SURGE
  SURFER, PROTEAN / LIBERO, the -ATE abilities, STRONG JAW / MEGA LAUNCHER / SHARPNESS, TOUGH CLAWS, SHEER FORCE,
  MULTISCALE, FLUFFY, REGENERATOR, IRON BARBS... Items: ASSAULT VEST, ROCKY HELMET, AIR BALLOON, WEAKNESS POLICY,
  HEAVY-DUTY BOOTS, ABSORB BULB / CELL BATTERY / SNOWBALL, PIXIE PLATE, FAIRY FEATHER, FLOAT STONE.
- Followers: Quetzal's overworld sprites for 387-1025 (tools/moemon_gen4/gen_followers.py; OGERPON has none).
- GENERATION: OPTIONS > BATTLE > GENERATION and HOST A GAME > GAME RULES (gMpConfig.generation 3-9, "generation" in
  the config; online the host's: MPR_RULES' last byte, protocol 67). MP_Gen_Cap: HOENN shows up to it, JOHTO / KANTO
  Gen 1-2 at 3, up to it from 4. From 5, JOHTO / KANTO wild slots and trainers' Pokémon are Quetzal's own where their
  generation is allowed, the Gen 4 cap's elsewhere (tools/quetzal_port/gen_modern.py -> src/data/wild_qz_modern.h,
  trainer_parties_qz_modern.h). FAIRY exists from Gen 6 (MP_Gen_SpeciesType / MoveType; Kai's older FAIRY Pokémon
  take it from 6). Moves and abilities past the cap don't work (greyed in CheckMoveLimitations; MP_Gen_FitBattler),
  down to Gen 3, the default (the user's rule: the restrictions reach Gen 3 whatever a save has). A party Pokémon whose species, a move or its ability is past
  the cap is SEALED: it can't battle (MP_Gen_SealedMon / MP_SEALED beside the battle code's EGG checks; the party menu
  says SEALED in its description slot, PARTYBOX_DESC_SEALED). Marks: a sealed species' icon uses a gray copy of its
  palette in the party menu and the PC (pokemon_icon.c MP_GrayIconPaletteTag, tags 0x5C40+; the PC's own icon
  palettes in CreateMonIconSprite), and its summary shows SEALED in red where the number goes (test seal_marks).
  Items have generations too (MP_Gen_OfItem; 0 = Kai's own and the story items): held ones past the cap go
  back to the bag when it drops (MP_Gen_ReturnHeldItems on map loads and OPTIONS), and in the bag they can't be used,
  given or used from a registered button (item_menu.c ItemLaterGen). Not done yet: sealed Pokémon in trades and
  online.
- Moves past the cap are skipped when learning (MonTryLearningNewMove, GiveBoxMonInitialMoveset, the relearner,
  CanMonLearnTMTutor, day care egg moves); evolutions into a later generation's species wait.
- OPTIONS > DISPLAY > SPRITE SCALING (a page, option_menu.c sSpritePage): FOLLOWERS and GARDEN MOéMON, each on or off
  (gMpConfig.scaleFollowers / scaleGarden, ini scale_followers / scale_garden; on by default). Only the PMD pictures
  (the game's own follower sprites are drawn to a trainer's scale already; the user's call): a POKéMON shorter than a
  trainer (MP_TRAINER_HEIGHT_DM 15, drawn MP_TRAINER_HEIGHT_PX 21 tall: Brendan standing) comes down toward its POKéDEX
  height beside one, never below just shorter than the player (MP_SCALE_MIN_PX 18: smaller was too small), measured
  against its PMD picture's drawn height, worked out once a POKéMON (mp_spritescale.c MP_SpriteScale_Mon); one as tall
  or taller keeps its picture (ALAKAZAM, ONIX).
  Any sprite can be drawn smaller: struct Sprite mpScale (Q8), noted per OAM entry with the point it shrinks around (the
  bottom middle of all its entries: mp_bigsprite.c MP_BigSprite_NoteScale), drawn by gba_easy_draw.c DrawScaledEntry:
  each pixel the average of the ones it covers, kept where half of it is covered (no dropped pixels), for ordinary
  and big (PMD) sprites; not with affine ones. `mp spritescale F G`; test follow_scale.
- The POKéDEX's AREA (pokedex_area_screen.c): every region's wild tables as the game uses them (mp_region.c
  MP_Region_WildInfoIn: a table seen as its region shows it, the GENERATION's newer slots, JOHTO's / KANTO's own tables,
  no starters), JOHTO's / KANTO's HEADBUTT trees (wild_encounter.c MP_HeadbuttTable), on HOENN's, JOHTO's, KANTO's or a
  Sevii Islands map (region_map.c MP_RegionMap_AreaShow / AreaGfx: the town maps' art, their 64x64 affine map's top-left
  32x32 as a text map; areas not in a map's layout get a marker at their box). It opens on the player's region if the
  species is there, else the first map with it; SELECT / UP / DOWN go to the next. Before, it read Emerald's own tables
  only (nothing past 386, nor JOHTO / KANTO), and its glow was built against whatever map the Town Map showed last.
  `mp dexorder MODE ORDER`; test dex_area.
- No starters in the wild (the user's call: five in an hour was too many): a starter line's slot in any wild table
  (Quetzal's JOHTO / KANTO tables, the newer generations' slots) gives the nearest earlier slot's POKéMON at its own
  levels (mp_region.c sStarterLines, MP_WildSlotSpecies; wild_encounter.c's land, water, ROCK SMASH, fishing and
  extra rolls). They come from the daily Outbreak instead: one day in four is a starter's (mp_events.c
  sOutbreakStarters, the generations HOENN's cap allows), and from the MoeGatcha. obtain_audit.py knows both rules.
  Test wild_no_starters.
- HOENN's pools by GENERATION (tools/moemon_gen59/gen_hoenn_modern.py -> src/data/hoenn_modern.h, applied in
  mp_region.c): each wild table gives one of its rarer slots to each generation 4-9 (1.62.1: the slots of generations
  past the cap go, in turn, to the species of those within it, so newer POKéMON are about 19% of ROUTE 104 at GEN 4
  as at GEN 9; `mp wildroll N` counts them) (a newer Pokémon sharing a type,
  the same evolution stage, never a legendary; pseudo-legendary lines only where the slot is Lv. 30+); about half of
  trainers' team members get a newer stand-in from a generation of their own (crc32-picked), bosses keep their ace.
  JOHTO / KANTO use Quetzal's own modern tables.
- Abilities (battle_gen59.c): arrival (MP_Gen59_SwitchInAbility, ArrivalForm), hits taken / given (the MoveEnd
  chain: TargetAbilityOnHit / AttackerAbilityOnHit, then MoveEndMore: CHEEK POUCH, GULP MISSILE's catch, DANCER,
  OPPORTUNIST, EMERGENCY EXIT / WIMP OUT), the end of turn (MP_Gen59_EndTurnAbility: MOODY, HEALER, HARVEST, CUD CHEW,
  SCHOOLING / SHIELDS DOWN, HUNGER SWITCH), moves taken in (MP_Gen59_AbsorbAbility), moves kept off
  (MP_Gen59_BlockMove: BULLETPROOF, OVERCOAT, GOOD AS GOLD, MAGIC BOUNCE as a block...), accuracy, status blocks
  (MP_Gen59_StatusBlocked, now called from SetMoveEffect), stat drops (MP_Gen59_OnStatLowered: DEFIANT...) and rises
  (MP_Gen59_OnStatRaised: OPPORTUNIST). MP_Gen4_MoveEnd calls MP_Gen59_MoveEnd until it returns NULL (each step sets
  its own G59M_* bit). Form changes are stat recomputes from a form's base stats (sForms: AEGISLASH, WISHIWASHI,
  PALAFIN, MINIOR, TERAPAGOS), not species changes; ZERO TO HERO remembers the party slots that left
  (gGen59Field.heroSlots). Approximations: PARENTAL BOND = one hit at x1.25, MAGIC BOUNCE blocks instead of
  reflecting, the primal weathers are lasting rain / sun, DANCER copies the stat dances only, ILLUSION does nothing.
  Tests gen59_forms, gen59_exit, gen59_dancer.
- Items: marts sell the newer battle / evolution items by GENERATION (shop.c MP_ShopItems), PICKUP's 95-97 rolls
  give an allowed newer item. Shops (1.61.5): JOHTO sells Pokemon Crystal's lists (GOLDENROD's clerks and the town
  marts) and KANTO FireRed / LeafGreen's (CELADON, the towns, the SEVII ISLANDS) as Kai's items (sMpBaseLists; nothing
  those games had that Kai doesn't, no older item they didn't sell), plus only the GEN 4+ items of the clerk's own
  Quetzal list (MP_Gen_OfItem); a clerk Quetzal added sells only those (empty at GEN 3: just CANCEL). Clerks are known
  by the map and the first item of their Quetzal list (MP_REGION_LIST for `pokemart 0`). The newer items are spread
  over the department stores' clerks by theme and over the town marts by place (sMpShopExtras: MP_EXTRA per clerk,
  MP_TOWN per mart), each from a badge count of that region. `mp shopdump`, `mp givebadges REGION`; tests
  shop_lists_gen3 / gen9 (tools/test/check_shop_lists.py).
- Everything obtainable at each GENERATION (1.61.3): tools/moemon_gen59/obtain_audit.py models every source
  (each region's wild tables as mp_region.c picks them, HEADBUTT, statics, the scripts' gifts / wild battles / eggs,
  starters, in-game trades, the Bug-Catching Contest, MoeGatcha's fixed prizes, evolutions, breeding) and
  item_audit.py every item's (marts and shop.c's lists, field and hidden items, gifts, PICKUP, MoeGatcha, held items);
  regress.sh fails if a POKéMON of the GENERATION's NATIONAL DEX or an item of its generation has no source (the
  reports land in the result folder; docs/audit_obtain.txt / audit_items.txt are copies). What filled the gaps:
  HOENN at GEN 3 has Ruby / Sapphire's SURSKIT (ROUTE 102), ROSELIA (117), ZANGOOSE (114), LUNATONE (METEOR FALLS)
  and MEDITITE (MT. PYRE) in rare slots (wild_encounters.json); the paradox POKéMON have a GEN 9 slot each
  (gen_hoenn_modern.py PARADOX); MANAPHY's eggs are PHIONE (daycare.c); SYLVEON is EEVEE with BABY-DOLL EYES (Lv 15 from
  GEN 6) and high friendship (pokemon.c EVO_MOVE; checked last, so it wins over ESPEON / UMBREON as in Gen 8);
  MILCERY takes any sweet, MELTAN evolves at 40, FARFETCH'D holding a STICK and MR. MIME holding NEVER-MELT ICE level
  into SIRFETCH'D / MR. RIME (gen_species.py EXTRA_EVOS: Kai has no GALARIAN forms); the LINKING CORD does every
  trade evolution (holding the trade item where one is needed) and is a Gen 3 item, so a lone player gets ALAKAZAM,
  GENGAR, STEELIX...; the forms Quetzal's statics use (GALARIAN birds, ZARUDE, MAGEARNA, ZYGARDE, HOOPA) are the species
  from the form's generation (gen_static.py, QzStatic.gen). Items: UP-GRADE and the two scrolls at the evolution
  counters, DREAM / FAB / RETRO MAIL at LILYCOVE's 2F, RARE BONE / ODD KEYSTONE / GIMMIGHOUL COIN by PICKUP, SHUCKLE's
  rare held item is BERRY JUICE. Test evo_ways (harness mp evotarget SLOT [ITEM], mp friendship SLOT N).
- The NATIONAL DEX holds the GENERATION's POKéMON (MP_Gen_DexCount: 386 at GEN 3, 493, 649, 721, 809, 905, 1025):
  pokedex.c's lists in every order, GetNationalPokedexCount (seen / caught counts everywhere) and HasAllMons (the
  diploma: every POKéMON past Gen 3 to the GENERATION too). Test dex_gen_limit (harness mp dexcount).
- Harness: MOEMON_BATTLETEXT=1 logs every battle message ("btext: ..."); MOEMON_GENERATION=N; mp gensetting N.
- Tests: gen59_moves, gen_seal, region_gens (the harness runs scenarios at GEN 9 unless their conf sets GEN).
- Harness: mp gencheck ITEM / genrefresh.

## The battle GIMMICKS (1.61.0)
MEGA EVOLUTION, Z-MOVES, DYNAMAX and TERASTALLIZATION (include/battle_gimmick.h; the battle side at the end of
src/battle_gen59.c, settings / pictures / menu text / AI in src/battle_gimmick.c).
- Settings: gMpConfig.gimmicks, 2 bits each (GIMMICK_SET_AUTO / ON / OFF; AUTO = on from its generation: 6, 7, 8, 9),
  "gimmicks" in the config; OPTIONS > GAME rows MEGA EVOLUTION / Z-MOVES / DYNAMAX / TERASTALLIZE; HOST A GAME > GAME
  RULES > MEGA EVOLUTION / Z-MOVES / DYNAMAX / TERASTALLIZE rows (the rules list scrolls: RULE_VISIBLE); online the host's (MPR_RULES' last byte, protocol 68).
  MP_Gimmick_Setting / MP_Gimmick_On. Tests: MOEMON_GIMMICKS=dddd (MEGA Z DYNAMAX TERA, 0 AUTO 1 ON 2 OFF).
- One gimmick a battle per trainer (Owner(): the side, or the battler in multi / co-op / triple / two-opponent
  battles). The engine sends what a battler may use in the move menu's byte 3 (BtlController_EmitChooseMove:
  GIMMICK_AVAIL_*, the Z-CRYSTAL's type in bits 3-7); START in the move list cycles MEGA, Z-MOVE (a damaging move of
  the crystal's type), DYNAMAX, TERA, none (battle_controller_player.c MpGimmickNext; the type line shows the choice,
  or "START: ..." every other 90 frames); the choice rides in the move byte's bits 4-6 (GIMMICK_MOVE_SHIFT), stripped
  in HandleTurnActionSelectionState into MP_Gimmick_Choose. The opponent controller asks MP_Gimmick_AiPick (MEGA at
  once, the others on the trainer's last Pokémon).
- MP_Gimmick_TurnStart (from CheckFocusPunch_ClearVarsBeforeTurnStarts, in the turn order, before any move) checks
  again and applies: MEGA (the form's types, ability, stats from its base stats; then its arrival ability through
  AbilityBattleEffects ON_SWITCHIN), TERA (the species' first type), DYNAMAX (3 turns, counted down in the field's
  end of turn: GimmickEndTurn), Z (Z-POWER on this turn's move). MEGA and TERA stay with the Pokémon by side and party
  slot (sG.megaOf / teraOf, reapplied in MP_Gimmick_SwitchIn); DYNAMAX ends on a switch.
- Effects: MP_Gimmick_Stab (TERA: 2x on its type when it was one of the Pokémon's own, the old STAB kept), 
  MP_Gimmick_MovePower (Z power and MAX power tables from the base power; TERA moves under 60 become 60),
  MP_Gen59_FinalDamage halves damage to a DYNAMAX Pokémon (instead of doubling its HP: nothing in the party
  changes), flinch blocked, status moves become PROTECT (MP_Gimmick_MoveUsed in HandleAction_UseMove), MAX MOVES'
  effects by type at the move's end (GimmickMoveEnd: weather, terrain, the side's / foes' stat).
- MEGA forms aren't species: src/data/mp_megas.h (tools/moemon_gimmicks/gen_megas.py) has each form's base species,
  stone (ITEM_NONE: RAYQUAZA with DRAGON ASCENT; KYOGRE / GROUDON: Emerald's BLUE / RED ORB key items in the bag),
  types, ability, stats, y offsets and pictures (graphics/moemon/mega: Moemon Quetzal's 50 MEGA / primal forms
  899-948; stats from pokeemerald-expansion; cache tools/moemon_gimmicks/megas.json).
- Pictures on every screen: the battle scripts play B_ANIM_MP_GIMMICK (always, behind a SUBSTITUTE too) with
  kind << 8 | value; AnimTask_MpGimmick flashes the Pokémon and loads the MEGA picture (all 4 frames of the buffer),
  or tints TERA (a color per type) / DYNAMAX (red), and remembers it by side and party slot (sPicMega / sPicTera /
  sPicDyna). MP_Gimmick_AfterMonGfx (end of BattleLoadOpponent/PlayerMonSpriteGfx) puts them back whenever the
  Pokémon is drawn again; MP_Gimmick_YDelta gives a MEGA picture its own height (GetBattlerYDelta).
- Items 553-617: 47 MEGA STONES (CHARIZARDITE X / Y are CHARZARDITE X / Y: 13 letters) and 18 Z-CRYSTALS (type
  order, MYSTERY skipped), tools/moemon_gen59/gen_items.py MEGA / ZCRYSTALS; generation 0 (only their gimmick's
  setting counts: MP_Gimmick_ItemAllowed); sold at LILYCOVE 3F, GOLDENROD 5F, CELADON 5F from 6 badges (shop.c sMpMegaStones / sMpZCrystals).
- Tests: gimmick_mega, gimmick_dynamax, gimmick_z, gimmick_ai.
## Save layout and migration (1.60.1)
The save (SaveBlock1/2, storage) must keep the same bytes on Linux and Windows and from version to version. 1.60.0 added
a byte to struct ObjectEvent, which MinGW's bitfield layout grew to 40 bytes: Windows saves moved every later field by 64
(0 badges, a divide by zero in DEWFORD's trends). Since 1.60.1:
- Graphics ids past 0xFF live in spare bits: ObjectEvent `graphicsIdHi:4` in the first bitfield word (ids up to 0xFFF),
  ObjectEventTemplate {localId, graphicsIdLo, kind, graphicsIdHi}. Use OBJ_GFX_ID / SET_OBJ_GFX_ID / TEMPLATE_GFX_ID.
- include/mp_savefix.h, src/mp/mp_savefix.c: the save format is the "<save>.region" sidecar's 'SAVF' chunk (0 = no
  sidecar, up to 1.59; 1 = 1.60.0; 2 = now). save.c reads SaveBlock1's sectors raw, accepts 1.60.0's Windows sizes
  (+64 in the last sector, SectorChecksumOk) and squeezes them back (MP_SaveFix_StoreSaveBlock1); LoadGameSave reads
  the sidecar before the objects and MP_SaveFix_Migrate fixes the template bytes and the high id bits once per load.
- Guards: tools/test/save_layout.py compiles the saved structs with gcc and MinGW and compares every size and offset with
  tools/test/save_layout.txt (`--update` only for a deliberate format change, with a migration step and a new
  MP_SAVE_FORMAT); tools/test/check_scripts.py checks bufferstdstring indices against gStdStrings (Quetzal's 0x1E-0x2D
  are there; ScrCmd_bufferstdstring writes "" past the table). Both run first in regress.sh and in the release workflow.
- tools/test/save_compat.sh [SAVE]: a save as it was before 1.60, as 1.60.0 Linux and as 1.60.0 Windows must load to the
  same `mp savesum` and continue.
- tools/test/map_sweep.sh [G0 G1]: every map started on directly, A mashed through its arrival scripts, a few steps;
  a crash, a hang or a full heap fails (the end of regress.sh runs groups 0-39). It found TRAINER TOWER's placeholder
  (Quetzal_EventScript_84736F6, from the rip) opening a message box in an ON_RESUME script, which hangs: the tower is
  closed now (receptionist and gate say so, the rest ends quietly; test region_closed_tower).

## Crash hunts and the region openings (1.60.2)
- tools/test/trainer_sweep.sh [IDS]: every trainer battle (HOENN 1-854, JOHTO / KANTO 864-1660), two Lv. 100 MEWTWO
  with BITE against the trainer's team at 1 HP and 1 max HP (MOEMON_TEST_FOE_HP1, test-only, battle_main.c); `mp trainer
  ID` starts one (battle_setup.c MP_Debug_TrainerBattle), `mp battlestyle 1` = SET. A Lv. 100 team that still wins
  leaves the harness in the party screen: a timeout there is the test, not the game.
- Trainer numbers 1022-1024 are Emerald's TRAINER_FRONTIER_BRAIN / PLAYER / SECRET_BASE: gen_trainers.py puts the
  Johto / Kanto trainers that landed there past the end (1658-1660); MP_Region_LoadFromSave moves their beaten flags.
- New games in JOHTO / KANTO: CB2_NewGame fades in (no ExecuteTruckSequence, which drew the truck's door into the
  house), BIRCH's farewell per region (main_menu.c, MP_Region_StartChoice), harness condition `regionmenu`.
- Quetzal's travelling visitors (JESSIE and JAMES, qz_specials.c TravelingEvent) wait for the region's League.
- tools/quetzal_port/fix_warp_tiles.py (run by import.py): FireRed door mats warp on both halves.
- Script numbers the import can't translate (a number set into a var, then additem VAR): check with a scan of
  `setvar VAR_0x80xx, N` before item / species / move commands; OAK's 5 POKé BALLS and RED's gift were the two.
- QzBufferLastReceivedMonForNickname reads gMpLastGivenMonPlace / Slot (pokemon.c GiveMonToPlayer), not VAR_RESULT.

## MOéMON in JOHTO's and KANTO's text (1.62.4)
- tools/moemon_text/moe_words.py: HOENN's rule as code (moe()): every POKé word -> MOé (POKéMON -> MOéMON, POKéDEX ->
  MOéDEX, POKéMANIAC, POKéRUS...; mixed case in capitals), POKé BALL(S) kept (the item's name; MOé FLUTE and MOé DOLL
  are the items' names). Run on JOHTO's and KANTO's map scripts and data/scripts/qz (.string lines only: 1761
  lines); re-run it after re-importing Quetzal's scripts.

## Running outdoors in JOHTO and KANTO (1.62.2)
- Some of Moemon Quetzal's outdoor maps forbid running (allow_running false on 21 JOHTO towns / routes: CHERRYGROVE
  CITY, ROUTE 23 and 26, the NATIONAL PARK, the SAFARI ZONE, MT. SILVER's outside...). The user called that silly:
  bike.c IsRunningDisallowed ignores the map flag on outdoor maps (IsMapTypeOutdoors: town, city, route, ocean route)
  outside HOENN, so re-imported maps need no edit. Indoor maps keep no running (the user's rule), caves keep their
  flag, HOENN keeps Emerald's; metatiles that forbid running still do.
## SINNOH (released in 1.63.0)
The region from start to HALL OF FAME and its postgame; everything below in this section and the next. Where to start
for a change: maps and people tools/sinnoh/gen_maps.py, words tools/sinnoh/words*.py + gen_words.py, story people
story_people.py, trainers gen_trainers.py, legendaries and story battles legends.py, gyms gen_gyms.py + s3d_gyms.c,
places of the game's own data/scripts/sinnoh_places.inc, the frontier s3d_hall.c / s3d_frontier.c, the engine s3d.c.
Every generator is re-run from the repository with the decomp's folder: `python3 -I tools/sinnoh/<gen>.py
POKEPLATINUM_DIR` (tools/sinnoh/fetch_decomp.sh DIR gets one), then `make`.

## A new game in SINNOH (1.63.1)
- BIRCH's region menu (main_menu.c sMenuActions_Region / sRegionMenuRegions) lists KANTO, JOHTO, HOENN, SINNOH in that
  order (REGION_MENU_FIRST_ROW: the cursor starts on HOENN); rows map to REGION_* through sRegionMenuRegions.
- MP_Region_StartNewGame SINNOH: the player wakes in TWINLEAF's house 2F (8,5) with no POKéMON, B-dash on, the heal
  spot TWINLEAF, VAR_SINNOH_ARRIVAL 2 (as if the boat had brought them) and FLAG_SINNOH_NEW_GAME (0x3D38).
- No team: StandardWildEncounter gives no wild battles while the party is empty; every story battle (legends.py) and
  BARRY first checks getpartysize -> Sinnoh_EventScript_StoryNoTeam.
- ROUTE 201's briefcase (story_people.py ROUTE_201_BRIEFCASE, need NEW_GAME unless STARTER): Sinnoh_EventScript_Briefcase,
  the special Sinnoh_StarterMenu (TURTWIG / CHIMCHAR / PIPLUP, Lv. 5), FLAG_SINNOH_STARTER (0x3D39). PROF. ROWAN's lab
  (Sinnoh_EventScript_RowanLab) then gives the POKéDEX and 5 POKé BALLS to a player without one.
- Debug `mp emptyparty` (party and POKéDEX flag cleared); test sinnoh_newgame.

## The KANTO / JOHTO starter table and BLUE's teams (1.63.2)
- The user's design (docs/user_images/57.png): OAK's / ELM's "choose one" ends with the player walking to the table's
  middle (qz_specials.c QzWalkToStarterTable: a path search over the map's collision and people to the first free
  tile below the middle ball, (9, 5) in both labs; the scripts wait with `waitmovement LOCALID_PLAYER`, since
  `waitmovement 0` waits on the last applymovement's object and let the table open mid-walk) and Emerald's starter
  screen in table mode (starter_choose.c MP_StarterTable_Open: the hand, the ball's wiggle, the label, the preview
  circle and YES / NO are Emerald's own; B does nothing: the table is left only with a POKéMON, 1.63.11). The table is drawn in full color under the
  layers (MP_StarterTable_Composite, src/mp/mp_startable_gfx.h from tools/moemon_starters/gen_table.py: the mock-up with
  its balls and hand painted out along the table's elliptical light bands, the right half the left mirrored).
- `special QzChooseStarterKanto/Johto` + `waitstate` keep their contract: RESULT the species (0 for B), plus VAR_TEMP_1
  the ball (0-2) and LAST_TALKED the ball's object (map objects at (8..10, 4)), so the region's own scripts go on.
  Only the region's three: KANTO BULBASAUR, SQUIRTLE, CHARMANDER (FireRed's order), JOHTO CYNDAQUIL, TOTODILE,
  CHIKORITA (sQzTableStarters; obtain_audit.py reads it). Harness condition `startertable`. Tests region_starter,
  region_starter_walk, kanto_rival_win / _lose.
- BLUE as in FireRed: he takes the ball strong against yours (OAK's script already walks him to it in FireRed's ball
  order; his line names it), and every BLUE battle (lab, ROUTE 22 twice, CERULEAN, S.S. ANNE, POKéMON TOWER, SILPH CO.,
  the CHAMPION) brings the version for his starter: qz_specials.c sQzBlueBattles / QzKantoRivalTrainer, read by
  mp_region.c MP_Region_TrainerData before the GEN 3 / GEN 5+ copies (FireRed's teams at every GENERATION). Quetzal had
  rewritten the SQUIRTLE versions; tools/moemon_starters/frlg_rival.py sets all 24 to FireRed's (pret/pokefirered's
  src/data/trainer_parties.h; --check to compare; re-run after re-importing Quetzal's trainers).

## SINNOH: nobody stands on a door (1.63.2)
- Platinum starts some people on a door mat for a scene; without the scene they shut players in or out. BARRY's mom
  in the player's house (she stood on the only door): her visit plays as in Platinum (sinnoh_places.inc
  Sinnoh_EventScript_RivalMomVisit: stepping next to the mat, she comes in, the player is pushed back a step the way
  they came, she talks, walks out and is gone; triggers from gen_maps.py EXTRA_TRIGGERS while VAR_SINNOH_ARRIVAL is 2,
  the visit sets 3: a trigger whose script only ends still holds the player on its tile, so its var turns it off).
  Moved beside the door (gen_maps.py OVERRIDES): CRASHER WAKE at PASTORIA's gym (with 3 badges: the 4th was shut in)
  and VEILSTONE's, the sailor in CANALAVE (his house's only door), STARK MOUNTAIN's policemen (the room's only way out).
  A scan for people on warp tiles is in the commit that did this; the rest are hidden for good (0x3FFF) or block on
  purpose until a story battle. Test sinnoh_rival_mom.

## SINNOH's briefcase: Platinum's starter screen (1.63.3)
- tools/sinnoh/build_starter.py draws Platinum's own scene ahead of time (pure Python: the release's runner has no
  Pillow / numpy; ~30 s) from res/prebuilt/graphic/ev_pokeselect.narc (fetch_decomp.sh PARTS 7): the upright case
  tipping over and opening with the balls hopping in (joint animation 0 on model 1, 41 frames), the cut to the open
  case (model 8) and the camera's six-frame move (-30 deg / 300 units to -50 deg / 200 at z 36, half fov 22 deg), each
  ball's rocking (73 frames) as patches, Platinum's glove (NARC 10/11); white light ahead, dark gray edge marking.
  Into the graphics pack as section STRT (before PKCH; s3d_gfx.c S3D_Gfx_Starter).
- tools/sinnoh/nitro.py JointAnim / Bca0: NSBCA joint animations (T / R / S tracks, steps 1/2/4 with lastInterp,
  pivot (Rot3) and 5-value (Rot5) rotations). Rot5: five u16, 13-bit _00 _01 _02 _10 _11 (>>3, fx12) and _12's low 15
  bits as a 1.15 number from their low 3 bits (v0 highest); its sign bit isn't stored: the row's unit length tells it.
  Both rotation kinds are stored for row vectors: transposed, as Node's full matrices. Model.draw(node_mats).
- The screen: starter_choose.c briefcase mode (MP_StarterBriefcase_Open, on top of the table mode): Task_BriefcaseIntro
  plays the frames at 30 a second (START skips), BriefcaseComposite draws the picture, the ball under the glove
  rocking and the glove under the GBA layers; Emerald's circle, cry and YES / NO; no labels; B closes the case.
  Sinnoh_StarterMenu (s3d_places.c) opens it when the pack has it, the list of names when not; VAR_RESULT the species.
  SpriteCB_StarterPokemon now lands from any spot (steps of at most the distance left). Test sinnoh_newgame.
- Not taken: Platinum's preview window (NARC 14, a white shape in tiles not yet laid out: Emerald's circle instead)
  and the glove's own animation (it bobs by a sine).

## Drop shadows and followers in grass and puddles (1.63.4)
- OPTIONS > DISPLAY > SHADOWS (gMpConfig.noShadows: 0 ON, the default; ini `shadows`): src/mp/mp_shadow.c gives every
  person on the field (object events: the player, NPCs in every region, friends) and every follower / Garden POKeMON
  (sprite->mpCastShadow, set by mp_follower.c's two sprite makers) a shadow sprite: an oval in a 32x16 frame (four
  sizes by the owner's width) at its feet, subpriority 255 (under every sprite) at the owner's priority (roofs and
  tree tops cover it). Its pixels darken what the backgrounds show there to half (sprite->mpShadow -> sprite.c
  gMpOamShadow -> gba_easy_draw.c ShadowPixel), so on SINNOH's 3D maps (drawn into the BG lines) too. None in water,
  while surfing / diving, for inanimate objects, hidden or clipped (swimming) owners. MP_Shadow_Update runs last in
  overworld.c OverworldBasic. Test shadows (`mp shadows [0|1]`: the option, or the count of shadows up).
- MP_SpriteDrawnRows (mp_shadow.c): a sprite's current frame's first and last drawn rows (cached by frame). The shadow
  sits at the picture's feet; a follower's water / ice reflection mirrors at them (mp_followfx.c SpriteCB_Reflection:
  a 64x64 POKeMON stands 16 higher in its frame and its reflection was drawn far below); a small follower (drawn
  rows < 22) in tall grass stands above the grass's top layer (priority 1: the top layer covered it whole).

1.67.0: shadows fade with the screen (gba_easy_draw.c ShadowKeepForFrame fits BG palette 0's shown colors against
its own, MP_FadeFit, and keeps that share of the darkening: a whole-screen fade to white or black takes them, a fade
of other palettes doesn't). A shadow sprite knows its owner (data[6]): an owner whose shadow id was reused by another
owner's shadow makes a new one (two owners used to move one shadow, the player's ending under an NPC off screen; `mp
shadows` prints each shadow's owner and "shadow shared" if it ever happens). A follower's reflection (a struct copy of
it) has no mpCastShadow. A small follower raised to priority 1 in tall grass behind its Trainer gives way to them:
sprite->mpUnder = the Trainer's sprite id + 1, sprite.c marks its OAM entries (gMpOamUnder, the Trainer's first
entry) and the renderer takes its pixels out of its layer where the Trainer's own pixels land (sUnderPix). Tests
follow_behind, follow_ground.

## Settings per save file, and switching save files in place (1.63.5)
- save_slots.c SaveSlots_File(name, shared): the save file in use's own moemon_coop.ini (mp_main.c ConfigPath),
  moemon_controls.ini (gamepad.c), moemon_display.ini (display.c Display_PreInit) and lobby.txt (mp_main.c
  MP_OnlineLobbyAddress) in saves/slotN/, each a copy of the shared one beside the game the first time a slot has none.
  No slots (--save / MOEMON_SAVE, every test) or the MOEMON_*_CONFIG overrides: the shared / given file as before. The
  updater's AutoUpdate still reads the shared moemon_coop.ini (app-wide). The card games' and every other side file were
  already "<save>.<x>" in the slot's folder.
- The title screen's SAVE FILE no longer starts the program again: SaveSlots_Switch sets the path and slot.txt,
  Platform_ReopenSaveFile (sdl2.c: the open file and the flash copy, as at the start), MP_SaveSlotChanged (mp_main.c: out
  of any room, the settings re-read and applied: LoadConfig, Gamepad_ReloadSettings, Display_ReloadSettings; lobby.txt
  read again; Tcg_ForgetSidecars: the three card collections and SEARCH lines), then main_menu.c MpReloadSave reads the
  save as intro.c does at boot and the main menu opens on it (ERASE of the file in use the same).
- sdl2.c ReadFlash: what isn't in the file reads as the flash copy (0xFF, erased flash): a new slot's empty file read
  nothing and kept the last file's sectors in the buffer ("erased due to corruption" after switching to an empty one).
- tools/test/save_slots.sh covers it (the switch in place, file 2's own settings copy).

## NEW in the BINDER, centered booster packs (1.63.6)
- src/tcg/tcg_new.c ("<save>.cardnew", per game a byte per card: the copies the BINDER has shown; owner = the POKeMON
  collection's, a new game on the save starts again): a card owned more times than that shows NEW in its game's
  BINDER (tcg_ui.c / ygo_ui.c BinderScreen, tt_ui.c ListScreen; not the shops' singles lists); the cursor on it makes it
  seen. No add site is hooked: packs, prizes, trades, wins all count. The first time (no file) everything owned counts
  as seen. Tcg_ForgetSidecars forgets it with the save file. `mp cards newcount`; test cards_new.
- BOOSTER PACKS (POKeMON, Yu-Gi-Oh!): the chosen pack in the middle, the others on both sides, gliding to the cursor
  (PacksScreen: a float position eased 35% a frame; no glide when the screen opens). Triple Triad's packs screen is a
  list beside the chosen pack already. Test cards_packs_center.

## 40 PC boxes (1.63.7)
- TOTAL_BOXES_COUNT 40 (pokemon_storage_system.h). The save still holds 14: its storage sectors are struct
  PokemonStorageLegacy (save.c's SAVEBLOCK_CHUNKs; tools/test/save_layout.py measures that one, unchanged), packed from
  MP_PersonalStorage() in UpdateSaveAddresses into a static buffer and unpacked in LoadGameSave (MP_Boxes_LoadFromSave,
  before MP_Region_LoadFromSave). Boxes 15-40 (POKeMON, names, wallpapers) and a box in use past 14 are in
  "<save>.boxes" (src/mp/mp_boxes.c; written by sdl2.c StoreSaveFile, backed up as "<save>.bakN.boxes"), with the
  save's counter and a checksum of the 14: the file written with the save wins, then a backup's that matches, then the
  newest anyway (never drop POKeMON because the save fell back to its older slot). None: boxes 15-40 empty, BOX15...
  An older game reading a 1.63.7 save sees its first 14 boxes (a box in use past 14 shows as box 1).
- Each region's PC and the GLOBAL PC ("<save>.region" chunks PCn / GLPC) are written with 40 boxes; 14-box chunks from
  before are read and converted (mp_region.c ReadStorage; GLOBAL15...).
- The shared PC: MP_SHARED_BOXES 40, protocol 70. The server's store file is MOEST3; MOEST2 (14 boxes) loads into
  the first 14 (mp_store.c). mp_sharedbox.c's per-box masks are 64-bit / per-box arrays (MP_SharedBox_TakeInfoChanges
  takes the box). shared_pc/check.py reads MOEST3.
- Tests tools/test/pc_boxes.sh (boxes 1 / 15 / 40 and box 33 in use across a save; no side file; a backup's; another
  save's), scenario shared_pc_v2 (a MOEST2 file; SHARED40 on screen), region_globalpc (1200 free). `mp curbox [N]`.

## 1.63.8: SINNOH polish, softer shadows
- Shadows (mp_shadow.c, gba_easy_draw.c ShadowPixel): a quarter darker than the ground (was half); the feet come
  from the lowest row of every frame of the sprite's animation (SteadyBottom, remembered per sprite while its
  pictures stay), not the frame shown, so running up / down doesn't move them. `mp shadowwatch N`: the player's
  shadow's gap over N frames (test shadows: "moved 0").
- The briefcase's glove (build_starter.py cursor_image): its RAHC data starts 0x18 + offset into the NCGR (was 0x10:
  8 bytes early, every row shifted). The release's pack is built again (make_pack.sh).
- SINNOH's briefcase starter sets FLAG_SYS_POKEMON_GET (sinnoh_places.inc); mp_region.c SinnohStarterMenuFlag gives
  saves with FLAG_SINNOH_STARTER (0x3D39) theirs on loading. Test sinnoh_newgame.
- The 3D view draws on CB2_OverworldBasic too (s3d.c OnField: battle_transition.c runs the field there), so a battle's
  opening keeps the map. Wavy transitions don't bend it (a picture, not the BG's scroll).
- POKé MART BUY screen (shop.c Mp3dRecord / BuyMenuDrawMapGraphics, s3d.c S3D_ShopPicture): on a 3D map the field's
  last 3D picture (s3d.c sColor, faded as the screen fades) moved so the player stands where Emerald's layout puts
  them, and the people at their 3D spots moved the same; no metatiles. CreateObjectGraphicsSprite loads JOHTO / KANTO /
  SINNOH palettes too (QzLoadObjectPalette: the clerk was black). Test sinnoh_marts shot buy.
- POKéMON CENTER healing (src/sinnoh/s3d_heal.c, field_effect.c FldEff_PokecenterHeal): Platinum's
  pokecenter_healing_machine_mini_pokeball model (packed as a named mesh: gymprops.py) in dynamic prop slots 30-35, one a
  ball every 16 frames with SE_BALL, Platinum's offsets (x +-4.5, z -4.5/0/+4.5, y 12) from the machine, which stands at
  (-22.2, -14.5) from the nurse's tile middle ((-24, -12) in the LEAGUE's north CENTER); then they blink every 8
  frames while MUS_HEAL plays (Platinum's flash is a material animation). `mp healballs`; test sinnoh_heal.

## 1.63.9 / 1.63.10: HOENN's tiles, music loops, SINNOH's water, moving props, warps; shadows
- field_camera.c DrawMetatileAt drew every metatile id from 0x300 (S3D_METATILE_BEHAVIOR) as metatile 1 in every region
  since 1.63.0; only on SINNOH's maps now (S3D_IsSinnohGroup): HOENN's secondary tilesets use 0x300-0x3FF for their own
  art (FALLARBOR's BATTLE TENT, ROUTE 104's bridge, RUSTBORO's paths). Anything SINNOH-only asks S3D_IsSinnohGroup
  (the user: KANTO, JOHTO and HOENN are "GEN 3" maps, SINNOH "GEN 4").
- Songs made from rips without a loop point loop from the start (tools/kaisong/add_loop.py; --check in kaisong_test.sh:
  every song in music/ loops but the fanfares).
- SINNOH's water (pack version 5): Platinum's fldtanime (fetch_decomp.sh PARTS 8): each animated map texture's frames
  decoded with the map texture's own palette and size (nitro.py decode frame=, the frame file's own header ignored as
  Platinum copies the bytes), swapped by s3d.c AnimateTextures (Platinum's field frames: 30 a second); a map texture of
  another size than its frames stays still (one set's "sea"). `mp s3dtex`; test sinnoh_water.
- Props that move all the time (version 6): Platinum's bm_anime_list (looping, not deferred) joint animations (NSBCA)
  drawn ahead as a mesh per frame (build_pack.py prop_frames: every 2nd frame, at most 48), s3d.c meshShown picks the
  frame. Not yet: texture scrolls (NSBTA), texture swaps (NSBTP) and the doors' one-shot animations.
- SINNOH's warps (build_pack.py warp_behavior; field_screen_effect.c SinnohArrivalDir; overworld.c keeps the facing):
  Platinum's entrance mats east / west / south are Emerald's arrow warps (same numbers); stairs 0x5E up / 0x5F down
  (Emerald's unused behaviors, non-animated doors in metatile_behavior.c) and escalators MB_UP / MB_DOWN_ESCALATOR by the
  floors' names; every other warp a non-animated door. Arriving, the player walks on the way they were going, back the
  other way if that is blocked, else stands. Stairs and escalators carry the player while the screen fades
  (DoSinnohRideWarp: walking across and up / down; standing on the escalator). Emerald's own escalators are unchanged.
- Shadows: people all the user's size (size 0, every LOOK); big (PMD) followers' shadow at their feet point, as wide as
  their widest frame; small sprites from their first picture's lowest row (standing); in 3D a shadow is flat (s3d.c
  OamDepth.flat: only a wall in front hides it). `mp shadowwatch` reports both shadows' gap and missing frames.

## 1.63.11: followers on the player's line, shadow flicker, LOOKS per save file
- PMD followers / Garden POKeMON: mp_pmd.c UpdateLift lifts every frame by PMD_FEET_DY + 1 - the IDLE frames' lowest row
  (StandingBottom), so a species' standing feet sit on the player's lowest row (PMD_FEET_DY below the ground point);
  mp_shadow.c puts a big sprite's shadow on that line.
- sprite.c LoadOam copies the shadow marks (gMpOamShadowLive) with the OAM: the marks were per OAM entry as the list was
  built, and a frame whose OAM wasn't copied (a fade's: CB2_Overworld drops the VBlank callback) drew a shadow solid
  and its owner darkened.
- mp_avatar.c MP_Avatar_SettingsLoaded (start-up and mp_main.c MP_SaveSlotChanged): the look's CREATE recipe and its
  sprite pack set up again from the save file's own settings (a switch kept the other file's, and a later save wrote
  them into this one's). `mp saveconfig`, `mp lookname`.
- KANTO's key-item gift script (Quetzal_EventScript_845571E) took the item as Quetzal's raw number in VAR_0x8000
  (import.py only translated the ones before callstd STD_OBTAIN_ITEM: now these too; 39 scripts fixed), and both
  regions' "put away ... Pocket:" scripts named Quetzal's own pockets by its numbers: special MP_BufferItemPocket
  (mp_bag.c) puts the BAG view's name (MP_Bag_ViewOfItem) in STR_VAR_3. CELADON's roof drinks / apples were raw numbers
  too. `mp qzputaway ID N`; test qz_putaway.
- gamepad.c: the north button (the CO-OP menu's) is no longer SELECT off the overworld (menus, text, battles, naming,
  PC): pressed as a message closed or a menu opened it used the registered item. SELECT is the Back button only.
- PMD walk: MP_Pmd_Animate keeps WALK for up to 6 frames after `walking` drops (MpPmdView.stopping): the follower's
  walking flag is off for a frame at each step's end, which went to IDLE and restarted the walk (Riolu's frames 0-2 of
  4: one foot). Horizontal anchor: MpBigSprite.shift = -(the average column of the IDLE frames' pixels for the facing,
  cached in MpPmdView.mid), so the body's middle is on the sprite's x for every facing (tails and ears hardly count);
  mp_bigsprite.c adds it. Test pmd_walk_sides.
- Sprite scaling: a PMD big sprite (MpBigSprite.footed) shrinks toward its point and feet line (x + shift, y + footY),
  not each frame's picture's bottom middle; the affine path (ball entrance, water reflection) applies sprite->mpScale
  too (gba_easy_draw.c DrawBigSprite: the turned picture, then shrunk toward the anchor), reflections copy mpScale
  (mp_followfx.c), and the scale is measured on the first IDLE frame facing down (MP_Pmd_ScaleFrame), never cached
  before the sheet is in. It used to come out of the ball full size and shrink when the entrance ended.
- Specials are numbered by their place in data/specials.inc and something reads them by number: add new ones at the
  end (MP_BufferItemPocket in the middle moved the rest by one and broke the boat to JOHTO: region_travel).
- KANTO / JOHTO starter table: B does nothing (starter_choose.c: only SINNOH's briefcase is put down with B).
- JOHTO: flag 0x14A4 hides Quetzal's Giovanni-story ROCKETs (CHERRYGROVE, AZALEA, ROUTE 35, GOLDENROD): set unless in
  the past (qz_specials.c QzGiovanniRockets, on every JOHTO map load and QzSetGiovanniModeFlag14A4, which cleared it).
  Test johto_no_rockets.

## 1.63.12: the nurse's UNION ROOM check
- After healing, the nurse scripts (Hoenn's pkmn_center_nurse.inc, KANTO's / JOHTO's qz shared scripts) ask special
  BufferUnionRoomPlayerName whether a friend is waiting in the wireless UNION ROOM (and say so once per save:
  FLAG_NURSE_UNION_ROOM_REMINDER, FLAG_QZ_0x220D, FLAG_QZ_0xA98). union_room.c's sUnionRoomPlayerName is zeros on PC
  until a map's ON_RESUME runs special InitUnionRoom (CableClub_OnResume: HOENN's and KANTO's centers; JOHTO's don't),
  and zeros aren't EOS: the special said yes and StringCopy read on past the buffer for the name. On PC it now always
  says no (there's no wireless UNION ROOM), and InitUnionRoom only clears the name: its Task_InitUnionRoom is a stub,
  so the struct it allocated was never freed (a leak per POKeMON CENTER entered). Test nurse_no_union (CHERRYGROVE).

## 1.63.14: every follower style placed as PMD, one shadow size, SPRITE SCALING in 5% steps, GAME CENTER pictures
- OPTIONS in five tabs (the user: friendlier, nothing removed): GAME (TEXT SPEED, FRAME, NAME CASE, CREATURES, BUTTON
  MODE, SOUND, BIKE / SURF MUSIC, CARD SCALING / ART / BACKS), BATTLE (BATTLE SCENE / STYLE / ANIMS, TYPE ICONS, LEVEL
  SCALING's page, GENERATION, MOVE KIND, the four gimmicks), DISPLAY (the window's five, FOLLOWERS, SPRITE SCALING's
  page, SHADOWS), CONTROLS, CO-OP. The header spreads the five names over its 26 tiles. `mp optionstab N`: 0 GAME,
  1 BATTLE, 2 DISPLAY, 3 CONTROLS, 4 CO-OP. Tests options_tabs, levelscale_options, sprite_scaling_page (SPRITE
  SCALING's A / LEFT with the real keys), fight_buttons, spec_menu walk the new places.
- OPTIONS > DISPLAY > COLOR (display.c BuildColorLut / UploadColored): DEFAULT, GBA CORRECTED, GBA SP, GBA SP 101, NDS
  from Pokefan531's color-correction shaders (libretro slang-shaders handheld/shaders/color, public domain): linear
  light (2.2), the screen's 3x3 matrix and brightness, out at 1/2.2 + darkness / 4.84 (GBA CORRECTED: darkness 1.0, the
  unlit GBA; GBA SP: the same profile without it). A 32768-entry table turns each frame into sColorTex (ARGB8888);
  card screens and spectating keep using the game's own texture. Config `color` in moemon_display.ini; `display
  color N` in the harness. Test display_color (each mode exact against check.py's own working).
- Screen grids per handheld (the user: matching the COLOR by default, mixed as liked): FILTER GBA / GBA SP / SP 101 / NDS
  GRID (display.c FilterMask's sGrid: cell edge, the next line in from 8, sub-pixel stripes) and MATCH (ShownFilter:
  the COLOR's own grid, DEFAULT none), the default; a moemon_display.ini from before COLOR (no `color` line) loads
  `filter = off` as MATCH. display_color checks MATCH and a mixed pair pixel for pixel.
- FILTER redone (the user: better than cheap lines): display.c FilterMask, a multiply mask (SDL_BLENDMODE_MOD) per
  screen pixel of a picture pixel: SCANLINES a beam's soft rows (0.38-1.0, brightest a little above the middle) and
  from 3 columns a grille (other channels 0.86); LCD GRID sub-pixel stripes (0.80) and soft cell edges (0.70, the
  next line 0.88 from 8). fight_display's check.py works out the same mask.
- GAME CENTER games follow OPTIONS > DISPLAY > SCALING (the user): PIXEL PERFECT / SHARP FIT show a VS cabinet's
  480x320 as the game picture (the game's 4:3 in the middle, black beside it), STRETCH only that 4:3 picture over the
  window (display.c srcRect from platform/fight.h Fight_ContentSpan: FIGHT_ARCADE_PIC_W 427, MUGEN's MF_GAME_W 426).
  MUGEN's 3:2 fitting is gone (FIGHT_DECISIONS "Screen fitting"): its picture is 4:3 (mf_host.c MfHost_ConfigLoaded,
  MfHost_Present centres it), no wider camera, no HUD moved, no screen sides filled. The countdown and signal overlays
  stay inside the 4:3 picture. The DANCE machine and the card games draw the whole 240x160 game picture already, so
  they follow every mode as the game does. Test fight_display (STRETCH: the 426 middle columns over the window).
- mp_shadow.c MP_SpriteStandShift: a follower's or Garden POKeMON's own (GBA-size) sprite stands as a PMD picture
  does: its first frame's lowest row on the player's line (PMD_GROUND_DY + PMD_FEET_DY below its tile's middle, as
  mp_pmd.c UpdateLift), the middle of its pixels for the facing (anim dir * 2, hFlip mirrored) on its point
  (MpBigSprite.shift's rule). Used by mp_follower.c (the 64x64 sprites' flat 16 up is the fallback) and
  mp_garden_life.c (DrawDoing's feet from MP_SpriteFeetRow). Scaled, such a sprite shrinks toward that feet row and
  point (mp_bigsprite.c MP_BigSprite_NoteScale, sprites marked mpCastShadow), as a PMD picture toward its feet.
- Shadows: every POKeMON's is the player's oval (the user: the same size as the player's), any style or size.
- SPRITE SCALING in 5% steps: gMpConfig.scaleFollowers / scaleGarden stay ON / OFF (A turns them on and off in one
  press: the user's rule), scaleFollowersPct / scaleGardenPct how far a small PMD picture goes toward its POKeDEX height
  while on (5-100%, MP_SCALE_STEP; LEFT / RIGHT, which also turn it on; MP_SpriteScale_ForMon: art - (art - target) *
  percent / 100). Config keys scale_followers / scale_garden as before, scale_*_percent new (defaults 100 and 50: the
  old ON's). `mp spritescale F G`: 0 off, 1 on, more: on at that percent.

## 1.63.13: scaled PMD pictures drawn whole, ledge shadows, VS cabinet seats
- gba_easy_draw.c DrawBigSprite skipped every screen line outside the big sprite's full-size picture before it looked
  at sprite->mpScale. A scaled picture shrinks toward its anchor (a PMD picture's feet), so a frame drawn above the
  feet (a hop, a walk's bob, the frames followers play after a menu or a message) comes down past the full-size
  picture's bottom, and those lines were never drawn: its lower part vanished (SPRITE SCALING on, PMD pictures only:
  the GBA-size sprites go straight to DrawScaledEntry, which keeps to the smaller picture's own lines). Test
  pmd_scale_hop (PIKACHU mid-hop: 10 rows whole, 8 cut).
- Ledges: event_object_movement.c DoShadowFieldEffect starts no FLDEFF_SHADOW while OPTIONS > SHADOWS is on (the
  drop shadow stays on the ground under the jump). `mp shadowwatch` also prints the most jump shadows seen at once.
  Test ledge_shadow.
- VS cabinets: their events moved to the chairs' rows 9-13; the cabinet's message no longer waits on Arcade_Check
  unpacking a whole ROM set (it reads the zips' lists of files: cps2recomp romset.cpp CheckRomSet) or on the MUGEN
  pack's hashes (read when a fight is asked for or answered, not at MP_Fight_SeatCheck).

## SINNOH's 3D engine
- Plan and rules: docs/SINNOH_HANDOFF.md. The data comes from pret/pokeplatinum (source.py: its map blocks, matrices,
  areas, props, texture sets and lights as files; fetch_decomp.sh gets them at a pinned commit, make_pack.sh fetches and
  converts). A Platinum ROM gives the same files byte for byte (nds.py: the cartridge's file system and NARCs;
  platinum.py: map matrices and land data; build_pack.py / render_ref.py --rom). nitro.py: Nitro models BMD0 and textures BTX0, every
  texture format, the render commands and display lists flattened to triangles). render_ref.py draws a reference
  picture offline (numpy); build_pack.py writes the 3D pack the game reads (format in its docstring): textures as RGBA,
  meshes (per material group: texture, repeat / flip, alpha, polygon id, culling, light switches, mode, triangles with
  positions, texel coordinates, colors and normals), the area light sets (data/arealight.narc's text: per time block 4
  lights, then diffuse / ambient / specular / emission), and per matrix cell its block mesh, props (build_model.narc
  meshes with their transforms) and the field engine's 32x32 map blocks from Platinum's terrain attributes (bit 15
  collision, low byte the behavior: Platinum's numbers match Emerald's for grass, water, sand, ledges; a General
  tileset metatile with that behavior is used). Each cell's area (so texture set, prop textures, lights) comes from
  pokeplatinum's map header table (include/data/map_headers.h). Textures are shared by content (a temporary model's
  id() is reused once freed, which had two props share a texture), so the pack is the same on every run.
- The pack is never committed (/sinnoh/ is ignored); the release runs make_pack.sh and package.sh ships it in sinnoh/.
  The game reads sinnoh/platinum3d.pak beside it (Updater_ExeFile, as music/) or MOEMON_S3D_PAK. Without it the SINNOH
  maps keep their placeholder layouts (every cell blocked).
- Maps: tools/sinnoh/gen_maps.py POKEPLATINUM_DIR makes the field engine's maps: every outdoor place of matrix 0 (66:
  towns, cities, routes, lakefronts, islands) as group 40, numbers kept (TWINLEAF TOWN 0, ROUTE 201 1, then by name),
  each the rectangle of its matrix cells (four are L-shaped: the corner is the matrix's filler, blocked), connected
  wherever two rectangles touch; music from the header's day song (MUSIC: SEQ -> slot), type and weather from the
  header (WEATHER), placeholder blocks; it also writes src/sinnoh/s3d_maps.h (sMaps) and data/event_scripts_sinnoh.s's
  includes, and keeps events already in a map.json. MAX_MAP_DATA_SIZE is 16384 (SEABREAK PATH is 32x256); the pack
  covers the whole matrix; fieldmap.c asks
  S3D_LayoutBlocks for a layout's blocks (the map and its connections), which gives the pack's for SINNOH maps. Script
  file data/event_scripts_sinnoh.s.
- Drawing (src/sinnoh/s3d.c): S3D_Update each overworld frame (after the camera) finds the map and Emerald's camera
  (mp_mappic.c's formula: screen (x, y) is map pixel (camX + x, camY + y), 16 a tile = Platinum's units) and sets the
  view: Platinum's default field camera (666.92 away, 59.05 degrees down, half fov 8.09 degrees at the DS's 96-pixel
  half height, so one unit a pixel at the target) looking at the player's tile centre (their sprite's middle, 8 above
  its feet), drawn where Emerald shows the player. gba_easy_draw.c calls gMpS3dLine for each line: on line 0 it renders
  the 3x3 cells around the target (opaque groups then translucent; a translucent pixel isn't drawn twice by the same
  polygon id, the DS's rule that keeps overlapping shadows even; textures modulate the lit vertex colors; lighting per
  vertex for the current light block, cached), then each line puts the picture (through the BG palettes' fade, as
  mp_mappic.c) in BG3 and clears BG1 / BG2. BG0 (text) and sprites go on top.
- Sprites: sprite.c UpdateOamCoords moves each camera-following sprite (coordOffsetEnabled) to where its spot on the
  ground shows in the 3D view (S3D_PlaceSprite: feet - 8 = tile centre -> world -> projected, + 8), and notes its depth
  per OAM entry (S3D_NoteOam, committed with the OAM in LoadOam); PutObjPixel asks gMpS3dOamHidden: a sprite is an
  upright card, a pixel dy above its feet dy * tan(59.05) nearer, hidden where the 3D depth is nearer still (trees and
  roofs in front of whoever stands behind them, as on the DS).
- Light blocks: Platinum's rule (area_light.c): the first block whose end time is past now, in 2-second units of the
  day; MOEMON_S3D_HOUR fixes the hour for tests.
- Heights (BDHC): the pack (version 2) holds each block's ground heights at every tile's centre (build_pack.py
  heights(): every plate, a rectangle around the block's centre in x / z and a plane, that holds the point; up to 3,
  a bridge over the ground; world heights, the matrix altitude only matters on filler cells). s3d.c GroundAt blends
  the 4 tile centres around a point, each tile's height the one nearest the object's last (TileHeight): the camera
  target stands on the ground under the player (sView.playerH) and every field sprite on the ground under its feet
  (S3D_PlaceSprite with its sprite id: sSpriteH keeps each one's layer). render_ref.py targets the same height.
  `mp s3dground X Y [NEAR]` reads it (map pixels). TWINLEAF's ground is 16, its pond 8.
- Buildings (pack version 3): tools/sinnoh/places.py decides which of Platinum's headers become maps (outdoor places on
  matrix 0; buildings, caves and the rest each on a matrix of their own, often shared: every POKéMON CENTER one;
  left out: the Underground, GTS, PAL PARK, Wi-Fi, the Communication Club, Super Contests, the Poffin House and the
  UNUSED / UNKNOWN leftovers). The pack holds every matrix a kept place uses, once, under its Platinum number (248;
  a building's area from the first place using it); sMaps names each map's matrix and s3d.c looks cells up in the
  current map's (CellIn / MatrixNumbered). gen_maps.py puts the 433 buildings in groups 41-44, 120 a group: a warp
  keeps its map number as an s8, so a group holds at most 127 (157 crashed UpdateEscapeWarp); SINNOH_GROUPS in
  s3d_maps.h for S3D_IsSinnohGroup. Every place's warps come from its events file (res/field/events, fetched since
  fetch_decomp.sh's parts 2), the region's coordinates made local, warps into places left out dropped. The pack
  stamps warp tiles: Platinum's south mat (0x65) stays Emerald's south arrow warp (General metatile 0x024), every
  other warp (doors 0x69, cave mouths, stairs, escalators) warps when stepped on (non-animated door, 0x0A7).
- The pack file is "S3DZ" + the pack deflated (zlib, already in the game for the fight engine): 93 MB -> 7 MB, inflated
  in about a quarter of a second when the first SINNOH map loads (s3d.c LoadPack).
- SINNOH is REGION_SINNOH (3, REGION_COUNT 4): MP_Region_OfGroup gives it for groups 40-44 (S3D_IsSinnohGroup). Its
  GENERATION cap is never below 4 (Platinum's own POKéMON are Gen 1-4, so GEN 3 is its vanilla too). Its flags start
  at REGION_SINNOH_FLAGS 0x3320 (Quetzal's end at 0x2EB7, its trainers' "beaten" flags at 0x3000-0x3319): FLAG_SINNOH_BADGE01.. its badges. Its boat comes in at
  CANALAVE's south dock (8, 62), but it isn't on the boat's list (QzGetReachableRegionsMask_) until its story is in.
  The region side file's level records hold 4 regions' EXP now: chunk LVL4; an old LVLS chunk (3 regions,
  struct RegionLevel3) is read and converted. Names: "SINNOH" in mp_main.c's lists.
- People, signs and coord events (gen_maps.py from each place's events file): where they stand (local tiles), the game's
  closest sprite and movement (tools/sinnoh/sprites.py, our own table; things that aren't people stay as hidden
  objects, flag 0x3FFF FLAG_SINNOH_HIDDEN, set when a SINNOH map loads, so the local ids keep Platinum's order), hide
  flags and coord variables as SINNOH numbers (tools/sinnoh/ids.py: flags from 0x3328, variables from 0x5180; only
  numbers reach the repository). The sprite table uses the game's own sprite when it has one of the same name
  (YOUNGSTER, LASS, HIKER, TEALA...), then the closest; POKeMON standing in the world get the nearest the game has.
  Who shows: anyone the story shows or hides (a hide flag in Platinum) stays hidden until their script is written,
  and so do a neighbor's people Platinum loads early (hidden_flag MAP_HEADER_*). Signs, mailboxes and books are
  Platinum objects: each becomes a sign event where it stands once it has words. `mp hideobjs` hides every object
  (test_poc's picture).
- Platinum's own words and scripts (2026-11-02, the user's call: "we have all the original dialogue and scripts";
  never write original words or scripts for SINNOH again). tools/sinnoh/build_text.py turns the decomp's message banks
  into sinnoh/platinum_text.pak (src/sinnoh/s3d_text.c reads it; a script's text is a placeholder, PLACEHOLDER_ID_SINNOH_TEXT
  0x40, naming a bank and a message). tools/sinnoh/gen_scripts.py converts Platinum's event scripts (res/field/scripts)
  into this game's commands: sinnoh/platinum_scripts.inc, included by data/event_scripts_sinnoh.s; make_pack.sh makes
  both (the release runs it before the builds) and writes sinnoh/decomp_dir, so the Makefile's rule (`gen_scripts.py
  --auto`) can make them again, or stubs (every label an `end`) without the decomp. Nothing of either is committed: a
  map's person / sign / bg runs `goto SinnohP_<script file>_<entry number>` (gen_words.py writes those stubs when given
  POKEPLATINUM_DIR: numbers only). A script converts when every command it reaches is one gen_scripts.py knows
  (`gen_scripts.py --report POKEPLATINUM_DIR`: 1208 of 2511 entries at first; the rest keep the written words below
  until their commands are added); flags and variables get SINNOH's numbers (ids.py); what this game's commands lack
  is the `sinnoh OP, ARG` command (0xF6, src/sinnoh/s3d_scrcmd.c: the player's / rival's / counterpart's names into
  STR_VAR_1-3, Platinum's YES = 0). People the story shows or hides (hide flags) keep their written words until the
  story's scripts are converted too. Test sinnoh_platinum_words (TWINLEAF's guitarist, the town sign).
- Platinum's story (2026-11-02, behind FLAG_SINNOH_PLATINUM_STORY 0x3D3B until it plays through): gen_scripts.py also
  emits, per map, Platinum's map scripts (scripts_init_*.s: on transition / load / resume after the map's own, frame
  rows after its own; a row whose script doesn't convert is left out, it would run every frame), its people (hide
  flag, Platinum's place, the converted script unless the person is one of Kai's roles) and its triggers (each tile
  of the area; the map's own where Platinum has none), as gSinnohPlatinumMapScripts. s3d_scrcmd.c
  Sinnoh_PlatinumMapScripts (called from overworld.c LoadCurrentMapData / LoadSaveblockMapHeader) swaps them into a
  RAM copy of gMapHeader's events (warps and bg events copied too: SetWarpEventPos / SetBgEventPos move them).
  Sinnoh_PlatinumStoryBegin (`mp platinumstory`) sets the flag and runs Platinum's scripts_init_new_game: SINNOH is
  always Platinum's story (the user, 2026-11-02: no saves partway through the written one to keep, no migration), so
  any SINNOH map load without the flag begins it. Platinum's "\r" is a new box (both lines scroll away): build_text.py
  makes it this game's page break (its one-line scroll put a page's second line out of the box and stalled the text).
  Platinum's messages have six string slots: 0-2 are STR_VAR_1-3, 3-5 PLACEHOLDER_ID_SINNOH_VAR4-6 (gSinnohStrVar).
- CANALAVE's ferry sailor (2026-11-02, the user: parity with JOHTO's and KANTO's ports): shown only while the boat can
  sail (SINNOH's ELITE FOUR beaten and the S.S. TICKET: qz_specials.c MP_Region_CanSail) and on the first boat in
  (VAR_SINNOH_ARRIVAL 1, the sailor's greeting): FLAG_SINNOH_HIDE_REGION_BOAT 0x3D3D, set on each SINNOH map load
  (s3d_scrcmd.c Sinnoh_PlatinumMapScripts) and given to the sailor by gen_scripts.py KAI_ALWAYS. SINNOH's S.S. TICKET
  comes with the CHAMPION's win (data/scripts/mp_region.inc EventScript_SinnohTicket, the game's own "obtained"
  message; Platinum has no ticket of its own and a game begun in SINNOH had no way to one). Customs and the GLOBAL PC
  were already the shared ones (the CENTERs' PCs are MB_PC tiles: EventScript_PC). Test sinnoh_ferry (`mp sinnohticket`).
- Partners shown from behind (2026-11-02): battle_tower.c SetQzPartner also asks s3d_gfx.c S3D_Gfx_PartnerBack for a
  SINNOH partner: Platinum's own back for BARRY, CHERYL, MIRA, RILEY, BUCK and MARLEY (sinnoh/platinum_gfx.pak "BACK":
  build_gfx.py backs(), each class's back.png drawn 0.8 times as big into four 64x64 frames, five-frame backs keeping
  0, 1, 3, 4 and BARRY's eight 0, 1, 3, 5), Kai's LOOKs' for DAWN and LUCAS (kai_dawn / kai_lucas), into the second
  partner slot (mp_avatar.c MP_Avatar_TilesBackPic). The catching tutorial's reshown screen (after the BAG) keeps its
  trainer (reshow_battle_screen.c gWallyTutorialBackPic). Tests sinnoh_platinum_partner, sinnoh_platinum_tag.
- SINNOH's TV (2026-11-02, the user: "Pokemon Platinum's TV system be built and function properly"): src/sinnoh/s3d_tv.c,
  ported from Platinum's tv_segment.c, ov6_022465FC.c, tv_commercials.c and scrcmd_tv_broadcast.c. A SINNOH TV tile
  (MB_TELEVISION: field_control_avatar.c) runs Platinum's own broadcast script (gSinnohTvBroadcast); the program on air
  is Platinum's weekly schedule's for the quarter hour (tv.narc, emitted by tools/sinnoh/tv_data.py into
  platinum_scripts.inc; MOEMON_TEST_TV_PROGRAM for tests): a greeting by the time of day, up to four segments, a
  farewell, then commercials until the next quarter hour (the closed features' commercials never air). Messages by
  Platinum's own names (S3D_TextNamed: tv_data.py's name-hash table; banks without names use "pl_msg_<bank>_<n>").
  TRAINER SIGHTINGS / RECORDS / INTERVIEWS segments are kept four each (a new one replacing one aired three times) in
  "<save>.region" chunk TVBC, recorded in SINNOH from the game's own events: wild battles (battle_setup.c), fishing
  (field_player_avatar.c), shopping (shop.c), eggs (egg_hatch.c), the GREAT MARSH (safari_zone.c), hidden items, the name
  rater, the PC bulletin, BERRY crops, POKé BALL effects (the SEAL CLUB SHOW, in the Seals' place). The reporters ask for
  one word on the game's own easy chat (GABBY and TY's screen: special Sinnoh_TvChooseWord). Online-only and Underground
  segments never air; those about systems SINNOH lacks yet (contests, photos, BERRY patches, roamers, the VILLA) wait
  for them (docs/SINNOH_CHECKLIST.md). `mp tv`, `mp tvwatch`, `mp tvcatch SPECIES BALLS`. Test sinnoh_tv.
- Seals as POKé BALL effects (2026-11-02, the user: "Poke Ball effects take the place of Seals wherever they would
  appear in the story, or in menus or shops"): src/sinnoh/s3d_seals.c. Each of Platinum's Seals (generated/seals.txt's
  ids) stands for an earned ball effect piece (mp_ballunlock.c's MOEGATCHA prizes in order, round again): a Seal given
  unlocks it, a Seal counted is 99 (Platinum's full case) once unlocked, its name is the piece's; the SUNYSHORE MARKET's
  Seal shop (PokeMartSeal: data/scripts/mp_region.inc EventScript_SinnohSealShop on the SINNOH menu) sells the day's
  Seals' pieces at Platinum's prices (tv_data.py seal_lines: gSinnohSealPrices / gSinnohSealShops); Platinum's SEAL
  CASE (item 679) still comes first from SOLACEON; the PC's BALL CAPSULES are the summary's POKé BALL page (its
  editor), and an effect saved there in SINNOH is the TV's SEAL CLUB SHOW. Test sinnoh_seals.
- The FASHION CASE (2026-11-02, SUPER CONTESTS' first piece): src/sinnoh/s3d_fashion.c keeps Platinum's 100
  accessories (the first 61 up to 9 each, the rest one) and 18 backdrops in "<save>.region" chunk FASH; AddAccessory,
  CanFitAccessory, BufferAccessoryName(WithArticle), AddContestBackdrop, CheckBackdrop, BufferContestBackdropName and the
  massage girl's gift convert (gen_scripts.py: ACCESSORY_* / BACKDROP_* are Platinum's numbers, dec.fashion_ids).
  POFFINS are this game's POKéBLOCKS (the notes' rule: what Kai has SINNOH uses): the POFFIN case checks and GivePoffin
  give a POKéBLOCK of the same flavors (smoothness as its feel). Plan: docs/SINNOH_CHECKLIST.md section 5.
- DAWN and LUCAS (2026-11-02): the counterpart walks with Platinum's own player sheets (res/graphics/field_sprites/player
  player_f / player_m only, fetch_decomp.sh parts 14; build_gfx.py appends them after every other sheet so no sprite's
  number moves: OBJ_EVENT_GFX_SINNOH_PLAYER_F / _M; sprites.py maps DAWN / LUCAS there). Platinum's GetNationalDexCaughtCount /
  CheckNationalDexCompleted count as its pokedex.c does (1-493 caught, its eleven mythicals left out, goal 482);
  CountPartyNonEggs; AdvanceEternaGymClock, PressPastoriaGymButton, PressSunyshoreGymButton on s3d_gyms.c's own.
- SUPER CONTESTS (2026-11-02, the user: "Super Contests should be added, as well as their unique functions"): the rules
  in src/sinnoh/s3d_contest.c as Platinum's code has them (docs/SUPER_CONTEST.md says where each comes from), the
  screens in s3d_contest_ui.c (480x320 like the BADGE CASE: IsActive / Step / Canvas / Frame in sdl2.c and display.c).
  Tables from the decomp when the game is built (tools/sinnoh/contest_data.py into sinnoh/platinum_scripts.inc, never
  committed): each move's contest type and effect (res/moves), the effects' appeal and messages, the computer's ACTING
  table (165 rows), contest_data.narc's 96 opponents, 12 judges, 96 dress-ups and 12 themes. The hall's scripts are
  Platinum's (scripts_contests.s: `sinnoh SINNOH_CONTEST` CC_*); the CONTEST HALL's lobby and stages and the POFFIN
  HOUSE are maps again (places.py no longer drops them; gen_maps.py now keeps every building's number and appends new
  ones). What Kai has, SINNOH uses: a win gives this game's contest ribbon of the type at the rank's level (NORMAL =
  COOL RIBBON ... MASTER = COOL RIBBON MASTER, named by Platinum's own ribbon_names words for them), entering rank N
  asks for that ribbon at level N, the party menu is the game's (special ChoosePartyMon; VAR_0x8004 kept), the save
  prompt the game's (Common_SaveGame = special SaveGame), the music the game's contest songs. The first win's
  accessory, the winner's photo (chunk CPHO, the lobby's frames: ScrCmd_0A8 shows it), the TV's CONTEST HALL
  segment. Link contests (StartBattleClient / Server with COMM_TYPE_CONTEST) answer "cancelled" until co-op contests
  are built; Platinum's POKéTCH-campaign coupons aren't gifts here (the POKéTCH is the game's key item). Test
  sinnoh_contest. JUBILIFE TV's dress-up studio (ScrCmd_0A6: CC_STUDIO) is the same dress-up screen without the
  timer; START keeps the photo (CPHO's sixth place) and Platinum's script asks its one-word title on the game's easy
  chat (SetDressUpPhotoTitle, BufferCustomMessageWord); the gallery's first frame shows it (ShowDressUpPhoto; the other
  frames held friends' photos by wireless). Test sinnoh_studio. Platinum's "choose a POKeMON" (SelectMoveTutorPokemon /
  GetSelectedPartySlot, 14 scripts) is the game's party menu (CC_PICK_*: VAR_0x8004 kept). Platinum's scripts go on
  after a Warp, on the new map (gen_scripts.py emits `sinnoh SINNOH_WARP_CONTINUE` + `goto LABEL` before the warp;
  script.c TryRunOnFrameMapScript starts LABEL there).
  The user's rules: Kai's systems stay and SINNOH uses them (tag battles = Kai's partner battle, battle_tower.c
  MP_StartPartnerBattle via DoSpecialTrainerBattle 15, with STEVEN's party pick; two trainers = battle_setup.c
  MP_StartTwoTrainerBattle; AddFreeCamera = SpawnCameraObject, which s3d.c SetupView follows; the briefcase =
  Sinnoh_StarterMenu, the pick kept in VAR_SINNOH_STARTER 0x52F8); everything else 1:1 (the build list:
  docs/SINNOH_HANDOFF.md); online-only and DS-only features closed (CheckDistributionEvent is 0). Platinum's key items
  are 662-684 (item_audit.py counts them as given by Platinum's scripts). Tests sinnoh_platinum_story,
  sinnoh_platinum_opening, sinnoh_platinum_walk, sinnoh_platinum_route201, sinnoh_platinum_tutorial,
  sinnoh_platinum_tag, sinnoh_platinum_gym, sinnoh_newgame.
- The Underground (2026-11-02, the user: "The Underground will ONLY be implemented for its Fossil-hunting features and
  Spiritomb prerequisites only - the Wireless feature is superceded by CO-OP"). src/sinnoh/s3d_underground.c: the map
  MAP_SINNOH_UNDERGROUND (Platinum's matrix 2, 480x480 tiles: places.py keeps it, MAX_MAP_DATA_SIZE 262144; the ETERNA
  CITY UNDERGROUND MAN's house too, both appended); the EXPLORER KIT (item_use.c ItemUseOutOfBattle_ExplorerKit,
  registrable) works on the region's matrix and goes down by Platinum's cell mapping (region cell -> Underground cell,
  two to one, tile 8 or 23 by parity), and from the Underground back up where the player went down (the last POKéMON
  CENTER if they never went down with it). SetWarpDestination and its kin take 16-bit x / y now (8 bits wrapped past
  127: the Underground is the first map that wide). The day's dig spots: 24 clusters of 6-11 on walls beside open
  floor (Platinum's Mining_SpawnMiningSpotsAndTraps, its sphere-dependent count left out), gone once dug that day; the
  DS's radar is a sparkle (FLDEFF_SPARKLE) on the near ones. A facing one: Platinum's "The wall is bulging slightly."
  YES/NO (data/scripts/sinnoh_places.inc Sinnoh_EventScript_UndergroundDig, field_control_avatar.c), then the digging
  screen, src/sinnoh/s3d_mining.c, 480x320 like the BADGE CASE (sdl2.c / display.c): Platinum's own art (graphics
  pack section "MINE", tools/sinnoh/build_mining.py from res/graphics/mining: the screen, the crack, the buttons, seven
  dirt layers, the tools' swing and spark, every buried object, the rotated ones turned the way their shapes say),
  Platinum's rules (mining.c: 13 x 10 cells, dirt 2 with patches of 4 and 6, 2-4 treasures, 3 the first time, by the
  Trainer ID's oddness and the NATIONAL DEX, each Plate once ever and never twice in a wall, up to 100 rock tries;
  PICKAXE -2 / -1 around and 4 off the wall, HAMMER -2 / -2 / -1 corners and 8 off, a bared rock stops the spread;
  196 to start; the shake), its words (underground_common: pinged, the first-time tutorial, everything dug up, the
  wall collapsed, "... was obtained.", "Too bad! Your Bag is full!"). The touch screen is a cursor: + Control Pad, A
  strikes, L / R / SELECT / B change tools. Treasures go straight in the BAG (Platinum's treasure case and its PC are
  the wireless side's); spheres aren't buried (their weights left out: the other treasures keep Platinum's odds among
  themselves). Each one airs the TV's UNDERGROUND TREASURES CORNER (s3d_tv.c segment 13). Kept in "<save>.region"
  chunk UNDG (the way back up, the day's dug spots, the Plates had, dug before). Platinum's scripts convert:
  the UNDERGROUND MAN gives the kit and his missions (going down and digging finish: FLAG_ENTERED_UNDERGROUND /
  FLAG_DIGGING_FOR_FOSSILS set from C through gSinnohUndergroundFlags; traps, spheres and goods he hands over are
  nothing, so his later missions can't be finished); ROARK's first-visit intro (UndergroundNPCMessage names the
  player, SetCommPlayerDir turns them, WarpOut is the game's fly_up; gen_maps.py SCENE_PEOPLE shows him by his flag);
  the vendors are never shown (gen_maps.py CLOSED_PEOPLE). SPIRITOMB: Platinum's counter (GetSpiritombCounter /
  ClearSpiritombCounter on VAR_SINNOH_SPIRITOMB 0x52F2) counts co-op friends talked to there, each once a visit
  (MP_Underground_TryTalk: A facing one); ROUTE 208's BLACK BELT and ROUTE 209's HALLOWED TOWER run Platinum's own
  scripts (StartWildBattle = BattleSetup_StartScriptedWildBattle), the written stand-in is gone (its three specials
  are Sinnoh_SpecialRetired, keeping the numbers). The Underground's song isn't made yet (SEQ_TANKOU: OREBURGH MINE's
  plays). `mp explorerkit`, `mp underground`, `mp ugspot` / `ugface`, `mp mining`, `mp minecell`, `mp checkitem`.
  Tests sinnoh_underground, sinnoh_underground_man, sinnoh_spiritomb, sinnoh_spiritomb_talk.
- Elevators in Platinum's own scripts (2026-11-02, src/sinnoh/s3d_elevator.c Sinnoh_ElevatorCall, `sinnoh
  SINNOH_ELEVATOR`): VEILSTONE's store, JUBILIFE TV, HEARTHOME's two houses, the RIBBON SYNDICATE and the VISTA
  LIGHTHOUSE run Platinum's attendants / panels / on-frame ride on Kai's elevator. Platinum's "special location" is the
  dynamic warp the door set (the floor the player came from); GetFloorsAbove is that floor's place in the elevator's
  table (top first, Platinum's FieldMenu_GetFloorsAbove counts); BufferFloorNumber Platinum's menu_entries words (0
  B1F); ShowCurrentFloor its "Current Floor" window (its own tiles at 300, gone with the floor menu: s3d_scrcmd.c
  Task_SinnohMenu); SetSpecialLocation sets the dynamic warp to that floor's door into the elevator;
  PlayElevatorAnimation is Emerald's MoveElevator (two floors' shake, up or down). gen_maps.py adds the game's own
  panel and floor menu (Sinnoh_EventScript_Elevator) only where Platinum's script doesn't convert. Test sinnoh_elevator.
- Platform lifts (2026-11-02, src/sinnoh/s3d_lift.c, Platinum's platform_lift.c): IRON ISLAND B1F's right room, B2F's
  left room, B3F, the five LEAGUE elevators up to the ELITE FOUR's rooms and the CHAMPION's, and the CHAMPION's room.
  Platinum's scripts drive them (InitPersistedMapFeaturesForPlatformLift / CheckPlatformLiftNotUsedWhenEnteredMap /
  TriggerPlatformLift = `sinnoh SINNOH_LIFT` 0 / 1 / 2, the script stopped while it rides); a 3 x 2 plate of tiles at the
  lift's height (gS3dHeightHook), its floor on arrival from where the player came in (the bottom floor's warp row), the
  same map's state kept through battles and menus. The lift's prop is the map's own, drawn at the lift's height (s3d.c
  S3D_SetLift: the props standing on the plate), or on IRON ISLAND B2F's left room Platinum's run-time one (gymprops.py:
  S3D_MODEL_IRON_ISLAND_PLATFORM_LIFT). IRON ISLAND's go 1 a frame both ways, the LEAGUE's 2 a frame and up only. After a
  ride the arrival step is over (gPlayerAvatar.runningState): the game asks a coord event again each frame while the
  last step's animation stands, Platinum doesn't, and the plate's trigger would freeze the player. `mp lift`,
  `mp collisionmap X Y W H`, `mp canstep`. Tests sinnoh_lift, sinnoh_lift_iron.
- Words (1.62.x, sinnoh-work; being replaced by Platinum's own, above): every person / sign / bg / trigger runs <map>_EventScript_Person<n> / _Sign<n> / _Bg<n>
  / _Trigger<n> when the map's scripts.inc has it (n: its place in Platinum's lists, from 1), else nothing (a trigger
  without a script isn't placed). tools/sinnoh/words.py holds the words (original writing; '|' a new page; ('nurse',)
  a healer, ('mart',) a clerk with pokemart 0, the region's badge list as JOHTO's); tools/sinnoh/gen_words.py writes
  them into the maps' scripts.inc between its markers (wrapped by the font's widths to 200 pixels, straight quotes
  made curly), then gen_maps.py gives each its script. tools/sinnoh/who.py PLATINUM_DIR PLACE lists who and what is
  on a place and their numbers (Platinum's script entry names hint at what a spot is: a TV, a fridge). Written so far:
  TWINLEAF TOWN (and its houses), ROUTE 201, SANDGEM TOWN (houses, MART, MOéMON CENTER, the lab); every town's
  and route's people and signs (tools/sinnoh/words_outdoor.py OUTDOOR, merged into WORDS). tools/sinnoh/todo.py
  POKEPLATINUM_DIR [outdoor|indoor] lists what's left: nothing since 2026-11-01 (1362 people and signs). The
  buildings' are tools/sinnoh/words_indoor.py: KINDS (bookshelves, beds, statues, gym guides...: by Platinum's script
  entry name) and INDOOR (each map's people). gen_words.py refuses a map written twice in any of the three files.
- Story people (2026-11-01, tools/sinnoh/story_people.py): the people and trainers Platinum shows and hides with its
  story (a hide flag: 256 of them: BARRY, ROWAN and the assistant, CYNTHIA, LOOKER, the commanders and grunts in
  JUBILIFE, the WINDWORKS, ETERNA, VEILSTONE, the lakes, the HQ, MT. CORONET, SPEAR PILLAR, STARK MOUNTAIN, the
  hostages, RILEY, BUCK, PALMER...) each show while Platinum's story would have them there, judged by SINNOH's
  badges (9 = cleared), a story battle beaten or not (legends.py STORY, named in story_people.DONE) and day / night
  (POLICEMEN battle by night and only talk by day, JOGGERS the other way round). NEVER: scenes the game has no use
  for (cutscene stand-ins, blockades, Wi-Fi, the VILLA, the RESTAURANT's paired diners). legends.py writes
  src/data/sinnoh_story_people.h; s3d_legends.c Sinnoh_StoryRefresh sets or clears every flag as a SINNOH map loads
  (overworld.c, before the map's on-transition script). Their words are STORY_WORDS (merged into words.py's WORDS);
  gen_maps.py places a story person with its flag once it has a script (trainers have theirs). Platinum's gifts from
  them have LINES (gifts.py; the POKé RADAR and the BICYCLE are given once). Test sinnoh_story_people.
- The BATTLE CASTLE and BATTLE ARCADE (2026-11-01, src/sinnoh/s3d_frontier.c, Sinnoh_EventScript_CastleCounter /
  _ArcadeCounter, VAR_0x8008 the facility through the script): Platinum's ideas on the game's frontier pieces. Three
  POKéMON chosen (copies at Lv 50: the party never changes; no EGG or legend) battle sets of seven frontier trainers
  (FillFrontierTrainerParty at FRONTIER_LVL_50; the trainer's band of the 300 rises every set), HP and PP carried
  through a set, BP 3 + sets won (10 at most) for each set, the next set healed. CASTLE: CP for each win (10, +5 with
  none fainted, +2 each above half HP), spent before a battle on REST (10 CP) or BERRIES (a SITRUS BERRY each, 6 CP).
  ARCADE: the board lands on one of 18 events for the next battle (healing, a status on either side, sun / rain /
  sandstorm through the field weather the battle starts with, +3 levels either side, berries either side, a free
  win, a bonus BP); MOEMON_TEST_ARCADE=N fixes it for tests. Records in "<save>.region" chunk FRNT. Not here: their
  Frontier Brains (no data for their teams). Tests sinnoh_castle, sinnoh_arcade.
- The BATTLE HALL (2026-11-01, src/sinnoh/s3d_hall.c, Sinnoh_EventScript_HallCounter): Platinum's rules with the
  game's own pieces. One POKéMON (Lv 30+, no legend: gFrontierBannedSpecies or a catch rate of 5 or less) picks the
  type of each opponent from a list with each type's rank (17 types, FAIRY from GEN 6); ten ranks a type. The
  opponent: Platinum's level formula (BattleHall_CalcOpponentsLevel: the player's level less three square roots, plus
  the rank and half the types battled, never above the player's), IVs 8 + 2 a rank, a POKéMON of that type from the
  rank's groups (the type's POKéMON the GENERATION allows, sorted by base stat total in four: ranks 1-2 the weakest
  group ... 9-10 the strongest two), held by a frontier trainer of the rank's strength (gBattleFrontierTrainers,
  BATTLE_TYPE_FRONTIER: no EXP, no prize, a loss ends the streak). The chosen POKéMON alone in the party for the
  battle, healed after. 3 + streak/10 BP (10 at most) every 10 wins. Record (species, personality, ranks, streak,
  best) in "<save>.region" chunk HALL; another POKéMON starts a new one. Not here: Platinum's HALL MATRON (no data
  for her team). `mp hall`; test sinnoh_hall.
- The BATTLE FACTORY (2026-11-01): SINNOH's FACTORY counter (Sinnoh_EventScript_FactoryGuide) leads into the game's
  own BATTLE FACTORY (rental battles, Platinum's rules too); its lobby, pre-battle and battle rooms are maps shared by
  every region (mp_region.c MP_Region_IsSharedMap), so the player stays in SINNOH there (no PC swap) and the lobby's
  door leads back to SINNOH's FACTORY (MP_Region_SharedExit). Test sinnoh_factory.
- Fossils (2026-11-01): OREBURGH's MINING MUSEUM scientist (Sinnoh_EventScript_FossilScientist) gives one SKULL or
  ARMOR FOSSIL (flag 0x3D37; Platinum's come from the Underground, which this game hasn't) and wakes any fossil the
  game knows at Lv 20 with JOHTO's / KANTO's specials (src/qz_specials.c sFossils). Test sinnoh_fossil.
- The DISTORTION WORLD (2026-11-01, data/scripts/sinnoh_places.inc): Platinum walks it on walls and ceilings through
  its own 14,000-line system (overlay009), so here it's a scene in GIRATINA's arena, Platinum's own 3D room (a rock
  path in the void; s3d_gyms.c GYM_DISTORTION keeps steps to its rows 23-28, and field_player_avatar.c never jumps a
  ledge a SINNOH place blocks). With SPEAR PILLAR's MARS and JUPITER beaten, CYRUS there (Sinnoh_EventScript_
  DistortionEnter) is pulled through a tear; the player follows, battles CYRUS (TRAINER_SINNOH_GALACTIC_BOSS_CYRUS_
  DISTORTION_WORLD, Platinum's team, gen_trainers.py LATE_STORY: after every other SINNOH trainer so no id moves;
  flag 0x3D34), meets GIRATINA (Lv47; 0x3D36; caught also sets TURNBACK CAVE's done flag 0x3D03, else that one
  stays), and CYNTHIA takes them back to SPEAR PILLAR (0x3D35, story_people.EXTRA_DONE 'DISTORTION': CYRUS is gone
  from SPEAR PILLAR after). The arena's people: gen_maps.py EXTRA_PEOPLE (with a flag). Test sinnoh_distortion.
- Field obstacles (2026-11-01): Platinum's CUT trees, ROCK SMASH rocks and STRENGTH boulders (690) are the game's
  own (gen_maps.py OBSTACLES: EventScript_CutTree / _RockSmash / _StrengthBoulder, no flag, back each time the map
  loads as in both games). HM badges are the region's: MP_Region_FieldMoveBadge (mp_region.c) gives HOENN's badge for
  a field move (the game's order, FIELD_MOVE_CUT .. WATERFALL), and on a SINNOH map Platinum's (CUT FOREST, FLASH and
  ROCK SMASH COAL, STRENGTH MINE, SURF and DIVE FEN, FLY COBBLE, WATERFALL BEACON); the party menu, the SURF /
  WATERFALL / DIVE prompts (field_control_avatar.c) and field_move_scripts.inc (special MP_FieldMoveHasBadge) ask it.
  Test sinnoh_obstacles.
- Items on the ground (tools/sinnoh/items.py): an item ball (Platinum script 7000 + k, the k-th entry of its visible
  items' list) gets the shared script for its item (data/scripts/sinnoh_items.inc, `finditem`, written by gen_maps.py)
  and keeps its Platinum "obtained" flag as a SINNOH number; a hidden item (script 8000 + k, the k-th of its hidden
  items' table) is a hidden_item bg event with flag 0x3E00 + k (ids.py's flags stay below: gen_maps.py asserts it).
  Platinum's names that differ (PARLYZ_HEAL...) are renamed; story keys stay hidden. Anything placed outside the place's
  map (Platinum lists some past a place's edge) is left out. 326 item balls, 232 hidden items. Test sinnoh_items.
- Trainers (tools/sinnoh/gen_trainers.py POKEPLATINUM_DIR, then gen_maps.py): the 403 trainers standing on maps, with
  Platinum's parties (species, levels, held items, moves, battle items; res/trainers/data) as the game's trainers from
  id 1664 (FIRST_SINNOH_TRAINER, after JOHTO's / KANTO's that end at 1660; TRAINER_SINNOH_<name>,
  include/constants/trainers_sinnoh.h). "Beaten" flags from 0x3780 (TRAINER_FLAG's SINNOH branch,
  include/constants/flags.h). Classes the game has are used; 17 new from 78 (TRAINER_CLASS_SINNOH_*: WORKER,
  VETERAN, GALACTIC...; names and prize money in src/data/text/trainer_class_names_sinnoh.h), with pictures the game
  already has (Emerald's or JOHTO's). SINNOH's encounter themes are music ids 24-36 (battle_setup.c). What they say
  is written for this game: tools/sinnoh/trainer_words.py, a few lines a kind of trainer, picked by name. Each gets
  Sinnoh_EventScript_Tr<Name> in data/scripts/sinnoh_trainers.inc (trainerbattle_single / _double), its object the
  trainer type and Platinum's sight range. Trainers the story shows (a hide flag) stay hidden until their scenes are
  written (31). IS_REAL_TRAINER and Tcg_IsTrainerId know the range. Test sinnoh_trainer.
- MOéMON CENTERs and MARTs everywhere: gen_words.py POKEPLATINUM_DIR gives every nurse ('nurse',) and every MART's
  clerk ('mart',) on maps words.py doesn't cover (roles()). tools/sinnoh/gen_heal.py (after gen_maps.py): a heal
  location a CENTER (18), in front of its door in the town, appended to src/data/heal_locations.json after JOHTO's /
  KANTO's, and each CENTER 1F's OnTransition sets it (setrespawn). Test sinnoh_whiteout.
- What the MARTs sell (tools/sinnoh/gen_marts.py POKEPLATINUM_DIR, before gen_words.py): Platinum's common clerk list
  with the badges each item waits for (src/data/sinnoh_marts.h; shop.c MP_ShopItems builds it for `pokemart 0` on a
  SINNOH map by the region's badges) and 20 specialty lists (data/scripts/sinnoh_marts.inc, Sinnoh_Mart_<Id>: a town's
  own goods, the VEILSTONE store's counters, the ETERNA herb shop). gen_words.py roles() reads each vendor's Platinum
  script (gen_marts.vendor_kind) for ('mart',) or ('martlist', label). Items the game lacks (Platinum's mail) are left
  out. `mp shopdump` prints "shop here:" with the list. Test sinnoh_marts. Newer items by GENERATION as in the
  other regions (shop.c sMpShopExtras' SINNOH rows): VEILSTONE's store by clerk as LILYCOVE's, and each town mart's
  common clerk a few that suit the town from SINNOH's badges (MP_COMMON: only the `pokemart 0` clerk). PICKUP, held
  items and Platinum's own field, hidden and trainers' items need nothing more (all Gen 4 or region-free).
- SINNOH by GENERATION: tools/moemon_gen59/gen_hoenn_modern.py (after gen_wild.py / gen_trainers.py) also writes
  src/data/sinnoh_modern.h, HOENN's rules with generations 5-9 (Platinum's tables are Gen 4 already; GEN 3 and 4 show
  them as they are), and a slot whose POKéMON is wild nowhere else in SINNOH stays. mp_region.c ModernWild /
  HoennModernTeam take the table and region. Test sinnoh_gens.
- Gym leaders (tools/sinnoh/words.py GYMS in Platinum's order: MAYLENE 3rd, WAKE 4th, FANTINA 5th, CANALAVE's BYRON 6th;
  the ('gym', n) kind; gen_words.py gym()): the eight leaders, the ELITE FOUR
  and the CHAMPION are trainers too (gen_trainers.py SPECIAL, ids after the route trainers; pictures the closest the
  game has). A leader battles (NO_MUSIC: SINNOH's gym leader theme plays by class), then gives the badge (flag
  0x3320 + n) and its TM (flag 0x3770 + n: ROARK TM76, GARDENIA TM86, FANTINA TM65, MAYLENE TM60, WAKE TM55,
  BYRON TM91, CANDICE TM72, VOLKNER TM57). Their words are written for this game; badges promise nothing about
  HMs (this game has no HM restrictions). Test sinnoh_gym (MOEMON_DEBUG_REGION=3: levels are per region).
  Co-op: SINNOH's trainers can be joined (IS_REAL_TRAINER knows their range), and the leaders are in mp_coop.c
  sQzGymLeaders (badge FLAG_SINNOH_BADGE01 + n, the gym script's _Defeated), so a friend who joins gets the badge
  and the TM too. Test sinnoh_coop_gym (a test that starts in SINNOH begins in TWINLEAF's house: warp after start).
- The LEAGUE (words.py ELITE / CHAMPION, the ('e4', n) and ('champion',) kinds): its rooms connect through their own
  elevator rooms (those have doors). Each of the ELITE FOUR battles when talked to; the CHAMPION waits until all four
  are beaten (their trainer flags), then battles; the win runs Quetzal_EventScript_835F6E4 (the region cleared, a
  save), sets the respawn to the LEAGUE's CENTER and plays the HALL OF FAME and credits (special GameClear), as
  JOHTO's and KANTO's do. `mp foehp HP` sets every foe's HP in a battle (tests; a foe's healing items undo it).
  Test sinnoh_champion.
- The boat (2026-11-01): SINNOH is on the boat's list (QzGetReachableRegionsMask_: a cleared region and the S.S.
  TICKET). With SINNOH on the list or the boat leaving it, Quetzal_EventScript_840F954 goes to
  EventScript_RegionPickAll (data/scripts/mp_region.inc): the other three regions' ports, MULTI_REGIONS_FROM_*
  (the first two in the order the menus without SINNOH had; MP_RegionBoatPick maps the choice back), then customs
  and QzTravelToRegion as before. SINNOH's port is CANALAVE's dock (11, 22, beside the harbor sailor, whose
  ('script', 'EventScript_SinnohRegionBoat') sails out). The first visit (the user's call: CANALAVE's GYM is the
  sixth, so the journey has to start in TWINLEAF): MP_Region_GoToStartHouse lands the player at the dock with
  VAR_SINNOH_ARRIVAL (0x52F0) 1; CANALAVE's ON_FRAME table runs Sinnoh_CanalaveCity_EventScript_Arrival (hand
  written above the generated words in its scripts.inc): the harbor sailor greets them and sails them to TWINLEAF's
  house (2F, 8, 5), VAR 2; HEAL_LOCATION_SINNOH_TWINLEAF_TOWN (gen_heal.py) is where they come back to until a
  CENTER. The house's mother is their host (words.py). Test region_trip_hoenn_sinnoh.
- Boats inside SINNOH: ('boat', [maps]) sailors in CANALAVE (IRON ISLAND, FULLMOON, NEWMOON), SNOWPOINT and the
  BATTLE ZONE, and back. Test sinnoh_boat.
- Flags (moved 2026-11-01: JOHTO's / KANTO's trainers' "beaten" flags are 0x3000-0x331C): SINNOH's badges
  0x3320-0x3327, ids.py's flags 0x3328-0x376F, gym TMs given 0x3770-0x3777, trainers 0x3780-0x3DFF, hidden items 0x3E00-0x3FFE, FLAG_SINNOH_HIDDEN
  0x3FFF.
- Behaviors (2026-11-01): a SINNOH block's metatile id is S3D_METATILE_BEHAVIOR (0x300) + the game's behavior
  (build_pack.py BEHAVIOR: Platinum's common behaviors share the game's numbers: grass, cave floors with
  encounters, water, waterfalls, ice, sand, one-way tiles, ledges, forced walking, arrow warps, counters, PCs, TVs,
  bookshelves...), read back by fieldmap.c GetMetatileAttributesById on SINNOH maps; field_camera.c draws those ids
  as metatile 1 (the 3D engine draws SINNOH). ROCK CLIMB walls are walkable; waterfalls are surfable.
- Warps keep their numbers: a warp to a place's N-th Platinum warp goes to that warp's number after the dropped
  ones (gen_maps.py kept_warps); warps into elevator rooms (no doors of their own, Platinum runs them by script)
  are left out until elevators are written.
- Reachability: tools/sinnoh/reach.py POKEPLATINUM_DIR [MAP...] lists the people and items no free tile reaches on
  their map; `--from MAP X Y` lists the maps a player gets to from there (doors and map edges; water walkable).
  From CANALAVE's dock: 327 of 499 (2026-11-01).
- (Superseded) Tiles: SINNOH's layouts use the General tileset and gTileset_Johto_GoldenrodUndergroundEntrances (only for behaviors:
  the 3D engine draws the picture): Platinum's table / PC / TV behaviors (0x80 / 0x83 / 0x86, the game's MB_COUNTER /
  MB_PC / MB_TELEVISION numbers) get that secondary's metatiles (build_pack.py BEHAVIOR_METATILE), so a nurse or clerk
  behind a counter is talked to across it. Test sinnoh_words.
- People as Platinum draws them: tools/sinnoh/build_gfx.py (make_pack.sh runs it; fetch_decomp.sh PARTS 3 fetches
  res/graphics/field_sprites and the trainers' front.png) writes sinnoh/platinum_gfx.pak beside the 3D pack, never
  committed, shipped by package.sh: 151 overworld sheets (Platinum's 16 walking frames as the game's 9; the nurse's bow
  as frame 9 for sAnimTable_Nurse; still 32x32 / 16x16 objects) and 105 class battle pictures (80x80, frame 0). The
  committed side is names only: include/constants/sinnoh_gfx.h (`--names`: OBJ_EVENT_GFX_SINNOH_* = 0x400 + n: object events keep 12 bits of an id,
  SINNOH_PIC_*), src/data/sinnoh_gfx_standins.h (each one's stand-in, sprites.py standin()) and
  src/data/sinnoh_trainer_pics.h (gen_trainers.py: each trainer's class picture). event_object_movement.c
  SinnohGraphicsInfo builds a graphics info from the pack (palette tag 0xB800 + n, loaded like JOHTO's / KANTO's by
  QzLoadObjectPalette); without the pack the stand-in. Battles: src/sinnoh/s3d_gfx.c S3D_Gfx_BattleTrainerPic, called by
  the opponent and link opponent controllers after the stand-in's sprite is made, loads Platinum's colors into its
  palette slot and draws the 80x80 picture whole with mpBig (feet at the game's height). Breakable rocks, CUT trees,
  boulders and item balls keep the game's own (their animations); the big legendaries and the grunts in a row aren't
  taken yet. tools/sinnoh/png8.py reads the PNGs without Pillow. Test sinnoh_pics. Anything that goes by a person's
  look (the card games' who-plays-what, tcg_field.c Tcg_LookGfx and sPicOfGfx) reads a Platinum sprite as its stand-in;
  a trainer's card challenge screen shows Platinum's picture cut to 64x64 (S3D_Gfx_TrainerPicPixels; test
  sinnoh_card_pic).
- Kai's own features in SINNOH: achievements (mp_achieve.c C_SINNOH: arriving, each badge, all eight, CHAMPION, the
  lake trio, GIRATINA, DIALGA and PALKIA, HEATRAN, REGIGIGAS, CRESSELIA, ARCEUS; WORLD TOUR / GRAND CHAMPION for all
  four regions: AK_STARTED / AK_CLEARED arg 0xFF; test ach_sinnoh); the MOéMON GARDEN by quick travel as from JOHTO
  (its escalator back to where the player left: test garden_sinnoh); co-op gyms (sinnoh_coop_gym), followers and
  their SPRITE SCALING, MoeGatcha (region-free) as everywhere.
- Legendaries on the map (tools/sinnoh/legends.py before gen_maps.py; src/sinnoh/s3d_legends.c): Platinum's standing
  ones where it stands them, with Emerald's static encounter script each (data/scripts/sinnoh_legends.inc) and a
  "done" flag 0x3D00 + n (caught or beaten). Their maps' OnTransition (special Sinnoh_LegendsRefresh) shows one unless
  done, past the GENERATION or not yet: UXIE (ACUITY CAVERN), AZELF (VALOR), MESPRIT (VERITY) and GIRATINA (TURNBACK
  CAVE) from the 7th BADGE (Platinum gives them in its story then); HEATRAN, REGIGIGAS (Lv. 1, as Platinum), CRESSELIA,
  DARKRAI, SHAYMIN and ARCEUS after the LEAGUE. Their sprites are Platinum's (build_gfx.py: 32-wide sheets' and the
  64-wide big ones' first frame, kind 4: 64x64), JOHTO's / KANTO's GBA ones (Quetzal's QZ_736-749) as the Gen 3 look
  and stand-in. DIALGA and PALKIA (Platinum brings them by script) stand at the top of SPEAR PILLAR after the LEAGUE:
  objects this game adds (legends.py ('at', x, z), gen_maps.py; hide flags 0x3D80 + n). Not yet: the REGIS' ruins,
  ROTOM. Tests sinnoh_legends, sinnoh_legends_pillar.
- The GREAT MARSH (2026-11-01): Platinum's six marsh headers share matrix 240, one block each, so the game keeps the
  player on GREAT_MARSH6's map for the whole marsh: gen_maps.py marsh_merge gives it the other five's people, item balls,
  signs and hidden items; the wild POKéMON are the block's underfoot (wild_encounter.c GetCurrentMapWildMonHeaderId ->
  s3d_legends.c Sinnoh_GreatMarshWildMap; the border counts as the nearest area). The SAFARI GAME is Emerald's
  (safari_zone.c: 30 SAFARI BALLS, 500 steps): PASTORIA's gate counter (the gate's Trigger1, ¥500) starts it, and
  data/scripts/safari_zone.inc ends it at the gate in SINNOH (special Sinnoh_InGreatMarsh: RETIRE, time's up, out of
  balls); walking out through the marsh's door asks (VAR_SAFARI_ZONE_STATE 3, the gate's ON_TRANSITION). Platinum's
  daily POKéMON: each area one of 32 picks a day (day + Trainer ID) in the 5% slots 6-7 while the SAFARI GAME is on,
  the second list after SINNOH's LEAGUE (Sinnoh_GreatMarshDaily; `mp marshdaily`). The marsh is 128x160 tiles:
  MAX_MAP_DATA_SIZE is 32768 (fieldmap.h; a map past it gets no grid at all: no walls, grass or warps). Test sinnoh_marsh.
- Small Platinum commands on Kai's systems (2026-11-02, src/sinnoh/s3d_misc.c, `sinnoh SINNOH_MISC, op` 61, chunk SMSC
  in "<save>.region"): JUBILIFE TV's lottery (GetJubilifeLotteryTrainerID is the game's own lottery number, the one
  HOENN's LOTTERY CORNER draws; CheckForJubilifeLotteryWinner Platinum's digit match over the party and the save's
  boxes; BufferVarPaddingDigits, BufferMonNicknameFromPC), the VEILSTONE DEPARTMENT STORE's regular (5 buys there:
  shop.c Sinnoh_StoreBought). AddToGameRecord is dropped (Platinum shows its records only in the closed rankings
  rooms). Test sinnoh_lottery.
- BERRY patches (2026-11-02, src/mp/mp_berrytrees.c): Platinum's 118 soil patches (OBJ_EVENT_GFX_BERRY_SOIL, its
  patch number in data[0]) are the game's own BERRY trees (BerryTreeScript, MOVEMENT_TYPE_BERRY_TREE_GROWTH: what Kai
  has, SINNOH uses, so the WAILMER PAIL waters them), numbered 128 + Platinum's patch number (gen_maps.py
  SINNOH_BERRY_TREES). The save's berryTrees[128] is full, so trees 128-255 live in "<save>.region" chunk BTRE
  (berry.c GetBerryTreeInfo; BerryTreeTimeUpdate grows both sets on the player's own clock). A new game plants
  Platinum's first BERRY in each, ripe, with Platinum's yield (include/data/berry_init.h, fetched since PARTS 17:
  gSinnohBerryInit from contest_data.py berry_lines). An empty SINNOH patch shows the soil mound (3D maps have no soil
  tile: event_object_movement.c). Test sinnoh_berry. Note: JOHTO's Quetzal trees (90-109) overlap the GARDEN farm's
  100-119 (docs/REGIONS_TODO.md, JOHTO).
- The roamers (2026-11-02, src/sinnoh/s3d_roamer.c from Platinum's roaming_pokemon.c, TryEncounterRoamer and
  roamer_after_battle.c): six slots in Platinum's ROAMING_SLOT_* order (MESPRIT 50, CRESSELIA 50, DARKRAI 40, unused as
  Platinum's, MOLTRES / ZAPDOS / ARTICUNO 60), set going by Platinum's own scripts (`sinnoh SINNOH_ROAMER, slot`, op 60):
  VERITY CAVERN's MESPRIT and FULLMOON ISLAND's CRESSELIA (their objects keep legends.py's rows and flags but run
  Platinum's script: legends.ROAMERS / platinum_people(); DrawPokemonPreview is showmonpic, FlickerObject an unrolled
  remove / add) and PROF. OAK in ETERNA (story_people.PLATINUM: his Platinum script, the UPGRADE and the birds; his
  gifts.py row keeps its flag number, unused). Each map change in SINNOH moves them (one in 16 anywhere on Platinum's
  29 routes, else to Platinum's nearby routes, never onto the map just left), FLY puts them anywhere; a wild encounter
  on a roamer's route is it half the time (roamer.c TryStartRoamerEncounter, the repel by RoamerEncounterLevel()); the
  game's roamer battle (it flees); after it battle_main.c Sinnoh_RoamerAfterBattle keeps its HP and status, or sets
  Platinum's VAR_ROAMING_*_STATE CAPTURED / DEFEATED (gSinnohRoamerStateVars from contest_data.py) and every roamer on
  that route moves; another wild battle there moves them 30% of the time. GameClear in SINNOH is Platinum's HALL OF
  FAME reset (DEFEATED -> RESET, MESPRIT / CRESSELIA back: Sinnoh_LegendDone). SINNOH NOW's segment 15, the ROAMER NEWS
  FLASH. Kept in "<save>.region" chunk ROAM. HOENN's LATIOS / LATIAS stay roamer.c's. `mp roamers [SLOT | meet]`.
  Test sinnoh_roamer.
- The GREAT MARSH's quick tram (2026-11-02, src/sinnoh/s3d_tram.c from Platinum's great_marsh_tram.c): the six
  stations (Platinum's safari_game.s entries 4-9 on the merged marsh's bg events: gen_words.py now converts that shared
  file's entries too) run Platinum's scripts; the car (great_marsh_train, a run-time model: gymprops.py,
  S3D_MODEL_GREAT_MARSH_TRAIN, DynProp slot 1) waits at x 66 by z 40 / 82 / 108; called it glides in with Platinum's
  speeds and slow-down points; ridden, the player (invisible in it, as Platinum's SetInvisible) walks the track faster
  then slower and the car follows (z + 8), the player kept at the platform's height (SetPlayerHeightCalculationEnabled
  FALSE: s3d_tram.c Height over the track's tiles). InitGreatMarshTram puts it back by AREAS 1 / 2; EndSafariGame is the
  game's ExitSafariMode. `mp tram`. Test sinnoh_tram.
- Elevators (2026-11-01): Platinum's run by script; here gen_maps.py elevators() finds them (a place with a warp to
  MAP_HEADER_DYNAMIC) and their floors (the places with a door into one; sorted by their names' B1F / nF, a place outside
  as 1F, anything else TOP: VISTA LIGHTHOUSE), keeps their doors as Emerald's MAP_DYNAMIC warp (SetupWarp sets it to
  the floor the player came from) and points the attendant and the panel (1,5; added where Platinum has none) at
  Sinnoh_EventScript_Elevator (data/scripts/sinnoh_elevators.inc): the floor menu (s3d_elevator.c Sinnoh_ElevatorMenu,
  SCROLL_MULTI_QZ_DYNAMIC), MoveElevator, the dynamic warp set to the floor's door (Sinnoh_ElevatorGo). Six: VEILSTONE's
  store (B1F-5F), JUBILIFE TV, the RIBBON SYNDICATE, the VISTA LIGHTHOUSE and two HEARTHOME houses (the LEAGUE's are
  corridors with doors). Test sinnoh_elevator. Gyms: tools/sinnoh/reach.py finds every leader and trainer reachable on
  foot; Platinum's moving parts move in the gyms done so far ("Gyms that move" below).
- Gyms that move (2026-11-01, src/sinnoh/s3d_gyms.c; data src/data/sinnoh_gyms.h from tools/sinnoh/gen_gyms.py):
  Platinum's overlay008/gym_features.c rewritten for the game's grid. The 3D pack is version 4: a table of named
  meshes, the models Platinum's gyms load at run time (tools/sinnoh/gymprops.py: each with its gym's textures; rerun
  tools/sinnoh/make_pack.sh); s3d.c S3D_DynProp / S3D_DynPropTurned draw them in the map's matrix world, gS3dHeightHook
  gives a gym's own ground heights, S3D_TileFloor / S3D_PlayerHeight the block's. overworld.c calls S3D_Gyms_MapLoaded
  after every InitMap (a warp, a continue); event_object_movement.c GetCollisionAtCoords asks S3D_Gyms_StepBlocked
  for each step; S3D_Update calls S3D_Gyms_Update (a step onto a button starts its script: data/scripts/sinnoh_places.inc).
  PASTORIA: the water floor at 0 / 32 / 64 (VAR_SINNOH_PASTORIA 0x52F3, Platinum's start the middle); in the pool
  (tiles 1-25 x 2-39) the ground is the floor, or the water where it stands higher and is nearer (Platinum's plate);
  deep water (0x59) where the water is the ground blocks, a step between heights 20+ apart blocks, the gates (0x56-0x58)
  open at their level (grid impassability); ten buttons (orange low, green middle, blue high) move it a unit a frame.
  SUNYSHORE: three rooms of gears (Platinum's gear tables and collision regions, read from gym_features.c), a room's
  quarter turn VAR_SINNOH_SUNYSHORE 0x52F4 (its own start 2 / 1 / 0, or 0 from its entrance: the save's pos at load);
  buttons turn every gear a quarter, back one or a half (5.625 degrees a frame), upright gears about the east-west axis.
  CANALAVE: four floors 160 units apart (the three above Platinum's run-time floor props, shown once the player is
  up to them), 24 platforms (Platinum's paths: a lift to its other floor, a sideways one carries the player along to its
  other end tile by tile, MOVEMENT_ACTION_WALK_FAST); their state VAR_SINNOH_CANALAVE_LO / _HI 0x52F5-0x52F6 (moved
  from Platinum's start), the player's floor VAR_SINNOH_CANALAVE_FLOOR 0x52F7 (both reset from the door). The map's
  own walls are lifted: each floor's walls are Platinum's tables (sCanalaveWalls bits); people stand on their floors
  (gS3dObjectHeightHook by local id, hidden above the player's floor) and trainers see only their own floor
  (trainer_see.c S3D_Gyms_SeesPlayer). gen_gyms.py walks the gym as the game does and stops if anyone is out of reach.
  `mp gym [solve]`: the state, and a walk from the door by every step and button with the real collision (who and
  which doors are reached, after how many buttons). Tests sinnoh_gym_pastoria, sinnoh_gym_sunyshore, sinnoh_gym_canalave,
  sinnoh_gym_eterna.
  ETERNA: the flower clock's hands (Platinum's angle math: the minute hand a full turn per trainer beaten, the hour
  hand a quarter, 4 minutes a frame the shortest way) lift and lower walls in the clock's area (x5-17 y7-19 and the
  fountain row y19); the state is the count of the gym's three trainers beaten (HasTrainerBeenFought) or 4 with the
  badge, so it needs no var; the hour hand's tip is a ledge jump (field_player_avatar.c S3D_Gyms_JumpAt). A change of
  state runs Sinnoh_EventScript_EternaClock (the special Sinnoh_EternaClock turns the hands while the player waits).
  gen_gyms.py eterna_check walks each state from the door and stops if the next trainer is out of reach.
  VEILSTONE: nine punching bags on tracks and eleven tire stacks (Platinum's tables, each object two tiles north of
  its table spot as Platinum places it; models from Platinum's field effects: gymprops.py "fx:" names, numbered
  0x8000 +, fetch_decomp.sh PARTS 6). Walking into a bag hits it (field_player_avatar.c S3D_Gyms_PushAt ->
  Sinnoh_EventScript_VeilstoneBag): it slides the way it was hit by Platinum's track table (sVeilstoneTrack: 1 off the
  track, 2 / 3 / 5 not that way, 4 a stop) at Platinum's speeds, swinging from its top, and knocks over a stack in its
  way. Bags and stacks block steps and trainers' sight; their state is kept in RAM for the map (a battle keeps it,
  coming in from elsewhere resets it, as Platinum's). gen_gyms.py veilstone_check pushes bags every way and stops if
  anyone is out of reach. `mp gym` prints the bags and stacks; test sinnoh_gym_veilstone.
  SNOWPOINT: Platinum's ice bowl. A slide's speed (0-3, field_player_avatar.c SnowpointSlip in place of the plain
  ice slip on this map) goes up a step down a slope and down a step up one; with none left, or blocked going up, the
  player slides back down facing on up the slope (Platinum's PlayerAvatar_TileMove_Ice). Slopes are Platinum's look
  half a step ahead on its floor plates (gen_gyms.py snow_slopes -> sSnowpointSlope; build_pack.py heights() takes an
  offset), not tile centres: the ice between two stair tiles is level. Its 19 snowballs (Platinum's objects, hidden by
  gen_maps: drawn here with Platinum's model) block; a slide at speed 1+ or a bike at speed breaks one it runs into
  (CheckForObjectEventCollision -> S3D_Gyms_BreakAt), several in a row. Its fences (0x49 / 0x4A) block steps
  north-south / west-east into, out of or across them (the stairs). Broken snowballs stay broken until the player comes
  in from elsewhere. gen_gyms.py snowpoint_check walks and slides the gym as the game does, stops if anyone is out of
  reach and prints the way to CANDICE; test sinnoh_gym_snowpoint.
  HEARTHOME: Platinum's dark rooms. The 3D picture is dimmed (s3d.c S3D_SetDarkness: Platinum's fog 109 of 127 ->
  0.2, 119 in the second trainer room -> 0.12, 91 with FANTINA's badge -> 0.35) but for screen circles of light set
  every frame around the player and each trainer and ahead of them where they face (their lamps; sprites aren't
  dimmed). In each trainer room one door is right, drawn with the clue's tile (Platinum's tables of where it may lie,
  never under a person) each time the player comes in from another room (a battle keeps them); the clue is Platinum's
  hint model of the right door's symbol on the floor, found by walking the dark. A wrong door's warp is redirected to
  the entrance room's warp 1 (field_control_avatar.c SetupWarp -> S3D_Gyms_RedirectWarp); test sinnoh_gym_hearthome.
- SINNOH trainers who battle when talked to (tools/sinnoh/gen_trainers.py TALK: the ETERNA gym's three, the TRAINERS'
  SCHOOL's kids, the POKéMON MANSION's maids, the RESTAURANT's diners; new classes MAID and CAMERAMAN): gen_maps.py
  talk_trainer() gives the object whose Platinum script battles one the trainer's own script (no sight line).
- TOWN MAP and FLY (2026-11-01): tools/sinnoh/gen_townmap.py (run by gen_maps.py) gives every SINNOH map its section
  from Platinum's location label (the header's mapLabelTextID: 120 places, MAPSEC_SINNOH_FIRST 0x140, names written from
  the labels' words) and writes include/constants/region_map_sections_sinnoh.h + src/data/region_map/sinnoh_region_map.h.
  Platinum's town map grid is matrix 0's cells (res/town_map/town_map_data.json blocks), 28 x 23 rows: the game's grid
  is 28 x 15, so SINNOH is two pages, NORTH (rows 6-20) and SOUTH (rows 14-28), region_map.c QZMAP_SINNOH_NORTH /
  _SOUTH (6 / 7), L / R between them; a place off Platinum's grid sits on the cell of its door from outside. The
  picture: Platinum's top screen map (tiles + its sea-and-land and roads screens; the decomp's gray sheets keep each
  pixel's color as 15 - its gray level) scaled 8/7 onto the game's 8-pixel grid, at twice the full view's resolution,
  built into sinnoh/platinum_gfx.pak ("TOWN" after the battle pictures; build_gfx.py, fetch_decomp.sh PARTS 4) and
  drawn by the renderer's picture hook (gba_easy_draw.c gMpBgPictureLine: BG gMpBgPictureBg's texture points, so the
  region map's affine BG2 scroll / zoom and the AREA page's text BG both work). The BG's palette (gS3dTownMapPal) is a
  white, a black and a signature: the picture follows the fades through the two and stops when another palette is
  loaded. FLY spots: the places with a heal location, each unlocked by flag 0x3D40 + n set on arriving
  (s3d_townmap.c Sinnoh_TownMapArrive from MP_Region_OnMapLoaded). Met location: MP_METGAME_SINNOH (13) with the
  section's number past MAPSEC_SINNOH_FIRST (mp_region.c MP_Region_TagMet, the summary screen). Test sinnoh_townmap.
- The POKéTCH (2026-11-01; ITEM_POKETCH 660, a key item usable in every region, registrable): src/sinnoh/poketch.c.
  PÓKETCH Co.'s president (JUBILIFE, 1F) gives it. It draws over the field's picture (MP_Poketch_Composite from VDraw)
  as Platinum's 192x160 LCD between a red case with the side button, and takes the buttons while open
  (MP_Poketch_FilterKeys from ReadKeys): L / R or the side button change apps, B closes, the D-pad moves a stylus and
  A touches; a mouse click touches where it points (sdl2.c Platform_PoketchMouse, display.c Display_WindowToGame).
  A script starting (an NPC, a trainer) closes it. The art is Platinum's own (tools/sinnoh/build_poketch.py: every
  app screen from its tilemap and tiles, every sprite sheet's cells from their OAM pieces, as 4 LCD shades; the
  DIGITAL WATCH's 8x8-block digits from its 320-wide screen, kept in 32-tile blocks) in sinnoh/platinum_gfx.pak
  ("PKCH" after "TOWN"; fetch_decomp.sh PARTS 5), colored by the COLOR CHANGER's 8 LCD colors. Apps (badges to unlock):
  DIGITAL WATCH, CALCULATOR, MEMO PAD, PEDOMETER (GAME_STAT_STEPS), PARTY (icons in shades, HP; touch: the cry),
  LINK SEARCHER (co-op friends, bars by how near) 0; COUNTER, ANALOG WATCH, COIN TOSS 1; COLOR CHANGER, DOWSING
  MACHINE (hidden items within 2 tiles of the touch, 12 pixels a tile around the player) 2; STOPWATCH (touch the
  Voltorb) 3; CALENDAR (the PC's date) 4; then FRIENDSHIP CHECKER (touch: hearts by friendship) 1, DAY-CARE CHECKER
  (levels with the steps walked, the EGG) 2, POKéMON HISTORY (the last 12 caught or hatched: MP_Poketch_OnCatch from
  mp_events.c / egg_hatch.c, only with the POKéTCH in the bag) 3, MARKING MAP (6 markers to drag; in SINNOH Platinum's
  map with the player's cell, matrix cell (x, z) at (10 + 6x, 8 + 6(z - 5)): s3d.c S3D_PlayerMatrixCell; elsewhere the
  region's own town map as cells: region_map.c MP_RegionMap_PoketchGrid) 3, MOVE TESTER (gTypeEffectiveness, FAIRY
  from GEN 6) 4, MATCHUP CHECKER (daycare.c MP_Daycare_Compatibility) and ROULETTE 5, KITCHEN TIMER and ALARM CLOCK
  (they ring, flash the LCD and open the POKéTCH on their app wherever the player is: CheckRinging) 6, DOT ARTIST 7,
  BACKLIGHT 0, BERRY SEARCHER (the map's BERRY trees around the player, 6 pixels a tile; ripe ones blink) 2, TRAINER
  COUNTER (the POKé RADAR's chains) with the POKé RADAR. Badges count from any region (the most in one): the POKéTCH works the same in HOENN, JOHTO and KANTO.
  "<save>.poketch" (version 2): the app, color, pedometer start, counter, memo, dots, markers, history, alarm,
  backlight. Tests: `mp poketch APP`, `mp poketchtap X,Y`; scenarios sinnoh_poketch, poketch_regions.
  Views and controls (2026-11-01, the user's design): hidden / SMALL (3/8, bottom right, 3/4 opaque; the FRIEND
  FINDER's radar is top right) / LARGE (7/8, opaque, the field around it): the right stick (gamepad.c flicks,
  INPUT_ACTION_POKETCH_*; keyboard I / K / J / L) up / down / the apps; its click (O) the cursor mode while it shows
  (hidden: the FRIEND FINDER). Cursor mode takes the D-pad / left stick, A, B (ends it), L / R, SELECT; apps with
  buttons lock it to them (Targets, a blinking frame), the others get a free pointer. It only shows on the field with
  the player free (Available). Art: the decomp's sprite PNGs (built with their cell files) hold each object's picture
  once, stacked top to bottom in tile order a pixel apart (build_poketch.py cells(): groups); the STOPWATCH's sheet is
  a compressed NCGR (Ncgr: LZ10, its sprites counting from the shared digits' 80 tiles); double-size turning OAMs keep
  their picture in the middle; every sheet's animations ride along (S3D_Gfx_PoketchAnimCell: Platinum's playback
  modes) and each app draws Platinum's sprites where its graphics.c puts them (Spr: DS screen points, the LCD at
  16, 16): KITCHEN TIMER arms / arrows / START STOP RESET, ALARM CLOCK switch / ears / eyes / arrows, STOPWATCH
  Voltorb (tap: start / stop; held: blinks, faster, blows up: 0), ANALOG WATCH hands (CellRot), ROULETTE arrow and
  PLAY / STOP / CLEAR, DAY-CARE digits and gender marks, MATCHUP CHECKER meter / LUVDISC / button, MOVE TESTER arrows
  and marks, LINK SEARCHER waves, PARTY item / MAIL marks, FRIENDSHIP hearts, MARKING MAP markers.
- The REGIS and ROTOM (2026-11-01; Platinum's own scripts since 2026-11-02): ROCK PEAK, ICEBERG and IRON RUINS run
  scripts_*_ruins.s: after SINNOH's LEAGUE (Platinum's FLAG_GAME_COMPLETED, set on every SINNOH map load while the region
  is cleared: s3d_scrcmd.c gSinnohGameCompletedFlag), the seven dots (ActivateRegiRuinsDot = s3d_legends.c
  Sinnoh_RegiDot, Platinum's tables of each room: a bit each, all seven = 260) wake the statue (ScrCmd_29F = SINNOH_QUAKE,
  the game's ShakeCamera, VAR_0x8004-7 kept), and with REGIGIGAS in the party (CheckPartyHasFatefulEncounterRegigigas:
  any REGIGIGAS, special Sinnoh_PartyHasRegigigas; Platinum's was a DS distribution's, SNOWPOINT TEMPLE's after the
  LEAGUE stands in) the statue gives REGIROCK / REGICE / REGISTEEL Lv. 30. The OLD CHATEAU's TV is Platinum's script
  (StartWildBattle = BattleSetup_StartScriptedWildBattle): ROTOM Lv. 20 at night. Tests sinnoh_regi, sinnoh_rotom.
- UNOWN (2026-11-02): Platinum's MessageUnown prints in its UNOWN font (res/fonts/font_unown, fetch_decomp.sh parts 16,
  tools/sinnoh/build_unown.py into platinum_gfx.pak "UNWN" in the game's own latin font layout; src/text.c FONT_UNOWN,
  `sinnoh SINNOH_MESSAGE_UNOWN` puts the font's code before the text; the normal font without the pack). The UNOWN
  letters seen (pokemon.c HandleSetPokedexFlag -> s3d_legends.c Sinnoh_UnownSeen, "<save>.region" chunk UNOF) answer
  GetUnownFormsSeenCount (the RUIN MANIAC's cave off ROUTE 214 grows with them). Test sinnoh_unown.
- The story's battles (2026-11-01; tools/sinnoh/legends.py STORY, gen_trainers.py STORY_TRAINERS): BARRY (8 of
  Platinum's battles, each in its three versions: Sinnoh_RivalVersion by the Trainer ID), MARS, JUPITER, SATURN and
  CYRUS stand where Platinum's story puts them (their objects' own hide flags, shown by Sinnoh_LegendsRefresh's
  table: from their SINNOH badge count in Platinum's order, or after the LEAGUE), battle once (two in a row at SPEAR
  PILLAR and STARK MOUNTAIN) with Platinum's parties and words written for this game, then leave (done flags
  0x3D10 +). legends.py hook() adds the map's on-load line to scripts it already has. Test sinnoh_story.
- The three card games in SINNOH: its people play by their sprite's look-alike (tcg_field.c Tcg_LookGfx: the specials
  first, then sSinnohStandIns), so the townsfolk and beaten trainers duel as JOHTO's and KANTO's do. Special card
  trainers (sQzSpecials' SINNOH rows): the eight leaders by role and type (ROARK - ROXANNE, MAYLENE - BRAWLY, WAKE -
  JUAN, FANTINA - TATE & LIZA, VOLKNER - WATTSON, GARDENIA - WINONA, CANDICE - FLANNERY, BYRON - NORMAN; badge flags
  0x3320+), the ELITE FOUR (once beaten), CYNTHIA (MP_Region_Cleared), BARRY the rival (shown once his scenes are
  written), PROF. ROWAN for STEVEN, JOHTO's JASMINE (SUNYSHORE) for WALLY; SINNOH's battle songs; their challenge
  screens show Platinum's pictures (SinnohSpecialTrainer). The GAME CENTER: an attendant in VEILSTONE's PRIZE EXCHANGE
  (tools/sinnoh/gen_maps.py EXTRA_PEOPLE, EventScript_GameCenterGuide), back down to it by the stairs. TRIPLE TRIAD:
  all of SINNOH is TT_REGION_SINNOH (OPEN, ELEMENTAL, SAME WALL; LAVARIDGE's pool), protocol 69 (the world rules sent
  online have 11 regions; the .tt file keeps its count). The card guide lists SINNOH's specials. Tests
  sinnoh_card_specials, sinnoh_card_pic, sinnoh_gamecenter, sinnoh_tt.
- OPTIONS > DISPLAY > SPRITE SCALING > SINNOH PEOPLE (gMpConfig.sinnohSprites, ini sinnoh_sprites; the player's own, not the host's):
  GEN 4 Platinum's sprites; MIXED a true Gen 3 equivalent where there is one, else Platinum's sprite made smaller;
  GEN 3 Gen 3 sprites for everyone (the equivalent, else the closest stand-in). The equivalents (tools/sinnoh/sprites.py
  GEN3 -> src/data/sinnoh_gfx_gen3.h): the sprite Emerald's maps give the same trainer class (cooltrainers MAN_3 /
  WOMAN_5, BATTLE GIRL GIRL_3, GUITARIST MAN_5...), the same job (NURSE, MART_EMPLOYEE, GYM GUIDE MAN_2...), the same
  Pokemon, or the character's look from Moemon's other branch (CYNTHIA, CYRUS, MARS, PROF. OAK: MP_GFX_CHAR_FIRST + n,
  their own palette by tag 0xBA00 + n). The smaller copies are in the pack (version 2: build_gfx.py shrink(): 0.84,
  Emerald's people being 0.80-0.88 of Platinum's same ones, around the feet; a Catmull-Rom filter widened for shrinking,
  then 15 colours by k-means from the sprite's own; palette tags 0xB900 + n). event_object_movement.c SinnohGraphicsInfo
  picks; the map's people change at the next map. `mp sinnohsprites N`; test sinnoh_people.
- Wild POKéMON: tools/sinnoh/gen_wild.py POKEPLATINUM_DIR puts each kept place's base encounter table (species and
  levels from res/field/encounters) into src/data/wild_encounters.json as gSinnoh_* entries (154 places): grass 12
  slots and surfing 5 as in Emerald; fishing OLD 2 / GOOD 3 / SUPER 5 of Platinum's 5 a rod. ROUTE 201: STARLY,
  BIDOOF, KRICKETOT. Not used: the POKé RADAR's and GBA-slot tables.
- Time of day, swarms and HONEY trees (2026-11-01, src/sinnoh/s3d_wild.c; tables in src/data/sinnoh_wild_extra.h from
  gen_wild.py): grass slots 2-3 are the table's in the morning (4:00-9:59), Platinum's day ones 10:00-19:59 and night
  ones after (the game clock, the room host's online). Swarms: once the LEAGUE is beaten, the twin in SANDGEM's house
  by the LAB (data/scripts/sinnoh_wild.inc; flag 0x3D30) tells of the day's: one of Platinum's 22 swarm routes a day
  (day + Trainer ID, only routes whose swarm the GENERATION allows), slots 0-1 its swarm all day. All in
  wild_encounter.c's land slot hook Sinnoh_LandSlot (after the GREAT MARSH's dailies); a slot already changed from
  Platinum's table (the GENERATION's own POKéMON) is left alone. HONEY trees: gen_maps.py honey_tree finds Platinum's
  honey_tree prop on 21 maps (2x2 tiles; its point is the bottom-right tile) and makes its two lower tiles north-facing
  signs (Sinnoh_EventScript_HoneyTree): HONEY on it, then 6-24 hours on the player's own clock (MP_ClockShift, as BERRY
  trees) a POKéMON (Lv. 5-15) from groups A / B / C (MUNCHLAX on four trees from the Trainer ID) at Platinum's odds,
  10% nothing; the trees are a chunk HONY in "<save>.region". FLOAROMA MEADOW's POKéFAN sells HONEY, and HONEY used in
  the field anywhere draws a wild POKéMON as SWEET SCENT (item_use.c ItemUseOutOfBattle_Honey). `mp landslots`,
  `mp swarm N`, `mp honeyage N[,G]`; tests sinnoh_wild_time, sinnoh_honey, honey_use. The TROPHY GARDEN: after the
  LEAGUE, MR. BACKLOT (the POKéMON MANSION's office) tells once a day of a new POKéMON from Platinum's 16 (not one of
  the two there, allowed by the GENERATION), which lives in the garden's grass slot 6, the one before in slot 7
  (Sinnoh_TrophyGardenToday / Add; kept with the HONEY trees). Test sinnoh_trophy.
- DRIFLOON and SPIRITOMB (2026-11-01): DRIFLOON floats by the VALLEY WINDWORKS' door on Fridays from the 1st BADGE
  (legends.py row FRIDAYS 0xFE, done flag 0x3D32; its battle sets VAR_SINNOH_FRIDAY_DAY 0x52F1 through special
  Sinnoh_FridayMet and a later day clears the flag; the weekday from the day count, day 1 = 1 Jan 2000, a Saturday;
  MOEMON_TEST_FRIDAY). ROUTE 208's BLACK BELT and ROUTE 209's HALLOWED TOWER are
  Platinum's own scripts now, the count Platinum's Underground talks (see "The Underground" below; the written
  stand-in of 32 battles is gone). Tests sinnoh_drifloon, sinnoh_spiritomb.
- Gifts (2026-11-01, tools/sinnoh/gifts.py): the people who hand over an item in Platinum (its scripts'
  Common_GiveItemQuantity, followed through their GoTos) and have words of their own here give it once: their words,
  then a line of this game's (gifts.py LINES; gen_words.py stops if one is missing), the item, flag 0x3DA0 + the
  gift's place in the sorted list (65 found, 42 people with words: TMs, the OLD / GOOD / SUPER RODs, held items,
  balls...). Story people, gym leaders and items the game hasn't (the JOURNAL, COUPONS, POFFIN CASE...) are left out,
  and Platinum's conditions (a time, a POKéMON shown) aren't asked; gifts.py KEY items (the COIN CASE, the rods) skip a
  player who has one. Test sinnoh_gift.
- VEILSTONE's GAME CORNER (data/scripts/sinnoh_places.inc): its 12 machines (Platinum's bg events 1-12) play Emerald's
  slots (GetSlotMachineId, playslotmachine) and its counter sells COINS through MAUVILLE's script; the COIN CASE from
  VEILSTONE's southeast house (a gift). The PRIZE EXCHANGE trades COINS for Platinum's 19 prizes at half its prices
  (the COIN CASE holds 9999: TM68 at that) in a scrolling list (src/sinnoh/s3d_places.c Sinnoh_PrizeMenu, the
  dynamic scroll multichoice). Tests sinnoh_slots, sinnoh_prizes.
- The SINNOH BATTLE TOWER (2026-11-02, the full port: docs/SINNOH_TOWER_PORT.md): Platinum's own lobby, elevator and
  BATTLE SALON scripts (gen_scripts.py, as every SINNOH map's) and its frontier scenes, the corridors and battle rooms
  (overlay104's own script language: gen_scripts.py FrontierConverter onto the rooms' maps; the scene's actors the
  map's people, FRONTIER_ACTORS; LaunchBattleFrontierScene / ChangeScript warp and go on there, ExitFrontier back after
  the launch). Platinum's rules in src/sinnoh/s3d_tower.c (`sinnoh SINNOH_TOWER, FUNC, ARG, DESTVAR`: its BT_FUNC_*):
  its 315 frontier trainers and 951 sets (src/data/sinnoh_frontier.h, tools/sinnoh/gen_frontier.py), seven a room by
  its ranges, PALMER at battles 21 and 49 (silver / gold prints, 20 BP), level 50 copies of the chosen POKéMON (any
  level may enter; Platinum's banned list in the party menu), single, double and multi beside a SALON partner (CHERYL,
  MIRA, RILEY, MARLEY, BUCK once met on the way: their teams made for each challenge, named in their lines; the battle
  is Kai's partner battle, MP_SetupTowerPartner, shown from behind), REST (the title screen, CONTINUE in the lobby,
  the challenge resumed), the STATS JUDGE. Records in "<save>.region" chunk TOWR; the BATTLE POINTS the game's own.
  Closed: the Wi-Fi rooms and machines, the wireless multi, Battle Videos, TEALA's trophies (secret base goods).
  `mp stower [MODE ROOM]`; tests sinnoh_tower, sinnoh_tower_palmer, sinnoh_tower_rest, sinnoh_tower_double,
  sinnoh_tower_multi, sinnoh_stats_judge. The BATTLE FRONTIER's two booths trade
  BP (the frontier's, HOENN's too) for Platinum's lists: items left, TMs right (s3d_places.c Sinnoh_BpMenu /
  Sinnoh_BpBuy; the BP in the question, the scroll list covers Emerald's BP window). Test sinnoh_bp.
- JUBILIFE TV's rankings machines (2026-11-02, src/sinnoh/s3d_rankings.c; Platinum's rankings.c / rankings_machine.c):
  the GLOBAL and GROUP ranking rooms' six machines (StartRankingsMachine 0-5: the BATTLE TOWER's, POKéMON's and
  CONTESTS' records, `sinnoh SINNOH_TOWER, 220, MACHINE`), Platinum's screen on the game's windows: the records and
  EXIT, a record's ranking (the player among the six best others heard of, rank numbers shared by equal values), its
  explanation, SELECT then A to delete someone else's entry (YES / NO; one's own can't be). Platinum's records arrive by
  record mixing: here CO-OP's (mp_linkx.c sends the player's 13 entries after the mix record, protocol 74; the friends'
  are sorted in by Platinum's SortRankings), the group being the CO-OP room (its name hashed: the group seed). The
  player's own values: the BATTLE TOWER's best streaks and average (s3d_tower.c), the game's stats (catches, eggs,
  fishing, contest wins and entries, ribbons: SINNOH's contests and tower ribbons now count them too) and the POKéMON
  defeated, counted in battle_script_commands.c's faint handling. "<save>.region" chunk RANK. `mp rankings [add STAT
  VALUE NAME [GROUP]]`; tests sinnoh_rankings, sinnoh_rankings_mix.
- Platinum's party, BAG and trade commands (1.69.2, src/sinnoh/s3d_party.c Sinnoh_PartyCall, `sinnoh SINNOH_PARTY
  (63), OP, A, B`; PARTY_ARG gives the next op an extra value, and its variable for an op that answers in it): ribbons
  (Platinum's ribbon ids; the Gen 3 ones on the POKéMON, the Gen 4 ones in "<save>.region" chunk RIBN), nature, types,
  HIDDEN POWER, contest stats, moves (CANALAVE's move deleter: Emerald's move screen and MoveDeleterForgetMove), EVs,
  footprints (DR. FOOTSTEP: src/data/sinnoh_footprints.h, tools/sinnoh/gen_footprints.py), money and COINS, the
  weather. In-game trades: Platinum's four (KAZZA's ABRA, CHARAP's CHATOT, GASPAR's HAUNTER, FOPPA's MAGIKARP) as
  src/data/sinnoh_ingame_trades.h (tools/sinnoh/gen_trades.py from res/npc_trades; trade.c InGameTrade picks them in
  SINNOH; struct InGameTrade has a language now, FOPPA's German); InitNPCTrade / StartNPCTrade run the game's own trade
  scene (VAR_0x8004 / 8005 set for it and put back). The MINING MUSEUM's fossils (FindFossilAtThreshold,
  GetSpeciesFromFossil: Platinum's fossil table; the menu adds its lines by a text number in a variable:
  AddMenuEntry VAR, SINNOH_MENU_ADD reads both by VarGet). The POKéDEX's language and form detection have no pages in
  the game's POKéDEX: the commands do nothing, the MEISTER's flag still set.
- Converted menus (s3d_scrcmd.c): a line's buffers are filled when it's added (Platinum's AddMenuEntry formats then);
  more than 7 lines make a list that scrolls (Emerald's ListMenu, 6 rows, arrows; its tiles from 0x64 kept below the
  COINS box's 0x141), ShowListMenuRememberCursor keeps the scroll and row in its two variables. Platinum's CURSOR_X in
  a text is the game's CLEAR_TO (build_text.py: the PRIZE EXCHANGE's prices line up).
- The COIN CASE holds 50,000 (Platinum's; MAX_COINS / MAX_COIN_DIGITS 5 in include/constants/coins.h, every region):
  VEILSTONE's PRIZE EXCHANGE (Platinum's script and prices, GetGameCornerPrizeData) sells up to 20,000. The slot
  machine keeps its coins in an s32, its 10,000s digit shown left of the CREDIT panel only past 9,999; roulette's in
  a u16 (TASK_COINS) with a fifth credit digit (SPR_CREDIT_DIG_10000). Tests sinnoh_prize_exchange, sinnoh_slots.
- Platinum's own scripts now for CANALAVE's move deleter, the trade people, the MINING MUSEUM's researcher, the GAME
  CORNER's clerk and FLOAROMA MEADOW's HONEY seller (the stand-ins in data/scripts/sinnoh_places.inc / sinnoh_wild.inc
  are gone; the GAME CORNER's TM64: below). Tests sinnoh_move_deleter, sinnoh_npc_trade, sinnoh_fossil, sinnoh_coins_honey.
- Platinum's SHARD move tutors (ROUTE 212, the SURVIVAL AREA, SNOWPOINT): src/data/sinnoh_tutors.h from
  tools/sinnoh/gen_tutors.py (res/pokemon/move_tutors.json: 38 moves, their RED / BLUE / YELLOW / GREEN SHARD costs and
  tutor; each species' learnset "by_tutor" as a bit mask, Platinum's 493 only). CheckHasLearnableTutorMoves,
  ShowMoveTutorMoveSelectionMenu (the moves it can learn and doesn't know, then EXIT, on the converted menu: s3d_scrcmd.c
  Sinnoh_MenuBegin), CheckCanAffordMove / PayShardCost (the game's SHARDS, items 48-51), ResetMoveSlot (the scripts
  give party slot, move slot, move: SetMonMoveSlot + RemoveMonPPBonus), ShowShardCost / CloseShardCostWindow (the
  costs, Platinum's "RED SHARDS  X 2" lines, menu_entries 273-277). Test sinnoh_move_tutor. GRANDMA WILMA's DRACO
  METEOR and ROUTE 228's three (1.69.3): OpenMoveTutorMenu is Platinum's move-learning screen for one move, here the
  game's move relearner offering only it (move_relearner.c gMoveRelearnerOnlyMove; VAR_0x8004 kept and put back),
  CheckLearnedTutorMove the slot it went in or 0xFF. Tests sinnoh_draco_meteor, relearner (FALLARBOR's, unchanged).
  SNOWPOINT's man (1.69.4): Platinum's "tough words" a day are the game's EASY CHAT TRENDY SAYINGS
  (TryBufferAndUnlockRandomToughWord: easy_chat.c UnlockRandomTrendySaying, as MAUVILLE's old man; -1 when all are
  known; CheckAllToughWordsUnlocked: all 33 known). Test sinnoh_tough_word.
- The GAME CORNER's TM64 (1.69.4): Platinum's clerk gives it for ten straight bonus rounds
  (VAR_CONSECUTIVE_BONUS_ROUND_WINS, SINNOH's 0x51AA: the best of a sitting). The game's slots have no bonus game to
  chain, so the record is the BONUS hits (any 7s: BIG / REG) of one sitting at a machine (slot_machine.c
  bonusesThisSitting, kept in SlotTask_EndGame while in SINNOH); CheckBonusRoundStreak: 10 or more. Test sinnoh_tm64.
- The POKéDEX's FORMS (1.69.4, Platinum's form viewer after ROUTE 218's gate scientist: TurnOnPokedexFormDetection ->
  s3d_legends.c Sinnoh_FormViewerEnable, bit 31 of the UNOWN letters' "<save>.region" chunk UNOF): on UNOWN's page
  SELECT shows the letters seen one at a time (pokedex.c MpDexFormsInput: LEFT / RIGHT, SELECT or B back). The game's
  art has no other forms Platinum's viewer shows. The POKéDEX's foreign-language pages: left out (the user,
  2026-11-03). `mp unownsee L...`, `mp formviewer`; test dex_forms.
- The START menu's POKéMON (1.69.3): Platinum shows it once the starter is saved, so SaveChosenStarter sets
  FLAG_SYS_POKEMON_GET too (before, only a reload's SinnohStarterMenuFlag did); and in every region but HOENN it's
  shown whenever the party has someone (start_menu.c BuildNormalStartMenu). Test sinnoh_platinum_route201 (mp startmenu).
- StartTrainerBattle by a variable: each trainer the file puts in that variable (SetVar VAR, TRAINER_*; a #define
  of it too) compared in turn, each with its own defeat line (the mansion's maids; BufferTrainerName is the game's
  buffertrainername). The BATTLEGROUND's and the POKéMON CENTERs' daily trainers aren't imported yet
  (tools/sinnoh/gen_trainers.py would need them appended after every other id: beaten flags are by id).
- The POKé RADAR (2026-11-01, src/mp/mp_pokeradar.c; ITEM_POKE_RADAR 661, ITEMS_COUNT 662, the DEVON SCOPE's icon):
  SANDGEM LAB's aide gives it after the LEAGUE (flag 0x3D33). It works in any region's tall grass, not on a bike or in
  the water, with a 50-step battery: up to four patches, one at each distance 1-4, rustle (Emerald's tall grass field
  effect, every 40 frames; a shiny patch every 24 with FLDEFF_SPARKLE). field_control_avatar.c CheckStandardWildEncounter
  asks MP_PokeRadar_Step first (before the 4 steps' immunity after a battle): a step into a patch makes the battle
  (wild_encounter.c MP_RadarWildMon: a grass slot's species, SINNOH's radar ones in slots 4/5/10/11 through
  s3d_wild.c Sinnoh_RadarSlot, or the chain's; shiny made with a personality that keeps the nature); while patches
  shake no other grass has an encounter, and 12 steps without one ends it. Beaten or caught (battle_setup.c
  CB2_EndWildBattle -> MP_PokeRadar_BattleEnd) the chain grows and new patches come at once (MP_PokeRadar_Update in
  OverworldBasic); a patch brings the chain's POKéMON 68-98% (the farther, the likelier); sparkling odds
  round(65535 / (8200 - 200 c)) in 65536, chain c up to 40 (1 in 200). Another POKéMON, running, losing, leaving the
  map or a bike ends it. The POKéTCH's TRAINER COUNTER (APP_RADAR, shown with the radar in the bag) draws the chain
  and the best three (Platinum's podium screen; "<save>.poketch" version 3). `mp radar [shiny|charge|find|near D]`;
  test sinnoh_radar.
- Words: SINNOH's people, signs and story are written for this game (in its scripts, MOéMON's words like HOENN's, so
  OPTIONS > CREATURES swaps them back like any text); nothing of Platinum's text is used or shipped (decided
  2026-10-31: the earlier text pack built from the decomp's message banks is gone).
- Not yet: animated textures (prop animations are linked to models in Platinum's code, not its data), billboard props, Platinum's door
  animations, the buildings' people and signs (step 4), speed (about 6 ms a frame), Windows not built with it yet.
- Map edges (1.101.0, the user: raised trees, a black void and white under the trees at every boundary): a matrix
  cell's altitude raises its block by altitude x 8 units (Platinum's land_data.c: MAP_OBJECT_TILE_SIZE / 2), not 16;
  the filler blocks between the maps (header EVERYWHERE: trees, sea, cliffs) take the area (textures, props, lights) of
  their nearest real map (build_pack.py filler_header), since Platinum dresses every block on screen in the area of the
  map the player stands in (EVERYWHERE's own set 0 gave other trees and a white ground). Textures with part see-through
  texels (A3I5 / A5I3: shadows, glass) are drawn after the solid groups like the DS sorts them, and a see-through texel
  of a solid polygon never writes depth (S3dTex.soft): the black ground under FLOAROMA's arch was a shadow drawn first.
- Test: tools/sinnoh/test_poc.sh POKEPLATINUM_DIR [ROM] (the 3D picture against render_ref.py: mean under 8 of 765 a
  pixel, now 2.9; a house's wall; TWINLEAF to ROUTE 201; the ground's heights; with a ROM, its pack the same as the
  decomp's).

## JOHTO's music from HeartGold / SoulSilver
- music/mus_johto_*.kaisong: HGSS's songs (the user's 2SF rip, not in the repository) through Moemon Music Studio's
  command line (tools/kaisong/hgss_johto.py RIP: the track -> song table). They replace JOHTO's own MUS_JOHTO_* songs
  (towns, routes, places, encounters, HO-OH / LUGIA), which only JOHTO plays; JOHTO's songs with no HGSS counterpart
  (MT. SILVER, the UNDERGROUND, CLIFF CAVE, jingles) stay Quetzal's. KANTO keeps FireRed / LeafGreen's.
- JOHTO's own battle and victory songs, MUS_JOHTO_VS_WILD .. MUS_JOHTO_VICTORY_GYM_LEADER (666-677): sound/song_table.inc
  makes each a name of its own for the FireRed / Emerald song it stands in for (`.set`), so a .kaisong of that name
  replaces it in JOHTO only and without one it plays as before. pokemon.c MpRegionBattleBGM picks them in JOHTO (wild,
  trainer, gym leader and ELITE FOUR, CHAMPION, rival = TRAINER_CLASS_QZ_RIVAL, Team Rocket = QZ_TEAM_ROCKET /
  QZ_ROCKET_ADMIN); MP_RegionVictorySong wraps the victory songs (battle_main.c, battle_script_commands.c); battle_setup.c
  MpJohtoBeastSong gives RAIKOU, ENTEI and SUICUNE their own. After adding a song name run tools/kaisong/gen.py (the
  game's name table, src/data/mp_kaisong_names.h; the release runs it) or the game logs "no music called".
- tools/moemon_music/gen_soundfont.py skips comments in voicegroup lines (JOHTO's drum sets name their keys).
- Tests: battle_music_johto (666 from its .kaisong), battle_music_kanto (510, unchanged).

## SINNOH's music
- 127 song slots (include/constants/songs.h MUS_SINNOH_*, 678-804, between the "SINNOH's songs" markers that
  tools/kaisong/sinnoh_songs.py slots writes; sound/song_table.inc names each for an Emerald song with `.set`, as
  JOHTO's battle slots) and music/mus_sinnoh_*.kaisong from the user's Diamond / Platinum rips (sinnoh_songs.py
  convert). Night versions: src/data/sinnoh_night_songs.h (generated) pairs a day slot with <slot>_night;
  overworld.c GetLocationMusic asks S3D_MapMusic on a SINNOH map (S3D_IsSinnohGroup), night = 20:00-4:00
  (MOEMON_S3D_HOUR in tests). Battles on a SINNOH map: pokemon.c MpSinnohBattleBGM, victories MP_RegionVictorySong.
- Tests sinnoh_music, sinnoh_music_night.
- 2026-11-02, from the same Platinum rip (the user's Platinum.zip; never committed, converted with
  `sinnoh_songs.py convert`'s steps): slots 805-815: the BATTLE FACTORY / HALL / CASTLE / ARCADE's own songs (were the
  TOWER's: gen_maps.py MUSIC), the VILLA's audio system (SEQ_PL_AUDIO; its piano, SEQ_PL_PIANO, is the music box slot
  `villa`), the SUPER CONTEST dances of the Master / Ultra ranks by category (s3d_contest_ui.c BeginDance) and a
  Frontier Brain beaten (for the Brains' battles, not made yet).


## TM51-TM92 (sinnoh-work)
- Diamond, Pearl and Platinum's TMs past Emerald's 50, as items 618-659 (ITEM_TM51..ITEM_TM92) beside TM01-TM50 and
  the HMs: tools/moemon_tms/gen_tm2.py writes include/constants/tms_hms_tm2.h (FOREACH_TM2; TM70 is FLASH_TM =
  MOVE_FLASH, since FLASH is also HM05), src/data/items_tm2.h, src/data/text/item_descriptions_tm2.h (written for this
  game), src/data/item_icons_tm2.h (the TM disc in the move's type colors; BUG's palette is new:
  graphics/items/icon_palettes/bug_tm_hm.pal) and src/data/pokemon/tm2_learnsets.h.
- The TM / HM index: TM01-TM50 0-49, HM01-HM08 50-57, TM51-TM92 58-99 (TMHM_INDEX / TMHM_ITEM / IS_ITEM_TMHM,
  NUM_TMHMS in include/constants/items.h; sTMHMMoves has all 100). Who learns 58-99: gTm2Learnsets (u64 a species,
  bit k = TM(51 + k)), checked by CanMonLearnTMHM / CanSpeciesLearnTMHM, so the generated TM/HM learnsets stay as they
  are. Species 1-493 from Platinum's TM lists (tools/moemon_gen4/gen4_data.json), the newer ones from their teachable
  moves (tools/moemon_gen59/species.json).
- Anything that took TMs as one block now uses those: the BAG's number ("No51"), HM checks (MP_IS_ITEM_HM), the
  daycare's egg moves, BufferTMHMMoveName, ItemIdToBattleMoveId. The TM pocket sorts by number with the HMs last
  (item.c MP_ItemNumberOrder, also mp_bag.c's tie-break) and holds 128 kinds (MP_BAG_TMHM_COUNT; the slots past the
  save block's 64 are in "<save>.bag"). MP_Gen_OfItem: 4. Test tm_new.

## The audit's batch H (1.107.0)
- Case 46: KANTO's 19 POKéMON CENTER 2Fs have the game's own attendants (Common_EventScript_UnionRoomAttendant etc.),
  Quetzal's MPLAYER talk is gone. 66: ROUTE 4's door into Quetzal's empty cave is FireRed's cliff again (the cave map stays,
  unreachable). 89: ROUTE 21's WADE fights FireRed's four Lv 27 MAGIKARP at every GENERATION (mp_region.c sFireRedTeams,
  ahead of the generation tables: case 57 can add every KANTO trainer there) and says FireRed's lines; test kanto_wade
  (`mp foeparty`). 115: INDIGO PLATEAU without its never-shown people, the ABRA teleport man gone after the HALL OF FAME
  (FLAG_QZ_0x11B1), statues readable only in the RUINS as in Crystal, no ICE PATH warning sign. 117: the 7th badge's
  RADIO TOWER news is PROF. ELM's Crystal call (ring, ElmPhoneRadioTowerRocketTakeoverText) instead of a public radio
  alert. 122: VERMILION's construction man and MACHOP on FireRed's dirt lot (171 tiles of Quetzal's market back to
  FireRed's, the MART's door at 29,17 again).
- Cases 124 and 126, on Kai's TRAINER CARD (mp_tcard.c, mp_cardscreen.c): CELADON GAME CORNER's POKéMON printer (¥50,
  NORMAL / BLACK / PINK / SEPIA: MP_TCard_PrintPhoto puts the party on the card's TEAM page, MpCardStyle.tint tints the
  icons as trainer_card.c's LoadMonIconGfx) and FOUR ISLAND's STICKER MAN (MP_TCard_StickerBragFlags /
  StickerManMet / StickerBrag: FireRed's levels for HALL OF FAME, EGGS hatched and link VICTORIES, the same story
  again gets "Hmm..."). The stickers are FireRed's art (graphics/trainer_card/frlg/stickers.png, mp_cardstickers.c) at
  the end of the TEAM page's row; MpCardView.stickers sits in what was padding (the album's records keep their size),
  "<save>.card" format 4 keeps them, friends get them as 2 bytes after the signature (older games ignore them). Test
  kanto_card_photo (`mp tcard info` prints the tint and stickers).

## The audit's batch J (1.109.0)
- **Case 45, JOHTO's leaders after the League**: see "JOHTO's gym leaders after the League" (they stay in their gyms).
- **Case 88, SAFFRON's POKéMON TRAINER FAN CLUB** is FireRed's (its scripts and 39 lines through restore_words,
  FireRed's sign): src/mp/mp_fanclub.c is FireRed's trainer_fan_club.c with its record in KANTO's vars
  (VAR_KANTO_FANCLUB_FANS = FireRed's struct: bit 7 "got first fans", fans in the high byte; _TIMER, _SCENE) and the
  four later members' flags FLAG_KANTO_HIDE_FANCLUB_* (0x1551-0x1554). The club's ON_TRANSITION calls MP_FanClub_Enter:
  before KANTO's League the four stay away; after it, FireRed's game clear (three fans, the first meeting), so saves
  past the League get it on their first visit. Fans come from battles against friends (mp_battle.c
  MP_FanClub_AfterPvp: FireRed's link battles) and leave with play time at five or more. The trainers the members talk
  about are the friends battled most (mp_tcard.c MP_TCard_PvpRival, FireRed's link records), else the rival, LT. SURGE
  and KOGA. FireRed's POKéMON JOURNAL (BRUNO) on the rack after the League. Quetzal's hint people are gone (some
  described things Kai hasn't got: DexNav, PRIMEAPE's evolution). Test kanto_fanclub.
- **Case 47, the DAY-CAREs** (Quetzal's "DAY CARE through the PC" and its incense shop are gone):
  - ROUTE 5's is FireRed's one-POKéMON DAY-CARE (its scripts and words): its own slot, daycare.c sRoute5DayCareMon,
    a step for each of the player's (ShouldEggHatch), kept in "<save>.region" as chunk R5DC; specials
    PutMonInRoute5Daycare etc. as FireRed names them. Test kanto_route5_daycare.
  - FOUR ISLAND's lady and the old man outside are FireRed's, on the game's DAY CARE pair (ROUTE 117's: one pair seen
    from every DAY CARE). Test kanto_fourisland_daycare.
  - JOHTO's is Crystal's (engine/events/daycare.asm turned into scripts, its words from common_1 / common_2): the MAN
    (who gives the ODD EGG first) and the LADY each raise one POKéMON of the pair; which slot is whose is a bit in
    VAR_JOHTO_DAYCARE_OWNERS (0x5041), kept right when slots are filled or shifted (StorePokemonInEmptyDaycareSlot,
    ShiftDaycareSlots). With an EGG the MAN waits on ROUTE 34 by the side door (MP_JohtoDaycare_Flags on both maps'
    ON_TRANSITION) and walks back in after. The two POKéMON walk in ROUTE 34's yard as Crystal's do: invisible map
    people 21 / 22 (FLAG_JOHTO_HIDE_DAYCARE_MON_1 / _2) with the POKéMON's follower picture on them
    (src/mp/mp_daycareyard.c, after the camera in overworld.c), and Crystal's lines when talked to (how they get on,
    from the game's compatibility). The couple stand facing each other in the house, Crystal's bookshelves too.
    Test johto_daycare.
  - money.c IsEnoughForCostInVar0x8005 returns u16: specialvar reads 16 bits and a bool8 left the upper byte unset on
    PC, so a DAY CARE could say "not enough money" with plenty (ROUTE 117's too).
- **Case 106, MT. SILVER and RED**: the VICTORY ROAD gate's BLACK BELT (FLAG_QZ_0x105E, worked out on every visit by
  mp_region.c MP_MtSilverOpen) lets trainers by only with JOHTO's and KANTO's 16 BADGES (Crystal's EVENT_OPENED_MT_SILVER).
  RED (Johto_SilverCaveSummit_EventScript_Red, both SUMMIT maps) says Crystal's words, fights Crystal's team
  (sCrystalTeams, custom moves, every GENERATION), gives nothing, leaves, and the credits roll (QzAfterCreditsReset);
  he's back on the next climb, as in Crystal. MT. SILVER's POKéMON CENTER has Crystal's granny again. Test johto_red.
- **Case 112**: the ELITE FOUR, LANCE (with MARY and OAK), the HALL OF FAME and CLAIR say Crystal's words on every run:
  Quetzal's "welcome back" branches on FLAG_QZ_0x11CA are gone, and their scripts with them
  (tools/quetzal_port/drop_dead_labels.py MAP: drops blocks nothing names and nothing falls into).

## The audit's batch I (1.108.0)
- Cases 25 / 71: CELADON DEPT. STORE is FireRed's: the directory, floor signs and people's lines (restore_words), one
  list per clerk with "May I help you?" (2F items + TMs, 4F POKé DOLL / mail / stones, 5F X items + vitamins) and
  no section menus; Kai's newer items ride on those lists in src/shop.c (HGSS balls at the 2F items clerk, battle and
  gimmick items at 5F's, training items and curios at the vitamins clerk); Quetzal's 3F shop and berry / herb lists are
  gone (every item still somewhere: item_audit.py).
- Case 74: FireRed's CYCLING ROAD slope (bike.c CyclingRoadSlope): on CELADON's metatiles 800-850 (FireRed's
  MB_CYCLING_ROAD_PULL_DOWN, lost in the port's behaviors) the bike rolls south unless B is held; any other way is
  ridden slowly. Test kanto_cycling_slope.
- Case 96: GOLDENROD DEPT. STORE's directory and floor signs are Crystal's, its shoppers too (2F youngster and
  gentleman, 3F super nerd and rocker, 4F bug catcher added; 4F GAMEBOY KID and 5F lass in Crystal's words), B1F's
  BURN HEAL. The roof keeps Quetzal's drawing (no binocular or doll machine art to hang Crystal's lines on).
- Case 120: VIRIDIAN's OLD MAN catches a WEEDLE to show how (battle_setup.c MP_KantoCatchingTutorial, FireRed's back
  picture from tools/quetzal_port/gen_oldman_back.py -> graphics/moemon/oldman_back.bin), then FireRed's advice; no
  TEACHY TV (the user's call). Test kanto_oldman_catch.
- Case 156: JOHTO's SAFARI ZONE gate clerks: the weather clerk sells the terrain seeds he talks about; the modern items
  clerk has GENERATION 5's held items and sells nothing below GENERATION 5 (special MP_Gen_CapNow).

## TM93-TM139 (1.106.0: RED / BLUE's and Crystal's TMs)
- The user's rule: a TM number keeps its Emerald / Platinum move; a RED / BLUE or Crystal TM with another move on the
  same number gets a new slot after the last TM; a move that is a TM or HM here already (on any number: SWORDS DANCE,
  ROCK SMASH = HM06, PSYCH UP, ENDURE, SWAGGER, SLEEP TALK, DREAM EATER, THUNDER WAVE, EXPLOSION, ROCK SLIDE,
  SUBSTITUTE) gets none. 47 new TMs, items 697-743 (ITEM_TM93..ITEM_TM139), in Gen 1 then Gen 2 order by number:
  tools/moemon_tms/gen_tm3.py (`python3 -I tools/moemon_tms/gen_tm3.py POKEAPI_CSV_DIR POKERED_DIR POKECRYSTAL_DIR`)
  writes include/constants/tms_hms_tm3.h (FOREACH_TM3), src/data/items_tm3.h, item_descriptions_tm3.h, item_icons_tm3.h
  (CURSE's disc in GHOST's colors: the ??? type has none) and src/data/pokemon/tm3_learnsets.h (gTm3Learnsets, u64,
  bit k = TM(93 + k): RED / BLUE / Crystal's TM lists, Emerald's tutor lists and PokeAPI's teachable moves).
- The index: TM93-TM139 are 103-149 after ROCK CLIMB / DEFOG / WHIRLPOOL (TMHM_INDEX_TM3); TMHM_INDEX / TMHM_ITEM /
  IS_ITEM_TMHM take the new range first; the BAG shows three digits from 100; the TM pocket holds 160 kinds
  (MP_BAG_TMHM_COUNT) and sorts TM93 on after TM92, the HMs last. MP_Gen_OfItem: 0 (every GENERATION).
- Where (items stay in their region, so each region has its own): JOHTO, Crystal's places: the leaders (FALKNER TM122
  MUD-SLAP, BUGSY TM138 FURY CUTTER, CHUCK TM94 DYNAMICPUNCH, PRYCE TM111 ICY WIND, CLAIR TM118 DRAGONBREATH at the
  DRAGON SHRINE; their words name the new numbers: crystal_words.py renumber_tms), ILEX FOREST's HEADBUTT man (TM96,
  FLAG_JOHTO_GOT_TM_HEADBUTT; then GOLDENROD 5F sells it too), ROUTE 34's ILEX gate teacher (TM108 SWEET SCENT), ROUTE
  31's sleepy man for KENYA's MAIL (TM139 NIGHTMARE), MOOMOO FARM for healing MILTANK with seven BERRIES (TM109 SNORE;
  VAR_QZ_0x501B counts 0-4, 6, 7 and 5 is healed, so Quetzal-era saves stay healed), GOLDENROD DEPT. STORE 5F (TM132,
  TM136, TM123; shop.c sMpBaseLists, HEADBUTT added in MP_ShopItems), balls (ROUTE 35 TM99 ROLLOUT, UNION CAVE B1F TM128
  SWIFT, MT. MORTAR 2F TM130 DEFENSE CURL with Crystal's other five balls, LAKE OF RAGE TM134 DETECT, DARK CAVE TM109
  SNORE). KANTO: CELADON DEPT. STORE 2F's TMs clerk sells every RED / BLUE TM that got a slot plus Crystal's two KANTO
  ones, CURSE and ZAP CANNON (sMpKantoOldTms). fix_item_balls.py leaves TM balls alone.
- Tests tm_gen12 (the pocket, teaching), johto_tm_gifts, shop_lists_gen3 / gen9 (tools/test/check_shop_lists.py);
  tools/moemon_gen59/item_audit.py counts them.

## The TRAINER CARD, the BADGE CASE (1.65.0; one card for every region since 1.66.0)

- **Records** (`src/mp/mp_tcard.c`, `include/mp_tcard.h`): "<save>.card" (format 3; MP_TCard_Forget on a save file
  switch; format 2 files are read and get the default look). The save never changes: a region's badges are its flags
  (mp_region.c MP_Region_BadgeFlag). A badge flag seen set for the first time is a badge just won, whatever script set
  it (MP_TCard_Update, twice a second on the map): its date (the PC's clock), the lead POKéMON, the friends of the co-op
  battle that ended within 10 minutes before (captured while MP_Coop_SeatCount() > 1). Badges had when the file is
  first made are "before records". Format 3 adds the card's look (struct MpCardStyle), the showcase POKéMON
  (MpCardShowcase; its pad byte = chosen on MY CARD, else the party's first is shown), the signature (192x64, a bit a
  pixel, row by row, the lowest bit leftmost) and the play time in each region (counted once a second, saved every
  5 minutes).
- **Polish** is Platinum's (pret/pokeplatinum include/trainer_case.h, trainer_case_badge_dirt.c, applications/
  trainer_case/main.c): 140 when won, 10 less a day (the PC's own local date, never the room host's shared clock:
  MP_ClockShift's rule), FILTHY < 100 <= DIRTY < 140 <= CLEAN < 170 <= 2 SPARKLES < 190 <= 4 SPARKLES <= 199;
  Platinum's rubs per point by level {1, 3, 4, 15, 15}. OPTIONS > GAME > BADGE FADING PLATINUM / SLOW (a third: 10 every
  3 days) / OFF, per save (in the card file). Tests: `mp tcard days N`, `mp tcard rub R B N`, `mp tcard style D A F P H`.
- **The BADGE CASE** (`src/mp/mp_badgecase.c`): SELECT on the card. A 480x320 picture of its own like a VS fight's
  (display.c: MP_BadgeCase_Canvas after Fight_Canvas; sdl2.c ScreenStep: MP_BadgeCase_Step, the game waits), drawn
  with tcg_gfx at scale 2 for the case and text and scale 1 for the badges. Badge art: PokeAPI's official artwork at a
  pinned commit, its white stroke taken off (`tools/moemon_badges/build_badges.py` -> graphics/moemon/badges/
  badges.bin, built in by mp_badge_data.c's .incbin), 64 px in a tray and 128 px up close; dirt is the badge's colors
  turned toward brown plus a grime texture made from its alpha (value noise and specks), sparkles are drawn stars.
  Moving over the tray is silent; A on a badge chimes and opens it up close. Up close, A chimes (SE_NOTE_G turned by
  m4aMPlayPitchControl: an octave down when filthy to an octave up at 199) and A held while either stick goes round
  polishes: a rub every 30 degrees (Gamepad_Sticks, atan2 of the stick pushed further; the D-Pad's quarter turns and
  the mouse circling the badge count too). Each rub rumbles by how dirty it is; a new level glints, rumbles harder and
  spins the badge (its width as a cosine; an idle spin every 5 seconds from CLEAN up). Closing waits for the buttons to
  be let go. A friend's case (MP_BadgeCase_OpenView) is looked at, not polished.
- **The card** (`src/mp/mp_cardscreen.c`): one design for every region, shown over the game's card for the player's
  own card and a friend's with the add-on (trainer_card.c OpenCardScreen in Task_TrainerCard's state 7, before the
  fade: the game's card never shows; the Union Room's and the Cable Club's cards stay the game's). It closes by fading
  to black (white back to the FRONTIER PASS) and stays so (MP_CardScreen_Hide from CloseTrainerCard) while the game's
  card fades out behind it. A 480x320 picture like the BADGE CASE's: the card drawn flat into a 456x288 texture, then
  tilted in perspective as a grid of triangles (SDL_RenderGeometry) by the right stick or the mouse pointing at it
  and turned over between pages; nothing else moves it (no idle sway, the holo follows only the tilt), and let go it
  settles flat (test card_still). The trainer picture is read from the game card's window (WIN_TRAINER_PIC's
  pixels and BG palette 8): every LOOK as the game draws it.
- **The designs**: Pokemon Battle Revolution's Battle Pass designs (the user's upload, docs/user_images/63.zip, as
  graphics/moemon/cards/passes/*.png; `tools/moemon_cards/build_passes.py` -> passes.bin "MPAS", built in by
  mp_pass_data.c's .incbin), decoded with stb_image and resampled to the card's size (premultiplied bilinear); a
  design named "recolorable" turns to any hue (MY CARD > HUE, a rotation of each pixel's hue). The card's color never
  follows achievements: it's the look chosen on MY CARD.
- **The grid** (the user's rule: the RENTAL PASS BLUE pass is the reference, docs/user_images/64.png): every design
  shares its elements in 340x214 pixels: a 3-pixel bevel, the BATTLE PASS label at (233..323, 9..20), the P emblem at
  (8..72, 170..208), the notch in the right edge at y 45..62. On the 228x144 card (GBA pixels) the content runs from
  x 10 to 218 (the label's right margin, both sides), the headings sit on the label's line in its lettering (LabelText:
  the game's font drawn white into a texture, then 6/5 its size from its foot, a picture pixel bolder, a one-pixel dark
  edge, light metal over darker metal, a soft shadow), the pages from y 19 to 111, and a row beside the emblem from
  x 52 (y 116..137). Panels are rounded and bevelled like the pass (a light top edge, a dark bottom one). Nothing is
  drawn over the label, the emblem or the notch; the frame follows the design's own edge (a chamfer distance from its
  alpha, so it goes round the notch). Test card_designs: the PROFILE on all 36 designs, the other pages, panels and
  frames (`mp cardscreen DESIGN PAGE PANEL ACCENT FRAME` shows the card so, held flat).
- **Pages** (L / R, the D-Pad, A for the next): PROFILE (the name on the label's line, the picture with its IDNo.,
  MONEY / MOéDEX / TIME / BADGES / STARS, the showcase POKéMON with its nickname, the title as an accent ribbon and the
  catchphrase (the profile's four words), and each region's stars beside the emblem); BADGES & STARS (each region's
  8 badges as the case shows them, its stars; UP / DOWN picks a region, SELECT opens its BADGE CASE; the region's
  details beside the emblem); RECORDS (12 game stats and the time in each region; link battles and trades beside the
  emblem); TEAM & SIGNATURE; on a friend's card YOU AND <NAME> (what you did together, counted on your own game).
  **Stars**: 0-3 a region (MP_CardView_Stars: its CHAMPION beaten, all 8 badges, all 8 at 4 sparkles).
- **MY CARD** (START on the own card): DESIGN, HUE (recolorable designs), ACCENT (12 colors: the frame, ribbons,
  cursors), FRAME (NONE / THIN / THICK / GLOW / DOUBLE), PANELS (LIGHT / DARK / GLASS / NONE), HOLO (rainbow foil, a
  streak of light and glints that follow the tilt, added light), SHOWCASE (a party POKéMON or none), TITLE, SHOW TITLE
  (also OPTIONS > GAME > CARD TITLE), CARD TEAM (the party as it is, or this team kept), SIGNATURE (a pad: A draws while
  the stick / D-Pad moves the pen or the mouse draws, L / R pen or eraser, SELECT clears) and FRIENDS' CARDS (the
  album). The look is kept in MY CARD's own copy and saved when MY CARD closes; a preview of the card beside the list.
- **Co-op** (protocol 72): LX_CARD ends with the card's add-on (MP_TCard_PackAddon, version 2: every region's badges and
  polish, the CHAMPIONs beaten, the team, the title, the gender, the look, the showcase, the records, the time in each
  region, the profile's words, the signature; at most TCARD_ADDON_MAX). A friend's card shows as they styled it. Every
  friend's card seen is kept with its picture (MP_TCard_KeepFriendCard, 16 friends): MY CARD > FRIENDS' CARDS.
- **Spectating**: friends see the card and the BADGE CASE (Platform_GetFrame: MP_BadgeCase_Frame, MP_CardScreen_Frame:
  the 480x320 canvas read back and halved, each 2x2 averaged, once a drawn frame).
- Tests: card_design (pages, MY CARD, the signature), card_designs, badge_case (the stick polishing with a virtual
  pad), card_friends (two clients), spec_card (two clients: the SPECTATE panel shows both screens).

## HMs a friend's progress made impossible to get (1.65.0)

`src/mp/mp_hmrescue.c`: four of HOENN's HMs come from a scene that happens once, by someone who then leaves: STRENGTH
(RUSTURF TUNNEL, VAR_RUSTURF_TUNNEL_STATE 4/5 -> 6), FLY (ROUTE 119's rival battle), DIVE (STEVEN's house in MOSSDEEP),
WATERFALL (WALLACE in SOOTOPOLIS). Online a friend can move the story past it (a smashed boulder is the room's), so a
player can come back with the scene gone. Only when the scene is over for good (STRENGTH: the tunnel open; FLY: the
rival beaten; DIVE: STEVEN's house empty and the 7th badge; WATERFALL: the 8th badge), the received flag is unset and the
HM is in neither the BAG, the PC nor held by a POKéMON in the party or the boxes, the giver walks up on the map (never on
the scene's own map) and hands it over, setting the flag; the walk-up is mp_tmrefund.c's. CUT, ROCK SMASH, FLASH and SURF
have givers who stay. Debug `mp hmrescue`; tests hm_rescue, hm_rescue_carried.
A friend's smash of RUSTURF TUNNEL's boulder while the player is on the map (mp_world.c WC_OBJ_GONE) sets the rock's
flag for good but never ran TryUpdateRusturfTunnelState on the player's game (only the smasher's EventScript_SmashRock
does): the tunnel stayed shut in their story and Wanda's boyfriend kept waiting (1.87.1). TunnelCatchUp (every half
second on that map) does it once the player's own story is at state 3 (the AQUA grunt's scene done), a rock is gone and
they're in sight of the spot: the map's ON_FRAME scene plays and hands STRENGTH over. Test coop_rusturf.

## Platinum's time of day in the 2D regions (1.67.0)
- src/mp/mp_daytint.c: on an outdoor map outside SINNOH (IsMapTypeOutdoors; SINNOH's 3D maps light themselves), the
  map's layers (BG1-3) and the sprites at OAM priority 1-3 (their palettes: people, POKéMON, field effects) pass
  through the renderer's scene hook (gMpScenePixel, which battle weather holds in battle) and are multiplied by a
  color per half hour, eased between them. The colors: tools/sinnoh/build_daylight.py, run by make_pack.sh, from
  Platinum's outdoor area light set (the one most of matrix 0 uses): flat ground lit by its light 0 (s3d.c Light's sum)
  over noon's, so noon is the maps as drawn. Shipped as sinnoh/platinum_daylight.bin (package.sh), never committed.
  Without it, no tint. Off under the test harness unless MOEMON_DAYTINT is set (pixel-compared pictures stay); `mp
  daytint` prints it. Test day_tint.
- On SINNOH's outdoor maps the same tint goes on the sprites only (the user: the light should tint the sprites as
  well; its 3D world is lit by s3d.c): gMpSceneLayers 0 there; MOEMON_S3D_HOUR (`mp s3dhour`) sets its hour in tests.



## JOHTO's fruit trees and KURT's BALLS (1.68.0)
Crystal's fruit trees (pokecrystal engine/events/fruit_trees.asm) in Quetzal's berry trees' places
(tools/quetzal_port/fix_fruit_trees.py), src/mp/mp_apricorn.c:
- APRICORN trees: AZALEA (WHT), ROUTE 37 (RED, BLU, BLK), ROUTE 42 (PNK, GRN, YLW; Crystal's three in a row, two
  added beside Quetzal's one). One APRICORN each a day, Crystal's words (its PACK is the BAG), script
  Johto_EventScript_FruitTree (data/scripts/mp_region.inc). Drawn as Crystal's fruit tree, the same all day as in
  Crystal (the user: no berries on them, no change when picked): OBJ_EVENT_GFX_FRUIT_TREE, the game's own sprite past
  the port's (data/object_events/kai_objects.h, graphics/object_events/pics/kai/fruit_tree.png from pokecrystal's
  gfx/sprites/fruit_tree.png). The day is gLocalTime.days.
- KURT (JOHTO's KURTS_HOUSE, after the SLOWPOKE WELL): Crystal's flow in place of Quetzal's ball shop. QzKurtChooseApricorn:
  the APRICORNs held and CANCEL, then how many (UP / DOWN one, LEFT / RIGHT ten); a day later the BALLS (RED: LEVEL,
  BLU: LURE, YLW: MOON, GRN: FRIEND, WHT: FAST, BLK: HEAVY, PNK: LOVE). Every ball the shop sold is still obtainable
  (item_audit.py).
- The other 17 trees are BERRY patches (the game's own 1x1 soil, BerryTreeScript) on tree numbers 90-99 / 120-127,
  planted once (MP_JohtoBerries_Init after a JOHTO map's transition scripts) with Crystal's BERRY in its Gen 3 name.
  Before, ten of Quetzal's trees were 100-109, the GARDEN farm's plots too.
- Vars (JOHTO's, the region sidecar): 0x52E0 picked today, 0x52E1 that day, 0x52E2-0x52E4 KURT's APRICORN, count and
  day, 0x52E5 the patches planted. Debug: mp apricorn. Test johto_apricorn.

## JOHTO's POKéGEAR (1.68.0)
src/mp/mp_pokegear.c: a screen of its own, drawn as a 2000s multifunction phone after the user's Crystal screens
(docs/user_images 65): Crystal's orange body (BG1, one solid tile) round a green backlit LCD (BG0: four windows, each
under 448 tiles: a bigger one didn't copy whole); the status bar (signal bars, the four card icons from ASCII bitmaps
with the open one inverted, the time, the battery), Crystal's double-ruled message box, soft keys ({A_BUTTON}CALL /
EXIT{B_BUTTON}); big seven-segment digits on the CLOCK (Digit()); a black FM dial with a red needle (UP / DOWN, 0.0-20.0
in 0.5 steps, a station only on its frequency). LEFT / RIGHT change cards, B off (any button on the CLOCK). CLOCK: the day and time from the game's clock (no setting). MAP:
FieldInitRegionMap, back to the POKéGEAR. PHONE: MOM (Crystal's mom.asm: where you are; her savings, YES / NO keeps,
starts or stops her saving: mp_mombank.c MP_MomBank_*) and PROF. ELM (Crystal's elm.asm lines by JOHTO badges).
RADIO (0.0-20.5): Crystal's stations (pokecrystal pokegear.asm RadioChannels: StationAt) with Crystal's own songs
and words, a line every 100 frames scrolling up the message box as Crystal's (ShowLine). In JOHTO: 4.5 OAK's POKéMON TALK
(MARY and OAK, _OPT_ words, a POKéMON of the region's grass tables as the GENERATION has them; 4-10 AM the POKéDEX SHOW:
a caught POKéMON's name, then its POKéDEX entry line by line), 7.5 POKéMON MUSIC (DJ BEN; the MARCH on Sundays,
Tuesdays, Thursdays, Saturdays, the LULLABY on the other days: gLocalTime.days % 7), 8.5 the LUCKY CHANNEL (REED; the
number is the LOTTERY CORNER's, GetLotteryNumber), 10.5 BUENA's PASSWORD (6 PM to midnight; the password is HELP, what
BUENA asks for on RADIO TOWER 2F; at midnight her goodbye, then off the air), 13.5 the UNOWN at the RUINS OF ALPH, 20.5
the EVOLUTION radio at MAHOGANY / ROUTE 43 / the LAKE OF RAGE from the red GYARADOS (VAR_QZ_0x500C >= 2) until TEAM
ROCKET takes the RADIO TOWER (VAR_QZ_0x500A 6); while they hold it (0x500A 6-9) TEAM ROCKET's broadcast is on every
JOHTO station (show ST_ROCKET under the station's name). Out of JOHTO: 16.5 PLACES AND PEOPLE (DJ LILY on a JOHTO /
KANTO trainer of the QZ table or a place), 18.5 LET'S ALL SING! (DJ FERN, the MARCH / LULLABY: Crystal's song list names
the BICYCLE song there, its show never plays it), 20.0 the POKé FLUTE (no EXPN CARD: Kai has none). The songs:
MUS_JOHTO_RADIO_* 816-826 (tools/kaisong/crystal_radio.py: pokecrystal's audio.o + ram.o linked with a small GBS
driver into banks 1-7, since a GBS player banks like MBC1 and the songs' banks are past 0x1F; built from the decomp
with rgbds 1.0.4, no ROM; the studio converts each track through an .m3u to music/mus_johto_radio_*.kaisong).
Between stations the map's music plays; it comes back when the card or screen closes. Item ITEM_POKEGEAR 693 (key, registrable): START > POKéGEAR in JOHTO
(start_menu.c MENU_ACTION_POKEGEAR, last in the enum), ItemUseOutOfBattle_Pokegear anywhere once JOHTO's LEAGUE is
beaten. MOM hands it over after her first talk (Johto_PlayersHouse1F, Crystal's words); a save past that scene gets it
in JOHTO (mp_region.c MP_Region_GiveDevices). Harness condition "pokegear", debug mp pokegear [open | tune FREQ] (FREQ in
half MHz: 9 = 4.5); tests johto_pokegear, johto_radio (8 PM, every station and place), johto_radio_morning.

## MY ROOM in JOHTO (1.68.0)
Johto_PlayersHouse2F decorated on the bedroom's logic: 12 decoration sprite slots (OBJ_EVENT_GFX_VAR_0..B,
FLAG_DECORATION_1..12), SecretBase_EventScript_SetDecorationFlags / InitDecorations on entering, the PC at (1,1) the
bedroom PC (player_pc.c turns it off with Johto_PlayersHouse2F_EventScript_TurnOffPC, not Littleroot's metatile swap).
Its list is its own: mp_region.c MP_RoomDecorations / MP_RoomDecorationPositions give JOHTO's vars 0x52D0-0x52DB as
bytes in that room (decoration.c, secret_base.c read through them); the in-use check counts both rooms
(sPlayerRoomItemsIndicesBuffer, 24). Quetzal's floor has no decoration behaviors, so the room's plain walkable floor
holds dolls and cushions (MP_RoomFloorHoldsDecor). Test johto_my_room.

## Walking partners, story people, counterpart looks, reflections (after 1.68.0)
- **Walking partners end.** Platinum's ClearHasPartner (gen_scripts.py) also runs SINNOH_MISC 24 (s3d_misc.c): it clears
  VAR_SINNOH_PARTNER_GFX and stops the one following (a facing movement); Platinum's scripts then move or remove the
  person by their own local id. A map that has its own visible copy of the partner (LAKE VERITY's BARRY, same graphics,
  a map local id) uses it: Sinnoh_PartnerUpdate (s3d_scrcmd.c MapHasOwnPartner) neither spawns nor keeps the carried
  0xF2 one. Test sinnoh_partner_ends.
- **Story people show by Platinum's flags.** gen_maps.py hides with 0x3FFF people the old written story had no script
  for; gen_scripts.py people() now gives every such person with a Platinum hide flag their flag, place and look in the
  story table (CYRUS, BARRY and the STARLY at LAKE VERITY, ROWAN on ROUTE 201, the VILLA's visitors... 44 in all).
  story_people.py's NEVER only mattered to the written story (Sinnoh_StoryRefresh is off under Platinum's).
- **16-bit sprite ids.** SET_OBJ_GFX_ID / SET_TEMPLATE_GFX_ID (global.fieldmap.h) work the id out once (an argument that
  read the object's own id came out half old, half new: DAWN's 0x4A4 became Hoenn's 164);
  VarGetObjectEventGraphicsId returns u16; QzVarGraphicsId is JOHTO's / KANTO's only. SINNOH maps start
  VAR_OBJ_GFX_ID_0 as the counterpart (Sinnoh_PlatinumMapScripts) and their VAR_0 people are VAR_0 (sprites.py; no MAY
  stand-in). Test sinnoh_counterpart_look.
- **Reflections in SINNOH** (s3d.c OverReflectiveCell): a reflection pixel shows only where its ray from the eye meets
  the ground's level on a reflective / ice cell (no land tiles' top layer hides it in 3D); SINNOH checks only the next
  tile south, as Platinum. `mp shorespots` finds test spots. Test sinnoh_reflection.
- **JOHTO's gym leaders after the League** stay in their gyms, as in Crystal (audit case 45): Quetzal's postgame MORTY,
  WHITNEY, PRYCE, JASMINE and CLAIR, who promised rematches at a FIGHTING DOJO that has none, are gone (FLAG_KAI_GONE),
  and CLAIR's "challenge the GYM LEADERS again" line with them. Test johto_leaders_after.
- **DAWN / LUCAS looks' actions** (mp_avatar.c ACT_PLATINUM): surfing, field moves and fishing use Platinum's own
  player_f / player_m sheets from the SINNOH graphics pack ("PACT", build_gfx.py player_actions(): one palette per player,
  the sheets' differently ordered colors joined), mapped onto Brendan's / May's frame layout (PlatinumFrameOf); the
  local player's palette tag is OBJ_EVENT_PAL_TAG_MP_PACK. Without the pack: the hybrid body as before. Platinum's
  watering can only faces north, so watering stays hybrid. Test look_platinum_actions.

## The card tables' choices shared with the phone (1.78.0)

- src/tcg/tcg_flow.c (include/tcg/flow.h): what a POKéMON card move asks for (the Trainers' and Pokemon Powers' steps,
  the attacks' menus and targets, the filters for spaces and cards, whether a hand card / attack / retreat can be used
  and why not, the retreat's Energy when there's nothing to choose) moved out of tcg_board.c, which keeps its screen and
  cursor and calls these; the phone drives the same steps through TcgFlow (docs/PHONE_LINK.md "The POKéMON card game on
  the phone"). Never copy them back into the board.
- Fixes the phone's test turned up, in the game too: while setting up, the Active spot is chosen first (a Basic put on
  the Bench with nothing Active could leave a player with no way on); an evolution card targets only Pokemon that can
  evolve this turn (not one put down this turn when another of the name can); Scoop Up / Super Scoop Up don't count as
  playable on your only Pokemon in play.

## YU-GI-OH! shared with the phone (1.79.0)

- The GBA games' duelists' decks as cards that can be played: src/ygo/ygo_rating.c Ygo_NpcDeckCards (it was
  ygo_field.c's GameDeckAt), used by the game's duelists and the phone's FREE DUEL.
- ygo_edo_data.c built with YGO_EDO_PAK_EXTERNAL takes EDOPro's pack from YgoEdo_SetPak (the phone app reads it from its
  files); the game keeps it built in (ygo_edo_pak.c).

## Moemon Maker, M1: quick edits on the base game

The user's own editor (docs/MOEMON_MAKER.md: the design, every decision, the build order). Hidden: the main menu's
MOéMON MAKER row appears only with `AllowSmallTweaks = 1` and "maker" typed since the start (src/maker/maker_gate.c,
text_input.c CheckTypedWords; a fifth main-menu item scrolls as MYSTERY EVENTS' does). Never mentioned in PATCH NOTES
or the README. The updater no longer has a dev channel: AllowSmallTweaks only unlocks the small tweaks.

- **The editor** is C++ in maker/ (Dear ImGui 1.91.9b-docking, maker/README.md), built into the game by the Makefile's
  MAKER_* rules, reaching the game only through include/maker_api.h (src/maker/*.c). Its own SDL window; the game's
  window is hidden meanwhile (display.c Display_SetMainWindowShown) and the game waits in CB2_MakerWaiting.
  Settings in moemon_maker.ini, panels in moemon_maker_layout.ini; the font is Liberation Sans (maker/fonts, OFL).
- **Names, not numbers**: tools/maker/gen_names.py makes include/constants/maker_names.h (every map; each map's people /
  signs / triggers by name from map.json; songs, looks, movement / trainer types, items, species, flags as
  `{"NAME", NAME}` so the compiler works out aliases) and maker_tilesets.h (each tileset's metatile count by sizeof, at
  the end of src/tilesets.c), both when the game is built (map_data_rules.mk); src/maker/maker_names.c holds them.
- **Drawing**: a map's tilesets go into the game's own BG VRAM (the screen is off) and the game's own tile animations
  run on them (tileset_anims.c TilesetAnims_InitForLayout, one step per game frame in CB2_MakerWaiting); the editor draws
  each metatile once into an atlas (field_camera.c DrawMetatile's layering: NORMAL over the 0x3014 filler). While the
  game plays (F5) the tiles go into a buffer of their own and don't animate. People: GetObjectEventGraphicsInfo and
  ObjectEvent_MakerPalette (the field's palette tags, only read). Behavior marks by the game's MetatileBehavior_Is*.
  SINNOH: by what each tile does (the 3D pack's blocks), with a 3D PREVIEW (s3d.c S3D_RenderPreview: the game's camera
  on any tile, the view's state put back).
- **The edits** (MoemonMaker/base/MAP_X.txt, one line each with the value it replaced; the format is at the top of
  src/maker/maker_overrides.c): the game reads them at start (Maker_ReadDir: the working copy MoemonMaker/ where there
  is one, else the published pack MoemonMakerPack/) and patches maps through Overworld_GetMapHeaderByGroupAndId (an
  edited header copy: people, warps, signs, triggers, connections, music) and fieldmap.c Maker_MapBlocks (tiles). An
  edit applies only while the game still has its "was"; else the game's own wins and the editor's PROBLEMS offers
  "Keep mine" / "Take the game's". Nothing for converters to skip: the overrides are never in the files they write.
  Wild slots apply after the GENERATION's pools (mp_region.c MP_Region_WildInfo -> Maker_WildInfo).
- **Added things**: people (script "nothing", "itemball" or one of the map's own by label), hidden items, signs, warps
  (after the map's own: other maps lead to them by number), connections. Item balls and hidden items get Maker flags
  (0x8000+, src/maker/maker_flags.c, "<save>.maker" by name: the save never changes layout); data/scripts/maker.inc
  Maker_EventScript_ItemBall + special Maker_ItemBallItem (the item rides in the template's berry tree / sight field).
- **Music**: MoemonMaker/music/ (mp_kaisong.c also scans it, its files first): <song>[@<place>].kaisong replaces; any
  other name is a new track, song numbers MAKER_SONG_FIRST (0x4000)... by name order, played on MUS_LITTLEROOT's BGM
  entry (m4a.c MP_Kaisong_TableIndex). A map's music edit: "music WAS NOW".
- **Editing**: maker/maker_edits.cpp (the model, undo / redo by change, Ctrl+S, recovery files in MoemonMaker/.recovery/,
  every change sent to the game at once: MakerApi_SetMapEdits), maker_mapview.cpp (MAP, TILES, INSPECTOR, PROBLEMS),
  maker_wild.cpp (ENCOUNTERS: list with chances spread over the fixed slots, and the slots), maker_music.cpp (MUSIC),
  maker_publish.cpp (the checker and PUBLISH).
- **F5** (src/maker/maker_play.c): a copy of a save (Emerald's continue-game warp to the spot) or a story point (a new
  game set up in Maker_PlayNewGame through the quick start's hook); the play's save is MoemonMaker/.play/play.sav, the
  real one is read again afterwards. Edits of the map the player is on: tiles redrawn in place, anything else by a quiet
  warp to the same spot (field_screen_effect.c DoMakerReloadWarp). F12 / closing the game's window: back to editing.
- **Publish** (src/maker/maker_publish.c, on its own thread): GitHub's git data API with the user's token (moemon_maker.ini
  only): changed files as blobs (Git blob ids compared), one tree, one commit with a "Moemon-Maker:" trailer, the
  default branch moved; history from the commits touching MoemonMaker/; a rollback commits the old MoemonMaker tree
  again and writes its files into the working copy. updater.c HttpRequest / Updater_Request (WinHTTP, libcurl).
- **The pack**: .github/workflows/maker-pack.yml checks MoemonMaker/ against the game (tools/maker/check_pack.py) and
  puts MoemonMakerPack.zip + maker-pack.txt (tools/maker/build_pack.sh; the id is a hash of its files) on the
  "maker-pack" pre-release; updater.c MakerPackWorker downloads it at each start of a release folder; package.sh
  ships it in every release as MoemonMakerPack/. The editor's first open on a PC starts its working copy from it.
- **Tests**: tools/test/maker_gate.sh, maker_shell.sh, maker_map.sh, maker_edit.sh, maker_wild.sh, maker_music.sh,
  maker_play.sh, maker_publish.sh (a stand-in for GitHub's API: maker_github_mock.py), maker_pack.sh, maker_sinnoh.sh;
  harness commands `maker ...` (maker_ui.cpp MakerUi_TestCommand) and `makershot NAME`.

## The 1.82.0 fixes (SINNOH, JOHTO, KANTO, the GAME CENTER)
- SINNOH's stairs: Platinum's WARP_STAIRS_EAST / WEST are the tile beside the stairs, walked off into them. build_pack.py
  STAIRS gives them Emerald's unused behaviors 0x5C / 0x5D (up, east / west) and 0x5E / 0x5F (down), arrow warps
  (metatile_behavior.c MetatileBehavior_IsSinnohStairs, SINNOH only), carried up or down by DoSinnohRideWarp, no arrow
  sprite. Test sinnoh_* (the house's stairs: walk beside them, nothing; walk into them, the floor changes).
- SINNOH's people: Platinum's sheets stand on row 29 of 32, Emerald's on 30: build_gfx.py feet_shift moves them down.
- Story people on Platinum's story: s3d_legends.c skips the old written story's battle rows (they cleared FLAG_HIDE_
  ROUTE_201_RIVAL on every visit). Test sinnoh_platinum_verity. The lab's first visit: gen_scripts.py converts
  GivePokedex and BufferPlayerCounterpartStarterSpeciesName (sinnoh 6 ..., 1: the counterpart's). Test sinnoh_platinum_lab.
- Quetzal's setmaplayoutindex numbers are Quetzal's layout ids (quetzal_rip/layouts quetzal_layout_id): the scripts name
  LAYOUT_ constants now (import.py fix, tools/test/check_scripts.py refuses a number). Three alternate layouts no map
  header points at were never ripped: SEAFOAM B3F / B4F with the currents stopped (899 / 900) and DUNSPARCE TUNNEL dug
  out (940). tools/quetzal_rip/rip_alt_layouts.py QUETZAL_ROM reads them from Quetzal's layout table (ROM 0x930A8C,
  entry = id - 1; ROM_MAP.md's 0x08939B54 is not it) into the rip and data/layouts (appended: no number moves);
  `mp layout` prints the layout a map shows; test region_alt_layouts.
- JOHTO's / KANTO's PC: FireRed's tiles 0x62 / 0x63 (field_specials.c PcTilePair). Test region_pc_tile.
- The BICYCLE (item 360) rides as the MACH BIKE. Test region_bicycle.
- Fossil lines are never wild before the region's LEAGUE (mp_region.c MP_WildSpeciesOk). Test region_fossils.
- JOHTO's BERRY patches: never a sprite of their own (the map's fruit tree is the tree, ripe or not, as in Crystal; until 1.91.0
  the ripe BERRY tree showed over it), Crystal's "There's nothing here…" otherwise, replanted when picked
  (mp_apricorn.c MP_JohtoBerries_*). Test johto_berry_patch.
- The GAME CENTER away from HOENN: the guide charges the admission, 3F's four patrons are OBJ_EVENT_GFX_VAR_0-3
  (MP_Region_GameCenterLooks), the coin counter names the region's GAME CORNER (MP_Region_BufferGameCorner). The CARD
  CLUB people use MP_Region_BufferName and give no second CARD CASE.
- Map pictures (mp_mappic.c) take the day tint (gMpScenePixel) as the map's tiles do.
- Battle weather keeps off every sprite but the battlers, and half of it off them too: gba_easy_draw.c records each
  pixel's sprite palette (gMpPixelObjPal), mp_battleweather.c MayCover.
- GB2 printed art: tools/tcg/trim_printed_rects.py moves the rectangles' edges past the card border; tcg_gfx.c
  CleanArtCorners cleans what's left in a rounded corner.
- JOHTO's item ball sprite 59 is drawn as 15006 (gen_objects.py DRAWN_AS); ETHAN's back picture frames reordered
  (moemon_chars/import.py BACK_ORDER).
- The TOWER pillar (SPROUT TOWER, TIN TOWER) is Quetzal's object 16066 (five frames), not map tiles: gen_objects.py
  OBSTACLE_ANIMS gives it object_event_anims.h sAnimTable_QzTowerPillar (Crystal's TilesetTowerAnim: 0 1 2 3 4 3 2 1,
  16 frames a step, for every anim index) and event_object_movement.c UpdateObjectEventSpriteAnimPause keeps it
  running (a standing object's facing anim is paused). Test johto_sprout_pillar (check_pillar_sway.py).
- Where Quetzal's KANTO layouts differ from FireRed's, FireRed's win (the user, 2026-11-03): tools/quetzal_port/
  frlg_blocks.py puts FireRed's blocks back in data/layouts at import (the rip keeps Quetzal's bytes); so far SEAFOAM
  B3F's two ledge-side tiles. The three alternate layouts are FireRed's own byte for byte.
- The MAGNET TRAIN (data/scripts/mp_region.inc, MP_MagnetTrainOpen: either LEAGUE + the S.S. TICKET) rides Crystal's
  scene (engine/events/magnet_train.asm) in src/mp/mp_magnettrain.c with art from tools/quetzal_port/gen_magnet_train.py
  POKECRYSTAL_DIR: BG1 the scenery (Crystal's 18 rows a row lower; the GBA's extra top and bottom rows carry on the
  trees), BG0 the train's band (tile 96, past the art, is the clear one: tile 0 is opaque), both VOFS cleared (the
  field's camera leaves BG1's scrolled). Test region_train_ride: tools/test/check_train_scroll.py checks two frames in a
  row differ in every 8-pixel row outside the train's band and that trees fill the top and bottom rows.
- No drop shadow in tall grass (mp_shadow.c InGrass), as the game's own grass covers the feet.

## 1.82.1: no shadow behind a tile drawn over the feet
- mp_shadow.c FeetCovered: a NORMAL / SPLIT metatile's top layer (BG1, over sprites at priority 2) with 8 or more of the
  32 pixels of its bottom two rows drawn hides the owner's feet: no shadow (it peeked out under a 2x1 tree's leaves,
  the user's picture). Followers and Garden POKeMON by their feet against the player's (SpriteFeetCovered). The GAME
  CENTER's counters are its pictures' "over" layer, not tiles: mp_mappic.c MP_MapPic_CoversFeet (the same 8 of the
  bottom rows' 32 pixels). Tests shadow_covered, shadow_covered_tree, shadow_covered_gamecenter (check_shadow_covered.py).

## JOHTO's phone, Crystal's (1.83.0)
- tools/quetzal_port/gen_phone.py POKECRYSTAL_DIR -> src/data/qz_phone.h: Crystal's 28 trainer contacts (data/phone/
  phone_contacts.asm; BILL, BUENA and the BIKE SHOP aren't in), every phone script (engine/phone/scripts but MOM's and
  ELM's, which mp_pokegear.c already has), the number-asking std scripts (engine/events/std_scripts.asm) and each
  contact's trainer `.Script` on its map, as bytecode for src/mp/mp_qzphone.c (Crystal's semantics: `end` returns
  from a call, the conditions test the last value); the words (data/phone/text, data/text, maps) as paragraphs run
  together, paged by the game to its windows; rematch teams Crystal's (data/trainers/parties.asm), the first fight the
  Quetzal trainer of the same name on the contact's map (Quetzal split ROUTE 26 and joined the LIGHTHOUSE floors).
- Field: data/scripts/trainer_battle.inc EventScript_NoNormalTrainerBattle asks QzPhone_TrainerTalk first, so a beaten
  contact runs Crystal's own script (rematch when ready, the gift kept for the player, the number asked once, then
  again); data/scripts/qz_phone.inc's loop asks QzPhone_Step what to show (text in gStringVar4, YES / NO, giveitem, a
  rematch through BattleSetup_ConfigureTrainerBattle's no-intro battle with Crystal's beaten words, the registration
  fanfare). Rematch teams go in through MP_Region_TrainerData (MP_QzPhone_RematchTrainer).
- Calls: field_control_avatar.c, each step in JOHTO or KANTO, a registered contact not on the map calls every few
  hundred steps (QzPhone_EventScript_IncomingCall). OPTIONS > CO-OP > MOENAV CALLS: BATTLES plays the call silently
  first (its own seed, the state put back) and lets only a call asking for a battle ring, as HOENN's MOENAV.
- POKéGEAR: the phone book is MOM, PROF. ELM, then the numbers (CLASS and name, scrolling); calling one plays its
  callee script's words (MP_QzPhone_Call), two lines a page.
- Crystal's story marks: Quetzal's fly flags (0x11BE-0x11C5, VERMILION 0x2262), badge flags, VAR_QZ_0x500A for the RADIO
  TOWER (6-9 ROCKETS in it, 10 cleared; the ROCKET HIDEOUT counted cleared from 6), the LEAGUEs; the day's BUG-CATCHING
  CONTEST, the DEPT. STORE sale and KANTO's SNORLAX read as not yet. Swarm news (RALPH, WILTON, ANTHONY, ARNIE) is told
  but no swarm appears yet.
- State: region vars 0x5240-0x5258 (numbers, Crystal's phone flags, fight counts, the step count). Debug `mp qzphone
  register|ready|fights|steps|moenav|call N [V]`. Tests johto_phone, johto_pokegear_contacts, johto_phone_battles.

## Voyages between regions, the first arrival's guide, KANTO's TVs (1.83.0)
- src/mp/mp_voyage.c MP_Voyage_Begin(eastbound, faded): FireRed's ferry scene (pokefirered src/seagallop.c, its art in
  graphics/moemon/voyage): the sea on BG3 scrolling 0x600 a frame, the ferry crossing with its wake every 5 frames,
  FireRed's letterbox (WIN0, rows 24-136; the backdrop black), 140 frames, then the fade and WarpIntoMap to the warp set
  before it (in DoWarp's place). The port's music plays on through it, as FireRed's. Logs `voyage: east|west|arrived`.
- qz_specials.c TravelNow: every boat trip between regions (and a friend following the host's trip) sails it, east when
  the destination comes later in HOENN, JOHTO, KANTO, SINNOH; the MAGNET TRAIN keeps its own ride.
  QzDoSeagallopFerryScene: the SEVII ferries, east / west by FireRed's sTravelDirectionMatrix (the script has faded the
  screen: faded TRUE).
- The first boat to JOHTO / KANTO (mp_region.c MP_Region_FirstBoatIn) comes in at OLIVINE's port (8,16) / VERMILION's
  dock (24,32) with VAR_REGION_ARRIVAL 0x52FD = the region; the ports' frame scripts (Moemon Kai lines in
  Johto_OlivinePort / Kanto_VermilionCity scripts.inc) run data/scripts/mp_region.inc EventScript_RegionArrival*: the
  port's sailor (local id 1 / 4) says the professor asked him to meet the player and warps them to NEW BARK's / PALLET's
  room. A new game started there and the MAGNET TRAIN still go straight to the house (MP_Region_GoToStartHouse).
  SINNOH's CANALAVE sailor was already that. tools/quetzal_port/story_vars.py reads include/mp_region.h's var names.
- tv.c SetTVMetatilesOnMap does nothing on a FireRed-tileset map (isFrlg): METATILE_Building_TV_On / Off are Emerald's
  numbers, which drew another tile over KANTO's / JOHTO's TVs when a HOENN show was on the air.
- Tests region_voyage (HOENN -> KANTO first trip, the guide, back west), kanto_seagallop (`mp seagallop FROM TO`),
  kanto_tv (`mp tvscreens [off]`).

## SINNOH's doors, moving lakes and the feet row (1.83.0)
- Doors: Platinum's door props (ov5_021D431C.c's list: door01, the wooden, iron, POKéMON CENTER, gym, DEPT. STORE...
  doors; the elevator's, a texture pattern, isn't played) carry 2 or 4 joint animations (open, close; four: walking
  in, then walking out). tools/sinnoh/build_pack.py (pack version 7) bakes each one's frames as meshes (door_frames)
  and turns a step warp with a door prop on it or beside it (door_at, Platinum's three-tile box) into MB_ANIMATED_DOOR,
  so the game's own door warp runs (field_screen_effect.c Task_DoDoorWarp / Task_ExitDoor); field_door.c hands its
  calls to src/sinnoh/s3d.c on SINNOH's maps: S3D_DoorAnimate (0 / 1 walking in, 2 / 3 walking out; a task the field
  waits on), S3D_DoorClose, S3D_DoorSound (sliding / the DEPT. STORE's chime: SE_SLIDING_DOOR, hinged: SE_DOOR). The
  door left open keeps its last frame until it shuts. Logs `[s3d] door N at X,Y: animation K`; `mp s3ddoor K` plays one
  in front of the player.
- Buildings drawn as Platinum draws them: build_model_matshp.dat (fetch_decomp.sh PARTS 22) lists the (material,
  shape) pairs of 478 building models; map_prop.c draws them one by one with the prop's transform and the position
  scale only, their nodes left out (nitro.py Model.draw_matshp). A house's own copy of its door hangs off a node at the
  doorway, and Platinum's way leaves it inside the house, so the door prop is what's seen (14 towns' pictures are
  otherwise the same pixel for pixel).
- Moving textures on props: an NSBTA (texture SRT: nitro.py Bta0 / SrtAnim, translation in texture sizes, rotation
  left out) on a looping prop is baked frame by frame like the joint animations (build_pack.py prop_frames): LAKE
  VERITY's water (prop 311, 61 frames a loop) and the other lakes and pools.
- The feet row: Platinum's sheets stand on row 29 of their frames, Emerald's on 30; build_gfx.py feet_shift now takes
  the standing frames only (112 sheets step down to row 30, and judged by those weren't moved).
- Tests sinnoh_doors, sinnoh_lake_water (tools/test/check_lake_water.py), sinnoh_feet_row (tools/test/check_feet_row.py).
- (1.83.0) Sprite palette slots 16-63 in a palette mask: battle_anim_throw.c SpriteCB_Ball_FadeOut faded
  `1 << (slot + 16)`; with the ball's palette in slot 17 that wrapped to bit 1, BG palette 1, the battle's YES / NO
  frame, left white after every catch. It fades by slot now (MP_BeginSpritePaletteFade); the battle animations' and the
  trade's masks use MP_SPRITE_PAL_BIT (palette.h: 0 for slots 16-63, no fade rather than the wrong palette's).
  Test catch_yesno_frame (tools/test/check_yesno_frame.py); `mp dumppltt FILE` / `mp dumpvram FILE` write into the
  test's picture folder for a relative FILE.

## KANTO / JOHTO restored toward FireRed / Crystal (1.88.0, the audit: docs/REGIONS_AUDIT.md)
The audit's findings are in docs/regions_audit/<AREA>_findings.md, the user's decision per case in docs/REGIONS_AUDIT.md
(JOHTO strictly Crystal; Gen 5+ legendaries only from MOEGATCHA). Tools and conventions for the work:
- `FLAG_KAI_GONE` (include/constants/kai_regions.h, always set by mp_region.c on load / new game): a Quetzal person or
  thing taken out wears it as its object flag, so the map's local ids stay as they are. New JOHTO / KANTO flags of
  this game's own go in kai_regions.h (free numbers in Quetzal's ranges; the map scripts, map events and C see it).
- tools/quetzal_port/crystal_words.py MAP LABEL [OURS]: Crystal's text from pret/pokecrystal as `.string` lines.
- Raw Quetzal trainer numbers in settrainerflag / cleartrainerflag are TRAINER_QZ_<n> now (raw ones went through
  TRAINER_FLAG into the special flags and past them: memory corruption on every gym win).
- qz_specials.c QzClearSeagallopRestrictions does nothing (it cleared KANTO's GAME CLEAR / CHAMPION flags);
  mp_region.c KantoLeagueFlags gives them back to saves with KANTO's LEAGUE on record.
- JOHTO's roaming RAIKOU / ENTEI: src/mp/mp_johto_roamer.c (Crystal's rules and RoamMaps; chunk JRMS; the BURNED TOWER's
  special MP_JohtoRoamer_Init; saves past the scene get them on load: MP_JohtoRoamer_CatchUp); their static objects are
  hidden (RefreshStatics); roamer.c's HOENN roamer only roams HOENN. Debug `mp jroamers [init|SLOT|meet]`; test johto_roamer.
- FireRed's boulders that fall through holes: field_player_avatar.c MP_BoulderFallThroughHole (the reveal flag in the
  template's trainer type; holes are non-animated door warps here), mp_world.c for friends' pushes; SEAFOAM B4F's
  current QzSeafoamCurrentEffect = FireRed's stop-surfing north. Test kanto_boulder_hole.
- KANTO's BIRTH ISLAND: field_specials.c's DEOXYS rock code takes FireRed's vars there (0x5101 level, 0x5102 steps,
  0x220B done) and moves the rock on the current map. Test kanto_birth_island.
- RADIO TOWER 1F: Crystal's RADIO CARD quiz (FLAG_JOHTO_RADIO_CARD gates the POKéGEAR's RADIO card; saves past
  Quetzal's WHITNEY quiz get it); WHITNEY's door guard and quiz gone. Test johto_radio_card.
- CHARCOAL KILN: FLAG_JOHTO_GOT_KILN_CHARCOAL, FARFETCH'D shown once HM01 is given (test johto_kiln). PRIMAL CHAMBER walled up
  (Mansion B1F's warp out, setmetatile 31,2). MAXIE / ARCHIE, ICE PATH's SWINUB, the FLOWER SHOP's gift girl and stalls,
  NEW BARK's ROUTE 27 guard: FLAG_KAI_GONE. Quetzal's NEW BARK berry planting removed.
- Also in 1.88.0: FireRed's hidden COINS (obtain_item.inc EventScript_MpHiddenCoins: a hidden spot with ITEM_NONE holds
  COINS by its flag, mp_region.c MP_HiddenCoinsAmount; test kanto_hidden_coins); VERMILION's pier after the S.S. ANNE
  sails ("The ship set sail.", FireRed's harbor words); start_menu.c sets FireRed's FLAG_OPENED_START_MENU (0x21E7);
  JESSIE & JAMES retired (qz_specials.c TravelingEvent hides them always); tools/quetzal_port/kai_gone.py takes a map's
  Quetzal objects / last warp / signs / triggers out; CERULEAN CAVE 2F's ladder to Quetzal's RAYQUAZA room drawn over.
- 1.89.0 (the audit's batch B):
  - The VICTORY ROAD gate between JOHTO and KANTO is customs, not a walk: its doors are triggers (VAR_TEMP_9) running
    data/scripts/mp_region.inc EventScript_RegionGateToKanto / ToJohto; qz_specials.c MP_RegionGateCross checks both
    LEAGUEs and the GLOBAL PC room, keeps the POKéMON in VAR_0x8005, sends the rest to the PC and travels
    (MP_Region_Travel). EventScript_RegionCustomsPick is customs without the first-visit greeting. Test region_gate_walk.
  - KANTO's rival is named by the player (FireRed's OAK's lab scene): VAR_KANTO_RIVAL_NAME (0x514C-0x514F),
    qz_specials.c NameRival(region) / MP_KantoRivalName / MP_JohtoRivalNameHere; online the room's name rides the
    rules (mp_main.c MP_RoomRivalName(region)); KANTO's lines say {RIVAL} where Quetzal wrote BLUE. Test kanto_rival_lose.
  - POKéMON TOWER 6F's MAROWAK is FireRed's ghost battle: battle_script_commands.c gMpUncatchableWild (set by
    QzStartMarowakGhostBattle_, cleared at the battle's end) makes a ball BattleScript_MpBallDodged. Test kanto_marowak_ghost.
  - FireRed's SELPHY (FIVE ISLAND's RESORT GORGEOUS): her request (VAR_QZ_0x5141 / 0x5142) and 250 steps to show it
    (VAR_KANTO_RESORT_STEPS, field_control_avatar.c MP_ResortGorgeousStep). Test kanto_selphy.
  - ICEFALL CAVE 1F's thin ice: field_tasks.c step callback 8 (FireRed's STEP_CB_ICE, metatiles 0x35A / 0x35B, temp
    flags 1-9 keep the cracks until the map reloads; VAR_TEMP_1 drops the player to B1F). Test kanto_icefall_ice.
  - Crystal's GS BALL: GOLDENROD's POKéMON CENTER (a receptionist, object 10, at the door once JOHTO's LEAGUE is beaten:
    MP_JohtoGsBallReady, FLAG_JOHTO_GOT_GS_BALL); KURT keeps it a day (FLAG_JOHTO_GAVE_GS_BALL, daily
    FLAG_DAILY_JOHTO_KURT_GS_BALL); ILEX's shrine calls CELEBI once. Debug `mp newday` (a new day for the daily events
    without touching the clock: the clock is always the PC's, so `mp weekday` alone never passes a day). Test johto_gs_ball.
  - Crystal's rival after the LEAGUE: INDIGO PLATEAU's POKéMON CENTER on Mondays / Wednesdays (FLAG_DAILY_JOHTO_PLATEAU_RIVAL),
    DRAGON'S DEN B1F on Tuesdays / Thursdays (FLAG_JOHTO_HIDE_DEN_RIVAL); special MP_JohtoWeekday. Test johto_rival_days.
  - Also: LAKE OF RAGE's RED GYARADOS shiny (script_pokemon_util.c MP_MakeScriptedWildShiny), Quetzal's viewpoint pans
    gone, MAHOGANY's old man by VAR_QZ_0x500C, BAOBA (Quetzal's SAFARI quest) gone until case 62's SAFARI ZONE.
- 1.90.0 (the audit's batch C, KANTO):
  - tools/quetzal_port/firered_words.py MAP LABEL [OURS] (FireRed's text, POKéMON as MOéMON, POKéDEX as MOéDEX, ' as
    ’; a local pokefirered under POKEFIRERED_DIR or GitHub) and restore_words.py with its list restore_words.txt
    (OUR_MAP / data path, OUR_LABEL, fr|gsc, THEIR_MAP, THEIR_LABEL): every text put back is listed there, by case.
  - FireRed's move people on Emerald's specials: FUCHSIA's MOVE DELETER, the COPYCAT's MIMIC (a POKé DOLL), TWO
    ISLAND's MOVE MANIAC (MULTI_QZ_0x6D mushroom menu), CAPE BRINK's starter tutor (TUTOR_MOVE_FRENZY_PLANT /
    BLAST_BURN / HYDRO_CANNON 30-32: party_menu.c CanLearnTutorMove gives them to VENUSAUR / CHARIZARD / BLASTOISE
    only; qz_specials.c MP_CapeBrinkLeadMove / MP_CapeBrinkTaught, flags 0x2226-0x2229), CELADON 3F's COUNTER.
    Tests kanto_move_people, kanto_cape_brink.
  - DAISY: the TOWN MAP (item_use.c ItemUseOutOfBattle_TownMap: the region map from the BAG or registered), her rest
    line, grooming after KANTO's LEAGUE every 500 steps (VAR_KANTO_MASSAGE_STEPS, MP_DaisyMassage) and her
    friendship reading. CERULEAN's BIKE SHOP (MULTI_QZ_0x57 is FireRed's BICYCLE ¥1,000,000 row). Test kanto_daisy_bike.
  - Size records on Emerald's code: pokemon_size_record.c Get/CompareMagikarpSize, Get/CompareHeracrossSize (vars
    VAR_KANTO_MAGIKARP_SIZE / VAR_KANTO_HERACROSS_SIZE). Tests kanto_size_records, kanto_size_heracross.
  - RUIN VALLEY: fldeff_cut.c SetUpFieldMove_Cut asks qz_specials.c MP_RuinValleyCutCheck first; CUT from the party
    menu below the door opens it (debug `mp usecut`; test kanto_ruin_valley_cut). ROCK TUNNEL requires FLASH.
  - GIOVANNI leaves VIRIDIAN's GYM after his farewell (FLAG_KANTO_HIDE_GYM_GIOVANNI; debug `mp beattrainer ID`; test
    kanto_giovanni_leaves, with FIVE ISLAND's POKéMON JOURNAL). The WATER LABYRINTH's TOGEPI EGG (test
    kanto_togepi_egg). The SEAGALLOP: FireRed's prompt, VERMILION once BILL's first trip is over, ISLANDS 4-7 by the
    RAINBOW PASS alone. TWO ISLAND's growing shop (the tickets in its last list). MELOETTA out of the CHANSEY dance.
- 1.91.0 (the audit's batch D, JOHTO, first part):
  - crystal_words.py reads Crystal's shared texts too (MAP may be a path: data/text/common_3), and turns text_ram
    buffers into {STR_VAR_1/2/3}, {PLAYER}, {RIVAL}. restore_words.txt lists every JOHTO text put back, by case.
  - MANIA lends SHUCKIE (qz_port.c qz_givestorymon 2: Lv15, ORAN BERRY, OT MANIA / 518 re-keyed after
    GiveMonToPlayer by MP_SetBoxMonOtIdKeepData; VAR_QZ_0x501D 4 lent, 5 done; FLAG_DAILY_JOHTO_SHUCKIE 0x925);
    qz_specials.c MP_ReturnShuckie scans the party (150+ friendship: it stays). The SANDSTORM house's loyalty check
    (MP_LeadFriendship, 150). LAKE OF RAGE's MAGIKARP record (Get/CompareJohtoMagikarpSize, VAR_JOHTO_MAGIKARP_SIZE
    0x5040, FLAG_JOHTO_MAGIKARP_RECORD 0x104A, FLAG_JOHTO_ELIXER_STANDBY 0x104B). The ROUTE 43 toll (1000 while
    FLAG_QZ_0x1051 is unset). Test johto_crystal_people.
  - DRATINI: qz_givestorymon 4 is Lv15 with Crystal's EXTREMESPEED moveset; the wrong-answer one Lv15 too.
  - BLACKTHORN's MOVE DELETER (FUCHSIA's script, Crystal's words; test johto_move_deleter). ECRUTEAK's ITEMFINDER
    (FLAG_JOHTO_GOT_ITEMFINDER 0x10A3). TOHJO FALLS: MOON STONE, the SUPER ROD fisher gone. ROUTE 32's guard: the
    ZEPHYRBADGE, then the EGG errand. MAHOGANY's RAGECANDYBAR and ROUTE 43's MAHOGANY gate follow the hideout
    (VAR_QZ_0x500C >= 14).
  - JOHTO's BERRY patches never draw a sprite (event_object_movement.c SetBerryTreeGraphics /
    MovementType_BerryTreeGrowth_Normal): the map's fruit tree is the tree (docs/user_images 68).
- 1.92.0 (the audit's batch E, JOHTO, second part):
  - Crystal's WEEKDAY siblings (tools/quetzal_port/fix_siblings.py): each object wears FLAG_JOHTO_HIDE_<NAME>, set or
    cleared on entering a JOHTO map by mp_region.c MP_JohtoWeekdayPeople (overworld.c, after MP_JohtoBerries_Init);
    met once (FLAG_JOHTO_MET_<NAME>), the gift keeps Quetzal's got-it flag. UNION CAVE B2F's LAPRAS out on Fridays
    (FLAG_JOHTO_HIDE_LAPRAS, FLAG_DAILY_JOHTO_LAPRAS once fought). GOLDENROD DEPT. STORE 5F's Sunday receptionist
    (FLAG_JOHTO_HIDE_DEPT_LADY, FLAG_DAILY_JOHTO_DEPT_TM 0x927; her counter tile is the clerks' talk-across one). `mp weekday D` works only while FLAG 0x51 is clear
    (FixStuckClockSave resets the offset otherwise). Tests johto_weekday_siblings, johto_friday_lapras, johto_sunday_lady.
  - Item balls hold Crystal's items (fix_item_balls.py; new balls ROUTE 30 / 26 on FLAG_JOHTO_ITEM_*); Crystal's 57
    JOHTO hidden items (gen_hidden_items.py: FLAG_JOHTO_HIDDEN_ITEM_n = 0x1600 + n, each spot checked against Crystal's
    map by eye, SPOTS in the script; elevation 0 so any height finds them; test johto_hidden_item). map_ascii.py prints
    a map's collision and objects for picking spots.
  - ROUTE 29's catching demonstration: battle_setup.c MP_JohtoCatchingTutorial (WALLY's tutorial with
    gWallyTutorialBackPic / Name / ThrowFirst; the DUDE's back picture graphics/moemon/dude_back.bin from
    gen_dude_back.py; FLAG_JOHTO_LEARNED_TO_CATCH). Test johto_catch_tutorial.
  - CHERRYGROVE's rival is qz_trainerbattle_13 (a loss doesn't black out). ELM's messages ring (SE_POKENAV_CALL) with
    Crystal's data/phone/text/elm words. GIOVANNI's leftovers and Quetzal's NUZLOCKE calls are gone. Multichoice menus
    of our own start at KAI_FIRST 240 (gen_multichoice.py checks the stock ids; touch src/script_menu.c after).
  - POKéMON in houses are OBJ_EVENT_GFX_MON_FIRST + dex objects with their cry and Crystal's line (KURT's and AZALEA's
    SLOWPOKE on FLAG_QZ_0x1085, until the WELL is saved).
- 1.93.0 (the audit's batch E, JOHTO, third part):
  - GOLDENROD's move tutor (Crystal's MoveTutorScript): FLAG_JOHTO_HIDE_GOLDENROD_TUTOR (LEAGUE + COIN CASE +
    Wednesday / Saturday, mp_region.c MP_JohtoWeekdayPeople), FLAG_DAILY_JOHTO_TUTOR 0x928, MULTI_KAI_GOLDENROD_TUTOR;
    the moves are TUTOR_MOVE_TM_FLAMETHROWER / THUNDERBOLT / ICE_BEAM (40-42, past TUTOR_MOVE_COUNT): party_menu.c
    GetTutorMove / CanLearnTutorMove use the TM's list, as Crystal did. Test johto_goldenrod_tutor.
  - Qz{Johto,Kanto}TravelingEventCheck run in every JOHTO / KANTO map's ON_RESUME and no longer write VAR_RESULT: a
    script back from the party menu (a tutor after the POKéMON forgot a move) read 0 there and took the "no" branch.
  - fix_johto_tms.crystal_text joins Crystal's "cam- " / "paign" (a split word with a space after its hyphen).
  - CIANWOOD's POKé SEER (the old woman's house; Crystal's engine/events/poke_seer.asm): qz_specials.c MP_PokeSeer
    (+ MP_PokeSeerOT / Level / Advice) on the met location and level (hatched: level 5, Crystal's EGG_LEVEL), own or
    traded by OT ID, METLOC_FATEFUL_ENCOUNTER = level only, METLOC_IN_GAME_TRADE = can't tell; no time of day (this game
    keeps none). crystal_words.py knows the wSeer* buffers and text_ram ... text_start. Test johto_poke_seer.
  - GOLDENROD GAME CORNER's TM prizes (Crystal's TMVendorScript on Quetzal's "prizes are to the right" man):
    MULTI_KAI_GOLDENROD_TM_PRIZES, 5500 coins each, additem (no obtain fanfare, as Crystal). Test johto_tm_prizes.
  - GOLDENROD's NAME RATER: Crystal's words (data/text/common_2 _NameRater*) on KANTO's LAVENDER flow
    (Common_EventScript_NameReceivedPartyMon, no OT check); his bookshelves Crystal's. Test johto_name_rater.
  - GOLDENROD UNDERGROUND as Crystal's: the HAIRCUT BROTHERS (qz_specials.c MP_HaircutBrother: Crystal's odds and
    friendship steps; FLAG_DAILY_JOHTO_HAIRCUT 0x929), the bargain merchant (Monday 4:00-9:59; MULTI_KAI_GOLDENROD_BARGAIN,
    Crystal's prices, one of each a day: FLAG_DAILY_JOHTO_BARGAIN_* 0x935-0x939), the herb lady at weekends (+ REVIVAL
    HERB), each on a FLAG_JOHTO_HIDE_* set by MP_JohtoWeekdayPeople, in the alcoves of Quetzal's flute / METEORITE /
    empty clerks; KUNI's scene gone, its triggers only move VAR_QZ_0x500A to 8. `mp hour H` sets the game clock's hour.
    crystal_words.py puts in {d:NAME} numbers from the map's DEFs. Tests johto_haircut, johto_bargain.
- 1.105.0 (the audit's batch F, JOHTO):
  - ROUTE 36's FLORIA (case 100, tools/quetzal_port/fix_floria.py, run once): by the odd tree (39,21, facing it) until
    met (FLAG_JOHTO_MET_FLORIA 0x1542), then in the FLOWER SHOP; talked to there (FLAG_JOHTO_TALKED_FLORIA 0x1543), back
    at the tree with her second line; with the SQUIRTBOTTLE (or the tree fought) in the shop for good. Her two places
    are FLAG_JOHTO_HIDE_FLORIA_ROUTE / _SHOP (0x1544 / 0x1545), set by MP_JohtoWeekdayPeople as Crystal's scripts move
    EVENT_FLORIA_AT_SUDOWOODO / _AT_FLOWER_SHOP. The teacher gives the bottle on Crystal's conditions (met, talked to in
    the shop, PLAINBADGE FLAG_QZ_0x11B4). Crystal's lines for both, ROUTE 36's lass (the GROWLITHE line gone), and the
    RUINS gate's gramps (obj 2) and officer (obj 1). Test johto_floria.
  - NATIONAL PARK and ROUTE 35 (case 101, tools/quetzal_port/fix_park.py, run once): Quetzal's man "with her by my
    side" is Crystal's PERSIAN again (OBJ_EVENT_GFX_MON_PERSIAN, its cry), the healer the mail mother (POKEFAN F, no
    healing), and the teacher walking by the sticker boys and the boy printing his POKéDEX are back; the balls are
    Crystal's PARLYZ HEAL and TM28 DIG (MOON STONE / SOOTHE BELL gone). ROUTE 35's OFFICER DIRK is no longer a
    trainer who spots you: talked to by day he only says your POKéMON look tough, at night he battles (QzIsNight, a
    special: 20:00-3:59 like JOHTO's night POKéMON). The GOLDENROD gate's fisher says Crystal's "150 kinds". Test
    johto_national_park.
  - The rest of case 138 (Crystal's small people, signs and lines):
    - ROUTE 32's RUINS gate: Crystal's officer and POKéFAN added (Quetzal kept only the boy).
    - Signs placed as primary metatile 3 (a wooden post every JOHTO map can use) with bg events: VIOLET CITY's town
      sign, SPROUT TOWER and EARL'S ACADEMY; ROUTE 33's by UNION CAVE.
    - ELM's lab window (bg 5-7,1): "He broke in through here!" from the theft (VAR_QZ_0x5004 >= 4) until VIOLET CITY
      (FLAG_JOHTO_VISITED_VIOLET 0x154A, set by VIOLET's ON_TRANSITION). ELM's house PC: five new metatiles at the end
      of johto_blackthorn_dragon_speech_house (856-860: the lab's desk computer over the house's wall and floor)
      where Quetzal had a bookshelf, with Crystal's research-paper words. The aide's four lines by stage.
    - ELM's calls follow Crystal's elm.asm by stage (mp_pokegear.c CallElm: MR. POKéMON, the theft, the EGG checked,
      the aide in VIOLET, the TOGEPI EGG, hatched: FLAG_JOHTO_TOGEPI_HATCHED 0x154B from egg_hatch.c, then
      FLAG_JOHTO_TOLD_ELM_TOGEPI 0x154C), so the lab has "I thought the EGG hatched. Where is the POKéMON?".
    - The RUINS OF ALPH chambers: the scientist's lines on Crystal's conditions (cryptic patterns, the tremor, the hole,
      every UNOWN caught: a JOHTO UNOWN dex, "<save>.region" chunk UNOC, mp_region.c MP_JohtoUnownCaught from
      HandleSetPokedexFlag). The four hidden walls as Crystal's unown_walls.asm (qz_specials.c QzRuinsWalls*): this
      one map holds all four chambers (HO-OH top left, KABUTO top right, OMANYTE bottom left, AERODACTYL bottom
      right); HO-OH leading or a WATER STONE carried open theirs on arrival, an ESCAPE ROPE used in the KABUTO chamber
      (item_use.c) or FLASH in the AERODACTYL chamber (fldeff_flash.c: usable there lit, a white flash) mark theirs
      (FLAG_JOHTO_RUINS_WALL_* 0x1546-0x1549) for the next visit; a marked wall opens with Crystal's tremor (ON_FRAME
      on VAR_TEMP_3). Reading a closed wall shows its word (ESCAPE / LIGHT / WATER / HO-OH) spelled with UNOWN icons
      (QzShowUnownWord, Crystal's DisplayUnownWords). `mp ruinsmark` acts as both used where the player stands.
      Tests johto_ruins_walls, johto_ruins_word, johto_elm_lab.
- 1.106.0 (the audit's batch G, JOHTO):
  - Case 105, Quetzal's small rewrites: no CHANSEY beside JOHTO's nurses (13 centers); GOLDENROD's center without
    the two "gold roof" people, its old woman Crystal's POKéFAN asking for EON MAIL (the game has none, so she always
    ends on Crystal's "Too bad"); ECRUTEAK's BILL gives Crystal's TIME CAPSULE speech; OLIVINE without the SAFARI ZONE
    news (JOHTO's SAFARI ZONE is case 62's) or the MARY fan, its sailor and boy Crystal's (POKéGEAR or POKéDEX line by
    ITEM_POKEGEAR); ROUTE 39 without BAOBA's HO-OH shadow scene or the "giant PIDGEY" man; ROUTE 38's TRAINER TIPS
    sign (stopping an evolution with B) where Quetzal had "NO SWIMMING"; OLIVINE GYM without the two cheering people
    and their triggers (nothing read VAR_QZ_0x500D 6 / 7), the MINERALBADGE raising DEFENSE; the warehouse grunt's
    Crystal lines; BURNED TOWER without the FIREBREATHER and BURGLAR; OLIVINE CAFE without JASMINE's dishes, its
    FISHING GURU and STRENGTH sailor in Crystal's words. Test johto_crystal_lines.
  - Case 49's JOHTO half: ILEX FOREST's HEADBUTT man (object 18) gives TM96 HEADBUTT with Crystal's words; the
    other copy (object 6) and OLIVINE's "manage moves from the summary screen" man are gone (kai_gone). Case 104: MOOMOO
    takes seven BERRIES and the farm gives TM109 SNORE. Case 15: LANCE's HM WHIRLPOOL (given since 1.101.0). The TMs
    themselves: "TM93-TM139". Test johto_tm_gifts.

## MAGIC (1.95.0: docs/MTG_DECISIONS.md, docs/MTG_HANDOFF.md)
- The rules are Forge's translated into C (src/mtg/mtg_game.c etc., include/mtg/mtg.h), reading Forge's own card
  scripts for the Premodern pool (graphics/mtg/forge.pak, built in by src/mtg/mtg_pak.c with .incbin; read once by
  mtg_integration.c Mtg_EnsureDb, ~10 ms). A card plays only when every piece of it is translated (docs/MTG_SCOPE.md).
- The collection is "<save>.mtg" (src/mtg/mtg_collection.c: cards and decks by name hash, so a rebuilt pack keeps
  saves; Mtg_GetCollection, written from Tcg_WriteSidecar, forgotten by Tcg_ForgetSidecars). Under the test harness
  its random numbers start the same (12345), so duels are dealt the same.
- Hidden until a deck (decisions 5, 6): TcgUi_CaseTabs shows a fourth tab (MAGIC; the others shorten to POKe / YGO /
  MTG / TT) and the L / R round takes in MAGIC (YU-GI-OH! <- L, TRIPLE TRIAD -> R) only once Mtg_Unlocked(): TRIPLE
  TRIAD is always the last side, tabs and round alike (the user, 2026-11-04: POKeMON, YU-GI-OH!, MAGIC, TRIPLE TRIAD); the case
  opens on MAGIC when it was used last (MTG_FLAG_CASE_SIDE) or is the only game with cards. Before a deck the regulars
  say they're only talking.
- MOSSDEEP's CARD SHOP (the GAME CORNER: data/scripts/mtg_shop.inc): the clerk (the counter man) gives a CARD CASE if
  needed and a starter (src/mtg/mtg_decks.c gMtgStarters: EMBER red, TIDEWATER blue, NIGHTFALL black, WILDWOOD green,
  DAYBREAK white; 40 cards, 17 lands), then offers the way down to the play room (warp to B1F 3,2: 1F's own stairs are
  behind the counter). B1F's stairs come back up by the front door (warp 0). Regulars (gMtgDuelists, by VAR_0x8004):
  AUGUST (the old man) and PIP on 1F, RHEA, MARLOW, TESS, NOLAN on B1F; specials Mtg_DuelistState /
  Mtg_DuelistChallenge. A win: a booster of a set into the CARD CASE (below), wins / beaten saved; a loss costs nothing.
  Once the player has a deck the clerk opens the counter (Mtg_ShopBoosters, waitstate) before offering the way down.
- The look (1.96.0, the user: "not similar to Pokemon or Yugioh at all"; then "tiny details like that to be respected,
  across all card games", and the table "more like Pokemon's ... that style of play area"): every MAGIC screen and the
  table are laid out, worded and keyed as the POKeMON / YU-GI-OH! sides' own. When one of those changes, change MAGIC's
  to match (and the other way round).
- Card faces (src/mtg/mtg_gfx.c, as ygo_gfx.c): the game's own frame (never the printed one) colored by the card's
  color (white, blue, black, red, green; gold for more than one, silver artifacts, brown lands), the mana cost as dots,
  the printed illustration, a type bar with the rarity's dot, text lines, a P/T plate; under 22 px wide the picture
  fills the card. CARD ART (gMpConfig.cardArt): FULL CARD shows the whole scan, the other two the illustration in the
  frame; CARD BACKS: the printed back or the game's own. Textures cached per size (12x17 .. 72x100).
- The printed cards: graphics/mtg/printed.bin (committed), built in by src/mtg/mtg_art_data.c (.incbin;
  json_data_rules.mk), from tools/mtg/fetch_art.py (Scryfall: each playable card's earliest old-frame printing from
  4th Edition to Scourge, the "normal" scan and the art crop; tools/mtg/art.json lists set / number / artist) and
  tools/mtg/build_printed.py (JPEG 160x223, the illustration's rectangle found by matching the art crop against the
  scan, the artist; looked up by name; cards already there kept). Rerun both as more cards play (fetch_art.py's cache
  is ~/.cache/moemon/mtg_art). The card view and the big picture credit the artist ("Art by ..."), as Scryfall asks.
  tools/mtg/test.sh checks every playable card has its picture.
- Screens (src/mtg/mtg_ui.c): HOME (the active deck's ACE CARD floating; DECKS, BINDER, PRACTICE DUEL against
  another starter, DUEL SPEED shared with the other games, CLOSE CASE; SELECT: HOW TO PLAY, tcg_guide.c's eight MAGIC
  pages), DECKS (the 8 slots, DECK RECIPES, BACK; EMPTY SLOT faded, IN USE; menu EDIT / RENAME / ACE CARD / DUEL WITH
  IT / COPY / DELETE, an empty slot BUILD A DECK / FROM A RECIPE; R: duel with it, START copy, SELECT delete), the
  recipes (the five starters built again from the cards owned, what's missing), ACE CARD (MtgCollection.ace, -1: the
  game's pick Mtg_AutoAce), the builder (POKeMON-style chips W U B R G C M L, a 5 x 2 grid with in-deck/owned, the
  deck's list on the right), the BINDER (chips with * for all, 7 x 2, xN, SELECT: every card with the unowned faded,
  NEW by tcg_new.c CARDNEW_MAGIC), a card's view (the face at 72x100, the words, rarity, artist, A: the big picture),
  the starters (CHOOSE A STARTER DECK: each by its best card), the challenge (their sprite and best card, DUEL WITH a
  deck or NOT NOW; their words first on the map: Mtg_DuelistHello -> gStringVar4), the booster (torn open, dealt out,
  turned one by one, RARE!, NEW).
- The save: "<save>.mtg" format 2 adds each deck's ACE CARD (format 1 still reads).
- The table (src/mtg/mtg_board.c), the POKeMON table's way (tcg_board.c): a 240x200 cloth mat (leather brown, gold
  lines, a soft grain) drawn once, nothing in its middle (the user: no POKe BALL-like mark), framed rows for each
  player's lands (15x20 faces, alike ones stacked with a count tab) and creatures and other permanents (22x30, an LCD
  plate of power / toughness under each, like the HP plates), centered; the opponent's cards (rows, graveyard, library,
  their spells on the stack) turned half round to face them, as the other tables do (the user, 2026-11-04); the side columns: graveyard and library piles
  (as DrawPile), the opponent's HAND, each player's LIFE plate. The hand is on the blue tray (count tab, arrow); with
  the cursor in the hand the tray comes up and the mat slides up 37 px (BarY / Pan as tcg_board.c). The step bar (UNT
  UPK DRW M1 CMB M2 END) and what just happened sit in the middle; the stack's cards there with a line to their target.
  Tooltips under / over the card under the cursor (green-framed there, gold here), prompts in the yellow-framed box,
  menus in the game's window beside the card, notes as toasts. Past a row's end the cursor goes to the piles: A on
  your library CONCEDE (asked first), on a graveyard LOOK (a grid, INSPECT for a card). B: the turn's menu (PASS, END
  TURN: passes until the opponent's turn, no attack). YOUR TURN / THEIR TURN sweep; the opponent's spells shown big as
  they're cast; cards slide to their places (7 frames); DUEL SPEED FAST shortens the sweep, the big card and the
  computer's pauses. A plays the card under the cursor (a menu when it can do more, then the target cursor, which
  starts on the opponent's side), START passes, SELECT / INSPECT shows the whole text, L held stops at every priority.
  It passes for the player when nothing else can be done, when their own spell is on top, and outside their main
  phases, the blocks and the opponent's end step.
- What happens is played out (1.98.0, the user: "actions queue up immediately without animations ... text conveyance is
  not enough, there must be clear animations ... especially on the enemy turn"): mtg_board.c draws B.view, a copy of the
  duel that trails the rules' B.duel by one event at a time (Play / StartEvent / EndEvent): each MtgEvent is acted out
  (a card from the hand or library flying to its place, a back flying to the opponent's hand as they draw, the cast spell
  flying to the stack, their spells shown big first, taps turning, attackers lunging, blocks lit with a line, damage as
  a glowing shot to its target and a number, deaths shattering into the graveyard, life plates lit with +N / -N, the
  stack's card lighting as it resolves, the step bar ticking) and told in the middle line; while it plays nobody acts
  (the computer waits, then pauses a moment), a button pressed in the last 45 frames of it is kept and used once it's
  over, and the log says "mtg: ready" when the player may act again (the test harness waits for it). Durations shorten
  with DUEL SPEED FAST (Frames). When every event has played, view = duel.
- Costs at a glance (the user): the card under the cursor shows, on its top edge, what it costs (in the hand: its mana
  cost, the generic amount numbered in a grey orb, a dot per colored mana) or what using it costs (on the table: its
  activated ability's, {T} first when it taps; lands' mana is left to their faces). Only that card, for the table's
  looks (the user, 2026-11-05).
- Tray colors (1.98.0): the hand tray on the MAGIC and POKeMON tables is in the deck's own main color (MAGIC: its cards'
  colors and its lands' mana counted; POKeMON: its Pokemon's types and its Energy cards), or what the deck's menu's TRAY
  COLOR sets (the last item, so older menu positions stay): AUTO, one of the game's colors, or any POKe BALL color the
  player has (the first 11 everyone's, the rest by MP_BallUnlock_Has). Shared kit: tcg_ui.c TcgUi_TrayRgb / DrawTray /
  TrayOpen / TrayStep / TrayDraw; TCG_TRAY_AUTO / GAME + n / BALL + n; saved per deck (TcgCollection.tray: "<save>.cards"
  format 10; MtgCollection.tray: "<save>.mtg" format 3); MP_BallStyle_Rgb gives the ball colors' shades.
- The tray by context (1.99.0, the user: "Pokemon's systems that automatically raise or lower the tray based on context"):
  mtg_board.c Refocus, once the table is still and the player may act: your turn starts -> the cursor in the hand on a
  card you can play, the tray up; their turn -> the cursor on your field, the tray down; a card played from the hand ->
  the next playable card (on your turn), else "nothing left to play" -> the field, the tray down; their spell on the
  stack and an answer in the hand -> the tray up on it; your draw shows the hand a moment (showHand); B in the hand ->
  down to the field (elsewhere B is the turn's menu); B out of a target -> back to the card in the hand; moving the
  cursor into / out of the hand raises / lowers it; DISCARD raises it. Logged "mtg: refocus: ...".
- Keywords under the name (the user): the table's tooltip has a second line with what the card has now (Mtg_Keywords:
  a pump's too; landwalk, protection), and the card views show them under the name (MtgGfx_KeywordLine).
- Choosing attackers and blockers (the user: "a little confusing"): who can't be chosen is dimmed, who can is lit; a
  chosen attacker steps forward framed red with ATTACK over it and a line to the opponent's LIFE; the prompt counts
  ("2 attacking: 5 damage if not blocked  START ATTACK!" / "NO ATTACK"; blocks: "5 damage coming! ... NO BLOCK", "2
  blocking: 1 damage still coming"); the tooltip says what A does there (ATTACK / HOLD BACK, BLOCK / DON'T BLOCK); A
  on one that can't says why (tapped, arrived this turn, DEFENDER); B clears the attackers; when picking which attacker
  to block, the ones it can block are lit. Prompts wrap to two lines as tcg_board.c's.
- The regulars have only their names (no descriptions: the user, 2026-11-05).
- Booster packs (1.100.0, the user: "a true Booster Pack implementation, the same as Pokemon and Yugioh"): the sets are
  gMtgSets (src/mtg/mtg_sets_data.c, generated with tools/mtg/sets.json by tools/mtg/fetch_sets.py from Scryfall: the 28
  core sets and expansions from Fourth Edition to Scourge, no Chronicles, each with the pool's cards printed in it and
  their rarity there; rerun it with --offline to only regenerate the C file), src/mtg/mtg_sets.c draws a booster from a
  set's cards the game plays (Mtg_CanUse), slot by slot by the set's rarity (most sets 11 commons, 3 uncommons, a rare;
  Homelands 6/1/1, Alliances 8/3/1; a rarity with none playable takes the next down), no card twice in a pack while it
  can help it. A set is on offer (Mtg_SetAvailable) once enough of it plays (23 of 28 now; more open as cards are
  translated). Held per set in MtgCollection.packs, opened counts in .opened / .packsOpened ("<save>.mtg" format 4: the
  sets by code). The CARD CASE's HOME has BOOSTER PACKS between BINDER and PRACTICE DUEL (the YU-GI-OH! side's row and
  words), a carousel of the packs (the set's colors, its rare with the highest mana value as the picture
  Mtg_SetMascot, its code, xN; the strip: the set's name and year, owned / playable cards of it), A opens one: torn,
  dealt out face down (15 in rows of eight and seven, 27x38), turned one by one, RARE! and NEW, the strip naming the
  card's rarity in that set; B goes back to the packs on the same set. A win against a regular puts a random available
  set's booster into the CASE; the MOSSDEEP clerk's counter (M_SHOP, the YU-GI-OH! counter's look with one BOOSTERS tab)
  sells any available set's for MTG_PACK_COST (12) CARD POINTS, the shared TcgCollection.dust.
- The result screen (1.100.0, the user: "the win message should be in the card game itself, just like Pokemon"): when
  the duel ends mtg_ui.c DuelEnded counts it (record, booster, achievements) and gives the table its words
  (MtgBoard_Result, as TcgBoard_Result): the regular's quote, then "A PLS booster went into your CARD CASE."; leaving it
  goes back to the map (a practice: to the case's HOME).
- Achievements (1.100.0): a MAGIC tab (C_MAGIC, between YU-GI-OH! and TRIPLE TRIAD) in src/mp/mp_achieve.c: mtgstart,
  mtgwin1 / mtgwin100 (counter mtgwin), mtgregulars6 (each regular's first defeat), mtgpack50, mtgsets5, mtgkinds250,
  mtgflawless (20 life or more), mtgfast (by the duel's 10th turn), mtgdecked, mtgcounter (the table, on their spell
  countered), mtgmono, mtgdomain.
- Triggered abilities (1.100.0): src/mtg/mtg_trigger.c (T: lines; ChangesZone, Phase, SpellCast, DamageDone, Attacks,
  AttackerBlocked, AttackerBlockedByCreature, Blocks, AttackerUnblocked; IsPresent; OptionalDecider$ You). On the table
  a trigger lights its card and flies to the stack (theirs shown big first, as their spells are); a trigger with more
  than one target asks for it (MODE_TARGET, no B; a graveyard card shown big over the middle); "you may" asks YES / NO
  and an unless cost PAY / DON'T PAY beside the card asking, its words (the trigger's description) in the prompt, which
  wraps to as many lines as it needs; a sacrifice of your choice lights the permanents (MODE_PICK), a discard of your
  choice uses the hand as at the end of the turn; +1/+1 and -1/-1 counters float their number.
- The cursor and its tooltip show only once the table has been still for 8 frames with the player to act (B.still,
  ShowFocus): never on and off through the opponent's turn (the user, 2026-11-05).
- The computer is mtg_ai.c, a STAND-IN until forge-ai's translation (docs/MTG_HANDOFF.md).
- Debug: `mp mtg starter N`, `mp mtg state`, `mp mtg duel N` (a regular's challenge), `mp mtg autoplay 1` (the
  computer plays your side too), `mp mtg focus NAME` (the cursor on that card), `mp mtg put PLAYER hand|field|grave
  NAME` (a card put on the table), `mp mtg points N` (CARD POINTS), `mp mtg pack CODE N` (boosters held), `mp mtg
  table`, `mp mtg win` / `lose`, `mp mtg save`. Logs: "mtg: your move: ...", "mtg: you: played / cast / used ...",
  "mtg: duel over: ...", "mtg: opened a CODE booster", "mtg: bought a CODE booster".
- Tests: tools/mtg/test.sh (the rules, triggers, the collection, boosters, the decks), scenarios mtg_shop, mtg_play,
  mtg_show, mtg_triggers.

## One-time legendaries come back (1.101.0)
- The user (2026-11-05): "One-time legendaries (Birth Island, Faraday, etc.): return 24 h after a KO, retroactively".
  src/mp/mp_legends.c keeps each KO's time in "<save>.legends" (lines `FLAG UNIXTIME`, the PC's own clock) and on every
  map entry (script.c RunOnTransitionMapScript, before the map's scripts) clears the done flag of each KO a day old, so
  the map's own scripts show the legendary again. HOENN: sHoenn (Emerald's DEFEATED flags: BIRTH ISLAND, FARAWAY,
  NAVEL ROCK, SOUTHERN ISLAND, the REGIs, MARINE / TERRA CAVE, SKY PILLAR), recorded by battle_setup.c
  CB2_EndScriptedWildBattle on B_OUTCOME_WON. JOHTO / KANTO: every legendary of qz_static_encounters.h (qz_specials.c
  AfterStatic; JOHTO's roaming RAIKOU / ENTEI left out). SINNOH keeps Platinum's own rule.
- Retroactive: a done flag with no time kept whose POKéMON was never caught (its own CAUGHT flag, or the POKéDEX
  where caught and beaten set the same flag) comes back on the next map change. A clock turned back skips the wait.
- Tests legend_return (an old KO back, a fresh one stays, KANTO's MOLTRES back), legend_return_due
  (MOEMON_LEGEND_RETURN=0); harness `mp legendko FLAG`.

## STREAMER MODE (1.101.0)
- The user (2026-11-05): "Streamer Mode in the options menu that obfuscates TRAINER names based on Gender to one of 3
  styles ... Game Style ..., Anime Style ..., Original Style ..., as well as obfuscation of POKEMON NICKNAMES to a
  selection of 3 nicknames per-species. The name you, other players, and Pokemon receive is decided via TID and SID per
  save file, as well as NATURE for Pokémon."
- OPTIONS > CO-OP > STREAMER MODE: OFF / GAME / ANIME / ORIGINAL (gMpConfig.streamer, ini `streamer_mode`).
  src/mp/mp_streamer.c: MP_Streamer_Build lists the names on screen (the player's, friends' in the room, link players'
  in a battle; nicknames that aren't the species' own name in the party, the foe's party and the PC's open box) with
  their stand-ins; mp_textopts.c MP_TextOpts_Filter (every string the text printer gets) swaps each whole word found.
  A trainer: the style's list for their gender (16 / 16 / 20 names a gender) by TID / SID; a POKéMON: one of three made
  from its species' name ("Torcy", "Lil Torch", "Torster") by its OT's TID / SID ^ nature. Only this screen's text
  changes; the naming screen shows what is typed. Test streamer_mode (`mp streamer N`, `mp namefilter`,
  tools/test/check_streamer.py).

## TRAINER HILL's floors and the POKéTCH CALENDAR (1.102.0)
- TRAINER HILL (trainer_hill.c SetUpDataStruct): the GBA copied a challenge's header and "the floors after it" in one
  CpuCopy32 (struct TrainerHillChallenge.floors[0] read out of bounds); on the PC sFloors_* aren't laid out after
  sChallenge_*, so the floors' tiles and trainers came from whatever followed: sFloorData copies each challenge's
  floors on their own. Test trainer_hill_floors.
- POKéTCH CALENDAR (poketch.c Calendar): a touch on a day rings it, another takes the ring off, as Platinum's; marks by
  month in "<save>.poketch" (version 4: calMarks). Its days are the cursor's targets (the targets' array is 40 now: a
  month has 31). The other apps were checked touch by touch (tools: `mp poketch N`, `mp poketchtap X,Y`). Test
  poketch_calendar.
- SINNOH's locked doors (s3d.c S3D_DoorLocked, event_object_movement.c GetCollisionAtCoords): Platinum's scripts keep
  a door shut by moving its warp off it (the VALLEY WINDWORKS until the WORKS KEY); its door tile was still walkable,
  and standing on it the door's sign was under the player, so A did nothing. A door tile with no warp on it is a wall
  now. Saves holding the WORKS KEY get Platinum's "has the key" flag (0x3564) on arriving (the map's own transition
  script). Test sinnoh_works_key.

