#include "config.h"
#include <Preferences.h>

Config cfg;
static Preferences prefs;

void configLoad() {
    prefs.begin("ti", true);
    cfg.brightness     = prefs.getUChar ("bri",   cfg.brightness);
    cfg.nightEnabled   = prefs.getBool  ("nEn",   cfg.nightEnabled);
    cfg.nightBright    = prefs.getUChar ("nBri",  cfg.nightBright);
    cfg.nightFrom      = prefs.getUChar ("nFrom", cfg.nightFrom);
    cfg.nightTo        = prefs.getUChar ("nTo",   cfg.nightTo);
    cfg.powerMa        = prefs.getUShort("pwr",   cfg.powerMa);
    cfg.usbSafeMa      = prefs.getUShort("usbMa", cfg.usbSafeMa);
    cfg.theme          = prefs.getUChar ("thm",   cfg.theme);
    cfg.timeColor      = prefs.getULong ("tCol",  cfg.timeColor);
    cfg.accentColor    = prefs.getULong ("aCol",  cfg.accentColor);
    cfg.animsEnabled   = prefs.getBool  ("aEn",   cfg.animsEnabled);
    cfg.transStyle     = prefs.getUChar ("trS",   cfg.transStyle);
    cfg.attractEnabled = prefs.getBool  ("atEn",  cfg.attractEnabled);
    cfg.attractMin     = prefs.getUShort("atMin", cfg.attractMin);
    cfg.wordsEnabled   = prefs.getBool  ("wEn",   cfg.wordsEnabled);
    cfg.wordsMin       = prefs.getUShort("wMin",  cfg.wordsMin);
    cfg.pacAmbient     = prefs.getBool  ("pac",   cfg.pacAmbient);
    cfg.mapRotate      = prefs.getUChar ("mRot",  cfg.mapRotate);
    cfg.mapSerp        = prefs.getBool  ("mSer",  cfg.mapSerp);
    cfg.mapFlip        = prefs.getBool  ("mFlp",  cfg.mapFlip);
    prefs.getString("tz",   cfg.tz,       sizeof(cfg.tz));
    prefs.getString("host", cfg.hostname, sizeof(cfg.hostname));
    prefs.end();
}

void configSave() {
    prefs.begin("ti", false);
    prefs.putUChar ("bri",   cfg.brightness);
    prefs.putBool  ("nEn",   cfg.nightEnabled);
    prefs.putUChar ("nBri",  cfg.nightBright);
    prefs.putUChar ("nFrom", cfg.nightFrom);
    prefs.putUChar ("nTo",   cfg.nightTo);
    prefs.putUShort("pwr",   cfg.powerMa);
    prefs.putUShort("usbMa", cfg.usbSafeMa);
    prefs.putUChar ("thm",   cfg.theme);
    prefs.putULong ("tCol",  cfg.timeColor);
    prefs.putULong ("aCol",  cfg.accentColor);
    prefs.putBool  ("aEn",   cfg.animsEnabled);
    prefs.putUChar ("trS",   cfg.transStyle);
    prefs.putBool  ("atEn",  cfg.attractEnabled);
    prefs.putUShort("atMin", cfg.attractMin);
    prefs.putBool  ("wEn",   cfg.wordsEnabled);
    prefs.putUShort("wMin",  cfg.wordsMin);
    prefs.putBool  ("pac",   cfg.pacAmbient);
    prefs.putUChar ("mRot",  cfg.mapRotate);
    prefs.putBool  ("mSer",  cfg.mapSerp);
    prefs.putBool  ("mFlp",  cfg.mapFlip);
    prefs.putString("tz",    cfg.tz);
    prefs.putString("host",  cfg.hostname);
    prefs.end();
}
