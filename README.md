# ALIEN Mobile

A native iOS creature creator connected to a physical evolutionary terrarium.
**The player authors the ancestor. The simulation authors the lineage.**

Start with an empty aquarium, choose exactly what to place, or make a creature from a
blank root. Edit real connected-cell anatomy, choose its color, release it, follow its
offspring, and save an interesting descendant for another experiment. A controlled
12-second trial compares the original with your edits before release.

## Run on iPhone Simulator

Requires macOS, Xcode with an iOS Simulator runtime and Metal command-line tools,
CMake, and Python 3.

```sh
git clone https://github.com/MaxHastings/alien-mobile.git
cd alien-mobile
./mobile/scripts/run-simulator.sh
```

The launcher builds Release, installs the app and starts it on an available iPhone
Simulator. Pass a Simulator UDID to select a specific device. No signing identity
is needed for Simulator; a physical-device build requires your own Apple signing setup.

## Build and test the simulation

The C++17 simulation and non-Metal tests also build on Linux.

```sh
cmake -S mobile -B mobile/build-release -DCMAKE_BUILD_TYPE=Release
cmake --build mobile/build-release -j4
ctest --test-dir mobile/build-release --output-on-failure -j4
```

## Current status

**Ready for human playtest in iPhone Simulator.** The Release create/compare/release/
follow/save/reload loop is verified alongside 34 core tests, focused sanitizer checks,
and ten-minute simulation observations. Physical-device qualification is out of scope
for this pass. Human touch feel and enjoyment remain the next test.
- [Controls and implementation overview](mobile/README.md)
- [Current playtest handoff and evidence](mobile/docs/human-playtest/HANDOFF.md)
- [Catalog genome fixtures](mobile/docs/discovery/finalists)

`mobile/core` contains the simulation, `mobile/ios` the UIKit/Metal app,
`mobile/tests` the tests and observers, and `mobile/scripts` the build/research tools.
The repository includes historical written reports and current bounded evidence.
Large historical recordings, raw research runs, build outputs and local app saves
are excluded from Git and remain on the original workstation. Historical reports
may reference those untracked artifacts. Evidence logs retain their original run paths;
those paths are historical metadata, not build prerequisites.

Inspired by [ALIEN by chrxh](https://github.com/chrxh/alien). This is a separate mobile
project, not the upstream ALIEN repository. No open-source license is granted by this
private repository.
