#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <Adafruit_NeoPixel.h>

#include <ArduinoJson.h>

#include "config.h"

#define USE_SERIAL Serial

uint8_t wiCount = 0;
uint32_t previousMillis = 0;

const char *prefix = "http://";
const char *postfix ="/api/printer?exclude=temperature,sd";

String url = prefix +OCTOIP+ postfix;
const char *URL = url.c_str();

ESP8266WiFiMulti WiFiMulti;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LEDPIN, NEO_GRB + NEO_KHZ800);

void setup() {

  strip.begin();

  strip.setBrightness(60);
  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.show(); // Initialize all pixels to 'off'

    USE_SERIAL.begin(115200);
   // USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println("try connecting to Wifi");

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(SSID, WPWD);
    delay(100);
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= pollInterval) {
      previousMillis = currentMillis;

      // wait for WiFi connection
      if(WiFiMulti.run() == WL_CONNECTED) {

          HTTPClient http;

          http.begin(URL); //HTTP

          //USE_SERIAL.print("[HTTP] GET...\n");
          // start connection and send HTTP header
          http.addHeader("X-Api-Key", APIKEY);
          int httpCode = http.GET();

          // httpCode will be negative on error
          if(httpCode > 0) {
              // HTTP header has been send and Server response header has been handled
              USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

              String payload = http.getString();
              USE_SERIAL.println(payload);

              if(httpCode == HTTP_CODE_OK) {
                // 200 get state flags..
                // Allocate JsonBuffer
                // Use arduinojson.org/assistant to compute the capacity.
                // {
                //   "state": {
                //     "flags": {
                //       "closedOrError": false,
                //       "error": false,
                //       "operational": true,
                //       "paused": false,
                //       "printing": false,
                //       "ready": true,
                //       "sdReady": false
                //     },
                //     "text": "Operational"
                //   }
                // }
                const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(7);
                DynamicJsonBuffer jsonBuffer(capacity);

                // Parse JSON object
                JsonObject& root = jsonBuffer.parseObject(payload);
                if (!root.success()) {
                  Serial.println(F("Parsing failed!"));
                  return;
                }

                if(root["state"]["flags"]["printing"])
                {
                  fadeInOut(strip.Color(255, 255, 0), 0, 230); // Yellow
                }
                else if(root["state"]["flags"]["paused"])
                {
                  fadeInOut(strip.Color(0, 0, 255), 0, 230); // blue
                  // TBD: should display rainbow now and on
                }
                else if(root["state"]["flags"]["error"])
                {
                  fadeInOut(strip.Color(255, 0, 0), 0, 230); // red
                }
                // finsihed flag does not exist :(
                // else if(root["state"]["flags"]["finished"])
                // {
                //   fadeInOut(strip.Color(0, 255, 0), 0, 230); // Green
                // }
                else if(root["state"]["flags"]["ready"])
                {
                  strip.setBrightness(255);
                  rainbowCycle(20);
                }
              }
              else if(httpCode == HTTP_CODE_UNAUTHORIZED)
              {
                  // wrong api key
                  fadeInOut(strip.Color(255, 0, 0), 0, 60); // Red
                  // TBD: should stop after 2min
              }
              else if(httpCode == HTTP_CODE_CONFLICT)
              {
                  // Printer is not operational
                  fadeInOut(strip.Color(0, 255, 0), 0, 60); // Green
              }
            }
            else
            {
                USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }

          http.end();
      }
      else if(WiFiMulti.run() == WL_CONNECT_FAILED)
      {
        USE_SERIAL.println("");
        USE_SERIAL.println("WL_CONNECT_FAILED");
        USE_SERIAL.println("Please Check Password");
      }
      else
      {
        USE_SERIAL.print(".");
        wiCount++;
        if(wiCount == 20)
        {
          USE_SERIAL.println("");
          USE_SERIAL.println("Please Check SSID");
          wiCount = 0;
        }
        //USE_SERIAL.println(WiFiMulti.run());
      }

    } // interval
} // loop

void fadeInOut(uint32_t color, int bottom, int top)
{
  int j, j2;
  strip.setBrightness(bottom);
  strip.setPixelColor(0, color);
  strip.show();

  for (j = bottom; j < top; j++) {
    strip.setBrightness(j);
    strip.setPixelColor(0, color);
    strip.show();
    delay(10);
  }
  delay(20);
  for (j2 = top; j2 > bottom; j2--) {
    strip.setBrightness(j2);
    strip.setPixelColor(0, color);
    strip.show();
    delay(10);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//Agent Cain Test
