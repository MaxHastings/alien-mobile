# Hybrid create-and-evolve formative playtest

**PLAYABLE WITH SPECIFIC CAVEATS** — September 13, 2026, America/Chicago.

The Release app is installed in iPhone 17 Pro / iOS 26.5 Simulator. This is a
formative candidate, not a device-qualified shipping build. Development breadth is
frozen pending human feedback.

## 1. What the game is now

You choose a strange cellular body, change its anatomy, give it a name, and let it
live in a tank with other organisms. Watch its motors work, offspring assemble,
and inherited differences appear. Save a descendant's genome and use it as the
starting material for another experiment.

## 2. What was removed

This pass removed misleading full-tank warnings for non-capacity placement failures,
uninformative repeated template subtitles, and the loss of the followed parent's
identity at death. It also removed unsafe assumptions that creator inputs always
contain a valid first gene and that truncating a name can split a Unicode character.

The existing product already excluded raw genome/controller panels, permanent food
fountains, developer ecology modes, kill zones, progression and a large catalog from
the player interface. They were not newly removed in this pass. No user saves or
historical source work were deleted. The repository was entirely untracked at entry;
no commit, publication or destructive repository cleanup was performed.

## 3. What was redesigned

Added constrained branch dragging, with stable scale during the gesture and collision
checks between moved and unmoved cells. Existing Turn now respects the same geometry
constraint. Editing preserves the inherited offspring-separation flag instead of
silently restoring independent release. The creator remains a small touch interface.

The library now explains each starting body's mechanism. Following a dead parent
can still reach a surviving direct child; Wide correctly clears that context. Placement
offers a fresh world only for actual capacity pressure, not every failure.

The baseline already contained the creator, saved library, automatic release focus,
child following, and mutation-continuity improvements. These were inspected and
retained, not claimed as newly built here.

## 4. Final core loop

Choose → edit and name → save/release → observe → follow offspring → save a genome →
place or edit another copy. Failure sends you back to a quick experiment, not a chore.

## 5. Creator UX

Tap a cell. Pick a role, Grow a tip, Turn its orientation, or Remove an outer tip.
Undo restores prior anatomy. Tap empty space to grow in that direction. A drag changes
a non-root branch attachment, snapping angle and bounding length. Save & release
stores the named genome and asks for one placement tap. Cancel leaves the source intact.
The world pauses throughout creation and placement. The editor automatically fits the
body; it does not expose a separate zoom tool or a neural-network editor.

## 6. Player-editable biology

Body, thrust motor, local food sensor, storage, contact attacker and digestor. Geometry
and motor orientation are real genome properties. The reproductive constructor is
retained; deletion only removes terminal cells and leaves at least two cells. New growth
stops at 32 cells; bounded inherited bodies up to 64 developed cells can be edited.
Existing specialized roles/controllers remain visible in inherited templates but are
not all offered as new palette choices.

## 7. Initialization and wiring

Editing a copy expands its bounded development into one physical body gene. It retains
expressed geometry, stiffness, organ/controller values, mutation rates and offspring
separation. It discards the source's modular developmental encoding; Place and saving
an unedited descendant retain exact DNA. This distinction matters for future mutation.

New body cells use generic defaults. Assigning a Motor or Attack organ initializes a
neural Activation bias of 0.65 and an EnergyIntensity weight of 0.3. Those values live
in inherited DNA and can mutate. Existing template wiring is retained until the player
changes that organ's role. No runtime process repairs controllers or guides a creature.

## 8. Built-in templates

| Template | Why it earns a place |
|---|---|
| Skiff | Four-cell fork with sensing and two thrust motors; an immediately legible swimmer. |
| Thread | Nine-cell elongated collector with a driven head; long geometry offers a different interception area and construction burden. |
| Whorl | Seven-cell, three-armed rotating body developed from two modules; distinct motion and geometry. |
| Husk | Five-cell passive storage body; exposes the difference between storing a feast and pursuing food. |

All four remain exact archived descendants from earlier simulation searches. Archive
identity tests passed in this pass. Prior mechanism-ablation evidence is historical,
not newly repeated: see `../playtest-pass/HANDOFF.md`. No hunter template is promised.

## 9. Release

Every release uses World::addSpecimenNearby → ordinary founder construction. The same
finite initial reserve is used for built-ins, authored copies and saved descendants.
The placement helper finds nearby available space without changing inhabitants. The
renderer stores a family name for display and follows the released organism. The engine
receives no player-creature class, shield, reproductive exception or ongoing subsidy.

## 10. Terrarium

One 48×48 bounded world, four ordinary starting organisms, six independently drifting
resource patches, finite consumable motes and physical connections. Limits: 240 cells
including offspring reservations and 5,000 motes. Resources do not inspect species,
lineage success or diversity. Death recycles material. Saturation can delay births and
reject introductions. Dominance and extinction are allowed.

## 11. World tools

Retained Food, Mutagen and Flow. Food adds a finite handful of particles. Mutagen lasts
20 simulated seconds in one non-stacking local field; exposure affects conception
probability, not chosen traits. Flow applies force to cells and particles. No direct
organism steering, radiation trait picker or permanent rescue resource exists.

## 12. Evolution and mutation

Each conception samples ordinary inherited mutation machinery. At most one ordinary
event occurs, with rare meta mutation excluded from structural events. Small controller
edits preserve controller mode; architecture can still change through other events.
Mutation changes roles, geometry, properties, nodes, modules or construction data.
Developmental failure remains possible. No rates were raised for spectacle in this pass.

Two fresh 30-minute worlds reached mature generations 39 and 36. This establishes that
continuing functional lineages are possible; it does not establish that every edited
design is viable or that long-term open-ended evolution is solved.

## 13. Lineage and following

Release follows your named ancestor. Tap another body to focus it, Follow child to move
to a living mature direct child, and Wide to leave follow. The label shows generation,
construction and supported birth-mutation categories. Colors provide family coherence.
The dead-parent identity is now retained for child lookup. There is no full ancestry
browser or automatic handoff to an arbitrary distant descendant.

## 14. My creatures

Atomic local JSON storage contains serialized exact genome, name, hue and origin.
Malformed records are bounded and validated when loading. Up to 100 personal entries;
Place, Edit copy and Delete saved entry are available. World state is not persisted.
Verified that authored Crescent and saved generation-2 Amber child survived relaunch.
Amber child was then successfully placed in a fresh world as a new generation-0 ancestor. Their exact exported DNA is alongside this report. Existing Sidefin was preserved.

## 15. Observed authored organisms

Fresh isolated 180-second normal-ecology tests edited each template by adding a motor
or storage tip. Skiff, Thread, Whorl and Husk produced respectively 13, 18, 2 and 4
births. All acquired food; motor-bearing designs produced thrust; passive Husk produced
none. See `creator-tests.log`. The same tests passed under ASan/UBSan.

Fanfin was a five-cell authored Skiff edit with an added motor, released at a fixed
resource patch into the untouched mixed world. Across seeds 42/43/44, 20-minute trials
produced 1/3/3 mature descendants and maximum generations 1/3/2. All three families
were extinct by the end. Movement and food acquisition were measured, not inferred.
See `mixed-creator.log`, birth CSVs and exact ancestor/child DNA.

In Simulator, Crescent was independently built with Grow and a new motor, renamed,
saved and released. Its body moved and began building offspring; the followed founder
then died. We did not verify a mature Crescent descendant. Do not confuse Crescent
with the Fanfin headless fixture or the later Amber child from a starting lineage.

## 16. Observed inherited changes

Fanfin seed 43 produced births at 6.0, 34.15 and 40.15 simulated seconds: controller,
controller, then role mutation, reaching generation 3. These bodies still had five cells.
Seed 44 showed property and controller changes. The authored added motor is not an
evolved novelty. No cell-count transformation is claimed for these Fanfin trials.

The ordinary 30-minute worlds recorded 59 and 63 completed structural-mutation births
respectively (see final population CSV rows). Simulator following displayed generation
1 with function variation, then generation 2; the latter was saved as Amber child.
Visual movement alone was not treated as proof of a genome change.

## 17. Failure cases

Fanfin became extinct in every tested mixed world despite reproducing. Crescent's
founder died during the UI trial. Capacity pressure occurred in longer worlds. Seed
913 ended with one extant lineage, which is allowed; differing outcomes are not
promised permanent diversity. Bad geometry, controller mismatch, starvation and
construction failure remain genuine risks, not errors the game automatically repairs.

## 18. No-cheat audit

Code-inspected the live simulation, resource economy, actuation, placement and mutation
paths. No scripted pursuit/fleeing, catalog-specific buffs, player protection, novelty
bonuses, complexity subsidies, lineage rescue or forced interesting mutation were found
in the normal experience. IDs/lineages feed display and ancestry, not reproductive
success. Storage changes real capacity; motors consume energy for force; sensors incur
cost; attack transfers contact resources that digestion must convert. Basic cells still
absorb food. Specialized anatomy is a contextual opportunity, not a universal advantage.

Authored templates, finite founder energy, initial neural defaults and player-triggered
food are explicit starting conditions/interventions. Historical research/debug fixtures
remain outside the normal player flow. Reference: [ALIEN](https://github.com/chrxh/alien)
for connected physical bodies and inherited biological machinery.

## 19. Tests and bounded runs

- All 31 baseline CTest targets passed, including host Metal and grid checks: `baseline-tests.log`.
- Final creator regression checks passed, including geometry rejection, controller
  preservation, invalid inputs, offspring separation and serialization: `creator-tests.log`.
- Final creator tests passed with AddressSanitizer and UndefinedBehaviorSanitizer:
  `creator-sanitized.log`; no diagnostic reported.
- Fresh 1,800-second worlds: seed 913, 841 births, mature generation 39, 35 final adults;
  seed 1913, 765 births, mature generation 36, 39 final adults. Peak sampled cells 210/218.
  Maximum energy-accounting error below 0.000016. Finite values and connection integrity
  checked at observation samples. Runs completed in about 33 host seconds each.
- Three 1,200-second authored mixed-world trials checked integrity every simulated second.
- No fresh multi-hour soak or physical-device memory/thermal test was performed. These
  bounded runs are not presented as proof against all leaks or long-run pathologies.

## 20. Simulator/device status

Release built, installed and launched on iPhone 17 Pro, iOS 26.5. Simulator uses CPU
physics and Metal rendering. Host Metal tests passed separately. No physical iPhone
was tested in this pass; device thermal, touch feel and sustained GPU behavior remain
unqualified. `simulator-build.log` records the final native build.

UI verified: template selection; edit copy; selecting cells; tap-empty-space growth in
an earlier build; final Grow, role assignment, Turn/Undo and naming; Save & release;
ordinary placement through the accessible center action; automatic follow; death text;
tapping another organism; Follow child; named descendant save; persistence after restart;
food, mutagen feedback, speed, pause/resume, Wide and accessible physical stirring.

## 21. Non-blocking limitations

Direct coordinate automation intermittently returned a host `noWindowsAvailable` error.
Direct branch dragging, pinch zoom and freehand Flow dragging are therefore not marked
end-to-end verified. Geometry mutation/rejection is headless-tested; button-based editing
is verified. The editor fits rather than manually zooms. Only portrait iPhone 17 Pro was
visually reviewed. Larger evolved bodies may be placeable but not editable. Predation
organs pass engine tests, but an authored hunting design was not visually qualified here.
The 240-cell cap and short-lived custom designs are material playtest caveats.

## 22. Exact human playtest instructions

Launch ALIEN Mobile. Pick a creature; Place it or Edit copy, then Save & release.
Tap life to follow it. Explore freely and stop whenever you want.

To rebuild/relaunch from the repository root: `./mobile/scripts/run-simulator.sh`.
Do not read mechanism or optimization advice before the first uncoached session.

## 23. Questions requiring a human

Is creation satisfying? Does naming and releasing create ownership? Is death understandable
enough to provoke another design? Do generations feel related? Does following or saving
an offspring matter? Is there enough discovery to keep watching voluntarily? Observe
spontaneous actions, confusion, reactions and the point they choose to stop; do not
require an hour or tell them which strategy ought to win.

## 24. Verdict

**PLAYABLE WITH SPECIFIC CAVEATS.** The create/release/observe/follow/save loop is concrete
and runnable. The caveats above must remain attached to the handoff. Stop adding major
features; use the candidate for a formative human session before further product work.
