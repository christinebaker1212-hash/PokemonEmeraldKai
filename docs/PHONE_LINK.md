# The phone link (Moemon Companion App, Phase 1)

The design is the user's, in docs/COMPANION_APP.md. This page is how the link works.

## Pieces

- `src/mp/phone_proto.c/.h`: the protocol core. Plain C with no game code, built by the game, by the test client
  (`tools/phone/phonecli.c`) and by the app through the NDK, so all three agree byte for byte. Crypto is Monocypher
  4.0.2 (`src/mp/monocypher/`, public domain / BSD-2): BLAKE2b and XChaCha20-Poly1305.
- `src/mp/mp_phone_net.c/.h`: the sockets (no game headers: winsock and the game's headers don't mix).
- `src/mp/mp_phone.c`: the game's side: "<save>.phone", pairing, sessions, the snapshot, orders, the PHONE LINK screen.
- `src/mp/qrcodegen/`: Nayuki's QR Code generator (MIT) for the pairing code.
- `tools/phone/gen_nopc.py` -> `src/data/mp_phone_nopc.h`: maps where the party can't trade places with the PC.
- `tools/phone/gate.py`: the Phase 1 gate (1,000 scripted edits, no POKéMON lost or copied).

## "<save>.phone"

Written when a phone pairs or is forgotten, and with every save (sdl2.c StoreSaveFile). It holds:
`MPHN`, version 1, the save's ID (8 random bytes: never the name or Trainer ID, which test clients and restarted games
can share), and up to 2 phones (16-byte phone ID, 32-byte key, name, when it paired; a third pushes out the oldest).
A new game gets a new ID (new_game.c), so the app knows a new game in the same slot is a different save. LoadGameSave
reads it (save.c); a save without one gets an ID, written with its next save.

## Pairing

OPTIONS > CO-OP > PHONE LINK opens the screen: a QR code, 6 letters to type, the phones paired. While it's open the
game accepts pairing. The QR code (alphanumeric, 33 modules at 3 pixels) holds
`MOEPHONE:1:ADDRESS:PORT:SAVE ID:SECRET:LETTERS` (SECRET: 16 random bytes in hex, stretched to the 32-byte pairing
secret by `PhoneProto_QrSecret`; the letters' secret is `PhoneProto_CodeSecret`). The phone sends (plain JSON)
`{"t":"pair","v":1,"phone":ID,"save":SAVE ID,"proof":BLAKE2b-keyed(secret, "pair:" ID SAVE),"name":...}`; the game
checks the proof against both secrets, makes a fresh 32-byte key for the phone, keeps it, and answers
`{"t":"paired","key":SEALED,"save":...}` (the key sealed with the pairing secret). Five wrong proofs and the letters and
secret change. SELECT on the screen forgets every phone.

## Finding the game and the session

The game listens on TCP 47610 (the next free port if taken; `MOEMON_PHONE_PORT` sets it for tests) while it has a
phone or the screen is open, and answers UDP broadcasts `MOEPHONE?` on 47611 with `MOEPHONE!SAVE ID:PORT:VERSION`.
Frames: a 4-byte big-endian length, then `P` + JSON (before the session) or `S` + sealed. A session:
`{"t":"hello","v":1,"phone":ID,"save":...,"nonce":24 bytes}` -> `{"t":"welcome","nonce":24 bytes,"v":1,"game":VERSION}`;
the session key is BLAKE2b-keyed(phone key, "sess:" phone nonce, game nonce). Sealed: 24-byte random nonce, 16-byte
mac, then (4-byte sequence number, JSON) under XChaCha20-Poly1305. Each side's sequence numbers only go up; a damaged
or replayed message closes the link. `{"t":"bye","why":...}` refuses a hello or a pair: `version` (the link's own
number, PHONE_LINK_VERSION, not the game's), `othersave`, `unknown`, `notpairing`, `wrongcode`.

## Snapshot and orders

`{"t":"snap"}` -> the save (ID, name as the game's text bytes in hex, Trainer ID, region, GENERATION, badges, money,
coins, the game's version), the place (map, map type, `pc`: the party may trade places with the PC here, `safe`: the
game is free on the overworld), the party, the box in use, all 40 boxes (name, wallpaper, 30 places), the BAG's 5
pockets, the POKéDEX (seen and caught as hex bit strings by national number). A POKéMON: species, egg, personality
`p`, Trainer ID `o`, level, held item, gender, shiny, nature, ability (its number in the ability table), ball (the item the summary shows: a special ball by its style), friendship, nickname (hex), moves; in the
party also HP, max HP and status.

Orders: `{"t":"order","id":N,"op":"move","from":PLACE,"to":PLACE,"p":..,"o":..,"tp":..,"to2":..}` (PLACE `{"box":B,
"slot":S}` or `{"party":P}`; p / o the POKéMON at `from`, tp / to2 the one at `to`, 0 / 0 for none),
`"op":"give","at":PLACE,"item":I,"p","o"` and `"op":"take","at":PLACE,"p","o"`. The game carries one out at once if
it's free on the overworld (MP_IsLocalFree, not in a battle), else answers `{"t":"queued","id":N}` and does them in
order when it is. The answer: `{"t":"done","id":N,"ok":1,"why":"","changed":[...]}` with the places as they are now
(the whole party when it changed, since party slots close up) and the BAG when it changed; or `ok:0` and why: `gone`
(the POKéMON isn't there any more), `changed` (the place's POKéMON isn't the one expected), `same`, `nopc` (party and
PC here: towns, cities, routes and buildings only, not caves, the Underground, the BATTLE FRONTIERs or the Elite
Four), `last` (the party's last POKéMON that isn't an egg), `full`, `egg`, `mail`, `noitem`, `cantheld`, `bagfull`,
`nothing`, `busy`, `bad`. An order's ID is answered once: sent again, it gets the same answer and is never done twice.
The place's `battle` is 1 in any battle: the app offers no orders then (Companion.locked). In a co-op or multi
battle the party is the player's own, set aside in the save block (mp_coop.c SavePlayerParty), never the battle's
mix with friends' POKéMON (mp_phone.c OwnParty). A POKéMON whose front picture's colors aren't its species' (shiny,
a Stadium hue) carries `pal`: its 16 colors (BGR555, 4 hex digits each); the export's front pictures are palette
indexes and data.json's species carry their own `pal`, so the app colors them exactly as the game does.
`{"t":"ping"}` -> `{"t":"state","rev":R,"queued":N,"applied":N,"place":...}`. `rev` (in "snap" too, and in a fresh
"done" that was carried out) is a hash of the party, the boxes, the BAG, money, coins and the POKéDEX: the app asks for
a new snapshot when the game's `rev` isn't its own (the player changed something in the game by hand).

Moves use the game's own functions (BoxMonToMon heals a POKéMON coming out of a box, as the PC does), the party closes
up (CompactPartySlots), the PC's rule about the last POKéMON holds, and held HMs stay as the PC keeps them.

## The gate

`python3 tools/phone/gate.py [EDITS] [SEED]`: a headless game with three boxes filled, paired with phonecli, then
random moves (box <-> box, party <-> PC, party <-> party), gives and takes, and a tenth of stale orders (the wrong
personality or Trainer ID) that must be refused. Each answer updates the client's model; every 100 edits a full
snapshot must equal the model, and the set of POKéMON (personality, Trainer ID, species) must equal the one at the
start: none lost, none copied. Passed: 1,000 edits with seeds 1, 2 and 3.

Test `phone_screen` (the screen, its QR code and letters). `mp phone [pair|forget]` prints the link's state.

## The app (android/)

A plain Android app (no AndroidX; minSdk 26), its screens drawn in the game's pixels by one custom view.

- `app/src/main/cpp/link_jni.c`: phone_proto.c and Monocypher through JNI (`Native`, `NativeCrypto`).
- `core/` (no Android code, run on a desktop JVM by `android/coretest/run.sh`): `Link` (frames, pairing, hello,
  discovery), `LinkClient` (a thread: the last address, else discovery; a snapshot, the waiting orders sent again,
  pings; a new snapshot when the game's `rev` isn't the model's), `Orders` (on disk until answered; "Couldn't do"),
  `Model` (the snapshot with each answer's places applied, on disk: the app opens on it, "AWAY 5 MIN AGO"), `Planned`
  (the waiting orders played on the model the way DoMove plays them, for the dashed outlines).
- Never the network on the screen's thread (Android closes the app: 1.70.0 did after every order): `LinkClient.send`
  hands every message to its sending thread; `Link.screenThread` makes the desktop test fail the same way.
- The screens: `GameView` (a whole-number scale, at least 240 pixels across; the link on top, BOXES / PARTY / BAG /
  DEX / SPAWNS at the bottom), `GameArt` (the game's font with its widths and two colors, icons, front pictures, BAG
  icons, PC wallpapers, window frames), the pages, `Menu` (the game's windows), `PairPage`, `ScanActivity` (camera2 and
  ZXing), `Updater` (the latest release's `.apk`, installed through PackageInstaller's "Install?").
- The art and tables come from the game: `MOEMON_EXPORT_APP=DIR ./pokeemerald64` (src/mp/mp_export.c: icons, front
  pictures, BAG icons, wallpapers as the PC draws them, window frames, the font, data.json) packed into sheets by
  `python3 android/build_assets.py` (assets/game, never committed). Run it before building the app.
- Build: `cd android && ./gradlew assembleDebug` (ANDROID_HOME set; NDK 27.2). Tests: `android/coretest/run.sh` (the
  core against a headless game: pairing, a move with the rev agreeing, a stale order refused, an order made away done
  on the next link, a change made in the game bringing a new snapshot, the spawn maps; `SNAP_OUT=DIR` saves a real
  snapshot) and `./gradlew testDebugUnitTest` (Robolectric's real graphics: ScreensTest draws every page from that snapshot,
  pictures in app/build/screens; LiveTest runs the whole app against a headless game: pairing, GIVE, MOVE, SPAWNS, by
  taps, after `make linux` and coretest/run.sh).
- Android draws the app under its bars (edge to edge, Android 15): MainActivity pads the view by the system bars'
  insets. Why the app last closed: CrashReport (its own Java crash note and Android's ApplicationExitInfo), COPY.
- Release: the release workflow builds and signs `MoemonCompanion.apk` with the repository secrets
  `MOEMON_APK_KEYSTORE_B64` (the keystore, base64) and `MOEMON_APK_PASSWORD` (alias `moemon`); without them the release
  has no app. The key must never change: an APK signed with another key can't update the installed one.

Not yet: the relay route away from home (the app is AWAY off the home network), the link moment's ceremony, sounds,
search and sort, RELEASE, TMs and medicine, the Global / shared PC.

## Phase 2: the device (docs/COMPANION_APP.md "Pillar 1: the second screen")

The roadmap's Phase 2 is the device: second-screen mode, the POKéTCH's and the POKéNAV's apps, the live map. Its
gate: touch feels instant on home Wi-Fi.

### The POKéTCH on the phone (1.71.0)

The app's DEVICE tab shows the game's own POKéTCH, live: no second copy of any app. `{"t":"watch","on":1}` starts it:
while on, the game draws the app into the LCD (sinnoh/poketch.c MP_Poketch_PhoneFrame: itself when the game's own
POKéTCH isn't out, which draws it anyway) up to 20 times a second and sends
`{"t":"watch","app":A,"color":C,"flags":F,"apps":BITS,"rle":HEX}` only when it changed: 192x160 shades 0-3,
run-length coded (a byte a run: the shade in the low 2 bits, the length less one above); `flags` 1 = BACKLIGHT,
2 = ringing (the colors inverted); `apps` the unlocked apps, a bit each. The phone colors it with the COLOR CHANGER's
table. `{"t":"touch","x":X,"y":Y,"d":0|1}` is the stylus (the LCD's pixels; merged with the game's own stylus while
its POKéTCH is out), `{"t":"watchapp","dir":±1}` the side button. `{"t":"watch","on":0}` (or the link closing) stops
it and saves "<save>.poketch". `{"t":"watch","none":1}`: the save has no POKéTCH where it is (MP_Region_DeviceHere).
The snapshot's `save.devices` is [POKéTCH, POKéNAV, POKéGEAR] (1 = has it). LiveTest opens it, presses the side
button and sees the next app.

### The POKéDEX, WHERE TO FIND and moves across boxes (1.71.0)

The POKéDEX page is made for a phone in the game's colors: the game's own list and entry screens are exported
(pokedex.c MP_DexListScreen / MP_DexInfoScreen: dex_list.png, dex_info.png; its title is taken from the list's) with
every footprint (footprints.png); the page has a search bar (GameView's keyboard: Page.typed / backspace /
typingDone), filters and cards with the types' colors (GameArt.typeColor, from the type icons). An entry: the picture
at twice its size on its type's color, the four games' texts, the size and footprint, WHERE TO FIND, base stats.
`{"t":"where","s":SPECIES}` -> `{"t":"where","s":..,"slots":[[group,map,method,slot,min,max,met],...]}`: every wild
slot that makes it (the GENERATION's pools); the app groups them by place and method, and rare ones (only slots of 1%
or less) are counted, not named, until met there. In BOXES, a POKéMON carried to a box's arrow turns the box
(BoxesPage.dragMove); MOVE stays on through swipes and the box list.

### USE (1.71.1)

`{"op":"use","at":{"party":P},"item":I,"p","o"[,"move":M]}`: an item on a party POKéMON as the BAG's USE does it
(mp_phone.c DoUse; party_menu.c MP_PhoneItemUse classifies by the item's field use: 1 medicine, 2 EV BERRY, 3 SACRED
ASH, 4 PP all, 5 PP one, 6 PP UP, 7 RARE CANDY, 8 TM / HM, 9 evolution stone; data.json's items carry it as "use").
The game's own effects (ExecuteTableBasedItemEffect), rules (MP_RareCandyCap, the TM checks, flutes and TMs kept) and,
for an evolution, its own scene (BeginEvolutionScene back to the field, started once it's free). A fresh answer adds
`learned`, `wants` (four moves known: taught in the game) and `evolve`. Refusals: notparty, noeffect, cap, knows,
cantlearn, full, cantuse, egg, noitem. LiveTest: a POTION, a RARE CANDY, a THUNDER STONE evolving PIKACHU in the game.

### Safety (1.71.2)

An order is written to "<save>.phone.busy" (its phone and ID) while the game carries it out; if the game closes in the
middle, the next start remembers it as `crashed` and answers so when it's sent again (BusyRecover). The app sends a
waiting order again at most 3 times (Orders.toResend) before listing it under COULDN'T DO. USE's evolution starts as a
co-op trade's does (a fade, CleanupOverworldWindowsAndTilemaps, then BeginEvolutionScene). Front palettes are read
through FirstColors (room for 64 colors: JIRACHI's has 64), never into 16. The POKéTCH draws for the phone on the field
only. The snapshot's `ball` is the item the summary shows (a special ball by its style). The app asks again for spawn
maps cached by an older game (no `region`), and says when the game is older than it.

### The devices, the live map, battle mode (1.72.0)

The DEVICE page takes on the region's device (HOENN the POKéNAV, SINNOH the POKéTCH, JOHTO and KANTO the POKéGEAR),
with chips to switch to any other the save has (DevicePage.device / show). Game side: src/mp/mp_phone_dev.c, through
mp_phone_int.h (the link's Buf / JSON helpers and OwnParty shared with mp_phone.c).

- The live map (MapPage): the town map pages as their full views show them (region_map.c MP_TownMap_Picture, exported
  as townmap_N.png; SINNOH's is Platinum's picture at twice the size; KANTO's black past its right edge made see-through
  by build_assets.py) and their cells' map sections in data.json `townmaps` (a cell (x, y) at ((x + 1) * 8,
  (y + 2) * 8)). The place adds `tm` [page, x, y] (MP_TownMap_Player: the cell the town map's own icon is on), `sec`,
  `time` [day of the week, hour, minute] and `friends` [[name, page, x, y, battling]] (the room's players;
  MP_TownMap_SecSpot). `{"t":"place","sec":N}` -> trainers [trainer, beaten, class, name] (people whose script starts
  with a trainer battle), items [item, picked up, hidden] (item balls' finditem, hidden items), berries [tree, item,
  stage], wild [species, seen, caught], over all the section's maps (tools/phone/gen_map_sizes.py makes the map groups'
  sizes). The regions shown: the one you're in and any with a BADGE (save `rbadges`). Markers: press and hold a cell
  (Platinum's six shapes), kept on the phone per save.
- MATCH CALL (CallsPage): `{"t":"calls"}` -> the list as pokenav_match_call_list.c builds it (special people, then
  registered trainers: class, name, place, rematch ready, trainer picture from trainers.png, the check page's four
  texts); `{"t":"call","k":0|1,"i":N}` -> the call's words from MatchCall_GetMessage / SelectMatchCallMessage, made only
  while the game is free (`busy` else; `r`: the call asked for a rematch).
- CONDITION (ConditionPage) and RIBBONS (RibbonsPage): a POKéMON's `cond` [COOL, BEAUTY, CUTE, SMART, TOUGH, sheen] and
  `rib` (pokenav_ribbons_summary.c MP_Ribbons_Of: its order) in the snapshot; ribbons.png and data.json `ribbons` /
  `giftRibbons` (a gift ribbon's words are the save's `gifts` kind).
- The POKéGEAR: CLOCK (the place's time), MAP, PHONE (`{"t":"gear","call":0|1}` MOM / PROF. ELM,
  `{"t":"gear","answer":0|1}` MOM's savings question) and RADIO (`{"t":"gear","radio":FREQ}`: the station's name, song
  and the show's next line, asked each second and a half), all mp_pokegear.c's own (MP_Pokegear_Phone*: their own
  state; `busy` while the game's POKéGEAR is open).
- Battle mode: in any battle the game sends `{"t":"battle","on":1,"mons":[...],"party":[...]}` when it changes (a look
  every 6 frames): each battler's side, species, level, status and types now; the player's own with HP, moves and PP,
  the others' HP only as a share; `{"t":"battle","on":0}` at the end. The phone shows the MOVE TESTER (each move against
  each foe by data.json's `typeChart`, the game's gTypeEffectiveness; STAB) and the party's HP.
- HIDE ON PC: `{"t":"watch","on":1,"hide":1}` puts the game's own POKéTCH away while the phone shows it
  (poketch.c MP_Poketch_PhoneHide).
- A sealed message the app can't read is passed over (Link.recv: `unreadable`), never a reason to link again.

The gate (touch feels instant on home Wi-Fi): LiveTest presses the POKéTCH's side button and times the game's next
app coming back (72 ms over the test's loopback, under the 250 it allows), and checks the live map's finds, MATCH CALL,
the radio, MOM's call and the battle feed against the game.

## Phase 3: the walk, training, the Garden in your pocket (1.73.0)

Game side: src/mp/mp_phone_walk.c (orders through mp_phone.c's Apply; the snapshot's `walk`, `train`, `today`, `now`).

- The walk (custody). `{"op":"walk","at":PLACE,"p","o"}`: one at a time (`walking`), never the party's last (`last`), no
  MAIL; a held HM back to the BAG. "<save>.walk" is written first (the POKéMON as it left, the phone, where it began,
  the badge cap), then the POKéMON leaves the save and the game saves at once (`cantsave` if it can't: it stays). A
  save that still holds the POKéMON the file names (closed between the two) drops the file. The answer and the
  snapshot (for that phone; `walkOther` for another) carry `walk`: id, sent, mapsec, region, cap, mon, wild [species,
  min, max] (its map's wild table, else its map section's, the GENERATION's), finds (the place's item balls and hidden
  items, then PICKUP's first finds), rules [10,000 steps a day, 2,000 a find, 3,000 a meeting, 2,500 a moment, 3
  meetings a day].
- Coming home. `{"op":"home","walk":ID,"steps":N,"finds":[...],"caught":[[species,level,ball],...]}` from the phone
  that holds it (`otherphone`, `nowalk` else): steps capped at 10,000 for each calendar day it was out (the PC's clock;
  DayNumber), finds only from the pool and no more than the steps earn, catches only from the wild table at its
  levels, 3 a day, to the PC (CreateMon, met there, the POKéDEX); EXP a step to the badge cap's level with the day
  care's level-ups (daycare.c MP_ApplyDaycareExperience), an EGG's cycles (256 steps each), Friendship moments
  (mp_life.c MP_Life_WalkHome: a MEM_WALK memory and its JOURNAL line, a Friendship step each 2,500 steps). Back in
  the party (else the PC), the file erased, saved at once; the answer's `home` says what came of it. A toast on the
  field (send-off and homecoming, with its cry). FORGET PHONES brings it home as it left.
- Training: `{"op":"train","at":BOX PLACE,"p","o","k":0 STAT|1 LEVEL|2 REST,"s":stat}` (`trainbox`, `slots`: 2 + one
  per 2 BADGES, up to 6), `{"op":"untrain","p","o"}`, `{"op":"collect"}` -> `gains` [p, o, kind, amount]. "<save>.train";
  gains from at most 24 hours since the last collection on the PC's clock: STAT an EV every 5 minutes (252, 510),
  LEVEL 10 EXP a minute to the badge cap, REST Friendship every 30 minutes and PP. Leaving the boxes ends it (the
  party: collected first; TrainTick every 2 seconds), so it never battles. The PC shows TRAINING where its item would
  be (pokemon_storage_system.c SetDisplayMonData).
- The Garden's kind acts: `{"op":"kind","at","p","o","verb":0 pet|1 BERRY,"item"}` by the Garden's own rules
  (mp_life_interact.c MP_Life_PhoneVerb: feelings, the bond with its Trainer, tastes, hunger; a real moment only when
  it was looking for food); `line` says what happened. `{"t":"journal","at","p","o"}` -> its JOURNAL's lines.
- The daily report's `today`: goals [text, progress, target] (mp_events.c MP_Events_PhoneGoals), berries [ready,
  growing], gatcha [free pulls, tickets].

App side: core/Walk.java (Android-free: readings spread over the days between them and capped at 10,000 each, a
restart's counter, finds and meetings by steps, the homecoming order); StepCounter (TYPE_STEP_COUNTER,
ACTIVITY_RECOGNITION), StepJob (JobScheduler every ~15 minutes: a reading, notifications, the widget), WalkPage (the
walker, FIND / MEET / HOME, TRAINING, GARDEN), CatchPage (MEET: a ring to time, the species' catch rate), JournalPage,
TodayPage (once a day), Notifier (kinds on their own switches in MENU > NOTIFICATIONS, 3 a day, quiet hours 21:00 to
08:00, each kind once a day), WalkWidget. Forgetting a save is refused while one of its POKéMON is out walking.

The gate ("a week of real walks: steps, caps and homecomings right"): tools/phone/walk_test.py (the game: custody
across a restart; a week's caps with the walk's start moved back 7 days; home once; training an hour back; kindness;
FORGET PHONES), android/coretest WalkCoreTest (a simulated week on the phone's side), LiveTest (send-off, steps, a
find, homecoming through the app).

## Phase 4: friends (docs/COMPANION_APP.md "Pillar 4")

### Presence, invites, linking away from home (1.74.0)

The co-op server (mp_server.c, protocol 75) gained two services anyone connected may use without a room:
- MAIL (MPMSG_C_MAIL / S_MAIL): records under 16-byte keys. MAIL_PUT (one record, the owner's 16-byte secret: its
  SHA-256 locks the key to whoever put it first), MAIL_ADD (up to MAIL_ENTRIES entries of MAIL_ENTRY_MAX bytes, anyone
  who knows the key), MAIL_GET, MAIL_DEL / MAIL_TAKE (the owner's). Each has a TTL; kept in moemon_mail.dat next to the
  server (saved a few seconds after a change). The server never reads them: keys are derived from friend keys
  (PhoneProto_Derive: BLAKE2b keyed by the key, over a word) and the contents are sealed with them.
- PIPE (MPMSG_C_PIPE / S_PIPE): a game hosts pipe ids (PIPE_HOST), a phone joins one (PIPE_JOIN), and the server passes
  DATA between them in chunks of up to 7,900 bytes; PIPE_CLOSE / CLOSED either way. A connection that uses pipes is
  never timed out as an idle lobby visitor.

Game side: src/mp/mp_phone_friends.c, "<save>.pals" (magic PALS: this save's friend key and owner secret, the sharing
level, up to 64 friends {save id, friend key, name, last seen}).
- Friends: in a co-op room, each game sends its save id, name and friend key on relay type 19 (MPR_FRIEND) every 10
  seconds; everyone met is kept.
- Presence: while connected to a server and playing (in the overworld once since loading), every 60 seconds a MAIL_PUT
  under Derive(friend key, "presence"), TTL 10 minutes, sealed JSON {n (name hex), t, and with EVERYTHING: sec, reg,
  do (field / battle / coop / cards), srv ("" = the online lobby), room, rkey, code (a game hosted on that PC), party
  [[species, level, egg]...]}. ONLINE ONLY sends n and t; HIDDEN deletes the record.
- Snapshot `friends`: {share, srv, online, list [{id, key, n, seen}]}. Orders: `{"op":"share","level"}` and
  `{"op":"join","srv","room","rkey"}` (connects and joins the room; `inroom` if the game is already in one).
- The phone through the server: the game hosts a pipe for each paired phone, id Derive(phone key, "pipe"), and runs
  the same sealed link over it (mp_phone_net.c conns with `pipe`, PhoneNet_PipeOpen / PipeIn / PipeClosed).

App side: core/Server.java (MAIL from the phone), core/Friends.java (presence for the friend list: online when its
`t` is under 5 minutes old), core/Link.viaServer (the pipe: when the game isn't found at home, LinkClient tries the
snapshot's `friends.srv`; status LINKED (AWAY)), FriendsPage (the share level, friends online with where, what and
their party, JOIN: an order to the linked game, else the invite code copied), Notifier kind FRIEND (off by default;
once a friend a day, from StepJob), DexPage's SPAWNS button.

The gate: android/coretest FriendsTest (a server, ALICE hosting a room, BOB joining: friend keys swapped, BOB's
presence read from the server, ALICE's phone linked through the server with a snapshot of more than one chunk);
ScreensTest "friends".

### Trades by mail (1.75.0)

The server's ESCROW (MAIL ops 7-10, protocol 76; mp_net.h ESC_*; "moemon_escrow.dat", written before each answer):
a record per trade (its 16-byte ID) with two sides, each a deposit (up to ESC_DATA_MAX) and a lock (the SHA-256 of
the side's secret, set by its first use). PUT deposits (refused once CLOSED), TAKE gives the other side's deposit
only when both are in, CANCEL closes it while both aren't in (and gives back the caller's own), LOOK tells the state.
Every op can be repeated and answers the state. Deposits nobody came back for stay 365 days; tools/phone/escrow_test.py.

Keys: each save has an X25519 pair ("<save>.pals" version 2; 1.74.0's file is converted, its friends kept); MPR_FRIEND
ends with the public key. Two saves' pair key (PhoneProto_PairKey: BLAKE2b over the X25519 secret) seals offers and
deposits, so only those two can read them (friend keys are seen by everyone in a room). The snapshot's friends carry
`pk` (the pair key, once met since 1.74.1), `me`, `tk` (the trade key: a side's secret is Derive(tk, hex(trade))),
`inbox` and `inown` (Derive(friend key / secret, "inbox")).

The flow:
- OFFER (the phone, core/Trades.offer): a random trade ID; {t, from, n, mon (the snapshot's keys), want (species, 0
  anything), at, exp (7 days), srv} sealed with the pair key into the friend's inbox (MAIL_ADD, [from id][sealed]);
  then the order `{"op":"offer","trade","to","at","p","o","want","exp","srv"}` to this save's game (queued when away):
  the POKéMON stays where it is, ON OFFER (the PC's label, no release in the PC, no walk or training; `off` in the
  snapshot).
- The friend's phone reads its inbox (Trades.incoming: each offer with the escrow's state); ACCEPT is the order
  `{"op":"accept",...}` to its game (the species must be the one wanted: `notwanted`); DECLINE is an escrow CANCEL
  from side 1, straight from the phone. CANCEL of one's own offer: CANCEL from side 0 plus `{"op":"untrade"}`.
- The games (src/mp/mp_phone_trade.c, "<save>.trades"), whenever on the trade's server: the accepting game deposits
  (side 1); the offering game, seeing it (LOOK), deposits too (side 0) and takes; the other takes on its next look.
  Custody as the walk's: written down, out of the save, saved at once, then the server; received: written down, in
  (party, else PC), saved at once; a game closed at any step works out on loading where it stands (the save holds it
  or not). A deposit called off comes home the same way. Offers run out after 7 days (the accepting side waits a day
  more before taking its deposit back).
- Then the game's own in-game trade scene (trade.c MP_MailTrade_Scene: the friend's name, ours going, theirs coming,
  any trade evolution) the next time the player is free on the field.

App: core/Server.java (look / cancel), core/Trades.java, FriendsPage (TRADES BY MAIL: offers with ACCEPT / DECLINE,
our trades and how each is getting on, CALL OFF; TRADE on each friend: ours, then "for what": anything or one of the
species in their party), Notifier kind TURN (a new offer; from StepJob).

The gate (trades pass two-client tests at lag): android/coretest TradeTest: two games and their phones, everything
through tools/mp_server/lagproxy.py at 120 ms + up to 60 ms each way: a trade (each POKéMON exactly once, the scene in
both), a turned-down offer (nothing moves), an offer answered while the offering game is closed (it completes after).

### TRIPLE TRIAD on the phone (1.76.0)

The game's own engine in the app: android/app/src/main/cpp/tt_jni.c over src/tt (tt_duel, tt_ai, tt_catalog,
tt_common, tt_hands, tt_pools, tt_rating(s), tt_collection; the build includes a folder holding only include/tt, as
the game's include/ has its own strings.h), Java Tt.java: the catalog and the rules' words, duels by handle (start,
play: the events as JSON, state, the AI that sees only what its side may), opponents as the game makes them
(tt_field.c Fill) with their hands fitted by CARD SCALING (Tt_FitOpponentHand), and the daily puzzle (a position three
cards from the end where one or two plays win against the best defense; the same for a day; Best() is a full search).
The card faces: the game's art pack (graphics/tt/cards.bin) cut to the large faces and the extras by
android/build_assets.py (assets game/tt_cards.bin, ~10 MB); TtArt composes them as tt_gfx.c Tt_DrawCard does.

The game's side, src/mp/mp_phone_cards.c: the snapshot's `tt` {on, day (Tcg_Today), region, tier (badges), rules and
trade of the region, skill, active, rec, today {packs, packsMax 3, puzzles, puzzlesMax 1}, packs[5], decks[6] {n, c},
owned [[id, count]], seen [ids]}; orders `ttdeck` {deck, cards, name, active} (Tt_DeckValid and owned copies:
`deckrules`, `notowned`), `ttwin` {kind: duel | puzzle, harder, perfect} (a BRONZE pack, SILVER for a tough or perfect
win, SILVER for the puzzle; per calendar day in "<save>.phonecards": `today`), `ttopen` {pack} (Tt_OpenPack: the cards
in the answer's `cards`). Rev() takes in the cards and the trades (PhoneCards_Hash, PhoneTrade_Hash), so the app sees
them change. Tests: tools/phone/cards_test.py, coretest TtCoreTest.

The app: CARDS (CardsPage: PLAY a FREE DUEL against a duelist of where you are under the region's rules without
RANDOM, cards never change hands; the DAILY PUZZLE; VS FRIENDS; PACKS to open; DECKS; the BINDER), TtDuelPage (upright
table, tap or drag, captures turning in waves, SAME / PLUS / COMBO, the AI, the prize order), TtDeckPage (the game's deck
rules), TtPackPage, TtCardPage.

Turn by turn with friends (core/TtFriends.java over core/CardDuels.java, shared with the POKéMON card game): the invite
into the friend's inbox {kind "tt", g, from, n, hand, rules, seed, first, at}; the game's record under Derive(pair key,
"ttgame" + g), entries sealed with the pair key: the friend's hand (seq 0, or declined), then each move {seq, slot,
cell}. OPEN always; the rules offered replay the same with
either side as the engine's seat 0 (no SWAP, CHAOS, SUDDEN DEATH), so each phone plays as seat 0 (TtFriendGame). Moves
not taken by the server are sent again; 3 days without a move: CLAIM WIN. Notifier TURN: a challenge, your turn.
Test: coretest TtFriendsTest (12 games through a real server, both sides agreeing on every board).

### The POKéMON card game on the phone (1.78.0)

The game's own engine in the app: android/app/src/main/cpp/tcg_jni.c over src/tcg (tcg_duel, tcg_effects, tcg_trainers,
tcg_powers, tcg_ai, tcg_catalog, tcg_collection, tcg_decks, tcg_scale, tcg_ratings, tcg_levels, tcg_link, and
tcg_flow; the build's include folder holds include/tcg and mp_version.h), Java Tcg.java: the catalog, deck checks
(Tcg_DeckValidate with the owned counts), CARD SCALING (TcgScale_RateDeck in a thread: about 0.1 s; an opponent fitted
as tcg_field.c Tcg_FitOpponent does for an ordinary card player, from any of the game's decks), duels by handle (state
as a seat may see it: the other hand, both decks and the other side's set-up Pokemon hidden; events after a sequence
number; options: what each card in the hand, each attack, retreat, each Pokemon Power can do now and why not), the AI
(TcgDuel_AiActionLevel) and the rules state's hash (TcgLink_Hash).

What a move asks for is the game's own duel board's: src/tcg/tcg_flow.c (include/tcg/flow.h) holds the Trainers' and
Pokemon Powers' steps, the attacks' menus and targets, the filters for spaces and cards, whether a hand card / attack /
retreat can be used and why not, the retreat's Energy when there's nothing to choose; tcg_board.c uses them (its
screen and cursor stay its own), and TcgFlow is the same as a machine without a screen (begin a move, answer SLOT /
PICK / MENU questions, take the action) that the phone drives. Moves are small JSON actions {t, h, s, a, g, o, c, x}.
Tests: coretest TcgCoreTest (150 whole duels with one or both sides moving only through flows with random legal
answers against the AI: every action a flow builds is taken by the rules, every duel ends, a replay from the actions
ends the same, the other side's hand stays hidden). It found three of the board's own: a Basic put on the Bench while
setting up with the Active spot empty (no way on with no other Basic: now the Active spot first), evolving a Pokemon
put down this turn when another of the name could evolve (now only those that can), Scoop Up on the last Pokemon in play.

The cards' pictures: graphics/tcg/printed.bin (the scans, "TCGP") re-encoded as WebP by build_assets.py (assets
game/tcg_cards.bin, ~13 MB, kept uncompressed in the APK so TcgArt reads a picture at a time); decoded at the size shown.

The game's side, src/mp/mp_phone_cards.c: the snapshot's `tcg` {on, day, active, skill, rated, badges, wins, today
{packs, packsMax 3}, packs[11], decks[8] {n, ace, c[60]}, owned [[id, count]]}; orders `tcgdeck` {deck, cards (60, or
[] to empty a slot), name, active} (`deckrules`, `notowned`, `empty`, `activedeck`), `tcgduel` {won, kind duel | friend,
deck and opp ratings, level} (CARD SCALING learns from every FREE DUEL, won or lost; a win: a random booster, 3 a day
counted in "<save>.phonecards" packs[1]), `tcgopen` {pack} (Tcg_OpenPack: `cards` and `new`, the first copies). Test:
tools/phone/cards_test.py.

The app: CARDS has a switch between the two games (remembered). POKéMON CARDS: a FREE DUEL, VS FRIENDS, BOOSTERS, DECKS,
the BINDER by set (TcgBinderPage). TcgDuelPage: upright, their side at the top turned to face them (their Bench, Active
Pokemon, Prize cards, deck and discard pile, mirrored as across a table, as the game's own table turns them; the user
2026-11-03: TRIPLE TRIAD stays as it is), yours below, the hand along the bottom. A tap on a card shows it up close with what it
can do (and why not); a move's questions glow on the table (SLOT), open a card picker (PICK, in order where the order
matters, Energy by value for a retreat), or a list (MENU). What happens is called out from the events (attacks, coins,
damage numbers over the Pokemon, conditions, Knock Outs, Prize cards, the AI's Trainers shown). TcgDeckPage (stacks to
take out, the collection by kind and type to put in, the game's rules as you go), TcgPackPage (ten cards turning over,
the holo shining, NEW!), TcgCardPage (the scan, then the catalog's words).

With friends by turns: core/CardDuels.java kind "tcg" (the invite {deck, prizes 6}; the friend's deck at seq 0; each
action {seq, + the action} numbered in the order made, whoever acts posts the next). The inviter is the engine's
player 0 on both phones (TcgFriendGame). The server keeps 16 entries a record, so a record is in pages of 12 numbers
(page n > 0 under Derive(pair key, kind + "game" + g + "#" + n)); a TRIPLE TRIAD game stays in the first. Concede on your
own turn (the rules take it from the player to move). Test: coretest TcgFriendsTest (3 duels, ~250 moves, through a real
server, the two phones' rules hash equal after every move). `ONLY=tcg android/coretest/run.sh`.

### YU-GI-OH! on the phone (1.79.0)

The game's own engine in the app: EDOPro's ocgcore (C++ with its Lua, built by CMake as the game's Makefile EDO_* does)
and the game's bridge src/ygo (ygo_edo, ygo_edo_data, ygo_edo_codes, ygo_catalog, ygo_fusions, ygo_ai, ygo_collection,
ygo_rating, ygo_npc_decks, ygo_card_ratings, tools/ygo/stb_zlib.c) through android/app/src/main/cpp/ygo_jni.c, Java
Ygo.java. The engine's pack (graphics/ygo/edo.pak, every card's database row and script) is an asset handed over at run
time (YGO_EDO_PAK_EXTERNAL: ygo_edo_data.c YgoEdo_SetPak; the game keeps its .incbin), so it isn't built into the
native library once per processor type. JSON text from the engine goes through Java's own UTF-8 decoder (EDOPro's card
texts aren't always Java's modified UTF-8). The cards' pictures: graphics/ygo/printed.bin ("YGOP") as WebP by
build_assets.py (assets game/ygo_cards.bin, ~19.5 MB), read by CardPack (shared with the POKéMON cards' TcgArt).

Duels by handle: the state as a seat may see it (the other hand, both decks, face-down cards hidden from the other
side), the question (YgoQuery: its type, prompt, items with their words from EDOPro's strings, and the names to
declare), answers as picks (+ a zone), Ygo.check (would it be taken), the AI's answer at a level, events, the state's
hash. A FREE DUEL's duelist: one of the GBA games' decks fitted as ygo_field.c FitOnce does (Ygo_NpcDeckCards in
ygo_rating.c, shared with the game's duelists), the player's deck rated from its cards (Ygo_DeckRating) plus skill.

The game's side, src/mp/mp_phone_cards.c: the snapshot's `ygo` {on, active, skill, rated, badges, wins, losses, list
(the decks' Forbidden & Limited list), today, packs [[set, count]], decks[8] {n, ace, m[], x[]}, owned [[id, count]]};
orders `ygodeck` {deck, main, extra, name, active}, `ygoduel` {won, kind, deck, opp, level} (Ygo_SkillResult; a win a
booster of a random set that can be had, 3 a day in "<save>.phonecards" packs[2]), `ygoopen` {set} (Ygo_OpenPack: nine
`cards` and `new`). Test: tools/phone/cards_test.py.

The app: CARDS' third tab. YgoDuelPage: upright, their Spell & Trap and Monster Zones at the top turned to face them
(as the game's table: the user, 2026-11-03), the Graveyard / Deck / Extra Deck / Field Zone at the row ends, Life
Points in the middle, the hand along the bottom; defense position sideways. In the Main and Battle Phases the cards
with something to do glow (tap: the card up close with its words and its actions: SUMMON, SET, ACTIVATE with the
effect's words...), the phases are buttons; the other questions open a card picker (CARDS, SUM with the running total,
SORT, COUNTER, OPTION / ZONE when several), a list (OPTION, POSITION, ZONE), a yes / no, or the names to declare.
Chances to chain while nothing happens are passed (YgoDuel_QuietChance, as the game's table unless L is held); MENU:
STOP AT EVERY CHANCE. YgoDeckPage (Main and Fusion Decks, the collection by kind, the list's limits), YgoPackPage,
YgoBinderPage, YgoCardPage. Friends by turns: CardDuels kind "ygo", each move an answer {p, z} (or {concede: seat},
only on your own move so move numbers never collide); YgoFriendGame rebuilds the duel from decks, seed and answers.
Companion's friend duels are one set of methods for both "tcg" and "ygo" (duelGames / syncDuelGames ...).
Tests: coretest YgoCoreTest (24 whole duels answered as the phone's table does: every answer taken, replays agreeing,
hidden cards hidden), YgoFriendsTest (3 duels through a real server, a concession), `ONLY=ygo android/coretest/run.sh`.

## The UI / UX pass (1.77.0 on)

- BOXES: the box drawn as large as the screen allows in whole screen pixels (BoxesPage.bk: 1.5x on a 1080-wide phone),
  and ALL BOXES (the user's ask, 2026-11-03): every box at once (each wallpaper's framed inside under a name strip, the
  PC's places scaled), the party pinned on top; hold a POKéMON and drag it to any place of any box (the list scrolls
  near the edges); SELECT several, then tap a box's name to send them there (each a move order, to the box's empty
  places in the order picked; the party only near a PC). A box's name opens it in ONE BOX.
- ON OFFER (a trade by mail) as a small orange tag on box places, and on the PC card and the PARTY's cards.
- The tab bar: a 12x12 pixel icon for each page above its name, the chosen one on a raised plate (GameView TAB_ICONS).

### The app's sounds (1.81.0)

The game's own sounds, played by the game's own sound engine and recorded when the APK is built; nothing is written
for the app and nothing is committed.

- **The export** (src/mp/mp_export_sound.c, part of MOEMON_EXPORT_APP; MOEMON_EXPORT_NO_SOUND=1 skips it): each sound
  is played on the game's players and mixed by its mixer, frame by frame, with the output taken from
  Platform_QueueAudio (gMpAudioCapture: nothing reaches the sound device). Sound effects (the list in sEffects: the
  card tables', the PC's, the POKéNAV's, the BAG's...), a cry for every species (PlayCry_NormalNoDucking; a cry that
  doesn't start, a cry player still taken, is tried again after a longer quiet), fanfares, and the card games' duel
  songs as each scene plays them: MP_Kaisong_Request with a file of the scene (tt_board.c, tcg_board.c, ygo_board.c),
  so a scene's own .kaisong plays, with no map (a borrowed save block: the export runs at the first frame) so no
  place's music does. A song's loop is found by its first track's GOTO (music_player.c gMpGotoWatch, at pattern level 0):
  the loop's end is that frame, its start the first frame the track was past the GOTO's place; 50 ms more is rendered
  past the end for the app's crossfade. Two scenes with the same music (no file of their own) share one copy.
- **sounds.bin** ("MSND", the format at the top of mp_export_sound.c): IMA ADPCM, mono at 21024 Hz for sound effects,
  10512 Hz for cries (their own rate), stereo 21024 Hz for fanfares and songs (the mixer's 42048 Hz taken down through
  a windowed-sinc filter). About 16 MB: 1,050 cries 7 MB, six songs 7 MB. tools/phone/sounds_pack.py lists it or
  writes WAVs (songs twice round, crossfaded as the app plays them).
- **Sound.java**: an AudioTrack at the phone's own rate, low latency, opened while something plays (let go after 1.5 s
  of quiet). Sound effects on three voices (the same one again starts over; else the oldest gives way), one cry (the
  music at 85/256 under it, PlayCry_Normal's ducking), a fanfare (the music waits, then goes on), one song (decoded off
  the main thread, a quarter-second fade in, a crossfade over the loop's seam). The music needs audio focus, never
  starts while another app's media is playing (USAGE_MEDIA in the active playback configurations), and stops when the
  app goes behind something (MainActivity onPause: Sound.front(false)); it comes back with the page that asked for it.
  MENU > SOUND: SOUND EFFECTS (cries too) and MUSIC, each on its own switch.
- **Where**: pages opening and closing SE_SELECT (GameView.showOver), the tabs SE_RG_BAG_POCKET, the MENU SE_WIN_OPEN
  and its rows SE_SELECT, a refusal SE_FAILURE and a done SE_SUCCESS (the toast); the SUMMARY plays the POKéMON's cry
  as it comes up and on the picture (not an EGG's), the POKéDEX entry its cry once seen and SE_DEX_PAGE turning; a
  MATCH CALL rings SE_POKENAV_CALL. The card tables play what the game's boards play for every event (tcg_board.c
  BoardEvent, ygo_board.c Present, tt_board.c Play: each page's sound(); a TCG attack's sound by its animation through
  tcg_flow.c TcgFlow_AttackSound, the board's table moved there and shared, Tcg.attackSound), SE_SELECT on a choice and
  SE_FAILURE on a refusal, the scene's duel song (TCG / YU-GI-OH!: VS TRAINER against a card player, the UNION ROOM's
  against a friend; TRIPLE TRIAD: FireRed's VS TRAINER), the scene's victory song on a win, the music stopping on a
  loss, SE_FAILURE on a TRIPLE TRIAD draw. Pack openings: SE_RG_CARD_OPEN, each card SE_RG_CARD_FLIP as it turns
  (SE_SHINY for a rare one), MUS_OBTAIN_ITEM at the end, as the game's pack screens.
- Tests: tools/phone/sounds_test.py (every sound the app names and the attack table's are in the pack, a cry for every
  species, loops inside their songs with a crossfade tail, nothing silent or clipped), SoundTest (`./gradlew
  testDebugUnitTest`: the ADPCM decodes, songs go round their loops with no jump at the seam).

### The POKéDEX's details, the type chart, LOOK (1.84.0)

- **The export** (mp_export.c, data.json per species): `"ab"` abilities, `"eg"` egg groups, `"ey"` base EXP., `"evy"`
  the effort it gives (HP, ATK, DEF, SP. ATK, SP. DEF, SPEED), `"it"` held items (common, rare), `"ec"` egg cycles,
  `"fr"` base friendship, `"evo"` [method, param, target] from gEvolutionTable, `"lv"` [level, move] and `"tm"` a hex
  mask of TM / HM indexes; top level `"tms"` the move of each TM / HM index.
- **DexEntryPage**: chips that jump to ENTRY / STATS / TYPES / EVOLVES / MOVES / MORE (sectionY); ABILITY; TYPE MATCHUPS
  (x4 / x2 / x1/2 / x1/4 / x0 rows, "TYPE CHART >"); EVOLUTION (the family walked from its first stage, each with its
  method; a tap opens that stage's entry); MOVES BY LEVEL / TM & HM (a tap: the move's card, moveInfo); BREEDING;
  TRAINING. **DexPage**: SORTS No. / ABC / TOTAL, a type filter with its own picker, CHART; filter, order and type kept
  in the app's settings.
- **TypeChartPage**: CHART (18 x 18, the game's type order), DEFENSE (one or two types), ATTACK (one type); the
  numbers from the game's own gTypeEffectiveness through data.json.
- **Look.java** (MENU > LOOK): THEMES (Page.SKY / SKY2 / BANNER / BANNER_DARK), the window frame (GameArt.userFrame:
  frame 0 drawn as the chosen one), ANIMATIONS (GameView: pages slide in over SLIDE_MS, the tab plate over TAB_MS,
  menus grow in, all through GameView.eased; off, everything is at once: the tests run with them off), OPEN ON (a
  page, or LAST). Settings look_theme / look_frame / look_start / look_anim / look_last.
- SUMMARY: the friendship rater's words under FRIENDSHIP, POKéDEX ENTRY; a move's PHYSICAL / SPECIAL / STATUS.
  POKéGEAR RADIO: < > under the dial step half a frequency (DevicePage.step).
- Tests: ScreensTest shots dex_entry_more1-5, dex_types, dex_fire, types_*, look_menu, look_night.

### The POKé BALL page (1.85.0)

The SUMMARY's BALL tab (BallPage.java): the game's POKé BALL page and ball effect editor on the phone.
- **The clips** (src/mp/mp_fxclip.c, MOEMON_EXPORT_FXCLIPS=DIR; android/build_assets.py runs it after the export,
  unthrottled through the test harness, about 13 s; MOEMON_EXPORT_NO_FXCLIPS skips it): the game opens its own ball
  effect editor (mp_ballfx_editor.c, MP_FxClip_Active: the preview's pieces set by MP_FxClip_Next, the Moemon seen from
  the front, a shorter pause) on a PIKACHU and plays every SEND-OUT, PARTICLES and RETURN once with the rest DEFAULT and
  RED energy (a first loop unrecorded: the fade-in); sdl2.c hands each finished frame (effects composited) to
  MP_FxClip_Frame, which keeps the battle field (112 x 112) XORed with the frame before, as runs. build_assets.py packs
  them into fxclips.bin ("MFXA": 95 clips at 30 frames a second, each frame XORed with the one before, zlib; about
  3.5 MB). FxClips.java unpacks one clip at a time.
- **The data** (data.json): "balls" (mp_ballstyle.c MP_BallStyle_ExportJson: the 41 colors with their five shades,
  the POKé BALL icon's palette and the slots 7-11 a color fills, the 68 decorations' 5x5 icon art, colors and place,
  their groups, the special balls' items) and "ballfx" (each piece's name, the editor's words for it, where an earned
  one comes from; the sounds; the ready-made presets as [name, pack, sound]).
- **The snapshot**: each POKéMON's "bst" (ball style), "fx" (MP_BallFx_Pack) and "fxs" (sound) when not 0; the save's
  "fxu" (per kind COLOR, SEND-OUT, PARTICLES, RETURN: the pieces it may pick, a bit each in hex, the first first) and
  "fxp" (MY PRESET 1-6: [pack, sound] or 0). PhoneBall_Hash is in the rev.
- **The orders** (src/mp/mp_phone_ball.c, as the game's own page and editor allow): "ballstyle" {at, p, o, item,
  color, deco} (another kind of ball costs one from the BAG: "noitem"; a color not earned: "locked"; a special ball
  is a POKé BALL underneath with its number in the style), "ballfx" {at, p, o, fx, fxs} (each piece earned or the one
  it carries, else "locked"), "fxparty" {fx, fxs} (every POKéMON in the party but EGGs), "fxpreset" {slot, fx, fxs}.
  The app sends "special" with "ballstyle" for Planned's preview; the game works it out itself.
- **The page**: the stage plays the piece of the row last touched (the effect's most telling part at first; a tap
  plays it again), the ball's icon in its corner drawn as the game draws it (the red top's shades, the emblem); the
  info box tells of the piece, or where a piece not earned comes from. Rows THE BALL (BALL, COLOR, DECOR) and
  SEND-OUT & RETURN (PRESET, SEND-OUT, PARTICLES, ENERGY, RETURN, SOUND): < > step, the middle opens the row's list
  (every choice, LOCKED ones marked). SAVE (a ball from the BAG asks first), PRESETS (use or save MY PRESET 1-6), PARTY,
  UNDO / BACK. Pieces not earned can be looked at, never kept.
- Tests: tools/phone/ball_test.py (the orders against a headless game), ScreensTest ball_page / ball_list /
  ball_locked.

### The ME page (1.85.0)

The ninth tab, ME (MePage.java): Kai's own things (src/mp/mp_phone_kai.c).
- **The snapshot's "kai"**: "ach" [done, all], "pulls", "tickets", "free" (MP_Gatcha_FreeWaiting), "jar" (the CANDY JAR
  in the BAG), "candy" (gMpCandies, a count per type number), "time" [hours, minutes], "farm" (the save's own GARDEN
  BERRY farm, its open plots: [berry item, stage, minutes to the next, yield, times watered]; a friend's farm online
  isn't sent). PhoneKai_Hash is in the rev.
- **Asked for**: {"t":"ach"} -> "cats" and "list" ([category, name, words, status, reward, done]: mp_achieve.c
  MP_Achieve_Info, the menu's order); {"t":"gatcha"} -> "pulls", "tickets", "banners" ([name, tagline, free 1, free
  10]).
- **Orders**: "pull" {banner, how: 0 free 10, 1 free 1, 2 TICKET, 3 one, 4 ten} -> mp_gatcha.c MP_Gatcha_PhonePull:
  the reel's own RollPrizes (pity, the 10-pull's R or better, random Moemon and ball pieces, achievements, the save
  after), friends told of a UR / EX; the answer's "prizes" [{r rarity, k kind, id (a Moemon's species, an item; a set's
  first item, a ball piece as a POKé BALL), sh, amt, n (game text), note}]; "nopulls". "candy" {type: -1 every kind}
  (MP_Candy_Convert / ConvertType; "nojar", "fewcandy", "bagfull"; the answer's "made"). "rename" {at, p, o, name: game
  text as hex, 1-10 letters, no control codes ("badname"); any POKéMON but an EGG, as the summary's SELECT}.
- **The pages**: MePage (the TRAINER CARD drawn in the app: name, ID, money, POKéDEX, time, each region's BADGES;
  ACHIEVEMENTS with SEE ALL; MOEGATCHA with PULL; the CANDY JAR's kinds and CONVERT; the farm's plots, a ripe one's
  yield, a growing one's time left), AchievementsPage (ALL / TO DO / DONE, < category >), GatchaPage (banners < >, the
  pulls the banner can take now, a reel of its own that races while the game answers, slows onto the best prize and
  flashes for SUPER RARE and up, then the pull's prizes; AGAIN / BACK). The SUMMARY: RENAME by the name (the phone's
  keyboard; letters the game's font has; DONE sends it). "$" in the app's words is the game's money sign (GameArt
  encode: the game's "$" ends a string, which hid the BAG's money).
- Tests: tools/phone/kai_test.py, ScreensTest me / achievements / gatcha / gatcha_prizes.

### MOéMON or POKéMON (1.86.0)

The app follows the game's OPTIONS > CREATURES, or its own MENU > LOOK > CREATURES (AS THE GAME / MOéMON / POKéMON;
Look.creatures, "look_art"; Look.pokemonArt sets GameArt.pokemonArt every frame, the widget too).
- **Pictures**: mp_export.c exports both sets (ExportMonsArt: front/ and front_p/ by MP_MONART with the option set; the
  party icons are the same in both, so once), each species' "pal" (Kai's) and "ppal" (the original's), and the POKéDEX
  list's header both ways (dex_list.png, dex_list_p.png). The snapshot: each POKéMON's "pal" and "ppal" when not its
  species' (shiny, a Stadium hue: MP_BoxMonFrontPal for Kai's, GetMonSpritePalFromSpeciesAndPersonality with the
  option set for the original's), and the save's "art" (the game's setting). GameArt.front draws from front_N or
  front_p_N with "pal" or "ppal"; GameArt.monPal(mon) picks a POKéMON's own.
- **Words** (GameArt.words, on every string drawn or measured): with POKéMON, the game's own swap (mp_textopts.c:
  MOé -> POKé in any case, MOEMON -> POKéMON); with MOéMON, the app's POKéMON / POKéDEX / POKéNAV become Kai's MOéMON /
  MOéDEX / MOéNAV (Kai kept POKé BALL, POKéGEAR...).
- Tests: ScreensTest party_pokemon, dex_pokemon.

Next: the rest of the pass.
