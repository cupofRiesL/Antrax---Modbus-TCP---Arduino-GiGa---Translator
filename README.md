# Antrax---Modbus-TCP---Arduino-GiGa---Translator
As the name says, it's a protocoll translator between ModbusTCP and the propietary protocoll of the Antrax multibox Pro



/*#############################################################################################

I tried to let Copilot write this Sketch. I tried to let ChatGPT and Grok correct it afterwards. In the end I had to completly rewrite it by Hand.
So this is not vibe coded and I'm really testing it. 
Leonard Ries January 2026

How to Deploy

    Get an Arduino Giga R1, an Ethernet Shield two and a SparkFun MAX3232
    https://store.arduino.cc/products/giga-r1-wifi
    https://store.arduino.cc/products/arduino-ethernet-shield-2?queryID=undefined
    https://www.sparkfun.com/sparkfun-transceiver-breakout-max3232.html

    In Arduino IDE:

        Set Flash Split to 1.5MB M7 + 0.5MB M4

        Upload M7 sketch with Target Core: Main Core

        Upload M4 sketch with Target Core: M4 Co-processor

    Use avahi-browse -a or ping giga-modbus.local to verify mDNS

    Use a Modbus TCP client to read holding register 0x01 

###############################################################################################

This Sketch(es) are a protocol adapter between Modbus TCP and 
two Antrax Multibox Pro (https://www.antrax.de/produkt/multibox-pro-seri/)

You need 10 cheap compliant sockets in your automation Project? This is how I solved this.

I used Open ModScan on ubuntu for debugging.

Device ID is 1 
Start Address is 0
MODBUS Point Type is 03: HOLDING REGISTER

I implemented DHCP with a fallback static address and mdns, RPC to talk to the M4 Co-Processor

The M7 is doing all the Heavie lifting while the M4 is doing the slow writes to the 
Antrax devices.

I used RPC asynchronous here.

[Trigger warning: The code is not nice and might have issues but its working. 
                                                         There might be leftovers from Testing]
#############################################################################################*/
