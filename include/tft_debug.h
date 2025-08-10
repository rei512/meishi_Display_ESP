#ifndef _TFT_DEBUG_H
#define _TFT_DEBUG_H

#include <Adafruit_ILI9341.h>
#include <Arduino.h>

extern Adafruit_ILI9341 tft;

// TFT Debug display settings
#define TFT_DEBUG_LINES 26          // Number of lines that fit on screen (320/12)
#define TFT_DEBUG_CHAR_WIDTH 6      // Character width in pixels
#define TFT_DEBUG_CHAR_HEIGHT 12    // Character height in pixels  
#define TFT_DEBUG_LINE_HEIGHT 12    // Line height in pixels
#define TFT_DEBUG_MAX_CHARS 40      // Max characters per line (240/6)

// Color definitions for log levels
#define TFT_COLOR_INFO     ILI9341_WHITE
#define TFT_COLOR_DEBUG    ILI9341_CYAN
#define TFT_COLOR_WARN     ILI9341_YELLOW
#define TFT_COLOR_ERROR    ILI9341_RED
#define TFT_COLOR_SUCCESS  ILI9341_GREEN
#define TFT_COLOR_SYSTEM   ILI9341_MAGENTA

// TFT Debug functions
void tftDebugInit();
void tftDebugPrint(const char* message, uint16_t color = TFT_COLOR_INFO);
void tftDebugPrintln(const char* message, uint16_t color = TFT_COLOR_INFO);
void tftDebugPrintf(uint16_t color, const char* format, ...);
void tftDebugClear();
void tftDebugScroll();

// Convenience macros for different log levels
#define TFT_LOG_INFO(msg)    tftDebugPrintln(msg, TFT_COLOR_INFO)
#define TFT_LOG_DEBUG(msg)   tftDebugPrintln(msg, TFT_COLOR_DEBUG)
#define TFT_LOG_WARN(msg)    tftDebugPrintln(msg, TFT_COLOR_WARN)
#define TFT_LOG_ERROR(msg)   tftDebugPrintln(msg, TFT_COLOR_ERROR)
#define TFT_LOG_SUCCESS(msg) tftDebugPrintln(msg, TFT_COLOR_SUCCESS)
#define TFT_LOG_SYSTEM(msg)  tftDebugPrintln(msg, TFT_COLOR_SYSTEM)

// Printf-style macros
#define TFT_LOGF_INFO(fmt, ...)    tftDebugPrintf(TFT_COLOR_INFO, fmt, ##__VA_ARGS__)
#define TFT_LOGF_DEBUG(fmt, ...)   tftDebugPrintf(TFT_COLOR_DEBUG, fmt, ##__VA_ARGS__)
#define TFT_LOGF_WARN(fmt, ...)    tftDebugPrintf(TFT_COLOR_WARN, fmt, ##__VA_ARGS__)
#define TFT_LOGF_ERROR(fmt, ...)   tftDebugPrintf(TFT_COLOR_ERROR, fmt, ##__VA_ARGS__)
#define TFT_LOGF_SUCCESS(fmt, ...) tftDebugPrintf(TFT_COLOR_SUCCESS, fmt, ##__VA_ARGS__)
#define TFT_LOGF_SYSTEM(fmt, ...)  tftDebugPrintf(TFT_COLOR_SYSTEM, fmt, ##__VA_ARGS__)

#endif