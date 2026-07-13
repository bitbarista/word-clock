#!/usr/bin/env bash
# Export all printable parts to build/*.stl (sequentially — parallel
# CGAL renders can silently OOM, per the plant-pot project).
set -e
cd "$(dirname "$0")"
mkdir -p build
for part in faceplate lattice diffuser shell stand_lowtail stand_feet coupon coupon_diffuser pod_coupon; do
    echo "=== $part"
    openscad -o "build/$part.stl" -D "part=\"$part\"" wordclock.scad 2>&1 \
        | grep -E 'Simple|ERROR|WARNING' || true
done
echo "=== stand_feet_tilt15"
openscad -o "build/stand_feet_tilt15.stl" -D 'part="stand_feet"' -D 'tilt=15' wordclock.scad 2>&1 \
    | grep -E 'Simple|ERROR|WARNING' || true
echo "Done — STLs in build/"

# Print-ready 3MF sets — ONE file per variant containing all its parts
# as distinct, individually-selectable objects (not merged into one
# mesh), so a non-technical user just opens one file and slices
# everything. OpenSCAD can only emit one object per export, so parts
# are exported individually into a scratch folder first, then combined
# by pack_3mf.py (needs `pip install trimesh numpy`). desk_set uses
# mount_type="desk" (power_pos=back, keyhole=false, both stand angles
# included); wall_set uses mount_type="wall" (power_pos=bottom,
# keyhole=true baked into faceplate/shell, no stand — it isn't fitted
# wall-mounted).
if python3 -c "import trimesh" 2>/dev/null; then
    mkdir -p build/.parts
    echo "=== desk_set parts"
    for part in faceplate lattice diffuser shell; do
        openscad -o "build/.parts/desk_$part.3mf" -D "part=\"$part\"" -D 'mount_type="desk"' wordclock.scad 2>&1 \
            | grep -E 'ERROR|WARNING' || true
    done
    openscad -o "build/.parts/desk_stand_feet_12.3mf" -D 'part="stand_feet"' -D 'mount_type="desk"' -D 'tilt=12' wordclock.scad 2>&1 \
        | grep -E 'ERROR|WARNING' || true
    openscad -o "build/.parts/desk_stand_feet_15.3mf" -D 'part="stand_feet"' -D 'mount_type="desk"' -D 'tilt=15' wordclock.scad 2>&1 \
        | grep -E 'ERROR|WARNING' || true
    echo "=== wall_set parts"
    for part in faceplate lattice diffuser shell; do
        openscad -o "build/.parts/wall_$part.3mf" -D "part=\"$part\"" -D 'mount_type="wall"' wordclock.scad 2>&1 \
            | grep -E 'ERROR|WARNING' || true
    done
    echo "=== packing combined 3MF sets"
    python3 pack_3mf.py build/desk_set.3mf \
        faceplate=build/.parts/desk_faceplate.3mf \
        shell=build/.parts/desk_shell.3mf \
        lattice=build/.parts/desk_lattice.3mf \
        diffuser=build/.parts/desk_diffuser.3mf \
        stand_feet_12deg=build/.parts/desk_stand_feet_12.3mf \
        stand_feet_15deg=build/.parts/desk_stand_feet_15.3mf
    python3 pack_3mf.py build/wall_set.3mf \
        faceplate=build/.parts/wall_faceplate.3mf \
        shell=build/.parts/wall_shell.3mf \
        lattice=build/.parts/wall_lattice.3mf \
        diffuser=build/.parts/wall_diffuser.3mf
    rm -rf build/.parts
    echo "Done — build/desk_set.3mf and build/wall_set.3mf"
else
    echo "Skipping 3MF sets: 'pip install trimesh numpy' first (used by pack_3mf.py to combine parts into one file per variant)."
fi
