# ALIEN — formative build handoff

**PLAYABLE WITH SPECIFIC CAVEATS**

Release native UIKit/Metal app built, installed and launched on iPhone 17 Pro,
iOS 26.5 Simulator. Real rendered frames were inspected and a 44.79-second clip
was captured. The Mac is locked and its owner is away, so physical gesture tests
and catalog-panel interaction could not be performed in this pass. Those are
pending, not presumed successful. No physical-phone results are claimed.

## Launch and first session

From the repository root:

```sh
./mobile/scripts/run-simulator.sh
```

The script builds Release, selects an available iPhone Simulator (preferring one
already booted), boots it, installs the app, and relaunches a fresh experiment.
An explicit Simulator UDID may be supplied as its first argument. Requires Xcode,
an installed iOS Simulator runtime, CMake and Python 3; these were available here.

App: `mobile/build-sim/Release-iphonesimulator/AlienMobileApp.app`.
Bundle: `com.example.AlienMobilePrototype`.
Tested Simulator: `6663A26B-E595-4E16-AA40-027076B713FA`.
Open Simulator after unlocking the Mac. The app is portrait-only.

Try a short session without reading the mechanism section first. Stop whenever
you want. There is no minimum duration, score, or prescribed experiment.

- **Life:** choose a body preview, then tap open water. Placement disarms after success; Cancel abandons it. Life also closes the catalog.
- **Food:** select, then tap to scatter 48 ordinary finite particles. Select Food again to leave the tool.
- **Mutagen:** select, then tap away from the world edge. One local field lasts 20 simulation seconds and disarms placement. Wait for it to expire before another dose.
- **Flow:** drag to stir; two fingers pan while Flow is selected. Select Flow again to leave it.
- With no tool selected, tap life to follow; drag to pan; pinch to zoom (Option-pinch in Simulator). **Wide** clears tools and shows the tank.
- **Pause / Play** stops/resumes; speed cycles **1× / 2× / 4×** independently. **↺** requests a new world, with a cancelable confirmation.

The loop is to notice a body, introduce another specimen, watch contact and
construction, change a food opportunity or exposure, and follow what happens
to organisms and their descendants. Six independent drifting food patches
support watching without repeated feeding. Introductions provide finite starting
energy, not continuing assistance. Every reset starts a fresh four-founder world.

## Changes in this pass

Kept the four genuine catalog genomes, body previews, local controllers,
connected-cell physics, resource economy, normal mutation and native renderer.
The baseline already contained substantial good work; no new genome search or
ecological redesign was warranted by this bounded pass.

Added a bounded mutagen intervention, its real world-space dotted boundary,
countdown and exposure language; separated Pause from the speed cycle; added
follow text for active construction and loss of a followed organism; corrected
empty-water taps so they do not display successful focus feedback; updated the
placement accessibility label. Husk now explicitly asks to be placed among food.

Removed pause from the multi-tap speed cycle, landscape orientation (the fixed
catalog tray is designed for portrait), and the old mandatory 30-minute playtest
protocol. Made Simulator selection portable across installed iPhones. Added
mutation-boundary and repeated-player-experiment regression checks plus a
reproducible bounded catalog/session runner.

There is no saving UI, genome collection, world persistence, hunter entry,
progression, currency or protected ancestry. Flow remains because it physically
moves both cells and food. No unrelated project work was removed. The entire
project was untracked at baseline; source was backed up before editing to
`/tmp/alien-before-playtest-pass.tgz`. No commit or external publication was made.

## Actual mechanisms and provenance

Every starting genome is still exactly equal to its archived DNA in
`docs/discovery/finalists`. Full-genome equality checks passed again after the
last catalog wording change. All four were selected in prior offline simulation
runs from authored starting architectures; this pass did not evolve or edit DNA.
Provenance below is from the retained archive/report, not a newly repeated search.

| Specimen | Physical mechanism | Provenance | Limitation |
|---|---|---|---|
| Skiff | Four-cell fork; local food sensing drives two paid thrust motors | Dart colony, seed 238, organism 346, generation 14 | Small reserve; strongly dominates these default trials |
| Thread | Nine-cell driven head and extended compliant collector body | Ribbon colony, seed 201, organism 91, generation 6 | Slow construction; competition and sparse food can eliminate it |
| Whorl | Seven cells from two developmental modules; three tangentially driven arms | Crown colony, seed 201, organism 45, generation 3 | Rotation helps in some contexts; not universally superior |
| Husk | Five-cell passive body with three storage cells; collects and saves energy | Vault colony, seed 201, organism 47, generation 2 | Needs food contact before storage helps; cannot pursue a departing patch |

Fresh paired checks used the actual archived finalists with mutation frozen only
for mechanism isolation. All used seed 1877, three initial specimens per isolated
trial. No genome or rule was tuned between paired conditions.

| Comparison | Duration / setting | Intact → ablated result |
|---|---|---|
| Skiff sensing | 300s, scarce moving food | Mature births 95 → 16; final adults 15 → 2 |
| Thread body span | 300s, default food | Births 43 → 28; final adults 13 → 8 |
| Whorl motors | 300s, default food | Births 34 → 19; final adults 16 → 17 |
| Husk storage | 600s, earned-reserve famine | Final adults 5 → 0; second-half births 5 → 0 |

Husk without storage had more early births (26 versus 16) before extinction;
Whorl without motors ended with one more adult despite fewer births. These
results support contextual mechanisms and costs, not universal superiority.
Historical broader evidence is in [the discovery report](../discovery/REPORT.md).

## Execution evidence, including failures

- Baseline: **28/28 CTest tests passed**.
- Candidate: **29/29 CTest tests passed**, including CPU/Metal checks and the new mutation test. The subsequently added three-seed player-experiment executable also passed. The final catalog wording was followed by another passing placement and exact-genome check. Thus all 30 current test executables have passing results for their relevant final code; no claim of one combined 30-test invocation.
- Mutation: 10,000 draws per actual catalog genome. Changed outcomes normal/exposed were Skiff 7,845/8,415; Thread 8,064/8,638; Whorl 7,215/7,750; Husk 7,899/8,480. Tests check local range, finite duration, rejection of stacking/invalid placement, reset, preserved inherited rates except ordinary meta-mutation, unchanged unexposed replay, and no reroll of already proposed offspring.
- Repeated player interventions: seeds 913/1913/2913, 360 seconds each, three fixed-time finite food plus exposure applications. Final adults **37/25/29**, mature births **144/108/122**. All finished with expired exposure and valid finite state/connections. This does not prove every dose changed a useful trait.
- Three mutation-on default sessions, 600 seconds each: final adults by starting ancestry **Skiff/Thread/Whorl/Husk** were seed 913 **31/0/0/0**, seed 1913 **25/3/0/0**, seed 2913 **31/0/2/0**. Every starting architecture produced mature descendants in each session, but Thread was already absent at 60 seconds in seed 2913. No universal collapse occurred; substantial ancestry loss did.
- One mutation-on default soak, seed 3913, 1,800 seconds: **31 adults**, all Skiff ancestry, 784 mature Skiff births, maximum mature generation 44. Of the final Skiff adults, 26 retained the starting architecture under the observer's topology/role definition, and four encoded architectures remained. One ancestry is not one unchanged genotype. This single run does not establish long-term balance.
- All 12 fresh archive/session processes exited successfully. Maximum recorded energy-accounting error was **0.00001846** units; finite-state/connection checks passed. Extinct ablations and lost ancestries are biological failures, not crashed runs.
- Real Simulator observation: seed 913 at normal speed, approximately 132 simulated seconds logged before a deliberate rebuild/relaunch. The log reached 28 mature births and recorded mutation, death and debris. Median reported frame rate was 60 fps on this Mac (startup minimum 52.57); this is Simulator performance only.

[Before/after real frames](live-start.png), [later frame](live-end.png),
[45-second recorded clip](live-45s.mp4), [live log](live-session.log),
[tests](final-tests.log), [player experiments](player-experiments.log),
[catalog/session results](sessions.log), [commands and raw data](sessions/results.json).
The recording precedes the final wording and portrait-lock-only build; simulation
and visible main-screen layout are identical. The final Release was rebuilt,
installed and launched afterward. No rendered event was fabricated.

## Causal audit and simplifications

Inspection found generic local particle sensing, edge-propagated internal
signals, inherited controllers, paid actuation, particle interception, physical
energy diffusion, paid construction, inheritance, mutation and starvation/damage.
Catalog metadata does not choose runtime behavior. Nearest-cell sensory sampling
is generic; the word “target” in physical constraints is not hidden species AI.
The authored four-founder opening occurs once per reset. There is no rescue,
lineage privilege, diversity score or guaranteed novelty.

Mutagen samples the constructor position when a new offspring proposal is first
drawn. Inside radius 4, ordinary event probability is multiplied by up to 1.5,
capped at 95% when the inherited rate is lower. High inherited rates are not
reduced. Relative mutation kinds and magnitudes are unchanged; at most one
ordinary event occurs, with the existing independent rare meta-mutation rule.
Exposure does not mutate the adult, alter stored offspring DNA, or inherit its
multiplier. For these already mutable genomes the effect is deliberately modest.
The countdown advances in simulation time and freezes with pause. The ring shows
exposure, not an adaptation or a successful mutation.

Physics is a consistent spring/constraint approximation, with isotropic drag;
internal contraction is not treated as free propulsion. CPU physics runs in
Simulator with Metal rendering. Device Metal physics has host parity tests,
but no physical-phone execution was performed. No successful hunter is advertised:
combat fixtures pass, while historical hunter establishment failures remain
reason to omit it from the playable catalog.

Reference review confirmed ALIEN's particle-network bodies, neural cell functions
and cell-by-cell offspring construction from its [current repository](https://github.com/chrxh/alien).
Cell Lab's [official description](https://cell-lab.net/) supports an experiment
mode, functional cell types, tiny mutations and zoomable vector presentation;
it explicitly trades biological realism for playability. Its author also reports
frustration with an overly lethal UV mechanic. These informed readable bodies
and conservative exposure; neither reference was executed locally in this pass.

## Remaining handoff caveats and questions

The normal screen, native rendering and real simulation executed. Gestures,
Pause/Play taps, reset dialog interaction, catalog selection, focused close-up
legibility and the new exposure-ring UI still need an unlocked-Simulator check.
Their code paths compile and core actions are tested; that is not a substitute
for touch testing. No claim of complete interaction-path verification or human
enjoyment is made. The smallest next step is a short unlocked session, not a
new research program or redesign.

Ask afterward: What did you want to try? What held your attention? Could you
form a hypothesis? Did any organism or descendant make you want to try again?
In particular, learn whether Skiff's prevalence and the subtle exposure effect
limit the experiment before changing ecology further.
