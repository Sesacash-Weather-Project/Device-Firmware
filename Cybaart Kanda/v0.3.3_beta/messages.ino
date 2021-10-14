
#define SEND_INTERVAL_1000          5000      // Sleep for these many millis in between send()
#define SEND_INTERVAL_900           10000      
#define SEND_INTERVAL_800           30000     
#define SEND_INTERVAL_700           60000     
#define SEND_INTERVAL_600           60000
#define SEND_INTERVAL_500           60000
#define SEND_INTERVAL_400           60000
#define SEND_INTERVAL_300           60000
#define SEND_INTERVAL_200           60000

#define EV_QUEUED       100
#define EV_PENDING      101
#define EV_ACK          102
#define EV_RESPONSE     103


void send_status( bool want_confirmed )
{
    uint8_t txBuffer[1];
  LoraEncoder encoder(txBuffer);

  encoder.writeBitmap( if_T_and_RH_sensor(),
                       if_P_sensor(),
                       SSD1306_FOUND,
                       gps_available(),
                       if_good_gps_quality(),
                       IF_WEB_OPENED,
                       if_received_downlink(),
                       IF_LAUNCHED );

  // Battery / solar voltage
  //encoder.writeUint8(0);

  ttn_cnt(COUNT);

  Serial.println("");
  Serial.print("If T and RH sensor found: ");
  if (if_T_and_RH_sensor()) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If P sensor found: ");
  if (if_P_sensor()) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If OLED found: ");
  if (SSD1306_FOUND) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If GPS currently available: ");
  if (gps_available()) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If Good GPS quality currently: ");
  if (if_good_gps_quality()) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If Web App opened: ");
  if (IF_WEB_OPENED) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If received downlink: ");
  if (if_received_downlink()) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If has been launched: ");
  if (IF_LAUNCHED) Serial.println("TRUE");
  else Serial.println("FALSE");
  
  Serial.println("Sending balloon status message.");
  ttn_send(txBuffer, sizeof(txBuffer), STATUS_PORT, want_confirmed);
  COUNT++;
}

void send_status() {
  send_status( false );
}

void send_authentication( String account ) {
  uint8_t txBuffer[13];  // 8 for uint64   ... 13 for string (12 characters plus \n)

  boolean confirmed = true;
  ttn_cnt(COUNT);
  Serial.println("Sending balloon authentication message.");

  account.getBytes(txBuffer, 13);
  
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
  snprintf(buffer, sizeof(buffer), "Elevation_2: %10.1f\n", ELEVATION_NOW);
  Serial.println(buffer);
  snprintf(buffer, sizeof(buffer), "Temperature: %10.1f\n", TEMPERATURE_NOW);
  Serial.println(buffer);
  screen_print(buffer);
  snprintf(buffer, sizeof(buffer), "Humidity: %10.1f\n", HUMIDITY_NOW);
  Serial.println(buffer);  
  screen_print(buffer);
  snprintf(buffer, sizeof(buffer), "Pressure: %10.2f\n", PRESSURE_NOW);
  Serial.println(buffer);
  screen_print(buffer);

  encoder.writeLatLng( gps_latitude(), gps_longitude() );
  encoder.writeUint16( gps_altitude() );
  encoder.writeRawFloat( gps_hdop() ); // possibly remove this field
  encoder.writeUint16( (uint16_t)ELEVATION_NOW ); // In meters
  encoder.writeUint16( (uint16_t)(PRESSURE_NOW*10.0) ); // Convert hpa to deci-paschals
  encoder.writeTemperature(TEMPERATURE_NOW);
  encoder.writeHumidity(HUMIDITY_NOW);
  encoder.writeBytes( LAUNCH_ID, 8 ); // Add 8 bytes for the launch_id
  encoder.writeBitmap( if_T_and_RH_sensor(),
                       if_P_sensor(),
                       SSD1306_FOUND,
                       gps_available(),
                       if_good_gps_quality(),
                       IF_WEB_OPENED,
                       if_received_downlink(),
                       IF_LAUNCHED );

 Serial.println("Built message successfully");

#if LORAWAN_CONFIRMED_EVERY > 0
  bool confirmed = (COUNT % LORAWAN_CONFIRMED_EVERY == 0);
#else
  bool confirmed = false;
#endif

  ttn_cnt(COUNT);

  Serial.println("");
  Serial.print("If T and RH sensor found: ");
  if (if_T_and_RH_sensor()) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If P sensor found: ");
  if (if_P_sensor()) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If OLED found: ");
  if (SSD1306_FOUND) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If GPS currently available: ");
  if (gps_available()) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If Good GPS quality currently: ");
  if (if_good_gps_quality()) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If Web App opened: ");
  if (IF_WEB_OPENED) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If received downlink: ");
  if (if_received_downlink()) Serial.println("TRUE");
  else Serial.println("FALSE");
  Serial.print("If has been launched: ");
  if (IF_LAUNCHED) Serial.println("TRUE");
  else Serial.println("FALSE");
  
  Serial.println("Sending balloon observation message.");
  ttn_send(txBuffer, sizeof(txBuffer), OBSERVATION_PORT, confirmed);
  //ttn_send(message.getBytes(),message.getLength(), OBSERVATION_PORT, confirmed);

  COUNT++;
}

uint32_t get_count() {
  return COUNT;
}

uint32_t set_send_interval( float pressure ) {
  switch( (uint8_t)floor(pressure/100) ) {
    case 9 :
      SEND_INTERVAL = SEND_INTERVAL_1000;
      break;
    case 8 :
      SEND_INTERVAL = SEND_INTERVAL_900;
      break;
    case 7 :
      SEND_INTERVAL = SEND_INTERVAL_800;
      break;
    case 6 :
      SEND_INTERVAL = SEND_INTERVAL_700;
      break;
    case 5 :
      SEND_INTERVAL = SEND_INTERVAL_600;
      break;
    case 4 :
      SEND_INTERVAL = SEND_INTERVAL_500;
      break;
    case 3 :
      SEND_INTERVAL = SEND_INTERVAL_400;
      break;
    case 2 :
      SEND_INTERVAL = SEND_INTERVAL_300;
      break;
    case 1 :
      SEND_INTERVAL = SEND_INTERVAL_200;
      break;
    default:
      SEND_INTERVAL = SEND_INTERVAL_1000;
      break;
  }
}

void callback(uint8_t message) {
  /* To be removed
  if (EV_JOINING == message) screen_print("Joining TTN...\n");
  if (EV_JOINED == message) screen_print("TTN joined!\n");
  if (EV_JOIN_FAILED == message) screen_print("TTN join failed\n");
  if (EV_REJOIN_FAILED == message) screen_print("TTN rejoin failed\n");
  if (EV_RESET == message) screen_print("Reset TTN connection\n");
  if (EV_LINK_DEAD == message) screen_print("TTN link dead\n");
  if (EV_ACK == message) screen_print("ACK received\n");
  if (EV_PENDING == message) screen_print("Message discarded\n");
  if (EV_QUEUED == message) screen_print("Message queued\n");
  */

  if (EV_TXCOMPLETE == message) {
    //screen_print("Message sent\n");

    if (LMIC.txrxFlags & TXRX_ACK)
    {
        GATEWAY_IN_RANGE = true;
    }

  }

  if (EV_RESPONSE == message) {

    if (LMIC.dataLen) 
    {
      GATEWAY_IN_RANGE = true; // if we receive downlink, we know this is true
        
      char tmp[25]; // set to size of screen, etc
      strncpy( tmp, (char*)(LMIC.frame+LMIC.dataBeg), LMIC.dataLen );
      tmp[ LMIC.dataLen ] = '\0'; // null terminate to make it a string

      DOWNLINK_RESPONSE = tmp; // Store the response as a string

      String respText = "Response:\n" + DOWNLINK_RESPONSE + "\n";
      screen_print(respText.c_str());
      Serial.println();
      Serial.println("LoRaWAN response: " + DOWNLINK_RESPONSE);   
    }

    if (!gps_available()) {
      screen_print("Warning:\n");
      screen_print("  GPS not yet locked.\n");
    } else 
    {
      screen_print(" \n \n");
    }
  } // end of check for response
}
