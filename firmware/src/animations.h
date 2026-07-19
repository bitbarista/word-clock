#pragma once
#include "grid.h"

// time-change transitions (blocking; async web server keeps running)
void animFade(const CellSet& oldSet, const CellSet& newSet);
void animPacEat(const CellSet& oldSet, const CellSet& newSet);
void animCannon(const CellSet& oldSet, const CellSet& newSet);
void animInvaderZap(const CellSet& oldSet, const CellSet& newSet);
void animTetris(const CellSet& oldSet, const CellSet& newSet);

// attract-mode shows
void animAttract(uint8_t which);   // AttractAnim, AT_RANDOM picks one

// extras
void animHiddenWord(uint8_t which);
void animBoot();
void animMapTest();
void animIdentify();
