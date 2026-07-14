# Development log — TIME INVADERS word clock

[README.md](README.md) covers what the project is and how to build
it; this file is the engineering history — bugs found, root causes,
fixes, and how each fix was verified. Sections appear in rough
chronological order; part names and parameters are as they were at
the time (the solid `stand`/`stand_sheet` discussed below were later
superseded by `stand_feet`/`stand_lowtail`, which inherited the same
profile work).

## ESP32 cradle & USB access — design evolution (v0.1.2–v0.1.5)

**v0.1.2 — ESP32 moved off the tray-wall corner to an internal cradle
above the pod** (`esp_x`/`esp_y`). Reasoning: routine USB access was
only ever needed for firmware flashing, and this design flashes once
on the bench pre-assembly, then updates OTA over WiFi from then on —
so there's no ongoing need for a side-wall port. This freed the corner
pad that used to be blocked by the cradle (see Ventilation below — all
4 corners are now uniform). **Trade-off, gone into with eyes open:**
if OTA ever bricks, recovery means opening the case and re-flashing
over USB on the bench, not a quick cable plug-in through the wall. The
old corner-cradle-against-the-wall design (and the faceplate's
matching tray-wall notch) is what earlier revisions of this doc
describe. **Faceplate reprinted** to drop the now-unused notch —
`build/faceplate.stl` is current (plain closed tray wall); an
already-printed faceplate from before this change still works fine
(the notch was always harmless, just unused), it just has the old
opening.

Long axis stays along X (`esp_l`) — matching the XIAO's long edges,
where its castellated pin pads actually are, same as the original
wall-mounted design (an earlier pass here rotated it 90° for no good
reason and got caught: nothing bounded the connector end, and it
landed 5.4mm from a pressure pad, not nearly enough room for a plug).
Retention is 4 small L-brackets, one per corner (`esp_corner_l`=2mm
along the long edge, `esp_tab_l`=3.5mm along the short edge, meeting
at the corner) — not full-length ribs. A 21×17.83mm FR4 board doesn't
need continuous edge support the way the 160mm flexible LED panel
does, so corner-only contact is plenty, and it sidesteps the whole
wire-exit-notch-position problem from the first two attempts at this:
the entire middle of both long edges (where the XIAO's actual pins
are — position unknown without the physical part in hand) and both
short edges (wherever the USB-C connector actually is) is just open.
The long-edge arm is deliberately short (2mm, just enough to locate
the board) rather than the 5mm first tried — it runs right alongside
the castellated pin row, so more reach only means more risk of
landing on a pad near the corner, for no retention benefit.
Nothing to guess, nothing to re-verify against real pin positions
later. Checked with `intersection()` against the pad grid, pod cavity,
vent rows, corner posts, and lip ring — all clear.

**Bug caught (v0.1.3):** `esp_clr` is documented as *per-edge*
clearance, but the channel-width formula only added it once across
the whole channel (`esp_w + esp_clr`) instead of once per edge
(`esp_w + 2*esp_clr`) — the board channel was 0.4mm tighter than
intended, on top of whatever an FDM print already eats (this project's
own fit-notes elsewhere show 0.2–0.4mm lost to that routinely). Fixed
both L-bracket arms to use `esp_w + 2*esp_clr`. Re-verified manifold
and re-ran all 5 `intersection()` checks (pad grid, pod, vents, posts,
lip ring) — still clear, the 0.4mm widening only ate into open space.

**v0.1.4 — universal footprint + less bulk.** Now that this feature is
a pure X/Y locator rather than a snug retention cradle (final
retention is the soldered leads + the foam/shell closing over
everything), it doesn't need to be sized to one specific board or
clear the USB-C connector's height. Checked a generic ESP32-S3
"supermini" against the XIAO: 22.52×18mm vs the XIAO's measured
21.12×17.83mm — widths agree to within 0.17mm, lengths differ by
1.4mm (supermini longer). Resized to `brd_w`=18/`brd_l`=22.52 (the
larger of each dimension) so either board locates the same way; the
shorter XIAO just gets ~1.4mm of lengthwise play, which doesn't matter
for a locator. `esp_l`/`esp_w` kept as reference (the XIAO's own
measurements), no longer wired into the cradle geometry.

Wall height dropped from `esp_usbc_h + 0.5` (≈5mm, sized to clear the
connector) to a flat `cradle_h` = 1.8mm (matches the rib-thickness
convention used elsewhere) — real bulk reduction. Deliberately **not**
done by cutting a recess into the 2.4mm lid itself: that would thin
the lid exactly where it needs to resist compressive load from the
foam pads pressing on it, trading bulk for a real strength risk.
Shortening material added *on top* of the lid instead leaves the
lid's own thickness untouched — same bulk win, no structural
trade-off. Re-verified manifold and re-ran all 5 `intersection()`
checks after the footprint grew ~1.4mm longer — still clear.

**v0.1.5 — 4 corners → 1 end-stop (`esp_stop_t`).** The v0.1.4
corner-bracket design had a real flaw: sizing the brackets to the
*union* of both boards' footprints meant the shorter XIAO wasn't
actually contained — it just had ~1.4mm of slop between end-stops
sized for the longer SuperMini, which isn't containment, it's a
rattle. A single rigid 4-corner cage can't snugly fit two different
lengths on both ends at once; there's no symmetric fix for that.

Resolved by not trying to contain both ends: a single straight wall
across ONE short edge only (`esp_stop_t` = 1.8mm thick, spanning
`brd_w + 2*esp_clr`), the other short edge and both long edges left
completely open. Both boards register against the same stop
regardless of length; the longer SuperMini's extra 1.4mm just extends
further into already-open space. Also lower pad-collision risk than
the old L-brackets: those touched a short-edge corner *and* reached
along the long edge (where most castellated pads run on both board
variants — pads come close to the board ends too, not just the long
edges, per physical inspection), whereas a single short-edge wall only
contacts the end face.

The geometry doesn't encode which end either board's USB-C connector
is on — genuinely can't, no verified pin/connector position data for
either board. **Assembly instruction, not a design assumption:**
insert the board with its connector facing the open end (away from
`esp_stop_t`), not the stopped end. Re-verified manifold and all 5
`intersection()` checks (pad grid, pod, vents, posts, lip ring) —
clear.

**Fit note (ESP32 board swap):** the cradle was originally sized from
a generic "ESP32-S3 supermini" guess and didn't fit any of three real
boards on hand (two supermini clones plus a Seeed XIAO ESP32S3).
Switched to the XIAO specifically — it's a tighter-toleranced, single-
vendor part. Re-measured with calipers rather than trusting a listing
(this project has been burned by that twice already): board 21.12 x
17.83mm, 1.15mm PCB, USB-C connector 4.49mm tall (tallest point,
measured from the underside of the board) and overhanging the board's
short edge by 1.49mm, centred on that edge, no mounting holes. Cross-
checked against third-party CAD bounding-box data (22.48 x 4.46 x
17.78mm including the connector overhang) — a close match, so these
are the numbers `esp_l`/`esp_w`/`esp_usbc_h` etc. are built from.
Retention is a friction-clip cradle since the XIAO has no mounting
holes, with a real `esp_clr` (0.4mm) clearance rather than folding
tolerance into the board-size variable. (Originally two full-length
ribs + a backstop wall; now 4 small corner L-brackets instead — see
Power & access for why.)

## Fit note: the lattice didn't fit the printed faceplate

**Fit note (v0.1.x reprint):** the first `lattice` export didn't
actually fit the printed faceplate. Root cause took two passes to
find — the frame's outer square (`ow`) was sized correctly, but the
per-cell crossbars are drawn independently at each LED-grid line, and
the outermost crossbar sits centred exactly on the grid edge
(`gs/2`), so its own width pushes it out to `gs/2 + lat_wall/2`
regardless of `ow`. That overshoot (161.2 mm total), not the frame,
was the part's real widest feature — so the first attempt to add
clearance by shrinking `ow` changed nothing measurable, because the
crossbars were never bounded by it. Fixed by clipping the whole
cross-section (frame + crossbars) to `ow` before extruding, so `ow`
is now genuinely the outer limit. Confirmed by measuring the actual
STL bounding box (160.2 mm, i.e. 0.4 mm/side against the fixed
161 mm opening), not just by reasoning about the code.
`fit_clr` (0.4 mm/side default) sizes `lattice`/`diffuser`/
`coupon_diffuser`; the lattice also shaves an extra 0.3 mm off the
outer frame for its first 0.6 mm (elephant-foot relief). This only
touches those three parts — **the faceplate's opening itself is
unchanged**, so an already-printed faceplate is still correct; just
reprint the lattice (and diffuser, if already printed).

## Fit note: shell rim vs. corner posts (the rsq() shrink bug)

**Fit note (shell rim vs. corner posts, and the real root cause):**
the shell's registration lip (a ring that seats inside the
faceplate's tray wall) ran into the four corner screw posts on test
fit. Root cause: the `rsq()` rounded-square helper's double
`offset(r) offset(-r)` silently shrank the *actual* shape by
`2×corner_r` beyond the size you passed it — confirmed against the
first-printed `faceplate.stl`, whose real bounding box was 178×178mm,
not the 184mm `face_w` the code implies. That ate most of the lip's
intended clearance against the posts, and shrank the post-to-tray-
wall corridor to ~0.6mm — too tight for the lip to route around the
posts at all without a sub-0.4mm-thick sliver of wall.

First attempt punched a plain circular hole through the lip at each
post — that cleared the interference (confirmed by rendering
`intersection()` of the faceplate and shell and checking it was
empty) but left an ugly break in the ring rather than routing around
the post. Decided against permanently living with either that
compromise or the tight corridor: **fixed `rsq()` itself** (drop the
redundant second `offset`, one line) and reprinted the faceplate at
its correct 184mm size instead of continuing to design around the
undersized one. That restores the corridor to ~2.6mm, which is
enough for the lip to route around each post as a real continuous
detour: the ring's inner edge locally bulges around a
`post_d + 2×post_relief_clr` circle at each post (`post_relief_clr`
= 1.0mm) instead of the outer boundary ever being touched. Verified
both ways — `intersection()` of faceplate and shell is empty
everywhere, and a 2D cross-section of the lip at each corner shows
one continuous band with a smooth circular notch, not a gap.
**Reprint both the faceplate and the shell** — the faceplate's actual
size changes (184mm, not 178mm), so the old one won't register with
the new shell correctly.

## v0.1.6 — depth-budget audit: this stack didn't actually fit its own contents

Caught late, after several rounds of shell-only rework: `foam_t` (now
`back_t`) and `panel_t` were both round numbers nobody had checked
against what actually has to physically live in them. Every check run
during the ventilation/pad-grid/cradle rework was a 2D XY-plane
`intersection()` — real and necessary, but it never once verified the
Z-axis (depth) against real component heights. Full audit, one pass,
covering everything rather than patching each part in isolation:

| Component | Old allowance | Real requirement | Fix |
|---|---|---|---|
| ESP32 XIAO (PCB→top of USB-C shell) | `foam_t` = 3.0mm | 4.49mm (measured, this project's own calipers) | Fit by `back_t` = 13mm |
| ESP32 SuperMini | 3.0mm | Unverified, no reason to expect shorter | Fit by `back_t` = 13mm |
| WS2812B panel (PCB + LEDs) | `panel_t` = 2.0mm | 1.8mm (measured: 0.25mm bare FPCB, 1.8mm total with LEDs) | Already fit — confirmed, not assumed |
| 1000µF capacitor | 3.0mm | 11.5mm minimum, any package (leaded or SMD — height doesn't change with mounting style) | Fit by `back_t` = 13mm (the tallest item — sets the whole budget) |
| Power socket | separate bump, own cavity (8mm) | 8mm (already correct — an earlier audit pass flagged this as broken based on a stale comment that was never reconciled with the real `pwr_cavity` value; retracted) | No change needed, relocated (see below) |
| Corner screw (M3×8) | 6.4mm clearance + 1.6mm engagement | ~6mm+ engagement for a reliable self-tap joint | M3×16 (~9.6mm engagement) |
| Panel lateral retention | none — bezel pocket was `baffle_d` deep only, panel sat in the much bigger tray opening beyond it | snug pocket matching the lattice/diffuser fit | Pocket extended through `panel_t` too |

**No bump-outs** — that was ruled out explicitly (a flat back was a
firm requirement), so instead of a local pocket for just the
capacitor, `back_t` grew to fit the *tallest* real component (the
cap) with margin, and everything else — the ESP32, the relocated
power socket — simply inherits that same headroom for free rather
than needing its own separate accommodation. `wall_top` and `slab_t`
both derive from `back_t` automatically, so this is one parameter
change cascading through the whole model, not a scattered patch.

**Power socket relocated, not just re-verified.** While auditing this,
reconsidered whether the socket should even stay as a separate bump-out
part at all: with real interior depth now available, it mounts
directly into the shell's own material instead — snap-fit flange
through the shell, cavity for wing deployment extending into the
(now roomy) interior. This removes the separate pod part *and* its
mounting screws entirely (retained purely by its own snap wings, same
mechanism as before) — one less part to print, and it means literally
every remaining screw on the assembled device is the same 4 corner
screws, all accessible from outside. **One number not re-verified
against the physical part:** the socket's flange now sits against the
shell's `lid_t` (2.4mm) rather than the old dedicated pod face
(2.0mm, the figure actually confirmed working) — 0.4mm thicker than
what was tested. **`part="pod_coupon"`** is a small standalone test
piece (40×40×10.4mm) of exactly this local geometry — flange cutout
through `lid_t`, cavity behind it — print that and check the real
socket snaps in properly before committing to a full shell print.

**Panel now has a real pocket.** The bezel ring used to extrude only
`baffle_d` deep, with its inner opening matching the lattice/diffuser
fit — but nothing beyond that depth constrained the panel at all; the
tray wall's own opening (179.2mm) is dramatically bigger than the
160mm panel, so it just sat loose. Extended the pocket depth to
`baffle_d + panel_t` so the same 0.5mm/side clearance the
lattice/diffuser already get now also captures the panel.

**Filament reduction, same pass.** That pocket wall used to be solid —
~11.5mm wide around the full perimeter, `baffle_d` deep, most of which
a slicer would fill as infill regardless of settings. Now that it's
even deeper (`baffle_d + panel_t`, capturing the panel too), rebuilt
it as a hollow "frame within a frame": thin outer + inner walls
(`bezel_wall_t` = 2mm) connected by sparse ribs (`bezel_rib_n` = 4 per
straight edge), with a thin solid cap (`bezel_cap_t` = 1.5mm) at the
front for a consistent visible bezel surface. Rough volume estimate:
~95cm³ solid → ~53cm³ hollow, **~44% less material** despite the
pocket being deeper than before. Checked with `intersection()` against
the corner posts (ribs sit in the same radial band, near the corners,
by design) — clear.

**Corner posts needed bracing.** Post height is `wall_top - t_front`,
which grew from 17.4mm to 27.4mm along with everything else in this
pass — a 7mm-diameter post that slender is a real snap risk, especially
under the torque of driving in the (now longer) M3×16 screws. Added
two ribs per post (`post_rib_w`), connecting each to the tray wall in
both X and Y. Kept clear of the shell's lip zone (the top `lip_h` of
the post, where the shell's lip registers and routes around the post)
by an explicit `post_rib_clr` = 2mm margin.

**Bug caught after the fact:** `rib_out` (how far each rib reaches
toward the tray wall) was computed as an absolute coordinate from the
faceplate's centre, then used as a local offset from the post's own
already-translated position — effectively adding `post_off` (~83.5mm)
twice, so the ribs shot ~81.6mm past the tray wall into open space.
The `intersection()` check against the lip zone didn't catch it: that
check only tests Z-separation from the lip's height band, which the
ribs satisfied regardless of how wrong their XY position was — it
verified the one thing that had been asked about without checking the
ribs' overall bounds against the faceplate outline at all. Fixed
(`rib_out` now correctly subtracts `post_off`) and re-checked two
ways: `intersection()` against the lip zone (empty, as before) *and*
`difference()` against the faceplate's own outer boundary — any rib
material left over after subtracting the full outline would mean it
pokes outside; empty, confirmed actually contained this time.

All of the above re-verified: manifold check on faceplate, shell, and
the full assembly, plus `intersection()` checks covering the new
pocket depth, the relocated socket mount, the post ribs against the
lip zone, and the existing pad grid/vents/cradle/lip ring/posts — all
clear.

## The stand never got updated for the new slab thickness

Missed entirely during the v0.1.6 depth-budget work — `groove_w`
(the stand's slot width, sized to the slab) is a formula
(`slab_t + groove_clr`) and scaled automatically when `slab_t` grew
21mm → 31mm. But the wedge it's cut *into* (`stand_depth`, `stand_h`,
the polygon in `stand()`) was all hardcoded and didn't move — so the
now-10mm-wider slot cut clean through the front lip, collapsing the
lip and support horn into a flat shell. Confirmed by rendering (image
inspection, not just the manifold flag — manifold only means
watertight, it doesn't mean "the slot stayed where it should").

First fix attempt (moving `groove_y` back to clear the lip) was
incomplete: it stopped the slot from touching the lip, but left only
**0.7mm** of material at the groove's lowest point — found by
computing the actual wedge-minus-slot cross-section as a point-sampled
2D region (`stand_depth`/`stand_h`/polygon vs. the slot cutter's real
transformed geometry), not by eyeballing a 3D render, which had
already produced one wrong "looks fine" read on this exact shape.
Raised the slot's Z-offset 4→8 (and the matching offset in
`assembly()`'s device positioning, 4.6→8.6 — the two have to move
together, there's no formula linking them) to get 4.69mm of margin at
that pinch point instead. Confirmed the fix with the same cross-section
method, plus a clean OpenSCAD `projection()` silhouette (a reliable
2D read, unlike the perspective renders that had already been
ambiguous twice on this shape) — lip, valley, and back wall all
present and solid.

Changed: `stand_depth` 92→104, horn position in the polygon 42/66→
60/84, `groove_y` 40→54 (now a shared top-level parameter — it used
to be a local variable in `stand()` *and* a separately hardcoded `40`
in `assembly()`, silently able to drift out of sync), slot Z-offset
4→8. Re-verified manifold on `stand()` and the full `assembly()`.

## Alternative stand: sheet-metal construction (`stand_sheet`, since superseded)

Optional, **not a replacement** for `stand()` — a second part, your
choice which to print. Same lip/horn/groove profile and angle as the
solid `stand()` (both now share `stand_profile_2d()`, so the sheet
version is guaranteed to hold the device at the same tilt — this
reuses the geometry that already took two rounds of fixing to get
right, rather than re-deriving the trig from scratch a third time),
but hollowed to a constant `sheet_t` (3mm) wall instead of solid —
genuine bent-sheet-metal construction, not a solid block that merely
looks thin. Extruded without end caps, so the open ends show the
material's cross-section, like a press-braked channel bracket viewed
end-on.

Real material saving, not just a different look: 49% less volume
than the solid wedge (179.2cm³ → 91.1cm³) — measured from the actual
exported meshes (signed-volume calculation over the STL triangles),
not estimated; an early draft of this note guessed 85% before
actually computing it.

Both `stand()` and `stand_sheet()` skip OpenSCAD's usual `Simple:`
manifold indicator in the CLI output — moving the groove cut into the
2D cross-section (shared by both) means there's no 3D CSG boolean at
the top level anymore, so OpenSCAD never invokes the CGAL check that
prints it. Verified watertightness a different way instead: exported
each STL and confirmed every mesh edge is shared by exactly 2
triangles (the standard watertight test), independent of what the
CLI chooses to report.

**Untested trade-off:** an open-ended channel is more prone to
racking/twisting under load than a solid wedge — 3D-printed PETG/PLA
has more inherent rigidity than real sheet steel even at this
thickness, but this hasn't been print-tested under the device's
actual weight. If it flexes more than expected, corner gussets or
closed end caps would be the next thing to try, not a reason to
distrust the base geometry (that part's shared with the
already-verified `stand()`).

## The pad grid never got updated for the new cavity depth either

Same root cause as the stand, caught by actually sweeping for it
afterward instead of waiting for it to surface part by part.
`pad_h` (1.0mm) was sized when the cavity behind the panel was 3mm
(`foam_t`): a short rigid nub plus the 3mm foam pad (BOM) together
just reached the panel, foam doing most of the bridging and providing
the actual clamping spring force. When `back_t` grew to 13mm for the
depth-budget fix, nobody revisited this — nub + foam only reached
4mm of the new 13mm gap, so the entire pad grid (added specifically
because a flexible PCB matrix sags at points the corner screws can't
reach) was clamping nothing.

Checked the other candidates that could have had the same problem
before assuming this was the only one: `cradle_h`/`esp_stop_t` (ESP32
locator wall) and `pwr_cavity[2]` (socket cavity) — neither was ever
meant to span the cavity, both are self-contained, both fine.
`pad_h` was the only dimension whose entire job was reaching across
the gap, and it's the one that broke.

Grown to 10.5mm — nub + the still-3mm foam (BOM unchanged) now
reaches 13.5mm against the 13mm gap, 0.5mm of deliberate interference
so the foam is under real compression rather than just touching, same
margin logic already used for the corner-post ribs. XY footprint is
unchanged, so the existing pad positions are still correct — only the
height changed. Re-verified manifold and re-ran all 5 `intersection()`
checks (vents, posts, lip ring, ESP32 cradle, socket mount) with the
taller pads — still clear.

## The stand's horn was sitting on top of the bottom vent row

Caught by inspection, not by me — the ventilation section below has
carried a "chimney effect, since the stand racks the face at a real
angle" claim since v0.1.1, but the stand's own horn geometry (the tall
support block the raked slab actually leans on, `Y=[60,84]` in the
stand's own frame, up to `stand_h`=34mm tall) was never checked against
where the vent slots land once the device is actually seated in it —
another instance of the same root cause as the stand/pad-grid misses
above: a change (or in this case, a design) evaluated in isolation
instead of against the full assembly.

Confirmed with a real measurement, not a re-eyeballed render (renders
of this exact junction had already been ambiguous once before, on the
groove fit): replicated the *identical* transform chain `assembly()`
applies to `device()` → `shell()` → the vent cut as an OpenSCAD
`function` (`vent_world_yz()`), placed marker spheres at the real vent
slot positions using it, exported both the stand and the markers to
STL, and checked containment with `trimesh`. Result: the bottom vent
row's exterior opening landed **0.19mm** clear of the horn's solid
material — inside the slicer's tolerance, i.e. functionally sealed, not
just tight. The top row was fine (>100mm standoff, nowhere near the
low stand). Checked whether `vent_y` alone could dodge it first: across
its entire achievable range the vent row's world position stays within
or right at the horn's footprint, so this wasn't a one-parameter fix.

Also worth noting for its own sake: the vent slot's full length (not
just its centre) matters here — after the tilt transform, a slot's
25mm long axis ends up spanning **21mm of world-Z** (nearly vertical),
not clustered near one point, so the fix below was sized against both
slot ends, not just the centre.

**Fix:** a relief notch cut straight into `stand_profile_2d()` (shared
by both `stand()` and `stand_sheet()`, so both get it automatically),
sized from the real vent geometry via `vent_world_yz()` rather than
hand-typed numbers — `vent_relief_y0/y1/z0/z1`, with 3mm margin added
on top of the measured slot extent. Because the whole stand extrudes
uniformly along `stand_w`, a single 2D notch in the cross-section opens
the full width automatically, reaching open air at both side edges of
the stand rather than needing 18 individual per-slot tunnels. The horn
keeps its full `Y=[60,68]` and `Y=[80,84]` depth solid on either side of
the notch, plus its full base below the notch — thinned locally where
the vents actually are, not hollowed out generally.

Re-verified with the same marker+`trimesh` method, this time sampling
both ends *and* the centre of every slot (108 points total, both rows):
worst-case clearance is now **3.0mm**, zero points inside solid
material. Stand volume dropped from 179.2cm³ to 164.2cm³ (~8% less,
consistent with a local notch, not a structural hollowing-out).

Also fixed in passing: `assembly()`'s device Z-offset (8.6) was still a
second hardcoded copy of the same number `stand_profile_2d()`'s own
groove fix depends on — flagged in that fix's own comment as a
future drift risk, and it was still separately typed in two places.
Hoisted to `device_z_off`, used by both.

**Untested trade-off:** the notch removes horn material at exactly the
point that takes the leaning slab's load. The remaining cross-section
looked reasonable in the render (base + both flanking Y-bands still
full height), but this is a CAD/geometry check, not a load test —
soak/flex-check the printed horn before trusting it holds the device's
weight unattended long-term, same caveat already standing for the
ventilation sizing itself below.

**Reprint required:** the stand only (today `stand_feet` or
`stand_lowtail`, which inherited this relief) — the device side of the
stack (faceplate/shell/lattice/diffuser/panel) is unaffected.

## Ventilation — full design history & sizing derivation (v0.1.1 — runs 24/7, so this isn't optional)

The original design pressed a **solid** 3 mm foam sheet against the
panel's full back face, sealed inside an otherwise-unvented shell —
fine mechanically, bad thermally. WS2812B panels dissipate real heat
(firmware caps sustained draw at 2.5 A/5 V = 12.5 W via
`setMaxPowerInVoltsAndMilliamps`), and running attract-mode content
unattended on a desk for hours had no way to shed that heat: sealed
PETG-GF shell, foam insulating the one surface that would otherwise
convect.

Fixed two ways, both in `shell()`:

- **Pressure ribs → pad grid.** The old ribs were already only 4 thin
  strips (not a full pad), so the real culprit was the *foam*, not the
  ribs — but a "#"-shaped pair of full-length strips still blocks two
  continuous bands across the whole back. Replaced with a sparse 3×3
  grid of 8×8 mm pads (`pad_off`/`pad_w`/`pad_h`), skipping the one
  point that lands in the pod cavity — 8 pads total. The lattice
  already supports the panel's front face at full 10 mm pitch, so the
  rear pads only need to take up stack tolerance, not hold the panel
  flat on their own. Foam gets cut to match: pads-worth of small
  squares, not a sheet — most of the back is now open air.

  **Corner pads:** the panel is a *flexible* PCB matrix, and the 3×3
  grid alone (reaching only ±`pad_off` = ±40) leaves the outer 40 mm
  of every edge — including all four corners at (±80,±80), the LED
  grid's actual extent — with zero clamping pressure, exactly where a
  flexible board sags most. Added pads at all 4 corners
  (±`corner_pad_off` = ±70 on both axes) reaching much closer to the
  corners, checked clear of the vent rows, corner posts, and lip ring.
  Uniform on all 4 now — an earlier pass here had the ESP32 cradle
  still parked in one of the corners, which cost that corner a full
  pad and needed a smaller special-cased one instead; moving the
  cradle to an internal position above the pod (see Power & access)
  freed it up, so this is simpler than it used to be, not more
  complex.

  **Edge-midpoint pads:** corners had dedicated support, but the
  left/right edge *midpoints* didn't — nearest pad was the inner
  grid's (±40, 0), 40mm from the true edge at ±80. Added (±`corner_pad_off`,
  0), reusing the same already-verified-clear X offset as the corner
  pads, cutting that to 10mm. 14 pads total now. Verified against a
  real physical concern, not just theory: checked the wire-exit
  positions from actual photos of the panel (three connector clusters
  along one edge) against the full pad grid, including these two —
  nearest pad-to-wire clearance is 17mm even along the tightest
  straight-line routing path, so the added pads don't crowd the
  wiring either. Manifold and all 5 `intersection()` checks (vents,
  posts, lip ring, ESP32 cradle, pod mount) re-verified with the
  larger grid — clear.

  Every pad in the grid — inner, corner, and these two — lands on an
  exact lattice wall intersection (walls at every 10mm: −80, −70, ...
  0, ... 70, 80; LEDs sit at the odd half-pitch positions in between,
  −75, −65, etc.), confirmed by checking pad coordinates against the
  actual wall positions computed from `lattice()`, not assumed from
  the round numbers looking plausible. Every pad presses on a rigid
  wall-to-wall crossing, never on open cell area or directly behind
  an LED.
- **Vent slots through the lid.** Two rows of 18 slots each, 3×25mm
  (`vent_slot_w/l/n`, `vent_span`, `vent_y`), positioned to clear the
  pad grid, the ESP32 cradle, the pod cutout, and the corner posts —
  checked with `intersection()` against each (all empty, OpenSCAD
  2021.01). Rows sit at the device's top and bottom edges (**shell y
  is mirrored vs the assembled device** — see the cradle comment in
  `shell()`) for a passive chimney effect, since the stand racks the
  face at a real angle: cool air in low, warm air out high, no fan.

  Sizing isn't a guess: the natural-convection stack-effect scaling
  A ≈ P / (ΔT^1.5 · √h), calibrated against the worked example in
  *Electronics Cooling*'s "A Practical Formula for Air-Cooled Boards
  in Ventilated Enclosures" (1997) — P′=10 W, ΔT=50°C, h=0.2m →
  643 mm²/vent — scaled to our h=0.136m (2×`vent_y`). Designed for
  P=10W (attract-mode-level sustained load, not just the ~3W word-
  display baseline) and ΔT=35°C, which calls for ~1300 mm²/row; 18
  slots at 3×25mm ≈ 1315 mm²/row. The original 9-slot pass (~291 mm²)
  only covered the best case (3W, 50°C rise) — undersized by 3–9×
  for realistic sustained load, hence the resize.

  This is still a rough lower bound, not CFD: the scaling omits the
  paper's own viscous flow-resistance term (real required area is
  probably somewhat *more* than this), and it's calibrated on a
  generic populated-board enclosure, not this specific flat-panel-
  near-vents geometry.

No filament swap needed — PETG-GF doesn't flex well enough for a
cantilever/spring-rib alternative anyway, and rigid ribs need the
foam's compressibility to take up tolerance regardless, so the pad
grid was the change, not the rib material.

**Not yet thermally validated** — the collision checks and the sizing
calc above are both just paper (well, CAD). Soak-test after reprinting
(attract mode, few hours, ambient desk conditions, check the panel/
ESP32 aren't running hot to the touch) before trusting this unattended
long-term; `vent_slot_n`/`vent_span` are easy to bump further if it's
still running warm.

**Reprint required:** `shell()` only — the stencil, diffuser, lattice,
and panel side of the stack are unchanged.

## README hero renders — lighting the face up (`face_display`)

The landing-page render originally showed the enclosure dark. Since
the letters are through-voids, "switching the clock on" in a render is
just dropping bright glyphs into the voids — `face_display` ("off" /
"time" / "invaders", in the [Part] Customizer tab) does exactly that,
reusing `letters2d`'s placement/mirror so the lit glyphs land in the
stencil voids, filled through the plate and 0.05 proud of the front
face. Preview-only: it draws inside `device()`, so no printable part
is affected. Word coordinates and the invader/cannon sprites are the
firmware's own tables (`grid.cpp` / `animations.cpp`, mirrored in
`docs/simulator.html`).

The committed renders (both 1400×1150, same camera so they read as a
pair):

```
openscad -o renders/assembly.png          --imgsize=1400,1150 \
  --camera=0,0,92,78,0,25,560 -D 'face_display="time"'     wordclock.scad
openscad -o renders/assembly_invaders.png --imgsize=1400,1150 \
  --camera=0,0,92,78,0,25,560 -D 'face_display="invaders"' wordclock.scad
```

`-o png` without `--render` uses the (fast, colour-capable) preview
renderer, which is what the colour() calls need anyway. The "time"
frame shows 4:23 — IT IS TWENTY THREE MINUTES PAST FOUR, the same
per-minute example the README leads with.
