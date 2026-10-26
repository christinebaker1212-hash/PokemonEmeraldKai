# Optional card system — implementation status

Feature branch: `dev/pokemon-card-system`. This is work in progress, **not a complete
or publishable first pass**. Main release workflow and main-game behavior must stay
unchanged. No card feature may appear in main builds until explicitly promoted.

## Agreed scope

- Optional FF8-style challenge of participating NPCs with Triangle / configurable
  equivalent. Normal interaction unchanged; no floating icons or unsolicited prompts.
- First valid challenge offers opt-in, Card Case, starter choice and optional tutorial.
  Declining leaves the system inactive and never removes an ordinary reward.
- Full original 228-card rules/AI reference, NPC challenges across Hoenn, every win
  earns a booster, no loss penalties or rematch cooldown.
- Collection, multiple decks, deck legality, inspection, unopened packs, practice,
  duplicate exchange and dedicated optional card rewards/banner.
- Smooth card movement and a safe adapter for Emerald's existing attack animations.
- Printed card images where appropriate, explicitly custom presentation for GB-only
  cards. High-resolution inspection, keyboard/controller navigation.
- Separate **one-time versioned artwork pack**, confirmed by the user. Keep routine
  updates small. Compress aggressively without sacrificing text readability.
- Dev collection changes are temporary; no writes to any real save or card profile.
- Online card duels are now explicitly requested by the user and implemented as
  a dev-only extension. Player trading remains outside this pass.

## Completed and tested

- Imported stable IDs, all 228 card definitions, 51 complete original decks, text,
  318 effect-command lists and original low-resolution artwork from pret/poketcg
  commit `2d81ec3cbff4ebb2fd3f1f770ba245227a702ead`.
- Native immutable catalogue, collection/deck storage, three original starter decks,
  four-copy-by-name validation, free Basic Energy, deterministic boosters,
  protected duplicate exchange, crafting, explicit versioned/CRC-protected profile
  encoding and read-only dev enforcement.
- Rules foundation: setup, independent RNG, piles containing physical card indices,
  Energy attachment/costs, evolution restrictions, damage/weakness/resistance,
  core conditions, Knock Outs, prizes, promotions, deck-out and sudden death.
- Explicit (incomplete) effect dispatch; unported effects fail closed rather than
  silently acting as basic damage. `tools/tcg/effect_coverage.json` records coverage.
- Emerald font glyphs and selected window-frame assets render the UI at native
  240x160; only the explicit close-up view uses high-resolution artwork.
- Dev Card Case item, opt-in starter flow, collection, deck editor/copy and NPC
  challenge integration are present. Runtime tests cover decline without opting in,
  Oldale field challenge, deck edits/copy and no save/profile writes.
- Collection and rules tests run under AddressSanitizer and UndefinedBehaviorSanitizer.
  A complete basic Scratch duel executes to a winner with card-conservation checks.
- Image host and GitHub API now work after network setting updates. The saved
  configuration still uses restricted access with the two required domains allowed;
  do not overwrite the user's network settings to make assumptions about full access.
- 165 confidently matched scans downloaded; other prints require review/fallbacks.
- Compressed prototype artwork pack: 139,219,277 source bytes -> 28,876,084 bytes
  including 160x220 thumbnails and per-file hashes (79.3% reduction). Adaptive
  progressive JPEG 4:4:4 uses the existing stb decoder and preserves source dimensions;
  per-image whole-scan and text-region PSNR threshold >=47 dB. This is the current
  partial matched-art pack, not a claim of all 228 scans being ready.

## Still required before the full first-pass dev release

- Complete and validate every original attack, trainer and power effect; the current
  118-entry attack command coverage is incomplete. All 34 Trainer handlers have
  selection/conservation tests, but their player-facing selection flows are unfinished.
  Several current formulae/edge cases still need source-by-source verification.
- Test special-condition timing, prevention, status immunity, energy conversion,
  trainer attachment expiration/stacking, simultaneous effects and card choices.
- Full original-deck opponent behavior and selection handling; no hidden-information
  cheating. A simple complete-duel test is not proof that every card works.
- Resolve remaining artwork mappings, distinguish holo variants/reprints, GB-only
  cards and original/metadata spelling or level differences. Never substitute an
  unrelated scan while claiming an exact print match.
- Content-pack installation/versioning/checksum verification and cache support.
- Finish the native 240x160 interface, Trainer selection flows, tutorial and duel
  inspection. Deck building follows the original type filters, in-deck/owned counts
  and left/right remove/add controls, using the wider Emerald screen for a
  compact, centered illustration-only preview. Full-card art remains in the inspector.
  The existing basic deck editor, opt-in/decline and real Oldale challenge have
  controller-input smoke coverage. Rename and more complete deck tools remain.
- Emerald animation adapter, NPC definitions, victory rewards and card-only MoeGatcha.
- Full functional/UI checks, Linux and Windows dev builds, opt-out and save protection
  regressions, build-time verification. Never add dependency/build work to Claude's
  main release workflow.
- Publish only a complete, verified dev build through Dev Release. The current
  workflow's manual branch guard must be adjusted deliberately for this feature
  branch if publishing from it; do not push experimental features as main.

## Commands and local assets

```
python3 tools/tcg/import_poketcg.py /tmp/moemon-poketcg-reference
python3 tools/tcg/generate_catalog.py
python3 tools/tcg/generate_effects.py
tools/tcg/test.sh
NOBUILD=1 tools/tcg/test_ui.sh
NOBUILD=1 tools/test/dev_save_test.sh
python3 tools/tcg/fetch_art.py
python3 tools/tcg/benchmark_art.py
python3 tools/tcg/build_art_pack.py
```

Original full scans under `graphics/tcg/printed/` and metadata download cache are
ignored. Keep them out of routine source/release commits. The compressed content
pack is in `dev/artwork/tcg-art/`; its immutable ID/hash is in `tools/tcg/art_pack.json`.
Do not recreate it in routine CI jobs. Publish it as a separately versioned asset
when artwork mapping is complete. Pillow/clang-format used for authoring are local
workspace tools, not new main-game runtime or release dependencies.

## Layout references

Use the pinned pret/poketcg source as the interface reference, adapting its
160x144 arrangement to Emerald's native 240x160 (3:2) canvas:

- `src/engine/menus/deck_configuration.asm`: `HandleDeckBuildScreen`,
  `HandleDeckCardSelectionList`, `PrintDeckBuildingCardList` and type counts.
  Keep type filters, count/owned columns, left/right remove/add, A for details
  and START for checking the completed deck. Extra width fits a side preview.
- `src/engine/duel/core.asm`: `DuelMenuData`, `DuelMenuFunctionTable` and Check
  shortcuts. The original six actions are Hand / Attack / Check / PKMN Power /
  Retreat / Done. The native duel menu now uses the same three-column/two-row
  arrangement. Check opens public play-area/discard views and the player's hand;
  an opponent's hidden hand/deck is never listed. The original glossary and
  advanced viewing shortcuts remain to be implemented.
- `src/constants/duel_interface_constants.asm`: public play-area positions and
  card/attack detail pages. Do not expose an opponent's hidden hand/deck.

Milestone verification: rules/collection/Trainer/AI sanitizer suites passed;
Linux and Windows dev targets compiled; controller-input UI smoke tests and
main/dev save-protection regression passed. Inspecting both play areas/discards
was also checked against an unchanged rules-state snapshot. Windows runtime is not exercised
in this Linux environment. No full card-system release has been published.

## Online card duels

The Card Case now has ONLINE CARDS. Join the same co-op room, both explicitly
open that screen, choose a connected friend and invite them. Accepting uses each
player's active validated deck. Invites never appear during ordinary Emerald play.
B cancels an invitation; B in a duel opens the existing concede confirmation.
Card duels wager no cards or money and do not grant farmable online win rewards.

`tcg_link.c` uses the existing TCP relay with extension ID 255 and card protocol
version 1. Existing main clients and the relay's base protocol are unchanged;
main clients ignore this optional message. Bump the card protocol version when
rules/catalogue or wire compatibility changes. This extension needs both clients
on a compatible dev build; ordinary main clients cannot accept card invitations.

The inviter is canonical player 0 and the recipient player 1. The presentation
swaps the recipient's view so their own hand is always on their side. Both peers
validate the decks and every action. Messages carry a session nonce, ordered
sequence and portable before/after rules hashes. Each action needs acknowledgment
before another local action. Invalid, stale or divergent actions stop the session.
Timeout/disconnect returns to the Card Case without a result or progression loss.
Do not serialize native C structs or send save/profile data.

This is deterministic peer replication, like a local link emulator: both client
processes contain the canonical duel, including hidden piles. The interface does
not reveal the opponent's hand/deck, but modified clients could inspect them.
It is not an authoritative competitive server or an anti-cheat implementation.

Verification: 20 deterministic paired-client engine duels under sanitizers;
real relay clients completing normal duels with matching winner/state hashes;
250 ms each-way latency plus 80 ms jitter with live actions and concession;
real peer disconnect leaving winner unset; malformed/foreign/stale packets;
turn ownership, consent, ACK gating, timeout and card conservation. Existing
native UI and dev save-protection tests also pass. The card rules/effects and
Trainer choice UI limitations listed above still apply; this does not claim a
complete 228-card release.

```
NOBUILD=1 tools/tcg/test_online.sh
NOBUILD=1 OUT="$PWD/dev/test-results/online-lag" tools/tcg/test_online.sh 250
```

Both native dev binaries are staged under `dev/builds/` by the normal dev helper.
Routine main release jobs and workflows have no added card-system build steps.
