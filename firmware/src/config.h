#pragma once
#include <Arduino.h>

#define FW_VERSION "0.4.8"

enum TransStyle : uint8_t {
    TR_RANDOM = 0, TR_PAC, TR_MATRIX, TR_CANNON, TR_INVADER, TR_TETRIS, TR_FADE,
    TR_COUNT
};

enum Theme : uint8_t {
    THEME_ARCADE = 0, THEME_MATRIX, THEME_AMBER, THEME_ICE, THEME_CUSTOM, THEME_PARTY
};

enum AttractAnim : uint8_t {
    AT_RANDOM = 0, AT_INVADERS, AT_GHOST, AT_PACCHASE, AT_CANNONDUEL, AT_MATRIX, AT_COIN,
    AT_ASTEROIDS, AT_GALAGA, AT_DONKEYKONG, AT_QBERT,
    AT_COUNT
};

struct Config {
    // display
    uint8_t  brightness  = 140;
    bool     nightEnabled = true;
    uint8_t  nightBright = 25;
    uint8_t  nightFrom   = 22;
    uint8_t  nightTo     = 7;
    uint16_t powerMa     = 2500;   // FastLED per-frame power budget (pod-powered)
    uint16_t usbSafeMa   = 400;    // budget clamp while a USB host is on the
                                    // programming port — see usbguard.h for why
    // theme / colours
    uint8_t  theme       = 0;      // Theme enum: arcade..custom, party = per-letter colours
    uint32_t timeColor   = 0xFFFFFF;
    uint32_t accentColor = 0xFFB300;
    // animations
    bool     animsEnabled   = true;
    uint8_t  transStyle     = TR_RANDOM;
    bool     attractEnabled = true;
    uint16_t attractMin     = 30;  // minutes between attract shows
    // hidden arcade words
    bool     wordsEnabled = true;
    uint16_t wordsMin     = 10;    // minutes between word glints
    // ambient
    bool     pacAmbient = true;    // wandering pac + chasing ghost
    // panel mapping
    uint8_t  mapRotate = 0;        // 0..3 quarter turns
    bool     mapSerp   = true;     // serpentine rows
    bool     mapFlip   = false;    // mirror columns
    // time / network
    char     tz[64]       = "GMT0BST,M3.5.0/1,M10.5.0";  // Europe/London
    char     hostname[24] = "timeinvaders";
};

extern Config cfg;
void configLoad();
void configSave();
