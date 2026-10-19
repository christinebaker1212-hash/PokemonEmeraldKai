MOEMON EMERALD KAI - CO-OP EDITION  (native PC, up to 4 players online)
=======================================================================

Moemon Emerald Kai v1.0 rebuilt as a native PC game. No emulator or ROM is
needed. On top of Kai it adds online co-op for up to 4 players:

  * Everyone plays their own game in the same world and sees each other
    walk, run, bike, surf and dive in real time, with a name tag above
    their head ("Misty Lv25").
  * Team up against NPC trainers: walk up to a friend who has just been
    challenged and join their battle as a Double Battle.
  * Watch a friend's battle: walk up to someone marked IN BATTLE and press A.
  * PvP with the BATTLE FLAG.
  * Dress up as any trainer or NPC in the game, recolor them, or draw your
    own sprites with the included Sprite Editor.
  * Proper fullscreen, sharp scaling at any window size, and display filters.


QUICK START
-----------
1. Unzip the folder anywhere. Every player needs their own copy.
2. Start MoemonEmeraldKaiCoop.exe and press START on the title screen.
3. Choose:
     HOST A GAME  - type a server name (and a password if you want one),
                    then START HOSTING. The game opens your router by
                    itself and shows an INVITE CODE like K7QM-3XPA.
                    It's copied, so just paste it to your friends.
     JOIN A GAME  - choose ENTER INVITE CODE... and type the code your
                    friend sent, then the password if they set one.
                    Friends on the same Wi-Fi also show up in the list.
     PLAY SOLO    - play alone. You can go online later from START > CO-OP.
4. Load your save or start a new game. That's it.

Typing works with the real keyboard (Ctrl+V pastes). The game remembers the
last server, so next time JOIN offers it straight away. While you host, the
invite code is also shown at the bottom of START > CO-OP.


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
* When an NPC trainer challenges a friend (they get the "!" or talk to the
  trainer), walk up next to your friend and face them. You'll see
  "Asking <friend> to team up..." (press B to change your mind).
* When your friend finishes reading the trainer's speech, they get:
  "<you> wants to join for a Double Battle! Is this OK?"  YES / NO
  This message can't be skipped or sped up, so nobody says yes by accident.
* Trainers with only one Pokemon refuse: "Hey, I only have one POKeMON!
  Let's go one on one!" and the battle goes ahead normally.
* Each player brings their first three healthy Pokemon and picks their own
  moves. Both players get the full prize money and EXP for their Pokemon.
  Evolutions happen on each player's game after the battle.
* A Pokemon that already knows four moves can't stop the battle to learn a
  new one. After the battle you're asked if you want to teach it a move,
  and the Move Reminder screen opens.
* If you lose, both of you black out. If someone disconnects, the battle is
  called off.
* Ordinary trainers count as beaten for both players. Story battles (gym
  leaders, rivals, villains) only count for the challenged player, so the
  partner can still get their badge by battling them on their own later.
* Items can't be used during a co-op trainer battle (same as Cable Club
  battles).


WILD BATTLES TOGETHER
---------------------
* Walking in tall grass with a friend nearby? When a wild Pokemon jumps out,
  the closest friend who is also standing in grass (within about a screen
  of you) jumps into the battle with you. There's nothing to accept: it just
  happens, as long as they're free (not in a menu, a battle or a trade).
* It's two of you against one wild Pokemon. Both of you get EXP.
* You can throw POKe BALLS (only Balls: other items can't be used). Whoever
  throws the Ball that catches it gets the Pokemon.
* Friends further away (or not in the grass) don't join: you battle alone,
  like normal.


TEAM GYM REMATCHES
------------------
Talk to a gym leader you've already beaten while a friend is on the same
map: they offer a team rematch with their strongest team (even stronger once
you have all eight badges). Your friend joins the usual way, by standing next
to you and facing you. Both of you get a reward for winning.


TOURNAMENT (START > CO-OP > TOURNAMENT)
---------------------------------------
A quick battle tournament between everyone online. Everyone battles everyone
once, a few matches at a time; with an odd number of players someone sits out
each round, but everyone plays the same number of matches. These are friendly
battles: no money changes hands and nobody blacks out. The champion gets the
best prize, second place a MOEGATCHA pull, everyone else a consolation prize.


WATCHING BATTLES
----------------
Players in a battle have "IN BATTLE" above their name. Walk up to them and
press A, then YES to watch live. Your own game pauses while you watch.
Press B (or START) to stop watching at any time. Watching ends by itself
when the battle is over.


PVP: THE BATTLE FLAG
--------------------
* You get the BATTLE FLAG (KEY ITEMS) the first time you connect. Register
  it to SELECT for quick use.
* Use it to stand guard like an NPC trainer. While the flag is planted your
  name tag and follower are hidden, so you blend in. When another player walks
  into your line of sight you get a "!", walk up to them and a link battle
  starts. Press B to pack the flag up.
* Winner gets 1/10 of the loser's money plus a random Poke Ball. The loser
  pays that and wakes up at their last Pokemon Center. HP/PP are restored
  and no EXP is gained, like Cable Club battles.


TRADING, THE SHARED PC AND DAILY GIFTS (START > CO-OP)
-----------------------------------------------------
TRADE...      Trade with a friend anywhere: a Pokemon, up to two kinds of
              items and money, in any mix. Both of you pick your side, choose
              READY, then confirm. Pokemon that evolve by trading evolve.
SHARED PC...  A PC that belongs to the server: anyone can put Pokemon and
              items in, and anyone can take them out. It's kept on the host's
              PC, so it's still there the next time the same server name and
              password are hosted.
DAILY GIFT... Send each friend online today's gift once a day. They get the
              gift and you get a thank-you bonus (bigger with more players).
              The gift changes every day.
Pokemon you get from a friend keep their original trainer (they gain more
EXP, but very high levels may not obey you until you have more badges).


PLAYING TOGETHER EVERY DAY (START > CO-OP > DAILY)
--------------------------------------------------
TEAM GOALS     Three new goals every day for everyone on your server, like
               "Beat 9 trainers as a team". Targets grow with the number of
               players online today. The co-op battle goal is partner by
               partner ("Win a co-op battle with BOB and CARL"), so with three
               players everyone teams up with everyone. Every finished goal
               rewards everyone; all three give a MOEGATCHA pull.
OUTBREAK       One Moemon appears a lot on one route today (more often with
               more players online). It's announced when you connect.
STREAK         Play 10 minutes with a friend online every day to build your
               co-op streak. Rewards grow each day; it's on your TRAINER CARD.
WEEKLY RACE    A different challenge every week with a leaderboard. The
               winner gets RARE CANDY x3, money and a MOEGATCHA pull.
ACHIEVEMENTS   Team milestones (Pokemon caught together, trainers beaten,
               co-op wins, badges, trades, gifts). Each gives every player a
               MOEGATCHA pull.
MOEGATCHA      (START > CO-OP > MOEGATCHA) Spend pulls on the prize reel:
               it races past prizes and slowly clicks down onto yours.
               Five rarities: COMMON 55%, UNCOMMON
               28%, RARE 12%, SUPER RARE 4.5%, ULTRA RARE 0.5%. The top ones
               include event Moemon with the moves of real event Pokemon
               (WISHMKR Jirachi, AURA Mew, AGETO Celebi, SPACE C Deoxys, the
               Pokemon Box eggs), shiny event Moemon, Master Balls and
               jackpots. Pity: a SUPER RARE or better at least every 40 pulls
               and an ULTRA RARE at least every 120. A 10-pull always has a
               RARE or better, and everyone gets one FREE 10-pull every week.
               The game saves by itself after every pull, so prizes are safe.
Didn't finish the goals or win the race? You get a consolation prize picked
for what you're short of: Rare Candy if your team is behind, healing items if
you're low, Poke Balls if you're out.
Days change at midnight UTC so friends in different countries share them.


HANGING OUT (START > CO-OP > SOCIAL)
------------------------------------
EMOTES & PHRASES  Emotes, quick lines and your four phrases (keys 1-8 too).
                  Do the same emote as a friend close by at the same moment
                  for a COMBO (DOUBLE SHOCK!!, TRUE FRIENDS!, DUET!...).
CHAT LOG          Everything said and emoted, and the news pop-ups.
SCRIBBLE          Stand facing a sign and leave one of your phrases on it.
                  Friends see it after the sign's own text.
PRANK A FRIEND    Costs a MOEGATCHA pull: Magikarp follower (1 min),
                  backwards controls (15 sec), confetti cannon, whoopee
                  cushion. The victim gets a free REVENGE prank for 2 minutes.
                  Nobody can be pranked twice in a row, or during a battle.
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


HANDY EXTRAS (they save time, they don't make the game easier)
---------------------------------------------------------------
* Battle: on the move list, R shows each move's power, accuracy and kind
  (PHYS./SPEC./STATUS). At FIGHT/BAG, L throws the same kind of Ball as last
  time (wild battles). In a co-op battle, R shows your partner's team.
* Summary screen: the stat your Pokemon's nature raises is red, the one it
  lowers is blue. SELECT on the first page renames a Pokemon you caught.
* Bag: in KEY ITEMS, press L or R on an item to put it on that button (like
  SELECT's REGISTER). On the map, L / R use them.
* Hold SELECT on the map to pick which Pokemon follows you (a quick tap of
  SELECT still uses your registered item).
* When a REPEL wears off you're asked if you want to use another one.
* PC: R on a Pokemon in a box takes it into your party; R on a party Pokemon
  stores it (you pick the box).
* START > CO-OP > MOVE TUTOR: the Move Tutor's service anywhere, still one
  HEART SCALE per move.
* Item descriptions show in the shared PC and in trades.
* Signs and lines you've already read appear instantly. Hold START during a
  scene or conversation you've already seen to play it 4x faster (questions
  and menus still wait for you).
* Coming online shows what your team did while you were away. Tournament
  matches show how healthy your team is before you say READY.


POKEMON THAT FOLLOW YOU
-----------------------
Open the POKEMON menu, pick a Pokemon with the cursor and press SELECT:
it walks behind you. Press SELECT on it again to put it back in its Ball.
All 386 Pokemon have their own walking sprites (with female and form
differences). Talk to it (A) and it's happy to see you. Other players see
your follower walking behind you too. It hides while you surf or dive and
comes back when you land. The choice is remembered in moemon_coop.ini.


DRESSING UP (START > CO-OP)
---------------------------
  SPRITE   any trainer or NPC in the game (Left/Right, L/R jump 10).
           It's used everywhere: walking, running, biking, surfing,
           diving, fishing, field moves, the TRAINER CARD and battles.
           Animations a sprite doesn't have use Brendan's/May's.
  SKIN     skin tone      OUTFIT / ACCENT   clothing colors
Everyone sees your look right away. SAVE AND CLOSE remembers it.

Characters without a battle back sprite: in battle your trainer isn't shown
(your Pokemon still are). You can draw one in the Sprite Editor.


SPRITE EDITOR
-------------
START > CO-OP > EDIT SPRITES... saves your current character as a sprite
sheet in the "sprites" folder and opens "Sprite Editor.html" in your web
browser (it works offline).

In the editor you can:
  * open any sprite sheet (or drag it onto the page),
  * pick the character it's based on (this is how you edit an existing
    trainer or NPC, e.g. give the Hiker a battle back sprite),
  * paint every animation: walking, running, Mach Bike, Acro Bike, surfing,
    diving, fishing, field moves (using items/Pokemon), watering,
    decorating, the TRAINER CARD picture and the battle back sprite,
  * import a picture into any frame, copy/paste/flip frames, undo/redo,
  * preview the walk cycle, and keep each group within 15 colors.
Save the PNG, then drag it onto the game window. You wear it immediately.

Your edited sprites are sent to the other players with your character, so
everyone sees YOUR version. Two players can edit the same trainer
differently; nobody's version overwrites anyone else's.

Older 144x32 / 288x32 walking sheets still work.


SETTINGS (START > SETTINGS)
---------------------------
  FULLSCREEN    borderless fullscreen (also F11 or Alt+Enter)
  WINDOW SCALE  1x to 10x
  SCALING       PIXEL PERFECT (whole-number zoom, sharpest),
                SHARP FIT (fills the screen, still crisp),
                STRETCH (fills the screen exactly)
  FILTER        OFF, SCANLINES, LCD GRID
  VSYNC         on/off
  CONTROLLER    which controller plays. Press A on this row, then press
                any button on the controller you want; that one is used
                from then on (it's remembered). Left/Right goes back to
                ALL CONTROLLERS.
  PAD BUTTONS   AUTO, NINTENDO (A = right button) or XBOX (A = bottom)
  STICK RUN     push the stick all the way to run (on/off)
  RUMBLE        on/off
The window can also be resized freely. Settings are kept in
moemon_display.ini and moemon_controls.ini.


CONTROLS (keyboard)
-------------------
  Arrow keys = D-Pad      Z = A      X = B      Enter = START
  Backslash (\) = SELECT  A = L      S = R
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
  Walking around    stick pushed all the way = run, X/Square = auto-run
                    on/off, Y/Triangle = CO-OP menu
  Text and menus    LT (hold) = keep pressing A, to get through text fast;
                    X/Square = B, Y/Triangle = SELECT
  Naming screen     X/Square = delete, Y/Triangle = switch letters
Controllers rumble when a trainer spots you, when a battle starts and when
a friend asks you to team up.


USING YOUR EXISTING SAVE
------------------------
The game saves to "pokeemerald.sav" next to the .exe. It's the normal
128 KB Emerald flash save, so a Moemon Emerald Kai save from mGBA should
work: BACK IT UP, copy it next to the .exe and rename it pokeemerald.sav.


FILES
-----
  MoemonEmeraldKaiCoop.exe   the game
  SDL2.dll                   needed by the game
  lobby.txt                  optional lobby server address (advanced)
  moemon_coop.ini            your co-op settings (written by the game)
  moemon_display.ini         display settings (written by the game)
  sprites\                   your sprite sheets
  Sprite Editor.html         the sprite editor
  server\                    optional standalone server + DEPLOY.md

Everyone should run the same version. A different version is refused
with a message saying so.


KNOWN LIMITATIONS
-----------------
* You see other players on your map and directly connected maps (e.g.
  Oldale <-> Route 101), not through a building door.
* Other players walk through you and NPCs, so nobody gets stuck.
* Busy areas can run out of sprite colors; a player may borrow another
  palette for a moment.
* Trading isn't networked.
