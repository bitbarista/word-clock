#pragma once
#include <Arduino.h>

void netSetup();          // STA with saved creds, else AP "TIME-INVADERS"
void netLoop();           // captive DNS while in AP mode
bool netIsAP();
String netIP();
void netApplyTz();        // re-apply cfg.tz to the clock/NTP
void netSaveCreds(const String& ssid, const String& pass);
