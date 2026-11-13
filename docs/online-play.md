# Online play (experimental)

The game's online modes between friends (the `online` branch; not in a release yet). There are no Xbox Live or game
servers: one player's PC hosts and the others connect to it, each with the same version of this port.

## Setting it up

The launcher's **Settings > Online**:

- **Online play**: on, on every PC.
- **Connection**: *Host* on one PC; *Join a friend* on the others, with the host's address and port as the host's
  launcher shows them under **Your address** (choose that row to copy it).
- On one network, the host's local address (192.168.x.x) is enough. Over the internet the friends type the host's
  public address, and the host's router must forward the UDP port (3074 by default, the **Port** row) to the host's
  PC. The host's game asks the router to do it while it runs (UPnP, when the router has it on; `online_upnp`), and its
  log says how it went (lines beginning `online: UPnP:`); a router without UPnP needs the port forwarded by hand. Or
  everyone joins a virtual network program (ZeroTier, Tailscale, Radmin VPN) and types the host's address on it.
  Some internet providers share one public address among many customers (carrier-grade NAT: the router's own
  "internet" address is a private one, such as 10.x.x.x or 100.64-127.x.x); a port cannot be forwarded there, so a
  friend without it hosts, or Steam, or a virtual network.
- **Through Steam** (experimental): the Steam friends running the game appear in its friends list, invites go through
  Steam and Valve's relays carry the game, so no port is opened. Steam must be running and signed in, and Valve's
  `steam_api64.dll` must be next to the game (not shipped yet); Steam shows the game as Spacewar (app 480).

## In the game

The online modes are in the game's own menus, as with Xbox Live on the console: the party, the friends list (the
connected players), invites, Pro Face-Off, Battle, Band Quickplay and the rest. An invite shows as a card at the top
of the screen for 15 seconds: hold START (or Enter) for 2 seconds to join.

The game's matchmaking also waits for strangers to fill a match's empty places, as on Xbox Live: up to 45 seconds
after the party starts a Band Quickplay or a duel. There are no strangers here, so the match waits 3 seconds
(`ghmetallica_online_host_wait`; 0 = the game's own times, for two separate groups of friends looking for each other,
Band vs Band). A slow connection plays fine: tested with 150-200 ms of delay and 5 % of the packets lost each way.

## The options

The launcher writes them to `ghmetallica_options.toml`; they can also go in `ghmetallica.toml` or on the command line:
`online_play`, `online_host` (`"a.b.c.d:port"`; empty: this PC hosts), `online_port`, `online_steam`, and
`online_xuid` (two instances on one PC with copies of one profile).
`ghmetallica_online_host_wait` (seconds, 3) is the matchmaking's wait above.
`online_upnp` (true): the host's router asked to forward its port.

## Turning it off

**Online play** off (the default): the game is offline, as before.
