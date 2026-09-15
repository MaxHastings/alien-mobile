#!/usr/bin/env python3
"""Same-seed direct assay of promoted versus previous catalog genomes."""
from pathlib import Path
import csv, json, subprocess
from concurrent.futures import ThreadPoolExecutor

root = Path(__file__).resolve().parents[1]
exe = root / "build-incubation" / "AlienMobileCatalogDiscovery"
out = root / "docs/incubation/catalog-comparison"
out.mkdir(parents=True, exist_ok=True)
names = ["Skiff", "Thread", "Whorl", "Husk"]
current = root / "docs/discovery/finalists"
previous = root / "docs/incubation/previous-finalists"
manifests = {}
for label, directory in [("promoted", current), ("previous", previous)]:
    manifest = out / f"{label}.list"
    manifest.write_text("\n".join(str(directory / f"{name}.dna") for name in names) + "\n")
    manifests[label] = manifest

jobs = [(label, seed, profile) for label in manifests for seed in [17, 43, 71] for profile in [0, 1, 2]]
def run(job):
    label, seed, profile = job
    folder = out / f"{label}-{seed}-{profile}"
    folder.mkdir(exist_ok=True)
    cmd = [str(exe), "assay", str(seed), str(profile), "1800", str(manifests[label]), str(folder), "none"]
    (folder / "command.json").write_text(json.dumps(cmd) + "\n")
    p = subprocess.run(cmd, capture_output=True, text=True)
    (folder / "summary.csv").write_text(p.stdout)
    (folder / "stderr.txt").write_text(p.stderr)
    if p.returncode: raise RuntimeError((job, p.stderr))
    return folder

with ThreadPoolExecutor(max_workers=6) as pool:
    list(pool.map(run, jobs))

rows = []
for label, seed, profile in jobs:
    folder = out / f"{label}-{seed}-{profile}"
    for row in csv.reader((folder / "summary.csv").open()):
        if row: rows.append([label, seed, profile] + row)
with (out / "summary.csv").open("w", newline="") as f:
    writer = csv.writer(f); writer.writerow(["catalog", "seed", "profile", "seed_out", "profile_out", "name", "mature", "births", "max_generation", "changed_births", "late_births", "physical_cells"]); writer.writerows(rows)
