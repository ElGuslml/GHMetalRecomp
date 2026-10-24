# Only the guitar and the kick drum heard in the songs

**Status: fixed (2026-10-24).** Confirmed by ear in "Ace of Spades" and other songs: the whole band plays.

## Symptom

When a song starts, its first second plays (the bass intro of "Ace of Spades"), then most of the band goes
silent: from there on only the guitar part and the kick drum are heard.

## Cause

Each song comes in three files (`DATA/MUSIC/<song>_1.fsb.xen`, `_2`, `_3`), and each file holds several stereo
tracks (the stems: drums, bass, vocals, the rest of the song...) mixed together packet by packet in one XMA stream.
The game's sound library gives every track the same 32 KB of the file and lets the console's XMA decoder pick each
track's packets: every packet says how many packets of the other tracks to skip before the track's next one, and
that skip goes on into the next 32 KB block.

The SDK's XMA decoder started every track at the first packet of the next block, which belongs to the first track
of the file. After the first block, every track but the first decoded the wrong packets: the first track of each
file (the guitar, the kick drum) was the only one left.

## Fix

A change in the SDK (change 40 of its `recomp` branch): the skip carries into the next block, so each track goes on
at its own packet (measured: the tracks continue at packets 1, 2 and 3 of the next block). Files with a single
track, such as the menu music, decode exactly as before.

## Turning it off

There is no setting: it is how the console's decoder reads these files.
