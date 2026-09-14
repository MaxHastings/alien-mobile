# ALIEN Mobile — make a creature, follow its family

An iPhone-first UIKit/Metal artificial-life sandbox. Create an ancestor, observe a
short controlled trial, release its ordinary inherited DNA into a physical ecosystem,
and save the descendants worth trying again.

## Launch on this Mac

From the repository root:

```sh
./mobile/scripts/run-simulator.sh
```

Requires Xcode with an iPhone Simulator runtime and Metal tools, CMake, and Python 3.
The script builds **Release**, opens Simulator, installs and launches the app, and
clears old observation-only Simulator settings. An optional first argument selects
a Simulator UDID. The app is at
`mobile/build-sim/Release-iphonesimulator/AlienMobileApp.app`.

## Play

- **Create:** choose Skiff, Thread, Whorl, Husk, a saved creature, or a blank constructor.
  **Edit & try** opens the creator; **Place** releases an exact copy without editing.
- **Build:** tap a cell, choose an organ, grow a branch, turn a motor, remove a tip,
  or drag a branch. Color and name are yours. The constructor remains connected.
  Normal edits retain inherited control. **Undo** restores edits.
- **Try body / Original:** watch 12 seconds of ordinary simulation in the same finite
  food arrangement. White outlines identify the subject; white lines show paid thrust.
  Offspring remain visible. Compare displacement, remaining cells and fully grown young.
  Red cells have depleted usable energy. The trial is evidence, not a survival score.
- **Rewire…** explicitly replaces cell controllers using the current anatomy. The result
  is ordinary inheritable DNA; it can fail. Compare it and Undo if needed.
- **Save & release:** saves the edited DNA before placement. Tap near gold food in the
  world. Browsing, editing, naming and placement pause the ecosystem.
- **Follow:** tap life; **Family →** follows a living child or another surviving member
  of its family. The selected body is outlined; others are dimmed. **Wide** shows the tank;
  **Find** returns to the selection. Its DNA remains available after death.
- **Save / Edit copy:** preserve a descendant or start an experiment from its exact DNA.
  The original creature continues its own life when you return to the world.
- **Food / Mutagen / Flow:** scatter finite food, place one 20-second mutation exposure,
  or drag a physical current. Tap a selected tool again to return to observing.
- Drag to pan, pinch to zoom; Flow uses two-finger pan. In Simulator, use Option-drag for
  pinch. **Pause**, **1× / 2× / 4×**, and **↺ New experiment** control time and retry.

Saved genomes persist across launches and new worlds. **The live world does not persist
when the app is terminated.** New experiment replaces the tank and reopens the chooser.
Failed placement explains boundaries, occupied water, invalid development or capacity;
capacity is never presented as biological death.

## Simulation contract

Cells, geometry, paid thrust, local food uptake, energy sharing, construction, damage,
and inherited controllers determine outcomes. No runtime steering, protected player
lineage, replacement organisms, identity-dependent buffs, or forced successful mutation.
Bending and contractile organs change shape through internal forces; they do not create
free swimming. Attack captures material; Digest makes it usable; Storage stores usable
energy and still needs food. Repeated developmental structures survive unambiguous edits.
An edit that requires an independent unfolded body is disclosed in the creator.

The normal 48×48 tank has a 240-cell and 5,000-mote limit. Six drifting resource patches
supply consumable food. Capacity can delay reproduction. All speeds use 1/120-second steps;
large workloads may run below the requested speed. Simulator uses CPU physics and Metal
rendering; physical devices use Metal physics.

## Verification

```sh
cmake -S mobile -B mobile/build-release -DCMAKE_BUILD_TYPE=Release
cmake --build mobile/build-release -j4
ctest --test-dir mobile/build-release --output-on-failure -j4
```

See [the current human-playtest handoff](docs/human-playtest/HANDOFF.md). Earlier reports
are historical, including `hybrid-final` and `biology-playable` evidence. The current
qualification is **iPhone 17 Pro / iOS 26.5 Simulator on Mac**; no physical iPhone was used.
