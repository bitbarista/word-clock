#pragma once
#include <Arduino.h>

void netSetup();          // try the last-joined network, else bring up the portal
void netLoop();           // captive DNS, background reconnect across saved networks
bool netIsAP();           // portal "TIME-INVADERS" is up
bool netIsSta();          // joined to a saved network
String netIP();
int32_t netNtpAgeS();     // seconds since last NTP sync, -1 = never this boot
void netApplyTz();        // re-apply cfg.tz to the clock/NTP
void netSaveCreds(const String& ssid, const String& pass);   // add/move to front
void netForget(const String& ssid);
uint8_t netSavedCount();
String netSavedSsid(uint8_t i);
