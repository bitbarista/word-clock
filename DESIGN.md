# Design notes — TIME INVADERS word clock

[README.md](README.md) is the build guide; this file holds the design
rationale and current-state detail behind it. [DEVLOG.md](DEVLOG.md)
has the engineering history of how it all got this way.

## Project status

**v0.1.6 — model validated (manifold + collision-checked).**
Faceplate, shell, and stand have all been printed and iterated on
previous revisions, but v0.1.6 changed the depth budget, the
bezel/pocket wall, the power socket mount, and (to actually fit the
thicker slab) the stand's own wedge geometry (see [DEVLOG.md](DEVLOG.md)) —
**reprint the faceplate, shell, and stand** before assembling against
this revision; the old stand's slot was cut for the pre-v0.1.6 slab
thickness and won't fit. Firmware exists on the `firmware` branch (fw
v0.3.0, not yet merged to `main`), not started from scratch.

(The solid `stand` wedge and its `stand_sheet` variant discussed in
the devlog were superseded by `stand_feet`/`stand_lowtail`; the
groove/tilt/vent-relief profile work described there carried over
into both current stands.)

## Component notes (full BOM detail)

- **Seeed XIAO ESP32S3 or a generic ESP32-S3 "supermini"** — WiFi NTP
  + web UI + animations. Originally XIAO-only after none of three
  boards (two supermini vendors + the XIAO) fit an earlier snug
  cradle — since resolved (v0.1.4): the cradle is now a universal
  locator (`brd_w`/`brd_l`) sized to fit either, single end-stop only
  (see ESP32 access below).
- **Snap-in USB-C power socket, 4P PD pigtail** — CHT-TS023R style,
  5 A; 2P variant won't work with C-to-C cables. Mounts directly into
  the shell now, no separate pod part or screws (see Power budget &
  protection below).
- **1000 µF electrolytic (≥6.3 V)** — across 5 V/GND at the panel
  pigtail. **Check the physical size**: standard 1000µF caps run
  8–10mm diameter × 11.5mm+ tall (leaded or SMD, height doesn't change
  with package) — this needs the deepened back cavity (v0.1.6, see
  Enclosure stack below), it did not fit the original 3mm gap.
- **3 mm foam sheet, cut into 14× 8×8 mm pads (not a full sheet)** —
  behind the panel, pressed by the shell's pad grid — see Ventilation
  below for why it's pads, not a sheet.
- **4× M3×16 self-tapping screws** — shell → faceplate corner posts.
  **Not M3×8** — the screw has to clear 6.4mm of shell material before
  reaching the post at all; M3×8 leaves just 1.6mm of thread
  engagement, M3×16 gives ~9.6mm.
- **Level shifter (optional)** — 3.3 V data usually drives WS2812B
  fine at 5 V; add 74AHCT125 if flaky.
- **Schottky diode, 1N5817 (on hand — 1N5822/1N5819 also fine)** —
  pod 5 V → ESP32 5 V pin, cathode toward the ESP32 — the XIAO likely
  has no onboard VBUS diode, so without this, don't plug in the pod
  and a flashing cable at the same time during any future case-open
  debugging session (routine use never exposes the port — see ESP32
  access below). Must be Schottky (low forward drop): ordinary
  rectifiers (1N4007/5399/5408) work but sag the ESP32's 5 V more
  under WiFi TX current spikes; fast-recovery types (FR107/FR207) and
  small-signal diodes (1N4148) are unsuitable here. 1N5817 is only
  1 A vs the others' 3 A, but D1 carries just the ESP32's own draw
  (panel is fed direct, bypassing D1), so that's plenty of margin.

## Printed parts reference

Every part is a plain single-colour print — no filament swaps.

| Part | File | Orientation | Colour |
|---|---|---|---|
| Faceplate (stencil + tray) | `part="faceplate"` | letters on the bed | dark |
| Baffle lattice (drop-in) | `part="lattice"` | flat on bed | dark |
| Diffuser sheet (drop-in) | `part="diffuser"` | flat on bed, 100 % infill | white or clear — experiment |
| Rear shell (incl. power socket mount) | `part="shell"` | outer face on bed | dark |
| Desk stand — pair of feet (current default; `tilt` 12°, a 15° variant is also exported) | `part="stand_feet"` | flat base on bed | dark |
| Desk stand — full-width low-tail wedge (alternative to the feet) | `part="stand_lowtail"` | flat base on bed | dark |
| Test coupon | `part="coupon"` | letters on bed | dark |
| Coupon diffuser strips | `part="coupon_diffuser"` | flat, 100 % infill | one per candidate |
| Power socket coupon | `part="pod_coupon"` | flange face on bed | any — test print, not visible |

The 3MF sets contain each part as an individually selectable object.
The STEP exports are exact BRep (analytic cylinders/planes), with
`wordclock_assembly.step` carrying faceplate/shell/stand feet as named
solids in their assembled positions; regenerating them
(`./export_step.sh`) needs FreeCAD — see `step_export/`.

## ESP32 access — flash once, then OTA

The ESP32 sits in an internal locator cradle above the power socket
(`esp_x`/`esp_y`) — there is no external USB port. The locator has a
universal footprint (`brd_w`/`brd_l`) that fits both the Seeed XIAO
ESP32S3 and generic ESP32-S3 "supermini" boards, registering either
against a single end-stop wall (`esp_stop_t`); retention is the
soldered leads plus the shell closing over everything.

**Flash once over USB on the bench before final assembly, then update
OTA over WiFi.** When inserting the board, point its USB-C connector
at the open end of the locator (away from the end-stop wall) — the
geometry can't encode which end the connector is on. **Trade-off, gone
into with eyes open:** if OTA ever bricks, recovery means opening the
case (four corner screws) and re-flashing on the bench, not a quick
cable plug-in through the wall.

(The cradle went through five design revisions — corner brackets, a
clearance bug, the universal footprint, the end-stop — see
[DEVLOG.md](DEVLOG.md).)

## Power budget & protection

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
   board.** A panel-mount USB-C power socket (pigtail type) mounts
   bottom-middle, above the stand horn, invisible in use. **v0.1.6:
   mounts directly into the shell now** — no separate pod part, no
   screws. Used to be a bump-out part screwed on from the inside
   (meant removing it required separating the whole shell from the
   faceplate to reach those screws); now that the shell has real
   interior depth (see Enclosure stack), the socket's snap-fit flange
   passes through the shell's own material directly, retained purely
   by its own snap wings, same as before, just without the extra part.
   Its 5 V pigtail feeds the panel pigtail AND the ESP32 5 V pin
   directly; grounds common (wires just lie in the open cavity between
   the foam pads — see Ventilation, no channel-cutting needed). The
   ESP32's own USB-C — in an internal cradle above the pod, not a
   side-wall port (see ESP32 access) — is bench-flash-only pre-
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
   (it should be — see the schematic in the README). Different failure
   mode than the firmware USB-power-guard in `usbguard.h`, which only
   throttles the *other* direction (a host over-powering the panel)
   and can't protect a laptop from rail backfeed.

Socket: **snap-in pigtail USB-C female, 4P PD/fast-charge variant**.
Mounting cutout: **14.7 × 5.4 mm R1.3** (measured off the actual
delivered part, long side trimmed 0.3 mm after a test fit — supersedes
the earlier CHT-TS023R drawing estimate). The socket snaps directly
into the shell's own `lid_t` (2.4mm) now — the verified-working figure
from the old separate-pod part was 2.0mm exactly, so this is 0.4mm
thicker than what was actually tested; worth confirming the snap
wings still engage properly on the physical part rather than assuming
(`part="pod_coupon"` is a small standalone test piece of exactly this
local geometry — print it and check the real socket snaps in before
committing to a full shell print). Cavity behind it for body + wing
deployment is 8mm deep (that number was always fine — see
[DEVLOG.md](DEVLOG.md) for what actually needed fixing).

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
- Socket mount dimensions (`pwr_snap_w/h/r`, `pwr_cavity`) are
  Customizer parameters — adjust them if a future socket differs.

Supporting cast, standard WS2812 practice: **1000 µF electrolytic**
across 5 V/GND at the panel pigtail (also stops the ESP32 browning
out on frame spikes — it shares the rail), **330 Ω** in the data line,
short 20 AWG power wires. Data from a GPIO to DIN, short lead.

The back panel is deliberately clean; if you ever outgrow USB-C power
entirely, add a cable exit in `shell()` and feed the pigtail from a
dedicated 5 V PSU.

## The face stack & the diffuser experiment

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

The font is a purpose-made 5×7 pixel stencil face (`font.scad`):
letters with enclosed counters (A B D O P Q R) carry a bridge, so the
stencil plate is self-supporting and nothing floats on the bed.

## The letter grid — how it fits

QBERT, not Q*BERT — the arcade game's actual title has an asterisk
(`font.scad` has no glyph for one, so the grid spells it without).
Kept as "Q*BERT" in prose elsewhere since that's the real name; only
the on-grid spelling drops the asterisk.

Per-minute wording fits 16×16 through letter-sharing: FOURTEEN,
SIXTEEN, SEVENTEEN, EIGHTEEN, NINETEEN carry FOUR, SIX, SEVEN, EIGHT,
NINE as prefixes; TWONE = TWO+ONE, THREELEVEN = THREE+ELEVEN,
THREEIGHT = THREE+EIGHT, SEVENINE = SEVEN+NINE, PASTO = PAST+TO, and
MINUTE is a prefix of MINUTES. Grammar: `ONE MINUTE`, `A QUARTER` /
`HALF` (no MINUTES), 21–29 = TWENTY + unit, and past 30 minutes it
counts down TO the next hour. The X in row 8 is a deliberate spacer:
MINUTES PAST is lit 48 minutes of every hour, so it earns a gap
(TWELVE OCLOCK stays adjacent — it shows for one minute twice a day).
The stand's front lip is deliberately low (13 mm) so the bottom
hall-of-fame row stays visible above it.

Since words change every minute, firmware runs the big game
transitions on 5-minute boundaries (configurable) and a quick
crossfade on ordinary minute ticks — Pac-Man every 60 s would wear
thin fast.

## Enclosure stack (front → back)

1.2 stencil plate → drop-in diffuser sheet (t_diff) → drop-in baffle
lattice (1.2 mm walls landing between LEDs, depth = 12 − t_diff) →
panel (2.0) → back cavity (13.0, houses foam pads + all electronics)
→ shell's pad grid presses the whole stack against the stencil. Shell
lip registers inside the tray wall, four M3×16 self-tappers into the
corner posts. Power enters via a socket mounted directly into the
shell; the ESP32 sits in an internal locator above it, flashed via
USB on the bench pre-assembly and OTA thereafter (see ESP32 access).

Face is 184×184 mm, slab **31 mm** thick (was 21mm — see the v0.1.6
depth-budget audit in [DEVLOG.md](DEVLOG.md)), 12° rake in the stand.

## Panel orientation

**Wire-exit edge toward the bottom** (the same edge the power pod and
ESP32 sit near). The panel used here has three separate connector
clusters along one edge (5V/GND/DIN, a mid-panel power-injection tap,
5V/GND/DOUT — not a single corner pigtail), so orientation actually
matters: bottom gives the shortest wire runs (60mm to the ESP32, 36mm
to the pod, vs. 140mm+ from the opposite edge) and every connector
clears the pad grid by 17mm or more. Firmware corrects whatever
reading rotation this produces — `cellToLed()` (`display.cpp`) fully
implements `mapRotate`/`mapFlip`/`mapSerp` — dial that in empirically
at first power-on (run a test pattern, see which corner lights first,
adjust in the web UI) rather than something computable in advance
without knowing this specific panel's default LED index origin.

## Ventilation

The clock runs 24/7, so the shell is built for passive cooling, not
sealed:

- **Pressure pads, not a foam sheet.** The panel is pressed flat by a
  sparse grid of 14 8×8 mm pads (a 3×3 inner grid minus the point that
  lands in the socket cavity, four corner pads, and two edge-midpoint
  pads). Every pad lands on an exact lattice wall crossing — never on
  open cell area or behind an LED — and the 3 mm foam (BOM) is cut
  into matching squares, so most of the panel's back face is open air
  instead of insulated.
- **Vent slots through the lid.** Two rows of 18 slots each, 3×25 mm,
  at the device's top and bottom edges for a passive chimney effect
  (the stand rakes the face at a real angle: cool air in low, warm air
  out high, no fan). Sized by natural-convection stack-effect scaling
  for 10 W sustained load at ΔT=35 °C (~1315 mm² per row), not guessed.

**Not yet thermally validated** — the collision checks and the sizing
calc are both just paper (well, CAD). Soak-test after printing
(attract mode, few hours, ambient desk conditions, check the panel/
ESP32 aren't running hot to the touch) before trusting this unattended
long-term; `vent_slot_n`/`vent_span` are easy to bump further if it's
still running warm.

The full sizing derivation and the design history (why pads, the
corner/edge-pad additions, the vent-count resize) are in
[DEVLOG.md](DEVLOG.md).

## Firmware notes

Beyond the README summary: a wandering-Pac ambient mode; hidden-word
easter eggs (the full 9-word list is on the grid — see README), each
firing on its own `wordsMin`/`attractMin` timer, default 10/30
minutes, adjustable in the web UI. MQTT/Home Assistant notification
words are a possible v2. The
[simulator](https://bitbarista.github.io/word-clock-demo/simulator.html)
is the behavioural spec — the grid, word logic and animations there
are what the firmware does.

## Running without internet (e.g. a work desk that can't join company WiFi)

No live internet connection is required for the clock to function.
WiFi is attempted once at boot (15s timeout, using whichever
credentials were last saved via the web UI); if that fails, it falls
back to hosting its **own access point** (`TIME-INVADERS`, password
`insertcoin`) and keeps running regardless, displaying whatever time
it currently has — it doesn't block on or require a connection to
work.

**Manual time sync needs no internet on either end.** The web UI has
a **"⏆ Sync time from this device"** button that POSTs the connected
phone/laptop's own clock straight to the word clock (`/api/time`).
Connect your phone or laptop to the clock's own `TIME-INVADERS`
hotspot, open its web UI, tap the button — no company WiFi, no
personal hotspot's internet, no NTP involved at all, just a direct
local connection to the clock itself. This is the practical option
when company WiFi isn't permitted and a personal hotspot won't be on
permanently: a quick visit to the clock's own hotspot, whatever
interval suits you, substitutes for NTP.

**Between syncs, accuracy depends on the ESP32-S3's own crystal** —
there's no battery-backed RTC chip in the BOM (e.g. a DS3231), so
timekeeping between syncs will drift at whatever rate the onboard
crystal happens to (typically low single digits of seconds/day, not
a guaranteed spec, and heat from nearby LEDs won't help). For a clock
that displays the exact minute, that's a real, visible effect over
days-to-weeks without a resync, not just theoretical.

**Power loss is handled as continuity, not correction.** The current
time is checkpointed to flash every 5 minutes, so a power cycle
without WiFi resumes from roughly where it left off rather than
resetting to zero — but whatever drift had already accumulated
carries through unchanged.
