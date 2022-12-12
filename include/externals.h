#ifndef EXTERNALS_H
#define EXTERNALS_H

#include "config.h"



////////////////////Externs////////////////////

extern WiFiClient espClient;  //Used to instantiate PubSubClient object below
extern PubSubClient mqttClient;  //Used for communication with MQTT broker via MQTT protocol
extern WiFiUDP ethernetUDP;  //Used for communication with NTP server via UDP protocol

extern bool pingCommandReceived;  //Triggers the sending of a ping message

extern Queue<String> dataSet;  //Primary rolling queue that continuously records measurements off of CPIN and VPIN
extern Queue<String> softCopy;  //Copy of queue after an excursion event occurs. Resource is shared between both threads 
extern pthread_mutex_t mutexHandle;  //Mutex to prevent conflicting operations on the shared resource softCopy

extern String publishTopicData;
extern String publishTopicInfo;
extern String subscribeTopic;

extern String globalClientID;
extern IPAddress globalNTPAddress;
extern float globalVoltageThreshold;  //Voltages above this threshold are reported to the broker.

extern time_t previousTime;
extern time_t currentTime;
extern unsigned short globalTimeCounter;  //Counter to differentiate timestamps that would otherwise be identical. 



////////////////////Network Object////////////////////

/* CLASS NAME: Network Object
 * PURPOSE: Contains network config information for the device (excluding NTP server address)
 */
class NetworkObject
{
  private:
    IPAddress clientIP;
    IPAddress clientDNS;
    IPAddress clientGateway;
    IPAddress clientSubnet;
    IPAddress mqttAddress;
    String site;
    String equipmentID;
        
  public:
    /* Constructor */
    NetworkObject( IPAddress clientIP_,
                   IPAddress clientDNS_,
                   IPAddress clientSubnet_,
                   IPAddress clientGateway_,
                   IPAddress mqttAddress_,
                   String site_,
                   String equipmentID_
                 );

    /* Getters */
    IPAddress getClientIP();
    IPAddress getClientDNS();
    IPAddress getClientGateway();
    IPAddress getClientSubnet();
    IPAddress getMQTTAddress();
    String getSite();
    String getEquipmentID();
    
    /* FUNCTION NAME: Eth Event
     * PURPOSE: Prints Ethernet status messages to Serial
     * ACTION: Checks for various Ethernet connection events
     */
    static void ethEvent(WiFiEvent_t event);
  
    /* FUNCTION NAME: Ethernet Init
     * PURPOSE: Configures and initiates Ethernet connection
     */
    void ethernetInit();
  
    /* FUNCTION NAME: MQTT Init
     * PURPOSE: Configures and initiates connection to MQTT broker, including formatting pub/sub topics
     */
    void mqttInit();

};



////////////////////Network Configuration Functions////////////////////

/* FUNCTION NAME: String to IP
 * PURPOSE: Converts an IP address from a String object to an IPAddress object
 */
IPAddress stringToIP(String IPString);


/* FUNCTION NAME: Get Chip ID
 * PURPOSE: Retrieves chip ID from microcontroller
 */
String getChipID();

/* FUNCTION NAME: Load Config
 * PURPOSE: Uses current config information to initiate a new network connection
 * ACTION: Retrieves config information from EEPROM and instantiates a new NetworkObject from it
 */
NetworkObject loadConfig();

/* FUNCTION NAME: Doc Inject
 * PURPOSE: Transfers targeted information from a source JsonDocument to a destination JsonDocument
 * ACTION: Transfers an individual piece of config information from sourceDoc to destinationDoc. User interface is provided if connected to Serial
 */
void docInject(const char* parameter, DynamicJsonDocument& destinationDoc, DynamicJsonDocument& sourceDoc, const char* mode);

/* FUNCTION NAME: Set Config
 * PURPOSE: Loads config information onto EEPROM
 * ACTION: Config information currently on EEPROM gets replaced appropriately with information in configMessage by repeatedly calling docInject
 */
void setConfig(const char* configMessage, const char* mode);  //Loads new config information from configMessage onto EEPROM



////////////////////Interrupt Functions////////////////////

/* FUNCTION NAME: Reconnect
 * PURPOSE: Attempts to reconnect to the MQTT broker if connection is lost
 * ACTION: Continuously attempts to reconnect while printing status messages to Serial
 */
void reconnect();

/* FUNCTION NAME: Reset
 * PURPOSE: Resets the device
 */
void reset();

/* FUNCTION NAME: IP To String
 * PURPOSE: Converts an IP address from an IPAddress object to a String for ease of display
 */
String ipToString(IPAddress ip);

/* FUNCTION NAME: Generate Ping
 * PURPOSE: Formats a ping message to be sent to MQTT broker
 * ACTION: Message contains current timestamp, current program version, and all current device config information
 */
String generatePing(NetworkObject object);

/* FUNCTION NAME: Callback
 * PURPOSE: Deals with all possible callback messages from MQTT broker
 * ACTION: Reconfigures the device, resets the devices, fulfills a ping request, or  depending on the callback message
 */
static void callback(char* topic, byte* payload, unsigned int length);  //Deals with all callback messages from MQTT broker



////////////////////Measurement Functions////////////////////

/* FUNCTION NAME: Get Voltage
 * PURPOSE: Gets value of EC20 input voltage
 * ACTION: Performs back calculations on voltage measured off of VPIN to get the original EC20 input voltage value
 */
String getVoltage();

/* FUNCTION NAME: Get Current
 * PURPOSE: Gets value of EC20 input current
 * ACTION: Performs back calculations on voltage measured off of CPIN to get the original EC20 input current value
 */
String getCurrent();

/* FUNCTION NAME: Get Time
 * PURPOSE: Formats timestamp for the current time
 * ACTION: Gets current time based on time reference. Also determines the value of globalTimeCounter based on extern previousTime
 */
String getTime();

/* FUNCTION NAME: Generate Entry
 * PURPOSE: Gets measurements and formats them into appropriate an JSON data string
 */
String generateEntry();



////////////////////NTP Functions////////////////////

/* FUNCTION NAME: Get Time Benchmark
 * PURPOSE: Gets a time benchmark (or time reference) from NTP server
 * ACTION: Sends a request message to the NTP server and then parses the response message for the time benchmark
 */
time_t getTimeBenchmark();

/* FUNCTION NAME: NTP Init
 * PURPOSE: Initiates connection to NTP server
 * ACTION: Initiates UDP protocol and sets getTimeBenchmark as the sync provider
 */
void ntpInit();



#endif
