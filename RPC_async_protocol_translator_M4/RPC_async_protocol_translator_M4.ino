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


#include "Arduino.h"
#include "RPC.h"

using namespace rtos;

Thread requestOutChangeThread;

uint16_t Register0Value = 0;
uint16_t ch[10];
uint16_t old_ch[10];
int LED = 86;

void setup() {
  RPC.begin();
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  Serial2.begin(9600);  // First MultiBox
  Serial3.begin(9600);  // Second MultiBox
  Serial2.println("{+}Modus=2");
  Serial3.println("{+}Modus=2");

  /*
  The GiGa comes with a Naming shift in the UART "Serial" Ports
  Serial = Serial1 = SerialUSB
  so:
  HW Serial1 is actually Serial2 in SW
  HW Serial2 is actually Serial3 in SW
  HW Serial3 is actually Serial4 in SW
  */

  requestOutChangeThread.start(requestOutChange);
}

void loop() {

  String buffer = "";
  while (RPC.available()) {
    buffer += (char)RPC.read();  // Fill the buffer with characters
  }
  if (buffer.length() > 0) {
    Register0Value = buffer.toInt();
    requestOutChange();
  }
}

void requestOutChange() {

  digitalWrite(LED, HIGH);
  //Serial2.println(Register0Value);

/*The following code is absolutely overkill in terms of speed but I wanted to solve this Problem in this way.
  I know that the Bottleneck is the 9600 Bauds on the UART but I like the code I wrote to sort the communication*/


  for (uint8_t i = 0; i <= 9; i++) {
    ch[i] = Register0Value >> i;
    /*  assign the value of the Register to the channel shifted by the number of turns through the loop
like:
Register0Value  = 0000001000000000

ch[0]           = 0000001000000000
ch[1]           = 0000000100000000
ch[2]           = 0000000010000000
ch[3]           = 0000000001000000
ch[4]           = 0000000000100000
ch[5]           = 0000000000010000
ch[6]           = 0000000000001000
ch[7]           = 0000000000000100
ch[8]           = 0000000000000010
ch[9]           = 0000000000000001

*/
    ch[i] = ch[i] & 0000000000000001;
    /* mask all bits out except the LSB
like:
ch[0]           = 0000000000000000
ch[1]           = 0000000000000000
ch[2]           = 0000000000000000
ch[3]           = 0000000000000000
ch[4]           = 0000000000000000
ch[5]           = 0000000000000000
ch[6]           = 0000000000000000
ch[7]           = 0000000000000000
ch[8]           = 0000000000000000
ch[9]           = 0000000000000001

*/
    if (ch[i] != old_ch[i]) {  // check for change
      switch (ch[i]) {         // switch by on or off
        case 0:                // in case the channel is set to off:
          switch (i) {         // switch by channel
            case 0:
              //construct controlstring here ch0 off
              Serial2.println("{1}off");
              break;
            case 1:
              //construct controlstring here ch1 off
              Serial2.println("{2}off");
              break;
            case 2:
              //construct controlstring here ch2 off
              Serial2.println("{3}off");
              break;
            case 3:
              //construct controlstring here ch3 off
              Serial2.println("{4}off");
              break;
            case 4:
              //construct controlstring here ch4 off
              Serial2.println("{5}off");
              break;
            case 5:
              //construct controlstring here ch5 off
              Serial3.println("{1}off");
              break;
            case 6:
              //construct controlstring here ch6 off
              Serial3.println("{2}off");
              break;
            case 7:
              //construct controlstring here ch7 off
              Serial3.println("{3}off");
              break;
            case 8:
              //construct controlstring here ch8 off
              Serial3.println("{4}off");
              break;
            case 9:
              //construct controlstring here ch9 off
              Serial3.println("{5}off");
              break;
          }
          break;
        case 1:         // in case the channel is set to on:
          switch (i) {  // switch by channel again
            case 0:
              //construct controlstring here ch0 on
              Serial2.println("{1}on");
              break;
            case 1:
              //construct controlstring here ch1 on
              Serial2.println("{2}on");
              break;
            case 2:
              //construct controlstring here ch2 on
              Serial2.println("{3}on");
              break;
            case 3:
              //construct controlstring here ch3 on
              Serial2.println("{4}on");
              break;
            case 4:
              //construct controlstring here ch4 on
              Serial2.println("{5}on");
              break;
            case 5:
              //construct controlstring here ch5 on
              Serial3.println("{1}on");
              break;
            case 6:
              //construct controlstring here ch6 on
              Serial3.println("{2}on");
              break;
            case 7:
              //construct controlstring here ch7 on
              Serial3.println("{3}on");
              break;
            case 8:
              //construct controlstring here ch8 on
              Serial3.println("{4}on");
              break;
            case 9:
              //construct controlstring here ch9 on
              Serial3.println("{5}on");
              break;
          }
          break;
      }
    }
    old_ch[i] = ch[i];
  }



  digitalWrite(LED, LOW);
}