# Project Overview

DISCLAIMER: Development work was versioned internally, this is the final version of my contributions to the project. All sensitive data/code has been removed and I 
received permission from my supervisors prior to publishing this.

This device is designed to protect the TRAX-20 edge computer (EC) from randomly occuring line transients of up to 250V observed on various peices of heavy machinery as well reporting these transients to a centralized database. The device consists of a PCB and an ESP-32 development board inside a NEMA enclosure, the enclosure has two amphenol 25 connectors for in-line wiring with the EC. The PCB is responsible for suppressing transient spikes and providing line voltage/current data to the MCU. In the event of a transient, the MCU records the spike over a user-specified duration, caches the data, and sends the data/timestamps to an influx database via MQTT over Ethernet as soon as a valid connection is present. Additional clients can also be connected via MQTT through the edge computer for the purpose of remotely reconfiguring the board via a JSON-based reconfiguration protocol; these configuration objects can include system variablkes like the voltage report threshold, measurement interval duration or system time as well as generic network parameters like the MCU's IP/MAC addresses. In the event of an unexpected shutdown, the MCU attempts to reconnect with the last saved network information as soon as power is available. If remote configuration/communication is otherwise unavailable, the device is also reconfigurable through a serial port connection.

# Hardware Layout

Included below is a diagram of the final PCB used in conjunction with the board (ESP32) as well as a picture of the prototytpe PCB within it's enclosure. Voltage was measured via a high-resistance divider to scale voltage down to Analog I/O acceptable levels. A hall-effect current sensor for the DC line was used instead of a resistor shunt due to greater efficiency and less risk of overvolting the second Analog I/O pin. Edge Computer overvoltage management was conducted via magnetic relay and a capacitor-driven uninterruptible power supply (UPS) was included in the board as well to retain MCU operation in the advent of system shutdowns due to overvoltage-induced disconnects. 

![pm](https://user-images.githubusercontent.com/62817066/207215054-6325a705-9be0-4183-b9c1-6a432b17dace.PNG)

![image001](https://user-images.githubusercontent.com/62817066/232631302-dcfb17cd-7354-4d48-b595-cb2305711c41.jpg)

# Remote Monitoring Functionality

The ESP32's dual-core architecture was used to implement a symmetric multiprocessing approach to networking and measurement collection. The system works by running a collection thread and a network thread. The measurement thread keeps a circular buffer of the last QUEUE_RANGE measurements; when an excursion is detected, OVERRIDE_RANGE more measurements are taken to capture the full spike and the filled buffer is copied to a shared FIFO resource. If the network thread detects that the FIFO is unlocked and populated, it sends the measurement data over MQTT as soon as a valid connection is present, removing the readings from the FIFO as it does so. In addition to an improved sampling rate, the shared FIFO structure in this approach allowed us to 'queue up' fully captured excursion events for publishing during disconnect periods while still retaining ability to take ongoing measurements and record new events.

The pictures below show the firmware control flow and the MQTT networking approach implemented respectfully. Dynamic re-configuration details are available in the .README file in the src folder

![fwLoop](https://user-images.githubusercontent.com/62817066/207207799-215c1a16-aaea-4c58-893d-5b96a06b81b5.PNG)

![netSnip](https://user-images.githubusercontent.com/62817066/207207844-887dbe89-953f-4aeb-beba-219463f6eac3.PNG)


