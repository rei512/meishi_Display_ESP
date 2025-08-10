# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-based business card display system ("meishi Display ESP") that combines:
- ILI9341 TFT display for visual output
- Ethernet/WiFi connectivity with web server
- LittleFS filesystem for web content
- OTA (Over-The-Air) update capability

The system displays business card information on a physical display while serving a web interface for configuration and updates.

## Build System

This project uses **PlatformIO** with the Arduino framework for ESP32 development.

### Common Commands

```bash
# Build the project
pio run

# Build for specific environment
pio run -e esp32-c3-devkitm-1
pio run -e upesy_wroom

# Upload firmware
pio run --target upload

# Upload filesystem (LittleFS)
pio run --target uploadfs

# Clean build files
pio run --target clean

# Monitor serial output
pio device monitor
```

### Environments

Two build environments are configured:
- `upesy_wroom` - Basic environment with display libraries
- `esp32-c3-devkitm-1` - Full environment with web server and OTA support

## Architecture

### Core Modules

- **main.cpp**: Entry point, display initialization, and main loop
- **ethernet.cpp/h**: Network connectivity, web server setup, and OTA updates
- **common.h**: Shared constants, data structures, and system configuration

### Hardware Configuration

Display pins (main.cpp:42-48):
- CS: GPIO 5, RESET: GPIO 6, DC: GPIO 7
- MOSI: GPIO 8, SCK: GPIO 9, LED: GPIO 10, MISO: GPIO 20

### Web Server Features

- Static file serving from LittleFS (`/data/` folder)
- OTA firmware updates via `/update/file`
- System reboot endpoint at `/reboot`
- Japanese language web interface for business card display

### Network Configuration

The system supports both DHCP and static IP configuration:
- DHCP by default (`CONFIG_DHCP=1`)
- Static IP option available in common.h (`CONFIG_STATIC_IP`)
- WiFi credentials referenced as `AP_SSID` and `AP_PASSWORD` (not defined in codebase)

### File System

- Uses LittleFS for web content storage
- Custom partition table with OTA support (1MB each partition)
- Web files stored in `/data/` directory

## Important Notes

- WiFi credentials (`AP_SSID`, `AP_PASSWORD`) are referenced but not defined - these need to be provided
- OTA update system supports both firmware.bin and littlefs.bin uploads
- System uses ESP-IDF logging with module-specific tags
- Display initialization happens in setup() but main display logic is not implemented in loop()