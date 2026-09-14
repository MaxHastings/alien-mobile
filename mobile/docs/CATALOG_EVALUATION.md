# Catalog evaluation — 2026-09-13

## Decision

**CATALOG IS READY FOR HUMAN EVALUATION.**

This is an ecological portfolio, not a symmetric roster. The entries are data
only: there is no catalog ID, runtime subclass, resource preference, or
creature-specific modifier after placement.

## Foundation inventory

The current engine supports physical cell trees; springs, joint angles and
stiffness; paid axial thrust, contraction and bending; collision/current/drag;
energy and creature receptors; edge-propagated signals, neural transforms,
oscillators, delay/integrator memory, paid local radio; finite moving food,
contact uptake, diffusion and storage; local attack, raw capture, digestion and
attached defense; construction, death recycling and fragmentation; and
heritable controller, geometry, organ-property, role and developmental-module
mutation. The catalog uses the systems with a clear present-day ecological and
visual consequence. Obstacle sensing and multi-gene developmental modules are
not yet catalog entries: their current effects are not distinct enough to earn
a slot over the six mechanisms above.

## Final portfolio

See [CREATURE_CATALOG.md](CREATURE_CATALOG.md) for player-facing cards and the
complete anatomy/strategy table.

| Genome | Cells | Defining testable system | Why it remains |
| --- | ---: | --- | --- |
| Dart | 4 | Food receptor → paired thrust fins | The only truly lean, direct forager; it establishes the low-complexity end of the design space. |
| Ribbon | 8 | Oscillator → contractile link + bending tail | A long physical swimmer whose motion and cost differ from direct thrust. |
| Crown | 6 | Asymmetric oscillator spar + bending | A distinct looping/search geometry rather than a second tail or a recolor. |
| Vault | 6 | Depot capacity/diffusion + attached defender | Its survival timing and attack response come from physical organs rather than armor stats. |
| Beacon | 7 | Paid finite-range sender/receiver | A body whose value increases when a player places a loose group. |
| Lancer | 7 | Creature sensing + memory + attack + digestion | The only trophic specialist; it makes living bodies into a conditional resource. |

The removed identities were **Glider**, **Kite**, **Bastion**, and **Chorus**:
their useful genome material was retained, but their presentation was too
generic. Dart is now a genuine no-reserve baseline; Crown, Vault and Beacon
name their actual anatomical mechanism rather than an archetype. No functional
entry was deleted because each still occupies a different generic engine
system.

## Causal mechanism validation

- Catalog Ribbon is used directly by `AlienMobileLocomotionDiversityTests`.
  With equal inherited signals, moving its tail changes trajectory by `0.358`
  world units over the fixture. Its flexible body remains viable under finite
  physical food (67 births, versus 117 for the cheap four-cell direct body),
  making its cost visible rather than decorative.
- `AlienMobileFunctionalTests` proves that depot capacity is finite and diffuses
  through real cell connections; that defense reduces only attached local
  attacks; and that radio is range-limited, synchronous and paid.
- `AlienMobileHuntingTests` removes receptor, motors, attacker and digestor in
  turn from the hunt/digest architecture. In rich prey, the complete pathway
  made one birth and digested 25.73 energy; no attacker or no digestor made
  zero births. This is the complete mechanism that Lancer combines with its
  slower memory node.
- `AlienMobileBehaviorSelectionTests` shows a receptor-to-fin response receives
  more food exposure and more births than the same body with only those weights
  removed. This is Dart's basic sensing principle.

## Frozen-founder environment matrix

`AlienMobileCatalogEcologyTests` ran six rotated starts for 180 seconds in each
of three ordinary resource landscapes (18 runs total): a moving concentrated
patch, a broad nearly still field, and sparse fast short-lived trails. Mutation
was frozen only for this diagnostic.

| Result over 18 runs | Dart | Ribbon | Crown | Vault | Beacon | Lancer |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Present at the end | 6 | 5 | 6 | 6 | 1 | 1 |
| Produced mature offspring | 14 | 5 | 10 | 6 | 2 | 2 |
| Largest final lineage | 4 | 0 | 1 | 1 | 0 | 0 |

There were **zero 70%-cell monopolies** in all three profiles. The broad still
field was the only profile that retained an end population: Dart led four
seeds, Crown one and Vault one. Moving and sparse profiles were harsh enough
to eliminate end populations, but still produced transient descendants (Dart,
Ribbon, Crown and Vault) and trophic activity. This is a deliberately demanding
matrix, not a claim that every founder must persist in every landscape.

The audit result is: **no universal dominant specimen observed.** Dart is the
best broad-food founder in this particular screen, but it neither monopolized a
run nor dominates the mobile/scarce cases; it also lacks all conflict and
reserve options. Beacon and Lancer are intentionally conditional: the former
needs nearby radio bodies and the latter needs prey. Their favorable conditions
are respectively a player-created cluster and the rich-prey hunt fixture.

## Mutation-on result

`AlienMobileGardenContinuityTests` enabled normal open structural mutation in
ordinary worlds. Dart reached generation 6 (21 births, 22 recognizable mature
bodies); Ribbon generation 4 (10 births, 11 recognizable bodies); Beacon
generation 4 (11 births, 12 recognizable bodies). Across 100,000 mutation
samples, 33,015 were controller/property changes, 3,656 geometry changes, 514
architectural changes and only three meta changes; a 2,500-step inherited chain
had no sterile development attempt. That preserves identity long enough to
observe descendant variation without freezing evolution.

## Player experience

The Life tray presents real silhouette thumbnails, a short name, and one
mechanism sentence. Lineage hue remains the primary color. At close range the
renderer distinguishes sensors, motors, storage, attack and radio through its
existing functional glow/ring treatment, so the anatomy teaches itself while a
player watches.

## Stability

The focused foundation and catalog suite passed: specimen catalog, garden
continuity, sensing selection, functional organs, articulation, locomotion
diversity, and hunting. The 18-run ecology matrix completed without invalid
state, invalid connections, or a monopoly.

## Human test build

```sh
./mobile/scripts/run-simulator.sh
```

On the existing iPhone Simulator: tap **Life** to inspect the six cards, tap
open water to place one or several specimens, use **Food** to create a finite
pulse, **Flow** to disturb moving food, then use **1x/2x/4x** and tap a body to
follow it. Suggested comparisons: one Dart versus Ribbon in a trail; several
Beacons in a loose cluster; Vault plus Lancer near a food pulse; Crown under a
strong Flow gesture. Watch descendants after accelerating time; a reset starts
a fresh normal experiment.
