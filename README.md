#Project Overview

DISCLAIMER: Development work was versioned internally, this is the final version of my contributions to the project. All sensitive data/code has been removed and I 
received permission from my supervisors prior to publishing this.

Device: Remote auxillary voltage regulation and monitoring system for edge computers installed on Teck's haul trucks

Design objective: Protect edge computer power supply from transient voltage spikes of up to ~200V and report full excursion event to database via MQTT with EC acting as broker.

#Hardware Layout

Included below is a diagram of the final PCB used in conjunction with the board (ESP32) as well as a picture of the prototytpe PCB within it's enclosure. Voltage was measured via a high-resistance divider to scale voltage down to Analog I/O acceptable levels. A hall-effect current sensor for the DC line was used instead of a resistor shunt due to greater efficiency and less risk of overvolting the second Analog I/O pin. Edge Computer overvoltage management was conducted via relay and a capacitor-driven UPS was included in the board as well to retain MCU operation in the advent of system shutdowns due to overvoltage-induced disconnects. 

![pm](https://user-images.githubusercontent.com/62817066/207215054-6325a705-9be0-4183-b9c1-6a432b17dace.PNG)

![image001](https://user-images.githubusercontent.com/62817066/232631302-dcfb17cd-7354-4d48-b595-cb2305711c41.jpg)

#Remote Monitoring Functionality

We decided to take advantage of the ESP32's dual-core architecture to implement a symmetric multiprocessing approach to networking and measurement collection. In addition to an improved sampling rate, the shared FIFO structure in this approach allowed us to 'queue up' fully captured excursion events for publishing during disconnect periods while still retaining ability to take ongoing measurements and record new events.

The pictures below show the firmware control flow and the MQTT networking approach implemented respectfully. Dynamic re-configuration details are available in the .README file in the src folder

![fwLoop](https://user-images.githubusercontent.com/62817066/207207799-215c1a16-aaea-4c58-893d-5b96a06b81b5.PNG)

![netSnip](https://user-images.githubusercontent.com/62817066/207207844-887dbe89-953f-4aeb-beba-219463f6eac3.PNG)


