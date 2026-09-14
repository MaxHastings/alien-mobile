# Creature catalog: a mechanical field guide

The catalog is a shelf of genomes, not a roster of protected species. A placed
specimen develops through the ordinary construction path, pays the same energy
costs, senses only locally, inherits mutation rates, can fragment, and can go
extinct. Its name and one-line card are the only catalog-only data.

## Foundations being used

| Foundation | What is real in the engine | Where to see it |
| --- | --- | --- |
| Form | Connected cells, elastic links, articulated angle constraints, per-link stiffness, branching developmental genomes | Ribbon's long tail, Crown's off-centre spar, Vault's rear lobe |
| Motion | Paid axial thrust, contractile link length changes, joint bending, drag, collisions, currents | Dart's fins, Ribbon's contraction/bend cycle, Crown's oscillator-driven sweep |
| Local control | Food, creature, and obstacle sensing; finite-range signaling; relays; neural weights; oscillator and memory cells | Dart/Beacon food tracking; Lancer's delayed pursuit |
| Economy | Finite drifting motes, contact uptake, cell capacity, energy diffusion, depots, construction and recycling | Dart's cheap body versus Vault's reservoir |
| Conflict | Local attack, raw capture, digestion, defender interception and damage fragmentation | Lancer against Vault |
| Evolution | Heritable controller, geometry, role, organ-property and developmental-program mutation | Every entry is an ordinary starting genome |

## The six specimens

| Specimen | Card | Physical identity | Opportunity | Cost / weakness |
| --- | --- | --- | --- | --- |
| **Dart** | *A four-cell nose that follows thin food trails.* | Small arrow: receptor at the nose, two offset thrust fins | Cheap construction and high food sensitivity make it the best simple scout on sparse trails | No depot, armor, radio, or predation; weak when food is broad but contested |
| **Ribbon** | *Flexes its whole tail to swim in a loose S.* | Long eight-cell body, oscillator, contractile tail link and bending tip | Its changing geometry turns fixed thrust into a broad swimming path; it can cover disturbed or moving food | Lots of cells and motor upkeep make quiet scarcity expensive |
| **Crown** | *An off-centre spar sweeps it through looping searches.* | Asymmetric Y with a tall oscillator spar and bending rudder | The spar continuously changes fin loading, sampling a much wider area around moving patches | Its loop can miss a narrow trail and the oscillator is paid hardware |
| **Vault** | *Carries a food vault behind a defensive lobe.* | Compact forager with a rear depot and attached defender | Stores a pulse and can blunt attacks that occur near its defensive lobe | More construction mass; defense does nothing against absence of food or distant attacks |
| **Beacon** | *Broadcasts a local food signal across nearby bodies.* | Forked seven-cell radio body: food receptor, sender, receiver, reserve and two fins | Bodies in a loose cloud share whatever local gradient one has found | Radio range and organ maintenance are liabilities when isolated; it has no species/ally rule |
| **Lancer** | *Remembers prey, strikes, then digests the capture.* | Seven-cell hunter: long receptor, integrator, paired pursuit fins, attack tooth, digestor | Converts nearby living energy into usable food, particularly in a crowded prey field | No food receptor; attack and digestion consume energy and it starves if prey disappears |

The first four organisms placed in a normal new world are **Vault, Beacon,
Ribbon, and Crown**. That makes the opening readable and leaves the lean scout
and predator as deliberate player experiments rather than making either a
default winner.

## Validation approach

The test suite separates system checks from ecology checks:

- `AlienMobileArticulationTests` proves morphology and motor placement alter
  motion, and verifies paid bending, contraction, stiffness, and momentum.
- `AlienMobileLocomotionDiversityTests` compares equal controllers on altered
  tail geometry and verifies that the contractile swimmer remains viable in a
  finite-resource world.
- `AlienMobileFunctionalTests` verifies finite depot capacity/diffusion,
  attached-only defense, paid range-limited communication, and memory.
- `AlienMobileHuntingTests` disables sensor, motor, attack, and digestion in
  turn; a hunter cannot reproduce in the rich-prey fixture without the full
  pathway.
- `AlienMobileCatalogEcologyTests` freezes founder mutation and rotates six
  starting positions across moving-patch, broad-still, and sparse-fast-trail
  environments. It records survivors, mature births, winner, attack and
  digestion energy rather than enforcing equal shares.
- `AlienMobileGardenContinuityTests` turns real mutation back on for Dart,
  Ribbon, and Beacon and requires multi-generation recognizable descendants.

Run the catalog-focused screen with:

```sh
ctest --test-dir mobile/build-release --output-on-failure -R 'AlienMobile(CatalogEcology|SpecimenCatalog|GardenContinuity|LocomotionDiversity|Hunting|Functional|Articulation)Tests'
```

The ecology screen is a guard against a catalogue member being shut out of all
tested landscapes. It is deliberately not an equal-win-rate target: extinction
in a particular run remains part of the game.
