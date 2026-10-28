# The standalone Moemon Co-op server (optional)

You don't need this to play online. When a player chooses HOST A GAME, the
game becomes the server, opens the router by itself (UPnP / NAT-PMP) and
shows an invite code. The port is closed again when they stop hosting.

This program is for groups that want a server that's always there, even when
the usual host is offline. It's the same server the game runs, with a lobby
where players pick a game by name.

## Easiest: on a PC at home

Run `moemon_server.exe` (Windows) or `./moemon_server_linux` (Linux). It
opens TCP port 27015 on the router automatically and prints an invite code:

```
Router port opened. Invite code for players: K7QM-3XPA  (address 203.0.113.7:27015)
```

Players type that code in JOIN A GAME > ENTER INVITE CODE, or put the
address in `lobby.txt` to see every game on the server by name. Press Ctrl+C
(or close the window) to stop; the router port is closed again. Use
`--no-router` if you forward the port yourself.

If it can't open the router, see "PLAYING OVER THE INTERNET" in README.txt.

## Alternative: a cloud machine

## What it needs

- A machine that stays on and has a public IP address.
- TCP port **27015** open to the internet (or any port you pick with `--port`).

### Setting one up

Any provider's free tier works. Oracle Cloud "Always Free" and Google Cloud's
`e2-micro` free tier are both enough. The steps are the same everywhere:

1. Create the smallest Linux machine (Ubuntu or Debian).
2. In the provider's firewall settings, allow **incoming TCP 27015**
   (Oracle: VCN > Security List > Ingress rule; Google Cloud: VPC network >
   Firewall > Create rule).
3. Connect to the machine (the provider's web "SSH" button is fine) and run:

   ```sh
   sudo apt-get update && sudo apt-get install -y gcc git
   git clone <this repository> moemon && cd moemon/tools/mp_server
   gcc -O2 -pthread -o moemon_server main.c ../../src/mp/mp_net.c ../../src/mp/mp_server.c ../../src/mp/mp_portmap.c ../../src/mp/mp_store.c ../../src/mp/mp_scores.c
   sudo cp moemon_server /usr/local/bin/
   ```

   Or copy the prebuilt Linux `moemon_server` from the release zip to the
   machine instead of building it.

4. Keep it running after you log out and after reboots (systemd):

   ```sh
   sudo tee /etc/systemd/system/moemon.service >/dev/null <<'EOF'
   [Unit]
   Description=Moemon Co-op lobby
   After=network-online.target

   [Service]
   ExecStart=/usr/local/bin/moemon_server --port 27015 --no-router
   Restart=always
   DynamicUser=yes

   [Install]
   WantedBy=multi-user.target
   EOF
   sudo systemctl enable --now moemon
   ```

5. Some images also run a firewall on the machine itself. On Ubuntu with
   ufw: `sudo ufw allow 27015/tcp`. On Oracle's Ubuntu images:
   `sudo iptables -I INPUT -p tcp --dport 27015 -j ACCEPT` (and
   `sudo netfilter-persistent save` to keep it).

6. Note the machine's **public IP** from the provider's console.

### Or forward the port yourself

1. Run `moemon_server.exe` (Windows) or `./moemon_server` (Linux/macOS).
   Allow it through the Windows firewall when asked.
2. On your router, forward **TCP 27015** to that PC.
3. Your address is your public IP (search "what is my ip"). If it changes
   often, a free dynamic DNS name (DuckDNS, No-IP) keeps it stable.

## Tell the game where it is

Open `lobby.txt` in the game folder and replace the example line with the
address, for example:

```
203.0.113.7
```

Add `:port` if you didn't use 27015 (`203.0.113.7:4000`). Save it and share
the folder. Players can also set `lobby = ...` in their `moemon_coop.ini`.

If you build the game yourself, `make winwsl LOBBY=203.0.113.7` builds the
address in, so even `lobby.txt` isn't needed.

## Checking it works

- On the server: `systemctl status moemon` shows it running, and
  `journalctl -u moemon -f` shows players connecting.
- In the game: HOST A GAME should start hosting right away. If the lobby
  can't be reached, JOIN says "Online lobby unreachable. Showing home network
  only." Check the port is open in both the provider's firewall and the
  machine's own firewall.

## Notes

- Passwords never leave the players' PCs in plain text: the server only sees
  a hash of server name + password. Wrong guesses are slowed down.
- Empty servers are removed when their last player leaves.
- Each server's SHARED PC, team goals and achievements are saved as
  `coop_store_<id>.dat` in the folder the server runs in (or `MOEMON_STORE_DIR`).
  Keep those files to keep everyone's shared PC.
- Limits: 512 connections and 256 servers at once, 4 players per server.
- `--lan` also makes the server answer local network searches (UDP 27016),
  useful when it runs on a PC on the same network as the players.
