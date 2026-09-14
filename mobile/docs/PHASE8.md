# Simulator following and Phase 8 — 2026-09-13

**Phase 7's Simulator visual gate passes. Phase 8 is implemented. Physical-device
validation remains a required gate before final ecosystem delivery.** This report
supersedes the unresolved Simulator gate in `BEHAVIOR_CHECKPOINT.md`; it does not
claim completion of the later selection/predation phases.

## Visible following

The opening camera is 44% closer (0.18 versus 0.125), and the starting food source
is offset from the founder so the first approach is visible without waiting for a
birth. Motor cells render a short directional exhaust glow whose orientation and
strength come from the actual paid thrust vector. Zero thrust produces no exhaust.
Food/danger handles render above organisms so a crowd cannot hide the draggable
source. The diffuse fields still render behind life.
Lineage colors, physical cells/connections, energy brightness and the same food,
danger, pan, zoom, speed and reset controls remain.

Watch `behavior/following-simulator-clip.mp4`: real-time footage extracted from the
longer `following-simulator.mp4`, recorded from the iPhone 17 Pro / iOS 26.5 Simulator.
`following-sequence.png` shows four successive frames. After the food moves, the
body rotates, its motors light, and it approaches the new source. The full log is
`following-simulator.log`.

Measured in the actual Simulator Metal run:

- Initial source: COM-to-food distance reaches 0.30 at 6.03 simulated seconds.
- Food moves from (−0.5, −1.5) to (2, 1.5) during seconds 18–19.
- At 19.12 s the body is 3.846 units away. It first continues slightly away, turns,
  then closes to 0.185 units at 28.20 s. It subsequently overshoots and oscillates.
- The isolated body contains four cells. Output was approximately 60 FPS in this
  Simulator run. This is not physical iPhone performance or thermal evidence.

The recorded integration fixture disables reproduction and hazard drain to isolate
locomotion; sensing, controller, propulsion, energy costs and Metal are unchanged.
Its scripted input moves **only the food field**, never a cell, velocity or neural
output. It is enabled only in Debug using `ALIEN_MOBILE_FOLLOWING_CHECK=1`, and is
off in ordinary play. CUA source dragging still fails with `noWindowsAvailable`;
this proves visible physical following, not successful automated drag gestures.
The regular Phase 8 build is separately launched without the fixture. An observed
normal-play snapshot reached 35 births, five behavioral mutation events, 60 cells
and maximum attempted generation four (`phase8-simulator-normal.log`).

## Phase 8 inheritance and mutation

At a birth, the child receives the parent's actual genome, including previous
mutations. The native playable configuration gives each birth a 12% chance of one
localized controller/connection/motor/sensor mutation. A candidate parameter group
is selected from applicable genes. Gaussian perturbations are truncated at three
standard deviations and then clamped to the existing valid genome range:

| Parameter | Sigma | Absolute bounds |
| --- | ---: | --- |
| Neural weight | 0.08 | −4 to 4 |
| Neural bias | 0.04 | −4 to 4 |
| Parent-edge signal weight | 0.05 | −2 to 2 |
| Motor strength | 0.08 | 0 to 4 |
| Sensor range | 0.12 | 0.5 to 6 |
| Sensor sensitivity | 0.06 | 0.1 to 4 |

The existing small period/phase mutations and morphology mutations remain separate
and can occur in the same birth. Roles and constructor identity do not mutate.
Neural matrices are never randomized at birth. Some perturbations are neutral
because a weight is not currently exercised; no mutation is guaranteed beneficial.

The overall mutation event still produces the existing small hue drift, not random
recoloring. Developer logs now count behavioral mutation events separately. The
portable default keeps the rate at zero for legacy/frozen fixtures; UIKit enables
0.12 explicitly. Each physical child cell copies the child's edited gene at its
normal paid construction step.

## Validation

`behavior/phase8-tests.log` records the Release suite, with assertions enabled:
all five suites pass. The new `AlienMobileBehaviorMutationTests` checks:

- 12,000 deterministic, replayed mutation trials; every new parameter category
  changes, every value stays valid, and each accepted event changes at most one
  of the new behavioral parameters with the maximum step bounded.
- Morphology and roles stay unchanged when their mutation rates are zero.
- Parent A → mutated neural child B → actual mature grandchildren with mutation
  disabled: descendants and runtime cell parameters retain B, never revert to A.
- An exercised inherited neural-weight mutation changes the six-second physical
  COM trajectory by 0.0228725 world units under otherwise identical conditions.
  This proves an effect, not increased fitness.
- Combined body/behavior mutation survives an integration session: 21 births,
  three behavioral mutations and maximum generation seven in the recorded trial.

Existing signal, sensor rotation/range, oscillator, paid locomotion, zero-thrust,
CPU/Metal parity, construction, death, reset and long structural-session tests also
pass. The signed iOS Release and iPhone Simulator Debug builds succeed; logs are
`phase8-ios-build.log` and `phase8-sim-build.log`. Sanitizer results are retained in
`phase8-sanitizers.log`: all five suites pass with AddressSanitizer and
UndefinedBehaviorSanitizer (220.57 seconds).

## Remaining gates

Next in the original program are the matched viable/degraded-controller reproductive
selection experiment and subsystem performance checkpoint, then creature sensing,
short-range paid attack, raw-energy digestion, inherited pursuit and ecology.
None of those downstream results is asserted here.

Before final delivery, run the full eventual ecosystem on a physical iPhone and
record sustained FPS, thermal state, population/cell counts, session length and
human drag/pinch/fun observations. Simulator evidence does not replace that gate.
The previous phone installation and locked-launch result also do not validate this
new Phase 8 build on a physical device.
