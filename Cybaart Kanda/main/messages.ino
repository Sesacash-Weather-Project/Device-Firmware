void send_status() {
  uint8_t txBuffer[1];
  LoraEncoder encoder(txBuffer);

  encoder.writeBitmap( _BME280_found, 
                       gps_available(),
                       _ssd1306_found,
                       _axp192_found,
                       _ifWebOpen,
                       _ifSetMiner,
                       _ifAuthenticated,
                       _ifLaunched );

  // Battery / solar voltage
  //encoder.writeUint8(0);

  boolean confirmed = false;
  ttn_cnt(_count);
  Serial.println("Sending balloon status message.");
  ttn_send(txBuffer, sizeof(txBuffer), STATUS_PORT, confirmed);
  _count++;
}

void send_authentication() {
  uint8_t txBuffer[8];
  LoraEncoder encoder(txBuffer);

  encoder.writeUint64( _miner_account );

  boolean confirmed = false;
  ttn_cnt(_count);
  Serial.println("Sending balloon authentication message.");
  ttn_send(txBuffer, sizeof(txBuffer), AUTHENTICATION_PORT, confirmed);

}

void send_observation() {
  //LoraMessage message;

  uint8_t txBuffer[31];
  LoraEncoder encoder(txBuffer);
  
  char buffer[40];
  snprintf(buffer, sizeof(buffer), "Latitude: %10.6f\n", gps_latitude());
  Serial.println(buffer);
  screen_print(buffer);
  snprintf(buffer, sizeof(buffer), "Longitude: %10.6f\n", gps_longitude());
  Serial.println(buffer);
  screen_print(buffer);
  snprintf(buffer, sizeof(buffer), "Altitude: %4.2fm\n", gps_altitude());
  Serial.println(buffer);
  screen_print(buffer);
  snprintf(buffer, sizeof(buffer), "Elevation_2: %10.1f\n", _elevation_now);
  Serial.println(buffer);
  screen_print(buffer);
  snprintf(buffer, sizeof(buffer), "Temperature: %10.1f\n", _temperature_now);
  Serial.println(buffer);
  screen_print(buffer);
  snprintf(buffer, sizeof(buffer), "Humidity: %10.1f\n", _humidity_now);
  Serial.println(buffer);  
  screen_print(buffer);
  snprintf(buffer, sizeof(buffer), "Pressure: %10.2f\n", _pressure_now);
  Serial.println(buffer);
  screen_print(buffer);

  encoder.writeLatLng( gps_latitude(), gps_longitude() );
  encoder.writeUint16( gps_altitude() );
  encoder.writeRawFloat( gps_hdop() );
  encoder.writeUint16( (uint16_t)_elevation_now ); // In meters
  encoder.writeUint16( (uint16_t)(_pressure_now*10.0) ); // Convert hpa to deci-paschals
  encoder.writeTemperature(_temperature_now);
  encoder.writeHumidity(_humidity_now);
  encoder.writeBytes( _launch_id, 8 ); // Add 8 bytes for the launch_id
  encoder.writeBitmap( _BME280_found, 
                       gps_available(),
                       _ssd1306_found,
                       _axp192_found,
                       _ifWebOpen,
                       _ifSetMiner,
                       _ifAuthenticated,
                       _ifLaunched );

 Serial.println("Built message successfully");

#if LORAWAN_CONFIRMED_EVERY > 0
  bool confirmed = (_count % LORAWAN_CONFIRMED_EVERY == 0);
#else
  bool confirmed = false;
#endif

  ttn_cnt(_count);
  Serial.println("Sending balloon observation message.");
  ttn_send(txBuffer, sizeof(txBuffer), OBSERVATION_PORT, confirmed);
  //ttn_send(message.getBytes(),message.getLength(), OBSERVATION_PORT, confirmed);

  _count++;
}

uint32_t get_count() {
  return _count;
}

uint32_t set_send_interval( float pressure ) {
  switch( (uint8_t)floor(pressure/100) ) {
    case 9 :
      _send_interval = SEND_INTERVAL_1000;
      break;
    case 8 :
      _send_interval = SEND_INTERVAL_900;
      break;
    case 7 :
      _send_interval = SEND_INTERVAL_800;
      break;
    case 6 :
      _send_interval = SEND_INTERVAL_700;
      break;
    case 5 :
      _send_interval = SEND_INTERVAL_600;
      break;
    case 4 :
      _send_interval = SEND_INTERVAL_500;
      break;
    case 3 :
      _send_interval = SEND_INTERVAL_400;
      break;
    case 2 :
      _send_interval = SEND_INTERVAL_300;
      break;
    case 1 :
      _send_interval = SEND_INTERVAL_200;
      break;
    default:
      _send_interval = SEND_INTERVAL_1000;
      break;
  }
}

void callback(uint8_t message) {
  if (EV_JOINING == message) screen_print("Joining TTN...\n");
  if (EV_JOINED == message) screen_print("TTN joined!\n");
  if (EV_JOIN_FAILED == message) screen_print("TTN join failed\n");
  if (EV_REJOIN_FAILED == message) screen_print("TTN rejoin failed\n");
  if (EV_RESET == message) screen_print("Reset TTN connection\n");
  if (EV_LINK_DEAD == message) screen_print("TTN link dead\n");
  if (EV_ACK == message) screen_print("ACK received\n");
  if (EV_PENDING == message) screen_print("Message discarded\n");
  if (EV_QUEUED == message) screen_print("Message queued\n");

  if (EV_TXCOMPLETE == message) {
    screen_print("Message sent\n");

    if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack"));
    if (LMIC.dataLen) {
        Serial.print(F("Received "));
        Serial.print(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
        if ( LMIC.dataLen > 8 )
        {
            for (int i = 0; i < LMIC.dataLen; i++) {
                if (LMIC.frame[LMIC.dataBeg + i] < 0x10) {
                    Serial.print(F("0"));
                }
                Serial.print(F("Received payload: "));
                Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
                _launch_id[i] = LMIC.frame[LMIC.dataBeg + i];
            }
            Serial.println();
        } else {
            Serial.print("Unexpected number of bytes from LMIC.frame");
        }
        
        // downlink (turn relais on when received payload = 1)
        if (LMIC.frame[LMIC.dataBeg] == 1)
        {
            digitalWrite(RELAIS_PIN, HIGH);
            Serial.println("RELAIS ON");
            _relais_on = true;
        }
        else
        {
            digitalWrite(RELAIS_PIN, LOW);
            Serial.println("RELAIS OFF");
            _relais_on = false;
        }
    }
    sleep();
  }

  if (EV_RESPONSE == message) {

    screen_print("[TTN] Response: ");

    size_t len = ttn_response_len();
    uint8_t data[len];
    ttn_response(data, len);

    char buffer[6];
    for (uint8_t i = 0; i < len; i++) {
      snprintf(buffer, sizeof(buffer), "%02X", data[i]);
      screen_print(buffer);
    }
    screen_print("\n");
  }
}
