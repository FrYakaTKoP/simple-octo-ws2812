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

//Animation Variables
//
uint8_t k = 0, stage = 0, modus = 255;
uint32_t last = 0;
//
//end Animaton Variables

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
                  modus = 3; // Yellow
                }
                else if(root["state"]["flags"]["paused"])
                {
                  modus = 2; // blue
                  // TBD: should display rainbow now and on
                }
                else if(root["state"]["flags"]["error"])
                {
                  modus = 0; // red
                }
                // finsihed flag does not exist :(
                // else if(root["state"]["flags"]["finished"])
                // {
                //   fadeInOut(strip.Color(0, 255, 0), 0, 230); // Green
                // }
                else if(root["state"]["flags"]["ready"])
                {
                  modus = 4;
                }
              }
              else if(httpCode == HTTP_CODE_UNAUTHORIZED)
              {
                  // wrong api key
                  modus = 0; // Red
                  // TBD: should stop after 2min
              }
              else if(httpCode == HTTP_CODE_CONFLICT)
              {
                // Printer is not operational
                modus = 2; // Green
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

 switch (modus) {
   case 0: {
     fade_red(&last, &k, &stage, lenght);
     break;
   }
   case 1: {
     fade_green(&last, &k, &stage, lenght);
      break;
    }

   case 2: {
      fade_blue(&last, &k, &stage, lenght);
      break;
    }
  case 3: {
   fade_yellow(&last, &k, &stage, lenght);
    break;
   }
  case 4: {
      rainbow(&last, &k, &stage, lenght);
      break;
    }
  case 5: {
      police(&last, &stage, lenght);
      break;
    }
  case 6: {
      flash(&last, &stage, lenght);
      break;
    }

  default: {

      break;
    }
  }//switch

} // loop



//Animationfunctions
//
void rainbow(uint32_t* lastmillis, uint8_t* k, uint8_t* stage, uint8_t lenght) {

  unsigned long  currentmillis = millis();

  if ((currentmillis - *lastmillis) >= 200) {
    *lastmillis = millis();
    switch (*stage) {
      case 0: {
          for (int i = 0; i < lenght; i++)  {
            LED.setPixelColor(i, 255 - *k, 0, *k);
          }
          break;
        }

      case 1: {
          for (int i = 0; i < lenght; i++)  {
            LED.setPixelColor(i, 0, *k, 255 - *k);
          }
          break;
        }

      case 2: {
          for (int i = 0; i < lenght; i++) {
            LED.setPixelColor(i, *k, 255 - *k, 0);
          }
          break;
        }
    }
    LED.show();
    if (*k == 254) {
      *stage = (*stage + 1) % 3;
    }
    *k = (*k + 1) % 255;
  }
}

void fade_red(uint32_t* lastmillis, uint8_t* k, uint8_t* stage, uint8_t lenght) {

  unsigned long  currentmillis = millis();

  if ((currentmillis - *lastmillis) >= 20) {
    *lastmillis = millis();
    switch (*stage) {
      case 0: {
          for (int i = 0; i < lenght; i++)  {
            LED.setPixelColor(i, 0, 63 + *k, 0);
          }
          break;
        }

      case 1: {
          for (int i = 0; i < lenght; i++)  {
            LED.setPixelColor(i, 0, 255 - *k, 0);
          }
          break;
        }

    }
    LED.show();
    if (*k == 191) {
      *stage = (*stage + 1) % 2;
    }
    *k = (*k + 1) % 192;
  }
}

void fade_green(uint32_t* lastmillis, uint8_t* k, uint8_t* stage, uint8_t lenght) {

  unsigned long  currentmillis = millis();

  if ((currentmillis - *lastmillis) >= 20) {
    *lastmillis = millis();
    switch (*stage) {
      case 0: {
          for (int i = 0; i < lenght; i++)  {
            LED.setPixelColor(i, 0, 63 + *k, 0);
          }
          break;
        }

      case 1: {
          for (int i = 0; i < lenght; i++)  {
            LED.setPixelColor(i, 0, 255 - *k, 0);
          }
          break;
        }

    }
    LED.show();
    if (*k == 191) {
      *stage = (*stage + 1) % 2;
    }
    *k = (*k + 1) % 192;
  }
}

void fade_blue(uint32_t* lastmillis, uint8_t* k, uint8_t* stage, uint8_t lenght) {

  unsigned long  currentmillis = millis();

  if ((currentmillis - *lastmillis) >= 20) {
    *lastmillis = millis();
    switch (*stage) {
      case 0: {
          for (int i = 0; i < lenght; i++)  {
            LED.setPixelColor(i, 0, 0, 63 + *k);
          }
          break;
        }

      case 1: {
          for (int i = 0; i < lenght; i++)  {
            LED.setPixelColor(i, 0, 0, 255 - *k);
          }
          break;
        }

    }
    LED.show();
    if (*k == 191) {
      *stage = (*stage + 1) % 2;
    }
    *k = (*k + 1) % 192;
  }
}

void fade_yellow(uint32_t* lastmillis, uint8_t* k, uint8_t* stage, uint8_t lenght) {

  unsigned long  currentmillis = millis();

  if ((currentmillis - *lastmillis) >= 20) {
    *lastmillis = millis();
    switch (*stage) {
      case 0: {
          for (int i = 0; i < lenght; i++)  {
            LED.setPixelColor(i, 63 + *k, 63 + *k, 0);
          }
          break;
        }

      case 1: {
          for (int i = 0; i < lenght; i++)  {
            LED.setPixelColor(i, 255 - *k, 255 - *k, 0);
          }
          break;
        }

    }
    LED.show();
    if (*k == 191) {
      *stage = (*stage + 1) % 2;
    }
    *k = (*k + 1) % 192;
  }
}

void police(uint32_t* lastmillis, uint8_t* stage, uint8_t lenght) {

  unsigned long  currentmillis = millis();

  if ((currentmillis - *lastmillis) >= 400 ) {
    *lastmillis = millis();
    if (*stage == 0) {
      for (int i = 0; i < lenght; i++) {
        LED.setPixelColor(i, 255, 0, 0);
      }
      LED.show();
      *stage = 1;
    }
    else {
      if (*stage == 1) {
        for (int i = 0; i < lenght; i++) {
          LED.setPixelColor(i, 0, 0, 255);
        }
        LED.show();
        *stage = 0;
      }
    }
  }
}

void flash(uint32_t* lastmillis, uint8_t* stage, uint8_t lenght) {

  unsigned long  currentmillis = millis();

  if ((currentmillis - *lastmillis) >= 40 ) {
    *lastmillis = millis();
    if (*stage == 0) {
      for (int i = 0; i < lenght; i++) {
        LED.setPixelColor(i, 255, 255, 255);
      }
      LED.show();
      *stage = 1;
    }
    else {
      if (*stage == 1) {
        for (int i = 0; i < lenght; i++) {
          LED.setPixelColor(i, 0, 0, 0);
        }
        LED.show();
        *stage = 0;
      }
    }
  }
}
//
//End Animationfunctions
