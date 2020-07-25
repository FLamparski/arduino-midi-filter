# MIDI filter firmware

This firmware powers my MIDIchlorian filter/thru box. It's really simple.

The controls are:

* Velocity - pass through or rewrite to max
* CCs - pass through or drop
* Realtime/Transport controls - pass through or drop (except for clock info)

The primary design use case is to control a number of Korg Volcas from an Arturia Keystep. This
device is required because the Volcas react to transport controls for their internal sequencers,
and the Keystep sends these commands when you play or stop its sequence. Additionally, pressing
Stop on the Keystep sets note sustain to 0 which is understandable but also slightly annoying.

Demonstration: https://www.instagram.com/p/CDCDuVWHm6M/

Licenced under GPL version 3.

You will need the following Arduino libraries:
* [Arduino MIDI Library][1] v5.0.2
* [The Timer library][2] master

[1]: https://github.com/FortySevenEffects/arduino_midi_library
[2]: https://github.com/brunocalou/Timer
