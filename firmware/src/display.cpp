#include "display.h"
#include "animations.h"
#include "usbguard.h"
#include <time.h>
#include <Preferences.h>

CRGB leds[NUM_CELLS];
volatile uint8_t reqKind  = RQ_NONE;
volatile int8_t  reqParam = -1;

static CellSet nowCells;
static String  nowPhrase;
static int     lastMin  = -1;
static int     lastHour = -1;
static uint32_t nextAttractAt = 0;
static uint32_t nextWordsAt   = 0;
static uint32_t lastEpochSave = 0;

// ---------- colour helpers ----------
static CRGB fromU32(uint32_t v) { return CRGB((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF); }
CRGB colTime()   { return fromU32(cfg.timeColor); }
CRGB colAccent() { return fromU32(cfg.accentColor); }

// ---------- mapping ----------
uint16_t cellToLed(uint16_t cell) {
    uint8_t r = cell / GRID_N, c = cell % GRID_N;
    // quarter turns
    for (uint8_t k = 0; k < (cfg.mapRotate & 3); k++) {
        uint8_t nr = c, nc = GRID_N - 1 - r;
        r = nr; c = nc;
    }
    if (cfg.mapFlip) c = GRID_N - 1 - c;
    if (cfg.mapSerp && (r & 1)) c = GRID_N - 1 - c;
    return r * GRID_N + c;
}

// ---------- frame helpers ----------
void clearFrame() { fill_solid(leds, NUM_CELLS, CRGB::Black); }
void setCell(uint16_t cell, const CRGB& c) { leds[cellToLed(cell)] = c; }
void fillCells(const CellSet& s, const CRGB& c) {
    for (uint8_t k = 0; k < s.n; k++) setCell(s.idx[k], c);
}

static uint8_t effectiveBrightness() {
    if (!cfg.nightEnabled) return cfg.brightness;
    time_t t = time(nullptr);
    struct tm tmv;
    localtime_r(&t, &tmv);
    uint8_t h = tmv.tm_hour;
    bool night = (cfg.nightFrom > cfg.nightTo)
        ? (h >= cfg.nightFrom || h < cfg.nightTo)
        : (h >= cfg.nightFrom && h < cfg.nightTo);
    return night ? cfg.nightBright : cfg.brightness;
}

void showFrame(uint16_t holdMs) {
    FastLED.setBrightness(effectiveBrightness());
    FastLED.show();
    if (holdMs) delay(holdMs);
}

void drawSprite(const char* const* rows, uint8_t w, uint8_t h,
                int offR, int offC, const CRGB& c, char on) {
    for (uint8_t r = 0; r < h; r++)
        for (uint8_t col = 0; col < w; col++) {
            int rr = offR + r, cc = offC + col;
            if (rows[r][col] == on && rr >= 0 && rr < GRID_N && cc >= 0 && cc < GRID_N)
                setCell(rr * GRID_N + cc, c);
        }
}

bool timeValid() { return time(nullptr) > 1600000000; }

void getNowCells(CellSet& out) {
    time_t t = time(nullptr);
    struct tm tmv;
    localtime_r(&t, &tmv);
    timeCells(tmv.tm_hour, tmv.tm_min, out);
}

String currentPhrase() { return timeValid() ? nowPhrase : String("INSERT COIN"); }

static bool usbGuardActive = false;
bool usbPowerLimited() { return usbGuardActive; }

void applyPower() {
    uint16_t budget = usbGuardActive ? min(cfg.powerMa, cfg.usbSafeMa) : cfg.powerMa;
    FastLED.setMaxPowerInVoltsAndMilliamps(5, constrain(budget, (uint16_t)300, (uint16_t)3000));
}

// re-evaluate the USB guard every tick; only touches FastLED on a
// genuine transition (plug/unplug), not every frame
static void pollUsbGuard() {
    bool present = usbHostPresent();
    if (present != usbGuardActive) {
        usbGuardActive = present;
        applyPower();
    }
}

// ---------- ambient pac ----------
struct PacState {
    int8_t r = 13, c = 3;
    int8_t histR[12], histC[12];
    uint8_t histN = 0;
    uint32_t nextStep = 0;
} pac;

static void pacStep() {
    if (millis() < pac.nextStep) return;
    pac.nextStep = millis() + 350;
    // shift history
    for (uint8_t k = 11; k > 0; k--) { pac.histR[k] = pac.histR[k-1]; pac.histC[k] = pac.histC[k-1]; }
    pac.histR[0] = pac.r; pac.histC[0] = pac.c;
    if (pac.histN < 12) pac.histN++;
    // pick a neighbour, preferring unlit cells
    static const int8_t dr[4] = {0, 0, 1, -1};
    static const int8_t dc[4] = {1, -1, 0, 0};
    uint8_t order[4] = {0, 1, 2, 3};
    for (uint8_t k = 0; k < 4; k++) { uint8_t j = random(4); uint8_t t = order[k]; order[k] = order[j]; order[j] = t; }
    int8_t bestR = pac.r, bestC = pac.c;
    bool found = false;
    for (uint8_t k = 0; k < 4; k++) {
        int8_t nr = pac.r + dr[order[k]], nc = pac.c + dc[order[k]];
        if (nr < 0 || nr >= GRID_N || nc < 0 || nc >= GRID_N) continue;
        if (!found) { bestR = nr; bestC = nc; found = true; }
        if (!nowCells.has(nr * GRID_N + nc)) { bestR = nr; bestC = nc; break; }
    }
    pac.r = bestR; pac.c = bestC;
}

static void drawAmbient() {
    if (!cfg.pacAmbient) return;
    if (pac.histN >= 8)
        setCell(pac.histR[7] * GRID_N + pac.histC[7], CRGB(180, 40, 40));   // chasing ghost
    setCell(pac.r * GRID_N + pac.c, CRGB(200, 170, 40));                    // pac
}

// ---------- base clock frame ----------
static void renderBase() {
    clearFrame();
    if (timeValid()) fillCells(nowCells, colTime());
    else { CellSet coin; hiddenCells(0, coin); fillCells(coin, colAccent()); } // "please set time"
    drawAmbient();
}

static void refreshTimeWords() {
    time_t t = time(nullptr);
    struct tm tmv;
    localtime_r(&t, &tmv);
    timeCells(tmv.tm_hour, tmv.tm_min, nowCells, &nowPhrase);
}

// ---------- engine ----------
void engineSetup() {
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_CELLS);
    // safe-by-default: assume a host MIGHT be present until the first
    // poll says otherwise, so even the boot animation can't overdraw
    // through the programming port's diode
    usbGuardActive = true;
    pollUsbGuard();
    applyPower();
    FastLED.setBrightness(cfg.brightness);
    clearFrame();
    FastLED.show();
    // restore approximate time saved before last power-off
    Preferences p;
    p.begin("ti", true);
    uint32_t saved = p.getULong("epoch", 0);
    p.end();
    if (saved > 1600000000 && !timeValid()) {
        struct timeval tv = { (time_t)saved, 0 };
        settimeofday(&tv, nullptr);
    }
    refreshTimeWords();
    nextAttractAt = millis() + cfg.attractMin * 60000UL;
    nextWordsAt   = millis() + cfg.wordsMin * 60000UL;
    animBoot();
}

static void doTransition(uint8_t style, const CellSet& oldSet, const CellSet& newSet) {
    if (style == TR_RANDOM) style = random(TR_PAC, TR_FADE);  // pac..tetris
    switch (style) {
        case TR_PAC:     animPacEat(oldSet, newSet);   break;
        case TR_MATRIX:  animMatrix(newSet, false);    break;
        case TR_CANNON:  animCannon(oldSet, newSet);   break;
        case TR_INVADER: animInvaderZap(oldSet, newSet); break;
        case TR_TETRIS:  animTetris(oldSet, newSet);   break;
        default:         animFade(oldSet, newSet);     break;
    }
}

void engineTick() {
    pollUsbGuard();   // must run every tick, ahead of any early return below

    // periodic epoch save for power-loss recovery (every 5 min)
    if (timeValid() && millis() - lastEpochSave > 300000UL) {
        lastEpochSave = millis();
        Preferences p;
        p.begin("ti", false);
        p.putULong("epoch", (uint32_t)time(nullptr));
        p.end();
    }

    // consume web requests
    uint8_t rq = reqKind;
    if (rq != RQ_NONE) {
        int8_t param = reqParam;
        reqKind = RQ_NONE;
        CellSet cur = nowCells;
        switch (rq) {
            case RQ_TRANSITION: refreshTimeWords(); doTransition(param < 0 ? cfg.transStyle : param, cur, nowCells); break;
            case RQ_ATTRACT:    animAttract(param < 0 ? AT_RANDOM : param); break;
            case RQ_WORDS:      animHiddenWord(param < 0 ? random(HIDDEN_COUNT) : param); break;
            case RQ_MAPTEST:    animMapTest();  break;
            case RQ_IDENTIFY:   animIdentify(); break;
        }
        return;
    }

    if (timeValid()) {
        time_t t = time(nullptr);
        struct tm tmv;
        localtime_r(&t, &tmv);
        if (tmv.tm_min != lastMin) {
            bool five = (tmv.tm_min % 5 == 0) || (tmv.tm_hour != lastHour);
            CellSet oldSet = nowCells;
            bool first = (lastMin < 0);
            lastMin = tmv.tm_min; lastHour = tmv.tm_hour;
            refreshTimeWords();
            if (!first) {
                if (cfg.animsEnabled && five) doTransition(cfg.transStyle, oldSet, nowCells);
                else                          animFade(oldSet, nowCells);
            }
        }
    }

    // attract mode
    if (cfg.animsEnabled && cfg.attractEnabled && millis() > nextAttractAt) {
        nextAttractAt = millis() + cfg.attractMin * 60000UL;
        animAttract(AT_RANDOM);
        return;
    }
    // hidden word glint
    if (cfg.wordsEnabled && millis() > nextWordsAt) {
        nextWordsAt = millis() + cfg.wordsMin * 60000UL;
        animHiddenWord(random(HIDDEN_COUNT));
        return;
    }

    pacStep();
    renderBase();
    showFrame(20);
}
