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
#include <esp_ota_ops.h>

extern volatile bool g_reboot;

// OTA safety net. A freshly OTA'd image stays "pending" until it proves it
// is reachable: joined WiFi, or served the web UI. If it crashes or resets
// first, the bootloader falls back to the previous image; if it runs but
// stays unreachable for 5 minutes, it rolls itself back.
extern "C" bool verifyRollbackLater() { return true; }
static bool otaPending = false;
static const uint32_t OTA_PROVE_MS = 300000;

static void otaCheck() {
    if (!otaPending) return;
    if (netIsSta() || webSeenClient()) {
        esp_ota_mark_app_valid_cancel_rollback();
        otaPending = false;
        Serial.println("[ota] new image confirmed");
    } else if (millis() > OTA_PROVE_MS) {
        Serial.println("[ota] unreachable, rolling back");
        esp_ota_mark_app_invalid_rollback_and_reboot();
        otaPending = false;         // only reached if no rollback target exists
    }
}

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.printf("\nTIME INVADERS fw v%s\n", FW_VERSION);
    esp_ota_img_states_t st;
    otaPending = esp_ota_get_state_partition(esp_ota_get_running_partition(), &st) == ESP_OK
                 && st == ESP_OTA_IMG_PENDING_VERIFY;
    if (otaPending) Serial.println("[ota] new image on probation");
    configLoad();
    netSetup();      // brings up WiFi/AP + NTP before first render
    engineSetup();   // LEDs, restored time, boot animation
    webSetup();
}

void loop() {
    netLoop();
    otaCheck();
    engineTick();
    if (g_reboot) {
        delay(600);      // let the HTTP response flush
        ESP.restart();
    }
}
