# Milestones 4–7 — implementation and evaluation

2026-09-13. Native iPhone / Metal ecosystem prototype.

**M4–M7 systems are implemented. The verdict is THE SYSTEMS WORK BUT NEED TUNING.**
The owner explicitly replaced the intermediate human gates with “write the code,
and test after … milestone 7,” and restricted execution to the iPhone Simulator
on this Mac. No intermediate human gate or physical-iPhone performance pass is
claimed. Feature development stops here for playtesting.

## 1. Milestone 4 result

Heritable behavioral mutation now covers neural weights/biases, genomic-edge
signal weights, motor strength, the existing local motor-axis angle, sensor
range/sensitivity, and oscillator period/phase/amplitude. Contractile amount and
rare motor-mode changes were added in M6.

Neural and motor/sensor parameter changes are localized, truncated Gaussian
perturbations, bounded to valid genome ranges. Neural row selection favors rows
already expressed in the inherited network 80% of the time while retaining access
to all rows. This is a generic genotype mutation rule, not a behavior policy.
One small parameter edit is attempted in 65% of births; geometry changes have an
8% probability, terminal addition 1.5%, removal 1%. A child inherits its actual
parent's genome, including earlier mutations. Tiny behavioral edits shift hue by
0.003–0.012 cycles; structural changes shift it by 0.025–0.060.

The 12-pair controlled functional/degraded-controller experiment completed 67
births versus zero, with successful reproduction in 7/12 versus 0/12 trials.
Five pairs defeated both controllers. That test retains the historical smooth
resource field to isolate controller selection; physical-mote foraging has its
own tests. A single exercised inherited weight edit changed a six-second physical
trajectory by 0.192735 world units in the recorded test.

Visible lineage-specific strategies have not received a human acceptance verdict.
Recorded frames show motion, different body arrangements, thrust, births, and
turnover; screenshots alone do not prove that a player can recognize behavioral
lineages reliably.

## 2. Milestone 5 result

The golden light emits 4 energy units/second as 0.16-unit motes: 25 motes/second.
Each mote has a stable ID, position, velocity, energy, and age. Motes originate
within the source radius, travel radially at 0.25–0.70 world units/second, and
persist up to 150 seconds. Moving the source leaves existing food behind.

Absorption requires actual cell/mote proximity: cell radius plus a 0.10-unit
capture radius. Cells overlapping the same mote share its finite energy according
to available capacity. Creature centers and source coordinates do not grant food.
Energy sensors sample the nearby mote distribution in their physical local frame;
the emitter position is absent from this sensor path.

Death returns 80% of remaining usable, raw, and embodied energy to motes. The rest
is recorded as dissipation. Expiration and allocation overflow are explicit
radiation sinks. Overflow never relocates new food to a distant existing mote.
The 3,000-mote cap bounds allocation. Reset discards all runtime particles and
recreates the finite, accounted 160-mote opening reserve.

The 48×48 world wraps. Springs, repulsion, cell orientation, sensors, attacks,
particles, and rendering use shortest wrapped displacement. Render instances
repeat across edges. The camera pans and zooms across a world larger than its
opening view.

Distributed physical food and source-history are implemented. Permanent crowding
has not been eliminated: the final recorded population visibly concentrates near
the emitter, with some peripheral bodies. It has not received a human acceptance pass. The source still
attracts concentrations; wider ecological use of the world needs playtesting.
No separation steering, wander AI, or exploration reward exists.

## 3. Milestone 6 result

Two motor modes exist: Thrust and Contractile. Thrust applies a paid force at the
actual motor cell along its physical parent-edge orientation plus inherited angle.
Contractile activation changes the designated parent spring's target around its
immutable baseline, by at most 40%. Zero activation or no energy restores the
baseline. Active contraction consumes energy; Metal receives fresh spring targets.
Flexible joints are not locked by fixed triangular braces.

The flexible feeder has a generator and contractile tail in addition to its
thrust/sensor body. Under identical inherited tonic motor signals, changing only
tail geometry produced displacements of 3.02523 and 2.99109 units over eight
seconds, with a 0.357533-unit trajectory difference. In separate two-minute
physical-resource trials, the ordinary feeder completed 107 births and the
flexible feeder 62. Both strategies can acquire resources and reproduce.

Internal contraction alone cannot propel center of mass under this engine's
uniform mass and isotropic drag. The new strategy combines deformation with real
external thrust; no artificial swimming force was added. “Pure contractile
swimming” is not claimed.

Motor-mode mutation is attempted at 0.8% of ecosystem births and selects a random
node, so actual mode changes are rarer. Structural limits allow up to ten nodes.

## 4. Milestone 7 result

CreatureSensor detects the nearest foreign physical cell within inherited range,
with deterministic tie ordering. It outputs only body-relative X/Y direction and
intensity. It does not return species, genome, fitness, or a target ID.

An activated Attacker spends usable energy and can extract at most 0.85 energy
units/second within 0.90 world units, subject to intervening-cell obstruction.
It drains real usable/raw/embodied victim resources into raw energy. An attached,
unfinished child is excluded from its parent's sensor and attack queries.
Detached organisms, including relatives, can be attacked; cannibalism is possible.

Raw energy circulates within the body but cannot pay construction or motor costs.
A physical Digestor converts up to 0.90 raw units/second at 85% efficiency, bounded
by actual raw supply and usable capacity. Attacker maintenance costs 0.14 usable
units/second and digestor maintenance 0.06; activated attacks cost an additional
0.025 × activation per second. These costs apply by organ, never by a predator flag.

The manually authored six-cell hunter has a constructor, creature sensor, two
stronger thrust motors, attacker, and digestor. Its pursuit policy is the inherited
neural matrix. Reset seeds three ordinary feeders, one flexible feeder, and one
hunter. No organism or lineage is respawned during play.

In the closed, finite, energy-rich prey fixture with no emitter, the complete
hunter captured 30.2672 energy units, digested 25.7271, and completed one birth.
Disabling attack or digestion prevented births. The fixture uses stationary,
energy-rich prey with reproduction held in cooldown to isolate the energy pathway;
it is not the default ecosystem or a claim of reliable hunting in every encounter.
In the sparse-prey ablation, disabling sensing or motors reduced captured energy;
disabling attack prevented extraction, and disabling digestion prevented conversion.

In the environmental-only control, the attack-disabled hunter machinery completed
18 births before extinction at two minutes, versus 107 births and 41 living
organisms for the feeder. Environmental food can finance a transient burst;
there is no absolute species-specific food restriction. Sustained predator
persistence remains a tuning limitation.

Final ten-minute CPU runs:

| Experiment | Completed births | Deaths, including unfinished bodies | Final population / cells | Hunter births / maximum generation | Captured / digested energy |
| --- | ---: | ---: | --- | --- | --- |
| Seed 0, stationary source | 597 | 879 | 49 / 142 | 8 / 3 | 60.1493 / 51.1269 |
| Seed 1, source moved at 120 s | 487 | 721 | 35 / 108 | 14 / 4 | 83.1033 / 70.6378 |
| Seed 2, hazard moved at 180 s | 38 | 72 | 0 / 0 | 17 / 4 | 121.616 / 103.374 |

All three lost the hunter lineage before ten minutes. Two retained feeders;
one became entirely extinct. Earlier tuning produced predator domination and
cannibalism, but that is not presented as the final configuration's outcome.
The interventions above belong only to test fixtures; normal play moves neither
source automatically.

## 5. Physics fidelity audit

No creature-level pursuit, velocity, translation, or heading controller was added.
Bodies remain physical cell networks. Torque follows motor placement, connected
geometry, spring deformation, and collisions. The playable ecosystem disables
the historical birth-separation impulse. No hidden long-body or hunter movement
bonus exists. Birth placement is genomic construction, not locomotion.

CPU/Metal contractile plus toroidal parity error after 720 steps was
0.00000408466 world units. Metal integrated runs exercise changing spring targets,
construction, death, and topology rebuilds using the same renderer/physics kernel
as the app.

## 6. No-scripting audit

Search log: `behavior/m7-no-scripting-audit.log`. The requested search terms match
only comments about avoiding invalidated references and order-dependent resource
allocation in production sources. There is no seek/chase/flee/steering function.

Remaining explicit special cases:

- Authored founder genomes and deterministic reset positions/initial resources.
- Generic sensor/organ dispatch by cell role.
- Exclusion of a parent's still-attached unfinished offspring.
- Test-only environment interventions and Debug-only historical following fixture.
- Legacy smooth-food and birth-impulse configurations retained for old tests;
  neither runs in the playable ecosystem.

## 7. Genome model

Inherited: node topology/geometry, constructor identity, roles, neural enablement,
weights, biases, signal weights, self-weight, motor channel/mode/strength/axis,
contractile amount, sensor range/sensitivity, oscillator period/phase/amplitude/
waveform. Cells copy their creature's corresponding gene at construction.

Mutable: weights, biases, signal weights, motor strength/axis/contractile amount,
sensor range/sensitivity, oscillator period/phase/amplitude, geometry, terminal
addition/removal, and rare motor modes.

Intentionally fixed in this iteration: constructor/role identity, neural enablement,
channel layout, motor channel, self-weight, and waveform. Attacker, digestor, and
creature-sensor terminals are protected from ordinary deletion. New terminal
nodes begin structural. Broad role mutation and evolved feeder threat sensing
were not added.

## 8. Energy accounting

Initial organisms and initial environmental reserve are recorded separately.
External emission enters the pending emitter balance, then spatial motes. Contact
moves mote energy into usable cell energy. Upkeep, hazard exposure, and paid
actuation leave through recorded expenditure. Construction transfers paid energy
into the new cell's usable and embodied accounts. Attacks transfer existing
victim energy into raw energy; digestion converts it with an explicit 15% loss.
Death returns a configured fraction to motes. Lifetime/cap losses are recorded.

For each ten-minute run, tests sum environmental energy, pending emission, all
living usable/raw/embodied energy, expiration, dissipation, organ costs, and
digestion loss against initial reserves plus external input. Final residuals were
0.0165993, 0.0130581, and 0.00115218 units out of approximately 2,441.2 supplied.
These are floating-point accumulation errors, not an unaccounted energy source.

## 9. Test results

Baseline: all five pre-change Release suites passed in 8.47 s; all five original
Debug ASan/UBSan suites passed in 188.93 s. Simulator Debug and signed physical-
device Release builds succeeded. The baseline device installed but lost its
CoreDevice connection before launch; subsequent owner instructions restricted
all execution to the Simulator. The baseline Simulator repeated food following,
births, mutation, death, and reset coverage. The isolated four-cell body closed
to approximately 0.20 units after source movement and then overshot.

Final Release: **9/9 passed**, 45.50 seconds, assertions enabled in test executables.
Final ASan/UBSan: **9/9 passed**, 144.55 seconds, with AddressSanitizer and UndefinedBehaviorSanitizer.

Suites:

1. AlienMobileLocomotionDiversityTests — matched signals/different geometry,
   successful physical-mote foraging for both body strategies, 5,000 replayed
   combined mutations including motor mode, contraction, amplitude, inheritance.
2. AlienMobileHuntingTests — sensor/motor/attack/digestor ablations, closed prey-
   energy reproduction, environmental-only economic comparison.
3. AlienMobileEcologyTests — emission, proximity, competition, recycling, mote
   sensing, wrapping, contraction cost, attacks/digestion, whole-system energy
   accounting, three ten-minute integrated seeds, IDs/topology/finite values/reset.
4. AlienMobileBehaviorSelectionTests — 12 paired controller trials and paid organs.
5. AlienMobileBehaviorMutationTests — 12,000 replayed bounded mutation trials,
   exact ancestor/child/grandchild inheritance and trajectory effects.
6. AlienMobileBehaviorTests — signals, oscillators, local sensing, physical thrust,
   and inherited controller construction.
7. AlienMobileCoreTests — existing foundational invariants and construction tests.
8. AlienMobileSessionTests — existing long sessions and three additional ten-minute
   M4 behavioral/morphological soaks.
9. AlienMobileMetalTests — parity, contractile/toroidal parity, two 120-second
   ecology/reset cycles, three historical growth/death/reset cycles, timing.

Final logs: `behavior/m7-final-tests.log`, `behavior/m7-final-sanitize.log`,
`behavior/m7-final-sim-build.log`. An earlier unoptimized sanitizer run was stopped
when tuning changed; the final run uses optimized RelWithDebInfo plus ASan/UBSan.

## 10. Physical iPhone performance / Simulator measurements

Physical iPhone: **not measured for this build**, per the Simulator-only instruction.
No iPhone thermal, memory, sustained-FPS, or touch-performance claim is made.

Simulator: iPhone 17 Pro / iOS 26.5 on this Mac. A captured measurement window covered 268.625 simulated seconds (about 4.5 minutes).
Median FPS was 60.0 (one-second samples 51.41–60.34); median TPS was 120.0
(samples 117.7–120.4). Peak population was 56 bodies / 168 cells, with up to 317
motes and 10 attacker-bearing bodies. Resident memory ranged 152.0–222.9 MB.
Reported thermal state stayed 0. Median reported biology time was 0.321 ms,
Metal kernel time 0.125 ms, render CPU time 1.176 ms, and render GPU time 0.151 ms.
The measurement snapshot is `behavior/m7-final-performance.json`; the log is
`behavior/m7-final-playtest.log`.
Simulator thermal state is the host process's reported state, not a phone thermal
measurement. Biology/synchronization values are cumulative per-step averages;
GPU and render timings are recent samples. Memory is process resident size.

Final CPU soak peaks were 167/159/121 cells and 287/764/3,000 motes. Mean biology
cost was approximately 0.0634/0.0747/0.0160 ms per step in those runs. The last
low mean includes extinction. These profiles did not justify a spatial grid or
GPU binning; straightforward local scans remain. The O(N²) collision kernel has
not been validated at phone scale on physical hardware.

## 11. Visual result

Black field, luminous connected cells, lineage colors, force-driven motor exhaust,
subtle gold resources, actual contracting connections, small organ center marks,
short energy-transfer glows, and digestion brightness pulses. No numbers, labels,
new controls, predator mode, or dashboard were added to the playing surface.

`behavior/m7-final-playtest.mp4` records the final Simulator configuration;
`behavior/m7-final-review.mp4` is a smaller viewing copy.
`behavior/m7-final-sequence.png` contains sampled frames. Earlier `m7-ecosystem.mp4`
is explicitly an earlier tuning run, not evidence for final parameters.

## 12. Observed emergence

Established by tests: controllers affect reproduction; morphology affects motion;
both feeder body strategies reproduce on spatial food; contact capture and
digestion can finance a birth; hunters reproduce through multiple generations;
source/hazard interventions change seeded ecological outcomes; extinction is real.

Not established: indefinite predator/prey coexistence, evolved fleeing, robust
regional refuges, consistently recognizable behavioral lineages, or the owner
wanting to keep experimenting. Random movement is not reported as hunting. The
hunting claim rests on contact energy transfer and causal subsystem ablations.

## 13. Human playtest build

Final app: `mobile/build-sim/Release-iphonesimulator/AlienMobileApp.app`.
It is installed and running in the existing Simulator. Open Simulator on this Mac.
To rebuild/install/restart from the repository root:

```sh
./mobile/scripts/run-simulator.sh
```

The script defaults to Simulator ID `6663A26B-E595-4E16-AA40-027076B713FA`.
Bundle ID: `com.example.AlienMobilePrototype`. Reset restarts the deterministic
experiment. No device provisioning is needed for this Simulator build.

## 14. Human playtest plan — 25 minutes

- 0–5 min: Watch births, compare the four-cell feeders and flexible tail founder,
  and move the light a few body lengths. Watch for delays and overshoot.
- 5–10 min: Move the light farther. Look for food left behind, organisms following
  different patches, and bodies leaving the original concentration.
- 10–15 min: Reset if hunters are extinct. Zoom into the violet founder's front
  cell. Look for pursuit ending in actual contact/transfer and subsequent growth.
- 15–20 min: Move the hazard across a dense patch; leave another patch available.
  Observe which branches survive and whether hunters find new organisms.
- 20–25 min: Reset and try a different source/hazard path. Compare outcomes.
  Judge whether branches look behaviorally distinct and whether interventions
  feel consequential. Predator extinction is a result to evaluate, not repaired.

## 15. Independent verdict

**THE SYSTEMS WORK BUT NEED TUNING.**

The causal chain is real and tested through prey-funded reproduction. Physics,
inheritance, resource accounting, and interventions form a working prototype.
Predators still disappear within ten minutes in all three final CPU seeds, and
crowding/visual lineage recognition remain human-evaluation questions. That is
not sufficient evidence to call the ecosystem magical or the player loop solved.
No additional biological feature is started after this build.

## ALIEN reference review

Reviewed the current [ALIEN repository](https://github.com/chrxh/alien) at commit
`235ecc624bf62b45db9320538821454c7be72b3b`: MutationProcessor, MuscleProcessor,
SensorProcessor, NeuronProcessor, GeneratorProcessor, AttackerProcessor,
DigestorProcessor, EnergyProcessor, ObjectConnectionProcessor, Map, Entities,
and Genome, plus corresponding mutation/sensor/muscle/attacker/digestor/generator
tests. Relevant concepts retained are inherited bounded parameter changes,
physical local actuation, spatial energy, raw/usable separation, and wrapped
queries. This is an independent small C++/Metal implementation, not a CUDA port.
