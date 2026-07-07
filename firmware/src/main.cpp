// =================================================================
// TIME INVADERS — 8-bit arcade word clock firmware
// ESP32-S3 supermini + 16x16 WS2812B panel
// Behavioural spec: ../concept/wordclock-concepts.html
// =================================================================
#include <Arduino.h>
#include "config.h"
#include "grid.h"
#include "display.h"
#include "network.h"
#include "web.h"

extern volatile bool g_reboot;

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.printf("\nTIME INVADERS fw v%s\n", FW_VERSION);
    configLoad();
    netSetup();      // brings up WiFi/AP + NTP before first render
    engineSetup();   // LEDs, restored time, boot animation
    webSetup();
}

void loop() {
    netLoop();
    engineTick();
    if (g_reboot) {
        delay(600);      // let the HTTP response flush
        ESP.restart();
    }
}
