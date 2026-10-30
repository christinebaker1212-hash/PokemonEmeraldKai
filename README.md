# The House Remembers (working title)

An original PC board game for 3-6 friends online: explore a haunted house that is built
room by room as you play, gather omens, and somewhere past the middle of the game the house
turns one of you against the others.

This branch is separate from the Moemon Emerald Kai project. It shares no code with it.

## What it is (and isn't)

It is a game in the *haunted house exploration / hidden traitor* genre. It is **not** a port of
Betrayal at House on the Hill: no rooms, cards, characters, haunt scenarios, rule text, art or
names from that game (or any other published game) go in here. Everything in `content/` is
written for this game. Genre ideas are fair game (tiles placed as you explore, a rising dread
track, a mid-game twist with a secret side); copying a specific game's content is not.

## How the online play works

```
 ┌─────────── player's PC ───────────┐        ┌──────── server ────────┐
 │  client (renders, sends INTENTS)  │ ─ws──▶ │  room (one per game)   │
 │  rules engine copy (predicts UI,  │ ◀─ws── │  rules engine (truth)  │
 │  shows legal moves)               │  VIEWS │  RNG + hidden state    │
 └───────────────────────────────────┘        └────────────────────────┘
```

- **Server-authoritative.** The traitor's secret goals, the face-down decks and the dice are
  hidden information, so no client is trusted with the full state. The server holds the state,
  rolls the dice, and sends each player a **view**: the state with what that seat may not know
  removed. (Peer-to-peer or lockstep, like a fighting game, would leak the traitor's papers.)
- **One rules engine, two places.** The engine is a pure function
  `apply(state, intent, rng) -> (state, events)`. The server runs it for real; the client runs the
  same code only to list legal moves and animate events. Written once, in TypeScript, shared by
  both sides.
- **Events, not diffs.** Each step returns events (`TileRevealed`, `DiceRolled`, `CardDrawn`,
  `TwistBegins`...) that the client plays as animations, filtered per seat like the views.
- **Replays and reconnects for free.** A game is its seed plus the list of intents. Rejoining
  sends you your current view; a saved game is a small JSON log; a desync bug is reproducible.
- **Rooms and invite codes.** A small lobby gives each room a short code (`HAUNT-4K7Q`). Friends
  type the code, nobody opens ports. The same server binary can also run on one friend's PC
  for LAN or self-hosting.
- **Turn-based, so latency barely matters.** WebSocket messages are enough; no rollback.

## Tech choices

| Part | Choice | Why |
|---|---|---|
| Rules engine | TypeScript, no dependencies | runs on server and client, easy to unit test |
| Server | Node + `ws` | one process holds many rooms; deploy anywhere (Fly.io, a VPS, a friend's PC) |
| Client | browser canvas (PixiJS) + HTML panels | plays in any browser for testing |
| PC build | Tauri (or Electron) around the client | the actual "PC port": a desktop exe/app |
| Content | JSON/YAML in `content/` | rooms, items, events, twists added without code |

Alternative if you'd rather have a game engine: Godot 4 with a headless Godot server. Same
architecture (authoritative server, per-seat views); more art tooling, less code sharing.

## Planned layout

```
engine/    rules engine (state, intents, events, views, seeded RNG) + tests
server/    lobby, rooms, websocket protocol, persistence of game logs
client/    renderer, input, animations, lobby screens
desktop/   Tauri wrapper for the PC build
content/   rooms, characters, items, events, omens, twists (all original)
tools/     content validator, replay viewer, bot players for testing
```

## Milestones

1. **Engine core**: house grid with doors, placing a drawn room, movement, traits with dice
   rolls, the dread track, seeded RNG. Bot-vs-bot games from the command line.
2. **Server + lobby**: rooms with invite codes, seats, per-seat views, reconnect, game logs.
3. **Playable client**: the house map, hand and character panels, dice and card animations.
   First online game among friends with placeholder art.
4. **The twist**: the turn where one player secretly changes sides, private rules for the
   traitor and the others, win conditions; 3-4 original twist scenarios.
5. **Content and art**: 40+ rooms over three floors, characters, items, events; sounds.
6. **PC build and release**: Tauri exe for Windows/macOS/Linux, auto-update, a hosted server.

## Open questions for the design

- Player count (3-6?), and should a bot fill an empty seat?
- How many twist scenarios at launch, and who writes their story text?
- Art direction: hand-drawn tiles, pixel art, or 3D-ish top-down?
- Where the server lives: hosted by us, or each group runs their own?
