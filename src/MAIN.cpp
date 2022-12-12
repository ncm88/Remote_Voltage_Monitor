#include "externals.h"
#include "config.h"
#include "Queue.h"



////////////////////Tasks////////////////////

TaskHandle_t MQTT_TASK_HANDLE;
TaskHandle_t VTC_TASK_HANDLE;



/* FUNCTION NAME: MQTT Task
 * PURPOSE: Maintains connection and communication with MQTT broker
 * ACTION: Initiates and maintains connection to broker, publishes data to broker (from shared resource),
 *         publishes information to broker (following a ping request), and listens for callback messages from broker
 */
void MQTT_TASK(void* pvParameters)
{
  NetworkObject networkHandler = loadConfig();

  
  //Displays config information to Serial
  Serial.print("\nNetwork Info:");
  Serial.print("\nClient IP: ");
  Serial.print(networkHandler.getClientIP());
  Serial.print("\nClient DNS: ");
  Serial.print(networkHandler.getClientDNS());
  Serial.print("\nClient Gateway: ");
  Serial.print(networkHandler.getClientGateway());
  Serial.print("\nClient Subnet: ");
  Serial.print(networkHandler.getClientSubnet());
  Serial.print("\nMQTT Broker Address: ");
  Serial.print(networkHandler.getMQTTAddress());
  Serial.print("\nNTP Server Address: ");
  Serial.print(globalNTPAddress);
  Serial.print("\nSite: ");
  Serial.print(networkHandler.getSite());
  Serial.print("\nEquipment ID: ");
  Serial.print(networkHandler.getEquipmentID());
  Serial.print("\nThreshold Voltage: ");
  Serial.print(globalVoltageThreshold);
  Serial.print("\nClient ID: ");
  Serial.print(globalClientID);
  Serial.print("\n");
  
  
  
  while (true)
  {
    if(!mqttClient.connected())
      reconnect();
    
    if(pthread_mutex_trylock(&mutexHandle) == 0)  //The mutex attempts to lock the shared resource unless VTC_TASK is already operating on it
    {
      //Publishes all data in shared resource at once
      while(softCopy.count()!=0)
        mqttClient.publish(publishTopicData.c_str(), softCopy.pop().c_str());
      pthread_mutex_unlock(&mutexHandle);
    }
    
    if(pingCommandReceived)
    {
      String ping = generatePing(networkHandler);
      mqttClient.publish(publishTopicInfo.c_str(), ping.c_str());
      pingCommandReceived = false;
    }

    mqttClient.loop();
  }
  
}



/* FUNCTION NAME: VTC Task
 * PURPOSE: Continuously takes in measurements and stores them on SRAM
 * ACTION: Continuously gathers measurements, formats them into JSON data strings, and pushes them into the primary rolling queue.
 *         When a voltage excursion occurs, override occurs, and the proceeding queue is copied to the shared resource
 */
void VTC_TASK(void* pvParameters)
{
  while(true)
  {
    float testVoltage = getVoltage().toFloat();

    if(testVoltage > globalVoltageThreshold)
    {
      //Override occurs, meaning measurements are continuously recorded to capture as much of the transient as needed within the primary queue
      for(int i = 0; i<OVERRIDE_RANGE; i++)
      {
        dataSet.push(generateEntry());
      }

      if(pthread_mutex_lock(&mutexHandle) == 0)  //The mutex locks the shared resource or waits until the resource is available to lock it
      {
        dataSet.copy(&softCopy);  //Copies primary queue to shared resource
        pthread_mutex_unlock(&mutexHandle);
      }
    }

    //At this point, the measurement is considered trivial, but it is still recorded in the case of an excursion later on
    else
    {
      dataSet.push(generateEntry());
    }
    
  }
  
}



////////////////////Start of Program////////////////////

void setup()
{
  Serial.begin(115200);  //Serial init
  Serial.println("Starting...");
  EEPROM.begin(4096); //Max amount of allocatable EEPROM memory on esp32
  pthread_mutex_init(&mutexHandle, NULL);  //Mutex handle init


  Serial.println("Do you want to change any config information? (Y/N)");
  Serial.setTimeout(8000);  //Wait 8 seconds for a response if user is connected to serial
  String answer = Serial.readStringUntil('\n');

  //Serial waits for user to enter a config string (in JSON format) 
  if(answer == "Y" || answer == "y")
  {
    Serial.println("Enter new config information as needed");
    
    String configMessage;
    while(true)
    {
      if(Serial.available())
      {
        configMessage = Serial.readStringUntil('\n');
        break;
      }
    }
    
    setConfig(configMessage.c_str(), "SERIAL");
  }
  
  else
  {
    Serial.println("Ok, nvm\n");
  }


  //MQTT task pinned to core 0
  xTaskCreatePinnedToCore( MQTT_TASK,            //Task function
                           "MQTT",               //Name of task
                           30000,                //Stack size of task
                           NULL,                 //Parameter of the task
                           0,                    //Priority of the task
                           &MQTT_TASK_HANDLE,    //Task handle for keeping track of task
                           0                     //Core that task is pinned to
                         );
  delay(500);


  //VTC task pinned to core 1
  xTaskCreatePinnedToCore( VTC_TASK,            //Task function
                           "VTC",               //Name of task
                           30000,               //Stack size of task
                           NULL,                //Parameter of the task
                           1,                   //Priority of the task
                           &VTC_TASK_HANDLE,    //Task handle for keeping track of task
                           1                    //Core that task is pinned to
                         );
  delay(500);
}



//Loop is empty since everything runs between the two tasks
void loop() {}
