# ALIEN Mobile — create an ancestor, watch its lineage

A native UIKit/Metal creature creator connected to a physical evolutionary terrarium.
Choose one of four templates, change its connected-cell anatomy, release it, and follow
what its descendants become. Saved genomes survive app relaunches and world resets.

## Play

```sh
./mobile/scripts/run-simulator.sh
```

Requires Xcode, an installed iPhone Simulator runtime, CMake and Python 3. The script
prefers an already booted iPhone; pass a Simulator UDID to select another.
The Release app is `build-sim/Release-iphonesimulator/AlienMobileApp.app`.

- **Create:** pick a template or saved creature, then **Place** or **Edit copy**.
- In the editor, tap a cell and choose its role. **Grow**, **Turn**, **Remove tip** and
  **Undo** change real anatomy. Tapping empty space grows from the selected cell;
  dragging a non-root cell reshapes its branch. Name it, then **Save & release**.
- Tap the world to release. The camera follows the new organism. Creating and placing
  pause the world so browsing does not consume its life.
- Tap life to follow it; **Follow child** advances to a living direct descendant.
  **Save specimen** stores the selected organism's exact genome in **My creatures**.
- **Food:** scatter a finite handful. **Mutagen:** one 20-second local exposure field
  can increase the chance of ordinary mutations in newly conceived offspring.
  **Flow:** drag to push actual cells and food.
- Drag to pan, pinch to zoom, **Wide** to see the tank. With Flow selected, two fingers
  pan. Simulator uses Option for pinch. **Pause / Play**, **1× / 2× / 4×**, and **↺**
  provide time control and a fresh experiment. Reset keeps saved creatures.

The world itself is not saved when the app closes. The current candidate was tested
on iPhone 17 Pro / iOS 26.5 Simulator, not a physical phone. Direct drag/pinch automation
had intermittent host errors; the tested button-based creator remains available.

Read [the current handoff](docs/hybrid-final/HANDOFF.md) for the exact tested experience,
limitations, simulation evidence and short human playtest instructions. Earlier milestone
and `playtest-pass` documents are historical; they describe obsolete controls and claims.

## Verification

```sh
cmake -S mobile -B mobile/build-release -DCMAKE_BUILD_TYPE=Release
cmake --build mobile/build-release -j4
ctest --test-dir mobile/build-release --output-on-failure -j4
mkdir -p /tmp/alien-observation
mobile/build-release/AlienMobileEvolutionResearch 913 1800 /tmp/alien-observation/world catalog default 240 30
mobile/build-release/AlienMobileHybridPlaytestObserver 43 1200 /tmp/alien-observation/edited
```

The observer records authored and inherited DNA, birth events and outcomes without
changing selection. The normal 48×48 tank has a 240-cell and 5,000-mote limit. Six
independent drifting patches supply consumable food. There are no automatic organism
replacements, protected lineages, diversity rewards or player-specific runtime rules.

CPU owns DNA, development, signaling and ecology. Device physics uses Metal; Simulator
uses CPU physics with Metal rendering. Every speed runs the same 1/120-second steps.
Skiff, Thread, Whorl and Husk remain exact archived genomes selected in earlier offline
simulation runs. Their identities do not affect live physics, energy or reproduction.
Inspired by [ALIEN](https://github.com/chrxh/alien).

Developer launch environment: `ALIEN_MOBILE_SEED` for a reproducible seed,
`ALIEN_MOBILE_DEBUG_OVERLAY=1` for instrumentation, `ALIEN_MOBILE_PHYSICS=cpu` for a
backend override. Historical ecology fixtures remain developer-only.
