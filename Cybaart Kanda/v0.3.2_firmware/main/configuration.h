/*

TTGO T-BEAM Tracker for The Things Network

Copyright (C) 2018 by Xose Pérez <xose dot perez at gmail dot com>

This code requires LMIC library by Matthijs Kooijman
https://github.com/matthijskooijman/arduino-lmic

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#include <Arduino.h>
#include <lmic.h>
void ttn_register(void (*callback)(uint8_t message));

// -----------------------------------------------------------------------------
// Version
// -----------------------------------------------------------------------------

#define APP_NAME                "Kanda WxBalloons"
#define APP_VERSION             "0.3.2"

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

//#define DEBUG_MODE 1

// If using a single-channel gateway, uncomment this next option and set to your gateway's channel
//#define SINGLE_CHANNEL_GATEWAY  0

// If you are having difficulty sending messages to TTN after the first successful send,
// uncomment the next option and experiment with values (~ 1 - 5)
#define CLOCK_ERROR             1

#define DEBUG_PORT              Serial      // Serial debug port
#define SERIAL_BAUD             115200      // Serial debug baud rate

#define BME_INTERVAL            30000       // Query the BME (more often could mean better elevation_2)
#define MESSAGE_TO_SLEEP_DELAY  5000        // Time after message before going to sleep
#define LOGO_DELAY              5000        // Time to show logo on first boot
#define QRCODE_DELAY            10000        // Time to show logo on first boot
#define OBSERVATION_PORT        1          // Port the obs messages will be sent to
#define STATUS_PORT             2          // Port the status messages will be sent to
#define AUTHENTICATION_PORT     3          // Port for authentication message
#define LORAWAN_CONFIRMED_EVERY 0           // Send confirmed message every these many messages (0 means never)
#define LORAWAN_SF              DR_SF7     // Spreading factor
#define LORAWAN_ADR             0           // Enable ADR

#define SEND_INTERVAL_1000          5000      // Sleep for these many millis in between send()
#define SEND_INTERVAL_900           5000      
#define SEND_INTERVAL_800           30000     
#define SEND_INTERVAL_700           120000     
#define SEND_INTERVAL_600           120000
#define SEND_INTERVAL_500           120000
#define SEND_INTERVAL_400           120000
#define SEND_INTERVAL_300           120000
#define SEND_INTERVAL_200           120000

#define MISSING_UINT            0
#define MISSING_FLOAT           0

//Wifi defines
const char* SSID     = "AscensionWx";
const char* PASSWORD = "balloons";

// -----------------------------------------------------------------------------
// DEBUG
// -----------------------------------------------------------------------------

#ifdef DEBUG_PORT
#define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

// -----------------------------------------------------------------------------
// Custom messages
// -----------------------------------------------------------------------------

#define EV_QUEUED       100
#define EV_PENDING      101
#define EV_ACK          102
#define EV_RESPONSE     103

// -----------------------------------------------------------------------------
// General
// -----------------------------------------------------------------------------

#define I2C_SDA         21
#define I2C_SCL         22
#define LED_PIN         14
#define RELAIS_PIN         14   // Works with TTGO LoRa32 V2.1 board (confirmed)
#define BUTTON_PIN      39

// -----------------------------------------------------------------------------
// OLED
// -----------------------------------------------------------------------------

#define SSD1306_ADDRESS 0x3C

// -----------------------------------------------------------------------------
// BME280
// -----------------------------------------------------------------------------

#define BME280_ADDRESS_0x76 0x76
#define BME280_ADDRESS_0x77 0x77

// -----------------------------------------------------------------------------
// GPS
// -----------------------------------------------------------------------------

#define GPS_BAUDRATE    9600
#define USE_GPS         1
#define ss              Serial2 // define GPSserial as ESP32 Serial2
#define GPS_RX_PIN      12
#define GPS_TX_PIN      15

// -----------------------------------------------------------------------------
// LoRa SPI
// -----------------------------------------------------------------------------

#define SCK_GPIO        5
#define MISO_GPIO       19
#define MOSI_GPIO       27
#define NSS_GPIO        18
#define RESET_GPIO      23
#define DIO0_GPIO       26
#define DIO1_GPIO       33
#define DIO2_GPIO       32

// -----------------------------------------------------------------------------
// AXP192 (Rev1-specific options)
// -----------------------------------------------------------------------------

#define AXP192_SLAVE_ADDRESS  0x34
#define GPS_POWER_CTRL_CH     3
#define LORA_POWER_CTRL_CH    2
#define PMU_IRQ               35
