# Post-M13 program: baseline and mutation inventory

This document preserves the starting audit and the mutation field inventory.
The final configuration, completed experiments and limitations are documented in
[M14_REPORT.md](M14_REPORT.md); historical baseline paragraphs below are not the
current app configuration.

The owner deferred all human-visible gates to the end of the program. Automated
and physical evidence must still be distinguished from human judgement. Stages
remain sequential; ecological failure is not a pass merely because code exists.

## Repository baseline (2026-09-13)

The working tree contains no committed mobile baseline: the entire `mobile/`
tree was untracked when this work began. Earlier reports are historical, often
contradicting later code. The actual normal app uses `depthPlaytestConfig()`:
72×72 torus, 1,200 physical cells, 5,000 motes, local cell death/fragmentation,
angular joints, 14 cell roles, local signals, development, structural mutation.
Default reset creates nine authored founders including four modular bodies,
feeders, a hunter, depot/defender and memory fixtures. It is not a primitive world.

Real implementations already present: bounded module calls, branches/repetitions,
paid incremental construction, raw/usable/embodied energy, physical interception,
local diffusion, attacks/digestion, memory/communication, CPU/Metal spatial grids.
There are no arbitrary morphology or novelty fitness bonuses in the traced paths.

Important limitations: physical connection graphs are genomic parent trees plus
derived joints/braces; all organs activate only at maturity; only the rooted
component can reproduce after damage; internal constructors are development calls,
not independent reproductive workers. All living cells intercept motes, including
single constructors. Stable abundant food can therefore favor tiny sessile bodies.

The previous mutation-only test repeatedly rejected sterile descendants before
continuing its artificial lineage. Its 32,000 attempts establish executable
variation, NOT ecological survival or persistent evolutionary novelty. Exported
examples were artificially fed while being grown. They must not be described as
naturally selected organisms. Previous reports explicitly left human gates open.

Existing M13 device logs do contain actual iPhone runtime measurements, around
1,170 cells, 60 FPS/120 ticks per second at the end of a roughly 270-second run,
with thermal state 2. This is historical short-run evidence, not a sustained
current-build performance pass. The stale README's 360-cell limit is incorrect.

## M14A mutation inventory

All fields below are copied with the parent's genome. Selection first chooses a
mutation category, then one gene uniformly and one node uniformly. Default rates:
neural .28, geometry .18, property .12, role .025, insertion .035, deletion .025,
gene duplication .012, gene deletion .006, section copy .018, move .012,
constructor .06; sum .773. At most one ordinary category is selected. If the sum
exceeds 1, probabilities are normalized by the sum. Blocked/no-op edits are not
reported as mutations. Conditional choices below describe the current code.

| Inherited field | Mutation and bound | Physical/behavioral consequence |
|---|---|---|
| genes/nodes | duplication/deletion/copy/move; ≤16 genes, ≤64 nodes/gene, ≤256 total | executable modules, cell layout and function; unused genes can be neutral |
| entryGene | index remapping only | developmental entry identity; deliberately no arbitrary entry switching |
| parentNode | insert/delete/copy/move remapping; parent precedes child | physical attachment and bidirectional signal route |
| relativePosition | geometry category, 70%; each axis ±.12, length .45001–2.09999 | cell spacing, orientation, reach, angular targets; root remains zero |
| stiffness | geometry, 10%, ±.2; .1–2 | spring and joint resistance |
| orientation | geometry, 10%, ±.18; ±6.284 | module rotation; previously non-entry orientations were frozen |
| phaseAdvance | geometry, 10%, ±.03; ±.5 | oscillator phase along repeated modules; inert without repeats/generators |
| neural weights (64) | neural subcategory, one ±.15; ±4 | replacement or residual local transfer matrix; unused channels can remain latent |
| neural biases (8) | neural subcategory, one ±.1; ±4 | replacement or residual local offset |
| signalWeight | neural subcategory ±.15; ±2 | genomic edge transmission; entry root has no incoming edge |
| selfWeight | neural subcategory ±.1; ±1 | previous-step recurrence; previously frozen in developmental mutation |
| neural | property toggle | residual relay vs replacement tanh matrix; previously forced on by every neural edit |
| role/constructorCell | role .025, uniform 14 roles; root retained | organ function; constructor flag synchronized; lost calls cleared |
| sensorRange | property ±.4; .5–6 | spatial sensing/broadcast/reception reach, sensor and sender costs |
| sensitivity | property ±.2; .1–4 | sensor gain and sensor cost |
| motorStrength | property ±.3; 0–4 | thrust or deformation magnitude; contractile/bending saturate at 1 |
| axisAngle | property ±.2; ±3.142 | body-relative sensor/thrust frame |
| motorMode | property uniform three modes | thrust, parent-edge contraction, or joint bending |
| motorChannel | property uniform eight channels | actuator signal input; previously frozen |
| bendingAngle | property ±.15; 0–1.2 | paid joint actuation amplitude |
| contraction | property ±.05; 0–.4 | paid spring rest-length modulation |
| period | property ±.4; .1–20 | local generator timing |
| phase | property ±.1; 0–1 | local generator phase |
| amplitude | property ±.08; 0–1 | generator output; previously frozen in developmental mutation |
| waveform | property uniform sine/square | generator waveform; previously frozen |
| extractionRate | property ±.2; .25–2 | contact extraction throughput and attacker costs |
| digestionRate | property ±.2; .25–2 | raw conversion throughput and digestion upkeep |
| storageCapacity | property ±.4; .5–6 | depot reservoir volume; inert in other roles |
| defenseStrength | property ±.2; .25–2 | paid local protection and upkeep |
| memoryTime | property ±.15; .02–2 | local delay/integration; discrete delay capped at 255 ticks |
| memoryMode | property uniform delay/integrate | previous signal retention |
| construction.targetGene | constructor subcategory, -1 through last gene | nested developmental invocation; constructors only |
| construction.branches | constructor ±1; 1–8 | repeated attached arms |
| construction.repetitions | constructor ±1; 1–16 | repeated sections |
| construction.angle | constructor ±.35; ±6.284 | module invocation rotation |
| construction.branchAngle | constructor ±.3; ±6.284 | arm spacing |
| construction.intervalScale | constructor ±.2; .1–10 | paid construction tempo |
| construction.repetitionAngle | final constructor choice on non-root, ±.2; ±3.142 | successive section rotation |
| construction.separateOffspring | final constructor choice on root, toggle | retain/release birth tether; non-root copies are currently inert |
| mutationRates (12) | inherited bounded probabilities; meta edits one rate | offspring mutation distribution; see M14D audit |

Property chooses each of its 19 subcategories equally. Constructor chooses one
of seven equally. Neural chooses one of four equally in either processing mode.
The residual relay is identity plus tanh(W input + bias), clamped to signal bounds;
zero weights/biases preserve the original relay exactly. Each matrix
entry or bias channel remains reachable. The earlier implementation disabled
working relays by setting neural=true without setting a usable matrix. Small
mutations now preserve that controller mode; toggling it is an occasional
property mutation and is allowed to be harmful.

Three existing scales are retained without raising one global probability:
small scalar/controller/geometry edits, occasional role/mode/terminal/development
edits, and rare coherent module/section operations. Copy operators copy controller
DNA with structure and remap attachment indices. Most edits touch one node or one
module parameter, leaving the rest of the ancestor intact. This is a mutation
spectrum, not a promise of viability or interesting forms at a fixed interval.

Remaining ineffectiveness is explicit: scalar organ parameters on other roles,
unused channels/modules, root edge weight, non-root release flag, and strengths
above 1 for deformation can be phenotypically silent. Dormant DNA is not forcibly
expressed or counted as observed novelty. A genotype change alone is not evidence
of a new behavior. Per-organ geometry/actuation marks already render real state;
no mutation badges or species announcements are added.

## Reference mechanisms

Reviewed ALIEN at commit `235ecc624bf62b45db9320538821454c7be72b3b`, particularly
[MutationProcessor](https://github.com/chrxh/alien/blob/235ecc624bf62b45db9320538821454c7be72b3b/source/EngineKernels/MutationProcessor.cuh).
It separates neuron/connection/property/geometry/mode changes from structural
operators, and initializes new neural nodes with identity transfer. This mobile
engine has a simpler tanh/pass-through representation; small edits now preserve
its existing processing mode instead of implicitly erasing signal transmission.
No ALIEN CUDA implementation or desktop UI was imported.

## Evidence log

`m14a/StructuralMutation.before.cpp` preserves the exact pre-change mutation code.
`m14a/tests.log`: initial corrected Release build, 17/17 tests, 106.10 s under
concurrent soak load. Includes 20,000 bounded localized mutation trials, formerly
frozen-field reachability, mode preservation, zero-rate inheritance, and all
existing physics/biology/Metal suites. Subsequent revisions require fresh tests.

The completed runs and subsequent fixes are indexed by M14_REPORT.md.
The observer CSV is diagnostic only and never feeds values back into the engine.
Human evaluation remains deferred, not passed.
