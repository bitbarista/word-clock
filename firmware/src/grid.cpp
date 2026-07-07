#include "grid.h"

const char* const LETTERS[GRID_N] = {
    "ITKISAHIGHSCOREZ",
    "TWENTYINSERTCOIN",
    "FOURTEENSIXTEENA",
    "SEVENTEENTWELVEB",
    "EIGHTEENNINETEEN",
    "THIRTEENQUARTERS",
    "THREELEVENTENZAP",
    "TWONEFIVEHALFPAC",
    "MINUTESXPASTOPOW",
    "TWONETHREEIGHTGO",
    "SEVENINEFOURFIVE",
    "SIXTENELEVENGAME",
    "TWELVEOCLOCKOVER",
    "SPACEINVADERSPEW",
    "GALAGADONKEYKONG",
    "ASTEROIDSQBERTUP"
};

// -------- time words (letter-sharing layout, see README) --------
static const WordRef W_IT     = {0, 0, 2};
static const WordRef W_IS     = {0, 3, 2};
static const WordRef W_A      = {0, 5, 1};
static const WordRef W_MINUTE = {8, 0, 6};   // prefix of MINUTES
static const WordRef W_MINUTES= {8, 0, 7};
static const WordRef W_PAST   = {8, 8, 4};
static const WordRef W_TO     = {8, 11, 2};
static const WordRef W_OCLOCK = {12, 6, 6};
static const WordRef W_M30    = {7, 9, 4};   // HALF

// minute numbers 1..20 (21-29 = TWENTY + unit)
static const WordRef W_MIN[21] = {
    {0,0,0},
    {7, 2, 3},  // ONE   (TW-ONE)
    {7, 0, 3},  // TWO
    {6, 0, 5},  // THREE (THREE-LEVEN)
    {2, 0, 4},  // FOUR  (FOUR-TEEN)
    {7, 5, 4},  // FIVE
    {2, 8, 3},  // SIX   (SIX-TEEN)
    {3, 0, 5},  // SEVEN (SEVEN-TEEN)
    {4, 0, 5},  // EIGHT (EIGHT-EEN)
    {4, 8, 4},  // NINE  (NINE-TEEN)
    {6, 10, 3}, // TEN
    {6, 4, 6},  // ELEVEN
    {3, 9, 6},  // TWELVE
    {5, 0, 8},  // THIRTEEN
    {2, 0, 8},  // FOURTEEN
    {5, 8, 7},  // QUARTER
    {2, 8, 7},  // SIXTEEN
    {3, 0, 9},  // SEVENTEEN
    {4, 0, 8},  // EIGHTEEN
    {4, 8, 8},  // NINETEEN
    {1, 0, 6},  // TWENTY
};

// hour numbers 1..12
static const WordRef W_HOUR[13] = {
    {0,0,0},
    {9, 2, 3},   // ONE  (TW-ONE)
    {9, 0, 3},   // TWO
    {9, 5, 5},   // THREE (THREE-IGHT)
    {10, 8, 4},  // FOUR
    {10, 12, 4}, // FIVE
    {11, 0, 3},  // SIX
    {10, 0, 5},  // SEVEN (SEVEN-INE)
    {9, 9, 5},   // EIGHT
    {10, 4, 4},  // NINE
    {11, 3, 3},  // TEN
    {11, 6, 6},  // ELEVEN
    {12, 0, 6},  // TWELVE
};

static void addWord(const WordRef& w, CellSet& out, String* phrase) {
    for (uint8_t k = 0; k < w.len; k++)
        out.add(w.r * GRID_N + w.c + k);
    if (phrase) {
        for (uint8_t k = 0; k < w.len; k++)
            *phrase += LETTERS[w.r][w.c + k];
        *phrase += ' ';
    }
}

void timeCells(int hour24, int minute, CellSet& out, String* phrase) {
    out.clear();
    if (phrase) *phrase = "";
    int h = hour24 % 12; if (h == 0) h = 12;

    addWord(W_IT, out, phrase);
    addWord(W_IS, out, phrase);

    if (minute == 0) {
        addWord(W_HOUR[h], out, phrase);
        addWord(W_OCLOCK, out, phrase);
        return;
    }
    bool past = minute <= 30;
    int  mm   = past ? minute : 60 - minute;

    if (mm == 15) { addWord(W_A, out, phrase); addWord(W_MIN[15], out, phrase); }
    else if (mm == 30) { addWord(W_M30, out, phrase); }
    else if (mm == 20) { addWord(W_MIN[20], out, phrase); addWord(W_MINUTES, out, phrase); }
    else if (mm > 20)  { addWord(W_MIN[20], out, phrase);
                         addWord(W_MIN[mm - 20], out, phrase);
                         addWord(W_MINUTES, out, phrase); }
    else               { addWord(W_MIN[mm], out, phrase);
                         addWord(mm == 1 ? W_MINUTE : W_MINUTES, out, phrase); }

    addWord(past ? W_PAST : W_TO, out, phrase);
    addWord(W_HOUR[past ? h : h % 12 + 1], out, phrase);
}

// -------- hidden arcade words --------
const HiddenWord HIDDEN[HIDDEN_COUNT] = {
    {"INSERT COIN",   0xFFB300, 1, {{1, 6, 10}}},
    {"HIGH SCORE",    0x4DD9FF, 1, {{0, 6, 9}}},
    {"GAME OVER",     0xFF4D4D, 2, {{11, 12, 4}, {12, 12, 4}}},
    {"ZAP PAC POW",   0xFFD93B, 3, {{6, 13, 3}, {7, 13, 3}, {8, 13, 3}}},
    {"SPACE INVADERS",0x4BE15F, 2, {{13, 0, 13}, {13, 13, 3}}},   // + PEW
    {"GALAGA",        0x9FD8FF, 1, {{14, 0, 6}}},
    {"DONKEY KONG",   0xFF8C3B, 1, {{14, 6, 10}}},
    {"ASTEROIDS",     0xC8D8FF, 1, {{15, 0, 9}}},
    {"QBERT",         0xFF8C3B, 2, {{15, 9, 5}, {15, 14, 2}}},    // + UP
};

void hiddenCells(uint8_t which, CellSet& out) {
    out.clear();
    if (which >= HIDDEN_COUNT) return;
    const HiddenWord& hw = HIDDEN[which];
    for (uint8_t p = 0; p < hw.nParts; p++)
        for (uint8_t k = 0; k < hw.parts[p].len; k++)
            out.add(hw.parts[p].r * GRID_N + hw.parts[p].c + k);
}
