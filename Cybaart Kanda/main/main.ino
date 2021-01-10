/*

  Main module

  # Modified by Kyle T. Gabriel to fix issue with incorrect GPS data for TTNMapper

  Copyright (C) 2018 by Xose Pérez <xose dot perez at gmail dot com>

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

#include "configuration.h"
#include "rom/rtc.h"
#include <TinyGPS++.h>
#include <Wire.h>
#include <Math.h>
//#include <esp_wifi.h>

#include <LoraMessage.h>
#include <LoraEncoder.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>


#ifdef T_BEAM_V10
#include "axp20x.h"
AXP20X_Class axp;
bool pmu_irq = false;
String baChStatus = "No charging";
#endif

bool _ssd1306_found = false;
bool _axp192_found = false;
bool _BME280_found = false;
bool _relais_on = false;

// Message _counter, stored in RTC memory, survives deep sleep
RTC_DATA_ATTR uint32_t _count = 0;

// BME 280 environment
float _temperature_now; // celcius
float _humidity_now; // percent (range 0 to 100)
float _pressure_now; // hpa
float _v_temperature_now; // kelvin
float _elevation_now; // meters
  
float _v_temperature_last;
float _elevation_last;
float _pressure_last;

// Calculated at each step based on pressure
int _send_interval;

// Launch-specific values
float _sfc_pressure = 0.0;
bool _ifWebOpen = false;
bool _ifSetMiner = false;
bool _ifAuthenticated = false;
bool _ifLaunched = false;
uint8_t _launch_id[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } ; // For launch_id downlink
uint64_t _miner_account = 0;

// Set web server port number to 80
AsyncWebServer _server(80);

// -----------------------------------------------------------------------------
// Application
// -----------------------------------------------------------------------------

void sleep() {
#if SLEEP_BETWEEN_MESSAGES

  // Show the going to sleep message on the screen
  char buffer[20];
  snprintf(buffer, sizeof(buffer), "Sleeping in %3.1fs\n", (MESSAGE_TO_SLEEP_DELAY / 1000.0));
  screen_print(buffer);

  // Wait for MESSAGE_TO_SLEEP_DELAY millis to sleep
  delay(MESSAGE_TO_SLEEP_DELAY);

  // Turn off screen
  screen_off();

  // Set the user button to wake the board
  sleep_interrupt(BUTTON_PIN, LOW);

  // We sleep for the interval between messages minus the current millis
  // this way we distribute the messages evenly every _send_interval millis
  uint32_t sleep_for = (millis() < _send_interval) ? _send_interval - millis() : _send_interval;
  sleep_millis(sleep_for);

#endif
}

void scanI2Cdevice(void)
{
    byte err, addr;
    int nDevices = 0;
    for (addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        err = Wire.endTransmission();
        if (err == 0) {
            Serial.print("I2C device found at address 0x");
            if (addr < 16)
                Serial.print("0");
            Serial.print(addr, HEX);
            Serial.println(" !");
            nDevices++;

            if (addr == SSD1306_ADDRESS) {
                _ssd1306_found = true;
                Serial.println("ssd1306 display found");
            }
            if (addr == AXP192_SLAVE_ADDRESS) {
                _axp192_found = true;
                Serial.println("axp192 PMU found");
            }
            if (addr == BME280_ADDRESS) {
                _BME280_found = true;
                Serial.println("BME280 sensor found");
            }
        } else if (err == 4) {
            Serial.print("Unknow error at address 0x");
            if (addr < 16)
                Serial.print("0");
            Serial.println(addr, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");
}

//////////////////////////////////////////
///////////// ARDUINO SETUP //////////////
//////////////////////////////////////////
void setup() {

  
  // Debug
  #ifdef DEBUG_PORT
  DEBUG_PORT.begin(SERIAL_BAUD);
  #endif

  delay(1000);

  #ifdef T_BEAM_V10
  Wire.begin(I2C_SDA, I2C_SCL);
  scanI2Cdevice();
  _axp192_found = true;
  if (_axp192_found) {
      if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS)) {
          Serial.println("AXP192 Begin PASS");
      } else {
          Serial.println("AXP192 Begin FAIL");
      }
      // axp.setChgLEDMode(LED_BLINK_4HZ);
      Serial.printf("DCDC1: %s\n", axp.isDCDC1Enable() ? "ENABLE" : "DISABLE");
      Serial.printf("DCDC2: %s\n", axp.isDCDC2Enable() ? "ENABLE" : "DISABLE");
      Serial.printf("LDO2: %s\n", axp.isLDO2Enable() ? "ENABLE" : "DISABLE");
      Serial.printf("LDO3: %s\n", axp.isLDO3Enable() ? "ENABLE" : "DISABLE");
      Serial.printf("DCDC3: %s\n", axp.isDCDC3Enable() ? "ENABLE" : "DISABLE");
      Serial.printf("Exten: %s\n", axp.isExtenEnable() ? "ENABLE" : "DISABLE");
      Serial.println("----------------------------------------");

      axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
      axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
      axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
      axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
      axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
      axp.setDCDC1Voltage(3300);

      Serial.printf("DCDC1: %s\n", axp.isDCDC1Enable() ? "ENABLE" : "DISABLE");
      Serial.printf("DCDC2: %s\n", axp.isDCDC2Enable() ? "ENABLE" : "DISABLE");
      Serial.printf("LDO2: %s\n", axp.isLDO2Enable() ? "ENABLE" : "DISABLE");
      Serial.printf("LDO3: %s\n", axp.isLDO3Enable() ? "ENABLE" : "DISABLE");
      Serial.printf("DCDC3: %s\n", axp.isDCDC3Enable() ? "ENABLE" : "DISABLE");
      Serial.printf("Exten: %s\n", axp.isExtenEnable() ? "ENABLE" : "DISABLE");

      pinMode(PMU_IRQ, INPUT_PULLUP);
      attachInterrupt(PMU_IRQ, [] {
          pmu_irq = true;
      }, FALLING);

      axp.adc1Enable(AXP202_BATT_CUR_ADC1, 1);
      axp.enableIRQ(AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_BATT_REMOVED_IRQ | AXP202_BATT_CONNECT_IRQ, 1);
      axp.clearIRQ();

      if (axp.isChargeing()) {
          baChStatus = "Charging";
      }
  } else {
      Serial.println("AXP192 not found");
  }
  #endif

  // Buttons & LED
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  // Hello
  DEBUG_MSG(APP_NAME " " APP_VERSION "\n");

  // Display
  screen_setup();

  // Init BME280
  bme_setup();

  // Init GPS
  gps_setup();
  GPS_SetHAB();

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

  // Show logo on first boot (if OLED attached)
  screen_show_logo();
  screen_update();
  delay(LOGO_DELAY);

  screen_show_qrcode();
  screen_update();

  // Init wifi access point
  WiFi.softAP(SSID, PASSWORD);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Defines how async server handles GET requests
  initServer();
  _server.begin();

  _send_interval = 15000; // Status message every 15 seconds
  uint32_t last = 0;
  while ( !_ifLaunched ) {

    if ( last == 0 || millis() - last > _send_interval) {
      // Check if pressure is 2 mb different
      if ( get_pressure() + 2.0 < _sfc_pressure )
      {
        _ifLaunched = true;
      }

      // Used only for testing purposes. NEEDS REMOVAL!
      if (millis() > 60000) {  // Trigger observations after 1 min no matter what
         _ifLaunched = true;
      }

      // Send status message every 15 seconds
      send_status();
      
      // Store for next iteration
      last = millis();
      
    } // End 15 second loop
  }// End _ifLaunched check

  // Reset send interval for observations
  set_send_interval( _sfc_pressure );

  // Close access to the server
  _server.end();
  // Ends wifi connection completely
  WiFi.softAPdisconnect(true);
  
}// End Arduino setup() method

/////////////////////////////////////////
///////////// ARDUINO LOOP //////////////
/////////////////////////////////////////
void loop() {
  gps_loop();
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
  
  if (0 == last || millis() - last > _send_interval) {

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
    set_send_interval( _pressure_now );

    // After we send, we should store the last variables
    _pressure_last = _pressure_now;
    _v_temperature_last = _v_temperature_now;
    _elevation_last = _elevation_now;
  }
}
