#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include <ETH.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include "LittleFS.h"

#include "common.h"
#include "ethernet.h"
#include "image_display.h"

int duty = 0;

AsyncWebServer server(80);

void WiFiEvent(arduino_event_id_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_AP_START:
        ESP_LOGI(LOG_TAG_ETHERNET, "WiFi AP Started successfully");
        Serial.printf("[WIFI] Access Point started - SSID: %s\n", AP_SSID);
        wifi_connected = true;
        break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
        ESP_LOGI(LOG_TAG_ETHERNET, "WiFi AP Stopped");
        Serial.println("[WIFI] Access Point stopped");
        wifi_connected = false;
        break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        ESP_LOGI(LOG_TAG_ETHERNET, "Client connected to AP");
        Serial.println("[WIFI] Client device connected to AP");
        break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        ESP_LOGI(LOG_TAG_ETHERNET, "Client disconnected from AP");
        Serial.println("[WIFI] Client device disconnected from AP");
        break;
    default:
        ESP_LOGD(LOG_TAG_ETHERNET, "Unhandled WiFi event: %d", event);
        break;
    }
}

void ethernet_init()
{
    ESP_LOGI(LOG_TAG_ETHERNET, "Starting LittleFS initialization...");
    Serial.println("[FS] Initializing LittleFS filesystem...");
    
    if (!LittleFS.begin())
    {
        ESP_LOGE(LOG_TAG_ETHERNET, "CRITICAL: Failed to mount LittleFS!");
        Serial.println("[FS] ERROR: LittleFS mount failed! Attempting format...");
        
        // Try to format and mount again
        if (!LittleFS.begin(true)) // true = format if mount fails
        {
            ESP_LOGE(LOG_TAG_ETHERNET, "CRITICAL: LittleFS format failed!");
            Serial.println("[FS] ERROR: LittleFS format failed! Web interface disabled.");
            return;
        } else {
            ESP_LOGI(LOG_TAG_ETHERNET, "LittleFS formatted and mounted successfully");
            Serial.println("[FS] LittleFS formatted and mounted successfully");
        }
    }

    ESP_LOGI(LOG_TAG_ETHERNET, "LittleFS mounted successfully");
    Serial.println("[FS] LittleFS filesystem mounted successfully");
    
    // Show filesystem info
    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    Serial.printf("[FS] Filesystem - Total: %d bytes, Used: %d bytes, Free: %d bytes\n", 
                  totalBytes, usedBytes, totalBytes - usedBytes);

    // Create images directory if it doesn't exist
    if (!LittleFS.exists("/images")) {
        if (LittleFS.mkdir("/images")) {
            ESP_LOGI(LOG_TAG_ETHERNET, "Created /images directory");
            Serial.println("[FS] Created /images directory for image storage");
        } else {
            ESP_LOGE(LOG_TAG_ETHERNET, "Failed to create /images directory");
            Serial.println("[FS] ERROR: Failed to create /images directory");
        }
    } else {
        ESP_LOGI(LOG_TAG_ETHERNET, "/images directory already exists");
        Serial.println("[FS] /images directory already exists");
    }

    ESP_LOGI(LOG_TAG_ETHERNET, "Setting up WiFi Access Point...");
    Serial.printf("[WIFI] Configuring Access Point - SSID: %s, Password: %s\n", AP_SSID, AP_PASSWORD);
    
    WiFi.onEvent(WiFiEvent);

    // Setup WiFi AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    IPAddress IP = WiFi.softAPIP();
    ESP_LOGI(LOG_TAG_ETHERNET, "AP IP address: %s", IP.toString().c_str());
    Serial.printf("[WIFI] Access Point IP: %s\n", IP.toString().c_str());

    ESP_LOGI(LOG_TAG_ETHERNET, "Waiting for WiFi AP to start...");
    while (!wifi_connected)
    {
        ESP_LOGD(LOG_TAG_ETHERNET, "Waiting for AP start...");
        delay(500);
    }

    ESP_LOGI(LOG_TAG_ETHERNET, "Setting up web server endpoints...");
    Serial.println("[WEB] Configuring web server endpoints...");
    
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    ESP_LOGI(LOG_TAG_ETHERNET, "Static file serving configured");

    // Image upload endpoint
    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        ESP_LOGI(LOG_TAG_ETHERNET, "Image upload completed");
        Serial.println("[WEB] Image upload request completed");
        request->send(200, "application/json", "{\"success\":true}");
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        static File uploadFile;
        
        if (!index) {
            ESP_LOGI(LOG_TAG_ETHERNET, "Upload Start: %s", filename.c_str());
            Serial.printf("[UPLOAD] Starting upload: %s\n", filename.c_str());
            String path = "/images/" + filename;
            uploadFile = LittleFS.open(path, "w");
            if (!uploadFile) {
                ESP_LOGE(LOG_TAG_ETHERNET, "Failed to create file: %s", path.c_str());
                Serial.printf("[UPLOAD] ERROR: Failed to create file %s\n", path.c_str());
                return;
            }
        }
        
        if (uploadFile && len) {
            if (uploadFile.write(data, len) != len) {
                ESP_LOGE(LOG_TAG_ETHERNET, "Failed to write data chunk");
                Serial.printf("[UPLOAD] ERROR: Failed to write %d bytes\n", len);
            }
        }
        
        if (final) {
            if (uploadFile) {
                uploadFile.close();
                ESP_LOGI(LOG_TAG_ETHERNET, "Upload Complete: %s (%u bytes)", filename.c_str(), index + len);
                Serial.printf("[UPLOAD] Completed: %s (%u bytes)\n", filename.c_str(), index + len);
            }
        }
    });

    // Image list endpoint
    server.on("/images", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{\"images\":[";
        File root = LittleFS.open("/images");
        if (root) {
            File file = root.openNextFile();
            bool first = true;
            while (file) {
                if (!file.isDirectory()) {
                    if (!first) json += ",";
                    json += "{\"name\":\"" + String(file.name()) + "\",\"size\":" + String(file.size()) + "}";
                    first = false;
                }
                file = root.openNextFile();
            }
            root.close();
        }
        json += "]}";
        request->send(200, "application/json", json);
    });

    // Image serve endpoint
    server.on("/image/*", HTTP_GET, [](AsyncWebServerRequest *request) {
        String path = request->url();
        path.replace("/image/", "/images/");
        if (LittleFS.exists(path)) {
            request->send(LittleFS, path);
        } else {
            request->send(404, "text/plain", "Image not found");
        }
    });

    // Display image endpoint
    server.on("/display/*", HTTP_POST, [](AsyncWebServerRequest *request) {
        String filename = request->url();
        filename.replace("/display/", "");
        ESP_LOGI(LOG_TAG_ETHERNET, "Display request for: %s", filename.c_str());
        Serial.printf("[DISPLAY] Displaying image: %s\n", filename.c_str());
        displayImage(filename.c_str());
        request->send(200, "text/plain", "OK");
    });

    // Delete image endpoint
    server.on("/delete/*", HTTP_DELETE, [](AsyncWebServerRequest *request) {
        String filename = request->url();
        filename.replace("/delete/", "");
        String path = "/images/" + filename;
        ESP_LOGI(LOG_TAG_ETHERNET, "Delete request for: %s", filename.c_str());
        Serial.printf("[DELETE] Deleting image: %s\n", filename.c_str());
        if (LittleFS.remove(path)) {
            ESP_LOGI(LOG_TAG_ETHERNET, "Deleted: %s", filename.c_str());
            Serial.printf("[DELETE] Successfully deleted: %s\n", filename.c_str());
            request->send(200, "text/plain", "OK");
        } else {
            ESP_LOGE(LOG_TAG_ETHERNET, "Failed to delete: %s", filename.c_str());
            Serial.printf("[DELETE] ERROR: Failed to delete: %s\n", filename.c_str());
            request->send(404, "text/plain", "File not found");
        }
    });

    server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  request->send(200, "text/plain", "Rebooting...");
				  ESP_LOGI(LOG_TAG_ETHERNET, "Rebooting...");
                  delay(100);
                  ESP.restart(); });

    // server.on(
    // 	"/fan",
    // 	HTTP_POST,
    // 	[](AsyncWebServerRequest *request) {},
    // 	NULL,
    // 	[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
    // 	{
    // 		data[len] = '\0';
    // 		duty = (int)(atoi((const char *)data) / 100.0f * 255.0f);
    // 		ESP_LOGI(LOG_TAG_ETHERNET, "Fan duty: %d", duty);

    // 		ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
    // 		ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    // 	});

    server.on("/update/file", HTTP_POST, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK"); }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
              {
                  if (!index)
                  {
                      ESP_LOGI(LOG_TAG_ETHERNET, "Update Start: %s\n", filename.c_str());
                      int command;
                      if(filename == "firmware.bin") {
                          command = U_FLASH;
                      } else if(filename == "littlefs.bin") {
                          command = U_SPIFFS;
                      } else {
                          ESP_LOGI(LOG_TAG_ETHERNET, "Unknown file type");
                          return;
                      }
                      if (!Update.begin(UPDATE_SIZE_UNKNOWN, command))
                      {
                          Update.printError(Serial);
                      }
                  }
                  if (!Update.hasError())
                  {
                      if (Update.write(data, len) != len)
                      {
                          Update.printError(Serial);
                      }
                  }
                  if (final)
                  {
                      if (Update.end(true))
                      {
                          ESP_LOGI(LOG_TAG_ETHERNET, "Update Success: %uB\n", index + len);
                        //   vTaskDelay(10000 / portTICK_PERIOD_MS);
						//   ESP_LOGW(LOG_TAG_ETHERNET, "!!!ESP REBOOT TRIGGER!!!");
                        //   vTaskDelay(5000 / portTICK_PERIOD_MS);
						//   ESP_LOGI(LOG_TAG_ETHERNET, "Rebooting...");
                        //   ESP.restart();
                      }
                      else
                      {
                          Update.printError(Serial);
                      }
                  } });

    // server.on("/api", HTTP_GET,
    //           [](AsyncWebServerRequest *request)
    //           {
    //             request->send(200, "application/json; charset=utf-8\nAccess-Control-Allow-Origin: *", (const char *)json_data.Invert_parse());
    //           });

    server.begin();
    ESP_LOGI(LOG_TAG_ETHERNET, "HTTP server started on port 80");
    Serial.println("[WEB] HTTP server started successfully");
    Serial.printf("[WEB] Web interface available at: http://%s/\n", WiFi.softAPIP().toString().c_str());
    Serial.println("[WEB] Server endpoints configured:");
    Serial.println("  GET  / - Main web interface");
    Serial.println("  POST /upload - Image upload");
    Serial.println("  GET  /images - Image list API");
    Serial.println("  GET  /image/* - Serve image files");
    Serial.println("  POST /display/* - Display image on TFT");
    Serial.println("  DELETE /delete/* - Delete image");
    Serial.println("  GET  /reboot - System reboot");
    
}

void displayImage(const char* filename) {
    displayImageFromFile(filename);
}