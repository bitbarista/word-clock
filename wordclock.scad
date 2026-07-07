// =================================================================
// TIME INVADERS — 16x16 word clock, subtle desk-wedge enclosure
// -----------------------------------------------------------------
// Shell concept "B — Subtle": plain slab in a raked desk stand.
// Face prints LETTERS-DOWN with two filament swaps:
//     0.0 – 1.0 mm  dark   (letter stencil layer)
//     1.0 – 1.6 mm  white  (diffuser the LEDs glow through)
//     1.6 mm – end  dark   (baffle lattice + tray body)
// Electronics: 16x16 WS2812B flexible panel (160x160, 10 mm pitch)
// + ESP32-S3 supermini.  See README.md for the print/assembly guide.
// =================================================================

VERSION = "0.1.0";
echo(str("word clock model v", VERSION));

include <font.scad>

/* [Part] */
part = "assembly"; // [assembly, faceplate, shell, stand, pod, coupon, face2d]

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
// dark stencil layer in front of the diffuser (print: swap to white here)
t_front = 1.0;
// white diffuser layer (print: swap back to dark after this)
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

/* [Body] */
// tray perimeter wall thickness
wall_t = 2.4;
// rear shell lid thickness
lid_t = 2.4;
// shell lip engagement depth
lip_h = 4;
// M3 self-tap pilot in the corner posts
post_hole_d = 2.7;

/* [Electronics] */
// side the USB-C exits, viewed from the FRONT (model x is mirrored)
usb_side = -1; // [-1:right, 1:left]
// height of the USB-C/pocket centre above the bottom face edge
usb_up = 28;
// ESP32 supermini board size incl. fitting tolerance
esp_l = 22.8;
esp_w = 18.4;
// Rear power pod: a shallow bump-out on the back of the shell that
// houses a panel-mount USB-C power socket (pigtail type) at the
// bottom-middle, above the stand horn. Feeds panel + ESP32 5V
// directly; supermini USB-C stays flash-only. MEASURE YOUR SOCKET
// before printing — defaults are guesses.
pwr_pod = true;
// socket housing width x height x depth
pwr_body = [16, 16, 14];
// round mounting hole in the pod face (socket's threaded collar/nose)
pwr_face_hole_d = 13;
// pod centre height above the bottom face edge (keep pod + plug
// clear of the 34 mm stand horn)
pod_up = 48;
pod_wall = 2.4;

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
t_face   = t_front + t_diff;                  // solid face thickness
face_w   = cells * pitch + 2 * bezel;         // 184
wall_top = t_face + baffle_d + panel_t + foam_t + 0.4;  // 19.0
slab_t   = wall_top + lid_t;                  // total device thickness
post_off = face_w/2 - 8.5;                    // corner post centres
lip_out  = face_w/2 - wall_t - 0.4;           // shell lip outer half-width
groove_w = slab_t + groove_clr;

// ----------------------------------------------------------------
// helpers
// ----------------------------------------------------------------
module rsq(w, r = corner_r) offset(r) offset(-r) square(w - 2*r, center = true);

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

    // stencil layer (dark on the bed)
    linear_extrude(t_front)
        difference() { rsq(fw); letters2d(n, grid); }

    // diffuser layer (white)
    translate([0, 0, t_front]) linear_extrude(t_diff) rsq(fw);

    // bezel ring + per-cell baffle lattice (dark again)
    translate([0, 0, t_face]) linear_extrude(baffle_d) {
        difference() { rsq(fw); square(op, center = true); }
        for (k = [0:n]) {
            translate([-gs/2 + k*pitch, 0]) square([lat_wall, op + 2], center = true);
            translate([0, -gs/2 + k*pitch]) square([op + 2, lat_wall], center = true);
        }
    }

    if (tray) {
        // perimeter tray wall up to the shell seat, notched so the
        // ESP32's onboard USB-C protrudes through the side (open to
        // the wall top: the connector drops in as the tray closes)
        difference() {
            translate([0, 0, t_face]) linear_extrude(wall_top - t_face)
                difference() { rsq(fw); rsq(fw - 2*wall_t, corner_r - 1); }
            translate([usb_side * fw/2, -fw/2 + usb_up, wall_top])
                cube([2*wall_t + 4, 13, 13], center = true);
        }
        // corner screw posts
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx*post_off, sy*post_off, t_face])
                difference() {
                    cylinder(d = 7, h = wall_top - t_face);
                    translate([0, 0, wall_top - t_face - 10])
                        cylinder(d = post_hole_d, h = 10 + eps);
                }
    }
}

module faceplate() face_core(cells, GRID, face_w);
module coupon()    face_core(4, COUPON, 4*pitch + 8, tray = false);

// ----------------------------------------------------------------
// rear shell: flat lid, inner lip, foam-pressure ribs,
// ESP32 supermini pocket, cable exit
// ----------------------------------------------------------------
module shell() {
    difference() {
        union() {
            linear_extrude(lid_t) rsq(face_w);
            // lip that registers inside the tray wall
            translate([0, 0, lid_t]) linear_extrude(lip_h)
                difference() { rsq(2*lip_out, 2); rsq(2*lip_out - 3.6, 1.2); }
            // ribs pressing the foam/panel against the baffle
            translate([0, 0, lid_t]) {
                for (x = [-40, 40]) translate([x, 0, 0])
                    linear_extrude(1.0) square([1.6, cells*pitch], center = true);
                for (y = [-40, 40]) translate([0, y, 0])
                    linear_extrude(1.0) square([cells*pitch, 1.6], center = true);
            }
            // ESP32 supermini cradle in the bottom corner, USB-C edge
            // against the side wall. NB the lid flips onto the tray,
            // so shell y is MIRRORED vs the assembled device: device
            // bottom = model +y, and model x = device x.
            translate([usb_side * (face_w/2 - wall_t - 0.2 - esp_l/2),
                       face_w/2 - usb_up, lid_t])
                linear_extrude(5) {
                    for (s = [-1, 1]) translate([0, s * (esp_w + 1.8)/2])
                        square([esp_l, 1.8], center = true);
                    translate([-usb_side * (esp_l + 1.8)/2, 0])
                        difference() {
                            square([1.8, esp_w + 3.6], center = true);
                            square([2.6, 8], center = true); // wire gap
                        }
                }
        }
        // corner screws: through-hole + counterbore in the outside face
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx*post_off, sy*post_off, -eps]) {
                cylinder(d = 3.4, h = lid_t + lip_h + 1);
                cylinder(d = 6.5, h = 1.4);
            }
        // open the lip + relieve the lid rim where the USB-C passes
        translate([usb_side * (face_w/2 - 3.2), face_w/2 - usb_up, 7.2])
            cube([7.8, 16, 12], center = true);
        if (pwr_pod) {
            // opening under the pod (socket body + wires pass through)
            translate([0, face_w/2 - pod_up, -eps]) linear_extrude(lid_t + 2)
                offset(2) offset(-2)
                    square([pwr_body[0] + 2, pwr_body[1] + 2], center = true);
            // pod ear screw holes (M3 from the inside, into the pod)
            for (s = [-1, 1])
                translate([s*(pwr_body[0]/2 + pod_wall + 6),
                           face_w/2 - pod_up, -eps])
                    cylinder(d = 3.4, h = lid_t + 2);
        }
    }
}

// ----------------------------------------------------------------
// rear power pod: houses the USB-C power socket, screws onto the
// back of the shell over the matching cutout. Printed as modelled
// (socket face on the bed, open flange up — no supports).
// ----------------------------------------------------------------
module pod() {
    iw = pwr_body[0] + 2;            // cavity w (1 mm play each side)
    ih = pwr_body[1] + 2;
    id = pwr_body[2] + 1;            // cavity depth
    oh = id + pod_wall;              // overall height
    ear_off = iw/2 + pod_wall + 6;   // matches shell ear holes
    difference() {
        union() {
            linear_extrude(oh) offset(3) offset(-3)
                square([iw + 2*pod_wall, ih + 2*pod_wall], center = true);
            // ears the shell screws into (flush with the open rim)
            for (s = [-1, 1]) translate([s*ear_off, 0, oh - 4])
                linear_extrude(4) offset(2) offset(-2)
                    square([12, 10], center = true);
        }
        // cavity
        translate([0, 0, pod_wall]) linear_extrude(oh)
            square([iw, ih], center = true);
        // socket nose hole in the face
        translate([0, 0, -eps]) cylinder(d = pwr_face_hole_d, h = pod_wall + 2*eps);
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
            rotate([0, 180, 0])  // flange against the lid, face outward
            translate([0, 0, -(pwr_body[2] + 1 + pod_wall)]) pod();
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
if (part == "shell")     shell();
if (part == "stand")     stand();
if (part == "pod")       pod();
if (part == "coupon")    coupon();
if (part == "assembly")  assembly();
if (part == "face2d")    // quick legibility check, reads correctly in top view
    for (r = [0:cells-1], c = [0:cells-1])
        translate([(c - (cells-1)/2) * pitch, ((cells-1)/2 - r) * pitch])
            glyph2d(GRID[r][c], px = font_px);
