/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <Arduino.h>

// #include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include <ETH.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include "LittleFS.h"
#include "driver/twai.h"

#include "common.h"
#include "ethernet.h"
#include "image_display.h"
#include "splash_screen.h"

#include <Adafruit_GFX.h> // Core graphics library
#include <SPI.h>
#include <Adafruit_ILI9341.h>

// #define VCC 13

#define CS 5
#define RESET 6
#define DC 7
#define MOSI 8
#define SCK 9
#define LED 10
#define MISO 20


// For the Adafruit shield, these are the default.
// #define TFT_DC 9
// #define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, DC, CS, RESET);
// If using the breakout, change pins as desired
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);



void setup()
{
  // Initialize USB Serial first
  Serial.begin(115200);
  
  // Wait for USB connection to stabilize after reset
  delay(1000);
  
  // Set logging levels for all modules
  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set(LOG_TAG_COMMON, ESP_LOG_DEBUG);  // Enable debug for image display
  esp_log_level_set(LOG_TAG_ETHERNET, ESP_LOG_INFO);
  esp_log_level_set(LOG_TAG_OTA, ESP_LOG_INFO);
  
  // Print startup banner to serial
  Serial.println();
  Serial.println("========================================");
  Serial.printf("ESP32-C3 Image Display System Starting\n");
  Serial.printf("Title: %s\n", TITLE);
  Serial.printf("Version: %s\n", VERSION);
  Serial.printf("Author: %s\n", AUTHOR);
  Serial.printf("Date: %s\n", DATE);
  Serial.println("========================================");
  
  ESP_LOGI(LOG_TAG_COMMON, "Starting system initialization...");
  
  // Initialize display hardware first
  ESP_LOGI(LOG_TAG_COMMON, "Initializing display hardware...");
  
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  ESP_LOGI(LOG_TAG_COMMON, "LED backlight enabled");

  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, HIGH);
  ESP_LOGI(LOG_TAG_COMMON, "Display reset pin configured");

  // SPIピン設定
  SPI.begin(SCK, MISO, MOSI, CS);
  ESP_LOGI(LOG_TAG_COMMON, "SPI initialized - SCK:%d, MISO:%d, MOSI:%d, CS:%d", SCK, MISO, MOSI, CS);

  tft.begin();
  ESP_LOGI(LOG_TAG_COMMON, "ILI9341 TFT display initialized");
  
  // Initialize display
  tft.setRotation(0); // Portrait mode for 240x320
  ESP_LOGI(LOG_TAG_COMMON, "Display cleared and set to portrait mode (240x320)");
  
  // Show splash screen
  showSplashScreen();
  ESP_LOGI(LOG_TAG_COMMON, "Splash screen displayed");

  // Initialize network
  ESP_LOGI(LOG_TAG_COMMON, "Starting network initialization...");
  ethernet_init();

  ESP_LOGI(LOG_TAG_COMMON, "=== System initialization complete ===");
  Serial.println("System ready! Connect to WiFi AP and access web interface.");
  
  // Show QR codes after network is ready
  showQRCodes();
  ESP_LOGI(LOG_TAG_COMMON, "QR codes displayed");
}

void loop(void)
{
  
}

