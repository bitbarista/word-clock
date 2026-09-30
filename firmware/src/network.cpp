#include "network.h"
#include "config.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <esp_sntp.h>

// Saved networks, most recently joined first. Slot 0 keeps the original
// "ssid"/"pass" keys so a rollback to older firmware still finds it.
static const uint8_t MAX_NETS = 5;
static String netSsid[MAX_NETS], netPass[MAX_NETS];
static uint8_t netCount = 0;

static const uint32_t TRY_MS          = 15000;  // per saved network
static const uint32_t CYCLE_GAP_MS    = 30000;  // pause between full passes
static const uint32_t AP_AFTER_MS     = 20000;  // STA down this long -> portal up
static const uint32_t AP_OFF_AFTER_MS = 60000;  // STA up this long (and no portal users) -> portal down

static DNSServer dns;
static bool apOn = false;
static bool staWasUp = false;
static uint32_t staDownSince = 0, staUpSince = 0;
static int8_t tryIdx = -1;                 // saved network being tried, -1 = idle
static uint32_t tryStarted = 0, nextCycleAt = 0;
static volatile uint32_t lastNtpSync = 0;  // millis() of last SNTP sync, 0 = never
static String pendingForget;               // set by the web task, applied in netLoop
static volatile bool forgetPending = false;

static String keySsid(uint8_t i) { return i ? "s" + String(i) : String("ssid"); }
static String keyPass(uint8_t i) { return i ? "p" + String(i) : String("pass"); }

static void loadNets() {
    Preferences p;
    p.begin("ti", true);
    netCount = 0;
    for (uint8_t i = 0; i < MAX_NETS; i++) {
        if (!p.isKey(keySsid(i).c_str())) continue;
        String s = p.getString(keySsid(i).c_str(), "");
        if (!s.length()) continue;
        netSsid[netCount] = s;
        netPass[netCount] = p.getString(keyPass(i).c_str(), "");
        netCount++;
    }
    p.end();
}

static void storeNets() {
    Preferences p;
    p.begin("ti", false);
    for (uint8_t i = 0; i < MAX_NETS; i++) {
        if (i < netCount) {
            p.putString(keySsid(i).c_str(), netSsid[i]);
            p.putString(keyPass(i).c_str(), netPass[i]);
        } else {
            if (p.isKey(keySsid(i).c_str())) p.remove(keySsid(i).c_str());
            if (p.isKey(keyPass(i).c_str())) p.remove(keyPass(i).c_str());
        }
    }
    p.end();
}

static void removeNet(const String& ssid) {
    for (uint8_t i = 0; i < netCount; i++) {
        if (netSsid[i] != ssid) continue;
        for (uint8_t j = i; j + 1 < netCount; j++) {
            netSsid[j] = netSsid[j + 1];
            netPass[j] = netPass[j + 1];
        }
        netCount--;
        return;
    }
}

bool netIsAP()  { return apOn; }
bool netIsSta() { return WiFi.status() == WL_CONNECTED; }
String netIP()  { return netIsSta() ? WiFi.localIP().toString() : WiFi.softAPIP().toString(); }
int32_t netNtpAgeS() { return lastNtpSync ? (int32_t)((millis() - lastNtpSync) / 1000) : -1; }
uint8_t netSavedCount() { return netCount; }
String netSavedSsid(uint8_t i) { return i < netCount ? netSsid[i] : String(); }

static void onNtpSync(struct timeval*) {
    uint32_t m = millis();
    lastNtpSync = m ? m : 1;
}

// Sets TZ and (re)starts SNTP. Safe without an uplink: SNTP just retries.
void netApplyTz() {
    configTzTime(cfg.tz, "pool.ntp.org", "time.google.com", "time.cloudflare.com");
}

void netSaveCreds(const String& ssid, const String& pass) {
    if (!ssid.length()) return;
    removeNet(ssid);
    if (netCount == MAX_NETS) netCount--;            // drop the oldest
    for (uint8_t j = netCount; j > 0; j--) {
        netSsid[j] = netSsid[j - 1];
        netPass[j] = netPass[j - 1];
    }
    netSsid[0] = ssid;
    netPass[0] = pass;
    netCount++;
    storeNets();
}

void netForget(const String& ssid) {
    pendingForget = ssid;
    forgetPending = true;
}

static void apStart() {
    if (apOn) return;
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("TIME-INVADERS", "insertcoin");
    dns.start(53, "*", WiFi.softAPIP());
    apOn = true;
    Serial.printf("[net] portal up %s (pw: insertcoin)\n", WiFi.softAPIP().toString().c_str());
}

static void apStop() {
    if (!apOn) return;
    dns.stop();
    WiFi.softAPdisconnect(true);   // drops the AP half only; STA stays up
    apOn = false;
    Serial.println("[net] portal down");
}

static void startTry(uint8_t i) {
    tryIdx = i;
    tryStarted = millis();
    Serial.printf("[net] trying %s\n", netSsid[i].c_str());
    WiFi.begin(netSsid[i].c_str(), netPass[i].c_str());
}

void netSetup() {
    loadNets();
    sntp_set_time_sync_notification_cb(onNtpSync);
    setenv("TZ", cfg.tz, 1);        // local time must be right even with no WiFi;
    tzset();                        // SNTP itself starts once a network is joined
    WiFi.persistent(false);         // background retries must not rewrite flash
    WiFi.setAutoReconnect(false);   // netLoop owns reconnection
    WiFi.setHostname(cfg.hostname);
    WiFi.mode(WIFI_STA);
    if (netCount) {
        startTry(0);
        while (WiFi.status() != WL_CONNECTED && millis() - tryStarted < TRY_MS) {
            delay(250);
            Serial.print('.');
        }
        Serial.println();
    }
    if (WiFi.status() == WL_CONNECTED) {
        staWasUp = true;
        staUpSince = millis();
        tryIdx = -1;
        Serial.printf("[net] STA %s on %s\n", WiFi.localIP().toString().c_str(), WiFi.SSID().c_str());
        netApplyTz();               // restart SNTP now the uplink exists
    } else {
        WiFi.disconnect();
        tryIdx = -1;
        nextCycleAt = millis() + CYCLE_GAP_MS;
        apStart();
    }
    MDNS.begin(cfg.hostname);
    MDNS.addService("http", "tcp", 80);
}

void netLoop() {
    if (apOn) dns.processNextRequest();

    if (forgetPending) {
        forgetPending = false;
        removeNet(pendingForget);
        storeNets();
        tryIdx = -1;
    }

    uint32_t now = millis();
    bool up = WiFi.status() == WL_CONNECTED;
    if (up && !staWasUp) {
        staWasUp = true;
        staUpSince = now;
        tryIdx = -1;
        Serial.printf("[net] STA %s on %s\n", WiFi.localIP().toString().c_str(), WiFi.SSID().c_str());
        netApplyTz();               // sync now instead of waiting out SNTP's backoff
    } else if (!up && staWasUp) {
        staWasUp = false;
        staDownSince = now;
        tryIdx = -1;
        nextCycleAt = now;          // start retrying straight away
        Serial.println("[net] STA lost");
    }

    if (up) {
        if (apOn && now - staUpSince > AP_OFF_AFTER_MS && WiFi.softAPgetStationNum() == 0) apStop();
        return;
    }

    if (!apOn && now - staDownSince > AP_AFTER_MS) apStart();
    if (!netCount) return;

    if (tryIdx >= 0) {
        if (now - tryStarted < TRY_MS) return;
        WiFi.disconnect();
        if (tryIdx + 1 < netCount) startTry(tryIdx + 1);
        else { tryIdx = -1; nextCycleAt = now + CYCLE_GAP_MS; }
        return;
    }
    if ((int32_t)(now - nextCycleAt) < 0) return;
    // Connecting scans every channel and would knock a phone off the portal,
    // so hold off while someone is using it.
    if (apOn && WiFi.softAPgetStationNum() > 0) { nextCycleAt = now + 5000; return; }
    startTry(0);
}
