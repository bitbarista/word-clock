#pragma once
#include <FastLED.h>
#include "grid.h"
#include "config.h"

extern CRGB leds[NUM_CELLS];

// grid cell (row*16+col) -> physical LED index, honouring mapping cfg
uint16_t cellToLed(uint16_t cell);

void engineSetup();
void engineTick();          // call from loop()
void applyPower();          // re-apply brightness/power budget

String currentPhrase();     // "IT IS TWENTY FIVE PAST TEN"

// requests posted by the web task, consumed by the engine
enum ReqKind : uint8_t { RQ_NONE = 0, RQ_TRANSITION, RQ_ATTRACT, RQ_WORDS,
                         RQ_MAPTEST, RQ_IDENTIFY };
extern volatile uint8_t reqKind;
extern volatile int8_t  reqParam;

// ---- shared helpers for animations.cpp ----
CRGB colTime();
CRGB colAccent();
void clearFrame();
void setCell(uint16_t cell, const CRGB& c);
void fillCells(const CellSet& s, const CRGB& c);
void showFrame(uint16_t holdMs);   // FastLED.show + delay
void drawSprite(const char* const* rows, uint8_t w, uint8_t h,
                int offR, int offC, const CRGB& c, char on = 'X');
bool timeValid();
void getNowCells(CellSet& out);    // current time's word cells
