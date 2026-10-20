# Triple Battles: decisions

The plan is in `TRIPLES_PLAN.md`. Where it left room for interpretation, these are the
answers agreed with the project owner (October 2026). They override the plan where they differ.

## Rules
- **Party:** 2 Pokémon per player (1 active + 1 reserve), packed 2 + 2 + 2 into the six-slot battle party.
  Co-op doubles keep 3 + 3.
- **Choosing the 2:** a remembered "triple pair" per player, set in the party menu (default: the first
  two usable Pokémon). Used automatically for wild and trainer triples; no prompt before battle.
- **EXP:** each owner gets a full share: a fainted foe's EXP is split only among that owner's own
  participating Pokémon, so joining friends never costs EXP.
- **Spread moves:** 3/4 damage in triples when 2+ targets are hit (snapshot at execution).
  Singles/doubles keep Emerald's rules. Surf stays foes-only.
- **Helping Hand:** either ally can be chosen; one boost per target per turn (a second one fails).
- **Targeting:** any living target, no distance rules, no Shift.

## Encounters
- **Wild triples:** usually 3 wild Pokémon when the area's table allows; fall back to 2 or 1 if the
  extra rolls fail.
- **RUN:** any player can choose RUN; the team runs only if all three choose RUN that turn. The roll uses
  the fastest player Pokémon.
- **Trainer triples:** the waiting player chooses when to start: after one friend joins they can start
  doubles or keep waiting; the battle starts automatically when a third player arrives. Eligibility is
  the original authored party size (3+), before any co-op padding.
- **Difficulty:** same as co-op doubles first (+2 levels, +3 bosses, existing padding, bosses up to 6),
  then measure and tune.
- **Excluded:** two-trainer/scripted doubles (Twins etc.) stay as they are; the 4th room member can
  spectate or keep exploring.

## Online
- **Disconnect mid-battle:** abort for everyone; catches and items already used stay as they are, HP/PP
  as they were, no victory rewards or badges.
- **Ally intents:** extend "X is selecting a move..." to show each ally's chosen move and target once
  picked; no editable intents or ready barrier in the first release.

## Presentation
- Six sprites at full size, overlapping like Black/White triples (mock-up "A"): no scaling.
  Foes at about (132,38) (178,30) (218,42), players at (30,82) (76,88) (120,84); the center one is drawn first.
- Compact 72x18 cards in the healthbox colors. Row 1: the full name (10 letters fit). Row 2: the level
  (always shown), the status badge when there is one, then the HP bar. Foes stacked top-left from (2,2),
  the team stacked right from (166,55), 19 px apart; a colored strip on the left of a player card shows its
  owner. Exact HP lives in the R team glance and the target screen.

## Process
- Work through the phases independently; commit and send a Windows build at each major milestone
  (engine, three-client battles, UI, settlement); ask only when a real choice comes up.
