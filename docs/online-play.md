# Online play (experimental)

The game's online modes between friends (since 1.0.0-beta.1). There are no Xbox Live or game servers: the players'
games connect to each other, each with the same version of this port.

## Setting it up

The launcher's **Settings > Online** (launcher 3.21): **Online play** on, on every PC; **Through Steam** turns on with
it. Steam must be running and signed in on every PC, the players must be Steam friends, and Valve's `steam_api64.dll`
must be next to the game (the release has it). Steam shows the game as Spacewar (app 480). Steam carries the game
between the players (its relays), so no port is opened.

Playing by address (one PC hosts, the others type its address and port) is in the game, but the launcher does not
offer it for now (`game::kOnlineByAddress` false, Gustavo's choice while online play is tried between homes); it
writes `online_steam = online_play`, an empty `online_host` and `online_upnp = false`. By hand, in the options file:
`online_host = "a.b.c.d:port"` on the friends' PCs, the host's router forwarding UDP 3074 (`online_port`), which the
host's game asks of it by UPnP (`online_upnp`, lines `online: UPnP:` in its log). An internet provider that shares one
address among its customers (carrier-grade NAT: the router's own internet address is a private one) makes hosting by
address impossible; Steam or a virtual network (ZeroTier, Tailscale, Radmin VPN) works there.

## In the game

The online modes are in the game's own menus, as with Xbox Live on the console: the party, the friends list, invites,
Pro Face-Off, Battle, Band Quickplay and the rest. The friends list shows the players connected and the Steam friends
with the game open. An invite shows as a card at the top of the screen for 15 seconds: hold START on a controller (or
Enter) for 2 seconds to join. A friend can also be invited from Steam's friends list (Invite to game), or joined
there (Join game): the game takes the player to the friend's party. That happens once per join: when the host later leaves
its party or goes online again (the game makes a new party), the friend stays in its own lobby until invited, as on
the console (SDK change 102; before it, the friend followed the host into every new party).

Steam does not show a friend's game when the friend's profile keeps its game details private (the friend looks
"online", not "in game"). The games then tell each other directly that they are running, so one of the two seeing the
other is enough for both friends lists (SDK change 101; before it, only one of the two listed the other, and the other
had to invite from Steam).

The game's matchmaking also waits for strangers to fill a match's empty places, as on Xbox Live: up to 45 seconds
after the party starts a Band Quickplay or a duel. There are no strangers here, so the match waits 3 seconds
(`ghmetallica_online_host_wait`; 0 = the game's own times, for two separate groups of friends looking for each other,
Band vs Band). A slow connection plays fine: tested with 150-200 ms of delay and 5 % of the packets lost each way.

## The options

The launcher writes them to `ghmetallica_options.toml`; they can also go in `ghmetallica.toml` or on the command line:
`online_play`, `online_steam`, `online_host` (`"a.b.c.d:port"`; empty: this PC hosts), `online_port`, `online_upnp`
(the host's router asked to forward its port), and `online_xuid` (two instances on one PC with copies of one profile).
`ghmetallica_online_host_wait` (seconds, 3) is the matchmaking's wait above.

## Turning it off

**Online play** off (the default): the game is offline, as before.
