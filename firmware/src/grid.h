// The 16x16 letter grid and per-minute word logic.
// This is the firmware port of the behavioural spec in
// ../concept/wordclock-concepts.html — keep the two in sync.
#pragma once
#include <Arduino.h>

constexpr uint8_t  GRID_N    = 16;
constexpr uint16_t NUM_CELLS = 256;

extern const char* const LETTERS[GRID_N];

struct WordRef { uint8_t r, c, len; };

struct CellSet {
    uint16_t idx[112];
    uint8_t  n = 0;
    void clear() { n = 0; }
    void add(uint16_t i) { if (n < 112) idx[n++] = i; }
    bool has(uint16_t i) const {
        for (uint8_t k = 0; k < n; k++) if (idx[k] == i) return true;
        return false;
    }
};

// cells + optional human-readable phrase for the given time
void timeCells(int hour24, int minute, CellSet& out, String* phrase = nullptr);

struct HiddenWord {
    const char* name;
    uint32_t    color;
    uint8_t     nParts;
    WordRef     parts[3];
};
constexpr uint8_t HIDDEN_COUNT = 9;
extern const HiddenWord HIDDEN[HIDDEN_COUNT];
void hiddenCells(uint8_t which, CellSet& out);
