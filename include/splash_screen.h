#ifndef _SPLASH_SCREEN_H
#define _SPLASH_SCREEN_H

#include <Adafruit_ILI9341.h>
#include <qrcode.h>

extern Adafruit_ILI9341 tft;

// Splash screen and QR code functions
void showSplashScreen();
void showQRCodes();
void drawQRCode(QRCode *qrcode, int x, int y, int scale);
void generateWiFiQRCode();
void generateWebQRCode();

// Colors (RGB565 format)
#define COLOR_PRIMARY   0x1C47      // Blue
#define COLOR_SECONDARY 0x2323      // Green  
#define COLOR_ACCENT    0xFD20      // Orange
#define COLOR_TEXT      0xFFFF      // White
#define COLOR_BACKGROUND 0x2965     // Dark Blue-Gray

#endif