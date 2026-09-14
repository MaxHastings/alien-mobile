# Depth program checkpoint — M8/M9 and M10 mechanics

September 13, 2026. **The complete M8–M13 program is not finished.** M8 and M9 have code, tests, and physical phenotype evidence. M10's mechanics pass numerical tests; its explicitly requested human movement gate is pending. M11–M13 have not been advanced past that gate. This is not the final M13 playtest build.

## 1. Baseline audit

The inherited genome was one flat, backward-parent-indexed tree. Normal play capped it at ten nodes and the world at 360 cells. Construction interpreted each node exactly once. Motors and signals used genome indices as physical-cell indices, making repeated modules impossible without changing those consumers. Mutation protected attacker, digestor, and foreign-sensor leaves from deletion. Spring triangulation approximated preferred geometry; there were no explicit angular constraints. Uniform drag prevented internal contraction from providing net thrust. Energy redistribution and death were organism-wide. CPU queries and GPU repulsion were pairwise; topology changes rebuilt GPU buffers. Rendering already showed physical cells, spring connections, motes, lineage hues, and local transfer flashes.

Per the owner's follow-up, baseline playtesting was skipped. Already-started validation finished: 9/9 host suites, 9/9 ASan/UBSan suites, simulator Release build, and unsigned physical-device Release compilation. See `depth/baseline-*.log`. No baseline human playtest or baseline physical-device performance claim is made.

## 2. M8 — developmental genome

`Genome` now owns `genes[]`. Each gene owns nodes, orientation, and a repeated-instance phase advance. Constructor nodes reference genes and specify branch count, repetition count, branch and repetition angles, orientation, and interval scaling. The entry constructor also controls whether a completed offspring remains tethered or separates.

`DevelopmentCursor` is a resumable invocation stack. Every invocation keeps its own mapping from local gene-node indices to realized physical nodes. Nested calls, branches, and repeated sections reuse the inherited DNA. The genome is never permanently replaced by a flattened phenotype. Only realized topology is retained on the organism, plus one pending construction result. Controllers and motors follow that realized topology.

Limits: 16 genes, 64 nodes per gene, 256 encoded nodes total, eight nested invocation levels, and 128 resulting cells in normal development. Invalid references and topology are rejected. Recursive calls are supported but exceeding a bound fails development rather than silently producing a truncated viable organism. A bounded dry run reserves offspring capacity before construction. Failed offspring attempts do not copy the parent's DNA as a repair.

New gameplay founders start as one physical root. Every additional cell uses timed construction, debits energy, attaches physically, and is initially dormant. Organs activate when development completes. Test energy supplementation is confined to isolated host fixtures; it is absent from normal play.

| Reference | Genes | Encoded nodes | Physical cells |
|---|---:|---:|---:|
| Repeated segment | 2 | 5 | 26 |
| Radial branches | 2 | 5 | 38 |
| Nested branching | 3 | 8 | 29 |
| Repeated appendages | 3 | 7 | 50 |

M8's archived physical snapshots, taken after incremental growth and ten seconds of CPU physics, are in `depth/m8-physical-phenotypes.png`. These are authored references, not evolved organisms. Founder actuator parameters were subsequently extended for M10.

Intentional simplification: nested calls grow attached modules. Separation is an entry-constructor offspring policy, not ALIEN's full system of independently reproducing internal constructors. Arbitrary loop closure/reconnection is not implemented.

## 3. M9 — mutation language

Normal depth play uses heritable, bounded mutation rates for neural weights/biases/signal weights; geometry and connection stiffness; organ properties; non-root roles; node insertion/deletion; gene duplication/deletion; section copying/moving; constructor references, branching, repetition, angles, timing and root separation; and subtle meta-mutation. Sensor range, motor strength, oscillator properties, extraction rate, and digestion rate are mutable. Functional extraction/digestion improvements incur matching maintenance and action costs. Bending mode and amplitude participate after M10.

Node operations remap local parent references. Gene deletion remaps surviving references and disables calls to deleted genes. Only the reproductive root is protected from role loss. The earlier protected hunting-organ deletion list was removed, including from the legacy mutation path. Normal play has no protected feeder/predator identities.

Lineage IDs now diverge after accumulated genetic change crosses a threshold. Smaller edits preserve ancestry with restrained hue drift. This distance is for ancestry visualization only; it is not fitness or a gameplay stat.

Final deterministic experiments:

- 24,000 forced structural/controller operations; all operators exercised. 616 bounded development failures, with no invalid memory or invalid genome references.
- Eight mutation-only lineages, 4,000 attempts each: 32,000 attempts, 22,472 changed descendants, 36 development-incompatible attempts.
- 95 distinct resulting cell counts, spanning **1–128**; **1–16 genes**; all **eight implemented roles**.

The lineage experiment retains development-capable descendants. It is not an ecological survival experiment and does not establish open-ended evolution. Four actual descendant genomes were separately grown cell by cell in supplied-energy fixtures and then simulated physically. Their layouts differ in branching, sizes, and role placement. See `depth/m9-tests.log`, `depth/evolved-*.txt`, and `depth/evolved-*.csv`.

## 4. M10 — physics

Articulated play replaces automatic triangular braces with three-cell angular constraints around inherited rest geometry. Spring stiffness is inherited. Angular restoring forces and damping act equally and oppositely on the participating cells. Metal stores each joint in the adjacency lists of its three participating cells, avoiding shared writes and floating-point atomics. CPU and Metal use corresponding equations and input snapshots.

Bending changes a joint's target angle around its inherited baseline. Contraction changes spring rest length. Direct propulsion remains a separate local motor mode. Bending pays for activation, excursion, and stiffness; insufficient energy reduces actuation. Repeated-instance oscillator phase variation can generate temporally staggered actuation without a creature controller.

Measured fixture results:

- Same controller, changed morphology: center-of-mass motion differed by **2.22739 world units** over five seconds.
- Same body, changed motor placement: motion differed by **2.14615 world units**.
- Tested stiffness factors **0.1, 1, 2** with thrust, contraction, and bending for twenty seconds each. Maximum observed cell speed was **2.89781 world units/s**, without non-finite state.
- Internal-force momentum test residual: **5.63505e-7**.
- CPU/Metal angular+bending position error after 1,200 steps, with living cells retained throughout: **5.73948e-6** at stiffness 0.1 and **5.16338e-6** at stiffness 2.
- Zero available energy restores baseline joint targets and produces no paid actuation.

These establish mechanical differences, not a human verdict on movement readability. **M10's human gate remains pending.** Crawling is deferred because this world has neither a contact substrate nor a hydrodynamic model. Internal bending/contraction must not be described as demonstrated free swimming.

## 5–7. M11, M12, M13

Not advanced. Local attacks still drain cell reservoirs but death remains organism-wide; robust local removal and fragmentation are not implemented. No depot, defender, memory, or communication cells have been added. No physical-iPhone scale profiling or sustainable 1,000-cell claim is made. The 360-cell and 3,000-mote limits remain.

## 8. ALIEN comparison

Reference: [ALIEN](https://github.com/chrxh/alien), inspected at commit `235ecc624bf62b45db9320538821454c7be72b3b`. Its [genome](https://github.com/chrxh/alien/blob/235ecc624bf62b45db9320538821454c7be72b3b/source/EngineKernels/Genome.cuh) and [constructor helper](https://github.com/chrxh/alien/blob/235ecc624bf62b45db9320538821454c7be72b3b/source/EngineKernels/ConstructorHelper.cuh) informed reusable genes, branches, concatenations, and separate developmental state. Its connection and muscle mechanisms informed local angular actuation. This implementation preserves physical cell networks, local signals, paid construction, and inherited structure. It remains simpler in constructor concurrency, shape language, topology, materials, metabolism, cell functions, and scale. No CUDA implementation was ported line for line.

## 9. Special-case audit

Founder functions still have historical descriptive names, but those functions only author genomes. Normal runtime behavior dispatches on local organs, connectivity, signals, resources, and physics. No predator/prey classes, colors-as-species checks, creature speed/armor/health/fitness statistics, or morphology bonus tables were added. Legacy fixture configurations retain smooth food fields and a birth separation impulse; normal depth play uses physical motes and no separation impulse.

## 10–11. Genome examples and observed behavior

The four exported descendants come from seed 1/generation 380 and seeds 2, 3, and 4/generation 151. Their files include developmental instructions and organ/controller values. The CSVs contain actual cell positions after supplied-energy development and five seconds of physics, not merely unexecuted genome coordinates.

A three-minute normal-resource CPU run initially reached 38-cell articulated bodies but ended dominated by small organisms. Widening the resource patch to radius 6 and increasing its emission to 12 energy units/s kept 38-cell bodies present through the three-minute experiment. At 170 seconds that run had 44 organism records, 27 mature organisms, 225 cells, 814 motes, and 135 completed births. This is the reason for the new environmental default; it applies uniformly to all organisms. See `depth/articulated-session.log` and `depth/broad-resource-session.log`.

Observed: cell-by-cell growth, larger connected bodies, changed descendant layouts, local actuation, thrust, births, resource uptake, and continued large-body presence in the broader patch. Not established: diverse long-lived ecological strategies, cooperation, migration, open-ended novelty in ecological play, fragmentation, storage-based survival, or satisfying human play.

## 12. Visual result

Connections are dimmer, organism luminance varies subtly while lineage hue stays dominant, and detailed role marks are suppressed at distant zoom. The initial depth view is wider. Cells remain physical geometry rather than replacement organism sprites. The current simulator capture is `depth/m10-simulator.png`; the motion recording is `depth/m10-simulator-motion.mp4`. These do not substitute for the requested human movement gate.

## 13. Tests and builds

Final host Release: **12/12 suites passed**, 72.60 seconds. Assertions remain enabled. Detailed numerical output is archived in `depth/final-host-details.log`. Coverage includes developmental bounds/heredity/paid growth/timing, structural mutation, articulation and momentum, previous behavior/ecology/session tests, and real Mac Metal parity and topology cycles.

The sanitizer run status is recorded in `depth/final-sanitizers.log`; see the final response for its completed result. The final simulator Release build and signed physical-device Release build both succeeded. Xcode initially used an outdated generated project and failed to link the new source files; explicitly reconfiguring both CMake/Xcode projects resolved that build failure. A sanitizer-detected cursor overflow when a fixture changed a genome during development was fixed by checking cursor/frame bounds. The associated core suite now passes.

The old hunting test's assertion that an unarmed hunter must go extinct was removed: extinction is an ecological outcome, not a protected category rule. Its no-attack assertion and paired reproductive-cost comparison remain and pass.

## 14. Physical iPhone status

Latest signed build installed and launched successfully on **Max's Phone, iPhone 13**, after a transient device-discovery failure and successful retry. Evidence: `depth/m10-device-install-retry.log` and `depth/m10-device-launch.log`. This verifies installation and launch, not performance.

FPS, thermal state, memory, sustained cell/organism/mote counts, and session duration on physical hardware are **not measured for this checkpoint**. M13 is not complete.

## 15. Known limitations

Whole-body death and coarse energy redistribution remain. New cells' organs activate at whole-development completion. Nested developmental constructors are not independent reproductive workers. Development still builds parent trees, with angular articulation rather than arbitrary physical loop closure. Long chains are vulnerable to resource geography; small organisms can dominate. Genome limits are finite, collision/query work remains pairwise, and topology snapshots rebuild frequently. Ecological novelty has not been established by the mutation-only tests. The human movement and experience gates are outstanding.

## 16. Review build

The app is already running on Max's Phone as `com.example.AlienMobilePrototype`. Reset starts physical embryos; let them grow, then pan and pinch to inspect movement. Drag the gold resource emitter or magenta hazard, use 1×/2×, and reset. There are no new player controls or genome interfaces.

Simulator:

```sh
./mobile/scripts/run-simulator.sh
```

To reinstall the signed build on the same connected, unlocked, trusted iPhone:

```sh
xcrun devicectl device install app --device 52CB901A-AB3A-5EC4-813D-C504FF30E8F4 mobile/build-ios/Release-iphoneos/AlienMobileApp.app
xcrun devicectl device process launch --device 52CB901A-AB3A-5EC4-813D-C504FF30E8F4 --terminate-existing com.example.AlienMobilePrototype
```

This is the M10 review build. The requested final 30-minute owner playtest belongs after M13, not here. The immediate gate is whether a human can visually distinguish movement styles without labels.

## 17. Provisional verdict

**SUBSTRATE IS PROMISING BUT STILL TOO CONSTRAINED.** Development and mutation are substantially more expressive, and local articulation works. Damage, functional diversity, sustained scale, and the human experience remain unproven or unfinished. Do not interpret this checkpoint as completion of the full program.
