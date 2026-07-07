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

Socket: **snap-in pigtail USB-C female, 4P PD/fast-charge variant**
(NinthQua CHT-TS023R-H160-P4 style — engineering drawing in Carl's
Downloads). The pod face is its mounting panel: 13.6 × 6.3 mm R1.3
cutout in a 2.0 mm face; the socket snaps in from the outside, body
and wires pass straight through into the enclosure. Rated 5 A.

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
- Pod dimensions are taken from the CHT-TS023R drawing; if your
  delivered part differs, adjust `pwr_snap_w/h/r`, `pwr_face_t`,
  `pwr_cavity` in the Customizer.

Supporting cast, standard WS2812 practice: **1000 µF electrolytic**
across 5 V/GND at the panel pigtail (also stops the ESP32 browning
out on frame spikes — it shares the rail), **330 Ω** in the data line,
short 20 AWG power wires. Data from a GPIO to DIN, short lead.

The back panel is deliberately clean; if you ever outgrow USB-C power
entirely, add a cable exit in `shell()` and feed the pigtail from a
dedicated 5 V PSU.

## Printed parts

| Part | File | Orientation | Colour |
|---|---|---|---|
| Faceplate + baffle | `part="faceplate"` | **as modelled** (letters on the bed) | dark → white → dark, see below |
| Rear shell | `part="shell"` | outer face on bed | dark |
| Desk stand | `part="stand"` | flat base on bed | dark |
| Power pod | `part="pod"` | socket face on bed | dark |
| Test coupon | `part="coupon"` | letters on bed | same swaps as faceplate |

Export: `openscad -o build/<part>.stl -D 'part="<part>"' wordclock.scad`
(STLs are not committed — regenerate from source.)

### Faceplate colour swaps (single-extruder filament change)

The plate prints **letters-down**; the STL is already in print
orientation, no flipping. Heights measured from the bed:

- **0 – 1.0 mm** dark (the letter stencil — letters are voids)
- **1.0 – 1.6 mm** white/natural (the diffuser the LEDs glow through)
- **1.6 mm – end** dark (baffle lattice + tray body)

At 0.2 mm layers that is: swap to white after layer 5, swap back to
dark after layer 8. Set the two filament changes in your slicer and
check its layer preview shows the letters closing over at the first
swap. **Print the coupon first** — it is a 48 mm four-cell sample that
validates the colour-change heights, letter legibility, and that the
stencil-bridge font anchors cleanly on the bed.

The font is a purpose-made 5×7 pixel stencil face (`font.scad`):
letters with enclosed counters (A B D O P Q R) carry a bridge, so no
first-layer island of plate material sits loose on the bed.

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

1.0 stencil + 0.6 diffuser + 12 baffle lattice (1.2 mm walls landing
between LEDs) → panel (2.0) → foam (3.0) → shell ribs press it all
against the baffle. Shell lip registers inside the tray wall, four M3
self-tappers into the corner posts. Cable exits an oval in the shell
30 mm above the bottom edge — just above the stand horn, hidden behind
the slab. ESP32 sits in a fenced pocket on the shell.

Face is 184×184 mm, slab 21.4 mm thick, 12° rake in the stand.

## Firmware (next phase)

ESP32-S3, NTP + web UI (animation pick/interval, colours, brightness
schedule, timezone), transition animations (Pac-Man eat, Matrix rain,
Tetris drop, invader zap), wandering-Pac ambient mode, attract mode,
hidden-word easter eggs. MQTT/Home Assistant notification words are a
possible v2. The artifact linked above is the behavioural spec — the
grid, word logic and animations there are what the firmware should do.
