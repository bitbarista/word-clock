// =================================================================
// TIME INVADERS — 16x16 word clock, subtle desk-wedge enclosure
// -----------------------------------------------------------------
// Shell concept "B — Subtle": plain slab in a raked desk stand.
// The face is a LAYERED STACK of single-colour prints (no filament
// swaps): dark stencil plate (letters are through-voids) → drop-in
// diffuser sheet (print several thicknesses/materials and A/B them)
// → drop-in baffle lattice → LED panel → foam → shell.
// Electronics: 16x16 WS2812B flexible panel (160x160, 10 mm pitch)
// + Seeed XIAO ESP32S3.  See README.md for the print/assembly guide.
// =================================================================

VERSION = "0.1.6";
echo(str("word clock model v", VERSION));

include <font.scad>

/* [Part] */
part = "assembly"; // [assembly, faceplate, lattice, diffuser, shell, stand_lowtail, stand_feet, coupon, coupon_diffuser, pod_coupon, face2d]
// assembly()'s real colours (see device()/assembly() near the bottom of
// this file) are a deliberately dark charcoal/near-black colourway,
// matching the actual enclosure — but that leaves almost no contrast
// in OpenSCAD's own preview, making edges and part boundaries hard to
// read while editing. Flip this off for a high-contrast preview
// palette instead. Print colour is actually arbitrary (whatever
// filament you choose) — true_colors=true (default) just previews a
// dark colourway as one example, not a manufacturing constraint.
true_colors = true; // [true, false]
// Light the face up in the assembly preview: "time" spells out
// IT IS TWENTY THREE MINUTES PAST FOUR (4:23 — shows off the
// per-minute wording), "invaders" shows a frame of the Space
// Invaders attract animation (sprites match the firmware's
// animations.cpp). Render/preview only — adds nothing to any
// printable part, and does nothing outside part="assembly".
face_display = "off"; // [off, time, invaders]

/* [Mounting] */
// Single switch for the two mounting styles — sets power_pos and
// keyhole together (see [Electronics] below) and drops the desk stand
// from the assembly() preview for "wall", so there's one place to look
// instead of hunting for the individual params. Still just a derived
// default: power_pos/keyhole are ordinary variables underneath, so
// advanced/mixed setups (e.g. bottom connector on a desk build) are
// still possible by editing them directly further down — this toggle
// just isn't exposed as a separate Customizer control for that case,
// since it's not the common path.
mount_type = "desk"; // [desk, wall]

/* [LED panel] */
// LED-to-LED spacing of the matrix panel
pitch = 10;
// LEDs per side
cells = 16;
// panel PCB + LED component height — measured (calipers): bare FPCB
// 0.25mm, 1.8mm total with LEDs mounted. 2.0mm modeled keeps a small
// margin over the measured 1.8mm.
panel_t = 2.0;
// Depth of the cavity behind the panel — houses the foam pressure
// pads, the ESP32 locator, the 1000uF cap, and (as of this revision)
// the power socket. Was 3.0mm ("foam_t"), sized only for a
// compressible foam sheet without ever checking it against the real
// components that have to physically live in it: the ESP32 (XIAO
// measured 4.49mm PCB-to-USB-C-top) and the 1000uF cap (11.5mm
// minimum, any standard package, leaded or SMD) both failed to fit —
// see README for the full audit. Resized to the tallest real
// component (the cap) plus margin, rather than patching each
// component separately or adding local bump-outs: a flat back was a
// deliberate requirement, so the extra depth goes into the whole
// device's thickness instead. 11.5mm + 1.5mm margin = 13mm.
back_t = 13.0;

/* [Face plate] */
// stencil plate thickness (letters are through-voids)
t_front = 1.2;
// diffuser sheet thickness — a SEPARATE drop-in part; print a few
// (0.6 / 0.9 / 1.2, clear vs white) and A/B them in the coupon slot
t_diff = 0.6;
// depth of the per-cell baffle lattice behind the diffuser
baffle_d = 12;
// baffle wall thickness (lands between LEDs)
lat_wall = 1.2;
// bezel width beyond the LED grid
bezel = 12;
// stack-pocket wall: hollow "frame within a frame" instead of solid
// (was a solid ring baffle_d deep, ~11.5mm wide around the whole
// perimeter -- most of that a slicer would fill as infill regardless
// of settings). Thin outer + inner walls connected by sparse ribs,
// with a thin solid cap at the front for a consistent visible
// bezel surface.
bezel_wall_t = 2.0;   // outer/inner wall thickness
bezel_cap_t = 1.5;    // solid front cap; rest of the depth is ribbed
bezel_rib_w = 3.0;    // rib width
bezel_rib_n = 4;      // ribs per straight edge (not counting corners)
// letter pixel size (letter = 5 x 7 pixels)
font_px = 1.1;
// face corner radius
corner_r = 3;
// per-side clearance for drop-in parts (lattice, diffuser) — FDM
// dimensional error + elephant-foot easily eats 0.2mm; this is
// clearance per EDGE, so the part shrinks by 2x this all round
fit_clr = 0.4;

/* [Body] */
// tray perimeter wall thickness
wall_t = 2.4;
// rear shell lid thickness
lid_t = 2.4;
// shell lip engagement depth
lip_h = 4;
// corner screw post outer diameter
post_d = 7;
// M3 self-tap pilot in the corner posts
post_hole_d = 2.7;
// radial clearance the shell's lip must keep from each corner post —
// the lip routes around the post rather than running through it, so
// this is a real per-edge allowance (not the ~0.4mm nominal gap the
// lip_out/wall_t formula implies, which the rsq() corner-rounding
// quirk eats into — see README fit note)
post_relief_clr = 1.0;
// support ribs for the corner posts: post height is wall_top-t_front,
// which grew substantially (17.4mm -> 27.4mm) when back_t increased
// for the depth-budget fix — a 7mm-diameter post that tall is a real
// snap risk, especially under screw-driving torque. Ribs connect each
// post to the tray wall in both X and Y. Kept clear of the shell's
// lip zone (the top lip_h of the post height, where the shell's lip
// registers and routes around the post — see shell()) by this much
// extra margin, so the ribs can't interfere with that routing.
post_rib_w = 3.5;     // rib width (tangential)
post_rib_clr = 2.0;   // margin below the lip zone

/* [Electronics] */
// Seeed XIAO ESP32S3 — measured with calipers (checked against
// third-party CAD bounding-box data: 22.48 x 4.46 x 17.78mm incl.
// USB-C overhang, a close match), not taken from a vendor listing.
// No mounting holes on this board — retained by a friction-clip
// slide-in cradle, not screws.
// XIAO ESP32S3 own measurements, kept for reference (no longer drive
// the cradle geometry directly — see brd_w/brd_l below)
esp_l = 21.12;             // board length (long axis, PCB only)
esp_w = 17.83;              // board width (PCB only)
esp_t = 1.15;               // bare PCB thickness
esp_clr = 0.4;              // per-edge clearance for the locator
// Universal locator footprint, not just XIAO-shaped: a generic
// "ESP32-S3 supermini" is 22.52 x 18mm (espboards.dev) vs the XIAO's
// measured 21.12 x 17.83mm — widths match to within 0.17mm, lengths
// differ by 1.4mm (supermini longer). Sized to the wider of each
// dimension so either board locates the same way; the shorter XIAO
// just gets ~1.4mm of lengthwise play, which is fine now that this
// is a locator, not a snug retention cradle (see below).
brd_w = 18.0;
brd_l = 22.52;
// cradle centre — internal position above the pod, long axis along X.
// Flash via USB on the bench pre-assembly, OTA after (see shell()
// cradle comment) — no more side-wall port, so no usb_side/usb_up
// needed.
esp_x = 0;
esp_y = 20;
// Now a pure locator (X/Y position only), not a snug friction-fit
// cradle — final retention comes from the soldered leads plus the
// foam/shell closing over everything, so it doesn't need to clear
// the USB-C connector's height (4.49mm) or grip the board tightly.
// Wall height dropped from that 5mm to a short 1.8mm step (matches
// the rib-thickness convention elsewhere) — real bulk reduction, and
// unlike cutting a recess INTO the 2.4mm lid (which would thin it
// right where it resists foam-pad compression), this only affects
// material added on top, so lid strength is untouched.
cradle_h = 1.8;
// Single end-stop, not corner L-brackets — those touched both a
// short-edge corner AND reached along the long edge, and pads on
// both board variants run close to the board ends, not just the
// long edges, so any corner contact risked landing on one. A single
// straight wall across ONE short edge only contacts the end face,
// leaves both long edges (where most of the castellated pads are)
// and the OTHER short edge completely untouched. Placed at -X;
// insert either board with its USB-C connector facing the open (+X)
// end — the geometry doesn't know or care which physical end either
// board's connector is actually on, so this is an assembly
// instruction, not a design assumption.
esp_stop_t = 1.8;   // end-stop wall thickness
// Power socket — snap-in USB-C, mounted DIRECTLY INTO the shell's
// own material now, not a separate bump-out part (v0.1.6). Used to
// be its own printed part, screwed onto the shell from the inside —
// meant removing it required separating the shell from the whole
// rest of the assembly to reach those screws. Now that back_t gives
// the shell real interior depth, the socket's snap-fit flange passes
// through the shell's own lid_t directly, with the wing-deployment
// cavity extending into the interior (that interior space was already
// there for the electronics — this doesn't need extra depth beyond
// back_t, just doesn't collide with what else lives there). No
// mounting screws at all: the socket's own snap wings retain it
// against the shell material, same mechanism as before, one less
// separate part and one less set of screws (all remaining screws —
// just the 4 corner ones — were already outside-accessible).
// Flange sits against lid_t = 2.4mm; the verified-working figure
// from the old separate-pod design was pwr_face_t = 2.0mm exactly —
// 0.4mm more here. Not verified against the physical part's snap-
// wing flex range; worth checking on the actual socket before
// trusting it blind.
pwr_pod = true;
// back: mounted through the shell's flat lid, port faces straight out
// the back (original design, works fine desk-mounted). bottom: mounted
// through the tray's own bottom wall instead, port faces down — the
// back mount is unusable wall-mounted (the port would be sandwiched
// against the wall), so this is the wall-mount config. The two are
// mutually exclusive (one physical socket, one orientation at a time),
// not a combinable pair with the desk stand — when wall-mounted the
// stand isn't fitted at all, so there's no cable-clearance conflict to
// design around.
// Driven by mount_type above (not its own Customizer control) — edit
// this line directly if you want a mixed setup (e.g. bottom connector
// on a desk build); valid values are "back" and "bottom".
power_pos = mount_type == "wall" ? "bottom" : "back";
// snap-in cutout in the shell
pwr_snap_w = 14.7;
pwr_snap_h = 5.4;
pwr_snap_r = 1.3;
// interior cavity: body + wing deployment (depth cd unchanged from
// the old separate-pod design — that's the verified-working number,
// just relocated, not re-derived)
pwr_cavity = [18, 10, 8];
// pod centre height above the bottom face edge (keep pod + plug
// clear of the 34 mm stand horn) — power_pos="back" only
pod_up = 48;
// power_pos="bottom" only: Z-height (device frame) of the connector's
// centreline — see pod_bottom_z below (defined next to wall_top,
// which it depends on; OpenSCAD evaluates top-level expressions in
// file order, same reason flat_foot_t had to move earlier in this
// file).

// Wall-mount keyhole slot, cut into the shell's lid with a local
// reinforcement boss added on the inside face — lid_t=2.4mm alone is
// too thin to safely bear the whole assembled weight hanging off a
// single screw. No real screw/anchor spec was given for this, so
// these dimensions are a reasonable assumption (sized for a common
// picture-hanging screw, ~7-8mm panhead), not a verified fit — check
// against the actual screw before trusting it blind, same caveat as
// pwr_pod's own flange dimensions above.
// Driven by mount_type above (not its own Customizer control) — edit
// this line directly for a mixed setup (e.g. a keyhole on a desk
// build for extra security on a shelf).
keyhole = mount_type == "wall";
// shell-local Y of the entry circle's centre — between the (0,0) and
// (0,-pad_off) pressure pads (see [Ventilation] below), clear of both
// with margin (verified by rendering + checking against the real pad
// geometry, not just this comment's own arithmetic).
keyhole_y = -18;
keyhole_entry_d = 9;      // screw head clearance (entry circle)
keyhole_slot_w = 4.5;     // screw shank clearance (the narrow part
                           // the shank rests in once hung)
keyhole_slot_len = 10;    // drop distance from the entry circle's own
                           // centre to the slot's closed (top) end
keyhole_boss_margin = 4;  // extra material radially around the cut,
                           // grown from the keyhole's own outline
                           // (not a separate circle) so the boss
                           // follows the slot shape closely
keyhole_boss_t = 3;       // extra thickness the boss adds beyond
                           // lid_t, on the inside face only — outside
                           // profile is unaffected

/* [Ventilation] */
// rear pressure-pad grid (replaces the old full-length ribs) —
// sparse contact points instead of long strips, so most of the back
// stays open for airflow. The lattice already supports the panel's
// front face at full 10 mm pitch, so these only need to take up
// stack tolerance, not provide fine flatness.
pad_off = 40;
pad_w = 8;
// pad_h was 1.0mm, sized when back_t (then foam_t) was 3.0mm: a
// short rigid nub plus a 3mm foam pad (BOM) together reached the
// panel, foam doing most of the bridging. Never revisited when
// back_t grew to 13mm for the depth-budget fix — nub + foam only
// reached 4mm of a 13mm gap, leaving the pads 9mm short of the
// panel and doing nothing at all. This is exactly the kind of
// downstream break the depth-budget change should have been
// checked against at the time, not found later. Grown so nub + the
// still-3mm foam (BOM unchanged) reaches 13.5mm — 0.5mm of
// deliberate interference so the foam is under real compression,
// not just barely touching, same margin logic used elsewhere in
// this file (e.g. the corner-post rib clearance).
pad_h = 10.5;
// extra pads reaching toward the panel corners (±80,±80 — the LED
// grid's actual corners) rather than stopping at pad_off like the
// inner grid. A flexible PCB matrix sags most at unsupported
// corners, and pad_off alone leaves the outer 40mm of every edge
// (incl. all 4 corners) with zero clamping pressure. Placed at ±70 on
// both axes to clear the vent rows (|x|<=60), the corner posts, and
// the lip ring — uniform on all 4 corners now that the ESP32 cradle
// moved off the tray-wall corner (see shell()); it used to block one
// of these and need a smaller special-cased pad instead.
corner_pad_off = 70;
// centre-row edge pads (±corner_pad_off, 0): the corners get
// dedicated support, but the left/right edge MIDPOINTS didn't —
// nearest pad was the inner grid's (±40,0), 40mm from the true edge
// at ±80. Reuses corner_pad_off (already a verified-clear X
// position) rather than a new offset, cutting that to 10mm.
// two rows of slots cut through the lid for passive convection
// (device sits raked in its stand, so top/bottom is a real chimney
// axis). Span/offset chosen to clear the pad grid (|x,y| <= pad_off)
// and the pod cutout (|x| <= 9, at smaller |y| than vent_y), and the
// corner posts (post_off) — see shell() for the clearance reasoning.
// (The ESP32 cradle no longer needs a clearance mention here — it
// moved off the tray-wall corner to an internal position near the
// pod, well clear of both vent rows.) FIRST PASS: this hasn't been
// thermally validated, only checked in CAD for collisions — soak-test
// after reprint before trusting it unattended.
// sized for a P=10W (attract-mode-level sustained load) / DeltaT=35C
// design point via the natural-convection stack-effect scaling
// A ~= P / (DeltaT^1.5 * sqrt(h)) -- calibrated against the worked
// example in Electronics Cooling's "A Practical Formula for Air-
// Cooled Boards in Ventilated Enclosures" (1997): P'=10W, DeltaT=50C,
// h=0.2m -> 643 mm^2/vent. Scaled to our h=0.136m (2*vent_y) gives
// ~1300 mm^2/row; 18 slots at 3x25mm = ~1315 mm^2. This scaling
// omits the paper's own viscous-resistance term, so it's a rough
// lower bound, not a CFD result -- soak-test before trusting it.
vent_slot_w = 3;
vent_slot_l = 25;
vent_slot_n = 18;
vent_span = 120;
vent_y = 68;

/* [Stand] */
// backwards rake of the face. Default 12° suits viewing from a bit of
// distance/near eye-level; 15° is a supported alternative for a clock
// sitting close to the viewer and glanced down at (e.g. right next to
// a keyboard) — both angles have been fully verified for stand_feet()
// (opening width, device interference, edge contact, vent clearance).
// Values outside {12, 15} fall back to whatever the geometry naturally
// does — re-run the verification suite before trusting a new value.
tilt = 12; // [5:25]
stand_w = 120;
// stand_depth/stand_h and the wedge polygon in stand() were sized
// for the old slab_t=21mm (groove_w~=21.8mm). Never revisited when
// back_t grew slab_t to 31mm (groove_w~=31.8mm, +10mm) — groove_w
// scales automatically (it's a formula) but the wedge cross-section
// didn't, so the enlarged slot cut clean through the front lip
// instead of just the gap it used to sit in, collapsing the lip and
// horn into a flat shell (rendered and visually confirmed broken
// before this fix). Grew stand_depth (92->104) and pushed the horn
// back in the polygon (see stand()) to restore clearance — verified
// by checking the lip's solid region doesn't intersect the groove
// cutter, not just by the numbers looking plausible (see stand()).
stand_depth = 104;
stand_h = 34;
// slot centre-line at the top face — was 40, moved to 54 so the
// wider groove (slab_t grew) clears the front lip (occupies
// Y=[24,32]) with a real margin instead of cutting through it.
// Shared between stand() and assembly() (device positioning) so
// there's one number to keep in sync, not two — assembly() used to
// hardcode this separately and silently go stale when stand()'s own
// copy changed.
groove_y = 54;
// slot clearance around the slab
groove_clr = 0.8;
// device's Z-offset in assembly() — was hardcoded separately there
// (and had already drifted once, 4.6 -> 8.6, when the groove-pinch-
// point fix above needed it to move; see that comment). Hoisted here
// so assembly() and the vent-relief calc below share one number
// instead of two that can silently disagree.
device_z_off = 8.6;

/* [Hidden] */
$fa = 4; $fs = 0.4;
eps = 0.01;

// face_display="invaders" attract-march frame index, 0-9 — drives the
// README GIF (see render_readme.sh): the invader ping-pongs across
// the grid's 5 free columns alternating the two classic sprite poses,
// the cannon drifts the opposite way. Default 2 = the committed
// static renders/assembly_invaders.png frame.
anim_t = 2;

// ----------------------------------------------------------------
// The letter grid, as read from the front. PER-MINUTE resolution:
//   IT IS TWENTY THREE MINUTES PAST FOUR
// Letter-sharing keeps it compact: FOURTEEN/SIXTEEN/SEVENTEEN/
// EIGHTEEN/NINETEEN carry FOUR/SIX/SEVEN/EIGHT/NINE as prefixes,
// THREELEVEN = THREE+ELEVEN, TWONE = TWO+ONE, THREEIGHT =
// THREE+EIGHT, SEVENINE = SEVEN+NINE, PASTO = PAST+TO, and
// MINUTE is a prefix of MINUTES.  Rows 0-8 = minutes, 9-12 =
// hours + OCLOCK, 13-15 = arcade hall of fame.  Hidden words:
// HIGH SCORE, INSERT COIN, ZAP/PAC/POW (stacked), GO, GAME/OVER
// (stacked), SPACE INVADERS, PEW, GALAGA, DONKEY KONG,
// ASTEROIDS, QBERT, UP.  The X in row 8 is a deliberate spacer
// so the ever-lit MINUTES PAST reads as two words.
// ----------------------------------------------------------------
GRID = [
    "ITKISAHIGHSCOREZ",
    "TWENTYINSERTCOIN",
    "FOURTEENSIXTEENA",
    "SEVENTEENTWELVEB",
    "EIGHTEENNINETEEN",
    "THIRTEENQUARTERS",
    "THREELEVENTENZAP",
    "TWONEFIVEHALFPAC",
    "MINUTESXPASTOPOW",
    "TWONETHREEIGHTGO",
    "SEVENINEFOURFIVE",
    "SIXTENELEVENGAME",
    "TWELVEOCLOCKOVER",
    "SPACEINVADERSPEW",
    "GALAGADONKEYKONG",
    "ASTEROIDSQBERTUP"
];
assert(len(GRID) == cells, "GRID row count != cells");
for (r = [0:len(GRID)-1])
    assert(len(GRID[r]) == cells, str("GRID row ", r, " is not ", cells, " chars"));

// 4x4 sample for a cheap test print (colour change + legibility)
COUPON = [
    "TIME",
    "GAME",
    "OVER",
    "WORD"
];

// derived
face_w   = cells * pitch + 2 * bezel;         // 184
wall_top = t_front + baffle_d + panel_t + back_t + 0.4;  // 28.6
slab_t   = wall_top + lid_t;                  // total device thickness
post_off = face_w/2 - 8.5;                    // corner post centres
lip_out  = face_w/2 - wall_t - 0.4;           // shell lip outer half-width
groove_w = slab_t + groove_clr;
// power_pos="bottom": Z-height (device frame) of the connector's
// centreline, through the tray's own bottom wall instead of the
// shell's lid. Centred in the same back_t cavity the back-mounted
// version already proves is clear of the lattice/panel stack — that
// cavity spans device Z=[t_front+baffle_d+panel_t, wall_top] =
// [15.2, 28.6], so the centre is 21.9. Not yet checked against the
// shell's own pads/ESP32 cradle at the corresponding device position
// (those are positioned in shell()'s own flipped frame) — verify
// before trusting this blind, same as everything else in this file.
pod_bottom_z = (t_front + baffle_d + panel_t + wall_top) / 2;

// ----------------------------------------------------------------
// helpers
// ----------------------------------------------------------------
// rounded square, actual outer size = w with corner radius r. (Was
// `offset(r) offset(-r) square(w-2*r)` — the redundant second offset
// eroded the dilation straight back off, silently delivering a
// w-2*r actual size for every rsq()'d part; see README fit note.)
module rsq(w, r = corner_r) offset(r) square(w - 2*r, center = true);

// stack-pocket wall, hollow: thin outer (fw) + inner (op) walls, a
// thin solid cap at the front for a consistent visible surface, and
// sparse ribs behind it connecting the two walls — see bezel_wall_t
// etc. comment. Ribs run along the 4 straight edges only (margin from
// each corner keeps them clear of both the outer rounding and the
// corner posts, which sit in this same radial band near the corners
// — verified with intersection(), not just by the margin looking
// big enough).
module bezel_frame(fw, op, depth) {
    rib_span = op - 20;
    union() {
        linear_extrude(bezel_cap_t)
            difference() { rsq(fw); square(op, center = true); }
        translate([0, 0, bezel_cap_t])
            linear_extrude(depth - bezel_cap_t) {
                difference() { rsq(fw); rsq(fw - 2*bezel_wall_t, corner_r - 1); }
                difference() {
                    square(op + 2*bezel_wall_t, center = true);
                    square(op, center = true);
                }
                for (rot = [0, 90, 180, 270])
                    rotate(rot)
                        for (i = [0 : bezel_rib_n - 1])
                            translate([-rib_span/2 + i*(rib_span/(bezel_rib_n - 1)),
                                       (op/2 + fw/2)/2])
                                square([bezel_rib_w, fw/2 - op/2], center = true);
            }
    }
}

// stadium vent slot, long axis along Y (rotate/place as needed) —
// width vent_slot_w, total length vent_slot_l
module vent_slot()
    hull() for (s = [-1, 1])
        translate([0, s * (vent_slot_l - vent_slot_w)/2]) circle(d = vent_slot_w);

// Letter voids for an n x n grid, MIRRORED so the text reads
// correctly from the front when the plate prints letters-down
// (model z=0 is the front face, on the bed).
module letters2d(n, grid) {
    mirror([1, 0, 0])
        for (r = [0:n-1], c = [0:n-1])
            translate([(c - (n-1)/2) * pitch, ((n-1)/2 - r) * pitch])
                glyph2d(grid[r][c], px = font_px);
}

// ----------------------------------------------------------------
// face plate: stencil + diffuser + baffle lattice (+ tray walls)
// ----------------------------------------------------------------
module face_core(n, grid, fw, tray = true) {
    gs = n * pitch;      // LED grid span
    op = gs + 1;         // panel opening (0.5 clearance/side)

    // stencil plate: letters are through-voids, the diffuser sheet
    // sits directly behind (dropped in at assembly)
    linear_extrude(t_front)
        difference() { rsq(fw); letters2d(n, grid); }

    // stack-pocket wall: captures diffuser + lattice + PANEL now
    // (depth = baffle_d + panel_t, was baffle_d alone). The old
    // baffle_d-only depth left the panel with no snug pocket at
    // all — beyond baffle_d the only wall is the tray wall further
    // out (fw-2*wall_t = 179.2mm), so the 160mm panel just floated
    // loose in that much bigger opening instead of being captured
    // the way the lattice/diffuser are. Same op opening continues
    // through both stages, so the panel gets the same 0.5mm/side
    // clearance convention as the lattice/diffuser drop-in fit.
    // Hollow frame-within-frame construction — see bezel_frame().
    translate([0, 0, t_front])
        bezel_frame(fw, op, baffle_d + panel_t);

    if (tray) {
        // perimeter tray wall up to the shell seat. Used to be
        // notched here for the ESP32's onboard USB-C to protrude
        // through the side; the ESP32 moved to an internal cradle
        // above the pod (flash once via USB on the bench before
        // final assembly, OTA after that — see shell() cradle
        // comment), so the wall is now a plain closed ring. An
        // already-printed faceplate from before this change still
        // has the old notch — it's just a harmless unused opening,
        // no need to reprint on its account alone.
        //
        // power_pos="bottom": this wall (not anything in shell()) IS
        // the true exterior surface at the edges, so the connector's
        // passage has to cut through it here, not through shell()'s
        // own (inset) lip. Device bottom = faceplate's own -Y (see
        // shell()'s own vent-cutting comment for the sign derivation —
        // shell()'s local +y is device bottom, and shell() gets
        // rotate([180,0,0]) in device(), so device_y=-shell_y and
        // device bottom is therefore -Y here, where shell()'s rotation
        // doesn't apply).
        difference() {
            translate([0, 0, t_front]) linear_extrude(wall_top - t_front)
                difference() { rsq(fw); rsq(fw - 2*wall_t, corner_r - 1); }
            if (pwr_pod && power_pos == "bottom")
                translate([0, -fw/2, pod_bottom_z])
                    rotate([-90, 0, 0])
                        pwr_connector_cut(wall_t);
        }
        // corner screw posts. Pilot hole is a fixed 10mm deep
        // regardless of post height (post is now wall_top-t_front =
        // 27.4mm tall, up from 17.4mm, since wall_top grew with
        // back_t — the post itself just got longer below the pilot,
        // not the engagement geometry).
        //
        // Screw length: the BOM's original M3x8 was never checked
        // against how much of that length is actually usable. The
        // screw has to clear lid_t+lip_h = 6.4mm of shell material
        // (pure clearance, no threads) before it even reaches this
        // post, which leaves an M3x8 only 1.6mm of actual thread
        // engagement — far short of the ~2x-diameter (6mm) guidance
        // for a reliable self-tapping joint into plastic. Needs
        // M3x16 (9.6mm engagement, just under the 10mm pilot depth)
        // — see BOM.
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx*post_off, sy*post_off, t_front])
                difference() {
                    cylinder(d = post_d, h = wall_top - t_front);
                    translate([0, 0, wall_top - t_front - 10])
                        cylinder(d = post_hole_d, h = 10 + eps);
                }
        // post support ribs — see post_rib_w/post_rib_clr comment
        // above. Each post gets two ribs (X and Y) to the tray wall.
        // rib_in overlaps 1mm into the post, rib_out overlaps 0.5mm
        // into the tray wall, so both ends fuse cleanly rather than
        // meeting at a zero-thickness tangent.
        // rib_in/rib_out are LOCAL offsets from the post centre (this
        // whole block is inside translate([sx*post_off, sy*post_off,
        // ...])) — rib_out MUST subtract post_off, or it's the tray
        // wall's position from the faceplate origin instead of from
        // the post, overshooting by post_off (~83.5mm) as an earlier
        // version of this did.
        rib_h   = wall_top - t_front - lip_h - post_rib_clr;
        rib_in  = post_d/2 - 1;
        rib_out = fw/2 - wall_t - post_off + 0.5;
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx*post_off, sy*post_off, t_front])
                linear_extrude(rib_h) {
                    translate([sx*(rib_in + rib_out)/2, 0])
                        square([rib_out - rib_in, post_rib_w], center = true);
                    translate([0, sy*(rib_in + rib_out)/2])
                        square([post_rib_w, rib_out - rib_in], center = true);
                }
    }
}

module faceplate() face_core(cells, GRID, face_w);

// drop-in baffle lattice: registers in the faceplate's opening,
// presses the diffuser sheet against the stencil plate. Depth is
// derived so stencil + diffuser + lattice = panel plane; if you
// settle on a different diffuser thickness, reprint to match.
module lattice() {
    gs = cells * pitch;
    // faceplate opening is gs+1 (fixed — already printed); shrink the
    // lattice to fit inside it with fit_clr clearance per edge
    ow = gs + 1 - 2*fit_clr;
    ld = baffle_d - t_diff;
    difference() {
        // clip the whole cross-section to ow: the outermost crossbars
        // sit AT the LED-grid edge (gs/2) and are lat_wall wide, so
        // uncapped they overshoot to gs+lat_wall (161.2mm) regardless
        // of ow — that overshoot, not the frame, was the real fit bug
        linear_extrude(ld) intersection() {
            union() {
                difference() { square(ow, center = true);
                               square(ow - 2*lat_wall, center = true); }
                for (k = [0:cells]) {
                    translate([-gs/2 + k*pitch, 0]) square([lat_wall, ow], center = true);
                    translate([0, -gs/2 + k*pitch]) square([ow, lat_wall], center = true);
                }
            }
            square(ow, center = true);
        }
        // elephant-foot relief: shave the first 0.6mm of the OUTER
        // frame by an extra 0.3mm/side so first-layer squish can't
        // bind the fit even before any slicer tuning
        translate([0, 0, -eps]) linear_extrude(0.6 + eps)
            difference() {
                square(ow + 1, center = true);
                square(ow - 0.6, center = true);
            }
    }
}

// diffuser sheet: the experiment. Print at t_diff in clear or white
// (0.6 / 0.9 / 1.2 are worth comparing), 100% infill.
module diffuser() {
    gs = cells * pitch;
    linear_extrude(t_diff) square(gs + 1 - 2*fit_clr, center = true);
}

// ----------------------------------------------------------------
// coupon: 4x4 sample of the face with a SLIDE-IN diffuser slot on
// one edge — swap test strips while holding it over a lit LED.
// ----------------------------------------------------------------
coupon_n = 4;
coupon_fw = coupon_n*pitch + 8;
coupon_slot = 1.2 + 2*fit_clr;   // fits the tallest planned test strip (1.2mm)

module coupon() {
    gs = coupon_n * pitch;
    op = gs + 1;
    // stencil
    linear_extrude(t_front)
        difference() { rsq(coupon_fw); letters2d(coupon_n, COUPON); }
    // spacer ring forming the slot, open on the -y edge
    translate([0, 0, t_front]) linear_extrude(coupon_slot)
        difference() {
            rsq(coupon_fw);
            square(op, center = true);
            translate([0, -coupon_fw/2]) square([op, coupon_fw], center = true);
        }
    // integrated mini lattice above the slot
    translate([0, 0, t_front + coupon_slot]) linear_extrude(10) {
        difference() { rsq(coupon_fw); square(op, center = true); }
        for (k = [0:coupon_n]) {
            translate([-gs/2 + k*pitch, 0]) square([lat_wall, op + 2], center = true);
            translate([0, -gs/2 + k*pitch]) square([op + 2, lat_wall], center = true);
        }
    }
}

// test strip for the coupon slot: sized to slide, with a pull tab
module coupon_diffuser() {
    w = coupon_n*pitch + 1 - 2*fit_clr;   // matches coupon's op = gs+1
    linear_extrude(t_diff) {
        square(w, center = true);
        translate([0, -w/2 - 4]) square([12, 9], center = true);
    }
}

// ----------------------------------------------------------------
// power socket coupon: standalone test of the snap-in cutout +
// cavity, same local geometry as the real cutout in shell()
// (pwr_snap_w/h/r, pwr_cavity, at lid_t thickness) — checks the one
// unverified assumption from the v0.1.6 pod-integration redesign
// before committing to a full shell print: the flange now sits
// against lid_t (2.4mm) instead of the previously-confirmed-working
// 2.0mm pod face. Front (flange) face prints on the bed, same
// orientation as the real shell.
// ----------------------------------------------------------------
pod_coupon_w = 40;
pod_coupon_h = 40;

module pod_coupon() {
    coupon_t = lid_t + pwr_cavity[2];
    difference() {
        linear_extrude(coupon_t) square([pod_coupon_w, pod_coupon_h], center = true);
        // snap-fit flange cutout — the untested dimension
        translate([0, 0, -eps]) linear_extrude(lid_t + 2*eps)
            offset(pwr_snap_r) offset(-pwr_snap_r)
                square([pwr_snap_w, pwr_snap_h], center = true);
        // cavity behind it: body + wing deployment, open at the back
        translate([0, 0, lid_t - eps]) linear_extrude(pwr_cavity[2] + 2*eps)
            square([pwr_cavity[0], pwr_cavity[1]], center = true);
    }
}

// Local frame shared by both power_pos mounting orientations: the
// mounting face is local z=0 (port facing local -Z), flange cut
// through `through_t` of material, cavity for body + wing deployment
// extending pwr_cavity[2] further into local +Z. Callers position/
// rotate this for whichever real wall it's cutting through — back
// mode uses it as-is (mounting face already faces -Z, matching the
// shell's own lid), bottom mode wraps it in rotate([-90,0,0]) (maps
// local -Z -> world -Y, i.e. port facing out the bottom edge; local Y
// -> world -Z, i.e. the flange's own height dimension becomes the
// device-Z placement, centred via the caller's own translate).
module pwr_connector_cut(through_t) {
    translate([0, 0, -eps]) linear_extrude(through_t + 2*eps)
        offset(pwr_snap_r) offset(-pwr_snap_r)
            square([pwr_snap_w, pwr_snap_h], center = true);
    translate([0, 0, through_t - eps]) linear_extrude(pwr_cavity[2] + eps)
        square([pwr_cavity[0], pwr_cavity[1]], center = true);
}

// Entry circle (bottom, where the screw head goes in) union'd with a
// slot (top, where the shank ends up once the item is hung and slides
// down under gravity — shell's own local -y is device top, per the
// vent-cutting comment below, so the slot's own closed end sits at
// MORE-negative y than the entry circle, i.e. further toward device
// top). Local origin is the entry circle's own centre.
module keyhole_2d() {
    circle(d = keyhole_entry_d);
    translate([0, -keyhole_slot_len/2])
        square([keyhole_slot_w, keyhole_slot_len], center = true);
}

// ----------------------------------------------------------------
// rear shell: flat lid, inner lip, foam-pressure ribs,
// ESP32 (XIAO) pocket, cable exit
// ----------------------------------------------------------------
module shell() {
    difference() {
        union() {
            linear_extrude(lid_t) rsq(face_w);
            // Wall-mount keyhole reinforcement boss — added on the
            // inside face only (outside profile at Z=0 is untouched),
            // grown from the keyhole's own outline (not a separate
            // circle) so the extra material follows the slot shape
            // closely rather than a loosely-fitting blob.
            if (keyhole)
                translate([0, keyhole_y, lid_t]) linear_extrude(keyhole_boss_t)
                    offset(keyhole_boss_margin) keyhole_2d();
            // lip that registers inside the tray wall — routed around
            // the faceplate's corner posts (rather than punched
            // through them) so the ring stays one continuous band
            translate([0, 0, lid_t]) linear_extrude(lip_h)
                difference() {
                    rsq(2*lip_out, 2);
                    union() {
                        rsq(2*lip_out - 3.6, 1.2);
                        for (sx = [-1, 1], sy = [-1, 1])
                            translate([sx*post_off, sy*post_off])
                                circle(d = post_d + 2*post_relief_clr);
                    }
                }
            // pressure-pad grid against the foam/panel (was
            // full-length ribs — see [Ventilation] params). Skips
            // (0, pad_off): it lands inside the pod cavity. Corner
            // pads are now uniform on all 4 corners — the ESP32
            // cradle moving off the tray-wall corner (see below)
            // freed the one that used to need a special-cased pocket
            // pad instead of the standard corner_pad_off square.
            translate([0, 0, lid_t]) {
                for (x = [-pad_off, 0, pad_off], y = [-pad_off, 0, pad_off])
                    if (!(x == 0 && y == pad_off))
                        translate([x, y, 0])
                            linear_extrude(pad_h) square(pad_w, center = true);
                for (x = [-corner_pad_off, corner_pad_off], y = [-corner_pad_off, corner_pad_off])
                    translate([x, y, 0])
                        linear_extrude(pad_h) square(pad_w, center = true);
                // centre-row edge pads — see corner_pad_off comment above
                for (x = [-corner_pad_off, corner_pad_off])
                    translate([x, 0, 0])
                        linear_extrude(pad_h) square(pad_w, center = true);
            }
            // ESP32 board locator — internal, above the pod, long axis
            // along X. Universal footprint (brd_w/brd_l) fits either a
            // Seeed XIAO ESP32S3 or a generic ESP32-S3 "supermini" —
            // see the brd_w/brd_l comment above for the size research.
            // Flash once via USB on the bench before final assembly;
            // OTA after that, so no side-wall port access needed (the
            // old corner-cradle-against-the-wall design this replaced
            // also cost one of the four corner pads above — see
            // [Ventilation]).
            //
            // Single end-stop at -X only (see esp_stop_t comment
            // above) — both long edges and the +X short edge are
            // completely open. Short (cradle_h), since this only
            // locates X/Y now rather than gripping the board.
            translate([esp_x, esp_y, lid_t])
                linear_extrude(cradle_h)
                    translate([-brd_l/2 - esp_stop_t/2, 0])
                        square([esp_stop_t, brd_w + 2*esp_clr], center = true);
        }
        // corner screws: through-hole + counterbore in the outside face
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx*post_off, sy*post_off, -eps]) {
                cylinder(d = 3.4, h = lid_t + lip_h + 1);
                cylinder(d = 6.5, h = 1.4);
            }
        if (pwr_pod && power_pos == "back") {
            // power socket mounts directly into the shell now (see
            // pwr_pod comment above) — snap-fit flange cutout through
            // the lid, cavity behind it for body + wing deployment.
            // No separate part, no screws. power_pos="bottom" cuts the
            // equivalent through faceplate()'s tray wall instead — see
            // pwr_connector_cut()'s own comment for why this can't
            // just be repositioned here (the true exterior surface at
            // an edge is the tray wall, not anything in shell() —
            // shell()'s own lip sits inset wall_t+0.4 from it).
            translate([0, face_w/2 - pod_up, 0])
                pwr_connector_cut(lid_t);
        }
        if (pwr_pod && power_pos == "bottom") {
            // The connector itself is cut through faceplate()'s tray
            // wall (see face_core()'s "if (tray)" block), but shell()
            // still has its own lip ring running around all four
            // edges to register inside that same wall — and this
            // corner of it lands exactly where the connector now
            // sits. Confirmed as a REAL interference, not a guess: a
            // probe volume matching the connector's exact device-frame
            // footprint, boolean-intersected against shell() placed
            // via its own real device() transform, found 74.5mm^3 of
            // overlap right in the lip band (Y=[-89.2,-87.4],
            // Z=[24.6,26.9] — precisely the lip's own inner/outer
            // radius and lid_t..lid_t+lip_h thickness).
            //
            // This cut's own placement is shell()'s local-frame
            // equivalent of the exact same device-frame cut used in
            // face_core() (translate([0,-face_w/2-1,pod_bottom_z])
            // rotate([-90,0,0])) — derived algebraically from shell()'s
            // own device()-placement transform (translate([0,0,slab_t])
            // rotate([180,0,0])), not eyeballed: for shell_local(x,y,z)
            // to land at that same device point after shell()'s own
            // transform, shell_local = rotate(90,[1,0,0]) then
            // translate([0, face_w/2+1, slab_t-pod_bottom_z]) — the
            // sign-flipped rotation (+90 here vs -90 in face_core())
            // is exactly what accounts for shell()'s own 180 flip.
            // Re-verified after implementing: the same probe-volume
            // intersection test now returns empty.
            translate([0, face_w/2 + 1, slab_t - pod_bottom_z])
                rotate([90, 0, 0])
                    translate([0, 0, -1])
                        linear_extrude(wall_t + pwr_cavity[2] + 2)
                            square([pwr_cavity[0], pwr_cavity[1]], center = true);
        }
        if (keyhole)
            translate([0, keyhole_y, -eps]) linear_extrude(lid_t + keyhole_boss_t + 2*eps)
                keyhole_2d();
        // ventilation: two rows of slots through the lid (see
        // [Ventilation] params for the clearance reasoning). Model
        // -y is device TOP (the lid flips onto the tray, so shell y
        // is MIRRORED vs the assembled device: device bottom = model
        // +y), fully clear of every other feature; model +y is
        // device BOTTOM, where the row's x-span keeps it inboard of
        // the pod cutout.
        for (vy = [-vent_y, vent_y])
            for (i = [0 : vent_slot_n - 1])
                translate([-vent_span/2 + i*(vent_span/(vent_slot_n - 1)),
                           vy, -eps])
                    linear_extrude(lid_t + 2*eps) vent_slot();
    }
}

// ----------------------------------------------------------------
// desk stand history: the original design was a solid raked wedge
// with the groove cut straight through it (stand()/stand_profile_2d()
// — removed once stand_feet() below fully replaced it: a permanent-
// mounting screw hole through a ~35mm-deep solid wedge needed either a
// boss protruding past the wedge's own silhouette, or an impractically
// long screw, and stand_feet()'s thin-wall construction solved that
// directly). A hollow "sheet steel" variant of the same wedge
// (stand_sheet()) went with it, for the same reason.
//
// stand_lowtail() below survives as a genuine alternative — a proper
// boolean sweep of the groove cut against the old wedge (not corner-
// point guesses, which had led to a wrong "keeps its full height on
// both sides" claim earlier) showed the groove ALONE thinned the
// wedge's rear horn to 5-19mm across most of Y=[45,75], with the only
// genuinely full-height (stand_h=34mm) material a narrow Y=[76,84]
// peak — stand_lowtail() trades that peak away deliberately (rear
// ceiling drops to ~11mm instead of continuing at stand_h), on the
// theory that it's the groove's own far-end backstop and not much
// else. Untested against an actual load.
// ----------------------------------------------------------------

// Rear ceiling drops to ~11mm (matching the front lip's own height)
// right where the groove is already doing most of the cutting,
// instead of continuing at stand_h up to Y=84 and then notching a
// hole in it. The transition point matters: putting it at Y=79 (right
// after the vent row) left ONE specific point 0.19mm from solid —
// not a bug in this shape, the groove's OWN boundary is naturally
// marginal there. Y=70 clears it properly (4.0mm, verified the same
// way as elsewhere in this file — markers at the real vent positions,
// both ends + centre of all 18 slots, checked against the exported
// mesh with trimesh).
module stand_profile_2d_lowtail() {
    tail_z = 11;
    difference() {
        polygon([
            [0, 0], [stand_depth, 0],
            [stand_depth, tail_z - 4], [70, tail_z],
            [60, stand_h], [32, 13], [24, 13], [0, 5]
        ]);
        translate([groove_y, 8]) rotate(-tilt)
            translate([-groove_w/2, 0])
                square([groove_w, 60]);
    }
}

module stand_lowtail() {
    rotate([90, 0, 90]) linear_extrude(stand_w, center = true)
        stand_profile_2d_lowtail();
}

// stand_feet(): two short feet instead of one continuous stand_w=120
// base. The vent row spans X=[-60,60]; the device itself extends to
// X=+-92 (face_w/2). That leaves 32mm of clear width on each side
// with no vents at all, so feet placed there need no relief of any
// kind. Centred at X=+-76 (16mm clear of both the vent span and the
// device edge). Untested trade-off: the device is now gripped by two
// 24mm-wide segments 152mm apart instead of one continuous 120mm
// groove — a normal way to hold a rigid flat panel (most
// picture-frame easels work this way), but a real change in HOW it's
// held, not just a material change.
foot_w = 24;
foot_x = 76;

// First pass reused the same solid wedge stand() itself uses (just
// narrower, extruded only across foot_w) — rejected once a permanent-
// mounting screw hole was wanted through it: every valid version
// either needed a boss visibly protruding past the wedge's own
// silhouette, or a very long screw to reach the wedge's own distant
// natural boundary (~35mm through the foot alone).
//
// Second pass replaced the wedge with a closed U-channel (front wall +
// back wall + a separate connecting base wall, all as one tilted
// shape) plus two separate ramp-shaped feet aimed at its corners —
// also rejected, for being more complicated than it needed to be: a
// closed channel base and two hand-aimed ramps, when the actual job
// is just "hold the device up at the right angle."
//
// This is that simplification: one flat foot pad (a plain untilted
// rectangle, Y=0..stand_depth) with the two angled uprights rising
// straight out of its top surface — no separate closed base, no ramps
// chasing the tilted wall's own path (that chase is what went wrong
// in the second pass: hand-aimed polygons that didn't actually follow
// the walls and left gaps). The uprights extend `wall_embed` past
// their own useful length, straight down into the foot's own
// material, so the union overlaps by construction instead of by
// aiming at a calculated point. 36.7cm³/pair vs 71.7cm³ for the
// original solid-wedge version (49% less).
// ----------------------------------------------------------------
// REBUILT from scratch after a serious bug: the previous version of
// this stand built the front stop and back wall as two INDEPENDENT
// polygons, each hand-aimed at where the device's edge should be. A
// fix for one specific local symptom ("front stop barely touches the
// device") shifted ONLY the front stop's own inner face 0.94mm toward
// the back wall (`front_stop_engage`), without touching the back
// wall — which silently shrank the total clear opening between the
// two walls from groove_w=31.8mm down to 31.8-0.94=30.86mm. That's
// narrower than the device itself (slab_t=31mm) — the device
// physically could not fit in the slot. Caught only when the user
// measured the actual opening directly (30.861mm) and compared it to
// the shell thickness (31mm) — every check run on this design before
// that (manifold, watertight, vent/hole clearance, device-edge
// contact sampling, even visual renders) tested contact and
// coverage, never the raw clear width of the opening itself, so nine
// rounds of "verified" fixes never caught the one number that
// actually determines whether the device fits.
//
// The fix is a change of INVARIANT, not another local patch: the
// front wall's device-facing face and the back wall's device-facing
// face are now ALWAYS defined as u = -groove_w/2 + shift and
// u = +groove_w/2 + shift respectively, for exactly one shared
// `groove_center_shift`. Because u is the coordinate perpendicular to
// the tilt direction (a rigid rotation of the plain Y,Z frame), the
// perpendicular distance between those two faces is exactly
// |(+groove_w/2+shift) - (-groove_w/2+shift)| = groove_w, for ANY
// value of shift — the two walls can be recentred together to fix a
// contact-asymmetry complaint, but they can never again drift apart
// independently, because there is only one shift, applied to both.
// This mirrors the original stand()'s own proven approach
// (stand_profile_2d() cuts a single groove_w-wide slot through one
// solid wedge, so both faces come from the same cut) — a boolean cut
// was tried here too, but a single rectangular cut tool wide enough
// to reach the front stop's low-Z region also swept through and
// severed the flat foot's connecting base in the middle (verified by
// checking the 2D profile's contour count, which jumped from 1 to 2
// the moment the cut's reach extended that far). Explicit polygons
// sharing the same u-based invariant give the same guarantee without
// that side effect.
channel_wall_t = 3;      // wall thickness beyond the device-facing face
channel_len = 30;        // back wall height — this one does the actual gripping
front_stop_height = 10;  // grounded in the original stand()'s own "lip 13 high"
flat_foot_t = 3;          // flat foot pad thickness — see the mounting-hole
                          // clearance comment below for why this shrank from 6
embed_target_z = 0.75;   // back wall's buried bottom edge height — inside the
                          // foot (flat_foot_t=3), clear of the true floor (Z=0)

// groove_center_shift recentres BOTH walls together to close the
// small contact asymmetry a real marker check found (0.74mm front gap
// vs 0.06mm back gap — assembly()'s device-centring offset doesn't
// split groove_clr symmetrically). (0.74-0.06)/2 = 0.34mm toward the
// back. Unlike the old front_stop_engage, this shift is applied to
// BOTH walls' u-values identically, so the opening between them stays
// exactly groove_w regardless of its value — it can only recentre the
// device within the slot, never narrow the slot itself.
groove_center_shift = 0.34;

function wall_v_bottom(u) = (embed_target_z - (device_z_off - u*sin(tilt))) / cos(tilt);
function upright_pt(u, v) =
    [groove_y + u*cos(tilt) + v*sin(tilt),
     device_z_off - u*sin(tilt) + v*cos(tilt)];
function v_at_z(u, z) = (z - (device_z_off - u*sin(tilt))) / cos(tilt);

// FRONT STOP: u_front_trail (device-facing) is exactly -groove_w/2
// from the shared centreline, plus the one shared shift — never
// adjusted independently of the back wall again.
u_front_trail = -groove_w/2 + groove_center_shift;
u_front_lead  = u_front_trail - channel_wall_t;
function front_stop_corners() =
    [
        upright_pt(u_front_lead,  v_at_z(u_front_lead, 0)),
        upright_pt(u_front_lead,  v_at_z(u_front_lead, front_stop_height)),
        upright_pt(u_front_trail, v_at_z(u_front_trail, front_stop_height)),
        upright_pt(u_front_trail, v_at_z(u_front_trail, 0))
    ];
module stand_front_stop_2d() polygon(front_stop_corners());

// BACK WALL: u_back_trail (device-facing) is exactly +groove_w/2 from
// the SAME shared centreline, plus the SAME shared shift — so the
// perpendicular gap between u_front_trail and u_back_trail is
// groove_w=31.8mm always, by construction, not by two numbers
// happening to agree.
u_back_trail = groove_w/2 + groove_center_shift;
u_back_lead  = u_back_trail + channel_wall_t;
function back_upright_corners() =
    let(vb_trail = wall_v_bottom(u_back_trail), vb_lead = wall_v_bottom(u_back_lead))
    [
        upright_pt(u_back_trail, vb_trail),
        upright_pt(u_back_trail, channel_len),
        upright_pt(u_back_lead, channel_len),
        upright_pt(u_back_lead, vb_lead)
    ];
module stand_back_upright_2d() polygon(back_upright_corners());

// FILL: hugs the device's underside between the two walls.
//
// First attempt shared front_stop_corners()[2] (its own trailing-TOP
// corner, at front_stop_height=10) as the fill's front anchor, to
// guarantee a zero-gap seam by construction. It did close the seam,
// but it also raised the fill's whole surface up to Z=10 there —
// ABOVE the device's real edge height (Z~8.6) — which pokes solid
// material into space the device itself needs to occupy. Caught by a
// proper interference test: a solid proxy box matching the device's
// real slab_t/face_w/tilt, positioned via assembly()'s own transform,
// boolean-intersected against the exported stand mesh. That test is
// what should have been run on every version of this stand from the
// start — every earlier check (manifold, watertight, vent/hole
// clearance, edge-CONTACT point sampling) confirms the device touches
// the stand somewhere, none of them confirm the stand doesn't also
// block the device from getting there in the first place.
//
// Second attempt kept the fill's own top boundary at the device's real
// edge line, but patched the seam against front_stop with a separate
// GUSSET triangle computed via a linear fraction along front_stop's
// own trailing edge. That fraction (t_b) assumed the device's front
// edge Y always falls BETWEEN the trailing edge's two endpoints — but
// after this stand's u_front_trail was recentred by
// groove_center_shift, the device's real edge Y (38.5) landed 0.11mm
// PAST the trailing edge's own top corner (38.39), so the fraction
// came out to 1.051 — just past the valid [0,1] range — and the
// gusset extrapolated a spike 0.52mm above front_stop_height. Visible
// in a real render as a sliver poking above the stop, and as the stop
// itself falling 0.11mm short of the device's actual edge (its own
// corner simply didn't reach that far). Caught from a screenshot, not
// from any of the numeric checks — none of them sample "does any
// fraction-based construction go outside [0,1]".
//
// Fixed by dropping fraction-based gussets entirely in favour of
// something that can't extrapolate: the fill polygon now includes
// front_stop_corners()[3] and [2] (its FULL trailing edge, both real
// corners of a real polygon, not an interpolated point along it)
// directly as vertices, with the device's real edge point inserted
// immediately after — so the seam is always a direct, in-range
// connection between two real geometric points, never a computed
// fraction that can land outside the shape it's supposed to describe.
// The back seam uses the same technique.
//
// The back also had a second, separate problem: the visible top
// surface flattened out into a ~3mm shelf before the back wall,
// because the flat foot rectangle (a constant flat_foot_t=3 slab)
// used to run the device's FULL depth, capping the fill's own sloped
// surface wherever it dipped below Z=flat_foot_t (inevitable near the
// back, since the device's real back edge sits at Z~2.15, below
// flat_foot_t itself). Fixed by starting the flat foot rectangle at
// the back wall's own trailing edge instead of the front — the
// front_stop+fill union already reaches Z=0 across its own full span
// on its own, so the flat foot's only remaining job is the back
// region the mounting hole actually needs it for, and the visible
// surface is now the continuous slope/wall, not an added flat cap.
function device_edge_front_yz() = [groove_y - groove_w/2 + groove_clr/2, device_z_off];
function device_edge_back_yz() =
    [slab_t*cos(tilt) + (groove_y - groove_w/2 + groove_clr/2),
     -slab_t*sin(tilt) + device_z_off];
edge_fill_overlap = 0.3;   // deliberate overlap into the device's edge,
                            // same margin convention as the wall shift

// Point on the back wall's own trailing (device-facing) edge at a
// given fraction t — used ONLY with t values that are provably inside
// [0,1] by construction (unlike the old front gusset's t_b), since
// flat_foot_t always sits between the wall's own bottom (embed_target_z)
// and top (channel_len).
function back_upright_trailing_edge_pt(t) =
    let(b0 = back_upright_corners()[0], b1 = back_upright_corners()[1])
    [b0[0] + t*(b1[0]-b0[0]), b0[1] + t*(b1[1]-b0[1])];
function fill_back_pt() =
    let(b0 = back_upright_corners()[0], b1 = back_upright_corners()[1],
        t = (flat_foot_t - b0[1]) / (b1[1] - b0[1]))
    back_upright_trailing_edge_pt(t);

module stand_edge_fill_2d() {
    c3 = front_stop_corners()[3];
    c2 = front_stop_corners()[2];
    pf = device_edge_front_yz();
    pb = device_edge_back_yz();
    polygon([
        c3,
        c2,
        [pf[0], pf[1] + edge_fill_overlap],
        [pb[0], pb[1] + edge_fill_overlap],
        fill_back_pt(),
        [fill_back_pt()[0], 0],
        [c3[0], 0]
    ]);
}

// The device's real back corner (device_edge_back_yz()) is exactly
// where the fill's sloped support surface meets the back wall's
// rising face — a sharp internal (concave) corner. That's fine for an
// idealised knife-edge device corner, but any real edge radius or
// chamfer on the actual shell would hit this sharp corner before its
// two flat faces (bottom + back) are both fully seated, holding the
// device slightly proud of its intended position. Relieved with a
// small notch cut right at that corner, angled to the wall's own
// (u,v) frame — the exact same rotate(-tilt) + translate(groove_y,
// device_z_off) chain the groove cut and back wall themselves use —
// so the cutter's own edges run parallel to the real corner instead
// of crossing it at an angle (an earlier axis-aligned version left a
// diagonal sliver of the actual chamfer uncut).
//
// This angled cut is correctly POSITIONED, but cutting it straight out
// of the baseplate left too little material behind it at this corner
// (point-probed as low as ~0.5mm in one attempt to shrink the cut
// itself, which fixed the thinness but visibly failed to clear the
// full chamfer at a shallow viewing angle — confirmed from a
// screenshot). Rather than keep shrinking the relief and re-litigating
// how much chamfer it actually clears, the fix is bottom_pad_t below —
// a full extra mm of material under the ENTIRE foot (not just the
// separate flat_foot_t rectangle, which starts at fill_back_pt() and
// never actually reaches this corner's Y range in the first place —
// checked directly, fill_back_pt()[0]=69.6 vs. this corner sitting
// around Y=68.5-69.2). The relief's own size/position here is
// unchanged from the original, correctly-angled version.
back_relief_z0 = device_edge_back_yz()[1] - 0.3;  // just below the device's real corner
back_relief_z1 = flat_foot_t + 0.5;               // a bit above fill_back_pt()
back_relief_depth = 0.6;                          // how far into the wall, along u

module stand_back_relief_2d() {
    v0 = v_at_z(u_back_trail, back_relief_z0);
    v1 = v_at_z(u_back_trail, back_relief_z1);
    translate([groove_y, device_z_off]) rotate(-tilt)
        translate([u_back_trail - back_relief_depth, v0])
            square([back_relief_depth, v1 - v0]);
}

// Extra material under the ENTIRE foot (not the separate flat_foot_t
// rectangle — see stand_edge_fill_2d()'s comment on why that one
// doesn't reach this corner), added below Z=0 across the whole depth.
// Doesn't touch anything above Z=0 — front stop height, opening width,
// relief position, mounting-hole depth are all unchanged, this just
// gives the relief cut (and everything else) more material to sit on.
bottom_pad_t = 1;
module stand_bottom_pad_2d() {
    y0 = front_stop_corners()[0][0];
    translate([y0, -bottom_pad_t])
        square([stand_depth - y0, bottom_pad_t]);
}

// flat_foot_t (defined up near channel_wall_t, see its own comment)
// had to shrink for a reason unrelated to material use: the mounting
// hole's axis runs at a shallow angle, and a straight screwdriver
// approaching along that line from outside the counterbore was
// hitting the top of a 6mm-thick foot only ~11.5mm past the screw
// head — not enough clear shaft length. Checked directly: at 6mm the
// foot's top surface intersects that line at 21.5mm out from the
// shell face (counterbore ends at 10mm, leaving only 11.5mm clear);
// at 3mm that pushes out to 35.9mm (25.9mm clear), confirmed by
// point-probing the actual exported mesh along the hole's own axis
// out to 37mm — open the entire way. (That screwdriver-clearance
// concern turned out not to matter in practice — see
// corner_mount_holes() below — but the 3mm baseline it produced is
// still the right starting point for this dimension.)
//
// Starts at fill_back_pt() rather than the front of the foot — see
// the fill's own comment above for why: front_stop+fill already
// covers Z=0 across their own full span, so this rectangle's only
// remaining job is providing the mounting-hole-bearing material from
// the back wall onward.
module stand_flat_foot_and_uprights_2d() {
    difference() {
        union() {
            translate([fill_back_pt()[0], 0])
                square([stand_depth - fill_back_pt()[0], flat_foot_t]);
            stand_front_stop_2d();
            stand_back_upright_2d();
            stand_edge_fill_2d();
            stand_bottom_pad_2d();
        }
        stand_back_relief_2d();
    }
}

// Permanent-mounting screw hole through the back wall (the one on the
// device's EXTERIOR-back side, u > u_back_trail — the device's own
// face side sits toward u_front_trail), coaxial with the SAME bottom
// corner-post hole already cut through the shell.
mount_clr_len = 6;
mount_cb_depth = 4;
mount_cb_d = 6.5;      // matches the shell's own corner counterbore
mount_clr_d = 3.4;     // matches the shell's own corner clearance hole
// Screw: 6mm clearance + 4mm counterbore = 10mm through the foot +
// 6.4mm shell clearance (lid_t+lip_h) + up to 9.6mm post engagement
// (same M3x16 already used for the 4 standard corner screws) ~= 26mm.
// M3x25 close enough (~8.6mm engagement), M3x30 for full engagement.

// A previous version of this file extended the drilled cylinder
// another 25mm past the counterbore, on the theory that a screwdriver
// needs a clear approach path beyond the screw itself. Wrong on two
// counts: mount_clr_d=3.4mm is sized for the SCREW's own shank, nowhere
// near wide enough for an actual screwdriver shaft/bit to pass through
// — so the extension couldn't have done its stated job even if the
// clearance concern were real — and in practice it isn't needed at
// all (the screw is started and driven at an angle, not down a dead
// straight tunnel). Reverted to the plain 10mm hole (clearance +
// counterbore only).

module corner_mount_holes() {
    for (sx = [-1, 1])
        translate([sx * post_off, -post_off, slab_t])
            union() {
                cylinder(d = mount_clr_d, h = mount_clr_len);
                translate([0, 0, mount_clr_len])
                    cylinder(d = mount_cb_d, h = mount_cb_depth + eps);
            }
}

module stand_feet() {
    difference() {
        for (sx = [-1, 1])
            translate([sx * foot_x, 0, 0])
                rotate([90, 0, 90]) linear_extrude(foot_w, center = true)
                    stand_flat_foot_and_uprights_2d();
        translate([0, groove_y - groove_w/2 + groove_clr/2, device_z_off])
            rotate([-tilt, 0, 0]) translate([0, 0, face_w/2])
                rotate([0, 0, 180]) rotate([90, 0, 0])
                    translate([0, 0, -0]) corner_mount_holes();
    }
}

// ----------------------------------------------------------------
// assembly view (form check only — F5 preview this)
// ----------------------------------------------------------------
// See true_colors near [Part] above — dark colourway vs. a high-
// contrast preview palette (distinct hues, not just lighter versions
// of the same dark colours, so the three parts stay easy to tell
// apart while editing).
//
// The dark colourway itself was originally near-black (#20242c/
// #2a2f3a/#181b22, all L=11-20% in the same blue-grey hue) — close
// enough to identical that the whole model read as a flat silhouette
// with no visible shading at all, not just "dark". Lightened to
// L~30-45% in the same hue/relative ordering (stand darkest, shell
// lightest) — still reads as a dark, subtle colourway, but OpenSCAD's
// own shading now actually shows surface detail and part boundaries.
faceplate_color = true_colors ? "#454b58" : "#e8734d";
shell_color     = true_colors ? "#565f70" : "#4a90d9";
stand_color     = true_colors ? "#363c48" : "#7cb342";

// face_display: bright glyphs dropped into the stencil voids so a
// render can show the clock switched on. Word coordinates
// [row, col, len] and the sprites match the firmware tables
// (grid.cpp / animations.cpp, mirrored in docs/simulator.html).
// The glyphs reuse letters2d's exact placement + mirror, filling
// each void through the plate and standing 0.05 proud of the front
// face so the lit face renders cleanly over the plate.
TIME_DEMO = [[0,0,2], [0,3,2], [1,0,6], [6,0,5], [8,0,7], [8,8,4],
             [10,8,4]];  // IT IS TWENTY THREE MINUTES PAST FOUR
INVADER_A = ["..X.....X..", "...X...X...", "..XXXXXXX..", ".XX.XXX.XX.",
             "XXXXXXXXXXX", "X.XXXXXXX.X", "X.X.....X.X", "...XX.XX..."];
INVADER_B = ["..X.....X..", "X..X...X..X", "X.XXXXXXX.X", "XXX.XXX.XXX",
             ".XXXXXXXXX.", "..XXXXXXX..", "..X.....X..", ".X.......X."];
CANNON    = ["...X...", "..XXX..", "XXXXXXX", "XXXXXXX"];

// attract-march ping-pong columns, indexed by anim_t (see [Hidden]):
// the 11-wide invader has columns 0-5 free on the 16-cell grid, the
// 7-wide cannon 0-9. anim_t=2 gives the invader at col 2 / cannon at
// col 4 — the static render frame.
INV_COLS    = [0, 1, 2, 3, 4, 5, 4, 3, 2, 1];
CANNON_COLS = [6, 5, 4, 3, 2, 2, 3, 4, 5, 6];

function word_cells(segs) = [for (s = segs, k = [0 : s[2]-1]) [s[0], s[1]+k]];
function sprite_cells(spr, r0, c0) =
    [for (r = [0:len(spr)-1], c = [0:len(spr[r])-1])
        if (spr[r][c] == "X") [r0+r, c0+c]];

module lit_cells(cl, col)
    color(col)
        translate([0, 0, -0.05]) linear_extrude(t_front + 0.05)
            mirror([1, 0, 0])
                for (rc = cl)
                    translate([(rc[1] - (cells-1)/2) * pitch,
                               ((cells-1)/2 - rc[0]) * pitch])
                        glyph2d(GRID[rc[0]][rc[1]], px = font_px);

module face_lit() {
    if (face_display == "time")
        lit_cells(word_cells(TIME_DEMO), "#ffd98c");
    if (face_display == "invaders") {
        lit_cells(sprite_cells(anim_t % 2 == 0 ? INVADER_A : INVADER_B,
                               2, INV_COLS[anim_t % 10]), "#4be15f");
        lit_cells(sprite_cells(CANNON, 12, CANNON_COLS[anim_t % 10]), "#e8ecff");
    }
}

// unlit LED-stack blank: in the real device the diffuser + panel sit
// directly behind the stencil, so unlit letters read near-black — but
// assembly() doesn't model the drop-ins, leaving the voids open
// straight through to the vents/background. Fill the panel opening
// with a dark slab so renders show unlit letters the way the built
// clock does. Preview-only, like everything else in device().
panel_blank_color = "#171a20";

module device() {
    color(faceplate_color) faceplate();
    face_lit();
    color(panel_blank_color) translate([0, 0, t_front])
        linear_extrude(1) square(cells * pitch + 1, center = true);
    color(shell_color) translate([0, 0, slab_t]) rotate([180, 0, 0]) shell();
}

module assembly() {
    // Desk stand isn't part of a wall-mount build (see mount_type) —
    // skip it in the preview so "wall" doesn't show a part you
    // wouldn't actually print/fit. Doesn't affect the device's own
    // transform below: groove_y/device_z_off/tilt etc. are plain
    // parameters, not derived from stand_feet()'s rendered geometry,
    // so this is a safe, preview-only change.
    if (mount_type != "wall")
        color(stand_color) stand_feet();
    translate([0, groove_y - groove_w/2 + groove_clr/2, device_z_off])
        rotate([-tilt, 0, 0]) translate([0, 0, face_w/2])
            rotate([0, 0, 180]) rotate([90, 0, 0])
                translate([0, 0, -0]) device();
}

// ----------------------------------------------------------------
if (part == "faceplate") faceplate();
if (part == "lattice")   lattice();
if (part == "diffuser")  diffuser();
if (part == "coupon_diffuser") coupon_diffuser();
if (part == "shell")     shell();
if (part == "stand_lowtail") stand_lowtail();
if (part == "stand_feet")    stand_feet();
if (part == "coupon")    coupon();
if (part == "pod_coupon") pod_coupon();
if (part == "assembly")  assembly();
if (part == "face2d")    // quick legibility check, reads correctly in top view
    for (r = [0:cells-1], c = [0:cells-1])
        translate([(c - (cells-1)/2) * pitch, ((cells-1)/2 - r) * pitch])
            glyph2d(GRID[r][c], px = font_px);
