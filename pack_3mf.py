#!/usr/bin/env python3
# Combine several single-part 3MF/STL files into one 3MF containing all
# of them as distinct, individually-selectable objects (not merged into
# one mesh) laid out on a simple non-overlapping grid. OpenSCAD itself
# can only emit one object per export, so this is a separate packing
# pass over its output — see export.sh.
#
# Usage: pack_3mf.py output.3mf name1=path1.3mf name2=path2.3mf ...
import sys
import numpy as np
import trimesh

def build_set(parts, out_path, max_row_w=400.0, gap=20.0):
    scene = trimesh.Scene()
    x_cursor = y_cursor = row_h = 0.0
    for name, path in parts:
        m = trimesh.load(path, force="mesh")
        m.apply_translation(-m.bounds[0])
        w, d = m.bounds[1][0] - m.bounds[0][0], m.bounds[1][1] - m.bounds[0][1]
        if x_cursor > 0 and x_cursor + w > max_row_w:
            y_cursor += row_h + gap
            x_cursor, row_h = 0.0, 0.0
        tf = np.eye(4)
        tf[0, 3], tf[1, 3] = x_cursor, y_cursor
        scene.add_geometry(m, node_name=name, geom_name=name, transform=tf)
        x_cursor += w + gap
        row_h = max(row_h, d)
    scene.export(out_path)
    print(f"{out_path}: {len(parts)} objects -> {[n for n, _ in parts]}")

if __name__ == "__main__":
    out_path = sys.argv[1]
    parts = [arg.split("=", 1) for arg in sys.argv[2:]]
    build_set(parts, out_path)
