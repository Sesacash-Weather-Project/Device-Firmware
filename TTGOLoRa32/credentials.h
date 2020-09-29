/*

Credentials file

*/

#pragma once

// Only one of these settings must be defined
#define USE_ABP
//#define USE_OTAA

#ifdef USE_ABP

// UPDATE WITH YOUR TTN KEYS AND ADDR. Should be MSB
static const PROGMEM u1_t NWKSKEY[16] = { 0x32, 0x09, 0x90, 0xAE, 0xCA, 0xA3, 0x50, 0x0E, 0xD8, 0xCE, 0x2C, 0x95, 0xDD, 0x6C, 0x1F, 0xDC };
static const u1_t PROGMEM APPSKEY[16] = { 0x37, 0xEF, 0x2A, 0xE0, 0xD0, 0xDC, 0xB3, 0xA0, 0xE3, 0x6B, 0xAA, 0x97, 0x80, 0x8C, 0xBB, 0x78 };
static const u4_t DEVADDR = 0x26013884; // <-- Change this address for every node! Appears to be MSB (Most significant byte)

#endif

#ifdef USE_OTAA

    // This EUI must be in little-endian format, so least-significant-byte
    // first. When copying an EUI from ttnctl output, this means to reverse
    // the bytes. For TTN issued EUIs the last bytes should be 0x00, 0x00,
    // 0x00.
    static const u1_t PROGMEM APPEUI[8]  = { 0x40, 0x43, 0x03, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };

    // This should also be in little endian format, see above. (least-significant-byte
    // first)
    static const u1_t PROGMEM DEVEUI[8]  = { 0x65, 0xFA, 0xF4, 0x01, 0x88, 0x88, 0xD1, 0x00 };

    // This key should be in big endian format (or, since it is not really a
    // number but a block of memory, endianness does not really apply). In
    // practice, a key taken from ttnctl can be copied as-is.
    // The key shown here is the semtech default key.
    static const u1_t PROGMEM APPKEY[16] = { 0xEF, 0xCE, 0x93, 0xC9, 0x3E, 0x4C, 0x30, 0x8B, 0x90, 0xE8, 0x5E, 0xB1, 0x7C, 0x70, 0x10, 0xD4 };

#endif
