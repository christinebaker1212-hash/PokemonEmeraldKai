# K2 findings: VERMILION / S.S. ANNE, LAVENDER / POKéMON TOWER, CELADON / ROCKET HIDEOUT, ROUTES 7, 8, 11-18

Method: every K2 map's current `data/maps/Kanto_*/scripts.inc` + `map.json` compared with pokefirered
(`/tmp/claude-0/frlg/pokefirered/data/maps/<Map>/scripts.inc`, `text.inc`, `map.json`, `data/scripts/*.inc`, and
`data/text/*.inc` read from the clone's git objects, since that folder isn't checked out). Text: every string matched
against FRLG's (normalised for MOé/POKé, quotes, line breaks); scripts: command multisets with the Quetzal flag/var ids
renamed through `tools/quetzal_rip/kanto_frlg_flags.json`; objects, signs, triggers, item balls and hidden items by
position. Helper scripts and raw diffs: `/tmp/claude-0/-home-user/a6424581-b487-50fe-8bae-5e1098371be0/scratchpad/audit/tmp_K2/`
(cmp.py, cmds.py, textdiff.txt, cmddiff.txt). Paths below are relative to /home/user/PokemonEmeraldKai (now) and to
/tmp/claude-0/frlg/pokefirered (FRLG). Items already done in docs/REGIONS_TODO.md were checked and are not repeated
(TM balls, leader TMs, prize room, HM01 / HM02 givers, CELADON roof TMs, ROUTE 12 TM27, ROUTE 15 EXP. SHARE,
SOFTBOILED tutor, Vermilion stalls hidden, the ship's departure): all confirmed in the current files.

### K2-1: The S.S. ANNE "comes back": Quetzal's harbor words and pier check outlive the ship's departure
- Maps: Kanto_VermilionCity, Kanto_SSAnne_Exterior (also Kanto_SSAnne_1F_Room6, see K2-20)
- Category: A (text promises a mechanic the game no longer has; the pier check doesn't block) + C (rewritten lines)
- Original: after the ship sails (VAR_MAP_SCENE_VERMILION_CITY = 3) the pier trigger runs the SEAGALLOP check and, with
  no pass, "The ship set sail." and walks the player back (data/maps/VermilionCity/scripts.inc:190-223, text.inc:74).
  Old man: "So, the S.S. ANNE has departed? She'll be back … this time next year." (scripts.inc:44, text.inc:10).
  Sailor: "It visits VERMILION once a year." (text.inc:27). Ferry sailor before departure: "Welcome to the S.S. ANNE!"
  (text.inc:56). Harbor sign: "VERMILION HARBOR" (text.inc:53). SEAGALLOP prompt "Would you like to board a SEAGALLOP
  ferry?" with FRLG's multichoice (text.inc:77/87).
- Now: the pier trigger (data/maps/Kanto_VermilionCity/scripts.inc:321 Kanto_VermilionCity_EventScript_8416875) never
  checks VAR_QZ_0x5135 = 3: after the departure it still says "Welcome to the S.S. ANNE! Excuse me, do you have a
  ticket?" and lets an S.S. TICKET holder walk the pier to Kanto_SSAnne_Exterior, whose gangway warps (32,14 / 33,15)
  into the (now invisible, mp_mappic.c hides it by flag 0x2313) ship are still there. Lines: old man "The S.S. ANNE
  returns to this pier. You can visit with your TICKET." (:493), sailor "Even after its voyages, you can always come
  back for a visit." (:497), harbor sign "S.S. ANNE: at the end of the pier. Island ferry: speak to the sailor…" (:524),
  ferry sailor "The S.S. ANNE is at the end of the pier. To travel to the islands, talk to me once you have a pass."
  (:530), "There is no ferry available yet. You can visit the S.S. ANNE at the end of the pier with your TICKET." (:550),
  "Ah, you have a TRI-PASS. Choose a destination (up/down). A: Sail B: Cancel" (:555 and :560, RAINBOW PASS).
- Restore: in 8416875 branch on VAR_QZ_0x5135 = 3 like FRLG (SEAGALLOP check, else "The ship set sail." + walk back);
  FireRed's words for the old man (departed / next year), the sailor (once a year), the ferry sailor ("Welcome to the
  S.S. ANNE!"; after: "The ship set sail."), the harbor sign ("VERMILION HARBOR") and the pass prompts ("Would you like
  to board a SEAGALLOP ferry?"). Keep Kai's region menu ("Regular voyages / Other regions", QzGetReachableRegionsMask)
  and the scrolling destination list if wanted, only reword its prompt.
- Notes: REGIONS_TODO says the ship is "gone for good once it sails"; these lines and the open pier contradict it.
  Whether the gangway tile is walkable without the ship picture wasn't run in game: confidence high for the words,
  medium for the re-boarding. This also keeps the Mew truck (K2-19) reachable after the departure.

### K2-2: VERMILION's construction man and his MACHOP are missing
- Maps: Kanto_VermilionCity
- Category: D
- Original: OldMan2 "I'm putting up a building on this plot of land…" and the MACHOP ("Guoh! Gogogoh!" / "A MACHOP is
  stomping the land flat.") (data/maps/VermilionCity/scripts.inc:230-243, text.inc:15-25).
- Now: not in data/maps/Kanto_VermilionCity/map.json (Quetzal put its market row there; the stalls are hidden by flag
  0x2225 but the two FRLG people were never put back).
- Restore: add both objects at FRLG's places with FRLG's scripts / words (the stall canopies are map tiles; check the
  spot is walkable/free).
- Notes: REGIONS_TODO hid the stalls but doesn't mention these two. Confidence high.

### K2-3: VERMILION POKéMON CENTER: the VS SEEKER woman gives a PP MAX while talking about an ABILITY PATCH
- Maps: Kanto_VermilionCity_PokemonCenter_1F
- Category: A / E (and C)
- Original: she gives the VS SEEKER ("The urge to battle with someone you've tangled with before…", text.inc:20-44).
- Now: data/maps/Kanto_VermilionCity_PokemonCenter_1F/scripts.inc:34 gives ITEM_PP_MAX, but her lines (:72, :78, :83)
  say "This ABILITY PATCH is rare… An ABILITY PATCH can change a MOéMON to its Hidden Ability". The game has no
  ABILITY PATCH item (include/constants/items.h).
- Restore: FireRed's VS SEEKER gift and words if K2-4 gets a rematch system; otherwise at least make the words match
  the item (PP MAX) or drop the gift.
- Notes: depends on K2-4. Confidence high.

### K2-4: KANTO trainer rematches never happen (no VS SEEKER; rematch branches are dead)
- Maps: every K2 route with trainers (Kanto_Route8, 11, 12, 13, 14, 15, 16, 17, 18)
- Category: D
- Original: FRLG's VS SEEKER (given in K2-3) makes route trainers ask for rematches with their own lines
  (data/scripts/trainers.inc, e.g. :433-443 Route11 Hugo; data/text/trainers.inc rematch intros).
- Now: each trainer runs `specialvar VAR_RESULT, ShouldTryRematchBattle` + `TRAINER_BATTLE_REMATCH` (e.g.
  data/maps/Kanto_Route8/scripts.inc:12-20), but ShouldTryRematchBattle (src/battle_setup.c:2221) only knows HOENN's
  gRematchTable, so a KANTO trainer is never ready: the rematch texts (all present, FireRed's) are unreachable.
- Restore: either a KANTO rematch table driven by a VS SEEKER (FRLG) or by Kai's own MOENAV/MATCH CALL rematch rule;
  user decision.
- Notes: systematic across KANTO (other areas will see it too). Confidence high.

### K2-5: CELADON DEPT. STORE: Quetzal's floors, signs, clerks' words and menus over FireRed's shop lists
- Maps: Kanto_CeladonCity_DepartmentStore_1F, _2F, _3F, _4F, _5F
- Category: C (and E: the menus name what isn't sold)
- Original: 1F directory "2F: TRAINER'S MARKET / 3F: TV GAME SHOP / 4F: WISE MAN GIFTS / 5F: DRUGSTORE"
  (CeladonCity_DepartmentStore_1F/text.inc:7); 2F two plain clerks (items; TMs) + "SUPER REPEL…" NPC + floor sign
  "Top-Grade Items for TRAINERS!"; 3F TV GAME SHOP (no shop; the clerk is the COUNTER tutor, K2-7); 4F one clerk (POKé
  DOLL, RETRO MAIL, four stones), the POKé DOLL man and youngster, sign "Express yourself with gifts! … Evolution
  Special!"; 5F X items + vitamins clerks, the gentleman / sailor explaining stat enhancers, sign "5F: DRUGSTORE".
- Now: 1F directory "2F: CATCHING & TRAVEL… 3F: BATTLE SUPPLIES… 4F: BERRIES & CARE… 5F: FORMS & TREATS"
  (Kanto_CeladonCity_DepartmentStore_1F/scripts.inc:25) and Quetzal's floor signs / clerk prompts ("Which section would
  you like?") on every floor. src/shop.c:1536-1548 / 1408-1417 give the clerks FireRed's lists at run time, so the menus
  lie: 2F "Flutes" sells the TMs (+ defense items), "Special Balls" is empty at GEN 3; 4F "Berries & Gracidea" sells the
  POKé DOLL / mail / stones; 4F "Vitamins" sells the vitamins FireRed sold on 5F; 5F "Form-changing items" sells the X
  items (+ MEGA STONES / Z-CRYSTALS with 6 badges); 3F is a held-item / type-booster / weather-rock shop that FireRed
  doesn't have (LEFTOVERS, CHOICE items, LIFE ORB, QUICK CLAW…; src/shop.c has no base list for it, and
  tools/test/check_shop_lists.py's "no older item" rule doesn't see it because MP_ShopDump only lists changed clerks).
  FRLG's 4F/5F NPC lines replaced by Quetzal's ("I was looking for vitamins…", "treats from many regions…").
- Restore: FireRed's floors: directory, floor signs, NPCs and their lines, one clerk per FireRed list with "May I help
  you?" (no section menus); 3F back to the TV GAME SHOP (COUNTER tutor). Kai's newer-item extras can stay appended to
  FireRed's lists (as shop.c already does) without Quetzal's section menus.
- Notes: decide where Quetzal's extra lists (held items, berries, treats, MEGA/Z) should live (REGIONS_TODO says the
  Vermilion stalls' items are sold here). Confidence high.

### K2-6: Empty or misleading sections inside the Dept. Store menus
- Maps: Kanto_CeladonCity_DepartmentStore_3F, _5F
- Category: A
- Original: none (Quetzal's sections).
- Now: 3F "Gems" opens Kanto_CeladonCity_DepartmentStore_3F_Mart_841BD78 (scripts.inc:171), which is only ITEM_NONE
  and has no base list in src/shop.c: an empty shop. 5F's lines promise "nectars for ORICORIO's forms"
  (_5F/scripts.inc:66, sign and clerk "Treats and nectars") but no nectar is on any list (the items don't exist here).
- Restore: goes away with K2-5; otherwise drop the "Gems" entry and the nectar words.
- Notes: confidence high.

### K2-7: The COUNTER move tutor (Dept. Store 3F) is missing
- Maps: Kanto_CeladonCity_DepartmentStore_3F
- Category: D
- Original: the 3F clerk at (10,9) is EventScript_CounterTutor (data/scripts/move_tutors.inc:287; words
  CeladonCity_DepartmentStore_3F/text.inc:50-70 "Oh, hi! I finally finished POKéMON… COUNTER").
- Now: that clerk is Quetzal's battle-supplies shop (Kanto_CeladonCity_DepartmentStore_3F/scripts.inc:5).
- Restore: FireRed's COUNTER tutor on Kai's tutor flow (as fix_move_tutors.py did for SOFTBOILED etc.; COUNTER is
  missing from that list in docs/REGIONS_TODO.md "Decisions").
- Notes: confidence high.

### K2-8: GAME CORNER hidden coins give nothing
- Maps: Kanto_CeladonCity_GameCorner
- Category: A
- Original: 12 hidden coin piles (10-100 COINS each) (CeladonCity_GameCorner/map.json bg_events, "quantity").
- Now: the 12 bg_hidden_item_event entries are ITEM_NONE with no quantity
  (data/maps/Kanto_CeladonCity_GameCorner/events.inc:28-39); the hidden-item script (data/scripts/obtain_item.inc:178)
  does `additem ITEM_NONE`: no coins.
- Restore: a hidden-coins handler (FRLG's quantities: 10,10,20,10,10,20,10,10,10,40,100,10) via a small script per
  spot (checkitem COIN_CASE, addcoins N, set the hidden flag) or engine support for coin hidden items.
- Notes: confidence high (code read, not run).

### K2-9: The CELADON game designer's diploma words are Quetzal's (and don't match the check)
- Maps: Kanto_CeladonCity_Condominiums_3F
- Category: C / E
- Original: "Is that right? I'm the game designer! Filling up your POKéDEX is tough, but don't quit! When you finish,
  come tell me!" / "Wow! Excellent! You completed your POKéDEX! Congratulations! …" (HasAllKantoMons,
  CeladonCity_Condominiums_3F/scripts.inc:19, text.inc:15-25).
- Now: "Register every NATIONAL MOéDEX species as caught in KANTO. Catches from other regions don't count…" and
  "You've completed the NATIONAL MOéDEX with your KANTO catches!" (scripts.inc:81, 89), while QzIsRegionalDexComplete
  (src/qz_specials.c:158) checks KANTO's 150 caught anywhere, like FireRed.
- Restore: FireRed's two lines.
- Notes: confidence high.

### K2-10: Quetzal's "no TMs / moves from the team menu / learn from an expert" lines
- Maps: Kanto_CeladonCity_Condominiums_RoofRoom, Kanto_Route14, Kanto_SSAnne_2F_Room3
- Category: B
- Original: pamphlet "It's a pamphlet on TMs. … There are fifty TMs in all. There are also seven HMs that can be used
  repeatedly." (CeladonCity_Condominiums_RoofRoom/text.inc:45); Route 14 CARTER "You need to use TMs to teach good moves
  to POKéMON." / "You have some HMs, right? …", BECK "They have TMs on sale at the CELADON DEPT. STORE…"
  (Route14/text.inc:1-26); S.S. ANNE 2F gentleman "…CUT is an HM technique. Once learned, it is not so easily
  discarded." (SSAnne_2F_Room3/text.inc:5).
- Now: "Moves can be managed directly from the MOéMON team menu. Field techniques are unlocked by earning BADGES."
  (RoofRoom/scripts.inc:148); "You should review your MOéMON's moves from the team menu." / "Field techniques become
  available as you earn BADGES." / "They have useful items on sale… Evolution items are rare…" (Route14/scripts.inc:166-
  191); "CUT is a field technique. Earn the right BADGE and learn it from an expert…" (SSAnne_2F_Room3/scripts.inc:18).
- Restore: FireRed's lines. Kai has TMs (reusable) and HMs as items; Kai lets HM moves be forgotten
  (COOP_NOTES "HMs (Moemon Kai)"), so the "not easily discarded / can't forget" halves are slightly off for Kai: user
  may want them as FireRed's anyway (vanilla) or lightly adjusted.
- Notes: confidence high.

### K2-11: LAVENDER's NAME RATER removed
- Maps: Kanto_LavenderTown_House2
- Category: D (+ E: the replacement line is wrong for Kai)
- Original: the NAME RATER (LavenderTown_House2/scripts.inc:4, text.inc:1-45).
- Now: one line "You can change it whenever you like from the party menu." (Kanto_LavenderTown_House2/scripts.inc:12).
  Kai renames from the summary screen (SELECT), not the party menu, and keeps HOENN's Slateport NAME RATER
  (COOP_NOTES line ~474).
- Restore: FireRed's NAME RATER (Emerald's own Name Rater script can drive it; FRLG's words).
- Notes: confidence high.

### K2-12: POKéMON TOWER 6F MAROWAK: no ghost battle without the SILPH SCOPE, and the MAROWAK can be caught
- Maps: Kanto_PokemonTower_6F (and the wild ghosts on 3F-7F)
- Category: C (+ D)
- Original: "Be gone… Intruders…" then StartMarowakBattle in every case: without the SILPH SCOPE an unidentifiable
  GHOST the player can only flee (then pushed back); with it, MAROWAK, which can't be caught (FRLG's ghost battle type);
  win: "The GHOST was the restless soul of CUBONE's mother!" (PokemonTower_6F/scripts.inc:4-30). In FireRed the
  tower's wild POKéMON are also unidentified GHOSTS until the SCOPE (engine side, not verifiable in this clone: src/ not
  checked out).
- Now: data/maps/Kanto_PokemonTower_6F/scripts.inc:6: no SCOPE -> Quetzal's "A spirit blocks the way. You need the
  Silph Scope to identify it." and pushed back, no battle; with it QzStartMarowakGhostBattle (src/qz_specials.c:538) is
  a plain scripted wild battle, and the script treats outcome 7 (caught) as success: CUBONE's mother can be caught.
  No GHOST battle type exists (BATTLE_TYPE_GHOST was repurposed: include/constants/battle.h:92).
- Restore: FireRed's words; make the MAROWAK uncatchable (e.g. a battle flag that makes balls fail as FRLG's ghost
  dodge); optionally the unidentified-GHOST battles (needs engine work).
- Notes: confidence high for the script, medium for the wild-ghost part.

### K2-13: Hidden PP UPs became NUGGETs
- Maps: Kanto_CeladonCity (55,20), Kanto_RocketHideout_B1F (16,17), Kanto_Route13 (23,14), Kanto_Route17 (4,68)
- Category: C
- Original: hidden PP UP at each (map.json bg_events).
- Now: ITEM_NUGGET (events.inc of each map). Same swap elsewhere in KANTO: SilphCo_5F, ThreeIsland (PP UP),
  TwoIsland_CapeBrink (PP MAX).
- Restore: ITEM_PP_UP (PP MAX at CAPE BRINK).
- Notes: all other K2 hidden items match FireRed. Confidence high.

### K2-14: In-game trades differ from FireRed's
- Maps: Kanto_VermilionCity_House2, Kanto_Route11_EastEntrance_2F, Kanto_Route18_EastEntrance_2F
- Category: C
- Original: VERMILION: SPEAROW -> FARFETCH'D "CH'DING" (OT ELYSSA); ROUTE 11: NIDORINO -> NIDORINA "NINA" (FireRed);
  ROUTE 18: GOLDUCK -> LICKITUNG "MARC" (FireRed; LeafGreen SLOWBRO) (INGAME_TRADE_FARFETCHD / _NIDORINOA / _LICKITUNG).
- Now: src/data/qz_ingame_trades.h entries 10-12 (scripts set VAR_0x8008 0xA / 0xB / 0xC: VermilionCity_House2
  scripts.inc:8, Route11_EastEntrance_2F:26, Route18_EastEntrance_2F:18): FARFETCH'D for a TAILLOW (nicknamed
  "FARFETCH'D"), NIDORINA nicknamed "NIDORINA", LICKILICKY (not LICKITUNG) for GOLDUCK.
- Restore: FireRed's requested species, offered species and nicknames (keep OT names).
- Notes: COOP_NOTES says KANTO/JOHTO trades are Quetzal's 17 by design (crash fix); this is about their contents.
  Confidence high.

### K2-15: OAK's AIDES count the NATIONAL dex, not KANTO's
- Maps: Kanto_Route11_EastEntrance_2F (ITEMFINDER, 30), Kanto_Route15_WestEntrance_2F (EXP. SHARE, 50),
  Kanto_Route16_NorthEntrance_2F (AMULET COIN, 40)
- Category: C
- Original: `setvar VAR_0x8004, 0` before GetPokedexCount = KANTO dex caught (Route11_EastEntrance_2F/scripts.inc:63,
  Route16_NorthEntrance_2F/scripts.inc:29, Route15_WestEntrance_2F the same).
- Now: `setvar VAR_0x8004, 1` (Route11_EastEntrance_2F:67, Route15_WestEntrance_2F:30, Route16_NorthEntrance_2F:32) ->
  QzGetPokedexCount national count (src/qz_specials.c:142): a player arriving from HOENN qualifies at once.
- Restore: VAR_0x8004 = 0 (KantoCount).
- Notes: other K areas' aides likely the same. Confidence high.

### K2-16: ROUTE 12 FISHING HOUSE: MAGIKARP size record replaced by "show me one"
- Maps: Kanto_Route12_FishingHouse
- Category: C
- Original: after the SUPER ROD, the record-MAGIKARP measuring (ChoosePartyMon / CompareMagikarpSize, NET BALL for a
  new record, the record chart sign) (Route12_FishingHouse/scripts.inc, text.inc:17-90).
- Now: "Bring one in your party, and I'll have a gift for you!" -> NET BALL for any MAGIKARP; sign "Fishing notes:
  Patience, a rod, and enthusiasm." (Kanto_Route12_FishingHouse/scripts.inc:38-75).
- Restore: FireRed's record contest (Emerald has the same size-record machinery for SEEDOT/LOTAD to reuse) and words.
- Notes: SUPER ROD gift itself matches FireRed. Confidence high.

### K2-17: CYCLING ROAD words rewritten (get off the bike, SURF, DexNav), FRLG's slope missing
- Maps: Kanto_Route16, Kanto_Route17, Kanto_Route18, Kanto_Route16_NorthEntrance_1F, Kanto_Route18_EastEntrance_1F
- Category: C + B (DexNav) + D (slope)
- Original: gates "No pedestrians are allowed on CYCLING ROAD!" / "CYCLING ROAD is a downhill course by the sea…" /
  "CYCLING ROAD is all uphill from here." / "You need a BICYCLE to go out on CYCLING ROAD!"; signs "Enjoy the slope!
  CYCLING ROAD", "CYCLING ROAD Slope ends here!", "CYCLING ROAD No pedestrians permitted!", TRAINER TIPS "Press the B
  Button to stay in place while on a slope." (Route16/text.inc:73, Route17/text.inc:114/128, Route18/text.inc:37,
  gate text.inc files). ROUTE 17 pulls a cyclist downhill.
- Now: "You need a BICYCLE to enter. Once inside, you can get off your bike and explore on foot."
  (Route16_NorthEntrance_1F/scripts.inc:128), "…If you can use SURF, you can also explore the water…"
  (Route18_EastEntrance_1F:123), "CYCLING ROAD Enjoy the ride!" (Route16:264), "CYCLING ROAD Continue towards FUCHSIA
  CITY." (Route17:292), "A BICYCLE is required to enter. You may explore on foot inside." (Route18:108), TRAINER TIPS
  "Get off your bike and move slowly when approaching a DexNav MOéMON." (Route17:278; Kai has no DexNav). No downhill
  pull exists in the engine (src/field_player_avatar.c has only Emerald's muddy slope).
- Restore: FireRed's lines; optionally the downhill pull (engine work) and no dismounting on CYCLING ROAD.
- Notes: confidence high for words, medium for the slope (FRLG engine source not in the clone).

### K2-18: POKéMON CENTER 2F attendants explain Quetzal's "MPLAYER" multiplayer
- Maps: Kanto_CeladonCity_PokemonCenter_2F, Kanto_LavenderTown_PokemonCenter_2F, Kanto_VermilionCity_PokemonCenter_2F
  (also Kanto_CeladonCity_Condominiums_RoofRoom's Wireless Adapter lecture)
- Category: B
- Original: UNION ROOM / WIRELESS CLUB / DIRECT CORNER attendants (data/scripts/cable_club.inc:738, 826…).
- Now: Quetzal_EventScript_83661FF / 8366208 / 8366211 (data/scripts/qz/kanto_shared.inc:30-78, texts :2064-2100):
  "Start multiplayer from the START menu! Choose MPLAYER… Your game and cheat modes must…". Kai's online play is its
  CO-OP menu; there is no MPLAYER or cheat mode. The RoofRoom lecture (FRLG's own words) describes the Wireless Adapter
  rooms that don't exist here either.
- Restore: words that point to Kai's CO-OP (or FireRed's attendants' lines reworded for Kai's link features); low
  priority for the lecture.
- Notes: kanto_shared.inc is used by every KANTO 2F. Confidence high.

### K2-19: Quetzal's overworld legendaries / mythicals, MEW under the truck, COSMOG in a bag
- Maps: Kanto_VermilionCity (TAPU KOKO 17,26; CELESTEELA 26,11), Kanto_CeladonCity (TAPU BULU 6,35),
  Kanto_PokemonTower_3F (MARSHADOW), Kanto_PokemonTower_7F (GIRATINA: hidden, SINNOH's), Kanto_RocketHideout_B1F
  (COSMOG, the bag at 7,17 "..."), Kanto_RocketHideout_B4F (IRON CROWN), Kanto_Route8 (VIRIZION), Kanto_Route14 (IRON
  LEAVES), Kanto_Route15 (KARTANA), Kanto_Route17 (KORAIDON, MIRAIDON), Kanto_Route18 (TORNADUS), ARCEUS trials on
  Routes 7, 11, 14, 16, 18 (hidden: ARCEUS is SINNOH's), Kanto_SSAnne_Exterior (MEW under the truck,
  scripts.inc:61, needs a Lv 100 NIDOKING with STRENGTH after the LEAGUE)
- Category: C
- Original: none of these exist in FireRed.
- Now: Quetzal_EventScript_840FBBD objects (kanto_shared.inc:344); RefreshStatics shows them after KANTO's LEAGUE;
  docs/REGIONS_TODO.md "Decisions": Gen 5-9 legendaries "have no home region here and stay", KANTO's own (MEW) stay.
- Restore: per the user's rule "Quetzal's fan additions aren't wanted": remove them, or keep by the recorded decision;
  MEW's truck event is not covered by any decision (FRLG has only the truck, no event).
- Notes: listed for completeness; the decision already exists for Gen 5-9, so user call. Confidence high.

### K2-20: S.S. ANNE 1F ROOM 6: Quetzal's bed, DARKRAI dream and "leave the ship" shortcut
- Maps: Kanto_SSAnne_1F_Room6
- Category: C
- Original: only the woman's "take a short rest" heal (SSAnne_1F_Room6/scripts.inc).
- Now: a bed sign "The bed looks comfortable. Would you like to sleep?" (heal), the DARKRAI dream object (disabled by
  decision; code still there), and after healing the woman asks "Would you like to leave the ship and return to
  VERMILION?" and warps you to the departure (data/maps/Kanto_SSAnne_1F_Room6/scripts.inc:11-37).
- Restore: FireRed's room (the woman's heal only); drop the bed sign events (4,5)/(4,6), DARKRAI object and the warp
  offer.
- Notes: confidence high.

### K2-21: JESSIE & JAMES travelling event, and its reward is broken
- Maps: Kanto_VermilionCity (objects 18-21), Kanto_LavenderTown (4-7), Kanto_CeladonCity (17-20)
- Category: C + A
- Original: none.
- Now: after the LEAGUE, one town a day (QzKantoTravelingEventCheck, src/qz_specials.c:1034-1066) shows them; a Lv 50
  battle (TRAINER_QZ_913) then `qz_givemon_ex VAR_0x8000` with VAR_0x8000 copied from VAR_QZ_0x514A
  (data/scripts/qz/kanto_shared.inc:156-195), which nothing ever sets: the gift is species 0 while the text says
  "received PIKACHU!".
- Restore: remove the event (brief: unwanted addition); if kept, set the species.
- Notes: confidence high (no setter anywhere in data/ or src/).

### K2-22: The rival's name is hard-coded BLUE
- Maps: Kanto_PokemonTower_2F (scripts.inc:95), Kanto_SSAnne_2F_Corridor (:169, :185), gym statues
  Kanto_CeladonCity_Gym (:255, :261), Kanto_VermilionCity_Gym (:417, :423)
- Category: E
- Original: {RIVAL} (e.g. CeladonCity_Gym/text.inc:145-151).
- Now: "BLUE" (26 lines in KANTO); no KANTO rival naming exists (JOHTO got one).
- Restore: {RIVAL} with a KANTO rival name (as JOHTO's), or leave BLUE as a decision.
- Notes: systematic (other K areas). Confidence high.

### K2-23: GAME CORNER photo printer "out of service"
- Maps: Kanto_CeladonCity_GameCorner
- Category: D (low)
- Original: the POKéMON printer, a ¥50 TRAINER CARD photo (data/scripts/trainer_card.inc:1).
- Now: "The photo booth is out of service." (scripts.inc:512).
- Restore: keep (no printer) or hook Kai's TRAINER CARD; low priority.
- Notes: confidence high.

### K2-24: Leftover Quetzal words no script uses
- Maps: Kanto_SSAnne_CaptainsOffice (METAL COAT lines, scripts.inc:77-90), Kanto_Route16_House ("received a RAZOR
  FANG", :44)
- Category: E (low)
- Original: n/a.
- Now: unreachable after the HM fixes.
- Restore: delete the dead texts (keeps future greps honest).
- Notes: confidence high.

### K2-25: ROCKET HIDEOUT B2F item balls moved
- Maps: Kanto_RocketHideout_B2F
- Category: C (low)
- Original: SUPER POTION (0,14), MOON STONE (2,5), TM12 (5,7), X SPEED (15,3) (RocketHideout_B2F/map.json).
- Now: MOON STONE (0,6), SUPER POTION (1,17), TM12 (5,13), X SPEED (15,3): same items, Quetzal's places.
- Restore: FireRed's positions if the tiles allow.
- Notes: S.S. ANNE 1F ROOM 2's TM31 ball also sits at (2,7) not (5,7) (noted in fix_kanto_tms.py). Low.

## Coverage
- Kanto_CeladonCity: K2-13, K2-19, K2-21 (SOFTBOILED tutor already back; otherwise same)
- Kanto_CeladonCity_Condominiums_1F: only trivial differences (TEA re-given if lost)
- Kanto_CeladonCity_Condominiums_2F: same as the original (FAME CHECKER journals out by decision)
- Kanto_CeladonCity_Condominiums_3F: K2-9
- Kanto_CeladonCity_Condominiums_Roof: same as the original
- Kanto_CeladonCity_Condominiums_RoofRoom: K2-10, K2-18 (EEVEE gift same)
- Kanto_CeladonCity_DepartmentStore_1F: K2-5 (Kai's YU-GI-OH! club person is Kai's own)
- Kanto_CeladonCity_DepartmentStore_2F: K2-5
- Kanto_CeladonCity_DepartmentStore_3F: K2-5, K2-6, K2-7
- Kanto_CeladonCity_DepartmentStore_4F: K2-5
- Kanto_CeladonCity_DepartmentStore_5F: K2-5, K2-6
- Kanto_CeladonCity_DepartmentStore_Elevator: only trivial differences (Emerald's elevator specials)
- Kanto_CeladonCity_DepartmentStore_Roof: only trivial differences (TMs fixed)
- Kanto_CeladonCity_GameCorner: K2-8, K2-23 (Kai's GAME CENTER guide is Kai's own)
- Kanto_CeladonCity_GameCorner_PrizeRoom: only trivial differences (fixed per REGIONS_TODO)
- Kanto_CeladonCity_Gym: K2-22 (TM19 fixed)
- Kanto_CeladonCity_Hotel: same as the original
- Kanto_CeladonCity_House1: same as the original
- Kanto_CeladonCity_PokemonCenter_1F: only trivial differences (Kai's card club / TT people)
- Kanto_CeladonCity_PokemonCenter_2F: K2-18
- Kanto_CeladonCity_Restaurant: only trivial differences
- Kanto_LavenderTown: K2-21
- Kanto_LavenderTown_House1: same as the original
- Kanto_LavenderTown_House2: K2-11
- Kanto_LavenderTown_Mart: only trivial differences (Kai's FRLG list in shop.c)
- Kanto_LavenderTown_PokemonCenter_1F: same as the original
- Kanto_LavenderTown_PokemonCenter_2F: K2-18
- Kanto_LavenderTown_VolunteerPokemonHouse: only trivial differences
- Kanto_PokemonTower_1F: same as the original
- Kanto_PokemonTower_2F: K2-22
- Kanto_PokemonTower_3F: K2-19
- Kanto_PokemonTower_4F: same as the original
- Kanto_PokemonTower_5F: same as the original
- Kanto_PokemonTower_6F: K2-12
- Kanto_PokemonTower_7F: K2-19 (GIRATINA hidden; the rest same)
- Kanto_RocketHideout_B1F: K2-13, K2-19
- Kanto_RocketHideout_B2F: K2-25
- Kanto_RocketHideout_B3F: same as the original (TM21 fixed)
- Kanto_RocketHideout_B4F: K2-19 (TM49, SILPH SCOPE, LIFT KEY same)
- Kanto_RocketHideout_Elevator: only trivial differences
- Kanto_Route11: K2-4, K2-19
- Kanto_Route11_EastEntrance_1F: same as the original
- Kanto_Route11_EastEntrance_2F: K2-14, K2-15
- Kanto_Route12: K2-4 (SNORLAX same; TM48 fixed)
- Kanto_Route12_FishingHouse: K2-16
- Kanto_Route12_NorthEntrance_1F: same as the original
- Kanto_Route12_NorthEntrance_2F: only trivial differences (TM27 fixed)
- Kanto_Route13: K2-4, K2-13
- Kanto_Route14: K2-4, K2-10, K2-19
- Kanto_Route15: K2-4, K2-19
- Kanto_Route15_WestEntrance_1F: same as the original
- Kanto_Route15_WestEntrance_2F: K2-15
- Kanto_Route16: K2-4, K2-17, K2-19
- Kanto_Route16_House: K2-24 (HM02 fixed)
- Kanto_Route16_NorthEntrance_1F: K2-17
- Kanto_Route16_NorthEntrance_2F: K2-15
- Kanto_Route17: K2-4, K2-13, K2-17, K2-19
- Kanto_Route18: K2-4, K2-17, K2-19
- Kanto_Route18_EastEntrance_1F: K2-17
- Kanto_Route18_EastEntrance_2F: K2-14
- Kanto_Route7: K2-19 (ARCEUS, hidden)
- Kanto_Route7_EastEntrance: same as the original
- Kanto_Route8: K2-4, K2-19
- Kanto_Route8_WestEntrance: same as the original
- Kanto_SSAnne_1F_Corridor: same as the original
- Kanto_SSAnne_1F_Room1: same as the original
- Kanto_SSAnne_1F_Room2: K2-25 (ball place; TM31 fixed)
- Kanto_SSAnne_1F_Room3: same as the original
- Kanto_SSAnne_1F_Room4: same as the original
- Kanto_SSAnne_1F_Room5: same as the original
- Kanto_SSAnne_1F_Room6: K2-20
- Kanto_SSAnne_1F_Room7: same as the original
- Kanto_SSAnne_2F_Corridor: K2-22
- Kanto_SSAnne_2F_Room1: same as the original
- Kanto_SSAnne_2F_Room2: same as the original
- Kanto_SSAnne_2F_Room3: K2-10
- Kanto_SSAnne_2F_Room4: same as the original
- Kanto_SSAnne_2F_Room5: same as the original
- Kanto_SSAnne_2F_Room6: same as the original
- Kanto_SSAnne_3F_Corridor: same as the original
- Kanto_SSAnne_B1F_Corridor: same as the original
- Kanto_SSAnne_B1F_Room1: same as the original
- Kanto_SSAnne_B1F_Room2: same as the original (TM44 fixed)
- Kanto_SSAnne_B1F_Room3: same as the original
- Kanto_SSAnne_B1F_Room4: same as the original
- Kanto_SSAnne_B1F_Room5: same as the original
- Kanto_SSAnne_CaptainsOffice: K2-24 (HM01 fixed)
- Kanto_SSAnne_Deck: same as the original
- Kanto_SSAnne_Exterior: K2-1, K2-19
- Kanto_SSAnne_Kitchen: same as the original
- Kanto_VermilionCity: K2-1, K2-2, K2-19, K2-21
- Kanto_VermilionCity_Gym: K2-22 (TM34 and the trash cans fine)
- Kanto_VermilionCity_House1: same as the original
- Kanto_VermilionCity_House2: K2-14
- Kanto_VermilionCity_House3: same as the original
- Kanto_VermilionCity_Mart: only trivial differences (FRLG list in shop.c)
- Kanto_VermilionCity_PokemonCenter_1F: K2-3, K2-4
- Kanto_VermilionCity_PokemonCenter_2F: K2-18
- Kanto_VermilionCity_PokemonFanClub: only trivial differences (BIKE VOUCHER same)
