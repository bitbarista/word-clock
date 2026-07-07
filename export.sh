#!/usr/bin/env bash
# Export all printable parts to build/*.stl (sequentially — parallel
# CGAL renders can silently OOM, per the plant-pot project).
set -e
cd "$(dirname "$0")"
mkdir -p build
for part in faceplate shell stand pod coupon; do
    echo "=== $part"
    openscad -o "build/$part.stl" -D "part=\"$part\"" wordclock.scad 2>&1 \
        | grep -E 'Simple|ERROR|WARNING' || true
done
echo "Done — STLs in build/"
