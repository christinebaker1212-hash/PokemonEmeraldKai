MOEMON EMERALD KAI - CO-OP EDITION  (native PC, up to 4 players online)
=======================================================================

Moemon Emerald Kai v1.0 rebuilt as a native PC game. No emulator or ROM is
needed. On top of Kai it adds online co-op for up to 4 players:

  * Everyone plays their own game in the same world and sees each other
    walk, run, bike, surf and dive in real time, with a name tag above
    their head ("Misty Lv25").
  * Team up against NPC trainers: walk up to a friend who has just been
    challenged and join their battle as a Double Battle (three of you: a
    Triple Battle).
  * Watch a friend's battle: walk up to someone marked IN BATTLE and press A.
  * PvP with the BATTLE FLAG.
  * Dress up as any trainer or NPC in the game, recolor them, or draw your
    own sprites with the included Sprite Editor.
  * MOEMON GARDEN: every MOEMON has its own temperament, feelings and
    memories, does its own thing on the map and keeps a JOURNAL page.
  * Proper fullscreen, sharp scaling at any window size, and display filters.


QUICK START
-----------
1. Unzip the folder anywhere. Every player needs their own copy.
   Tip: before unzipping, right-click the zip > Properties > tick
   "Unblock" > OK. Windows then doesn't treat the game as a download.
2. Start MoemonEmeraldKaiCoop.exe and press START on the title screen.
   If Windows shows "Windows protected your PC" (SmartScreen), click
   "More info" and then "Run anyway". Windows shows this for any new
   program that isn't signed with a paid certificate; the game asks for
   no special rights and doesn't change your system.
3. Choose:
     HOST A GAME  - type a server name (and a password if you want one),
                    then START HOSTING. The game opens your router by
                    itself and shows an INVITE CODE like K7QM-3XPA.
                    It's copied, so just paste it to your friends.
                    WELCOME NOTE (optional) is two short lines your
                    friends see when they join. TEAM PADDING (ON at
                    first) gives trainers one extra Moemon when friends
                    team up against them (see TEAMING UP below).
                    ANIM SPEED (+30% at first) makes battle animations
                    (moves and everything else) play 25% or 30% faster
                    for everyone in the server, or NORMAL.
     JOIN A GAME  - choose ENTER INVITE CODE... and type the code your
                    friend sent, then the password if they set one.
                    Friends on the same Wi-Fi also show up in the list.
     PLAY SOLO    - play alone. Go online later from the CO-OP menu (C key, or
                    Y/Triangle on a controller).
4. Load your save or start a new game. That's it.
   In a new game, PROF. BIRCH asks to see you right after "boy or girl?":
   that's the LOOK menu, so you can dress your trainer before the adventure.
   Before you leave, he offers to SKIP AHEAD: say YES to pick and name your
   first MOEMON right there and start in LITTLEROOT with the POKEDEX, POKE
   BALLS and RUNNING SHOES, as if you'd already beaten your rival on Route 103.

Typing works with the real keyboard (Ctrl+V pastes). The game remembers the
last server, so next time JOIN offers it straight away. While you host, the
invite code is also shown at the bottom of the CO-OP menu.


PLAYING OVER THE INTERNET
-------------------------
No cloud server, port forwarding or VPN app is needed in most homes:

  * When you host, the game asks your router to open its port (UPnP, or
    NAT-PMP on Apple routers) and works out your home's internet address.
    That becomes the invite code. Friends anywhere in the world type it.
  * When you stop hosting or close the game, the port is closed again.
    If the game ever crashes, the router closes it by itself within two
    hours, and the next time you start the game it cleans up.
  * Windows asks whether to allow the game through the firewall the first
    time you host. Tick BOTH private and public networks, then Allow.

If the game says it couldn't open your router:
  1. Turn on "UPnP" in your router's settings page (often under Advanced,
     NAT or Gaming), then host again. This fixes most cases.
  2. If it says your internet provider shares one address between many
     homes (called CGNAT, common on mobile and some fiber connections),
     nobody can reach your PC directly. Let a friend with a normal
     connection host instead, or use the last resort below.
  3. Last resort: Radmin VPN (free). The host and every friend install it;
     the host creates a network in it and friends join that network. Then
     host again: the game notices Radmin VPN and gives you an invite code
     that works inside that network.

If a friend can't connect:
  * Internet games never show up in your friend's JOIN list (that list is
    only for the same home network). Your friend must choose JOIN A GAME >
    ENTER CODE OR IP and type the invite code.
  * Windows Firewall is the usual cause. Windows asks the first time you
    host, with only "Private networks" ticked, but Windows 11 often counts
    home networks as "Public", and Radmin VPN's network is always "Public".
    Fix (once, on the host's PC): Windows Security > Firewall & network
    protection > Allow an app through firewall > Change settings > find
    the game and tick both Private and Public. If the game is listed more
    than once, tick every entry.
  * VPN apps (Surfshark, NordVPN...) can get in the way: turn them off.
  * With Radmin VPN, friends can also type the host's Radmin address
    (26.x.x.x, shown when you host) instead of the code.

Advanced: lobby.txt can point the game at a lobby server instead (see the
"server" folder and server\DEPLOY.md). You don't need one for normal play.


TEAMING UP FOR A DOUBLE BATTLE
------------------------------
* When an NPC trainer challenges you and a friend is on the same map or
  route, after the trainer's speech you're asked "Wait for another trainer
  to join you?" Say YES and a "Join me!" bubble appears over your head.
  Your friend walks up next to you, faces you, and they're in. Press B at
  any time to stop waiting and battle alone. While you wait, nobody else can
  talk to that trainer.
* If that trainer spots a friend while you wait (they step into its line of
  sight), it walks up to them as usual and they go straight into your battle:
  "You team up with <you>!", no question. If two of you are already waiting
  on the same trainer, whoever started waiting first leads and the other
  joins them on their own.
* Two trainers spotting you at once (like a pair facing each other across a
  path): you're asked about teaming up once, after the second one's speech.
  With a friend it's the two of you against both trainers, each trainer with
  their own Moemon on their side of the field; alone it's the usual battle
  against both. (No Triple Battle against two trainers.)
* When an NPC trainer challenges a friend (they get the "!" or talk to the
  trainer), walk up next to your friend and face them. You'll see
  "Asking <friend> to team up..." (press B to change your mind; turn away
  and face them again to ask again).
* When your friend finishes reading the trainer's speech, they get:
  "<you> wants to join for a Double Battle! Is this OK?"  YES / NO
  This message can't be skipped or sped up, so nobody says yes by accident.
* TEAM PADDING (a host setting: HOST A GAME > TEAM PADDING, ON at first):
  when friends team up against a trainer, the trainer gets one extra
  Moemon, never more. That's why a trainer with just one Moemon can still
  take on two of you, and one with two can take on three. With TEAM
  PADDING OFF, trainers only ever use their own team: one with a single
  Moemon refuses ("Hey, I only have one MOEMON! Let's go one on one!") and
  it takes three of their own for a Triple Battle. Everyone in the server
  plays by the host's setting.
* The game's clock (what time of day it is, the tides, daily events of the
  original game) is the host's PC clock while you're in their game, even if
  you live in another time zone, so everyone plays at the same time of day.
  Offline, or in a game you made, it's your own PC's clock. Berry trees grow
  by the time that really passes on your own clock, so joining or leaving a
  host in another time zone never makes them jump ahead (or stop).
* The game's clock is the PC's clock: the bedroom clock starts at your PC's
  time. Earlier versions had the clock stuck at midnight on day 1, so Berries
  never grew and daily events never came round (SKIP THE INTRO also left
  them off). Old saves are fixed when loaded: Berry trees carry on from where
  they are (no years of growth at once) and daily events get one new day.
* Two against one is meant to be a fight: the trainer's Moemon are 2 levels
  higher (3 for gym leaders, Team Aqua/Magma bosses and rematches), they
  use their items and they choose their moves smarter.
* In co-op battles you can use healing items (Potions, status heals,
  Revives...) from the BAG on your own Moemon or your partner's, including
  ones waiting on the bench (your partner's are the gold boxes).
* Emotes and quick phrases work in battle too: hold LT / RT (Q / E on the
  keyboard), pick with the D-Pad, change page with L / R and let go to send.
  Your bubble pops up over your Moemon on your partner's (or opponent's)
  screen. Phrases are edited outside battle.
* When you've picked your move first, the battle tells you what your partner
  is doing ("ALICE is choosing an item to use..."). Once your partner has
  picked, your menu shows their choice ("ALICE: EMBER -> ZIGZAGOON").
* Each player brings their CO-OP TEAM: pick POKeMON in the START menu, choose
  a Moemon and CO-OP TEAM to mark it 1, 2 or 3 (again to take it off). Marked
  Moemon go first; if fewer than three are marked (or one has fainted), your
  first healthy others fill the gaps. Nothing marked = your first three
  healthy Moemon.
* Both players get the full prize money, and each player gets the full EXP
  for their own Moemon (shared only among your own Moemon that battled), with
  your own EXP bonuses. Evolutions happen on each player's game after the
  battle.
* Everyone's EXP comes at once, at a single battle's pace: one "gained EXP"
  line (your screen names your own Moemon) and everyone levels up together
  with one "grew to LV." line.
* Then every player goes through their own level-up stats and "delete a move
  to make room?" choices on their own screen, at the same time. Whoever is
  done first sees "Waiting for the others..." until everyone is finished,
  then one line shows the moves everyone learned. It all happens during the
  battle, the same as in a single battle.
* If you lose, both of you black out. If someone disconnects, the battle is
  called off: your party goes back to how it was before the battle (no EXP,
  no prize money), but Moemon you already caught and items you already used
  stay that way.
* There's no running from a trainer, same as on your own.
* Ordinary trainers count as beaten for both players. Story battles (gym
  leaders, rivals, villains) only count for the challenged player, so the
  partner can still get their badge by battling them on their own later.


WILD BATTLES TOGETHER
---------------------
* Walking in tall grass with a friend nearby? When a wild Moemon jumps out,
  the closest friend who is also standing in grass (within about a screen
  of you) jumps into the battle with you. There's nothing to accept: it just
  happens, as long as they're free (not in a menu, a battle or a trade).
* It's two of you against one wild Moemon, and now and then two of them
  jump out at once (2 on 2). Both of you get EXP.
* You can throw POKe BALLS and use healing items. Whoever throws the Ball
  that catches it gets the Moemon. Against two, you pick which one to throw
  at (the D-Pad moves between them, A throws, B puts the Ball back), and
  catching one doesn't end the battle: the other one is still there.
* If your target was already caught or beaten earlier in the turn (say your
  partner's Ball got it first), your throw is called off and the Ball goes
  back to your BAG.
* Balls can't be thrown when your party and the PC are both full.
* CATCH EXP: catching a Moemon gives EXP too, to everyone in the battle (and
  when you play alone). The more of its HP you knocked off first, the more:
  almost a whole faint's worth at 1 HP, a tenth of it at full HP. Level-ups
  from it evolve after the battle as usual.
* Older versions turned a Moemon that a friend (not the host) caught into a
  BAD EGG. That's fixed. When you load a save, any BAD EGGS in your party or
  PC are removed and you get 10 MoeGatcha pulls for each one.
* RUN is a team decision: you get away only if both of you choose RUN in the
  same turn (one try, with your fastest Moemon). Choosing it alone just lets
  your partner know you want to leave.
* Friends further away (or not in the grass) don't join: you battle alone,
  like normal.
* Wild Moemon you fight together are 2 levels higher, and 60% of the time
  two of them show up.


TRIPLE BATTLES (THREE PLAYERS)
------------------------------
* Three friends can fight side by side: each of you controls one Moemon, all
  three are out at once, against up to three wild or trainer Moemon.
* Wild: when a wild Moemon jumps out, the two closest friends standing in
  grass nearby both jump in (usually three wild Moemon come out). With only
  one friend around it's the usual Double Battle.
* Trainers: once one friend has joined your battle you're asked "<friend>
  has joined! Wait for another?" NO starts the Double Battle right away. YES
  shows "<friend> is in! Waiting for one more for a Triple Battle..." and the
  "Join me!" bubble stays up. A second friend who walks up and faces you
  joins, and the Triple Battle starts as soon as all three of you are there
  (also if they turn up while the question is still on screen). To start a
  Double Battle without waiting after all, press B and answer YES (B on that
  question keeps waiting, so mashing through text can't start it by
  accident). You keep waiting as long as anyone else is online, even if
  they're busy in a battle or a menu right now. Two friends
  who ask at the same time both get in. Trainers with three or more Moemon
  do Triple Battles, counting the extra one from TEAM PADDING (so with it
  ON, two of their own are enough). Two-trainer battles like the Twins stay
  Double Battles. When a
  friend joins and another is online but the trainer can't do a Triple
  Battle, you're told so and it's a Double Battle.
* Each player brings two Moemon: the first two of your CO-OP TEAM (or your
  first two healthy ones). One is out, the other waits on the bench and comes
  in when the first faints or when you switch.
* Any Moemon can target any other: there's no "too far away" in these Triple
  Battles. Moves that hit both foes (Surf, Rock Slide...) hit all three foes
  for 3/4 damage; Earthquake and friends hit your allies too. Helping Hand
  lets you pick which ally to boost.
* The party screen shows the three Moemon out in battle on the left (yours
  first) and the ones waiting on the right, in the same order. Your allies'
  boxes are gold; you can heal any of the six.
* The cards show the name and gender, the level and the HP bar. A status
  (PSN, PAR, SLP, FRZ, BRN) covers the level, as on the usual boxes. The
  cards are see-through, so Moemon behind them stay visible.
* R on the action menu shows both allies' Moemon with their HP, and once an
  ally has picked, your menu shows it ("ALICE: EMBER").
* While you pick a target, the box on the right names it, with its level (a
  foe) or its exact HP (an ally). The D-Pad moves along all six.
* Moemon with the same name on one side get a letter, left to right
  (ZIGZAGOON A, B, C), on their cards and in the target box.
* Moves that change a stat of everyone on one side (Growl, Tail Whip, Leer,
  String Shot, Sweet Scent) play one animation over all of them and say it
  once: "The party's DEFENSE was lowered!" or "The foes' ATTACK was lowered!".
  This works in Double Battles too.
* Prize money, EXP, catches, disconnects and RUN (all three must choose it)
  work like in Double Battles.


TEAM GYM REMATCHES
------------------
Talk to a gym leader you've already beaten while a friend is on the same
map: they offer a team rematch with their strongest team (even stronger once
you have all eight badges). Your friend joins the usual way, by standing next
to you and facing you. Both of you get a reward for winning.


TOURNAMENT (CO-OP menu > TOURNAMENT)
---------------------------------------
A quick battle tournament between everyone online. Everyone battles everyone
once, a few matches at a time; with an odd number of players someone sits out
each round, but everyone plays the same number of matches. These are friendly
battles: no money changes hands and nobody blacks out. The champion and
second place get prize money; in the day's first tournament with three or
more players and no forfeits they also get MOEGATCHA pulls (2 and 1).
Everyone else gets a consolation prize.


WATCHING, BETTING ON OR JOINING A FRIEND'S BATTLE
-------------------------------------------------
Players in a battle have "IN BATTLE" above their name ("IN MENU" means
they're in the START or CO-OP menu, their BAG, a shop, the PC...). Walk up to someone
IN BATTLE and press A: "Would you like to spectate, bet, or join?"

  SPECTATE  Watch live. Your own game pauses while you watch. Press B (or
            START) to stop watching at any time. Watching ends by itself
            when the battle is over.
  BET       Bet $500 on how it ends, then watch (see BETTING).
  JOIN      Ask to join in. At the start of their next turn your friend is
            asked "<your name> wants to join your battle! Is this okay?" (B while
            you wait: never mind). NO: you're told so (and they aren't asked
            again in that battle). YES: your trainer slides into their battle
            ("<your name> has come to help!") and sends out your first Moemon.

Which battles: a wild Moemon met in grass, caves or water, or an ordinary
trainer (not a GYM LEADER, rival, TEAM AQUA / MAGMA or other story battle),
and Double Battles with a friend. Two friends can join at once.

  - A battle alone becomes a Double Battle (or a Triple Battle when two
    friends join together); a Double Battle with a friend becomes a Triple.
  - Everyone takes their Moemon in party order, the one out now first:
    three each in a Double Battle, two each in a Triple. The others wait in
    your party as they are (any that leveled up still get to evolve, and
    learn their moves, after a win).
  - The trainer gets a spot for each new player and sends out its next
    Moemon there if it has one (TEAM PADDING: at most one extra Moemon).
  - A Double Battle you started alone: your second Moemon goes back to its
    Ball and your friend takes that spot.
  - If someone leaves the game during the battle, it's called off, as in
    any team battle, and everyone's party is back to how it was when
    they joined.


FRIENDS' SCREENS (OPTIONS > CO-OP)
----------------------------------
Turn FRIENDS' SCREENS ON to see up to 3 friends' screens live, all the time:
walking around, in menus or in battle. Each card has the friend's name and
where they are (a red IN BATTLE tag in a battle).
Every friend online has a row: A opens their page.
  SHOW   ON / OFF: show this friend's screen at all.
  PLACE  BELOW, ABOVE, LEFT or RIGHT of your screen, or OWN WINDOW: a window
         of its own you can move anywhere (another monitor too). Closing that
         window turns SHOW off for that friend only; the others stay.
  SIZE   1X, 2X, 3X or 4X the game's size.
NEW FRIENDS is the size for friends you haven't set up yet (they start BELOW).
Your own screen gets smaller to make room, so everything stays on screen; when
a size doesn't fit your window or monitor a screen uses the biggest one that
does (the menu says "4X (2X FITS)"). The window grows to fit the screens and
goes back to its own size when you turn them off or your friends leave.
Everything is saved, each friend's by their name. Each friend sends about
20 KB/s while you watch.


PVP: THE BATTLE FLAG
--------------------
* You get the BATTLE FLAG (KEY ITEMS) the first time you connect. Register
  it to SELECT for quick use.
* Using it asks "Set phrases, or plant?".
  PLANT    Stand guard like an NPC trainer. While the flag is planted your
           name tag and follower are hidden, so you blend in. When another
           player walks into your line of sight you get a "!", walk up to
           them and a link battle starts. Press B to pack the flag up.
  PHRASES  Make up what you say, from the game's word list: your CHALLENGE
           (when you walk up to someone), what you say if you WIN and what
           you say if you LOSE. Pick one to change it, DONE when you're done.
           These are the same phrases the BATTLE TOWER uses.
* Before the battle both of you see your CHALLENGE (A closes it, or it closes
  by itself after a few seconds; the battle starts when both are closed).
  Afterwards the winner sees the loser's LOSE phrase and the loser sees the
  winner's WIN phrase.
* The winner gets a tenth of the loser's money plus a random Poke Ball. It
  isn't taken from the loser: they keep all their money and wake up at their
  last Moemon Center. HP/PP are restored and no EXP is gained, like Cable
  Club battles.


TRADING, THE SHARED PC AND DAILY GIFTS (CO-OP menu)
-----------------------------------------------------
TRADE...      Trade with a friend anywhere: a Moemon, up to two kinds of
              items and money, in any mix. Both of you pick your side, choose
              READY, then confirm. Moemon that evolve by trading evolve.
SHARED PC...  A PC that belongs to the server: anyone can put Moemon and
              items in, and anyone can take them out. It's kept on the host's
              PC, so it's still there the next time the same server name and
              password are hosted. It can't be reached from inside caves and
              dungeons: pack what you need before you go in.
              MOEMON: 14 shared boxes of 30, in the PC's own box screen (move,
              SUMMARY, MARK, RELEASE, TAG groups, box names and wallpapers,
              which everyone shares). Friends in the same box show as hands in
              their color with their name, carrying whatever they hold; a
              Moemon somebody holds can't be taken by anyone else. Along the
              top: PARTY, MY PC (your own boxes, and back; a Moemon you hold
              comes along) and CLOSE. A Moemon's menu (and the TAG menu) has
              SEND TO PC / SEND TO SHARED: straight into your own boxes or the
              shared ones. Anyone can release a shared Moemon ("Release LUNA's
              PIKACHU?"); the candy goes to whoever releases it.
              ITEMS: the shared items, and your own PC's (L/R to switch).
              DEPOSIT FROM BAG puts items in; on an item: WITHDRAW (to your
              BAG), SEND TO PC / SEND TO SHARED, CANCEL. TAG (X/Square, F)
              marks items to move them all at once. Friends' names show on the
              row they're looking at. Key items and HMs stay with you.
DAILY GIFT... Send each friend online today's gift once a day. They get the
              gift and you get a thank-you bonus (bigger with more players).
              The gift changes every day.
Moemon you get from a friend keep their original trainer's name and ID, but
always obey you as if you were their trainer (they still gain the traded
EXP bonus). The same goes for in-game trades.


PLAYING TOGETHER EVERY DAY (CO-OP menu > DAILY)
--------------------------------------------------
TEAM GOALS     Three new goals every day for everyone on your server:
                 one everybody helps with (beat trainers, catch POKEMON of
                   today's type, register new POKEDEX entries, catch the
                   outbreak, win without a faint...)
                 one for co-op battles (win co-op battles, catch in co-op
                   wild battles, use a friend's field move...)
                 one to do together (trade, take a TEAM PHOTO, battle each
                   other, bet on a friend's battle, blend BERRIES, visit a
                   SECRET BASE, swap TRAINER CARDS, send gifts)
               Playing alone, the last two become more "everybody" goals.
               Offline too: DAILY shows the day's three goals, counted on
               your own save, with the same rewards.
               Targets grow with the number of players online today; friends
               count after 5 minutes online. Each finished goal gives
               everyone money plus POKE BALLS / healing / a REVIVE. All three
               give a MOEGATCHA pull, up to 5 days a week (money after that).
               If the day ends with goals left, everyone who helped gets a
               small consolation prize.
OUTBREAK       One Moemon appears a lot on one route today (more often with
               more players online, offline too). It's announced when you
               connect.
STREAK         Play 10 minutes on the same map as a friend every day to build
               your co-op streak. Rewards grow each day, every 7th day gives a
               MOEGATCHA pull; it's on your TRAINER CARD.
WEEKLY RACE    A different challenge every week with a leaderboard. When two
               or more players raced, the winner gets a PP UP, money and a
               MOEGATCHA pull.
ACHIEVEMENTS   184 personal accomplishments, also viewable offline, in six
               tabs (L/R): JOURNEY (the story in order, HMs, landmarks,
               BATTLE FRONTIER symbols), BATTLE (levels, a full team,
               flawless gym wins, the BATTLE TOWER), CATCHING (the
               POKEDEX, every type, legendaries, the HOENN POKEDEX, eggs),
               HOENN (shops, contests, TV, money, the GAME CORNER...),
               TOGETHER (co-op wins, Triple Battles, rescues, trades,
               gifts, races, streaks) and EXTRAS (MOEGATCHA pulls and
               rare prizes, followers, your look, POKE BALL styles).
               Short goals give 3 MOEGATCHA pulls, longer ones 10, the
               longest 20. Anything your save has already done counts:
               new achievements you've already earned are given all at
               once, with one message.
GYM REMATCHES  The day's first team rematch win gives a MOEGATCHA pull and a
               RARE CANDY; later wins give money.
RARE CANDIES   Can't raise a Moemon past the next gym leader's ace level
               (15, 20, 27, 33, 36, 41, 47, 51; no limit after 8 badges).
MOEGATCHA      (CO-OP menu > MOEGATCHA) Spend pulls on the prize reel:
               it races past prizes and slowly clicks down onto yours
               (the prizes going by never show the same picture twice on
               screen).
               L/R switches banners:
                 STANDARD    event Moemon, items and money (always there).
                             Its event Moemon change every day: four at
                             R (shiny at SR) from Generation 1-3 events
                             and gifts (Surfing / Flying Pikachu, Amnesia
                             Psyduck, the ODD EGG babies with DIZZY PUNCH,
                             the Dragon's Den Dratini, Kirk's Shuckie,
                             Elm's Togepi, Colosseum's Espeon / Umbreon,
                             the Pokemon Box eggs) and six legends at UR
                             (shiny at EX: AURA / classic / Faraway Mew,
                             AGETO / GS BALL Celebi, WISHMKR / CHANNEL
                             Jirachi, SPACE C / Birth Island Deoxys, the
                             ticket Latias & Latios, Lugia, XD Lugia,
                             Ho-Oh). The banner's line shows today's. A
                             shiny RED GYARADOS is always an SR.
                             BERRIES too: everyday ones (N/U), SITRUS /
                             LUM and the rare POKeBLOCK ones (R), the
                             Mirage Island pinch Berries (SR), and
                             LANSAT, STARF and the e-Reader Berries (SSR).
                             Treats from other regions (N), EVIOLITE and
                             QUICK POWDER (R).
                 PICKUP      a new one every week: every UR is one of
                             its Moemon, every EX one of them shiny. Mew &
                             Mewtwo, Celebi, Jirachi, Dragonite, Tyranitar,
                             Salamence, Metagross, Ditto, Eevee, Feebas,
                             Deoxys, Hitmonlee & Hitmonchan, the three
                             birds, the Kanto starters, the Johto starters,
                             the three beasts, Lugia & Ho-Oh, Latios &
                             Latias, and a random shiny week. PICKUP
                             Moemon have OT GACHA (ID 23195); the event
                             ones carry the real events' levels and moves.
                             Moemon you haven't seen yet only show their
                             shadow. REROLL VOTE: with friends online,
                             SELECT on PICKUP asks everyone whether to
                             reroll this week's PICKUP. Nobody picks the
                             new one: if more than half say YES (you
                             count), a random different PICKUP is drawn
                             for everyone, for the rest of the week. Pity
                             counts stay. One reroll a week; friends who
                             come online later get it too.
                 TRAINING    Rare Candies, vitamins, PP Ups, Lucky Egg...
                 POKE BALLS  Net, Dive, Nest, Repeat, Timer, Luxury,
                             Premier and Master Balls
               Seven rarities (STANDARD): N 52%, U 28%, R 13.5%, SR 5%,
               SSR 1.2%, UR 0.25%, EX 0.05% (each banner's odds are on its
               bottom line). Legendary and mythical Moemon only come as UR,
               their shiny forms only as EX. Event Moemon carry the moves of
               the real events (WISHMKR Jirachi, AURA Mew, AGETO Celebi,
               SPACE C Deoxys, the ticket legends, the Pokemon Box eggs).
               Pity, kept per banner: an SR or better at least every 40
               pulls, an SSR or better every 100, a UR or better every 200.
               EX has no pity: pure luck. A 10-pull always has an R or
               better.
               FREE PULLS (offline too, and they pile up if you don't use
               them): a free 10-pull on STANDARD every week, and a free
               pull on every banner each day you play (FREE x10 / FREE x1
               in the menu).
               MOEGATCHA TICKETS: one pull on any banner (TICKET in the
               menu; they pile up too). People who give you an important
               item in the story (key items, HMs, the gym leaders' TMs, the
               EXP. SHARE) hand you a ticket with it, once per item. The
               Mauville GAME CORNER's prize counter (the doll clerk) sells
               them for 2,500 COINS. A save from before tickets existed gets
               the ones it would have had, once, for the story items it
               already got. Winning a BATTLE TENT challenge (three wins in
               a row) gives 3 with the prize.
               Legendary prizes obey you like traded Moemon: by your badges.
               SR and up get a show that grows with the rarity: the "?" box
               climbs through the colors. A legendary turns the screen dark,
               its silhouette rises to battle music, then it's revealed with
               its cry. EX cracks the screen open. Friends online get a
               message when you pull a UR or EX.
               The game saves by itself after every pull.
Didn't finish the goals or win the race? You get a consolation prize picked
for what you're short of: Rare Candy if your team is behind, healing items if
you're low, Poke Balls if you're out.
Days change at midnight UTC so friends in different countries share them.


HANGING OUT (CO-OP menu > SOCIAL)
------------------------------------
EMOTES & PHRASES  Opens the emote / phrase pick around you (same as Q / E or
                  L2 / R2, see the controls). Keys 1-4 send emotes and keys
                  5-8 your first four phrases straight away.
                  Do the same emote as a friend close by at the same moment
                  for a COMBO (DOUBLE SHOCK!!, TRUE FRIENDS!, DUET!...).
CHAT LOG          Everything said and emoted, and the news pop-ups.
PINGS             V (or left stick click) puts "NAME: Look here!" over the
                  tile in front of you: an item, a person, a path. Friends on
                  the same map see it for a few seconds, with a blinking arrow
                  at the edge of the screen while it's out of view.
SCRIBBLE          Stand facing a sign and leave one of your phrases (or a
                  ready-made one like "Over here!") on it.
                  Friends see it after the sign's own text.
PRANK A FRIEND    Costs a MOEGATCHA pull: Magikarp follower (1 min),
                  backwards controls (15 sec), confetti cannon, whoopee
                  cushion. The victim gets a free REVENGE prank for 2 minutes.
                  Nobody can be pranked twice in a row, or during a battle.
LEND FIELD MOVES  No CUT in your party? If a friend standing within 4 steps
                  has a Moemon that knows CUT, SURF, STRENGTH, ROCK SMASH,
                  WATERFALL or DIVE, the tree, rock, boulder or water works
                  with theirs ("ALICE's PELIPPER used SURF!") and they get a
                  note. You still need the badge for the move. A Moemon
                  holding the HM counts too (see HMs).
RESCUE            Lose a battle with a friend within 10 steps and you don't
                  black out right away: they're asked to help. A REVIVE from
                  their bag brings your first Moemon back at half HP, a MAX
                  REVIVE at full HP. While they keep watch you can also use
                  a REVIVE from your own bag (A). Paying to heal your whole
                  team on the spot is dear on purpose ($150 per Moemon plus
                  $4 per missing HP, times 1 + half your badges), and in caves
                  and dungeons there's no paying at all: only REVIVEs help, so
                  stock up before you go in. If nobody helps within 15
                  seconds, or you give up (B), you black out as usual.
TEAM PHOTO        With friends on the same map: everyone's trainer picture
                  lines up on a stage with your names, the place and the date,
                  framed like an instant photo. It's saved as a PNG in the
                  "photos" folder next to the game, and the friends in it
                  get a note.
PIGGYBACK         Face a friend and press A: hop on their back and ride along
                  wherever they go (SURF, doors and FLY too). B hops off.
TOGETHER          Stay within about a screen of a friend and the TOGETHER
                  meter fills: Lv1 EXP +3%, Lv2 EXP +6% and better shiny odds,
                  Lv3 EXP +10%. A bit more with more friends close, never past
                  +14%. Shown at the bottom of the CO-OP menu.
CATCH-UP          Fewer badges than a friend online? +10% EXP per badge behind
                  (up to +30%).
WHERE IS EVERYONE Colored arrows at the screen edge point to friends you can't
                  see (red, blue, green, yellow by player).
BETTING           When you watch a friend's battle you can bet $500 on them to
                  win or lose. Right: double back. Leave early: bet lost.


THE GAME'S OWN LINK FEATURES, ONLINE
------------------------------------
RECORD MIXING   Happens by itself while friends are online (when someone
                joins, then every 20 minutes): TV shows, SECRET BASES, the
                Old Man's songs, Dewford trends, Battle Tower trainers and
                the rest, just like mixing records at a MOEMON CENTER.
                (SECRET BASES only arrive once you have SECRET POWER, like in
                the original.)
DEWFORD TREND   Online, a phrase anyone tells the boy outside Dewford Hall
                is the new trend right away, in everyone's game (alone, the
                original rules apply). Broken phrases from old saves (shown
                as ??? ???) are replaced by new ones and never mixed in.
SECRET BASES    CO-OP menu > FRIENDS > pick a friend > VISIT SECRET BASE walks
                you straight into their base, decorations and all (you can
                battle its owner). Leaving puts you outside it, on its route.
TRAINER CARDS   CO-OP menu > FRIENDS > pick a friend > TRAINER CARD (both
                sides).
DAY CARE        A MOEMON left alone at your DAY CARE can make an EGG with one
                left alone at a friend's. Each of you gets your own EGG from
                the DAY CARE MAN.
ROAMERS         Latias/Latios roam the same routes for everyone, so you can
                hunt them together.
WEATHER         Routes 119 and 123's weather, and the abnormal weather after
                the game, are the same for everyone.
HALL OF FAME    Entering the HALL OF FAME shows the teams of the friends online
                after yours, and they stand with you at the end. They get a
                MOEGATCHA pull for cheering (the first time each friend makes it).
BERRY BLENDER   CO-OP menu > BERRY GAMES > BERRY BLENDER asks every friend online. Everyone
                who joins picks a BERRY from their own BAG and you blend
                together, wherever you are (friends take the other seats).
                Everyone gets the same POKEBLOCK. You need a POKEBLOCK CASE.
BERRY CRUSH     BERRY GAMES > BERRY CRUSH: mash A together to crush BERRIES into
                BERRY POWDER. You need the POWDER JAR. You can play again
                if everyone says YES.
CONTESTS        Go to the LINK CONTEST counter in Lilycove's CONTEST HALL (the
                right receptionist) and enter the same contest as your friends.
                While "Please wait" is up, the game waits for friends online
                who picked the same contest, then you all go on stage
                together (the game fills the empty spots, as in the original).
                It starts once every friend online is ready, or 10 seconds
                after the last one got ready. B cancels.


HANDY EXTRAS (they save time, they don't make the game easier)
---------------------------------------------------------------
* Battle: on the move list, R shows each move's power, accuracy and kind
  (PHYS./SPEC./STATUS). At FIGHT/BAG, L throws the same kind of Ball as last
  time (wild battles). In a co-op battle, R shows your partner's team.
* Battles are quicker: messages print twice as fast as your text speed
  setting, stay up a shorter time, and A or B moves on to the next one.
  The low HP alarm beeps three times and then stops. Sending Moemon out and
  the fade at the end of a battle are 30% quicker, and held items like
  LEFTOVERS (and Ingrain, Wish, Leech Seed) show a short, quick animation.
  Move animations play 30% faster too (the host's ANIM SPEED: NORMAL,
  +25% or +30%; playing solo, the one in your own HOST A GAME screen).
* Summary screen: the stat your Moemon's nature raises is red, the one it
  lowers is blue. SELECT on the first page renames a Moemon
  (traded ones too; the NAME RATER in SLATEPORT renames them as well).
* POKeMON STADIUM COLORS: a Moemon with a nickname gets its own colors,
  worked out the way POKeMON STADIUM does it: its nickname, its OT's name
  and the Trainer ID pick a hue within what suits its species. The same
  nickname always gives the same colors, and everyone else sees them too.
  Faces, arms and legs keep their skin tones (with OPTIONS > CREATURES on
  POKeMON the whole picture turns). Shows in battle, on the summary screen,
  in the PC and party, on its icon and when it follows you. Without a
  nickname (or with its species' name) it keeps its usual colors.
  The fifth page (POKé BALL) shows its ball: press A to move it to any kind
  of ball in your bag (uses one), or to recolor a basic POKé BALL (red,
  blue, green, yellow, pink, purple, orange, cyan, black, silver, gold, and
  30 more colors to earn, from CRIMSON to BRONZE).
  The color shows when it's sent out in battle.
* BALL EFFECTS (SELECT on the POKé BALL page): how each Moemon comes out of
  its ball and goes back. Pick a ready-made mix (HAUNTED, SUPERSTAR,
  FIRECRACKER, TEAM ROCKET'S BUDGET, GLITCH, BLOOM, CLASSIC ANIME, ROYALTY,
  STORM, DEEP SEA, SHOOTING STAR, and 15 more once you have their pieces:
  SWEETHEART, POP STAR, WINTER, WILDFIRE, HIGH TIDE, FOREST SPIRIT, HANAMI,
  JACKPOT, PARTY TIME, SHADOW REALM, CYBERSPACE, GALAXY, DESERT WIND,
  DAYDREAM, ANGEL) or set each part yourself:
    MATERIALIZATION     how its body forms: anime beam, smoke, bubble,
                        lightning, haunted fade, digital, ink, crystal,
                        meteor drop, pop-in, rise up (or the game's own);
                        to earn: teleport, somersault, vortex, dash in,
                        jelly drop, card flip, hologram, stardust, sand
                        sculpt, flame pillar, geyser, leaf whirl, shadow
                        step, prism, bounce in
    ENERGY COLOR        the color of beams, glows, motes, pixels and ink
                        (11 colors, and the same 30 to earn as the ball)
    ENTRANCE PARTICLES  confetti, explosion, smoke puff, fireworks, flower
                        bloom, bubbles, lightning bolt, wisps + eyes, pixel
                        bits, ink splash, crystal shards, spotlight, dust
                        ring, royal sparkles (or none, or the game's own);
                        to earn: hearts, music notes, snowfall, feathers,
                        leaf gust, star shower, rainbow, fire ring, splash,
                        sakura storm, pay day, balloons, butterflies,
                        gemstones, power aura
    RECALL EFFECT       classic beam, dissolve, reverse confetti, smoke
                        retreat, digital shutdown, curtain call; to earn:
                        teleport out, backflip, vortex, dash off, melt away,
                        card flip, switch off, stardust, sand crumble, flame
                        out, geyser lift, leaf whirl, shadow sink, prism
                        twinkle, bounce back, heart pop, bubble float,
                        lightning strike, ice shatter, balloon ride, super
                        jump, dive, feather scatter, black hole, ghost fade,
                        pixel rain, firework, nap time, sakura scatter,
                        spirit orb
    SOUND ACCENT        auto, none, chime, pop, boom, fanfare, thunder, spooky
                        (AUTO also gives many of the newer recalls a sound)
    SAVE AS             keeps your mix as MY PRESET 1 to 6 ({LEFTRIGHT} picks,
                        A saves). PRESET then offers it for any Moemon.
    APPLY TO PARTY      gives the mix to every Moemon in your party (A)
  The preview shows it on a battle field: the entrance, a short pause, then
  the recall. SELECT shows it from the other side, START saves, B goes back.
  The pieces "to earn" and the 30 newer colors show up as you get them:
  half come with achievements (CO-OP > DAILY > ACHIEVEMENTS lists what each
  one gives, and an achievement you already have counts), half are
  MOEGATCHA prizes (mostly on the POKE BALLS banner; one you have already
  turns into a MOEGATCHA TICKET). The effect stays with the Moemon (trades
  too) and every player sees it in link battles, co-op battles and PvP,
  whatever they have earned. It never makes sending out or calling back
  take longer than the game's own. Your follower pops out of its ball with
  it too (and goes back with its recall), for everyone around.
* Special POKé BALLS (only from the MoeGatcha POKE BALLS banner):
    LOVE BALL   x8 on your Moemon's species, opposite gender
    FRIEND BALL the Moemon caught is friendly right away
    FAST BALL   x4 on Moemon with base Speed 100+
    LEVEL BALL  x2 / x4 / x8 when your Moemon is over 1x / 2x / 4x its level
    LURE BALL   x4 on Moemon hooked with a ROD
    HEAVY BALL  better on heavy Moemon (+20 / +30), worse under 100 kg
    MOON BALL   x4 on Moemon that evolve with a MOON STONE
    DREAM BALL  x4 on sleeping Moemon
    HEAL BALL   heals the Moemon caught
    DUSK BALL   x3 in caves or at night (8 PM - 6 AM)
    QUICK BALL  x5 on the first turn
  They can't be recolored or decorated. Other games see a Moemon caught in
  one as being in a POKE BALL; this game shows the special ball.
* GENESIS BALL and EPOCH BALL (MoeGatcha POKE BALLS banner): the POKE BALLS
  of Pokemon Red and Pokemon Crystal, catch rules and all.
    GENESIS BALL  Red's catch: sleep or freeze can catch at once, poison,
                  burn or paralysis help too; at about a third of its HP or
                  less a Moemon that passes the first check is caught. The
                  ball can miss (a failed throw shown Red's way). Red's own
                  messages, throw, shakes and sounds.
    EPOCH BALL    Crystal's catch, with its two known bugs fixed (poison,
                  burn and paralysis now help; Moemon with lots of HP no
                  longer break the math). Success is decided first; a
                  failed ball wobbles 0-3 times. Crystal's own messages,
                  throw, wobbles and sounds.
  A Moemon caught in one comes out and goes back the way it did in that game
  (smoke, then it grows in; it shrinks away), unless you give it a ball
  effect of your own. Battle speed settings never change the odds. In co-op
  the game running the battle decides the throw once and everyone sees the
  same.
* e-Reader BERRIES: the twelve Berries Japan got from e-Reader cards for
  Ruby and Sapphire are here as their own items (BERRIES No.47-58), so you
  can keep them all: PUMKIN (thaws a frozen Moemon), DRASH (cures poison),
  EGGANT (cures infatuation), STRIB, CHILAN, NUTPEA, GINEMA (restores
  lowered stats), KUO, YAGO (heals a burn), TOUGA (cures confusion), NINIKU
  and TOPO. Their flavors, firmness and held effects are the cards'; the
  ones with no effect are for POKeBLOCKS. Plant them, crush or blend them
  like any Berry. They only come from the MoeGatcha (STANDARD, SSR), and an
  unmodded game can't take them in a trade.
* Items from later games (from the MoeGatcha STANDARD banner):
  QUICK POWDER    held by DITTO: doubles its Speed (not while it's
                  transformed).
  EVIOLITE        held by a Moemon that can still evolve: DEFENSE and
                  SP. DEF x1.5.
  Treats          OLD GATEAU, CASTELIACONE, LUMIOSEGALETE (Lumiose Galette),
                  SHALOUR SABLE, BIG MALASADA, PEWTERCRUNCHY (Pewter
                  Crunchies), RAGECANDYBAR: heal all status problems and
                  confusion, from the BAG or in battle, like a LAVA COOKIE.
* Bag: START sorts the pocket you're in: TYPE (medicine, things used on a
  Moemon, battle items, field items, hold items, mail, the rest), ID, ABC,
  OLDEST or NEWEST (when you got it), or MANUAL (your own order: move items
  with SELECT; moving one switches the pocket to MANUAL). Each pocket keeps
  its order, shown under {START} at the left. TMs and Berries are by number
  until you pick something else. Items you just got have a "!" before their
  name until the cursor lands on them.
* Bag: KEY ITEMS > REGISTER asks which button the item goes on: SELECT, L,
  R, or the extra item button (X/Square on a controller, F on the keyboard).
  Pick the button it's already on (OFF) to take it off. Pressing L or R on a
  key item in the list also puts it on that button. On the map those buttons
  use the item. (L and R need BUTTON MODE NORMAL in OPTION.)
* PC boxes, MOVE POKeMON: TAG (X/Square on a controller, F on the keyboard)
  marks the Moemon under the cursor; tag as many as you like in one box, or
  in your party (tagging somewhere else drops the old tags, B drops them
  all). Then A: MOVE, RELEASE or CANCEL.
    MOVE      picks them all up, keeping how they were arranged (party
              Moemon in a row). A puts them down: as arranged if they fit
              there, otherwise in the empty slots from the cursor on; a box
              without room for all of them says no (a buzz). R, or DOWN to
              PARTY POKeMON and A, puts them in your party when it has room.
    RELEASE   "Release all selected MOEMON?" Not for EGGS or Moemon holding
              MAIL, and you keep a Moemon that can battle in your party and
              at least two Moemon in all. Held HMs go back to the BAG.
* CANDIES: every Moemon you release leaves a candy of its type (its first
  type). After your first release, PROF. BIRCH walks over with the CANDY
  JAR (a KEY ITEM) as soon as you're free on the map; from then on candies
  go into it by themselves. Use the jar from the BAG or a registered
  button: the lid pops off and your candies pour in, a candy for each one
  (past 96 the jar is full and the candies get smaller, each standing for
  several); they settle in layers, kind by kind. The list shows each kind
  and its count: a blue count has enough for a RARE CANDY. Move the
  cursor to a kind (its candies hop in the jar) and the message box says
  how many more it needs; A on a blue one asks, YES turns every ten of it
  into a RARE CANDY: they fly out of the jar and the RARE CANDY goes to
  your BAG. ALL (bottom right) converts every kind at once. SELECT shakes
  the jar. B: back. The candies are saved with your game.
* Hold SELECT on the map to pick which Moemon follows you (a quick tap of
  SELECT still uses your registered item).
* If the game crashes it saves crash_log.txt next to the game and says so.
  Please send that file (and what you were doing) to the developer.
* AUTOSAVE: the game saves itself quietly every 5 minutes on the map (and a
  minute after a battle), the moment you're standing free. Not where the START
  menu has no SAVE (SAFARI ZONE, BATTLE PIKE / PYRAMID, link rooms), and a new
  game started over an older save waits until you save it by hand once.
  moemon_coop.ini: autosave = minutes (0 = off). Older saves are kept as
  <save>.bak1-3.
* A bigger BAG: up to 200 kinds of items, 64 kinds of POKE BALLS, key items,
  TMs/HMs and BERRIES, and up to 999 of each. The save file stays a normal
  Emerald save (its own BAG is the first slots); the rest is kept next to it
  in <save>.bag, written with every save (the .bak files keep theirs as
  .bak1.bag and so on, so a .bak renamed back still has the whole BAG; a
  save played in another program since keeps only the normal BAG).
* A shared world: a tree a friend CUT or a rock they smashed is gone for
  everyone on that map, a STRENGTH boulder moves for everyone, CUT grass
  is cut for everyone, and ashy grass a friend walked through on Route 113
  is clear for you too (the soot is theirs). As in the game, it all comes
  back once everyone has left the map; a friend who walks in while one of
  you is still there sees it as you do.
* Dark caves: every friend carries their own light, and a friend who used
  FLASH lights up the cave around them on your screen too. Co-op notes and
  speech bubbles stay readable in the dark.
* Gender ratios are the original games' again (Moemon Kai had made most
  Moemon female): Torchic is mostly male, Zigzagoon half and half...
* Weather in battle: SANDSTORM and HAIL hit everyone at the same time with
  one line ("Everyone was buffeted by the sandstorm!"); only one Pokemon
  hit is named as before. "The sunlight is strong.", "Rain continues to
  fall." and the like, and the "Everyone..." line, show the first time only
  until that weather ends.
* When a REPEL wears off you're asked if you want to use another one.
* PC: R on a Moemon in a box takes it into your party; R on a party Moemon
  stores it (you pick the box).
* CO-OP menu > MOVE TUTOR: the Move Tutor's service anywhere, still one
  HEART SCALE per move.
* Item descriptions show in the shared PC and in trades.
* Signs and lines you've already read appear instantly. Hold START during a
  scene or conversation you've already seen to play it 4x faster (questions
  and menus still wait for you).
* Coming online shows what your team did while you were away. Tournament
  matches show how healthy your team is before you say READY.


MOEMON THAT FOLLOW YOU
-----------------------
Open the MOEMON menu, pick a Moemon with the cursor and press SELECT:
it walks behind you. Press SELECT on it again to put it back in its Ball.
All 386 Moemon have their own walking sprites (with female and form
differences). Talk to it (A) and it's happy to see you. Other players see
your follower walking behind you too. It hides while you surf or dive and
comes back when you land. It walks the ground as you do: tall grass shakes
around it, it leaves footprints in sand, splashes through puddles and
shallow water, sinks into hot springs, and shows in ponds and on ice. The choice is remembered in moemon_coop.ini.
Walking together makes it happier: choosing it gives a little friendship
(once a day per Moemon), and it gains more than the rest of the party as
you walk.


TMs
---
  TMs never wear out, as in later games: teach a TM's move as often as
  you like, it stays in your BAG. A POKé MART sells each TM once ("You
  already have that TM."). If your save used TMs up before 1.18.1, the
  LOTTERY CORNER clerk from Lilycove walks up to you once with your
  "lottery winnings": every TM your save got from a person, a gym or an
  item ball and no longer has. (TMs bought or won at a counter left no
  trace in the save, so those can't come back.)

HMs
---
  No HM restrictions: an HM move can be forgotten like any other move
  (a TM can take its place, a new move at a level up, the MOVE DELETER),
  and you can release a Moemon that knows SURF or DIVE.
  A Moemon can HOLD an HM (BAG > TMs & HMs > GIVE, or its ITEM > GIVE):
  it can then use that HM's move outside battle as if it knew it (its
  party menu lists it, and trees, rocks, water and boulders ask as usual;
  you still need the badge). In battle a held HM does nothing. Take it
  back any time with ITEM > TAKE. A held HM never leaves you: when you
  release, trade or store in the shared PC a Moemon holding one, the HM
  goes back to your BAG first.


DRESSING UP (CO-OP menu > LOOK)
---------------------------------
  GROUP    which sprites SPRITE goes through: NAMED NPC (people with a
           name: DEFAULT, Brendan, May, then over a hundred characters from
           every generation - Ethan, Red, Cynthia, Steven, Iono, Leon... -
           then Emerald's and FireRed/LeafGreen's own named people), GENERICS
           (classes and townsfolk, grunts included) or CUSTOM (CREATE 1-3 and
           your own sprite sheets). Left/Right; each group remembers the
           sprite you last wore in it.
  SPRITE   a sprite of that group (Left/Right, L/R jump 10). CUSTOM 1-3 are
           the trainers you make in CREATE (one not made yet looks like
           DEFAULT). It's used
           everywhere: walking, running, biking, surfing, diving, fishing,
           field moves. The named characters have their own running, Mach
           Bike, Acro Bike and diving sprites. For the actions a sprite has
           no frames of its own (surfing, fishing, watering, field moves,
           decorating, and the bikes of the others) it
           wears Brendan's or May's body for that action, in its own
           clothes' colors, with its own head on top. CREATE trainers have
           their own Mach Bike too. Pokemon sprites (Mew, Wingull...) ride
           and surf as themselves. Other sprites run in their walking frames.
  BIKE     only for a character who rides a Pokemon or a vehicle instead of
           a bike (Blue, Serena on her Rhyhorn, Prof. Birch running from a
           Zigzagoon...): RIDE shows it as drawn, BIKE a bicycle.
           Scenes that show you as someone else (Battle Tent and Battle
           Frontier walk-ins, the Contest Hall, interviews) use your look too.
  ID PIC   the picture on your TRAINER CARD and in battles. MATCH (the
           default) uses the one that goes with your sprite; or pick any
           other: Emerald's, FireRed/LeafGreen's, the extra pictures from
           Pokemon Showdown, or the characters' pictures (and HeartGold/
           SoulSilver's Archer, Bugsy, Karen...) at the end (L/R jump 10).
  BK PIC   your back in battles: MATCH, Brendan, May, Red, Leaf, Wally,
           Steven, the Old Man, the Poke Dude, the Platinum and Diamond/
           Pearl ones (Lucas, Dawn, Barry, Cheryl, Riley, Marley, Buck, Mira),
           Gold, Kris, a Gold/Silver boy, every named character's own back,
           or NONE (hidden).
  SKIN / HAIR / OUTFIT / ACCENT   colors. They change your sprite and the
           ID and back pictures together (the characters' too). (The extra Showdown pictures and
           your own drawn sprites keep their own colors.)
  CREATE...   make your own trainer from parts (see below).
  START       ID PIC and BK PIC go back to MATCH (on one of those rows, just
              that one). If the sprite has no matching picture, ID PIC becomes
              DEFAULT (Brendan/May) and BK PIC becomes NONE.
  SAVE LOOK   keep the look and go back (B does the same).
  REVERT      undo every change made since you opened LOOK.
  SELECT on a row puts just that one back the way it was.
The picture next to the menu shows your ID PIC (or BK PIC on that row) as
you change it. Everyone sees your look right away.

CREATE (LOOK > CREATE...) builds a trainer from parts: BODY (boy or girl),
SKIN, HAIR, TOP, BOTTOM, HAT and BAG. Left/Right picks the style, L/R the
color. The field sprite and the picture change as you go; START flips the
picture between the front (ID PIC) and the back (BK PIC). SELECT puts a row
back. SLOT picks CUSTOM 1, 2 or 3; SAVE keeps it there and puts it on you,
CANCEL (or B) goes back to the look you had. Other players see it too. Your
three trainers are kept in the co-op settings file (custom1-3).
The field sprite (walking, running, Mach Bike) is drawn in Gen 3 style to
match the picture: hair, hats, shirt and pants from a Gen 3 trainer kit, other
clothes and bags from official FireRed/LeafGreen sprites (skirts, shorts, long
coat, overalls, swimwear, Red's and Leaf's bags, the Hiker's pack), small head
accessories (crown, glasses, cat ears...) from the picture's own parts, and
skin in exact Gen 3 colors. FIELD picks its hairstyle: AUTO matches HAIR, or
pick any of 144 hairstyles (L/R jumps 5): the 30 from the Gen 3 kit, then 114
named after the characters they come from (VALERIE, MARNIE, HUGH...). They all
take your HAIR color; ribbons and clips take your top's colors.

Characters without a battle back sprite: in battle your trainer isn't shown
(your Moemon still are), unless you pick a BK PIC. You can also draw one in
the Sprite Editor.

OPTIONS > FRAME also has the FireRed/LeafGreen window frames (21-29).
OPTIONS > TYPE ICONS ON shows each Moemon's types as small colored icons
next to its box in every battle (Single, Double and Triple): the second type
behind, the first type in front of it. OFF (the default) leaves the battle
screen as it always was.

Extra trainer pictures come from play.pokemonshowdown.com/sprites/trainers.
The Platinum, Diamond/Pearl and Gold/Silver/Crystal back pictures were ripped
by MufasaKong. CREATE uses a fan-made trainer customization parts kit for the
pictures and a Gen 3 trainer kit (made for Top Down Sprite Maker) for the
field sprites. The extra field hairstyles are cut from aveontrainer's
FRLG-style overworld sprites (deviantart.com/aveontrainer, gallery OVERWORLD),
used with credit for this non-commercial project as the artist allows.
A few are fan art, used as drawn: BATTLE GIRL G6, EXPERT F G6, PRESCHOOLER
G6, SCHOOL KID G8 and SWIMMER G6 by kyledove, KRIS by Selena, YOUNGSTER G7
by Beliot419.


SPRITE EDITOR
-------------
CO-OP menu > EDIT SPRITES... saves your current character as a sprite
sheet in the "sprites" folder and opens "Sprite Editor.html" in your web
browser (it works offline).

In the editor you can:
  * open any sprite sheet (or drag it onto the page),
  * pick the character it's based on (this is how you edit an existing
    trainer or NPC, e.g. give the Hiker a battle back sprite),
  * paint every animation: walking, running, Mach Bike, Acro Bike, surfing,
    diving, fishing, field moves (using items/Moemon), watering,
    decorating, the TRAINER CARD picture and the battle back sprite,
  * import a picture into any frame, copy/paste/flip frames, undo/redo,
  * preview the walk cycle, and keep each group within 15 colors.
Save the PNG, then drag it onto the game window. You wear it immediately.

Your edited sprites are sent to the other players with your character, so
everyone sees YOUR version. Two players can edit the same trainer
differently; nobody's version overwrites anyone else's.

Older 144x32 / 288x32 walking sheets still work.


OPTIONS (START > OPTIONS, or OPTIONS on the title screen)
---------------------------------------------------------
Every setting in one place, in four tabs: L / R switches tabs, Up / Down picks a
row, Left / Right (or A) changes it, B or CLOSE leaves. The line at the bottom
says what the row does.
GAME      TEXT SPEED, BATTLE SCENE, BATTLE STYLE, SOUND, BUTTON MODE, FRAME,
          TYPE ICONS, NAME CASE, CREATURES (as before), and
  BIKE MUSIC    ON: the cycling song on a bike. OFF: the map's own song.
  SURF MUSIC    ON: the surfing and underwater songs. OFF: surfing keeps the
                map's song, and diving keeps the song of the sea above.
DISPLAY
  FULLSCREEN    borderless fullscreen (also F11 or Alt+Enter)
  WINDOW SCALE  1x to 10x
  SCALING       PIXEL PERFECT (whole-number zoom, sharpest),
                SHARP FIT (fills the screen, still crisp),
                STRETCH (fills the screen exactly)
  FILTER        OFF, SCANLINES, LCD GRID
  VSYNC         on/off
CONTROLS
  CONTROLLER    which controller plays. Press A on this row, then press
                any button on the controller you want; that one is used
                from then on (it's remembered). Left/Right goes back to
                ALL CONTROLLERS.
  PAD BUTTONS   AUTO, A = RIGHT (Nintendo style) or A = BOTTOM (Xbox style)
  STICK RUN     push the stick all the way to run (on/off; off in a new
                game)
  RUMBLE        on/off
  BUTTON ICONS  the button pictures in help lines: AUTO (the keyboard or
                controller you used last), or always KEYBOARD, XBOX,
                PLAYSTATION or NINTENDO
CO-OP
  FRIENDS' SCREENS and a row for each friend online: see FRIENDS' SCREENS.
  NEW FRIENDS   the screen size for friends you haven't set up yet
  AUTOSAVE      a quiet save on the map every 5 to 60 minutes, or OFF
  FLAG RANGE    how many tiles you see with the BATTLE FLAG planted (1-8)
  MOENAV CALLS  ALL, or BATTLES: trainers only call when they want a battle
                (no chatting). Story calls always come.
The window can also be resized freely. Settings are kept in
moemon_display.ini, moemon_controls.ini and moemon_coop.ini.


CONTROLS (keyboard)
-------------------
  Arrow keys = D-Pad      Z = A      X = B      Enter = START
  Backslash (\) = SELECT  A = L      S = R
  C = CO-OP menu          1-8 = emotes and your phrases
  V = "look here!" ping   F = the extra registered key item (BAG > REGISTER);
                          in the PC boxes, TAG
  Q (hold) = quick emotes   E (hold) = quick phrases
            (pick with the arrows, let go to send; A / S flip pages)
  Ctrl+P = pause          F11 / Alt+Enter = fullscreen


CONTROLS (controller)
---------------------
Plug in any controller (Xbox, PlayStation, Switch Pro, 8BitDo and most
others) and it just works, even mid-game. The buttons are set up for you:

  D-Pad / left stick     move              Start / Options / +   START
  A / B                  A / B (*)         Back / Share / -      SELECT
  LB / RB                L / R

  (*) Nintendo-style pads use the buttons labeled A and B. Xbox and
      PlayStation pads use the bottom button for A and the right one for B.

The other buttons change with what's on screen:
  Walking around    X/Square = the extra registered key item (BAG >
                    REGISTER), stick pushed all the way = run (with
                    STICK RUN on), Y/Triangle = CO-OP menu, left stick click =
                    "look here!" ping on the tile in front of you
                    L2/LT (hold) = quick emotes, R2/RT (hold) = quick
                    phrases. They appear around you; push a direction,
                    then let go to send it (A sends right away, B cancels).
                    L2 pages (LB/RB): EMOTES (! heart ? music), MOODS
                    (angry, sweat, sleepy, laugh), REACT (idea, sparkle,
                    sad, ...) and HYPE (fire, crown, POKe BALL, skull).
                    R2 pages (LB/RB): MINE (up to 32 of your own phrases,
                    4 per page; MINE 2 to 8 show up as you fill the
                    page before) and ready-made ones (GREET, MOVE, BATTLE,
                    MORE). An empty slot on the first MINE page offers a
                    ready-made phrase: X/Square swaps it, or pick one on
                    another page and press X/Square to pin it there.
                    Y/Triangle on a MINE page edits that phrase.
  Text and menus    LT (hold) = keep pressing A, to get through text fast;
                    X/Square = B, Y/Triangle = SELECT
  Naming screen     X/Square = delete, Y/Triangle = switch letters
  PC boxes          X/Square = TAG (F on the keyboard), Y/Triangle = SELECT
Button hints on screen show your controller's own buttons (Xbox,
PlayStation or Nintendo) when you last used it, and the GBA buttons when
you last used the keyboard.
Controllers rumble when a trainer spots you, when a battle starts and when
a friend asks you to team up.


POKEDEX ENTRIES
---------------
A MOEMON's page in the POKEDEX tells you everything RUBY, SAPPHIRE,
EMERALD and FIRERED had to say about it, as one long entry (an entry
that says exactly what an earlier game said is shown only once). The page
shows the beginning; a small arrow means there's more.
  TEXT (the first button at the top, where the cursor starts)  A = read
  While reading   UP / DOWN = scroll, B = done
After reading, UP / DOWN go to the next MOEMON again. AREA, CRY and SIZE
work as before (move right to them). A MOEMON you've only seen has no
entry yet. The page that appears after a catch keeps EMERALD's entry.

MOEMON GARDEN
-------------
Every MOEMON has a life of its own: a temperament (bold or timid, warm
or aloof, lively, curious, social, proud), needs, feelings and memories
of what it lived through with you. Nothing is ever lost while you're
away; a MOEMON doesn't suffer or sulk because you didn't play.
  On the map    Stand still and your follower does its own thing: looks
                around, sniffs at things, naps when tired, splashes in
                water, basks, sits by you, or invites you to play (a ball
                bubble). Once in a while a curious one finds something
                hidden. Walk on and it hurries back.
  Bubbles       Round pink bubbles over a MOEMON are its own feelings
                (the square ones are players' emotes).
  A             In front of your follower: the fitting thing to do
                (comfort it, play, praise it, or just talk).
  Hold A        A wheel: PET, PLAY, FEED, TALK / PRAISE, COMFORT, GROOM,
                SIT WITH / WATCH, KEEPSAKE. Let go of A on your choice.
                (Watching it now and then teaches you something new
                about it, at most every half hour of play.)
                Each MOEMON likes some things more than others, and some
                would rather not be petted; it'll just step aside.
  JOURNAL       A page in its summary (after the POKE BALL page): what
                you've noticed about it so far. A = next page.
Big moments together (a badge, a hungry MOEMON fed, comfort when it was
scared) can raise its friendship a little, once. A MOEMON that wants to
come out wiggles on the party screen, and the hold-SELECT follower
picker says WANTS TO COME.

In battle, a MOEMON remembers what stands out: hanging on at the last
moment, the knockout that won it, a comeback, a friend who stepped in.
Round pink bubbles on its sprite (with a soft sound) show how it feels;
the square white ones are players'. Very rarely, after the biggest
moments, a pink line in the message box tells you what it's feeling.
With friends, MOEMON get to know each other and you: followers meet
when you stand together, friends can pet or feed yours (from their own
bag), and a MOEMON's life story travels with it in a trade.

THE GARDEN: every POKEMON CENTER's second floor. Ride the escalator up
and you're in a meadow inside the CENTER, where your whole party lives
freely: napping, playing tag, splashing at the pond, keeping you
company. The escalator right where you arrive takes you back down to
the CENTER you came from. After your first visit, START > GARDEN (and
your PC's MOEMON GARDEN row) takes you there from outdoors or a CENTER;
the way down then leads to the last POKEMON CENTER you visited. Not
from caves, dungeons, the SAFARI ZONE or the middle of a story moment:
a message says so.
Every type has a place of its own there, where its MOEMON go for what
they love: a cozy nook (NORMAL), a sand training ground (FIGHTING), the
open meadow (FLYING), a puddle (POISON), soft sand to dig (GROUND), a
plateau of boulders (ROCK) with a lookout over its ledge (DRAGON), tall
grass (BUG), a dark hollow in the trees (GHOST, DARK), shiny stones
(STEEL), a hot spring you can walk into (FIRE), the pond (WATER), the
flower bed (GRASS), the storm stone (ELECTRIC), a standing stone in a
ring of flowers (PSYCHIC) and a shaded spring with frost crystals (ICE).
Every badge your group holds adds a place: 1 a long grass field, 2 a
marsh, 3 a boulder field, 4 a dig field, 5 more shiny stones, 6 a perch
grove, 7 a second flower bed, 8 a stone circle. With friends, the Garden
shows the most badges among you.
  GARDEN BOARD  DECORATE: put dolls and cushions from your SECRET BASE
                collection out for them, as you decorate a SECRET BASE:
                DECORATE > DOLL or CUSHION > pick one, move it over
                the Garden with the + Control Pad, A: "Place it here?".
                PUT AWAY: move the cursor onto one, A. CANCEL ends.
                WATCH: sit down and watch; A tells you what's going on,
                B stands up.
With friends there, everyone's MOEMON and decorations are there
together (whichever CENTER each of you came up in).

Over time, a MOEMON may take up something it loves in the Garden:
climbing the trees, splashing in the pond, singing, collecting pretty
stones, digging tunnels or showing the younger ones. It gets better at
it, and the JOURNAL celebrates the big steps. Once in a long while, when the
right MOEMON, the time of day and the weather line up, something rare
happens; the JOURNAL keeps the ones you saw. A MOEMON hatched from an
EGG knows its parents and may take after them a little. After it
evolves, it takes a while to get used to its new body, and its friends
come to see.

UPDATES
-------
When the game starts it checks for a newer version (online, at the
game's GitHub page). If there is one, it shows "UPDATING TO VERSION ...",
downloads it, checks that the download is complete and undamaged,
installs it into this folder and starts the new version by itself.

* Your save (pokeemerald.sav and its backups), your .ini settings and
  your sprites are never touched. Only the game's own files are replaced.
* ESC while it downloads: play this version now; it asks again next time.
* No internet, or anything goes wrong: the game simply starts as it is
  (and says why if the update itself failed). Nothing is left half-done:
  a failed install puts every file back.
* Turn it off with "AutoUpdate = 0" in moemon_coop.ini.
* Play online with friends on the same version: if one of you updated,
  the others get the update the next time they start the game.
* It's an ordinary download, like a browser's: nothing about the firewall
  or Windows settings is changed.
* On Linux it needs libcurl (installed on nearly every system).


USING YOUR EXISTING SAVE
------------------------
The game saves to "pokeemerald.sav" next to the .exe. It's the normal
128 KB Emerald flash save, so a Moemon Emerald Kai save from mGBA should
work: BACK IT UP, copy it next to the .exe and rename it pokeemerald.sav.

Every time you save, the game first keeps your previous save as
pokeemerald.sav.bak1 (and the two before that as .bak2 and .bak3). If a
save ever goes wrong, close the game and rename a .bak file to
pokeemerald.sav.


FILES
-----
  MoemonEmeraldKaiCoop.exe   the game
  SDL2.dll                   needed by the game
  lobby.txt                  optional lobby server address (advanced)
  moemon_coop.ini            your co-op settings (written by the game;
                             AllowSmallTweaks = 1 also opens a log window
                             next to the game, handy when reporting bugs)
  moemon_display.ini         display settings (written by the game)
  sprites\                   your sprite sheets
  Sprite Editor.html         the sprite editor
  server\                    optional standalone server + DEPLOY.md
  moemon_release.txt         this version's number (the updater needs it)

Everyone should run the same version. A different version is refused
with a message saying so.


KNOWN LIMITATIONS
-----------------
* You see other players on your map and directly connected maps (e.g.
  Oldale <-> Route 101), not through a building door.
* Other players walk through you and NPCs, so nobody gets stuck.
* Trading isn't networked.
