#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <qrcode.h>
#include "common.h"
#include "ethernet.h"
#include "splash_screen.h"
#include "esp_log.h"

// QR code data buffers (size for version 3 QR code)
uint8_t wifiQrCodeData[178];  // qrcode_getBufferSize(3) = 178
uint8_t webQrCodeData[178];   // qrcode_getBufferSize(3) = 178
QRCode wifiQrCode;
QRCode webQrCode;

void showSplashScreen() {
    ESP_LOGI(LOG_TAG_COMMON, "Displaying splash screen");
    
    // Clear screen with gradient background
    tft.fillScreen(COLOR_BACKGROUND);
    
    // Draw header with system name
    tft.fillRect(0, 0, 240, 60, COLOR_PRIMARY);
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(2);
    
    // Center text calculation
    String title = "ESP32 Display";
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    int titleX = (240 - w) / 2;
    tft.setCursor(titleX, 20);
    tft.print(title);
    
    // Draw subtitle
    tft.setTextSize(1);
    String subtitle = "Image Display System";
    tft.getTextBounds(subtitle, 0, 0, &x1, &y1, &w, &h);
    int subtitleX = (240 - w) / 2;
    tft.setCursor(subtitleX, 40);
    tft.print(subtitle);
    
    // Draw version info
    tft.setTextColor(COLOR_ACCENT);
    tft.setCursor(10, 80);
    tft.printf("Version: %s", VERSION);
    
    tft.setCursor(10, 95);
    tft.printf("Author: %s", AUTHOR);
    
    // Draw feature list
    tft.setTextColor(COLOR_TEXT);
    tft.setCursor(10, 120);
    tft.print("Features:");
    
    tft.setCursor(20, 135);
    tft.print("- WiFi Access Point");
    tft.setCursor(20, 150);
    tft.print("- PNG/JPEG Display");
    tft.setCursor(20, 165);
    tft.print("- Web Interface");
    tft.setCursor(20, 180);
    tft.print("- 240x320 TFT");
    
    // Draw initialization progress
    tft.setTextColor(COLOR_SECONDARY);
    tft.setCursor(10, 210);
    tft.print("Initializing...");
    
    // Progress bar background
    tft.drawRect(10, 230, 222, 20, COLOR_TEXT);
    tft.fillRect(11, 231, 220, 18, COLOR_BACKGROUND);
    
    // Animated progress bar
    for(int i = 0; i <= 220; i += 10) {
        tft.fillRect(11, 231, i, 18, COLOR_SECONDARY);
        delay(100);
    }
    
    // Completion message
    tft.setCursor(10, 260);
    tft.setTextColor(COLOR_ACCENT);
    tft.print("Ready!");
    
    delay(1000);
}

void generateWiFiQRCode() {
    ESP_LOGI(LOG_TAG_COMMON, "Generating WiFi QR code");
    
    // WiFi QR code format: WIFI:T:WPA;S:SSID;P:PASSWORD;;
    String wifiString = "WIFI:T:WPA;S:";
    wifiString += AP_SSID;
    wifiString += ";P:";
    wifiString += AP_PASSWORD;
    wifiString += ";;";
    
    ESP_LOGI(LOG_TAG_COMMON, "WiFi string: %s", wifiString.c_str());
    
    int result = qrcode_initText(&wifiQrCode, wifiQrCodeData, 3, ECC_LOW, wifiString.c_str());
    ESP_LOGI(LOG_TAG_COMMON, "WiFi QR code result: %d, size: %d", result, wifiQrCode.size);
}

void generateWebQRCode() {
    ESP_LOGI(LOG_TAG_COMMON, "Generating web interface QR code");
    
    // Web URL QR code - use simple URL format
    String webUrl = "http://192.168.4.1/";  // Remove trailing slash
    ESP_LOGI(LOG_TAG_COMMON, "Web URL: %s", webUrl.c_str());
    
    int result = qrcode_initText(&webQrCode, webQrCodeData, 3, ECC_LOW, webUrl.c_str());
    ESP_LOGI(LOG_TAG_COMMON, "Web QR code result: %d, size: %d", result, webQrCode.size);
}

void drawQRCode(QRCode *qrcode, int x, int y, int scale) {
    // Calculate QR code dimensions with quiet zone (4 modules minimum)
    int quietZone = 4;
    int totalSize = (qrcode->size + 2 * quietZone) * scale;
    
    // Clear the entire QR code area including quiet zone with white background
    tft.fillRect(x, y, totalSize, totalSize, ILI9341_WHITE);
    
    // Draw QR code modules with quiet zone offset
    int offsetX = x + (quietZone * scale);
    int offsetY = y + (quietZone * scale);
    
    for (int j = 0; j < qrcode->size; j++) {
        for (int i = 0; i < qrcode->size; i++) {
            if (qrcode_getModule(qrcode, i, j)) {
                tft.fillRect(offsetX + (i * scale), offsetY + (j * scale), scale, scale, ILI9341_BLACK);
            }
            // White modules are already handled by the background fill above
        }
    }
}

void showQRCodes() {
    ESP_LOGI(LOG_TAG_COMMON, "Displaying QR codes");
    
    // Generate QR codes
    generateWiFiQRCode();
    generateWebQRCode();
    
    // Clear screen
    tft.fillScreen(COLOR_BACKGROUND);
    
    // Draw header
    tft.fillRect(0, 0, 240, 50, COLOR_PRIMARY);
    tft.setTextColor(COLOR_TEXT);
    tft.setTextSize(2);
    
    String title = "Connection Info";
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    int titleX = (240 - w) / 2;
    tft.setCursor(titleX, 15);
    tft.print(title);
    
    // WiFi AP Section
    tft.setTextColor(COLOR_ACCENT);
    tft.setTextSize(1);
    tft.setCursor(10, 65);
    tft.print("WiFi Access Point:");
    
    tft.setTextColor(COLOR_TEXT);
    tft.setCursor(10, 80);
    tft.printf("SSID: %s", AP_SSID);
    tft.setCursor(10, 95);
    tft.printf("Password: %s", AP_PASSWORD);
    
    // Draw WiFi QR code (scale 3 with proper spacing and quiet zone)
    drawQRCode(&wifiQrCode, 2, 110, 3);   // x=2 to fit 111px QR (2+111=113, fits in 240px width)
    
    // Web Interface Section  
    tft.setTextColor(COLOR_ACCENT);
    tft.setCursor(125, 65);
    tft.print("Web Interface:");
    
    tft.setTextColor(COLOR_TEXT);
    tft.setCursor(125, 80);
    tft.print("URL:");
    tft.setCursor(125, 95);
    tft.print("http://192.168.4.1/  ");  // Shorter text to fit better
    
    // Draw Web QR code (scale 3 with proper spacing and quiet zone)
    drawQRCode(&webQrCode, 127, 110, 3);  // x=127 to fit 111px QR (127+111=238, fits in 240px width)
    
    // Instructions (moved down to avoid QR code overlap)
    tft.setTextColor(COLOR_SECONDARY);
    tft.setCursor(10, 235);  // Moved down from 220
    tft.print("Instructions:");
    tft.setTextColor(COLOR_TEXT);
    tft.setCursor(10, 250);  // Moved down from 235
    tft.print("1. Connect to WiFi (scan left QR)");
    tft.setCursor(10, 265);  // Moved down from 250
    tft.print("2. Open web page (scan right QR)");
    tft.setCursor(10, 280);  // Moved down from 265
    tft.print("3. Upload 240x320 PNG/JPG images");
    tft.setCursor(10, 295);  // Moved down from 280
    tft.print("4. View images on this display");
    
    // Status indicator (moved to fit)
    tft.fillCircle(225, 310, 6, COLOR_SECONDARY);  // Smaller circle, lower position
    tft.setCursor(185, 307);  // Adjusted position
    tft.setTextColor(COLOR_SECONDARY);
    tft.print("Ready");
}