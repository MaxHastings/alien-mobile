# ALIEN Mobile — refoundation handoff

September 13, 2026. Read after the first uncued play session.

## 1. What the game is now

A living terrarium. Introduce a physical organism, scatter a finite handful of
food, or stir the water. Watch what survives, develops, reproduces and disappears.
Tap an individual to follow it; return to the wider ecology whenever curiosity
moves elsewhere. There is one normal experience, with no campaign or upgrade UI.

## 2. What changed

Rebuilt the player interface around Life, Food and Flow. Added a six-card catalog,
explicit placement/cancel states, finite food, tap-to-follow, generation-only
focus feedback, Wide, pause and 4× time. Reset now protects against accidental
loss with a small confirmation. Tool changes cancel previous selection cleanly.

Replaced the two-organism opening with Fork, Lantern, Ribbon and Spinner in
separate patches. Tightened the landscape from 60×60 to 48×48, adjusted patch
spacing/drift, energy diffusion and resource productivity, and retuned larger
founders' paid motors and sensor sensitivity. Starting specimens receive a
finite 0.85 energy per cell. Descendants still pay ordinary construction costs.

Improved luminous cell readability, retained functional nuclei and activity
signals, added a subtle tank boundary and touch feedback. Catalog previews now
interpret development rather than assuming the entry gene is the whole body.

Hardened placement against invalid genomes, overlaps, tank edges, capacity and
space already reserved by developing offspring. Fixed runtime specimen energy
accounting and fractional autonomous food accounting. Updated the research tool
with an explicit `catalog` founder selection so tests reproduce the app preset.

The old light, kill region, fixed beds and origin fixtures remain developer
comparisons only. Radiation was not added: three player verbs are sufficient to
test the central experience. No major biological system was added.

## 3. Why these choices

The first Simulator inspection showed two tiny bodies in a mostly empty field.
The old catalog also had conflicting tool states and a horizontal row that hid
entries. Four distinct bodies and a visible six-card catalog make the first
experiment accessible without a setup screen.

The first revised ecology supported reproduction but consistently favored
Pebble. Larger founders carried additional drag with essentially the same
propulsion. Retuning their actual motor hardware and sensor costs improved their
persistence. Keeping Pebble available to introduce, rather than in the opening,
then produced a substantially more varied default. This is a starting-condition
choice, not an ongoing rule: no organism retains a catalog privilege.

Food changes local opportunity; Flow changes physical conditions. Both produce
immediate visible input and use the existing ecology/physics afterward. The
camera provides attachment without a statistics inspector.

## 4. Normal controls

Life → specimen → tap. Food → tap. Flow → drag. Tap the selected tool to cancel.
Tap life to follow; drag to pan; pinch to zoom; Wide shows the tank. In Flow,
two fingers pan. Time cycles 1×, 2×, 4×, pause. ↺ starts a new experiment.

## 5. Starting experience

Four moving, differently colored, connected-cell bodies occupy separate resource
patches. The background is dark, food is gold, and there is room between groups.
Life, Food and Flow stay at the bottom; camera/time/reset stay at the top.
A short contextual line explains the currently available gesture.

## 6. Organisms and specimens

The catalog contains Pebble, Ribbon, Spinner, Fork, Lantern and Hunter. Entries
are genome snapshots, names, display hues and finite initial reserves. They are
constructed through the normal founder path, not species subclasses. Once placed,
they use normal signaling, force, feeding, development, damage and inheritance.
Hunter is dependent on ecological opportunity; it is not promised to succeed.

A future discovered genome can fit the same snapshot structure and developmental
preview. Saving, collections, unlocks and a genome editor are deliberately absent.

## 7. Evolution

The authored mutation tempo produces approximately 61% unchanged offspring
attempts, 22% controller edits, 12% property edits and 4.5% geometry edits. Role,
node and module changes occupy the remaining small probability. At most one
ordinary mutation is drawn per attempt; meta-mutation is independently very rare.
Edits are inherited. No successful architecture is repaired or protected.

Continuity is primarily small mutation magnitude and a high unchanged fraction,
not viability filtering. Invalid memory/topology is rejected; sterile or damaged
biological outcomes are allowed. Display hues track ancestry/divergence and have
no energetic consequence. A birth attempt keeps its proposed DNA while waiting
for capacity instead of repeatedly drawing for a conveniently small offspring.

## 8. Ecology

Six independent patches slowly move and pulse according to world time and their
initial phases. Nominal total productivity is 9 energy/second before the pulse
factor; each mote carries 0.12 energy and lasts at most 55 seconds. Every mote is
finite, drifts, and can be acquired only by contact. A player handful adds 48
ordinary motes, totaling 5.76 energy, once. There is no permanent player source.

Cells pay upkeep, sensing, actuation and construction. Energy diffuses along
physical bonds. Local attack extracts material, digestors convert raw energy,
and death recycles a fraction into food. Default unperturbed runs did not establish
predator populations; attack/digestion are separately covered by the hunting tests.

The tank has 240 cell slots and 5,000 mote slots. Developing offspring reserve
space, so capacity pressure can begin below 240 visible cells. Crowding comes
from contact, local resource competition and birth geometry, not separation AI.
Extinction is permanent unless the player introduces new life or resets.

## 9. Visual communication

Lineage hue is the primary identity. Energy affects brightness, young construction
fades in, starvation dims cells, and disconnected debris fades. At close zoom,
internal role cues distinguish anatomy. Actual paid thrust creates exhaust;
feeding, attack and digestion drive their own activity flashes. These effects
report simulated state rather than inventing behavior. The selected organism is
followed by camera position, with only its generation shown in the interface.

## 10. Observed events

**Authored, not evolved:** the initial four forms, all six catalog genomes,
starting colors, patch geography and starting reserves.

**Actually recorded in the final default:** normal cell-by-cell offspring,
lineage loss and expansion, population turnover, and blue/magenta groups moving
through different feeding regions. At ten and twenty simulated minutes, the
Simulator showed mixed local concentrations and open water, not one global pile.

Seed 42 retained two lineages at 30 minutes; seeds 43 and 44 retained one. Their
histories differed. In seed 43, a generation-15 nine-node descendant was recorded
as a complete nine-cell body at 1,200 seconds; at 1,230 seconds it had lost three
physical cells while retaining the nine-node genome. In the hour-long seed-42
soak, five-node descendants appeared at generations 25 and 34. One was recorded
across a 30-second interval. Neither had a sampled mature child. These are
observed variations and failures, not evidence of a lasting complexity increase.

## 11. Failure cases

Lineages can disappear. Ribbon and Fork are not durable in every run. A single
lineage can eventually dominate; introducing Pebble can strongly change the
competition. Some mutant bodies lose cells or fail to leave descendants. Dense
feeding groups occur locally, and capacity can stall construction. Hunter can
starve without useful prey contact. These are visible ecological outcomes, not
hidden automatic failure/recovery scripts.

## 12. No-cheating audit

The normal path contains no scripted creature pursuit/fleeing, hidden fitness
bonuses, lineage protection, complexity subsidy, forced novelty or species rescue.
Sensor queries return local signals. Controllers turn those signals into paid
cell force; contact determines feeding and attack. Patch motion never reads
population, lineage or hunger. The catalog and initial reserves author only
starting conditions. Diagnostic diversity, phenotype and ancestry measurements
never feed back into selection. Historical ablation/debug fixtures are not
selected by the normal UI or default configuration.

## 13. Test and soak evidence

| Default seed | Simulated duration | Births | Maximum mature generation | Final mature organisms | Final lineages | Peak cells |
|---|---:|---:|---:|---:|---:|---:|
| 42 | 30 min | 450 | 23 | 30 | 2 | 214 |
| 43 | 30 min | 391 | 23 | 26 | 1 | 205 |
| 44 | 30 min | 423 | 20 | 25 | 1 | 209 |
| 42 | 60 min | 970 | 42 | 28 | 2 | 220 |

All sampled worlds remained finite with valid connections. Maximum absolute
energy-accounting error in the hour soak was 0.0000212. The accounting correction
was also checked against identical outcome hashes in the earlier cast: it did
not change trajectories.

The complete 25-suite Release run passed, including host Metal parity/grid,
physical locomotion, hunting, ecology, development and hereditary continuity.
Focused placement regression includes invalid snapshots, overlaps, edges,
capacity, reserved offspring space, finite food and the energy ledger. Focused
AddressSanitizer/UndefinedBehaviorSanitizer checks cover the player world,
resource input, specimen placement and continuity.

Evidence: `refoundation/cast`, `refoundation/soak`, `playtest-tests.log`,
`placement-regression.log`, `playtest-sanitizers.log`, and
`validation-summary.json`. Earlier candidate directories document tuning,
not the delivered default.

## 14. Simulator and device status

Release builds target iOS 17+. Simulator uses the iPhone 17 Pro / iOS 26.5 runtime,
CPU physics and Metal rendering. The device-target arm64 Release build compiles
and links with Metal physics available. No physical iPhone was used for this
refoundation pass; device thermal, battery and touch performance are unclaimed.

The final ecological candidate was observed in Simulator from opening through
30 simulated minutes at developer 8×, with captures keyed to logged simulation
time rather than wall-time estimates. Normal-speed interaction checks are
separate from that accelerated ecological observation. Catalog selection,
placement while paused, finite-food feedback, resume, tap-to-follow, Wide and
reset were verified visually. Flow selection was verified, and its physical
field is covered by host tests. Desktop drag injection repeatedly produced
tap-like input, so it did not reliably validate native pan/Flow drags. A physical
pinch was not exercised. Coordinate round-trip tests pass, and the gesture
handlers were reviewed; direct touch gesture feel remains a human check.

During the recorded Simulator session, median frame rate was 60 fps (minimum
logged 58), median stepping rate 960 steps/second at developer 8×, and median
biology time 0.217 ms/step. Logged memory ranged from 140.6 to 226.1 MB and ended
at 141.9 MB. This is bounded Simulator evidence, not a device benchmark.

## 15. Important non-blocking limits

Worlds do not persist across app termination. The screen cannot guarantee that
an evolved structural variant survives or that several lineages persist forever.
Selection often narrows the cast, and major morphological change is rare in one
session. There is no lineage genealogy browser or saved-discovery UI yet.
Simulator evidence is not a device shipping/performance certification. Native
pan/pinch/Flow touch feel was not established by the desktop automation; it is an
explicit part of the human input check, not claimed as a passed gesture test.

## 16. Human play instructions

Open ALIEN Mobile. Use Life, Food or Flow. Tap life to follow; drag/pinch the camera.
Use time and ↺ freely. Play uncued for at least 30 minutes; continue to 45–60 if
you naturally want to. Read this report afterward.

## 17. Questions for the human

Is it fun to intervene and watch? Can you understand enough to form a hypothesis?
Do you care about an organism or its descendants? Are surprises meaningful?
Does the narrowing of lineages create tension or boredom? Do the tools feel
consequential? Do you voluntarily want another experiment?

## 18. Verdict

READY FOR REAL HUMAN FUN/DEPTH PLAYTEST

This verdict covers a coherent, stable candidate with the stated validation
limits. It does not establish fun, lasting diversity, or physical-device input
quality. Development is frozen pending human feedback. Desktop/Simulator control
was stopped at the owner's request; no further GUI actions are required for the
handoff.

Reference: [ALIEN by chrxh](https://github.com/chrxh/alien), for connected-cell,
developmental and signal-driven physical life. This project is a selective native
mobile implementation; it does not claim ALIEN's desktop scale or full biology.
