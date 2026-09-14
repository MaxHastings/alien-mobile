# First-playable finalization — 2026-09-13

**Independent verdict: CORE LOOP IS PROMISING.** The system produces ongoing
heritable variation and visibly different survival/reproduction outcomes under
the existing two environmental controls. This is sufficient for a human fun
test. It is not evidence that long-term engagement or touch feel is proven.

## Inherited state and corrections

The repository had no commits; all mobile files were untracked. There was no
historical diff to inspect. The complete source tree comprised portable C++
biology/physics, native UIKit input, Metal compute/rendering, one C++ test binary,
and CMake. The README described Milestone 1 despite substantial Milestone 2/3
code. Initial host tests passed; the initial iOS build could not compile its
shaders until Apple's missing Metal toolchain was installed.

The most important defects and corresponding changes:

1. **Camera/render/input disagreement:** the shader divided world coordinates by
   pixel dimensions while touch assumed clip-space zoom. Corrected the transform,
   aspect ratio and launch zoom; added round-trip/Retina/landscape camera tests.
2. **Unbounded resource:** every cell received an independent supply. Added a
   shared total food budget so crowding has a physiological consequence.
3. **Root-only construction:** collecting outer cells did not feed reproduction.
   Added body-wide energy sharing and distributed construction debit. Bigger
   bodies have more collection sites and also more upkeep/building cost.
4. **Fragile morphology:** tree springs did not retain genome angles. Added a
   derived brace per eligible node so physical footprints reflect inherited shape.
5. **Rightward offspring conveyor:** every birth appeared to the right with a
   large impulse. Birth orientation now varies; offspring stay physically attached
   during cell-by-cell construction and receive a smaller detachment impulse.
6. **CPU/GPU ordering risk and excessive rebuilds:** synchronized once before
   biology and batched topology changes into one upload per step. Added an ownership
   mock plus real Metal birth/death/reset tests.
7. **CPU/GPU numerical mismatch:** spring damping previously observed sequentially
   modified CPU velocities. Both backends now use the input snapshot; coincident
   repulsion separates cells in opposite directions on both backends.
8. **Memory lifetime:** enabled ARC; the inherited renderer continually created
   owned buffers without releasing them. Simulation state now outlives renderer
   destruction. Render command buffers retain their submitted resources.
9. **Construction/cap gaps:** reserve full offspring capacity, allow an unborn
   record to wait for its first paid cell, and retain energy-gated pauses.
10. **Mutation edge cases:** reject non-finite root geometry; keep random units
    below one; blocked additions no longer become accidental removals; unchanged
    geometry no longer creates a fake new lineage/color change.
11. **Presentation:** luminous cell membranes/cores/halos, soft physical connection
    strokes, distinct resource/danger handles, biologically matched field falloff,
    cell-specific appearance and starvation fades. A transient interaction hint
    replaces reliance on undocumented simulator controls.
12. **Input:** safe-area buttons, minimum screen-space source targets, nearest
    source selection, preserved drag offset, source-capture haptic, focal pinch
    zoom, bounded camera panning, reset to 1×, and no catch-up after suspension.
13. **Measured CPU bottleneck:** moved connection exclusion behind the distance
    rejection in repulsion. No grid/hash or new engine architecture was added.
14. **Validation/build:** enabled assertions in Release test targets, introduced
    session and real Metal test binaries, made shader compilation SDK-specific,
    produced signed iPhone builds and rebuilt documentation around actual behavior.

## What remains derived from ALIEN

The conceptual reference is [chrxh/alien](https://github.com/chrxh/alien).
Reviewed its Entities, Genome, ConstructorProcessor, ConstructorHelper, Physics,
ObjectConnectionProcessor and ConstructorTests, plus cell, line and tone-mapping
shaders. Retained connected physical cells, paid incremental construction,
heritable structure, separation, energy/selection, and luminous artificial-life
presentation. The mobile code does not import or build those sources. Desktop
editing, neural systems, CUDA scale, laboratory controls and the full render
pipeline are intentionally absent.

## Traced behavior

**Frame:** UIKit mutates source position/camera on the main thread. The renderer
accumulates fixed simulation steps. A step reads GPU state, updates energy/death/
constructors, uploads topology once if changed, and submits GPU motion. Draws on
that same queue read the new physical buffer; metadata carries CPU biology.

**Reproduction:** copied/mutated child genome → empty pending child → paid root
cell and parent tether → one paid node per eligible interval with genome edge and
brace → final node → remove that tether → mature child and separation impulse →
parent cooldown. Child constructors use the same process.

**Mutation into another generation:** each child owns the actual modified genome;
unmutated grandchildren copy that modified genome and its lineage. The tests
explicitly seed a mutated genome and verify all descendants, including mature
generation 2, inherit it without replacement by the default genome.

**Death:** spatial depletion and upkeep drain the shared reserve; the starvation
timer drives fading. Removal collects unfinished dependent offspring, compacts
cells, remaps root/connection indices, clears surviving parent references, then
uploads the rebuilt GPU state before the next motion step.

**Intervention:** drag screen point → camera inverse → bounded source world point
→ per-cell spatial exposure on the next simulation step → shared intake or drain
→ construction opportunities, reserve depletion, deaths and descendant counts.
No mutation outcome or trait bonus is directly selected by the drag.

## Validation evidence

See `validation-*.log` alongside this report for retained command outputs. Tests
measure simulated time; a 30-minute soak is not a 30-minute human/device playtest.

- Initial host suite: 1/1 passed. Initial iOS build: failed, missing Metal toolchain.
- Final host suite: 3/3 passed with assertions active (29.72 seconds).
- Release host suite: 3/3 passed (7.50 seconds). Default 30-minute experiment ended with 59 cells,
  1,530 completed births, 1,760 removals (including unfinished offspring), maximum
  attempted generation 44, 539 mutations and seven extant lineage IDs; peak 84 cells.
- Food moved from its initial position to (7, −6) at 60 seconds: extinction by
  the 10-minute checkpoint, with 17 completed births.
- Danger placed over food at 60 seconds: 26 cells at the 10-minute checkpoint,
  95 births and 142 removals, versus 66 cells and 481 births in the unchanged
  experiment at the same 10-minute checkpoint. The 30-minute ecology remains viable.
- Real Metal parity at 1,000 steps: maximum sampled position error 0.000002652,
  velocity error 0.000001181; tolerance 0.0001. Three GPU growth/death/reset cycles
  each reached 150 cells and 41 completed births before forced depletion.
- AddressSanitizer + UndefinedBehaviorSanitizer: all three suites passed, including
  the complete session scenarios and Metal topology cycles; no diagnostic failures
  (87.58 seconds).
- iOS device SDK Debug and Release, and iPhone Simulator Debug: successful builds.
  Signed Debug and Release application bundles produced using the existing account.
- iPhone 17 Pro Simulator / iOS 26.5: actual app launch, GPU runtime/parity, visible
  growth/brightness/lineage forms, portrait and landscape, 2× speed, and reset back
  to the seed/1× verified. Normal UI contains no developer counters.
- Physical iPhone: **not validated**. Installation failed because the paired iPhone
  13's CoreDevice tunnel was unavailable. No claim of device performance or touch
  satisfaction is made. Freeform simulator gesture automation also failed in the
  computer-control service (`noWindowsAvailable`), although accessible buttons and
  rotation worked. Camera math and source/input code were inspected; a human must
  still exercise dragging and pinching on the phone.

## Performance and meaningful limits

On this development Mac, the representative CPU 240-cell physics step fell from
about 5.14 ms to roughly 0.09–0.11 ms after distance-first rejection. Final Release
samples at 60/120/240 cells were approximately 0.005/0.023/0.099 ms. Metal step plus
synchronous readback was approximately 0.50/0.36/0.40 ms at those sizes; repeated
runs varied with host load. These are
host microbenchmarks, not iPhone GPU/frame timings. Normal default ecology peaked
at 84 cells in the tested soak; the safety cap is 240, and real Metal growth tests
reached 150. This range is comfortable on the host. iPhone comfort is unmeasured.

At this scale GPU submission/readback dominates its tiny compute workload. CPU
biology scans, topology rebuilding, per-frame render allocations and pairwise
repulsion remain scaling limits. All positions must currently be read back each
biology step. There is no evidence justifying a large parallel-biology rewrite.

The ecology is passive: population spread comes from construction, detachment and
collisions, not intent. Energy transport is instantaneous inside each body. Cell
mass and separation impulses are simplified. Genomes contain 2–6 nodes; very
similar branches can be difficult to distinguish. There is no persistence or
background evolution. Reset repeats a deterministic baseline. Source fields are
finite-rate supplies rather than a depleted nutrient substrate. None of these
limitations are concealed behind upgrade statistics.

## Human playtest and next step

Use the physical-device steps in [README](../README.md). Watch one birth, move food
slightly toward one branch, then move danger toward a competitor. Notice whether
you can predict which bodies brighten, keep constructing or fade. Zoom into a
changed shape and watch for descendants. Try moving food too far away, then reset.
Repeat a different intervention from the same starting world.

The next step is a 10–30 minute iPhone fun test with two or three people who have
not seen the code. Record their first source drag, first causal hypothesis, first
noticed inherited shape, and whether they voluntarily reset to try again. Confirm
heat/frame pacing and pinch/drag feel on the actual phone. Tune the existing loop
from those observations before adding sensors, muscles, predators or metagame.
