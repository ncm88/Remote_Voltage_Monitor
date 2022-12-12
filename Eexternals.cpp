#include "externals.h"
#include "config.h"
#include "Queue.h"



////////////////////Externs////////////////////

WiFiClient espClient;
PubSubClient mqttClient(espClient);
WiFiUDP ethernetUDP;

bool pingCommandReceived = false;

Queue<String> dataSet(QUEUE_RANGE);
Queue<String> softCopy(QUEUE_RANGE);
pthread_mutex_t mutexHandle;

String publishTopicData = "";
String publishTopicInfo = "";
String subscribeTopic = "";

String globalClientID = "";
IPAddress globalNTPAddress;
float globalVoltageThreshold;

time_t previousTime = 0;
time_t currentTime = 0;
unsigned short globalTimeCounter = 0;



////////////////////Network Object////////////////////

//Constructor
NetworkObject::NetworkObject(IPAddress clientIP_,IPAddress clientDNS_,IPAddress clientGateway_,
                             IPAddress clientSubnet_,IPAddress mqttAddress_,String site_,String equipmentID_)
{
  pingCommandReceived = false;
    
  this->clientIP = clientIP_;
  this->clientDNS = clientDNS_;
  this->clientGateway = clientGateway_;
  this->clientSubnet = clientSubnet_;

  this->mqttAddress = mqttAddress_;
  this->site = site_;
  this->equipmentID = equipmentID_;

  ethernetInit();
  ntpInit();
  mqttInit();
}



//Getter functions
IPAddress NetworkObject::getClientIP() { return this->clientIP; }
IPAddress NetworkObject::getClientDNS() { return this->clientDNS; }
IPAddress NetworkObject::getClientGateway() { return this->clientGateway; }
IPAddress NetworkObject::getClientSubnet() { return this->clientSubnet; }

IPAddress NetworkObject::getMQTTAddress() { return this->mqttAddress; }
String NetworkObject::getSite() { return this->site; }
String NetworkObject::getEquipmentID() { return this->equipmentID; }



/**
 * @brief Sets hostname if necessary, prints diagnostic data for debug purposes
 * 
 * @param event Ethernet signal
 */
void NetworkObject::ethEvent(WiFiEvent_t event)
{
  switch (event)
  {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      ETH.setHostname("esp32-ethernet"); //Set Eth hostname here
      break;
    
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      
      if (ETH.fullDuplex())
        Serial.print(", FULL_DUPLEX");

      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      
      break;
    
    return;
  }
}


/**
 * @brief Initializes ethernet connection
 * 
 */
void NetworkObject::ethernetInit()
{
  WiFi.onEvent(ethEvent);
  ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
  ETH.config(getClientIP(), getClientGateway(), getClientSubnet(), getClientDNS());
}


/**
 * @brief Initializes MQTT Connection via Ethernet
 * 
 */
void NetworkObject::mqttInit()
{
  mqttClient.setServer(getMQTTAddress(), MQTT_PORT);
  mqttClient.setCallback(callback);
  mqttClient.setBufferSize(PUBLISH_BUFFER_SIZE);
  
  publishTopicData = String(ROOT_TOPIC) + "/" + getSite() + "/" + getEquipmentID() + "/Data";
  publishTopicInfo = String(ROOT_TOPIC) + "/" + getSite() + "/" + getEquipmentID() +  "/Info";
  subscribeTopic = String(ROOT_TOPIC) + "/" + getSite() + "/" + globalClientID;
}



////////////////////Network Configuration Functions////////////////////

/**
 * @brief Takes a string argument and convert to IP address object for network function utility
 * While strange for true embedded development, I opted to work with strings here as the standard intermediary value as it was easy to work with the EEPROM (dynamic reconfig) and allowed easy conversion to IP address and ArduinoJSON objects
 * @param IPString 
 * @return IPAddress 
 */
IPAddress stringToIP(String IPString)
{
  int addr[4];
  int commaCount = 0;
  String entry;

  IPString += "."; //Ensures the fourth octet in IP gets added to addr array

  for( int i = 0; i < IPString.length(); i++) 
  {
    if(IPString[i] != '.')
    {
      entry += IPString[i];
    }
    else
    {
      addr[commaCount] = entry.toInt();
      commaCount += 1;
      entry = "";
    }

  }

  IPAddress newIP(addr[0], addr[1], addr[2], addr[3]);
  return newIP;
}


/**
 * @brief Get the Chip ID object
 * 
 * @return String 
 */
String getChipID()
{
  uint32_t chipID = 0;
  
  for(int i=0; i<17; i=i+8)
	  chipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  
  return String(chipID);
}
  

/**
 * @brief Instantiates a NetworkObject from the JSON document stored on the system's EEPROM
 * 
 * @return NetworkObject with new configuration info
 */
NetworkObject loadConfig()
{
  IPAddress clientIP_;
  IPAddress clientDNS_;
  IPAddress clientGateway_;
  IPAddress clientSubnet_;
  IPAddress mqttAddress_;
  String site_;
  String equipmentID_;


  DynamicJsonDocument configDoc(JSON_BUFFER_CAPACITY);
  EepromStream streamFromEEPROM(0,JSON_BUFFER_CAPACITY);
  deserializeJson(configDoc, streamFromEEPROM);

 
  if(configDoc["IP"]){
    String ipString = configDoc["IP"];
    clientIP_ = stringToIP(ipString);
  }


  if(configDoc["DNS"]){
    String dnsString = configDoc["DNS"];
    clientDNS_ = stringToIP(dnsString) ;
  }


  if(configDoc["SUBNET"]){
    String subnetString = configDoc["SUBNET"];
    clientSubnet_ = stringToIP(subnetString) ;
  }


  if(configDoc["GATEWAY"]){
    String gatewayString = configDoc["GATEWAY"];
    clientGateway_ = stringToIP(gatewayString) ;
  }


  if(configDoc["MQTT"]){
    String mqttString = configDoc["MQTT"];
    mqttAddress_ = stringToIP(mqttString) ;
  }


  if(configDoc["NTP"]){
    String ntpString = configDoc["NTP"];
    globalNTPAddress = stringToIP(ntpString) ;
  }


  if(configDoc["SITE"]){
    String siteString = configDoc["SITE"];
    site_ = siteString;
  }
  

  if(configDoc["EQUIPMENTID"]){
    String eqpString = configDoc["EQUIPMENTID"];
    equipmentID_ = eqpString;
  }


  if(configDoc["CLIENTID"]){
    String idString = configDoc["CLIENTID"];
    globalClientID = idString;
  }
  else globalClientID = getChipID();

  
  if(configDoc["VTHRESHOLD"]){
    String vString = configDoc["VTHRESHOLD"];
    globalVoltageThreshold = vString.toFloat();
  }
  
  
  NetworkObject networkHandler(clientIP_, clientDNS_, clientGateway_, clientSubnet_, mqttAddress_, site_, equipmentID_);


  return networkHandler;
}


/**
 * @brief Writes contents of source document into destination document
 * 
 * @param parameter JSON subcontent accessor string
 * @param destinationDoc 
 * @param sourceDoc 
 * @param mode Mode dictating whether system reconfiguration is being performed via SPI or MQTT
 */
void docInject(const char* parameter, DynamicJsonDocument& destinationDoc, DynamicJsonDocument& sourceDoc, const char* mode)
{
  const char* value = sourceDoc[parameter];
  
  if(value)
  {
    if(strcmp(mode, "SERIAL") == 0)
    {
      Serial.print("\nConfirm ");
      Serial.print(parameter);
      Serial.print(" = ");
      Serial.print(value);
      Serial.print("\n(Y/N)\n");
    
      String answer;
      while(true)
      {
        if(Serial.available())
        {
          answer = Serial.readStringUntil('\n');
          break;
        }
      }

    if(answer == "Y" || answer == "y")
      destinationDoc[parameter] = value;
    }
    
    else if(strcmp(mode, "MQTT") == 0)
    {
      destinationDoc[parameter] = value;
    }
   
  }
}


/**
 * @brief Set the Config object in the system's EEPROM
 * 
 * @param configMessage Reconfiguration data
 * @param mode Dictates whether message should be handled as an SPI or MQTT based config message
 */
void setConfig(const char* configMessage, const char* mode)
{
  DynamicJsonDocument currentDoc(JSON_BUFFER_CAPACITY);
  EepromStream streamFromEEPROM(0, JSON_BUFFER_CAPACITY);
  deserializeJson(currentDoc, streamFromEEPROM);
	
	
  DynamicJsonDocument configDoc(JSON_BUFFER_CAPACITY);
  DeserializationError error = deserializeJson(configDoc, configMessage);
  
  if(error)
  {
    if(strcmp(mode, "SERIAL") == 0)
      Serial.println("Error: Config string is an invalid JSON string");

    return;
  }


  docInject("IP", currentDoc, configDoc, mode);
  docInject("DNS", currentDoc, configDoc, mode);
  docInject("GATEWAY", currentDoc, configDoc, mode);
  docInject("SUBNET", currentDoc, configDoc, mode);
  docInject("MQTT", currentDoc, configDoc, mode);
  docInject("NTP", currentDoc, configDoc, mode);
  docInject("SITE", currentDoc, configDoc, mode);
  docInject("EQUIPMENTID", currentDoc, configDoc, mode);
  docInject("CLIENTID", currentDoc, configDoc, mode);
  docInject("VTHRESHOLD", currentDoc, configDoc, mode);


  EepromStream streamToEEPROM(0, JSON_BUFFER_CAPACITY);
  serializeJson(currentDoc, streamToEEPROM);
  EEPROM.commit();
  Serial.println("Committed new config information to EEPROM");
	
	
  if( strcmp(mode, "MQTT") == 0)
    reset();
}



////////////////////Interrupt Functions////////////////////

/**
 * @brief Function to attempt reconnect to MQTT broker
 * 
 */
void reconnect()
{
  Serial.println("Attempting MQTT connection...");

  while(!mqttClient.connected())
  {
    if(mqttClient.connect(globalClientID.c_str()))
    {
      if (mqttClient.connect(globalClientID.c_str(), MQTT_USERNAME, MQTT_PASSWORD))
      { 
        Serial.println("Connected");	
        mqttClient.subscribe(subscribeTopic.c_str());
	pingCommandReceived = true; //Ensures init ping is sent to broker now that a new connection has been established
      } 
      else
      {
        Serial.print("Failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println("Trying again in 5 seconds");
        delay(5000);
      }
    }
  }
}


/**
 * @brief Hard reset via WDT interrupt. Called during reconfiguration
 * 
 */
void reset()
{
  delay(100); //Allows previous operations to complete
  ESP.restart();
  delay(100);
}



String ipToString(IPAddress ip)
{
  return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}


/**
 * @brief Builds sytem diagnostic ping message to send over MQTT when prompted
 * 
 * @param object Network params
 * @return String of diagnostic info
 */
String generatePing(NetworkObject object)
{
  String pingMessage = "{\"TIME\":\"" + getTime() + "\"," +
                        "\"VERSION\":\"" + VERSION + "\"," +
                        "\"IP\":\"" + ipToString(object.getClientIP()) + "\"," +
                        "\"DNS\":\"" + ipToString(object.getClientDNS()) + "\"," +
                        "\"GATEWAY\":\"" + ipToString(object.getClientGateway()) + "\"," +
                        "\"SUBNET\":\"" + ipToString(object.getClientSubnet()) + "\"," +
	  		"\"MQTT\":\"" + ipToString(object.getMQTTAddress()) + "\"," +
	  		"\"NTP\":\"" + ipToString(globalNTPAddress) + "\"," +
                        "\"SITE\":\"" + object.getSite() + "\"," +
                        "\"EQUIPMENTID\":\"" + object.getEquipmentID() + "\"," +
                        "\"CLIENTID\":\"" + globalClientID + "\"," +
                        "\"VTHRESHOLD\":\"" + String(globalVoltageThreshold,1) + "\"}";
  return pingMessage;
}


/**
 * @brief Parses MQTT messages and determines appropraite callback response
 * 
 * @param topic 
 * @param payload 
 * @param length 
 */
static void callback(char* topic, byte* payload, unsigned int length)
{
  DynamicJsonDocument root(JSON_BUFFER_CAPACITY);
  DeserializationError error = deserializeJson(root, (const char*)payload, length);
  
  if(error)
  {
    mqtt.publish(publishTopicInfo.c_str(), "Error: Message is an invalid JSON string");
    return;
  }
	
  const char* CMD = root["CMD"];
    
  if(CMD)
  {
    if (strcmp(CMD, "CNFG") == 0)
    {
      mqttClient.publish(publishTopicInfo.c_str(), "Reconfiguring device");
      
      String configMessage;
      serializeJson(root["CNFG"], configMessage);
	    
      setConfig(configMessage.c_str(), "MQTT");
    }
    
    else if (strcmp(CMD, "RST") == 0)
    {
      mqttClient.publish(publishTopicInfo.c_str(), "Resetting device");
      
      reset();
    }
    
    else if (strcmp(CMD, "PNG") == 0)
    {      
      pingCommandReceived = true;
    }
    
    else
    {
      mqttClient.publish(publishTopicInfo.c_str(), "Error: CMD is invalid");
    }
  }	
}



////////////////////Measurement Functions////////////////////

/**
 * @brief Get the Voltage reading off analog I/O
 * 
 * @return String 
 */
String getVoltage()
{
  float rawVoltage = analogRead(VPIN);
  float voltage = rawVoltage;//(0.0409)*rawVoltage-71.336; //Function to be changed
  return String(voltage,1);
}


/**
 * @brief Get the Current reading off analog I/O
 * 
 * @return String 
 */
String getCurrent()
{
  float rawCurrent = analogRead(CPIN);
  float current = rawCurrent;//(0.0173)*rawCurrent - 29.195 + 0.75; //Function to be changed
  return String(current,1);
}


/**
 * @brief Get the Time object via NTP
 * 
 * @return String 
 */
String getTime()
{
  char bufferT[23];

  time_t currentTime = now();

  //Record "milliseconds" according to whether or not this measurement occurs in the same second as the previous one
  if(currentTime == previousTime)
    globalTimeCounter+=1;
  else
    globalTimeCounter = 0;
  
  previousTime = currentTime;

  snprintf(bufferT, sizeof(bufferT), "%4hu-%02hu-%02hu %02hu:%02hu:%02hu %02hu\0",
	   year(currentTime), month(currentTime), day(currentTime),
	   hour(currentTime), minute(currentTime), second(currentTime), globalTimeCounter);

  return String(bufferT);
}


/**
 * @brief Builds measurement string to add to circular queue event capture object
 * 
 * @return String 
 */
String generateEntry()
{
  return "{\"Time\":\"" + getTime() + "\",\"Voltage\":" + getVoltage() + ",\"Current\":" + getCurrent() + "}";
}



////////////////////NTP Functions////////////////////

/**
 * @brief Gets benchmark time during system boot/reboot
 * 
 * @return time_t 
 */
time_t getTimeBenchmark()
{
  time_t timeValue = 0;

  byte ntpMessageBuffer[NTP_MESSAGE_SIZE]; //Holds incoming/outgoing NTP messages. NTP time message is 48 bytes long

  memset(ntpMessageBuffer, 0, NTP_MESSAGE_SIZE); //Set all bytes in timeMessageBuffer to 0

  //Create the NTP request message
  ntpMessageBuffer[0] = 0b11100011;  // LI, Version, Mode
  ntpMessageBuffer[1] = 0;           // Stratum, or type of clock
  ntpMessageBuffer[2] = 6;           // Polling Interval
  ntpMessageBuffer[3] = 0xEC;        // Peer Clock Precision
  //Array index 4 to 11 is left unchanged - 8 bytes of zero for Root Delay & Root Dispersion
  ntpMessageBuffer[12] = 49;
  ntpMessageBuffer[13] = 0x4E;
  ntpMessageBuffer[14] = 49;
  ntpMessageBuffer[15] = 52;

  //Send timeMessageBuffer to NTP server via UDP at port 123
  ethernetUDP.beginPacket(globalNTPAddress, UDP_PORT); //getNTP is a method of the NetworkObject class
  ethernetUDP.write(ntpMessageBuffer, NTP_MESSAGE_SIZE);
  ethernetUDP.endPacket();

  uint32_t beginWait = millis(); //Wait for response

  while (millis() - beginWait < 1500)
  {
    if(ethernetUDP.parsePacket() >= NTP_MESSAGE_SIZE)
    {
      ethernetUDP.read(ntpMessageBuffer, NTP_MESSAGE_SIZE);
        
      unsigned long highWord = word(ntpMessageBuffer[40], ntpMessageBuffer[41]);
      unsigned long lowWord = word(ntpMessageBuffer[42], ntpMessageBuffer[43]);
        
      unsigned long secsSince1900 = highWord << 16 | lowWord;
            
      timeValue = secsSince1900 - 2208988800UL + (TIMEZONE*3600);
    }
  }
    
  return timeValue; 
}


/**
 * @brief Initializes NTP connection
 * 
 */
void ntpInit()
{
  ethernetUDP.begin(UDP_PORT);
  setSyncProvider(getTimeBenchmark);
}
