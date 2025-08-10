#include "tft_debug.h"
#include <stdarg.h>
#include <stdio.h>

// Global variables for TFT debug
static int currentLine = 0;
static bool tftDebugInitialized = false;

void tftDebugInit() {
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(1);
    tft.setTextWrap(true);
    currentLine = 0;
    tftDebugInitialized = true;
    
    // Display header
    tft.setTextColor(TFT_COLOR_SYSTEM);
    tft.setCursor(0, 0);
    tft.println("ESP32-C3 DEBUG CONSOLE");
    tft.setTextColor(TFT_COLOR_DEBUG);
    tft.println("========================");
    currentLine = 2;
}

void tftDebugClear() {
    tft.fillScreen(ILI9341_BLACK);
    currentLine = 0;
    
    // Redisplay header
    tft.setTextColor(TFT_COLOR_SYSTEM);
    tft.setCursor(0, 0);
    tft.println("ESP32-C3 DEBUG CONSOLE");
    tft.setTextColor(TFT_COLOR_DEBUG);
    tft.println("========================");
    currentLine = 2;
}

void tftDebugScroll() {
    if (!tftDebugInitialized) return;
    
    // Scroll the display up by one line
    // This is a simplified approach - clear screen and start over
    if (currentLine >= TFT_DEBUG_LINES - 1) {
        // Clear bottom half and reset cursor
        tft.fillRect(0, TFT_DEBUG_LINE_HEIGHT * 15, 240, 320 - (TFT_DEBUG_LINE_HEIGHT * 15), ILI9341_BLACK);
        currentLine = 15;
    }
}

void tftDebugPrint(const char* message, uint16_t color) {
    if (!tftDebugInitialized) {
        tftDebugInit();
    }
    
    tftDebugScroll();
    
    tft.setTextColor(color);
    tft.setCursor(0, currentLine * TFT_DEBUG_LINE_HEIGHT);
    
    // Print message, handling long lines
    String msg = String(message);
    if (msg.length() > TFT_DEBUG_MAX_CHARS) {
        msg = msg.substring(0, TFT_DEBUG_MAX_CHARS - 3) + "...";
    }
    
    tft.print(msg);
}

void tftDebugPrintln(const char* message, uint16_t color) {
    tftDebugPrint(message, color);
    currentLine++;
    
    if (currentLine >= TFT_DEBUG_LINES) {
        tftDebugScroll();
    }
}

void tftDebugPrintf(uint16_t color, const char* format, ...) {
    if (!tftDebugInitialized) {
        tftDebugInit();
    }
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    tftDebugPrintln(buffer, color);
}