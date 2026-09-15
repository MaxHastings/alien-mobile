#!/usr/bin/env python3
"""Render actual engine poses for the promoted and previous catalog."""
from pathlib import Path
import csv, sys
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1]
current = root / "docs/incubation/normal-playable-final-42/bodies.csv"
oldroot = root / "docs/discovery"
names = ["Skiff", "Thread", "Whorl", "Husk"]
colors = {1: "#9cb9ad", 3: "#ffc15a", 4: "#60dcf1", 8: "#e889f2"}
times = [0, 60, 120, 179]

def read(path):
    return list(csv.DictReader(path.open()))

def draw(canvas, rows, second, title, ox0, oy0):
    draw = ImageDraw.Draw(canvas)
    at = [r for r in rows if int(r["second"]) == second]
    if not at: return
    rootrow = next((r for r in at if int(r["node"]) == 0), at[0])
    ox, oy = float(rootrow["x"]), float(rootrow["y"])
    bynode = {int(r["node"]): r for r in at}
    def point(r): return (ox0 + 110 + int((float(r["x"]) - ox) * 12), oy0 + 120 - int((float(r["y"]) - oy) * 12))
    for node, r in bynode.items():
        parent = int(r["parent"])
        if parent in bynode:
            p = bynode[parent]
            draw.line([point(r), point(p)], fill="#527b85", width=3)
        x, y = point(r); draw.ellipse((x-5, y-5, x+5, y+5), fill=colors.get(int(r["role"]), "#cacaca"))
    draw.text((ox0 + 8, oy0 + 8), title, fill="white")

canvas = Image.new("RGB", (4 * 220, 8 * 250), "#071b20")
current_rows = read(current)
parent = {}
for r in current_rows:
    parent.setdefault(int(r["id"]), int(r.get("organism_parent", r["parent"])))
def family(cid):
    seen = set()
    while cid not in seen and cid in parent and parent[cid] not in (-1, 4294967295):
        seen.add(cid); cid = parent[cid]
    return cid

for row, name in enumerate(names):
    # Pick the largest living representative of each starting lineage at
    # every timestamp; founders may die while their ordinary descendants live.
    now = []
    for t in times:
        sample = [r for r in current_rows if int(r["second"]) == t and family(int(r["id"])) == row]
        counts = {}
        for r in sample: counts[int(r["id"])] = counts.get(int(r["id"]), 0) + 1
        chosen = max(counts, key=counts.get) if counts else None
        now += [r for r in sample if int(r["id"]) == chosen]
    oldpath = oldroot / ("observe-" + name) / "bodies.csv"
    old = read(oldpath) if oldpath.exists() else []
    for col, t in enumerate(times):
        draw(canvas, now, t, f"incubated {name} · {t}s", col * 220, row * 250)
        draw(canvas, old, t, f"previous {name} · {t}s", col * 220, (row + 4) * 250)
out = root / "docs/incubation/catalog-comparison.png"; canvas.save(out)
print(out)
