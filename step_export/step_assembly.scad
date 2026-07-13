// Wrapper for STEP export via FreeCAD's importCSG: identical geometry,
// but vent_slot() is redefined without hull() (importCSG can't convert
// hull to BRep and would fall back to a mesh). A stadium is exactly a
// centred rectangle plus the two end circles, so this is equivalent.
include <../../wordclock.scad>

module vent_slot() {
    square([vent_slot_w, vent_slot_l - vent_slot_w], center = true);
    for (s = [-1, 1])
        translate([0, s * (vent_slot_l - vent_slot_w)/2]) circle(d = vent_slot_w);
}

// letters2d() without mirror() (also unconvertible) and without the
// ~20 overlapping pixel squares per glyph (OCC fails fusing all ~2400
// of them in the faceplate's single 2D difference). Each glyph is one
// precomputed outline polygon (letter_polys.scad, generated from
// font.scad at px=1.1 bleed=0.12 — exactly the union glyph2d() draws).
// The x-mirror is folded in by negating x coordinates:
// mirror(x) . translate([tx,ty]) == translate([-tx,ty]) . mirror(x).
include <letter_polys.scad>

module glyph_poly_flipped(ch) {
    hit = search([ch], LETTER_POLYS);
    if (ch != " " && hit != [] && hit[0] != [])
        for (lp = LETTER_POLYS[hit[0]][1])
            polygon([for (p = lp) [-p[0], p[1]]]);
}

module letters2d(n, grid) {
    for (r = [0:n-1], c = [0:n-1])
        translate([-(c - (n-1)/2) * pitch, ((n-1)/2 - r) * pitch])
            glyph_poly_flipped(grid[r][c]);
}
