#!/usr/bin/env python3
"""Genere une carte SVG a partir de sorties/topologie.csv (stdlib uniquement)."""

from __future__ import annotations

import csv
import sys
from pathlib import Path


def main() -> int:
    base = Path(__file__).resolve().parent.parent
    csv_path = base / "sorties" / "topologie.csv"
    out_path = base / "sorties" / "topologie.svg"

    if len(sys.argv) > 1:
        csv_path = Path(sys.argv[1])
    if len(sys.argv) > 2:
        out_path = Path(sys.argv[2])

    if not csv_path.is_file():
        print(f"Fichier introuvable : {csv_path}", file=sys.stderr)
        print("Lancez d'abord la simulation ns-3 (projet-densite-hotzone).", file=sys.stderr)
        return 1

    rows: list[tuple[str, float, float, str]] = []
    with csv_path.open(newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(
                (
                    row["role"],
                    float(row["x_m"]),
                    float(row["y_m"]),
                    row["node_id"],
                )
            )

    if not rows:
        print("CSV vide.", file=sys.stderr)
        return 1

    xs = [r[1] for r in rows]
    ys = [r[2] for r in rows]
    pad = 40.0
    min_x, max_x = min(xs), max(xs)
    min_y, max_y = min(ys), max(ys)
    span_x = max(max_x - min_x, 1.0)
    span_y = max(max_y - min_y, 1.0)
    w = 800.0
    h = 600.0
    sx = (w - 2 * pad) / span_x
    sy = (h - 2 * pad) / span_y
    scale = min(sx, sy)

    def tx(x: float) -> float:
        return pad + (x - min_x) * scale

    def ty(y: float) -> float:
        return h - pad - (y - min_y) * scale

    colors = {"STA": "#dc5034", "AP": "#2e8b57", "INFRA": "#4169e1"}
    radii = {"STA": 4, "AP": 10, "INFRA": 8}

    parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{w:.0f}" height="{h:.0f}">',
        '<rect width="100%" height="100%" fill="#f7f7f7"/>',
        f'<text x="{pad}" y="24" font-family="sans-serif" font-size="14">'
        f"Zone dense IMT-2030 (echelle reduite) — {len(rows)} noeuds</text>",
    ]

    ap_xy = None
    for role, x, y, nid in rows:
        cx, cy = tx(x), ty(y)
        if role == "AP":
            ap_xy = (cx, cy)
        parts.append(
            f'<circle cx="{cx:.1f}" cy="{cy:.1f}" r="{radii.get(role, 5)}" '
            f'fill="{colors.get(role, "#888")}" opacity="0.85"/>'
        )

    if ap_xy:
        for role, x, y, _ in rows:
            if role != "STA":
                continue
            cx, cy = tx(x), ty(y)
            parts.append(
                f'<line x1="{ap_xy[0]:.1f}" y1="{ap_xy[1]:.1f}" '
                f'x2="{cx:.1f}" y2="{cy:.1f}" stroke="#cccccc" stroke-width="0.5"/>'
            )

    for role, x, y, nid in rows:
        if role == "STA":
            continue
        cx, cy = tx(x), ty(y)
        parts.append(
            f'<text x="{cx + 12:.1f}" y="{cy + 4:.1f}" font-size="11" '
            f'font-family="sans-serif">{role}</text>'
        )

    parts.append("</svg>")
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(parts), encoding="utf-8")
    print(f"SVG ecrit : {out_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
