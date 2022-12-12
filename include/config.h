#ifndef CONFIG_H
#define CONFIG_H



////////////////////Libraries////////////////////

#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ETH.h>

#include "Queue.h"
#include <string.h>

#include <TimeLib.h>
#include <WiFiUDP.h>

#include <EEPROM.h>
#include <StreamUtils.h>
#include <ArduinoJson.h>

#include <pthread.h>



////////////////////Constants////////////////////

#define VERSION "1.1"

#define QUEUE_RANGE 40  //Length of the rolling queue object aka the max number of measurements the queue can hold
#define OVERRIDE_RANGE 35  //Number of times the device will push new measurements to the rolling queue after it detects an excursion event. Must be less than QUEUE_RANGE

#define CPIN 14  //Pin on board measuring voltage as a factor of EC20 input current
#define VPIN 15  //Pin on board measuring voltage as a factor of EC20 input voltage

#define TIMEZONE -6  //Number of hours ahead(+) or behind(-) Unix time according to device timezone e.g. MDT is 6 hours behind UTC so use -6
#define NTP_PORT 8888
#define UDP_PORT 123
#define NTP_MESSAGE_SIZE 48  //Size of messages being sent back and forth from NTP server

#define MQTT_PORT 1883
#define MQTT_USERNAME "demoSPOOF"
#define MQTT_PASSWORD "howdyhowdy69"
#define ROOT_TOPIC "NARCCCCC!"
 
#define JSON_BUFFER_CAPACITY JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(9) + 208  //Provides enough buffer room for any possible JSON string formed
#define PUBLISH_BUFFER_SIZE 300  //Max size of messages sent to MQTT broker



///////////////////Ethernet Configuration//////////////////////////////////////////

//#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12

//#define ETH_CLK_MODE ETH_CLOCK_GPIO0_OUT  //Version with PSRAM
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT  //Version with no PSRAM

#define ETH_POWER_PIN -1  //Pin# of the enable signal for the external crystal oscillator (-1 to disable for internal APLL source)

#define ETH_TYPE ETH_PHY_LAN8720  //Type of the Ethernet PHY (LAN8720 or TLK110)

#define ETH_ADDR 0  //I²C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110)

#define ETH_MDC_PIN 23  //Pin# of the I²C clock signal for the Ethernet PHY

#define ETH_MDIO_PIN 18  //Pin# of the I²C IO signal for the Ethernet PHY

#define NRST 5



#endif
