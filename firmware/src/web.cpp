#include "web.h"
#include "config.h"
#include "display.h"
#include "network.h"
#include "webui.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Update.h>
#include <time.h>

static AsyncWebServer server(80);
volatile bool g_reboot = false;

static String colHex(uint32_t v) {
    char b[8];
    snprintf(b, sizeof(b), "#%06x", (unsigned)(v & 0xFFFFFF));
    return String(b);
}
static uint32_t hexCol(const char* s) {
    if (s && *s == '#') s++;
    return s ? (uint32_t)strtoul(s, nullptr, 16) : 0;
}

static void sendJson(AsyncWebServerRequest* req, JsonDocument& doc) {
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
}

static void handleStatus(AsyncWebServerRequest* req) {
    JsonDocument doc;
    time_t t = time(nullptr);
    struct tm tmv;
    localtime_r(&t, &tmv);
    char hm[6];
    snprintf(hm, sizeof(hm), "%02d:%02d", tmv.tm_hour, tmv.tm_min);
    doc["time"]    = timeValid() ? hm : "--:--";
    doc["phrase"]  = currentPhrase();
    doc["hasTime"] = timeValid();
    doc["epoch"]   = (uint32_t)t;
    doc["ntpAge"]  = netNtpAgeS();
    doc["ip"]      = netIP();
    doc["ap"]      = netIsAP();
    doc["sta"]     = netIsSta();
    doc["ssid"]    = netIsSta() ? WiFi.SSID() : String();
    doc["rssi"]    = netIsSta() ? WiFi.RSSI() : 0;
    doc["host"]    = cfg.hostname;
    doc["fw"]      = FW_VERSION;
    doc["usbLimited"] = usbPowerLimited();
    sendJson(req, doc);
}

static void handleGetConfig(AsyncWebServerRequest* req) {
    JsonDocument doc;
    doc["brightness"]  = cfg.brightness;
    doc["nightEnabled"]= cfg.nightEnabled;
    doc["nightBright"] = cfg.nightBright;
    doc["nightFrom"]   = cfg.nightFrom;
    doc["nightTo"]     = cfg.nightTo;
    doc["powerMa"]     = cfg.powerMa;
    doc["usbSafeMa"]   = cfg.usbSafeMa;
    doc["theme"]       = cfg.theme;
    doc["timeColor"]   = colHex(cfg.timeColor);
    doc["accentColor"] = colHex(cfg.accentColor);
    doc["animsEnabled"]   = cfg.animsEnabled;
    doc["transStyle"]     = cfg.transStyle;
    doc["attractEnabled"] = cfg.attractEnabled;
    doc["attractMin"]     = cfg.attractMin;
    doc["wordsEnabled"] = cfg.wordsEnabled;
    doc["wordsMin"]     = cfg.wordsMin;
    doc["pacAmbient"]   = cfg.pacAmbient;
    doc["mapRotate"]    = cfg.mapRotate;
    doc["mapSerp"]      = cfg.mapSerp;
    doc["mapFlip"]      = cfg.mapFlip;
    doc["tz"]           = cfg.tz;
    doc["hostname"]     = cfg.hostname;
    sendJson(req, doc);
}

static void applyConfigJson(JsonDocument& doc) {
    if (doc["brightness"].is<int>())   cfg.brightness  = doc["brightness"];
    if (doc["nightEnabled"].is<bool>())cfg.nightEnabled= doc["nightEnabled"];
    if (doc["nightBright"].is<int>())  cfg.nightBright = doc["nightBright"];
    if (doc["nightFrom"].is<int>())    cfg.nightFrom   = doc["nightFrom"];
    if (doc["nightTo"].is<int>())      cfg.nightTo     = doc["nightTo"];
    if (doc["powerMa"].is<int>())    { cfg.powerMa     = doc["powerMa"]; applyPower(); }
    if (doc["usbSafeMa"].is<int>()) { cfg.usbSafeMa   = doc["usbSafeMa"]; applyPower(); }
    if (doc["theme"].is<int>())        cfg.theme       = doc["theme"];
    if (doc["timeColor"].is<const char*>())   cfg.timeColor   = hexCol(doc["timeColor"]);
    if (doc["accentColor"].is<const char*>()) cfg.accentColor = hexCol(doc["accentColor"]);
    if (doc["animsEnabled"].is<bool>())   cfg.animsEnabled   = doc["animsEnabled"];
    if (doc["transStyle"].is<int>())      cfg.transStyle     = doc["transStyle"];
    if (doc["attractEnabled"].is<bool>()) cfg.attractEnabled = doc["attractEnabled"];
    if (doc["attractMin"].is<int>())      cfg.attractMin     = doc["attractMin"];
    if (doc["wordsEnabled"].is<bool>())   cfg.wordsEnabled   = doc["wordsEnabled"];
    if (doc["wordsMin"].is<int>())        cfg.wordsMin       = doc["wordsMin"];
    if (doc["pacAmbient"].is<bool>())     cfg.pacAmbient     = doc["pacAmbient"];
    if (doc["mapRotate"].is<int>())       cfg.mapRotate      = doc["mapRotate"];
    if (doc["mapSerp"].is<bool>())        cfg.mapSerp        = doc["mapSerp"];
    if (doc["mapFlip"].is<bool>())        cfg.mapFlip        = doc["mapFlip"];
    if (doc["tz"].is<const char*>())    { strlcpy(cfg.tz, doc["tz"], sizeof(cfg.tz)); netApplyTz(); }
    if (doc["hostname"].is<const char*>()) strlcpy(cfg.hostname, doc["hostname"], sizeof(cfg.hostname));
    configSave();
}

// accumulate a JSON body, then hand the parsed doc to `fn`
typedef void (*JsonHandler)(AsyncWebServerRequest*, JsonDocument&);
static ArBodyHandlerFunction jsonBody(JsonHandler fn) {
    return [fn](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t index, size_t total) {
        if (index == 0) req->_tempObject = new String();
        String* body = (String*)req->_tempObject;
        body->concat((const char*)data, len);
        if (index + len == total) {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, *body);
            delete body;
            req->_tempObject = nullptr;
            if (err) { req->send(400, "application/json", "{\"err\":\"bad json\"}"); return; }
            fn(req, doc);
        }
    };
}

void webSetup() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(200, "text/html", INDEX_HTML);
    });

    server.on("/api/status", HTTP_GET, handleStatus);
    server.on("/api/config", HTTP_GET, handleGetConfig);

    server.on("/api/config", HTTP_POST, [](AsyncWebServerRequest*) {}, nullptr,
        jsonBody([](AsyncWebServerRequest* req, JsonDocument& doc) {
            applyConfigJson(doc);
            req->send(200, "application/json", "{\"ok\":true}");
        }));

    server.on("/api/action", HTTP_POST, [](AsyncWebServerRequest*) {}, nullptr,
        jsonBody([](AsyncWebServerRequest* req, JsonDocument& doc) {
            String what = doc["do"] | "";
            int param = doc["param"] | -1;
            if      (what == "transition") { reqParam = param; reqKind = RQ_TRANSITION; }
            else if (what == "attract")    { reqParam = param; reqKind = RQ_ATTRACT; }
            else if (what == "words")      { reqParam = param; reqKind = RQ_WORDS; }
            else if (what == "maptest")    { reqKind = RQ_MAPTEST; }
            else if (what == "identify")   { reqKind = RQ_IDENTIFY; }
            req->send(200, "application/json", "{\"ok\":true}");
        }));

    server.on("/api/time", HTTP_POST, [](AsyncWebServerRequest*) {}, nullptr,
        jsonBody([](AsyncWebServerRequest* req, JsonDocument& doc) {
            uint32_t epoch = doc["epoch"] | 0;
            if (epoch > 1600000000) {
                struct timeval tv = { (time_t)epoch, 0 };
                settimeofday(&tv, nullptr);
            }
            req->send(200, "application/json", "{\"ok\":true}");
        }));

    server.on("/api/preview", HTTP_GET, [](AsyncWebServerRequest* req) {
        String out;
        out.reserve(NUM_CELLS * 6 + 2);
        for (uint16_t cell = 0; cell < NUM_CELLS; cell++) {
            const CRGB& c = leds[cellToLed(cell)];
            char b[7];
            snprintf(b, sizeof(b), "%02x%02x%02x", c.r, c.g, c.b);
            out += b;
        }
        req->send(200, "text/plain", out);
    });

    server.on("/api/scan", HTTP_GET, [](AsyncWebServerRequest* req) {
        int n = WiFi.scanComplete();
        if (n == WIFI_SCAN_FAILED) { WiFi.scanNetworks(true); req->send(200, "application/json", "{\"scanning\":true}"); return; }
        if (n == WIFI_SCAN_RUNNING) { req->send(200, "application/json", "{\"scanning\":true}"); return; }
        JsonDocument doc;
        JsonArray arr = doc["nets"].to<JsonArray>();
        for (int i = 0; i < n && i < 20; i++) {
            JsonObject o = arr.add<JsonObject>();
            o["ssid"] = WiFi.SSID(i);
            o["rssi"] = WiFi.RSSI(i);
            o["open"] = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
        }
        WiFi.scanDelete();
        sendJson(req, doc);
    });

    server.on("/api/wifi", HTTP_POST, [](AsyncWebServerRequest*) {}, nullptr,
        jsonBody([](AsyncWebServerRequest* req, JsonDocument& doc) {
            netSaveCreds(doc["ssid"] | "", doc["pass"] | "");
            req->send(200, "application/json", "{\"ok\":true,\"reboot\":true}");
            g_reboot = true;
        }));

    server.on("/api/networks", HTTP_GET, [](AsyncWebServerRequest* req) {
        JsonDocument doc;
        JsonArray arr = doc["saved"].to<JsonArray>();
        for (uint8_t i = 0; i < netSavedCount(); i++) arr.add(netSavedSsid(i));
        sendJson(req, doc);
    });

    server.on("/api/forget", HTTP_POST, [](AsyncWebServerRequest*) {}, nullptr,
        jsonBody([](AsyncWebServerRequest* req, JsonDocument& doc) {
            netForget(doc["ssid"] | "");
            req->send(200, "application/json", "{\"ok\":true}");
        }));

    server.on("/api/restart", HTTP_POST, [](AsyncWebServerRequest* req) {
        req->send(200, "application/json", "{\"ok\":true}");
        g_reboot = true;
    });

    // web OTA: upload a firmware .bin
    server.on("/update", HTTP_POST,
        [](AsyncWebServerRequest* req) {
            bool ok = !Update.hasError();
            req->send(200, "application/json", ok ? "{\"ok\":true}" : "{\"ok\":false}");
            if (ok) g_reboot = true;
        },
        [](AsyncWebServerRequest* req, String filename, size_t index, uint8_t* data, size_t len, bool final) {
            if (index == 0) Update.begin(UPDATE_SIZE_UNKNOWN);
            if (len) Update.write(data, len);
            if (final) Update.end(true);
        });

    // captive portal: anything unknown asked over the portal lands on the UI
    server.onNotFound([](AsyncWebServerRequest* req) {
        if (netIsAP() && req->client()->localIP() == WiFi.softAPIP()) req->redirect("/");
        else req->send(404, "text/plain", "not found");
    });

    server.begin();
}
