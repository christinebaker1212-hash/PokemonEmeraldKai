MOEMON EMERALD KAI - CO-OP EDITION  (native PC, up to 4 players online)
=======================================================================

Moemon Emerald Kai v1.0 rebuilt as a native PC game from the pokeemerald
decompilation (Kurausukun's SDL2 PC port). No emulator or ROM is needed to
play. On top of Kai it adds online co-op:

  * Up to 4 players in the same world, seeing each other walk, run, bike
    and surf in real time (same map and directly connected maps).
  * Each player's name and the level of their strongest party Moemon is
    shown above their head (e.g. "Misty Lv25").
  * BATTLE FLAG key item: stand guard like an NPC trainer. You freeze and
    look around; when another player walks into your line of sight you get
    a "!", walk up to them and a REAL link battle starts (both players pick
    their own moves).
  * Winner gets 1/10 of the loser's money plus a random Poke Ball.
    The loser pays that money and wakes up at their last Pokemon Center.
  * Character customization: base sprite (Brendan, May, RS Brendan, RS May,
    or your own PNG sprite sheet), skin tone, outfit color, accent color.


QUICK START
-----------
1. Unzip the folder anywhere. Every player needs their own copy.
2. Run MoemonEmeraldKaiCoop.exe once. It creates "moemon_coop.ini"; close it.
3. ONE player is the host. In the host's moemon_coop.ini set:
       host = 1
       server = 127.0.0.1
4. Everyone else sets in their moemon_coop.ini:
       host = 0
       server = <the host's IP address>
5. Start the game and load your save (or start a new one). You connect
   automatically once you're walking around in the overworld. The text
   window behind the game shows "[mp] joined server as player N".

Playing over the internet needs one of these:
  * EASIEST: everyone installs a free virtual LAN such as Tailscale,
    ZeroTier or Radmin VPN, joins the same network, and uses the host's
    address from that app as "server". No router setup is needed.
  * OR the host forwards TCP port 27015 on their router to their PC and
    gives friends their public IP (search "what is my ip").
  * OR run moemon_server.exe on any always-on PC or a cheap VPS. Then
    nobody needs host = 1, and everyone uses that machine as "server".
    (moemon_server.exe --port 27015)


CONTROLS (keyboard)
-------------------
  Arrow keys = D-Pad      Z = A      X = B      Enter = START
  Backslash (\) = SELECT  A = L      S = R      Space (hold) = fast forward
  Ctrl+P = pause
  On Windows an Xbox-style controller also works.
  The window can be resized; it starts at 3x.


THE BATTLE FLAG (Trainer Mode)
------------------------------
* You receive it automatically (KEY ITEMS) the first time you connect.
* Register it to SELECT from the bag for quick use.
* Use it: "<you> planted the BATTLE FLAG!". You now stand still and look
  around like an NPC trainer. Press B to pack it up.
* Players who walk into your line of sight (4 tiles by default, set
  trainer_range in the ini, max 8) get challenged. Players who are in a
  menu, a cutscene, a battle, or are holding a flag themselves can't be
  spotted.
* The flag is packed up automatically after each battle. Plant it again
  for another round.
* Battles are single battles with your whole party. Like Cable Club battles,
  HP/PP are restored afterwards and no EXP is gained. The money and ball
  rewards are kept. SAVE afterwards to keep them.


CUSTOMIZING YOUR CHARACTER
--------------------------
In game: START > CO-OP. Change SPRITE / SKIN / OUTFIT / ACCENT with
Left/Right. Your sprite updates live and everyone sees the change.
"SAVE AND CLOSE" writes it to moemon_coop.ini.

Custom sprite: draw a sheet like custom_sprite_template.png
  * 288x32 PNG = 18 frames of 16x32 (9 walking + 9 running), same frame
    order as the template. 144x32 (walking only) also works.
  * At most 15 colors + transparency (transparent pixels, or the color of
    the top-left pixel if the PNG has no transparency).
Put the PNG next to the game and set in moemon_coop.ini:
       sprite = custom
       custom_sprite = my_sprite.png
Bike/surf/dive still use the Brendan/May sprites (with your skin and
outfit colors).


USING YOUR EXISTING SAVE
------------------------
The game saves to "pokeemerald.sav" next to the .exe. It is the normal
128 KB Emerald flash save, so a Moemon Emerald Kai save from mGBA should
work: BACK IT UP, copy it next to the .exe and rename it
pokeemerald.sav. (Several players on one PC: run with
--save other_name.sav)


moemon_coop.ini REFERENCE
-------------------------
  enabled        1 = online co-op on, 0 = plain single-player
  host           1 = this game also runs the server
  server         address of the server
  port           TCP port (default 27015)
  name           name shown above your head (empty = trainer name)
  sprite         default | brendan | may | rs_brendan | rs_may | custom
  custom_sprite  PNG file for sprite = custom
  skin           default | porcelain | light | tan | brown | dark | deep
  outfit/accent  default | red | blue | green | yellow | purple | pink |
                 orange | black | white | teal
  trainer_range  how far a BATTLE FLAG trainer can see (1-8)

Everyone should run the same build. Players on different versions are
refused with "VERSION MISMATCH".


KNOWN LIMITATIONS
-----------------
* You only see players on your map or a directly connected map (e.g.
  Oldale <-> Route 101), not across a building door.
* Remote players walk through you and NPCs, so nobody can get stuck.
* Busy areas can run out of sprite palettes. A remote player may then
  borrow another palette until a slot frees up.
* If a player disconnects mid-battle, the battle is called off with no
  rewards.
* Only single battles. Double/multi battles and trading are not
  networked.
