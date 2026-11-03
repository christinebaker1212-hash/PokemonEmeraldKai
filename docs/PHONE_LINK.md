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

Next: Phase 3, the walk (docs/COMPANION_APP.md "Pillar 2"), training facilities and the Garden in your pocket.
