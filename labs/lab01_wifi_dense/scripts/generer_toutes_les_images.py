#!/usr/bin/env python3
"""Genere SVG + page HTML pour captures (CV, rapport). Stdlib uniquement."""

from __future__ import annotations

import csv
import html
from collections import defaultdict
from pathlib import Path


def topologie_svg(rows: list[dict[str, str]], out: Path, title: str) -> None:
    pts = []
    for row in rows:
        pts.append(
            (
                row["role"],
                float(row["x_m"]),
                float(row["y_m"]),
            )
        )
    xs = [p[1] for p in pts]
    ys = [p[2] for p in pts]
    pad = 50.0
    w, h = 860.0, 620.0
    min_x, max_x = min(xs), max(xs)
    min_y, max_y = min(ys), max(ys)
    span_x = max(max_x - min_x, 1.0)
    span_y = max(max_y - min_y, 1.0)
    scale = min((w - 2 * pad) / span_x, (h - 2 * pad) / span_y)

    def tx(x: float) -> float:
        return pad + (x - min_x) * scale

    def ty(y: float) -> float:
        return h - pad - (y - min_y) * scale

    colors = {"STA": "#dc5034", "AP": "#2e8b57"}
    radii = {"STA": 5, "AP": 12}
    lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{w:.0f}" height="{h:.0f}">',
        '<rect width="100%" height="100%" fill="#fafafa"/>',
        f'<text x="{pad}" y="28" font-family="Segoe UI,sans-serif" font-size="16">{html.escape(title)}</text>',
    ]
    ap_xy = None
    for role, x, y in pts:
        cx, cy = tx(x), ty(y)
        if role == "AP":
            ap_xy = (cx, cy)
        lines.append(
            f'<circle cx="{cx:.1f}" cy="{cy:.1f}" r="{radii.get(role, 5)}" '
            f'fill="{colors.get(role, "#888")}"/>'
        )
    if ap_xy:
        for role, x, y in pts:
            if role == "STA":
                lines.append(
                    f'<line x1="{ap_xy[0]:.1f}" y1="{ap_xy[1]:.1f}" '
                    f'x2="{tx(x):.1f}" y2="{ty(y):.1f}" stroke="#ddd" stroke-width="0.6"/>'
                )
    lines.append("</svg>")
    out.write_text("\n".join(lines), encoding="utf-8")


def comparatif_svg(rows: list[dict[str, str]], out: Path) -> None:
    """Derniere paire baseline/adaptive pour chaque (nSta, simTime_s)."""
    groups: dict[tuple[str, str], dict[str, dict[str, str]]] = defaultdict(dict)
    for row in rows:
        key = (row["nSta"], row["simTime_s"])
        groups[key][row["policy"]] = row

    series = []
    for key, pols in sorted(groups.items(), key=lambda x: int(x[0][0])):
        if "baseline" in pols and "adaptive" in pols:
            series.append((key, pols["baseline"], pols["adaptive"]))

    if not series:
        out.write_text(
            "<svg xmlns='http://www.w3.org/2000/svg' width='400' height='80'>"
            "<text x='10' y='40'>Pas assez de runs baseline+adaptive</text></svg>",
            encoding="utf-8",
        )
        return

    _, base, adap = series[-1]
    labels = ["Livraison", "Jain", "Throttle/100"]
    bvals = [
        float(base["deliveryRatio"]) * 100,
        float(base["jainFairness"]) * 100,
        float(base["throttleEvents"]),
    ]
    avals = [
        float(adap["deliveryRatio"]) * 100,
        float(adap["jainFairness"]) * 100,
        float(adap["throttleEvents"]),
    ]
    w, h = 720.0, 380.0
    pad = 60.0
    chart_w = w - 2 * pad
    bar_w = chart_w / (len(labels) * 2.5)
    max_v = max(max(bvals), max(avals), 1.0) * 1.15

    def bar_x(i: int, offset: float) -> float:
        return pad + i * (chart_w / len(labels)) + offset

    lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{w:.0f}" height="{h:.0f}">',
        '<rect width="100%" height="100%" fill="#fafafa"/>',
        f'<text x="{pad}" y="32" font-family="Segoe UI,sans-serif" font-size="15">'
        f'Comparatif baseline vs adaptive — {base["nSta"]} STA, {base["simTime_s"]} s</text>',
        f'<rect x="{w-200}" y="12" width="14" height="14" fill="#4a6fa5"/>',
        '<text x="' + str(w - 180) + '" y="24" font-size="12">baseline</text>',
        f'<rect x="{w-110}" y="12" width="14" height="14" fill="#c45c3e"/>',
        '<text x="' + str(w - 90) + '" y="24" font-size="12">adaptive</text>',
    ]
    for i, lab in enumerate(labels):
        by = h - pad - (bvals[i] / max_v) * (h - 2 * pad)
        ay = h - pad - (avals[i] / max_v) * (h - 2 * pad)
        lines.append(
            f'<rect x="{bar_x(i, 8):.1f}" y="{by:.1f}" width="{bar_w:.1f}" '
            f'height="{h - pad - by:.1f}" fill="#4a6fa5"/>'
        )
        lines.append(
            f'<rect x="{bar_x(i, 8) + bar_w + 6:.1f}" y="{ay:.1f}" width="{bar_w:.1f}" '
            f'height="{h - pad - ay:.1f}" fill="#c45c3e"/>'
        )
        lines.append(
            f'<text x="{bar_x(i, 8):.1f}" y="{h - pad + 22:.1f}" font-size="11">{lab}</text>'
        )
    lines.append("</svg>")
    out.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    base = Path(__file__).resolve().parent.parent
    sorties = base / "sorties"
    sorties.mkdir(parents=True, exist_ok=True)

    topo_csv = sorties / "topologie.csv"
    res_csv = sorties / "resultats.csv"

    if topo_csv.is_file():
        with topo_csv.open(newline="", encoding="utf-8") as f:
            topo_rows = list(csv.DictReader(f))
        pol = topo_rows[0].get("policy", "run") if topo_rows else "run"
        gs = topo_rows[0].get("gridSide", "?") if topo_rows else "?"
        title = f"Topologie zone dense — {pol}, grille {gs}×{gs}"
        topologie_svg(topo_rows, sorties / "image_topologie.svg", title)
        print(f"OK {sorties / 'image_topologie.svg'}")
    else:
        print("Manquant : sorties/topologie.csv (relancer la simu imt2030-density-gestion)")

    if res_csv.is_file():
        with res_csv.open(newline="", encoding="utf-8") as f:
            res_rows = list(csv.DictReader(f))
        comparatif_svg(res_rows, sorties / "image_comparatif.svg")
        print(f"OK {sorties / 'image_comparatif.svg'}")
    else:
        print("Manquant : sorties/resultats.csv")

    index = sorties / "portfolio_visuel.html"
    parts = [
        "<!DOCTYPE html>",
        "<html lang='fr'><head><meta charset='utf-8'/>",
        "<title>Portfolio visuel IMT-2030</title>",
        "<style>body{font-family:Segoe UI,sans-serif;margin:24px;background:#f4f4f4}"
        "h1{font-size:1.25rem}.box{background:#fff;padding:16px;margin:16px 0;"
        "border-radius:8px;box-shadow:0 1px 4px #0001}img{max-width:100%;height:auto}"
        "code{font-size:0.9rem}</style></head><body>",
        "<h1>Simulation ns-3 — zone dense / gestion de charge</h1>",
        "<p>Captures : ouvrez cette page dans le navigateur puis "
        "<strong>Win+Shift+S</strong> (Windows) pour enregistrer des PNG.</p>",
    ]
    for name, caption in [
        ("image_topologie.svg", "Figure 1 — Disposition des terminaux (STA) et point d'acces"),
        ("image_comparatif.svg", "Figure 2 — Comparatif baseline vs adaptive (derniere campagne)"),
    ]:
        p = sorties / name
        if p.is_file():
            parts.append(f"<div class='box'><h2>{html.escape(caption)}</h2>")
            parts.append(f"<img src='{name}' alt='{html.escape(caption)}'/></div>")
    docs_img = base / "docs" / "img"
    for fname, caption in [
        ("terminal-baseline-10x10.png", "Figure 3 — Run baseline (10×10, 12 s)"),
        (
            "terminal-baseline-et-adaptive-10x10.png",
            "Figure 4 — Baseline puis adaptive (meme charge)",
        ),
    ]:
        src = docs_img / fname
        if src.is_file():
            dest = sorties / fname
            if not dest.exists() or src.stat().st_mtime > dest.stat().st_mtime:
                dest.write_bytes(src.read_bytes())
            parts.append(f"<div class='box'><h2>{html.escape(caption)}</h2>")
            parts.append(f"<img src='{fname}' alt='{html.escape(caption)}'/></div>")
    parts.append(
        "<div class='box'><h2>Figure 5 — Terminal</h2>"
        "<p>Autres captures : dossier <code>docs/img/</code>.</p></div>"
    )
    parts.append("</body></html>")
    index.write_text("\n".join(parts), encoding="utf-8")
    print(f"OK {index}")
    print(f"\nOuvrir : file:///C:/Users/MOH/Documents/Projet_Perso/ns-allinone-3.43/projet-visuel-imt2030/sorties/portfolio_visuel.html")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
