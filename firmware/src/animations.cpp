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

static const char* const ROCK_BIG[5]    = { ".X.X.", "XXXXX", "X.XXX", "XXXXX", ".X.X." };
static const char* const ROCK_SMALL[3]  = { ".X.", "XXX", ".X." };
static const char* const BEE_SPR[4]     = { "..X..", ".XXX.", "XXXXX", "X.X.X" };
static const char* const DK_SPR[4]      = { "XX.XX", "XXXXX", ".XXX.", "X.X.X" };

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
        uint8_t up = step >= 16 ? 255 : step * 16, down = 255 - up;   // 16*16 would wrap to 0
        clearFrame();
        for (uint8_t k = 0; k < oldSet.n; k++)
            if (!newSet.has(oldSet.idx[k])) setCell(oldSet.idx[k], scaled(timeCellColor(oldSet.idx[k]), down));
        for (uint8_t k = 0; k < newSet.n; k++)
            setCell(newSet.idx[k], oldSet.has(newSet.idx[k]) ? timeCellColor(newSet.idx[k])
                                                             : scaled(timeCellColor(newSet.idx[k]), up));
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
            if (!eaten[oldSet.idx[k]]) setCell(oldSet.idx[k], timeCellColor(oldSet.idx[k]));
        if (i >= 7 && i - 7 < pn) setCell(path[i - 7], GHOST_RED);
        if (i < pn) setCell(path[i], PAC_YELLOW);
        showFrame(28);
    }
    clearFrame();
    showFrame(250);
    for (uint8_t step = 0; step <= 12; step++) {
        clearFrame();
        for (uint8_t k = 0; k < newSet.n; k++) setCell(newSet.idx[k], scaled(timeCellColor(newSet.idx[k]), step * 21));
        showFrame(35);
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
        for (uint8_t k = 0; k < oldSet.n; k++) if (remain[oldSet.idx[k]]) setCell(oldSet.idx[k], timeCellColor(oldSet.idx[k]));
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
    // Clear stage: the old time fades out, the invader crosses an empty
    // screen dropping bombs, then the new time fades in.
    for (uint8_t s = 1; s <= 8; s++) {
        clearFrame();
        uint8_t lvl = 255 - 255 * s / 8;
        for (uint8_t k = 0; k < oldSet.n; k++) setCell(oldSet.idx[k], scaled(timeCellColor(oldSet.idx[k]), lvl));
        showFrame(35);
    }
    int ix = -2, iy = 0;
    int bombR = -1, bombC = 0;                                // one shot in flight, -1 = none
    for (uint8_t step = 0; step < 22; step++) {
        ix = (step < 8) ? ix + 1 : (step < 16 ? ix - 1 : ix + 1);
        if (step == 8 || step == 16) iy++;
        const char* const* frm = (step & 1) ? INVADER_B : INVADER_A;
        if (step % 2 == 0 && bombR < 0) {                     // fire from a random gun column
            bombC = constrain(ix + 1 + (int)random(0, 9), 0, GRID_N - 1);
            bombR = iy + 8;
        }
        for (uint8_t sub = 0; sub < 4; sub++) {               // bomb falls a row per sub-frame
            clearFrame();
            drawSprite(frm, 11, 8, iy, ix, SI_GREEN);
            if (bombR >= GRID_N) bombR = -1;
            if (bombR >= 0) {
                setCell(bombR * GRID_N + bombC, colAccent());
                if (bombR > iy + 8) setCell((bombR - 1) * GRID_N + bombC, scaled(colAccent(), 70));
                bombR++;
            }
            showFrame(55);
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
            if (r < GRID_N) setCell(r * GRID_N + oldSet.idx[k] % GRID_N, timeCellColor(oldSet.idx[k]));
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
                    (y >= target) ? timeCellColor(newSet.idx[k])
                                  : scaled(timeCellColor(newSet.idx[k]), 140));
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

static void drawGhost14(const CRGB& body) {
    clearFrame();
    for (uint8_t r = 0; r < 14; r++)
        for (uint8_t c = 0; c < 14; c++) {
            char ch = GHOST14[r][c];
            if (ch == '.') continue;
            CRGB col = (ch == 'B') ? body : (ch == 'W') ? CRGB::White : CRGB(0x3B, 0x5B, 0xFF);
            setCell((r + 1) * GRID_N + c + 1, col);
        }
}

static void attractGhost() {
    static const CRGB GCOL[4] = { GHOST_RED, CRGB(0xFF, 0xB8, 0xDE), GHOST_CYAN, CRGB(0xFF, 0xB8, 0x47) };
    for (uint8_t g = 0; g < 4; g++) { drawGhost14(GCOL[g]); showFrame(900); }
    for (uint8_t f = 0; f < 3; f++) {                 // frightened flash
        drawGhost14(CRGB(0x3B, 0x5B, 0xFF)); showFrame(260);
        drawGhost14(CRGB(0xE8, 0xEC, 0xFF)); showFrame(180);
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
    // ship spins at the centre of the field; rocks drift in and get picked off
    static const float H8[8][2] = {
        { -1, 0 }, { -0.71f, 0.71f }, { 0, 1 }, { 0.71f, 0.71f },
        { 1, 0 }, { 0.71f, -0.71f }, { 0, -1 }, { -0.71f, -0.71f }
    };
    static const CRGB SHIP_WHITE(0xE8, 0xEC, 0xFF);
    auto ship = [](uint8_t head) {
        float ur = H8[head][0], uc = H8[head][1], pr = uc, pc = -ur;
        auto px = [](float r, float c) { setRC((int)lroundf(r), (int)lroundf(c), SHIP_WHITE); };
        px(7 + 2.2f * ur, 7 + 2.2f * uc);                       // nose
        px(7 + 1.1f * ur, 7 + 1.1f * uc);
        px(7, 7);
        px(7 - 1.2f * ur + 0.9f * pr, 7 - 1.2f * uc + 0.9f * pc);   // rear wings
        px(7 - 1.2f * ur - 0.9f * pr, 7 - 1.2f * uc - 0.9f * pc);
    };
    auto headingToward = [](float r, float c) -> uint8_t {      // nearest of the 8 headings, 0 = up
        float a = atan2f(c - 7, -(r - 7));
        int h = (int)lroundf(a / 0.7853982f);                   // pi/4
        return (uint8_t)(((h % 8) + 8) % 8);
    };
    struct Rock { float r, c, vr, vc; int8_t wait; };
    struct Frag { float r, c, vr, vc; };
    Rock rocks[2] = { { -5, 10, 0.45f, -0.10f, 0 }, { 16, 0, -0.40f, 0.22f, 10 } };
    Frag frags[4];
    uint8_t nRocks = 2, nFrags = 0, head = 6, spin = 6;
    int8_t doneSpin = -1;
    struct { float r, c, vr, vc; bool live, fresh; } bullet = {};
    for (uint8_t f = 0; f < 80; f++) {
        for (uint8_t i = 0; i < nRocks; i++) {                  // rocks menace the ship but never reach it
            Rock& k = rocks[i];
            if (k.wait > 0) { k.wait--; continue; }
            float nr = k.r + k.vr, nc = k.c + k.vc;
            if (hypotf(nr + 2 - 7, nc + 2 - 7) > 6.0f) { k.r = nr; k.c = nc; }
        }
        for (uint8_t i = 0; i < nFrags; i++) { frags[i].r += frags[i].vr; frags[i].c += frags[i].vc; }
        for (int8_t i = nFrags - 1; i >= 0; i--)
            if (frags[i].r < -3 || frags[i].r > GRID_N || frags[i].c < -3 || frags[i].c > GRID_N)
                frags[i] = frags[--nFrags];
        if (spin > 0) { head = (head + 1) % 8; spin--; }
        else if (!bullet.live && nRocks && rocks[0].wait == 0) {
            int t = headingToward(rocks[0].r + 2, rocks[0].c + 2);
            int diff = ((t - head) % 8 + 8) % 8;
            if (diff == 0) {
                // fire: the bullet flies at the rock's centre (heading is 8-way quantised)
                float dr = rocks[0].r + 2 - 7, dc = rocks[0].c + 2 - 7, d = hypotf(dr, dc);
                bullet = { 7 + 2.2f * dr / d, 7 + 2.2f * dc / d, 1.2f * dr / d, 1.2f * dc / d, true, true };
            }
            else head = (head + (diff <= 4 ? 1 : 7)) % 8;
        }
        if (bullet.live && !bullet.fresh) {
            bullet.r += bullet.vr; bullet.c += bullet.vc;
            if (nRocks && hypotf(bullet.r - (rocks[0].r + 2), bullet.c - (rocks[0].c + 2)) < 1.7f) {
                Rock& k = rocks[0];
                float d = hypotf(bullet.vr, bullet.vc), pr = bullet.vc / d, pc = -bullet.vr / d;
                frags[nFrags++] = { k.r + 1, k.c + 1, k.vr * 0.6f + pr, k.vc * 0.6f + pc };
                frags[nFrags++] = { k.r + 1, k.c + 1, k.vr * 0.6f - pr, k.vc * 0.6f - pc };
                rocks[0] = rocks[1]; nRocks--; bullet.live = false;
                if (!nRocks) doneSpin = 8;                      // victory spin while the frags drift off
            }
            else if (bullet.r < -1 || bullet.r > GRID_N || bullet.c < -1 || bullet.c > GRID_N)
                bullet.live = false;
        }
        bullet.fresh = false;
        if (doneSpin > 0) { head = (head + 1) % 8; doneSpin--; }
        clearFrame();
        for (uint8_t i = 0; i < nRocks; i++)
            if (rocks[i].wait == 0)
                drawSprite(ROCK_BIG, 5, 5, (int)rocks[i].r, (int)rocks[i].c, ASTEROID_GREY);
        for (uint8_t i = 0; i < nFrags; i++)
            drawSprite(ROCK_SMALL, 3, 3, (int)lroundf(frags[i].r), (int)lroundf(frags[i].c), ASTEROID_GREY);
        ship(head);
        if (bullet.live) setRC((int)lroundf(bullet.r), (int)lroundf(bullet.c), colAccent());
        showFrame(90);
        if (doneSpin == 0 && !nRocks && !nFrags) break;
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
    // the classic scene: DK hurls a barrel down the girders and Mario jumps it
    static const CRGB GIRDER(0xF9, 0x4D, 0x6A);
    static const CRGB MARIO_BLUE(0x5A, 0x6A, 0xFF);
    static const CRGB PAULINE(0xFF, 0x9A, 0xD9);
    auto scene = [](bool marioUp, uint8_t dkShift) {
        clearFrame();
        for (uint8_t r = 5; r < GRID_N; r += 5)
            for (uint8_t c = 0; c < GRID_N; c++) setRC(r, c, GIRDER);
        for (uint8_t r = 6; r < 10; r++) setRC(r, 3, GHOST_CYAN);       // ladders
        for (uint8_t r = 11; r < 15; r++) setRC(r, 12, GHOST_CYAN);
        drawSprite(DK_SPR, 5, 4, 1, dkShift, CRGB(0xC8, 0x69, 0x3B));   // DK on the top girder
        setRC(3, 14, PAULINE); setRC(4, 14, PAULINE);                   // Pauline
        int mr = marioUp ? 11 : 13;                                     // Mario, 2x2: cap + overalls
        setRC(mr, 8, GHOST_RED); setRC(mr, 9, GHOST_RED);
        setRC(mr + 1, 8, MARIO_BLUE); setRC(mr + 1, 9, MARIO_BLUE);
    };
    for (uint8_t f = 0; f < 6; f++) { scene(false, f & 1); showFrame(180); }   // DK beats his chest
    int8_t path[43][2];
    uint8_t n = 0;
    for (int8_t c = 5; c <= 14; c++) { path[n][0] = 3; path[n++][1] = c; }     // top level, rolls right
    path[n][0] = 6; path[n++][1] = 14;                                         // falls off the edge
    for (int8_t c = 14; c >= 0; c--) { path[n][0] = 8; path[n++][1] = c; }     // middle level, rolls left
    path[n][0] = 11; path[n++][1] = 0;                                         // falls again
    for (int8_t c = 0; c <= 15; c++) { path[n][0] = 13; path[n++][1] = c; }    // bottom level, exits right
    for (uint8_t i = 0; i < n; i++) {
        int br = path[i][0], bc = path[i][1];
        scene(br == 13 && bc >= 5 && bc <= 11, 0);                             // Mario jumps the barrel
        setRC(br, bc, DK_ORANGE); setRC(br, bc + 1, DK_ORANGE);
        setRC(br + 1, bc, DK_ORANGE); setRC(br + 1, bc + 1, DK_ORANGE);
        showFrame(80);
    }
    scene(false, 0); showFrame(400);
}

static void attractQbert() {
    // 6-level pyramid of 2x2 cubes fills the face: level l top row 2+2l, cube p left col 7-l+2p
    static const CRGB CUBE_BASE(0x4B, 0x5A, 0xB8);
    static const CRGB QBERT_COL(0xFF, 0xD9, 0x3B);
    bool lit[6][6] = {};

    auto draw = [&](int qR, int qC, bool flash, uint16_t ms) {  // qR/qC = Q*Bert's 2x2 top-left, -1 = absent
        clearFrame();
        for (uint8_t l = 0; l < 6; l++)
            for (uint8_t p = 0; p <= l; p++) {
                int r = 2 + 2 * l, c = 7 - l + 2 * p;
                CRGB top = lit[l][p] ? (flash ? CRGB(CRGB::White) : DK_ORANGE) : CUBE_BASE;
                CRGB side = scaled(top, 110);                   // darker lower face for a hint of depth
                setRC(r, c, top); setRC(r, c + 1, top);
                setRC(r + 1, c, side); setRC(r + 1, c + 1, side);
            }
        if (qR >= 0)
            for (uint8_t dr = 0; dr < 2; dr++)
                for (uint8_t dc = 0; dc < 2; dc++) setRC(qR + dr, qC + dc, QBERT_COL);
        showFrame(ms);
    };
    static const int8_t path[6][2] = { {0,0}, {1,1}, {2,1}, {3,2}, {4,2}, {5,3} };  // zigzag hop down
    auto perchC = [](int l, int p) { return 7 - l + 2 * p; };   // Q*Bert's spot on top of cube (l,p)
    int qr = 0, qc = perchC(0, 0);
    draw(qr, qc, false, 500);
    for (uint8_t i = 0; i < 6; i++) {
        int l = path[i][0], p = path[i][1];
        int r1 = 2 * l, c1 = perchC(l, p);
        if (i) draw((qr + r1) / 2 - 1, (qc + c1 + 1) / 2, false, 140);  // hop apex
        lit[l][p] = true; qr = r1; qc = c1;
        draw(qr, qc, false, 360);
    }
    draw(12, 10, false, 140);                                   // hops off the bottom edge
    draw(14, 12, false, 140);
    for (uint8_t f = 0; f < 6; f++) draw(-1, 0, f & 1, 170);    // changed cubes flash
}

void animAttract(uint8_t which) {
    if (which == AT_RANDOM || which >= AT_COUNT) {          // Matrix slot retired
        which = random(AT_INVADERS, AT_COUNT - 1);
        if (which >= AT_MATRIX) which++;
    }
    switch (which) {
        case AT_INVADERS:   attractInvaders();   break;
        case AT_GHOST:      attractGhost();      break;
        case AT_PACCHASE:   attractPacChase();   break;
        case AT_CANNONDUEL: attractCannonDuel(); break;
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
        fillTime(now);
        fillCells(word, scaled(wc, sin8(t * 9)));
        showFrame(45);
    }
}

// Boot logo: the web UI's favicon ghost filling the panel, with clock-face
// eyes whose hands sweep round before settling. Same art as icons/favicon-16.
static const char* const LOGO16[16] = {
    "................", ".....BBBBBB.....", "...BBBBBBBBBB...", "..BHBBBBBBBBBB..",
    "..BWWWWBBWWWWB..", ".BBWWWWBBWWWWBB.", ".BBWWWWBBWWWWBB.", ".BBWWWWBBWWWWBB.",
    ".BBBBBBBBBBBBBB.", ".BBBBBBBBBBBBBB.", ".BBBBBBBBBBBBBB.", ".BBBBBBBBBBBBBB.",
    ".BBBBBBBBBBBBBB.", ".BBBBBBBBBBBBBB.", ".BB.BBB..BBB.BB.", "................"
};
static const uint8_t LOGO_EYE_R = 4, LOGO_EYE_C[2] = {3, 9};    // top-left of each 4x4 eye
static const int8_t LOGO_V[4][2] = {{1, 0}, {1, 3}, {2, 1}, {2, 2}};   // resting "V" hands
// sweep: a rim cell plus the centre cell next to it, clockwise from 12
static const int8_t LOGO_RIM[8][2]  = {{0,1},{0,2},{1,3},{2,3},{3,2},{3,1},{2,0},{1,0}};
static const int8_t LOGO_HUB[8][2]  = {{1,1},{1,2},{1,2},{2,2},{2,2},{2,1},{2,1},{1,1}};

static void drawLogo(uint8_t lvl, const int8_t (*hands)[2], uint8_t nHands) {
    clearFrame();
    for (uint8_t r = 0; r < GRID_N; r++)
        for (uint8_t c = 0; c < GRID_N; c++) {
            char ch = LOGO16[r][c];
            if (ch == '.') continue;
            CRGB col = ch == 'B' ? CRGB(0x03, 0xDD, 0xF7) : ch == 'H' ? CRGB(0x8A, 0xEE, 0xFB) : CRGB(0xFA, 0xF9, 0xFB);
            setCell(r * GRID_N + c, scaled(col, lvl));
        }
    for (uint8_t e = 0; e < 2; e++)
        for (uint8_t k = 0; k < nHands; k++)
            setCell((LOGO_EYE_R + hands[k][0]) * GRID_N + LOGO_EYE_C[e] + hands[k][1],
                    scaled(CRGB(0xFB, 0xC6, 0x0A), lvl));
}

void animBoot() {
    for (uint8_t s = 1; s <= 8; s++) { drawLogo(s * 255 / 8, LOGO_V, 4); showFrame(40); }
    showFrame(300);
    for (uint8_t t = 0; t < 16; t++) {                      // hands sweep round twice
        int8_t h[2][2] = {{LOGO_HUB[t % 8][0], LOGO_HUB[t % 8][1]}, {LOGO_RIM[t % 8][0], LOGO_RIM[t % 8][1]}};
        drawLogo(255, h, 2);
        showFrame(90);
    }
    drawLogo(255, LOGO_V, 4); showFrame(800);
    for (uint8_t s = 8; s > 0; s--) { drawLogo((s - 1) * 255 / 8, LOGO_V, 4); showFrame(40); }
    if (timeValid()) {
        CellSet now; getNowCells(now);
        animFade(CellSet(), now);
    }                                                       // else the base frame shows INSERT COIN
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
