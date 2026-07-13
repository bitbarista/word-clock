#!/usr/bin/env bash
# Export STEP versions of all printable parts + the assembled device to
# build/*.step (exact BRep: analytic cylinders/planes; 2D profile arcs
# tessellated with the model's own $fa/$fs, i.e. identically to the STL
# exports). See step_export/ for how and why.
#
# FreeCAD's own .scad/.csg importer cannot convert this model (it nulls
# out on 2D offset cuts, fused disjoint solids and tangent seams), so
# the pipeline is: OpenSCAD flattens the model to .csg with hull()/
# mirror()/pixel-letters replaced by importable equivalents
# (step_export/step_assembly.scad wrapper, geometry verified identical);
# flatten2d.py pre-evaluates every linear_extrude body with OpenSCAD's
# exact 2D engine; csg_eval.py rebuilds the tree as BRep solids inside
# headless FreeCAD and writes STEP.
#
# Requires: openscad, python3, FreeCAD 1.1+ (default: the flatpak,
#   flatpak install --user flathub org.freecad.FreeCAD
# — or point FREECADCMD at any freecadcmd binary).
set -e
cd "$(dirname "$0")"
FREECADCMD="${FREECADCMD:-flatpak run --command=freecadcmd org.freecad.FreeCAD}"

work=build/.step_tmp
mkdir -p "$work"
# step_assembly.scad resolves include <../../wordclock.scad> from two
# levels down, so it (and the generated letter polygons it includes)
# must run from inside $work.
python3 step_export/gen_letter_polys.py font.scad "$work/letter_polys.scad"
cp step_export/step_assembly.scad "$work/"

export LOOPS_JSON STEP_OUT CSG_IN PART_NAME
for part in faceplate lattice diffuser shell stand_feet assembly; do
    echo "=== $part"
    out="$part.step"
    [ "$part" = assembly ] && out="wordclock_assembly.step"
    # absolute -o path: the snap build of openscad rejects relative
    # paths that contain a directory component
    openscad -o "$PWD/$work/$part.csg" -D "part=\"$part\"" "$work/step_assembly.scad" 2>&1 \
        | grep -E 'ERROR|WARNING' || true
    python3 step_export/flatten2d.py "$work/$part.csg" \
        "$work/flat_$part.csg" "$work/flat_$part.json"
    CSG_IN="$PWD/$work/flat_$part.csg"
    LOOPS_JSON="$PWD/$work/flat_$part.json"
    STEP_OUT="$PWD/build/$out"
    PART_NAME="$part"
    $FREECADCMD "$PWD/step_export/csg_eval.py" >/dev/null 2>&1 || true
    cat "$work/flat_$part.csg.eval.log"
    grep -q '^EXPORTED' "$work/flat_$part.csg.eval.log"
done
rm -rf "$work"/flat2d_*
echo "Done — STEP files in build/"
