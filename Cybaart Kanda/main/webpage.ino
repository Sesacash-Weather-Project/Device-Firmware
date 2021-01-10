const char* PARAM_INPUT_1 = "input1";

String get_html()
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
  String("<p>Pressure: " + String(get_pressure()) + " hPa</p>") +
  
  R"(
  <p> </p>
  <form action="/get">
    Telos Miner Account: <input type="text" name="input1" style="height:24px">
    <input type="submit" value="Submit" style="height:24px">
  </form><br>
</body></html>)";

return html;
}



void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void initServer() 
{
  // Send web page with input fields to client
  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", get_html().c_str());
    Serial.println("Client connected.");
    _ifWebOpen = true;
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  _server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println("Web app submitted following Miner name: " + inputMessage);

    // Convert the input
    _miner_account = eosio_name_to_uint64(inputMessage);
    _miner_account = 123;
    
    // Send the LoRaWAN message
    send_authentication();
    
    request->send_P(200, "text/html", get_html().c_str());

    /*
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
    */
  });
  
  _server.onNotFound(notFound);
}

int char_to_symbol(int c)
{ 

  int a = (int)'a';
  int z = (int)'z';
  int one = (int)'1';
  int five = (int)'5';

  //Serial.println("The char code for 'a' is "+(int)'a');

  if (c >= a && c <= z ) {
    return (c - a + 6);
  }

  if (c >= one && c <= five ) {
    return (c - one + 1);
  }

}

uint64_t eosio_name_to_uint64(String name)
{
  Serial.println(" ");
  Serial.println(" ");

  uint64_t n = 0;

  int len = 0;

  for (uint8_t i=0; i < 12 && name[i]; i++)
  {
    // n |= BigInt(char_to_symbol(s.charCodeAt(i)) & 0x1f) << BigInt(64 - 5 * (i + 1));
    int c = (int)name[i];

    int64_t tmp = char_to_symbol(c) & 0x1f;
    int64_t tmp2 = 64 - 5*(i+1);
    n |= tmp << tmp2;
    len=i;
  }

  if (len==12)
  {
    int c = (int)name[12];
    n |= (uint64_t)(char_to_symbol(c) & 0x0f);
  }

  return n;
}
