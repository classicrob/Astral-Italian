#!/usr/bin/env python3
"""Render an objdiff JSON report as a self-contained, grouped SVG treemap."""
import argparse
from collections import defaultdict
from html import escape
import json
from pathlib import Path


def squarify(items, x, y, width, height):
    """Yield (key, x, y, width, height), preserving each positive weight's area."""
    items = sorted(((key, float(size)) for key, size in items if size > 0),
                   key=lambda item: (-item[1], str(item[0])))
    if not items:
        return
    scale = width * height / sum(size for _, size in items)
    pending = [(key, size * scale) for key, size in items]

    def worst(row, side):
        areas = [size for _, size in row]
        total = sum(areas)
        return max(side * side * max(areas) / total**2,
                   total**2 / (side * side * min(areas)))

    row = []
    index = 0
    while index < len(pending) or row:
        side = min(width, height)
        if index < len(pending):
            candidate = pending[index]
            if not row or worst(row + [candidate], side) <= worst(row, side):
                row.append(candidate)
                index += 1
                continue
        total = sum(size for _, size in row)
        if width >= height:
            strip = total / height
            offset = y
            for key, size in row:
                span = size / strip
                yield key, x, offset, strip, span
                offset += span
            x, width = x + strip, max(0, width - strip)
        else:
            strip = total / width
            offset = x
            for key, size in row:
                span = size / strip
                yield key, offset, y, span, strip
                offset += span
            y, height = y + strip, max(0, height - strip)
        row = []


def folder(name):
    parts = name.split('/')
    if parts[0] == 'main':
        parts = parts[1:]
    if parts[0] == 'Game' and len(parts) > 2:
        return '/'.join(parts[:2])
    return parts[0] if len(parts) > 1 else 'Other'


def colour(percent):
    if percent >= 100:
        return '#7ac943'
    if percent <= 0:
        return '#333333'
    low, high = (20, 59, 99), (55, 151, 219)
    return '#' + ''.join(f'{round(a + (b - a) * percent / 100):02x}'
                         for a, b in zip(low, high))


def render(report):
    groups = defaultdict(list)
    for unit in report.get('units', []):
        measures = unit.get('measures', {})
        size = int(measures.get('total_code', 0))
        if size > 0:
            groups[folder(unit['name'])].append((unit['name'], size,
                float(measures.get('matched_code_percent', 0))))
    if not groups:
        raise ValueError('report has no positive-code units')
    total = sum(size for units in groups.values() for _, size, _ in units)
    matched = report.get('measures', {}).get('matched_code_percent')
    heading = 'Astral Italian — decompilation progress'
    if matched is not None:
        heading += f' · {float(matched):.2f}% matched code'
    svg = [
        '<svg xmlns="http://www.w3.org/2000/svg" width="1024" height="512" viewBox="0 0 1024 512" role="img" aria-labelledby="title desc">',
        f'<title id="title">{escape(heading)}</title>',
        f'<desc id="desc">{total:,} code bytes. Rectangle area represents unit code size. Green: 100% matched; blue: partial; dark grey: 0%. Grouped by source folder.</desc>',
        '<rect width="1024" height="512" rx="14" fill="#181b20"/>',
        f'<text x="16" y="27" fill="#eeeeee" font-family="sans-serif" font-size="16">{escape(heading)}</text>',
    ]
    weights = [(name, sum(size for _, size, _ in units))
               for name, units in groups.items()]
    labels = []
    for group, x, y, w, h in squarify(weights, 12, 42, 1000, 432):
        units = groups[group]
        percents = {name: percent for name, _, percent in units}
        for name, ux, uy, uw, uh in squarify(
                [(name, size) for name, size, _ in units], x, y, w, h):
            percent = percents[name]
            svg.append(f'<rect x="{ux:.6f}" y="{uy:.6f}" width="{uw:.6f}" height="{uh:.6f}" fill="{colour(percent)}" stroke="#181b20" stroke-width="0.5"><title>{escape(name)} — {percent:.2f}% matched code</title></rect>')
        svg.append(f'<rect x="{x:.6f}" y="{y:.6f}" width="{w:.6f}" height="{h:.6f}" fill="none" stroke="#181b20" stroke-width="2" pointer-events="none"/>')
        if w > 110 and h > 25:
            labels.append(f'<g pointer-events="none"><rect x="{x + 2:.2f}" y="{y + 2:.2f}" width="{len(group) * 7 + 8}" height="20" rx="3" fill="#181b20" fill-opacity="0.85"/><text x="{x + 6:.2f}" y="{y + 16:.2f}" fill="white" font-family="sans-serif" font-size="12">{escape(group)}</text></g>')
    svg.extend(labels)
    svg.append('<text x="16" y="497" fill="#cccccc" font-family="sans-serif" font-size="12">Area = code bytes · green = 100% matched · blue = partial · grey = 0% · hover for unit details</text>')
    svg.append('</svg>')
    return '\n'.join(svg) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('report', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    svg = render(json.loads(args.report.read_text()))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(svg)


if __name__ == '__main__':
    main()
