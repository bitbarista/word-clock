# TIME INVADERS — arcade word clock

A 16×16 word clock with an 8-bit arcade heart: the time reads as words
to the exact minute (`IT IS TWENTY THREE MINUTES PAST FOUR`), but time
changes are game events —
Pac-Man eats the old time, Matrix rain resolves into the new one — and
the filler letters hide arcade words (INSERT COIN, GAME OVER, HIGH
SCORE, plus a bottom-rows hall of fame: SPACE INVADERS, GALAGA,
DONKEY KONG, ASTEROIDS, Q*BERT). Enclosure is the "subtle" concept: a plain dark slab in a
raked desk wedge, so it passes at a work desk; the arcade lives in the
light, not the shell.

Interactive concept render (live simulation of face + animations):
<https://claude.ai/code/artifact/36cffa93-97e1-411f-91b9-300dda619f61>

**Status: v0.1.0 — model validated (manifold) but NOT yet test-printed.
Print the coupon before committing to the 184 mm faceplate. Firmware
not started.**

## Hardware (BOM)

| Item | Notes |
|---|---|
| 16×16 WS2812B flexible panel | 160×160 mm, 10 mm pitch — the common ~£12–15 one |
| Seeed XIAO ESP32S3 | WiFi NTP + web UI + animations; swapped from a generic "supermini" after none of three boards (two supermini vendors + this one) fit the cradle — the XIAO's dimensions are measured and tightly controlled |
| 5 V / 3 A USB-C supply | into the rear power breakout; see power budget below |
| Snap-in USB-C power socket, 4P PD pigtail | CHT-TS023R style, 5 A; 2P variant won't work with C-to-C cables |
| 2× 5.1 kΩ resistors | CC1/CC2 → GND if the socket's CC wires are unterminated |
| 1000 µF electrolytic (≥6.3 V) | across 5 V/GND at the panel pigtail |
| 330 Ω resistor | in the data line to DIN |
| 3 mm foam sheet, cut into 8× 8×8 mm pads (not a full sheet) | behind the panel, pressed by the shell's pad grid — see Ventilation below for why it's pads, not a sheet |
| 4× M3×8 self-tapping screws | shell → faceplate corner posts |
| Level shifter (optional) | 3.3 V data usually drives WS2812B fine at 5 V; add 74AHCT125 if flaky |
| Schottky diode, 1N5817 (on hand — 1N5822/1N5819 also fine) | pod 5 V → ESP32 5 V pin, cathode toward the ESP32 — the XIAO likely has no onboard VBUS diode, so without this, don't plug in the pod and a flashing cable at the same time during any future case-open debugging session (routine use never exposes the port — see Power & access). Must be Schottky (low forward drop): ordinary rectifiers (1N4007/5399/5408) work but sag the ESP32's 5 V more under WiFi TX current spikes; fast-recovery types (FR107/FR207) and small-signal diodes (1N4148) are unsuitable here. 1N5817 is only 1 A vs the others' 3 A, but D1 carries just the ESP32's own draw (panel is fed direct, bypassing D1), so that's plenty of margin |

### Power & access

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
`STLs/faceplate.stl` is current (plain closed tray wall); an
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

### Power budget & protection

The numbers (WS2812B ≈ 60 mA/LED full white):

| Frame | @30 % brightness |
|---|---|
| Word display, worst wording (~32 cells white) | ~0.6 A |
| Full-face sprite (~130 cells, coloured) | ~1.2–2 A |
| All 256 white (the trap) | ~4.6 A |

A global brightness cap therefore does NOT guarantee a safe current
for arbitrary animation frames. Two-layer solution:

1. **Per-frame power limiting in firmware — mandatory.**
   `FastLED.setMaxPowerInVoltsAndMilliamps(5, 2500)` scales each
   frame so its computed power never exceeds the budget. Sparse word
   frames render at full set brightness; dense attract-mode frames
   dim themselves automatically. This is the guarantee — no animation
   we ever add can exceed the budget (and it doubles as brown-out
   protection for the shared 5 V rail).
2. **Separate power inlet — panel current never touches the dev
   board.** A panel-mount USB-C power socket (pigtail type) sits in
   a small **pod** on the back of the shell — bottom-middle, above
   the stand horn, invisible in use. The socket body is too deep to
   live inside the slab (the interior is all panel + foam), so the
   pod bump-out provides its depth; it's a separate 10-minute print
   that screws onto the lid from the inside over a matching cutout.
   Its 5 V pigtail feeds the panel pigtail AND the ESP32 5 V pin
   directly; grounds common (wires just lie in the open cavity between
   the foam pads — see Ventilation, no channel-cutting needed). The
   ESP32's own USB-C — now in an internal cradle above the pod, not a
   side-wall port (see Power & access) — is bench-flash-only pre-
   assembly, then OTA.

   **⚠ The XIAO ESP32S3 does not have the onboard VBUS Schottky diode
   that some "supermini" boards use to isolate their two USB/power
   inputs** (unconfirmed against Seeed's schematic, but assume not
   present). Without D1, the ESP32's own USB-C and the pod's 5 V feed
   are directly tied together on the same rail with nothing stopping
   the pod's 5 V from backfeeding into a laptop's USB port if both are
   connected at once.

   This isn't really an *accidental*-connection risk anymore — the
   port is buried in an internal cradle, so reaching it at all means
   deliberately opening the case, which never happens during normal
   use. But that's exactly the situation where it stays relevant:
   **future firmware debugging** (serial monitoring, re-flashing)
   naturally wants the panel running normally off the pod *while* a
   laptop is also connected to the now-exposed port — same backfeed
   hazard, just reached through a deliberate step instead of an
   accidental one. D1 is a $0.20 part already on hand and already in
   the schematic, so there's no reason to remove it on the strength of
   routine access going away — **don't plug both in at the same time
   during any future case-open debugging session** unless D1 is fitted
   (it should be — see the schematic below). Different failure mode
   than the firmware USB-power-guard in `usbguard.h`, which only
   throttles the *other* direction (a host over-powering the panel)
   and can't protect a laptop from rail backfeed.

Socket: **snap-in pigtail USB-C female, 4P PD/fast-charge variant**.
The pod face is its mounting panel: **14.7 × 5.4 mm R1.3 cutout in a
2.0 mm face** (measured off the actual delivered part, long side
trimmed 0.3 mm after a test fit — supersedes the earlier CHT-TS023R
drawing estimate), its long axis aligned with
the two mounting ears; the socket snaps in from the outside, body and
wires pass straight through into the enclosure. Total pod height is
10.0 mm.

- **Buy the 4P fast-charge/PD pinout (V+, V−, CC1, CC2) — not the
  2P, and not the 4P "data" pinout (D+/D−).** Without CC pins a
  compliant USB-C supply never enables VBUS on a C-to-C cable.
- The CC wires connect to **nothing downstream** — their only job is
  the supply handshake. Solder **5.1 kΩ from CC1 to GND and 5.1 kΩ
  from CC2 to GND** (one each, not shared) right at the pigtail,
  heat-shrink, tuck away. This requests 5 V/3 A. If the delivered
  socket has the resistors built in (listing says "with CC resistor",
  or only V+/V− emerge), skip this — VBUS and GND are then the only
  connections.
- **It must present 5 V.** These passthrough sockets don't negotiate
  voltage themselves, but verify with a multimeter before first
  connection to the panel — 9/12/20 V kills WS2812s instantly.
- Pod dimensions (`pwr_snap_w/h/r`, `pwr_face_t`, `pwr_cavity`) are
  Customizer parameters — adjust them if a future socket differs.

Supporting cast, standard WS2812 practice: **1000 µF electrolytic**
across 5 V/GND at the panel pigtail (also stops the ESP32 browning
out on frame spikes — it shares the rail), **330 Ω** in the data line,
short 20 AWG power wires. Data from a GPIO to DIN, short lead.

The back panel is deliberately clean; if you ever outgrow USB-C power
entirely, add a cable exit in `shell()` and feed the pigtail from a
dedicated 5 V PSU.

### Complete Circuit Schematic

```
+=========================================================================+
|                   TIME INVADERS -- Circuit Schematic                    |
+=========================================================================+

POWER

  USB-C Power Pod (snap-in, 4P fast-charge/PD pigtail)
  +-------------+
  |  V+         |----------------------------------------------- 5V RAIL
  |  V-         |----------------------------------------------- GND RAIL
  |  CC1        |--[ R1 5.1k ]--+
  |  CC2        |--[ R2 5.1k ]--+-------------------------------- GND RAIL
  +-------------+

  R1/R2 request 5V/3A from the supply. Omit them if the socket
  already has CC resistors built in (only V+/V- emerge in that case).

  ESP32-S3 SuperMini
  5V RAIL  --[ D1 ]------------------------------------------------ 5V pin
  GND RAIL ------------------------------------------------------ GND pin

  D1: pod 5V -> ESP32 5V pin, cathode (banded end) toward the ESP32.
  Reinstates the isolation the XIAO's own USB port likely lacks (see
  note below) -- without it, don't power from the pod and a flashing
  cable at once.

  On hand: 1N5817 -- genuine Schottky (same 1N581x family as the
  1N5822/1N5819 the BOM recommends), 20V/1A, VF ~0.2-0.45V. Only
  difference from the BOM pick is current rating (1A vs 3A), which is
  fine here: D1 only carries the ESP32's own draw, not the panel
  (panel is fed V+/V- direct, bypassing D1) -- 1A comfortably covers
  the XIAO even during WiFi TX current spikes.


  ESP32-S3's OWN programming USB-C (internal cradle, bench-flash only
  pre-assembly -- OTA thereafter, see Power & access)

    USB-C connector
     VBUS ----------[ onboard Schottky ]---------------------- 5V RAIL
     GND  ------------------------------------------------------ GND RAIL

  This diode blocks GND RAIL -> VBUS backfeed, but NOT the reverse:
  if a laptop's VBUS sits above the pod's output, the laptop can end
  up sourcing panel current through this small onboard diode. The
  firmware's USB power guard detects a host on this port (via USB
  enumeration) and automatically clamps LED current to a safe budget
  while it's connected -- no extra wiring, but don't rely on the
  diode alone.


LED PANEL (16x16 WS2812B, 160x160mm, 256 LEDs)

  5V RAIL  --+----------------------------------------------------- Panel VCC
             |
         [ C1 1000uF ]
             |
  GND RAIL --+----------------------------------------------------- Panel GND

  ESP32-S3                                            Panel
  GPIO4 -----[ R3 330R ]----------------------------- DIN

  GPIO4 is a build flag (LED_PIN in platformio.ini) -- change there,
  not here, if your wiring differs. Optional: insert a 74AHCT125
  level shifter between R3 and DIN if 3.3V data proves unreliable
  driving the panel from 5V.


All GND labels share a common ground.
All 5V RAIL labels are the same net: fed primarily by the power pod
(V+/V- direct, no diode), and -- only while a host is enumerated on
the programming port -- additionally by that port's VBUS through the
onboard diode (current-limited in firmware when this happens).
```

## Printed parts

Every part is a plain single-colour print — no filament swaps.

| Part | File | Orientation | Colour |
|---|---|---|---|
| Faceplate (stencil + tray) | `part="faceplate"` | letters on the bed | dark |
| Baffle lattice (drop-in) | `part="lattice"` | flat on bed | dark |
| Diffuser sheet (drop-in) | `part="diffuser"` | flat on bed, 100 % infill | white or clear — experiment |
| Rear shell | `part="shell"` | outer face on bed | dark |
| Desk stand | `part="stand"` | flat base on bed | dark |
| Power pod | `part="pod"` | socket face on bed | dark |
| Test coupon | `part="coupon"` | letters on bed | dark |
| Coupon diffuser strips | `part="coupon_diffuser"` | flat, 100 % infill | one per candidate |

Export: `openscad -o build/<part>.stl -D 'part="<part>"' wordclock.scad`
(STLs are not committed — regenerate from source.)

### The face stack & the diffuser experiment

The faceplate is a 1.2 mm dark stencil (letters are through-voids)
with the bezel ring and tray behind it. The **diffuser is a separate
sheet** that drops into the opening against the stencil's back, and
the **lattice drops in after it**, pressing it flat; the panel, foam
and shell clamp the whole stack. The diffuser is therefore swappable
forever — four shell screws.

Finding the right diffuser is the point of the **coupon**: a 48 mm
four-cell sample with a slide-in slot along one edge. Print
`coupon_diffuser` strips at several thicknesses (set `t_diff` to 0.6 /
0.9 / 1.2) in clear and white, slide each into the slot over a lit
LED ~12 mm behind, and judge hotspots and glow. Clear PETG frosts
usefully at 4–6 layers; white diffuses more per layer but costs
brightness. When you've picked, set `t_diff` to the winner and print
the full-size `diffuser` — the lattice depth derives from `t_diff`,
so print the lattice after the decision.

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

The font is a purpose-made 5×7 pixel stencil face (`font.scad`):
letters with enclosed counters (A B D O P Q R) carry a bridge, so the
stencil plate is self-supporting and nothing floats on the bed.

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

## The letter grid (per-minute)

```
IT K IS A HIGHSCORE Z         rows 0-8: minute words
TWENTY INSERTCOIN             rows 9-12: hour words + OCLOCK
FOURTEEN SIXTEEN A            rows 13-15: arcade hall of fame
SEVENTEEN TWELVE B
EIGHTEEN NINETEEN             hidden arcade words:
THIRTEEN QUARTER S            HIGH SCORE, INSERT COIN,
THREELEVEN TEN ZAP            ZAP/PAC/POW (stacked), GO,
TWONE FIVE HALF PAC           GAME/OVER (stacked),
MINUTES X PASTO POW           SPACE INVADERS, PEW, GALAGA,
TWONE THREEIGHT GO            DONKEY KONG, ASTEROIDS, QBERT, UP
SEVENINE FOUR FIVE
SIX TEN ELEVEN GAME
TWELVE OCLOCK OVER
SPACEINVADERS PEW
GALAGA DONKEYKONG
ASTEROIDS QBERT UP
```

QBERT, not Q*BERT — the arcade game's actual title has an asterisk
(`font.scad` has no glyph for one, so the grid spells it without).
Kept as "Q*BERT" in prose elsewhere in this doc since that's the real
name; only the on-grid spelling drops the asterisk.

Per-minute wording fits 16×16 through letter-sharing: FOURTEEN,
SIXTEEN, SEVENTEEN, EIGHTEEN, NINETEEN carry FOUR, SIX, SEVEN, EIGHT,
NINE as prefixes; TWONE = TWO+ONE, THREELEVEN = THREE+ELEVEN,
THREEIGHT = THREE+EIGHT, SEVENINE = SEVEN+NINE, PASTO = PAST+TO, and
MINUTE is a prefix of MINUTES. Grammar: `ONE MINUTE`, `A QUARTER` /
`HALF` (no MINUTES), 21–29 = TWENTY + unit, and past 30 minutes it
counts down TO the next hour. The X in row 8 is a deliberate spacer:
MINUTES PAST is lit 48 minutes of every hour, so it earns a gap
(TWELVE OCLOCK stays adjacent — it shows for one minute twice a day). The stand's front lip is deliberately
low (13 mm) so the bottom hall-of-fame row stays visible above it.

Since words now change every minute, firmware should run the big
game transitions on 5-minute boundaries (configurable) and a quick
crossfade on ordinary minute ticks — Pac-Man every 60 s would wear
thin fast.

## Enclosure stack (front → back)

1.2 stencil plate → drop-in diffuser sheet (t_diff) → drop-in baffle
lattice (1.2 mm walls landing between LEDs, depth = 12 − t_diff) →
panel (2.0) → foam pads (3.0) → shell's pad grid presses the whole
stack against the stencil. Shell lip registers inside the tray wall,
four M3 self-tappers into the corner posts. Power enters the rear
pod; the ESP32 sits in an internal cradle above the pod, flashed via
USB on the bench pre-assembly and OTA thereafter (see Power & access).

Face is 184×184 mm, slab ~21 mm thick, 12° rake in the stand.

Assembly order: faceplate letters-down on the desk → diffuser sheet
into the opening → lattice on top of it → panel (data pigtail routed
toward the ESP32's internal cradle above the pod) → foam pads at the
12 grid points (see below — wires just lie in the open cavity between
them, no channel-cutting needed anymore) → shell with ESP32 and pod
wiring attached → four screws.

### Ventilation (v0.1.1 — runs 24/7, so this isn't optional)

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

## Firmware (next phase)

ESP32-S3, NTP + web UI (animation pick/interval, colours, brightness
schedule, timezone), transition animations (Pac-Man eat, Matrix rain,
Tetris drop, invader zap), wandering-Pac ambient mode, attract mode,
hidden-word easter eggs. MQTT/Home Assistant notification words are a
possible v2. The artifact linked above is the behavioural spec — the
grid, word logic and animations there are what the firmware should do.
