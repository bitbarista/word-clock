// =================================================================
// Programming-port power guard
// -----------------------------------------------------------------
// The supermini's own USB-C (poking out the side, for flashing) is
// wired straight into the shared 5V rail through the board's onboard
// Schottky. That diode stops the rail from back-feeding INTO a
// laptop's port, but it does nothing in the other direction: if the
// laptop's VBUS happens to sit above the pod's regulated output
// (very possible — laptop ports commonly run 5.05-5.25V), the diode
// forward-conducts and the laptop ends up sourcing panel current
// through a diode and cable never sized for that, regardless of
// whether the pod is also plugged in.
//
// Detection: tud_mounted() reflects real USB-bus enumeration — true
// for the entire time a host (PC/laptop) has the port open, not just
// while a serial monitor is attached, and it can only go true when
// D+/D- are actually driven by a host. The pod is power-only (no
// data lines), so it can never trigger this — the signal is clean.
//
// Response: while a host is present, clamp the FastLED power budget
// to cfg.usbSafeMa (default 400mA — comfortably inside a standard
// USB2 500mA allocation with headroom for the S3 itself) regardless
// of cfg.powerMa. This reuses the existing per-frame limiter, so no
// animation needs special-casing: whatever frame would have been
// drawn just gets scaled down to fit, exactly like the normal
// power-budget mechanism already does.
// =================================================================
#pragma once
#include <Arduino.h>
extern "C" {
#include "tusb.h"
}

inline bool usbHostPresent() {
    return tud_inited() && tud_mounted();
}
