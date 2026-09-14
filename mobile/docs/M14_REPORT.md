# M14 — evolutionary implementation and pre-playtest evidence

2026-09-13. This report supersedes the older milestone summaries for the playable
configuration. Human evaluation has **not** happened. No M15 or metagame was built.
The owner waived intermediate human gates and then requested the Mac iPhone
Simulator as the final evaluation target.

The implementation preserves the existing C++/UIKit/Metal architecture. It fixes
mutation continuity, capacity-biased offspring rerolling, ineffective module
duplication, inactive developing organs, and cumulative energy precision. The
default now starts with primitive life in a finite, heterogeneous resource field.

## 1. Current genome model

DNA contains a bounded list of modules and nodes, attachment geometry, organ
parameters, eight-channel local controllers, construction calls and inherited
mutation probabilities/magnitudes. Modules have orientation and phase advance.
Copies retain controller DNA and remap references. Ordinary mutation modifies
DNA before a real paid birth; it does not directly design the physical result.

The normal eight founders are copies of one three-cell genome: constructor,
short-range energy sensor and weak unsteered motor. They share initial ancestry.
There is no authored hunter, long swimmer, defender or memory circuit in this
default. Those remain explicit mechanism/regression fixtures.

## 2. Developmental expressiveness

Attached modules, repetitions, branches, intervals, joint geometry and separate
offspring flags support physical developmental trees. Limits remain 16 modules,
64 nodes per module, 256 stored nodes and bounded expansion/depth. Unreachable
DNA can persist. Genome length is therefore not a measure of body complexity.

Incremental construction consumes real energy and time. Growing organs can now
sense, propel and pay upkeep before the entire body matures. Fragments and failed
development do not become free reproductive organisms. Physical parent trees,
derived braces/joints, finite reservoirs and local damage remain the architecture.

## 3. Mutation classes

Small neural, geometry and property edits; role changes; node insertion/deletion;
module duplication/deletion; section copying/moving; and constructor edits remain
distinct. The [field inventory](M14_PROGRAM.md) gives the bounds and consequences.
Formerly frozen waveform, amplitude, motor channel, recurrence and non-entry
module orientation are reachable. Geometry edits are smaller and local.

Previously a small neural edit silently switched a zero-matrix relay into a
replacement controller and cut its signals. Small edits now retain the mode;
zero-DNA residual mode remains an exact relay and supports incremental changes.
An explicit, less frequent property edit can still switch modes and fail.

Module duplication formerly redirected the only reference to the copy, often
leaving no additional expressed structure. It now retains the original section
and adds an attached constructor call to the copied module. It may be costly,
sterile or mechanically poor; no viability filter protects it.

The capacity fix is particularly important: a parent retains its proposed
offspring DNA while waiting for space. It cannot reroll every tick until a tiny
genome happens to fit. Invalid proposals consume a cooldown rather than a rapid
free search. Full-body capacity reservation is still an explicit practical limit.

## 4. Meta-mutation model

Twelve probabilities and three scalar mutation magnitudes are inherited. A slow
independent meta event chooses one value, perturbs it multiplicatively by up to
4% plus a small additive term, and clamps it to a technical bound. Ordinary
probabilities are bounded at .5, the meta probability at .01, magnitudes at 4.
Zero is permitted. No minimum mutation rate or stagnation rescue exists.

The meta draw now remains reachable even when ordinary probabilities sum to at
least one. Tests verify inherited tempo differences, bounds, zero-rate stability
and simultaneous ordinary/meta mutations. In 20,000 saturated trials, 187 meta
events occurred; ordinary no-op categories were not falsely counted as changes.

Natural long-run divergence is **modest**: for example, observed property rates
in long seed 41 span .116646–.127049 around .12. The data demonstrate inherited
regime variation and continued viable change, but not strongly separated
“explorer” and “conservative” ecological strategies. That stronger M14D aspiration
is not claimed as an achieved result.

## 5. Complexity economy

Every cell pays basal upkeep (.016 energy/s in this configuration). Each new cell
requires construction energy, a transferred usable reservoir and a construction
interval. A larger daughter imposes that repeated burden before birth completes.
Sensors pay range-squared × sensitivity costs; motors, contraction and bending
pay for actuation. Attackers/digestors pay substantial upkeep and work costs.
Memory and defense also have explicit maintenance. Depot capacity is finite
storage, not an energy source. Internal constructor nodes do not create free
parallel reproductive workers.

Large authored bodies failed through the combined construction/reproductive
burden, dormant growing organs, poor resource geometry and a capacity allocator
that repeatedly selected smaller proposals. The latter two implementation defects
were fixed; costs were not removed to preserve elaborate creatures.

A controlled three-cell versus duplicated seven-cell comparison collected
.3 versus .7 energy from the same dispersed finite particles, but .1 versus .1
from one concentrated particle. Basal cost was .048 versus .112/s. This proves a
physical interception benefit and a real tradeoff, not universal size advantage.
In current research seed 16, 49 mature descendants have 4–7 living cells after
30 minutes; seed 1 retains mostly 4–5 cells. Small bodies nevertheless dominate
many other runs and both five-hour endpoints.

## 6. Energy ecology

The default 72×72 torus supports at most 360 cells and 5,000 motes. Mean source
emission is 4 energy/s with a smooth physical time variation; particles expire
after 30 seconds. A concentrated patch travels within radius 6, with 25% broad
background emission. A bounded circular fluid force applies to cells and motes.
These functions depend on position, the movable source and simulated time only.

Dragging the gold source relocates future supply and its flow field; existing
motes remain physical objects. The magenta hazard locally dissipates usable
energy. Contact extraction, raw energy, digestion losses, local diffusion,
fragments and recycling remain causal. No population or lineage statistic tunes
the environment. Initial primitive founders are placed in usable initial light;
there are no later injections or population rescues.

Cell and mote energy reservoirs now use double precision. Before the fix,
float accumulation lost several energy units during a 15-minute run. Current
five-hour runs stay below .000453 total accounting error without correcting the
ledger or changing physical energy to force agreement.

## 7. Body/brain coevolution

Copied structure carries its own controller and organ parameters. Signals cross
physical inherited connections; orientation and geometry affect sensing and
actuation. Residual processing permits incremental controller change without
first constructing a complete replacement matrix.

The regression suite demonstrates neural response, flexible locomotion,
contractile/bending organs, hunting ablations, memory and communication mechanics.
These fixtures are not evidence that those strategies evolved spontaneously.
Primitive descendants in research seed 16 retain a sensor and expand to 1–4
motors, while seed 13 develops motor-rich branches with some attacker cells.
Most successful worlds lose expensive organs. Organ presence alone does not prove
coordinated propulsion, navigation or profitable predation.

A diagnostic repeat of seed 16 measured actual paid thrust: at 30 minutes six
of 49 mature descendants were actively propelling, in generations 103–108 and
both four- and six-cell families. Other sampled moments had inactive motors.
This confirms intermittent actuator expression, not an invented always-swimming
interpretation of fluid-carried bodies (`m14e/motor-expression`).

## 8. Multi-seed and long-run results

- [Twenty final-default-scale research arenas](m14e/viable-start/ANALYSIS.md): 18 survived to
  1,800 simulated seconds; seeds 5 and 11 went extinct at about 943 and 1,723 s.
  Survivors span 29–318 mature organisms, 101–284 completed generations or sampled
  mature lower bounds, and distinctly different body distributions.
- [Wide-resource stress runs](m14e/phone-scale/ANALYSIS.md): the earlier 1,200-cell
  candidate. Three 30-minute runs and seventeen additional 15-minute runs are
  tabulated separately. They consistently favored crowded minimum bodies, so
  the already-tested smaller arena became the final default. This was a global
  resource/population-scale choice, not a seed-specific intervention.
- [Two long runs](m14d/long/ANALYSIS.md): 18,000 simulated seconds each, 108,741 and
  97,361 real births, with sampled completed-generation lower bounds 1,712 and
  1,436. Runtime was 630.805 and 811.372 wall seconds on this Mac under concurrent
  work. No invalid topology, non-finite state or conservation failure was found.

Older `maximumGeneration` includes a proposed offspring. New diagnostics track
completed births separately; older data use sampled mature lower bounds. “Seen
lineages” depends on sample frequency and is not an exact all-birth count.
The cap flag means capacity was encountered, not that every later tick was full.

Failed environmental experiments remain in `m14e/harsh-environment`, the earlier
root-level seed files and `m14e/circulation`. Unbounded shear transported life out
of usable food; spawning primitive roots away from the initial patch caused early
starvation. Fixes altered physical circulation and initial placement, not death
rules, evolutionary outcomes or failed-seed inclusion.

## 9. Persistent lineage divergence

Research seed 1 has seven observed changes in the most numerous lineage; seed 16
has five. Families survive at least 120 seconds in every final research run,
including runs that eventually go extinct. Long runs contain 51 and about 66
such families, with 47 and 68 sampled dominant-lineage replacements. CSV-rounded
timestamps can move an exact 120-second boundary by one observation.

This is inheritance and replacement evidence, not proof of speciation. Related
colors are intentionally close; hue does not encode a species rule. Crowded
one-cell populations can remain difficult to follow as individual families.

## 10. Novel organisms actually observed

[Blind observation notes](m14e/BLIND_OBSERVATION.md) were written before opening
the selected seed-1 genomes. Four-, five- and six-cell radiating bodies visibly
differed from the primitive chain. The prediction was passive interception with
duplicated/rearranged branches, not an invented swimming controller. Subsequent
DNA inspection confirmed constructor-rich branches without sensors or motors.
They are related variants of one family, not three independent breakthroughs.

Seed 8 contains seven-cell, two-module forms; seed 16 retains sensor/motor-bearing
4–7-cell branches. These came from ordinary primitive-founder heredity, mutation
and selection. No candidate was exported, fed, improved and reintroduced.

## 11. Unplanned behaviors actually observed

Populations spread with the current, concentrate around supply, undergo resource
waves and replace lineages. Some descendants form broad passive collectors;
others retain motor-bearing branching bodies. Attacker activity occurred in
some evolved runs, but sustained evolved digestion/predator-prey networks were
**not** established. No observed behavior is described as strategic intent.

## 12. Stagnation modes that remain

The low-cost constructor remains a powerful local optimum. Both five-hour runs
end mostly or entirely single-celled. Long seed 42 accumulated 8–9 modules and
84–101 stored nodes while expressing one cell: that is latent DNA expansion,
not growing phenotypic complexity. Sensors and motors often disappear because
passive interception pays better in the tested pressure field.

The 30-minute results establish repeatable alternatives, not indefinite open-ended
innovation. Large trophic innovations and strong natural meta-regime divergence
remain unproven. If human observation feels too homogeneous, investigate resource
and movement tradeoffs before adding roles or game systems. Do not compensate
with novelty rewards, protected variants or scheduled species appearances.

## 13. No-hack audit

The runtime paths were inspected as well as searched. No `seekFood`, `chase`,
`flee`, desired heading, target velocity, predator/prey AI, novelty/rarity/diversity
bonus, complexity subsidy, species protection, stagnation mutation boost,
population rescue or forced-interesting-mutation mechanism remains.

Search matches are ordinary “avoids invalidating” and “avoids vector order”
comments and the primitive genome's “unsteered propulsor” description. Generic
sensor queries, physical environmental flow and authored test DNA are explicit
exceptions to a blanket textual search, not outcome-level behavior shortcuts.
The corrected capacity queue removes an accidental hidden bias toward small DNA.

Sampling at 10 versus 60 seconds produces the same final RNG/physical-state hash
`16981754880449427413` for seed 72, despite different observed lineage counts.
Observer metrics never enter fitness, source control, mutation or reproduction.

## 14. Physical iPhone and technical validation

A signed Release build ran on the connected iPhone 13 for 914 simulated seconds
at normal speed before a clean exit/disconnection. The log records approximately
60 FPS and 120 simulation ticks/s through high population, stable tens-of-MB
memory and thermal state 1 (“fair”) near the end. This is real device evidence;
it is not inferred from Simulator. Battery drain and real-finger comfort were
not measured. The final connection/root graphic polish was built successfully
but could not be installed after disconnection. The owner explicitly selected
Simulator-only completion afterward.

The final host suite contains 21 passing tests, including CPU/Metal checks,
20 repeated primitive resets, and paired source/hazard interventions. Moving the
source 24 world units reduced the next minute's uptake from 293.92 to 79.14
energy and left six cells versus 238 in the control; the hazard intervention
left 109 cells without teleporting life. Those are causal test outcomes, not promised gameplay.

AddressSanitizer/UndefinedBehaviorSanitizer checks cover mutation continuity,
meta-mutation, developing organs and a five-minute ecological run. No sanitizer
failure occurred. Detailed runtime/UI measurements are in
[VALIDATION.md](m14e/VALIDATION.md), with raw logs alongside it.

## EVOLUTIONARY WATCHABILITY

The separate audit uses the [Cell Lab reference](https://cell-lab.net/) for rapid
real reproduction and fast-forward, and [ALIEN's mutation architecture](https://github.com/chrxh/alien/blob/235ecc624bf62b45db9320538821454c7be72b3b/source/EngineKernels/MutationProcessor.cuh)
for local/structural causal operators. No editor, challenge structure, scripted
adaptation or fake fast evolution was copied.

The final-default-scale research survivors generate about 51–432 births per simulated minute
at 1× and reach 101–284 mature generations within 30 minutes. The three full
wide-resource stress runs produce 1,165–1,219 births/minute and reach 279–333 completed
generations. This is rapid paid turnover, not declared generation advancement.
Native 2× executes the same fixed steps; measured throughput and actual wall-time
capture timestamps are recorded in VALIDATION.md. Headless long runs achieve
22–29× simulated real time; thousands of generations can be compressed without
altering biological rules.

New diagnostics distinguish all completed births, DNA-changing births, structural
operator births and meta events. These measure **candidate mutation cadence**, not
meaningful visible innovation. Physical morphotype counts use cell/module/branch
and organ counts and are labeled as sampling-sensitive proxies. Injury can alter
them. No fabricated precise “mean time to an interesting organism” is reported.

Recorded physical snapshots at roughly 0, 5, 10, 20 and 30 simulated minutes show
three-cell chains becoming branched related shapes in several research runs;
other worlds become crowded dots. Both native sequences preserve actual observed
simulation and wall times: the earlier wide world transitions from 1× to 2×;
the final default uses explicitly displayed developer 8× and completes its
30-minute capture in 230.61 wall seconds at a median 60 FPS. Shape changes
are observable on a session timescale. Major locomotion/ecological innovations
are not guaranteed. Strongly similar hues and crowded simple bodies remain a
watchability risk, even though birth counts are high.

The visual polish brightens real connections, reduces oversized root graphics,
shows uptake flashes and renders paid motor exhaust separately from fluid drift.
It adds no species announcements, mutation badges or novelty effects.

**Answer:** evolution is more than a mathematical counter: physical descendant
variation, family replacement and pressure responses are observable. Whether it
is enjoyable to watch is deliberately left to the human; raw turnover is not a
substitute for that judgment.

## 15. Human playtest result

Not yet performed. Use the [uncoached protocol](FIRST_HUMAN_PLAYTEST.md). Give only
basic controls initially and do not prime the evaluator with these outcomes.

## 16. Independent verdict and stop boundary

**EVOLUTION IS INTERESTING BUT STILL TOO REPETITIVE** is the conservative research
verdict. It is not a declaration of fun. The implementation and evidence support
a subjective playtest of this living world, while the original strongest claims
of sustained open-ended ecological novelty and sharply different evolved
mutation regimes remain unestablished.

M14A–C mechanisms have direct regression and primitive-world evidence. M14D's
inheritance machinery is verified with modest natural divergence. M14E has
multi-seed turnover and different body histories, alongside clearly documented
long-run simplification. Intermediate human gates remain deferred, not silently
marked passed. Do not equate passing tests with passing the original curiosity
or depth gates.

Freeze feature breadth here. The next decision is the owner's actual observation
of the Simulator build. Do not start M15, add a metagame, or invent another cell
role before that feedback.
