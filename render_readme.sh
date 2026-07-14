#!/usr/bin/env bash
# Re-render the README images from the model:
#   renders/assembly.png          — hero: face_display="time" (4:23,
#                                   IT IS TWENTY THREE MINUTES PAST FOUR)
#   renders/assembly_invaders.png — static attract frame (anim_t=2)
#   renders/assembly_invaders.gif — 10-frame looping attract march
# All three share one camera so they read as a set. No --render on
# purpose: PNG export defaults to the preview renderer, which is both
# fast and the only one that honours the color() calls the lit-face
# overlay depends on. GIF assembly needs ImageMagick.
# NOTE: openscad here is snap-confined — output paths must stay inside
# the repo (it can't write to /tmp).
set -e
cd "$(dirname "$0")"
CAM="--imgsize=1400,1150 --camera=0,0,92,78,0,25,560"

echo "=== stills"
openscad -o renders/assembly.png $CAM \
    -D 'face_display="time"' wordclock.scad 2>&1 | grep -i '^ERROR' || true
openscad -o renders/assembly_invaders.png $CAM \
    -D 'face_display="invaders"' wordclock.scad 2>&1 | grep -i '^ERROR' || true

echo "=== attract-march frames"
mkdir -p renders/.gif_frames
for t in 0 1 2 3 4 5 6 7 8 9; do
    openscad -o "renders/.gif_frames/f$t.png" $CAM \
        -D 'face_display="invaders"' -D "anim_t=$t" wordclock.scad 2>&1 \
        | grep -i '^ERROR' || true
done

echo "=== assembling GIF"
# 35/100 s per frame ~ the firmware's march tempo. OptimizeFrame
# crops each frame to its changed region (~halves the file). NOT
# -layers Optimize: that variant remapped the whole background to a
# wrong palette entry (cream -> lavender) on these frames.
magick -delay 35 -loop 0 renders/.gif_frames/f?.png \
    -layers OptimizeFrame renders/assembly_invaders.gif
rm -rf renders/.gif_frames
ls -la renders/assembly.png renders/assembly_invaders.png renders/assembly_invaders.gif
echo "Done."
