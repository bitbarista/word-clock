# TIME INVADERS — arcade word clock

A 16×16 word clock with an 8-bit arcade heart: the time reads as words
(`IT IS TWENTY FIVE PAST TEN`), but time changes are game events —
Pac-Man eats the old time, Matrix rain resolves into the new one — and
the filler letters hide arcade words (INSERT COIN, GAME OVER, HIGH
SCORE, PLAYER ONE, plus a bottom-rows hall of fame ending in THE
MATRIX). Enclosure is the "subtle" concept: a plain dark slab in a
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
| 5 V / 4 A PSU | firmware caps brightness; full-white would need 15 A, we never go there |
| 3 mm foam sheet | behind the panel, pressed by the shell ribs |
| 4× M3×8 self-tapping screws | shell → faceplate corner posts |
| Level shifter (optional) | 3.3 V data usually drives WS2812B fine at 5 V; add 74AHCT125 if flaky |

### Power & access

The ESP32 sits in a corner cradle with its **onboard USB-C protruding
through the side wall** (right side viewed from the front; `usb_side`
flips it) — so you can flash/update firmware without opening the case,
and a single USB-C lead can power the clock for normal use: the word
display lights ~20–30 LEDs and stays well under 1 A. Note the panel's
current then flows through the supermini's VBUS diode, so firmware
must cap global brightness (~30 %) and attract-mode/full-face
animations especially.

If you ever want to run brighter, wire the PSU 5 V/GND directly to
the panel pigtail and the ESP32 5 V pin — the back panel is clean by
design, so add a cable exit in `shell()` first (or use a right-angle
USB-C lead and a beefier supply, accepting the diode limit). Data from
a GPIO to DIN, short lead, either way.

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

## The letter grid

```
A IT K IS HIGHSCORE S        time words     hidden arcade words
INSERTCOIN READY B           ─────────      ──────────────────
A TWENTY FIVE PLAY C         IT IS          HIGH SCORE, READY
QUARTER BONUS HALF           TWENTY FIVE    INSERT COIN, PLAY
TEN D GAMEOVER PAST          QUARTER HALF   BONUS, GAME OVER
TO F LEVELUP CREDIT          TEN PAST TO    LEVEL UP, CREDIT
ONE TWO THREE GHOST          ONE..TWELVE    GHOST, STAR, WAKA
FOUR FIVE SIX SEVEN          OCLOCK         PACMAN, HUNT, CHERRY
EIGHT NINE TEN STAR                         PLAYER ONE
ELEVEN TWELVE WAKA                          SPACE INVADERS, PEW
OCLOCK PACMAN HUNT                          GALAGA, DONKEY KONG
PLAYERONE CHERRY Z                          ASTEROIDS, FROGGER
SPACEINVADERS PEW                           THE MATRIX, QBERT, UP
GALAGA DONKEYKONG
ASTEROIDS FROGGER
THEMATRIX QBERT UP
```

Time resolves to 5 minutes; the four corner LEDs add +1..+4 minutes
(QLOCKTWO style). The stand's front lip is deliberately low (13 mm) so
the bottom hall-of-fame row stays visible above it.

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
