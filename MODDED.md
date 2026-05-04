(anything that isn't in the future category that says 'will' or 'will be' or 'planned' is now in the source, so it is now: 'has', 'is', etc, etc).

# What's this mod?
An enhanced version of the the port of the game Cro-Mag Rally.
Notably, there will be new features and enhanced old ones, some of which will be committed to this repository soon (they aren't github ready yet).

# What features are there?
Enhanced old features and brand new ones (based on racing games of it's time and newer ones). 
Since we're working with C, some things aren't possible without extra libraries and goodies that may not be compatible with Cro-Mag Rally's specific libraries it already has
(coming from Java background, so there will be mistakes, none that mess with memory however).

## Here's a list of the old enhanced features:

- CPUs now cheat! CPUs are fairly dumb, so to give them some edge, they can turn invisible, have random boosts at any time, and their cars are enhanced randomly at the start, plus more little things.
- CPUs now fire throwable items slower or faster depending on RNG.
- Nitro is now dependent on placement in the race: You get less nitro time if your in first, versus if your in last place.
- Bottle Rockets/Roman Candles' items now give you more per collection, as they are the most effective against CPUs and the player (15-20ish).
- Bone Bombs, Oil, and the other non-candle/rocket items give you a random amount between 2-3, or 5-7, etc.-etc., depending on RNG.
- Oil now spreads further and is wider to cause more slipping for careless drivers
- Homing Pigeons fly faster and start up in the air more.
- Bottle Rockets fly faster.
- Torpedos used in non-water tracks don't display bubbles or play looping sounds (as they are not loaded, which caused errors, fixed the error, also unintendedly fixes the immersion a bit).
- Submarine turns into a plane in normal levels (doesn't look like one, but does remove bubble particles) and if you collect anything but nitro, the item is a SUPER BURST, giving you a 1 of each; Roman Candles, Bottle Rockets, and Torpedos, which can hit CPUs in non-sub vehicles.
- Terrain is coord stretched horizontally (not vertically to not ruin the vertical stretch physics feature), and CPUs can drive on it no problem. The track is now wider and longer.
- Some previously static objects now dance or move to make the scene a little more active, but not too distracting. For example, the trees now stretch up and down smoothly to look like they are moving in the wind.
- Arrowheads appear in practice *and* tournament mode. In practice mode, arrowheads give you items depending on RNG

## Here's a list of the brand new features:
- New mod menu options
- Silence announcer option (makes the announcer stay quiet)
- Submarines for all players in normal tracks option
- Singleplayer Survival gamemode support
- New behavior: CPUs chase the player and can move around in battle mode (multiplayer maps and gamemodes)
- Zapped Player State (has no proper icon yet): when zapped, you will turn small and move much slower than your usual speed. When the timer runs out, you will regain your original speed and size.
- Night Mode: selected tracks will have a night time/sunset lighting and coloring option (and sometimes level changes to objects). The skybox, terrain and objects will be shaded and have an alternative appearance (it is currently working but is not synced nor is 'night' in the sense of lighting/dark shadows, as the custom C code and the OpenGL-like system doesn't have good support for dynamic lights, that are needed for full effect, and such).
- Fast Music on Lap 3+
- Completed race music
- Added items to arrowhead when in non-tournament modes
- Zapper Item: zaps only players/CPUs who did not fire the zapper for 5 seconds, if user who fired zapper is already zapped, their zapper timer will be reset to 0
- Beam Item: fire a beam like the Crete statues at nearby players, but be warned, they can too!
- Whirlwind Item: fire a moving-based-on-speed nearly invisible wind that packs a punch when it expires


Note:
**Do not report mod suggestions/issues/bugs to the original GitHub that Jorio has made, as that port is for official changes only, which may propagate if they add features or fix critical issues, or optimize the game more.**
