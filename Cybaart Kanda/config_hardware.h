

#define DEBUG_PORT              Serial      // Serial debug port
#define SERIAL_BAUD             115200      // Serial debug baud rate

#ifdef DEBUG_PORT
#define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

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
// T, Pressure, Humidity
// -----------------------------------------------------------------------------

#define BME280_ADDRESS_0x76 0x76
#define BME280_ADDRESS_0x77 0x77
#define SPL06_ADDRESS_0x76 0x76
#define SPL06_ADDRESS_0x77 0x77
#define SHT21_ADDRESS 0x40

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

#define CLOCK_ERROR             1

// -----------------------------------------------------------------------------
// AXP192 (Rev1-specific options)
// -----------------------------------------------------------------------------

#define AXP192_SLAVE_ADDRESS  0x34
#define GPS_POWER_CTRL_CH     3
#define LORA_POWER_CTRL_CH    2
#define PMU_IRQ               35
