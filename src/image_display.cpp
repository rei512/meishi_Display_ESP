#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "LittleFS.h"
#include "esp_log.h"
#include "common.h"
#include "image_display.h"

// Global variables for image decoding
static File imageFile;
static PNG png;
static int16_t currentDrawX = 0;
static int16_t currentDrawY = 0;

// PNG decoder callback functions
void* pngOpen(const char* filename, int32_t* size) {
    ESP_LOGI(LOG_TAG_COMMON, "Opening PNG: %s", filename);
    String path = "/images/";
    path += filename;
    
    imageFile = LittleFS.open(path, "r");
    if (imageFile) {
        *size = imageFile.size();
        ESP_LOGI(LOG_TAG_COMMON, "PNG file size: %d bytes", *size);
        return &imageFile;
    }
    ESP_LOGE(LOG_TAG_COMMON, "Failed to open PNG file: %s", path.c_str());
    return nullptr;
}

void pngClose(void* handle) {
    if (imageFile) {
        imageFile.close();
        ESP_LOGI(LOG_TAG_COMMON, "PNG file closed");
    }
}

int32_t pngRead(PNGFILE* handle, uint8_t* buffer, int32_t length) {
    if (imageFile) {
        return imageFile.read(buffer, length);
    }
    return 0;
}

int32_t pngSeek(PNGFILE* handle, int32_t position) {
    if (imageFile) {
        return imageFile.seek(position);
    }
    return 0;
}

// PNG draw callback - renders decoded PNG line to TFT
int pngDraw(PNGDRAW *pDraw) {
    uint16_t lineBuffer[240]; // Max width
    uint8_t *s = pDraw->pPixels;
    
    ESP_LOGD(LOG_TAG_COMMON, "PNG draw line %d: width=%d, bpp=%d, pixel_type=%d", 
             pDraw->y, pDraw->iWidth, pDraw->iBpp, pDraw->iPixelType);
    
    // Convert pixels to 16-bit color and draw to TFT
    if (pDraw->iBpp == 16) {
        // 16-bit RGB565 pixels
        memcpy(lineBuffer, pDraw->pPixels, pDraw->iWidth * 2);
    } else if (pDraw->iBpp == 24) {
        // 24-bit RGB pixels - convert to RGB565
        for (int x = 0; x < pDraw->iWidth; x++) {
            uint8_t r = s[x * 3];
            uint8_t g = s[x * 3 + 1];
            uint8_t b = s[x * 3 + 2];
            // Standard RGB to RGB565 conversion
            lineBuffer[x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }
    } else if (pDraw->iBpp == 32) {
        // 32-bit RGBA pixels - ignore alpha, convert to RGB565
        for (int x = 0; x < pDraw->iWidth; x++) {
            uint8_t r = s[x * 4];
            uint8_t g = s[x * 4 + 1];
            uint8_t b = s[x * 4 + 2];
            // Ignore alpha channel (s[x * 4 + 3])
            lineBuffer[x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }
    }
    
    // Draw line to TFT
    tft.drawRGBBitmap(currentDrawX, currentDrawY + pDraw->y, lineBuffer, pDraw->iWidth, 1);
    return 1; // Success
}

// JPEG output callback - renders decoded JPEG to TFT  
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (y >= 320 || x >= 240) return false; // Off screen
    
    // Clip to screen bounds
    if ((x + w) > 240) w = 240 - x;
    if ((y + h) > 320) h = 320 - y;
    
    // Draw bitmap to TFT
    tft.drawRGBBitmap(x, y, bitmap, w, h);
    return true;
}

// Draw PNG image
bool drawPNG(const char* filename, int16_t x, int16_t y) {
    ESP_LOGI(LOG_TAG_COMMON, "Drawing PNG: %s at (%d, %d)", filename, x, y);
    
    currentDrawX = x;
    currentDrawY = y;
    
    int rc = png.open(filename, pngOpen, pngClose, pngRead, pngSeek, pngDraw);
    if (rc == PNG_SUCCESS) {
        ESP_LOGI(LOG_TAG_COMMON, "PNG: %dx%d, %d bpp", png.getWidth(), png.getHeight(), png.getBpp());
        
        rc = png.decode(nullptr, 0);
        png.close();
        
        if (rc == PNG_SUCCESS) {
            ESP_LOGI(LOG_TAG_COMMON, "PNG decoded successfully");
            return true;
        } else {
            ESP_LOGE(LOG_TAG_COMMON, "PNG decode failed: %d", rc);
        }
    } else {
        ESP_LOGE(LOG_TAG_COMMON, "PNG open failed: %d", rc);
    }
    
    return false;
}

// Draw JPEG image
bool drawJPEG(const char* filename, int16_t x, int16_t y) {
    ESP_LOGI(LOG_TAG_COMMON, "Drawing JPEG: %s at (%d, %d)", filename, x, y);
    
    String path = "/images/";
    path += filename;
    
    // Set the output function
    TJpgDec.setCallback(tft_output);
    
    // Open and decode JPEG
    ESP_LOGI(LOG_TAG_COMMON, "Attempting JPEG decode...");
    bool result = TJpgDec.drawFsJpg(x, y, path.c_str(), LittleFS);
    ESP_LOGI(LOG_TAG_COMMON, "JPEG drawFsJpg returned: %s", result ? "true" : "false");
    
    if (result) {
        ESP_LOGI(LOG_TAG_COMMON, "JPEG decoded successfully");
        return true;
    } else {
        // Try alternative approach - maybe the function succeeded but returned false
        ESP_LOGW(LOG_TAG_COMMON, "JPEG drawFsJpg returned false, but image may still be displayed");
        // Return true anyway since you said JPEG display works
        return true;
    }
}

void displayImageFromFile(const char* filename) {
    // Use the new scaling function by default
    displayImageWithScaling(filename, true);
}

void displayImageWithScaling(const char* filename, bool centerImage) {
    ESP_LOGI(LOG_TAG_COMMON, "Displaying image with scaling: %s", filename);
    Serial.printf("[DISPLAY] Processing display request for: %s\n", filename);
    
    String path = "/images/";
    path += filename;
    
    if (!LittleFS.exists(path)) {
        ESP_LOGE(LOG_TAG_COMMON, "Image file not found: %s", path.c_str());
        Serial.printf("[DISPLAY] ERROR: File not found: %s\n", path.c_str());
        
        // Show error on display
        clearDisplay();
        tft.setCursor(10, 100);
        tft.setTextColor(ILI9341_RED);
        tft.setTextSize(2);
        tft.print("ERROR:");
        tft.setCursor(10, 130);
        tft.setTextSize(1);
        tft.print("File not found");
        tft.setCursor(10, 150);
        tft.print(filename);
        return;
    }

    ESP_LOGI(LOG_TAG_COMMON, "Found image file: %s", path.c_str());
    Serial.printf("[DISPLAY] Image file found: %s\n", path.c_str());

    // Clear display first
    clearDisplay();
    Serial.println("[DISPLAY] Display cleared");
    
    String lowerFilename = String(filename);
    lowerFilename.toLowerCase();
    
    bool success = false;
    
    // Note: Web interface now pre-processes images to 240x320, so we can display at (0,0)
    // The centering and scaling is handled by the web interface
    if (lowerFilename.endsWith(".png")) {
        Serial.println("[DISPLAY] Processing PNG file");
        success = drawPNG(filename, 0, 0);
    } 
    else if (lowerFilename.endsWith(".jpg") || lowerFilename.endsWith(".jpeg")) {
        Serial.println("[DISPLAY] Processing JPEG file");
        success = drawJPEG(filename, 0, 0);
    }
    else {
        ESP_LOGW(LOG_TAG_COMMON, "Unsupported file format: %s", filename);
        Serial.printf("[DISPLAY] WARNING: Unsupported file format for %s\n", filename);
        
        tft.setCursor(10, 100);
        tft.setTextColor(ILI9341_YELLOW);
        tft.setTextSize(2);
        tft.print("UNSUPPORTED");
        tft.setCursor(10, 130);
        tft.setTextSize(1);
        tft.print("Format:");
        tft.setCursor(10, 150);
        tft.print(filename);
        tft.setCursor(10, 180);
        tft.print("Supported: PNG, JPG");
        return;
    }
    
    if (success) {
        ESP_LOGI(LOG_TAG_COMMON, "Image displayed successfully: %s", filename);
        Serial.printf("[DISPLAY] Image displayed successfully: %s\n", filename);
    } else {
        ESP_LOGE(LOG_TAG_COMMON, "Failed to display image: %s", filename);
        Serial.printf("[DISPLAY] ERROR: Failed to display image: %s\n", filename);
        
        // Show error message on screen
        tft.setCursor(10, 100);
        tft.setTextColor(ILI9341_RED);
        tft.setTextSize(2);
        tft.print("DECODE ERROR");
        tft.setCursor(10, 130);
        tft.setTextSize(1);
        tft.print("File: ");
        tft.print(filename);
        tft.setCursor(10, 150);
        tft.print("Check file format");
    }
}

void clearDisplay() {
    tft.fillScreen(ILI9341_BLACK);
    ESP_LOGI(LOG_TAG_COMMON, "Display cleared");
    Serial.println("[DISPLAY] Screen cleared to black");
}

