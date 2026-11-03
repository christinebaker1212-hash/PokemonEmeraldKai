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
`p`, Trainer ID `o`, level, held item, gender, shiny, nature, ability (its number in the ability table), ball, friendship, nickname (hex), moves; in the
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
  snapshot) and `./gradlew testDebugUnitTest` (Robolectric's real graphics: every page from that snapshot, pictures in
  app/build/screens).
- Release: the release workflow builds and signs `MoemonCompanion.apk` with the repository secrets
  `MOEMON_APK_KEYSTORE_B64` (the keystore, base64) and `MOEMON_APK_PASSWORD` (alias `moemon`); without them the release
  has no app. The key must never change: an APK signed with another key can't update the installed one.

Not yet: the relay route away from home (the app is AWAY off the home network), the link moment's ceremony, sounds,
search and sort, RELEASE, TMs and medicine, the Global / shared PC.
