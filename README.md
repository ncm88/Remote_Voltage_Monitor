# Nodal-Analysis-and-Regulation-Circuit-NARC-

DISCLAIMER: Development work was versioned internally, this is the final version of my contributions to the project. All sensitive data/code has been removed and I 
received permission from my supervisors prior to publishing this.

Overview: This was the primary focus of my eight month co-op with Teck Resources Ltd. The purpose of this device was a remote voltage and current monitoring system 
communicating to an influx database over MQTT. Further code was added to allow reconfiguration of network parameters remotely and a symmetric multiprocessing approach was implemented to 
streamline sampling and communication procedures stimultaneously. The three pictures included below showcase this functionality graphically.

In addition, included below is a diagram of the final PCB used in conjunction with the board (ESP32). Voltage was measured via a high-resistance divider to scale voltage down to Analog I/O acceptable levels. A hall-effect current sensor for the DC line was used over a resistor shunt due to greater efficiency and less risk of overvolting the second Analog I/O pin. Primary overvoltage management was conducted via a relay and a capacitor-driven UPS was included in the board as well to retain MCU operation in the advent of system shutdown due to overvoltage-induced disconnect. I should clarify that I did not directly design/CAD most of the components used in the PCB but I was responsible for much of the overall board layout planning and dictation of design requirements, primarily via the suggestion of a capacitor-based UPS; I played a significant role in the UPS design via assesment of microcontroller compatibility and determination of target specifications for the device, including calculations for UPS capacitance and voltage tolerance. I also designed and built simple prototype UPS designs early on as a proof-of concept.

I hope you enjoy my work
