# M14 validation record

## Configuration and evidence boundaries

Final target: native Release arm64 app in iPhone 17 Pro / iOS 26.5 Simulator.
The default uses eight primitive founders, 360 physical-cell capacity, radius-6
finite resource field, mean 4 energy/s, a 120-second resource cycle and the same
1/120-second biological step. The historical wide-resource candidate remains a
1,200-cell stress scenario, not another player mode.

`default-equivalence/default.log` and `research.log` have identical state hash
`14225781207534141478` after 300 seconds at seed 16. Thus the completed 20-seed
research batch uses the final default's biological configuration. The selected
native recording seed is a representative branched run from that batch; it is
not presented as an unbiased sample or the sole successful seed.

## Automated verification

- `final-tests.log`: 21/21 passing tests, 41.92 seconds for the final parameter
  revision. Assertions enabled in Release. Includes host Metal/grid parity.
- `sanitize-tests.log`: ASan/UBSan mutation continuity, inherited evolvability,
  developing organs and 300 simulated seconds of the same heterogeneous biology.
- `pressure-tests.log`: three paired worlds and 20 new-world resets. Resource
  movement changes actual uptake; hazard movement causes local loss; neither
  teleports cells. Finite state and topology remain valid through repeated resets.
- `observer-check`: identical outcome hash with 10- and 60-second sampling.
- `../m14d/long`: two five-hour worlds, more than 200,000 combined paid births,
  1,712/1,436 sampled completed generations, energy error below .000453.

## Performance

The first native wide-resource observation used Metal physics in Simulator.
At 2× under concurrent research load its median rendering rate was only about
12 FPS. Repeated virtual-GPU synchronization was the demonstrated bottleneck.
The same existing CPU physics backend now runs in Simulator; Metal still renders
the world. Physical devices retain Metal physics. Both backends use the same
forces and fixed-step biology; long chaotic runs need not remain bit-identical
across CPU/GPU floating-point execution.

The CPU wide-resource check (`simulator-final-session.log`, a historical filename)
ran 191.76 simulated seconds at 1×, reaching 1,079 living cells. FPS median 60.00,
5th percentile 59.96; median 120 ticks/s. Memory ranged 127–223.4 MB and fell to
127 MB by the end. This was measured while other research runs were active.

The final 360-cell recording uses developer 8× mode. It executes the same fixed
steps, and its button truthfully displays 8×. That override is absent from the
normal 1×/2× handoff. The exact timing samples are in
`observation-default/recording.log` and `recording.metrics.json` (the full console
log also includes subsequent interactive checks). Through the 30-minute capture,
median FPS is 60.00, 5th percentile 59.99, and median throughput 960 ticks/s.
The 1,804.61-second frame arrived at 230.61 wall seconds: about 7.83× including
launch/capture overhead. It contained 1,547 completed births, approximately 402.5
births per actual wall minute during this accelerated recording. Memory fell
from a peak 216.6 MB to 146.3 MB; no monotonic runaway was observed.
CPU stage timings in logs are cumulative averages, not per-frame percentiles.

The earlier physical iPhone 13 run (`device-session.log`) reached 1,101 living
cells across 914.48 simulated seconds at 1×. Median 60.04 FPS, median 120.1
ticks/s, 35.3–48.1 MB, thermal state 1 throughout the reported samples. Render
GPU time had a median 7.79 ms and a maximum sampled 12.83 ms. The process exited
cleanly and the device disconnected. This validates the prior wider biology
under device load, not installation of the final visual/default-world changes.
The owner subsequently instructed completion in Simulator only. No battery-drain
number, physical-finger test or final-device-install claim is inferred.

## Observation records

`observation/` contains the earlier wide-resource native run: 0, 300.97, 601.45,
1,201.42 and 1,800.17 simulated seconds, captured at approximately 0.66, 191.85,
367.08, 732.42 and 1,085.12 wall seconds. It started at 1×, then used 2×. This
sequence exposed dense minimum-body crowding and poor virtual-GPU frame pacing.

`observation-default/` contains the final default biology, rendering and Simulator
backend at 0/5/10/20/30 simulated minutes. `captures.json` records actual observed
simulation and wall times rather than pretending accelerated time is wall time.
The first image is a launch snapshot; the later labels use the console sample
immediately before capture, so sub-second capture latency is not exact tick lock.

`viable-start/seed-01-worlds.png`, `seed-08-worlds.png` and `seed-16-worlds.png`
show recorded physical states. Their earliest old observer sample is 30 seconds,
correctly labeled 0.5 minutes. They are not falsely relabeled as time zero.
New native launch snapshots and the latest observer include growing founder roots.

The native final-default sequence visibly shows inherited branches, different
arm lengths, changing density, dim/starving cells and real cell links. It does
not establish a sophisticated evolved navigation or predation strategy.

## Watchability metrics and their limits

Final-default seed 16 over 30 simulated minutes: 1,544 births (51.47/min at 1×),
109 completed generations, 1,008 DNA-changing completed births (33.60/min),
124 completed structural-operator births (4.13/min) and two meta births.
The respective aggregate inter-event periods are 1.79 seconds for a DNA-changing
birth and 14.52 seconds for a structural-operator birth. These are **not** means
for noticeable or beneficial mutations; many changes are silent or fail quickly.

There are 55 sampled physical morphotype signatures, a proxy affected by injury,
module expression and observation cadence. Dominant-lineage replacements were
observed at 270, 600, 960, 1,680 and 1,710 seconds. 88 of 91 observed families were
absent at the end; six persisted across at least 120 seconds of observation.
About 64% of sampled mature individual IDs were replaced between adjacent
30-second observations on average, with roughly 1.54 births per mean mature
population per minute. No such statistic is used by the engine.

Visual body differences appeared by the five-minute sample and persisted across
later samples. Exact first-visible-change time is bounded by sampling, not known
to the second. Instantaneous paid-force diagnostics confirm that six final
descendants in generations 103–108 were actually propelling; motor presence alone
is not used as proof of activity. A human's meaningful-phenotype-change cadence
and enjoyment remain unmeasured until the playtest.

## Native controls and visual checks

The actual speed button was exercised from developer 8× to normal 1×, then to
2×. Three actual UI resets restored primitive roots, the original camera and
1×. Portrait and both supported landscape orientations were inspected; controls
stayed inside safe areas. Upside-down portrait is not a supported app orientation.
`ui/landscape.png` preserves a landscape check and `ui/normal-observation.mp4`
contains an actual one-minute native observation at normal speed, encoded
to 720-pixel width without changing playback speed. The original simulator
recording is retained under the ignored build directory.

The source/hazard pressure mechanics and coordinate transforms pass automated
core checks; the existing pan/pinch recognizer wiring was inspected. Live drag
and pinch synthesis could not be completed: the Computer Use coordinate-action
backend returned `noWindowsAvailable` despite successful screenshots and AX
button actions, including after raising the window and changing orientation.
This is a testing-tool limitation, not an observed in-app drag failure. No claim
of a newly completed automated finger-gesture test is made. Actual mouse drag
and Option-drag pinch remain part of the hands-on Simulator evaluation.

For this Xcode 26.6 Simulator, Apple's documented pinch interaction is holding
Option while dragging the simulated touch points. See
[Apple's Simulator gesture guide](https://developer.apple.com/library/archive/documentation/IDEs/Conceptual/iOS_Simulator_Guide/InteractingwithiOSandwatchOS/InteractingwithiOSandwatchOS.html).

## Build identity

Xcode 26.6, build 17F113; C++17 core, Release arm64 Simulator app. Source and
executable hashes are recorded in FINAL_SOURCE_SHA256.txt. Fresh app launches and resets both use a new random seed; a developer seed
override remains available and is printed only in diagnostic logs. The extra
core-default reference seed also survived 30 minutes: 8,383 births, 220 completed
generations, 286 mature organisms and conservation error under .000001
(`initial-default/ANALYSIS.md`). That brings the short/medium validation to 41
worlds, in addition to the two long runs. The final handoff
launch has no diagnostic speed override, no genome inspection UI and no seeded
specialist fixture. All developer observation sessions are stopped at handoff.
