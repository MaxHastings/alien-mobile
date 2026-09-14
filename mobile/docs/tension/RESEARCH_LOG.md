# Persistent tension research log

Work in progress; no readiness claim. Baseline implementation sources and the M14
report preserve the starting point. Experiments are developer scenarios, not
player modes or outcome-conditioned changes.

## 1. Spatial beds, unchanged uptake/biology (spatial-v1)

Hypothesis: separated finite resource stores can create local depletion and make
movement/interception useful. Four beds, 14 units apart, each 64 fixed resource
sites in radius 4, each holding at most .4 energy. Light captures up to 4 energy/s
across all sites, reduced by distance/30 from movable light and by full stores.
No current. All cells retain ordinary contact uptake. Four seeds × two founder
conditions × original/spatial environment, 30 minutes, 360-cell cap. The garden
condition uses eight ordinary feeder/store variants; primitive uses eight copies
of M14's three-cell ancestor. Spatial founders occupy the four beds; original
founders occupy the central feeding area. This initial comparison changes
geography, transport and usable supply, so it cannot isolate each contribution.

Early failure: primitive spatial seeds 1–3 extinct at 42–94 seconds. Local site
regeneration (~.01 energy/s after light falloff) is below cell maintenance (.016)
and sparse contacts can miss sites entirely. Garden seed 1 persisted with 11
mature organisms, 273 births at 30 minutes. This does not establish long-run depth.
The mechanism tests separately confirm local depletion, finite recovery, exact
energy capture accounting and causal response to moving light.

## 2. Bed density heterogeneity (beds)

Keep site counts, storage, light, uptake and biology; vary bed radii physically
from 1.8 to 4.5. Test whether dense regions support primitive/simple life while
sparse stores permit alternative interception/movement economics. No regional
organ bonus and no organism-dependent replenishment. Regional observer records
stock, cumulative harvest, mature body counts, lineage counts and organ counts.

## 3. Initialization / minimal Origin (planted-v3)

Plant embryos on existing resource sites (no extra energy). Also compare a
one-constructor ancestor against the original unsteered three-cell primitive.
Six seeds per initial condition. All six minimal Origin runs survived 30 minutes,
with 1–3-cell descendants in final samples; five of six three-cell primitive
runs died. Garden runs retained 2–6-cell descendants. This separates an unsuitable
primitive founder from an inability of a single replicator to reproduce/change.
It does not establish complex emergent behavior in Origin.

## 4. Connected geography (connected-v4)

Only reduce toroidal world width from 72 to 32, retaining 14-unit bed spacing,
light, energy costs, resources and genomes. Twenty 30-minute runs (10 Origin,
10 garden), plus four garden runs targeted at five hours. Early long seeds 1 and
3 still extinct at 31 and 38 minutes. Shorter empty routes alone do not solve
sustainability. Retain experimental evidence; do not claim connectivity fixed it.

The regional ledger shows genuine local exhaustion: planted garden seed 1 starts
with 25.6 energy per bed; at 30 seconds beds 0–2 hold .47, 4.64 and 1.14 energy,
while the unoccupied fourth bed holds 21.36. Later abandoned bed 2 returns to
25.6. This proves resource feedback but is insufficient for playtest readiness.

## 5. Primary productivity (productive-v5)

The first spatial setups lose light with distance and leave full beds unharvested,
so actual captured energy is substantially below M14's unconditional 4/s emission.
Double incident light to 8/s, changing no consumer or mutation parameter. Test
whether the tiny surviving garden populations and subsequent stochastic losses
are a carrying-capacity issue before adding another ecological mechanism.

## 6. Functional ablations and competition

In the productive environment, matched frozen-DNA garden controls versus
motor-strength-zero controls distinguish useful actuation from free drift (there
is no current in this environment). Initial seeds 1–2: controls have 28–31 mature
organisms and 825–1009 births at 30 minutes; passive controls have 10–11 and
425–464 births. Removing food sensors (and their cost) caused initial seeds 1–3
to die at 540–966 seconds. Preserve the complete four-seed ablation set.

The first two evolving productive garden runs survived five hours. Their final
physical bodies include seven-cell moving descendants (seed 1) and six-cell
moving descendants (seed 2); seed 3 went extinct at about 15,880 seconds. Frozen
controls survive but cannot be cited as evolutionary evidence; injured clone
bodies can lose cells despite unchanged genomes.

Crucial competitive check: initialize one minimal constructor and one ordinary
five-cell sensor/motor/depot genome per bed, without changing biological rules.
All six 30-minute mixed runs retain simple and moving organisms. Seeds 1–3 have
seven-, eight- and nine-cell moving branches. Regions differ: seed 3 has moving
8–9-cell bodies in bed 0 and mostly single cells in beds 1–2. This is not a
programmed biome preference: the outcome differs by seed. Four mixed runs target
five hours; additional seeds extend the short validation to 20 mixed and 20
minimal Origin runs.

The read-only migration observer counts an individual only after it is sampled
inside two different bed neighborhoods; it does not count a nearest-region
boundary crossing alone. Several 30-minute mixed runs show such transitions.
Counts are sampling-sensitive lower bounds, not exact migration rates.

## Validation scope update

The owner explicitly changed the target to Mac iPhone Simulator, then authorized
headless completion while the Mac remains locked and will test manually later.
Physical iPhone and manual Simulator gesture validation are therefore not blockers
for this handoff. Do not claim either was performed. Simulator rendering captures,
runtime instrumentation, headless interaction tests and reset tests remain in scope.
