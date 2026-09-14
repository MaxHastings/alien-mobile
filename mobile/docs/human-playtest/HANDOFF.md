# Human playtest candidate — 14 September 2026

**READY FOR HUMAN PLAYTEST (Mac iPhone Simulator).**

ALIEN Mobile is now a creature experiment loop: build or edit an ancestor, compare it
with its original in a controlled observation, release ordinary DNA into the ecosystem,
follow living relatives, preserve a descendant, and try another body. The player authors
an ancestor; physical biology and inheritance determine what happens next.

## Product changes

- Anatomy edits preserve controllers. An unchanged edit releases exact original DNA.
  Unambiguous local edits preserve the developmental program, unused genes and growth
  timing. An edit to a shared repeated part may require an independent unfolded body;
  the creator explains this before release. Undo restores the original structure.
- Rewire is explicit and undoable. Synthesized controllers become ordinary inherited
  DNA, with no runtime steering or protection.
- Try body and Original run the same 12-second finite-food practice arrangement.
  Parent outlines, paid-force lines, food sensing, depletion and a path make behavior
  visible. Comparison includes displacement, remaining cells and completed young.
  Incomplete offspring can remain visible when the completed-young count is zero.
- Follow keeps selected DNA available after death. Family skips missing generations;
  Wide/Find returns to the selection. Other organisms are dimmed during selection.
  Generation and living-family counts are separated from a specimen's saved name.
- Observed failure messages distinguish depleted cells, physical attack, incomplete
  development and the tank's capacity limit. Placement failures are atomic and explain
  occupied space, boundaries, invalid DNA or capacity. A full tank offers a fresh world.
- Save failures retain edits. Library writes are atomic, DNA round-trips exactly, and
  corrupt input is preserved instead of silently overwritten. Names and colors persist.
- Clean launch opens the chooser; a new experiment returns there. The launch script
  clears historical Simulator fixtures. The library has an explicit Close action and
  explains that worlds restart while saved genomes persist.

## Removed or simplified

Automatic controller replacement on ordinary edits is gone. Deformation is described
as shape change rather than promised swimming. The main UI stays a small creator,
practice view, family controls and three environmental tools; no dashboard, campaign,
progression, new biology or player-specific survival rules were added.

## Current validation

- Release build and install: Xcode, iPhone 17 Pro / iOS 26.5 Simulator (arm64).
- 34/34 CTest tests pass, with assertions enabled in Release. Four affected tests
  were rerun successfully after the final loss-history fix. Includes CPU/Metal checks,
  inheritance/mutation, organ combinations, damage, capacity, spatial safety and sessions.
- AddressSanitizer + UndefinedBehaviorSanitizer: biology bridge and native library tests
  both pass. This is focused coverage, not a sanitizer run of every UI path.
- All four templates replay exactly when unchanged; geometry and storage edits run
  with finite values and valid topology. Direction reversal reverses thrust; disabled
  motors do not propel; powered bending/contractile shapes do not create free COM motion.
- Player-empty world, seed 43, five-cell authored edit, 600 simulated seconds:
  31 descendants, 28 different from the ancestor, 7 changed cell counts, generation 8,
  2 living family members. No invalid development and no capacity waits in this run.
- Mixed world, seed 43, same edit, 600 simulated seconds: three changed descendants,
  then extinction. The wider ecosystem continues reproducing. This is a failure example,
  not a reason to protect the player's lineage.
- Actual Simulator interaction: select a motor, Turn, Try body, Original, name,
  Save & release, place near food, follow Generation 1, save it, reinstall/relaunch,
  and place that saved descendant. Wide/Find and time controls were also exercised.
- Visual comparison: the one-motor edit moved 3.7 cell widths with no completed child
  by 12 seconds; Original moved 5.5 widths and completed one child. Both retained four
  ancestor cells. This demonstrates a consequence; it does not establish which is better.
- Final Release: 219.65 simulated seconds, generation 9, 64 births, median 60 FPS,
  279.7 MB peak reported resident memory. Ancestor death evidence and following a
  surviving relative were checked visually. A preceding 663.49-second soak reached
  generation 18 before the observation-only loss-cache fix.
- Actual Simulator stability observations and executable/source hashes are recorded in
  `verification.json`. Tests, biology output and bounded birth/DNA evidence are alongside
  this file. Earlier report folders are historical and were not treated as current proof.

Automation checks correctness and captures evidence. Visual inspection establishes
readability in these screens; it cannot establish enjoyment or attachment.

## Exact launch

On this Mac:

```sh
cd /Users/maxhastings/Documents/ChatGPT/ALIEN
./mobile/scripts/run-simulator.sh
```

Or from a fresh checkout:

```sh
git clone https://github.com/MaxHastings/alien-mobile.git
cd alien-mobile
./mobile/scripts/run-simulator.sh
```

The script builds Release, boots/opens an available iPhone Simulator and launches
`com.example.AlienMobilePrototype`. The app bundle is
`mobile/build-sim/Release-iphonesimulator/AlienMobileApp.app`. Pass a Simulator UDID as
the first argument if multiple iPhones are installed. No signing identity is required.

## Ten-minute human playtest

1. Choose **Skiff → Edit & try → Original**. Watch a full trial. Return to **Build**,
   tap one outer motor, **Turn** once, then **Try body**. Can you explain the difference?
2. Name it and **Save & release**. Tap near gold food. Watch at 1×, then try 2×.
   Use **Wide → Find** and **Family →** when it appears. Do you care what happens next?
3. Save a descendant. **Edit copy**, change an organ or grow a branch, and compare again.
   Try **Rewire…** only as a deliberate experiment; use **Undo** to compare the tradeoff.
4. Release something away from food, or remove a motor. Watch for depletion, stillness,
   incomplete offspring or death. Can you tell what happened and retry without guessing?
5. **↺ → New world**. Reuse a saved creature; quit/relaunch and confirm it remains.
   Try another template or a blank constructor. Does another experiment feel worthwhile?

Also try direct branch dragging, camera panning, and Option-drag Simulator pinch.
The remaining decisions are human: fun, attachment, understandable consequences,
surprising descendants, and whether the world feels alive.

## Material caveats

- World state is session-only; saved creature DNA persists. Termination starts a fresh tank.
- No physical iPhone is connected; physical GPU performance, thermals and touch feel are
  unqualified. Simulator uses CPU physics with Metal rendering.
- Automated native drag did not reliably generate Simulator pan events; pinch could not
  be conclusively exercised through that input bridge. The gesture handlers are reviewed,
  and the short-drag end-state handling was corrected, but direct human gesture validation
  remains. Button-based anatomy editing and Wide/Find are available.
- A 12-second finite-food trial is deliberately limited. Mutation can fail, lineages can
  die out, and capacity can pause reproduction. Requested fast time may run slower under
  load. Editing a repeated developmental part can unfold it, as disclosed in the UI.
