const char* miner_input = "miner_input";

String get_base_html()
{
  // HTML web page to handle 3 input fields (input1, input2, input3)
String html PROGMEM = R"(
  <!DOCTYPE HTML><html><head>
  <title>ESP32 Weather Station</title>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}   body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}
  p {font-size: 16px;color: #444444;margin-bottom: 10px;} form {font-size: 24px;color: #444444;margin-bottom: 10px;}
  </style>
  </head>
  <body>
  <h1>AscensionWx Balloon Weather Station</h1>)" + 
  
  String("<p>Temperature: " + String(get_temperature()) + " &deg;C</p>") +
  String("<p>Humidity: " + String(get_humidity()) + "%</p>") +
  String("<p>Pressure: " + String(get_pressure_hpa()) + " hPa</p>");

  return html;
}

String get_miner_html_enabled()
{
  String html PROGMEM = R"(
  <p> </p>
  <form action="/get">
    Telos Miner Account: <input type="text" name="miner_input" style="height:24px">
    <input type="submit" value="Submit" style="height:24px">
  </form><br>
</body></html>)";

  return html;
}

String get_miner_html_disabled()
{
  String html PROGMEM = R"(
  <p> </p>
  <form action="/get">
    Telos Miner Account: <input type="text" name="miner_input" style="height:24px" disabled>
    <input type="submit" value="Submit" style="height:24px" disabled>
  </form><br>
</body></html>)";

  return html;
}

String get_verify_html()
{
  if ( DOWNLINK_RESPONSE.length() > 0 )
    return String("<p>" + String(DOWNLINK_RESPONSE) + "</p");
  else
    return String("<p>No downlink. Node-RED server error.</p>");
}


void checkServer() 
{
  // Constructed with:
  //https://randomnerdtutorials.com/esp32-web-server-arduino-ide/

  // client connection
  //   only lasts about 2 secs max, so shouldnt interfere with status messages
  const long timeoutTime = 2000; // millis
  long previousTime = 0;
  long currentTime;

  // Stores response from client
  String header;
  
  // Listen for incoming clients
  CLIENT = SERVER.available();

  if (CLIENT) // Init correctly
  {
    if ( !IF_WEB_OPENED ) // Only do this once on initial connection
    {
      Serial.println("Client connected.");
      screen_print("Smartphone connected!\n");
      screen_print("Waiting for Miner ID...\n");
      screen_print("\n\n"); // two lines
    
      IF_WEB_OPENED = true;      
    }

    // Store for timeout purposes
    currentTime = millis();
    previousTime = currentTime;

    // Stores incoming client data
    String currentLine = "";

    while( CLIENT.connected() && currentTime - previousTime <= timeoutTime )
    {
      currentTime = millis();
      if (CLIENT.available())
      {
        char c = CLIENT.read();
        header += c; // store most recent character

        if ( c== '\n' )
        {
          if ( currentLine.length() == 0 ) // two newline characters in a row... end of client HTTP request
          {
            // HTTP header response preamble
            CLIENT.println("HTTP/1.1 200 OK");
            CLIENT.println("Content-type:text/html");
            CLIENT.println("Connection: close");
            CLIENT.println();

            // TEMPORARY: Printing client's response so that we can read it later
            Serial.println("This is the full header of the client's response:\n");
            Serial.println(header);

            // Check what the client input
            //if (header.indexOf( "GET /
            String tmp = "GET /get?";
            int index = header.indexOf( tmp + "miner_input" );

            String message, newpage;
            if ( index >= 0) // 
            {
              index = 9 + strlen("miner_input") + 1; // 9 comes from "GET /get?"
              Serial.println("User input:");
              String miner_input = header.substring(index, index+12);
              Serial.println("Miner input: " + miner_input); // 12 from eosio account name
              Serial.println();

              // Send the data over LoRaWAN
              send_authentication( miner_input );
              os_runloop_once();
              const int auth_send_time = millis();

              screen_print("Miner ID received: \n");
              screen_print(miner_input.c_str());
              screen_print("\n\n"); // two lines

              bool status_sent = false;
              while ( millis() - auth_send_time < 12000) // wait for nodered to query blockchain
              {
                os_runloop_once();

                // The downlink will happen after the next message, so
                //   we send an additional status message to receive downlinked field
                if ( millis() - auth_send_time > 3000 && !status_sent )
                {
                  send_status();
                  status_sent = true;
                }
              }
              
              // Construct new html page based on the received downlink
              newpage = get_base_html() + get_miner_html_disabled() + get_verify_html();
            }
            else 
            {
              newpage = get_base_html() + get_miner_html_enabled() + String("<p>" + String(DOWNLINK_RESPONSE) + "</p");
            }

            // Print the webpage to the client
            CLIENT.println( newpage.c_str() );
            CLIENT.println();
            
            break; // break out of while loop

          }
          else // newline received
          {
            currentLine = "";
          }
        }
        else if (c != '\r') // Received something other than carriage return
        {
          currentLine += c;
        } 
      } // end check for client.available
    } // end check for client connection

    CLIENT.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
    
  } // end check that client was init correctly

}
