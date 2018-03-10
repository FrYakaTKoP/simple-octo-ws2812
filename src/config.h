
// User config
// more info in the README.md in the root of the repository

#ifndef CONFIG  // not #ifnotdef
#define CONFIG

  #define LEDPIN D2
  const char* SSID = ""; // wlan SSID
  const char* WPWD = ""; // wlan password
  const char* APIKEY = ""; // octoprint api key (optain it in the web interface)
  String OCTOIP = "192.168.1.3"; // the ip address of octoprint
  uint16_t pollInterval = 500; // ms, lower value will increase poll rate, but the animation time will limit the poll rate too
  const uint8_t lenght = 1; // number of LEDs

#endif
