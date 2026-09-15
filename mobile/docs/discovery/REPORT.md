# Creature catalog refoundation

> Superseded as the catalog-selection protocol by the two-phase incubation
> pipeline in [docs/incubation/PROTOCOL.md](../incubation/PROTOCOL.md). This
> report remains the baseline comparison for the pre-incubation catalog.

**READY FOR HUMAN CATALOG PLAYTEST**

**Long-duration follow-up:** subsequent two-hour default-world tests found loss of starting-strategy diversity and, in some seeds, convergence to tiny passive bodies. See [the follow-up report](../longevity/REPORT.md). The playtest build remains available; this earlier verdict does not establish sustained unattended ecological diversity.

The playable catalog is now **Skiff, Thread, Whorl, and Husk**. All four are exact archived descendants of ordinary simulation organisms. None was manually tuned into its final form. The catalog presents larger visual previews, names, and one short description in a two-by-two layout.

This establishes mechanical readiness for a human playtest, not proof that the catalog is fun or globally optimal. Skiff remains the strongest generalist. Thread is a slower, less competitive specialist. Evolution can eventually erase a starting architecture; the catalog does not preserve identities forever.

## 1. Engine possibility space

Inspection covered the implementation, not just documentation. The detailed inventory and experimental protocol are in [METHOD.md](METHOD.md).

| Primitive | Actual consequence |
|---|---|
| Tree morphology, stiffness, springs, angular constraints | Contact area, drag, torque, flexibility, diffusion distance and vulnerability |
| Repeated/branched developmental modules | Compact inherited programs grow larger physical bodies; every cell must be constructed and maintained |
| Thrust motors | Local, directional force paid from cell energy, requiring a physical orientation edge |
| Contractile and bending motors | Paid changes to spring lengths and angular targets; real articulation, but no free center-of-mass propulsion from internal forces in isotropic drag |
| Food, creature and obstacle sensors | Local signals in physical body axes, with range-dependent upkeep; no species-aware target selection |
| Eight-channel neural processing | Edge propagation, residual or replacement tanh controllers, biases and recurrent weights |
| Oscillation | Sine/square signals and inherited phase; generators produce signals, not energy |
| Memory and communication | Delay/integration and paid, attenuated broadcasts exist; no strong independent ecological niche was established in this search |
| Storage | Depots increase actual capacity; reserves must be acquired, and diffuse over physical connections |
| Attack, digestion, defense | Local extraction with physical occlusion; raw material must be converted; adjacent defenders spend energy to resist |
| Construction | Paid staged development, birth tethers, offspring release, finite capacity reservation and heritable developmental failure |
| Energy and death | Finite particle interception, metabolism, organ costs, local starvation, fragmentation and partial recycling |
| Mutation | Neural, geometric, property, role, node, module, section and constructor changes, plus inherited mutation-rate changes |

The useful space is broader than the selected catalog. Implementation of an organ is not evidence that an authored organism uses it effectively.

## 2. Search / evolution method

`AlienMobileCatalogDiscovery` links the actual core and advances the same 1/120-second CPU simulation used by Simulator. There is no surrogate locomotion model, runtime fitness score, population equalizer, or rescue mechanism.

1. Archive the historical genomes and add primitive and contractile-feeder starting material.
2. Run 21 separate natural-selection colonies: seven starting architectures × three resource profiles, 600 seconds each. Three founders occupy different patches with randomized positions and orientations.
3. Use the existing generic `MutationRates` defaults during discovery, rather than the historical catalog's especially conservative rates. Export generation-two-or-later adults only after they produce two mature children. Their genomes remain unedited.
4. Preserve up to 40 qualifying adults per colony, then screen representative later-generation and structurally different descendants alongside ancestors.
5. Screen 24 genomes in ten fresh-seed/profile combinations each. Inspect physical poses, compare fixed-genome ancestors and descendants, and ablate mechanisms.
6. Investigate failures with prey-rich and earned-reserve famine experiments. Examine additional smaller Crown descendants when the first choices prove neutral or inefficient.
7. Freeze the four selected genome files, then test them with their inherited mutation rates on unseen seeds and perturbed world parameters. Follow with mixed-world rotations.
8. Compile those exact archives into the game and verify equality in a regression test.

The retained data contains **527 run directories and over 50.4 observed simulated hours**, conservatively summed from recorded histories. Repeated verification runs are not counted twice. No single lifespan, speed, body-size, birth-count or population statistic determined selection.

The archive favors early establishment because it retains the first 40 qualifying adults. This is a bounded discovery process, not an exhaustive optimum search. Separate colonies preserve search opportunities, not runtime species privileges.

## 3. Starting material

**Human-authored:** historical Dart, Ribbon, Crown, Vault and Lancer; existing primitive and contractile-feeder fixtures; the engine; broad experimental environments; selection of the final portfolio, names and descriptions.

**Simulation-derived:** every final genome change, including geometry, controller changes, a lost defense organ and inherited mutation parameters. Each finalist had matured and reproduced inside its training colony before export. Final DNA was not edited after selection.

All research placements and the final catalog use the same finite **1.0 energy per cell**, capped by generic capacity. Depots do not start full. Search/screening worlds omit the game's initial 160-mote wake and must establish on incoming resources. The playable reset retains its ordinary finite wake.

[checksums.json](finalists/checksums.json) identifies the exact DNA files. `DiscoveryCatalogTests.cpp` checks complete genome equality, including every neural weight, construction field and mutation parameter.

## 4. Candidate history

| Candidate | Finding and disposition |
|---|---|
| Historical hunter / Lancer | Extinct in all three training colonies; no sustained lineage in ten screening trials. A prey-rich follow-up also failed. Removed; no attack discounts or hunting shortcuts added. |
| Primitive descendants | Strong population growth in several environments, including reduced two-cell bodies. Preserved in the archive, rejected as redundant compact strategies with weak visual differentiation. |
| One-cell Crown descendant | Extremely rapid reproduction, but lost the defining body and behavior. Preserved as evidence of simplification, not promoted because it topped population counts. |
| Thirteen-cell Crown | A real unplanned developmental expansion. Viable, visually striking, but substantially slower; reducing its arms improved every tested non-famine profile. Rejected. |
| First seven-cell Crown shortlist | Some mutations affected unused parameters and produced identical fixed-genome outcomes to the ancestor. Rejected as evidence of improvement; additional archived geometry variants were tested. |
| Contractile descendants | Viable; tail actuation helped in one scarce-patch paired trial, but broader benefit was inconsistent and the silhouette/feeding strategy overlapped the compact feeder. Archived, omitted from the smallest portfolio. |
| Ten-cell Ribbon descendants | Extra structure was viable but did not consistently outperform the nine-cell alternative. Omitted. |
| Vault without defense | Evolution removed an upkeep-bearing defender while retaining storage. This became Husk after reserve and famine tests. |

## 5. Surprising discoveries

The strongest surprises were **developmental simplification**, **spontaneous expansion to thirteen cells**, and **loss of an unnecessary defense organ**. These were actual inherited mutations, not changes scripted into the search.

No wholly new successful hunting or communication architecture emerged. The final portfolio is principally simulation-refined human starting material. Claiming autonomous invention of four entirely new body plans would overstate the evidence.

## 6. Final catalog

### Skiff

- **Form:** four-cell fork, two propulsors and a sensory nose.
- **Mechanisms / behavior:** food-responsive differential thrust; observed translation and turning.
- **Strengths:** fast establishment and strong performance in scarce moving food; best generalist in the mixed tests.
- **Weaknesses:** prolonged food interruptions eliminate it; limited storage; some descendants simplify substantially.
- **History:** Dart colony, seed 238, adult **346**, generation **14**, archived at **558 seconds** after two mature children. Inherited geometry and controller edits, including altered sensor/fin offsets and relay/recurrent parameters.
- **Evidence:** fixed-genome default comparison produced 163 mature births versus the ancestor's 150; scarce comparison 89 versus 87, with 28 versus 20 final adults. Disabling sensors in the scarce paired trial reduced 89 births to 2 and caused extinction; disabling motors reduced them to 5 and also caused extinction.
- **Slot:** the compact active forager, with demonstrably useful sensing and propulsion.

### Thread

- **Form:** nine-cell serial body with a driven head and long compliant tail.
- **Mechanisms / behavior:** propulsion plus distributed physical interception; observed trailing, turning and deformation. It is not an internally powered worm.
- **Strengths:** genuine benefit from its extended collector body; continuing distinctive long descendants.
- **Weaknesses:** slow, expensive construction; distant cells depend on diffusion; weaker mixed-world competitor and occasional scarce-patch extinction.
- **History:** Ribbon colony, seed 201, adult **91**, generation **6**, archived at **367 seconds** after two mature children. The simulation changed head/tail geometry and inherited signal parameters.
- **Evidence:** fixed default comparison produced 53 births versus the ancestor's 46. Screening scarce-profile runs produced 47 combined births versus 25 for the ancestor, though a fixed scarce trial failed to persist: this is not uniform improvement. Shortening structural edges reduced default births from 53 to 19 and final adults from 21 to 5; disabling motors reduced births to 8.
- **Slot:** the long articulated collector. It earns a place through physical distinctness and viability, not a tournament win requirement.

### Whorl

- **Form:** seven developed cells from two modules; three repeated arms.
- **Mechanisms / behavior:** tangential propulsors rotate the collector; arms and depots are grown from an inherited developmental invocation.
- **Strengths:** repeated architecture, visible rotation, viable in diffuse food and some dense competitions.
- **Weaknesses:** slower establishment than Skiff; reduced rotational benefit in some contexts; insufficient reserves for the long famine protocol; later mutations sometimes remove or expand arms.
- **History:** Crown colony, seed 201, adult **45**, generation **3**, archived at **250 seconds** after two mature children. An inherited motor-edge geometry change shortened and offset the repeated terminal segment; the ancestor's controller was not manually rewritten.
- **Evidence:** fixed scarce comparison produced 19 births versus 12 for the ancestor, and 10 versus 3 final adults; default performance was lower. Exact-finalist motor ablations across six paired trials reduced total births from **181 to 108** and births in the second half from **77 to 38**. This is a contextual improvement, not universal superiority.
- **Slot:** rotational feeding and repeated developmental structure, visibly different from translation-driven bodies.

### Husk

- **Form:** five-cell compact branched reservoir, with three storage cells and a simple terminal cell.
- **Mechanisms / behavior:** passive interception and earned reserves; largely stationary in observed poses, with growth and reproduction providing its visible activity.
- **Strengths:** continues through food interruptions after accumulating reserves; avoids sensing, propulsion and unused defense upkeep.
- **Weaknesses:** cannot chase a departing patch, establishes slowly and often fails from unfavorable single placements or under competition. Storage is not immunity to immediate starvation.
- **History:** Vault colony, seed 201, adult **47**, generation **2**, archived at **497 seconds** after two mature children. Evolution changed one depot's geometry and replaced the defender with a structural cell.
- **Evidence:** fixed default comparison produced 29 births versus the ancestor's 15, with 28 versus 17 final adults. In three earned-reserve famine pairs, intact Husk ended with 12, 10 and 5 adults and reproduced after interruptions; removing depots caused extinction in all three. Without famine, depots can slow reproduction—an actual cost of this strategy.
- **Slot:** a stationary reserve strategy with a demonstrated ecological niche, rather than another motorized feeder.

[Actual final-genome physical poses](final-physical-motion.png) were inspected at equal spatial scale. The live Simulator was also inspected, including growing descendants and the catalog tray. Still poses establish physical differences; they do not establish affection or fun.

## 7. Generalization test

Final genomes were frozen before the holdouts. Each received **18 mutation-on trials**, 600 seconds each: three unseen seeds × six profiles, using three founders per isolated lineage trial. Unseen seeds also perturb patch anchors, emission, radius and drift by bounded amounts. Finalists were not changed after these results.

Profiles include default, scarce moving food, richer diffuse food, smaller bounded worlds with local currents, earned-reserve famine, and compressed resource geography. The famine profile allows 180 seconds of feeding, then 60 seconds without emission, repeatedly. Existing motes remain physical during emission gaps.

“Sustained” below means at least one mature organism at the endpoint **and** mature births during the second half. It is an offline diagnostic, not a runtime fitness value or a guarantee for every individual placement.

| Finalist | Sustained trials | Mature births | Maximum mature generation |
|---|---:|---:|---:|
| Skiff | 15 / 18 | 4,635 | 29 |
| Thread | 14 / 18 | 997 | 10 |
| Whorl | 15 / 18 | 1,140 | 26 |
| Husk | 17 / 18 | 820 | 8 |

All three active forms failed the three long-famine holdouts. Husk sustained all three. Thread additionally failed one scarce-patch trial; Husk failed one compressed-geography trial. Performance is explicitly conditional.

## 8. Dominance / redundancy audit

**Skiff is the strongest generalist; this has not been equalized away.** Across 42 mixed mutation-on trials with one initial founder of each entry, adult-count leaders were Skiff 28, Whorl 3, Husk 5, one Thread/Whorl tie, and five complete extinctions. Thread did not uniquely lead adult counts. An earlier working observation described it as leading; the tie-aware audit corrects that.

The 24 fully rotated repeats additionally measured actual physical-cell occupancy, including developing offspring. Leaders were Skiff **15**, Husk **6**, Whorl **1**, and two complete extinctions. Examples:

- Rich dispersed food, seed 5101: Skiff 35 cells, Thread 30, Whorl **116**, Husk 56.
- Earned-reserve famine, seed 5105: only Husk remained, with **53 cells**.
- Compressed geography, seed 5100: Skiff extinct, Thread 46, Whorl 31, Husk **155**.
- Default, seed 5100: Skiff 73 cells and Thread 72, with Whorl 9 and Husk extinct.

Thus Skiff does not dominate essentially everything. Nevertheless, small-body success and late simplification remain real tendencies of this engine. Thread is retained as a viable, mechanically and visually distinct collector, not because every entry must win. Its weaker competitive position is disclosed for the human playtest.

Across the 42 mixed trials, final survival occurred in 30 Skiff, 19 Thread, 21 Whorl and 19 Husk trials. No finalist is broadly useless. Equal population shares and equal win rates were never targets.

## 9. Mutation-on continuity

The final catalog retains the exact mutation parameters inherited from discovery. No special preservation code or post-selection reduction was added.

“Recognizable” requires the same encoded topology, roles, motor modes, module invocations and branch/repetition counts; small controller/geometry changes are allowed. This is an explicit architectural measurement, not a judgment of recognizability to a person.

| Finalist | Recognizable mature descendants, generations 1–3 | Recognizable across all mature births | Changed adults that themselves produced mature children | Observed mature descendant sizes |
|---|---:|---:|---:|---|
| Skiff | 832 / 957 (86.9%) | 2,986 / 4,635 | 2,009 | 1–7, 9 cells |
| Thread | 539 / 613 (87.9%) | 805 / 997 | 390 | 7–11, 19 cells |
| Whorl | 385 / 452 (85.2%) | 545 / 1,140 | 451 | 1–5, 7–10, 13–16, 34 cells |
| Husk | 505 / 646 (78.2%) | 568 / 820 | 243 | 3–9, 11 cells |

Recognizable descendants still existed at the endpoints for every finalist across the holdout set. Later identity erosion is substantial, especially in Whorl and Skiff; some surviving descendants become very small. The selected starting structures last through several real generations, but are not permanent species. Whether this pace feels meaningful is a human question.

## 10. Generic engine issues found

No generic runtime mechanics needed changing to reach this portfolio.

- Contractile locomotion has a genuine limitation in isotropic drag. Internal articulation is real, but treating it as self-propulsion would be misleading. No fake thrust was added.
- Some inherited parameters are inactive in their current organs/controllers. Genotypic novelty alone was therefore insufficient; fixed-genome replay rejected neutral “improvements.”
- Defense had insufficient opportunity to repay its cost in the selected reservoir's ecology. Evolution removed it; the catalog follows that evidence.
- Hunter failure did not establish a generic combat implementation bug. Existing combat tests pass; no costs or AI were altered to rescue this concept.
- Early famine tests killed nearly everything and did not test storage after reserves had accumulated. A later-feast protocol resolved that experimental-design error.
- An old continuity test referenced catalog index 4. It was corrected to iterate every current entry and inspect invoked-module organs. This was a stale test assumption, not a simulation fix.

The full integrated suite passed 27 of 28 tests before that stale-index correction. The corrected continuity test plus placement and exact-archive tests then all passed. All 28 tests have passing results for their relevant final code. CPU and Metal tests passed. Holdout energy-accounting error stayed below **0.000005 energy units**; finite-state and connection-validity checks also passed.

## 11. No-cheat audit

`CatalogGenomes.cpp` only constructs genome data and UI metadata. `World::addSpecimen` validates and materializes an ordinary genome. Creature runtime state contains no catalog identifier.

No catalog-name/ID decisions were added to sensing, metabolism, actuation, combat, targeting, development, mutation, physics or reproduction. Lineage IDs/hues remain generic ancestry/display information. Opening placements happen once at reset. There is no replenishment, lineage rescue, protected reproduction, bespoke AI, speed bonus or metabolic multiplier.

The only runtime integration change outside catalog data is accepting the four-entry opening cast instead of requiring at least five entries. UI changes affect layout and preview size. Historical human-authored fixtures remain available to research; they are not the playable catalog.

## 12. Playable build status

**Release Simulator build succeeded, installed, launched and visually inspected.**

- Simulator: **iPhone 17 Pro / iOS 26.5**.
- Device ID: `6663A26B-E595-4E16-AA40-027076B713FA`.
- Bundle ID: `com.example.AlienMobilePrototype`.
- App: `mobile/build-sim/Release-iphonesimulator/AlienMobileApp.app`.

From the repository root:

```sh
./mobile/scripts/run-simulator.sh
```

The Simulator is left paused with the catalog open. Select a creature, tap open water, then tap the paused time control to run at 1×. **Life** opens the catalog; **Food** scatters finite particles; **Flow** stirs; tap an organism to follow; drag to pan; Option-pinch zooms; **Wide** shows the tank; **↺** resets.

To reproduce the research pipeline, build `AlienMobileCatalogDiscovery`, export historical seeds with `seeds 101 0 180 seeds docs/discovery/baseline none` using paths relative to `mobile`, then run `run-discovery.py evolve`, `discovery-genomes.py`, and `run-discovery.py assay`. Additional stages are `ablation`, `storage`, `reference`, `crown`, `holdout`, `rotation`, and `final-mechanism`. The selected files and all exact per-run commands are retained. `export-discovered-catalog.py` regenerates the C++ data, and `summarize-discovery.py` regenerates [evidence.json](evidence.json).

## 13. Human questions remaining

- Which forms make you want to place another one?
- Does Thread's visible body compensate for slower, weaker competition?
- Is Husk's quiet reserve behavior discoverable, or does it look inactive?
- Does Skiff feel too prevalent during ordinary play, despite its measured weaknesses?
- Does Whorl's rotation and later branching/simplification make descendants worth following?
- Is architectural change fast enough to surprise and slow enough to care about?
- Do these four entries feel like a coherent invitation to experiment?

These are not settled by the numerical results. Further tuning should follow playing, not an invented fun metric.

**Final verdict: READY FOR HUMAN CATALOG PLAYTEST.**
