# MOéMON GARDEN: design specification

Working name: Pokémon Garden. In the game: **MOéMON GARDEN** (Kai says MOéMON everywhere).

An A-Life simulation for Moemon Emerald Kai Co-op, inspired by Sonic Adventure 2's Chao Garden. Party Pokémon become
autonomous, emotionally expressive individuals whose personalities, relationships, memories and behavior grow
throughout the adventure, for one to four players online.

This document is the outcome of a staged design process with the project owner. Every decision below was chosen
and confirmed by them; the decision log at the end lists them in order. It describes gameplay and player
experience only. Implementation, data layout and testing come later and must follow it.

---

## 1. Ground rules

These hold everywhere and override anything that seems to contradict them.

**The four principles**

- **Needs create behavior.**
- **Experiences create emotions.**
- **Relationships create attachment.**
- **Player neglect creates nothing, because ordinary absence isn't neglect.**

**What follows from them**

- Negative emotions are consequences of experiences, never of player inactivity. Not playing, a Pokémon resting in
  the PC, a long break: none of these make a Pokémon sad, resentful or worse off.
- Low Friendship is a legitimate state (independent, proud, cautious, new), never a narrative accusation of
  mistreatment.
- Needs exist (hunger, rest, play...) and Pokémon meet them **themselves**. A need is an opportunity for
  autonomous behavior, never a demand on the Trainer.
- **Background vs urgent needs.** While a Pokémon can't act (in its ball, in a long battle, deep in a berry-less
  cave) a need may keep building, but only as a preference: after a long adventure it is more likely to choose a nap
  once it reaches the Garden. A need never escalates into suffering because the player didn't provide an
  opportunity.
- **Kindness is not an optimization.** Pokémon read the context of an interaction. A berry while it is actually
  foraging means more than a berry right after a meal. Repetition is never punished; it simply counts for less than
  a moment that matters. This applies to petting, praising, playing and every other interaction.
- **Presentation, never balance.** The A-Life never changes battle outcomes.
- **No settings.** Everyone gets the A-Life as designed; it costs the player nothing.

**Preserved from Emerald**

- Friendship keeps all of its Emerald mechanics (Return, Frustration, Friendship evolutions, every existing gain and
  loss).
- Natures, IVs, EVs and battle mechanics are untouched.
- The A-Life never blocks player movement or slows story progression.

---

## 2. Who a Pokémon is

### 2.1 Identity: three layers that grow

| Layer | What it is | Changes? |
|---|---|---|
| Species instincts | What its species does (Zigzagoon zigzags and digs up items, Wingull circles the sky) | Switches on evolution |
| Temperament | Rolled per individual: bold or cautious, warm or aloof, easily bored, playful, proud... Loosely nudged by Nature, never decided by it | Drifts slowly within a range around its innate self |
| Growth | Learned traits from experience: afraid of thunder, loves bridges, protective of the youngest | Gained and softened through experience |

Two Pokémon of the same species differ from day one and grow further apart. Nature stays the battle stat.

### 2.2 How players understand a Pokémon

- **Body language, moment to moment:** animations, faces, gestures, emotes, where it chooses to be. Moemon are
  anime-style characters, so faces and posture can carry far more than ordinary creature sprites.
- **A journal on its summary,** written as the Trainer's observations in plain sentences, never numbers. It fills in
  as the Trainer gets to know the Pokémon: what it likes, what changed it, who it's close to, things witnessed
  (`??? (not known yet)` for what hasn't been learned). Watching a Pokémon fills in journal lines.

> *LOTAD's journal*
> - Always naps by water, never on grass.
> - Got very brave after the WATTSON battle. Hasn't flinched at thunder since.
> - Follows KAZUYA's SKITTY around the GARDEN.

### 2.3 Where the simulation runs

- **The follower and Garden Pokémon:** simulated moment to moment.
- **The rest of the party:** a lighter background simulation. They still live through battles, places and story
  events, and their background needs build into preferences.
- **The PC (personal and shared):** a calm rest. Nothing builds up there and nothing is learned there.

---

## 3. Needs and self-care

### 3.1 The needs

- **Core five, for everyone:** hunger, rest, play, curiosity, and a **social battery**. Some Pokémon recharge with
  company, others by being alone; solitude is a valid need, not loneliness.
- **Type needs:** one or two per type, for example:

| Type | Need |
|---|---|
| Water | soak in water |
| Fire | bask in warmth |
| Grass | sunlight on its leaves |
| Ghost | shade, dusk |
| Ground | dig |
| Flying | open sky |
| Electric | perks up in storms |
| ... | (the rest are defined later, one or two per type) |

- **Dual types** have both types' needs; the individual's temperament decides which one leads. One Marshtomp mostly
  wants to soak, another mostly wants to dig.

### 3.2 Meeting needs

- **From the world itself.** Followers use what the world already has: berry trees, tall grass, water, shade, sunny
  spots, quiet corners, and items they find (like PICKUP). This reads the land's features, not map names, so it works
  in any region.
- **In the Garden,** every need has a place (see section 10).
- **Trainer help is a gift, not a chore.** Bringing a berry to a Pokémon that is actually looking for food saves it a
  search. It notices, and that can become a moment (section 5.4).

### 3.3 Showing needs

Small cues, shaped by temperament: a yawn, a rumbling belly, restlessness. A proud Pokémon hides that it's tired, a
dramatic one flops over, a shy one drifts closer to its Trainer. When it can't act on a need the cue stays small, so
it never looks like distress.

### 3.4 Tastes

Tastes start from Emerald's Nature flavor likes and dislikes, then become its own: a comfort food after a hard day, a
favorite nap spot, a toy or keepsake it's attached to.

---

## 4. Emotions and memory

### 4.1 Emotions

Eight short-lived emotions that blend, over a slower overall mood:

**joy · fear · frustration · sadness · surprise · affection · pride · calm**

Blends express nuance: proud + nervous before a Gym, joy + affection at a reunion. The slow mood is a calm
stretch or an unsettled stretch.

### 4.2 Memory

- **Moments:** experiences that are strong, new or part of the story become memories: what happened, where, with
  whom, how it felt.
- **Familiarity:** repeated small experiences don't each become a memory. They build familiarity and habits
  (knowing Fortree, a favorite nap spot, a daily routine).

### 4.3 Healing

Fears and grudges soften mostly when new experiences contradict them (riding out a storm beside a friend), and
slowly with calm time. Pokémon can overcome things, and the journal tells the story. The player has no duty; life
brings those experiences naturally.

### 4.4 Growth

Experiences add learned traits; the core temperament drifts slowly within a range. A bold Pokémon after many frights
grows cautious but keeps a spark of its old self. Big transformations take a long time.

---

## 5. Relationships and Friendship

### 5.1 What a bond is

Every pair (Pokémon-Trainer, Pokémon-Pokémon) has five hidden parts:

| Part | Meaning |
|---|---|
| Familiarity | how well they know each other |
| Trust | safe together |
| Affection | fondness |
| Respect | admiration, competitive regard |
| Friction | they rub each other the wrong way |

Friction is separate from affection, so bonds can be complicated: best friends (trust + affection), fond rivals
(respect + friction, warm), squabbling siblings (affection + friction), admirers from afar (respect, little
familiarity), the wary (low trust). Players see behavior and journal lines, never the values.

### 5.2 Who has bonds

- **Full bonds:** with all four Trainers and all fellow Pokémon in the group (up to 24 party Pokémon).
- **Attitudes:** feelings about everyone else (May / Brendan, Gym Leaders, a species that once frightened it) are
  memories and learned traits, not full bonds.

### 5.3 First meetings and growth

- **The opening stance** comes from temperament (shy: wary, bold: curious, warm: friendly), colored by the first
  meeting's circumstances (met right after a shared victory: a warm start; first seen across a battle: respect and
  a little friction), with a light touch of Pokémon lore (Zangoose and Seviper start with friction, prey and
  predator species start wary). All of these are starting points that experience can overturn.
- **Bonds grow from shared life:** battles side by side, play, story events, comfort. **Absence never erodes a
  bond.**
- **Reunions.** After a long separation, meeting again is a reunion moment (joy, affection). Time apart counts only
  when the Trainer's adventure moved on meaningfully meanwhile (a badge, a new area, a few hours of play; in the
  postgame, hours of play), so depositing and withdrawing can't farm reunions.

### 5.4 Friendship and the A-Life

Friendship stays Emerald's, mechanically authoritative. The A-Life bond is separate and governs social behavior. They
intertwine **through moments**:

- **A Friendship moment is a good memory that involves the owner Trainer.** Comforting a Pokémon frightened after a
  storm, bringing a berry while it was foraging, sharing its first badge, joining a game it invited you to, a
  reunion. Each such memory raises Friendship once, in an Emerald-sized step, and can appear in the journal. Because
  only memories count, ordinary repeated actions never raise Friendship: the anti-grind rule is part of the design.
- **Friendship → A-Life:** Friendship sets the head start of the owner bond (most useful for Pokémon raised before
  the Garden existed) and colors how unclear moments are read (a high-Friendship Pokémon gives its Trainer the
  benefit of the doubt).
- **Losses:** only Emerald's existing causes lower Friendship (fainting, bitter herbal medicine...). The A-Life adds
  no new losses, never through neglect.
- **Other players' moments** never change your Pokémon's Friendship.
- **Frustration builds** get no special protection (owner's decision): Garden moments always count, and Emerald's
  bitter herbs remain the way to lower Friendship.

---

## 6. Direct interaction

### 6.1 Starting an interaction

- **Press A** next to a Pokémon: the Trainer does the most fitting thing (comforts a frightened one, greets a calm
  one, joins the game it's inviting you to).
- **Hold A** for a verb wheel, built like the game's existing emote wheel.

### 6.2 Verbs

| Group | Verbs |
|---|---|
| Core | talk, pet, feed (berry, Pokéblock), play (toss a ball, chase, hide and seek) |
| Emotional | praise, comfort |
| Quiet | watch, sit with (nothing asked of the Pokémon; fills journal lines; good for shy Pokémon) |
| Care | groom, give a keepsake (can become a cherished object) |

### 6.3 How an interaction lands

- **What it depends on:** whether it fits the moment (need, emotion), the Pokémon's mood, and the bond with that
  person.
- **Repeats** become ordinary, never negative.
- **Boundaries.** Each Pokémon has boundaries from its temperament: a proud one dislikes being petted in front of
  others, a shy one dislikes being picked up. It shows the affection it does like instead (sitting close, leaning
  against a leg). Boundaries are personality, never failure.
- **Crossing a boundary:** the Pokémon sidesteps, and the journal notes the preference. Insisting several times in a
  row brings brief frustration that fades within minutes. The bond is never marked.

### 6.4 Other players

Any player can interact with any Pokémon. It responds through its bond with that person. Food from a friend comes from
the friend's own bag. Others never change Friendship.

---

## 7. Followers

- **One follower per player.** The player chooses, as now.
- **Asking to come out.** A party Pokémon may ask to come out: a cue on the party screen **and in the hold-SELECT
  follower menu**, especially a curious one near a place it likes. Saying no leaves no mark.
- **How free it is.** When the Trainer stands still or strolls, the follower drifts a few tiles to investigate,
  forage or nap. When the Trainer moves with purpose, it hurries back, and reappears beside them if far behind or out
  of sight. It never takes a tile the player needs, never blocks a path, and stays out of scripts.
- **Rare gentle hints.** A curious or PICKUP-like follower sometimes sniffs at a hidden item's spot or perks up near
  something rare. Never puzzle solutions, never warnings about trainers.
- **When followers meet.** When Trainers pause together, their followers greet, chat, play tag or squabble according
  to their bonds, and leave with their own Trainer (maybe looking back).
- **Story scenes:** see 8.4.

---

## 8. Story and world

### 8.1 Took part, witnessed, heard later

| Tier | Who | Result |
|---|---|---|
| Took part | battled, or was at the center of it | strong memory, strong emotion, maybe a learned trait |
| Witnessed | in the party nearby | a milder memory |
| Heard later | told by teammates in the Garden or party; Pokémon in the PC hear once they're back, never while boxed | proud by association, no memory of being there |

**Telling is in character.** A boastful Pokémon makes the battle sound bigger; a nervous one makes Groudon sound
scarier. The listener's reaction depends on its bond with the teller (it believes a trusted friend; a rival's boasting
irritates it). This adds emotional color only; the journal never records false facts.

### 8.2 Events

- **Tagged story beats:** Gyms, rival battles, Team Magma / Aqua, legendaries, Wally, the Elite Four, the Sootopolis
  crisis.
- **General events that work in any region:** first visit to a town, first snow, nearly fainting, first evolution, a
  near catch.

A new region adds its own tags; the general events already work there.

### 8.3 Places, in any region

- **Place traits:** every area is described by traits, so preferences carry across regions (a Pokémon that loves
  quiet forests loves them anywhere).
  - Terrain: grass, water, cave, city, mountain, desert, snow, sea.
  - Feel: quiet, busy, dark, eerie, high up.
  - Weather.
- **Personal places:** where it was caught, where it evolved, a favorite tree, its home region. Memories keep their
  place and region when traveling.

### 8.4 Story scenes

The follower stays where it is, out of the way, and reacts through its face and emotes (awe at Rayquaza, fear at
Groudon, cheering after a Gym). If the scene moves the Trainer, the follower reappears beside them afterward.

---

## 9. Battles

- **Presentation only.** No change to stats, accuracy, critical hits or turn order, in any battle. Return and
  Frustration work as always.
- **What battles teach:**
  - Standout moments become memories (the clutch knockout, hanging on at low HP, fainting, a comeback).
  - Partners in Double and Triple Battles grow trust and respect.
  - Strong foes earn respect; recurring foes (May's team, a friend's PvP Pokémon) can become rivals.
  - Routine wins only add a little confidence, so grinding battles doesn't flood memories.
- **Pokémon battle emotes** sit on the Pokémon's own sprite, with their own rounded bubble shape, color and quiet
  sound. They must never be mistaken for Trainer battle emotes, even in a busy Triple Battle.
- **Battle lines are exceptionally rare:**
  - **When:** only when the battle creates one of the strongest memories (a best friend saved at 1 HP, the final
    battle with a long-time rival, the first Gym won together), and never twice for the same player within a long
    stretch of play. A player might see a handful across the whole story.
  - **How:** each has its own look and a short chime; it adds no wait, is never held, and follows the battle speed
    and turbo settings.
- **PvP (BATTLE FLAG):** the same treatment. Facing a friend's Pokémon builds respect and, over many battles, friendly
  rivalries with history. Friction from PvP only turns into dislike for competitive temperaments.

---

## 10. Multiplayer society

- **Each save keeps its own side.** Your Pokémon keep their feelings and memories about friends and friends' Pokémon
  in your save. Feelings can be one-sided (your Skitty adores their Mightyena, who only tolerates her). This works
  with friends offline and needs no server. Pokémon are recognized by their own identity, so a traded or renamed
  Pokémon is still known.
- **Solo is complete.** Your own six form a full society. Playing together adds Trainers and Pokémon, never something
  solo play lacks.
- **Trades and the shared PC:** everything travels with the Pokémon (personality, memories, bonds, including with its
  first Trainer). It greets its old Trainer warmly ever after. Friendship follows Emerald's own trade rules. The shared
  PC is a calm rest like the personal one.
- **Favorites.** A Pokémon may love another Trainer most. This is shown warmly (it runs to greet them), never as
  betrayal. The owner bond stays special: Friendship, reunions, following, and only the owner's moments raise
  Friendship.
- **A new LOOK.** Pokémon always know who a Trainer is. A big change of LOOK gets a brief double take (surprise, a
  sniff, happy recognition); shy ones hesitate a moment longer, and the double take shortens for frequent changers.

---

## 11. The MOéMON GARDEN

### 11.1 Whose Garden

- **One Garden per player,** in their own save.
- **Alone,** it is a full small Garden: every need has a place, it grows with the journey, the player's decorations
  work, and their six live full social lives there.
- **With friends,** the **host's** Garden opens up to full size for up to 24 Pokémon (more meadow, more zones).
  - Guests' decorations stay theirs: they appear in the host's Garden while the guest is there, and return to the
    same spots on later visits.
  - The group's travels leave growth marks in the host's Garden.

### 11.2 Getting there

The Garden is every POKéMON CENTER's second floor (since 1.16; it was a meadow west of LITTLEROOT before): the
CENTER's escalator rides up into it and its own escalator rides back down to that CENTER. After the first visit,
START > GARDEN and the PC's row go there from outdoors or a CENTER, and the way down leads to the last CENTER
visited (COOP_NOTES 1.16.0).

### 11.3 The place

- **It grows with the journey.** It starts as a meadow with a pond, a few berry trees and shade. Places the group has
  been leave their mark: a cutting from FORTREE's trees grows into a perch grove, warm sand from the desert becomes a
  dig patch, a stone from a storm becomes a spot where Electric types gather. Every type need gets a home in time.
- **Decorations.** Players place earned SECRET BASE decorations as they please. They come from the placer's
  collection and can be moved back at any time; a decoration in the Garden isn't in the Secret Base at the same time.
- **Pokémon use decorations their own way:** cushions and mats for naps, dolls and balls for play, plants as shade, a
  tire to climb. A Pokémon may grow a favorite. Decorations have no stat or Friendship effects: they are chances for
  behavior, never a layout to optimize.

### 11.4 Watching

- **Walking:** you walk among the Pokémon as your Trainer, and can interact as in section 6.
- **WATCH mode:** sit down to WATCH, and the view drifts to whatever interesting thing is happening (a game starting,
  a squabble, a pile of nappers, a shy one being coaxed out). There's no text unless you ask, and what you see fills
  in journals.

### 11.5 What happens there

The Garden runs on everything above:
- Needs drive foraging, napping, soaking, digging and basking.
- Social batteries draw some Pokémon together and send others off to quiet corners.
- Bonds form groups on their own: friends play, rivals compete, siblings squabble.
- News spreads in character.
- Pursuits progress (section 12).

**Spontaneous play between Pokémon** (chasing, tag, hide and seek invented on the spot) is everyday behavior. There
are no structured group games or competitions.

---

## 12. Long-term life

- **Rare combinations.** Rare behaviors appear only when temperament, bonds, place, weather and time of day line up:
  two best friends stargazing on a clear night, a Water type teaching a nervous Fire type to paddle, old rivals
  finally sharing a nap. There's nothing to grind for and no album to fill. The journal keeps the ones you witnessed.
- **Own pursuits.** A Pokémon may take up a pursuit that fits its interests (climbing the perch grove, swimming laps,
  singing, collecting pretty stones, teaching younger ones) and visibly get better at it, celebrated in the journal
  ("reached the top branch!"). There are no stats and no prizes.
- **Eggs.**
  - A hatchling gets slight temperament leanings from its parents (never a copy), and sometimes a small family trait
    (a song, a favorite food, a way of greeting).
  - It knows its parents; if they're in the group, they share a family bond.
- **Evolution.**
  - Evolution is a strong memory.
  - The species-instinct layer switches to the new species; temperament, memories and bonds stay.
  - The Pokémon takes a short while to get used to its new body (a stumble, a surprised look at its reflection).
  - Friends react: proud, awed, a rival a little jealous.

Long-term engagement comes from attachment to individuals and their social lives, never from maintenance.

---

## 13. The Chao reference: documented vs new

| Area | Chao Garden (documented) | MOéMON GARDEN |
|---|---|---|
| Emotions | Hidden values: joy, anger, sadness ("urge to cry"), fear, surprise, dizziness, relaxation | Eight blending emotions incl. affection and pride, over a slow mood (new set) |
| Needs | Hunger, sleepiness, tiredness, boredom, loneliness (and an urge to breed) | Core five incl. a two-way social battery + type needs; self-met; never punishing (new) |
| Bonds | One "like" value per playable character | Five-part bonds with every Trainer and fellow Pokémon (new) |
| Neglect | Happiness falls when ignored; unhappy Chao can die instead of reincarnating | **Excluded.** Absence creates nothing |
| Life cycle | Cocoons, reincarnation, death | **Excluded.** Evolution is "same self, new body" |
| Skills | Learned from toys and lessons; races and karate | Own pursuits without stats; no structured competitions |
| Story | Gardens sit outside the story | Story tiers, word spreading, places in any region (new) |
| Memory | Mostly who they like | Episodic memories + familiarity + journal (new) |

---

## 14. Constraints and risks (design-level)

- **Art volume:** the biggest cost of the design.
  - Faces and gestures for eight emotions.
  - Need cues shaped by temperament.
  - Type-need behaviors for about 18 types.
  - Pursuit and decoration animations.
  - Pokémon battle emotes.
  - All of it across many Moemon.

  Moemon's human-like expressiveness is the main asset; shared poses across Moemon will matter.
- **Writing volume:** journal lines, rare battle lines, and in-character telling.
- **Readability at 240×160:** 24 Pokémon in the Garden, four followers in the field, and emotes in Triple Battles
  must all stay readable.
- **Online traffic:** four players' followers and a 24-Pokémon Garden have to stay smooth at internet lag.
- **Save size:** memories and bonds for every Pokémon (and the friends they know) must stay modest.
- **Hardware:** this build already exceeds the GBA's sprite and palette limits. The design is bounded by art,
  readability and network rather than by the GBA.

## 15. Open details for later design passes

These are refinements inside decided systems, not new decisions:
- The full type-need list (one or two per type) and which Garden zones host each.
- The place-trait list for every Hoenn map, and the trait vocabulary for future regions.
- The catalog of rare combinations, pursuits and decoration uses.
- The tagged story beat list and each beat's tier rules.
- Journal tone and line templates; the in-character telling styles.
- What exactly makes a memory strong enough to count as a Friendship moment, as tuning within the rule.

---

## 16. Decision log

| Stage | Decision |
|---|---|
| Baseline | Friendship keeps Emerald's mechanics; A-Life relationships separate and multidimensional; social behavior from personality and bonds, not the Friendship stat; no guilt or neglect penalties; Frustration stays functional |
| Stipulations | Needs create behavior / experiences create emotions / relationships create attachment / neglect creates nothing; background needs build into preferences, never suffering; context over repetition |
| 1 Foundations | Layers that grow; body language + journal; everyone at different detail (PC calm); Friendship intertwines through moments; no Frustration protection |
| 2 Needs | Core five + type needs (dual types: temperament weights); met from the world; cues shaped by temperament; Nature tastes that become its own |
| 3 Emotions & memory | Eight blending emotions + slow mood; moments + familiarity; healing through new experiences; learned traits + slow drift; Friendship moment = good memory involving the owner |
| 4 Relationships | Five parts with friction; full bonds within the group, attitudes for others; temperament + first impression + a touch of lore; shared life, never eroded; reunions need story progress apart |
| 5 Interaction | Smart A + verb wheel; all four verb groups; fit + personal boundaries; anyone can interact through their own bond; crossing a boundary: sidestep, brief frustration if pushed, bond never marked |
| 6 Followers | Free nearby, hurries back; rare gentle hints; followers mingle when Trainers pause; player chooses, Pokémon may ask (party screen + hold-SELECT menu) |
| 7 Story & world | Three tiers + word spreads (in character); story tags + general events; place traits + personal places; follower reacts in place during scenes |
| 8 Battles | Presentation only; moments, partners, foes; distinct Pokémon emotes; exceptionally rare lines (top memories + a long gap); PvP rivalries grow |
| 9 Multiplayer | Each side keeps its own; solo is complete; everything travels in trades and the shared PC; a Pokémon may love another Trainer most, warmly; a double take at a new LOOK |
| 10 Garden | One Garden per player: full small Garden alone, the host's opens to full size with friends; walk there then a shortcut; grows with the journey + Secret Base decorations (placer's own, Pokémon use them); walk + WATCH mode |
| 11 Long-term | Rare combinations; own pursuits only (spontaneous play stays); egg leanings + family; same self, new body |
| Cross-cutting | No settings (A-Life always on); named MOéMON GARDEN |
