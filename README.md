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
| ESP32-S3 supermini | in stock; WiFi NTP + web UI + animations |
| 5 V / 3 A USB-C supply | into the rear power breakout; see power budget below |
| Snap-in USB-C power socket, 4P PD pigtail | CHT-TS023R style, 5 A; 2P variant won't work with C-to-C cables |
| 2× 5.1 kΩ resistors | CC1/CC2 → GND if the socket's CC wires are unterminated |
| 1000 µF electrolytic (≥6.3 V) | across 5 V/GND at the panel pigtail |
| 330 Ω resistor | in the data line to DIN |
| 3 mm foam sheet | behind the panel, pressed by the shell ribs |
| 4× M3×8 self-tapping screws | shell → faceplate corner posts |
| Level shifter (optional) | 3.3 V data usually drives WS2812B fine at 5 V; add 74AHCT125 if flaky |

### Power & access

The ESP32 sits in a corner cradle with its **onboard USB-C protruding
through the side wall** (right side viewed from the front; `usb_side`
flips it) — flash/update firmware without opening the case, and one
USB-C lead powers everything.

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
   directly; grounds common (route the wires through a small channel
   cut in the foam). The supermini's own side USB-C becomes
   flash/serial only — its VBUS diode now usefully isolates the two
   sources, so a laptop and the power supply can be connected at the
   same time. (If your supermini has no diode — no ~0.3 V drop
   between USB VBUS and the 5 V pin — don't plug both in at once.)

Socket: **snap-in pigtail USB-C female, 4P PD/fast-charge variant**.
The pod face is its mounting panel: **15.0 × 5.4 mm R1.3 cutout in a
2.0 mm face** (measured off the actual delivered part — supersedes
the earlier CHT-TS023R drawing estimate), its long axis aligned with
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
  5V RAIL  ------------------------------------------------------ 5V pin
  GND RAIL ------------------------------------------------------ GND pin


  ESP32-S3's OWN programming USB-C (side of case, flashing only)

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
panel (2.0) → foam (3.0) → shell ribs press the whole stack against
the stencil. Shell lip registers inside the tray wall, four M3
self-tappers into the corner posts. Power enters the rear pod; the
ESP32's USB-C pokes through the side wall for flashing.

Face is 184×184 mm, slab ~21 mm thick, 12° rake in the stand.

Assembly order: faceplate letters-down on the desk → diffuser sheet
into the opening → lattice on top of it → panel (data pigtail to the
ESP32 corner) → foam (cut a channel for the power wires) → shell with
ESP32 and pod wiring attached → four screws.

## Firmware (next phase)

ESP32-S3, NTP + web UI (animation pick/interval, colours, brightness
schedule, timezone), transition animations (Pac-Man eat, Matrix rain,
Tetris drop, invader zap), wandering-Pac ambient mode, attract mode,
hidden-word easter eggs. MQTT/Home Assistant notification words are a
possible v2. The artifact linked above is the behavioural spec — the
grid, word logic and animations there are what the firmware should do.
