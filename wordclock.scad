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

VERSION = "0.1.0";
echo(str("word clock model v", VERSION));

include <font.scad>

/* [Part] */
part = "assembly"; // [assembly, faceplate, lattice, diffuser, shell, stand, pod, coupon, coupon_diffuser, face2d]

/* [LED panel] */
// LED-to-LED spacing of the matrix panel
pitch = 10;
// LEDs per side
cells = 16;
// panel PCB thickness
panel_t = 2.0;
// compressible foam behind the panel
foam_t = 3.0;

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
// Rear power pod: a shallow bump-out on the back of the shell that
// houses a SNAP-IN panel-mount USB-C power socket at the
// bottom-middle, above the stand horn. Feeds panel + ESP32 5V
// directly; the XIAO's own USB-C stays flash-only.
// Dimensions from the NinthQua CHT-TS023R-H160-P4 drawing (4P
// PD/fast-charge pigtail variant, 5A): cutout 13.6 x 6.3 R1.3,
// flange 16.7 x 10.3 x 2.0, body ~12 x 5.3 x 14 deep, snap wings
// spread to ~16 behind a ~2 mm panel.
pwr_pod = true;
// snap-in cutout in the pod face
pwr_snap_w = 14.7;
pwr_snap_h = 5.4;
pwr_snap_r = 1.3;
// pod face (= panel the socket snaps onto) thickness
pwr_face_t = 2.0;
// interior cavity: body + wing deployment + wire pass-through
// (w,h keep a similar clearance margin around the cutout as before;
// depth set so cavity + pwr_face_t = 10.0mm total pod height)
pwr_cavity = [18, 10, 8];
// pod centre height above the bottom face edge (keep pod + plug
// clear of the 34 mm stand horn)
pod_up = 48;
pod_wall = 2.4;

/* [Ventilation] */
// rear pressure-pad grid (replaces the old full-length ribs) —
// sparse contact points instead of long strips, so most of the back
// stays open for airflow. The lattice already supports the panel's
// front face at full 10 mm pitch, so these only need to take up
// stack tolerance, not provide fine flatness.
pad_off = 40;
pad_w = 8;
pad_h = 1.0;
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
// backwards rake of the face
tilt = 12; // [5:25]
stand_w = 120;
stand_depth = 92;
stand_h = 34;
// slot clearance around the slab
groove_clr = 0.8;

/* [Hidden] */
$fa = 4; $fs = 0.4;
eps = 0.01;

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
wall_top = t_front + baffle_d + panel_t + foam_t + 0.4;  // 18.6
slab_t   = wall_top + lid_t;                  // total device thickness
post_off = face_w/2 - 8.5;                    // corner post centres
lip_out  = face_w/2 - wall_t - 0.4;           // shell lip outer half-width
groove_w = slab_t + groove_clr;

// ----------------------------------------------------------------
// helpers
// ----------------------------------------------------------------
// rounded square, actual outer size = w with corner radius r. (Was
// `offset(r) offset(-r) square(w-2*r)` — the redundant second offset
// eroded the dilation straight back off, silently delivering a
// w-2*r actual size for every rsq()'d part; see README fit note.)
module rsq(w, r = corner_r) offset(r) square(w - 2*r, center = true);

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

    // bezel border ring down to the panel plane; the separate
    // lattice part drops into the opening
    translate([0, 0, t_front]) linear_extrude(baffle_d)
        difference() { rsq(fw); square(op, center = true); }

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
        translate([0, 0, t_front]) linear_extrude(wall_top - t_front)
            difference() { rsq(fw); rsq(fw - 2*wall_t, corner_r - 1); }
        // corner screw posts
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx*post_off, sy*post_off, t_front])
                difference() {
                    cylinder(d = post_d, h = wall_top - t_front);
                    translate([0, 0, wall_top - t_front - 10])
                        cylinder(d = post_hole_d, h = 10 + eps);
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
// rear shell: flat lid, inner lip, foam-pressure ribs,
// ESP32 (XIAO) pocket, cable exit
// ----------------------------------------------------------------
module shell() {
    difference() {
        union() {
            linear_extrude(lid_t) rsq(face_w);
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
        if (pwr_pod) {
            // opening under the pod (wires pass straight through)
            translate([0, face_w/2 - pod_up, -eps]) linear_extrude(lid_t + 2)
                offset(2) offset(-2)
                    square([pwr_cavity[0], pwr_cavity[1]], center = true);
            // pod ear screw holes (M3 from the inside, into the pod)
            for (s = [-1, 1])
                translate([s*(pwr_cavity[0]/2 + pod_wall + 6),
                           face_w/2 - pod_up, -eps])
                    cylinder(d = 3.4, h = lid_t + 2);
        }
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
// rear power pod: houses the USB-C power socket, screws onto the
// back of the shell over the matching cutout. Printed as modelled
// (socket face on the bed, open flange up — no supports).
// ----------------------------------------------------------------
module pod() {
    cw = pwr_cavity[0];
    ch = pwr_cavity[1];
    cd = pwr_cavity[2];
    oh = cd + pwr_face_t;            // overall height (face on the bed)
    ear_off = cw/2 + pod_wall + 6;   // matches shell ear holes
    difference() {
        union() {
            linear_extrude(oh) offset(3) offset(-3)
                square([cw + 2*pod_wall, ch + 2*pod_wall], center = true);
            // ears the shell screws into (flush with the open rim)
            for (s = [-1, 1]) translate([s*ear_off, 0, oh - 4])
                linear_extrude(4) offset(2) offset(-2)
                    square([12, 10], center = true);
        }
        // cavity (socket body + snap wings + wires straight through)
        translate([0, 0, pwr_face_t]) linear_extrude(oh)
            square([cw, ch], center = true);
        // snap-in rectangular cutout in the face, R-corners per drawing
        translate([0, 0, -eps]) linear_extrude(pwr_face_t + 2*eps)
            offset(pwr_snap_r) offset(-pwr_snap_r)
                square([pwr_snap_w, pwr_snap_h], center = true);
        // ear screw pilots (M3 self-tap)
        for (s = [-1, 1]) translate([s*ear_off, 0, oh - 4 - eps])
            cylinder(d = 2.7, h = 4 + 2*eps);
    }
}

// ----------------------------------------------------------------
// desk stand: raked slot in a wedge block
// ----------------------------------------------------------------
module stand() {
    groove_y = 40;   // slot centre-line at the top face
    difference() {
        // low lip in front (must not cover the bottom letter row:
        // lip 13 high ⇒ ~9 mm of slab hidden < 12 mm bezel), tall
        // support horn behind where the slab leans on it
        rotate([90, 0, 90]) linear_extrude(stand_w, center = true)
            polygon([
                [0, 0], [stand_depth, 0],
                [stand_depth, 8], [66, stand_h],
                [42, stand_h], [32, 13], [24, 13], [0, 5]
            ]);
        translate([0, groove_y, 4]) rotate([-tilt, 0, 0])
            translate([-stand_w/2 - 5, -groove_w/2, 0])
                cube([stand_w + 10, groove_w, 60]);
    }
}

// ----------------------------------------------------------------
// assembly view (form check only — F5 preview this)
// ----------------------------------------------------------------
module device() {
    color("#20242c") faceplate();
    color("#2a2f3a") translate([0, 0, slab_t]) rotate([180, 0, 0]) shell();
    if (pwr_pod)
        color("#2a2f3a") translate([0, -face_w/2 + pod_up, slab_t])
            rotate([0, 180, 0])  // open rim against the lid, face outward
            translate([0, 0, -(pwr_cavity[2] + pwr_face_t)]) pod();
}

module assembly() {
    color("#181b22") stand();
    translate([0, 40 - groove_w/2 + groove_clr/2, 4.6])
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
if (part == "stand")     stand();
if (part == "pod")       pod();
if (part == "coupon")    coupon();
if (part == "assembly")  assembly();
if (part == "face2d")    // quick legibility check, reads correctly in top view
    for (r = [0:cells-1], c = [0:cells-1])
        translate([(c - (cells-1)/2) * pitch, ((cells-1)/2 - r) * pitch])
            glyph2d(GRID[r][c], px = font_px);
