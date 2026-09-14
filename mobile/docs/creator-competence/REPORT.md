# Creator competence pass

## 1. Root causes found

The editor gave newly assigned motors only a fixed activation bias and energy-intensity weight. Directional sensor channels were not connected to those motors. Existing replacement-mode neural organs also cut off the residual relay path, so a receptor placed beyond a motor could become behaviorally disconnected. Long and asymmetric bodies exposed this most clearly. Early metabolism, organ upkeep, finite food, construction cost, and dry placement then converted that technical failure into starvation.

## 2. Approach chosen

The release path now performs a small body-aware controller compilation. It derives the authored body frame, motor axes, branch degree, available receptor types, torque leverage, and oscillator presence, then writes a modest relay and directional response into ordinary neural behavior genes. It preserves existing developmental genomes when the player has not changed the expressed body. I rejected a universal controller, runtime autopilot, economic buffs, body replacement, and simulation search because each would erase authored differences or violate the no-helper boundary.

## 3. Architecture changes

`compileCreatorBody` and `prepareCreatorRelease` live in `Creator.h`. The editor remembers the source genome, compiles only an edited body, offers explicit Food versus Creatures sensing, and logs preparation latency. Two headless tools and a CTest target cover varied morphologies, directional response, inheritance, serialization, and latency.

## 4. Create to release

The world pauses while editing. Save & release compares the edited genome with the expressed source body. An untouched developmental specimen keeps its original modular genome. An edited body is compiled in place, named, saved, and passed to the same ordinary placement flow as every other specimen. Placement creates a normal founder; no later repair or guidance occurs.

## 5. Genetic and inheritance integrity

Generated relays, frame rotations, biases, and motor responses are fields of `BehaviorGene` inside the serialized genome. Offspring copy them through normal construction and mutation. The dedicated test constructs a child with mutation disabled and verifies exact DNA equality, then verifies ordinary neural mutation can change it.

The developmental flattening audit found no material competence or descendant-quality loss: source versus flat catalog runs were effectively equal (12 trials, 1,138.4 total survival seconds in both; 36 versus 37 births; 166.4 versus 167.7 food units). The no-edit path now explicitly preserves the original modular genome, so structural evolution remains available for inherited specimens.

## 6. Player-body preservation

Cell count, parent links, relative geometry, stiffness, roles, motor mode, motor strength, motor axis, sensors, storage, attack, digestion, and developmental separation are preserved. Compilation changes controller parameters only. It never deletes organs, moves cells, changes motor orientation, or replaces the body.

## 7. Generalization set

The evaluation set contains 12 generated bodies: compact, long, asymmetric, branched, many-motor, minimal-motor, multiple-sensor, opposed-motor, storage-bearing, hunter, awkward-spinner, and no-motor. Six additional holdouts use zigzag, radial, hammer, rear-eye, life-eye, and seven-segment combination morphologies.

## 8. Baseline versus final

Across 36 releases (12 bodies × 3 seeds), mean ancestor survival increased from 56.65 s to 70.81 s, median survival from 49.52 s to 65.92 s, acquired food from 204.49 to 262.28 units, mature births from 26 to 54, and trials with a living family at 120 s from 5/36 to 13/36. On 18 held-out releases, mean survival increased from 70.69 s to 92.57 s, median from 59.89 s to 116.50 s, food from 169.15 to 208.98 units, and living-family trials from 5/18 to 9/18.

The directional ablation kept the compiled bodies but removed directional sensor weights: food fell to 203.60 units and mature births to 33, showing that the gain is not only slower movement. A receptor beyond a motor changed from zero response to 1.632 response spread. Left and right stimuli generated opposite torque (0.685 and -0.685).

## 9. Behavioral diversity

The final traces retain different displacement, turning, and force patterns. On seed 42, compact, long, asymmetric, branched, many-motor, minimal-motor, opposed-motor, storage, hunter, awkward-spinner, and no-motor bodies all produced distinct responses; the no-motor body produced zero paid motor force. The compiler supplies competence through each body’s own axes and leverage rather than forcing one trajectory.

## 10. Failure cases

The failure audit still records legitimate root starvation for dry compact, long, and hunter designs. Hunter bodies can acquire, extract, and digest prey yet still die when encounters do not cover upkeep. Awkward motor orientations and no-motor storage designs remain slow or stationary. The system does not guarantee survival or reproduction.

## 11. Latency

The 64-cell headless Release preparation benchmark measured 0.039 ms median and 0.050 ms p95 across the 1,000-run benchmark (the later test process had one scheduler outlier). The Simulator app log measured 0.036 ms for the edited body used in the smoke test. There is no training screen.

## 12. No-cheat audit

There is no post-release helper AI, food attraction, steering correction, movement correction, creator-only metabolism, extra reserve, invulnerability, reproduction bonus, or persistent player-creature flag. All controller work ends before release and is inherited DNA.

## 13. Test and Simulator status

The full Release CTest suite passes 32/32. The dedicated competence test passes under AddressSanitizer and UndefinedBehaviorSanitizer. The Release iOS Simulator build succeeds, installs, launches, and was exercised through template → edit → role choice → grow → rename → compile → placement → release. The released custom body is visible in the Simulator and can be paused, followed, and saved.

## 14. Exact human test instructions

1. Run `./mobile/scripts/run-simulator.sh` or open the already installed `AlienMobileApp` in the iPhone 17 Pro Simulator.
2. Tap **Skiff**, then **Edit copy**.
3. Select a motor cell, tap **Turn**, select another cell, tap **Grow**, select the new cell, tap **Sensor**, and choose **Food**.
4. Rename it, tap **Save & release**, then tap the center placement action.
5. Watch the new body for one minute. Its motors should produce paid thrust and respond differently as food moves around it.
6. Repeat with **Creatures** sensing, an asymmetric long body, and a deliberately awkward motor direction. The first two should show a coherent attempt; the last may legitimately fail.
7. Tap **Save specimen**, create a new experiment, and place the saved creature. Its authored body and inherited controller should remain recognizable.

## 15. Verdict

**CUSTOM CREATURES NOW RECEIVE A FAIR CHANCE TO EXPRESS THEIR DESIGN**
