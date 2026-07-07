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
| 5 V / 3 A USB-C supply | powers everything through the side port; see power budget below |
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
   `FastLED.setMaxPowerInVoltsAndMilliamps(5, 1200)` scales each
   frame so its computed power never exceeds the budget. Sparse word
   frames render at full set brightness; dense attract-mode frames
   dim themselves automatically. This is the guarantee — no animation
   we ever add can exceed the budget.
2. **Bypass the VBUS diode for headroom.** Measure USB VBUS vs the
   5 V pin: a ~0.3 V drop means a Schottky (SS14-class, 1–2 A) is in
   the path (some clones have none). If present, solder the panel's
   5 V feed to the USB side of it (or jumper across). The budget can
   then rise to ~2–2.5 A with a 5 V/3 A USB-C supply.

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
