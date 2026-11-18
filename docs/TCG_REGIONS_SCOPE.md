# The regional sets: what plays (tools/tcg/regions/scope.py)

Generated; rerun after translating (docs/TCG_REGIONS.md). A card is finished when every text and rule on it
plays (tools/tcg/regions/effects.py); only finished cards appear in the game.

| Set | Cards | Finished |
|---|---|---|
| gym1 | 126 | 115 |
| gym2 | 126 | 114 |
| neo1 | 105 | 96 |
| neo2 | 75 | 71 |
| neo3 | 66 | 57 |
| neo4 | 113 | 97 |
| ecard1 | 159 | 157 |
| ecard2 | 182 | 173 |
| ecard3 | 182 | 172 |
| basep | 53 | 45 |
| np | 40 | 37 |
| **all** | 1227 | 1134 |

## Rules the engine doesn't play yet (cards needing each)


## Wordings to translate (84; the most cards first)

- 3 x POWER: Entei (neo3-17): As long as Entei is your Active Pokemon, Entei and Energy cards attached to it aren't affected by effects from Trainer cards other than Stadium cards. As long as this Power is active, discard any Trainer cards attached to E
- 2 x POWER: Gengar (ecard1-13): Once during your turn (before your attack), if your opponent has 3 or fewer Prizes, you may move 1 damage counter from 1 Pokemon (yours or your opponent's) to another (even if it would Knock Out the other Pokemon). This 
- 2 x POWER: Lanturn (ecard2-21): You may use this power once during each of your turns (before your attack). All Lightning Energy attached to your Active Pokemon becomes Water Energy for the rest of the turn. (This effect ends if your Active Pokemon re
- 2 x POWER: Magneton (ecard2-22): Once during your turn (before your attack), if Magneton is your Active Pokemon, you may flip a coin. If heads, choose 2 of your opponent's Pokemon that have Energy cards attached to them. Choose 1 of the Energy cards a
- 2 x attack: Slowking (ecard2-34): Look at a number of cards on top of your opponent's deck equal to the number of Energy cards attached to the Defending Pokemon. Put those cards in any order, and then put them back on top of your opponent's deck.
- 2 x POWER: Gengar (ecard3-10): When you play Gengar from your hand to evolve your Active Pokemon, you may put a Basic Pokemon (excluding Baby Pokemon) from your discard pile onto your Bench. Then, flip 3 coins. For each heads, choose a basic Energy ca
- 2 x POWER: Magcargo (ecard3-18): When you play Magcargo from your hand to evolve your Active Pokemon, you may discard the top 3 cards of your deck and and then shuffle 3 basic Energy cards from your discard pile into your deck. If you do, your opponen
- 2 x attack: Magneton (ecard3-19): You may discard all Lightning Energy cards attached to Magneton when you use this attack. If you do, put damage counters equal to the amount of Energy cards removed in this way on any number of your opponent's Benched 
- 2 x attack: Politoed (ecard3-25): Move 2 Water Energy cards attached to Politoed to 1 or 2 of your Benched Pokemon. (You may put both on the same Pokemon or 1 each on 2 different Pokemon.) If you have no Benched Pokemon, ignore this effect.
- 1 x POWER: Brock's Rhydon (gym1-2): As long as Brock's Rhydon is Benched, whenever 1 of your Benched Pokemon is damaged, you may do 10 of that damage to Brock's Rhydon instead. (If more than 1 of your Benched Pokemon is damaged at the same time, you m
- 1 x attack: Erika's Clefable (gym1-3): Flip a coin. If heads, you may return any number of your Pokemon in play and all cards attached to them to your hand.
- 1 x POWER: Misty's Tentacruel (gym1-10): If an attack does damage to Misty's Tentacruel while it's your Active Pokemon, you may switch it with 1 of your Benched Pokemon, which prevents all other effects of that attack on Misty's Tentacruel. This power
- 1 x TRAINER: Blaine's Quiz #1 (gym1-97): Put a Basic Pokemon or Evolution card from your hand face down in front of you and tell your opponent its name. Your opponent guesses the length of that Pokemon. Flip the card over. If your opponent guessed right
- 1 x TRAINER: Charity (gym1-99): Attach Charity to your Active Pokemon. Unless that Pokemon gets Knocked Out, return Charity to your hand at the end of your turn. If that Pokemon attacks and does damage to the Defending Pokemon, you may reduce that damag
- 1 x TRAINER: No Removal Gym (gym1-103): This card stays in play when you play it. Discard this card if another Stadium card comes into play. A player must discard 2 cards from his or her hand in order to play an Energy Removal or Super Energy Removal ca
- 1 x TRAINER: Lt. Surge's Treaty (gym1-112): Your opponent chooses 1 of the following: everyone chooses 1 of his or her own Prizes and put it into his or her hand, or you draw a card.
- 1 x TRAINER: Sabrina's ESP (gym1-117): Attach Sabrina's ESP to 1 of your Pokemon with Sabrina in its name. At the end of your turn, discard Sabrina's ESP. If that Pokemon uses and attack that involves flipping coins, Sabrina's ESP lets you re-flip those
- 1 x TRAINER: Tickling Machine (gym1-119): Flip a coin. If heads, your opponent sets aside all the cards in his or her hand face down. Nobody may look at those cards. At the end of your opponent's next turn, your opponent puts those cards back into his o
- 1 x TRAINER: Vermilion City Gym (gym1-120): This card stays in play when you play it. Discard this card if another Stadium card comes into play. Whenever a player attacks with a Pokemon with Lt. Surge in its name, he or she may flip a coin. If heads, an
- 1 x TRAINER: Narrow Gym (gym1-124): This card stays in play when you play it. Discard this card if another Stadium card comes into play. No player may have more than 4 Pokemon on his or her Bench. When this card is played, if a player has 5 Pokemon on h
- 1 x POWER: Brock's Ninetales (gym2-3): Once during your turn (before your attack), you may attach an Evolution card from your hand to Brock's Ninetales. (This doesn't count as evolving Brock's Ninetales.) Treat Brock's Ninetales as if it were that Pok
- 1 x attack: Sabrina's Hypno (gym2-56): Choose 1 Basic Pokemon in any player's discard pile. Put it onto that player's Bench. Put a number of damage counters on that Pokemon equal to half its HP (rounded down to the nearest 10). (You can't put a Pokemon
- 1 x attack: Sabrina's Jynx (gym2-57): Choose 1 of your opponent's Pokemon. Remove any number of damage counters from that Pokemon, then draw that many cards.
- 1 x attack: Misty's Psyduck (gym2-90): Flip 3 coins. If exactly 1 is heads, draw a card. If exactly 2 are heads, this attack does 20 damage. If all 3 are heads, choose 1 of the Defending Pokemon's attacks. Misty's Psyduck copies that attack except for 
- 1 x TRAINER: Brock's Protection (gym2-101): Attach Brock's Protection to 1 of your Pokemon with Brock in its name. Energy cards attached to that Pokemon can't be removed by your opponent's attacks or Trainer cards. (This doesn't stop the rest of the att
- 1 x TRAINER: Chaos Gym (gym2-102): This card stays in play after being played. Discard this card if another Stadium card comes into play. Whenever a player plays a Trainer card other than a Stadium card, he or she flips a coin. If heads, that player pla
- 1 x TRAINER: Lt. Surge's Secret Plan (gym2-107): Put 1 card from your hand face down onto your Bench. (You can't play this card if your Bench is full.) Treat that card as a Basic Pokemon as long as it's face down. Flip the card if either player needs to
- 1 x TRAINER: Misty's Wish (gym2-108): Look at 1 of your Prize cards. Then, ask your opponent if you may switch that card with 1 of the cards in your hand. If your opponent accepts and if you have any other cards in your hand, switch those cards. If your
- 1 x TRAINER: Blaine's Quiz #2 (gym2-111): Put a card from your hand face down in front of you. Your opponent guesses whether it is an Energy card, a Trainer card, or a Pokemon (Basic or Evolution) card. Flip the card over. If your opponent guessed right
- 1 x TRAINER: Blaine's Quiz #3 (gym2-112): Put a Basic or Evolution card from your hand face down in front of you and tell your opponent the name of an attack of that card. Your opponent guesses the name of that card. Flip the card over. If your opponent
- 1 x TRAINER: Koga's Ninja Trick (gym2-115): Attach Koga's Ninja Trick to your Active Pokemon with Koga in its name. If this Pokemon goes to your Bench, discard this card. When your opponent attacks, you may switch this Pokemon with 1 of your Benched Pok
- 1 x TRAINER: Sabrina's Psychic Control (gym2-121): Flip a coin. If heads, choose a Trainer card in your opponent's discard pile that isn't put in play (like PlusPower or Mysterious Fossil). You may use that card as if it were in your hand, if you do eve
- 1 x POWER: Slowking (neo1-14): Whenever your opponent plays a Trainer card, you may flip a coin. If heads, that card does nothing. Put it on top of your opponent's deck. This power can't be used if Slowking is Asleep, Confused, or Paralyzed.
- 1 x attack: Togetic (neo1-16): Flip a coin. If heads, choose an attack on 1 of your opponent's Pokemon. Super Metronome copies that attack except for its Energy cost. (You must still do anything else in order to use that attack.) (No matter what type t
- 1 x attack: Bayleef (neo1-29): Flip a coin. If heads and if any of your Pokemon have any damage counters on them, then remove 2 damage counters from 1 of them (or 1 if it only has 1). If tails and if any of your opponent's Pokemon have any damage count
- 1 x POWER: Lanturn (neo1-38): You may make Floodlight do 10 more damage for each Water Energy attached to Lanturn, but not used to pay for Floodlight's Energy cost. This power can't be used if Lanturn is Asleep, Confused, or Paralyzed.
- 1 x attack: Quilava (neo1-47): If the Defending Pokemon doesn't have a Char counter on it, flip a coin. If heads, put a Char counter on it. A Char counter requires your opponent to flip a coin after every turn. If tails, put 2 damage counters on the Po
- 1 x TRAINER: Ecogym (neo1-84): This card stays in play when you play it. Discard this card if another Stadium card comes into play. Whenever an attack, Pokemon Power, or Trainer card discards another player's non-Colorless Energy card from a Pokemon, re
- 1 x TRAINER: Time Capsule (neo1-90): Your opponent may choose 5 Basic Pokemon, Evolution, and/or basic Energy cards in his or her discard pile. (If your opponent doesn't have that many, he or she chooses all or none of them.) If your opponent chooses an
- 1 x TRAINER: Card-Flip Game (neo1-92): Choose 1 of your opponent's face-down Prizes. Guess whether it is an Energy card, a Trainer card, or a Pokemon (Basic or Evolution) card. Flip the card face up (and leave it face up). If you guessed right, draw 2 c
- 1 x TRAINER: Double Gust (neo1-100): If you have any Benched Pokemon, your opponent chooses 1 of them and switches it with your Active Pokemon. Then, if your opponent has any Benched Pokemon, choose 1 of them and switch it with his or her Active Pokemon
- 1 x POWER: Eevee (neo2-38): Whenever you attach an Energy card to Eevee, flip a coin. If heads, search your deck for a card that evolves from Eevee that is the same type as the Energy card you attached to Eevee. Attach that card to Eevee. This counts 
- 1 x POWER: Unown [E] (neo2-67): When you play Unown [E] from your hand, your opponent may shuffle his or her hand into his or her deck and then draw 4 cards. Either way, you may shuffle your hand into your deck and draw 4 cards.
- 1 x POWER: Unown [O] (neo2-69): Once during your turn (before your attack), you may look at 5 cards from the top of your opponent's deck and put them back in the same order.
- 1 x TRAINER: Fossil Egg (neo2-72): Flip a coin. If heads, search your deck for a card that evolves from Mysterious Fossil and put it onto your Bench or put a card that evolves from Mysterious Fossil from your hand onto your Bench. Either way, treat the 
- 1 x attack: Ampharos (neo3-1): Flip 3 coins. For each heads, you may search your deck for a Lightning Energy card and attach it to 1 of your Lightning Pokemon. Shuffle your deck afterward.
- 1 x POWER: Entei (neo3-6): When you play Entei from your hand, you may discard the top 5 cards of your deck. (If you have fewer cards in your deck than that, discard all of them.) If any of those are Fire Energy cards, attach them to any of your Fire 
- 1 x attack: Jumpluff (neo3-9): Choose any number of your Hoppips and Skiplooms. Then, for each Pokemon you chose in this way, you may search your deck for a card that evolves from that Pokemon and attach it to that Pokemon. (This counts as evolving tho
- 1 x POWER: Porygon2 (neo3-12): Once during your turn (before your attack), you may choose 1 basic Energy card attached to 1 of your Pokemon and choose an Energy type. Treat that Energy card as that type until the end of your turn. This power can't be 
- 1 x POWER: Parasect (neo3-35): As long as Parasect is in play, cards in any player's discard piles are not affected by attacks or Pokemon Powers. This power stops working if Parasect becomes Asleep, Confused, or Paralyzed.
- 1 x POWER: Unown [K] (neo3-58): Your opponent's attacks, Pokemon Powers, and Trainer cards can't discard Energy cards from your Pokemon with Unown in their names. (Any other effects still happen.)
- 1 x POWER: Dark Porygon2 (neo4-8): Once during your turn (before your attack), you may flip a coin. If heads, choose a Stadium card from your discard pile and put it into play. (If there's already a Stadium card in play, discard it.) This power can't 
- 1 x POWER: Light Dragonite (neo4-14): As long as Light Dragonite is your Active Pokemon, each Special Energy card provides Colorless Energy instead of its usual type and its other effects stop working. This power stops working while Light Dragonite is
- 1 x POWER: Light Togetic (neo4-15): When you play Light Togetic from your hand, your opponent may search his or her deck for a Pokemon Tool card, show that card to you, and put it into his or her hand. Either way, you may do the same, and then each pl
- 1 x ENERGY: Miracle Energy (neo4-16): You can't have more than 1 Miracle Energy in your deck. Attach Miracle Energy to 1 of your Shining or Light Pokemon. At the end of your turn, discard Miracle Energy. While in play, Miracle Energy counts as every ty
- 1 x POWER: Dark Slowking (neo4-20): Once during your turn (before your attack), you may flip a coin. If heads, look at the top card of your opponent's deck. Then, you may have your opponent shuffle his or her deck. This power can't be used if Dark Slo
- 1 x attack: Dark Ursaring (neo4-21): Look at your opponent's hand. If he or she has any Baby Pokemon and/or Basic Pokemon there, you may put any number of them onto your opponent's Bench (as long as there's room). Then, your opponent looks at your hand
- 1 x attack: Light Lanturn (neo4-23): Flip a coin. If heads, each player may choose a card from his or her discard pile and put it into his or her hand.
- 1 x attack: Light Golduck (neo4-47): Your opponent looks at the top 3 cards of his or her deck. If any of them are basic Energy cards, he or she may show any number of them to you and put them into his or her hand. You do the same. Either way, each pla
- 1 x attack: Light Slowbro (neo4-51): Your opponent may choose up to 3 Baby Pokemon, Basic Pokemon, and/or Evolution cards from his or her discard pile and shuffle them into his or her deck. Either way, you may do the same.
- 1 x attack: Light Venomoth (neo4-53): Your opponent may choose a Baby Pokemon, Basic Pokemon, or Evolution card from his or her discard pile and put it into his or her hand. Either way, you may do the same.
- 1 x attack: Light Wigglytuff (neo4-54): Your opponent may choose 1 of his or her Pokemon and search his or her deck for a card that evolves from that Pokemon. Your opponent attaches that card to that Pokemon. This counts as evolving that Pokemon. Eithe
- 1 x POWER: Unown [S] (neo4-87): Once during your turn (before your attack), you may look at 1 of your Prize cards. Return that Prize card face down. This power can be used even if Unown [S] is Asleep, Confused, or Paralyzed.
- 1 x POWER: Unown [T] (neo4-88): Once during your turn (before your attack), you may flip a coin. If heads, look at your opponent's hand and show your hand to your opponent. This power can be used even if Unown [T] is Asleep, Confused, or Paralyzed.
- 1 x TRAINER: EXP.ALL (neo4-93): Attach EXP.ALL to 1 of your Pokemon that doesn't have a Pokemon Tool attached to it. During your opponent's turn, if your Active Pokemon would be Knocked Out by your opponent's attack, you may take 1 of the basic Energy c
- 1 x TRAINER: Impostor Professor Oak's Invention (neo4-94): Look at your opponent's Prize cards. You may have your opponent shuffle them into his or her deck. If you do, your opponent takes that many cards from the top of his or her deck and sets them as
- 1 x TRAINER: Pokemon Personality Test (neo4-102): Put an Evolution card from your hand face down in front of you. Your opponent guesses whether it is a Pokemon card with Light in its name, a Pokemon card with Dark in its name, or neither one. Flip the c
- 1 x attack: Elekid (ecard2-9): Flip a coin. If heads, choose an Energy card attached to the Defending Pokemon. Your opponent moves that card to 1 of his or her other Pokemon. (If your opponent has no Benched Pokemon, this attack does nothing.)
- 1 x POWER: Furret (ecard2-48): Once during your turn (before your attack), you may put 2 cards from your hand into your deck. Then, search your deck for an Energy card, show it to your opponent, and put it into your hand. Shuffle your deck afterward. 
- 1 x TRAINER: Time Shard (ecard2-135): Attach Time Shard to 1 of your Pokemon that doesn't already have a Pokemon Tool card attached to it. If that Pokemon is Knocked Out, discard this card. If the Pokemon this card is attached to is Knocked Out by damag
- 1 x POWER: Noctowl (ecard3-40): Once during your turn (before your attack), you may look at the top 2 cards of any player's deck or at up to 2 of any player's Prizes. Put any cards you looked at back in the same order. This power can't be used if Noct
- 1 x TRAINER: Star Piece (ecard3-139): Attach Star Piece to 1 of your Pokemon that doesn't already have a Pokemon Tool attached to it. If that Pokemon is Knocked Out, discard this card. At any time between turns, if the Pokemon this card is attached to i
- 1 x POWER: Dragonite (basep-5): Once during your turn (before your attack), you may draw a card. If you do, choose a card from your hand and put it on top of your deck. This power can't be used if Dragonite is Asleep, Confused, or Paralyzed.
- 1 x attack: Meowth (basep-10): Flip a coin. If heads, this attack does 20 damage. If tails and if your opponent has any Benched Pokemon, he or she chooses 1 of them and this attack does 20 damage to it. (Don't apply Weakness and Resistance for Benched 
- 1 x POWER: Eevee (basep-11): This power can only be used when a Pokemon evolves. Search your deck for a card that evolves from Eevee and attach it to Eevee. This counts as evolving Eevee. Shuffle your deck afterward. This power can't be used if Eevee 
- 1 x attack: Mewtwo (basep-12): Flip a coin. If heads, choose a basic Energy card attached to 1 of your opponent's Pokemon and attach it to another of your opponent's Pokemon of your choice.
- 1 x attack: Cool Porygon (basep-15): You may change Cool Porygon's Resistance to a type of your choice other than Colorless. If the Defending Pokemon has a Weakness, you may change it to a type of your choice other than Colorless. (Benching either Poke
- 1 x attack: Smeargle (basep-32): Flip a coin. If heads, choose a type (other than Colorless) and put a Coloring counter on the Defending Pokemon. That Pokemon is now the type you chose. If it already had a Coloring counter, remove the old one. If tails
- 1 x TRAINER: Pokemon Tower (basep-42): This card stays in play when you play it. Discard this card if another Stadium card comes into play. If the effect of a Pokemon Power, attack, Energy card, or Trainer card would put a card in a discard pile into it
- 1 x attack: Electabuzz (basep-46): Choose 1 of your opponent's Pokemon and put a Lightning Rod marker on it. (A Pokemon can have only 1 Lightning Rod marker on it at a time.)
- 1 x attack: Electabuzz (basep-46): This attack does 20 damage to any Pokemon with a Lightning Rod marker on it. Apply Weakness and Resistance.
- 1 x TRAINER: Championship Arena (np-28): This card stays in play when you play it. Discard this card if another Stadium card comes into play. If another card with the same name is in play, you can't play this card. At the end of each player's turn, if t
- 1 x BODY: Suicune (np-30): Whenever you attach a Water Energy card from your hand to Suicune, discard an Energy card attached to Suicune. (You can't attach a Water Energy card from your hand if Suicune has no Energy cards attached.)
- 1 x attack: Mew (np-40): Flip a coin. If heads, choose 1 of either player's Evolved Pokemon, remove the highest stage Evolution card from that Pokemon, and put it into that player's hand.
