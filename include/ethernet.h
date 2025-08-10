#ifndef _ETHERNET_H
#define _ETHERNET_H


#ifndef ETHERNET_BUFFER_SIZE
#define ETHERNET_BUFFER_SIZE 5
#endif

#define AP_SSID "ESP32-ImageDisplay"
#define AP_PASSWORD "12345678"

static bool wifi_connected = false;

extern AsyncWebServer server;

void WiFiEvent(WiFiEvent_t event);
void ethernet_init();
void displayImage(const char* filename);

#endif