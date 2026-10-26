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
- Online card duels and player trading are a separate expansion, not this pass.

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
  and left/right remove/add controls, using the wider Emerald screen for a preview.
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
  Retreat / Done. The present basic duel sidebar still needs this alignment.
- `src/constants/duel_interface_constants.asm`: public play-area positions and
  card/attack detail pages. Do not expose an opponent's hidden hand/deck.

Milestone verification: rules/collection/Trainer/AI sanitizer suites passed;
Linux and Windows dev targets compiled; controller-input UI smoke tests and
main/dev save-protection regression passed. Windows runtime is not exercised
in this Linux environment. No full card-system release has been published.
