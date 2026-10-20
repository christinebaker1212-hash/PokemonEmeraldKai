**Pokémon Emerald Kai: three-player co-op Triple Battles**

**Repository:** https://github.com/christinebaker1212-hash/PokemonEmeraldKai  
**Inspected branch:** `claude/clever-clarke-hqati2`  
**Inspected commit:** `ea41d48eae00d170bf339f34aa66683938e3950c`  
**Prepared:** October 20, 2026

This is an implementation brief and a paste-ready coding prompt. The repository was inspected through GitHub; the game was not modified, built, or run for this review. The observations below describe the inspected source. Proposed identifiers, new files, mechanics, and tests are recommendations, not features already present.

Implement unrestricted three-player co-op triples: three human players, each controlling one active Pokémon, against up to three active NPC or wild Pokémon. Preserve the existing single-player adventure, two-player co-op doubles, four-player rooms, and PvP features.

**Decisions to carry into implementation**

| Question | Recommended first release |
| --- | --- |
| Targeting | Doubles-style targeting with three Pokémon per side. No distance restrictions or Shift command. |
| Ownership | Each player commands only their own active Pokémon and replacements. An ally may use their own medicine on another player's Pokémon. |
| Player reserves | Two selected Pokémon per player in triples: one active and one reserve. Existing doubles retain their three-per-player arrangement. |
| Composite battle party | Keep `PARTY_SIZE == 6`. Pack triples as 2 + 2 + 2, with explicit mappings back to each owner's original six-slot party. |
| Room capacity | Keep four players. Three may participate in a triple; the fourth may spectate or remain in the world. |
| Enemy party | Up to six NPC Pokémon, with up to three active and a shared reserve pool. Wild encounters contain one to three individuals and no generated reserve Pokémon. |
| Trainer eligibility | A trainer must originally have at least three Pokémon in the authored party for this encounter/rematch variant, before co-op padding or scaling, to allow triples. Two-Pokémon trainers are capped at doubles. |
| Battle format | Store an explicit transient format field. Every bit of the existing 32-bit battle-type flag word is already assigned. |
| Legacy mechanics | Preserve singles/doubles behavior. Document intentional triples-only rules, including the proposed spread-damage modifier. |
| New PvP formats | Retain existing PvP and tournaments. Six-human 3-vs-3 PvP is a separate project and is not implied by six battler slots. |
| Recovery from disconnects | Start with a coordinated abort and reliable settlement. Host migration and mid-battle admission are separate future work. |

Two Pokémon per participant is a deliberate first-release limit. It fits the existing six-slot battle party while giving everyone a replacement. Supporting three or six reserves per participant later should use a separate owner-party abstraction, rather than enlarging Emerald's persistent party structures.

**What the current source establishes**

| Existing implementation | Consequence for triples |
| --- | --- |
| `mp_coop.c` uses `BATTLE_TYPE_TOWER_LINK_MULTI`, one `partner`, and a 3 + 3 party split. | A third player needs a participant roster and an owner map; another partner variable is insufficient. |
| `mp_battle.c` has one `peer`, `recvFlags[2]`, `PeerLinkId()` using XOR 1, and two-player readiness/synchronization. | The battle transport must address three real participants independently. |
| `include/constants/battle.h` defines four battlers; `BATTLE_PARTNER(id)` uses XOR 2. | IDs 4 and 5 would produce invalid partner IDs 6 and 7. Iterate ally sets instead. |
| Controller execution flags use four-bit groups; faint markers occupy bits 28–31 of a 32-bit word. | Merely increasing the battler count causes mask overlap and invalid faint-marker shifts. |
| Leech Seed encodes its source in two bits; infatuation has four source bits immediately below other statuses. | Source references for battlers 4 and 5 need a wider transient representation. |
| `BattleStruct.palaceFlags` is a byte with a size assertion depending on battlers plus move slots. | Six live battlers would fail that assertion; decouple the fields or widen the transient representation. |
| `RecordedBattleSave.battleRecord` uses `MAX_BATTLERS_COUNT` and must fit a fixed save sector. | Freeze the legacy recorded-battle format at four battlers. Increasing its stored array is not a safe capacity upgrade. |
| `CalculateBaseDamage` halves `MOVE_TARGET_BOTH` damage when exactly two opposing Pokémon are alive. | Three targets would bypass this exact-two condition unless handled explicitly. This repo does not currently use BW's spread calculation. |
| Surf's stored target is `MOVE_TARGET_BOTH`; Earthquake uses `MOVE_TARGET_FOES_AND_ALLY`. | Preserve Emerald's foes-only Surf initially. Earthquake provides existing friendly-fire combinations. |
| Helping Hand is stored as `MOVE_TARGET_USER`, and its effect selects the XOR partner. | A triples-aware target resolver must make either ally selectable for this particular effect. |
| Catch arrays and catch-owner values describe two wild Pokémon and two players. | Generalize individual capture records, explicit targets, and real thrower ownership. |
| Medicine targets are encoded in bits 12–14 of the selected item, and ownership uses the 3 + 3 split. | Replace the ownership inference and use explicit item-target data for the new session commands. |
| `MP_Events_OnCoopWin(partner)` increments a pair counter and an aggregate co-op counter together. | Calling it twice would count two battles and omit the pair formed by the two guests. |
| `mp_avatar.c` has one custom partner back-sprite tile/palette buffer. | Loading two distinct partners can overwrite that shared buffer; allocate resources per participant. |
| The test runner dispatches one- and two-client scenarios only. | Add real three-client and four-client coverage, including latency and nonconsecutive room IDs. |

Read the current versions of `CLAUDE.md`, `BUILDING_COOP.md`, the source files, and the test scripts before implementing. Some player documentation is less current than the code. Recheck these observations if HEAD has moved.

**Paste-ready implementation prompt**

You are implementing three-player co-op Triple Battles in the repository linked above. Carry out the following phases in order. Keep the work reviewable with focused changes and a working checkpoint after each phase. Continue through the complete feature and its validation; do not stop after scaffolding or a successful six-sprite screenshot.

Use the inspected commit as context, not as an instruction to overwrite newer work. Read applicable repository instructions, inspect the current branch, and adapt the exact function edits to its current source. Preserve unrelated changes. Report actual builds and tests separately from tests that could not be run.

The intended experience is three friends each controlling one active Pokémon in the same battle. All three can attack any legal enemy target, and ally-targeted support can reach either teammate. Left, center, and right are visual positions. There are no BW-style range exceptions, privileged center position, or formation-shifting turns.

Trainer triple eligibility is a hard pre-scaling rule: read the original authored party for the exact selected trainer/rematch variant before adding filler Pokémon, applying co-op party expansion, or choosing the battle format. An original party of one allows one player; an original party of two allows at most two players in a Double Battle; an original party of three or more may allow three players in a Triple Battle. Padding a two-Pokémon trainer to three or six must never make that trainer triple-eligible. Extra online players do not override this gate. For an ineligible third participant, explain that the battle has two player slots and allow spectating or remaining in the field. This is a start-of-battle eligibility check; an already-running Triple Battle continues normally when enemy Pokémon faint and fewer than three remain.

The implementation must include matchmaking, party selection, ownership, six-battler simulation, transport, targeting, replacement, items, captures, HUD, social features, rewards, progression, compatibility, and automated tests.

1. **Establish the baseline and separate live capacity from legacy formats.**

   Read `CLAUDE.md`, `BUILDING_COOP.md`, `tools/test/README.md`, `tools/test/regress.sh`, and the battle/MP headers. Establish a Linux build and run the existing regression suite before changing behavior. Preserve the baseline logs. Record the current persistent Pokémon, save-block, and recorded-battle structure sizes and relevant offsets.

   Audit uses of `MAX_BATTLERS_COUNT`, `MAX_POSITION_COUNT`, `MAX_LINK_PLAYERS`, `MULTI_PARTY_SIZE`, `BATTLE_PARTNER`, `BATTLE_OPPOSITE`, `BIT_FLANK`, `Random() & 1`, hard-coded battler IDs, and exact-two alive checks. Search controller masks, animation helpers, raw array strides, party-order packing, EXP routing, item AI, and script operands as well as ordinary battle logic.

   Classify constants by meaning. Two sides, four move slots, four room members, four animation frames, six party slots, and four legacy recorded battlers are distinct quantities. Do not replace every occurrence of 2 or 4.

   Introduce explicit counts for singles, doubles, and triples: two, four, and six live battlers. Expand live capacity to six only after replacing assignments that currently use `MAX_BATTLERS_COUNT` to mean 'a double battle'. Preserve the numeric identities of existing battlers and positions 0–3; append the third pair.

   In `src/recorded_battle.c`, retain the legacy four-battler serialized layout, checksum placement, sector-size assertion, and compatible playback. Decouple its stored array bounds and associated copies/loops from the new live capacity. Triples must not enter the legacy recorder, including incidental action-recording hooks. Make unsupported triple recording an explicit capability decision rather than silently producing a corrupt replay. Live spectator streaming remains supported.

   **Exit criterion:** singles, doubles, existing recorded battles, and the baseline test suite still work with capacity separated from active counts. Persistent save formats remain compatible.

2. **Create a battle model that represents format, participants, and ownership explicitly.**

   Add a transient battle-format enum or equivalent context with singles, doubles, and unrestricted triples. Do not allocate `1 << 32`, reuse an occupied flag, or silently widen every saved/wire battle flag. Keep existing flags where they retain their original meaning; make new format helpers authoritative for active-count and target behavior.

   Define a battle-session descriptor containing a unique battle/session ID, authority room-player ID, participant count, roster of real room IDs, compact battle-link IDs, encounter type, format, and synchronized encounter data. A room-player ID, link-player ID, battler ID, side, and party slot are different values.

   A suitable canonical mapping is:

   | Participant seat | Human battler | Composite party slots | Original party slots |
   | --- | --- | --- | --- |
   | 0 | 0 | 0–1 | Two chosen indices from owner 0's original party |
   | 1 | 2 | 2–3 | Two chosen indices from owner 1's original party |
   | 2 | 4 | 4–5 | Two chosen indices from owner 2's original party |
   | NPC team | 1, 3, 5 | Its own six-slot enemy party | Independent of human ownership |

   Keep canonical IDs consistent across clients. Highlight the locally owned Pokémon through presentation rather than changing simulation IDs. Existing doubles can retain their 3 + 3 packing behind the same ownership interface.

   Add helpers for side, slot-on-side, live allies/enemies, owner of battler, owner of composite slot, owner's selected slots, original slot mapping, available replacements, and whether a battler is present and alive. Proposed names such as `Battle_GetOwner`, `Battle_GetLiveAllies`, and `Battle_GetLegalTargets` are examples; use the repository's conventions.

   Validate a player owns an actor and replacement before accepting a command. Never infer ownership with `partyIndex < 3`, `partyIndex / MULTI_PARTY_SIZE`, a flank bit, or a room player's parity.

   Replace four-battler packing where necessary:

   - Represent controller readiness and outbound synchronization through helpers capable of tracking six battlers for every real participant. Separate masks per participant are a clear option. If retaining a packed word, derive every stride/mask and assert they cannot overlap; six battlers alone do not require increasing room capacity.
   - Move fainted-battler membership out of the four high bits of `gHitMarker` into a six-bit transient mask and migrate all reads/writes and script-facing aggregate checks. Preserve other hit-marker semantics.
   - Store Leech Seed source IDs and infatuation source masks in transient fields that represent IDs 0–5. Do not steal adjacent status bits or change saved Pokémon data. Handle clear-on-switch, fainting, Baton Pass, and relevant status-copy operations explicitly.
   - Split or widen transient Palace flags without changing the original facility rules.
   - Replace four-battler Mirror Move history strides such as `target * 8` with typed arrays or dimension-derived indexing. Audit initialization and resets too.
   - Size EXP participation by enemy battler/party identity rather than two flank buckets. Preserve the distinction between active battler slots and six-member party slots.

   **Exit criterion:** ownership and masks remain correct for battlers 4/5 and a roster such as room IDs 0, 2, and 3. Local debug battles can initialize all six actors without relying on the old two-human Tower Multi handshake.

3. **Generalize battle execution and targeting.**

   Start with `src/battle_controllers.c`, `src/battle_main.c`, `src/battle_util.c`, `src/battle_util2.c`, `src/battle_script_commands.c`, and the player/link/NPC controllers. Add a triples setup path rather than sending a third human through a stock Tower Link Multi setup that fabricates NPC link players.

   The encounter authority runs enemy AI, move resolution, battle scripts, and battle RNG as in the existing co-op architecture. Each client supplies only its owner's decisions. Use legal-target masks and explicit iteration everywhere that currently finds 'the partner' or flips between two enemies.

   Define the new resolver's behavior:

   | Target category | Triple behavior |
   | --- | --- |
   | Selected target | Any living, present Pokémon allowed by that move's existing category/effect; distance imposes no extra restriction. |
   | Opposing team / `MOVE_TARGET_BOTH` | All living, present opposing battlers, up to three. |
   | All others / `MOVE_TARGET_FOES_AND_ALLY` | All eligible other battlers, up to five; retain friendly fire. |
   | Self | The user only. |
   | Selected ally | Either living teammate, excluding the user unless that particular effect permits self-targeting. |
   | Team/field effects | Apply once to the appropriate team/field; do not run a team-wide effect separately for each sprite. |
   | Random opponent | Select from the complete legal opposing set using authoritative RNG. |
   | Effect-dependent target | Resolve through the effect's rules, including type-dependent moves; do not force it into a generic category. |

   Special-case Helping Hand through the resolver/effect metadata: its current table entry says USER and its script picks the XOR partner. In triples it needs an explicit choice of either ally. Preserve its existing one-boost-per-recipient behavior initially, with clear feedback when two players propose the same boost. Do not silently introduce multiplicative stacking or change legacy doubles.

   Preserve Emerald's current move categories. Surf remains foes-only in this first release; Earthquake and Explosion retain their existing all-other targeting. A later friendly-fire Surf change would be a separate, documented balance decision. Do not import moves or abilities from later generations as an unstated prerequisite.

   For triples only, use an initial spread-damage factor of 3/4 when a damaging spread move has at least two eligible live targets at move execution; use full damage with one. Treat this as a proposed balance rule. Snapshot the eligible target set before individual hit results so the factor does not change halfway through the same move. Protection and immunity do not turn the remaining target into an accidental full-power hit. Do not apply a damage factor to status moves, fixed-damage effects, or residual field damage merely because several actors are affected.

   Preserve legacy singles/doubles damage. The inspected `CalculateBaseDamage` uses a 1/2 factor for a particular exact-two condition; add a format-aware helper instead of letting that condition accidentally define triples. Make Reflect/Light Screen use their intended multi-battle reduction when three targets are alive; retain their existing rounding where applicable.

   Generalize accuracy, per-target secondary effects, contact abilities, immunity, damage accounting, redirection, and move-end processing. `MOVEEND_NEXT_TARGET` must traverse a target list rather than toggle an XOR partner. Spend PP once per move. Apply recoil, self-KO, drain, Shell Bell, and other move-wide bookkeeping with an explicit rule for multiple victims, avoiding duplicate application or unintended behavior changes in legacy formats.

   Fix source/target selection for Follow Me, Lightning Rod, Trace, Intimidate, Pressure, trapping, Counter/Mirror Coat, Bide, Future Sight, Wish, Transform, Substitute, and multi-turn moves. If several redirectors qualify, preserve the engine's existing ordering principles and define a deterministic extension to three actors. Do not add modern Lightning Rod immunity/stat boosts to this Gen III engine incidentally.

   Generalize fainting, simultaneous KOs, replacement prompts, and win detection. An individual owner's exhaustion does not end a battle while teammates can still fight. An absent ally slot stays absent until its owner has an eligible replacement, including a Pokémon restored by a teammate's Revive. Do not transfer command of an exhausted player's slot to another human by default.

   Preserve slot-based targeting across ordinary switches. If a selected enemy has disappeared before a single-target attack executes, extend the existing doubles retargeting policy to the complete legal enemy set using authoritative resolution. Ally support fails when its selected ally is unavailable rather than jumping to another person. Ball targets remain explicit individual wild targets and follow the cancellation rule below.

   Define RUN as a shared wild-encounter escape request visible in the planning UI and acknowledged through team readiness. Execute at most one team escape attempt per turn, using the designated runner and a documented comparison against the fastest living wild opponent rather than an arbitrary XOR-opposite enemy. A successful escape ends the encounter for all participants and settles earlier captures. A failed attempt spends the runner's action while other committed actions may proceed. Suppress duplicate RUN attempts in that turn. Preserve the existing prohibition/forfeit policy for trainer battles; do not reinterpret a trainer RUN command as a successful wild escape.

   Human replacements come from that owner's selected pool. NPC replacements come from the trainer's shared reserves, with a reservation mechanism preventing two fainted enemy slots or simultaneous switches from selecting the same Pokémon. Process all six actors' end-of-turn effects once in a deterministic order.

   **Exit criterion:** deterministic engine scenarios exercise six actors, all target categories, battlers 4/5 status sources, spread moves, three simultaneous KOs, replacements, and partial exhaustion. A move cannot reach an absent slot or a nonexistent XOR partner.

4. **Extend battle transport from one peer to a participant roster.**

   Generalize the battle-specific transport in `src/mp/mp_battle.c`. `src/mp/mp_grouplink.c` provides useful existing examples of per-seat queues, roster mapping, and group synchronization, but its contest matchmaking/lifecycle should remain isolated. Keep the existing NetLink entry points and existing contest/record-mixing callers compatible.

   Replace single-peer fields with participant-indexed readiness, receive flags, queues, link-player data, and synchronization sequences. Send a message to actual roster members, not every room member. NPC battlers do not require network seats or acknowledgements. The fourth player's spectator connection must never enter a battle-controller barrier.

   Preserve the current transport invariants:

   - Every sender receives the appropriate local echo exactly once.
   - Queued blocks enter `gBlockRecvBuffer` when the consumer requests status, not while it clears the previous flag and is about to read the previous block.
   - A controller command is given a chance to execute before a same-frame completion message can clear its state.
   - Readiness waits for all real battle participants to enter the battle before commands are sent.
   - Link send/receive tasks execute once per frame even when visual animations receive additional turbo steps.
   - A slow client does not cause the others to overwrite an unread block. Queue exhaustion produces an explicit controlled failure rather than silent command loss.

   Carry battle ID and phase/turn/sequence identity on new session messages. Validate payload lengths, sender membership, actor ownership, legal targets, item/party indices, and phase. Ignore stale messages after abort, retry, roster change, or the next encounter. Snapshot the roster before simulation begins; mid-battle admission is outside this release.

   Put command data into an explicit structure/encoding rather than additional high bits of an item ID. It should distinguish actor, action, move slot, target battler, target party member, replacement slot, and item ID. Keep integer widths and byte order explicit for new wire fields.

   Add turn-boundary state digests for debugging and test comparison. Canonical state includes parties, active mappings, HP/PP/status, transient source fields, field effects, RNG, inventory-action settlement, capture records, and turn number. It excludes pointers, graphics, text cursors, and client-local appearance. This is a diagnostic consistency check, not a requirement that nonauthoritative clients independently rerun the entire simulation.

   Update `include/mp.h`, relay dispatch in `src/mp/mp_main.c`, and `src/mp/mp_net.h` coherently. The co-op dispatch currently reserves 20 message IDs; stay within documented ranges or revise allocation and dispatch together. Bump `MP_PROTOCOL_VERSION` to the next version relative to the implementation baseline; the inspected value is 20. Preserve the existing helpful 'who needs to update' version-rejection messages and rebuild the standalone server as required.

   **Exit criterion:** three clients start, submit commands, animate, synchronize, and close a battle at zero latency and through the existing 250 ms / 80 ms jitter test profile. Nonparticipant room members cannot stall it. Existing two-client battles and contests still use their correct transport paths.

5. **Expand encounter joining and party selection.**

   Refactor `src/mp/mp_coop.c` around session participants rather than a scalar `partner`. Keep the authority associated with the encounter initiator; do not assume the room host is always that player.

   For trainers, extend the existing held-NPC and 'Join me!' flow subject to the original-party eligibility gate. Only trainers whose original authored party has at least three Pokémon may accept two friends or offer a choice to start doubles or wait for a third. Once three eligible participants have accepted and selected their Pokémon, start triples. A two-Pokémon trainer accepts at most one friend, offers no third-player waiting option, and stays capped at doubles even if co-op scaling later adds filler Pokémon. Preserve an easy route to battle alone. A declined or missing third player must not destroy an otherwise valid doubles session.

   Select two usable Pokémon per owner for triples. Default to suitable current party order and remember the selection where appropriate, while allowing a player to change it. A player with one usable Pokémon may join with one and an empty reserve. Eggs and fainted leads cannot be selected as active Pokémon. Keep the complete original six-slot party saved and its mapping intact.

   For wild land encounters, preserve the existing proximity, grass-tile, free-player, and busy checks. Gather up to two eligible nearby friends concurrently, using distance and a stable tie-breaker. Preserve the existing automatic team-up experience rather than inserting repeated confirmation menus into every grass encounter. Finalize as solo, doubles, or triples according to the committed eligible participants; timeout values must account for real relay round trips.

   Resolve competing encounter requests deterministically so two nearby encounters cannot reserve the same player indefinitely. A participant can be reserved by only one session. Release reservations, held NPCs, movement locks, and waiting bubbles on cancellation, refusal, timeout, disconnect, or failed initialization.

   Do not start the battle until party mappings and the final roster are acknowledged. Derive the maximum trainer format from the recorded original party count before scaling, then choose within that limit according to the committed roster. If the roster shrinks before start, recalculate the format and party limit and reconfirm the relevant selections. Do not repack a three-player party halfway through simulation or downgrade a running triple because an enemy fainted.

   Restrict promotion to supported co-op encounters. Preserve existing scripted battles, tutorials, Safari, Frontier, partner events, and fishing/surfing exclusions unless an individual path is deliberately implemented and tested. Preserve one-Pokémon trainers as one-on-one and enforce the two-Pokémon trainer doubles cap before scaling in every co-op entry path. Returning to the ordinary encounter path must preserve its original format; a solo player can still encounter a vanilla scripted Double Battle.

   **Exit criterion:** the initiating player can occupy any room slot; three players join one trainer or land encounter; refusal/timeouts fall back cleanly; a fourth player remains free; late requests cannot enter a running battle.

6. **Generalize medicine, capture targeting, and capture ownership.**

   Update `src/party_menu.c`, `src/item_use.c`, `src/battle_controller_player.c`, `src/battle_util.c`, `src/battle_script_commands.c`, `data/battle_scripts_1.s`, and co-op result handling.

   Medicine comes from the acting player's bag. Its target can be any of the six selected team Pokémon, active or benched. Show owner names clearly. A healing target is a stable composite party identity; resolve whether it is active at execution time rather than accidentally healing whichever Pokémon now occupies an old battler slot. A switch command still selects only the acting owner's reserve.

   Replace the hard-coded 3 + 3 medicine-owner calculation. Support HP healing, status cures, full restores, and permitted Revive/Max Revive effects without adding unsupported battle item categories. Preserve the existing restrictions on PvP items. Track item reservation, execution, cancellation/refund, and owner settlement explicitly so a failed start, duplicate message, or aborted battle cannot consume the same item twice or duplicate a refund.

   Wild battles may have one, two, or three active enemies. Represent each wild Pokémon and capture separately. The current `wildMon2`, `catches[2]`, two-value owner enum, `WildHasTwo`, and `OtherWildOut` helpers must become count/roster-based data and queries. Count remaining living uncaught foes rather than finding one opposite slot.

   Add an explicit wild target selector for balls. The L quick-ball shortcut must use the same selector if several wild Pokémon remain; it may skip it when exactly one remains. Preserve the remembered ball type. Do not keep `Cmd_handleballthrow`'s default assumption that the target is `BATTLE_OPPOSITE(gBattlerAttacker)`.

   The catcher is the owner of the actor that executed the throw. Only that owner receives the captured Pokémon. Each successful capture removes exactly that wild battler, and combat continues while another wild opponent remains. All clients receive controller-driven visual removal and ball-throw effects, including the third player's screen.

   Define conflict behavior before implementing it: if two players commit balls at the same wild Pokémon, resolve actions in the engine's deterministic item order. If the first throw fails, the later throw can still execute. If the target was already caught, fainted, or removed, cancel the later throw and release/refund its item once. Its turn remains spent; never silently redirect it to a different wild Pokémon.

   Preserve special-ball multipliers and cosmetic ball data. Level/Love and similar checks use the actual thrower's Pokémon; Quick Ball uses the shared battle turn; Dusk and encounter-specific checks use authoritative encounter context. Friend/Heal ball effects and the special/color/decor bits must survive capture delivery and party restoration. Do not turn a legendary or scripted encounter into three copies to fill slots.

   Check delivery capacity against the owner's original party and PC, not the temporary two-Pokémon selection. Refuse a throw when the owner has no delivery capacity, with clear feedback and no spent ball. If delivery still cannot complete unexpectedly, retain a pending owner-specific delivery record rather than discarding or duplicating the Pokémon.

   Preserve the shared Pokédex design: everyone may receive seen/caught flags, but 'CAUGHT BY' identifies the actual catcher and ownership of the Pokémon remains singular. Shared dex updates must not manufacture additional personal catches or daily catch rewards.

   **Exit criterion:** every owner can heal either ally or their bench, revive a benched Pokémon, catch any of three wild targets, and receive only their own catches. Cover same-target throws, partial catches followed by a KO or escape, a full original party, full storage, and repeated result delivery.

7. **Build readable six-Pokémon presentation and cooperative planning.**

   Update `src/battle_interface.c`, `src/battle_anim_mons.c`, `src/battle_anim.c`, animation helpers, `src/reshow_battle_screen.c`, `src/battle_message.c`, relevant controllers, `src/mp/mp_battlechat.c`, `src/mp/mp_emote.c`, and `src/mp/mp_avatar.c`.

   Add a format-specific three-per-side layout. Work inside the actual game framebuffer and emulated sprite/tile/palette budgets; enlarging the SDL window does not create additional logical UI space. Preserve the existing singles/doubles layouts. Use compact status cards with readable HP/status/owner identity, and make the selected target and locally controlled Pokémon unmistakable. Preserve the Moemon sprite proportions and art.

   A spread-move preview highlights every affected actor, including allies where applicable. A selected ally or ball target displays its owner/name. Target navigation must reach all six slots and skip absent ones using actual target legality. Never expose a target choice that the executor cannot honor.

   Extend the existing partner-team glance on R to show both teammates and their selected reserves. Preserve R's move-information behavior in the move menu, the quick-ball shortcut, controller-family button hints, and LT/RT battle emotes.

   Replace one-partner standby status with a compact list of the other participants' states. Let players select simultaneously. Add ally-only tentative intents such as move plus target, medicine target, switching, or ready. Carry turn and revision numbers so changing a choice supersedes the old preview. Keep other players' inventories private beyond information required for the chosen action.

   Add a ready/commit barrier for triples: tentative choices can change until the complete turn is committed, then the authority resolves the fixed commands. If an edit races with commitment, the authoritative committed revision wins and the UI must show that result. Preserve forced actions, recharge, and multi-turn moves as ready without requiring impossible input. Existing PvP standby must not reveal opponents' pending moves, bags, or switches.

   Resolve battle emote ownership through the participant/battler map. A phrase from the third player appears over that player's Pokémon rather than the first other ally. Clear bubbles when the referenced actor disappears and restore overlays after bag, party, or summary screens.

   Give each simultaneously visible trainer back sprite independent tiles, palette, frame images, and lifetime. The current single `gMpPartnerBackTiles`/palette buffer is insufficient for two custom partners. Update runtime back-picture table entries and callers as needed. Test three different custom looks on every client's screen, and honor each player's hidden-back option. Do not treat arrays of four animation frames as four-player arrays.

   Extend coordinate helpers, sprite priorities, ball trajectories, Substitute/Transform graphics, weather overlays, hit flashes, and screen-wide animation bounds to six actors. Preserve the existing visual speed-up behavior without running controller/network tasks extra times. Large network buffers should use the appropriate native allocation path rather than competing unnecessarily with the battle graphics heap.

   **Exit criterion:** native-size screenshots show readable six-actor status and target selection; all three owners' custom back sprites, emotes, ball styles, and team panels remain distinct through send-out, switching, fainting, and menu restoration.

8. **Settle results, EXP, progression, and disconnects by owner.**

   Refactor result state in `src/mp/mp_coop.c` into per-owner payloads. Each includes final selected Pokémon, original-slot mapping validation, EXP/EV/level data, learned/deferred moves, evolution eligibility, caught Pokémon, outcome, prize entitlement, and item-action settlement.

   Preserve complete original parties: restore all six saved entries, then apply final changes only to that owner's selected original slots. Untouched reserves and their held items, PP, identities, and ball-style metadata remain intact. Party ordering changes during battle must not lose the original mapping.

   Compute EXP from the existing participation/EXP Share rules, generalized to three active owners and their selected reserves. Support and protection players who participated normally should not need the final hit to receive their usual participation credit. Do not award full EXP to every Pokémon merely because another human joined. Preserve Gen III capture EXP behavior.

   Route EXP animations, active stat refreshes, and level-up/move-learning data to the correct owner, including battler 4 and an owner's benched Pokémon. Preserve deferred learning when a Pokémon already knows four moves and the post-battle evolution flow.

   Store deferred-learning queues per owner and provide enough bounded capacity for the supported level-up sequence. The existing single `MAX_LEARN` queue must not silently drop the third owner's entries. Validate queue counts and slot mappings when encoding and receiving results.

   Make buddy, TOGETHER, catch-up, and traded-mon treatment owner-specific. The inspected EXP path calls the local `MP_ExpBonusPercent()` while processing a composite party; a three-owner result needs each owner's validated, bounded bonus context rather than borrowing the authority's personal bonus for everyone. Synchronize the bonus inputs/snapshot, calculate once, and avoid applying it again on receipt. A participant's own Pokémon are not traded merely because their data lives in another client's temporary party.

   Give trainer-win money and appropriate badges/rematch rewards once per eligible participant, preserving existing reward amounts and daily caps unless deliberately adjusted. Use each participant's own story state for eligibility. Run required local badge/end scripts after link teardown. Do not copy arbitrary host event flags or replay unrelated story scripts on every guest.

   Tag final results and per-item/capture settlement with battle/owner identity. Resending a result after a delayed acknowledgement must not duplicate money, Pokémon, EXP, badges, pulls, or items. Retain completion receipts long enough for the supported reconnect path. If results can be replayed after process restart, use an owner-local durable pending-result/receipt mechanism with a defined recovery order; an in-memory boolean is not crash-safe exactly-once settlement. Do not claim an atomic transaction across three independent save files.

   Define disconnect behavior explicitly. Before start, release reservations and negotiate a smaller format. During battle, stop advancing when a required participant is lost and perform the supported coordinated abort. Restore every owner's mapped state, reconcile consumed items/catches from the last acknowledged settlement boundary, and issue no invented victory/badge rewards. Avoid silently restoring all spent resources to the pre-battle snapshot while retaining earned catches. A disconnect during final-result delivery must settle or retry the same result, not create a second reward event.

   Individual exhaustion is distinct from team defeat. Allow an exhausted player to inspect the team and communicate while the remaining players finish; a teammate's permitted Revive can make that owner's reserve available again. Retain the intended co-op defeat penalties and return flow. Existing field rescue starts outside co-op/link battles: finish and clear the battle session before invoking any supported rescue/whiteout logic. Do not introduce a nested rescue handshake into a live battle.

   Autosave must not serialize the temporary composite party. Let normal saving resume only after original-party restoration, result settlement, and any evolution/learning sequences are in a valid state. Follow-up field actions may resume independently per client once the shared battle has closed; one player's evolution menu must not hold the other two inside a link barrier.

   **Exit criterion:** players with different selected indices, bonus levels, badges, and learning/evolution states each receive the correct result exactly once under normal completion, repeated messages, disconnects, and return-to-field processing.

9. **Adapt existing features and encounter difficulty.**

   Apply the following feature changes as part of the release, rather than leaving triples as a separate battle demo:

   | Existing feature | Required adjustment | Main source targets |
   | --- | --- | --- |
   | Walk-up trainer co-op | Original-party eligibility gate, up to two guests where eligible, ready state, held-NPC cleanup, doubles/triples choice, names for both partners. | `mp_coop.c`, trainer interaction hooks |
   | Nearby grass team-ups | Gather up to two nearby eligible friends, choose format before start, authoritative one-to-three wild generation. | `mp_coop.c`, `wild_encounter.c` |
   | Trainer scaling | Record the original authored count before scaling and cap the format from that count. Pass participant count and active enemy count into construction; filler never changes eligibility. Preserve trainer identity, ace handling, filler restrictions, and maximum six Pokémon. | `mp_coop.c`, `battle_main.c` |
   | Enemy move AI | Score all legal targets, support both allies, consider three threats, reserve switches uniquely, and avoid duplicate trainer-item use. | `battle_ai_script_commands.c`, `battle_ai_switch_items.c`, `battle_util.c` |
   | Medicine and party screens | Owner labels and all six selected team slots; acting owner's bag, own-only switching. | `party_menu.c`, player controller, item scripts |
   | Balls and quick throw | Explicit selected wild target, actual catcher ownership, cancellation/refunds, partial-capture continuation. | player controller, `battle_script_commands.c`, `mp_coop.c` |
   | Special/custom balls | Correct thrower/target checks, cosmetics and friendship/heal effects preserved in owner results. | `mp_ballstyle.c`, `pokeball.c`, throw animations |
   | Shared Pokédex | Process all seen wild species; share flags while crediting the actual catcher and preserving one Pokémon owner. | `mp_dex.c`, catch/result paths |
   | Standby and team glance | Show both partners, selected reserves, and ally intent/ready state. | `mp_coop.c`, player controller |
   | Battle chat/emotes | Map each sender to their own battler, including the third owner. | `mp_battlechat.c`, `mp_emote.c` |
   | Custom appearances | Independent runtime resources for the two remote partner backs and their palettes. | `mp_avatar.c`, back-picture tables, partner controller |
   | EXP and TOGETHER | Snapshot per-owner bonuses and committed participant membership; preserve existing caps and battle-time meter behavior. | EXP scripts, `mp_together.c`, `mp_friends.c` |
   | Daily co-op goals | Count one battle, credit every distinct participant pair, preserve the active-friend eligibility rules. | `mp_events.c` |
   | Achievements and MoeGatcha | One personal event per eligible participant and existing daily/weekly pull caps; support players receive equal participation credit. | `mp_achieve.c`, `mp_events.c`, reward callers |
   | Gym wins and rematches | All eligible participants receive their own badge/rematch reward once; execute local scripts safely after teardown. | `mp_coop.c`, `mp_rematch.c`, `data/scripts/mp_rematch.inc` |
   | Spectating and bets | Let the fourth room member watch one participant's existing stream; bind outcome and wager to this battle ID. Abort/no result uses the existing refund behavior. | `mp_spectate.c` |
   | PvP battle flag and tournament | Preserve current matches, friendly/no-money semantics, busy checks, and concealed opponent choices. Do not count co-op triples as a PvP tournament match. | `mp_trainer.c`, `mp_tourney.c`, `mp_battle.c` |
   | Field rescue | Evaluate supported rescue/whiteout only after co-op teardown; individual slot exhaustion is not an immediate field rescue. | `mp_rescue.c`, battle-end callbacks |
   | Followers, piggyback, field lending, pranks | Suspend/restore through the common battle lifecycle; busy checks cover all participants; no pranks or shared-PC mutation during the battle. | corresponding `src/mp/` modules |
   | Trade/shared PC/autosave | Exclude temporary composite parties and avoid concurrent party mutations; restore original parties before these systems resume. | `mp_trade.c`, `mp_sharedpc.c`, `mp_autosave.c` |
   | Contests, minigames, record mixing, photos | Preserve four-player functionality and existing group IDs; these require regression checks rather than a triples redesign. | `mp_grouplink.c`, `mp_linkx.c`, `mp_minigame.c`, `mp_photo.c` |
   | Battle recordings and Frontier | Keep legacy formats and recorded save sectors compatible. Gate unsupported triple entry while preserving ordinary facilities. | `recorded_battle.c`, battle setup/controllers |

   For daily pair goals, a win by A/B/C updates A–B, A–C, and B–C once each, but updates the aggregate co-op battle count once. Personal participation counters update once on each participating client. The authority should publish one roster-aware event instead of looping the old partner callback. A room member who watched or stood nearby receives no co-op win credit. Preserve the existing five-minute-active friend qualification and other relevant reward eligibility/caps.

   Preserve TOGETHER's existing behavior during battles: it freezes rather than draining. Use the committed party for battle-specific participation and any agreed bonus snapshot, while preserving the existing room-wide catch-up policy. Do not recalculate a co-op bonus from stale overworld coordinates halfway through a turn or stack it again for each ally.

   Use transparent, configurable triple encounter parameters. Check the selected encounter's original authored party count before all co-op expansion: only an original count of at least three permits a triple. Then start from the existing +2 route/+3 boss level policy and filler constraints, with eligible triple trainers fielding three usable foes and bosses having up to six total. Existing route padding already adds roughly half the original party; do not apply it a second time or use its output to override the eligibility gate. An original two-Pokémon trainer remains a doubles encounter even if that existing padding produces three foes. Keep one-Pokémon trainers one-on-one and preserve scripted encounter exclusions. Then measure whether different triple-specific values are needed.

   For ordinary three-player land encounters, start with three independently rolled legal wild Pokémon when the encounter table supports them. Preserve the existing two-player 60% double-wild setting. Retain one- and two-foe layouts for deliberate encounter rules or failed extra-mon rolls, and expose any future three-player size weights as tuning data. Generate extras only from supported ordinary land tables; unique/scripted opponents are never cloned for numerical symmetry. Roll and boost the encounter once on the authority, then synchronize the resulting individuals.

   Improve enemy teams through compatible native support and coverage rather than adding large hidden stat multipliers. Existing tools include Helping Hand, Follow Me, screens, weather, status, Protect, and friendly-fire-aware teams. Do not assume Tailwind, Wide Guard, or other later-generation mechanics exist in this source. NPC trainer item inventory is shared by the trainer, not duplicated per active Pokémon; inspect the current two-entry AI item arrays before indexing with the third enemy.

   Enemy AI can coordinate attacks and use public battle information, but it must not read the new ally-intent network messages or uncommitted human choices as privileged knowledge. Evaluate triple-friendly challenge through action economy, survival opportunities, battle length, support usefulness, and repetitive spread-move/focus-fire patterns.

   **Exit criterion:** triples contribute correctly to the adventure and existing social/economic systems, while nonparticipants and unrelated multiplayer activities remain unaffected.

10. **Add meaningful automated coverage and finish release integration.**

   Extend the existing headless harness, not just a new isolated test binary. The debug command file is `src/mp_debug.c` at repository root under `src/`; some existing documentation names an older location. Harness timing and input live in `src/platform/sdl2.c`. Read their current implementations.

   Add a reusable proposed `tools/test/runN.sh` or equivalent runner for one through four clients, keeping existing `run1.sh`/`run2.sh` entry points compatible. Extend `regress.sh` to support third/fourth scripts, isolated saves/configuration and logs, independent per-client party/warp inputs, and expected assertions for every client. Clear per-scenario variables so expectations cannot leak between scenarios.

   Add narrowly scoped debug support for constructing deterministic triple encounters, choosing moves/targets, dumping owner mappings and canonical state, checking bag counts and pending settlements, and stopping a selected client. New commands/conditions must be documented as new, not assumed already present. Confirm the existing action-menu condition works for each local owner, not only the original lead battler.

   Cover at least this matrix:

   | Scenario | Required evidence |
   | --- | --- |
   | Existing solo and co-op doubles | Baseline scenarios still pass; doubles still use four live battlers and existing party limits. |
   | Three-player trainer victory | Three local menus, correct owner commands, three NPC actors, all clients return to field. |
   | Trainer eligibility before scaling | With three friends available, original one-/two-/three-Pokémon trainer parties permit at most one/two/three human participants respectively. Padding an original two-Pokémon party to three or six still cannot start triples. Check the actual selected rematch variant and every trainer entry path. |
   | Three-player wild battles | One-, two-, and three-wild variants; absent opponent slots never act or receive targets. |
   | Owner identity | Nonconsecutive room IDs; initiating guest rather than room host; nontrivial original party selection/order. |
   | Six-battler masks | Actors 4 and 5 faint, recover, switch, receive Leech Seed/Attract, and contribute to Mirror Move/history without overlap. |
   | Multi-target rules | Foes-only Surf/Rock Slide; Earthquake across allies and enemies; Protect, immunities, and a single remaining target; PP spent once. |
   | Support and screens | Helping Hand can choose either ally; duplicate boosts behave as documented; Follow Me/redirection and three-actor screens resolve consistently. |
   | Replacement conflicts | Three KOs in one turn, NPC shared reserves, exhausted human pool, teammate Revive, no duplicate active Pokémon. |
   | Running triple after a KO | A battle that validly started as a triple stays in that format after the enemy is reduced to two or one living Pokémon. |
   | Healing | Each owner uses their own item on themselves, either ally, and a benched Pokémon; switched targets and repeated messages do not misapply/duplicate consumption. |
   | Captures | Third player catches; three distinct catches; two throws at one target; last-foe removal; catch followed by escape/KO; full party/PC handling. |
   | Ball styles | Special, colored, and decorated balls survive send-out/capture/result round trips with correct owner-specific effects. |
   | Progression | Unequal badges/bonus context, EXP Share, traded status, level-up, four-known-move learning, and evolution map to each owner's original slot. |
   | Pair/daily rewards | Exactly three pair increments and one aggregate battle increment; one personal win each; no spectator credit; caps remain effective. |
   | Matchmaking races | Third refuses, third times out, competing wild invitations, host cancels, late join, simultaneous acceptance, reservation cleanup. |
   | Network delay | Three clients through 250 ms delay + 80 ms jitter at real-time FPS; readiness and queues remain correct. |
   | Settlement/disconnect | Guest/authority leave before start, during selection, during animation, and during result delivery; duplicate/stale messages; no duplicate rewards or lost ownership. |
   | Presentation | Three different custom trainers, long permitted names, six statuses, emotes, target previews, weather/Substitute/Transform, bag/party restoration, controller icon families. |
   | Fourth room member | Free overworld or spectator behavior, no extra battle acknowledgement, bet outcome/refund bound to the right battle. |
   | Legacy save and replay | Load a pre-change save and recorded battle; save/reload after triples; persistent layout and checksum compatibility retained. |
   | Existing other modes | PvP rewards, tournament friendly semantics, link contests/group transport, minigames and record mixing still pass focused smoke checks. |

   Prefer behavior assertions: actor ownership, legal target masks, exact HP/PP/item deltas, restored Pokémon identities, reward counters, and matching canonical state at defined boundaries. A screenshot of six Pokémon and a log saying 'battle started' do not establish the implementation is complete. Capture screenshots for layout verification at native resolution and compare relevant state across all participants.

   Add selected sanitizer/debug builds for the mask/array work where the build supports them. Run the Linux build and complete regression suite after the relevant changes. Cross-compile Windows through the documented target if dependencies are available; distinguish compilation from execution on a real Windows machine.

   Update `CLAUDE.md`, `BUILDING_COOP.md`, `dist_extras/README.txt`, test documentation, and any visible co-op text that currently says 'both players', 'your partner', or 'Double Battle' for sessions that can now be triples. Document the original-trainer-party eligibility gate before scaling, the two-Pokémon triple selection per player, unrestricted targeting, Emerald Surf behavior, spread factor, item/capture conflict rules, and disconnect policy. Keep accurate text for existing doubles.

   **Final acceptance:** three actual clients can find one another, select their own Pokémon, play a complete triple trainer/wild encounter with useful support, switch/heal/catch correctly, and return with the right personal state and rewards. Existing modes remain compatible. Provide the changed-file map, documented rules, test results, visual checks, and any concrete remaining limitations. Do not describe unrun checks as passing.

**Suggested review sequence**

Use focused checkpoints for: (1) legacy-capacity separation and model, (2) six-actor engine/rules, (3) three-client transport, (4) matchmaking and party selection, (5) medicine/captures, (6) HUD/intents/custom appearances, (7) owner-specific settlement/progression/social counters, and (8) encounter tuning, regressions, and documentation. Tests should be added alongside the behavior they verify; the final checkpoint runs the complete matrix. These checkpoints are review units, not separate requests for permission to continue.

**Source map for this plan**

All links below refer to the inspected commit so later repository changes do not silently alter the evidence.

- [Architecture and build notes](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/BUILDING_COOP.md)
- [Repository implementation notes](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/CLAUDE.md)
- [Battler constants, partner macros, flags, and packed statuses](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/include/constants/battle.h)
- [Battle structures and Palace assertion](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/include/battle.h)
- [Controller execution-mask helpers](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/include/battle_controllers.h)
- [Battle initialization and controller transport](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/battle_controllers.c)
- [Existing co-op lifecycle, party packing, captures, and results](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/mp/mp_coop.c)
- [Pair-only battle NetLink adapter and PvP flow](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/mp/mp_battle.c)
- [Existing group transport for contests](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/mp/mp_grouplink.c)
- [Legacy recorded-battle save structure](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/recorded_battle.c)
- [Damage, screens, and exact-two spread checks](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/pokemon.c)
- [Move target metadata](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/data/battle_moves.h)
- [Helping Hand, EXP, medicine, and ball scripts](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/battle_script_commands.c)
- [Custom trainer runtime sprite buffers](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/mp/mp_avatar.c)
- [Daily goals and pair/aggregate co-op counters](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/mp/mp_events.c)
- [Existing TOGETHER and catch-up policies](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/mp/mp_together.c)
- [Shared Pokédex and catcher credits](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/mp/mp_dex.c)
- [Headless test suite documentation](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/tools/test/README.md)
- [Current regression runner](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/tools/test/regress.sh)
- [Debug commands and test conditions](https://github.com/christinebaker1212-hash/PokemonEmeraldKai/blob/ea41d48eae00d170bf339f34aa66683938e3950c/src/mp_debug.c)

