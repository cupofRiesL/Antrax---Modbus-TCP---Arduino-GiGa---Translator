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
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <ArduinoMDNS.h>
#include <ArduinoModbus.h>
#include "RPC.h"

//int count;
//int overrunns;

uint16_t regValue = 0;
uint16_t oldregValue = 0;

byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x2F, 0x26 };

EthernetUDP udp;
MDNS mdns(udp);

EthernetServer ethServer(502);
ModbusTCPServer modbusServer;


void setup() {
  RPC.begin();
  Serial.begin(115200);
  //RPC.bind("printcount", printcount);


  // Clear Serial Terminal
  for (uint8_t i = 255; i > 0; i--) {
    Serial.println();
  }
  Serial.write(27);     // ESC command
  Serial.print("[2J");  // Clear screen
  Serial.write(27);
  Serial.print("[H");  // Cursor to home
  Serial.println();
  //This does not work in the Serial Terminal of the Arduino IDE

  delay(1000);

  // Start Ethernet
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Serial.println("falling back to static IP");
    IPAddress ip(192, 168, 1, 9);
    IPAddress dns(192, 168, 1, 3);
    IPAddress gateway(192, 168, 1, 3);
    IPAddress subnet(255, 255, 255, 0);
    Ethernet.begin(mac, ip, dns, gateway, subnet);

    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.");
      while (true) {
        delay(1);
      }
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
  }

  // Start Modbus TCP Server
  Serial.println("Starting Modbus TCP Server!");
  ethServer.begin();
  if (!modbusServer.begin()) {
    Serial.println("Failed to start Modbus TCP Server!");
    while (1)
      ;
  }
  modbusServer.configureHoldingRegisters(0x00, 1);  // Adjusted to 0x00 to match deployment instructions

  // Start mDNS
  Serial.println("Starting MDNS Advertisement!");
  mdns.begin(Ethernet.localIP(), "giga-modbus");
  mdns.addServiceRecord("GiGA Anthrax Modbus TCP Translator", 502, MDNSServiceTCP);

  // Print network info
  Serial.println("INIT Finished!");
  Serial.print("  MAC: ");
  for (int i = 0; i < 5; i++) {
    Serial.print(mac[i], HEX);
    Serial.print(":");
  }
  Serial.println(mac[5], HEX);
  Serial.print("  IP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("  DNS: ");
  Serial.println(Ethernet.dnsServerIP());
  Serial.print("  GATEWAY: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("  MASK: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("  STATUS: ");
  Serial.println(Ethernet.linkStatus());
  Serial.println("Modbus TCP + mDNS running. Reach me on giga-modbus.local:502");
}

void loop() {
  mdns.run();
  EthernetClient client = ethServer.available();
  if (client) {
    modbusServer.accept(client);
  }

  modbusServer.poll();
  updateREGs();
}

void updateREGs() {

  oldregValue = regValue;
  //regValue++;
  oldregValue = regValue;
  regValue = modbusServer.holdingRegisterRead(0x00);
  modbusServer.holdingRegisterWrite(0x00, regValue);
  if (regValue != oldregValue) {
    Serial.print("ValUpdate: ");
    Serial.println(regValue);
    RPC.println(regValue);  // print the register value to the M4
    Serial.println("Value transfered via RPC");
  }
}