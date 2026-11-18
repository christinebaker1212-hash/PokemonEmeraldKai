# The regional sets: what plays (tools/tcg/regions/scope.py)

Generated; rerun after translating. "Plays now": every attack, power and rule on the card matches what the
engine already does (numbers to carry over), or it has none. See docs/TCG_REGIONS.md.

| Region | Set | Cards | Play now | Waiting | Attacks matched | Attacks to translate | Powers / Bodies | Trainers / Energy |
|---|---|---|---|---|---|---|---|---|
| KANTO | gym1 | 132 | 36 | 96 | 57 | 65 | 11 | 34 |
| KANTO | gym2 | 132 | 26 | 106 | 57 | 80 | 13 | 29 |
| JOHTO | neo1 | 111 | 40 | 71 | 72 | 48 | 11 | 21 |
| JOHTO | neo2 | 75 | 18 | 57 | 37 | 43 | 18 | 4 |
| JOHTO | neo3 | 66 | 10 | 56 | 27 | 53 | 19 | 5 |
| JOHTO | neo4 | 113 | 25 | 88 | 36 | 83 | 24 | 15 |
| E-CARD | ecard1 | 165 | 70 | 95 | 88 | 75 | 37 | 17 |
| E-CARD | ecard2 | 182 | 24 | 158 | 71 | 138 | 57 | 30 |
| E-CARD | ecard3 | 182 | 25 | 157 | 80 | 158 | 62 | 26 |
| PROMO | basep | 53 | 21 | 32 | 41 | 25 | 9 | 2 |
| PROMO | np | 40 | 10 | 30 | 18 | 30 | 10 | 4 |
| | **all** | 1251 | 305 | 946 | 584 | 798 | 271 | 187 |

899 different wordings to translate. The most common (count, kind, an example):

- 14 x attack: Lt. Surge's Fearow (gym1): The Defending Pokémon can't retreat during your opponent's next turn.
- 12 x attack: Typhlosion (ecard1): Flip a coin. If heads, the Defending Pokémon is now Burned.
- 9 x attack: Raichu (ecard1): If there are any Lightning Energy cards in your discard pile, flip a coin. If heads, attach 1 of them to Raichu.
- 8 x attack: Graveler (neo3): Don't apply Resistance.
- 7 x attack: Electrode (ecard2): This attack's damage isn't affected by Weakness, Resistance, Poké-Powers, Poké-Bodies, or any other effects on the Defending Pokémon.
- 7 x Poké-Body: Entei (ecard2): To attach a Fire Energy card from your hand to Entei, you must discard an Energy card attached to Entei. (Attach the Fire Energy, and then discard an Energy from Entei.)
- 7 x attack: Houndoom (ecard2): Flip a coin. If tails, discard a Fire Energy card attached to Houndoom.
- 6 x attack: Weezing (ecard1): Flip a coin. If tails, put 6 damage counters on Weezing.
- 6 x Poké-Body: Zapdos (ecard2): You can't attach Lightning Energy cards from your hand to Zapdos.
- 6 x TRAINER: Darkness Cube 01 (ecard2): Attach this card to 1 of your Darkness Pokémon in play. That Pokémon may use this card's attack instead of its own. At the end of your turn, discard Darkness Cube 01.
- 6 x Poké-Body: Flareon (ecard3): Whenever you attach a Fire Energy card from your hand to Flareon, remove all Special Conditions affecting Flareon.
- 6 x Poké-Body: Celebi (ecard3): Whenever you attach a Grass, Water, or Psychic basic Energy card from your hand to Celebi, Celebi's type (color) becomes the same as that type of Energy until the end of the turn.
- 5 x attack: Misty's Poliwag (gym1): Flip a coin. If heads, the Defending Pokémon is now Paralyzed. If tails, the Defending Pokémon is now Asleep.
- 5 x attack: Croconaw (neo1): Until the end of your next turn, if an attack damages the Defending Pokémon (after applying Weakness and Resistance), that attack does 20 more damage to the Defending Pokémon
- 5 x Poké-Body: Golem (ecard1): All damage done by attacks to Golem is reduced by 10 (after applying Weakness and Resistance).
- 5 x attack: Magikarp (ecard1): This attack does 10 damage times the number of damage counters on Magikarp.
- 5 x TRAINER: Multi Technical Machine 01 (ecard1): Attach this card to 1 of your Pokémon in play. That Pokémon may use this card's attack instead of its own. At the end of your turn, discard Multi Technical Machine 01.
- 5 x attack: Suicune (ecard2): Flip a coin. If heads, this attack does 30 damage plus 20 more damage. If tails, this attack does 30 damage and the Defending Pokémon is now Asleep.
- 5 x attack: Arcanine (ecard3): Your opponent switches the Defending Pokémon with 1 of his or her Benched Pokémon, if any.
- 4 x attack: Lt. Surge's Magnemite (gym1): If the Defending Pokémon has any Energy cards attached to it, flip a coin. If heads, choose 1 of those Energy cards and discard it.
- 4 x attack: Donphan (neo1): If your opponent has any Benched Pokémon, he or she chooses 1 of them and switches it with his or her Active Pokémon, then, if you have any Benched Pokémon, you switch 1 of them with your Active Pokémon. 
- 4 x attack: Raikou (neo3): If your opponent has any Benched Pokémon, flip a coin. If heads, choose 1 of them and this attack does 20 damage to it. (Don't apply Weakness and Resistance for Benched Pokémon.)
- 4 x attack: Clefable (ecard1): Flip 2 coins. This attack does 20 times the number of heads.
- 4 x attack: Mewtwo (ecard1): This attack does 20 damage plus 10 more damage for each Energy card attached to the Defending Pokémon.
- 4 x attack: Poliwrath (ecard1): Flip a number of coins equal to the amount of Water Energy attached to Poliwrath. This attack does 40 damage plus 10 more damage for each heads.
- 4 x attack: Cyndaquil (ecard1): Discard a Fire Energy card attached to Cyndaquil.
- 3 x attack: Misty's Tentacruel (gym1): Flip a coin. If heads, the Defending Pokémon is now Poisoned. If tails, the Defending Pokémon is now Confused.
- 3 x attack: Brock's Sandslash (gym1): Flip a coin. If heads, the Defending Pokémon is now Paralyzed and Poisoned.
- 3 x attack: Koga's Kakuna (gym2): Flip a coin. If heads, the Defending Pokémon is now Poisoned. It now takes 20 Poison damage instead of 10 after each player's turn (even if it was already Poisoned).
- 3 x attack: Ampharos (neo1): Flip a coin. If heads, the Defending Pokémon is now Paralyzed and this attack does 10 damage to each of your opponent's Benched Pokémon. (Don't apply Weakness and Resistance for Benched Pokémon.)
- 3 x attack: Houndour (neo2): Before doing damage, discard all Trainer cards attached to the Defending Pokémon (before they affect the damage).
- 3 x attack: Omastar (neo2): Flip a number of coins equal to the number of Water Energy attached to Omastar. This attack does 20 damage plus 20 more damage for each heads.
- 3 x attack: Ampharos (neo3): Flip a coin. If heads, this attack does 40 damage plus 20 more damage. If tails, this attack does 40 damage and the Defending Pokémon is now Paralyzed.
- 3 x Pokémon Power: Entei (neo3): As long as Entei is your Active Pokémon, Entei and Energy cards attached to it aren't affected by effects from Trainer cards other than Stadium cards. As long as this Power is active, discard any Trainer ca
- 3 x attack: Dark Ampharos (neo4): Discard all Lightning Energy cards attached to Dark Ampharos or this attack does nothing.
- 3 x attack: Dark Porygon2 (neo4): Flip a coin. If heads, prevent all damage done by attacks to Dark Porygon2 during your opponent's next turn. (Any other effects of attacks still happen.)
- 3 x attack: Alakazam (ecard1): If Alakazam and the Defending Pokémon don't have the same number of Energy cards attached to them, this attack's base damage is 20 instead of 80.
- 3 x attack: Gengar (ecard1): Switch Gengar with 1 of your Benched Pokémon, if any.
- 3 x attack: Pichu (ecard1): Take a basic Energy card attached to 1 of your Pokémon and attach it to another of your Pokémon.
- 3 x attack: Electabuzz (ecard1): Search your deck for a basic Energy card and attach it to Electabuzz. Shuffle your deck afterward.
- 3 x attack: Electabuzz (ecard1): If you have any Benched Pokémon and if there are any basic Energy cards attached to Electabuzz, take 1 of those Energy cards and attach it to 1 of those Pokémon.
- 3 x attack: Magmar (ecard1): Flip a coin. If heads this attack does 30 damage plus 10 more damage. If tails, Magmar does 10 damage to itself.
- 3 x attack: Ninetales (ecard2): Flip a coin. If heads, choose a Special Condition (Asleep, Burned, Confused, Paralyzed, or Poisoned). The Defending Pokémon is now affected by that Special Condition.
- 3 x attack: Sudowoodo (ecard2): Choose 1 of the Defending Pokémon's attacks. Copy copies that attack. This attack does nothing if Sudowoodo doesn't have the Energy necessary to use that attack. (You must still do anything else requi
- 3 x Poké-Body: Kingdra (ecard2): Whenever you attach a Water, Lightning, or Psychic basic Energy card from your hand to Kingdra, Kingdra's type (color) becomes the same as that Energy card type until the end of the turn.
- 3 x attack: Flareon (ecard3): The Defending Pokémon is now Burned.
- 3 x attack: Poliwrath (ecard3): Flip a coin until you get tails. This attack does 40 damage plus 20 more damage for each heads.
- 2 x attack: Lt. Surge's Electabuzz (gym1): Take up to 2 Lightning Energy cards from your discard pile and attach them to Lt. Surge's Electabuzz.
- 2 x TRAINER: Brock (gym1): Remove 1 damage counter from each of your Pokémon that has any damage counters on it.
- 2 x TRAINER: Erika (gym1): You may draw up to 3 cards, then your opponent may draw up to 3 cards.
- 2 x TRAINER: Lt. Surge (gym1): You can play this card only if you have at least 1 Basic Pokémon card in your hand. Put a Basic Pokémon card from your hand into play as your Active Pokémon. Put your old Active Pokémon onto your Bench.
- 2 x TRAINER: Misty (gym1): Discard 2 of the other cards in your hand in order to play this card. If this turn's attack does damage to the Defending Pokémon (after applying Weakness and Resistance), and if the attacking Pokémon has Mi
- 2 x attack: Brock's Golem (gym1): If your opponent has any Benched Pokémon, choose up to 3 of them. This attack does 10 damage to each of them. (Don't apply Weakness and Resistance for Benched Pokémon.)
- 2 x attack: Rocket's Snorlax (gym1): Rocket's Snorlax is now Asleep (after doing damage).
- 2 x attack: Sabrina's Venomoth (gym1): Flip 3 coins. For each heads, remove 1 damage counter from each of your Pokémon. If a Pokémon has fewer damage counters than the number of heads, remove all of them.
- 2 x attack: Blaine's Magmar (gym1): Discard the top 5 cards of your deck. (If there are fewer than 5 cards in your deck, discard all of them.) This attack does 20 damage for each Fire Energy card you discarded in this way.
- 2 x attack: Erika's Exeggcute (gym1): During your opponent's next turn, whenever Erika's Exeggcute takes damage, divide that damage in half (rounded down to the nearest 10). (Any other effects still happen.)
- 2 x attack: Erika's Exeggutor (gym1): Shuffle your hand into your deck, then draw 5 cards.
- 2 x attack: Misty's Seaking (gym1): If your opponent has any Benched Pokémon, choose 1 of them and flip a coin. If heads, this attack does 10 damage to that Pokémon. (Don't apply Weakness and Resistance for Benched Pokémon.)
- 2 x attack: Sabrina's Slowbro (gym1): You can't use this attack during your next turn.
- 2 x attack: Erika's Oddish (gym1): If there are any damage counters on Erika's Oddish, remove 1 of them.
- 2 x attack: Erika's Venusaur (gym2): Flip a coin. If heads, you may attach up to 2 Grass Energy cards from your hand to Erika's Venusaur.
- 2 x attack: Rocket's Zapdos (gym2): If there are any Lightning Energy cards in your discard pile, attach 1 of them to Rocket's Zapdos.
- 2 x TRAINER: Blaine (gym2): During this turn, instead of attaching your free Energy card, you may instead attach 2 Fire Energy cards to 1 of your Pokémon with Blaine in its name. (Playing additional copies of this card this turn has 
- 2 x TRAINER: Giovanni (gym2): Choose 1 of your Pokémon in play with Giovanni in its name. For the rest of your turn, you may evolve that Pokémon even if you just played or evolved it this turn or if this is your first turn. This effe
- 2 x TRAINER: Koga (gym2): If an attack from a Pokémon with Koga in its name does damage to a Defending Pokémon this turn, that Pokémon is then Poisoned.
- 2 x TRAINER: Sabrina (gym2): Take all Energy cards attached to 1 of your Pokémon with Sabrina in its name and attach them to another 1 of your Pokémon with Sabrina in its name.
- 2 x attack: Blaine's Ninetales (gym2): Flip a coin. If tails, discard all Fire Energy cards attached to Blaine's Ninetales.
- 2 x attack: Lt. Surge's Jolteon (gym2): Flip a coin. If heads, your opponent can't play Trainer cards during his or her next turn.
- 2 x attack: Giovanni's Nidorino (gym2): If the Defending Pokémon already has any damage counters on it, this attack does 20 damage plus 20 more damage. If not, this attack does 20 damage.
- 2 x attack: Sabrina's Haunter (gym2): Flip a coin. If heads, and if your opponent has any Benched Pokémon, choose 1 of them and this attack does 30 damage to it. (Don't apply Weakness and Resistance for Benched Pokémon.)
- 2 x attack: Sabrina's Abra (gym2): This attack can't be used unless Sabrina's Abra and the Defending Pokémon have the same number of Energy cards attached to them.
- 2 x attack: Sabrina's Abra (gym2): Look at your opponent's hand.
- 2 x attack: Jumpluff (neo1): If this attack damages the Defending Pokémon (after applying Weakness and Resistance), remove 1 damage counter from Jumpluff, if it has any.
- 2 x attack: Slowking (neo1): Flip a coin. If heads, this attack does 20 damage plus 10 more damage and the Defending Pokémon is now Confused. If tails, this attack does 20 damage.
- 2 x ENERGY: Metal Energy (neo1): Damage done to the Pokémon Metal Energy is attached to is reduced by 10 (after applying Weakness and Resistance). If the Pokémon Metal Energy is attached to isn't Metal, whenever it damages a Pokémon
- 2 x attack: Xatu (neo1): Look at the top 3 cards of either player's deck and rearrange them as you like.
- 2 x attack: Spinarak (neo1): Flip a coin. If heads, until the end of your opponent's next turn, the Defending Pokémon can't attack or retreat.
- 2 x TRAINER: Gold Berry (neo1): Attach Gold Berry to 1 of your Pokémon that doesn't have a Pokémon Tool attached to it. At any time between turns, if there are at least 4 damage counters on the Pokémon Gold Berry is attached to, you 
- 2 x TRAINER: Moo-Moo Milk (neo1): Choose 1 of your Pokémon. Flip 2 coins. Remove 2 damage counters times the number of heads from that Pokémon. If the Pokémon has fewer damage counters than that, remove all of them.
