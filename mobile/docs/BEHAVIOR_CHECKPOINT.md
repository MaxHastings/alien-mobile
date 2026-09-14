# Behavioral program checkpoint — 2026-09-13

Historical Phase 7 checkpoint. See [Phase 8](PHASE8.md) for the now-passed Simulator
visual gate, behavioral mutations and the outstanding physical-device gate.

**Status: implementation through Phase 7; its interactive fun gate is unresolved.**
This is a partial checkpoint, not delivery of the requested predator/prey ecosystem.
The signed feeder build is installed on the physical iPhone 13. Launch was rejected
because the phone is locked. Simulator renders the new founder, but automated source
dragging fails with `noWindowsAvailable`. No downstream gate is claimed passed.

## 1. Foundational changes

- `Behavior.h/.cpp`: five roles (Structural, Constructor, Generator, Motor,
  EnergySensor), eight fixed channels, cell-local evaluation, physical orientation,
  field receptors, and paid actuation.
- Genome nodes own role, oscillator, orientation, motor, connection, sensor and
  8×8 neural parameters. Runtime cells copy these at actual construction, retain
  a node index, and own current/next signals and oscillator age.
- Incoming signals accumulate across genomic parent edges. Braces are mechanical;
  construction tethers do not carry signals between organisms. Every cell reads
  the old signal snapshot, then all cells commit together. Unfinished bodies are
  dormant. The optional transform is `tanh(W input + bias)` with an inherited
  self-input coefficient. There is no organism-level brain function.
- CPU prepares one immutable per-cell thrust snapshot for every Metal submission.
  The Metal physics kernel adds it to spring/repulsion forces before integration.
  The CPU reference integrates the identical force. No new velocity or position
  writes occur in the behavior code.
- Construction now checks the Constructor role. The existing `constructorCell`
  genome field is retained for compatibility and validated against that role.
- Creature sensing, attackers, raw energy and digestors are **not implemented**.

## 2. Why these choices

Reference: [ALIEN](https://github.com/chrxh/alien), local reference revision
`235ecc624bf62b45db9320538821454c7be72b3b`.
Reviewed GeneratorProcessor, MuscleProcessor, SensorProcessor, NeuronProcessor,
AttackerProcessor, DigestorProcessor, MutationProcessor, Entities and Genome, plus
corresponding generator, neuron, muscle, sensor, attack, digestion and neuron
mutation test semantics. Those CUDA suites were inspected, not executed here.

Retained periodic heritable signals, weighted connection input, separate future
neural activity, inherited matrix/bias ownership, body-relative actuation and
sensing, and the later raw/usable energy distinction. ALIEN's direct movement
updates a cell velocity; this version submits a force to Metal as your brief
requires. It uses sine/square waves, a small FP32 CPU matrix, two rings of 16 field
samples, no sensor target IDs, no memory neurons, and no CUDA framework. Predation
semantics have only been studied; no attack or digestion behavior is faked.

## 3. Current genome model

Heritable: existing tree morphology and constructor identity; role; waveform,
period, phase, amplitude; motor strength/channel/axis; parent-edge signal weight;
sensor range/sensitivity; neural enablement, self coefficient, weights and biases.
Each descendant owns its genome copy. Tests construct mature grandchildren with
modified controller values and verify both genomes and physical cell parameters.

Currently mutable: existing edge geometry and terminal additions/removals, plus
small bounded oscillator period/phase perturbations. Oscillator mutations retain
ancestry, and unmutated copies inherit the changed period/phase. Neural weights,
biases, motor strength and sensor parameters are inherited but **not yet mutated**;
that is Phase 8, after the interactive gate. Role mutation remains disabled.

## 4. Current behavior model

Completed GPU state → spatial energy/upkeep → old neighbor signals + local field
samples + optional own signal input → cell-local transform → simultaneous signal
commit → motor activation → energy debit and thrust → Metal motion.

The feeder has a constructor, an energy sensor and two motors. Their direction
comes from actual physical parent edges rotated by inherited local angles. Unequal
neural biases break exact left/right symmetry; local direction weights produce
crude turning. These constants are in `makeFeederGenome`, not in runtime steering.
Morphology changes the physical reference and lever arms. Missing/coincident
reference edges disable thrust. Negative activation closes the one-way propulsor.

## 5. No-scripting audit

No seek, chase, flee, target steering, predator class or hunger policy was added.
The sensor samples the smooth resource field at finite receptor positions; it does
not hand a controller the source coordinates. Signals carry local X/Y and intensity.
The only policy is the founder's inheritable matrix and biases.

Existing non-neural behavior remains explicit: spatial resource collection,
metabolism, energy sharing, threshold/timer construction, random birth orientation,
starvation removal, and the pre-existing small detachment impulse. The impulse is
not evidence of locomotion: propulsion tests suppress births and start at rest.
Initial velocity remains in legacy structural fixtures; the playable feeder starts
at rest. Braces and world-boundary forces are mechanical rules outside the genome.

## 6. Energy ecology

The existing source supplies finite energy, apportioned by actual cell exposure.
Every cell pays upkeep and hazard drain. Bodies share their remaining reserve.
Motors pay `positiveActivation × strength × dt × 0.035`; insufficient energy scales
both expenditure and force down together. Exhausted cells cannot propel themselves.
Construction retains its existing threshold and per-node debit; child starting
energy cannot exceed the amount paid. Whole-body starvation/death remains intact.
Generators, sensing and neural evaluation currently have no additional charge.
Attack costs, captured raw energy, digestion and hunting-supported reproduction are
absent. This checkpoint is resource-feeding life, not a predator/prey ecology.

## 7. Test results

Reproducible logs are in `docs/behavior/`; untouched-stage baseline logs are
`docs/behavior-baseline-host.log` and `docs/behavior-baseline-ios.log`.

- Baseline: 3/3 host suites passed, with real Metal reproduction/death/reset, and
  iOS Release built successfully before biology changes.
- Current Release: 4/4 suites pass (8.51 s in the retained run); assertions enabled.
- Signal tests: two-pass propagation delay, reversed connection traversal, zero
  connection weight, deterministic neural evaluation and unfinished-body silence.
- Generator: waveform/period/phase checkpoints, deterministic output, bounded
  period/phase mutations and inherited mutated values.
- Motor: enabled vs disabled COM displacement, exact energy accounting, zero-energy
  force suppression, missing/coincident reference handling, changed morphology,
  and real Metal locomotion with CPU parity.
- Sensor: food on all four sides, body-only 180° rotation, whole-scene rotation,
  finite field footprint and correct local output.
- Actual paid construction preserves changed controller/motor genes through mature
  grandchildren; legacy mutation, hazards, finite supply, death and reset tests pass.
- The current iOS device Release build is signed and installed. iOS Debug and
  Simulator Debug builds are also checked; see their build logs.
- AddressSanitizer + UndefinedBehaviorSanitizer: 4/4 suites pass (156.07 s).
  The last added directional/reset checks are also rerun in the sanitized behavior
  target; see `sanitizer-final-behavior.log`. These are host tests, not phone tests.

Measured oscillator fixture: 1.3953 world-unit displacement over six simulated
seconds; disabled body displacement zero; motor expenditure 0.105 energy. Metal
COM differed from CPU by 0.000000274 world units. Legacy 1,000-step Metal maximum
position/velocity errors remain approximately 0.00000265 / 0.00000118.

The four feeder trials start 4.25–5.01 world units from food and reach closest COM
distances of 0.0127–0.1001 over 40 simulated seconds. These are isolated physics
trials, not proof of optimal navigation. Food behind at the edge of detection can
still be missed; finite sensor range is intentional.

A five-minute simulated feeder session, with food moved by (1.5, −1.5) at one minute,
ended with 61 cells, peak 76, 115 completed births, 121 removals (including unfinished
children) and maximum attempted generation 7. It exercises inherited morphology
with fixed controller weights; it is **not** the Phase 9 behavioral selection test.

Host timings at the baseline: 240-cell CPU physics about 0.113 ms; Metal submission
plus synchronous readback about 0.318 ms. The final retained 240-cell Metal sample
is about 0.534 ms, with substantial run-to-run host variation (other runs near
0.33 ms). Per-subsystem profiling and phone timing remain outstanding; no spatial
grid or Metal repulsion rewrite was justified or performed at this checkpoint.

## 8. Physical iPhone results

Device: physical iPhone 13, iOS 26.6.1. Initial installation lost its tunnel;
subsequent attempts installed successfully, including the final Release build.
Launch returned `Locked: Unable to launch ... because the device was not ... unlocked`.

Actual FPS: **unmeasured**. Thermals: **unmeasured**. Running population/cells:
**unmeasured**. Session length: **no launched physical-device session**. Do not
substitute host or Simulator measurements. Developer-only logging is ready to
record frame rate, iOS thermal state, population, lineage, births/deaths and motor
expenditure when the unlocked phone can launch.

## 9. Observed behavior

Automated physical simulation demonstrates periodic propulsion, morphology-dependent
motion, directional approach using inherited sensor/controller/motor wiring,
controller inheritance, and continued births after a moderate food move.
Simulator inspection shows the four-cell feeder, physical connections and existing
food/danger controls. Interactive following has **not** been visually verified.
No pursuit, hunting, behavioral selection, prey adaptation or population cycles
have been observed or claimed.

## 10. Visual changes

Actual motor force subtly increases the cell's lineage-colored brightness.
Generators brighten during the positive half-cycle. Existing luminous cells,
physical connections, energy brightness, newborn growth and starvation fades remain.
No role palette, text, neural editor or extra toolbar was introduced.

## 11. Current fun loop

The human drags food a moderate distance, watches whether connected bodies can
reorient and reach it, and compares them with descendants whose shape differs.
Moving food farther or into danger can cause failure. Pan/zoom, 1×/2× and reset
remain the other controls. Whether this is understandable and worth repeating is
still an experimental question, not a demonstrated result.

## 12. Known limitations

The requested ecosystem is incomplete. Phase 7's interactive gate is unresolved;
Phases 8–22 are not claimed delivered. Sensor receptors approximate a field gradient
and can miss small/distant fields. Steering overshoots and can fail outside range.
Only one propulsor mode exists. Signals have no separate timestep scheduler or
refractory memory. Mechanical braces do not carry signals. Cells copy fixed-size
parameters, increasing memory; actuation snapshots allocate a buffer per submission.
CPU signal traversal is intentionally simple. Existing instantaneous body energy
sharing, whole-creature death, 240-cell cap and birth impulses remain approximations.
No sustained phone performance or human enjoyment evidence exists yet.

## 13. Human playtest instructions — 15–30 minutes

1. Unlock the connected iPhone and open ALIEN Mobile, or let the agent relaunch with
   diagnostics. Confirm the four-cell feeder is present after reset.
2. Minutes 0–3: watch a complete birth; zoom close enough to distinguish individual
   cells and the brighter working motors. Do not explain a food-seeking story first.
3. Minutes 3–8: move the gold food ring roughly one body length sideways. Record
   whether a person can identify turning and forward movement. Repeat behind a
   body. Record misses and overshoot as well as success.
4. Minutes 8–12: move food farther away, then reset. Try a gentler sequence of moves.
   Record whether the person predicts which bodies will reach the field.
5. Minutes 12–15: move danger toward the feeding area, test pinch/pan and 2×, then
   reset. Ask the person to describe what changed without offering an explanation.
6. Optional minutes 15–30: let the person freely repeat experiments. Record whether
   they voluntarily continue, which gestures frustrate them, and heat/frame pacing.

For this gate, evidence needed is visible, comprehensible food-following and a
usable phone interaction. It does not certify behavioral selection or predation.
Only after it passes should Phase 8 controller mutation begin.

## 14. Independent verdict

**BEHAVIORAL LIFE IS PROMISING BUT NEEDS TUNING** — provisional, based on physical
simulation tests. Human fun and sustained iPhone performance remain unassessed.
This verdict does not assert that the full ecosystem is delivered.

## 15. Next-phase boundary

Do not add behavioral selection, creature sensors, attacks or digestion until the
current food-following gate is assessed. Resume from this checkpoint after the phone
is unlocked and the interaction is observed. The next implementation work is bounded
heritable controller mutation, followed by the explicit A/B reproductive selection
experiment and performance checkpoint, in the order of the supplied brief.
