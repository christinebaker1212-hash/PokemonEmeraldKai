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
  * Windows may ask whether to allow the game through the firewall the
    first time you host. Choose Allow (private and public networks).

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
* Items can't be used during a co-op battle (same as Cable Club battles).


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
* Use it to stand guard like an NPC trainer. When another player walks
  into your line of sight you get a "!", walk up to them and a link battle
  starts. Press B to pack the flag up.
* Winner gets 1/10 of the loser's money plus a random Poke Ball. The loser
  pays that and wakes up at their last Pokemon Center. HP/PP are restored
  and no EXP is gained, like Cable Club battles.


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


DISPLAY (START > DISPLAY)
-------------------------
  FULLSCREEN    borderless fullscreen (also F11 or Alt+Enter)
  WINDOW SCALE  1x to 10x
  SCALING       PIXEL PERFECT (whole-number zoom, sharpest),
                SHARP FIT (fills the screen, still crisp),
                STRETCH (fills the screen exactly)
  FILTER        OFF, SCANLINES, LCD GRID
  VSYNC         on/off
The window can also be resized freely. Settings are kept in
moemon_display.ini.


CONTROLS (keyboard)
-------------------
  Arrow keys = D-Pad      Z = A      X = B      Enter = START
  Backslash (\) = SELECT  A = L      S = R      Space (hold) = fast forward
  Ctrl+P = pause          F11 / Alt+Enter = fullscreen
  An Xbox-style controller also works.


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
