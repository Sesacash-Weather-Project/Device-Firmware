/*

  Main module

  # Modified by Kyle T. Gabriel to fix issue with incorrect GPS data for TTNMapper

  Copyright (C) 2018 by Xose Pérez <xose dot perez at gmail dot com>

  This program nis free software: you can redistribute it and/or modify
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

#define APP_NAME                "Kanda WxBalloons"
#define APP_VERSION             "0.3.3"

#include <Arduino.h>
#include <lmic.h>
void ttn_register(void (*callback)(uint8_t message));

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "config_hardware.h"
#include "config_user.h"
#include "rom/rtc.h"
#include <TinyGPS++.h>
#include <Wire.h>
#include <Math.h>

#include <LoraMessage.h>
#include <LoraEncoder.h>

#include <WiFi.h>

bool SSD1306_FOUND = false;
bool BME280_FOUND = false;
bool SHT21_FOUND = false;
bool SPL06_FOUND = false;
bool AXP192_FOUND = false;
bool RELAIS_ON = false;

// Message _counter, stored in RTC memory, survives deep sleep
RTC_DATA_ATTR uint32_t COUNT = 0;

// BME 280 environment
float TEMPERATURE_NOW; // celcius
float HUMIDITY_NOW; // percent (range 0 to 100)
float PRESSURE_NOW; // hpa
float V_TEMPERATURE_NOW; // kelvin
float ELEVATION_NOW; // meters
  
float V_TEMPERATURE_LAST;
float ELEVATION_LAST;
float PRESSURE_LAST;

// Calculated at each step based on pressure
int SEND_INTERVAL;
bool GATEWAY_IN_RANGE;
String DOWNLINK_RESPONSE = "";

// Launch-specific values
float SFC_PRESSURE = 0.0;
bool IF_WEB_OPENED = false;
bool IF_LAUNCHED = false;
uint8_t LAUNCH_ID[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } ; // For launch_id downlink

// Set web server port number to 80
WiFiServer SERVER(80);
WiFiClient CLIENT;


// -----------------------------------------------------------------------------
// Application
// -----------------------------------------------------------------------------



//////////////////////////////////////////
///////////// ARDUINO SETUP //////////////
//////////////////////////////////////////
void setup() {
  
  // Debug
  #ifdef DEBUG_PORT
  DEBUG_PORT.begin(SERIAL_BAUD);
  #endif

  while ( millis() < 500 ); // Brief wait for serial baud
  
  Wire.begin(I2C_SDA, I2C_SCL);
  scanI2Cdevice();

  // Buttons & LED
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Hello
  DEBUG_MSG(APP_NAME " " APP_VERSION "\n");

  // Display
  screen_setup();

  Serial.begin(115200);
  
  #ifndef IGNORE_TPRH_CHECK
  if (!if_T_and_RH_sensor() || !if_P_sensor())
  {
    screen_print("[ERR]\n");
    screen_print("No weather sensor \n");
    screen_print("connected! Check wiring.");
    Serial.println("[ERR] No weather sensor connected!");
    delay(MESSAGE_TO_SLEEP_DELAY);
    screen_off();
    sleep_forever();
  }
  #endif

  // Init BME280
  bme_setup();

  // Init GPS
  gps_setup();
  //GPS_SetHAB();

  // TTN setup
  if (!ttn_setup()) {
    screen_print("[ERR] Radio module not found!\n");
    Serial.println("[ERR] Radio module not found!");
    delay(MESSAGE_TO_SLEEP_DELAY);
    screen_off();
    sleep_forever();
  }

  ttn_register(callback);
  ttn_join();
  ttn_sf(LORAWAN_SF);
  ttn_adr(LORAWAN_ADR);
  if(!LORAWAN_ADR){
    LMIC_setLinkCheckMode(0); // Link check problematic if not using ADR. Must be set after join
  }

  // Request confirmed status message
  send_status(true);

  // Show logo on first boot (if OLED attached)
  screen_print(APP_NAME " " APP_VERSION, 0, 0 ); // print firmware version in upper left corner
  screen_show_logo();
  screen_update();
  while ( millis() < 5000 ) // Wait 5 seconds, but don't block
    os_runloop_once();

  // Send second confirmed message (and receive possible downlink)
  send_status(true);
  while ( millis() < 10000 ) // Wait another 5 seconds, but don't block
    os_runloop_once();

  // Do this, because the device may have simply restarted on its own
  checkIfLaunched();  

  if ( IF_LAUNCHED )
  {
    set_send_interval( SFC_PRESSURE );
    return; // skip all the WiFi functions and go straight to Arduino loop
  }

  // Check that at least one confirmed message was received
  #ifndef IGNORE_GATEWAY_CHECK
  if (!GATEWAY_IN_RANGE) 
  {
    if (!IF_LAUNCHED) // Do this check, because a launched balloon may not hear downlink
    {
      screen_print("[ERR]\n");
      screen_print("Cannot connect to TTN!\n");
      Serial.println("[ERR] TTN network not connected!");
      delay(MESSAGE_TO_SLEEP_DELAY);
      screen_off();
      sleep_forever();
    }
  }
  #endif

  //screen_show_qrcode();
  //screen_update();
  screen_print("TTN Connected!\n");
  screen_print("WiFi AP:  AscensionWx\n");
  screen_print("  password:  balloons\n");
  screen_print("\n");
  screen_print("Then go to  192.168.4.1\n");

  // Init wifi access point
  WiFi.softAP(SSID, PASSWORD);

  // Defines how async server handles GET requests
  //initServer();
  SERVER.begin();

  SEND_INTERVAL = 30000; // Status message this many millis
  uint32_t last = 0;
  while ( !IF_LAUNCHED ) {

    os_runloop_once();

    checkServer();
    gps_read();
    screen_loop();

    if ( millis() - last > SEND_INTERVAL) {
      
      // Check if pressure is 1 mb different
      checkIfLaunched();

      #ifdef DEBUG_MODE
      if (millis() > 120000) {  // In debug mode, we start broadcasting observations after 2 minutes
         IF_LAUNCHED = true;
      }
      #endif

      // Send status message every 15 seconds
      send_status();
      
      // Store for next iteration
      last = millis();
      
    } // End 15 second loop
  }// End IF_LAUNCHED check

  // Reset send interval for observations
  set_send_interval( SFC_PRESSURE );

  // Close access to the server
  SERVER.end();
  // Ends wifi connection completely
  WiFi.softAPdisconnect(true);
  
}// End Arduino setup() method

/////////////////////////////////////////
///////////// ARDUINO LOOP //////////////
/////////////////////////////////////////
void loop() {
  gps_read();
  ttn_loop();
  screen_loop();

  static uint32_t last = 0;
  static bool first = true;
  static uint32_t last_bme = 0;
  static bool first_bme = true;

  if (0 == last_bme || millis() - last_bme > BME_INTERVAL) {
    last_bme = millis();
    first_bme = millis();

    bme_loop(); // Gets BME data and calculates alternative elevation
  }
  
  if (0 == last || millis() - last > SEND_INTERVAL) {

    // Do a BME loop
    bme_loop();
    
    if (0 < gps_hdop() && gps_hdop() < 50 && gps_latitude() != 0 && gps_longitude() != 0) {
      // GPS Lock successful
    } else {
      if (first) {
        screen_print("Waiting GPS lock\n");
        first = false;
      }
    }

    // Will send, even if GPS doesn't have a lock
    last = millis();
    first = false;
    Serial.println("TRANSMITTING");
    send_observation();
    set_send_interval( PRESSURE_NOW );

    // After we send, we should store the last variables
    PRESSURE_LAST = PRESSURE_NOW;
    V_TEMPERATURE_LAST = V_TEMPERATURE_NOW;
    ELEVATION_LAST = ELEVATION_NOW;
  }
}
