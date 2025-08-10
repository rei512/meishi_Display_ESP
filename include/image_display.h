#ifndef _IMAGE_DISPLAY_H
#define _IMAGE_DISPLAY_H

#include <Adafruit_ILI9341.h>
#include "LittleFS.h"
#include <TJpg_Decoder.h>
#include <PNGdec.h>

extern Adafruit_ILI9341 tft;

// Image display functions
void displayImageFromFile(const char* filename);
void displayImageWithScaling(const char* filename, bool centerImage = true);
void clearDisplay();

// PNG functions
bool drawPNG(const char* filename, int16_t x, int16_t y);
int pngDraw(PNGDRAW *pDraw);

// JPEG functions  
bool drawJPEG(const char* filename, int16_t x, int16_t y);
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);

// File reading functions
void* pngOpen(const char* filename, int32_t* size);
void pngClose(void* handle);
int32_t pngRead(PNGFILE* handle, uint8_t* buffer, int32_t length);
int32_t pngSeek(PNGFILE* handle, int32_t position);

#endif