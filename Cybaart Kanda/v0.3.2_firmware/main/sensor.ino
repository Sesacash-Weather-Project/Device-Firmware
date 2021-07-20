/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor
  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650
  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.
  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!
  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
  See the LICENSE file for details.
 ***************************************************************************/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


uint32_t humidity_bme;
uint32_t pressure_bme;
uint32_t temperature_bme;

char bme_char[32]; // used to sprintf for Serial output

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C

void bme_setup() {
  
    Serial.begin(115200);
    bool status;
    status = bme.begin(0x76);
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
    }

    // Wake up sensor
    delay(20);
    get_pressure();
    delay(500);
    
    // Store ground pressure in hpa (make sure values are reasonable)
    while (SFC_PRESSURE < 150.0 || SFC_PRESSURE > 1020.0)
    {
      SFC_PRESSURE = get_pressure();
      delay(500);
    }

    // Compute prior variables
    PRESSURE_LAST = SFC_PRESSURE;
    V_TEMPERATURE_LAST = calc_v_temperature( get_temperature(), 
                                             get_humidity(), 
                                             PRESSURE_LAST );
    ELEVATION_LAST = 0;

}

void bme_loop() {
      // Update environment state
      TEMPERATURE_NOW = get_temperature();
      HUMIDITY_NOW = get_humidity();
      PRESSURE_NOW = get_pressure(); // Store as hpa
      V_TEMPERATURE_NOW = calc_v_temperature( TEMPERATURE_NOW, 
                                              HUMIDITY_NOW, 
                                              PRESSURE_NOW );
      ELEVATION_NOW = calc_elevation( PRESSURE_LAST, 
                                      V_TEMPERATURE_LAST, 
                                      ELEVATION_LAST, 
                                      PRESSURE_NOW, 
                                      V_TEMPERATURE_NOW);
}

float get_humidity() {
    float rh = bme.readHumidity();
    if (rh==0) { return MISSING_FLOAT; }
    else { return rh; }
}

float get_pressure() {
    float p = bme.readPressure();
    if (p==0) { return MISSING_FLOAT; }
    else { return (p / 100.0); } // Convert to hpa
}

float get_temperature() {
    float t = bme.readTemperature();
    if (t==0) { return MISSING_FLOAT; }
    else { return t; }
}

float calc_v_temperature(float t, float rh, float p) {
  
  // calc saturation vapor pressure es(T)
  // https://journals.ametsoc.org/jamc/article/57/6/1265/68235/A-Simple-Accurate-Formula-for-Calculating
  float es;
  if ( t>0 )
  {
    es = exp( 34.494 - (4924.99/(t+237.1)) ) /  pow(t+105, 1.57);
  } else {
    es = exp( 43.494 - (6545.8/(t+278)) ) /  pow(t+868, 2);
  }
  
  // calc actual vapor pressure using  rh = e/es
  float e = (rh/100)*es;
  e = e/100.0; // Convert to hpa
  
  // calc mixing ratio
  float w = 0.62197 * (e / (p-e));

  // calc virtual temperature in Kelvin
  float tv = (1+(0.61*w))*(t+273.15);

  return tv;
  
}

float calc_elevation(float p1, float tv1, float z1, float p2, float tv2) {
  // Uses COUNT and last values to get the elevation
  //   p2 and tv2 are the most recent pressure and virtual temp values
  if (COUNT == 0)
  {
     // Turning on device at about 1 meter above surface
     return 1;
  }
  
  float R = 287.058; // specific gas constant for dry air
  float g = 9.80664; // gravitational acceleration

  // Approx average virtual temperature of layer
  float tv_avg = tv1 + (tv2 - tv1)/2.0;

  // Use natural logarithm
  float elevation = (R*tv_avg/g)*log(p1/p2) + z1;

  return elevation;

}

void checkIfLaunched()
{ 
  // Checks if the balloon is rising and stores value in global variable
  if ( get_pressure() < SFC_PRESSURE - 0.7 )
  {
    IF_LAUNCHED = true;
  }
}

bool if_T_and_RH_sensor()
{
  if (BME280_FOUND) return true;
  else return false;
}

bool if_P_sensor()
{
  if (BME280_FOUND) return true;
  else return false;
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
                SSD1306_FOUND = true;
                Serial.println("ssd1306 display found");
            }
            if (addr == AXP192_SLAVE_ADDRESS) {
                AXP192_FOUND = true;
                Serial.println("axp192 PMU found");
            }
            if (addr == BME280_ADDRESS_0x76 || addr == BME280_ADDRESS_0x77) {
                BME280_FOUND = true;
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
