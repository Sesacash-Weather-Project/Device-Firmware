/*

  GPS module

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

#include <TinyGPS++.h>

const unsigned long GPS_WaitAck_mS = 2000;        //number of mS to wait for an ACK response from GPS
uint8_t GPS_Reply[12];                //byte array for storing GPS reply to UBX commands (12 bytes)

uint32_t LatitudeBinary;
uint32_t LongitudeBinary;
uint16_t altitudeGps;
uint8_t hdopGps;
uint8_t sats;
char t[32]; // used to sprintf for Serial output

TinyGPSPlus _gps;
HardwareSerial _serial_gps(GPS_SERIAL_NUM);

void gps_time(char * buffer, uint8_t size) {
    snprintf(buffer, size, "%02d:%02d:%02d", _gps.time.hour(), _gps.time.minute(), _gps.time.second());
}

float gps_latitude() {
    return _gps.location.lat();
}

float gps_longitude() {
    return _gps.location.lng();
}

float gps_altitude() {
    return _gps.altitude.meters();
}

float gps_hdop() {
    return _gps.hdop.hdop();
}

uint8_t gps_sats() {
    return _gps.satellites.value();
}

void gps_setup() {
    _serial_gps.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

boolean gps_available() {
  return _serial_gps.available();
}

static void gps_loop() {
    while (gps_available()) {
        _gps.encode(_serial_gps.read());
    }
}

void GPS_SetHAB()
{
  const uint8_t SetNavigation[]  = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05,
            0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC}; //44

  const byte GPS_attempts = 1; //number of times the sending of GPS config will be attempted.

  Serial.print(F("GPS High Altitude Balloon Settings "));
  size_t size = sizeof(SetNavigation); 
  GPS_Send_Config(SetNavigation, size, 4, GPS_attempts);
}

void GPS_Send_Config(const uint8_t *Progmem_ptr, byte arraysize, byte replylength, byte attempts)
{
  byte byteread,index;
  byte config_attempts = attempts;
  boolean GPS_Config_Error;

  Serial.print(F("("));
  Serial.print(arraysize); 
  Serial.print(F(") "));
  
  do
  {

    if (config_attempts == 0)
    {
      Serial.println(F("No Response from GPS"));
      GPS_Config_Error = true;
      break;
    }

   for (index = 0; index < arraysize; index++)
    {
      byteread = pgm_read_byte_near(Progmem_ptr++);
      _serial_gps.write(byteread);
      Serial.print(byteread, HEX);
      Serial.print(" ");
    }

    Progmem_ptr = Progmem_ptr - arraysize;     //put Progmem_ptr back to start value in case we need to re-send the config
 
    Serial.println();
    
    if (replylength == 0)
    {
      Serial.println(F("Reply not required"));
      break;
    }

    config_attempts--;
  } while (!GPS_WaitAck(GPS_WaitAck_mS, replylength));

  delay(50);                                         //GPS can sometimes be a bit slow getting ready for next config
}

boolean GPS_WaitAck(unsigned long waitms, byte length)
{
  //wait for Ack from GPS
  byte i, j;
  unsigned long endms;
  endms = millis() + waitms;
  byte ptr = 0;                             //used as pointer to store GPS reply
  
  do
  {
    while (_serial_gps.available() > 0)
      i = _serial_gps.read();
  }
  while ((i != 0xb5) && (millis() < endms));

  if (i != 0xb5)
  {
    Serial.print(F("Timeout "));
    return false;
  }
  else
  {
    Serial.print(F("Ack "));
    Serial.print(i, HEX);

    length--;

    for (j = 1; j <= length; j++)
    {
      Serial.print(F(" "));

      while (_serial_gps.available() == 0);
      i = _serial_gps.read();

      if (j < 12)
      {
        GPS_Reply[ptr++] = i;                  //save reply in buffer, but no more than 10 characters
      }

      if (i < 0x10)
      {
        Serial.print(F("0"));
      }
      Serial.print(i, HEX);
    }
  }
  Serial.println();
  return true;
}

#if defined(PAYLOAD_USE_FULL)

    // More data than PAYLOAD_USE_CAYENNE
    void buildPacket(uint8_t txBuffer[10])
    {
        LatitudeBinary = ((_gps.location.lat() + 90) / 180.0) * 16777215;
        LongitudeBinary = ((_gps.location.lng() + 180) / 360.0) * 16777215;
        altitudeGps = _gps.altitude.meters();
        hdopGps = _gps.hdop.value() / 10;
        sats = _gps.satellites.value();

        sprintf(t, "Lat: %f", _gps.location.lat());
        Serial.println(t);
        sprintf(t, "Lng: %f", _gps.location.lng());
        Serial.println(t);
        sprintf(t, "Alt: %f", altitudeGps);
        Serial.println(t);
        sprintf(t, "Hdop: %d", hdopGps);
        Serial.println(t);
        sprintf(t, "Sats: %d", sats);
        Serial.println(t);

        txBuffer[0] = ( LatitudeBinary >> 16 ) & 0xFF;
        txBuffer[1] = ( LatitudeBinary >> 8 ) & 0xFF;
        txBuffer[2] = LatitudeBinary & 0xFF;
        txBuffer[3] = ( LongitudeBinary >> 16 ) & 0xFF;
        txBuffer[4] = ( LongitudeBinary >> 8 ) & 0xFF;
        txBuffer[5] = LongitudeBinary & 0xFF;
        txBuffer[6] = ( altitudeGps >> 8 ) & 0xFF;
        txBuffer[7] = altitudeGps & 0xFF;
        txBuffer[8] = hdopGps & 0xFF;
        txBuffer[9] = sats & 0xFF;
    }

#elif defined(PAYLOAD_USE_CAYENNE)

    // CAYENNE DF
    void buildPacket(uint8_t txBuffer[11])
    {
        sprintf(t, "Lat: %f", _gps.location.lat());
        Serial.println(t);
        sprintf(t, "Lng: %f", _gps.location.lng());
        Serial.println(t);        
        sprintf(t, "Alt: %f", _gps.altitude.meters());
        Serial.println(t);        
        int32_t lat = _gps.location.lat() * 10000;
        int32_t lon = _gps.location.lng() * 10000;
        int32_t alt = _gps.altitude.meters() * 100;
        
        txBuffer[2] = lat >> 16;
        txBuffer[3] = lat >> 8;
        txBuffer[4] = lat;
        txBuffer[5] = lon >> 16;
        txBuffer[6] = lon >> 8;
        txBuffer[7] = lon;
        txBuffer[8] = alt >> 16;
        txBuffer[9] = alt >> 8;
        txBuffer[10] = alt;
    }

#endif
