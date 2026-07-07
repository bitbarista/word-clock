#include "network.h"
#include "config.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>

static DNSServer dns;
static bool apMode = false;

bool netIsAP() { return apMode; }
String netIP() { return apMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString(); }

void netApplyTz() {
    configTzTime(cfg.tz, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
}

void netSaveCreds(const String& ssid, const String& pass) {
    Preferences p;
    p.begin("ti", false);
    p.putString("ssid", ssid);
    p.putString("pass", pass);
    p.end();
}

void netSetup() {
    Preferences p;
    p.begin("ti", true);
    String ssid = p.getString("ssid", "");
    String pass = p.getString("pass", "");
    p.end();

    WiFi.setHostname(cfg.hostname);
    if (ssid.length()) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());
        Serial.printf("[net] connecting to %s", ssid.c_str());
        uint32_t t0 = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
            delay(250);
            Serial.print('.');
        }
        Serial.println();
    }
    if (WiFi.status() == WL_CONNECTED) {
        apMode = false;
        Serial.printf("[net] STA %s\n", WiFi.localIP().toString().c_str());
        netApplyTz();
    } else {
        apMode = true;
        WiFi.mode(WIFI_AP);
        WiFi.softAP("TIME-INVADERS", "insertcoin");
        dns.start(53, "*", WiFi.softAPIP());
        Serial.printf("[net] AP mode %s (pw: insertcoin)\n", WiFi.softAPIP().toString().c_str());
    }
    MDNS.begin(cfg.hostname);
    MDNS.addService("http", "tcp", 80);
}

void netLoop() {
    if (apMode) dns.processNextRequest();
}
