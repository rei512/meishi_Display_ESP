/**
 * @file common.h
 * @brief Common definitions and configurations for the T-ETH Lite project.
 *
 * This header file contains macro definitions, constants, and type definitions
 * used across various modules of the T-ETH Lite project.
 */

#ifndef _COMMON_H
#define _COMMON_H

#define TITLE "Test"
#define AUTHOR "Deltav-lab."
#define VERSION "v0.01"
#define DESCRIPTION "test"
#define DATE "2025-08-03"


// Define the log tag for each module
#define LOG_TAG_COMMON "common"
#define LOG_TAG_ETHERNET "ethernet"
#define LOG_TAG_TCP "tcp"
#define LOG_TAG_CAN "can"
#define LOG_TAG_OTA "ota"
#define LOG_TAG_TASK "task_serv"

#define TASK_PRIO_ETH (5)
#define TASK_PRIO_CAN (2)
#define TASK_PRIO_WS (3)

#define CAN_BUFFER_SIZE 64
#define ETHERNET_BUFFER_SIZE 5
#define WS_BUFFER_SIZE 5
#define WS_RX_BUFFER_STR_LENGTH 20
#define WS_TX_BUFFER_STR_LENGTH 2500

#define BOARD_NUM 17 // Number of boards

#define WS_SEND_INTERVAL 50	   // ms
#define CAN_CH_CHANGE_INTERVAL 100 // ms
#define CAN_CH_CHANGE_RETRY 10

#define CMDID_CHANGE_OUTPUT_STATE 0b001
#define CMDID_VOLTAGE_CURRENT 0b011
#define CMDID_TEMPERATURE 0b100

#define FAN_PWM_PIN GPIO_NUM_15
#define FAN_FREQ 25000
#define FAN_CHANNEL LEDC_CHANNEL_0
#define FAN_PWM_RESOLUTION LEDC_TIMER_8_BIT

#define BUTTON_PIN GPIO_NUM_2

typedef struct
{
	float voltage;
	float current;
	float temperature;
	bool output_state;
} channel_data_t;

#define CONFIG_STATIC_IP 0

#if CONFIG_STATIC_IP
#define IP(a, b, c, d) ((uint32_t)(((d) & 0xff) << 24) | ((uint32_t)(((c) & 0xff) << 16)) | ((uint32_t)(((b) & 0xff) << 8)) | (uint32_t)((a) & 0xff))
#define ETH_MUSK(a) ((uint32_t)(htonl(0xffffffff << (32 - (a)))))

#define ETH_IPV4 IP(10, 240, 2, 180)
#define ETH_GATEWAY IP(10, 240, 2, 254)
#define ETH_SUBNET ETH_MUSK(23)
#else
#define CONFIG_DHCP 1
#endif

#endif