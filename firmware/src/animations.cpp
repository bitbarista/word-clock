#include "animations.h"
#include "display.h"
#include "config.h"

// ================= sprites =================
static const char* const INVADER_A[8] = {
    "..X.....X..", "...X...X...", "..XXXXXXX..", ".XX.XXX.XX.",
    "XXXXXXXXXXX", "X.XXXXXXX.X", "X.X.....X.X", "...XX.XX..."
};
static const char* const INVADER_B[8] = {
    "..X.....X..", "X..X...X..X", "X.XXXXXXX.X", "XXX.XXX.XXX",
    ".XXXXXXXXX.", "..XXXXXXX..", "..X.....X..", ".X.......X."
};
static const char* const GHOST14[14] = {
    "....BBBBBB....", "..BBBBBBBBBB..", ".BBBBBBBBBBBB.", ".BWWWBBBBWWWB.",
    "BBWWWBBBBWWWBB", "BBWPPBBBBWPPBB", "BBBBBBBBBBBBBB", "BBBBBBBBBBBBBB",
    "BBBBBBBBBBBBBB", "BBBBBBBBBBBBBB", "BBBBBBBBBBBBBB", "BBBBBBBBBBBBBB",
    "BBBBBBBBBBBBBB", "BB.BBB..BBB.BB"
};
static const char* const PAC5_OPEN[5]   = { ".XXX.", "XXXX.", "XXX..", "XXXX.", ".XXX." };
static const char* const PAC5_CLOSED[5] = { ".XXX.", "XXXXX", "XXXXX", "XXXXX", ".XXX." };
static const char* const GHOST5[5]      = { ".XXX.", "XXXXX", "XXXXX", "XXXXX", "X.X.X" };
static const char* const CANNON[4]      = { "...X...", "..XXX..", "XXXXXXX", "XXXXXXX" };
static const char* const BOOM_A[5]      = { "..X..", ".....", "X.X.X", ".....", "..X.." };
static const char* const BOOM_B[5]      = { "X.X.X", ".X.X.", "X.X.X", ".X.X.", "X.X.X" };

static const char* const SHIP[4]        = { "..X..", ".XXX.", "XXXXX", ".X.X." };
static const char* const ROCK_BIG[5]    = { ".X.X.", "XXXXX", "X.XXX", "XXXXX", ".X.X." };
static const char* const ROCK_SMALL[3]  = { ".X.", "XXX", ".X." };
static const char* const BEE_SPR[4]     = { "..X..", ".XXX.", "XXXXX", "X.X.X" };
static const char* const DK_SPR[4]      = { "XX.XX", "XXXXX", ".XXX.", "X.X.X" };

static const CRGB MATRIX_HEAD(0xD9, 0xFF, 0xDC);
static const CRGB MATRIX_G1(0x4B, 0xE1, 0x5F);
static const CRGB MATRIX_G2(0x2E, 0x94, 0x40);
static const CRGB MATRIX_G3(0x1C, 0x5C, 0x2A);
static const CRGB PAC_YELLOW(0xFF, 0xD9, 0x3B);
static const CRGB GHOST_RED(0xFF, 0x4D, 0x4D);
static const CRGB GHOST_CYAN(0x4D, 0xD9, 0xFF);
static const CRGB SI_GREEN(0x4B, 0xE1, 0x5F);
static const CRGB GALAGA_BLUE(0x9F, 0xD8, 0xFF);    // matches HIDDEN[5] GALAGA glow colour
static const CRGB DK_ORANGE(0xFF, 0x8C, 0x3B);      // matches HIDDEN[6] DONKEY KONG / HIDDEN[8] QBERT glow colour
static const CRGB ASTEROID_GREY(0xC8, 0xD8, 0xFF);  // matches HIDDEN[7] ASTEROIDS glow colour

static CRGB scaled(const CRGB& c, uint8_t s) { CRGB r = c; r.nscale8_video(s); return r; }

// setCell() writes straight to the LED array with no bounds check, so any
// animation deriving row/col from a running float (drift, hop paths) must
// go through this clipped wrapper instead — a raw negative row/col would
// underflow the uint16_t cell index in setCell().
static void setRC(int r, int c, const CRGB& col) {
    if (r >= 0 && r < GRID_N && c >= 0 && c < GRID_N) setCell(r * GRID_N + c, col);
}

// ================= transitions =================
void animFade(const CellSet& oldSet, const CellSet& newSet) {
    for (uint8_t step = 0; step <= 16; step++) {
        uint8_t up = step * 16, down = 255 - up;
        clearFrame();
        for (uint8_t k = 0; k < oldSet.n; k++)
            if (!newSet.has(oldSet.idx[k])) setCell(oldSet.idx[k], scaled(colTime(), down));
        for (uint8_t k = 0; k < newSet.n; k++)
            setCell(newSet.idx[k], oldSet.has(newSet.idx[k]) ? colTime()
                                                             : scaled(colTime(), up));
        showFrame(35);
    }
}

void animPacEat(const CellSet& oldSet, const CellSet& newSet) {
    // serpentine path over every row that holds a lit cell
    bool rowLit[GRID_N] = {false};
    for (uint8_t k = 0; k < oldSet.n; k++) rowLit[oldSet.idx[k] / GRID_N] = true;
    uint16_t path[NUM_CELLS];
    uint16_t pn = 0;
    uint8_t dir = 0;
    for (uint8_t r = 0; r < GRID_N; r++) {
        if (!rowLit[r]) continue;
        for (uint8_t c = 0; c < GRID_N; c++)
            path[pn++] = r * GRID_N + (dir ? GRID_N - 1 - c : c);
        dir ^= 1;
    }
    bool eaten[NUM_CELLS] = {false};
    for (uint16_t i = 0; i < pn + 8; i++) {
        if (i < pn) eaten[path[i]] = true;
        clearFrame();
        for (uint8_t k = 0; k < oldSet.n; k++)
            if (!eaten[oldSet.idx[k]]) setCell(oldSet.idx[k], colTime());
        if (i >= 7 && i - 7 < pn) setCell(path[i - 7], GHOST_RED);
        if (i < pn) setCell(path[i], PAC_YELLOW);
        showFrame(28);
    }
    clearFrame();
    showFrame(250);
    for (uint8_t step = 0; step <= 12; step++) {
        clearFrame();
        for (uint8_t k = 0; k < newSet.n; k++) setCell(newSet.idx[k], scaled(colTime(), step * 21));
        showFrame(35);
    }
}

void animMatrix(const CellSet& newSet, bool longIntro) {
    struct Drop { float y, v; uint8_t len; };
    Drop d[GRID_N];
    for (uint8_t c = 0; c < GRID_N; c++)
        d[c] = { -(float)random(0, 18), 0.35f + random(0, 75) / 100.0f, (uint8_t)random(4, 8) };
    uint32_t until = millis() + (longIntro ? 6000 : 3200);
    while (millis() < until) {
        clearFrame();
        for (uint8_t c = 0; c < GRID_N; c++) {
            d[c].y += d[c].v;
            int head = (int)d[c].y;
            for (uint8_t k = 0; k < d[c].len; k++) {
                int r = head - k;
                if (r >= 0 && r < GRID_N)
                    setCell(r * GRID_N + c, k == 0 ? MATRIX_HEAD : k < 2 ? MATRIX_G1
                                          : k < 4 ? MATRIX_G2 : MATRIX_G3);
            }
            if (head - d[c].len > GRID_N)
                d[c] = { -(float)random(0, 10), 0.35f + random(0, 75) / 100.0f, (uint8_t)random(4, 8) };
        }
        showFrame(45);
    }
    // rain resolves into the time
    clearFrame();
    fillCells(newSet, MATRIX_G1);
    showFrame(600);
    for (uint8_t step = 0; step <= 12; step++) {
        CRGB c = blend(MATRIX_G1, colTime(), step * 21);
        clearFrame();
        fillCells(newSet, c);
        showFrame(40);
    }
}

void animCannon(const CellSet& oldSet, const CellSet& newSet) {
    bool remain[NUM_CELLS] = {false};
    bool colHas[GRID_N] = {false};
    for (uint8_t k = 0; k < oldSet.n; k++) { remain[oldSet.idx[k]] = true; colHas[oldSet.idx[k] % GRID_N] = true; }
    uint8_t cols[GRID_N], cn = 0;
    for (uint8_t c = 0; c < GRID_N; c++) if (colHas[c]) cols[cn++] = c;
    for (uint8_t k = 0; k < cn; k++) { uint8_t j = random(cn); uint8_t t = cols[k]; cols[k] = cols[j]; cols[j] = t; }

    int cannonC = 8;
    auto drawScene = [&](int shotR, int shotC) {
        clearFrame();
        for (uint8_t k = 0; k < oldSet.n; k++) if (remain[oldSet.idx[k]]) setCell(oldSet.idx[k], colTime());
        drawSprite(CANNON, 7, 4, 12, cannonC - 3, SI_GREEN);
        if (shotR >= 0) setCell(shotR * GRID_N + shotC, colAccent());
        showFrame(26);
    };
    for (uint8_t ci = 0; ci < cn; ci++) {
        while (cannonC != cols[ci]) { cannonC += (cols[ci] > cannonC) ? 1 : -1; drawScene(-1, 0); }
        for (int r = 11; r >= 0; r--) {
            uint16_t cell = r * GRID_N + cannonC;
            if (remain[cell]) { remain[cell] = false; setCell(cell, CRGB::White); }
            drawScene(r, cannonC);
        }
    }
    animFade(CellSet(), newSet);  // reveal (leftover rows 12-15 already covered by cannon)
}

void animInvaderZap(const CellSet& oldSet, const CellSet& newSet) {
    bool remain[NUM_CELLS] = {false};
    for (uint8_t k = 0; k < oldSet.n; k++) remain[oldSet.idx[k]] = true;
    int ix = -2, iy = 0;
    for (uint8_t step = 0; step < 22; step++) {
        ix = (step < 8) ? ix + 1 : (step < 16 ? ix - 1 : ix + 1);
        if (step == 8 || step == 16) iy++;
        const char* const* frm = (step & 1) ? INVADER_B : INVADER_A;
        clearFrame();
        for (uint8_t k = 0; k < oldSet.n; k++) if (remain[oldSet.idx[k]]) setCell(oldSet.idx[k], colTime());
        drawSprite(frm, 11, 8, iy, ix, SI_GREEN);
        showFrame(220);
        if (step % 2 == 0) {           // fire straight down from a random gun column
            uint8_t gc = constrain(ix + 1 + (int)random(0, 9), 0, GRID_N - 1);
            for (int r = iy + 8; r < GRID_N; r++) {
                uint16_t cell = r * GRID_N + gc;
                if (remain[cell]) { remain[cell] = false; setCell(cell, CRGB::White); }
                setCell(cell, colAccent());
                showFrame(22);
            }
        }
    }
    animFade(CellSet(), newSet);
}

void animTetris(const CellSet& oldSet, const CellSet& newSet) {
    // old time drops off the bottom of the screen
    for (uint8_t shift = 0; shift <= GRID_N; shift += 2) {
        clearFrame();
        for (uint8_t k = 0; k < oldSet.n; k++) {
            uint8_t r = oldSet.idx[k] / GRID_N + shift;
            if (r < GRID_N) setCell(r * GRID_N + oldSet.idx[k] % GRID_N, colTime());
        }
        showFrame(45);
    }
    // new time falls in, staggered like pieces locking
    uint16_t maxFrames = newSet.n * 2 + GRID_N + 4;
    for (uint16_t t = 0; t < maxFrames; t++) {
        clearFrame();
        bool allLanded = true;
        for (uint8_t k = 0; k < newSet.n; k++) {
            int target = newSet.idx[k] / GRID_N;
            int y = (int)t - (int)k * 2;
            if (y < 0) { allLanded = false; continue; }
            if (y < target) allLanded = false;
            int rr = min(y, target);
            setCell(rr * GRID_N + newSet.idx[k] % GRID_N,
                    (y >= target) ? colTime() : scaled(colTime(), 140));
        }
        showFrame(30);
        if (allLanded) break;
    }
}

// ================= attract shows =================
static void attractInvaders() {
    int iy = 1;
    for (uint8_t step = 0; step < 26; step++) {
        int ix = 2 + ((step < 9) ? step % 9 : (step < 18) ? 8 - step % 9 : step % 9) / 2;
        if (step == 9 || step == 18) iy += 2;
        clearFrame();
        drawSprite((step & 1) ? INVADER_B : INVADER_A, 11, 8, iy, ix, SI_GREEN);
        showFrame(260);
    }
}

static void attractGhost() {
    static const CRGB GCOL[4] = { GHOST_RED, CRGB(0xFF, 0xB8, 0xDE), GHOST_CYAN, CRGB(0xFF, 0xB8, 0x47) };
    auto draw = [&](const CRGB& body) {
        clearFrame();
        for (uint8_t r = 0; r < 14; r++)
            for (uint8_t c = 0; c < 14; c++) {
                char ch = GHOST14[r][c];
                if (ch == '.') continue;
                CRGB col = (ch == 'B') ? body : (ch == 'W') ? CRGB::White : CRGB(0x3B, 0x5B, 0xFF);
                setCell((r + 1) * GRID_N + c + 1, col);
            }
    };
    for (uint8_t g = 0; g < 4; g++) { draw(GCOL[g]); showFrame(900); }
    for (uint8_t f = 0; f < 3; f++) {                 // frightened flash
        draw(CRGB(0x3B, 0x5B, 0xFF)); showFrame(260);
        draw(CRGB(0xE8, 0xEC, 0xFF)); showFrame(180);
    }
}

static void attractPacChase() {
    for (uint8_t pass = 0; pass < 2; pass++) {
        for (int x = -6; x < GRID_N + 16; x++) {
            clearFrame();
            for (int8_t c = 1; c < GRID_N; c += 2)          // dot trail
                if (c > x + 2) setCell(8 * GRID_N + c, scaled(CRGB::White, 90));
            drawSprite((x & 1) ? PAC5_OPEN : PAC5_CLOSED, 5, 5, 6, x, PAC_YELLOW);
            drawSprite(GHOST5, 5, 5, 6, x - 8, pass ? GHOST_CYAN : GHOST_RED);
            drawSprite(GHOST5, 5, 5, 6, x - 15, pass ? GHOST_RED : GHOST_CYAN);
            showFrame(90);
        }
    }
}

static void attractCannonDuel() {
    int ix = 0, dirx = 1, cannonC = 2;
    for (uint8_t step = 0; step < 18; step++) {           // invader drifts, cannon tracks
        ix += dirx; if (ix > 4 || ix < 0) { dirx = -dirx; ix += dirx; }
        int targetC = ix + 5;
        if (cannonC < targetC) cannonC++; else if (cannonC > targetC) cannonC--;
        clearFrame();
        drawSprite((step & 1) ? INVADER_B : INVADER_A, 11, 8, 1, ix, SI_GREEN);
        drawSprite(CANNON, 7, 4, 12, cannonC - 3, SI_GREEN);
        showFrame(200);
    }
    for (int r = 11; r >= 5; r--) {                        // the shot
        clearFrame();
        drawSprite(INVADER_A, 11, 8, 1, ix, SI_GREEN);
        drawSprite(CANNON, 7, 4, 12, cannonC - 3, SI_GREEN);
        setCell(r * GRID_N + cannonC, colAccent());
        showFrame(45);
    }
    for (uint8_t f = 0; f < 4; f++) {                      // explosion + POW!
        clearFrame();
        drawSprite(CANNON, 7, 4, 12, cannonC - 3, SI_GREEN);
        drawSprite((f & 1) ? BOOM_B : BOOM_A, 5, 5, 3, cannonC - 2, (f & 1) ? colAccent() : CRGB::White);
        CellSet pow; hiddenCells(3, pow);                  // ZAP PAC POW group
        fillCells(pow, GHOST_RED);
        showFrame(220);
    }
}

static void attractCoin() {
    CellSet coin, score;
    hiddenCells(0, coin);
    hiddenCells(1, score);
    for (uint8_t f = 0; f < 3; f++) {
        clearFrame(); fillCells(coin, colAccent()); showFrame(420);
        clearFrame(); showFrame(180);
    }
    clearFrame(); fillCells(score, GHOST_CYAN); showFrame(1000);
}

static void attractAsteroids() {
    int shipC = 2;
    float rockR = -4, rockC = 8;
    while (rockR < 8) {                                    // rock drifts down toward the ship
        rockR += 0.6f;
        clearFrame();
        drawSprite(SHIP, 5, 4, 12, shipC, CRGB(0xE8, 0xEC, 0xFF));
        drawSprite(ROCK_BIG, 5, 5, (int)rockR, (int)rockC, ASTEROID_GREY);
        showFrame(90);
    }
    int hitR = (int)rockR + 2;
    for (int r = 11; r > hitR; r--) {                       // the shot
        clearFrame();
        drawSprite(SHIP, 5, 4, 12, shipC, CRGB(0xE8, 0xEC, 0xFF));
        drawSprite(ROCK_BIG, 5, 5, (int)rockR, (int)rockC, ASTEROID_GREY);
        setRC(r, shipC + 2, CRGB::White);
        showFrame(35);
    }
    float lC = rockC, rC = rockC + 2, lR = rockR, rR = rockR;
    for (uint8_t step = 0; step < 10; step++) {             // rock splits, fragments drift apart
        lC -= 0.9f; rC += 0.9f; lR += 0.4f; rR += 0.4f;
        clearFrame();
        drawSprite(SHIP, 5, 4, 12, shipC, CRGB(0xE8, 0xEC, 0xFF));
        drawSprite(ROCK_SMALL, 3, 3, (int)lR, (int)lC, ASTEROID_GREY);
        drawSprite(ROCK_SMALL, 3, 3, (int)rR, (int)rC, ASTEROID_GREY);
        showFrame(70);
    }
}

static void attractGalaga() {
    struct Slot { float r, c; int8_t targetC; };
    Slot slots[3] = { { -4, 2, 3 }, { -4, 13, 12 }, { -6, 7, 7 } };
    for (uint8_t step = 0; step < 20; step++) {             // swoop into formation
        clearFrame();
        for (auto& s : slots) {
            if (s.r < 3) s.r += 0.5f;
            if (s.c < s.targetC) s.c += 0.6f; else if (s.c > s.targetC) s.c -= 0.6f;
            drawSprite(BEE_SPR, 5, 4, (int)s.r, (int)s.c, GALAGA_BLUE);
        }
        showFrame(110);
    }
    float dr = slots[2].r, dc = slots[2].c;
    for (uint8_t step = 0; step < 14 && dr < GRID_N; step++) {  // one peels off and dives
        dr += 0.9f;
        dc += (step & 1) ? 0.6f : -0.6f;
        clearFrame();
        drawSprite(BEE_SPR, 5, 4, (int)slots[0].r, (int)slots[0].c, GALAGA_BLUE);
        drawSprite(BEE_SPR, 5, 4, (int)slots[1].r, (int)slots[1].c, GALAGA_BLUE);
        drawSprite(BEE_SPR, 5, 4, (int)dr, (int)dc, GALAGA_BLUE);
        showFrame(70);
    }
}

static void attractDonkeyKong() {
    static const int8_t path[][2] = {                      // zigzag down alternating girders
        {2,1},{2,3},{2,5},{2,7},{2,9},{2,11},
        {4,11},{4,9},{4,7},{4,5},{4,3},{4,1},
        {6,1},{6,3},{6,5},{6,7},{6,9},{6,11},
        {8,11},{8,9},{8,7},{8,5},{8,3},{8,1},
        {10,1},{10,3},{10,5},{10,7},{10,9},{10,11},
        {12,11},{13,11},{14,11},{15,11}
    };
    for (auto& p : path) {
        clearFrame();
        drawSprite(DK_SPR, 5, 4, 0, 0, CRGB(0xC8, 0x69, 0x3B));
        setRC(p[0], p[1],     DK_ORANGE);
        setRC(p[0], p[1] + 1, DK_ORANGE);
        showFrame(70);
    }
}

static void attractQbert() {
    static const CRGB CUBE_BASE(0x4B, 0x5A, 0xB8);
    static const CRGB QBERT_COL(0xFF, 0xD9, 0x3B);
    CRGB level[5][9];
    for (auto& row : level) for (auto& c : row) c = CUBE_BASE;

    auto draw = [&](int hopL) {
        clearFrame();
        for (uint8_t l = 0; l <= 4; l++) {
            uint8_t w = l * 2 + 1;
            int startC = 8 - l;
            for (uint8_t p = 0; p < w; p++) setRC(6 + l, startC + p, level[l][p]);
        }
        if (hopL >= 0) setRC(6 + hopL, 8 - hopL, QBERT_COL);
        showFrame(260);
    };
    draw(-1);
    for (uint8_t l = 0; l <= 4; l++) {                      // hop diagonally, changing each cube
        level[l][0] = DK_ORANGE;
        draw(l);
    }
}

void animAttract(uint8_t which) {
    if (which == AT_RANDOM || which >= AT_COUNT) which = random(AT_INVADERS, AT_COUNT);
    switch (which) {
        case AT_INVADERS:   attractInvaders();   break;
        case AT_GHOST:      attractGhost();      break;
        case AT_PACCHASE:   attractPacChase();   break;
        case AT_CANNONDUEL: attractCannonDuel(); break;
        case AT_MATRIX:   { CellSet now; getNowCells(now); animMatrix(now, true); return; }
        case AT_COIN:       attractCoin();       break;
        case AT_ASTEROIDS:   attractAsteroids();  break;
        case AT_GALAGA:      attractGalaga();     break;
        case AT_DONKEYKONG:  attractDonkeyKong(); break;
        case AT_QBERT:       attractQbert();      break;
    }
    // settle back into the time
    CellSet now; getNowCells(now);
    animFade(CellSet(), now);
}

// ================= extras =================
void animHiddenWord(uint8_t which) {
    CellSet word; hiddenCells(which, word);
    CellSet now;  getNowCells(now);
    CRGB wc((HIDDEN[which].color >> 16) & 0xFF, (HIDDEN[which].color >> 8) & 0xFF, HIDDEN[which].color & 0xFF);
    for (uint8_t t = 0; t < 56; t++) {
        clearFrame();
        fillCells(now, colTime());
        fillCells(word, scaled(wc, sin8(t * 9)));
        showFrame(45);
    }
}

void animBoot() {
    CellSet coin; hiddenCells(0, coin);
    for (uint8_t f = 0; f < 2; f++) {
        for (uint8_t s = 0; s <= 10; s++) { clearFrame(); fillCells(coin, scaled(colAccent(), s * 25)); showFrame(30); }
        for (uint8_t s = 10; s > 0; s--)  { clearFrame(); fillCells(coin, scaled(colAccent(), s * 25)); showFrame(30); }
    }
    if (timeValid()) { CellSet now; getNowCells(now); animMatrix(now, false); }
}

void animMapTest() {
    // orient the panel: red = top-left, green = top-right, blue = bottom-left
    for (uint8_t f = 0; f < 4; f++) {
        clearFrame();
        setCell(0, CRGB::Red);
        setCell(GRID_N - 1, CRGB::Green);
        setCell((GRID_N - 1) * GRID_N, CRGB::Blue);
        setCell(NUM_CELLS - 1, CRGB::White);
        showFrame(700);
        clearFrame();
        for (uint8_t c = 0; c < GRID_N; c++) setCell(c, CRGB::Red);       // row 0
        for (uint8_t r = 0; r < GRID_N; r++) setCell(r * GRID_N, CRGB::Blue); // col 0
        showFrame(700);
    }
}

void animIdentify() {
    for (uint8_t f = 0; f < 2; f++) {
        fill_solid(leds, NUM_CELLS, CRGB::White);
        showFrame(250);
        clearFrame();
        showFrame(200);
    }
}
