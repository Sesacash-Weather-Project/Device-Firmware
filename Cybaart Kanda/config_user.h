
// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define DEBUG_MODE 1
//#define IGNORE_GATEWAY_CHECK 1
#define IGNORE_TPRH_CHECK 1 // Uncomment to allow no temperature, pressure, humidity sensor to be soldered

// If using a single-channel gateway, uncomment this next option and set to your gateway's channel
//#define SINGLE_CHANNEL_GATEWAY  0

#define BME_INTERVAL            30000       // Query the BME (more often could mean better elevation_2)
#define MESSAGE_TO_SLEEP_DELAY  5000        // Time after message before going to sleep
#define LOGO_DELAY              5000        // Time to show logo on first boot
#define QRCODE_DELAY            10000        // Time to show logo on first boot
#define OBSERVATION_PORT        1          // Port the obs messages will be sent to
#define STATUS_PORT             2          // Port the status messages will be sent to
#define AUTHENTICATION_PORT     3          // Port for authentication message
#define LORAWAN_CONFIRMED_EVERY 0           // Send confirmed message every these many messages (0 means never)
#define LORAWAN_SF              DR_SF7     // Spreading factor
#define LORAWAN_ADR             0           // Enable ADR

#define MISSING_UINT            0
#define MISSING_FLOAT           0

//Wifi defines
const char* SSID     = "AscensionWx";
const char* PASSWORD = "balloons";
