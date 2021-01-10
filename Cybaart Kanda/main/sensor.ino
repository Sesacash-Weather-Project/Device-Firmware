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

    // Wake up the sensor
    get_pressure();
    delay(500);
    
    // Store ground pressure in hpa
    _sfc_pressure = get_pressure();

    // Compute prior variables
    _pressure_last = _sfc_pressure;
    _v_temperature_last = calc_v_temperature( get_temperature(), 
                                             get_humidity(), 
                                             _pressure_last );
    _elevation_last = 0;

}

void bme_loop() {
      // Update environment state
      _temperature_now = get_temperature();
      _humidity_now = get_humidity();
      _pressure_now = get_pressure(); // Store as hpa
      _v_temperature_now = calc_v_temperature( _temperature_now, 
                                              _humidity_now, 
                                              _pressure_now );
      _elevation_now = calc_elevation( _pressure_last, 
                                      _v_temperature_last, 
                                      _elevation_last, 
                                      _pressure_now, 
                                      _v_temperature_now);
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
  // Uses _count and last values to get the elevation
  //   p2 and tv2 are the most recent pressure and virtual temp values
  if (_count == 0)
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
